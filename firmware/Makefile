ARDUINO_DIR					= $(realpath $(HOME)/arduino/arduino-current)
ARDUINO_LIBS				:= Adafruit_NeoPixel Bounce2
USER_LIB_PATH				:= $(realpath ./lib)

ARDUINO_PORT				:= /dev/ttyACM0

ARCHITECTURE				= avr
ALTERNATE_CORE			= adafruit
BOARD_TAG						= feather32u4

CXXFLAGS      += -std=gnu++11 -fno-threadsafe-statics
CFLAGS        += -std=gnu11

include Arduino.mk
