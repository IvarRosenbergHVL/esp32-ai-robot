#pragma once

// Keep hardware subsystems independently switchable during first-board bring-up.
// Audio requires Waveshare's bundled es7210 and es8311 Arduino libraries.
#define ROBOT_ENABLE_ONBOARD_AUDIO 0

// Plays a quiet 440 Hz tone for 300 ms after successful audio initialization.
// Enable only after a speaker is connected and begin with low volume.
#define ROBOT_AUDIO_STARTUP_TONE 0

#define ROBOT_AUDIO_SAMPLE_RATE 16000
#define ROBOT_AUDIO_CODEC_VOLUME 35
#define ROBOT_AUDIO_LEVEL_REPORT_MS 500

