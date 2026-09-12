# Board capability plan

The board platform is introduced in independently testable increments.

| Capability | Hardware | Status in MVP branch |
|---|---|---|
| Dual displays | 2 x GC9A01, 240 x 240 | Implemented |
| Idle eye motion | LVGL | Implemented |
| BOOT button | GPIO 0 | Implemented; triggers `notice` |
| Touch | Not fitted on selected board | Not initialized; no LVGL pointer devices |
| Flash/PSRAM diagnostics | ESP32-S3R8 | Implemented |
| Wi-Fi radio test | ESP32-S3 | Passive network-count scan implemented |
| microSD | 1-bit SD_MMC | Initialization and capacity report implemented |
| Onboard microphone | ES7210 + I2S | Implemented behind compile-time flag; physical test pending |
| Speaker output | ES8311 + amplifier enable | Implemented behind compile-time flag; physical test pending |
| Local keyword model | ESP-SR | Planned after microphone validation |
| Battery operation | 3.7 V connector/charger | Physical power test pending |
| BLE | ESP32-S3 | Deferred until a robot use case is defined |

The first flash should validate displays, BOOT button,
memory, Wi-Fi scanning, and optional microSD before audio is enabled. Audio is
kept as the next isolated increment because incorrect codec or I2S setup can
reset the board and obscure otherwise working display functionality.
# Onboard audio

The firmware now contains an isolated audio hardware layer based on Waveshare's
reference configuration. It initializes the ES7210 microphone ADC and ES8311
speaker DAC on a shared 16 kHz, 16-bit stereo I2S bus, exposes PCM read/write
helpers, reports microphone RMS/peak levels, controls the amplifier, and can
generate a low-volume startup test tone. It remains disabled by default until
physical-board validation.
