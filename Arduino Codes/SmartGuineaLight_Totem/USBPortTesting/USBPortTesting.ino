void setup() {
  pinMode(40, OUTPUT);
  digitalWrite(40, HIGH);
  Serial.begin(115200);
  Serial.println("GPIO40 HIGH");
}
void loop() {}