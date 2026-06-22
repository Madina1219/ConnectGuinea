#include <Adafruit_NeoPixel.h>

#define PIN    38
#define NUMPIXELS 8

Adafruit_NeoPixel ring(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

void setup() {
  ring.begin();
  ring.setBrightness(80);
  ring.show();
}

void loop() {
  // Green - all clear
  for (int i = 0; i < NUMPIXELS; i++) ring.setPixelColor(i, ring.Color(0, 255, 0));
  ring.show();
  delay(2000);

  // Blue - flood watch
  for (int i = 0; i < NUMPIXELS; i++) ring.setPixelColor(i, ring.Color(0, 0, 255));
  ring.show();
  delay(2000);

  // Yellow - flood warning
  for (int i = 0; i < NUMPIXELS; i++) ring.setPixelColor(i, ring.Color(255, 180, 0));
  ring.show();
  delay(2000);

  // Red - severe alert
  for (int i = 0; i < NUMPIXELS; i++) ring.setPixelColor(i, ring.Color(255, 0, 0));
  ring.show();
  delay(2000);

  // White pulsing - no signal
  for (int b = 0; b < 255; b += 5) {
    for (int i = 0; i < NUMPIXELS; i++) ring.setPixelColor(i, ring.Color(b, b, b));
    ring.show();
    delay(10);
  }
  for (int b = 255; b >= 0; b -= 5) {
    for (int i = 0; i < NUMPIXELS; i++) ring.setPixelColor(i, ring.Color(b, b, b));
    ring.show();
    delay(10);
  }
}