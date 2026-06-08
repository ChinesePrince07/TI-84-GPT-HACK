# AP Physics C E&M FRQ Shortcuts — Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Add five new commands to ANDYGPT (DERIVE / TRANSLATE / LAB / JUSTIFY / SOLVER) under `PHYSICS > EM`, with the first four producing multi-line step-by-step output and the last a single-line numeric answer.

**Architecture:** ESP32 firmware (`esp32.ino`) gains five new HTTP-handler functions that wrap user input in topic-tuned prompts and POST to the existing `/gpt/ask?math=1` endpoint. TI-BASIC launcher (`LAUNCHER.8xp.txt`) gains a new `EM` submenu and a shared two-phase display routine (`D8` preprocess to build a step-index list, `D9` paginated render of variable-height steps). No server changes.

**Tech Stack:** ESP32 Arduino C++ (CBL2 / TIVar / HTTPClient libs), TI-BASIC, Python `tivars` library for .txt→tokens.

**Reference spec:** `docs/superpowers/specs/2026-05-14-em-frq-shortcuts-design.md`

---

## Task 1: Preflight — toolchain + portable compile script

**Files:**
- Modify: `compile_launcher.py:146-147` (hardcoded macOS paths)

- [ ] **Step 1: Install Python `tivars` library**

Run: `pip install tivars`
Expected: `Successfully installed tivars-…`. Verify with `python3 -c 'import tivars; from tivars.models import TI_84P; print("ok")'` → prints `ok`.

- [ ] **Step 2: Make `compile_launcher.py` use repo-relative paths**

The script currently has hardcoded `/Users/master/Desktop/...` paths. Change `main()` to use paths relative to the script's own location.

Replace lines 143-147 in `compile_launcher.py`:

```python
def main():
    import os

    source_file = '/Users/master/Desktop/GitHub/TI-84 GPT Hacks/programs/LAUNCHER.8xp.txt'
    output_header = '/Users/master/Desktop/GitHub/TI-84 GPT Hacks/esp32/launcher.h'
```

with:

```python
def main():
    import os

    repo_root = os.path.dirname(os.path.abspath(__file__))
    source_file = os.path.join(repo_root, 'programs', 'LAUNCHER.8xp.txt')
    output_header = os.path.join(repo_root, 'esp32', 'launcher.h')
```

- [ ] **Step 3: Dry-run the compile to confirm tooling works**

Run: `python3 compile_launcher.py`
Expected: stdout shows `Generated /home/andy/TI-84-GPT-HACK/esp32/launcher.h (NNNN bytes)`. The current `launcher.h` is overwritten with bytes matching the current `LAUNCHER.8xp.txt` (no functional change yet).

- [ ] **Step 4: Commit**

```bash
git add compile_launcher.py
git -c user.name='Pandizzler' -c user.email='94189480+ChinesePrince07@users.noreply.github.com' \
  commit -m "Make compile_launcher.py use repo-relative paths"
```

Skip the launcher.h regen commit — we'll regenerate at the end with all changes baked in.

---

## Task 2: ESP32 — wire up command table, MAXCOMMAND, version

**Files:**
- Modify: `esp32/esp32.ino:53` (firmware version)
- Modify: `esp32/esp32.ino:178-181` (forward declarations)
- Modify: `esp32/esp32.ino:228-232` (commands table + MAXCOMMAND)

- [ ] **Step 1: Add forward declarations for five new handlers**

In `esp32/esp32.ino`, immediately after line 179 (`void math_solver();`), insert:

```cpp
void em_derive();
void em_translate();
void em_lab();
void em_justify();
void em_solver();
```

- [ ] **Step 2: Add five entries to `commands[]`**

In `esp32/esp32.ino`, immediately after line 228 (`{ 36, "math_solver", 1, math_solver, true },`), insert:

```cpp
  { 37, "em_derive", 1, em_derive, true },
  { 38, "em_translate", 1, em_translate, true },
  { 39, "em_lab", 1, em_lab, true },
  { 40, "em_justify", 1, em_justify, true },
  { 41, "em_solver", 1, em_solver, true },
```

- [ ] **Step 3: Bump `MAXCOMMAND` 36 → 41**

In `esp32/esp32.ino` line 232, change:

```cpp
constexpr int MAXCOMMAND = 36;
```

to:

```cpp
constexpr int MAXCOMMAND = 41;
```

- [ ] **Step 4: Bump `FIRMWARE_VERSION` "1.5.0" → "1.6.0"**

In `esp32/esp32.ino` line 53, change:

```cpp
#define FIRMWARE_VERSION "1.5.0"
```

to:

```cpp
#define FIRMWARE_VERSION "1.6.0"
```

- [ ] **Step 5: Commit**

```bash
git add esp32/esp32.ino
git -c user.name='Pandizzler' -c user.email='94189480+ChinesePrince07@users.noreply.github.com' \
  commit -m "Wire ESP32 command table for EM FRQ shortcuts (37-41)"
```

---

## Task 3: ESP32 — implement five handler functions

**Files:**
- Modify: `esp32/esp32.ino` (append after `math_solver()` which ends near line 1231)

- [ ] **Step 1: Locate insertion point**

Open `esp32/esp32.ino` and find the end of `math_solver()` (look for the line containing `setSuccess(response);` near line 1230, then the next `}`). All five new handlers go immediately after the closing brace of `math_solver()`.

- [ ] **Step 2: Add `em_derive` handler**

```cpp
void em_derive() {
  const char* topic = strArgs[0];
  out.print("em_derive: ");
  out.println(topic);

  String prompt =
    "You derive AP Physics C E&M equations step-by-step. "
    "Start from a fundamental law (Gauss/Ampere/Faraday/Coulomb/Ohm/Kirchhoff). "
    "Each step on its own line, separated by '>'. "
    "Prefix each step with its number like '1)' '2)'. "
    "Each step <=48 chars. UPPERCASE only, no LaTeX. "
    "Use SQRT() for roots, ^ for powers, * for mult, / for div. "
    "NEVER write a literal '>' inside a step; use 'GT' instead. "
    "Max 10 steps. Problem: " + String(topic);
  auto url = String(SERVER) + String("/gpt/ask?math=1&question=") + urlEncode(prompt);

  size_t realsize = 0;
  if (makeRequest(url, response, MAXHTTPRESPONSELEN, &realsize)) {
    setError("REQUEST FAILED");
    return;
  }
  setSuccess(response);
}
```

- [ ] **Step 3: Add `em_translate` handler**

```cpp
void em_translate() {
  const char* problem = strArgs[0];
  out.print("em_translate: ");
  out.println(problem);

  String prompt =
    "Translate between physics representations (equation <-> graph <-> diagram <-> words) "
    "for AP Physics C E&M. Each step on its own line, separated by '>'. "
    "Prefix each step with its number like '1)' '2)'. Each step <=48 chars. "
    "UPPERCASE only. Identify axes, slopes, intercepts, and physical meaning. "
    "Use SQRT(), ^, *, / for math. "
    "NEVER write a literal '>' inside a step; use 'GT' instead. "
    "Max 10 steps. Problem: " + String(problem);
  auto url = String(SERVER) + String("/gpt/ask?math=1&question=") + urlEncode(prompt);

  size_t realsize = 0;
  if (makeRequest(url, response, MAXHTTPRESPONSELEN, &realsize)) {
    setError("REQUEST FAILED");
    return;
  }
  setSuccess(response);
}
```

- [ ] **Step 4: Add `em_lab` handler**

```cpp
void em_lab() {
  const char* problem = strArgs[0];
  out.print("em_lab: ");
  out.println(problem);

  String prompt =
    "AP Physics C E&M lab analysis. Design experiments or linearize equations to Y=MX+B form. "
    "Each step on its own line, separated by '>'. Prefix each step with its number. "
    "Each step <=48 chars. UPPERCASE only. "
    "State in order: what to measure, X axis, Y axis, slope meaning, intercept meaning. "
    "Use SQRT(), ^, *, /. "
    "NEVER write a literal '>' inside a step; use 'GT' instead. "
    "Max 10 steps. Problem: " + String(problem);
  auto url = String(SERVER) + String("/gpt/ask?math=1&question=") + urlEncode(prompt);

  size_t realsize = 0;
  if (makeRequest(url, response, MAXHTTPRESPONSELEN, &realsize)) {
    setError("REQUEST FAILED");
    return;
  }
  setSuccess(response);
}
```

- [ ] **Step 5: Add `em_justify` handler**

```cpp
void em_justify() {
  const char* problem = strArgs[0];
  out.print("em_justify: ");
  out.println(problem);

  String prompt =
    "Justify an AP Physics C E&M claim with quantitative reasoning. "
    "Each step on its own line, separated by '>'. Prefix each step with its number. "
    "Each step <=48 chars. UPPERCASE only. "
    "Format in order: claim, relevant equation(s), substitution/limit reasoning, conclusion. "
    "Use SQRT(), ^, *, /. "
    "NEVER write a literal '>' inside a step; use 'GT' instead. "
    "Max 10 steps. Problem: " + String(problem);
  auto url = String(SERVER) + String("/gpt/ask?math=1&question=") + urlEncode(prompt);

  size_t realsize = 0;
  if (makeRequest(url, response, MAXHTTPRESPONSELEN, &realsize)) {
    setError("REQUEST FAILED");
    return;
  }
  setSuccess(response);
}
```

- [ ] **Step 6: Add `em_solver` handler**

```cpp
void em_solver() {
  const char* problem = strArgs[0];
  out.print("em_solver: ");
  out.println(problem);

  String prompt =
    "Solve this AP Physics C E&M problem and return ONLY the final numeric answer with SI units "
    "(or a short closed-form symbolic answer if the inputs are symbolic). "
    "Use Coulomb's, Gauss's, Ohm's, Kirchhoff's, Biot-Savart, Ampere's, Faraday's, and Lenz's laws as appropriate. "
    "UPPERCASE only, no LaTeX, no steps. Use SQRT() for roots, ^ for powers. "
    "Problem: " + String(problem);
  auto url = String(SERVER) + String("/gpt/ask?math=1&question=") + urlEncode(prompt);

  size_t realsize = 0;
  if (makeRequest(url, response, MAXHTTPRESPONSELEN, &realsize)) {
    setError("REQUEST FAILED");
    return;
  }
  setSuccess(response);
}
```

- [ ] **Step 7: Verify the file still parses syntactically**

If `arduino-cli` is installed: `arduino-cli compile --fqbn esp32:esp32:esp32 esp32 2>&1 | head -40` — expected: no errors (or only the "no upload port" type warnings).

If `arduino-cli` is NOT installed (it isn't on this machine by default): do a quick visual review — every new function has matching braces, every prompt string is closed, every `setSuccess` / `setError` is present. The user will compile + flash through the Arduino IDE later in Task 7.

- [ ] **Step 8: Commit**

```bash
git add esp32/esp32.ino
git -c user.name='Pandizzler' -c user.email='94189480+ChinesePrince07@users.noreply.github.com' \
  commit -m "Add ESP32 handlers for EM FRQ commands"
```

---

## Task 4: Launcher — update PHYSICS menu and add EM submenu blocks

**Files:**
- Modify: `programs/LAUNCHER.8xp.txt:432` (PHYSICS menu — add EM entry)
- Modify: `programs/LAUNCHER.8xp.txt:~482` (after INELASTIC block — insert P3 + P4-P8)

- [ ] **Step 1: Add EM to the PHYSICS menu**

In `programs/LAUNCHER.8xp.txt`, find line 432:

```basic
Menu("PHYSICS","ELASTIC",P1,"INELASTIC",P2,"BACK",B0)
```

Replace with:

```basic
Menu("PHYSICS","ELASTIC",P1,"INELASTIC",P2,"EM",P3,"BACK",B0)
```

- [ ] **Step 2: Locate the end of the INELASTIC block (P2)**

In `programs/LAUNCHER.8xp.txt`, find the `Lbl P2:` block (around line 459). Scroll past it to the line `Goto P2` (around line 482). The new P3 submenu and its command blocks go immediately after that line and before `Lbl U0:` (around line 484).

- [ ] **Step 3: Insert P3 submenu and P4-P7 FRQ command blocks**

Insert this between the end of P2 (`Goto P2`) and the start of U0 (`Lbl U0:`):

```basic
Lbl P3:
Menu("EM","DERIVE",P4,"TRANSLATE",P5,"LAB",P6,"JUSTIFY",P7,"SOLVER",P8,"BACK",P0)

Lbl P4:
ClrHome
37->C
Disp "TOPIC:"
Input "",Str0
ClrHome
Output(4,4,"DERIVING...")
Send(C)
Send(Str0)
0->S
Repeat S
Get(S)
End
Get(Str0)
Goto D8

Lbl P5:
ClrHome
38->C
Disp "PROBLEM:"
Input "",Str0
ClrHome
Output(4,4,"TRANSLATING...")
Send(C)
Send(Str0)
0->S
Repeat S
Get(S)
End
Get(Str0)
Goto D8

Lbl P6:
ClrHome
39->C
Disp "PROBLEM:"
Input "",Str0
ClrHome
Output(4,4,"ANALYZING...")
Send(C)
Send(Str0)
0->S
Repeat S
Get(S)
End
Get(Str0)
Goto D8

Lbl P7:
ClrHome
40->C
Disp "CLAIM:"
Input "",Str0
ClrHome
Output(4,4,"JUSTIFYING...")
Send(C)
Send(Str0)
0->S
Repeat S
Get(S)
End
Get(Str0)
Goto D8
```

- [ ] **Step 4: Insert P8 (SOLVER) command block + P9 single-line pagination**

Immediately after the P7 block from Step 3, append:

```basic
Lbl P8:
ClrHome
41->C
Disp "PROBLEM:"
Input "",Str0
ClrHome
Output(4,4,"SOLVING...")
Send(C)
Send(Str0)
0->S
Repeat S
Get(S)
End
Get(Str0)
length(Str0)->L
0->P
Lbl P9:
ClrHome
96P+1->T
If T>L:Then
max(P-1,0)->P
96P+1->T
End
If L-T+1>96:Then
Output(1,1,sub(Str0,T,96))
Else
Output(1,1,sub(Str0,T,L-T+1))
End
Output(7,1,"L/R=PAGE")
Output(8,1,"CLEAR=EXIT")
0->K
Repeat max(K={24,26,45,105})
getKey->K
End
If K=45
Goto P3
If K=24:Then
max(P-1,0)->P
Goto P9
End
If K=26:Then
If 96(P+1)+1<=L
P+1->P
Goto P9
End
Goto P8
```

- [ ] **Step 5: Commit**

```bash
git add programs/LAUNCHER.8xp.txt
git -c user.name='Pandizzler' -c user.email='94189480+ChinesePrince07@users.noreply.github.com' \
  commit -m "Add EM submenu and command blocks to launcher"
```

---

## Task 5: Launcher — add D8 init + D9 multi-line display routines

**Files:**
- Modify: `programs/LAUNCHER.8xp.txt` (append at end of file)

**Design note:** The existing launcher source doesn't use TI-84 lists anywhere, and the `tivars` tokenizer's handling of bare `L1` (vs. the actual TI list token `ʟ1`) is not verified for this project. To avoid that uncertainty, D9 simulates from the beginning of `Str0` on every page render — no list storage required. With ≤10 steps total, the cost is negligible. The state across calls is a single integer `P` (current page).

- [ ] **Step 1: Append D8 (init) block**

At the end of `programs/LAUNCHER.8xp.txt`, append (with a leading blank line):

```basic

Lbl D8:
length(Str0)->L
0->P
```

D8 simply captures the string length and resets the page counter. The actual step-scanning happens inline in D9. After D8, fall through to D9.

- [ ] **Step 2: Append D9 (display) block**

Immediately after the D8 block, append:

```basic
Lbl D9:
ClrHome
1->I
0->X
1->R
Lbl D2:
If I>L:Then
X->P
Goto D3
End
inString(Str0,">",I)->W
If W=0:Then
L-I+1->Q
L+1->U
Else
W-I->Q
W+1->U
End
1->H
If Q>16:2->H
If Q>32:3->H
If R+H-1>7:Then
If X=P:Goto D3
X+1->X
1->R
End
If X=P:Then
Output(R,1,sub(Str0,I,min(Q,16)))
If H>=2:Output(R+1,1,sub(Str0,I+16,min(Q-16,16)))
If H>=3:Output(R+2,1,sub(Str0,I+32,Q-32))
End
R+H->R
U->I
Goto D2
Lbl D3:
Output(8,1,"L/R=PAGE  CLR=X")
0->K
Repeat max(K={24,26,45})
getKey->K
End
If K=45:Goto P3
If K=24:Then
If P>0:P-1->P
Goto D9
End
If K=26:Then
P+1->P
Goto D9
End
Goto D3
```

How it works:

- `I` = current cursor into `Str0`; `X` = the page currently being simulated; `R` = next display row; `P` = target page (state across renders); `H` = rows needed for current step.
- For each step, `inString(Str0, ">", I)` locates the next `>` from position `I` onward. If found, the step ends at `W-1`; if not, it ends at `L`. `U` is the position to advance to after this step (past the delimiter).
- `H` is chosen by step length: `Q≤16→1`, `17≤Q≤32→2`, `Q>32→3`. Steps with `Q>48` will visually truncate (rows 33-48 displayed; chars beyond 48 lost) — acceptable since the prompt enforces ≤48 chars.
- "Does step fit on this page?": if `R+H-1 > 7`, this step belongs on the next page. If we're currently rendering the target page (`X=P`), we're done — jump to D3. Otherwise, advance to the next simulated page (`X+1`, `R=1`) and continue.
- "Are we on the target page?": if `X=P`, output the step (wrapping at 16-char boundaries). Otherwise, just advance `R` and `I` without drawing — we're simulating past prior pages.
- End-of-string: if `I>L`, we've consumed all steps; clamp `P=X` (so the user can't navigate past the last page) and jump to D3.
- D3 handles keypress: L/R/CLEAR. Forward navigation always increments `P` and re-enters D9; the next render's end-of-string clamp self-corrects if the user went one past.

Variable scope:

- `P`, `K`, `L` reuse the existing launcher's semantics (page, keypress, length).
- `I`, `X`, `R`, `H`, `Q`, `U`, `W` are scratch — only valid during D9. Since no other label depends on them, no harm done.

- [ ] **Step 3: Commit**

```bash
git add programs/LAUNCHER.8xp.txt
git -c user.name='Pandizzler' -c user.email='94189480+ChinesePrince07@users.noreply.github.com' \
  commit -m "Add D8 init and D9 multi-line display routines"
```

---

## Task 6: Regenerate launcher.h

**Files:**
- Modify: `esp32/launcher.h` (auto-regenerated)

- [ ] **Step 1: Run the compile script**

Run: `python3 compile_launcher.py`
Expected: `Generated /home/andy/TI-84-GPT-HACK/esp32/launcher.h (NNNN bytes)` where NNNN is several hundred bytes larger than the previous size (the new menu + 5 command blocks + D8/D9 add roughly 600-900 bytes of tokens).

- [ ] **Step 2: Sanity-check the regenerated header**

Run: `head -3 esp32/launcher.h && wc -c esp32/launcher.h`
Expected: First three lines start with `// Auto-generated launcher program`, `// Program name: ANDYGPT`, and `unsigned int __launcher_var_len = NNNN;`. The byte count should be larger than before (compare against `git diff esp32/launcher.h` — should show many `+` lines of hex array contents).

- [ ] **Step 3: Commit**

```bash
git add esp32/launcher.h
git -c user.name='Pandizzler' -c user.email='94189480+ChinesePrince07@users.noreply.github.com' \
  commit -m "Regenerate launcher.h with EM FRQ commands"
```

---

## Task 7: Compile, deploy, hardware test

This task involves toolchains and hardware the agentic worker can't drive directly. Present the commands to the user and have them execute / confirm each step.

**Files:**
- Built artifact: `firmware.bin` (ESP32 compile output)
- Deployed: server's `firmware/firmware.bin` + `firmware/launcher.bin` + `firmware/version.txt`

- [ ] **Step 1: Compile the ESP32 firmware**

Open `esp32/esp32.ino` in the Arduino IDE (the project is already configured for it per the README). Select the ESP32 board, then **Sketch → Export Compiled Binary**. The `.bin` will land next to the sketch in `esp32/build/.../esp32.ino.bin` (path varies by IDE version).

Alternatively, if installing `arduino-cli`: `arduino-cli compile --fqbn esp32:esp32:esp32 esp32 --output-dir build/`.

Expected: a clean compile with the new MAXCOMMAND=41, FIRMWARE_VERSION="1.6.0", and the five new handler functions referenced by the commands table.

If you see "undefined reference to em_derive" or similar, the forward declarations from Task 2 Step 1 are missing.

- [ ] **Step 2: Upload firmware binary to server**

From the build directory:

```bash
curl -X POST 'https://api.andypandy.org/firmware/upload?version=1.6.0' \
  -H 'Content-Type: application/octet-stream' \
  --data-binary @esp32.ino.bin
```

Expected: response body `OK`.

- [ ] **Step 3: Upload launcher binary to server**

The server's `/firmware/upload_launcher` endpoint accepts the raw `.8xp` and strips the header itself. The `LAUNCHER.8xp` binary needs to be rebuilt from the updated `.txt` first — easiest path is to use the `tivars` library directly:

```bash
python3 -c "
from tivars import TIProgram
from tivars.models import TI_84P
with open('programs/LAUNCHER.8xp.txt') as f:
    code = f.read()
p = TIProgram(name='ANDYGPT')
p.load_string(code, model=TI_84P)
with open('programs/LAUNCHER.8xp', 'wb') as f:
    f.write(p.export())
print('LAUNCHER.8xp rebuilt')
"
```

Then upload:

```bash
curl -X POST 'https://api.andypandy.org/firmware/upload_launcher?version=1.6.0' \
  -H 'Content-Type: application/octet-stream' \
  --data-binary @programs/LAUNCHER.8xp
```

Expected: response body `OK`.

- [ ] **Step 4: Verify the server reports the new version**

Run: `curl https://api.andypandy.org/firmware/version`
Expected: `1.6.0`

- [ ] **Step 5: Trigger OTA from the calculator**

On the TI-84: launch `ANDYGPT` → `SETTINGS` → `UPDATE`. The calc displays `FLASHING FIRMWARE...` and the ESP32 reboots. After reboot, the new launcher is pushed to the calc automatically.

When the launcher arrives, verify by going to `SETTINGS` → `VERSION` — should show `CURRENT: 1.6.0` and `NEWEST: 1.6.0`.

- [ ] **Step 6: Smoke-test each new command**

From the main menu, navigate `PHYSICS → EM` and exercise each command using the test inputs from the spec:

| Command   | Input                                  | Expected behavior |
|-----------|----------------------------------------|-------------------|
| DERIVE    | `DISCHARGING CAPACITOR`                | 6-8 numbered rows showing V=IR → … → Q=Q0*E^(-T/RC); L/R paginates; CLEAR returns to EM menu |
| TRANSLATE | `RC CHARGING V VS T GRAPH TO EQUATION` | Multi-line response identifying V=V0(1-E^(-T/TAU)), axes meaning |
| LAB       | `LINEARIZE I=I0*E^(-T/TAU)`            | Multi-line with TAKE LN BOTH SIDES, Y=LN(I), X=T, M=-1/TAU, etc. |
| JUSTIFY   | `WHY DIELECTRIC INCREASES C`           | Multi-line claim + equation + reasoning + conclusion |
| SOLVER    | `FORCE BETWEEN 2 MICROCOULOMB AT 5 CM` | Single-line numeric answer, ~14.4 N |

Pass criteria:
- Each step appears on its own row (or wraps cleanly 2–3 rows)
- Step numbers (`1)`, `2)`, …) visible at the start of each step
- L/R paginates without crashes; CLEAR returns to EM submenu
- SOLVER answer within ~1% of expected

If a command misbehaves:
- **Wrong delimiter splits**: GPT wrote a `>` inside a step. Check the response in the serial monitor (`telnet <esp32-ip> 23`); refine the prompt if needed.
- **Steps too long, overflowing rows**: GPT exceeded 48 chars. Tighten the prompt to "≤40 chars per step" and reflash.
- **Calc freezes during display**: D8 or D9 may have a bug. Use the serial monitor to see the raw response, then re-trace through the D9 algorithm by hand.

- [ ] **Step 7: Final commit (if any post-test fixes)**

If the smoke tests pass, you're done — no further commit needed. If fixes were required, commit them:

```bash
git add -A
git -c user.name='Pandizzler' -c user.email='94189480+ChinesePrince07@users.noreply.github.com' \
  commit -m "Tune EM FRQ prompts after hardware test"
```

---

## Self-Review

**Spec coverage:**

| Spec section | Implementing task |
|---|---|
| Five new commands under PHYSICS > EM | Task 2 (table), Task 3 (handlers), Task 4 (menu) |
| Multi-line output protocol with `>` delimiter, numbered steps | Task 3 prompts, Task 5 D8/D9 |
| Calculator display routine D9 (two-phase preprocess + render) | Task 5 |
| SOLVER reuses single-line pagination | Task 4 Step 4 (P9 block) |
| ESP32 handler templates | Task 3 |
| Per-command prompt bodies | Task 3 Steps 2-6 (verbatim from spec) |
| Launcher changes (menu + blocks) | Task 4 |
| FIRMWARE_VERSION bump 1.5.0 → 1.6.0 | Task 2 Step 4 |
| MAXCOMMAND bump 36 → 41 | Task 2 Step 3 |
| Implementation order | Tasks 2 → 3 → 4 → 5 → 6 → 7 |
| Testing plan | Task 7 Step 6 |

No gaps.

**Type/name consistency check:**
- Handler names `em_derive`, `em_translate`, `em_lab`, `em_justify`, `em_solver` used identically across forward declarations (Task 2 Step 1), commands table (Task 2 Step 2), and definitions (Task 3 Steps 2-6).
- Command IDs `37/38/39/40/41` consistent across commands table (Task 2 Step 2), TI-BASIC `Send(C)` calls (Task 4 Step 3 & 4), and the `MAXCOMMAND=41` upper bound (Task 2 Step 3).
- Label names `P3-P9`, `D2`, `D3`, `D8`, `D9` are new; no collisions with existing launcher labels (P0/P1/P2/M0-M9/etc. checked).
- D8/D9 variables: `I` (cursor in Str0), `X` (currently-simulating page), `R` (display row), `H` (rows-needed), `Q` (step length), `U` (next step's start), `W` (delimiter position). Reused-existing: `P` (page, state across renders), `K` (keypress), `L` (Str0 length). All except `P` are scratch — no cross-call state.

**Placeholder scan:** No TBD/TODO/"appropriate error handling" instances. Every code step shows the exact code.

---

## Notes for the Executor

- The system has no automated test suite. "Verification" at each task boundary is either a compile check, a byte-count diff, or a hardware smoke test. Don't fabricate test scaffolding for this project.
- Git commits use a per-command author override (`git -c user.name=… -c user.email=…`) because global git config is not set on this machine. Do not run `git config --global …` to fix it.
- The user has an AP Physics C E&M quiz the day after the plan was written. Speed matters — execute straight through and don't introduce optional refactors.
