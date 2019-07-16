#include <stdint.h>
#include "streetlight-control.h"
#include "datetime.h"
#include "relay.h"
#include "automation.h"

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