#include <Adafruit_NeoPixel.h>
#include <Bounce2.h>

#define DATA_PIN 1
#define BUTT_PIN 2
#define TRIM_PIN A3

#define BUTT_DEBOUNCE_MS 50

Bounce butt;

#define NUM_PIXELS 90          //max=103 (nope, <100 now)
#define DEFAULT_BRIGHTNESS 255 //0-255
#define DELAY_MS 5             //how long to pause between loops

Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUM_PIXELS, DATA_PIN, NEO_GRB + NEO_KHZ800);
uint8_t offset = 0;
uint8_t bright = DEFAULT_BRIGHTNESS;

typedef enum {
  MODE_CYCLE,
  MODE_PULSE,
  MODE_WATERFALL,
} mode_t;

mode_t mode = MODE_CYCLE;

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

  // Handle odd number of pixels
  if (NUM_PIXELS % 2 == 1) {
    set_pixel_color(NUM_PIXELS/2+1, offset - 1);
  }

  // Handle even/rest of pixels
  for (uint16_t i=0; i<NUM_PIXELS/2; i++) {
    set_pixel_color(NUM_PIXELS/2 + i, offset + i);
    set_pixel_color(NUM_PIXELS/2 - i, offset + i);
  }

  offset++;

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
// MISC
////////////////////////////////////////////////////////////////////////

uint8_t get_brightness_trim() {
  uint16_t reading = analogRead(TRIM_PIN);
  return map(reading, 0, 1023, 0, 255);
}

////////////////////////////////////////////////////////////////////////
// SETUP - done once
////////////////////////////////////////////////////////////////////////

void setup() {
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
        mode = MODE_WATERFALL;
        break;
      case MODE_WATERFALL:
        mode = MODE_CYCLE;
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
  delay(DELAY_MS);

}

// vi: syntax=arduino
