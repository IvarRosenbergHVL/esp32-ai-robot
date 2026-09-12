import assert from "node:assert/strict";
import test from "node:test";
import type { AddressInfo } from "node:net";
import { createApp } from "../src/app.js";
import { config } from "../src/config.js";
import type { Providers } from "../src/providers/index.js";

function wav(): Buffer {
  const buffer = Buffer.alloc(46);
  buffer.write("RIFF", 0);
  buffer.writeUInt32LE(38, 4);
  buffer.write("WAVEfmt ", 8);
  buffer.writeUInt32LE(16, 16);
  buffer.writeUInt16LE(1, 20);
  buffer.writeUInt16LE(1, 22);
  buffer.writeUInt32LE(16000, 24);
  buffer.writeUInt32LE(32000, 28);
  buffer.writeUInt16LE(2, 32);
  buffer.writeUInt16LE(16, 34);
  buffer.write("data", 36);
  buffer.writeUInt32LE(2, 40);
  return buffer;
}

function body(bytes: Buffer): Blob {
  return new Blob([Uint8Array.from(bytes)]);
}

const providers: Providers = {
  stt: { transcribe: async () => "Hei" },
  llm: { reply: async (_text, history = []) => ({
    speech: history.length ? "Vi snakker videre" : "Hei på deg",
    emotion: "happy",
    actions: [{ type: "blink", atMs: 100 }]
  }) },
  tts: { synthesize: async () => ({ audio: Buffer.from([1, 2, 3]), contentType: "audio/mpeg" }) }
};

async function withServer(run: (baseUrl: string) => Promise<void>) {
  const server = createApp({ ...config, ROBOT_API_KEY: "test-key" }, providers).listen(0);
  await new Promise<void>(resolve => server.once("listening", resolve));
  try {
    await run(`http://127.0.0.1:${(server.address() as AddressInfo).port}`);
  } finally {
    await new Promise<void>((resolve, reject) => server.close(error => error ? reject(error) : resolve()));
  }
}

test("runs the conversation pipeline and retains bounded session context", async () => {
  await withServer(async baseUrl => {
    const headers = { "content-type": "audio/wav", authorization: "Bearer test-key", "x-session-id": "robot-1" };
    const first = await fetch(`${baseUrl}/api/v1/conversation`, { method: "POST", headers, body: body(wav()) });
    assert.equal(first.status, 200);
    const firstBody = await first.json() as any;
    assert.equal(firstBody.reply, "Hei på deg");
    assert.equal(firstBody.audio.data, "AQID");
    assert.equal(firstBody.input.sampleRate, 16000);

    const second = await fetch(`${baseUrl}/api/v1/conversation`, { method: "POST", headers, body: body(wav()) });
    assert.equal((await second.json() as any).reply, "Vi snakker videre");
  });
});

test("rejects unauthorized and malformed audio requests", async () => {
  await withServer(async baseUrl => {
    const unauthorized = await fetch(`${baseUrl}/api/v1/conversation`, {
      method: "POST", headers: { "content-type": "audio/wav" }, body: body(wav())
    });
    assert.equal(unauthorized.status, 401);

    const malformed = await fetch(`${baseUrl}/api/v1/conversation`, {
      method: "POST",
      headers: { "content-type": "audio/wav", authorization: "Bearer test-key" },
      body: body(Buffer.from("not wav"))
    });
    assert.equal(malformed.status, 400);
    assert.equal((await malformed.json() as any).error, "invalid_wav");
  });
});
