
#include <Wire.h>
#include <DS3231.h>

DS3231 rtc;

void setup() {
  Serial.begin(115200);
  delay(1000);
  Wire.begin(41, 42);

  Serial.println("DS3231 RTC test — V3 totem");

  // Set time once — comment out after first upload
  // Year, Month, Day, Hour, Minute, Second
  // rtc.setYear(26);    // 2026
  // rtc.setMonth(7);    // July
  // rtc.setDate(2);     // 2nd
  // rtc.setHour(15);    // 15:00
  // rtc.setMinute(30);
  // rtc.setSecond(0);

  Serial.println("Time set — reading now:");
}

void loop() {
  bool h12Flag, pmFlag;
  bool century = false;

  int sec  = rtc.getSecond();
  int mins = rtc.getMinute();
  int hour = rtc.getHour(h12Flag, pmFlag);
  int day  = rtc.getDate();
  int mon  = rtc.getMonth(century);
  int yr   = rtc.getYear();

  // Print timestamp
  Serial.print("20"); Serial.print(yr);  Serial.print("-");
  if (mon  < 10) Serial.print("0"); Serial.print(mon);  Serial.print("-");
  if (day  < 10) Serial.print("0"); Serial.print(day);  Serial.print(" ");
  if (hour < 10) Serial.print("0"); Serial.print(hour); Serial.print(":");
  if (mins < 10) Serial.print("0"); Serial.print(mins); Serial.print(":");
  if (sec  < 10) Serial.print("0"); Serial.println(sec);

  delay(1000);
}