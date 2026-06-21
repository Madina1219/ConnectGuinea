// RGB LED pins
const int redPin   = 19;
const int greenPin = 20;
const int bluePin  = 21;

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
}

void loop() {
  // RED
  allOff();
  digitalWrite(redPin, HIGH);
  Serial.println("RED");
  delay(2000);

  // GREEN
  allOff();
  digitalWrite(greenPin, HIGH);
  Serial.println("GREEN");
  delay(2000);

  // BLUE
  allOff();
  digitalWrite(bluePin, HIGH);
  Serial.println("BLUE");
  delay(2000);
}