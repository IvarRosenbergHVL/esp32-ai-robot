# Robot conversation backend

Small Node.js 20/TypeScript service for the first voice robot MVP. It accepts a
short WAV recording, transcribes Norwegian speech, produces a controlled robot
reply, synthesizes Norwegian speech, and returns audio plus eye cues.
Outbound calls to Azure use a shared Axios client with bounded timeouts and
sanitized upstream error messages.

## Run locally in mock mode

```bash
cp .env.example .env
npm install
npm run dev
```

Test it with any non-empty WAV file (mock mode does not inspect the audio):

```bash
curl --data-binary @sample.wav \
  -H 'Content-Type: audio/wav' \
  http://localhost:3000/api/v1/conversation
```

Set `STT_PROVIDER=azure`, `LLM_PROVIDER=azure`, and `TTS_PROVIDER=azure` after
creating the resources described in `.env.example`. Test each provider switch
independently before enabling all three.

## API

`POST /api/v1/conversation` accepts a 16-bit PCM WAV recording (mono or stereo,
8/16/24/48 kHz). The current Azure
STT endpoint is intended for short utterances. The response includes transcript,
reply, controlled eye cues, and a 16 kHz/16-bit mono PCM WAV encoded as base64.
This format can be sent directly to the ESP32 I²S playback layer. Base64 keeps the first
firmware integration simple; a streaming or binary endpoint can replace it later.

`GET /health` returns `{ "status": "ok" }`.

The MVP voice is `en-US-AndrewMultilingualNeural` with configurable SSML
prosody. The default multipliers are pitch `1.83`, rate `1.14`, and volume
`1.30`, rendered as `+83%`, `+14%`, and `+30%` respectively.

If `ROBOT_API_KEY` is set, send it as a bearer token. Azure credentials must never
be stored in firmware.

Send a stable `x-session-id` (letters, digits, `_` and `-`, maximum 64 characters)
to retain the latest six conversation turns for up to `SESSION_TTL_MS`. If it is
omitted, the backend creates an ID and returns it as `sessionId`; send that value
on the next request. MVP session state is kept in memory and resets on deployment.

Every response carries `x-request-id`; errors also include it in JSON. `GET /ready`
reports configured provider modes for deployment readiness checks without exposing
credentials.

## Deploy

Build with `docker build -t esp32-ai-robot-backend .` or deploy the `backend`
directory to Azure Container Apps/App Service. Configure all production values
from `.env.example` as platform environment variables or secret references.
Expose port 3000 and configure HTTPS. Do not copy `.env` into the image.

Before deploying, run:

```bash
npm ci
npm test
npm run typecheck
npm run build
```
