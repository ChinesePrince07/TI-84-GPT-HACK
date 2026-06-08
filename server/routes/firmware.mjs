import express from "express";
import { r2GetBuffer, r2GetText, r2PutBuffer, r2PutText } from "../lib/r2.mjs";

const VERSION_KEY = "ti84/firmware/version.txt";
const FIRMWARE_KEY = "ti84/firmware/firmware.bin";
const LAUNCHER_KEY = "ti84/firmware/launcher.bin";

export function firmware() {
  const router = express.Router();

  // Debug endpoint
  router.get("/debug", async (req, res) => {
    try {
      const version = await r2GetText(VERSION_KEY);
      const fw = await r2GetBuffer(FIRMWARE_KEY);
      const launcher = await r2GetBuffer(LAUNCHER_KEY);
      res.json({
        store: "r2",
        versionExists: version !== null,
        firmwareExists: fw !== null,
        launcherExists: launcher !== null,
        version: version ?? "N/A",
        firmwareBytes: fw ? fw.length : 0,
        launcherBytes: launcher ? launcher.length : 0,
      });
    } catch (e) {
      console.error(e);
      res.status(500).json({ store: "r2", error: String(e?.message ?? e) });
    }
  });

  // Current firmware version (defaults to 1.0.0 if none stored)
  router.get("/version", async (req, res) => {
    try {
      const version = await r2GetText(VERSION_KEY);
      res.send(version ? version.trim() : "1.0.0");
    } catch (e) {
      console.error(e);
      res.sendStatus(500);
    }
  });

  // Download launcher binary (for calculator OTA)
  router.get("/launcher", async (req, res) => {
    try {
      const buf = await r2GetBuffer(LAUNCHER_KEY);
      if (!buf) {
        res.status(404).send("No launcher available");
        return;
      }
      res.setHeader("Content-Type", "application/octet-stream");
      res.send(buf);
    } catch (e) {
      console.error(e);
      res.sendStatus(500);
    }
  });

  // Upload launcher (.8xp): strip header, prepend size word, store in R2
  router.post("/upload_launcher", express.raw({ type: "application/octet-stream", limit: "1mb" }), async (req, res) => {
    try {
      const version = req.query.version;
      if (!version) {
        res.status(400).send("version required");
        return;
      }
      const bytes = new Uint8Array(req.body);
      const programBytes = bytes.subarray(74, bytes.length - 2);
      const varBytes = Buffer.from([programBytes.length & 0xff, (programBytes.length >> 8) & 0xff, ...programBytes]);

      await r2PutBuffer(LAUNCHER_KEY, varBytes);
      await r2PutText(VERSION_KEY, String(version));
      res.send("OK");
    } catch (e) {
      console.error(e);
      res.sendStatus(500);
    }
  });

  // Download firmware binary (for ESP32 OTA)
  router.get("/download", async (req, res) => {
    try {
      const buf = await r2GetBuffer(FIRMWARE_KEY);
      if (!buf) {
        res.status(404).send("No firmware available");
        return;
      }
      res.setHeader("Content-Type", "application/octet-stream");
      res.setHeader("Content-Disposition", "attachment; filename=firmware.bin");
      res.send(buf);
    } catch (e) {
      console.error(e);
      res.sendStatus(500);
    }
  });

  // Upload new firmware
  router.post("/upload", express.raw({ type: "application/octet-stream", limit: "4mb" }), async (req, res) => {
    try {
      const version = req.query.version;
      if (!version) {
        res.status(400).send("version required");
        return;
      }
      await r2PutBuffer(FIRMWARE_KEY, Buffer.from(req.body));
      await r2PutText(VERSION_KEY, String(version));
      res.send("OK");
    } catch (e) {
      console.error(e);
      res.sendStatus(500);
    }
  });

  return router;
}
