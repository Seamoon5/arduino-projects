/*
  ESP8266 Welcome - blink + WiFi scanner
  --------------------------------------
  What it does (no WiFi password needed):
  1. Blinks the built-in LED so you know the board is alive
  2. Scans for nearby WiFi networks every 10 seconds
  3. Prints results on the Serial Monitor at 115200 baud

  Board: most ESP8266 boards (NodeMCU, Wemos D1 mini, etc.)
  Upload with: esp8266:esp8266:nodemcuv2  (or d1_mini)
*/

#include <ESP8266WiFi.h>

// ESP8266 built-in LED is usually active-LOW (ON = LOW).
// LED_BUILTIN is defined by the board package (GPIO2 on NodeMCU/D1 mini).
#ifndef LED_BUILTIN
#define LED_BUILTIN 2
#endif

const unsigned long SCAN_INTERVAL_MS = 10000; // scan every 10 seconds
unsigned long lastScanMs = 0;
unsigned long lastBlinkMs = 0;
bool ledOn = false;

void printDivider(const char *title) {
  Serial.println();
  Serial.println(F("----------------------------------------"));
  Serial.println(title);
  Serial.println(F("----------------------------------------"));
}

void blinkOnce() {
  digitalWrite(LED_BUILTIN, LOW);  // ON
  delay(150);
  digitalWrite(LED_BUILTIN, HIGH); // OFF
  delay(150);
}

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH); // LED off at boot

  Serial.begin(115200);
  delay(200); // let USB serial settle

  printDivider("ESP8266 Welcome Sketch v1.0");
  Serial.print(F("Chip ID: "));
  Serial.println(ESP.getChipId());
  Serial.print(F("CPU MHz: "));
  Serial.println(F_CPU / 1000000);
  Serial.print(F("Free heap: "));
  Serial.println(ESP.getFreeHeap());

  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);

  printDivider("Ready - scanning WiFi...");
}

void scanWiFi() {
  printDivider("WiFi networks nearby");

  int n = WiFi.scanNetworks();
  if (n == 0) {
    Serial.println(F("No networks found."));
    return;
  }
  if (n < 0) {
    Serial.println(F("WiFi scan failed."));
    return;
  }

  Serial.print(F("Found "));
  Serial.print(n);
  Serial.println(F(" network(s):"));
  Serial.println();

  for (int i = 0; i < n; i++) {
    // SSID may contain spaces - print with index and signal bars
    Serial.print(i + 1);
    Serial.print(F(". "));
    Serial.print(WiFi.SSID(i));
    Serial.print(F("  |  RSSI "));
    Serial.print(WiFi.RSSI(i));
    Serial.print(F(" dBm  |  ch "));
    Serial.print(WiFi.channel(i));
    Serial.print(F("  |  "));
    Serial.println(WiFi.encryptionType(i) == ENC_TYPE_NONE ? F("OPEN") : F("secured"));
  }

  WiFi.scanDelete();
}

void loop() {
  // Fast blink while idle between scans (shows board is running)
  unsigned long now = millis();
  if (now - lastBlinkMs >= 500) {
    lastBlinkMs = now;
    ledOn = !ledOn;
    digitalWrite(LED_BUILTIN, ledOn ? LOW : HIGH);
  }

  if (now - lastScanMs >= SCAN_INTERVAL_MS || lastScanMs == 0) {
    lastScanMs = now;
    scanWiFi();
  }
}
