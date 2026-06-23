#include <Wire.h>
#include <Adafruit_INA219.h>

Adafruit_INA219 ina219(0x41);

const int redPin   = 19;
const int greenPin = 20;
const int bluePin  = 21;

void setup() {
  pinMode(redPin,   OUTPUT);
  pinMode(greenPin, OUTPUT);
  pinMode(bluePin,  OUTPUT);
  digitalWrite(redPin,   LOW);
  digitalWrite(greenPin, LOW);
  digitalWrite(bluePin,  LOW);

  Serial.begin(115200);
  delay(1000);
  Wire.begin(41, 42);
  ina219.begin();
  Serial.println("LED strip OFF - Heltec only");
}

void loop() {
  float busVoltage = ina219.getBusVoltage_V();
  float current_mA = ina219.getCurrent_mA();
  float power_mW   = ina219.getPower_mW();

  Serial.print("Voltage: "); Serial.print(busVoltage); Serial.println(" V");
  Serial.print("Current: "); Serial.print(current_mA); Serial.println(" mA");
  Serial.print("Power:   "); Serial.print(power_mW);   Serial.println(" mW");
  Serial.println("----");
  delay(1000);
}