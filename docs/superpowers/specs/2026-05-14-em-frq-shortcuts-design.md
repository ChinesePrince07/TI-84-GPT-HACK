# AP Physics C E&M FRQ Shortcuts — Design

**Date:** 2026-05-14
**Author:** Andy
**Status:** Draft, awaiting review

## Motivation

Andy has an AP Physics C E&M quiz on 2026-05-15. The existing launcher has `MATH > SOLVER` (generic) and `PHYSICS > ELASTIC/INELASTIC` (mechanics only); there is no E&M-specific support. AP grades E&M FRQs along four skill types:

1. **Mathematical Routines** — calculus-based derivations (e.g., E from Gauss's Law, B from Ampere's Law)
2. **Translation Between Representations** — connecting diagrams, equations, graphs
3. **Experimental Design and Analysis** — designing labs, sketching apparatus, linearizing data to `Y=MX+B`
4. **Qualitative/Quantitative Translation** — justifying a physical claim with mathematical reasoning

The new commands map 1:1 to these skill types, plus a catch-all numeric solver for multiple-choice plug-and-chug.

## Scope

In scope:

- Five new commands under `PHYSICS > EM`
- Multi-line "one step per row" output for the four FRQ-type commands
- E&M-tuned system prompts (sent via existing `/gpt/ask?math=1`)
- ESP32 firmware bump and launcher recompile

Out of scope:

- Hardware changes
- New server endpoints (reuses `/gpt/ask?math=1`)
- Generalizing to other AP subjects (mechanics, chem, etc.)

## Menu Structure

```
Main → PHYSICS → ELASTIC, INELASTIC, EM, BACK
                          ↓
             EM → DERIVE, TRANSLATE, LAB, JUSTIFY, SOLVER, BACK
```

## Commands

| ID | Menu label | C handler     | FRQ type / purpose                      | Output type    |
|----|------------|---------------|-----------------------------------------|----------------|
| 37 | DERIVE     | `em_derive`   | Mathematical Routines                   | Multi-line steps |
| 38 | TRANSLATE  | `em_translate`| Translation Between Representations     | Multi-line steps |
| 39 | LAB        | `em_lab`      | Experimental Design and Analysis        | Multi-line steps |
| 40 | JUSTIFY    | `em_justify`  | Qualitative/Quantitative Translation    | Multi-line steps |
| 41 | SOLVER     | `em_solver`   | Catch-all numeric MC plug-and-chug      | Single-line answer |

`MAXCOMMAND` in `esp32/esp32.ino` rises from 36 to 41.

The C handler names are prefixed `em_` because:

- `translate()` already exists for the language utility (ID 23)
- `solve()` already exists for the vision endpoint (ID 8)

Menu labels remain unprefixed; the conflict is internal to the firmware only.

## Multi-Line Output Protocol (commands 37–40)

GPT returns a single string where:

- Steps are separated by a literal `>` character
- Each step is prefixed with its step number followed by `)`, e.g. `1) V=IR`, `2) I=-DQ/DT`
- Each step is ≤ 48 characters (so it fits in at most three 16-char rows)
- Max 10 steps per response

Example DERIVE response for "DISCHARGING CAPACITOR":

```
1) V=IR>2) I=-DQ/DT>3) V=Q/C>4) Q/C=R*DQ/DT>5) DQ/Q=-DT/(RC)>6) LN(Q/Q0)=-T/(RC)>7) Q=Q0*E^(-T/RC)>8) TAU=RC
```

Why `>`:

- Single character (cheap to scan with `inString`)
- Already in `asciiToTIToken` (maps to TI token `0x6C`, displays as `>`)
- Rarely appears within physics derivations; prompt explicitly bans it inside a step and instructs GPT to write "GT" if needed

### Calculator display routine (shared label `D9`)

The pagination is tricky because steps have variable row heights (1, 2, or 3 rows), so "page N starts at step N×7" does not work — a page packed with 3-row steps fits only 2 of them. The routine handles this in two phases:

**Phase 1 (preprocess, runs once after `Get(Str0)`):**

Build list `L1` of step start positions by scanning `Str0` for `>` once. After this, `dim(L1)` equals the total number of steps and `L1(K)` is the 1-indexed position in `Str0` where step `K` begins.

**Phase 2 (display, runs on every page render):**

Maintain list `L2` where `L2(P+1)` is the index of the first step on page `P` (0-indexed page). `L2(1) = 1` always. On the first render of page `P`, walk steps from `L2(P+1)` accumulating row heights until the next step would push past row 7; record the next step's index into `L2(P+2)` so the next-page jump is O(1).

Pseudocode:

```
Lbl D9:
ClrHome
L2(P+1) -> N                ; first step index on this page
1 -> R                      ; current display row (1..7)

Lbl D10:
If N > dim(L1): Goto D11    ; no more steps

; compute length of step N (exclude trailing '>')
If N < dim(L1)
  L1(N+1) - L1(N) - 1 -> Q
Else
  length(Str0) - L1(N) + 1 -> Q

; rows needed
1 -> H
If Q > 16: 2 -> H
If Q > 32: 3 -> H

; does this step fit on the current page?
If R + H - 1 > 7: Goto D11

; emit the step, wrapping at 16-char boundaries
Output(R, 1, sub(Str0, L1(N), min(Q, 16)))
If H >= 2: Output(R+1, 1, sub(Str0, L1(N)+16, min(Q-16, 16)))
If H >= 3: Output(R+2, 1, sub(Str0, L1(N)+32, Q-32))

R + H -> R
N + 1 -> N
Goto D10

Lbl D11:
; record start of next page (only on first visit of this page)
If P+2 > dim(L2): N -> L2(P+2)

Output(8, 1, "L/R=PAGE  CLR=X")
0 -> K
Repeat max(K={24,26,45})
  getKey -> K
End
If K = 45: Goto P3                       ; CLEAR → EM submenu
If K = 24: Then                          ; ← previous page
  If P > 0: P - 1 -> P
  Goto D9
End
If K = 26: Then                          ; → next page
  If N <= dim(L1): P + 1 -> P            ; only advance if more steps remain
  Goto D9
End
Goto D11                                 ; ignore other keys
```

`P` is the 0-indexed page, initialized to 0 by the caller. The shared routine always returns to `P3` (EM submenu) on CLEAR; no per-caller return label is needed because every multi-line command lives under that submenu.

Single-letter variables used by this routine: `N` (step index), `R` (row), `Q` (step length), `H` (rows-needed), `P` (page), `K` (keypress). The caller must also reset `dim(L1) = 0` and `dim(L2) = 1, L2(1) = 1` before jumping in.

## SOLVER Output (command 41)

The catch-all `EM SOLVER` returns a single-line numeric or short symbolic answer. It uses the existing 96-char paginated single-line wrapper from `MATH > SOLVER` (label `M8` in the current launcher), not the new `D9` routine. Reason: nothing to step through; just a number with units.

## ESP32 Handlers

All five handlers follow the existing math-handler template (e.g., `derivative()` in `esp32.ino`). Sketch for `em_derive`:

```cpp
void em_derive() {
  const char* topic = strArgs[0];
  String prompt = "You derive AP Physics C E&M equations step-by-step. "
    "Start from a fundamental law (Gauss/Ampere/Faraday/Coulomb/Ohm/Kirchhoff). "
    "Each step on its own line, separated by '>'. Prefix each step with its "
    "number like '1)' '2)'. Each step <=48 chars. UPPERCASE only, no LaTeX. "
    "Use SQRT() for roots, ^ for powers, * for mult, / for div. "
    "NEVER write a literal '>' inside a step; use 'GT' instead. "
    "Max 10 steps. Problem: " + String(topic);
  auto url = String(SERVER) + "/gpt/ask?math=1&question=" + urlEncode(prompt);
  size_t realsize = 0;
  if (makeRequest(url, response, MAXHTTPRESPONSELEN, &realsize)) {
    setError("REQUEST FAILED");
    return;
  }
  setSuccess(response);
}
```

Same shape for `em_translate`, `em_lab`, `em_justify`, `em_solver` — only the prompt body changes.

### Per-command prompt bodies

**em_derive:**

> You derive AP Physics C E&M equations step-by-step. Start from a fundamental law (Gauss/Ampere/Faraday/Coulomb/Ohm/Kirchhoff). Each step on its own line, separated by `>`. Prefix each step with its number like `1)` `2)`. Each step ≤48 chars. UPPERCASE only, no LaTeX. Use SQRT() for roots, `^` for powers, `*` for mult, `/` for div. NEVER write a literal `>` inside a step; use `GT` instead. Max 10 steps. Problem: `<input>`

**em_translate:**

> Translate between physics representations (equation ↔ graph ↔ diagram ↔ words) for AP Physics C E&M. Each step on its own line, separated by `>`. Prefix each step with its number like `1)` `2)`. Each step ≤48 chars. UPPERCASE only. Identify axes, slopes, intercepts, and physical meaning. Use SQRT(), `^`, `*`, `/` for math. NEVER write a literal `>` inside a step; use `GT` instead. Max 10 steps. Problem: `<input>`

**em_lab:**

> AP Physics C E&M lab analysis. Design experiments or linearize equations to `Y=MX+B` form. Each step on its own line, separated by `>`. Prefix each step with its number. Each step ≤48 chars. UPPERCASE only. State (in order): what to measure, X axis, Y axis, slope meaning, intercept meaning. Use SQRT(), `^`, `*`, `/`. NEVER write a literal `>` inside a step; use `GT` instead. Max 10 steps. Problem: `<input>`

**em_justify:**

> Justify an AP Physics C E&M claim with quantitative reasoning. Each step on its own line, separated by `>`. Prefix each step with its number. Each step ≤48 chars. UPPERCASE only. Format (in order): claim, relevant equation(s), substitution/limit reasoning, conclusion. Use SQRT(), `^`, `*`, `/`. NEVER write a literal `>` inside a step; use `GT` instead. Max 10 steps. Problem: `<input>`

**em_solver:**

> Solve this AP Physics C E&M problem and return ONLY the final numeric answer with SI units (or a short closed-form symbolic answer if the inputs are symbolic). Use Coulomb's, Gauss's, Ohm's, Kirchhoff's, Biot-Savart, Ampere's, Faraday's, and Lenz's laws as appropriate. UPPERCASE only, no LaTeX, no steps. Use SQRT() for roots, `^` for powers. Problem: `<input>`

## Launcher Changes (`programs/LAUNCHER.8xp.txt`)

New menu in PHYSICS:

```basic
Lbl P0:
Menu("PHYSICS","ELASTIC",P1,"INELASTIC",P2,"EM",P3,"BACK",B0)

Lbl P3:
Menu("EM","DERIVE",P4,"TRANSLATE",P5,"LAB",P6,"JUSTIFY",P7,"SOLVER",P8,"BACK",P0)
```

Each FRQ command block (P4–P7) looks like (DERIVE shown; others identical bar command ID and "DERIVING..." text):

```basic
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
prgmEMPREP            ; builds L1 from Str0, resets L2 to {1}, sets P=0
Goto D9
```

`prgmEMPREP` is a small sub-program (or an inline block) that:

1. Scans `Str0` once with `inString` to build `L1` of step start positions
2. Initializes `1 -> dim(L2) : 1 -> L2(1)`
3. Sets `0 -> P`

It is inlined as a labeled section (e.g., `Lbl D8`) in the same launcher rather than a separate `.8xp` program to avoid the multi-file dependency.

P8 (SOLVER) follows the existing single-line pagination pattern from `M8` (math solver), with command ID 41 and a "SOLVING..." progress message.

Shared D9 display routine: see the pseudocode above. CLEAR returns to `P3` (EM submenu).

## Implementation Order

1. Update `esp32/esp32.ino`:
   - Add 5 forward declarations
   - Add 5 entries to `commands[]`
   - Bump `MAXCOMMAND` 36 → 41
   - Bump `FIRMWARE_VERSION` `"1.5.0"` → `"1.6.0"`
   - Implement 5 handler functions
2. Update `programs/LAUNCHER.8xp.txt`:
   - Add `EM` entry to PHYSICS menu (P0)
   - Add `P3` submenu
   - Add `P4`–`P8` command blocks
   - Add shared `D9` multi-line display routine
3. Run `compile_launcher.py` to regenerate `esp32/launcher.h`
4. Build + flash ESP32 (or use OTA after pushing new firmware to the server)
5. Push new launcher binary to the server for delivery to the calc
6. Test each command on the calc

## Testing Plan

For each command, try these inputs and compare to known AP answers:

- **DERIVE:** `DISCHARGING CAPACITOR`, `E FIELD FROM INFINITE SHEET`, `B FIELD INSIDE SOLENOID`
- **TRANSLATE:** `RC CHARGING V VS T GRAPH TO EQUATION`, `GAUSSIAN SURFACE FOR INFINITE WIRE`
- **LAB:** `LINEARIZE I=I0*E^(-T/TAU)`, `EXPERIMENT TO FIND CAPACITANCE FROM RC`
- **JUSTIFY:** `WHY DIELECTRIC INCREASES C`, `WHY LENZ LAW DIRECTION`
- **SOLVER:** `FORCE BETWEEN 2 MICROCOULOMB AT 5 CM`, `EQUIV R OF 3 RESISTORS IN PARALLEL 10 OHM EACH`

Pass criteria:

- Multi-line output displays one step per row (or wraps cleanly 2–3 rows for long steps)
- Step number prefix visible at the start of each step
- L/R pagination works; CLEAR returns to EM submenu
- SOLVER returns a number within ~1% of the expected answer

## Risks / Open Questions

| Risk | Mitigation |
|------|------------|
| GPT exceeds 48 chars per step despite the prompt | Prompt says "if too long, split into 2 sub-steps." Display truncates cleanly at 48 either way. |
| GPT forgets the `N)` numbering | Degraded behavior: still one step per row, just unnumbered. Acceptable. |
| GPT writes a literal `>` inside a step | Prompt explicitly bans it; says use `GT`. If it happens anyway, the parser splits incorrectly and the displayed step boundaries shift — visually obvious but not catastrophic. |
| TI-BASIC `inString` is slow on long strings | The preprocess (`D8`) scans `Str0` once to build `L1`, then `D9` never calls `inString` again — pagination reads `L1` directly. Worst case is ~10 calls over a ~500-char string at receive time, not per page render. |
| Variable name collisions with existing launcher routines | New routine uses `N` (step idx), `R` (row), `Q` (step len), `H` (rows-needed), `P` (page), `K` (keypress), plus lists `L1` and `L2`. Of these, `P` and `K` are already used elsewhere with the same semantics; `N` is also used by the wifi-scan handler but only locally, no cross-call dependency. Lists `L1`/`L2` aren't used elsewhere in the launcher. |

## Out-of-scope follow-ups

- Mechanics versions of DERIVE/TRANSLATE/LAB/JUSTIFY (would generalize the EM submenu into PHYSICS > FRQ)
- Server-side enforcement of the `>` step-separator format (currently relying on GPT compliance)
- A history / favorites view for previously-asked FRQ questions
