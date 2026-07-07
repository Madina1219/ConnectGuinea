#include <WiFi.h>
#include <esp_now.h>
#include <esp_wifi.h>

#define PRG_BUTTON 0

uint8_t broadcastAddress[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

typedef struct struct_message {
  char text[32];
} struct_message;

struct_message myData;
bool txOngoing = false;

// NEW callback format for your ESP32 core
void OnDataSent(const wifi_tx_info_t *info, esp_now_send_status_t status) {
  Serial.print("Delivery Status: ");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Sent OK" : "Delivery Failed");
  txOngoing = false;
}

void setup() {
  Serial.begin(115200);
  delay(2000);

  pinMode(PRG_BUTTON, INPUT_PULLUP);

  Serial.println("ESP-NOW FLOOD TRANSMITTER STARTING");

  WiFi.mode(WIFI_STA);
  WiFi.disconnect();

  esp_wifi_set_channel(1, WIFI_SECOND_CHAN_NONE);

  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  esp_now_register_send_cb(OnDataSent);

  esp_now_peer_info_t peerInfo;
  memset(&peerInfo, 0, sizeof(peerInfo));
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.channel = 1;
  peerInfo.encrypt = false;

  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("Failed to add broadcast peer");
    return;
  }

  Serial.print("Transmitter MAC Address: ");
  Serial.println(WiFi.macAddress());

  Serial.println("TRANSMITTER READY - press PRG to send FLOOD");
}

void loop() {
  if (digitalRead(PRG_BUTTON) == LOW && !txOngoing) {
    delay(50);

    if (digitalRead(PRG_BUTTON) == LOW) {
      txOngoing = true;

      strcpy(myData.text, "FLOOD");

      Serial.println("Sending FLOOD alert via ESP-NOW...");

      esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *)&myData, sizeof(myData));

      if (result != ESP_OK) {
        Serial.println("Error sending data");
        txOngoing = false;
      }

      while (digitalRead(PRG_BUTTON) == LOW) {
        delay(10);
      }
    }
  }
}