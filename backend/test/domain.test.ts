import assert from "node:assert/strict";
import test from "node:test";
import { robotReplySchema } from "../src/domain.js";

test("accepts a valid controlled robot reply", () => {
  const result = robotReplySchema.parse({
    speech: "Hei!", emotion: "happy", actions: [{ type: "blink", atMs: 200 }]
  });
  assert.equal(result.emotion, "happy");
});

test("rejects unknown eye actions", () => {
  assert.throws(() => robotReplySchema.parse({
    speech: "Hei!", emotion: "happy", actions: [{ type: "spin_head", atMs: 0 }]
  }));
});
