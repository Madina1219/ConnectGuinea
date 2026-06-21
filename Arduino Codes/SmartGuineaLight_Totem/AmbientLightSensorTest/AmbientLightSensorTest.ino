// RGB LED pins
const int redPin   = 19;
const int greenPin = 20;
const int bluePin  = 21;

// TEMT6000 on GPIO1 (ADC1_CH0)
const int lightPin = 1;

void allOff() {
  digitalWrite(redPin,   LOW);
  digitalWrite(greenPin, LOW);
  digitalWrite(bluePin,  LOW);
}

void setup() {
  pinMode(redPin,   OUTPUT);
  pinMode(greenPin, OUTPUT);
  pinMode(bluePin,  OUTPUT);
  Serial.begin(115200);
  Serial.println("TEMT6000 light sensor test starting...");
}

void loop() {
  int lightLevel = analogRead(lightPin); // 0 (dark) to 4095 (bright)
  Serial.print("Light level: ");
  Serial.println(lightLevel);

  allOff();

  if (lightLevel < 500) {
    digitalWrite(redPin, HIGH);
    Serial.println("DARK → RED");
  } else if (lightLevel < 2000) {
    digitalWrite(greenPin, HIGH);
    Serial.println("DIM → GREEN");
  } else {
    digitalWrite(bluePin, HIGH);
    Serial.println("BRIGHT → BLUE");
  }

  delay(300);
}