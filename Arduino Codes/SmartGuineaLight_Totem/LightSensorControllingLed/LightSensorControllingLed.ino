const int redPin   = 19;
const int greenPin = 20;
const int bluePin  = 12;
const int lightPin = 1;

// Two thresholds to prevent feedback loop
const int TURN_ON_THRESHOLD  = 30;   // must get THIS dark to turn on
const int TURN_OFF_THRESHOLD = 150;  // must get THIS bright to turn off

bool lightsAreOn = false;

void lightsOn() {
  digitalWrite(redPin,   HIGH);
  digitalWrite(greenPin, HIGH);
  digitalWrite(bluePin,  HIGH);
  lightsAreOn = true;
}

void lightsOff() {
  digitalWrite(redPin,   LOW);
  digitalWrite(greenPin, LOW);
  digitalWrite(bluePin,  LOW);
  lightsAreOn = false;
}

void setup() {
  pinMode(redPin,   OUTPUT);
  pinMode(greenPin, OUTPUT);
  pinMode(bluePin,  OUTPUT);
  Serial.begin(115200);
  Serial.println("SmartLight ready");
}

void loop() {
  int lightLevel = analogRead(lightPin);
  Serial.print("Light level: ");
  Serial.println(lightLevel);

  if (!lightsAreOn && lightLevel < TURN_ON_THRESHOLD) {
    lightsOn();
    Serial.println("DARK - lights ON");
  } else if (lightsAreOn && lightLevel > TURN_OFF_THRESHOLD) {
    lightsOff();
    Serial.println("BRIGHT - lights OFF");
  }

  delay(500);
}