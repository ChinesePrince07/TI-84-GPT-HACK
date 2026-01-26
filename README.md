# TI-84

A mod for the TI-84 Plus Silver Edition & TI-84 Plus C Silver Edition calculators to give them internet access and add other features, like test mode breakout and camera support.

![built pcb](./pcb/built.png)

## About

**Author:** Andy (ChinesePrince07)

## Features

- ChatGPT integration - ask questions directly from your calculator
- Wi-Fi connectivity via ESP32 with **captive portal configuration**
- Program downloads over the air
- Image display support
- Chat functionality
- **Pre-configured server** - no need to run your own!

## Hardware Required

- TI-84 Plus Silver Edition or TI-84 Plus C Silver Edition
- ESP32-C3 (XIAO recommended) or ESP32-S3 Sense (for camera)
- Custom PCB (gerber files in `/pcb/0.1/`)
- 4x 1kΩ resistors
- 2x MOSFETs (AO3401)
- Wires and soldering equipment

## Setup

### 1. ESP32 Setup
1. Flash the code in `/esp32` to your ESP32
2. No configuration files needed!

### 2. WiFi Configuration (Captive Portal)
1. Power on the ESP32
2. Connect to the WiFi network named **"calc"** from your phone/computer
3. A captive portal will automatically open (or navigate to `192.168.4.1`)
4. Enter your WiFi name and password
5. Click "Save & Connect"
6. The ESP32 will connect to your network and remember the settings

The server is pre-configured - no need to run your own!

### 3. Calculator Setup
Transfer the LAUNCHER program to your calculator using the TI-84 menu.

## Reconfiguring WiFi

If you need to change WiFi settings:
- The captive portal is available for a few seconds on every boot
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

## Planned Features

- ~~Change Wi-Fi settings directly from calculator~~ **Done!**
- Support for color images
- Multi-page GPT responses
- Chat history
- Basic web browsing
- Weather integration
- Camera support (ESP32-S3)

## Known Issues

- Images don't work consistently
- ~~GPT menu may close immediately when receiving response~~ **Fixed!**
- App transfer occasionally fails

## Credits

- Rebuilt and maintained by Andy (ChinesePrince07)

## License

GPL v3 - See [LICENSE](LICENSE) for details.
