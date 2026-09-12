import { readFile } from "node:fs/promises";
import type { Config } from "../config.js";
import type { LlmProvider, RobotReply, SttProvider, TtsProvider, TtsResult } from "../domain.js";

export class MockSttProvider implements SttProvider {
  constructor(private readonly config: Config) {}
  async transcribe(): Promise<string> { return this.config.MOCK_TRANSCRIPT; }
}

export class MockLlmProvider implements LlmProvider {
  async reply(transcript: string): Promise<RobotReply> {
    return {
      speech: `Jeg hørte: ${transcript}`,
      emotion: "happy",
      actions: [{ type: "look_center", atMs: 0 }, { type: "blink", atMs: 700 }]
    };
  }
}

export class MockTtsProvider implements TtsProvider {
  constructor(private readonly config: Config) {}
  async synthesize(): Promise<TtsResult> {
    const audio = this.config.MOCK_TTS_FILE ? await readFile(this.config.MOCK_TTS_FILE) : Buffer.alloc(0);
    return { audio, contentType: "audio/mpeg" };
  }
}
