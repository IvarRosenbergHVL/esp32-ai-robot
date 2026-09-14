#include "Audio_Hardware.h"
#include "Robot_Config.h"

#include <math.h>

#if ROBOT_ENABLE_ONBOARD_AUDIO
#include "ESP_I2S.h"
#include "driver/i2c.h"
#include "es7210.h"
#include "es8311.h"

namespace {
constexpr gpio_num_t kMclkPin = GPIO_NUM_12;
constexpr gpio_num_t kBclkPin = GPIO_NUM_13;
constexpr gpio_num_t kLrclkPin = GPIO_NUM_14;
// Waveshare naming is from the codec perspective: GPIO16 carries audio from
// the ESP32 to ES8311, while GPIO15 carries ES7210 microphone data to ESP32.
constexpr gpio_num_t kDataOutPin = GPIO_NUM_16;
constexpr gpio_num_t kDataInPin = GPIO_NUM_15;
constexpr gpio_num_t kAmplifierEnablePin = GPIO_NUM_9;
constexpr uint32_t kMclkRatio = 256;

I2SClass audioI2s;
es7210_dev_handle_t microphoneCodec = nullptr;
es8311_handle_t speakerCodec = nullptr;
RobotAudioStatus status = {true, false, 0.0f, 0};
uint32_t lastLevelReportMs = 0;

bool initializeMicrophoneCodec() {
  const es7210_i2c_config_t i2cConfig = {
    .i2c_port = I2C_NUM_0,
    .i2c_addr = ES7210_ADDRRES_00
  };
  if (es7210_new_codec(&i2cConfig, &microphoneCodec) != ESP_OK) return false;

  es7210_codec_config_t codecConfig = {
    .sample_rate_hz = ROBOT_AUDIO_SAMPLE_RATE,
    .mclk_ratio = kMclkRatio,
    .i2s_format = ES7210_I2S_FMT_I2S,
    .bit_width = ES7210_I2S_BITS_16B,
    .mic_bias = ES7210_MIC_BIAS_2V87,
    .mic_gain = ES7210_MIC_GAIN_30DB
  };
  codecConfig.flags.tdm_enable = true;
  return es7210_config_codec(microphoneCodec, &codecConfig) == ESP_OK &&
         es7210_config_volume(microphoneCodec, 10) == ESP_OK;
}

bool initializeSpeakerCodec() {
  speakerCodec = es8311_create(I2C_NUM_0, ES8311_ADDRESS_0);
  if (!speakerCodec) return false;
  const es8311_clock_config_t clockConfig = {
    .mclk_inverted = false,
    .sclk_inverted = false,
    .mclk_from_mclk_pin = true,
    .mclk_frequency = ROBOT_AUDIO_SAMPLE_RATE * static_cast<int>(kMclkRatio),
    .sample_frequency = ROBOT_AUDIO_SAMPLE_RATE
  };
  return es8311_init(speakerCodec, &clockConfig, ES8311_RESOLUTION_16, ES8311_RESOLUTION_16) == ESP_OK &&
         es8311_voice_volume_set(speakerCodec, ROBOT_AUDIO_CODEC_VOLUME, nullptr) == ESP_OK &&
         es8311_microphone_config(speakerCodec, false) == ESP_OK;
}
}  // namespace

bool Audio_Hardware_Init() {
  pinMode(kAmplifierEnablePin, OUTPUT);
  digitalWrite(kAmplifierEnablePin, LOW);

  audioI2s.setPins(kBclkPin, kLrclkPin, kDataOutPin, kDataInPin, kMclkPin);
  audioI2s.setTimeout(100);
  if (!audioI2s.begin(I2S_MODE_STD, ROBOT_AUDIO_SAMPLE_RATE,
                      I2S_DATA_BIT_WIDTH_16BIT, I2S_SLOT_MODE_STEREO)) {
    Serial.println("[audio] I2S initialization failed");
    return false;
  }
  if (!initializeMicrophoneCodec()) {
    Serial.println("[audio] ES7210 initialization failed");
    return false;
  }
  if (!initializeSpeakerCodec()) {
    Serial.println("[audio] ES8311 initialization failed");
    return false;
  }

  status.initialized = true;
  Serial.printf("[audio] Ready: %d Hz, 16-bit stereo, ES7210 + ES8311\n", ROBOT_AUDIO_SAMPLE_RATE);
#if ROBOT_AUDIO_STARTUP_TONE
  Audio_Hardware_PlayTestTone();
#endif
  return true;
}

size_t Audio_Hardware_ReadPcm(int16_t *samples, size_t sampleCount, uint32_t timeoutMs) {
  if (!status.initialized || !samples || sampleCount == 0) return 0;
  audioI2s.setTimeout(timeoutMs);
  return audioI2s.readBytes(reinterpret_cast<char *>(samples), sampleCount * sizeof(int16_t));
}

size_t Audio_Hardware_WritePcm(const int16_t *samples, size_t sampleCount) {
  if (!status.initialized || !samples || sampleCount == 0) return 0;
  const uint8_t *data = reinterpret_cast<const uint8_t *>(samples);
  const size_t bytes = sampleCount * sizeof(int16_t);
  size_t written = 0;
  uint32_t lastProgressAt = millis();
  while (written < bytes) {
    const size_t count = audioI2s.write(data + written, bytes - written);
    if (!count) {
      if (millis() - lastProgressAt >= 1000) {
        Serial.printf("[audio] I2S write stalled after %u/%u bytes\n",
                      static_cast<unsigned>(written), static_cast<unsigned>(bytes));
        break;
      }
      vTaskDelay(pdMS_TO_TICKS(1));
      continue;
    }
    written += count;
    lastProgressAt = millis();
  }
  return written;
}

void Audio_Hardware_SetSpeakerEnabled(bool enabled) {
  if (!status.initialized) return;
  digitalWrite(kAmplifierEnablePin, enabled ? HIGH : LOW);
  delay(50);
}

void Audio_Hardware_PlayTestTone(uint16_t frequencyHz, uint16_t durationMs) {
  if (!status.initialized || frequencyHz == 0) return;
  constexpr size_t kFramesPerChunk = 160;
  int16_t stereo[kFramesPerChunk * 2];
  const uint32_t frameCount = static_cast<uint32_t>(ROBOT_AUDIO_SAMPLE_RATE) * durationMs / 1000;
  const float amplitude = 32767.0f * 0.12f;
  Audio_Hardware_SetSpeakerEnabled(true);
  for (uint32_t first = 0; first < frameCount; first += kFramesPerChunk) {
    const size_t frames = min(static_cast<uint32_t>(kFramesPerChunk), frameCount - first);
    for (size_t frame = 0; frame < frames; ++frame) {
      const float phase = 2.0f * PI * frequencyHz * (first + frame) / ROBOT_AUDIO_SAMPLE_RATE;
      const int16_t value = static_cast<int16_t>(sinf(phase) * amplitude);
      stereo[frame * 2] = value;
      stereo[frame * 2 + 1] = 0;
    }
    Audio_Hardware_WritePcm(stereo, frames * 2);
  }
  Audio_Hardware_SetSpeakerEnabled(false);
  Serial.println("[audio] Test tone complete");
}

bool Audio_Hardware_PlayWav(const uint8_t *wav, size_t bytes) {
  if (!status.initialized || !wav || bytes < 44 || memcmp(wav, "RIFF", 4) || memcmp(wav + 8, "WAVE", 4)) return false;
  uint16_t channels = 0, bits = 0;
  uint32_t sampleRate = 0;
  const uint8_t *pcm = nullptr;
  size_t pcmBytes = 0;
  size_t offset = 12;
  while (offset + 8 <= bytes) {
    const uint32_t chunkBytes = static_cast<uint32_t>(wav[offset + 4]) |
      (static_cast<uint32_t>(wav[offset + 5]) << 8) | (static_cast<uint32_t>(wav[offset + 6]) << 16) |
      (static_cast<uint32_t>(wav[offset + 7]) << 24);
    if (offset + 8 + chunkBytes > bytes) return false;
    if (!memcmp(wav + offset, "fmt ", 4) && chunkBytes >= 16) {
      channels = wav[offset + 10] | (wav[offset + 11] << 8);
      sampleRate = static_cast<uint32_t>(wav[offset + 12]) | (static_cast<uint32_t>(wav[offset + 13]) << 8) |
        (static_cast<uint32_t>(wav[offset + 14]) << 16) | (static_cast<uint32_t>(wav[offset + 15]) << 24);
      bits = wav[offset + 22] | (wav[offset + 23] << 8);
    } else if (!memcmp(wav + offset, "data", 4)) {
      pcm = wav + offset + 8;
      pcmBytes = chunkBytes;
    }
    offset += 8 + chunkBytes + (chunkBytes & 1);
  }
  if (!pcm || bits != 16 || sampleRate != ROBOT_AUDIO_SAMPLE_RATE || (channels != 1 && channels != 2)) return false;
  const int16_t *input = reinterpret_cast<const int16_t *>(pcm);
  const size_t inputSamples = pcmBytes / sizeof(int16_t);
  int16_t stereo[320];
  Audio_Hardware_SetSpeakerEnabled(true);
  if (channels == 2) {
    for (size_t first = 0; first < inputSamples; first += 320)
      Audio_Hardware_WritePcm(input + first, min(static_cast<size_t>(320), inputSamples - first));
  } else {
    for (size_t first = 0; first < inputSamples; first += 160) {
      const size_t count = min(static_cast<size_t>(160), inputSamples - first);
      for (size_t index = 0; index < count; ++index) {
        stereo[index * 2] = input[first + index];
        stereo[index * 2 + 1] = 0;
      }
      Audio_Hardware_WritePcm(stereo, count * 2);
    }
  }
  Audio_Hardware_SetSpeakerEnabled(false);
  return true;
}

void Audio_Hardware_Update() {
  if (!status.initialized) return;
  int16_t samples[320];
  const size_t bytes = Audio_Hardware_ReadPcm(samples, 320, 25);
  const size_t count = bytes / sizeof(int16_t);
  if (!count) return;

  double sumSquares = 0.0;
  uint16_t peak = 0;
  for (size_t index = 0; index < count; ++index) {
    const int32_t value = samples[index];
    const uint16_t absolute = static_cast<uint16_t>(value < 0 ? -value : value);
    peak = max(peak, absolute);
    sumSquares += static_cast<double>(value) * value;
  }
  status.inputRms = sqrt(sumSquares / count) / 32768.0f;
  status.inputPeak = peak;

  const uint32_t now = millis();
  if (now - lastLevelReportMs >= ROBOT_AUDIO_LEVEL_REPORT_MS) {
    lastLevelReportMs = now;
    Serial.printf("[audio] input rms=%.3f peak=%u\n", status.inputRms, status.inputPeak);
  }
}

RobotAudioStatus Audio_Hardware_Status() { return status; }
I2SClass *Audio_Hardware_I2S() { return status.initialized ? &audioI2s : nullptr; }

#else

namespace { RobotAudioStatus status = {false, false, 0.0f, 0}; }
bool Audio_Hardware_Init() {
  Serial.println("[audio] Disabled in Robot_Config.h");
  return false;
}
void Audio_Hardware_Update() {}
RobotAudioStatus Audio_Hardware_Status() { return status; }
size_t Audio_Hardware_ReadPcm(int16_t *, size_t, uint32_t) { return 0; }
size_t Audio_Hardware_WritePcm(const int16_t *, size_t) { return 0; }
void Audio_Hardware_SetSpeakerEnabled(bool) {}
void Audio_Hardware_PlayTestTone(uint16_t, uint16_t) {}
bool Audio_Hardware_PlayWav(const uint8_t *, size_t) { return false; }
I2SClass *Audio_Hardware_I2S() { return nullptr; }

#endif
