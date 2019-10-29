#include "arduino.h"
#include "debug.h"
#include "relay.h"
#include "datetime.h"
#include "streetlight-control.h"

const int RELAY_PINS[] = {RELAY0_PIN, RELAY1_PIN, RELAY2_PIN, RELAY3_PIN};

void relay_setup()
{
    for (unsigned int i = 0; i < (sizeof(RELAY_PINS) / sizeof(int)); i++)
    {
        switchRelayOff(i);
        pinMode(RELAY_PINS[i], OUTPUT);
    }
}

void switchRelayOff(int index)
{
    LOG_MSG("Switch relay OFF\n");

    digitalWrite(RELAY_PINS[index], RELAY_OFF);
}

void switchRelayOn(int index)
{
    LOG_MSG("Switch relay ON\n");

    digitalWrite(RELAY_PINS[index], RELAY_ON);
}

bool isRelayOn(int index)
{
    return digitalRead(RELAY_PINS[index]) == RELAY_ON ? 0x1 : 0x0;
}
