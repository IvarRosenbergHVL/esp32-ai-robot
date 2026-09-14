# Hardware inventory

## Confirmed platform and planned components

| Component | Model | Initial role |
|---|---|---|
| Main controller/display | Waveshare ESP32-S3-DualEye-LCD-1.28 (non-touch) | Main controller and eyes |
| Proximity sensor | VL53L1X ToF, up to 400 cm | Presence gate; initial threshold 100 cm |
| Microphone module | GY-MAX4466, analog ADC | External microphone experiments |
| Speaker | 40 mm, 8 ohm, 2-3 W | Speech output |
| Development controller | ESP32-S3 development boards | Peripheral experiments |
| Camera | Small ESP32-compatible camera, exact model TBD | Optional local presence/direction input |

The Waveshare board's onboard microphone and audio path will be evaluated before
an external microphone or amplifier becomes part of the main build.

## Display pinout

These values follow the official Waveshare Arduino example.

| Signal | GPIO |
|---|---:|
| SPI MISO | 40 |
| SPI MOSI | 42 |
| SPI SCLK | 41 |
| LCD 1 CS | 47 |
| LCD 2 CS | 38 |
| Shared LCD DC | 45 |
| LCD 1 reset | 48 |
| LCD 2 reset | 8 |
| LCD 1 backlight | 46 |
| LCD 2 backlight | 39 |
| I2C bus 1 SDA / SCL | 11 / 10 |
| I2C bus 2 SDA / SCL | 3 / 2 |
| Audio MCLK / BCLK / LRCLK | 12 / 13 / 14 |
| Audio data in / out | 15 / 16 |
| Audio amplifier enable | 9 |
| microSD CLK / CMD / D0 | 17 / 21 / 18 |
| BOOT button | 0 |

Do not assign external peripherals to these GPIOs. Remaining pins will be
allocated after checking the complete schematic and connector breakout.

## Activation pipeline

VL53L1X gates the attentive state but does not replace wake-word recognition:

`idle -> presence detected -> keyword spotting -> recording -> API -> playback`

Keyword spotting stays local. The API will eventually expose NB-Whisper STT and
Chatterbox TTS endpoints, but it is not required for the display MVP.
