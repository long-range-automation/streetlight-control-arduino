#include "streetlight-control.h"

#define RELAY0_PIN A0
#define RELAY1_PIN A1
#define RELAY2_PIN A2
#define RELAY3_PIN A4

#define RELAY_MODE_OFF 0
#define RELAY_MODE_ON 1
#define RELAY_MODE_AUTO 2

#define RELAY_ON LOW
#define RELAY_OFF HIGH

#define RELAY_MODE(VALUE, INDEX) (((VALUE) >> (2 * (INDEX))) & 0x3)

void relay_setup();
void switchRelayOff(int index);
void switchRelayOn(int index);
bool isRelayOn(int index);