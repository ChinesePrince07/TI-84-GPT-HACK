import express from "express";
import openai from "openai";
import jimp from "jimp";
import crypto from "crypto";
import { r2GetJson, r2PutJson } from "../lib/r2.mjs";

const CHAT_KEY = "ti84/chat/db.json";
const DAY_MS = 24 * 60 * 60 * 1000;

async function readDb() {
  return await r2GetJson(CHAT_KEY, { conversations: {} });
}
async function writeDb(data) {
  await r2PutJson(CHAT_KEY, data);
}

export async function chatgpt() {
  const routes = express.Router();

  // Lazy-init so a missing/invalid OPENAI_API_KEY only breaks /gpt routes,
  // not firmware OTA / programs / the rest of the server.
  let _gpt;
  const getGpt = () => (_gpt ??= new openai.OpenAI());

  routes.get("/ask", async (req, res) => {
    const question = req.query.question ?? "";
    if (Array.isArray(question)) {
      res.sendStatus(400);
      return;
    }

    const hasSid = "sid" in req.query;

    try {
      // Stateless mode (derivative, translate, etc.)
      if (!hasSid) {
        const isMath = "math" in req.query;
        const systemPrompt = isMath
          ? "You are a precise math solver for a TI-84 calculator. Compute the EXACT answer. Show ONLY the final numerical result or simplified expression. Use UPPERCASE. NEVER use LaTeX, backslashes, or curly braces. Write fractions as A/B, exponents as X^N, pi as PI, sqrt as SQRT(). Keep under 200 characters."
          : "You are answering questions on a TI-84 calculator. Keep responses under 100 characters, use UPPERCASE letters only. NEVER use LaTeX, backslashes, or curly braces. Write fractions as A/B, exponents as X^N, pi as PI, sqrt as SQRT().";
        const result = await getGpt().chat.completions.create({
          messages: [
            { role: "system", content: systemPrompt },
            { role: "user", content: question },
          ],
          model: isMath ? "gpt-5.4" : "gpt-5.4-nano",
        });
        res.send(result.choices[0]?.message?.content ?? "no response");
        return;
      }

      // Chat mode with session
      const data = await readDb();

      // Cleanup old conversations
      const now = Date.now();
      for (const [id, conv] of Object.entries(data.conversations)) {
        if (now - conv.created > DAY_MS) delete data.conversations[id];
      }

      let sessionId = req.query.sid;
      let history = [];

      if (sessionId && data.conversations[sessionId]) {
        history = data.conversations[sessionId].messages;
      } else {
        sessionId = crypto.randomBytes(4).toString("hex");
        data.conversations[sessionId] = { created: now, messages: [] };
      }

      const messages = [
        {
          role: "system",
          content:
            "You are answering questions on a TI-84 calculator. Keep responses under 100 characters, use UPPERCASE letters only. NEVER use LaTeX, backslashes, or curly braces. Write fractions as A/B, exponents as X^N, pi as PI, sqrt as SQRT().",
        },
        ...history.slice(-10),
        { role: "user", content: question },
      ];

      const result = await getGpt().chat.completions.create({ messages, model: "gpt-5.4-nano" });
      const answer = result.choices[0]?.message?.content ?? "NO RESPONSE";

      data.conversations[sessionId].messages.push(
        { role: "user", content: question },
        { role: "assistant", content: answer }
      );
      await writeDb(data);

      res.send(`${sessionId}|${answer}`);
    } catch (e) {
      console.error(e);
      res.sendStatus(500);
    }
  });

  routes.get("/history", async (req, res) => {
    const sid = req.query.sid ?? "";
    const page = parseInt(req.query.p ?? "0");

    if (!sid) {
      res.status(400).send("NO SESSION");
      return;
    }

    const data = await readDb();
    const conv = data.conversations[sid];
    if (!conv) {
      res.send("0/0|NO HISTORY");
      return;
    }

    const totalPairs = Math.floor(conv.messages.length / 2);
    if (page < 0 || page >= totalPairs) {
      res.send(`${page}/${totalPairs}|NO MORE`);
      return;
    }

    const q = conv.messages[page * 2].content.substring(0, 80);
    const a = conv.messages[page * 2 + 1].content.substring(0, 150);
    res.send(`${page}/${totalPairs}|Q:${q} A:${a}`);
  });

  // Solve a math equation from an uploaded image (in-memory, no disk write).
  routes.post("/solve", async (req, res) => {
    try {
      const contentType = req.headers["content-type"];
      if (contentType !== "image/jpg") {
        res.status(400).send(`bad content-type: ${contentType}`);
        return;
      }

      const image = await jimp.read(req.body);
      const jpegBuffer = await image.getBufferAsync(jimp.MIME_JPEG);
      const encoded_image = jpegBuffer.toString("base64");

      const question_number = req.query.n;
      const question = question_number
        ? `What is the answer to question ${question_number}?`
        : "What is the answer to this question?";

      const result = await getGpt().chat.completions.create({
        messages: [
          {
            role: "system",
            content:
              "You are a helpful math tutor, specifically designed to help with basic arithmetic, but also can answer a broad range of math questions from uploaded images. You should provide answers as succinctly as possible, and always under 100 characters. Be as accurate as possible.",
          },
          {
            role: "user",
            content: [
              {
                type: "text",
                text: `${question} Do not explain how you found the answer. If the question is multiple-choice, give the letter answer.`,
              },
              { type: "image_url", image_url: { url: `data:image/jpeg;base64,${encoded_image}`, detail: "high" } },
            ],
          },
        ],
        model: "gpt-5.4-nano",
      });

      res.send(result.choices[0]?.message?.content ?? "no response");
    } catch (e) {
      console.error(e);
      res.sendStatus(500);
    }
  });

  return routes;
}
