#ifndef __datetime_h__
#define __datetime_h__
typedef struct
{
    int year;
    uint8_t month;
    uint8_t day;
} s_date;

typedef struct
{
    uint8_t hour;
    uint8_t minute;
    uint8_t second;
} s_time;
#endif

bool date_get(s_date *date);
bool time_get(s_time *time);