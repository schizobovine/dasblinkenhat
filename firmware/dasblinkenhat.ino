#include <Adafruit_NeoPixel.h>
#include <Bounce2.h>

const byte LED_PIN = 4;
const byte BUTT_PIN = 3;
const byte DATA_PIN = 2;

const int BUTT_DEBOUNCE_MS = 50; 
Bounce butt;
boolean buttState = false;

const int NUM_PIXELS = 120;
Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUM_PIXELS, DATA_PIN, NEO_GRB + NEO_KHZ800);
uint8_t offset = 0;

// Maps 0-255 range to a cycling rainbow
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

void setup() {
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);
  butt.attach(BUTT_PIN, INPUT_PULLUP, BUTT_DEBOUNCE_MS);
  strip.begin();
  strip.show();
}

void loop() {

  if (butt.update() && butt.fell()) {
    buttState = !buttState;
  }
  digitalWrite(LED_PIN, buttState);

  for (uint16_t i=0; i<NUM_PIXELS; i++) {
    set_pixel_color(i, offset + i);
  }
  strip.show();
  offset++;
  delay(5);

}
