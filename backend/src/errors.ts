export class AppError extends Error {
  constructor(
    public readonly status: number,
    public readonly code: string,
    message: string,
    public readonly expose = true
  ) {
    super(message);
    this.name = "AppError";
  }
}

