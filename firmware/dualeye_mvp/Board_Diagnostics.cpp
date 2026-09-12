#include "Board_Diagnostics.h"

#include <Arduino.h>
#include <WiFi.h>

void Board_Diagnostics_Run() {
  Serial.println("[self-test] ESP32-S3 board diagnostics");
  Serial.printf("[self-test] Chip: %s rev %d, cores: %d\n",
                ESP.getChipModel(), ESP.getChipRevision(), ESP.getChipCores());
  Serial.printf("[self-test] Flash: %u MB\n", ESP.getFlashChipSize() / 1024 / 1024);
  Serial.printf("[self-test] PSRAM: %u MB, free: %u KB\n",
                ESP.getPsramSize() / 1024 / 1024, ESP.getFreePsram() / 1024);
  Serial.printf("[self-test] Heap free: %u KB\n", ESP.getFreeHeap() / 1024);

  WiFi.mode(WIFI_STA);
  WiFi.disconnect(false, false);
  delay(100);
  const int networks = WiFi.scanNetworks(false, true);
  Serial.printf("[self-test] Wi-Fi scan: %d network(s) found\n", networks);
  WiFi.scanDelete();
  WiFi.mode(WIFI_OFF);
}
