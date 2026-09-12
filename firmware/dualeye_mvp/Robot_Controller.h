#pragma once

#include <Arduino.h>

enum class RobotState : uint8_t {
  Booting, Idle, WakeListening, Recording, Thinking, Speaking, Error
};

void Robot_Controller_Init();
void Robot_Controller_Update();
RobotState Robot_Controller_State();
void Robot_Controller_TriggerWakeWord();

