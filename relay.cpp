#include <Arduino.h>
#include "debug.h"
#include "relay.h"
#include "datetime.h"
#include "streetlight-control.h"

const int RELAY_PINS[] = {RELAY0_PIN, RELAY1_PIN, RELAY2_PIN, RELAY3_PIN};

void relay_setup()
{
    for (int i = 0; i < sizeof(RELAY_PINS); i++)
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
           (value < ende && value >= 0 && ende < start) ||
           (value > start && value < ende);
}

void checkAutomation()
{
    s_time time;

    if (!time_get(&time)) {
        return;
    }

    uint8_t timeByte = timeToByte(time);
    bool isInOnRange = false;

    if (global_config.timeOn != global_config.timeOff)
    {
        if (global_config.outageOn != global_config.outageOff)
        {
            isInOnRange = isInRange(global_config.timeOn, global_config.outageOn, timeByte) ||
                          isInRange(global_config.outageOff, global_config.timeOff, timeByte);
        }
        else
        {
            isInOnRange = isInRange(global_config.timeOn, global_config.timeOff, timeByte);
        }
    }

    for (short i = 0; i < 4; i++)
    {
        if (getRelayMode(global_config.relayModes, i) != RELAY_MODE_AUTO)
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