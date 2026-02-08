// Project: TI-84 GPT HACK
// Author:  Andy (ChinesePrince07)
// Date:    2025

#include "./launcher.h"
#include "./ti_tokens.h"
#include <TICL.h>
#include <CBL2.h>
#include <TIVar.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <WiFiClient.h>
#include <HTTPClient.h>
#include <UrlEncode.h>
#include <Preferences.h>
#include <WebServer.h>
#include <DNSServer.h>

// Default server URL
#define SERVER "https://api.andypandy.org"
#define SECURE

// Captive portal settings
#define AP_SSID "calc"
#define AP_PASS ""
#define DNS_PORT 53

WebServer webServer(80);
DNSServer dnsServer;
bool portalActive = false;

// Stored WiFi credentials
String storedSSID = "";
String storedPass = "";
String storedChatName = "calculator";

// #define CAMERA

#ifdef CAMERA
#include <esp_camera.h>
#define CAMERA_MODEL_XIAO_ESP32S3
#include "./camera_pins.h"
#include "./camera_index.h"
#endif

constexpr auto TIP = D1;
constexpr auto RING = D10;
constexpr auto MAXHDRLEN = 16;
constexpr auto MAXDATALEN = 4096;
constexpr auto MAXARGS = 5;
constexpr auto MAXSTRARGLEN = 256;
constexpr auto PICSIZE = 756;
constexpr auto PICVARSIZE = PICSIZE + 2;
constexpr auto PASSWORD = 42069;

CBL2 cbl;
Preferences prefs;

// whether or not the user has entered the password
bool unlocked = true;

// Arguments
int currentArg = 0;
char strArgs[MAXARGS][MAXSTRARGLEN];
double realArgs[MAXARGS];

// the command to execute
int command = -1;
// whether or not the operation has completed
bool status = 0;
// whether or not the operation failed
bool error = 0;
// error or success message
char message[MAXSTRARGLEN];
// list data
constexpr auto LISTLEN = 256;
constexpr auto LISTENTRYLEN = 20;
char list[LISTLEN][LISTENTRYLEN];
// http response
constexpr auto MAXHTTPRESPONSELEN = 4096;
char response[MAXHTTPRESPONSELEN];
// image variable (96x63)
uint8_t frame[PICVARSIZE] = { PICSIZE & 0xff, PICSIZE >> 8 };

void connect();
void disconnect();
void gpt();
void send();
void launcher();
void snap();
void solve();
void image_list();
void fetch_image();
void fetch_chats();
void send_chat();
void program_list();
void fetch_program();
void setup_wifi();
void derivative();
void integrate();

struct Command {
  int id;
  const char* name;
  int num_args;
  void (*command_fp)();
  bool wifi;
};

struct Command commands[] = {
  { 0, "connect", 0, connect, false },
  { 1, "disconnect", 0, disconnect, false },
  { 2, "gpt", 1, gpt, true },
  { 4, "send", 2, send, true },
  { 5, "launcher", 0, launcher, false },
  { 7, "snap", 0, snap, false },
  { 8, "solve", 1, solve, true },
  { 9, "image_list", 1, image_list, true },
  { 10, "fetch_image", 1, fetch_image, true },
  { 11, "fetch_chats", 2, fetch_chats, true },
  { 12, "send_chat", 2, send_chat, true },
  { 13, "program_list", 1, program_list, true },
  { 14, "fetch_program", 1, fetch_program, true },
  { 15, "setup_wifi", 0, setup_wifi, false },
  { 16, "derivative", 1, derivative, true },
  { 17, "integrate", 1, integrate, true },
};

constexpr int NUMCOMMANDS = sizeof(commands) / sizeof(struct Command);
constexpr int MAXCOMMAND = 17;

uint8_t header[MAXHDRLEN];
uint8_t data[MAXDATALEN];

// lowercase letters make strings weird,
// so we have to truncate the string
void fixStrVar(char* str) {
  int end = strlen(str);
  for (int i = 0; i < end; ++i) {
    if (isLowerCase(str[i])) {
      --end;
    }
  }
  str[end] = '\0';
}

int onReceived(uint8_t type, enum Endpoint model, int datalen);
int onRequest(uint8_t type, enum Endpoint model, int* headerlen,
              int* datalen, data_callback* data_callback);

void startCommand(int cmd) {
  command = cmd;
  status = 0;
  error = 0;
  currentArg = 0;
  for (int i = 0; i < MAXARGS; ++i) {
    memset(&strArgs[i], 0, MAXSTRARGLEN);
    realArgs[i] = 0;
  }
  strncpy(message, "no command", MAXSTRARGLEN);
}

void setError(const char* err) {
  Serial.print("ERROR: ");
  Serial.println(err);
  error = 1;
  status = 1;
  command = -1;
  strncpy(message, err, MAXSTRARGLEN);
}

void setSuccess(const char* success) {
  Serial.print("SUCCESS: ");
  Serial.println(success);
  error = 0;
  status = 1;
  command = -1;
  strncpy(message, success, MAXSTRARGLEN);
}

int sendProgramVariable(const char* name, uint8_t* program, size_t variableSize);

bool camera_sign = false;

// Captive portal HTML
const char* portalHTML = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <title>TI-84 WiFi Setup</title>
    <style>
        * { box-sizing: border-box; font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', Roboto, sans-serif; }
        body { background: linear-gradient(135deg, #1a1a2e 0%, #16213e 100%); min-height: 100vh; margin: 0; display: flex; justify-content: center; align-items: center; padding: 20px; }
        .container { background: white; border-radius: 16px; padding: 32px; max-width: 380px; width: 100%; box-shadow: 0 20px 60px rgba(0,0,0,0.3); }
        h1 { margin: 0 0 8px 0; font-size: 24px; color: #1a1a2e; }
        .subtitle { color: #666; margin-bottom: 24px; font-size: 14px; }
        .logo { text-align: center; margin-bottom: 16px; }
        .logo img { width: 64px; height: 64px; border-radius: 50%; }
        label { display: block; margin-bottom: 6px; font-weight: 600; color: #333; font-size: 14px; }
        input { width: 100%; padding: 14px; border: 2px solid #e0e0e0; border-radius: 8px; font-size: 16px; margin-bottom: 16px; }
        input:focus { outline: none; border-color: #4a6cf7; }
        button { width: 100%; padding: 16px; background: linear-gradient(135deg, #4a6cf7 0%, #6366f1 100%); color: white; border: none; border-radius: 8px; font-size: 16px; font-weight: 600; cursor: pointer; }
        .footer { text-align: center; margin-top: 20px; font-size: 12px; color: #999; }
    </style>
</head>
<body>
    <div class="container">
        <div class="logo"><img src="data:image/jpeg;base64,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"></div>
        <h1>TI-84 WiFi Setup</h1>
        <p class="subtitle">Configure your calculator's WiFi connection</p>
        <form action="/save" method="POST">
            <label>WiFi Network Name</label>
            <input type="text" name="ssid" placeholder="Enter your WiFi SSID" required>
            <label>WiFi Password</label>
            <input type="password" name="password" placeholder="Enter your WiFi password">
            <label>Chat Name (optional)</label>
            <input type="text" name="chatname" placeholder="Your display name" value="calculator">
            <button type="submit">Save & Connect</button>
        </form>
        <div class="footer">TI-84 GPT HACK by Andy</div>
    </div>
</body>
</html>
)rawliteral";

const char* successHTML = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <title>Connected!</title>
    <style>
        body { background: #1a1a2e; color: white; font-family: sans-serif; display: flex; justify-content: center; align-items: center; height: 100vh; margin: 0; }
        .box { text-align: center; padding: 40px; }
        h1 { color: #4ade80; }
    </style>
</head>
<body>
    <div class="box">
        <h1>Connected!</h1>
        <p>WiFi credentials saved. You can close this page.</p>
    </div>
</body>
</html>
)rawliteral";

void handleRoot() {
  webServer.send(200, "text/html", portalHTML);
}

void handleSave() {
  storedSSID = webServer.arg("ssid");
  storedPass = webServer.arg("password");
  storedChatName = webServer.arg("chatname");

  // Save to preferences
  prefs.begin("wifi", false);
  prefs.putString("ssid", storedSSID);
  prefs.putString("pass", storedPass);
  prefs.putString("chatname", storedChatName);
  prefs.end();

  Serial.println("Saved WiFi credentials:");
  Serial.println("SSID: " + storedSSID);
  Serial.println("Chat Name: " + storedChatName);

  webServer.send(200, "text/html", successHTML);

  delay(2000);
  portalActive = false;

  // Try to connect
  WiFi.softAPdisconnect(true);
  WiFi.begin(storedSSID.c_str(), storedPass.c_str());
}

void handleNotFound() {
  webServer.sendHeader("Location", "http://192.168.4.1/", true);
  webServer.send(302, "text/plain", "");
}

void startCaptivePortal() {
  Serial.println("[Starting Captive Portal]");
  WiFi.mode(WIFI_AP);
  WiFi.softAP(AP_SSID, AP_PASS);

  Serial.print("AP IP: ");
  Serial.println(WiFi.softAPIP());

  dnsServer.start(DNS_PORT, "*", WiFi.softAPIP());

  webServer.on("/", handleRoot);
  webServer.on("/save", HTTP_POST, handleSave);
  webServer.onNotFound(handleNotFound);
  webServer.begin();

  portalActive = true;
  Serial.println("Captive portal started on SSID: calc");
}

void loadSavedCredentials() {
  prefs.begin("wifi", true);
  storedSSID = prefs.getString("ssid", "");
  storedPass = prefs.getString("pass", "");
  storedChatName = prefs.getString("chatname", "calculator");
  prefs.end();
}

void tryAutoConnect() {
  if (storedSSID.length() > 0) {
    Serial.println("Found saved credentials, connecting...");
    Serial.println("SSID: " + storedSSID);
    WiFi.begin(storedSSID.c_str(), storedPass.c_str());

    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 20) {
      delay(500);
      Serial.print(".");
      attempts++;
    }

    if (WiFi.status() == WL_CONNECTED) {
      Serial.println("\nConnected to WiFi!");
      Serial.print("IP: ");
      Serial.println(WiFi.localIP());
    } else {
      Serial.println("\nFailed to connect. Use SETUP in calculator to configure WiFi.");
    }
  } else {
    Serial.println("No saved credentials. Use SETUP in calculator to configure WiFi.");
  }
}

void setup() {
  Serial.begin(115200);
  Serial.println("[CBL]");

  cbl.setLines(TIP, RING);
  cbl.resetLines();
  cbl.setupCallbacks(header, data, MAXDATALEN, onReceived, onRequest);
  // cbl.setVerbosity(true, (HardwareSerial *)&Serial);

  pinMode(TIP, INPUT);
  pinMode(RING, INPUT);

  Serial.println("[preferences]");
  prefs.begin("ccalc", false);
  auto reboots = prefs.getUInt("boots", 0);
  Serial.print("reboots: ");
  Serial.println(reboots);
  prefs.putUInt("boots", reboots + 1);
  prefs.end();

#ifdef CAMERA
  Serial.println("[camera]");

  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.frame_size = FRAMESIZE_UXGA;
  // this needs to be pixformat grayscale in the future
  config.pixel_format = PIXFORMAT_JPEG;  // for streaming
  config.grab_mode = CAMERA_GRAB_WHEN_EMPTY;
  config.fb_location = CAMERA_FB_IN_PSRAM;
  config.jpeg_quality = 12;
  config.fb_count = 1;

  // if PSRAM IC present, init with UXGA resolution and higher JPEG quality
  //                      for larger pre-allocated frame buffer.
  if (config.pixel_format == PIXFORMAT_JPEG) {
    if (psramFound()) {
      config.jpeg_quality = 10;
      config.fb_count = 2;
      config.grab_mode = CAMERA_GRAB_LATEST;
    } else {
      // Limit the frame size when PSRAM is not available
      config.frame_size = FRAMESIZE_SVGA;
      config.fb_location = CAMERA_FB_IN_DRAM;
    }
  } else {
    // Best option for face detection/recognition
    config.frame_size = FRAMESIZE_240X240;
#if CONFIG_IDF_TARGET_ESP32S3
    config.fb_count = 2;
#endif
  }

  // camera init
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x\n", err);
    return;
  } else {
    Serial.println("camera ready");
    camera_sign = true;  // Camera initialization check passes
  }

  sensor_t* s = esp_camera_sensor_get();
  // enable grayscale
  s->set_special_effect(s, 2);
#endif

  strncpy(message, "default message", MAXSTRARGLEN);
  delay(100);
  memset(data, 0, MAXDATALEN);
  memset(header, 0, 16);

  // Load saved WiFi credentials (connect when user selects CONNECT)
  loadSavedCredentials();

  Serial.println("[ready]");
}

void (*queued_action)() = NULL;

void loop() {
  // Handle captive portal
  if (portalActive) {
    dnsServer.processNextRequest();
    webServer.handleClient();
  }

  if (queued_action) {
    // dont ask me why you need this, but it fails otherwise.
    // probably relates to a CBL2 timeout thing?
    delay(1000);
    Serial.println("executing queued actions");
    // dont ask me
    void (*tmp)() = queued_action;
    queued_action = NULL;
    tmp();
  }
  if (command >= 0 && command <= MAXCOMMAND) {
    for (int i = 0; i < NUMCOMMANDS; ++i) {
      if (commands[i].id == command && commands[i].num_args == currentArg) {
        if (commands[i].wifi && !WiFi.isConnected()) {
          setError("wifi not connected");
        } else {
          Serial.print("processing command: ");
          Serial.println(commands[i].name);
          commands[i].command_fp();
        }
      }
    }
  }
  cbl.eventLoopTick();
}

int onReceived(uint8_t type, enum Endpoint model, int datalen) {
  char varName = header[3];

  Serial.print("unlocked: ");
  Serial.println(unlocked);

  // check for password
  if (!unlocked && varName == 'P') {
    auto password = TIVar::realToLong8x(data, model);
    if (password == PASSWORD) {
      Serial.println("successful unlock");
      unlocked = true;
      return 0;
    } else {
      Serial.println("failed unlock");
    }
  }

  if (!unlocked) {
    return -1;
  }

  // check for command
  if (varName == 'C') {
    if (type != VarTypes82::VarReal) {
      return -1;
    }
    int cmd = TIVar::realToLong8x(data, model);
    if (cmd >= 0 && cmd <= MAXCOMMAND) {
      Serial.print("command: ");
      Serial.println(cmd);
      startCommand(cmd);
      return 0;
    } else {
      Serial.print("invalid command: ");
      Serial.println(cmd);
      return -1;
    }
  }

  if (currentArg >= MAXARGS) {
    Serial.println("argument overflow");
    setError("argument overflow");
    return -1;
  }

  switch (type) {
    case VarTypes82::VarString:
      {
        // Get the token length from the first two bytes
        int tokenLen = data[0] | (data[1] << 8);
        Serial.print("token len: ");
        Serial.println(tokenLen);

        // Decode the tokens to readable text
        decodeTokenString(data + 2, tokenLen, strArgs[currentArg], MAXSTRARGLEN);
        Serial.print("Str");
        Serial.print(currentArg);
        Serial.print(" (decoded): ");
        Serial.println(strArgs[currentArg]);
        currentArg++;
      }
      break;
    case VarTypes82::VarReal:
      realArgs[currentArg++] = TIVar::realToFloat8x(data, model);
      Serial.print("Real");
      Serial.print(currentArg - 1);
      Serial.print(" ");
      Serial.println(realArgs[currentArg - 1]);
      break;
    default:
      // maybe set error here?
      return -1;
  }
  return 0;
}

uint8_t frameCallback(int idx) {
  return frame[idx];
}

char varIndex(int idx) {
  return '0' + (idx == 9 ? 0 : (idx + 1));
}

int onRequest(uint8_t type, enum Endpoint model, int* headerlen, int* datalen, data_callback* data_callback) {
  char varName = header[3];
  char strIndex = header[4];
  char strname[5] = { 'S', 't', 'r', varIndex(strIndex), 0x00 };
  char picname[5] = { 'P', 'i', 'c', varIndex(strIndex), 0x00 };
  Serial.print("request for ");
  Serial.println(varName == 0xaa ? strname : varName == 0x60 ? picname
                                                             : (const char*)&header[3]);
  memset(header, 0, sizeof(header));
  switch (varName) {
    case 0x60:
      if (type != VarTypes82::VarPic) {
        return -1;
      }
      *datalen = PICVARSIZE;
      TIVar::intToSizeWord(*datalen, &header[0]);
      header[2] = VarTypes82::VarPic;
      header[3] = 0x60;
      header[4] = strIndex;
      *data_callback = frameCallback;
      break;
    case 0xAA:
      if (type != VarTypes82::VarString) {
        return -1;
      }
      // TODO right now, the only string variable will be the message, but ill need to allow for other vars later
      *datalen = TIVar::stringToStrVar8x(String(message), data, model);
      TIVar::intToSizeWord(*datalen, header);
      header[2] = VarTypes82::VarString;
      header[3] = 0xAA;
      // send back as same variable that was requested
      header[4] = strIndex;
      *headerlen = 13;
      break;
    case 'E':
      if (type != VarTypes82::VarReal) {
        return -1;
      }
      *datalen = TIVar::longToReal8x(error, data, model);
      TIVar::intToSizeWord(*datalen, header);
      header[2] = VarTypes82::VarReal;
      header[3] = 'E';
      header[4] = '\0';
      *headerlen = 13;
      break;
    case 'S':
      if (type != VarTypes82::VarReal) {
        return -1;
      }
      Serial.print("sending S=");
      Serial.println(status);
      *datalen = TIVar::longToReal8x(status, data, model);
      TIVar::intToSizeWord(*datalen, header);
      header[2] = VarTypes82::VarReal;
      header[3] = 'S';
      header[4] = '\0';
      *headerlen = 13;
      break;
    default:
      return -1;
  }
  return 0;
}

int makeRequest(String url, char* result, int resultLen, size_t* len) {
  memset(result, 0, resultLen);

#ifdef SECURE
  WiFiClientSecure client;
  client.setInsecure();
#else
  WiFiClient client;
#endif
  HTTPClient http;

  Serial.println(url);
  http.begin(client, url.c_str());
  http.addHeader("ngrok-skip-browser-warning", "true");

  // Send HTTP GET request
  int httpResponseCode = http.GET();
  Serial.print(url);
  Serial.print(" ");
  Serial.println(httpResponseCode);

  int responseSize = http.getSize();
  WiFiClient* httpStream = http.getStreamPtr();

  Serial.print("response size: ");
  Serial.println(responseSize);

  if (httpResponseCode != 200) {
    return httpResponseCode;
  }

  if (httpStream->available() > resultLen) {
    Serial.print("response size: ");
    Serial.print(httpStream->available());
    Serial.println(" is too big");
    return -1;
  }

  while (httpStream->available()) {
    *(result++) = httpStream->read();
  }
  *len = responseSize;

  http.end();

  return 0;
}

void connect() {
  if (storedSSID.length() == 0) {
    setError("no wifi configured");
    return;
  }
  Serial.print("SSID: ");
  Serial.println(storedSSID);
  Serial.print("PASS: ");
  Serial.println("<hidden>");
  WiFi.begin(storedSSID.c_str(), storedPass.c_str());
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20) {
    delay(500);
    attempts++;
    if (WiFi.status() == WL_CONNECT_FAILED) {
      setError("failed to connect");
      return;
    }
  }
  if (WiFi.status() == WL_CONNECTED) {
    setSuccess("connected");
  } else {
    setError("connection timeout");
  }
}

void disconnect() {
  WiFi.disconnect(true);
  setSuccess("disconnected");
}

void setup_wifi() {
  // Stop any existing WiFi connection
  WiFi.disconnect(true);
  delay(100);

  // Start the captive portal
  startCaptivePortal();
  setSuccess("SETUP: connect to 'calc' WiFi");
}

void gpt() {
  const char* prompt = strArgs[0];
  Serial.print("prompt: ");
  Serial.println(prompt);

  auto url = String(SERVER) + String("/gpt/ask?question=") + urlEncode(String(prompt));

  size_t realsize = 0;
  if (makeRequest(url, response, MAXHTTPRESPONSELEN, &realsize)) {
    setError("error making request");
    return;
  }

  Serial.print("response: ");
  Serial.println(response);

  setSuccess(response);
}

void derivative() {
  const char* expr = strArgs[0];
  Serial.print("derivative of: ");
  Serial.println(expr);

  // Use GPT with derivative prompt
  String prompt = "Find the derivative of " + String(expr) + ". Give only the answer, no explanation.";
  auto url = String(SERVER) + String("/gpt/ask?question=") + urlEncode(prompt);

  size_t realsize = 0;
  if (makeRequest(url, response, MAXHTTPRESPONSELEN, &realsize)) {
    setError("error making request");
    return;
  }

  Serial.print("result: ");
  Serial.println(response);

  setSuccess(response);
}

void integrate() {
  const char* expr = strArgs[0];
  Serial.print("integrate: ");
  Serial.println(expr);

  // Use GPT with integral prompt
  String prompt = "Find the indefinite integral of " + String(expr) + ". Give only the answer with +C, no explanation.";
  auto url = String(SERVER) + String("/gpt/ask?question=") + urlEncode(prompt);

  size_t realsize = 0;
  if (makeRequest(url, response, MAXHTTPRESPONSELEN, &realsize)) {
    setError("error making request");
    return;
  }

  Serial.print("result: ");
  Serial.println(response);

  setSuccess(response);
}

void send() {
  const char* recipient = strArgs[0];
  const char* message = strArgs[1];
  Serial.print("sending \"");
  Serial.print(message);
  Serial.print("\" to \"");
  Serial.print(recipient);
  Serial.println("\"");
  setSuccess("OK: sent");
}

void _sendLauncher() {
  sendProgramVariable("ANDYGPT", __launcher_var, __launcher_var_len);
}

void launcher() {
  // we have to queue this action, since otherwise the transfer fails
  // due to the CBL2 library still using the lines
  queued_action = _sendLauncher;
  setSuccess("queued transfer");
}

void snap() {
#ifdef CAMERA
  if (!camera_sign) {
    setError("camera failed to initialize");
  }
#else
  setError("pictures not supported");
#endif
}

void solve() {
#ifdef CAMERA
  if (!camera_sign) {
    setError("camera failed to initialize");
  }
#else
  setError("pictures not supported");
#endif
}

void image_list() {
  int page = realArgs[0];
  auto url = String(SERVER) + String("/image/list?p=") + urlEncode(String(page));

  size_t realsize = 0;
  if (makeRequest(url, response, MAXSTRARGLEN, &realsize)) {
    setError("error making request");
    return;
  }

  Serial.print("response: ");
  Serial.println(response);

  setSuccess(response);
}

void fetch_image() {
  memset(frame + 2, 0, 756);
  // fetch image and put it into the frame variable
  int id = realArgs[0];
  Serial.print("id: ");
  Serial.println(id);

  auto url = String(SERVER) + String("/image/get?id=") + urlEncode(String(id));

  size_t realsize = 0;
  if (makeRequest(url, response, MAXHTTPRESPONSELEN, &realsize)) {
    setError("error making request");
    return;
  }

  if (realsize != PICSIZE) {
    Serial.print("response size:");
    Serial.println(realsize);
    setError("bad image size");
    return;
  }

  // load the image
  frame[0] = realsize & 0xff;
  frame[1] = (realsize >> 8) & 0xff;
  memcpy(&frame[2], response, 756);

  setSuccess(response);
}

void fetch_chats() {
  int room = realArgs[0];
  int page = realArgs[1];
  auto url = String(SERVER) + String("/chats/messages?p=") + urlEncode(String(page)) + String("&c=") + urlEncode(String(room));

  size_t realsize = 0;
  if (makeRequest(url, response, MAXSTRARGLEN, &realsize)) {
    setError("error making request");
    return;
  }

  Serial.print("response: ");
  Serial.println(response);

  setSuccess(response);
}

void send_chat() {
  int room = realArgs[0];
  const char* msg = strArgs[1];

  auto url = String(SERVER) + String("/chats/send?c=") + urlEncode(String(room)) + String("&m=") + urlEncode(String(msg)) + String("&id=") + urlEncode(storedChatName);

  size_t realsize = 0;
  if (makeRequest(url, response, MAXSTRARGLEN, &realsize)) {
    setError("error making request");
    return;
  }

  Serial.print("response: ");
  Serial.println(response);

  setSuccess(response);
}

void program_list() {
  int page = realArgs[0];
  auto url = String(SERVER) + String("/programs/list?p=") + urlEncode(String(page));

  size_t realsize = 0;
  if (makeRequest(url, response, MAXSTRARGLEN, &realsize)) {
    setError("error making request");
    return;
  }

  Serial.print("response: ");
  Serial.println(response);

  setSuccess(response);
}


char programName[256];
char programData[4096];
size_t programLength;

void _resetProgram() {
  memset(programName, 0, 256);
  memset(programData, 0, 4096);
  programLength = 0;
}

void _sendDownloadedProgram() {
  if (sendProgramVariable(programName, (uint8_t*)programData, programLength)) {
    Serial.println("failed to transfer requested download");
    Serial.print(programName);
    Serial.print("(");
    Serial.print(programLength);
    Serial.println(")");
  }
  _resetProgram();
}

void fetch_program() {
  int id = realArgs[0];
  Serial.print("id: ");
  Serial.println(id);

  _resetProgram();

  auto url = String(SERVER) + String("/programs/get?id=") + urlEncode(String(id));

  if (makeRequest(url, programData, 4096, &programLength)) {
    setError("error making request for program data");
    return;
  }

  size_t realsize = 0;
  auto nameUrl = String(SERVER) + String("/programs/get_name?id=") + urlEncode(String(id));
  if (makeRequest(nameUrl, programName, 256, &realsize)) {
    setError("error making request for program name");
    return;
  }

  queued_action = _sendDownloadedProgram;

  setSuccess("queued download");
}

/// OTHER FUNCTIONS

int sendProgramVariable(const char* name, uint8_t* program, size_t variableSize) {
  Serial.print("transferring: ");
  Serial.print(name);
  Serial.print("(");
  Serial.print(variableSize);
  Serial.println(")");

  int dataLength = 0;

  // IF THIS ISNT SET TO COMP83P, THIS DOESNT WORK
  // seems like ti-84s cant silent transfer to each other
  uint8_t msg_header[4] = { COMP83P, RTS, 13, 0 };

  uint8_t rtsdata[13] = { variableSize & 0xff, variableSize >> 8, VarTypes82::VarProgram, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
  int nameSize = strlen(name);
  if (nameSize == 0) {
    return 1;
  }
  memcpy(&rtsdata[3], name, min(nameSize, 8));

  auto rtsVal = cbl.send(msg_header, rtsdata, 13);
  if (rtsVal) {
    Serial.print("rts return: ");
    Serial.println(rtsVal);
    return rtsVal;
  }

  cbl.resetLines();
  auto ackVal = cbl.get(msg_header, NULL, &dataLength, 0);
  if (ackVal || msg_header[1] != ACK) {
    Serial.print("ack return: ");
    Serial.println(ackVal);
    return ackVal;
  }

  auto ctsRet = cbl.get(msg_header, NULL, &dataLength, 0);
  if (ctsRet || msg_header[1] != CTS) {
    Serial.print("cts return: ");
    Serial.println(ctsRet);
    return ctsRet;
  }

  msg_header[1] = ACK;
  msg_header[2] = 0x00;
  msg_header[3] = 0x00;
  ackVal = cbl.send(msg_header, NULL, 0);
  if (ackVal || msg_header[1] != ACK) {
    Serial.print("ack cts return: ");
    Serial.println(ackVal);
    return ackVal;
  }

  msg_header[1] = DATA;
  msg_header[2] = variableSize & 0xff;
  msg_header[3] = (variableSize >> 8) & 0xff;
  auto dataRet = cbl.send(msg_header, program, variableSize);
  if (dataRet) {
    Serial.print("data return: ");
    Serial.println(dataRet);
    return dataRet;
  }

  ackVal = cbl.get(msg_header, NULL, &dataLength, 0);
  if (ackVal || msg_header[1] != ACK) {
    Serial.print("ack data: ");
    Serial.println(ackVal);
    return ackVal;
  }

  msg_header[1] = EOT;
  msg_header[2] = 0x00;
  msg_header[3] = 0x00;
  auto eotVal = cbl.send(msg_header, NULL, 0);
  if (eotVal) {
    Serial.print("eot return: ");
    Serial.println(eotVal);
    return eotVal;
  }

  Serial.print("transferred: ");
  Serial.println(name);
  return 0;
}