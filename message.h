#define PROTOCOL_VERSION 0

#define LOCATION_TYPE 0
#define HEARTBEAT_TYPE 1
#define CONFIGURATION_TYPE 2

#define LOCATION_LENGTH 8
#define HEARTBEAT_LENGTH 7
#define CONFIGURATION_LENGTH 6

#ifndef __message_h__
#define __message_h__
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
bool packLocationMessage(uint8_t *data);

void processConfigurationMessage(uint8_t *data);