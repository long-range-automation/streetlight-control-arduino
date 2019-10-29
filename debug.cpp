#include <Arduino.h>
#include "debug.h"

static int uart_putchar (char c, FILE *)
{
    if (c == '\n')
        Serial.write('\r');

    Serial.write(c) ;

    return 0 ;
}

void debug_setup() {
    Serial.begin(SC_DEBUG_BAUD_RATE);

    pinMode(STATUS_LED_JOINED, OUTPUT);
    pinMode(STATUS_LED_GPS, OUTPUT);
    pinMode(STATUS_LED_TX, OUTPUT);

    while (!Serial);

    // create a FILE structure to reference our UART output function
    static FILE uartout;
    memset(&uartout, 0, sizeof(uartout));

    // fill in the UART file descriptor with pointer to writer.
    fdev_setup_stream (&uartout, uart_putchar, NULL, _FDEV_SETUP_WRITE);

    // The uart is the standard output device STDOUT.
    stdout = &uartout ;
}

void turnLedOn(uint8_t pin) {
    digitalWrite(pin, HIGH);
}
void turnLedOff(uint8_t pin) {
    digitalWrite(pin, LOW);
}