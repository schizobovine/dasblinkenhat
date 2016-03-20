ARDUINO_DIR					= $(realpath $(HOME)/arduino/arduino-current)
ARDUINO_LIBS				:= Adafruit_NeoPixel Bounce2
USER_LIB_PATH				:= $(realpath ./lib)

#ARDUINO_PORT				:= /dev/ttyUSB0

#ARDUINO_VAR_PATH		= $(realpath $(HOME)/sketchbook/hardware)

#ARDMK_VENDOR				= attiny
#ARCHITECTURE				= avr
BOARD_TAG						= attiny
BOARD_SUB						= attiny85
F_CPU							= 8000000L

ISP_PORT						:= /dev/ttyUSB*

ALTERNATE_CORE			= attiny
#ALTERNATE_CORE_PATH	= $(realpath $(HOME)/.arduino15/packages/)

CXXFLAGS      += -std=gnu++11 -fno-threadsafe-statics
CFLAGS        += -std=gnu11
LDFLAGS       += -lm

include Arduino.mk
