import assert from "node:assert/strict";
import test from "node:test";
import { inspectWav } from "../src/wav.js";

test("rejects bytes that are not RIFF/WAVE", () => {
  assert.throws(() => inspectWav(Buffer.alloc(44)), /RIFF\/WAVE/);
});

