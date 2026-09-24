/*
  ESP8266 Anywhere Control v3.0
  -----------------------------
  Control onboard LED + 5 loads from ANYWHERE via Blynk Cloud,
  plus a local fallback page on your home network.

  Blynk Template : Quickstart Template
  Template ID    : TMPL6Hkyu5N7f

  Phone (worldwide):
    Blynk app -> device "Anywhere Loads" -> switches ON/OFF

  Phone (home network, local page):
    http://<board-ip>   (board IP is printed on Serial at 115200)

  Channels (same wiring as v2.0):
    V0/0  LED    GPIO2   (built-in, active-LOW)
    V1/1  LOAD1  D1  GPIO5
    V2/2  LOAD2  D2  GPIO4
    V3/3  LOAD3  D5  GPIO14
    V4/4  LOAD4  D6  GPIO12
    V5/5  LOAD5  D7  GPIO13
    V6        -> emergency ALL OFF button

  Safety: outputs are 3.3V logic only. Never drive mains/motors direct.

  FQBN: esp8266:esp8266:nodemcuv2
*/

#define BLYNK_TEMPLATE_ID           "TMPL6Hkyu5N7f"
#define BLYNK_TEMPLATE_NAME         "Quickstart Template"
#define BLYNK_FIRMWARE_VERSION      "3.0.0"

#define BLYNK_PRINT Serial
#define APP_DEBUG

#include "BlynkEdgent.h"
#include <ESP8266WebServer.h>

const int CHANNEL_COUNT = 6;
const int PIN_ALL_OFF   = 6;   // V6 = emergency all off

struct Channel {
  const char *name;
  const char *label;
  uint8_t pin;
  bool activeLow;
  bool on;
};

Channel channels[CHANNEL_COUNT] = {
  { "LED",   "Onboard  GPIO2",  2,  true,  false },
  { "LOAD1", "D1  GPIO5",       5,  false, false },
  { "LOAD2", "D2  GPIO4",       4,  false, false },
  { "LOAD3", "D5  GPIO14",     14,  false, false },
  { "LOAD4", "D6  GPIO12",     12,  false, false },
  { "LOAD5", "D7  GPIO13",     13,  false, false },
};

// Local fallback web server (port 80). Blynk.Edgent also uses port 80 during
// provisioning only, so we start ours after the board reaches RUNNING state
// and stop it again before Edgent can claim the port.
ESP8266WebServer web(80);
bool webRunning = false;

/* ---------------- channel control ---------------- */

void applyChannel(int i) {
  if (channels[i].activeLow) {
    digitalWrite(channels[i].pin, channels[i].on ? LOW : HIGH);
  } else {
    digitalWrite(channels[i].pin, channels[i].on ? HIGH : LOW);
  }
}

void setChannel(int i, bool on) {
  if (i < 0 || i >= CHANNEL_COUNT) return;
  if (channels[i].on == on) return;
  channels[i].on = on;
  applyChannel(i);
  Serial.print(F("CH")); Serial.print(i); Serial.print(F(" "));
  Serial.print(channels[i].name); Serial.print(F(" -> "));
  Serial.println(on ? F("ON") : F("OFF"));
  // keep the app widgets in sync
  if (Blynk.connected()) Blynk.virtualWrite(i, on ? 1 : 0);
}

void allOff() {
  for (int i = 0; i < CHANNEL_COUNT; i++) {
    channels[i].on = false;
    applyChannel(i);
  }
  if (Blynk.connected()) {
    for (int i = 0; i < CHANNEL_COUNT; i++) Blynk.virtualWrite(i, 0);
    Blynk.virtualWrite(PIN_ALL_OFF, 0);
  }
  Serial.println(F("ALL -> OFF"));
}

void syncAllToBlynk() {
  for (int i = 0; i < CHANNEL_COUNT; i++) {
    Blynk.virtualWrite(i, channels[i].on ? 1 : 0);
  }
}

/* ---------------- Blynk handlers ---------------- */

BLYNK_CONNECTED() {
  syncAllToBlynk();
}

BLYNK_WRITE(V0) { setChannel(0, param.asInt()); }
BLYNK_WRITE(V1) { setChannel(1, param.asInt()); }
BLYNK_WRITE(V2) { setChannel(2, param.asInt()); }
BLYNK_WRITE(V3) { setChannel(3, param.asInt()); }
BLYNK_WRITE(V4) { setChannel(4, param.asInt()); }
BLYNK_WRITE(V5) { setChannel(5, param.asInt()); }

BLYNK_WRITE(V6) {
  if (param.asInt()) {
    allOff();
    Blynk.logEvent("load_changed", "EMERGENCY ALL OFF pressed");
  }
}

/* ---------------- local fallback page ---------------- */

String jsonStatus() {
  String j = F("{\"channels\":[");
  for (int i = 0; i < CHANNEL_COUNT; i++) {
    if (i) j += F(",");
    j += F("{\"id\":"); j += i;
    j += F(",\"name\":\""); j += channels[i].name;
    j += F("\",\"pin\":\""); j += channels[i].label;
    j += F("\",\"state\":\""); j += channels[i].on ? F("on") : F("off");
    j += F("\"}");
  }
  j += F("],\"blynk\":\""); j += Blynk.connected() ? F("online") : F("offline");
  j += F("\",\"ip\":\""); j += WiFi.localIP().toString();
  j += F("\"}");
  return j;
}

String htmlPage() {
  String p;
  p += F("<!DOCTYPE html><html><head><meta charset='utf-8'>");
  p += F("<meta name='viewport' content='width=device-width,initial-scale=1'>");
  p += F("<title>Anywhere Loads</title><style>");
  p += F("body{font-family:system-ui,sans-serif;margin:0;background:#0f172a;color:#e2e8f0}");
  p += F("header{padding:20px 16px 8px;text-align:center}");
  p += F("h1{font-size:1.25rem;margin:0}p{color:#94a3b8;margin:6px 0 0;font-size:.9rem}");
  p += F(".grid{display:grid;grid-template-columns:1fr 1fr;gap:12px;padding:16px;max-width:480px;margin:0 auto}");
  p += F(".card{background:#1e293b;border:1px solid #334155;border-radius:14px;padding:14px}");
  p += F(".card.on{border-color:#22c55e;box-shadow:0 0 16px rgba(34,197,94,.15)}");
  p += F(".name{font-weight:700;font-size:1.05rem;display:flex;align-items:center;gap:8px}");
  p += F(".dot{width:10px;height:10px;border-radius:50%;background:#475569}");
  p += F(".card.on .dot{background:#22c55e;box-shadow:0 0 8px #22c55e}");
  p += F(".pin{color:#94a3b8;font-size:.75rem;margin:4px 0 12px}");
  p += F(".row{display:flex;gap:8px}");
  p += F("a.btn{flex:1;text-align:center;padding:10px 0;border-radius:10px;text-decoration:none;font-weight:700;font-size:.95rem}");
  p += F(".onb{background:#fff;color:#16a34a;border:2px solid #22c55e}");
  p += F(".offb{background:#fff;color:#dc2626;border:2px solid #ef4444}");
  p += F(".active-on{background:#22c55e;color:#fff;border:2px solid #16a34a}");
  p += F(".active-off{background:#334155;color:#94a3b8;border:2px solid #475569;opacity:.7}");
  p += F(".stop{display:block;max-width:480px;margin:4px auto 0;padding:14px 0;");
  p += F("background:#dc2626;color:#fff;border-radius:14px;text-decoration:none;");
  p += F("font-weight:800;text-align:center;letter-spacing:.05em}");
  p += F("footer{text-align:center;color:#64748b;font-size:.75rem;padding:8px 16px 24px}");
  p += F("</style></head><body>");
  p += F("<header><h1>Anywhere Loads</h1>");
  p += F("<p>Blynk cloud + local control</p></header>");
  p += F("<div class='grid'>");

  for (int i = 0; i < CHANNEL_COUNT; i++) {
    bool on = channels[i].on;
    p += F("<div class='card"); if (on) p += F(" on");
    p += F("'><div class='name'><span class='dot'></span>");
    p += channels[i].name;
    p += F("</div><div class='pin'>");
    p += channels[i].label;
    p += F(" &middot; "); p += on ? F("ON") : F("OFF");
    p += F("</div><div class='row'>");
    p += F("<a class='btn "); p += on ? F("active-on") : F("onb");
    p += F("' href='/set?id="); p += i; p += F("&state=on'>ON</a>");
    p += F("<a class='btn "); p += !on ? F("active-off") : F("offb");
    p += F("' href='/set?id="); p += i; p += F("&state=off'>OFF</a>");
    p += F("</div></div>");
  }

  p += F("</div>");
  p += F("<a class='stop' href='/all?state=off'>ALL OFF</a>");
  p += F("<footer>Blynk: ");
  p += Blynk.connected() ? F("online") : F("offline");
  p += F(" &middot; http://"); p += WiFi.localIP().toString();
  p += F("</footer></body></html>");
  return p;
}

void handleRoot()      { web.send(200, "text/html", htmlPage()); }
void handleStatus()    { web.send(200, "application/json", jsonStatus()); }

void handleSet() {
  if (!web.hasArg("id") || !web.hasArg("state")) {
    web.send(400, "text/plain", "Need id and state"); return;
  }
  int id = web.arg("id").toInt();
  String st = web.arg("state"); st.toLowerCase();
  if (id < 0 || id >= CHANNEL_COUNT)     { web.send(400, "text/plain", "Bad id"); return; }
  if (st != "on" && st != "off")         { web.send(400, "text/plain", "state must be on or off"); return; }

  setChannel(id, st == "on");
  web.sendHeader("Location", "/");
  web.send(303, "text/plain", "Redirect");
}

void handleAll() {
  String st = web.arg("state"); st.toLowerCase();
  if (st == "off") {
    allOff();
    web.sendHeader("Location", "/");
    web.send(303, "text/plain", "Redirect");
    return;
  }
  web.send(400, "text/plain", "Use /all?state=off");
}

void webBegin() {
  web.on("/", HTTP_GET, handleRoot);
  web.on("/set", HTTP_GET, handleSet);
  web.on("/all", HTTP_GET, handleAll);
  web.on("/status", HTTP_GET, handleStatus);
  web.onNotFound([]() { web.send(404, "text/plain", "Not found. Open /"); });
  web.begin();
  webRunning = true;
  Serial.print(F("[local] page ready at http://"));
  Serial.println(WiFi.localIP());
}

void webStop() {
  web.stop();
  webRunning = false;
}

/* ---------------- WiFi connect diagnostics ---------------- */

const char* wifiStatusText(int s) {
  switch (s) {
    case 0:  return "IDLE - starting";
    case 1:  return "SSID NOT FOUND - network not visible";
    case 3:  return "CONNECTED";
    case 4:  return "CONNECT FAILED";
    case 5:  return "CONNECTION LOST";
    case 6:  return "WRONG PASSWORD";
    case 7:  return "DISCONNECTED - not associated";
    default: return "UNKNOWN";
  }
}

// Runs from edgentTimer, which keeps ticking inside Blynk's blocking
// connect wait - so this prints every 5 s while the board is still trying.
void printWifiStatus() {
  if (BlynkState::is(MODE_RUNNING)) return;
  int s = WiFi.status();
  Serial.print(F("[wifi] status="));
  Serial.print(s);
  Serial.print(F(" ("));
  Serial.print(wifiStatusText(s));
  Serial.print(F(") try="));
  Serial.println(WiFi.SSID());
}

/* ---------------- setup / loop ---------------- */

void setup() {
  // Safe boot: every load OFF before anything else runs.
  for (int i = 0; i < CHANNEL_COUNT; i++) {
    pinMode(channels[i].pin, OUTPUT);
  }
  allOff();

  Serial.begin(115200);
  delay(200);
  Serial.println();
  Serial.println(F("--------------------------------------------"));
  Serial.println(F(" ESP8266 Anywhere Control v3.0"));
  Serial.println(F(" Template: Quickstart Template"));
  Serial.println(F("--------------------------------------------"));

  BlynkEdgent.begin();
  edgentTimer.setInterval(5000L, printWifiStatus);
}

void loop() {
  // Local page only owns port 80 while Blynk is RUNNING.
  if (BlynkState::is(MODE_RUNNING)) {
    if (!webRunning) webBegin();
    web.handleClient();
  } else if (webRunning) {
    webStop();
  }

  BlynkEdgent.run();
}
