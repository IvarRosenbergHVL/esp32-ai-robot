# Board capability plan

The board platform is introduced in independently testable increments.

| Capability | Hardware | Status in MVP branch |
|---|---|---|
| Dual displays | 2 x GC9A01, 240 x 240 | Implemented |
| Idle eye motion | LVGL | Implemented |
| BOOT button | GPIO 0 | Implemented; triggers `notice` |
| Dual touch | 2 x CST816S | Driver and LVGL input registered |
| Flash/PSRAM diagnostics | ESP32-S3R8 | Implemented |
| Wi-Fi radio test | ESP32-S3 | Passive network-count scan implemented |
| microSD | 1-bit SD_MMC | Initialization and capacity report implemented |
| Onboard microphone | ES7210 + I2S | Next hardware increment |
| Speaker output | ES8311 + amplifier enable | Next hardware increment |
| Local keyword model | ESP-SR | Planned after microphone validation |
| Battery operation | 3.7 V connector/charger | Physical power test pending |
| BLE | ESP32-S3 | Deferred until a robot use case is defined |

The first flash should validate displays, touch initialization, BOOT button,
memory, Wi-Fi scanning, and optional microSD before audio is enabled. Audio is
kept as the next isolated increment because incorrect codec or I2S setup can
reset the board and obscure otherwise working display functionality.
