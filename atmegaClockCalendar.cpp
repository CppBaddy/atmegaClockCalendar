#include "atmegaClock.hpp"

#include <avr/wdt.h>
#include <avr/sleep.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#include "i2c.h"
#include "rtc3231.h"

#include "display.h"
#include "digits.h"
#include "fonts/font.h"

#include "RotaryEncoder.hpp"

#define delay_ms(x)	_delay_ms(x)


enum eUserInput
{
    eHours = 1,
    eMinutes,
    eDisplay,
};

enum ePower
{
    PowerFull,
    PowerSave,
    PowerSleep
};

static const uint8_t weekDays[7][12] = {
        "\302\356\361\352\360\345\361\345\355\374\345", //"Воскресенье",
        "\317\356\355\345\344\345\353\374\355\350\352", //"Понедельник",
        "\302\362\356\360\355\350\352    ",             //"Вторник    ",
        "\321\360\345\344\340      ",                   //"Среда      ",
        "\327\345\362\342\345\360\343    ",             //"Четверг    ",
        "\317\377\362\355\350\366\340    ",             //"Пятница    ",
        "\321\363\341\341\356\362\340    ",             //"Суббота    "
};

//static const uint8_t weekDays[7][10] = {
//        "Sunday",
//        "Monday",
//        "Tuesday",
//        "Wednesday",
//        "Thursday",
//        "Friday",
//        "Saturday"
//};

static const uint8_t monthNames[12][10] = {
        "\337\355\342\340\360\374",         //"Январь",
        "\324\345\342\360\340\353\374",     //"Февраль",
        "\314\340\360\362",                 //"Март",
        "\300\357\360\345\353\374",         //"Апрель",
        "\314\340\351",                     //"Май",
        "\310\376\355\374",                 //"Июнь",
        "\310\376\353\374",                 //"Июль",
        "\300\342\343\363\361\362",         //"Август",
        "\321\345\355\362\377\341\360\374", //"Сентябрь",
        "\316\352\362\377\341\360\374",     //"Октябрь",
        "\315\356\377\341\360\374",         //"Ноябрь",
        "\304\345\352\340\341\360\374",     //"Декабрь"
};

static const uint8_t sMonthDays[] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

//static const uint8_t monthNames[12][10] = {
//        "January",
//        "February",
//        "March",
//        "April",
//        "May",
//        "June",
//        "July",
//        "August",
//        "September",
//        "October",
//        "November",
//        "December"
//};

struct rtc_time gTime;
struct rtc_date gDate;
struct temp_t gTemp;

struct rtc_time gTimeSetup;
struct rtc_date gDateSetup;

volatile static uint8_t timeChangedFlag;

volatile static uint8_t userInputFlag;
volatile static uint8_t userInput;
volatile static uint8_t prevInput;

volatile static uint8_t gTimeout;
volatile static uint8_t gLongPressTimer;
yr::RotaryEncoder gEncoder;

volatile static uint8_t avgCounter;
volatile static uint8_t inputBuf;

volatile static uint8_t adcState = 1;

volatile static uint8_t gBatteryVoltage = BatteryFull;

static uint8_t gPowerSaveMode;
static uint8_t mBrightness;
static uint8_t gMaxBrightness = MaxBrightness;


/* INT0 ext interrupt handler */
//ISR( INT0_vect )
//{}

/* INT1 ext interrupt handler */
//ISR( INT1_vect )
//{}

/* PCI2 pin change handler */
ISR( PCINT2_vect )
{
    inputBuf = Encoder_Read();
    avgCounter = InputAveraging;

    Encoder_Disable();
    Timer0_Enable();
}

/* Timer0 interrupt handler */
//ISR( TIMER0_OVF_vect )
//{}

/* Timer0 interrupt handler */
ISR( TIMER0_COMPA_vect ) //2KHz
{
    if(avgCounter)
    {
        --avgCounter;

        uint8_t val = Encoder_Read();

        if(inputBuf != val)
        {
            avgCounter = InputAveraging;
            inputBuf = val;
        }
        else if(0 == avgCounter)
        {
            userInput = inputBuf;

            uint8_t change = prevInput ^ userInput;

            //TODO process inputs
            if((_BV(PD4) | _BV(PD5)) & change)  //encoder moved
            {
                uint8_t i = ((_BV(PD4) | _BV(PD5)) & prevInput) >> 2;
                i |= ((_BV(PD4) | _BV(PD5)) & userInput) >> 4;

                if(/*0b0010 == i ||*/ 0b1101 == i) // x1 impl
                {
                    gEncoder.dec();
                }
                else if(0b1000 == i /*|| 0b0111 == i*/)
                {
                    gEncoder.inc();
                }
            }

            if(_BV(PD6) & change) //Push button event
            {
                if(_BV(PD6) & userInput)
                {
                    gLongPressTimer = 0;
                }
                else
                {
                    gEncoder.pressed();
                    gLongPressTimer = LongHoldButtonSec;
                }
            }

            prevInput = userInput;

            userInputFlag = true;
        }
    }

    if(0 == avgCounter)
    {
        Timer0_Disable();
        Encoder_Enable();
    }
}

/* Timer1 interrupt handler */
ISR( TIMER1_COMPA_vect ) // 1Hz
{
    timeChangedFlag = 1;

    if(gLongPressTimer)
    {
        --gLongPressTimer;

        if(0 == gLongPressTimer)
        {
            gEncoder.longPressed();
            userInputFlag = true;
        }
    }

    if(PowerSleep != gPowerSaveMode && gTimeout)
    {
        if(gTimeout < SetupIdleTimeout)
        {
            ++gTimeout;
        }
    }

    ADC_Start();
}

/* Timer2 interrupt handler */
//ISR( TIMER2_OVF_vect ) //wakes up each second
//{}

//ISR( TIMER0_COMPB_vect )
//{}

/* Timer1 interrupt handler */
//ISR( TIMER1_OVF_vect )
//{}

/* Timer1 interrupt handler */
//ISR( TIMER1_COMPB_vect )
//{}

// ADC interrupt service routine
ISR( ADC_vect )
{
	if(adcState)
	{
		uint8_t v = ~(ADCH + 96); //converting to positive slope

		gBatteryVoltage = (gBatteryVoltage + v) >> 1; //averaging

		if(PowerSleep != gPowerSaveMode)
		{
			AdcIn_Light();
            LightSensor_On();

			adcState = 0;
		}
	}
	else
	{
		uint8_t v = (ADCH >> 1) + MinBrightness;

		if(v > gMaxBrightness) //clamp value
		{
		    v = gMaxBrightness;
		}

		mBrightness = v; //TODO TEMP DEBUG

		Display_Brighness(v);

		AdcIn_Voltage();
        LightSensor_Off();

		adcState = 1;
	}

	ADC_Disable();
}


void onPowerFull()
{
	gPowerSaveMode = PowerFull;
    gMaxBrightness = 255;
}

void onPowerSave()
{
    gPowerSaveMode = PowerSave;
    gMaxBrightness = PowerSaveBrightness;

    Display_On();
	Encoder_Enable();
}

void onPowerSleep()
{
	gPowerSaveMode = PowerSleep;

    Encoder_Disable();
    Display_Off();
}

enum
{
    Setup_Year,
    Setup_Month,
    Setup_Day,
    Setup_WeekDay,
    Setup_Hour,
    Setup_Minute,
    Setup_Second
};

static uint8_t gSetupMode;

void onDateTimeSetup()
{
    gDateSetup = gDate;
    gTimeSetup = gTime;

    gSetupMode = Setup_Year;
    gEncoder.setMax(99);
    gEncoder.setValue(gDateSetup.year);

    gTimeout = 1;
}

void DisplaySetup();

bool DateTimeSetup()
{
    uint8_t val = gEncoder.getValue();
    int8_t btn = gEncoder.getButton();
    bool ret = false;

    switch(gSetupMode)
    {
        case Setup_Year:
            gDateSetup.year = val;

            if(yr::Btn_Pressed == btn)
            {
                gSetupMode = Setup_Month;
                gEncoder.setMax(12-1);
                gEncoder.setValue(gDateSetup.month - 1);
            }
            break;
        case Setup_Month:
            gDateSetup.month = val + 1;

            if(yr::Btn_Pressed == btn)
            {
                gSetupMode = Setup_Day;

                uint8_t days =  sMonthDays[gDateSetup.month - 1];

                if(2 == gDateSetup.month && 0 == gDateSetup.year % 4)
                {
                    ++days; //Feb in leap year
                }

                gEncoder.setMax(days - 1);
                gEncoder.setValue(gDateSetup.day - 1);
            }
            break;
        case Setup_Day:
            gDateSetup.day = val + 1;

            if(yr::Btn_Pressed == btn)
            {
                gSetupMode = Setup_WeekDay;
                gEncoder.setMax(6);
                gEncoder.setValue(gDateSetup.wday - 1);
            }
            break;
        case Setup_WeekDay:
            gDateSetup.wday = val + 1;

            if(yr::Btn_Pressed == btn)
            {
                gSetupMode = Setup_Hour;
                gEncoder.setMax(23);
                gEncoder.setValue(gTimeSetup.hour);
            }
            break;
        case Setup_Hour:
            gTimeSetup.hour = val;

            if(yr::Btn_Pressed == btn)
            {
                gSetupMode = Setup_Minute;
                gEncoder.setMax(59);
                gEncoder.setValue(gTimeSetup.min);
            }
            break;
        case Setup_Minute:
            gTimeSetup.min = val;

            if(yr::Btn_Pressed == btn)
            {
                gSetupMode = Setup_Second;
                gEncoder.setValue(0);
                gTimeSetup.sec = 0;
                ret = true;
            }
            break;
        default:
            break;
    }

    DisplaySetup();

    return ret;
}

void DS3231_Update()
{
    rtc3231_write_time(&gTimeSetup);
    rtc3231_write_date(&gDateSetup);
}

void DS3231_Read()
{
    rtc3231_read_datetime(&gTime, &gDate);
    rtc3231_read_temperature(&gTemp);
}

void PowerUpdate()     //check power save mode
{
    switch(gPowerSaveMode)
    {
    case PowerFull:
        if(gBatteryVoltage < BatteryNormal)
        {
            onPowerSave();
        }
        break;

    case PowerSave:
        if(gBatteryVoltage < BatteryDischarging)
        {
            onPowerSleep();
        }
        else if(gBatteryVoltage > BatteryNormal)
        {
            onPowerFull();
        }
        break;

    case PowerSleep:
        if(gBatteryVoltage > BatteryDischarging)
        {
            onPowerSave();
        }
        break;

    default:
        gPowerSaveMode = PowerFull;
        break;
    }
}

inline void DisplayDateTime()
{
    DisplayTime();
    DisplayDate();
}

//TODO  
//	add chime at noon, short beep each hour from 8:00 to 20:00

int main( void )
{
    Setup(); //setup avr hardware
    
    i2c_init();

    rtc3231_init();

    uint8_t timeSec = 4;

    uint8_t mode = ModeNormal;

    for(;;)
    {
        bool displayShow = false;

        if(userInputFlag) //buttons changed their state
        {
            userInputFlag = 0;
            gEncoder.Update();

            if(gTimeout)
            {
                gTimeout = 1; //restart timeout counter
            }

            if(yr::Btn_PressedLong == gEncoder.isPressed())
            {
                mode = ModeSetup;
                gEncoder.getButton();
                onDateTimeSetup();
            }

            displayShow = true;
        }

        PowerUpdate();

        if(PowerSleep != gPowerSaveMode)
        {
            switch(mode)
            {
                case ModeNormal:
                    if(timeChangedFlag)
                    {
                        timeChangedFlag = 0;

                        if(++timeSec == 5) //each 5 sec
                        {
                            DS3231_Read();

                            timeSec = gTime.sec % 5; //correct second ticks

                            DisplayDateTime();

                            displayShow = true;
                        }
                    }
                    break;
                case ModeSetup:
                    if(DateTimeSetup())
                    {
                        DS3231_Update();

                        gTime = gTimeSetup;
                        gDate = gDateSetup;

                        DisplayDateTime();
                        mode = ModeNormal;
                    }
                    else if(SetupIdleTimeout == gTimeout)
                    {
                        gTimeout = 0;
                        DisplayDateTime();
                        mode = ModeNormal;
                    }
                    break;
            }

            if(displayShow)
            {
                //DisplayTest();

                DisplayUpdate();
            }
        }

        set_sleep_mode(SLEEP_MODE_IDLE);
        sleep_mode();
    }

    return 0;
}

inline void Setup()
{
    //Setup clock frequency
    //CLKPR = _BV(CLKPCE); //enable Clock Prescale Register write
    //CLKPR = 0;           //change prescaler to 1, effectively set 8MHz system clock

    //PB0 - output + pullup
    //PB2 - output + pullup
	DDRB |= _BV(DDB5) | _BV(DDB3) | _BV(DDB2) | _BV(DDB1) | _BV(DDB0); //SPI output for LCD

	DDRC |= _BV(PC3); // | _BV(PC2) | _BV(PC1); //enable output ports
	PORTC |= _BV(PC4) | _BV(PC5); //enable pullups on SDA and SCL

	DDRD |= _BV(DDD2) | _BV(DDD3) | _BV(DDD7); //enable output ports
    PORTD |= EncoderMask | _BV(PD7);  //enabling pull ups for inputs and release LCD reset
    PCMSK2 |= EncoderMask;  //enable pin change interrupt mask

    Encoder_Enable();
    Display_On();

    DIDR0 |= _BV(ADC0D); //disable digital input on ADC0

    ADCSRA |= _BV(ADPS2) | _BV(ADPS1) | _BV(ADIE) | _BV(ADEN); //enable ADC, interrupt and set prescaler 64
    AdcIn_Voltage();

    //Timer0 2KHz for switch signal de-bouncing
    // 1953Hz = 8MHz / 1024 / 4
    TCCR0A = _BV(WGM01); //CTC
    TCCR0B |= _BV(CS02) | _BV(CS00); //prescaler 1024
    OCR0A = 4;
    Timer0_Disable();

    //Timer1 1Hz local time tick
    // 1.000064Hz = 8MHz / 1024 / 7812
    TCCR1A = 0;
    TCCR1B = _BV(WGM12) | _BV(CS12) | _BV(CS10); //CTC, prescaler 1024
    OCR1A = 7812;
    Timer1_Enable();

    //Timer2 4KHz PWM LED Brightness
    // 3906.25Hz = 8MHz / 1 / 256
    TCCR2A = _BV(COM2B1) | _BV(WGM20); //PWM phase correct mode
    TCCR2B |= _BV(CS21); //prescaler 8
    OCR2B = 128; //50% duty

    PRR |= _BV(PRUSART0) | _BV(PRADC); //disable usart and adc clocks

    sei();
}

void DisplayTime()
{
    uint8_t hh = gTime.hour / 10;
    uint8_t hl = gTime.hour % 10;

    if(0 == hh)
    {
        hh = 0; //blank if zero
    }
    
    uint8_t mh = gTime.min / 10;
    uint8_t ml = gTime.min % 10;

    uint8_t x = 20;

    digits_select_font(0);

    digit_output(hh, x);

    digit_output(hl, x + 18);

    digit_output(10, x + 36);

    digit_output(mh, x + 54);
    digit_output(ml, x + 72);
}

void DisplayTest()
{
    display_printf(0, 0, FONTID_6X8M, "%3d", gEncoder.getValue());
    display_printf(0, 10, FONTID_6X8M, "%3d", gEncoder.isPressed());
    display_printf(0, 20, FONTID_6X8M, "%3d", gBatteryVoltage);
}

void DisplaySetup()
{
    uint8_t hh = gTimeSetup.hour / 10;
    uint8_t hl = gTimeSetup.hour % 10;

    if(0 == hh)
    {
        hh = 0; //blank if zero
    }

    uint8_t mh = gTimeSetup.min / 10;
    uint8_t ml = gTimeSetup.min % 10;

    uint8_t x = 20;

    digits_select_font(0);

    if(Setup_Hour == gSetupMode)
    {
        digit_output_inv(hh, x);
        digit_output_inv(hl, x + 18);
    }
    else
    {
        digit_output(hh, x);
        digit_output(hl, x + 18);
    }

    digit_output(10, x + 36); // :

    if(Setup_Minute == gSetupMode)
    {
        digit_output_inv(mh, x + 54);
        digit_output_inv(ml, x + 72);
    }
    else
    {
        digit_output(mh, x + 54);
        digit_output(ml, x + 72);
    }

    //display_printf(108, 0, FONTID_6X8M, "%3d", gBatteryVoltage);
    //display_printf(108, 10, FONTID_6X8M, "%3d", mBrightness);

    x = 0;

    if(Setup_Year == gSetupMode)
    {
        display_printf(x, 55, FONTID_6X8M, "\b20%02d\b", gDateSetup.year);
    }
    else
    {
        display_printf(x, 55, FONTID_6X8M, "20%02d", gDateSetup.year);
    }

    x += 6*6;

    if(Setup_Month == gSetupMode)
    {
        display_printf(x, 55, FONTID_6X8M, "\b%s\b %d   ", monthNames[gDateSetup.month - 1], gDateSetup.day);
    }
    else if(Setup_Day == gSetupMode)
    {
        display_printf(x, 55, FONTID_6X8M, "%s \b%d\b   ", monthNames[gDateSetup.month - 1], gDateSetup.day);
    }
    else
    {
        display_printf(x, 55, FONTID_6X8M, "%s %d   ", monthNames[gDateSetup.month - 1], gDateSetup.day);
    }

    if(Setup_WeekDay == gSetupMode)
    {
        display_printf(23, 42, FONTID_6X8M, "\b%s\b", weekDays[gDateSetup.wday - 1]);
    }
    else
    {
        display_printf(23, 42, FONTID_6X8M, "%s", weekDays[gDateSetup.wday - 1]);
    }
}

inline void DisplayBatteryIcon()
{
    display_DrawRectangle(120, 5, 123, 6);
    display_DrawRectangle(116, 7, 127, 21);

    uint8_t level = 9 + BatteryFull - gBatteryVoltage;
    for(uint8_t y = 9; y < 20; y += 3)
    {
        uint8_t v = (y >= level);
        display_Line(118, y, 125, y, v);
        display_Line(118, y+1, 125, y+1, v);
    }
}

inline void RemoveBatteryIcon()
{
    display_FillRectangle(116, 5, 127, 21, 0);
}

void DisplayDate()
{
    if(gBatteryVoltage < BatteryFull - 1)
    {
        DisplayBatteryIcon();
    }
    else
    {
        RemoveBatteryIcon();
    }

    display_DrawString(23, 42, FONTID_6X8M, weekDays[gDate.wday - 1]);

    display_printf(128-4*6 - 1, 48, FONTID_6X8M, "%d\7C", gTemp.degrees/*, gTemp.subdegr / 10*/);

    display_printf(0, 55, FONTID_6X8M, "%2d %s 20%02d   ", gDate.day, monthNames[gDate.month - 1], gDate.year);
}

