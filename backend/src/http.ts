import axios, { AxiosError, type AxiosInstance } from "axios";

export function createHttpClient(timeoutMs: number): AxiosInstance {
  return axios.create({
    timeout: timeoutMs,
    maxContentLength: 10_000_000,
    maxBodyLength: 10_000_000,
    validateStatus: status => status >= 200 && status < 300
  });
}

export function describeHttpError(error: unknown, service: string): Error {
  if (!(error instanceof AxiosError)) return error instanceof Error ? error : new Error(`${service} failed`);
  const status = error.response?.status;
  const raw = error.response?.data;
  const detail = typeof raw === "string" ? raw : raw ? JSON.stringify(raw) : error.message;
  return new Error(`${service}${status ? ` returned ${status}` : " failed"}: ${detail.slice(0, 1000)}`);
}
