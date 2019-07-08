#ifndef __datetime_h__
#define __datetime_h__
typedef struct
{
    int year;
    byte month;
    byte day;
} s_date;

typedef struct
{
    byte hour;
    byte minute;
    byte second;
} s_time;
#endif

bool date_get(s_date *date);
bool time_get(s_time *time);