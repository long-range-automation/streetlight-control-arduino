#include "relay.h"
#include "lora.h"
#include "debug.h"
#include "message.h"
#include "gps.h"
#include "streetlight-control.h"

#define DEFAULT_LATITUDE 47.63249
#define DEFAULT_LONGITUDE 9.52935

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
