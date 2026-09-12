# Getting started

## Requirements

- Waveshare ESP32-S3-DualEye-Touch-LCD-1.28
- USB-C data cable
- Arduino IDE
- Espressif Arduino core **3.2.0**
- LVGL **8.3.10**

These versions match Waveshare's official example and should remain pinned until
the first physical-board test is complete.

## Initial Arduino IDE settings

- Board: `ESP32S3 Dev Module`
- USB CDC On Boot: `Enabled`
- CPU Frequency: `240MHz (WiFi)`
- Flash Mode: `QIO 80MHz`
- Flash Size: `16MB (128Mb)`
- Partition Scheme: `16M Flash (3MB APP/9.9MB FATFS)`
- PSRAM: `OPI PSRAM`
- Upload Mode: `UART0 / Hardware CDC`

## Build and upload

1. Install LVGL 8.3.10 in Arduino IDE.
2. Open `firmware/dualeye_mvp/dualeye_mvp.ino`.
3. Select the settings above and the board's serial port.
4. Compile and upload.
5. Open Serial Monitor at 115200 baud.

Expected output:

```text
[robot] Starting DualEye MVP
[robot] Both displays initialized; eye animation running
```

Both displays should show cream-colored eyeballs with brown irises. They should
look in the same direction and blink together.

## First-board acceptance test

- Both displays turn on without flicker or resets.
- Left and right display orientation is correct.
- Gaze direction matches on both physical eyes.
- Blink closes and opens both eyes simultaneously.
- Animation runs for at least ten minutes without a watchdog reset.
- Serial output contains no initialization error.

Record mirrored or rotated behavior before changing pinout or rotation flags.
