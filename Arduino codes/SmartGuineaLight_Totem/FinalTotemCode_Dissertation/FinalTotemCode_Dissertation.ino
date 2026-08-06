#include <WiFi.h>
#include <esp_now.h>
#include <esp_wifi.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <RTClib.h>
#include <Adafruit_INA219.h>
#include <Adafruit_NeoPixel.h>
#include <math.h>

// ================================================================
// RTC TIME SETTING
// ================================================================
//
// FIRST UPLOAD:
// Change FORCE_SET_RTC to true.
// Upload while your computer clock shows the correct UK time.
//
// SECOND UPLOAD:
// Change FORCE_SET_RTC back to false.
// Upload again.
//
// After that, leave it false. The RTC coin cell will preserve time
// whenever the Heltec or main battery turns off.
//
#define FORCE_SET_RTC false

// ================================================================
// RGB LED STRIP
// ================================================================
const int redPin   = 19;
const int greenPin = 20;
const int bluePin  = 4;

// ================================================================
// LIGHT SENSOR
// ================================================================
const int lightPin = 1;

const int TURN_ON_THRESHOLD  = 50;
const int TURN_OFF_THRESHOLD = 400;

bool lightsAreOn = false;

// ================================================================
// NEOPIXEL RINGS
//
// LEDs 0–11   = top illumination ring
// LEDs 12–23  = bottom status ring
// ================================================================
#define NEO_PIN   38
#define NUMPIXELS 24

Adafruit_NeoPixel rings(
  NUMPIXELS,
  NEO_PIN,
  NEO_GRB + NEO_KHZ800
);

// ================================================================
// I2C PINS
// ================================================================
constexpr int I2C_SDA_PIN = 41;
constexpr int I2C_SCL_PIN = 42;

// ================================================================
// INA219 CURRENT SENSOR
//
// Keep 0x41 because this is the address currently working in your
// assembled Totem.
// ================================================================
Adafruit_INA219 ina219(0x41);
bool inaReady = false;

// ================================================================
// DS3231 RTC
// ================================================================
RTC_DS3231 rtc;
bool rtcReady = false;

// ================================================================
// SD CARD
// ================================================================
#define SD_CS   33
#define SD_SCK  34
#define SD_MOSI 35
#define SD_MISO 36

SPIClass spi = SPIClass(HSPI);
bool sdReady = false;

const char *LOG_FILE = "/totem_log.csv";

// ================================================================
// ESP-NOW MESSAGE
// ================================================================
typedef struct struct_message {
  char text[32];
} struct_message;

struct_message incomingData;

volatile bool floodMessageReceived = false;

// ================================================================
// FLOOD ALERT TIMING
// ================================================================
bool floodAlert = false;
bool lightsWereOnBeforeFlood = false;

unsigned long floodStartTime = 0;

const unsigned long FLOOD_FLASH_TIME     = 5000;
const unsigned long FLOOD_STEADY_TIME    = 2000;
const unsigned long FLOOD_TOTAL_TIME     =
  FLOOD_FLASH_TIME + FLOOD_STEADY_TIME;

const unsigned long FLOOD_FLASH_INTERVAL = 250;

// ================================================================
// DATA LOGGING
// ================================================================
unsigned long lastPowerLog = 0;
unsigned long lastEnergyUpdate = 0;

const unsigned long LOG_INTERVAL = 5000;

// Energy accumulated since the most recent Heltec startup.
// It will restart from zero after complete Heltec power loss.
// The raw timestamped power data remains available for analysis.
double sessionEnergy_mWh = 0.0;

unsigned long sampleNumber = 0;

// ================================================================
// RTC DATE AND TIME STRUCTURE
// ================================================================
struct TimeRecord {
  String timestamp;
  String date;
  String dayName;
  String time;
  uint32_t unixTime;
  bool valid;
};

// ================================================================
// NEOPIXEL HELPERS
// ================================================================
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

// ================================================================
// RGB LED STRIP HELPERS
// ================================================================
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

// ================================================================
// NORMAL OPERATING MODE
// ================================================================
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

// ================================================================
// ESP-NOW RECEIVE CALLBACK
// ================================================================
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

// ================================================================
// START FLOOD ALERT
// ================================================================
void triggerFloodAlert() {
  lightsWereOnBeforeFlood = lightsAreOn;

  floodAlert = true;
  floodStartTime = millis();

  Serial.println("FLOOD ALERT RECEIVED");
  Serial.println("Blue flashing for 5 seconds");
  Serial.println("Blue steady for 2 seconds");
}

// ================================================================
// FLOOD LIGHT SEQUENCE
// ================================================================
void updateFloodState() {
  if (!floodAlert) {
    return;
  }

  unsigned long elapsed =
    millis() - floodStartTime;

  // Stage 1: flash blue.
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

  // Stage 2: steady blue.
  if (elapsed < FLOOD_TOTAL_TIME) {
    stripBlue();
    setTopRing(0, 0, 255);

    return;
  }

  // Stage 3: restore normal state.
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

// ================================================================
// STATUS RING
// ================================================================
void updateStatusRing() {
  if (floodAlert) {
    unsigned long elapsed =
      millis() - floodStartTime;

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

    setBottomRing(255, 0, 0);

    return;
  }

  if (lightsAreOn) {
    // Green = light active and system operating normally.
    setBottomRing(0, 255, 0);
  } else {
    // Amber = resting / charging state.
    setBottomRing(255, 80, 0);
  }
}

// ================================================================
// DAY NAME
// ================================================================
String getDayName(uint8_t dayOfWeek) {
  const char *dayNames[] = {
    "Sunday",
    "Monday",
    "Tuesday",
    "Wednesday",
    "Thursday",
    "Friday",
    "Saturday"
  };

  if (dayOfWeek > 6) {
    return "UNKNOWN";
  }

  return String(dayNames[dayOfWeek]);
}

// ================================================================
// GET RTC DATE AND TIME
// ================================================================
TimeRecord getTimeRecord() {
  TimeRecord result;

  result.timestamp = "NO_RTC";
  result.date       = "NO_DATE";
  result.dayName    = "NO_DAY";
  result.time       = "NO_TIME";
  result.unixTime   = 0;
  result.valid      = false;

  if (!rtcReady) {
    return result;
  }

  DateTime now = rtc.now();

  // Reject obviously invalid RTC years.
  if (now.year() < 2025 || now.year() > 2099) {
    return result;
  }

  char dateBuffer[11];
  char timeBuffer[9];
  char timestampBuffer[20];

  snprintf(
    dateBuffer,
    sizeof(dateBuffer),
    "%04d-%02d-%02d",
    now.year(),
    now.month(),
    now.day()
  );

  snprintf(
    timeBuffer,
    sizeof(timeBuffer),
    "%02d:%02d:%02d",
    now.hour(),
    now.minute(),
    now.second()
  );

  snprintf(
    timestampBuffer,
    sizeof(timestampBuffer),
    "%04d-%02d-%02d %02d:%02d:%02d",
    now.year(),
    now.month(),
    now.day(),
    now.hour(),
    now.minute(),
    now.second()
  );

  result.timestamp = String(timestampBuffer);
  result.date       = String(dateBuffer);
  result.dayName    = getDayName(now.dayOfTheWeek());
  result.time       = String(timeBuffer);
  result.unixTime   = now.unixtime();
  result.valid      = true;

  return result;
}

// ================================================================
// WRITE CSV HEADER
// ================================================================
void createLogFileIfNeeded() {
  if (!sdReady) {
    return;
  }

  if (SD.exists(LOG_FILE)) {
    return;
  }

  File file = SD.open(LOG_FILE, FILE_WRITE);

  if (!file) {
    Serial.println("Could not create SD log file");
    return;
  }

  file.println(
    "sample,"
    "date,"
    "day,"
    "time,"
    "timestamp,"
    "unix_time,"
    "uptime_seconds,"
    "system_state,"
    "flood_alert,"
    "lights_on,"
    "bus_voltage_V,"
    "shunt_voltage_mV,"
    "load_voltage_V,"
    "current_mA,"
    "power_mW,"
    "session_energy_mWh,"
    "light_level,"
    "ina219_ok,"
    "rtc_ok,"
    "sd_ok"
  );

  file.close();

  Serial.println("New CSV log file created");
}

// ================================================================
// SD CARD LOGGING
// ================================================================
void logToSD(
  const TimeRecord &timeRecord,
  const String &state,
  bool inaOk,
  float busVoltage,
  float shuntVoltage_mV,
  float loadVoltage,
  float current_mA,
  float power_mW,
  int lightLevel
) {
  if (!sdReady) {
    return;
  }

  File file = SD.open(LOG_FILE, FILE_APPEND);

  if (!file) {
    Serial.println("Could not open SD log file");
    return;
  }

  sampleNumber++;

  file.print(sampleNumber);
  file.print(",");

  file.print(timeRecord.date);
  file.print(",");

  file.print(timeRecord.dayName);
  file.print(",");

  file.print(timeRecord.time);
  file.print(",");

  file.print(timeRecord.timestamp);
  file.print(",");

  file.print(timeRecord.unixTime);
  file.print(",");

  file.print(millis() / 1000UL);
  file.print(",");

  file.print(state);
  file.print(",");

  file.print(floodAlert ? 1 : 0);
  file.print(",");

  file.print(lightsAreOn ? 1 : 0);
  file.print(",");

  if (inaOk) {
    file.print(busVoltage, 3);
    file.print(",");

    file.print(shuntVoltage_mV, 3);
    file.print(",");

    file.print(loadVoltage, 3);
    file.print(",");

    file.print(current_mA, 3);
    file.print(",");

    file.print(power_mW, 3);
  } else {
    file.print("NA,NA,NA,NA,NA");
  }

  file.print(",");

  file.print(sessionEnergy_mWh, 6);
  file.print(",");

  file.print(lightLevel);
  file.print(",");

  file.print(inaReady ? 1 : 0);
  file.print(",");

  file.print(rtcReady ? 1 : 0);
  file.print(",");

  file.println(sdReady ? 1 : 0);

  file.flush();
  file.close();
}

// ================================================================
// POWER MEASUREMENT
// ================================================================
void logPower(int lightLevel) {
  float busVoltage = 0.0;
  float shuntVoltage_mV = 0.0;
  float loadVoltage = 0.0;
  float current_mA = 0.0;
  float power_mW = 0.0;

  bool measurementValid = inaReady;

  if (inaReady) {
    busVoltage =
      ina219.getBusVoltage_V();

    shuntVoltage_mV =
      ina219.getShuntVoltage_mV();

    current_mA =
      ina219.getCurrent_mA();

    power_mW =
      ina219.getPower_mW();

    loadVoltage =
      busVoltage + (shuntVoltage_mV / 1000.0);

    // Protect the dataset from failed or invalid readings.
    if (
      isnan(busVoltage) ||
      isnan(shuntVoltage_mV) ||
      isnan(current_mA) ||
      isnan(power_mW) ||
      isinf(busVoltage) ||
      isinf(shuntVoltage_mV) ||
      isinf(current_mA) ||
      isinf(power_mW)
    ) {
      measurementValid = false;

      busVoltage = 0.0;
      shuntVoltage_mV = 0.0;
      loadVoltage = 0.0;
      current_mA = 0.0;
      power_mW = 0.0;
    }
  }

  unsigned long nowMillis = millis();

  if (lastEnergyUpdate == 0) {
    lastEnergyUpdate = nowMillis;
  }

  float hoursPassed =
    (nowMillis - lastEnergyUpdate) / 3600000.0;

  if (measurementValid) {
    sessionEnergy_mWh +=
      power_mW * hoursPassed;
  }

  lastEnergyUpdate = nowMillis;

  String stateStr;

  if (floodAlert) {
    stateStr = "FLOOD_ALERT";
  } else if (lightsAreOn) {
    stateStr = "LIGHTS_ON";
  } else {
    stateStr = "RESTING";
  }

  TimeRecord timeRecord =
    getTimeRecord();

  Serial.println(
    "─── TOTEM POWER ───────────────────"
  );

  Serial.print("Time: ");
  Serial.println(timeRecord.timestamp);

  Serial.print("Day: ");
  Serial.println(timeRecord.dayName);

  Serial.print("Bus voltage: ");
  Serial.print(busVoltage, 3);
  Serial.println(" V");

  Serial.print("Shunt voltage: ");
  Serial.print(shuntVoltage_mV, 3);
  Serial.println(" mV");

  Serial.print("Load voltage: ");
  Serial.print(loadVoltage, 3);
  Serial.println(" V");

  Serial.print("Current: ");
  Serial.print(current_mA, 3);
  Serial.println(" mA");

  Serial.print("Power: ");
  Serial.print(power_mW, 3);
  Serial.println(" mW");

  Serial.print("Session energy: ");
  Serial.print(sessionEnergy_mWh, 6);
  Serial.println(" mWh");

  Serial.print("Light level: ");
  Serial.println(lightLevel);

  Serial.print("State: ");
  Serial.println(stateStr);

  Serial.print("INA219: ");
  Serial.println(
    measurementValid ? "OK" : "READ ERROR"
  );

  Serial.print("RTC: ");
  Serial.println(
    timeRecord.valid ? "OK" : "INVALID TIME"
  );

  Serial.print("SD: ");
  Serial.println(
    sdReady ? "OK" : "NOT FOUND"
  );

  Serial.println(
    "───────────────────────────────────"
  );

  logToSD(
    timeRecord,
    stateStr,
    measurementValid,
    busVoltage,
    shuntVoltage_mV,
    loadVoltage,
    current_mA,
    power_mW,
    lightLevel
  );
}

// ================================================================
// SETUP
// ================================================================
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

  // Start shared I2C bus.
  Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN);
  Wire.setClock(100000);
  delay(100);

  // --------------------------------------------------------------
  // INA219
  // --------------------------------------------------------------
  if (!ina219.begin(&Wire)) {
    Serial.println("INA219 not found");
    inaReady = false;
  } else {
    Serial.println("INA219 OK");
    inaReady = true;
  }

  // --------------------------------------------------------------
  // RTC
  // --------------------------------------------------------------
  if (!rtc.begin(&Wire)) {
    Serial.println("RTC not found");
    rtcReady = false;
  } else {
    Serial.println("RTC OK");
    rtcReady = true;

    if (FORCE_SET_RTC) {
      rtc.adjust(
        DateTime(
          F(__DATE__),
          F(__TIME__)
        )
      );

      Serial.println(
        "RTC SET FROM COMPUTER COMPILE TIME"
      );
    }

    if (rtc.lostPower()) {
      Serial.println(
        "WARNING: RTC previously lost backup power"
      );

      if (!FORCE_SET_RTC) {
        Serial.println(
          "Set FORCE_SET_RTC to true and upload once"
        );
      }
    }

    TimeRecord startupTime =
      getTimeRecord();

    Serial.print("RTC current time: ");
    Serial.println(startupTime.timestamp);
  }

  // --------------------------------------------------------------
  // SD CARD
  // --------------------------------------------------------------
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

    createLogFileIfNeeded();
  }

  // --------------------------------------------------------------
  // ESP-NOW
  // --------------------------------------------------------------
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

// ================================================================
// LOOP
// ================================================================
void loop() {
  int lightLevel =
    analogRead(lightPin);

  // Process flood messages outside the ESP-NOW callback.
  if (floodMessageReceived) {
    floodMessageReceived = false;

    triggerFloodAlert();
  }

  // Pause normal automatic lighting during flood alerts.
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
