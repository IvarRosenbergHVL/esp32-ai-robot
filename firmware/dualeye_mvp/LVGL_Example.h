#pragma once

#include "LVGL_Driver.h"
#include "LCD_Driver.h"

void Lvgl_Example1(void);
void LVGL_Backlight_adjustment(uint8_t Backlight);
void Eye_Notice(void);
void Eye_ShowMarquee(const char *text, uint32_t durationMs = 6500);

enum class EyeEmotion : uint8_t {
  Neutral, Attentive, Happy, Curious, Thinking, Surprised, Skeptical, Sleepy, Error
};

void Eye_SetEmotion(EyeEmotion emotion);
bool Eye_SetEmotion(const char *emotion);
bool Eye_PerformAction(const char *action);
