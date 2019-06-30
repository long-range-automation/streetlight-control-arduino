#include <Arduino.h>
#include <TinyGPS.h>
#include <SoftwareSerial.h>
#include "gps.h"

TinyGPS gps;
SoftwareSerial ss(4, 3); // Arduino RX, TX to conenct GPS

bool readGPSDateTime(s_date *date)
{
    unsigned long age;
    byte hundredths;
    gps.crack_datetime(&date->year, &date->month, &date->day, &date->hour, &date->minute, &date->second, &hundredths, &age);

    return age != TinyGPS::GPS_INVALID_AGE;
}

bool readGPS(s_coords *coords, s_date *date)
{
    float flat, flon;
    unsigned long age;
    gps.f_get_position(&flat, &flon, &age);

#ifdef DEBUG
    Serial.print("INVALID=");
    Serial.print(TinyGPS::GPS_INVALID_F_ANGLE);
    Serial.print(" LAT=");
    Serial.print(flat);
    Serial.print(" LON=");
    Serial.print(flon);
    Serial.print(" SAT=");
    Serial.println(gps.satellites() == TinyGPS::GPS_INVALID_SATELLITES ? 0 : gps.satellites());
#endif

    if (flat >= TinyGPS::GPS_INVALID_F_ANGLE || flon >= TinyGPS::GPS_INVALID_F_ANGLE)
    {
        return false;
    }

    coords->latitude = flat;
    coords->longitude = flon;

    return readGPSDateTime(date);
}

void smartdelay(unsigned long ms)
{
    unsigned long start = millis();
    do
    {
        while (ss.available())
        {
            gps.encode(ss.read());
        }
    } while (millis() - start < ms);
}
