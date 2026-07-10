#include <WiFi.h>
#include <esp_now.h>
#include <esp_wifi.h>

#define PRG_BUTTON 0

uint8_t broadcastAddress[] = {
  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF
};

typedef struct struct_message {
  char text[32];
} struct_message;

struct_message outgoingData;

bool lastButtonState = HIGH;

void OnDataSent(const wifi_tx_info_t *info, esp_now_send_status_t status) {
  Serial.print("Send result: ");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "SUCCESS" : "FAILED");
}

void setup() {
  Serial.begin(115200);
  delay(1500);

  pinMode(PRG_BUTTON, INPUT_PULLUP);

  WiFi.mode(WIFI_STA);
  WiFi.disconnect();

  esp_wifi_set_channel(1, WIFI_SECOND_CHAN_NONE);

  if (esp_now_init() != ESP_OK) {
    Serial.println("ESP-NOW initialization failed");
    return;
  }

  esp_now_register_send_cb(OnDataSent);

  esp_now_peer_info_t peerInfo = {};
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.channel = 1;
  peerInfo.encrypt = false;

  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("Failed to add broadcast peer");
    return;
  }

  strcpy(outgoingData.text, "FLOOD");

  Serial.print("Transmitter MAC: ");
  Serial.println(WiFi.macAddress());

  Serial.println("TRANSMITTER READY");
  Serial.println("Press PRG to send FLOOD");
}

void loop() {
  bool currentButtonState = digitalRead(PRG_BUTTON);

  if (lastButtonState == HIGH && currentButtonState == LOW) {
    Serial.println("PRG PRESSED - SENDING FLOOD");

    esp_err_t result = esp_now_send(
      broadcastAddress,
      reinterpret_cast<uint8_t *>(&outgoingData),
      sizeof(outgoingData)
    );

    if (result != ESP_OK) {
      Serial.print("Send command error: ");
      Serial.println(result);
    }
  }

  lastButtonState = currentButtonState;
  delay(30);
}