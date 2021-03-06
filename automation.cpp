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

bool checkAutomation(s_configuration *config)
{
    bool relayStateChanged = false;
    s_time time;

    if (!time_get(&time)) {
        return relayStateChanged;
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
        if (RELAY_MODE(config->relayModes, i) != RELAY_MODE_AUTO)
        {
            continue;
        }

        if (isInOnRange && !isRelayOn(i))
        {
            switchRelayOn(i);

            relayStateChanged = true;
        }
        else if (!isInOnRange && isRelayOn(i))
        {
            switchRelayOff(i);

            relayStateChanged = true;
        }
    }

    return relayStateChanged;
}