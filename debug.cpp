#include "debug.h"

void debug_setup() {
    Serial.begin(SC_DEBUG_BAUD_RATE);

    pinMode(STATUS_LED_JOINED, OUTPUT);
    pinMode(STATUS_LED_GPS, OUTPUT);
    pinMode(STATUS_LED_TX, OUTPUT);

    while (!Serial);
}

void turnLedOn(uint8_t pin) {
    digitalWrite(pin, HIGH);
}
void turnLedOff(uint8_t pin) {
    digitalWrite(pin, LOW);
}