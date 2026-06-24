#include <Adafruit_NeoPixel.h>

// RGB LED pins
const int redPin   = 19;
const int greenPin = 20;
const int bluePin  = 21;

// NeoPixel - 2 rings chained = 24 LEDs
#define NEO_PIN    38
#define NUMPIXELS  24
Adafruit_NeoPixel rings(NUMPIXELS, NEO_PIN, NEO_GRB + NEO_KHZ800);

// Button
const int buttonPin = 47;

// Ring helpers
void setTopRing(uint8_t r, uint8_t g, uint8_t b) {
  for (int i = 0; i < 12; i++) rings.setPixelColor(i, rings.Color(r, g, b));
  rings.show();
}

void setBottomRing(uint8_t r, uint8_t g, uint8_t b) {
  for (int i = 12; i < 24; i++) rings.setPixelColor(i, rings.Color(r, g, b));
  rings.show();
}

void allRingsOff() {
  rings.clear();
  rings.show();
}

void lightsOn() {
  digitalWrite(redPin,   HIGH);
  digitalWrite(greenPin, HIGH);
  digitalWrite(bluePin,  HIGH);
}

void lightsOff() {
  digitalWrite(redPin,   LOW);
  digitalWrite(greenPin, LOW);
  digitalWrite(bluePin,  LOW);
}

void setup() {
  pinMode(redPin,    OUTPUT);
  pinMode(greenPin,  OUTPUT);
  pinMode(bluePin,   OUTPUT);
  pinMode(buttonPin, INPUT_PULLUP);

  rings.begin();
  rings.setBrightness(80);
  rings.show();

  Serial.begin(115200);
  Serial.println("SmartLight ready");
}

void loop() {
  bool buttonOn = (digitalRead(buttonPin) == LOW);

  if (buttonOn) {
    lightsOn();
    setTopRing(255, 255, 255); // white = illumination
    setBottomRing(0, 255, 0);  // green = active
    Serial.println("ON");
  } else {
    lightsOff();
    allRingsOff();
    Serial.println("OFF");
  }

  delay(100);
}