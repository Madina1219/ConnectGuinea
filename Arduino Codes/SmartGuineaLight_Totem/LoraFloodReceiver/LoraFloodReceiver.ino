#include <WiFi.h>
#include <esp_now.h>
#include <esp_wifi.h>

#define TOTEM_ALERT_PIN 35   // change later if needed

typedef struct struct_message {
  char text[32];
} struct_message;

struct_message incomingData;

void OnDataRecv(const esp_now_recv_info *info, const uint8_t *incomingDataPtr, int len) {
  memcpy(&incomingData, incomingDataPtr, sizeof(incomingData));

  Serial.print("RECEIVED: ");
  Serial.println(incomingData.text);

  if (strcmp(incomingData.text, "FLOOD") == 0) {
    Serial.println("FLOOD ALERT RECEIVED");

    for (int i = 0; i < 5; i++) {
      digitalWrite(TOTEM_ALERT_PIN, HIGH);
      delay(150);
      digitalWrite(TOTEM_ALERT_PIN, LOW);
      delay(150);
    }
  }
}

void setup() {
  Serial.begin(115200);
  delay(2000);

  pinMode(TOTEM_ALERT_PIN, OUTPUT);
  digitalWrite(TOTEM_ALERT_PIN, LOW);

  Serial.println("ESP-NOW TOTEM RECEIVER STARTING");

  WiFi.mode(WIFI_STA);
  WiFi.disconnect();

  // Force same Wi-Fi channel as transmitter
  esp_wifi_set_channel(1, WIFI_SECOND_CHAN_NONE);

  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  esp_now_register_recv_cb(OnDataRecv);

  Serial.print("Receiver MAC Address: ");
  Serial.println(WiFi.macAddress());

  Serial.println("RECEIVER READY - listening on ESP-NOW channel 1...");
}

void loop() {
}