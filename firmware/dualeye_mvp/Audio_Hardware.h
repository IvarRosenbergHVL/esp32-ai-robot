#pragma once

#include <Arduino.h>

class I2SClass;

struct RobotAudioStatus {
  bool enabled;
  bool initialized;
  float inputRms;
  uint16_t inputPeak;
};

// Initializes the shared full-duplex I2S bus, ES7210 microphone ADC and ES8311
// speaker DAC. When ROBOT_ENABLE_ONBOARD_AUDIO is 0 these functions are safe
// no-ops and do not require the codec libraries to compile.
bool Audio_Hardware_Init();
void Audio_Hardware_Update();
RobotAudioStatus Audio_Hardware_Status();

// PCM is signed 16-bit, stereo, 16 kHz. The returned value is bytes read/written.
size_t Audio_Hardware_ReadPcm(int16_t *samples, size_t sampleCount, uint32_t timeoutMs);
size_t Audio_Hardware_WritePcm(const int16_t *samples, size_t sampleCount);

void Audio_Hardware_SetSpeakerEnabled(bool enabled);
void Audio_Hardware_PlayTestTone(uint16_t frequencyHz = 440, uint16_t durationMs = 300);
bool Audio_Hardware_PlayWav(const uint8_t *wav, size_t bytes);
I2SClass *Audio_Hardware_I2S();
