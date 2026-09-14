#pragma once

#include <Arduino.h>

constexpr uint8_t ROBOT_MAX_EYE_ACTIONS = 12;

struct TimedEyeAction { char type[20]; uint32_t atMs; };
struct RobotBackendResponse {
  bool ok;
  char emotion[16];
  uint8_t *audio;
  size_t audioBytes;
  TimedEyeAction actions[ROBOT_MAX_EYE_ACTIONS];
  uint8_t actionCount;
};

bool Backend_ConnectWifi();
bool Backend_SendConversation(const uint8_t *wav, size_t wavBytes, RobotBackendResponse &result);
void Backend_FreeResponse(RobotBackendResponse &result);

