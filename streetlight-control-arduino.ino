#include "relay.h"
#include "lora.h"
#include "debug.h"
#include "message.h"
#include "gps.h"
#include "automation.h"

s_configuration global_config = {
    .relayModes = 0,
    .timeOn = 0xff,
    .timeOff = 0xff,
    .outageOn = 0xff,
    .outageOff = 0xff,
};

unsigned long lastAutomationRun = millis();

void setup()
{
  debug_setup();

  LOG_MSG("Starting... [otaa-gps-device]");

  gps_setup();

  relay_setup();

  lora_start();
}

void loop()
{
  if (millis() - lastAutomationRun > 5000)
  {
    lastAutomationRun = millis();

    LOG_MSG("Check automation");

    if (checkAutomation(&global_config))
    {
      lora_send_immediately();
    }
  }

  lora_once();

  //@TODO check time since last message exchange (in case tx message was not received, there will be no scheduled job)
}
