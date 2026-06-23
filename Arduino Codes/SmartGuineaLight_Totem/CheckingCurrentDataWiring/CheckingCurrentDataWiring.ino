#include <Wire.h>
#include <Adafruit_INA219.h>

Adafruit_INA219 ina219(0x41); // specify address 0x41

void setup() {
  Serial.begin(115200);
  delay(1000);
  Wire.begin(41, 42);
  Serial.println("INA219 Power Monitor");

  if (!ina219.begin()) {
    Serial.println("Failed to find INA219!");
    while (1) delay(10);
  }
  Serial.println("INA219 found OK!");
}

void loop() {
  float busVoltage = ina219.getBusVoltage_V();
  float current_mA = ina219.getCurrent_mA();
  float power_mW   = ina219.getPower_mW();

  Serial.println("-----------------------------");
  Serial.print("Voltage: "); Serial.print(busVoltage); Serial.println(" V");
  Serial.print("Current: "); Serial.print(current_mA); Serial.println(" mA");
  Serial.print("Power:   "); Serial.print(power_mW);   Serial.println(" mW");
  Serial.println("-----------------------------");

  delay(1000);
}