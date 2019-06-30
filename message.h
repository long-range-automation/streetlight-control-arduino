#define PROTOCOL_VERSION 0

#define HEARTBEAT_TYPE 1
#define CONFIGURATION_TYPE 2

#define HEARTBEAT_LENGTH 7
#define CONFIGURATION_LENGTH 6

#ifndef MESSAGE_H
#define MESSAGE_H
typedef struct
{
    uint8_t relayModes;
    uint8_t timeOn;
    uint8_t timeOff;
    uint8_t outageOn;
    uint8_t outageOff;
} s_configuration;
#endif

bool packHeartbeatMessage(uint8_t *data);
void packGPSCoordinates(float latitude, float longitude, uint8_t *data);

void processConfigurationMessage(uint8_t *data);