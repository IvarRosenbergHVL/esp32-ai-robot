import express, { type NextFunction, type Request, type Response } from "express";
import type { Config } from "./config.js";
import type { Providers } from "./providers/index.js";
import { randomUUID } from "node:crypto";
import { AppError } from "./errors.js";
import { SessionStore } from "./sessions.js";
import { inspectWav } from "./wav.js";

export function createApp(config: Config, providers: Providers) {
  const app = express();
  const sessions = new SessionStore(config.SESSION_TTL_MS, config.MAX_SESSIONS);
  app.disable("x-powered-by");

  app.use((request, response, next) => {
    const requestId = request.header("x-request-id")?.slice(0, 128) || randomUUID();
    response.setHeader("x-request-id", requestId);
    response.setHeader("cache-control", "no-store");
    response.locals.requestId = requestId;
    next();
  });

  app.get("/health", (_request, response) => response.json({ status: "ok" }));
  app.get("/ready", (_request, response) => response.json({ status: "ready", providers: {
    stt: config.STT_PROVIDER, llm: config.LLM_PROVIDER, tts: config.TTS_PROVIDER
  }}));

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

        const wav = inspectWav(request.body);
        const suppliedSessionId = request.header("x-session-id")?.trim();
        if (suppliedSessionId && !/^[A-Za-z0-9_-]{1,64}$/.test(suppliedSessionId)) {
          throw new AppError(400, "invalid_session_id", "x-session-id may contain only letters, digits, _ and -");
        }
        const sessionId = suppliedSessionId || randomUUID();

        const startedAt = performance.now();
        const transcript = await providers.stt.transcribe(request.body, request.header("content-type") ?? "audio/wav");
        const reply = await providers.llm.reply(transcript, sessions.get(sessionId));
        const synthesized = await providers.tts.synthesize(reply.speech);
        sessions.append(sessionId, { user: transcript, assistant: reply.speech });

        response.json({
          requestId: response.locals.requestId,
          sessionId,
          transcript,
          reply: reply.speech,
          emotion: reply.emotion,
          actions: reply.actions,
          audio: {
            contentType: synthesized.contentType,
            encoding: "base64",
            data: synthesized.audio.toString("base64")
          },
          processingMs: Math.round(performance.now() - startedAt),
          input: { sampleRate: wav.sampleRate, channels: wav.channels, bitsPerSample: wav.bitsPerSample }
        });
      } catch (error) { next(error); }
    }
  );

  app.use((error: unknown, _request: Request, response: Response, _next: NextFunction) => {
    const requestId = response.locals.requestId;
    const status = error instanceof AppError ? error.status : 502;
    const code = error instanceof AppError ? error.code : "conversation_failed";
    const message = error instanceof AppError && error.expose ? error.message : "A backend service failed";
    console.error(JSON.stringify({ level: "error", requestId, code, detail: error instanceof Error ? error.message : String(error) }));
    response.status(status).json({ error: code, message, requestId });
  });
  return app;
}
