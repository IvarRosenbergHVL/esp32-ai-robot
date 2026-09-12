# ESP32 AI robot

The MVP feature branch contains the interaction skeleton: proximity wake-up,
WakeNet adapter boundary, silence-terminated PSRAM recording, Azure conversation
backend, PCM-WAV playback, controlled eye cues, local SD greeting, and a central
robot state machine. Hardware features stay off by default until the non-touch
board is validated subsystem by subsystem.

Firmware for an AI voice robot based on the
[Waveshare ESP32-S3-DualEye-LCD-1.28](https://www.waveshare.com/wiki/ESP32-S3-DualEye-LCD-1.28),
the version without touch controllers.

The first MVP brings up both 240 x 240 GC9A01 displays and renders synchronized
animated eyes. The repository also contains a small backend for the Azure
STT/LLM/TTS loop. Later firmware increments add onboard audio, local wake-word
detection, and VL53L1X proximity sensing.

## Current milestone: DualEye bring-up

`firmware/dualeye_mvp` is an Arduino sketch based on Waveshare's official
Arduino 3.2.0 display example. It provides:

- initialization of both round LCD panels;
- synchronized gaze movement;
- periodic blinking;
- serial startup diagnostics at 115200 baud.

See [docs/getting-started.md](docs/getting-started.md) for build and upload
instructions, [docs/hardware.md](docs/hardware.md) for the hardware inventory,
and [backend/README.md](backend/README.md) for the conversation service.

## Planned interaction flow

1. VL53L1X detects a person inside a configurable distance (initially 100 cm).
2. The robot enters an attentive state and enables local keyword spotting.
3. A wake word starts utterance recording.
4. Audio is sent over Wi-Fi to the robot API.
5. The MVP API uses Azure Speech for STT/TTS and Azure OpenAI for the response.
   Provider interfaces allow later use of NB-Whisper and Chatterbox.
6. The robot plays returned audio while executing eye/animation cues.

The display MVP runs without a backend. No continuous microphone or camera
stream should be sent to the API.

## Upstream reference

Low-level display initialization and the GC9A01 driver originate from
Waveshare's official
[`ESP32-S3-DualEye-LCD-1.28`](https://github.com/waveshareteam/ESP32-S3-DualEye-Touch-LCD-1.28/tree/main/example/ESP32-S3-DualEye-LCD-1.28)
examples.
