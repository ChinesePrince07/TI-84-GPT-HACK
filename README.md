# TI-84 GPT HACK

A hardware mod that gives your TI-84 calculator internet access, ChatGPT integration, and more.

## Demo

<video src="demo.mp4" controls width="600"></video>

![built pcb](./pcb/built.png)

## About

This project turns your dusty graphing calculator into a WiFi-enabled device capable of querying ChatGPT and downloading programs over the air.

The original concept came from ChromaLock, who made a [video about it](https://www.youtube.com/watch?v=Bicjxl4EcJg) back in 2024. His repo got nuked, so I rebuilt the whole thing from scratch based on what I could piece together. Most of the code, PCB design, and implementation is my own work at this point.

**Author:** Andy (ChinesePrince07)

## Features

- **ChatGPT integration** - ask questions directly from your calculator, with persistent chat sessions (`HISTORY` recalls your last conversation, `NEW CHAT` starts fresh)
- **AP Physics C E&M cheat-sheet** - a deep menu of static derivations, equations, ASCII graphs, right-hand-rule mnemonics, conceptual behavior, and physics laws — all served locally from firmware, no network needed
- **Physics SOLVER** - GPT-backed numeric problem solver for circuits, fields, energy, and custom plug-and-chug
- **Multi-page output** - long GPT/solver responses paginate with ◄/► keys
- Wi-Fi connectivity via ESP32 with **captive portal configuration**
- **WiFi scanner** - scan for nearby networks and connect directly from the calculator
- **WPA2-Enterprise (eduroam)** - connect to university/enterprise WiFi networks via the captive portal
- **One-tap Suffield** - quick-connect shortcut to Suffield Academy WiFi from the SETTINGS menu
- **MAC address spoofing** - view and change the ESP32's MAC address
- **IP viewer** - see the ESP32's current IP for the remote serial monitor (`telnet <ip> 23`)
- **Smart case input** - letters default to lowercase; wrap in parentheses for uppercase, e.g. `A(B)CD` becomes `aBcd`
- Image display support (96x63 monochrome)
- **OTA updates** - update both ESP32 firmware and calculator program wirelessly
- **Deep sleep** - ESP32 sleeps after 2 minutes of inactivity to save battery, wakes instantly when the calculator sends a command
- **Pre-configured server** - no need to run your own!

## How It Works

The TI-84 has a 2.5mm link port originally designed for calculator-to-calculator communication using Texas Instruments' proprietary link protocol. This mod exploits that by wiring an ESP32 microcontroller to impersonate another calculator.

When your calculator sends a variable (like a command number), the ESP32 intercepts it using the CBL2 library, which reverse-engineers TI's protocol. The ESP32 then performs the actual work (WiFi requests, API calls, etc.) and sends the results back as calculator variables (strings, numbers, or even pictures).

The TI-BASIC launcher program on your calculator provides the UI - it sends commands to the ESP32, waits for responses, and displays the results. From the calculator's perspective, it's just talking to another calculator. It has no idea there's WiFi involved.

## Building One

I'm intentionally leaving out a detailed hardware tutorial. If you know enough to build one, you probably don't need step-by-step instructions. If you don't, this isn't the project to learn on. This also keeps people from easily mass-producing these to cheat on exams.

The schematic and PCB files are in the repo if you want to figure it out yourself.

## Software Setup

### 1. ESP32 Setup
1. Open `/esp32/esp32.ino` in Arduino IDE
2. Install required libraries: TICL, CBL2, TIVar, WiFi, HTTPClient, UrlEncode, Preferences
3. Flash the code to your ESP32
4. No configuration files needed!

### 2. WiFi Configuration (Captive Portal)
1. Power on your calculator and run the **ANDYGPT** program
2. Go to **Settings → SETUP** to broadcast the captive portal
3. On your phone/computer, connect to the WiFi network named **"calc"**
4. A captive portal will automatically open (or navigate to `192.168.4.1`)
5. Enter your WiFi name and password
6. Click "Save & Connect"
7. The ESP32 will remember the settings for next time

The server is pre-configured to use my hosted instance - no need to run your own!

### 3. Calculator Setup
Transfer the **ANDYGPT** program to your calculator using Settings → Update in the launcher, or by connecting via USB.

## Running Your Own Server (Optional)

By default, the ESP32 connects to my hosted server. If you want to run your own:

1. Set up the server:
   ```bash
   cd server
   npm install
   ```

2. Create `server/.env` and add your OpenAI API key:
   ```
   OPENAI_API_KEY=sk-your-key-here
   ```

3. Run the server:
   ```bash
   node index.mjs
   ```

4. Expose it with ngrok or similar:
   ```bash
   ngrok http 8080
   ```

5. Change the server URL in `/esp32/esp32.ino` - find the `SERVER` define and update it to your ngrok URL.

## Reconfiguring WiFi

If you need to change WiFi settings:
- Go to **Settings → MORE → SETUP** in the ANDYGPT program to broadcast the captive portal
- Or use **Settings → SCAN WIFI** to scan for nearby networks and connect directly
- Or use **Settings → SUFFIELD** for a one-tap Suffield Academy connect
- Or erase ESP32 flash and re-upload the firmware

## Connecting to Eduroam (University WiFi)

1. Go to **Settings → MORE → SETUP** to open the captive portal
2. Connect to the **"calc"** WiFi from your phone/computer
3. Open `192.168.4.1` if the portal doesn't auto-open
4. Click the **Eduroam** tab
5. Enter your university email and password
6. Click "Connect to Eduroam"

## Case-Sensitive Input

The TI-84 keyboard only types uppercase letters. To enter lowercase or mixed-case text (e.g. WiFi passwords):

- All letters are **lowercase by default**
- Wrap a letter in parentheses to make it **uppercase**: `(H)ELLO` → `Hello`
- Numbers and symbols are unchanged: `PASSWORD123!` → `password123!`

## Using GPT

1. From the main menu, select **GPT → ASK**
2. Type your question and press ENTER
3. Wait for the response to appear; use **◄/►** to flip pages on long answers
4. Press any other key to ask a follow-up (the conversation persists), or **CLEAR** to return to the menu

The GPT submenu also has:
- **HISTORY** — replay your previous messages and responses for the current chat session
- **NEW CHAT** — clear the server-side session and start a fresh conversation

**Note:** The TI-84's math glyphs (∫, Σ, etc.) don't fully decode over the link port. Type math in plain words (e.g., "integrate x squared from 0 to 1") instead of using special tokens.

## Using Physics

The **PHYSICS** menu is built around AP Physics C E&M. Most entries serve pre-computed cheat-sheets straight from the firmware (no network round-trip); **SOLVER** is the only branch that calls GPT.

From the main menu, select **PHYSICS**, then pick a branch:

- **DERIVE** — step-by-step derivations
  - `CIRCUITS` (RC charge/discharge, LC oscillation, LR charge/discharge)
  - `GAUSS` (sphere, cylinder, plane, parallel plate)
  - `INDUCT` (motional EMF, rotating loop, changing B)
  - `MAGNET` (loop B, infinite wire, solenoid)
  - `POTENT` (sphere V, cylinder V, ring, capacitor U)
- **EQUATION** — formula lookup
  - `STATIC`, `OHMS`, `FORCES`, `FIELDS`, `INDUCT`, `ENERGY`
- **GRAPHS** — ASCII descriptions of standard E&M curves (transient, oscillation, field profiles)
- **RHR** — right-hand-rule mnemonics (wire B, force on charge / wire, Lenz, loop B, cross product)
- **BEHAVIOR** — conceptual reasoning
  - `STATICS` (conductors, dielectrics, V/E relationships, sharp points, field lines, Faraday cage)
  - `CIRCUITS` (caps/inductors at t=0 and t=∞, networks, power, meters)
  - `GENERAL` (circulating B, reference voltages, infinitesimal charge elements, grounding, charge redistribution)
  - `RULES` (junction, loop, charges, Lenz, conservation, symmetry)
- **SOLVER** — GPT-backed numeric solver
  - `CIRCUITS` (current, voltage, resistance, capacitance, inductance, power)
  - `FIELDS` (E field, B field, flux, potential)
  - `ENERGY` (cap U, ind U, charge)
  - `OTHER` (force, EMF, τ, frequency, torque)
  - `CUSTOM` — plain-text catch-all; type the problem in your own words

Each cheat-sheet returns a multi-line answer that paginates with **◄/►**. **CLEAR** returns to the menu.

## OTA Updates

Both the ESP32 firmware and the calculator launcher program can be updated over-the-air:

1. Go to **Settings → UPDATE** on your calculator
2. The ESP32 checks the server for a new version
3. If available, it flashes the new ESP32 firmware and reboots
4. After reboot, the updated launcher program is automatically pushed to your calculator

To check your current version and the latest available, go to **Settings → VERSION**.


## Troubleshooting

- **ESP32 not responding:** Check your TIP/RING connections. They might be swapped.
- **WiFi won't connect:** Make sure you're connecting to a 2.4GHz network. The ESP32-C3 doesn't support 5GHz.
- **ChatGPT returns garbage:** The response might contain characters the calculator can't display. Working on it.
- **Calculator freezes:** The link protocol is timing-sensitive. Try again, or power cycle both devices.

## Planned Features

- Basic web browsing
- Camera support (ESP32-S3)
- Mechanics + chemistry cheat-sheets alongside the existing E&M suite

## Known Issues

- Images don't work consistently
- **Complex math expressions** (integrals, derivatives, summations) use TI tokens that aren't decoded yet — type questions as plain text instead of using math symbols
- The standalone **MATH** menu (derivative / integral / series / avg value / etc.) and **UTILS** menu (translate / define) have been archived from the launcher UI to make room for the static physics formulas. The underlying ESP32 handlers are still in firmware and reachable by command ID if you build a custom client.

## Credits

- Original concept by [ChromaLock](https://www.youtube.com/watch?v=Bicjxl4EcJg) (RIP his repo)
- Everything else by Andy

## License

GPL v3 - See [LICENSE](LICENSE) for details.
