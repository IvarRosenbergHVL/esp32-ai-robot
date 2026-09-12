import type { Config } from "../config.js";
import { actionTypes, emotions, robotReplySchema, type LlmProvider, type RobotReply } from "../domain.js";
import { createHttpClient, describeHttpError } from "../http.js";

const instructions = `Du er stemmen og personligheten til en liten, vennlig vikingrobot med to runde øyne. Svar kort og naturlig på norsk. Velg én kontrollert øyefølelse og høyst noen få øyehandlinger. Handlingstidene er millisekunder fra starten av lydavspillingen. Ikke lag andre følelser eller handlinger enn de skjemaet tillater.`;

function extractOutputText(payload: { output_text?: string; output?: Array<{ content?: Array<{ type?: string; text?: string }> }> }): string {
  if (payload.output_text) return payload.output_text;
  for (const item of payload.output ?? []) {
    for (const content of item.content ?? []) if (content.type === "output_text" && content.text) return content.text;
  }
  throw new Error("Azure OpenAI returned no output text");
}

export class AzureLlmProvider implements LlmProvider {
  private readonly http;

  constructor(private readonly config: Config) {
    this.http = createHttpClient(config.REQUEST_TIMEOUT_MS);
  }

  async reply(transcript: string): Promise<RobotReply> {
    const endpoint = this.config.AZURE_OPENAI_ENDPOINT.replace(/\/$/, "");
    const url = `${endpoint}/openai/responses?api-version=${encodeURIComponent(this.config.AZURE_OPENAI_API_VERSION)}`;
    try {
      const response = await this.http.post(url, {
        model: this.config.AZURE_OPENAI_DEPLOYMENT,
        instructions,
        input: transcript,
        max_output_tokens: 500,
        text: {
          format: {
            type: "json_schema",
            name: "robot_reply",
            strict: true,
            schema: {
              type: "object",
              additionalProperties: false,
              required: ["speech", "emotion", "actions"],
              properties: {
                speech: { type: "string", minLength: 1, maxLength: 1200 },
                emotion: { type: "string", enum: emotions },
                actions: {
                  type: "array", maxItems: 20,
                  items: {
                    type: "object", additionalProperties: false, required: ["type", "atMs"],
                    properties: {
                      type: { type: "string", enum: actionTypes },
                      atMs: { type: "integer", minimum: 0, maximum: 120000 }
                    }
                  }
                }
              }
            }
          }
        }
      }, {
        headers: { "api-key": this.config.AZURE_OPENAI_API_KEY, "Content-Type": "application/json" }
      });
      return robotReplySchema.parse(JSON.parse(extractOutputText(response.data)));
    } catch (error) {
      throw describeHttpError(error, "Azure OpenAI Responses");
    }
  }
}
