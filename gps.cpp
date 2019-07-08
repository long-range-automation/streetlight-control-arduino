#include <Arduino.h>
#include <TinyGPS.h>
#include <SoftwareSerial.h>
#include "gps.h"

TinyGPS gps;
SoftwareSerial ss(4, 3); // Arduino RX, TX to conenct GPS

void gps_setup()
{
    ss.begin(9600);
}

bool readFromSerial()
{
    for (unsigned long start = millis(); millis() - start < 1000;)
    {
        while (ss.available())
        {
            char c = ss.read();
            // Serial.write(c); // uncomment this line if you want to see the GPS data flowing
            if (gps.encode(c)) // Did a new valid sentence come in?
                return true;
        }
    }

    return false;
}

bool readGPSDateTime2(s_date *date)
{
    unsigned long age;
    byte hundredths;

    gps.crack_datetime(&date->year, &date->month, &date->day, &date->hour, &date->minute, &date->second, &hundredths, &age);

    return age != TinyGPS::GPS_INVALID_AGE;
}

bool readGPSDateTime(s_date *date)
{
    readFromSerial();

    return readGPSDateTime2(date);
}

bool readGPS(s_coords *coords, s_date *date)
{
    float flat, flon;
    unsigned long age;

    readFromSerial();

    gps.f_get_position(&flat, &flon, &age);

#ifdef DEBUG
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

    return readGPSDateTime2(date);
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

void gps_debug_loop()
{
    bool newData = false;
    unsigned long chars;
    unsigned short sentences, failed;

    // For one second we parse GPS data and report some key values
    for (unsigned long start = millis(); millis() - start < 1000;)
    {
        while (ss.available())
        {
            char c = ss.read();
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