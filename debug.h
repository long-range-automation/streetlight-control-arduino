#define LOG_MSG(f, ...) printf(f, ## __VA_ARGS__)

#define SC_DEBUG_BAUD_RATE 9600

#define STATUS_LED_JOINED 30
#define STATUS_LED_GPS 31
#define STATUS_LED_TX 32

void debug_setup();
void turnLedOn(uint8_t pin);
void turnLedOff(uint8_t pin);