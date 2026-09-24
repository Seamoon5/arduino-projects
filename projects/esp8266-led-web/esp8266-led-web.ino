/*
  ESP8266 Multi-Load Web Server v2.0
  -----------------------------------
  Control the onboard LED + 5 spare outputs from your phone.
  Spare pins are ready for future loads (LEDs, relays, transistors).

  Phone steps:
    1. Join WiFi  SSID: ESP8266-LED   password: esp8266led
    2. Open       http://192.168.4.1
    3. Tap ON/OFF for any channel

  Channels (NodeMCU / Wemos D1 mini labels):
    0  Onboard LED   GPIO2   (built-in, active-LOW)
    1  LOAD 1        D1 / GPIO5
    2  LOAD 2        D2 / GPIO4
    3  LOAD 3        D5 / GPIO14
    4  LOAD 4        D6 / GPIO12
    5  LOAD 5        D7 / GPIO13

  Safety: spare pins output 3.3V logic only. Do NOT drive motors/relays
  directly from the pin - use a transistor, MOSFET, or relay module.

  FQBN: esp8266:esp8266:nodemcuv2
*/

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

const char *AP_SSID = "ESP8266-LED";
const char *AP_PASS = "esp8266led";
const uint16_t HTTP_PORT = 80;
const int CHANNEL_COUNT = 6;

struct Channel {
  const char *name;   // short UI name
  const char *label;  // pin description
  uint8_t pin;
  bool activeLow;     // true = LOW means ON (onboard LED)
  bool on;
};

Channel channels[CHANNEL_COUNT] = {
  { "LED",   "Onboard  GPIO2",     2,  true,  false },
  { "LOAD1", "D1  GPIO5",          5,  false, false },
  { "LOAD2", "D2  GPIO4",          4,  false, false },
  { "LOAD3", "D5  GPIO14",         14, false, false },
  { "LOAD4", "D6  GPIO12",         12, false, false },
  { "LOAD5", "D7  GPIO13",         13, false, false },
};

ESP8266WebServer server(HTTP_PORT);

void applyChannel(int i) {
  bool levelOn = channels[i].on;
  if (channels[i].activeLow) {
    digitalWrite(channels[i].pin, levelOn ? LOW : HIGH);
  } else {
    digitalWrite(channels[i].pin, levelOn ? HIGH : LOW);
  }
}

void allOff() {
  for (int i = 0; i < CHANNEL_COUNT; i++) {
    channels[i].on = false;
    applyChannel(i);
  }
}

String jsonStatus() {
  String j = F("{\"channels\":[");
  for (int i = 0; i < CHANNEL_COUNT; i++) {
    if (i) j += F(",");
    j += F("{\"id\":");
    j += i;
    j += F(",\"name\":\"");
    j += channels[i].name;
    j += F("\",\"pin\":\"");
    j += channels[i].label;
    j += F("\",\"state\":\"");
    j += channels[i].on ? F("on") : F("off");
    j += F("\"}");
  }
  j += F("],\"ip\":\"");
  j += WiFi.softAPIP().toString();
  j += F("\"}");
  return j;
}

String htmlPage() {
  String page;
  page += F("<!DOCTYPE html><html><head><meta charset='utf-8'>");
  page += F("<meta name='viewport' content='width=device-width,initial-scale=1'>");
  page += F("<title>ESP8266 Loads</title><style>");
  page += F("body{font-family:system-ui,sans-serif;margin:0;background:#0f172a;color:#e2e8f0;}");
  page += F("header{padding:20px 16px 8px;text-align:center;}");
  page += F("h1{font-size:1.25rem;margin:0}p{color:#94a3b8;margin:6px 0 0;font-size:.9rem}");
  page += F(".grid{display:grid;grid-template-columns:1fr 1fr;gap:12px;padding:16px;max-width:480px;margin:0 auto;}");
  page += F(".card{background:#1e293b;border:1px solid #334155;border-radius:14px;padding:14px;}");
  page += F(".card.on{border-color:#22c55e;box-shadow:0 0 16px rgba(34,197,94,.15)}");
  page += F(".name{font-weight:700;font-size:1.05rem;display:flex;align-items:center;gap:8px}");
  page += F(".dot{width:10px;height:10px;border-radius:50%;background:#475569}");
  page += F(".card.on .dot{background:#22c55e;box-shadow:0 0 8px #22c55e}");
  page += F(".pin{color:#94a3b8;font-size:.75rem;margin:4px 0 12px}");
  page += F(".row{display:flex;gap:8px}");
  page += F("a.btn{flex:1;text-align:center;padding:10px 0;border-radius:10px;text-decoration:none;font-weight:700;font-size:.95rem}");
  page += F(".onb{background:#fff;color:#16a34a;border:2px solid #22c55e}");
  page += F(".offb{background:#fff;color:#dc2626;border:2px solid #ef4444}");
  page += F(".active-on{background:#22c55e;color:#fff;border:2px solid #16a34a}");
  page += F(".active-off{background:#334155;color:#94a3b8;border:2px solid #475569;opacity:.7}");
  page += F("footer{text-align:center;color:#64748b;font-size:.75rem;padding:8px 16px 24px}");
  page += F("</style></head><body>");
  page += F("<header><h1>ESP8266 Multi-Load</h1>");
  page += F("<p>Onboard LED + 5 spare outputs</p></header>");
  page += F("<div class='grid'>");

  for (int i = 0; i < CHANNEL_COUNT; i++) {
    bool on = channels[i].on;
    page += F("<div class='card");
    if (on) page += F(" on");
    page += F("'><div class='name'><span class='dot'></span>");
    page += channels[i].name;
    page += F("</div><div class='pin'>");
    page += channels[i].label;
    page += F(" &middot; ");
    page += on ? F("ON") : F("OFF");
    page += F("</div><div class='row'>");

    // ON button
    page += F("<a class='btn ");
    if (on) page += F("active-on");
    else page += F("onb");
    page += F("' href='/set?id=");
    page += i;
    page += F("&state=on'>ON</a>");

    // OFF button
    page += F("<a class='btn ");
    if (!on) page += F("active-off");
    else page += F("offb");
    page += F("' href='/set?id=");
    page += i;
    page += F("&state=off'>OFF</a>");

    page += F("</div></div>");
  }

  page += F("</div>");
  page += F("<footer>SSID ");
  page += AP_SSID;
  page += F(" &middot; http://192.168.4.1 &middot; ALL OFF: <a style='color:#f87171' href='/all?state=off'>stop</a>");
  page += F("</footer></body></html>");
  return page;
}

void handleRoot() {
  server.send(200, "text/html", htmlPage());
}

void handleSet() {
  if (!server.hasArg("id") || !server.hasArg("state")) {
    server.send(400, "text/plain", "Need id and state");
    return;
  }
  int id = server.arg("id").toInt();
  String st = server.arg("state");
  st.toLowerCase();

  if (id < 0 || id >= CHANNEL_COUNT) {
    server.send(400, "text/plain", "Bad id");
    return;
  }
  if (st != "on" && st != "off") {
    server.send(400, "text/plain", "state must be on or off");
    return;
  }

  channels[id].on = (st == "on");
  applyChannel(id);

  Serial.print(F("CH"));
  Serial.print(id);
  Serial.print(F(" "));
  Serial.print(channels[id].name);
  Serial.print(F(" -> "));
  Serial.println(channels[id].on ? F("ON") : F("OFF"));

  server.sendHeader("Location", "/");
  server.send(303, "text/plain", "Redirect");
}

void handleAll() {
  String st = server.arg("state");
  st.toLowerCase();
  if (st == "off") {
    allOff();
    Serial.println(F("ALL -> OFF"));
    server.sendHeader("Location", "/");
    server.send(303, "text/plain", "Redirect");
    return;
  }
  server.send(400, "text/plain", "Use /all?state=off");
}

void handleStatus() {
  server.send(200, "application/json", jsonStatus());
}

void setup() {
  for (int i = 0; i < CHANNEL_COUNT; i++) {
    pinMode(channels[i].pin, OUTPUT);
  }
  allOff();

  Serial.begin(115200);
  delay(200);

  Serial.println();
  Serial.println(F("----------------------------------------"));
  Serial.println(F("ESP8266 Multi-Load Web Server v2.0"));
  Serial.println(F("----------------------------------------"));

  WiFi.mode(WIFI_AP);
  if (!WiFi.softAP(AP_SSID, AP_PASS)) {
    Serial.println(F("ERROR: softAP failed"));
  }

  server.on("/", HTTP_GET, handleRoot);
  server.on("/set", HTTP_GET, handleSet);
  server.on("/all", HTTP_GET, handleAll);
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
  Serial.println(F("Channels:"));
  for (int i = 0; i < CHANNEL_COUNT; i++) {
    Serial.print(F("  ["));
    Serial.print(i);
    Serial.print(F("] "));
    Serial.print(channels[i].name);
    Serial.print(F("  "));
    Serial.println(channels[i].label);
  }
  Serial.println(F("----------------------------------------"));
}

void loop() {
  server.handleClient();
}
