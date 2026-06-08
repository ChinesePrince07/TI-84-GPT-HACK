#!/usr/bin/env bash
# Configure the TI-84 GPT server's LLM provider (base URL + API key + model) on
# the ti84-api Vercel project, then redeploy.
#
# Sets env vars via the Vercel REST API: the `vercel env add` CLI (v54.x)
# silently stores EMPTY values when fed via stdin, so we POST directly using the
# token saved by `vercel login`.
#
#   Usage:  bash scripts/configure-llm.sh
set -uo pipefail
cd "$(dirname "$0")/.."   # -> ti84/server (the linked Vercel project dir)

AUTH="$HOME/.local/share/com.vercel.cli/auth.json"
if [ ! -f "$AUTH" ]; then echo "Run 'npx vercel login' first."; exit 1; fi
if [ ! -f .vercel/project.json ]; then echo "Link this dir: 'npx vercel link --project ti84-api'."; exit 1; fi

BASE_DEFAULT="https://vip.aipro.love/v1"
read -rp "Provider base URL [$BASE_DEFAULT]: " BASE
BASE="${BASE:-$BASE_DEFAULT}"; BASE="$(printf '%s' "$BASE" | tr -d '[:space:]')"

read -rsp "Paste API key: " KEY; echo
KEY="$(printf '%s' "$KEY" | tr -d '[:space:]')"
[ -z "$KEY" ] && { echo "No key. Aborting."; exit 1; }

echo "Fetching models from $BASE/models ..."
curl -fsS -H "Authorization: Bearer $KEY" "$BASE/models" 2>/dev/null \
  | node -e 'let s="";process.stdin.on("data",d=>s+=d).on("end",()=>{try{const j=JSON.parse(s);const ids=(j.data||j.models||[]).map(m=>m.id||m).filter(Boolean);console.log(ids.join("\n"))}catch(e){console.log("(could not list models — type the name your provider expects)")}})' \
  | nl -w3 -s'. '

read -rp "Model (LLM_MODEL): " MODEL; MODEL="$(printf '%s' "$MODEL" | tr -d '[:space:]')"
[ -z "$MODEL" ] && { echo "No model. Aborting."; exit 1; }
read -rp "Separate math model (LLM_MODEL_MATH) [blank = same as above]: " MMATH
MMATH="$(printf '%s' "$MMATH" | tr -d '[:space:]')"

echo "Setting env vars on ti84-api (production) via the Vercel API..."
BASE="$BASE" KEY="$KEY" MODEL="$MODEL" MMATH="$MMATH" node --input-type=module -e '
import fs from "fs";
const {token}=JSON.parse(fs.readFileSync(process.env.HOME+"/.local/share/com.vercel.cli/auth.json","utf8"));
const p=JSON.parse(fs.readFileSync(".vercel/project.json","utf8"));
async function setEnv(key,value){
  if(!value) return;
  const r=await fetch(`https://api.vercel.com/v10/projects/${p.projectId}/env?teamId=${p.orgId}&upsert=true`,{
    method:"POST",headers:{Authorization:`Bearer ${token}`,"Content-Type":"application/json"},
    body:JSON.stringify({key,value,type:"encrypted",target:["production"]})});
  console.log("  "+key+(r.ok?" set":" FAILED "+r.status));
}
await setEnv("OPENAI_BASE_URL",process.env.BASE);
await setEnv("OPENAI_API_KEY",process.env.KEY);
await setEnv("LLM_MODEL",process.env.MODEL);
await setEnv("LLM_MODEL_MATH",process.env.MMATH);
'
echo "Redeploying ti84-api to production..."
npx vercel deploy --prod --yes
echo
echo "Done. Verify with: curl 'https://api.andypandy.org/gpt/ask?question=HELLO'"
