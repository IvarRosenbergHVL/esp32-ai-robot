#pragma once

// Keep hardware subsystems independently switchable during first-board bring-up.
// Audio requires Waveshare's bundled es7210 and es8311 Arduino libraries.
#define ROBOT_ENABLE_ONBOARD_AUDIO 1
#define ROBOT_ENABLE_NETWORK 0
#define ROBOT_ENABLE_PROXIMITY 0
#define ROBOT_ENABLE_WAKE_WORD 1

// Plays a quiet 440 Hz tone for 300 ms after successful audio initialization.
// Enable only after a speaker is connected and begin with low volume.
#define ROBOT_AUDIO_STARTUP_TONE 0

// Plays the bundled 16 kHz, signed 16-bit mono PCM WAV once during startup.
#define ROBOT_AUDIO_BOOT_WAV 1

#define ROBOT_AUDIO_SAMPLE_RATE 16000
#define ROBOT_AUDIO_CODEC_VOLUME 75
#define ROBOT_AUDIO_LEVEL_REPORT_MS 500

#define ROBOT_RECORD_MAX_MS 10000
#define ROBOT_RECORD_MIN_MS 900
#define ROBOT_RECORD_SILENCE_MS 1100
#define ROBOT_RECORD_SILENCE_RMS 0.018f

#define ROBOT_PROXIMITY_WAKE_MM 1500
#define ROBOT_PROXIMITY_CUE_MM 600
#define ROBOT_PROXIMITY_TOO_CLOSE_MM 250
#define ROBOT_PROXIMITY_RESET_MM 1800
#define ROBOT_PROXIMITY_CUE_COOLDOWN_MS 75000

#define ROBOT_BACKEND_TIMEOUT_MS 35000
#define ROBOT_SESSION_ID "mime-01"
