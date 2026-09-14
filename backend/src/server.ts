import { createApp } from "./app.js";
import { assertProviderConfiguration, config } from "./config.js";
import { createProviders } from "./providers/index.js";

assertProviderConfiguration(config);
const app = createApp(config, createProviders(config));
const server = app.listen(config.PORT, () => {
  console.log(`Robot backend listening on port ${config.PORT}`);
  console.log(`Providers: STT=${config.STT_PROVIDER}, LLM=${config.LLM_PROVIDER}, TTS=${config.TTS_PROVIDER}`);
});

for (const signal of ["SIGTERM", "SIGINT"] as const) {
  process.on(signal, () => server.close(error => process.exit(error ? 1 : 0)));
}
