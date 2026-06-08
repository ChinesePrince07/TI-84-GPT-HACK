// One-time (idempotent) bootstrap: copy the committed firmware.bin + version.txt
// into R2 so /firmware/version and /firmware/download work the moment the
// api.andypandy.org domain moves to Vercel.
//
// Run from ti84/server with R2 creds in the environment:
//   R2_ENDPOINT=... R2_ACCESS_KEY_ID=... R2_SECRET_ACCESS_KEY=... R2_BUCKET_NAME=... \
//   node scripts/seed-firmware.mjs
import fs from "fs";
import path from "path";
import { fileURLToPath } from "url";
import { r2PutBuffer, r2PutText } from "../lib/r2.mjs";

const __dirname = path.dirname(fileURLToPath(import.meta.url));
const fwDir = path.join(__dirname, "..", "firmware");

const fw = fs.readFileSync(path.join(fwDir, "firmware.bin"));
const version = fs.readFileSync(path.join(fwDir, "version.txt"), "utf8").trim();

await r2PutBuffer("ti84/firmware/firmware.bin", fw);
await r2PutText("ti84/firmware/version.txt", version);
console.log(`Seeded R2: firmware.bin (${fw.length} bytes), version ${version}`);
