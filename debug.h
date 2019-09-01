#include <Arduino.h>

#define LOG(message) Serial.print(message)
#define LOG_LN(message) Serial.println(message)
#define LOG_MSG(message) Serial.println(F(message))

#define SC_DEBUG_BAUD_RATE 9600

void debug_setup();