#include <Wire.h>
#include <DS3231.h>

DS3231 rtc;
bool century = false;
bool h12Flag;
bool pmFlag;

void setup() {
  Serial.begin(115200);
  delay(1000);
  Wire.begin(41, 42);

  Serial.println("DS3231 RTC test - V3 totem");

  // --- END SET BLOCK ---
}

void loop() {
  Serial.print(rtc.getYear(), DEC);
  Serial.print("-");
  Serial.print(rtc.getMonth(century), DEC);
  Serial.print("-");
  Serial.print(rtc.getDate(), DEC);
  Serial.print(" ");
  Serial.print(rtc.getHour(h12Flag, pmFlag), DEC);
  Serial.print(":");
  Serial.print(rtc.getMinute(), DEC);
  Serial.print(":");
  Serial.println(rtc.getSecond(), DEC);
  delay(1000);
}