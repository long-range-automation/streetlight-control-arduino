#include "debug.h"

void debug_setup() {
    Serial.begin(SC_DEBUG_BAUD_RATE);

    while (!Serial);
}