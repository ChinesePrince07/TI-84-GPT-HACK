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

async function main() {
  const port = +(process.env.PORT ?? 8080);
  if (!port || !Number.isInteger(port)) {
    console.error("bad port");
    process.exit(1);
  }

  const app = express();
  app.use(morgan("dev"));
  app.use(cors("*"));
  app.use(
    bodyParser.raw({
      type: "image/jpg",
      limit: "10mb",
    })
  );
  app.use((req, res, next) => {
    console.log(req.headers.authorization);
    next();
  });

  // Capture inbound requests (must come before route handlers so res.on('finish') fires)
  app.use(captureMiddleware());

  // Request monitor
  app.use("/requests", requests());

  // Programs
  app.use("/programs", programs());

  // ChatGPT
  app.use("/gpt", await chatgpt());

  // Images
  app.use("/image", images());

  // Firmware OTA updates
  app.use("/firmware", firmware());

  // Remote serial monitor
  app.use("/logs", logs());

  app.listen(port, () => {
    console.log(`listening on ${port}`);
  });
}

main();
