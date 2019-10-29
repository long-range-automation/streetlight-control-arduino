BOARD_TAG    = mega
BOARD_SUB    = atmega2560
ARDUINO_DIR  = /home/klaus/Downloads/arduino-1.8.9
ARDUINO_LIBS = SPI arduino-lmic TinyGPS SoftwareSerial
#MONITOR_PORT = /dev/ttyUSB5
CPPFLAGS += -DSC_DEBUG # -DNAME=${NAME}

include /usr/share/arduino/Arduino.mk

SRC_DIR = .
TEST_DIR = tests
BUILD_DIR = build-test
BIN_DIR = bin-test

T_SRCS = $(SRC_DIR)/message.cpp
T_OBJS = $(patsubst $(SRC_DIR)/%.cpp,$(BUILD_DIR)/%.o,$(T_SRCS))

TESTS = $(wildcard $(TEST_DIR)/*.cpp)
TARGETS = $(patsubst $(TEST_DIR)/%.cpp,$(BIN_DIR)/%,$(TESTS))

test: $(TARGETS)

$(TARGETS): $(BIN_DIR)/%: $(T_OBJS)
	@$(MKDIR) $(dir $@)
	g++ -D __TEST_RUN $(TEST_DIR)/$*.cpp $^ -o $@
	./$@

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp
	@$(MKDIR) $(dir $@)
	g++ -D __TEST_RUN -c -o $@ $<