# Robot API boundary

The first firmware milestone has no backend dependency. The MVP backend lives in
`backend/` and exposes `POST /api/v1/conversation` for short WAV utterances.

Planned logical operations:

| Operation | Input | Output |
|---|---|---|
| Speech to text | Recorded utterance | Azure Speech transcript |
| Conversation | Transcript and session context | Response text and animation cues |
| Text to speech | Response text | Azure Speech MP3 and metadata |

NB-Whisper and Chatterbox remain planned alternative providers. Firmware must
only contain the robot API address/device credential and never Azure secrets.

## MVP request

```http
POST /api/v1/conversation
Content-Type: audio/wav
Authorization: Bearer <optional robot API key>
```

The body is a short mono PCM WAV recording, preferably 16 kHz and 16-bit.

## MVP response

```json
{
  "transcript": "Hei, hvem er du?",
  "reply": "Jeg er MIME, en liten vikingrobot.",
  "emotion": "happy",
  "actions": [{ "type": "blink", "atMs": 700 }],
  "audio": {
    "contentType": "audio/mpeg",
    "encoding": "base64",
    "data": "..."
  },
  "processingMs": 1234
}
```
