import type { Config } from "../config.js";
import type { LlmProvider, SttProvider, TtsProvider } from "../domain.js";
import { AzureLlmProvider } from "./azure-llm.js";
import { AzureSttProvider } from "./azure-stt.js";
import { AzureTtsProvider } from "./azure-tts.js";
import { MockLlmProvider, MockSttProvider, MockTtsProvider } from "./mock.js";

export interface Providers { stt: SttProvider; llm: LlmProvider; tts: TtsProvider; }

export function createProviders(config: Config): Providers {
  return {
    stt: config.STT_PROVIDER === "azure" ? new AzureSttProvider(config) : new MockSttProvider(config),
    llm: config.LLM_PROVIDER === "azure" ? new AzureLlmProvider(config) : new MockLlmProvider(),
    tts: config.TTS_PROVIDER === "azure" ? new AzureTtsProvider(config) : new MockTtsProvider(config)
  };
}
