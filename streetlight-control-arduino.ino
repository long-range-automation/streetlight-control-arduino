#include "relay.h"
#include "lora.h"
#include "debug.h"
#include "message.h"
#include "gps.h"
#include "automation.h"
#include "config/local.h"

s_configuration global_config = {
    .relayModes = 0,
    .timeOn = 0xff,
    .timeOff = 0xff,
    .outageOn = 0xff,
    .outageOff = 0xff,
};

unsigned long lastAutomationRun = millis();
bool automationEnabled = false;

void setup()
{
  debug_setup();

  LOG("Starting... ");
  LOG(__NAME);
  LOG_MSG("");

  pinMode(MAINTENANCE_PIN, INPUT_PULLUP);

  gps_setup();

  relay_setup();

  lora_start();
}

void loop()
{
  if (automationEnabled && millis() - lastAutomationRun > 5000)
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
