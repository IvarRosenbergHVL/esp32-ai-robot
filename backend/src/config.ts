import "dotenv/config";
import { z } from "zod";

const provider = z.enum(["mock", "azure"]);

const schema = z.object({
  PORT: z.coerce.number().int().positive().default(3000),
  MAX_AUDIO_BYTES: z.coerce.number().int().positive().default(4_000_000),
  REQUEST_TIMEOUT_MS: z.coerce.number().int().positive().default(30_000),
  SESSION_TTL_MS: z.coerce.number().int().positive().default(30 * 60_000),
  MAX_SESSIONS: z.coerce.number().int().positive().max(10_000).default(100),
  ROBOT_API_KEY: z.string().default(""),
  STT_PROVIDER: provider.default("mock"),
  LLM_PROVIDER: provider.default("mock"),
  TTS_PROVIDER: provider.default("mock"),
  AZURE_SPEECH_REGION: z.string().default("norwayeast"),
  AZURE_SPEECH_KEY: z.string().default(""),
  AZURE_SPEECH_LANGUAGE: z.string().default("nb-NO"),
  AZURE_SPEECH_VOICE: z.string().default("en-US-AndrewMultilingualNeural"),
  AZURE_SPEECH_PITCH: z.coerce.number().min(0.5).max(2).default(1.83),
  AZURE_SPEECH_RATE: z.coerce.number().min(0.5).max(2).default(1.14),
  AZURE_SPEECH_VOLUME: z.coerce.number().min(0).max(2).default(1.3),
  AZURE_OPENAI_ENDPOINT: z.string().default(""),
  AZURE_OPENAI_API_KEY: z.string().default(""),
  AZURE_OPENAI_DEPLOYMENT: z.string().default(""),
  AZURE_OPENAI_API_VERSION: z.string().default("2025-04-01-preview"),
  MOCK_TRANSCRIPT: z.string().default("Hei, hvem er du?"),
  MOCK_TTS_FILE: z.string().default("")
});

export type Config = z.infer<typeof schema>;
export const config = schema.parse(process.env);

export function assertProviderConfiguration(value: Config): void {
  if (value.STT_PROVIDER === "azure" || value.TTS_PROVIDER === "azure") {
    const missing = [["AZURE_SPEECH_KEY", value.AZURE_SPEECH_KEY], ["AZURE_SPEECH_REGION", value.AZURE_SPEECH_REGION]]
      .filter(([, entry]) => !entry).map(([name]) => name);
    if (missing.length) throw new Error(`Missing Azure Speech configuration: ${missing.join(", ")}`);
  }
  if (value.LLM_PROVIDER === "azure") {
    const missing = [
      ["AZURE_OPENAI_ENDPOINT", value.AZURE_OPENAI_ENDPOINT],
      ["AZURE_OPENAI_API_KEY", value.AZURE_OPENAI_API_KEY],
      ["AZURE_OPENAI_DEPLOYMENT", value.AZURE_OPENAI_DEPLOYMENT]
    ].filter(([, entry]) => !entry).map(([name]) => name);
    if (missing.length) throw new Error(`Missing Azure OpenAI configuration: ${missing.join(", ")}`);
  }
}
