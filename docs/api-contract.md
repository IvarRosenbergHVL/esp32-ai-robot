# Future robot API boundary

The first firmware milestone has no backend dependency. Audio transport will be
implemented behind a separate client interface when the service is available.

Planned logical operations:

| Operation | Input | Output |
|---|---|---|
| Speech to text | Recorded utterance | NB-Whisper transcript |
| Conversation | Transcript and session context | Response text and animation cues |
| Text to speech | Response text | Chatterbox audio stream/file and metadata |

Exact URLs, authentication and media formats remain intentionally undecided.
Firmware code must not hard-code service URLs or credentials.
