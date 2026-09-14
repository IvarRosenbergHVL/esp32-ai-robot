#include "Wake_Word.h"
#include "Audio_Hardware.h"
#include "Robot_Config.h"
#include <Arduino.h>

#if ROBOT_ENABLE_WAKE_WORD
#include "ESP_SR.h"
#include "esp_partition.h"

namespace {
volatile bool wakeDetected = false;
bool wakeInitialized = false;

bool modelPartitionReady() {
  const esp_partition_t *partition = esp_partition_find_first(
    ESP_PARTITION_TYPE_DATA, ESP_PARTITION_SUBTYPE_DATA_SPIFFS, "model");
  if (!partition) {
    Serial.println("[wake] Missing model partition; select the ESP SR 16M partition scheme");
    return false;
  }
  uint8_t header[16]{};
  if (esp_partition_read(partition, 0, header, sizeof(header)) != ESP_OK) {
    Serial.println("[wake] Cannot read model partition");
    return false;
  }
  bool erased = true;
  for (uint8_t value : header) erased = erased && value == 0xff;
  if (erased) {
    Serial.println("[wake] Model partition is empty; upload firmware again to flash srmodels.bin");
    return false;
  }
  return true;
}

void onWakeEvent(sr_event_t event, int commandId, int phraseId) {
  (void)commandId;
  (void)phraseId;
  if (event == SR_EVENT_WAKEWORD) {
    wakeDetected = true;
    Serial.println("[wake] Hi ESP detected");
  }
}
}  // namespace
#endif

bool Wake_Word_Init() {
#if ROBOT_ENABLE_WAKE_WORD
  if (!modelPartitionReady()) return false;
  I2SClass *i2s = Audio_Hardware_I2S();
  if (!i2s) {
    Serial.println("[wake] Cannot start: onboard audio is not initialized");
    return false;
  }
  ESP_SR.onEvent(onWakeEvent);
  wakeInitialized = ESP_SR.begin(*i2s, nullptr, 0, SR_CHANNELS_STEREO, SR_MODE_WAKEWORD, "MM");
  Serial.println(wakeInitialized ? "[wake] WakeNet ready; say Hi ESP" : "[wake] WakeNet initialization failed");
  return wakeInitialized;
#else
  Serial.println("[wake] Disabled; BOOT button simulates keyword detection");
  return false;
#endif
}

bool Wake_Word_Detected() {
#if ROBOT_ENABLE_WAKE_WORD
  if (wakeDetected) {
    wakeDetected = false;
    return true;
  }
#endif
  return false;
}

bool Wake_Word_Pause() {
#if ROBOT_ENABLE_WAKE_WORD
  return !wakeInitialized || ESP_SR.pause();
#else
  return true;
#endif
}

bool Wake_Word_Resume() {
#if ROBOT_ENABLE_WAKE_WORD
  return !wakeInitialized || ESP_SR.resume();
#else
  return true;
#endif
}
