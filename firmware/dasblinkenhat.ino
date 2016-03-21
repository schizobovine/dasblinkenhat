#include <Adafruit_NeoPixel.h>
#include <Bounce2.h>

const byte LED_PIN = 4;
const byte BUTT_PIN = 3;
const byte DATA_PIN = 2;

const int BUTT_DEBOUNCE_MS = 50; 
Bounce butt;

const int NUM_PIXELS = 100; //max=103
Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUM_PIXELS, DATA_PIN, NEO_GRB + NEO_KHZ800);
uint8_t offset = 0;
uint8_t bright = 255;
uint16_t delay_ms = 5;

typedef enum {
  MODE_CYCLE,
  MODE_PULSE,
} mode_t;

mode_t mode = MODE_CYCLE;

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
// SETUP - done once
////////////////////////////////////////////////////////////////////////

void setup() {
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);
  butt.attach(BUTT_PIN, INPUT_PULLUP, BUTT_DEBOUNCE_MS);
  strip.begin();
  strip.show();
}

////////////////////////////////////////////////////////////////////////
// LOOP - done many times, like your mom
////////////////////////////////////////////////////////////////////////

void loop() {

  // Check button state for mode change
  if (butt.update() && butt.fell()) {

    // Advance to the next mode
    switch (mode) {
      case MODE_CYCLE:
        mode = MODE_PULSE;
        break;
      case MODE_PULSE:
        mode = MODE_CYCLE;
        break;
      default: //wat
        seppuku();
        break;
    }

  }

  // Dispatch color setting by mode
  switch (mode) {
    case MODE_CYCLE:
      rainbow_cycle();
      break;
    case MODE_PULSE:
      rainbow_pulse();
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
