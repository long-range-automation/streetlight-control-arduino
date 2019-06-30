#include "relay.h"
#include "lora.h"
#include "debug.h"
#include "message.h"
#include "streetlight-control.h"

#define DEFAULT_LATITUDE 47.63249
#define DEFAULT_LONGITUDE 9.52935

s_configuration global_config = {
    .relayModes = 0,
    .timeOn = 0,
    .timeOff = 0,
    .outageOn = 0,
    .outageOff = 0,
};

void setup()
{
  debug_setup();

  LOG_MSG("Starting... [otaa-gps-device]");

  relay_setup();

  lora_start();
}

void loop()
{
  lora_once();
}
