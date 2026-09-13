#include "Robot_Controller.h"
#include "Audio_Hardware.h"
#include "Audio_Recorder.h"
#include "Backend_Client.h"
#include "LVGL_Example.h"
#include "Proximity_Sensor.h"
#include "Robot_Config.h"
#include "SD_MMC.h"
#include "Wake_Word.h"

namespace {
RobotState state = RobotState::Booting;
bool personPresent = false;
bool wakeRequested = false;
bool playedCueThisApproach = false;
uint32_t lastCueAt = 0;
uint32_t stateEnteredAt = 0;

bool playLocalProximityCue() {
  File file = SD_MMC.open("/marseillaise.wav", FILE_READ);
  if (!file || file.isDirectory() || file.size() < 44) {
    if (file) file.close();
    return false;
  }
  const size_t bytes = file.size();
  uint8_t *wav = static_cast<uint8_t *>(ps_malloc(bytes));
  if (!wav) { file.close(); return false; }
  const bool read = file.read(wav, bytes) == bytes;
  file.close();
  const bool played = read && Audio_Hardware_PlayWav(wav, bytes);
  free(wav);
  return played;
}

void enter(RobotState next) {
  if (state == next) return;
  state = next;
  stateEnteredAt = millis();
  Serial.printf("[robot] state=%u\n", static_cast<unsigned>(state));
  switch (state) {
    case RobotState::Idle: Eye_SetEmotion(EyeEmotion::Neutral); break;
    case RobotState::WakeListening: Eye_SetEmotion(EyeEmotion::Attentive); break;
    case RobotState::Recording: Eye_SetEmotion(EyeEmotion::Curious); break;
    case RobotState::Thinking: Eye_SetEmotion(EyeEmotion::Thinking); break;
    case RobotState::Speaking: Eye_SetEmotion(EyeEmotion::Happy); break;
    case RobotState::Error: Eye_SetEmotion(EyeEmotion::Error); break;
    default: break;
  }
}

void playResponse(RobotBackendResponse &response) {
  enter(RobotState::Speaking);
  Eye_SetEmotion(response.emotion);
  const uint32_t playbackStarted = millis();
  uint8_t nextAction = 0;
  // PCM playback currently blocks; apply zero-time cues now. Timed scheduling is
  // retained in the API and will move to the audio task after hardware bring-up.
  while (nextAction < response.actionCount && response.actions[nextAction].atMs == 0)
    Eye_PerformAction(response.actions[nextAction++].type);
  Audio_Hardware_PlayWav(response.audio, response.audioBytes);
  while (nextAction < response.actionCount) {
    if (response.actions[nextAction].atMs <= millis() - playbackStarted)
      Eye_PerformAction(response.actions[nextAction].type);
    ++nextAction;
  }
}

void finishRecording() {
  RobotRecording recording = Audio_Recorder_Take();
  if (!recording.wav) { enter(RobotState::Error); return; }
  enter(RobotState::Thinking);
  RobotBackendResponse response{};
  const bool ok = Backend_SendConversation(recording.wav, recording.bytes, response);
  free(recording.wav);
  if (!ok) {
    Backend_FreeResponse(response);
    enter(RobotState::Error);
    return;
  }
  playResponse(response);
  Backend_FreeResponse(response);
  enter(personPresent ? RobotState::WakeListening : RobotState::Idle);
}
}  // namespace

void Robot_Controller_Init() {
  Proximity_Init();
  Wake_Word_Init();
  if (ROBOT_ENABLE_NETWORK) Backend_ConnectWifi();
  enter(RobotState::Idle);
}

void Robot_Controller_Update() {
  if (state == RobotState::WakeListening && Wake_Word_Detected()) wakeRequested = true;
  const ProximityReading proximity = Proximity_Read();
  if (proximity.valid) {
    if (!personPresent && proximity.millimeters <= ROBOT_PROXIMITY_WAKE_MM) {
      personPresent = true;
      playedCueThisApproach = false;
      if (state == RobotState::Idle) enter(RobotState::WakeListening);
    } else if (personPresent && proximity.millimeters >= ROBOT_PROXIMITY_RESET_MM) {
      personPresent = false;
      playedCueThisApproach = false;
      if (state == RobotState::WakeListening) enter(RobotState::Idle);
    }
    if (personPresent && proximity.millimeters <= ROBOT_PROXIMITY_TOO_CLOSE_MM && state == RobotState::WakeListening)
      Eye_SetEmotion(EyeEmotion::Surprised);
    if (personPresent && !playedCueThisApproach && proximity.millimeters <= ROBOT_PROXIMITY_CUE_MM &&
        state == RobotState::WakeListening &&
        (!lastCueAt || millis() - lastCueAt >= ROBOT_PROXIMITY_CUE_COOLDOWN_MS)) {
      lastCueAt = millis();
      playedCueThisApproach = true;
      Eye_Notice();
      if (!playLocalProximityCue()) Serial.println("[robot] Put a 16 kHz/16-bit mono PCM /marseillaise.wav on SD for the proximity cue");
    }
  }

  if (wakeRequested && (state == RobotState::WakeListening || state == RobotState::Idle)) {
    wakeRequested = false;
    if (Audio_Recorder_Start()) enter(RobotState::Recording);
    else enter(RobotState::Error);
  }
  if (state == RobotState::Recording && !Audio_Recorder_Update()) finishRecording();
  if (state == RobotState::Error) {
    if (millis() - stateEnteredAt > 2000) enter(personPresent ? RobotState::WakeListening : RobotState::Idle);
  }
}

RobotState Robot_Controller_State() { return state; }
void Robot_Controller_TriggerWakeWord() { wakeRequested = true; }
