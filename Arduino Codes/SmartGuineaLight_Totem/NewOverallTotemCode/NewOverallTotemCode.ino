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

// ─── INA219 ──────────────────────────────────────
Adafruit_INA219 ina219(0x41);

// ─── SD CARD ─────────────────────────────────────
#define SD_CS   33
#define SD_SCK  34
#define SD_MOSI 35
#define SD_MISO 36
SPIClass spi = SPIClass(HSPI);
bool sdReady = false;

// ─── FLOOD ALERT ─────────────────────────────────
// Set to true when LoRa flood alert received (placeholder for now)
bool floodAlert = false;

// ─── TIMERS ──────────────────────────────────────
unsigned long lastPowerLog = 0;
const unsigned long LOG_INTERVAL = 5000;

// ─── RING HELPERS ────────────────────────────────
// Ring 1 (LEDs 0–12)  = top ring, faces up = white illumination only
// Ring 2 (LEDs 13–23) = bottom ring, faces down = status indicator

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

// ─── STATUS RING UPDATE ───────────────────────────
void updateStatusRing() {
  if (floodAlert) {
    setBottomRing(0, 0, 255);    // BLUE = flood alert — overrides everything
  } else if (lightsAreOn) {
    setBottomRing(0, 255, 0);    // GREEN = night, lights on, system active
  } else {
    setBottomRing(255, 0, 0);    // RED = daytime, lights off, monitoring
  }
}

// ─── SD LOGGING ──────────────────────────────────
void logToSD(String state, float voltage, float current, float power, int lightLevel) {
  if (!sdReady) return;
  File f = SD.open("/totem_log.csv", FILE_APPEND);
  if (f) {
    f.print(millis());   f.print(",");
    f.print(state);      f.print(",");
    f.print(voltage);    f.print(",");
    f.print(current);    f.print(",");
    f.print(power);      f.print(",");
    f.println(lightLevel);
    f.close();
  }
}

// ─── POWER LOG ───────────────────────────────────
void logPower(int lightLevel) {
  float voltage    = ina219.getBusVoltage_V();
  float current_mA = ina219.getCurrent_mA();
  float power_mW   = ina219.getPower_mW();
  String stateStr  = floodAlert ? "FLOOD_ALERT" :
                     lightsAreOn ? "LIGHTS_ON" : "STANDBY";

  Serial.println("─── POWER ───────────────────");
  Serial.print("Voltage:   "); Serial.print(voltage);    Serial.println(" V");
  Serial.print("Current:   "); Serial.print(current_mA); Serial.println(" mA");
  Serial.print("Power:     "); Serial.print(power_mW);   Serial.println(" mW");
  Serial.print("Light lvl: "); Serial.println(lightLevel);
  Serial.print("State:     "); Serial.println(stateStr);
  Serial.print("Flood:     "); Serial.println(floodAlert ? "ALERT!" : "clear");
  Serial.print("SD card:   "); Serial.println(sdReady ? "OK" : "NOT FOUND");
  Serial.println("─────────────────────────────");

  logToSD(stateStr, voltage, current_mA, power_mW, lightLevel);
}

// ─── SETUP ───────────────────────────────────────
void setup() {
  pinMode(redPin,   OUTPUT);
  pinMode(greenPin, OUTPUT);
  pinMode(bluePin,  OUTPUT);

  Serial.begin(115200);
  delay(1000);

  rings.begin();
  rings.setBrightness(80);
  allRingsOff();

  Wire.begin(41, 42);
  if (!ina219.begin()) {
    Serial.println("INA219 not found!");
  } else {
    Serial.println("INA219 OK");
  }

  spi.begin(SD_SCK, SD_MISO, SD_MOSI, SD_CS);
  if (!SD.begin(SD_CS, spi, 1000000)) {
    Serial.println("SD card FAILED");
    sdReady = false;
  } else {
    Serial.println("SD card OK");
    sdReady = true;
    if (!SD.exists("/totem_log.csv")) {
      File f = SD.open("/totem_log.csv", FILE_WRITE);
      if (f) {
        f.println("millis,state,voltage_V,current_mA,power_mW,light_level");
        f.close();
      }
    }
  }

  Serial.println("SmartLight Totem - Ready");
  updateStatusRing(); // red at start = daytime/standby
  setTopRing(0, 0, 0);
}

// ─── LOOP ────────────────────────────────────────
void loop() {
  int lightLevel = analogRead(lightPin);

  // ── Light sensor — top ring + LED strip ──
  if (!lightsAreOn && lightLevel < TURN_ON_THRESHOLD) {
    lightsOn();
    setTopRing(255, 255, 255);
    Serial.println("DARK - LIGHTS ON");
  } else if (lightsAreOn && lightLevel > TURN_OFF_THRESHOLD) {
    lightsOff();
    setTopRing(0, 0, 0);
    Serial.println("BRIGHT - LIGHTS OFF");
  }

  // ── Always update status ring ──
  updateStatusRing();

  // ── Power log every 5 seconds ──
  if (millis() - lastPowerLog >= LOG_INTERVAL) {
    logPower(lightLevel);
    lastPowerLog = millis();
  }
}
