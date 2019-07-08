#define RELAY0_PIN A0
#define RELAY1_PIN A1
#define RELAY2_PIN A2
#define RELAY3_PIN A4

#define RELAY_MODE_OFF 0
#define RELAY_MODE_ON 1
#define RELAY_MODE_AUTO 2

#define MAX_TIME_BYTE 239

void relay_setup();
void switchRelayOff(int index);
void switchRelayOn(int index);
int getRelayState(int index);
uint8_t getRelayMode(uint8_t modes, short number);
void checkAutomation();