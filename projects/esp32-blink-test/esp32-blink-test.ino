#include <Arduino.h>

const unsigned long HEARTBEAT_INTERVAL_MS = 1000;

#if defined(LED_BUILTIN)
const int TEST_LED_PIN = LED_BUILTIN;
#else
const int TEST_LED_PIN = -1;
#endif

void setup() {
  Serial.begin(115200);
  delay(1000);

  if (TEST_LED_PIN >= 0) {
    pinMode(TEST_LED_PIN, OUTPUT);
    digitalWrite(TEST_LED_PIN, HIGH);
  }

  Serial.println();
  Serial.println(F("ESP32 blink test v1.0"));
  Serial.print(F("Chip model: "));
  Serial.println(ESP.getChipModel());
  Serial.print(F("CPU MHz: "));
  Serial.println(ESP.getCpuFreqMHz());
  Serial.print(F("Free heap: "));
  Serial.println(ESP.getFreeHeap());
  Serial.print(F("LED test: "));
  if (TEST_LED_PIN >= 0) {
    Serial.println(TEST_LED_PIN);
  } else {
    Serial.println(F("not defined; serial heartbeat only"));
  }
}

void loop() {
  static unsigned long lastUpdateMs = 0;
  static bool ledOn = false;
  const unsigned long nowMs = millis();

  if (nowMs - lastUpdateMs < HEARTBEAT_INTERVAL_MS) {
    return;
  }
  lastUpdateMs = nowMs;

  if (TEST_LED_PIN >= 0) {
    ledOn = !ledOn;
    digitalWrite(TEST_LED_PIN, ledOn ? LOW : HIGH);
  }

  Serial.print(F("ESP32 alive "));
  Serial.print(nowMs / 1000);
  Serial.println(F("s"));
}
