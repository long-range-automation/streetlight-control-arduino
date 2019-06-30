#include <Arduino.h>
#include "gps.h"
#include "relay.h"
#include "lora.h"
#include "message.h"
#include "streetlight-control.h"

bool packHeartbeatMessage(uint8_t *data)
{
    s_coords coords;
    s_date date;
    bool hasGPSSignal = readGPS(&coords, &date);

    int maintenanceMode = 0;
    int gpsSignal = hasGPSSignal ? 1 << 6 : 0;
    int relayStates = getRelayState(0) + (getRelayState(1) << 1) + (getRelayState(2) << 2) + (getRelayState(3) << 3);

    data[0] = (PROTOCOL_VERSION << 4) + HEARTBEAT_TYPE;
    data[1] = 0; // checksum
    data[2] = 0; // hour
    data[3] = 0; // minute
    data[4] = (TX_INTERVAL / 60);
    data[5] = maintenanceMode + gpsSignal + relayStates;
    data[6] = 0;
}

void packGPSCoordinates(float latitude, float longitude, uint8_t *data)
{
    int32_t lat = (latitude, 6) * 10000; //save six decimal places
    int32_t lon = (longitude, 6) * 10000;

    data[0] = lat >> 16;
    data[1] = lat >> 8;
    data[2] = lat;
    data[3] = lon >> 16;
    data[4] = lon >> 8;
    data[5] = lon;
}

void updateRelays(uint8_t modes)
{
    for (short i = 0; i < 4; i++)
    {
        uint8_t mode = getRelayMode(modes, i);

        if (mode == RELAY_MODE_OFF)
        {
            switchRelayOff(i);
        }
        else if (mode == RELAY_MODE_ON)
        {
            switchRelayOn(i);
        }
    }
}

void unpackConfigurationMessage(uint8_t *data, s_configuration *config)
{
    config->relayModes = data[0];
    config->timeOn = data[1];
    config->timeOff = data[2];
    config->outageOn = data[3];
    config->outageOff = data[4];
}

void processConfigurationMessage(uint8_t *data)
{
    unpackConfigurationMessage(data, &global_config);

    updateRelays(global_config.relayModes);
}