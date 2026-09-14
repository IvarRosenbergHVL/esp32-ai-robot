#include "Backend_Client.h"
#include "Robot_Config.h"

#if ROBOT_ENABLE_NETWORK
#include "Robot_Secrets.h"
#include <ArduinoJson.h>
#include <HTTPClient.h>
#include <WiFi.h>
#include <mbedtls/base64.h>

bool Backend_ConnectWifi() {
  if (WiFi.status() == WL_CONNECTED) return true;
  WiFi.mode(WIFI_STA);
  WiFi.begin(ROBOT_WIFI_SSID, ROBOT_WIFI_PASSWORD);
  const uint32_t started = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - started < 15000) delay(100);
  Serial.printf("[network] Wi-Fi %s\n", WiFi.status() == WL_CONNECTED ? "connected" : "failed");
  return WiFi.status() == WL_CONNECTED;
}

bool Backend_SendConversation(const uint8_t *wav, size_t wavBytes, RobotBackendResponse &result) {
  memset(&result, 0, sizeof(result));
  if (!wav || !wavBytes || !Backend_ConnectWifi()) return false;
  HTTPClient http;
  http.setTimeout(ROBOT_BACKEND_TIMEOUT_MS);
  if (!http.begin(ROBOT_BACKEND_URL)) return false;
  http.addHeader("Content-Type", "audio/wav");
  http.addHeader("X-Session-Id", ROBOT_SESSION_ID);
  if (strlen(ROBOT_BACKEND_API_KEY)) http.addHeader("Authorization", String("Bearer ") + ROBOT_BACKEND_API_KEY);
  const int status = http.POST(const_cast<uint8_t *>(wav), wavBytes);
  if (status != HTTP_CODE_OK) {
    Serial.printf("[network] Backend HTTP %d\n", status);
    http.end();
    return false;
  }
  String body = http.getString();
  http.end();
  DynamicJsonDocument document(body.length() + 16384);
  if (deserializeJson(document, body)) return false;
  strlcpy(result.emotion, document["emotion"] | "neutral", sizeof(result.emotion));
  JsonArray actions = document["actions"].as<JsonArray>();
  for (JsonObject action : actions) {
    if (result.actionCount >= ROBOT_MAX_EYE_ACTIONS) break;
    TimedEyeAction &target = result.actions[result.actionCount++];
    strlcpy(target.type, action["type"] | "", sizeof(target.type));
    target.atMs = action["atMs"] | 0;
  }
  const char *encoded = document["audio"]["data"] | "";
  size_t outputBytes = 0;
  if (mbedtls_base64_decode(nullptr, 0, &outputBytes,
      reinterpret_cast<const unsigned char *>(encoded), strlen(encoded)) != MBEDTLS_ERR_BASE64_BUFFER_TOO_SMALL) return false;
  result.audio = static_cast<uint8_t *>(ps_malloc(outputBytes));
  if (!result.audio || mbedtls_base64_decode(result.audio, outputBytes, &result.audioBytes,
      reinterpret_cast<const unsigned char *>(encoded), strlen(encoded)) != 0) {
    Backend_FreeResponse(result);
    return false;
  }
  result.ok = true;
  return true;
}

void Backend_FreeResponse(RobotBackendResponse &result) {
  if (result.audio) free(result.audio);
  memset(&result, 0, sizeof(result));
}
#else
bool Backend_ConnectWifi() { Serial.println("[network] Disabled in Robot_Config.h"); return false; }
bool Backend_SendConversation(const uint8_t *, size_t, RobotBackendResponse &result) { memset(&result, 0, sizeof(result)); return false; }
void Backend_FreeResponse(RobotBackendResponse &result) { if (result.audio) free(result.audio); memset(&result, 0, sizeof(result)); }
#endif
