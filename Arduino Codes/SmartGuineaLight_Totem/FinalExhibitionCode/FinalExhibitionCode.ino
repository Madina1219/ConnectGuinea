#include <WiFi.h>
#include <esp_now.h>
#include <esp_wifi.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <Adafruit_INA219.h>
#include <Adafruit_NeoPixel.h>

// RGB LED STRIP
const int redPin   = 19;
const int greenPin = 20;
const int bluePin  = 4;

// LIGHT SENSOR
const int lightPin = 1;
const int TURN_ON_THRESHOLD  = 50;
const int TURN_OFF_THRESHOLD = 400;
bool lightsAreOn = false;

// NEOPIXEL RINGS
#define NEO_PIN 38
#define NUMPIXELS 24
Adafruit_NeoPixel rings(NUMPIXELS, NEO_PIN, NEO_GRB + NEO_KHZ800);

// INA219
Adafruit_INA219 ina219(0x41);

// SD CARD
#define SD_CS   33
#define SD_SCK  34
#define SD_MOSI 35
#define SD_MISO 36
SPIClass spi = SPIClass(HSPI);
bool sdReady = false;

// ESP-NOW MESSAGE
typedef struct struct_message {
  char text[32];
} struct_message;

struct_message incomingData;
volatile bool floodMessageReceived = false;

// FLOOD TIMERS
bool floodAlert = false;
unsigned long floodStartTime = 0;
const unsigned long FLOOD_FLASH_TIME = 10000;  // 10 sec
const unsigned long FLOOD_TOTAL_TIME = 20000;  // total alert time

// LOGGING
unsigned long lastPowerLog = 0;
unsigned long lastEnergyUpdate = 0;
const unsigned long LOG_INTERVAL = 5000;
float dailyEnergy_mWh = 0;

// RINGS
void setTopRing(uint8_t r, uint8_t g, uint8_t b) {
  for (int i = 0; i < 12; i++) rings.setPixelColor(i, rings.Color(r, g, b));
  rings.show();
}

void setBottomRing(uint8_t r, uint8_t g, uint8_t b) {
  for (int i = 12; i < 24; i++) rings.setPixelColor(i, rings.Color(r, g, b));
  rings.show();
}

// STRIP
void stripOff() {
  digitalWrite(redPin, LOW);
  digitalWrite(greenPin, LOW);
  digitalWrite(bluePin, LOW);
}

void stripWhite() {
  digitalWrite(redPin, HIGH);
  digitalWrite(greenPin, HIGH);
  digitalWrite(bluePin, HIGH);
}

void stripBlue() {
  digitalWrite(redPin, LOW);
  digitalWrite(greenPin, LOW);
  digitalWrite(bluePin, HIGH);
}

void normalLightsOn() {
  stripWhite();
  setTopRing(255, 255, 255);
  lightsAreOn = true;
}

void normalLightsOff() {
  stripOff();
  setTopRing(0, 0, 0);
  lightsAreOn = false;
}

// ESP-NOW CALLBACK
void OnDataRecv(const esp_now_recv_info *info, const uint8_t *incomingDataPtr, int len) {
  memcpy(&incomingData, incomingDataPtr, sizeof(incomingData));

  Serial.print("RECEIVED: ");
  Serial.println(incomingData.text);

  if (strcmp(incomingData.text, "FLOOD") == 0) {
    floodMessageReceived = true;
  }
}

void triggerFloodAlert() {
  floodAlert = true;
  floodStartTime = millis();

  stripBlue();
  setTopRing(0, 0, 255);

  Serial.println("FLOOD ALERT ACTIVE - STRIP BLUE, STATUS FLASHING RED");
}

void updateStatusRing() {
  if (floodAlert) {
    unsigned long elapsed = millis() - floodStartTime;

    if (elapsed < FLOOD_FLASH_TIME) {
      if ((millis() / 300) % 2 == 0) {
        setBottomRing(255, 0, 0);   // flashing red
      } else {
        setBottomRing(0, 0, 0);
      }
    } else {
      setBottomRing(255, 0, 0);     // steady red
    }
    return;
  }

  if (lightsAreOn) {
    setBottomRing(0, 255, 0);       // green = normal active
  } else {
    setBottomRing(255, 80, 0);      // amber = resting/daytime
  }
}

void updateFloodState(int lightLevel) {
  if (!floodAlert) return;

  unsigned long elapsed = millis() - floodStartTime;

  if (elapsed >= FLOOD_FLASH_TIME) {
    if (lightsAreOn || lightLevel < TURN_ON_THRESHOLD) {
      normalLightsOn();
    } else {
      normalLightsOff();
    }
  }

  if (elapsed >= FLOOD_TOTAL_TIME) {
    floodAlert = false;
    Serial.println("Flood alert cleared");
  }
}

void logToSD(String state, float voltage, float current, float power, int lightLevel) {
  if (!sdReady) return;

  File f = SD.open("/totem_log.csv", FILE_APPEND);
  if (f) {
    f.print(millis()); f.print(",");
    f.print(state); f.print(",");
    f.print(voltage); f.print(",");
    f.print(current); f.print(",");
    f.print(power); f.print(",");
    f.print(dailyEnergy_mWh); f.print(",");
    f.println(lightLevel);
    f.close();
  }
}

void logPower(int lightLevel) {
  float voltage = ina219.getBusVoltage_V();
  float current_mA = ina219.getCurrent_mA();
  float power_mW = ina219.getPower_mW();

  unsigned long now = millis();
  float hoursPassed = (now - lastEnergyUpdate) / 3600000.0;
  dailyEnergy_mWh += power_mW * hoursPassed;
  lastEnergyUpdate = now;

  String stateStr = floodAlert ? "FLOOD_ALERT" :
                    lightsAreOn ? "LIGHTS_ON" : "RESTING";

  Serial.println("─── TOTEM POWER ─────────────");
  Serial.print("Voltage: "); Serial.print(voltage); Serial.println(" V");
  Serial.print("Current: "); Serial.print(current_mA); Serial.println(" mA");
  Serial.print("Power:   "); Serial.print(power_mW); Serial.println(" mW");
  Serial.print("Energy today: "); Serial.print(dailyEnergy_mWh); Serial.println(" mWh");
  Serial.print("Light level: "); Serial.println(lightLevel);
  Serial.print("State: "); Serial.println(stateStr);
  Serial.print("SD: "); Serial.println(sdReady ? "OK" : "NOT FOUND");
  Serial.println("─────────────────────────────");

  logToSD(stateStr, voltage, current_mA, power_mW, lightLevel);
}

void setup() {
  Serial.begin(115200);
  delay(2000);

  pinMode(redPin, OUTPUT);
  pinMode(greenPin, OUTPUT);
  pinMode(bluePin, OUTPUT);
  stripOff();

  rings.begin();
  rings.setBrightness(80);
  rings.clear();
  rings.show();

  Wire.begin(41, 42);

  if (!ina219.begin()) {
    Serial.println("INA219 not found");
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
        f.println("millis,state,voltage_V,current_mA,power_mW,daily_energy_mWh,light_level");
        f.close();
      }
    }
  }

  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  esp_wifi_set_channel(1, WIFI_SECOND_CHAN_NONE);

  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
  } else {
    esp_now_register_recv_cb(OnDataRecv);
    Serial.println("ESP-NOW receiver ready");
  }

  Serial.print("Receiver MAC Address: ");
  Serial.println(WiFi.macAddress());

  lastEnergyUpdate = millis();

  normalLightsOff();
  updateStatusRing();

  Serial.println("SmartLight Totem Exhibition Ready");
}

void loop() {
  int lightLevel = analogRead(lightPin);

  if (floodMessageReceived) {
    floodMessageReceived = false;
    triggerFloodAlert();
  }

  if (!floodAlert) {
    if (!lightsAreOn && lightLevel < TURN_ON_THRESHOLD) {
      normalLightsOn();
      Serial.println("DARK - LIGHTS ON");
    } else if (lightsAreOn && lightLevel > TURN_OFF_THRESHOLD) {
      normalLightsOff();
      Serial.println("BRIGHT - LIGHTS OFF");
    }
  }

  updateFloodState(lightLevel);
  updateStatusRing();

  if (millis() - lastPowerLog >= LOG_INTERVAL) {
    logPower(lightLevel);
    lastPowerLog = millis();
  }
}