# TI-84 GPT HACK

A hardware mod that gives your TI-84 calculator internet access, ChatGPT integration, and more.

## Demo

<video src="demo.mp4" controls width="600"></video>

![built pcb](./pcb/built.png)

## About

This project turns your dusty graphing calculator into a WiFi-enabled device capable of querying ChatGPT, downloading programs over the air, and chatting with other modded calculators.

The original concept came from ChromaLock, who made a [video about it](https://www.youtube.com/watch?v=Bicjxl4EcJg) back in 2024. His repo got nuked, so I rebuilt the whole thing from scratch based on what I could piece together. Most of the code, PCB design, and implementation is my own work at this point.

**Author:** Andy (ChinesePrince07)

## Features

- ChatGPT integration - ask questions directly from your calculator, with **loop mode** to keep asking without returning to menu
- Wi-Fi connectivity via ESP32 with **captive portal configuration**
- Program downloads over the air
- Image display support (96x63 monochrome)
- Chat functionality
- **Pre-configured server** - no need to run your own!

## How It Works

The TI-84 has a 2.5mm link port originally designed for calculator-to-calculator communication using Texas Instruments' proprietary link protocol. This mod exploits that by wiring an ESP32 microcontroller to impersonate another calculator.

When your calculator sends a variable (like a command number), the ESP32 intercepts it using the CBL2 library, which reverse-engineers TI's protocol. The ESP32 then performs the actual work (WiFi requests, API calls, etc.) and sends the results back as calculator variables (strings, numbers, or even pictures).

The TI-BASIC launcher program on your calculator provides the UI - it sends commands to the ESP32, waits for responses, and displays the results. From the calculator's perspective, it's just talking to another calculator. It has no idea there's WiFi involved.

## Hardware Required

- TI-84 Plus Silver Edition or TI-84 Plus C Silver Edition
- Seeeduino XIAO ESP32-C3 (or ESP32-S3 Sense for camera support)
- Custom PCB (gerber files in `/pcb/0.1/`)
- 4x 1kΩ resistors
- 2x N-channel MOSFETs (MMBF170 or similar)
- Magnet wire (30-34 AWG recommended)
- Soldering iron with fine tip
- Patience

## Hardware Assembly

Order the PCB from `/pcb/0.1/`, solder the components, wire TIP/RING/5V/GND to your calculator's link port and battery terminals. Check the schematic if you get stuck. If you fry something, that's on you.

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
- Go to **Settings → SETUP** in the ANDYGPT program to broadcast the captive portal
- Or erase ESP32 flash and re-upload the firmware

## Commands

| ID | Command | Description |
|----|---------|-------------|
| 0 | connect | Connect to configured WiFi |
| 1 | disconnect | Disconnect from WiFi |
| 2 | gpt | Query ChatGPT |
| 5 | launcher | Transfer launcher program |
| 9 | image_list | List available images |
| 10 | fetch_image | Download image to calculator |
| 11 | fetch_chats | Get chat messages |
| 12 | send_chat | Send chat message |
| 13 | program_list | List available programs |
| 14 | fetch_program | Download program |
| 15 | setup_wifi | Broadcast captive portal for WiFi config |

## Using GPT

1. From the main menu, select **GPT**
2. Type your question and press ENTER
3. Wait for the response to appear
4. Press **any key** to ask another question, or **CLEAR** to return to the menu

The GPT mode loops so you can have a continuous conversation without navigating back to the menu each time.

## Troubleshooting

- **ESP32 not responding:** Check your TIP/RING connections. They might be swapped.
- **WiFi won't connect:** Make sure you're connecting to a 2.4GHz network. The ESP32-C3 doesn't support 5GHz.
- **ChatGPT returns garbage:** The response might contain characters the calculator can't display. Working on it.
- **Calculator freezes:** The link protocol is timing-sensitive. Try again, or power cycle both devices.

## Planned Features

- Support for color images
- Multi-page GPT responses
- Chat history
- Basic web browsing
- Weather integration
- Camera support (ESP32-S3)

## Known Issues

- Images don't work consistently
- App transfer occasionally fails

## Credits

- Original concept by [ChromaLock](https://www.youtube.com/watch?v=Bicjxl4EcJg) (RIP his repo)
- Everything else by Andy

## License

GPL v3 - See [LICENSE](LICENSE) for details.
