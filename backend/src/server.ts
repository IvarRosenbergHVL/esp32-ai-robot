import { createApp } from "./app.js";
import { assertProviderConfiguration, config } from "./config.js";
import { createProviders } from "./providers/index.js";

assertProviderConfiguration(config);
const app = createApp(config, createProviders(config));
app.listen(config.PORT, () => {
  console.log(`Robot backend listening on port ${config.PORT}`);
  console.log(`Providers: STT=${config.STT_PROVIDER}, LLM=${config.LLM_PROVIDER}, TTS=${config.TTS_PROVIDER}`);
});
