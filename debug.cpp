#include "debug.h"

void debug_setup() {
    Serial.begin(DEBUG_BAUD_RATE);

    while (!Serial);
}