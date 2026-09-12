import express, { type NextFunction, type Request, type Response } from "express";
import type { Config } from "./config.js";
import type { Providers } from "./providers/index.js";

export function createApp(config: Config, providers: Providers) {
  const app = express();

  app.get("/health", (_request, response) => response.json({ status: "ok" }));

  app.post(
    "/api/v1/conversation",
    express.raw({ type: ["audio/wav", "audio/x-wav"], limit: config.MAX_AUDIO_BYTES }),
    async (request: Request, response: Response, next: NextFunction) => {
      try {
        if (config.ROBOT_API_KEY && request.header("authorization") !== `Bearer ${config.ROBOT_API_KEY}`) {
          response.status(401).json({ error: "unauthorized" });
          return;
        }
        if (!Buffer.isBuffer(request.body) || request.body.length === 0) {
          response.status(400).json({ error: "Send a non-empty WAV body with Content-Type: audio/wav" });
          return;
        }

        const startedAt = performance.now();
        const transcript = await providers.stt.transcribe(request.body, request.header("content-type") ?? "audio/wav");
        const reply = await providers.llm.reply(transcript);
        const synthesized = await providers.tts.synthesize(reply.speech);

        response.json({
          transcript,
          reply: reply.speech,
          emotion: reply.emotion,
          actions: reply.actions,
          audio: {
            contentType: synthesized.contentType,
            encoding: "base64",
            data: synthesized.audio.toString("base64")
          },
          processingMs: Math.round(performance.now() - startedAt)
        });
      } catch (error) { next(error); }
    }
  );

  app.use((error: unknown, _request: Request, response: Response, _next: NextFunction) => {
    const message = error instanceof Error ? error.message : "Unknown error";
    console.error(error);
    response.status(502).json({ error: "conversation_failed", message });
  });
  return app;
}
