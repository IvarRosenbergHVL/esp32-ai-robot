# ESP32 AI robot

Firmware for an AI voice robot based on the
[Waveshare ESP32-S3-DualEye-Touch-LCD-1.28](https://docs.waveshare.com/ESP32-S3-DualEye-Touch-LCD-1.28).

The first MVP brings up both 240 x 240 GC9A01 displays and renders synchronized
animated eyes. Later increments add onboard audio, local wake-word detection,
VL53L1X proximity sensing, and the backend STT/LLM/TTS loop.

## Current milestone: DualEye bring-up

`firmware/dualeye_mvp` is an Arduino sketch based on Waveshare's official
Arduino 3.2.0 display example. It provides:

- initialization of both round LCD panels;
- synchronized gaze movement;
- periodic blinking;
- serial startup diagnostics at 115200 baud.

See [docs/getting-started.md](docs/getting-started.md) for build and upload
instructions and [docs/hardware.md](docs/hardware.md) for the hardware inventory.

## Planned interaction flow

1. VL53L1X detects a person inside a configurable distance (initially 100 cm).
2. The robot enters an attentive state and enables local keyword spotting.
3. A wake word starts utterance recording.
4. Audio is sent over Wi-Fi to the future robot API.
5. The API uses NB-Whisper for STT, an LLM for the response, and Chatterbox for TTS.
6. The robot plays returned audio while executing eye/animation cues.

The display MVP runs without a backend. No continuous microphone or camera
stream should be sent to the API.

## Upstream reference

Low-level display initialization and the GC9A01 driver originate from
Waveshare's official
[`ESP32-S3-DualEye-Touch-LCD-1.28`](https://github.com/waveshareteam/ESP32-S3-DualEye-Touch-LCD-1.28)
examples.
