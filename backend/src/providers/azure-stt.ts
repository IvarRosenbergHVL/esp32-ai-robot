import type { Config } from "../config.js";
import type { SttProvider } from "../domain.js";
import { createHttpClient, describeHttpError } from "../http.js";

export class AzureSttProvider implements SttProvider {
  private readonly http;

  constructor(private readonly config: Config) {
    this.http = createHttpClient(config.REQUEST_TIMEOUT_MS);
  }

  async transcribe(audio: Buffer, contentType: string): Promise<string> {
    const query = new URLSearchParams({ language: this.config.AZURE_SPEECH_LANGUAGE, format: "detailed" });
    const url = `https://${this.config.AZURE_SPEECH_REGION}.stt.speech.microsoft.com/speech/recognition/conversation/cognitiveservices/v1?${query}`;
    let result: { RecognitionStatus?: string; DisplayText?: string; NBest?: Array<{ Display?: string }> };
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
    if (result.RecognitionStatus !== "Success") throw new Error(`Speech was not recognized: ${result.RecognitionStatus ?? "unknown"}`);
    const text = result.DisplayText ?? result.NBest?.[0]?.Display;
    if (!text) throw new Error("Azure Speech STT returned an empty transcript");
    return text;
  }
}
