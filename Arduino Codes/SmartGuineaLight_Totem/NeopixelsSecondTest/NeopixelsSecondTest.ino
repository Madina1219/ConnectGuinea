#include <Arduino.h>
#include <Adafruit_NeoPixel.h>

#define NEO_PIN   38
#define NEO_COUNT 24

Adafruit_NeoPixel rings(NEO_COUNT, NEO_PIN, NEO_GRB + NEO_KHZ800);

void setup() {
  Serial.begin(115200);
  rings.begin();
  rings.setBrightness(80);
  rings.clear();
  rings.show();
  delay(500);
  Serial.println("NeoPixel test — V3.2");

  // Ring 1 (face-up, white illumination) — first 12 LEDs white
  for (int i = 0; i < 12; i++) {
    rings.setPixelColor(i, rings.Color(255, 255, 255));
  }
  // Ring 2 (face-down, status) — next 12 LEDs green
  for (int i = 12; i < 24; i++) {
    rings.setPixelColor(i, rings.Color(0, 255, 0));
  }
  rings.show();
  Serial.println("Ring 1 = white, Ring 2 = green");
}

void loop() {}
