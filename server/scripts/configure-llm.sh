#!/usr/bin/env bash
# Configure the TI-84 GPT server to use a custom OpenAI-compatible provider.
# Prompts for the API key, lists the provider's models, lets you pick one, then
# pushes OPENAI_BASE_URL + OPENAI_API_KEY + LLM_MODEL to the ti84-api Vercel
# project (production) and redeploys.
#
#   Usage:  bash scripts/configure-llm.sh
set -uo pipefail
cd "$(dirname "$0")/.."   # -> ti84/server (the linked Vercel project dir)

BASE_DEFAULT="https://vip.aipro.love/v1"
read -rp "Provider base URL [$BASE_DEFAULT]: " BASE
BASE="${BASE:-$BASE_DEFAULT}"
BASE="$(printf '%s' "$BASE" | tr -d '[:space:]')"

read -rsp "Paste API key for $BASE: " KEY; echo
KEY="$(printf '%s' "$KEY" | tr -d '[:space:]')"
if [ -z "$KEY" ]; then echo "No key entered. Aborting."; exit 1; fi

echo "Fetching models from $BASE/models ..."
MODELS="$(curl -fsS -H "Authorization: Bearer $KEY" "$BASE/models" 2>/dev/null \
  | node -e 'let s="";process.stdin.on("data",d=>s+=d).on("end",()=>{try{const j=JSON.parse(s);const ids=(j.data||j.models||[]).map(m=>m.id||m).filter(Boolean);process.stdout.write(ids.join("\n"))}catch(e){}})')"

if [ -n "$MODELS" ]; then
  echo "Available models:"
  echo "$MODELS" | nl -w3 -s'. '
  echo
else
  echo "(Could not list models automatically — type the model name your provider expects.)"
fi

read -rp "Model to use (LLM_MODEL): " MODEL
MODEL="$(printf '%s' "$MODEL" | tr -d '[:space:]')"
if [ -z "$MODEL" ]; then echo "No model entered. Aborting."; exit 1; fi

read -rp "Separate (stronger) model for the math solver? [blank = same as above]: " MODEL_MATH
MODEL_MATH="$(printf '%s' "$MODEL_MATH" | tr -d '[:space:]')"

echo "Pushing env vars to ti84-api (production)..."
printf '%s' "$BASE"  | npx vercel env add OPENAI_BASE_URL production --force >/dev/null 2>&1 && echo "  OPENAI_BASE_URL set"
printf '%s' "$KEY"   | npx vercel env add OPENAI_API_KEY  production --force >/dev/null 2>&1 && echo "  OPENAI_API_KEY set"
printf '%s' "$MODEL" | npx vercel env add LLM_MODEL       production --force >/dev/null 2>&1 && echo "  LLM_MODEL set ($MODEL)"
if [ -n "$MODEL_MATH" ]; then
  printf '%s' "$MODEL_MATH" | npx vercel env add LLM_MODEL_MATH production --force >/dev/null 2>&1 && echo "  LLM_MODEL_MATH set ($MODEL_MATH)"
fi

echo "Redeploying ti84-api to production..."
npx vercel deploy --prod --yes
echo
echo "Done. Tell Claude to verify /gpt/ask."
