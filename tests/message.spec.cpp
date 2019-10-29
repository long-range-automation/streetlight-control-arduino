#include <stdint.h>
#include <cassert>
#include <cstdio>
#include <math.h>
#include "../message.h"
#include "../gps.h"
#include "../datetime.h"

#define PROTOCOL_VERSION 0
#define LOCATION_TYPE 0
#define HEARTBEAT_TYPE 1

#define GPS_AVAILABLE 1
#define IS_MAINTENANCE 1
#define RELAY_STATE 0x9
#define HOUR 18
#define MINUTE 7

bool isRelayOn(int index) {
    return !!((1 << index) & RELAY_STATE);
}

bool gps_read_coords(s_coords *coords) {

    coords->latitude = 47.6955538;
    coords->longitude = 9.2719656;

    return GPS_AVAILABLE;
}

bool time_get(s_time *time) {

    time->hour = HOUR;
    time->minute = MINUTE;
    time->second = 0;

    return true;
}

int digitalRead(uint8_t pin) {
    if (pin == 33) {
        return !IS_MAINTENANCE;
    }

    return !0;
}

void switchRelayOff(int index)
{
}

void switchRelayOn(int index)
{
}

s_configuration global_config = {
    .relayModes = 0,
    .timeOn = 0xff,
    .timeOff = 0xff,
    .outageOn = 0xff,
    .outageOff = 0xff,
};

void printHex(uint8_t *data, unsigned int length)
{
    for (int i = 0; i < length; i++)
    {
        printf("%02x ", data[i]);
    }
    printf("\n");
}

int main() {
    uint8_t data[8];

    packLocationMessage(data);
    printf("Location:  ");
    printHex(data, sizeof(data));

    assert(data[0] == (PROTOCOL_VERSION << 4) + LOCATION_TYPE);
    //data[1] is next receive window
    float unpackedLatitude = ((data[2] << 16) + (data[3] << 8) + data[4]) / 10000.0 - 180;
    float unpackedLongitude = ((data[5] << 16) + (data[6] << 8) + data[7]) / 10000.0 - 180;

    assert(fabs(unpackedLatitude - 47.6955) < 0.0005);
    assert(fabs(unpackedLongitude - 9.2719) < 0.0005);

    packHeartbeatMessage(data);
    printf("Heartbeat: ");
    printHex(data, 7);

    assert(data[0] == (PROTOCOL_VERSION << 4) + HEARTBEAT_TYPE);
    assert(data[1] == global_config.timeOn ^ global_config.timeOff ^ global_config.outageOn ^ global_config.outageOff);
    assert(data[2] == HOUR);
    assert(data[3] == MINUTE);
    //data[4] is next receive window
    assert(data[5] == ((IS_MAINTENANCE << 7) + (GPS_AVAILABLE << 6) + RELAY_STATE));
    assert(data[6] == global_config.relayModes);

    return 0;
}