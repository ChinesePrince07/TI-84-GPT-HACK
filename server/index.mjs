import express from "express";
import cors from "cors";
import bodyParser from "body-parser";
import morgan from "morgan";
import dot from "dotenv";
import { chatgpt } from "./routes/chatgpt.mjs";
import { images } from "./routes/images.mjs";
import { programs } from "./routes/programs.mjs";
import { firmware } from "./routes/firmware.mjs";
import { logs } from "./routes/logs.mjs";
import { requests, captureMiddleware } from "./routes/requests.mjs";

dot.config();

const app = express();
app.use(morgan("dev"));
app.use(cors("*"));
app.use(bodyParser.raw({ type: "image/jpg", limit: "10mb" }));
app.use((req, res, next) => {
  console.log(req.headers.authorization);
  next();
});

// Capture inbound requests (before route handlers so res.on('finish') fires)
app.use(captureMiddleware());

app.use("/requests", requests());
app.use("/programs", programs());
app.use("/gpt", await chatgpt());
app.use("/image", images());
app.use("/firmware", firmware());
app.use("/logs", logs());

// Vercel imports this module and uses the default export as the handler.
export default app;

// Local dev: only listen when run directly (node index.mjs), not on Vercel.
if (process.argv[1] && import.meta.url === `file://${process.argv[1]}`) {
  const port = +(process.env.PORT ?? 8080);
  app.listen(port, () => console.log(`listening on ${port}`));
}
