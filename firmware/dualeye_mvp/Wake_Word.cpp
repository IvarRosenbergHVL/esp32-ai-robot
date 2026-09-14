#include "Wake_Word.h"
#include "Audio_Hardware.h"
#include "Robot_Config.h"
#include <Arduino.h>

#if ROBOT_ENABLE_WAKE_WORD
#include "ESP_SR.h"

namespace {
volatile bool wakeDetected = false;
bool wakeInitialized = false;

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
