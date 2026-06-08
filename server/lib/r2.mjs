import { S3Client, GetObjectCommand, PutObjectCommand } from "@aws-sdk/client-s3";

// Vercel env vars sometimes carry stray newlines/whitespace when pasted.
function envTrim(name) {
  return (process.env[name] || "").trim();
}

const s3 = new S3Client({
  region: "auto",
  endpoint: envTrim("R2_ENDPOINT") || undefined,
  credentials: {
    accessKeyId: envTrim("R2_ACCESS_KEY_ID"),
    secretAccessKey: envTrim("R2_SECRET_ACCESS_KEY"),
  },
});

const BUCKET = envTrim("R2_BUCKET_NAME") || "afilmory-photos";

function isNotFound(err) {
  return err?.$metadata?.httpStatusCode === 404 || err?.name === "NoSuchKey";
}

export async function r2GetBuffer(key) {
  try {
    const res = await s3.send(new GetObjectCommand({ Bucket: BUCKET, Key: key }));
    if (!res.Body) return null;
    const bytes = await res.Body.transformToByteArray();
    return Buffer.from(bytes);
  } catch (err) {
    if (isNotFound(err)) return null;
    throw err;
  }
}

export async function r2GetText(key) {
  const buf = await r2GetBuffer(key);
  return buf ? buf.toString("utf8") : null;
}

export async function r2GetJson(key, fallback) {
  const txt = await r2GetText(key);
  if (txt === null) return fallback;
  try {
    return JSON.parse(txt);
  } catch {
    return fallback;
  }
}

export async function r2PutBuffer(key, body, contentType = "application/octet-stream") {
  await s3.send(new PutObjectCommand({ Bucket: BUCKET, Key: key, Body: body, ContentType: contentType }));
}

export async function r2PutText(key, text, contentType = "text/plain") {
  await r2PutBuffer(key, Buffer.from(text, "utf8"), contentType);
}

export async function r2PutJson(key, obj) {
  await r2PutBuffer(key, Buffer.from(JSON.stringify(obj)), "application/json");
}
