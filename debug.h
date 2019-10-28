#include <Arduino.h>

#define LOG(message) Serial.print(message)
#define LOG_LN(message) Serial.println(message)
#define LOG_MSG(message) Serial.println(F(message))

#define SC_DEBUG_BAUD_RATE 9600

#define STATUS_LED_JOINED 30
#define STATUS_LED_GPS 31
#define STATUS_LED_TX 32

void debug_setup();
void turnLedOn(uint8_t pin);
void turnLedOff(uint8_t pin);