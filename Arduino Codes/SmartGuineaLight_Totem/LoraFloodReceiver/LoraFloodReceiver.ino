#include "LoRaWan_APP.h"
#include "Arduino.h"

#define RF_FREQUENCY 866300000

static RadioEvents_t RadioEvents;

void OnRxDone(uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr) {
  char msg[64];

  if (size >= sizeof(msg)) size = sizeof(msg) - 1;
  memcpy(msg, payload, size);
  msg[size] = '\0';

  Serial.print("RECEIVED: ");
  Serial.println(msg);

  if (String(msg) == "FLOOD") {
    Serial.println("FLOOD ALERT RECEIVED");
  }

  Radio.Rx(0);
}

void setup() {
  Serial.begin(115200);
  delay(2000);

  Serial.println("TOTEM RECEIVER STARTING");

  Mcu.begin(HELTEC_BOARD, SLOW_CLK_TPYE);

  RadioEvents.RxDone = OnRxDone;

  Radio.Init(&RadioEvents);
  Radio.SetChannel(RF_FREQUENCY);

  Radio.SetRxConfig(MODEM_LORA, 1, 9, 1, 0, 8, 0, false, 0, true, 0, 0, false, true);

  Serial.println("RECEIVER READY - listening...");
  Radio.Rx(0);
}

void loop() {
  Radio.IrqProcess();
}