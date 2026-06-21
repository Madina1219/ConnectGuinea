// RGB LED pins
const int redPin   = 19;
const int greenPin = 20;
const int bluePin  = 21;

void setup() {
  pinMode(redPin,   OUTPUT);
  pinMode(greenPin, OUTPUT);
  pinMode(bluePin,  OUTPUT);

  digitalWrite(redPin,   HIGH);
  digitalWrite(greenPin, HIGH);
  digitalWrite(bluePin,  HIGH);

  Serial.begin(115200);
  Serial.println("White - steady");
}

void loop() {
  // nothing needed
}
