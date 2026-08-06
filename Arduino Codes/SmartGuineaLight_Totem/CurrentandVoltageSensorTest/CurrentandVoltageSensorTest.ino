constexpr int SDA_PIN = 41;
constexpr int SCL_PIN = 42;

void printLineStates(const char* stage) {
  Serial.print(stage);

  Serial.print(" | SDA: ");
  Serial.print(digitalRead(SDA_PIN) == HIGH ? "HIGH" : "LOW");

  Serial.print(" | SCL: ");
  Serial.println(digitalRead(SCL_PIN) == HIGH ? "HIGH" : "LOW");
}

void setup() {
  Serial.begin(115200);
  delay(1500);

  Serial.println();
  Serial.println("GPIO 42 / I2C SCL diagnostic");
  Serial.println("----------------------------");

  // Release both lines and enable weak internal pull-ups.
  pinMode(SDA_PIN, INPUT_PULLUP);
  pinMode(SCL_PIN, INPUT_PULLUP);

  delay(500);
  printLineStates("Initial state");

  if (digitalRead(SCL_PIN) == LOW) {
    Serial.println("SCL is being held LOW.");
  } else {
    Serial.println("SCL can rise normally.");
  }

  /*
    Try standard I2C bus recovery:
    pull SCL low, then release it, up to nine times.
    We never drive the line HIGH.
  */
  Serial.println("Attempting nine recovery clock pulses...");

  for (int pulse = 1; pulse <= 9; pulse++) {
    // Pull SCL low safely.
    pinMode(SCL_PIN, OUTPUT_OPEN_DRAIN);
    digitalWrite(SCL_PIN, LOW);
    delayMicroseconds(10);

    // Release SCL and let the pull-up raise it.
    pinMode(SCL_PIN, INPUT_PULLUP);
    delayMicroseconds(10);

    Serial.print("Pulse ");
    Serial.print(pulse);
    Serial.print(": SCL = ");
    Serial.println(digitalRead(SCL_PIN) == HIGH ? "HIGH" : "LOW");
  }

  delay(100);
  printLineStates("After recovery");

  if (digitalRead(SCL_PIN) == LOW) {
    Serial.println();
    Serial.println("RESULT: SCL remains LOW.");
    Serial.println("A connected device or wiring path is holding GPIO 42 low.");
  } else {
    Serial.println();
    Serial.println("RESULT: SCL recovered and is now HIGH.");
    Serial.println("A device may previously have been stuck mid-transaction.");
  }
}

void loop() {
}