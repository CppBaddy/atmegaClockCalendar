#include "rtc3231.h"
#include "i2c.h"

#define RTC_WADDR 0b11010000
#define RTC_RADDR 0b11010001

static unsigned char bcd (unsigned char data)
{
    unsigned char bc;

    bc = (data >> 4) * 10 + (data & 0x0f);

    return bc;
}

static unsigned char bin(unsigned char dec)
{
    char bcd = ((dec / 10) << 4) | (dec % 10);
    return bcd;
}


void rtc3231_init(void)
{
    if(i2c_start_condition())
    {
        i2c_send_byte(RTC_WADDR);
        i2c_send_byte(0x0E);
        i2c_send_byte(0x04); //desable 1Hz SQW pin
        i2c_send_byte(0x00); //reset OSF flag, disable 32kHz output

        i2c_stop_condition();
    }
}

void rtc3231_read_time(struct rtc_time *time)
{
    if(i2c_start_condition())
    {
        i2c_send_byte(RTC_WADDR);
        i2c_send_byte(0x00);
        i2c_stop_condition();
    }

    if(i2c_start_condition())
    {
        i2c_send_byte(RTC_RADDR);

        time->sec = bcd(i2c_recv_byte());
        time->min = bcd(i2c_recv_byte());
        time->hour = bcd(i2c_recv_last_byte());

        i2c_stop_condition();
    }
}

void rtc3231_read_date(struct rtc_date *date)
{
    if(i2c_start_condition())
    {
        i2c_send_byte(RTC_WADDR);
        i2c_send_byte(0x03);
        i2c_stop_condition();
    }

    if(i2c_start_condition())
    {
        i2c_send_byte(RTC_RADDR);

        date->wday = bcd(i2c_recv_byte());
        date->day = bcd(i2c_recv_byte());
        date->month = bcd(i2c_recv_byte());
        date->year = bcd(i2c_recv_last_byte());

        i2c_stop_condition();
    }
}

void rtc3231_read_datetime(struct rtc_time *time, struct rtc_date *date)
{
    if(i2c_start_condition())
    {
        i2c_send_byte(RTC_WADDR);
        i2c_send_byte(0x00);
        i2c_stop_condition();
    }

    if(i2c_start_condition())
    {
        i2c_send_byte(RTC_RADDR);

        time->sec = bcd(i2c_recv_byte());
        time->min = bcd(i2c_recv_byte());
        time->hour = bcd(i2c_recv_byte());

        date->wday = bcd(i2c_recv_byte());
        date->day = bcd(i2c_recv_byte());
        date->month = bcd(i2c_recv_byte());
        date->year = bcd(i2c_recv_last_byte());

        i2c_stop_condition();
    }
}

void rtc3231_read_temperature(struct temp_t *t)
{
    if(i2c_start_condition())
    {
        i2c_send_byte(RTC_WADDR);
        i2c_send_byte(0x11);
        i2c_stop_condition();

        if(i2c_start_condition())
        {
            i2c_send_byte(RTC_RADDR);

            t->degrees = i2c_recv_byte();
            uint8_t sub = i2c_recv_last_byte();

            i2c_stop_condition();

            t->subdegr = (25 * (sub >> 6)) % 100;
        }
    }
}

void rtc3231_write_time(struct rtc_time *time)
{
    if(i2c_start_condition())
    {
        i2c_send_byte(RTC_WADDR);
        i2c_send_byte(0x00);
        i2c_send_byte(bin(time->sec));
        i2c_send_byte(bin(time->min));
        i2c_send_byte(bin(time->hour));

        i2c_stop_condition();
    }
}

void rtc3231_write_date(struct rtc_date *date)
{
    if(i2c_start_condition())
    {
        i2c_send_byte(RTC_WADDR);
        i2c_send_byte(0x03);
        i2c_send_byte(bin(date->wday));
        i2c_send_byte(bin(date->day));
        i2c_send_byte(bin(date->month));
        i2c_send_byte(bin(date->year));

        i2c_stop_condition();
    }
}

