import { z } from "zod";

export const emotions = [
  "neutral", "attentive", "happy", "curious", "thinking",
  "surprised", "skeptical", "sleepy", "error"
] as const;

export const actionTypes = [
  "blink", "double_blink", "wink_left", "wink_right", "look_left",
  "look_right", "look_up", "look_down", "look_center", "widen",
  "squint", "startle", "reset"
] as const;

export const robotReplySchema = z.object({
  speech: z.string().min(1).max(1200),
  emotion: z.enum(emotions),
  actions: z.array(z.object({
    type: z.enum(actionTypes),
    atMs: z.number().int().min(0).max(120_000)
  })).max(20).default([])
});

export type RobotReply = z.infer<typeof robotReplySchema>;
export interface ConversationTurn { user: string; assistant: string; }

export interface SttProvider { transcribe(audio: Buffer, contentType: string): Promise<string>; }
export interface LlmProvider { reply(transcript: string, history?: ConversationTurn[]): Promise<RobotReply>; }
export interface TtsResult { audio: Buffer; contentType: string; }
export interface TtsProvider { synthesize(text: string): Promise<TtsResult>; }
