#ifndef __DS3231_H__
#define __DS3231_H__

#include <stdio.h>

struct rtc_time
{
    uint8_t sec;
    uint8_t min;
    uint8_t hour;
};

struct rtc_date
{
    uint8_t wday;
    uint8_t day;
    uint8_t month;
    uint8_t year;
};

struct temp_t
{
    uint8_t degrees;
    uint8_t subdegr;
};

void rtc3231_init(void);

void rtc3231_read_time(struct rtc_time *time);

void rtc3231_read_date(struct rtc_date *date);

void rtc3231_read_datetime(struct rtc_time *time, struct rtc_date *date);

void rtc3231_write_time(struct rtc_time *time);

void rtc3231_write_date(struct rtc_date *date);

void rtc3231_read_temperature(struct temp_t *t);

#endif
