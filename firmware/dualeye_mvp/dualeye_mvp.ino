#include "LCD_Driver.h"
#include "LVGL_Driver.h"
#include "LVGL_Example.h"

void setup()
{
  Serial.begin(115200);
  delay(250);
  Serial.println("[robot] Starting DualEye MVP");
  LCD_INIT();
  Lvgl_Init();
  Lvgl_Example1();

  vTaskDelay(pdMS_TO_TICKS(100));
  LVGL_Start();
  Serial.println("[robot] Both displays initialized; eye animation running");
}

void loop() {
  vTaskDelay(pdMS_TO_TICKS(5));
}
