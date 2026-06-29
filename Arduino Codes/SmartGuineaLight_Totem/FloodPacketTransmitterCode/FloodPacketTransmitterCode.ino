#include "LoRaWan_APP.h"

#define RF_FREQUENCY        868000000
#define TX_OUTPUT_POWER     14
#define LORA_BANDWIDTH      0
#define LORA_SPREADING_FACTOR 7
#define LORA_CODINGRATE     1
#define LORA_PREAMBLE_LENGTH 8
#define LORA_FIX_LENGTH_PAYLOAD_ON false
#define LORA_IQ_INVERSION_ON false

// Built-in PRG button
const int buttonPin = 0;
bool lastButtonState = true;

void setup() {
  Serial.begin(115200);
  pinMode(buttonPin, INPUT_PULLUP);

  Radio.Init(NULL);
  Radio.SetChannel(RF_FREQUENCY);
  Radio.SetTxConfig(
    MODEM_LORA, TX_OUTPUT_POWER, 0,
    LORA_BANDWIDTH, LORA_SPREADING_FACTOR,
    LORA_CODINGRATE, LORA_PREAMBLE_LENGTH,
    LORA_FIX_LENGTH_PAYLOAD_ON, true, 0, 0,
    LORA_IQ_INVERSION_ON, 3000
  );

  Serial.println("Flood Alert Transmitter ready");
  Serial.println("Press PRG button to send FLOOD alert");
}

void loop() {
  bool buttonState = digitalRead(buttonPin);

  if (buttonState == LOW && lastButtonState == HIGH) {
    Serial.println("Button pressed - sending FLOOD alert...");

    uint8_t payload[] = "FLOOD";
    Radio.Send(payload, sizeof(payload));
    delay(100);
    Radio.Sleep();

    Serial.println("FLOOD packet sent!");
    delay(500); // debounce
  }

  lastButtonState = buttonState;
  delay(10);
}