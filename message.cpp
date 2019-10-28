#include <Arduino.h>
#include <stdint.h>
#include "gps.h"
#include "relay.h"
#include "lora.h"
#include "message.h"
#include "datetime.h"
#include "streetlight-control.h"

bool packHeartbeatMessage(uint8_t *data)
{
    s_coords coords;
    s_time time;
    bool hasGPSSignal = gps_read_coords(&coords);
    bool hasValidTime = time_get(&time);

    int maintenanceMode = (digitalRead(MAINTENANCE_PIN) == LOW) << 7;
    int gpsSignal = hasGPSSignal ? 1 << 6 : 0;
    int relayStates = isRelayOn(0) + (isRelayOn(1) << 1) + (isRelayOn(2) << 2) + (isRelayOn(3) << 3);

    data[0] = (PROTOCOL_VERSION << 4) + HEARTBEAT_TYPE;
    data[1] = global_config.timeOn ^ global_config.timeOff ^ global_config.outageOn ^ global_config.outageOff; // checksum
    data[2] = hasValidTime ? time.hour : 0xff; // hour
    data[3] = hasValidTime ? time.minute : 0xff; // minute
    data[4] = (TX_INTERVAL / 60);
    data[5] = maintenanceMode + gpsSignal + relayStates;
    data[6] = global_config.relayModes;

    return true;
}

static void packGPSCoordinates(float latitude, float longitude, uint8_t *data)
{
    //use 10m accuracy and make sure value is positive
    uint32_t lat = (latitude + 180) * 10000;
    uint32_t lon = (longitude + 180) * 10000;

    data[0] = lat >> 16;
    data[1] = lat >> 8;
    data[2] = lat;
    data[3] = lon >> 16;
    data[4] = lon >> 8;
    data[5] = lon;
}

bool packLocationMessage(uint8_t *data) {
    s_coords coords;
    bool hasGPSSignal = gps_read_coords(&coords);

    data[0] = (PROTOCOL_VERSION << 4) + LOCATION_TYPE;
    data[1] = (TX_INTERVAL / 60);

    if (hasGPSSignal) {
        packGPSCoordinates(coords.latitude, coords.longitude, data + 2);

        return true;
    }

    for(int i = 0; i < 6; i++) {
        data[2 + i] = 0xff;
    }

    return false;
}

static void updateRelays(uint8_t modes)
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

static void unpackConfigurationMessage(uint8_t *data, s_configuration *config)
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