#include <WiFi.h>
#include <esp_now.h>
#include <esp_wifi.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <RTClib.h>
#include <Adafruit_INA219.h>
#include <Adafruit_NeoPixel.h>

// ─────────────────────────────────────────────
// RGB LED STRIP
// ─────────────────────────────────────────────
const int redPin   = 19;
const int greenPin = 20;
const int bluePin  = 4;

// ─────────────────────────────────────────────
// LIGHT SENSOR
// ─────────────────────────────────────────────
const int lightPin = 1;

const int TURN_ON_THRESHOLD  = 50;
const int TURN_OFF_THRESHOLD = 400;

bool lightsAreOn = false;

// ─────────────────────────────────────────────
// NEOPIXEL RINGS
// 24 LEDs total:
// LEDs 0–11   = top illumination ring
// LEDs 12–23 = bottom status ring
// ─────────────────────────────────────────────
#define NEO_PIN    38
#define NUMPIXELS  24

Adafruit_NeoPixel rings(
  NUMPIXELS,
  NEO_PIN,
  NEO_GRB + NEO_KHZ800
);

// ─────────────────────────────────────────────
// INA219 CURRENT SENSOR
// ─────────────────────────────────────────────
Adafruit_INA219 ina219(0x41);
bool inaReady = false;

// ─────────────────────────────────────────────
// DS3231 RTC
// ─────────────────────────────────────────────
RTC_DS3231 rtc;
bool rtcReady = false;

// ─────────────────────────────────────────────
// SD CARD
// ─────────────────────────────────────────────
#define SD_CS   33
#define SD_SCK  34
#define SD_MOSI 35
#define SD_MISO 36

SPIClass spi = SPIClass(HSPI);
bool sdReady = false;

// ─────────────────────────────────────────────
// ESP-NOW MESSAGE
// ─────────────────────────────────────────────
typedef struct struct_message {
  char text[32];
} struct_message;

struct_message incomingData;

volatile bool floodMessageReceived = false;

// ─────────────────────────────────────────────
// FLOOD ALERT TIMING
// ─────────────────────────────────────────────
bool floodAlert = false;
bool lightsWereOnBeforeFlood = false;

unsigned long floodStartTime = 0;

const unsigned long FLOOD_FLASH_TIME     = 5000;
const unsigned long FLOOD_STEADY_TIME    = 2000;
const unsigned long FLOOD_TOTAL_TIME     =
  FLOOD_FLASH_TIME + FLOOD_STEADY_TIME;

const unsigned long FLOOD_FLASH_INTERVAL = 250;

// ─────────────────────────────────────────────
// POWER LOGGING
// ─────────────────────────────────────────────
unsigned long lastPowerLog = 0;
unsigned long lastEnergyUpdate = 0;

const unsigned long LOG_INTERVAL = 5000;

float dailyEnergy_mWh = 0.0;

// ─────────────────────────────────────────────
// NEOPIXEL HELPERS
// ─────────────────────────────────────────────
void setTopRing(uint8_t r, uint8_t g, uint8_t b) {
  for (int i = 0; i < 12; i++) {
    rings.setPixelColor(i, rings.Color(r, g, b));
  }

  rings.show();
}

void setBottomRing(uint8_t r, uint8_t g, uint8_t b) {
  for (int i = 12; i < 24; i++) {
    rings.setPixelColor(i, rings.Color(r, g, b));
  }

  rings.show();
}

void allRingsOff() {
  rings.clear();
  rings.show();
}

// ─────────────────────────────────────────────
// RGB LED STRIP HELPERS
// ─────────────────────────────────────────────
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

// ─────────────────────────────────────────────
// NORMAL OPERATING MODE
// ─────────────────────────────────────────────
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

// ─────────────────────────────────────────────
// ESP-NOW RECEIVE CALLBACK
// ─────────────────────────────────────────────
void OnDataRecv(
  const esp_now_recv_info *info,
  const uint8_t *incomingDataPtr,
  int len
) {
  if (len <= 0) {
    return;
  }

  memset(&incomingData, 0, sizeof(incomingData));

  int bytesToCopy = len;

  if (bytesToCopy > sizeof(incomingData)) {
    bytesToCopy = sizeof(incomingData);
  }

  memcpy(
    &incomingData,
    incomingDataPtr,
    bytesToCopy
  );

  incomingData.text[31] = '\0';

  Serial.print("RECEIVED: ");
  Serial.println(incomingData.text);

  if (strcmp(incomingData.text, "FLOOD") == 0) {
    floodMessageReceived = true;
  }
}

// ─────────────────────────────────────────────
// START FLOOD ALERT
// ─────────────────────────────────────────────
void triggerFloodAlert() {
  lightsWereOnBeforeFlood = lightsAreOn;

  floodAlert = true;
  floodStartTime = millis();

  Serial.println("FLOOD ALERT RECEIVED");
  Serial.println("Blue flashing for 3 seconds");
  Serial.println("Blue steady for 2 seconds");
}

// ─────────────────────────────────────────────
// FLOOD LIGHT SEQUENCE
// ─────────────────────────────────────────────
void updateFloodState() {
  if (!floodAlert) {
    return;
  }

  unsigned long elapsed =
    millis() - floodStartTime;

  // Stage 1:
  // Flash blue for 3 seconds
  if (elapsed < FLOOD_FLASH_TIME) {
    bool flashOn =
      ((elapsed / FLOOD_FLASH_INTERVAL) % 2) == 0;

    if (flashOn) {
      stripBlue();
      setTopRing(0, 0, 255);
    } else {
      stripOff();
      setTopRing(0, 0, 0);
    }

    return;
  }

  // Stage 2:
  // Steady blue for 2 seconds
  if (elapsed < FLOOD_TOTAL_TIME) {
    stripBlue();
    setTopRing(0, 0, 255);

    return;
  }

  // Stage 3:
  // Return to previous normal state
  floodAlert = false;

  if (lightsWereOnBeforeFlood) {
    normalLightsOn();
  } else {
    normalLightsOff();
  }

  Serial.println(
    "FLOOD ALERT CLEARED - NORMAL MODE RESTORED"
  );
}

// ─────────────────────────────────────────────
// STATUS RING
// ─────────────────────────────────────────────
void updateStatusRing() {
  if (floodAlert) {
    unsigned long elapsed =
      millis() - floodStartTime;

    // Flash red during first 3 seconds
    if (elapsed < FLOOD_FLASH_TIME) {
      bool flashOn =
        ((elapsed / FLOOD_FLASH_INTERVAL) % 2) == 0;

      if (flashOn) {
        setBottomRing(255, 0, 0);
      } else {
        setBottomRing(0, 0, 0);
      }

      return;
    }

    // Steady red during next 2 seconds
    setBottomRing(255, 0, 0);

    return;
  }

  if (lightsAreOn) {
    setBottomRing(0, 255, 0);
  } else {
    setBottomRing(255, 80, 0);
  }
}

// ─────────────────────────────────────────────
// SD CARD LOGGING
// ─────────────────────────────────────────────
void logToSD(
  String timestamp,
  String state,
  bool inaOk,
  float voltage,
  float current,
  float power,
  int lightLevel
) {
  if (!sdReady) {
    return;
  }

  File file =
    SD.open("/totem_log.csv", FILE_APPEND);

  if (!file) {
    Serial.println("Could not open SD log file");

    return;
  }

  file.print(timestamp);
  file.print(",");

  file.print(millis());
  file.print(",");

  file.print(state);
  file.print(",");

  if (inaOk) {
    file.print(voltage, 3);
    file.print(",");
    file.print(current, 3);
    file.print(",");
    file.print(power, 3);
  } else {
    file.print("NA,NA,NA");
  }

  file.print(",");

  file.print(dailyEnergy_mWh, 3);
  file.print(",");

  file.println(lightLevel);

  file.close();
}

// ─────────────────────────────────────────────
// POWER MEASUREMENT
// ─────────────────────────────────────────────
void logPower(int lightLevel) {
  float voltage = 0.0;
  float current_mA = 0.0;
  float power_mW = 0.0;

  if (inaReady) {
    voltage = ina219.getBusVoltage_V();
    current_mA = ina219.getCurrent_mA();
    power_mW = ina219.getPower_mW();
  }

  unsigned long now = millis();

  if (lastEnergyUpdate == 0) {
    lastEnergyUpdate = now;
  }

  float hoursPassed =
    (now - lastEnergyUpdate) / 3600000.0;

  dailyEnergy_mWh +=
    power_mW * hoursPassed;

  lastEnergyUpdate = now;

  String stateStr;

  if (floodAlert) {
    stateStr = "FLOOD_ALERT";
  } else if (lightsAreOn) {
    stateStr = "LIGHTS_ON";
  } else {
    stateStr = "RESTING";
  }

  String timestamp = "NO_RTC";

  if (rtcReady) {
    DateTime rtcNow = rtc.now();

    char buf[20];

    sprintf(
      buf,
      "%04d-%02d-%02d %02d:%02d:%02d",
      rtcNow.year(), rtcNow.month(), rtcNow.day(),
      rtcNow.hour(), rtcNow.minute(), rtcNow.second()
    );

    timestamp = String(buf);
  }

  Serial.println(
    "─── TOTEM POWER ─────────────"
  );

  Serial.print("Time: ");
  Serial.println(timestamp);

  Serial.print("Voltage: ");
  Serial.print(voltage, 3);
  Serial.println(" V");

  Serial.print("Current: ");
  Serial.print(current_mA, 3);
  Serial.println(" mA");

  Serial.print("Power:   ");
  Serial.print(power_mW, 3);
  Serial.println(" mW");

  Serial.print("Energy today: ");
  Serial.print(dailyEnergy_mWh, 3);
  Serial.println(" mWh");

  Serial.print("Light level: ");
  Serial.println(lightLevel);

  Serial.print("State: ");
  Serial.println(stateStr);

  Serial.print("INA219: ");
  Serial.println(
    inaReady ? "OK" : "NOT FOUND"
  );

  Serial.print("RTC: ");
  Serial.println(
    rtcReady ? "OK" : "NOT FOUND"
  );

  Serial.print("SD: ");
  Serial.println(
    sdReady ? "OK" : "NOT FOUND"
  );

  Serial.println(
    "─────────────────────────────"
  );

  logToSD(
    timestamp,
    stateStr,
    inaReady,
    voltage,
    current_mA,
    power_mW,
    lightLevel
  );
}

// ─────────────────────────────────────────────
// SETUP
// ─────────────────────────────────────────────
void setup() {
  Serial.begin(115200);
  delay(2000);

  pinMode(redPin, OUTPUT);
  pinMode(greenPin, OUTPUT);
  pinMode(bluePin, OUTPUT);

  stripOff();

  rings.begin();
  rings.setBrightness(80);
  allRingsOff();

  // I2C bus (shared by INA219 and RTC)
  Wire.begin(41, 42);

  if (!ina219.begin()) {
    Serial.println("INA219 not found");
    inaReady = false;
  } else {
    Serial.println("INA219 OK");
    inaReady = true;
  }

  if (!rtc.begin()) {
    Serial.println("RTC not found");
    rtcReady = false;
  } else {
    Serial.println("RTC OK");
    rtcReady = true;

    // Only uncomment the line below ONCE to set the
    // time from your computer's clock, then upload,
    // then re-comment it and upload again.
    // Leaving it active resets the clock to compile
    // time on every single reboot.
    // rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }

  // SD card
  spi.begin(
    SD_SCK,
    SD_MISO,
    SD_MOSI,
    SD_CS
  );

  if (!SD.begin(SD_CS, spi, 1000000)) {
    Serial.println("SD card FAILED");
    sdReady = false;
  } else {
    Serial.println("SD card OK");
    sdReady = true;

    if (!SD.exists("/totem_log.csv")) {
      File file =
        SD.open("/totem_log.csv", FILE_WRITE);

      if (file) {
        file.println(
          "timestamp,millis,state,voltage_V,current_mA,"
          "power_mW,daily_energy_mWh,light_level"
        );

        file.close();
      }
    }
  }

  // ESP-NOW
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();

  esp_wifi_set_channel(
    1,
    WIFI_SECOND_CHAN_NONE
  );

  if (esp_now_init() != ESP_OK) {
    Serial.println(
      "Error initializing ESP-NOW"
    );
  } else {
    esp_now_register_recv_cb(OnDataRecv);

    Serial.println(
      "ESP-NOW receiver ready"
    );
  }

  Serial.print("Receiver MAC Address: ");
  Serial.println(WiFi.macAddress());

  lastEnergyUpdate = millis();

  normalLightsOff();
  updateStatusRing();

  Serial.println(
    "SmartLight Totem Exhibition Ready"
  );
}

// ─────────────────────────────────────────────
// LOOP
// ─────────────────────────────────────────────
void loop() {
  int lightLevel =
    analogRead(lightPin);

  // Process flood message safely
  // outside the ESP-NOW callback
  if (floodMessageReceived) {
    floodMessageReceived = false;

    triggerFloodAlert();
  }

  // Normal light-sensor behaviour
  // is paused during flood alerts
  if (!floodAlert) {
    if (
      !lightsAreOn &&
      lightLevel < TURN_ON_THRESHOLD
    ) {
      normalLightsOn();

      Serial.println(
        "DARK - LIGHTS ON"
      );
    } else if (
      lightsAreOn &&
      lightLevel > TURN_OFF_THRESHOLD
    ) {
      normalLightsOff();

      Serial.println(
        "BRIGHT - LIGHTS OFF"
      );
    }
  }

  updateFloodState();
  updateStatusRing();

  if (
    millis() - lastPowerLog >=
    LOG_INTERVAL
  ) {
    logPower(lightLevel);

    lastPowerLog = millis();
  }

  delay(5);
}