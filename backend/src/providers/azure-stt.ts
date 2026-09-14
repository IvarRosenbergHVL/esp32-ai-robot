import type { Config } from "../config.js";
import type { SttProvider } from "../domain.js";
import { createHttpClient, describeHttpError } from "../http.js";

interface RecognitionResult {
  RecognitionStatus?: string;
  DisplayText?: string;
  NBest?: Array<{ Display?: string }>;
}

export function extractTranscript(payload: RecognitionResult | RecognitionResult[]): string {
  const results = Array.isArray(payload) ? payload : [payload];
  const successful = results.filter(result => result.RecognitionStatus === "Success");
  const transcript = successful
    .map(result => result.DisplayText ?? result.NBest?.[0]?.Display ?? "")
    .filter(Boolean)
    .join(" ")
    .trim();
  if (transcript) return transcript;
  const status = results.map(result => result.RecognitionStatus).filter(Boolean).join(", ") || "unknown";
  throw new Error(`Speech was not recognized: ${status}`);
}

export class AzureSttProvider implements SttProvider {
  private readonly http;

  constructor(private readonly config: Config) {
    this.http = createHttpClient(config.REQUEST_TIMEOUT_MS);
  }

  async transcribe(audio: Buffer, contentType: string): Promise<string> {
    const query = new URLSearchParams({ language: this.config.AZURE_SPEECH_LANGUAGE, format: "detailed" });
    const url = `https://${this.config.AZURE_SPEECH_REGION}.stt.speech.microsoft.com/speech/recognition/conversation/cognitiveservices/v1?${query}`;
    let result: RecognitionResult | RecognitionResult[];
    try {
      const response = await this.http.post(url, audio, {
        headers: {
          "Ocp-Apim-Subscription-Key": this.config.AZURE_SPEECH_KEY,
          "Content-Type": contentType,
          Accept: "application/json"
        }
      });
      result = response.data;
    } catch (error) {
      throw describeHttpError(error, "Azure Speech STT");
    }
    return extractTranscript(result);
  }
}
