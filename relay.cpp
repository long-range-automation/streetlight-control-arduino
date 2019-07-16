#include <Arduino.h>
#include "debug.h"
#include "relay.h"
#include "datetime.h"
#include "streetlight-control.h"

const int RELAY_PINS[] = {RELAY0_PIN, RELAY1_PIN, RELAY2_PIN, RELAY3_PIN};

void relay_setup()
{
    for (unsigned int i = 0; i < (sizeof(RELAY_PINS) / sizeof(int)); i++)
    {
        pinMode(RELAY_PINS[i], OUTPUT);
    }
}

void switchRelayOff(int index)
{
    LOG_MSG("Switch relay OFF");

    digitalWrite(RELAY_PINS[index], LOW);
}

void switchRelayOn(int index)
{
    LOG_MSG("Switch relay ON");

    digitalWrite(RELAY_PINS[index], HIGH);
}

int getRelayState(int index)
{
    return digitalRead(RELAY_PINS[index]);
}

uint8_t getRelayMode(uint8_t modes, short number)
{
    return (modes >> (2 * number)) & 0x3;
}

uint8_t timeToByte(s_time input)
{
    return ((input.hour % 24) * 10) + (int(input.minute / 6));
}

bool isInRange(uint8_t start, uint8_t ende, uint8_t value)
{
    return (value > start && value <= MAX_TIME_BYTE && ende < start) ||
           (value < ende && ende < start) ||
           (value > start && value < ende);
}

void checkAutomation(s_configuration *config)
{
    s_time time;

    if (!time_get(&time)) {
        return;
    }

    uint8_t timeByte = timeToByte(time);
    bool isInOnRange = false;

    if (config->timeOn != config->timeOff)
    {
        if (config->outageOn != config->outageOff)
        {
            isInOnRange = isInRange(config->timeOn, config->outageOn, timeByte) ||
                          isInRange(config->outageOff, config->timeOff, timeByte);
        }
        else
        {
            isInOnRange = isInRange(config->timeOn, config->timeOff, timeByte);
        }
    }

    for (short i = 0; i < 4; i++)
    {
        if (getRelayMode(config->relayModes, i) != RELAY_MODE_AUTO)
        {
            continue;
        }

        if (isInOnRange)
        {
            switchRelayOn(i);
        }
        else
        {
            switchRelayOff(i);
        }
    }
}