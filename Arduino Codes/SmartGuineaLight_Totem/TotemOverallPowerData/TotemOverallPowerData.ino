#include <Wire.h>
#include <Adafruit_INA219.h>
#include <Adafruit_NeoPixel.h>

// ─── RGB LED STRIP ───────────────────────────────
const int redPin   = 19;
const int greenPin = 20;
const int bluePin  = 21;

// ─── LIGHT SENSOR ────────────────────────────────
const int lightPin = 1;
const int TURN_ON_THRESHOLD  = 50;
const int TURN_OFF_THRESHOLD = 400;
bool lightsAreOn = false;

// ─── NEOPIXEL ────────────────────────────────────
#define NEO_PIN     38
#define NUMPIXELS   8
Adafruit_NeoPixel ring(NUMPIXELS, NEO_PIN, NEO_GRB + NEO_KHZ800);

// ─── BUTTON ──────────────────────────────────────
const int buttonPin = 47;
bool lastButtonState = false;

// ─── INA219 POWER MONITOR ────────────────────────
Adafruit_INA219 ina219(0x41);

// ─── STATE MACHINE ───────────────────────────────
enum State { STANDBY, LIGHTS_ON, BLINKING };
State currentState = STANDBY;
unsigned long onStartTime = 0;
unsigned long lastBlinkTime = 0;
bool blinkState = false;
const unsigned long ON_DURATION = 5 * 60 * 1000UL; // 5 minutes

// ─── POWER LOGGING ───────────────────────────────
unsigned long lastPowerLog = 0;
const unsigned long LOG_INTERVAL = 5000; // log every 5 seconds

// ─── HELPERS ─────────────────────────────────────
void lightsOn() {
  digitalWrite(redPin,   HIGH);
  digitalWrite(greenPin, HIGH);
  digitalWrite(bluePin,  HIGH);
  lightsAreOn = true;
}

void lightsOff() {
  digitalWrite(redPin,   LOW);
  digitalWrite(greenPin, LOW);
  digitalWrite(bluePin,  LOW);
  lightsAreOn = false;
}

void setRing(uint8_t r, uint8_t g, uint8_t b) {
  for (int i = 0; i < NUMPIXELS; i++) ring.setPixelColor(i, ring.Color(r, g, b));
  ring.show();
}

void ringOff() {
  ring.clear();
  ring.show();
}

void logPower() {
  float voltage    = ina219.getBusVoltage_V();
  float current_mA = ina219.getCurrent_mA();
  float power_mW   = ina219.getPower_mW();

  Serial.println("─── POWER ───────────────────");
  Serial.print("Voltage: "); Serial.print(voltage);    Serial.println(" V");
  Serial.print("Current: "); Serial.print(current_mA); Serial.println(" mA");
  Serial.print("Power:   "); Serial.print(power_mW);   Serial.println(" mW");

  // State label
  Serial.print("State:   ");
  if (currentState == STANDBY)   Serial.println("STANDBY");
  if (currentState == LIGHTS_ON) Serial.println("LIGHTS ON");
  if (currentState == BLINKING)  Serial.println("BLINKING");
  Serial.println("─────────────────────────────");
}

void setup() {
  // RGB pins
  pinMode(redPin,   OUTPUT);
  pinMode(greenPin, OUTPUT);
  pinMode(bluePin,  OUTPUT);

  // Button
  pinMode(buttonPin, INPUT_PULLUP);

  // NeoPixel
  ring.begin();
  ring.setBrightness(80);
  ring.show();

  // INA219
  Wire.begin(41, 42);
  Serial.begin(115200);
  delay(1000);

  if (!ina219.begin()) {
    Serial.println("INA219 not found - check wiring!");
    while (1) delay(10);
  }

  Serial.println("SmartLight Totem - Full System Ready");
  setRing(255, 100, 0); // amber = standby
}

void loop() {
  // ── Button ──
  bool buttonPressed = (digitalRead(buttonPin) == LOW);
  bool justPressed   = buttonPressed && !lastButtonState;
  lastButtonState    = buttonPressed;

  // ── Light sensor ──
  int lightLevel = analogRead(lightPin);

  // ── State machine ──
  switch (currentState) {

    case STANDBY:
      lightsOff();
      setRing(255, 100, 0); // amber = standby

      // Auto on if dark
      if (!lightsAreOn && lightLevel < TURN_ON_THRESHOLD) {
        currentState = LIGHTS_ON;
        onStartTime  = millis();
        Serial.println("DARK detected - AUTO ON");
      }
      // Manual on via button
      if (justPressed) {
        currentState = LIGHTS_ON;
        onStartTime  = millis();
        Serial.println("Button pressed - MANUAL ON");
      }
      break;

    case LIGHTS_ON:
      lightsOn();
      setRing(0, 255, 0); // green = active

      // Auto off if bright
      if (lightsAreOn && lightLevel > TURN_OFF_THRESHOLD) {
        lightsOff();
        currentState = STANDBY;
        Serial.println("BRIGHT detected - AUTO OFF");
      }
      // Timer expired
      if (millis() - onStartTime >= ON_DURATION) {
        lightsOff();
        currentState = BLINKING;
        Serial.println("Timer expired - BLINKING");
      }
      // Button resets timer
      if (justPressed) {
        onStartTime = millis();
        Serial.println("Timer reset");
      }
      break;

    case BLINKING:
      lightsOff();
      if (millis() - lastBlinkTime >= 500) {
        blinkState = !blinkState;
        if (blinkState) setRing(255, 180, 0); // yellow blink
        else ringOff();
        lastBlinkTime = millis();
      }
      // Button restarts
      if (justPressed) {
        currentState = LIGHTS_ON;
        onStartTime  = millis();
        Serial.println("Button pressed - RESTART");
      }
      break;
  }

  // ── Power logging every 5 seconds ──
  if (millis() - lastPowerLog >= LOG_INTERVAL) {
    logPower();
    lastPowerLog = millis();
  }

  delay(50);
}