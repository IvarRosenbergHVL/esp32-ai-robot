# Getting started

## Requirements

- Waveshare ESP32-S3-DualEye-LCD-1.28 (non-touch version)
- USB-C data cable
- Arduino IDE
- Espressif Arduino core **3.2.0**
- LVGL **8.3.10**
- Waveshare's bundled `OneButton` library
- Waveshare's bundled `es7210` and `es8311` libraries (needed when onboard audio is enabled)

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

The serial self-test also reports chip information, flash, PSRAM, nearby Wi-Fi
network count, and microSD status. A short BOOT-button click centers the eyes in
a temporary `notice` animation. The selected board has no touch controllers;
the firmware therefore registers no LVGL pointer devices.

## First-board acceptance test

- Both displays turn on without flicker or resets.
- Left and right display orientation is correct.
- Gaze direction matches on both physical eyes.
- Blink closes and opens both eyes simultaneously.
- Animation runs for at least ten minutes without a watchdog reset.
- Serial output contains no initialization error or attempts to initialize CST816S.

Record mirrored or rotated behavior before changing pinout or rotation flags.

## Onboard audio bring-up

Audio is disabled by default so display problems remain isolated. After the
first display test succeeds:

1. Install/copy Waveshare's bundled `es7210` and `es8311` libraries into the
   Arduino libraries directory.
2. Set `ROBOT_ENABLE_ONBOARD_AUDIO` to `1` in `Robot_Config.h`.
3. Compile and inspect Serial Monitor. A successful initialization reports
   `ES7210 + ES8311` followed by RMS/peak input levels every 500 ms.
4. Speak near the onboard microphones and verify that RMS/peak values change.
5. Connect the intended speaker, start at low volume, set
   `ROBOT_AUDIO_STARTUP_TONE` to `1`, and reboot once. A quiet 440 Hz tone should
   play for 300 ms.
6. Set the startup-tone flag back to `0` after validation.

The amplifier is disabled after the test tone to reduce idle noise. PCM helpers
use signed 16-bit stereo at 16 kHz and form the boundary for recording, ESP-SR,
and later backend playback.
## Robot interaction foundation

The firmware has a central controller with this primary flow:

`Idle → WakeListening → Recording → Thinking → Speaking → Idle`

During development, the BOOT button simulates a detected wake word. Hardware
features are independently enabled in `Robot_Config.h`. Copy
`Robot_Secrets.example.h` to the ignored `Robot_Secrets.h` before enabling
networking. Networking requires ArduinoJson; proximity requires the Pololu
VL53L1X Arduino library.

The 16 kHz, 16-bit mono PCM `marseillaise.wav` is bundled into the firmware, so
no microSD card is needed. It plays once at boot when `ROBOT_AUDIO_BOOT_WAV` is
enabled. The proximity cue reuses it once per approach, observes a 75-second
cooldown, and rearms only after the person leaves the reset distance. The source
asset remains in `assets/audio/marseillaise.wav`.

Recording uses PSRAM, converts stereo microphone input to mono WAV, and stops
after sustained silence or ten seconds. Azure TTS returns the same PCM format
for direct I2S playback.
