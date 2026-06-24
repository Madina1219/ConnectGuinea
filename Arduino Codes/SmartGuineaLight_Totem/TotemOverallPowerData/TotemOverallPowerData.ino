#include <Wire.h>
#include <SPI.h>
#include <SD.h>
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

// ─── NEOPIXEL (2 rings chained = 24 LEDs) ────────
#define NEO_PIN    38
#define NUMPIXELS  24
Adafruit_NeoPixel rings(NUMPIXELS, NEO_PIN, NEO_GRB + NEO_KHZ800);

// ─── BUTTON ──────────────────────────────────────
const int buttonPin = 47;
bool lastButtonState = false;

// ─── INA219 ──────────────────────────────────────
Adafruit_INA219 ina219(0x41);

// ─── SD CARD ─────────────────────────────────────
#define SD_CS   33
#define SD_SCK  34
#define SD_MOSI 35
#define SD_MISO 36
SPIClass spi = SPIClass(HSPI);
bool sdReady = false;

// ─── STATE MACHINE ───────────────────────────────
enum State { STANDBY, LIGHTS_ON, BLINKING };
State currentState = STANDBY;
unsigned long onStartTime  = 0;
unsigned long lastBlinkTime = 0;
bool blinkState = false;
const unsigned long ON_DURATION = 30 * 1000UL; // 30 seconds for testing

// ─── TIMERS ──────────────────────────────────────
unsigned long lastPowerLog = 0;
const unsigned long LOG_INTERVAL = 5000;

// ─── RING HELPERS ────────────────────────────────
// Ring 1 (0–11) = top ring, facing up = white ambient light
// Ring 2 (12–23) = bottom ring, facing down = status indicator

void setTopRing(uint8_t r, uint8_t g, uint8_t b) {
  for (int i = 0; i < 12; i++) rings.setPixelColor(i, rings.Color(r, g, b));
  rings.show();
}

void setBottomRing(uint8_t r, uint8_t g, uint8_t b) {
  for (int i = 12; i < 24; i++) rings.setPixelColor(i, rings.Color(r, g, b));
  rings.show();
}

void setAllRings(uint8_t r, uint8_t g, uint8_t b) {
  for (int i = 0; i < NUMPIXELS; i++) rings.setPixelColor(i, rings.Color(r, g, b));
  rings.show();
}

void ringsOff() {
  rings.clear();
  rings.show();
}

// ─── RGB LED HELPERS ─────────────────────────────
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

// ─── SD CARD LOGGING ─────────────────────────────
void logToSD(String state, float voltage, float current, float power, int lightLevel) {
  if (!sdReady) return;
  File f = SD.open("/totem_log.csv", FILE_APPEND);
  if (f) {
    f.print(millis());
    f.print(",");
    f.print(state);
    f.print(",");
    f.print(voltage);
    f.print(",");
    f.print(current);
    f.print(",");
    f.print(power);
    f.print(",");
    f.println(lightLevel);
    f.close();
  }
}

// ─── POWER LOGGING ───────────────────────────────
void logPower() {
  float voltage    = ina219.getBusVoltage_V();
  float current_mA = ina219.getCurrent_mA();
  float power_mW   = ina219.getPower_mW();
  int   lightLevel = analogRead(lightPin);

  String stateStr = "";
  if (currentState == STANDBY)   stateStr = "STANDBY";
  if (currentState == LIGHTS_ON) stateStr = "LIGHTS_ON";
  if (currentState == BLINKING)  stateStr = "BLINKING";

  Serial.println("─── POWER ───────────────────");
  Serial.print("Voltage:    "); Serial.print(voltage);    Serial.println(" V");
  Serial.print("Current:    "); Serial.print(current_mA); Serial.println(" mA");
  Serial.print("Power:      "); Serial.print(power_mW);   Serial.println(" mW");
  Serial.print("Light lvl:  "); Serial.println(lightLevel);
  Serial.print("State:      "); Serial.println(stateStr);
  Serial.print("SD card:    "); Serial.println(sdReady ? "OK" : "NOT FOUND");
  Serial.println("─────────────────────────────");

  logToSD(stateStr, voltage, current_mA, power_mW, lightLevel);
}

// ─── SETUP ───────────────────────────────────────
void setup() {
  pinMode(redPin,    OUTPUT);
  pinMode(greenPin,  OUTPUT);
  pinMode(bluePin,   OUTPUT);
  pinMode(buttonPin, INPUT_PULLUP);

  Serial.begin(115200);
  delay(1000);

  // NeoPixel
  rings.begin();
  rings.setBrightness(80);
  ringsOff();

  // INA219
  Wire.begin(41, 42);
  if (!ina219.begin()) {
    Serial.println("INA219 not found!");
  } else {
    Serial.println("INA219 OK");
  }

  // SD card
  spi.begin(SD_SCK, SD_MISO, SD_MOSI, SD_CS);
  if (!SD.begin(SD_CS, spi, 1000000)) {
    Serial.println("SD card FAILED");
    sdReady = false;
  } else {
    Serial.println("SD card OK");
    sdReady = true;
    // Write CSV header if file doesn't exist
    if (!SD.exists("/totem_log.csv")) {
      File f = SD.open("/totem_log.csv", FILE_WRITE);
      if (f) {
        f.println("millis,state,voltage_V,current_mA,power_mW,light_level");
        f.close();
      }
    }
  }

  Serial.println("SmartLight Totem - Full System Ready");
  setBottomRing(255, 100, 0); // amber = standby on status ring
  setTopRing(0, 0, 0);        // top ring off at start
}

// ─── LOOP ────────────────────────────────────────
void loop() {
  bool buttonPressed = (digitalRead(buttonPin) == LOW);
  bool justPressed   = buttonPressed && !lastButtonState;
  lastButtonState    = buttonPressed;

  int lightLevel = analogRead(lightPin);

  switch (currentState) {

    case STANDBY:
      lightsOff();
      setTopRing(0, 0, 0);           // top ring off
      setBottomRing(255, 100, 0);    // amber = standby

      if (!lightsAreOn && lightLevel < TURN_ON_THRESHOLD) {
        currentState = LIGHTS_ON;
        onStartTime  = millis();
        Serial.println("DARK - AUTO ON");
      }
      if (justPressed) {
        currentState = LIGHTS_ON;
        onStartTime  = millis();
        Serial.println("Button - MANUAL ON");
      }
      break;

    case LIGHTS_ON:
      lightsOn();
      setTopRing(255, 255, 255);     // top ring white = illumination
      setBottomRing(0, 255, 0);      // bottom ring green = active

      if (lightsAreOn && lightLevel > TURN_OFF_THRESHOLD) {
        lightsOff();
        currentState = STANDBY;
        Serial.println("BRIGHT - AUTO OFF");
      }
      if (millis() - onStartTime >= ON_DURATION) {
        lightsOff();
        currentState = BLINKING;
        Serial.println("Timer expired - BLINKING");
      }
      if (justPressed) {
        onStartTime = millis();
        Serial.println("Timer reset");
      }
      break;

    case BLINKING:
      lightsOff();
      setTopRing(0, 0, 0);
      if (millis() - lastBlinkTime >= 500) {
        blinkState = !blinkState;
        if (blinkState) setBottomRing(255, 180, 0); // yellow blink
        else setBottomRing(0, 0, 0);
        lastBlinkTime = millis();
      }
      if (justPressed) {
        currentState = LIGHTS_ON;
        onStartTime  = millis();
        Serial.println("Button - RESTART");
      }
      break;
  }

  // Power log every 5 seconds
  if (millis() - lastPowerLog >= LOG_INTERVAL) {
    logPower();
    lastPowerLog = millis();
  }

  delay(50);
}