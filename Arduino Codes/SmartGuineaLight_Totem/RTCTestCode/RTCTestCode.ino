#include <Wire.h>
#include <DS3231.h>

DS3231 rtc;
bool century = false;
bool h12Flag;
bool pmFlag;

// ------------------------------------------------------------------
// STEP 1: Fill these in with the REAL current date/time right before
// you hit Upload (check your PC clock at the moment you click it —
// there's a few seconds of upload delay, so round up slightly).
// Month is 1-12. Year is last two digits only (e.g. 26 for 2026).
// Day-of-week: Sunday=1, Monday=2, Tuesday=3, Wednesday=4,
//              Thursday=5, Friday=6, Saturday=7.
// ------------------------------------------------------------------
const byte SET_SECOND = 0;
const byte SET_MINUTE = 55;
const byte SET_HOUR   = 22;   // 24-hour format
const byte SET_DOW     = 4;   // Wednesday
const byte SET_DATE    = 8;
const byte SET_MONTH   = 7;
const byte SET_YEAR    = 26;

// STEP 2: leave this as true for ONE upload to write the time above.
// STEP 3: after confirming the Serial Monitor shows the correct time
//         and it's counting up correctly second by second, change
//         this to false and re-upload. That stops the RTC being
//         reset back to these fixed values on every future boot —
//         from then on it just keeps ticking off its own battery-
//         backed oscillator (CR2032), even through power cycles.
#define SET_TIME_NOW false

void setup() {
  Serial.begin(115200);
  delay(1500); // give Serial Monitor time to connect before first prints
  Wire.begin();

  rtc.setClockMode(false); // false = 24 hour mode, true = 12 hour mode

  if (SET_TIME_NOW) {
    rtc.setSecond(SET_SECOND);
    rtc.setMinute(SET_MINUTE);
    rtc.setHour(SET_HOUR);
    rtc.setDoW(SET_DOW);
    rtc.setDate(SET_DATE);
    rtc.setMonth(SET_MONTH);
    rtc.setYear(SET_YEAR);
    Serial.println("RTC time has been SET to the values above.");
    Serial.println("Once you confirm this is correct below, change");
    Serial.println("SET_TIME_NOW to false and re-upload.");
    Serial.println("----");
  }

  Serial.println("DS3231 RTC test - V3 totem");
}

void loop() {
  // Print as YYYY-MM-DD HH:MM:SS
  Serial.print(2000 + rtc.getYear(), DEC);
  Serial.print("-");
  int month = rtc.getMonth(century);
  if (month < 10) Serial.print("0");
  Serial.print(month, DEC);
  Serial.print("-");
  int date = rtc.getDate();
  if (date < 10) Serial.print("0");
  Serial.print(date, DEC);
  Serial.print(" ");

  int hour = rtc.getHour(h12Flag, pmFlag);
  if (hour < 10) Serial.print("0");
  Serial.print(hour, DEC);
  Serial.print(":");
  int minute = rtc.getMinute();
  if (minute < 10) Serial.print("0");
  Serial.print(minute, DEC);
  Serial.print(":");
  int second = rtc.getSecond();
  if (second < 10) Serial.print("0");
  Serial.println(second, DEC);

  delay(1000);
}