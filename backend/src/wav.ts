import { AppError } from "./errors.js";

export interface WavInfo {
  audioFormat: number;
  channels: number;
  sampleRate: number;
  bitsPerSample: number;
  dataBytes: number;
}

export function inspectWav(buffer: Buffer): WavInfo {
  if (buffer.length < 44 || buffer.toString("ascii", 0, 4) !== "RIFF" || buffer.toString("ascii", 8, 12) !== "WAVE") {
    throw new AppError(400, "invalid_wav", "Body must be a valid RIFF/WAVE file");
  }

  let offset = 12;
  let format: Omit<WavInfo, "dataBytes"> | undefined;
  let dataBytes: number | undefined;
  while (offset + 8 <= buffer.length) {
    const id = buffer.toString("ascii", offset, offset + 4);
    const size = buffer.readUInt32LE(offset + 4);
    const start = offset + 8;
    if (start + size > buffer.length) throw new AppError(400, "invalid_wav", "WAV contains a truncated chunk");
    if (id === "fmt " && size >= 16) {
      format = {
        audioFormat: buffer.readUInt16LE(start),
        channels: buffer.readUInt16LE(start + 2),
        sampleRate: buffer.readUInt32LE(start + 4),
        bitsPerSample: buffer.readUInt16LE(start + 14)
      };
    } else if (id === "data") {
      dataBytes = size;
    }
    offset = start + size + (size % 2);
  }

  if (!format || dataBytes === undefined) throw new AppError(400, "invalid_wav", "WAV must contain fmt and data chunks");
  if (format.audioFormat !== 1 || ![1, 2].includes(format.channels) || ![8000, 16000, 24000, 48000].includes(format.sampleRate) || format.bitsPerSample !== 16) {
    throw new AppError(415, "unsupported_wav", "Use 16-bit PCM WAV, mono or stereo, at 8, 16, 24, or 48 kHz");
  }
  if (dataBytes === 0) throw new AppError(400, "empty_audio", "WAV audio data is empty");
  return { ...format, dataBytes };
}

