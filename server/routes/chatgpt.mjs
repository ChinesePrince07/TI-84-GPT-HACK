import express from "express";
import openai from "openai";
import i264 from "image-to-base64";
import jimp from "jimp";
import { JSONFilePreset } from "lowdb/node";
import crypto from "crypto";

const db = await JSONFilePreset("db.json", { conversations: {} });
const DAY_MS = 24 * 60 * 60 * 1000;

export async function chatgpt() {
  const routes = express.Router();

  const gpt = new openai.OpenAI();

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
          ? "You are a precise math solver for a TI-84 calculator. Compute the EXACT correct answer. You MUST do the actual computation, not guess. Use UPPERCASE. NEVER use LaTeX, backslashes, or curly braces. Write fractions as A/B, exponents as X^N, pi as PI, sqrt as SQRT(). Keep under 250 characters."
          : "You are answering questions on a TI-84 calculator. Keep responses under 100 characters, use UPPERCASE letters only. NEVER use LaTeX, backslashes, or curly braces. Write fractions as A/B, exponents as X^N, pi as PI, sqrt as SQRT().";
        const result = await gpt.chat.completions.create({
          messages: [
            { role: "system", content: systemPrompt },
            { role: "user", content: question },
          ],
          model: "gpt-5.2",
        });
        res.send(result.choices[0]?.message?.content ?? "no response");
        return;
      }

      // Chat mode with session
      await db.read();

      // Cleanup old conversations
      const now = Date.now();
      for (const [id, conv] of Object.entries(db.data.conversations)) {
        if (now - conv.created > DAY_MS) delete db.data.conversations[id];
      }

      let sessionId = req.query.sid;
      let history = [];

      if (sessionId && db.data.conversations[sessionId]) {
        history = db.data.conversations[sessionId].messages;
      } else {
        sessionId = crypto.randomBytes(4).toString("hex");
        db.data.conversations[sessionId] = { created: now, messages: [] };
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

      const result = await gpt.chat.completions.create({
        messages,
        model: "gpt-5.2",
      });

      const answer = result.choices[0]?.message?.content ?? "NO RESPONSE";

      db.data.conversations[sessionId].messages.push(
        { role: "user", content: question },
        { role: "assistant", content: answer }
      );
      await db.write();

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

    await db.read();
    const conv = db.data.conversations[sid];
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

  // solve a math equation from an image.
  routes.post("/solve", async (req, res) => {
    try {
      const contentType = req.headers["content-type"];
      console.log("content-type:", contentType);

      if (contentType !== "image/jpg") {
        res.status(400);
        res.send(`bad content-type: ${contentType}`);
      }

      const image_data = await new Promise((resolve, reject) => {
        jimp.read(req.body, (err, value) => {
          if (err) {
            reject(err);
            return;
          }
          resolve(value);
        });
      });

      const image_path = "./to_solve.jpg";

      await image_data.writeAsync(image_path);
      const encoded_image = await i264(image_path);
      console.log("Encoded Image: ", encoded_image.length, "bytes");
      console.log(encoded_image.substring(0, 100));

      const question_number = req.query.n;

      const question = question_number
        ? `What is the answer to question ${question_number}?`
        : "What is the answer to this question?";

      console.log("prompt:", question);

      const result = await gpt.chat.completions.create({
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
              {
                type: "image_url",
                image_url: {
                  url: `data:image/jpeg;base64,${encoded_image}`,
                  detail: "high",
                },
              },
            ],
          },
        ],
        model: "gpt-5.2",
      });

      res.send(result.choices[0]?.message?.content ?? "no response");
    } catch (e) {
      console.error(e);
      res.sendStatus(500);
    }
  });

  return routes;
}
