/*
  ESP8266 LED Web Server - ON/OFF from your phone
  ------------------------------------------------
  How it works (no home WiFi password needed):
  1. ESP8266 creates its own WiFi network (access point)
  2. Phone joins that WiFi network
  3. Open http://192.168.4.1 in the phone browser
  4. Tap ON / OFF to control the built-in LED

  Network name (SSID): ESP8266-LED
  Password:            esp8266led
  Web page:            http://192.168.4.1

  Board: NodeMCU / Wemos D1 mini / most ESP8266
  FQBN:  esp8266:esp8266:nodemcuv2
*/

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

#ifndef LED_BUILTIN
#define LED_BUILTIN 2
#endif

// --- Your hotspot (change these if you want) ---
const char *AP_SSID = "ESP8266-LED";
const char *AP_PASS = "esp8266led"; // min 8 chars for WPA2
const uint16_t HTTP_PORT = 80;

ESP8266WebServer server(HTTP_PORT);
bool ledOn = false;

void applyLed() {
  // Built-in LED is active-LOW on most ESP8266 boards
  digitalWrite(LED_BUILTIN, ledOn ? LOW : HIGH);
}

String htmlPage() {
  String color = ledOn ? "#22c55e" : "#64748b";
  String btnOn = ledOn
    ? "background:#22c55e;color:#fff;border:3px solid #16a34a;"
    : "background:#fff;color:#22c55e;border:3px solid #22c55e;";
  String btnOff = (!ledOn)
    ? "background:#ef4444;color:#fff;border:3px solid #dc2626;"
    : "background:#fff;color:#ef4444;border:3px solid #ef4444;";
  String state = ledOn ? "ON" : "OFF";

  String page;
  page += F("<!DOCTYPE html><html><head><meta charset='utf-8'>");
  page += F("<meta name='viewport' content='width=device-width,initial-scale=1'>");
  page += F("<title>ESP8266 LED</title><style>");
  page += F("body{font-family:system-ui,sans-serif;margin:0;background:#0f172a;color:#e2e8f0;");
  page += F("display:flex;flex-direction:column;align-items:center;justify-content:center;min-height:100vh;}");
  page += F("h1{font-size:1.4rem;margin:0 0 8px}p{color:#94a3b8;margin:0 0 24px}");
  page += F(".dot{width:80px;height:80px;border-radius:50%;background:");
  page += color;
  page += F(";margin-bottom:24px;box-shadow:0 0 30px ");
  page += color;
  page += F(";}");
  page += F(".state{font-size:2rem;font-weight:700;margin-bottom:28px;letter-spacing:4px;}");
  page += F(".row{display:flex;gap:16px;}");
  page += F("a.btn{display:inline-block;padding:18px 40px;font-size:1.2rem;font-weight:700;");
  page += F("border-radius:14px;text-decoration:none;}");
  page += F("small{margin-top:28px;color:#64748b}</style></head><body>");
  page += F("<h1>ESP8266 LED Control</h1>");
  page += F("<p>Built-in LED via WiFi</p>");
  page += F("<div class='dot'></div>");
  page += F("<div class='state'>");
  page += state;
  page += F("</div><div class='row'>");
  page += F("<a class='btn' style='");
  page += btnOn;
  page += F("' href='/led?state=on'>ON</a>");
  page += F("<a class='btn' style='");
  page += btnOff;
  page += F("' href='/led?state=off'>OFF</a>");
  page += F("</div><small>SSID: ");
  page += AP_SSID;
  page += F(" &middot; http://192.168.4.1</small>");
  page += F("</body></html>");
  return page;
}

void handleRoot() {
  server.send(200, "text/html", htmlPage());
}

void handleLed() {
  String q = server.arg("state");
  q.toLowerCase();
  if (q == "on") {
    ledOn = true;
  } else if (q == "off") {
    ledOn = false;
  } else {
    server.send(400, "text/plain", "Use /led?state=on or /led?state=off");
    return;
  }
  applyLed();
  Serial.print(F("LED -> "));
  Serial.println(ledOn ? F("ON") : F("OFF"));

  // If request came from the buttons, go back to the page.
  // If it's an API-style GET without expecting HTML, still return simple OK + redirect page.
  String accept = server.header("Accept");
  if (accept.indexOf("text/html") >= 0 || server.arg("ajax").length() == 0) {
    server.sendHeader("Location", "/");
    server.send(303, "text/plain", "Redirect");
  } else {
    server.send(200, "application/json", ledOn ? "{\"led\":\"on\"}" : "{\"led\":\"off\"}");
  }
}

void handleStatus() {
  String json = String("{\"led\":\"") + (ledOn ? "on" : "off") + "\",\"ip\":\"" + WiFi.softAPIP().toString() + "\"}";
  server.send(200, "application/json", json);
}

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  ledOn = false;
  applyLed();

  Serial.begin(115200);
  delay(200);

  Serial.println();
  Serial.println(F("----------------------------------------"));
  Serial.println(F("ESP8266 LED Web Server v1.0"));
  Serial.println(F("----------------------------------------"));

  WiFi.mode(WIFI_AP);
  bool apOk = WiFi.softAP(AP_SSID, AP_PASS);
  if (!apOk) {
    Serial.println(F("ERROR: softAP failed"));
  }

  server.on("/", HTTP_GET, handleRoot);
  server.on("/led", HTTP_GET, handleLed);
  server.on("/status", HTTP_GET, handleStatus);
  server.onNotFound([]() {
    server.send(404, "text/plain", "Not found. Open /");
  });
  server.begin();

  Serial.print(F("WiFi SSID: "));
  Serial.println(AP_SSID);
  Serial.print(F("Password:  "));
  Serial.println(AP_PASS);
  Serial.print(F("IP:        http://"));
  Serial.println(WiFi.softAPIP());
  Serial.println(F("Join this WiFi on your phone, then open the IP."));
  Serial.println(F("----------------------------------------"));
}

void loop() {
  server.handleClient();
}
