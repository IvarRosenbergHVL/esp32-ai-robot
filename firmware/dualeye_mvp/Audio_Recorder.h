#pragma once

#include <Arduino.h>

struct RobotRecording {
  uint8_t *wav;
  size_t bytes;
  uint32_t durationMs;
};

bool Audio_Recorder_Start();
bool Audio_Recorder_Update();
bool Audio_Recorder_IsRecording();
RobotRecording Audio_Recorder_Take();
void Audio_Recorder_Discard();

