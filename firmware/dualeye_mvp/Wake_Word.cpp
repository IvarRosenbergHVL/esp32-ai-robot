#include "Wake_Word.h"
#include "Audio_Hardware.h"
#include "Robot_Config.h"
#include <Arduino.h>
#include <math.h>

#if ROBOT_ENABLE_WAKE_WORD
#include "ESP_SR.h"
#include "esp_partition.h"

namespace {
volatile bool wakeDetected = false;
bool wakeInitialized = false;

const char *detectInputFormat() {
  double leftSquares = 0.0;
  double rightSquares = 0.0;
  size_t frames = 0;
  uint16_t leftPeak = 0;
  uint16_t rightPeak = 0;
  int16_t samples[320];
  for (uint8_t chunk = 0; chunk < 100; ++chunk) {
    const size_t count = Audio_Hardware_ReadPcm(samples, 320, 30) / sizeof(int16_t);
    for (size_t index = 0; index + 1 < count; index += 2) {
      const int32_t left = samples[index];
      const int32_t right = samples[index + 1];
      const uint16_t leftAbsolute = static_cast<uint16_t>(left < 0 ? -left : left);
      const uint16_t rightAbsolute = static_cast<uint16_t>(right < 0 ? -right : right);
      leftPeak = max(leftPeak, leftAbsolute);
      rightPeak = max(rightPeak, rightAbsolute);
      leftSquares += static_cast<double>(left) * left;
      rightSquares += static_cast<double>(right) * right;
      ++frames;
    }
  }
  const float leftRms = frames ? sqrt(leftSquares / frames) / 32768.0f : 0.0f;
  const float rightRms = frames ? sqrt(rightSquares / frames) / 32768.0f : 0.0f;
  const char *format = "MM";
  if (leftRms > rightRms * 8.0f) format = "MN";
  else if (rightRms > leftRms * 8.0f) format = "NM";
  Serial.printf("[wake] channel test: left rms=%.4f peak=%u, right rms=%.4f peak=%u, format=%s\n",
                leftRms, leftPeak, rightRms, rightPeak, format);
  return format;
}

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
  const char *inputFormat = detectInputFormat();
  wakeInitialized = ESP_SR.begin(*i2s, nullptr, 0, SR_CHANNELS_STEREO, SR_MODE_WAKEWORD, inputFormat);
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
