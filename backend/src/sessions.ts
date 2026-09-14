import type { ConversationTurn } from "./domain.js";

interface Session { touchedAt: number; turns: ConversationTurn[]; }

export class SessionStore {
  private readonly sessions = new Map<string, Session>();

  constructor(private readonly ttlMs: number, private readonly maxSessions: number) {}

  get(id: string): ConversationTurn[] {
    this.prune();
    const session = this.sessions.get(id);
    if (!session) return [];
    session.touchedAt = Date.now();
    return [...session.turns];
  }

  append(id: string, turn: ConversationTurn): void {
    this.prune();
    const session = this.sessions.get(id) ?? { touchedAt: Date.now(), turns: [] };
    session.touchedAt = Date.now();
    session.turns = [...session.turns, turn].slice(-6);
    this.sessions.delete(id);
    this.sessions.set(id, session);
    while (this.sessions.size > this.maxSessions) this.sessions.delete(this.sessions.keys().next().value!);
  }

  private prune(): void {
    const cutoff = Date.now() - this.ttlMs;
    for (const [id, session] of this.sessions) if (session.touchedAt < cutoff) this.sessions.delete(id);
  }
}

