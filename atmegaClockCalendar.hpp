#ifndef ATMEGA_CLOCK_H
#define ATMEGA_CLOCK_H

#include <stdint.h>
#include <stdbool.h>

#include <avr/io.h>

/*
Running on internal 8 MHz clock

CPU Clock Freq = 8 MHz
Main prescaler = 1

Timer0 8MHz / 1024 / 4 = 1953 Hz

Timer1 8MHz / 1024 / 7812 = 1.000064 Hz

Timer2 PWM LED light level 8MHz / 1 / 256 = 3906.25 Hz

Serial Port
PD0 : Rx
PD1 : Tx

Encoder with button
PD4 : A
PD5 : B
PD6 : Push

PD2 : Display power control
PD3/OC2B : Timer2 PWM LED light control
PD7 : Display controller reset

SPI: write only 128x64 LCD display module
PB2 : SS
PB3 : MOSI
PB5 : SCK

PC1 : +Speaker
PC2 : -Speaker
PC3 : On/Off light sensor

I2C: DS3231 time calendar module
PC4 : SDA
PC5 : SCL

ADC
PC0/ADC0 : Measure ambient light level with Vref = Vcc
ADC BG: Measure bandgap voltage 1.1V with Vref = Vcc ==> measure Vcc

SLEEP MODE
Wakeup sources
PCINT2 pin change
Timer0 2 KHz (when enabled by PCINT2 to scan inputs)
Timer1 1 Hz
Timer2 PWM LED light 4 KHz

*/

#ifndef F_CPU
    #define F_CPU   8000000
#endif

#include <avr/io.h>

#include "display.h"


#define ADC_VREF_VCC        0                           // Vcc as voltage reference
#define ADC_VREF_1V1        _BV(REFS1)                  // 1.1V internal VREF without external capacitor
#define ADC_VREF_2V56      (_BV(REFS2) | _BV(REFS1))    // 2.56V internal VREF without external capacitor

#define VBANDGAP_1v1_IN    (_BV(MUX3) | _BV(MUX2))      //0x0c 1.1V bandgap voltage source
#define TEMPERATURE_IN     (_BV(MUX3) | _BV(MUX2) | _BV(MUX1) | _BV(MUX0)) //0x07 ADC4 internal temperature

#define ADC_LEFT_JUSTIFIED  _BV(ADLAR)  //0x20 - left justified, so we can use ADCH as a 8 bit result

#define VCC_ADC		255
#define BG_VOLTAGE	11/10
#define VOLTAGE(x)	(255 - 96 - ((VCC_ADC * BG_VOLTAGE) / x)


enum
{
    EncoderMask         = (_BV(PD4) | _BV(PD5) | _BV(PD6)),

    InputAveraging      = 3,

    None                = 0,

    BatteryFull         = 92, // 4.2V
    BatteryNormal       = 88, // 4.0V
    BatteryDischarging  = 83, // 3.7V

    BatteryFullCapacity = BatteryFull - BatteryDischarging + 1,

    MinBrightness       = 10,
    MaxBrightness       = 128,
    PowerSaveBrightness = 20,

    LongHoldButtonSec   = 5,

    ModeNormal,
    ModeSetup,

    SetupIdleTimeout = 90,
};


void Setup();

void DisplayTest();

void DisplayTime();
void DisplayDate();

inline void DisplayUpdate()
{
    display_UpdateFromBuff();
}

inline void Timer0_Enable()
{
    PRR &= ~_BV(PRTIM0); //clock enable
    TIFR0 |= _BV(OCF0A);
    TIMSK0 |= _BV(OCIE0A);
}

inline void Timer0_Disable()
{
    TIMSK0 &= ~_BV(OCIE0A);
	PRR |= _BV(PRTIM0); //clock disable
}

inline void Timer1_Enable()
{
    PRR &= ~_BV(PRTIM1); //clock enable
    TIFR1 |= _BV(OCF1A);
    TIMSK1 |= _BV(OCIE1A);
}

inline void Timer1_Disable()
{
    TIMSK1 &= ~_BV(OCIE1A);
    PRR |= _BV(PRTIM1); //clock disable
}

inline void Timer2_Enable()
{
    PRR &= ~_BV(PRTIM2); //clock enable
    TIFR2 |= _BV(OCF2A);
    TIMSK2 |= _BV(OCIE2A);
}

inline void Timer2_Disable()
{
    TIMSK2 &= ~_BV(OCIE2A);
    PRR |= _BV(PRTIM2); //clock disable
}

inline void ADC_Enable()
{
	PRR &= ~_BV(PRADC);
}

inline void ADC_Disable()
{
	PRR |= _BV(PRADC);
}

inline void ADC_Start()
{
	ADC_Enable();
	ADCSRA |= _BV(ADSC);
}

inline void AdcIn_Voltage()
{
	ADMUX = _BV(REFS0) | _BV(ADLAR) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
}

inline void AdcIn_Light()
{
	ADMUX = _BV(REFS0) | _BV(ADLAR);
}

inline void LightSensor_On()
{
    PORTC |= _BV(PC3);
}

inline void LightSensor_Off()
{
    PORTC &= ~_BV(PC3);
}

inline void Encoder_Enable()
{
    PCIFR |= _BV(PCIF2); //clear int flag
	PCICR |= _BV(PCIE2); //enable pin change int
}

inline void Encoder_Disable()
{
	PCICR &= ~_BV(PCIE2); //disable pin change int
}

inline uint8_t Encoder_Read()
{
	return EncoderMask & PIND;
}

inline void Display_On()
{
    PRR &= ~(_BV(PRSPI) | _BV(PRTIM2)); //start SPI & timer2 PWM

    DDRB |= _BV(DDB3); //OC2B as PWM output
    PORTD |= _BV(PD2); //switch on power to display module

	display_Init();
}

inline void Display_Off()
{
	PORTD &= ~_BV(PD2); //switch off power to display module

    DDRB &= ~_BV(DDB3); //OC2B PWM disconnect
	PRR |= _BV(PRSPI) | _BV(PRTIM2); //stop SPI & timer2 PWM
}

inline void Display_Brighness(uint8_t v)
{
    OCR2B = v;
}

inline void SetLED(bool v)
{
	if(v)
	{
		PORTB |= _BV(PB1);
	}
	else
	{
		PORTB &= ~_BV(PB1);
	}
}

inline void SetPB0(bool v)
{
    if(v)
    {
        PORTB |= _BV(PB0);
    }
    else
    {
        PORTB &= ~_BV(PB0);
    }
}

inline void ToggleLED()
{
	PINB |= _BV(PB0); //toggle output port
}

void SPI_MasterInit(void)
{
	/* Set MOSI and SCK output, all others input */
	DDRB = _BV(PB3) | _BV(PB5);

	/* Enable SPI, Master, set clock rate fck/16 */
	SPCR = _BV(SPE) | _BV(MSTR) | _BV(SPR0);
}
void SPI_MasterTransmit(char cData)
{
	/* Wait for prev transmission complete */
	while(!(SPSR & _BV(SPIF)))
	;

	/* Start new transmission */
	SPDR = cData;
}

void GLCD_Command(char cmd)		/* GLCD command function */
{
	SPI_MasterTransmit(0xf8);  //command sync
	SPI_MasterTransmit(cmd & 0x0f);
	SPI_MasterTransmit(cmd << 4);
}

void GLCD_Init()			/* GLCD initialize function */
{
	SPI_MasterInit();

	GLCD_Command(0x3E);		/* Display OFF */
	GLCD_Command(0x40);		/* Set Y address (column=0) */
	GLCD_Command(0xB8);		/* Set x address (page=0) */
	GLCD_Command(0xC0);		/* Set z address (start line=0) */
	GLCD_Command(0x3F);		/* Display ON */
}


#endif //ATMEGA_CLOCK_H
