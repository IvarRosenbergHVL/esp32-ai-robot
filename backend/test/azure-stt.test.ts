import assert from "node:assert/strict";
import test from "node:test";
import { extractTranscript } from "../src/providers/azure-stt.js";

test("joins successful segmented STT output and ignores partial results", () => {
  assert.equal(extractTranscript([
    { RecognitionStatus: "Success", DisplayText: "Altså det naturlige." },
    { RecognitionStatus: "Success", DisplayText: "Ja, grunnen til den." },
    { RecognitionStatus: "Partial", DisplayText: "nei" }
  ]), "Altså det naturlige. Ja, grunnen til den.");
});

test("accepts the single-result response used by short recognition", () => {
  assert.equal(extractTranscript({
    RecognitionStatus: "Success",
    NBest: [{ Display: "Hei MIME." }]
  }), "Hei MIME.");
});
