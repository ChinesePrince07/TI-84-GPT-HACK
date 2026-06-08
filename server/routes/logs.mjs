import { Router } from "express";
import express from "express";

const MAX_LINES = 500;
let logLines = [];

export function logs() {
  const router = Router();

  // ESP32 posts log lines
  router.post("/", express.text({ limit: "1mb" }), (req, res) => {
    const lines = req.body.split("\n").filter((l) => l.length > 0);
    logLines.push(...lines);
    if (logLines.length > MAX_LINES) {
      logLines = logLines.slice(-MAX_LINES);
    }
    res.send("OK");
  });

  // View logs in browser
  router.get("/", (req, res) => {
    const since = parseInt(req.query.since) || 0;
    const output = logLines.slice(since);
    res.json({ total: logLines.length, lines: output });
  });

  // View logs as plain text
  router.get("/text", (req, res) => {
    res.type("text/plain").send(logLines.join("\n"));
  });

  // Clear logs
  router.delete("/", (req, res) => {
    logLines = [];
    res.send("OK");
  });

  // Live view page
  router.get("/live", (req, res) => {
    res.type("text/html").send(`<!DOCTYPE html>
<html><head><title>ESP32 Serial Monitor</title>
<style>
body{background:#1a1a2e;color:#0f0;font-family:monospace;margin:0;padding:20px}
h1{color:#4a6cf7;font-family:sans-serif}
#log{white-space:pre-wrap;font-size:14px;height:80vh;overflow-y:auto;border:1px solid #333;padding:10px;background:#0d0d1a}
</style></head><body>
<h1>ESP32 Serial Monitor</h1>
<div id="log"></div>
<script>
let idx=0;
async function poll(){
  try{
    const r=await fetch('/logs?since='+idx);
    const d=await r.json();
    if(d.lines.length>0){
      document.getElementById('log').textContent+=d.lines.join('\\n')+'\\n';
      idx=d.total;
      document.getElementById('log').scrollTop=999999;
    }
  }catch(e){}
  setTimeout(poll,1000);
}
poll();
</script></body></html>`);
  });

  return router;
}
