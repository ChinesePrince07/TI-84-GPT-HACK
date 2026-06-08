import { Router } from "express";

const MAX = 300;
let buffer = [];
let counter = 0;

// Friendly label for known paths
function classify(method, path, query) {
  if (path.startsWith("/gpt/ask")) {
    return "math" in query ? "MATH" : "sid" in query ? "CHAT" : "GPT";
  }
  if (path === "/firmware/version") return "VER CHECK";
  if (path === "/firmware/download") return "OTA DOWNLOAD";
  if (path === "/firmware/launcher") return "LAUNCHER DOWNLOAD";
  if (path === "/firmware/upload") return "FW UPLOAD";
  if (path === "/firmware/upload_launcher") return "LAUNCHER UPLOAD";
  if (path === "/firmware/debug") return "FW DEBUG";
  if (path.startsWith("/image/")) return "IMAGE";
  if (path.startsWith("/programs/")) return "PROGRAM";
  if (path === "/logs") return method === "POST" ? "SERIAL LOG" : "VIEW LOGS";
  if (path === "/requests" || path.startsWith("/requests/")) return "VIEW REQS";
  return method;
}

function truncate(s, n = 300) {
  if (typeof s !== "string") return s;
  return s.length > n ? s.slice(0, n) + "…" : s;
}

// Middleware: capture every inbound request
export function captureMiddleware() {
  return (req, res, next) => {
    // Skip self-requests to the viewer so it doesn't spam its own feed
    if (req.path.startsWith("/requests")) return next();

    const start = Date.now();
    const id = ++counter;

    res.on("finish", () => {
      const q = { ...req.query };
      for (const k of Object.keys(q)) {
        if (typeof q[k] === "string") q[k] = truncate(q[k], 300);
      }
      const entry = {
        id,
        ts: new Date().toISOString(),
        method: req.method,
        path: req.path,
        type: classify(req.method, req.path, req.query),
        query: q,
        status: res.statusCode,
        durationMs: Date.now() - start,
        ip: req.ip || req.headers["x-forwarded-for"] || "",
        ua: truncate(req.headers["user-agent"] || "", 80),
      };
      buffer.push(entry);
      if (buffer.length > MAX) buffer = buffer.slice(-MAX);
    });

    next();
  };
}

export function requests() {
  const router = Router();

  // JSON feed (paginated by id)
  router.get("/", (req, res) => {
    const since = parseInt(req.query.since) || 0;
    const filter = req.query.type;
    let out = buffer.filter((e) => e.id > since);
    if (filter) out = out.filter((e) => e.type === filter);
    res.json({ total: counter, count: out.length, entries: out });
  });

  // Clear
  router.delete("/", (req, res) => {
    buffer = [];
    res.send("OK");
  });

  // Live HTML viewer — built entirely via safe DOM APIs (no innerHTML for user data)
  router.get("/live", (req, res) => {
    res.type("text/html").send(LIVE_HTML);
  });

  return router;
}

// Static HTML for the live viewer. All dynamic content is appended via
// createElement/textContent in the client script — never via innerHTML —
// so there is no XSS risk from request data.
const LIVE_HTML = `<!DOCTYPE html>
<html lang="en"><head><meta charset="utf-8">
<title>ESP32 Request Monitor</title>
<meta name="viewport" content="width=device-width,initial-scale=1">
<style>
:root { --bg:#0d0d1a; --fg:#e8e8f0; --accent:#4a6cf7; --good:#0fa; --bad:#f55; --warn:#fa3; --dim:#666; }
* { box-sizing: border-box; }
body { background: var(--bg); color: var(--fg); font-family: -apple-system, system-ui, sans-serif; margin:0; padding:16px; }
header { display:flex; align-items:center; gap:16px; margin-bottom:12px; flex-wrap:wrap; }
h1 { color: var(--accent); margin:0; font-size:18px; }
.stats { color: var(--dim); font-size:13px; }
.controls { margin-left:auto; display:flex; gap:8px; flex-wrap:wrap; }
button, select { background:#1a1a2e; color:var(--fg); border:1px solid #333; padding:6px 12px; border-radius:4px; font-size:13px; cursor:pointer; font-family:inherit; }
button:hover { background: #2a2a4e; }
table { width:100%; border-collapse: collapse; font-family: ui-monospace, "SF Mono", Menlo, monospace; font-size:12px; }
thead th { position: sticky; top:0; background: var(--bg); text-align:left; padding:6px 8px; border-bottom:2px solid #333; color: var(--accent); font-weight:600; }
td { padding:5px 8px; border-bottom:1px solid #1a1a2e; vertical-align: top; }
tr:hover td { background: #161628; }
.type-gpt   { color:#9af; }
.type-fw    { color:#fa3; }
.type-up    { color:#f6f; }
.type-log   { color:#0fa; }
.type-asset { color:#0cf; }
.status-2 { color: var(--good); }
.status-4, .status-5 { color: var(--bad); }
.q { color:#bbf; word-break: break-word; max-width: 480px; }
.dim { color: var(--dim); }
.empty { text-align:center; padding:40px; color: var(--dim); }
</style></head>
<body>
<header>
  <h1>ESP32 Request Monitor</h1>
  <span class="stats" id="stats">connecting…</span>
  <div class="controls">
    <select id="filter">
      <option value="">All types</option>
      <option value="MATH">MATH</option>
      <option value="GPT">GPT</option>
      <option value="CHAT">CHAT</option>
      <option value="VER CHECK">VER CHECK</option>
      <option value="OTA DOWNLOAD">OTA DOWNLOAD</option>
      <option value="LAUNCHER DOWNLOAD">LAUNCHER DOWNLOAD</option>
      <option value="SERIAL LOG">SERIAL LOG</option>
      <option value="IMAGE">IMAGE</option>
      <option value="PROGRAM">PROGRAM</option>
    </select>
    <button id="pause">Pause</button>
    <button id="clear">Clear</button>
  </div>
</header>
<table>
  <thead><tr>
    <th>Time</th><th>Type</th><th>Path</th><th>Query</th><th>Status</th><th>ms</th><th>IP</th>
  </tr></thead>
  <tbody id="rows"></tbody>
</table>
<div id="empty" class="empty">No requests yet. POST/GET to any endpoint to see traffic.</div>
<script>
'use strict';
let lastId = 0;
let paused = false;
let filter = '';
const rows = document.getElementById('rows');
const empty = document.getElementById('empty');
const stats = document.getElementById('stats');

const TYPE_CLASSES = {
  'MATH':'type-gpt','GPT':'type-gpt','CHAT':'type-gpt',
  'VER CHECK':'type-fw','OTA DOWNLOAD':'type-fw','LAUNCHER DOWNLOAD':'type-fw',
  'FW UPLOAD':'type-up','LAUNCHER UPLOAD':'type-up',
  'SERIAL LOG':'type-log','VIEW LOGS':'type-log',
  'IMAGE':'type-asset','PROGRAM':'type-asset'
};

function fmtTime(iso) {
  const d = new Date(iso);
  const pad = (n,w) => String(n).padStart(w,'0');
  return pad(d.getHours(),2)+':'+pad(d.getMinutes(),2)+':'+pad(d.getSeconds(),2)+'.'+pad(d.getMilliseconds(),3);
}

function fmtQuery(q) {
  const keys = Object.keys(q || {});
  if (keys.length === 0) return null;
  return keys.map(k => k+'='+String(q[k])).join('&');
}

function statusClass(s) {
  if (s >= 200 && s < 300) return 'status-2';
  if (s >= 400 && s < 500) return 'status-4';
  if (s >= 500) return 'status-5';
  return '';
}

function appendCell(tr, text, className) {
  const td = document.createElement('td');
  if (className) td.className = className;
  if (text === null || text === undefined) {
    const span = document.createElement('span');
    span.className = 'dim';
    span.textContent = '—';
    td.appendChild(span);
  } else {
    td.textContent = String(text);
  }
  tr.appendChild(td);
}

async function poll() {
  if (paused) { setTimeout(poll, 1000); return; }
  try {
    const url = '/requests?since=' + lastId + (filter ? '&type='+encodeURIComponent(filter) : '');
    const r = await fetch(url);
    const d = await r.json();
    if (d.entries.length > 0) {
      empty.style.display = 'none';
      for (const e of d.entries) {
        const tr = document.createElement('tr');
        appendCell(tr, fmtTime(e.ts), 'dim');
        appendCell(tr, e.type, TYPE_CLASSES[e.type] || '');
        appendCell(tr, e.method + ' ' + e.path);
        appendCell(tr, fmtQuery(e.query), 'q');
        appendCell(tr, e.status, statusClass(e.status));
        appendCell(tr, e.durationMs, 'dim');
        appendCell(tr, e.ip || '-', 'dim');
        rows.insertBefore(tr, rows.firstChild);
        if (e.id > lastId) lastId = e.id;
      }
      while (rows.children.length > 300) rows.removeChild(rows.lastChild);
    }
    stats.textContent = d.total + ' total · ' + rows.children.length + ' shown';
  } catch (err) {
    stats.textContent = 'connection error';
  }
  setTimeout(poll, 800);
}

document.getElementById('pause').addEventListener('click', (e) => {
  paused = !paused;
  e.target.textContent = paused ? 'Resume' : 'Pause';
});
document.getElementById('clear').addEventListener('click', async () => {
  await fetch('/requests', { method: 'DELETE' });
  while (rows.firstChild) rows.removeChild(rows.firstChild);
  lastId = 0;
  empty.style.display = '';
});
document.getElementById('filter').addEventListener('change', (e) => {
  filter = e.target.value;
  while (rows.firstChild) rows.removeChild(rows.firstChild);
  lastId = 0;
});
poll();
</script></body></html>`;
