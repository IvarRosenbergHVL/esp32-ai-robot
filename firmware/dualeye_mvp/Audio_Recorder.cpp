#include "Audio_Recorder.h"
#include "Audio_Hardware.h"
#include "Robot_Config.h"

#include <math.h>

namespace {
constexpr size_t kHeaderBytes = 44;
uint8_t *wavBuffer = nullptr;
size_t pcmBytes = 0;
size_t capacity = 0;
uint32_t startedAt = 0;
uint32_t silenceStartedAt = 0;
bool recording = false;

void writeLe16(uint8_t *target, uint16_t value) {
  target[0] = value & 0xff;
  target[1] = value >> 8;
}

void writeLe32(uint8_t *target, uint32_t value) {
  for (uint8_t index = 0; index < 4; ++index) target[index] = value >> (index * 8);
}

void finalizeHeader() {
  memcpy(wavBuffer, "RIFF", 4);
  writeLe32(wavBuffer + 4, 36 + pcmBytes);
  memcpy(wavBuffer + 8, "WAVEfmt ", 8);
  writeLe32(wavBuffer + 16, 16);
  writeLe16(wavBuffer + 20, 1);
  writeLe16(wavBuffer + 22, 1);
  writeLe32(wavBuffer + 24, ROBOT_AUDIO_SAMPLE_RATE);
  writeLe32(wavBuffer + 28, ROBOT_AUDIO_SAMPLE_RATE * 2);
  writeLe16(wavBuffer + 32, 2);
  writeLe16(wavBuffer + 34, 16);
  memcpy(wavBuffer + 36, "data", 4);
  writeLe32(wavBuffer + 40, pcmBytes);
}
}  // namespace

bool Audio_Recorder_Start() {
  Audio_Recorder_Discard();
  if (!Audio_Hardware_Status().initialized) return false;
  capacity = kHeaderBytes + static_cast<size_t>(ROBOT_AUDIO_SAMPLE_RATE) * 2 * ROBOT_RECORD_MAX_MS / 1000;
  wavBuffer = static_cast<uint8_t *>(ps_malloc(capacity));
  if (!wavBuffer) {
    Serial.printf("[record] PSRAM allocation failed: %u bytes\n", static_cast<unsigned>(capacity));
    return false;
  }
  pcmBytes = 0;
  startedAt = millis();
  silenceStartedAt = 0;
  recording = true;
  Serial.println("[record] Started");
  return true;
}

bool Audio_Recorder_Update() {
  if (!recording) return false;
  int16_t stereo[320];
  const size_t samples = Audio_Hardware_ReadPcm(stereo, 320, 30) / sizeof(int16_t);
  if (samples < 2) {
    if (millis() - startedAt >= ROBOT_RECORD_MAX_MS) {
      recording = false;
      finalizeHeader();
    }
    return recording;
  }

  double sum = 0;
  int16_t *mono = reinterpret_cast<int16_t *>(wavBuffer + kHeaderBytes + pcmBytes);
  const size_t framesAvailable = (capacity - kHeaderBytes - pcmBytes) / sizeof(int16_t);
  const size_t frames = min(samples / 2, framesAvailable);
  for (size_t frame = 0; frame < frames; ++frame) {
    const int16_t value = stereo[frame * 2];
    mono[frame] = value;
    sum += static_cast<double>(value) * value;
  }
  pcmBytes += frames * sizeof(int16_t);
  const float rms = frames ? sqrt(sum / frames) / 32768.0f : 0;
  const uint32_t now = millis();
  const uint32_t elapsed = now - startedAt;
  if (elapsed >= ROBOT_RECORD_MIN_MS && rms < ROBOT_RECORD_SILENCE_RMS) {
    if (!silenceStartedAt) silenceStartedAt = now;
  } else {
    silenceStartedAt = 0;
  }
  if (elapsed >= ROBOT_RECORD_MAX_MS || frames == framesAvailable ||
      (silenceStartedAt && now - silenceStartedAt >= ROBOT_RECORD_SILENCE_MS)) {
    recording = false;
    finalizeHeader();
    Serial.printf("[record] Complete: %u ms, %u bytes\n", elapsed, static_cast<unsigned>(pcmBytes + kHeaderBytes));
  }
  return recording;
}

bool Audio_Recorder_IsRecording() { return recording; }

RobotRecording Audio_Recorder_Take() {
  if (recording || !wavBuffer) return {nullptr, 0, 0};
  RobotRecording result = {wavBuffer, pcmBytes + kHeaderBytes,
                           static_cast<uint32_t>(pcmBytes * 1000 / (ROBOT_AUDIO_SAMPLE_RATE * 2))};
  wavBuffer = nullptr;
  pcmBytes = capacity = 0;
  return result;
}

void Audio_Recorder_Discard() {
  if (wavBuffer) free(wavBuffer);
  wavBuffer = nullptr;
  pcmBytes = capacity = 0;
  recording = false;
}
