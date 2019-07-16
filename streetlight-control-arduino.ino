#include "relay.h"
#include "lora.h"
#include "debug.h"
#include "message.h"
#include "gps.h"

s_configuration global_config = {
    .relayModes = 0,
    .timeOn = 0xff,
    .timeOff = 0xff,
    .outageOn = 0xff,
    .outageOff = 0xff,
};

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
  lora_once();
}
