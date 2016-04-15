/*
 * dashblinkenhat.ino
 *
 * Author: Sean Caulfield <sean@yak.net>
 * License: GPLv2.0
 *
 */

#include <Arduino.h>
#include <Adafruit_NeoPixel.h>
#include <Bounce2.h>

#define DEBUG 0
#define SERIAL_BAUD 9600

#ifdef DEBUG
#define DPRINT(...) Serial.print(__VA_ARGS__)
#define DPRINTLN(...) Serial.println(__VA_ARGS__)
#else
#define DPRINT(...)
#define DPRINTLN(...)
#endif

#define BUTT_PIN 5
#define DATA_PIN 6
#define BRIGHT_TRIM_PIN A4
#define DELAY_TRIM_PIN A5
#define BATT_DIV_PIN 9

#define BUTT_DEBOUNCE_MS 50

Bounce butt;

#define NUM_PIXELS 120         //max=103 (nope, <100 now)
#define DEFAULT_BRIGHTNESS 255 //0-255

#define DELAY_MIN 5
#define DELAY_MAX 50
#define DELAY_DEFAULT DELAY_MAX

Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUM_PIXELS, DATA_PIN, NEO_GRB + NEO_KHZ800);

uint8_t offset = 0;
uint8_t bright = DEFAULT_BRIGHTNESS;

uint16_t delay_ms = DELAY_DEFAULT;
uint16_t last_delay_ms = DELAY_DEFAULT;

uint16_t battery_level = 0;
uint16_t last_battery_level = 0;

typedef enum {
  MODE_CYCLE,
  MODE_PULSE,
  MODE_WATERFALL,
  MODE_OFF,
} mode_t;

mode_t mode = MODE_WATERFALL;

////////////////////////////////////////////////////////////////////////
// COLOR HELPERS
////////////////////////////////////////////////////////////////////////

//
// Maps 0-255 to RGB values, red -> green -> blue -> red (cycles)
//
void set_pixel_color(uint16_t pixel_num, uint8_t k) {
  uint8_t r, g, b;
  if (k < 85) {
    r = 255 - k * 3;
    g = 0;
    b = k * 3;
  } else if (k < 170) {
    k -= 85;
    r = 0;
    g = k * 3;
    b = 255 - k * 3;
  } else {
    k -= 170;
    r = k * 3;
    g = 255 - k * 3;
    b = 0;
  }
  strip.setPixelColor(pixel_num, r, g, b);
}

//
// Maps 0-255 range to a cycling rainbow (colors ripple down the strip, each
// LED a different color)
//
void rainbow_cycle() {
  for (uint16_t i=0; i<NUM_PIXELS; i++) {
    set_pixel_color(i, offset + i);
  }
  offset++;
}

//
// Maps 0-255 range to a pulsing rainbow (all colors changing at once)
//
void rainbow_pulse() {
  for (uint16_t i=0; i<NUM_PIXELS; i++) {
    set_pixel_color(i, offset);
  }
  offset++;
}

//
// Maps 0-255 range to a cycling rainbow starting at midpoint of strand,
// mirrored on both halves to create a waterfall-like effect
//
void rainbow_waterfall() {
  uint16_t i = 0;

  // Set colors from both front & back of array, eventually meeting in the
  // middle (or not, if there's an odd number of elements--that case is handled
  // below), using the location to further offset the color for each pixel.
  for (i=0; i<NUM_PIXELS/2; i++) {
    set_pixel_color(i, offset + i);
    set_pixel_color(NUM_PIXELS - 1 - i, offset + i);
  }

  // Handle odd number of pixels; re-use last value of from loop to get next
  // color in sequence
  if (NUM_PIXELS % 2 == 1) {
    set_pixel_color(i, offset + i);
  }

  // Increment offset to create flowing effect
  offset++;

}

//
// Turns entire strip off
//
void strip_off() {
  for (uint16_t i=0; i<NUM_PIXELS; i++) {
    strip.setPixelColor(i, 0, 0, 0);
  }
}

////////////////////////////////////////////////////////////////////////
// ERROR HANDLER
////////////////////////////////////////////////////////////////////////

//
// Die an honorable death, and hopefully an error message (?)
//
void seppuku(uint8_t error_code=0) {

  for (uint16_t i=0; i<NUM_PIXELS; i++) {

    //Turn first 8 leds into status indicator
    if (i < sizeof(error_code)) {
      if (error_code & 1) {
        strip.setPixelColor(i, 0, 255, 0); //green <=> 1
      } else {
        strip.setPixelColor(i, 255, 0, 0); //red <=> 0
      }
      error_code >>= 1;

    // And turn the rest off
    } else {
      strip.setPixelColor(i, 0, 0, 0);
    }

  }

}

////////////////////////////////////////////////////////////////////////
// ANALOG HELPERS
////////////////////////////////////////////////////////////////////////

//
// Get battery level (0-1023 through voltage divider)
//
uint16_t get_batt_level() {
  uint16_t reading = analogRead(BATT_DIV_PIN);
  return reading;
  //return reading * 2.0 * 3.3 / 1024;
}

//
// Get battery voltage
//
float get_batt_voltage(uint16_t level) {
  return level * 2.0 * 3.3 / 1024;
}

//
// Get brightness level from brightness trimmer setting
//
uint8_t get_brightness_trim() {
  uint16_t reading = analogRead(BRIGHT_TRIM_PIN);
  return map(reading, 0, 1023, 0, 255);
}

//
// Get cycle delay time based on delay trimmer setting
//
uint16_t get_delay_trim() {
  uint16_t reading = analogRead(DELAY_TRIM_PIN);
  return map(reading, 0, 1023, DELAY_MIN, DELAY_MAX);
}

////////////////////////////////////////////////////////////////////////
// SETUP - done once
////////////////////////////////////////////////////////////////////////

void setup() {

  butt.attach(BUTT_PIN, INPUT_PULLUP, BUTT_DEBOUNCE_MS);

  strip.begin();
  strip.show();

#if DEBUG
  Serial.begin(SERIAL_BAUD);
  while (!Serial)
    ;
  Serial.println("HAI");
  pinMode(13, OUTPUT);
  digitalWrite(13, HIGH);
#endif

}

////////////////////////////////////////////////////////////////////////
// LOOP - done many times, like your mom
////////////////////////////////////////////////////////////////////////

void loop() {

  // Check button state for mode change
  if (butt.update() && butt.fell()) {

    // Advance to the next mode
    DPRINT(F("Mode switch to "));
    switch (mode) {
      case MODE_CYCLE:
        mode = MODE_PULSE;
        DPRINTLN(F("MODE_PULSE"));
        break;
      case MODE_PULSE:
        mode = MODE_WATERFALL;
        DPRINTLN(F("MODE_WATERFALL"));
        break;
      case MODE_WATERFALL:
        mode = MODE_OFF;
        DPRINTLN(F("MODE_OFF"));
        break;
      case MODE_OFF:
        mode = MODE_CYCLE;
        DPRINTLN(F("MODE_CYCLE"));
        break;
      default: //wat
        seppuku();
        break;
    }

  }

  // Check current brightness setting & (re)set if needed
  bright = get_brightness_trim();
  if (bright != strip.getBrightness()) {
    strip.setBrightness(bright);
    DPRINT(F("Set brightness to "));
    DPRINTLN(bright);
  }

  // Check delay setting & (re)set if needed
  delay_ms = get_delay_trim();
  if (delay_ms != last_delay_ms) {
    last_delay_ms = delay_ms;
    DPRINT(F("Set delay to "));
    DPRINTLN(delay_ms);
  }

  // Check battery level
  battery_level = get_batt_level();
  if (battery_level != last_battery_level) {
    last_battery_level = battery_level;
    DPRINT(F("Battery level is "));
    DPRINT(battery_level);
    DPRINT(F(" "));
    DPRINTLN(get_batt_voltage(battery_level));
  }

  // Dispatch color setting by mode
  switch (mode) {
    case MODE_CYCLE:
      rainbow_cycle();
      break;
    case MODE_PULSE:
      rainbow_pulse();
      break;
    case MODE_WATERFALL:
      rainbow_waterfall();
      break;
    case MODE_OFF:
      strip_off();
      break;
    default: //wat
      seppuku();
      break;
  }

  // Update strip to display
  strip.show();

  // Pauses to slow the motion effect of the color changing
  delay(delay_ms);

}

// vi: syntax=arduino
