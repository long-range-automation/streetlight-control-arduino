BOARD_TAG    = mega
BOARD_SUB    = atmega2560
ARDUINO_DIR  = /home/klaus/Downloads/arduino-1.8.9
ARDUINO_LIBS = SPI arduino-lmic TinyGPS SoftwareSerial
#MONITOR_PORT = /dev/ttyUSB5
CPPFLAGS += -DSC_DEBUG # -DNAME=${NAME}

include /usr/share/arduino/Arduino.mk
