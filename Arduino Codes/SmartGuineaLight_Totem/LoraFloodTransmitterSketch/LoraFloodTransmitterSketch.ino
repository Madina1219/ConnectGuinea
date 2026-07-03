#include "LoRaWan_APP.h"
#include "Arduino.h"

#define RF_FREQUENCY 866300000
#define PRG_BUTTON 0

static RadioEvents_t RadioEvents;

void OnTxDone(void) {
  Serial.println("Sent OK");
}

void OnTxTimeout(void) {
  Serial.println("TX Timeout");
}

void setup() {
  Serial.begin(115200);
  delay(2000);

  pinMode(PRG_BUTTON, INPUT_PULLUP);

  Serial.println("FLOOD TRANSMITTER STARTING");

  Mcu.begin(HELTEC_BOARD, SLOW_CLK_TPYE);

  RadioEvents.TxDone = OnTxDone;
  RadioEvents.TxTimeout = OnTxTimeout;

  Radio.Init(&RadioEvents);
  Radio.SetChannel(RF_FREQUENCY);

  Radio.SetTxConfig(MODEM_LORA, 0, 0, 1, 9, 1, 8, false, true, 0, 0, false, 3000);

  Serial.println("TRANSMITTER READY - press PRG");
}

void loop() {
  Radio.IrqProcess();

  if (digitalRead(PRG_BUTTON) == LOW) {
    Serial.println("Sending FLOOD...");
    Radio.Send((uint8_t *)"FLOOD", 5);
    delay(1500);
  }
}