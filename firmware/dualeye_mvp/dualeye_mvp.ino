#include "LCD_Driver.h"
#include "LVGL_Driver.h"
#include "LVGL_Example.h"
#include "Button_Driver.h"
#include "Board_Diagnostics.h"
#include "SD_Card.h"
#include "I2C_Driver.h"

void setup()
{
  Serial.begin(115200);
  delay(250);
  Serial.println("[robot] Starting DualEye MVP");
  Board_Diagnostics_Run();
  SD_Init();
  I2C_Init();
  LCD_INIT();
  Lvgl_Init();
  Button_Init();
  Lvgl_Example1();

  vTaskDelay(pdMS_TO_TICKS(100));
  LVGL_Start();
  Serial.println("[robot] Both displays initialized; eye animation running");
}

void loop() {
  if (BOOT_KEY_State == Click) {
    BOOT_KEY_State = None;
    Eye_Notice();
    Serial.println("[robot] BOOT click: notice animation");
  }
  vTaskDelay(pdMS_TO_TICKS(5));
}
