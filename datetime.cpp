#include "arduino.h"
#include "datetime.h"
#include "gps.h"

s_time latestGPSTime;
unsigned long latestMillis;
bool hasFallbackTime = false;

bool _get_fallback_time(s_time *time) {
    unsigned long currentMillis = millis();
    unsigned long diff;

    if (!hasFallbackTime) {
        return false;
    }

    diff = currentMillis - latestMillis;
    diff /= 1000;

    time->hour = (latestGPSTime.hour + (diff / 60 / 60)) % 24;
    time->minute = (latestGPSTime.minute + (diff / 60)) % 60;
    time->second = (latestGPSTime.second + diff) % 60;

#ifdef SC_DEBUG
    Serial.print("FALLBACK HOUR=");
    Serial.print(time->hour);
    Serial.print(" MINUTE=");
    Serial.print(time->minute);
    Serial.print(" SECOND=");
    Serial.println(time->second);
#endif

    return true;
}

bool date_get(s_date *date)
{
    return gps_read_date(date);
}

bool time_get(s_time *time)
{
    bool validGPSTime = gps_read_time(time);

    if (validGPSTime) {
        latestGPSTime = *time;
        latestMillis = millis();
        hasFallbackTime = true;
    }

    if (!validGPSTime && hasFallbackTime) {
        return _get_fallback_time(time);
    }

    return validGPSTime;
}