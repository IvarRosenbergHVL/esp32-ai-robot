#include "Wake_Word.h"
#include "Robot_Config.h"
#include <Arduino.h>

bool Wake_Word_Init() {
#if ROBOT_ENABLE_WAKE_WORD
  Serial.println("[wake] Add the selected ESP-SR/WakeNet model adapter before enabling this flag");
#else
  Serial.println("[wake] Disabled; BOOT button simulates keyword detection");
#endif
  return false;
}

bool Wake_Word_Detected() {
  // The adapter will consume microphone frames only in WakeListening, avoiding
  // simultaneous readers on the shared I2S input.
  return false;
}
