#ifndef __TEST_RUN
    #include <Arduino.h>
#else
    #include <stdint.h>

    #define HIGH 1
    #define LOW 0

    int digitalRead(uint8_t);
#endif