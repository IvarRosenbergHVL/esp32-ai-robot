import type { Config } from "../config.js";
import type { TtsProvider, TtsResult } from "../domain.js";
import { createHttpClient, describeHttpError } from "../http.js";

function escapeXml(text: string): string {
  return text.replace(/[<>&'\"]/g, character => ({
    "<": "&lt;", ">": "&gt;", "&": "&amp;", "'": "&apos;", "\"": "&quot;"
  })[character]!);
}

function relativePercent(multiplier: number): string {
  const percent = Math.round((multiplier - 1) * 100);
  return `${percent >= 0 ? "+" : ""}${percent}%`;
}

export class AzureTtsProvider implements TtsProvider {
  private readonly http;

  constructor(private readonly config: Config) {
    this.http = createHttpClient(config.REQUEST_TIMEOUT_MS);
  }

  async synthesize(text: string): Promise<TtsResult> {
    const pitch = relativePercent(this.config.AZURE_SPEECH_PITCH);
    const rate = relativePercent(this.config.AZURE_SPEECH_RATE);
    const volume = relativePercent(this.config.AZURE_SPEECH_VOLUME);
    const ssml = `<speak version="1.0" xml:lang="${this.config.AZURE_SPEECH_LANGUAGE}"><voice name="${this.config.AZURE_SPEECH_VOICE}"><prosody pitch="${pitch}" rate="${rate}" volume="${volume}">${escapeXml(text)}</prosody></voice></speak>`;
    const url = `https://${this.config.AZURE_SPEECH_REGION}.tts.speech.microsoft.com/cognitiveservices/v1`;
    try {
      const response = await this.http.post<ArrayBuffer>(url, ssml, {
        responseType: "arraybuffer",
        headers: {
          "Ocp-Apim-Subscription-Key": this.config.AZURE_SPEECH_KEY,
          "Content-Type": "application/ssml+xml",
          "X-Microsoft-OutputFormat": "riff-16khz-16bit-mono-pcm",
          "User-Agent": "esp32-ai-robot"
        }
      });
      return { audio: Buffer.from(response.data), contentType: "audio/wav" };
    } catch (error) {
      throw describeHttpError(error, "Azure Speech TTS");
    }
  }
}
