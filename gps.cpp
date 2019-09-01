#include <Arduino.h>
#include <TinyGPS.h>
#include <SoftwareSerial.h>
#include "datetime.h"
#include "gps.h"

TinyGPS gps;
SoftwareSerial gpsSerial(4, 3); // Arduino RX, TX to conenct GPS

void gps_setup()
{
    gpsSerial.begin(9600);
}

bool _read_from_serial()
{
    for (unsigned long start = millis(); millis() - start < 1000;)
    {
        while (gpsSerial.available())
        {
            char c = gpsSerial.read();
            // Serial.write(c); // uncomment this line if you want to see the GPS data flowing
            if (gps.encode(c)) // Did a new valid sentence come in?
                return true;
        }
    }

    return false;
}

bool gps_read_date(s_date *date)
{
    unsigned long age;
    byte hour, minute, second, hundredth;

    _read_from_serial();

    gps.crack_datetime(&date->year, &date->month, &date->day, &hour, &minute, &second, &hundredth, &age);

    return age != TinyGPS::GPS_INVALID_AGE;
}

bool gps_read_time(s_time *time)
{
    unsigned long age;
    int year;
    byte month, day, hundredths;

    _read_from_serial();

    gps.crack_datetime(&year, &month, &day, &time->hour, &time->minute, &time->second, &hundredths, &age);

#ifdef SC_DEBUG
    Serial.print("HOUR=");
    Serial.print(time->hour);
    Serial.print(" MINUTE=");
    Serial.print(time->minute);
    Serial.print(" SECOND=");
    Serial.println(time->second);
#endif

    return age != TinyGPS::GPS_INVALID_AGE;
}


bool gps_read_coords(s_coords *coords)
{
    float flat, flon;
    unsigned long age;

    _read_from_serial();

    gps.f_get_position(&flat, &flon, &age);

#ifdef SC_DEBUG
    Serial.print("LAT=");
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

    return true;
}

void gps_delay(unsigned long ms)
{
    unsigned long start = millis();

    do
    {
        while (gpsSerial.available())
        {
            gps.encode(gpsSerial.read());
        }
    } while (millis() - start < ms);
}

void gps_debug_loop()
{
    bool newData = false;
    unsigned long chars;
    unsigned short sentences, failed;

    // For one second we parse GPS data and report some key values
    for (unsigned long start = millis(); millis() - start < 1000;)
    {
        while (gpsSerial.available())
        {
            char c = gpsSerial.read();
            // Serial.write(c); // uncomment this line if you want to see the GPS data flowing
            if (gps.encode(c)) // Did a new valid sentence come in?
                newData = true;
        }
    }

    if (newData)
    {
        float flat, flon;
        unsigned long age;
        gps.f_get_position(&flat, &flon, &age);
        Serial.print("LAT=");
        Serial.print(flat == TinyGPS::GPS_INVALID_F_ANGLE ? 0.0 : flat, 6);
        Serial.print(" LON=");
        Serial.print(flon == TinyGPS::GPS_INVALID_F_ANGLE ? 0.0 : flon, 6);
        Serial.print(" SAT=");
        Serial.print(gps.satellites() == TinyGPS::GPS_INVALID_SATELLITES ? 0 : gps.satellites());
        Serial.print(" PREC=");
        Serial.print(gps.hdop() == TinyGPS::GPS_INVALID_HDOP ? 0 : gps.hdop());
    }

    gps.stats(&chars, &sentences, &failed);
    Serial.print(" CHARS=");
    Serial.print(chars);
    Serial.print(" SENTENCES=");
    Serial.print(sentences);
    Serial.print(" CSUM ERR=");
    Serial.println(failed);
    if (chars == 0)
        Serial.println("** No characters received from GPS: check wiring **");
}