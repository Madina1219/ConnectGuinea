#include <Adafruit_NeoPixel.h>

// RGB LED pins
const int redPin   = 19;
const int greenPin = 20;
const int bluePin  = 21;

// NeoPixel
#define NEO_PIN     38
#define NUMPIXELS   8
Adafruit_NeoPixel ring(NUMPIXELS, NEO_PIN, NEO_GRB + NEO_KHZ800);

// Button
const int buttonPin = 47;

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

void setRing(uint8_t r, uint8_t g, uint8_t b) {
  for (int i = 0; i < NUMPIXELS; i++) ring.setPixelColor(i, ring.Color(r, g, b));
  ring.show();
}

void setup() {
  pinMode(redPin,   OUTPUT);
  pinMode(greenPin, OUTPUT);
  pinMode(bluePin,  OUTPUT);
  pinMode(buttonPin, INPUT_PULLUP);

  ring.begin();
  ring.setBrightness(80);
  ring.show();

  Serial.begin(115200);
  Serial.println("SmartLight ready");
}

void loop() {
  bool buttonOn = (digitalRead(buttonPin) == LOW);

  if (buttonOn) {
    lightsOn();
    setRing(0, 255, 0); // green = ON
    Serial.println("ON");
  } else {
    lightsOff();
    setRing(255, 100, 0); // amber = OFF
    Serial.println("OFF");
  }

  delay(100);
}