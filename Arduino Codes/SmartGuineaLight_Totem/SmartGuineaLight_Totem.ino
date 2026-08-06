#include <Wire.h>

constexpr int SDA_PIN = 41;
constexpr int SCL_PIN = 42;

void setup() {
  Serial.begin(115200);
  delay(1500);

  Wire.begin(SDA_PIN, SCL_PIN);
  Wire.setClock(100000);

  Serial.println();
  Serial.println("RTC-only I2C test");
  Serial.println("-----------------");

  Serial.print("SDA state: ");
  Serial.println(digitalRead(SDA_PIN) ? "HIGH" : "LOW");

  Serial.print("SCL state: ");
  Serial.println(digitalRead(SCL_PIN) ? "HIGH" : "LOW");

  Wire.beginTransmission(0x68);
  uint8_t result = Wire.endTransmission();

  Serial.print("RTC address 0x68 response: ");
  Serial.println(result);

  if (result == 0) {
    Serial.println("RTC FOUND at 0x68.");
  } else {
    Serial.println("RTC NOT FOUND.");
  }
}

void loop() {
}