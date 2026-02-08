# Presentation Script: Transistors & MOSFETs
## AP Physics C: Electricity & Magnetism

**Total time: ~18-22 minutes**

---

## SUBSCRIPT PRONUNCIATION GUIDE

When reading equations out loud, say the full names:

| Symbol | Say This | Meaning |
|--------|----------|---------|
| V_GS | "V gate-source" | Voltage from gate to source |
| V_th | "V threshold" | Threshold voltage (minimum to turn on) |
| V_DD | "V D D" or "V supply" | Power supply voltage |
| V_out | "V out" | Output voltage |
| R_DS | "R drain-source" or "R D S on" | MOSFET on-resistance |
| R_L | "R load" | Load resistor |
| R_G | "R gate" | Gate resistor |
| C_G | "C gate" | Gate capacitance |
| t_ox | "t oxide" or "t ox" | Oxide thickness |
| I_C | "I collector" | Collector current (BJT) |
| I_B | "I base" | Base current (BJT) |
| τ | "tau" | Time constant |
| β | "beta" | Current gain (BJT) |

---

## SLIDE 1: Title (30 seconds)

**SAY:**
"Today I'm going to talk about transistors — specifically MOSFETs — and show you how I used them in a real project I built. By the end, you'll understand the physics behind the most manufactured object in human history."

**BACKGROUND FOR YOU:**
- Transistors were invented in 1947 at Bell Labs
- About 10²² (10 sextillion) transistors are made every year
- That's more than any other manufactured object — more than screws, more than nails, more than anything

---

## SLIDE 2: What is a Transistor? (1.5 minutes)

**SAY:**
"So what is a transistor? At its core, a transistor is just an electrically controlled switch. Instead of physically flipping a switch with your finger, you use a small electrical signal to control whether current flows or not."

"Think about a light switch on your wall. You push it, metal contacts touch, and current flows to the light. A transistor does the same thing, but with no moving parts — it uses electric fields to control current flow."

"Why does this matter? Mechanical switches are slow — they take milliseconds to flip. They wear out over time. And they're huge compared to what we need for computers. Transistors switch in nanoseconds — that's a million times faster. They never wear out because nothing is physically moving. And we can make them nanometers in size — smaller than a virus."

**BACKGROUND FOR YOU:**
- A transistor is basically: "small signal controls big current"
- The input (gate/base) takes almost no power to control
- The output (drain-source / collector-emitter) can handle lots of power
- This is called "gain" or "amplification" — not of voltage, but of power/current capability
- Without transistors, computers would still use vacuum tubes and fill entire rooms

---

## SLIDE 3: Two Main Types (1.5 minutes)

**SAY:**
"There are two main types of transistors you'll encounter: BJTs and MOSFETs."

"BJT stands for Bipolar Junction Transistor. It's current-controlled — you need to push current into the base terminal to turn it on. The collector current equals beta times the base current, where beta is typically 100-300. BJTs are great for analog circuits and amplifiers."

"MOSFET stands for Metal-Oxide-Semiconductor Field-Effect Transistor. It's voltage-controlled — you just need to apply a voltage to the gate, and almost no current flows into the gate. When V gate-source — that's the voltage between the gate and source terminals — exceeds V threshold, the MOSFET turns on."

"Here's the key stat: over 99% of all transistors made today are MOSFETs. Every computer chip, every phone, every digital device uses MOSFETs. Why? I'll explain on the next slide."

**BACKGROUND FOR YOU:**
- **BJT (Bipolar Junction Transistor):**
  - Has three terminals: Base, Collector, Emitter
  - Current-controlled: I_C = β × I_B
  - β (beta) is the current gain, typically 100-300
  - You MUST supply continuous base current to keep it on
  - Used in: audio amplifiers, analog circuits, power supplies

- **MOSFET (Metal-Oxide-Semiconductor Field-Effect Transistor):**
  - Has three terminals: Gate, Drain, Source
  - Voltage-controlled: V_GS > V_th means ON
  - Gate is INSULATED — no DC current flows into it
  - Used in: CPUs, memory, digital logic, power switching

---

## SLIDE 4: Why MOSFETs Dominate (1.5 minutes)

**SAY:**
"The reason MOSFETs dominate comes down to one thing: the gate draws no DC current."

"In a BJT, the base needs continuous current to stay on. If beta is 100 and you want 100mA through your load, you need 1mA constantly flowing into the base. That's power being wasted as heat, even when the transistor is just sitting there in the ON state."

"In a MOSFET, the gate is separated from the channel by a thin insulating layer of silicon dioxide — glass, basically. No DC current can flow through an insulator. The only current the gate draws is during switching, when you're charging or discharging the tiny gate capacitance."

"This means the static power consumption of an ideal MOSFET is essentially zero. That's why your phone can have 10 billion transistors and run on a small battery without melting. If those were BJTs drawing even microamps each, the phone would catch fire."

**BACKGROUND FOR YOU:**
- **Why this matters for scaling:**
  - If each transistor needed even 1 microamp of base current...
  - 10 billion transistors × 1 μA = 10,000 Amps!
  - That's impossible to power or cool

- **MOSFET gate is a capacitor:**
  - Gate (metal) — Oxide (insulator) — Semiconductor
  - This forms a capacitor structure
  - Capacitors block DC current!
  - Only current during charging/discharging (switching)

- **Power equation:**
  - Static power ≈ 0 (ideal)
  - Dynamic power = ½CV²f (only when switching)
  - This is why clock speed and voltage matter for heat

---

## SLIDE 5: AP Problem Circuit (1 minute)

**SAY:**
"Now let's look at a real circuit and solve some AP-style problems. This is a basic MOSFET switching circuit."

"We have a 5 volt supply at the top — that's V D D. The load resistor R load is 1 kilohm — this could be an LED, a motor, whatever we're trying to control. The MOSFET M1 acts as our switch. When it's on, current flows through R load. When it's off, no current flows."

"On the left side, we have the gate drive circuit. The 10 kilohm resistor R gate and the 50 picofarad gate capacitance C gate form an RC circuit that determines how fast the MOSFET turns on."

"The MOSFET parameters: V threshold is 1.5 volts — that's the minimum gate voltage needed to turn it on. When on, it has a small drain-source resistance R D S of 5 ohms."

**BACKGROUND FOR YOU:**
- **What each component does:**
  - V_DD (5V): Power supply, provides energy
  - R_L (1kΩ): Load resistor, limits current, could represent any load
  - M1: The MOSFET switch we're controlling
  - R_G (10kΩ): Limits current into gate, sets charging speed
  - C_G (50pF): Gate capacitance (built into the MOSFET, not a separate part)

- **How it works:**
  - Gate LOW (0V): MOSFET is OFF, no current through R_L, V_out = 5V
  - Gate HIGH (5V): MOSFET is ON, current flows, V_out ≈ 0V
  - This is called an "inverter" — high input gives low output

---

## SLIDE 6: AP Problem Questions (30 seconds)

**SAY:**
"Here are the five parts of the problem. We'll solve each one. Part A asks about current and power when the MOSFET is on. Part B is the RC time constant. Part C asks when the MOSFET turns on. Part D is the electric field in the gate oxide. And Part E is about dynamic power consumption."

*[Move to next slide quickly — this is just showing the questions]*

---

## SLIDE 7: Solution (a) & (b) (2 minutes)

**SAY:**
"Part A: When the MOSFET is on, it acts like a 5 ohm resistor — that's R drain-source. So we have R drain-source plus R load in series: 5 ohms plus 1000 ohms, giving us 1005 ohms total. Using Ohm's law, current equals voltage over resistance: 5 volts divided by 1005 ohms gives us 4.98 milliamps."

"For power in the load resistor, we use P equals I squared R. That's 4.98 milliamps squared times 1000 ohms, which gives us 24.8 milliwatts."

"Part B: The time constant for an RC circuit is just tau equals R times C. That's R gate times C gate: 10 kilohms times 50 picofarads. Let me work through the units: 10 times 10 to the third ohms, times 50 times 10 to the negative 12 farads. That gives us 500 times 10 to the negative 9 seconds, or 500 nanoseconds."

**BACKGROUND FOR YOU:**
- **Part A step-by-step:**
  - When MOSFET is ON, it's like a small resistor (R_DS = 5Ω)
  - Total resistance = R_DS + R_L = 5 + 1000 = 1005Ω
  - Ohm's Law: I = V/R = 5V / 1005Ω = 0.00498 A = 4.98 mA
  - Power: P = I²R = (0.00498)² × 1000 = 0.0248 W = 24.8 mW
  - Note: Almost all power is in R_L because R_DS << R_L

- **Part B step-by-step:**
  - RC circuit time constant: τ = RC
  - R = 10 kΩ = 10 × 10³ Ω = 10,000 Ω
  - C = 50 pF = 50 × 10⁻¹² F = 0.00000000005 F
  - τ = 10,000 × 0.00000000005 = 0.0000005 s = 500 ns
  - This means it takes about 500ns to charge to 63% of final voltage

---

## SLIDE 8: Solution (c) & (d) (2 minutes)

**SAY:**
"Part C: This is where it gets interesting. The gate voltage doesn't jump instantly to 5 volts — it charges up like any RC circuit. The equation is V of t equals V final times the quantity 1 minus e to the negative t over tau."

"We need to find when V gate reaches V threshold, which is 1.5 volts. So: 1.5 equals 5 times 1 minus e to the negative t over tau. Dividing both sides by 5: 0.3 equals 1 minus e to the negative t over tau. Rearranging: e to the negative t over tau equals 0.7."

"Taking the natural log of both sides: negative t over tau equals ln of 0.7, which is negative 0.357. So t equals 0.357 times tau, which is 0.357 times 500 nanoseconds, giving us 178 nanoseconds."

"Part D: The electric field in the gate oxide is straightforward. For a parallel plate capacitor, E equals V over d. The voltage across the oxide is V gate-source, which is 5 volts. The oxide thickness, t oxide, is 50 nanometers. So E equals 5 divided by 50 times 10 to the negative 9, which gives us 10 to the 8 volts per meter — that's 100 million volts per meter!"

**BACKGROUND FOR YOU:**
- **Part C step-by-step (this is the hardest part):**
  - Capacitor charging equation: V(t) = V_final × (1 - e^(-t/τ))
  - We want V(t) = V_th = 1.5V, and V_final = 5V
  - 1.5 = 5 × (1 - e^(-t/τ))
  - 1.5/5 = 1 - e^(-t/τ)
  - 0.3 = 1 - e^(-t/τ)
  - e^(-t/τ) = 1 - 0.3 = 0.7
  - Take ln of both sides: -t/τ = ln(0.7) = -0.357
  - t = 0.357 × τ = 0.357 × 500 ns = 178 ns

  **Why this matters:** The MOSFET doesn't turn on instantly! It takes 178ns for the gate to charge up enough. This limits how fast you can switch.

- **Part D step-by-step:**
  - Electric field between parallel plates: E = V/d
  - V = V_GS = 5V (voltage across the oxide)
  - d = t_ox = 50 nm = 50 × 10⁻⁹ m
  - E = 5 / (50 × 10⁻⁹) = 5 / 0.00000005 = 100,000,000 V/m = 10⁸ V/m

  **Why this matters:** This is an ENORMOUS field! SiO₂ breaks down at about 10⁹ V/m, so we're at 10% of breakdown. This is why gate oxide thickness matters — too thin and it breaks down, too thick and you need more voltage.

---

## SLIDE 9: Solution (e) (1.5 minutes)

**SAY:**
"Part E: Every time the MOSFET switches, we have to charge and discharge the gate capacitor. The energy stored in a capacitor is one-half C V squared."

"Plugging in: one-half times 50 picofarads times 5 volts squared. That's one-half times 50 times 10 to the negative 12 times 25, which gives us 6.25 times 10 to the negative 10 joules per switching cycle."

"If we're switching at 1 megahertz — one million times per second — the power is energy times frequency. That's 6.25 times 10 to the negative 10 times 10 to the 6, giving us 0.625 milliwatts."

"This is called dynamic power consumption. Notice it scales with frequency and with voltage squared. This is exactly why modern processors use lower voltages — if you halve the voltage, you quarter the dynamic power."

**BACKGROUND FOR YOU:**
- **Part E step-by-step:**
  - Energy in a capacitor: E = ½CV²
  - C = 50 pF = 50 × 10⁻¹² F
  - V = 5V
  - E = ½ × (50 × 10⁻¹²) × (5)² = ½ × 50 × 10⁻¹² × 25 = 625 × 10⁻¹² J = 6.25 × 10⁻¹⁰ J

  - Power = Energy × frequency
  - f = 1 MHz = 10⁶ Hz
  - P = 6.25 × 10⁻¹⁰ × 10⁶ = 6.25 × 10⁻⁴ W = 0.625 mW

- **Why this matters for real chips:**
  - A CPU has ~10 billion transistors
  - If each used 0.625 mW at 1 MHz... that's 6.25 MW at 1 MHz!
  - But CPUs run at ~5 GHz (5000× faster)
  - Real CPUs use much smaller capacitances and lower voltages
  - Still, dynamic power is why your laptop gets hot under load

---

## SLIDE 10: MOSFET Structure (1.5 minutes)

**SAY:**
"Now let's look at what's actually inside a MOSFET. This cross-section shows an N-channel MOSFET."

"At the bottom, we have the P-type substrate — silicon doped with atoms that create 'holes,' which are positive charge carriers. On the left and right, we have N+ regions — heavily doped with atoms that provide extra electrons. These are the source and drain."

"On top, separated by the silicon dioxide insulator, is the metal gate. When we apply a positive voltage to the gate, the electric field pushes through the oxide and into the semiconductor below."

"That field repels the holes in the P-type material away from the surface. And it attracts electrons to the surface. When enough electrons accumulate, they form a thin conductive channel connecting source to drain. Current can now flow."

"The threshold voltage is the minimum gate voltage needed to form this channel. Below threshold, no channel, no current. Above threshold, channel forms, current flows."

**BACKGROUND FOR YOU:**
- **P-type vs N-type silicon:**
  - Pure silicon has 4 valence electrons, forms crystal lattice
  - P-type: Doped with boron (3 electrons) → missing electron = "hole" (acts positive)
  - N-type: Doped with phosphorus (5 electrons) → extra electron (acts negative)

- **MOSFET structure (N-channel enhancement mode):**
  - Substrate: P-type silicon (bulk of the device)
  - Source & Drain: N+ regions (heavily doped N-type)
  - Gate oxide: SiO₂, about 1-50 nm thick
  - Gate: Metal or polysilicon (conductor)

- **How channel forms:**
  1. Apply positive V_GS
  2. Electric field points from gate into substrate
  3. Field repels holes (positive) away from surface
  4. Field attracts electrons (negative) to surface
  5. Electrons accumulate at surface = conductive channel
  6. Channel connects N+ source to N+ drain
  7. Current can flow through the channel

- **Why it's called "Field Effect":**
  - The electric FIELD controls the conductivity
  - No physical contact between gate and channel
  - The gate is completely insulated!

---

## SLIDE 11: How MOSFETs Work (1 minute)

**SAY:**
"Let me summarize the physics. Step 1: Apply a voltage V gate-source to the gate. Step 2: This creates an electric field through the oxide, E equals V gate-source over the oxide thickness. Step 3: The field repels holes from the surface. Step 4: The field attracts electrons to the surface. Step 5: The electron layer becomes a conductive channel. Step 6: Current flows from drain to source."

"OFF state: V gate-source is less than V threshold. No channel, no current. The resistance is essentially infinite — like an open switch."

"ON state: V gate-source is greater than V threshold. Channel forms, current flows. The resistance drops to just a few ohms — like a closed switch."

"The fundamental rule: V gate-source greater than V threshold means ON. That's all you need to remember."

---

## SLIDE 12: Real-World Application (1 minute)

**SAY:**
"Now let me show you how I used this in a real project. I built a device that connects a TI-84 calculator to ChatGPT."

"The calculator can't connect to WiFi on its own. So I built a custom PCB with an ESP32 microcontroller that has WiFi. But there's a problem: the calculator and the ESP32 need to communicate, and that's where MOSFETs come in."

"The data flows like this: Calculator sends a question through its link port, the MOSFETs on my PCB transfer those signals to the ESP32, the ESP32 sends it to ChatGPT over WiFi, and the response comes back the same way."

**BACKGROUND FOR YOU:**
- **What the TI-84 link port is:**
  - 2.5mm audio jack on the calculator
  - Used for calculator-to-calculator data transfer
  - Two signal lines: TIP (data) and RING (clock)
  - Plus ground

- **What the ESP32 is:**
  - A microcontroller (tiny computer) with built-in WiFi
  - Runs my code that talks to ChatGPT's API
  - Can be programmed via USB

- **Why I need MOSFETs:**
  - The calculator and ESP32 need to exchange signals
  - But they use "open-drain" signaling (explained next slide)
  - MOSFETs provide the interface between them

---

## SLIDE 13: The Engineering Challenge (1 minute)

**SAY:**
"Here's the challenge I faced. The TI-84 link port uses something called open-drain signaling."

"In open-drain, a device can only pull a line LOW — it cannot push it HIGH. The line is held HIGH by a resistor, and devices pull it down to signal. This allows multiple devices to share the same wire without fighting each other."

"The link port has two lines: TIP carries the data bits, RING is the clock signal. And it's bidirectional — both the calculator and my device need to be able to pull lines low and read when the other pulls low."

"So I need a circuit that passes signals in both directions, automatically, using open-drain logic. The solution is an N-channel MOSFET level shifter."

**BACKGROUND FOR YOU:**
- **Open-drain signaling explained:**
  - Imagine two people sharing one rope
  - Either person can PULL the rope (make it tight/LOW)
  - Neither person can PUSH the rope (they just let go)
  - A spring (pull-up resistor) returns rope to rest position (HIGH)
  - If anyone pulls, the rope goes tight (LOW)
  - Both people can feel when the other pulls

- **Why open-drain is used:**
  - Multiple devices can share one wire safely
  - If two devices tried to push different voltages, you'd get a short circuit
  - With open-drain, worst case is two devices both pulling LOW (which is fine)
  - Used in: I²C bus, calculator link protocol, many other protocols

- **TI-84 link protocol basics:**
  - Synchronous serial communication
  - RING is the clock (timing signal)
  - TIP is the data (0s and 1s)
  - Both sides can transmit by pulling lines LOW
  - HIGH (idle) = 1, LOW (pulled) = 0

---

## SLIDE 14: Why Resistors Are Needed (1.5 minutes)

**SAY:**
"Before I show the MOSFET circuit, let's talk about those pull-up resistors. Why do we need them?"

"In open-drain signaling, nothing actively pushes the line HIGH. So we need a resistor connected to the power supply to 'pull' the line up to the HIGH voltage when no one is pulling it LOW."

"The resistors define the HIGH state. Without them, when nothing is pulling LOW, the line would 'float' at some random voltage — it could pick up noise and give false readings."

"They also limit current. When something pulls the line LOW, current flows through the resistor. With 3.3 volts and 1 kilohm, that's 3.3 milliamps max. This protects the circuit."

"And they set the speed. Remember RC time constants? The resistor and the wire capacitance form an RC circuit. Larger resistor means slower rise time. That's why I chose 1 kilohm — it's a good balance between speed and power consumption. With about 20 picofarads of wire capacitance, the time constant is only 20 nanoseconds."

"If the resistor was too small, like 100 ohms, we'd waste power — 33 milliamps every time the line goes LOW. If it was too big, like 100 kilohms, the rise time would be 2 microseconds, way too slow for our data rate."

**BACKGROUND FOR YOU:**
- **Pull-up resistor function:**
  1. Defines HIGH voltage when line is idle
  2. Limits current when line is pulled LOW (I = V/R)
  3. Determines rise time (τ = RC, where C is parasitic capacitance)

- **Why 1kΩ specifically:**
  - Current when LOW: I = 3.3V / 1000Ω = 3.3 mA (acceptable)
  - Rise time: τ = 1kΩ × 20pF = 20 ns (fast enough for ~9600 baud)
  - Power when LOW: P = V²/R = 3.3² / 1000 = 10.9 mW (acceptable)

- **What happens without pull-ups:**
  - Line "floats" at undefined voltage
  - Can pick up electromagnetic interference (noise)
  - Random bits, communication errors
  - Circuit won't work reliably

---

## SLIDE 15: MOSFET Level Shifter (2 minutes)

**SAY:**
"Here's the circuit. At the top we have 3.3 volts powering both sides. Each side has a 1 kilohm pull-up resistor. In the middle is an N-channel MOSFET, with its gate tied directly to 3.3 volts."

"Let me walk through how this works. When both sides are idle, both lines are pulled HIGH to 3.3 volts by their resistors. The gate is at 3.3 volts. The source — which is the lower-voltage side of the MOSFET — is also at 3.3 volts. So V gate-source equals 3.3 minus 3.3, which is zero. That's below the threshold voltage, so the MOSFET is OFF. The two sides are isolated."

"Now let's say the TI-84 pulls its side LOW to 0 volts. The source of the MOSFET is now at 0 volts, while the gate is still at 3.3 volts. V gate-source is now 3.3 minus 0, which equals 3.3 volts. That's above threshold, so the MOSFET turns ON."

"When the MOSFET turns on, it connects the two sides. Current flows from the ESP32 side through the MOSFET to ground. The ESP32 side gets pulled LOW too. The ESP32 sees the LOW signal — data received!"

"The beautiful thing is this works in both directions. If the ESP32 pulls its side LOW, the same physics happens — V gate-source increases, MOSFET turns on, calculator side goes LOW."

**BACKGROUND FOR YOU:**
- **Key insight: The source follows the lower voltage**
  - In this circuit, the "source" is whichever side has lower voltage
  - MOSFETs are symmetric — current can flow either direction
  - The body diode also helps with bidirectional conduction

- **State-by-state analysis:**

  **State 1: Both HIGH (idle)**
  - ESP32 side: 3.3V (pulled up by resistor)
  - TI-84 side: 3.3V (pulled up by resistor)
  - Gate: 3.3V
  - Source (lower side): 3.3V
  - V_GS = 3.3 - 3.3 = 0V < V_th
  - MOSFET: OFF
  - Result: Sides isolated, both read HIGH

  **State 2: TI-84 pulls LOW**
  - TI-84 side: 0V (calculator pulling down)
  - Gate: 3.3V
  - Source: 0V (TI-84 side is lower)
  - V_GS = 3.3 - 0 = 3.3V > V_th
  - MOSFET: ON
  - ESP32 side: Pulled to ~0V through MOSFET
  - Result: Both sides LOW, ESP32 sees the signal

  **State 3: ESP32 pulls LOW**
  - ESP32 side: 0V (ESP32 pulling down)
  - Gate: 3.3V
  - Source: 0V (ESP32 side is lower)
  - V_GS = 3.3 - 0 = 3.3V > V_th
  - MOSFET: ON
  - TI-84 side: Pulled to ~0V through MOSFET
  - Result: Both sides LOW, calculator sees the signal

---

## SLIDE 16: Why the MOSFET is Crucial (1 minute)

**SAY:**
"Why couldn't I just use a wire to connect the calculator to the ESP32?"

"First, a wire doesn't give you automatic bidirectional level shifting. If I needed different voltages on each side, a wire wouldn't work. With MOSFETs, each side can have its own pull-up voltage."

"Second, the MOSFET provides automatic switching. I don't need extra logic to decide when to connect the sides — the physics of V_GS does it automatically."

"Third, when both sides are HIGH, they're isolated. This prevents any weird interactions when the line is idle."

"And fourth, the gate draws no DC current. The only power consumed is through the pull-up resistors. If I used a BJT-based solution, I'd need more components and waste more power."

"The MOSFET's field-effect switching — controlled purely by voltage, drawing no gate current — is exactly what makes this elegant, simple, and efficient."

---

## SLIDE 17: My Custom PCB (1 minute)

**SAY:**
"Here's the PCB I designed and built. You can see the components clearly."

"The four brown cylinders are the 1 kilohm pull-up resistors. There are four because each signal line — TIP and RING — needs pull-ups on both sides of the MOSFET."

"The two small green components are BSS138 N-channel MOSFETs. That's one for TIP and one for RING."

"The module on the right is a Seeed Studio XIAO ESP32-S3. It has WiFi, a USB-C port for power and programming, and plenty of GPIO pins for our signals."

"On the left side, you can see the solder pads labeled 5V, GND, TIP, and RING. That's where I connect wires that go to the calculator's link port."

"The silkscreen says TI-84 HACKS, ANDYPANDY, 2025 — my project branding."

---

## SLIDE 18: Design Choices (1 minute)

**SAY:**
"Let me explain why I chose each component."

"BSS138 MOSFETs because they have a low threshold voltage around 1.5 volts, which works great with 3.3 volt logic. They're also cheap and commonly used for level shifting."

"1 kilohm pull-ups give a good balance — fast enough rise time around 20 nanoseconds, but low enough current draw around 3 milliamps max."

"Four resistors total because each signal needs pull-ups on both sides of the MOSFET. That's two for TIP and two for RING."

"The XIAO ESP32 because it's tiny, has built-in USB-C, built-in WiFi, and enough GPIO pins for what I need."

"And simple solder pads instead of a connector because it lets me wire directly to the calculator's link port. More reliable than a flimsy audio connector."

---

## SLIDE 19: Complete System (1 minute)

**SAY:**
"Here's how the complete system works, end to end."

"Step 1: The user types a question on the TI-84 calculator using a BASIC program I wrote."

"Step 2: The calculator sends the text as bytes through its link port using open-drain signaling on the TIP and RING lines."

"Step 3: The MOSFET level shifters transfer those signals to the ESP32. Every time the calculator pulls a line LOW, the MOSFET turns on and the ESP32 sees it."

"Step 4: The ESP32, running my firmware, receives the bytes, assembles the question, and sends it to OpenAI's ChatGPT API over WiFi."

"Step 5: ChatGPT processes the question and sends back a response."

"Step 6: The ESP32 sends the response bytes back through the MOSFETs to the calculator, which displays the answer on screen."

"All of this happens in a few seconds. You type a math question, and ChatGPT's answer appears on your calculator."

---

## SLIDE 20: Conclusion (1 minute)

**SAY:**
"So what did we learn? The physics you study in AP E&M directly enables real technology."

"Electric fields: E equals V over d. This determines how strong the field is in the MOSFET's gate oxide."

"The threshold condition: V gate-source greater than V threshold. This is the fundamental switching rule that makes digital electronics possible."

"RC time constants: tau equals RC. This determines how fast circuits can switch."

"And with just these concepts — electric fields, capacitors, and RC circuits — I built something that connects a graphing calculator to artificial intelligence."

"Two MOSFETs. Four resistors. That's all it took."

"Any questions?"

---

## POTENTIAL QUESTIONS AND ANSWERS

**Q: Why not just use WiFi directly in the calculator?**
A: Texas Instruments doesn't put WiFi in their calculators. The TI-84 is basically 1990s technology (Z80 processor). The link port is the only way to get data in and out.

**Q: How fast is the data transfer?**
A: About 9600 bits per second. That's slow by modern standards, but plenty fast for text.

**Q: Could this work with other calculators?**
A: Yes! Any TI calculator with a 2.5mm link port (TI-83, TI-84, TI-89, etc.) uses the same protocol.

**Q: What's the threshold voltage of the BSS138?**
A: About 1.5V typically, with a max of 2.0V guaranteed by the datasheet.

**Q: Why is the electric field in the oxide so high?**
A: Because the oxide is SO thin (nanometers). Even a small voltage creates a huge field when d is tiny. E = V/d, and d is very small.

**Q: How much power does the whole thing use?**
A: The ESP32 uses about 100-200mA when WiFi is active. The MOSFET circuit itself uses almost nothing — just the pull-up current when lines are LOW.

**Q: Is this cheating?**
A: *[Pause for laughter]* I built it for the engineering challenge, not for tests. It's actually harder to use this than to just learn the math!

---

## SUMMARY OF KEY PHYSICS CONCEPTS

1. **Electric Field in Parallel Plates:** E = V/d
2. **Capacitor Charging:** V(t) = V₀(1 - e^(-t/τ))
3. **RC Time Constant:** τ = RC
4. **Energy in Capacitor:** E = ½CV²
5. **Power:** P = E × f = ½CV²f
6. **Ohm's Law:** V = IR, I = V/R, P = I²R = V²/R
7. **MOSFET Switching Condition:** V_GS > V_th → ON
