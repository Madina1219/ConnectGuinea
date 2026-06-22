#include <SPI.h>
#include <SD.h>

#define CS_PIN   7
#define SCK_PIN  5
#define MOSI_PIN 6
#define MISO_PIN 4

SPIClass spi = SPIClass(HSPI);

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("SD card test starting...");

  spi.begin(SCK_PIN, MISO_PIN, MOSI_PIN, CS_PIN);

  if (!SD.begin(CS_PIN, spi, 4000000)) {  // 4MHz - slower speed
    Serial.println("SD card FAILED to initialise.");
    return;
  }
  Serial.println("SD card initialised OK!");

  File f = SD.open("/test.txt", FILE_WRITE);
  if (f) {
    f.println("SmartLight Totem - SD test OK");
    f.close();
    Serial.println("File written OK");
  } else {
    Serial.println("Failed to write file");
  }

  f = SD.open("/test.txt");
  if (f) {
    Serial.println("Reading file:");
    while (f.available()) Serial.write(f.read());
    f.close();
  }
}

void loop() {}