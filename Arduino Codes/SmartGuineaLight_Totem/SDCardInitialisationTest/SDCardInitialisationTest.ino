#include <SPI.h>
#include <SD.h>

#define CS_PIN   33
#define SCK_PIN  34
#define MOSI_PIN 35
#define MISO_PIN 36

SPIClass spi = SPIClass(HSPI);

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("SD card test starting...");

  spi.begin(SCK_PIN, MISO_PIN, MOSI_PIN, CS_PIN);

  if (!SD.begin(CS_PIN, spi, 1000000)) {
    Serial.println("SD card FAILED to initialise.");
    return;
  }
  Serial.println("SD card initialised OK!");

  File f = SD.open("/test.txt", FILE_WRITE);
  if (f) {
    f.println("SmartLight Totem - SD test OK");
    f.close();
    Serial.println("File written OK");
  }

  f = SD.open("/test.txt");
  if (f) {
    Serial.println("Reading file:");
    while (f.available()) Serial.write(f.read());
    f.close();
  }
}

void loop() {}