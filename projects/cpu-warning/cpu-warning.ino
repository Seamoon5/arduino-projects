/*
  CPU Warning Light
  -----------------
  Listens on Serial (9600 baud) for a CPU usage number, e.g. "67" or "CPU 67".
  If CPU usage > 60%, turns the onboard LED (pin 13) ON.
  If CPU usage <= 60%, turns the LED OFF.
  If no data arrives for 3 seconds, turns the LED OFF (safe fallback).
*/

const int LED_PIN = LED_BUILTIN; // pin 13 on Uno
const int THRESHOLD = 60;        // percent CPU
const unsigned long SERIAL_TIMEOUT_MS = 3000;

char lineBuf[32];
byte lineLen = 0;
unsigned long lastDataMs = 0;
bool ledState = false;

void setup() {
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);
  Serial.begin(9600);
  Serial.println(F("CPU-WARNING ready"));
}

void setLed(bool on) {
  if (on == ledState) return;
  ledState = on;
  digitalWrite(LED_PIN, on ? HIGH : LOW);
}

int parseCpuPercent(const char *buf) {
  // Accept: "67", "67.5", "CPU 67", "cpu:67"
  const char *p = buf;
  while (*p) {
    if ((*p >= '0' && *p <= '9') || *p == '.') {
      return (int)atof(p); // stop at first number found
    }
    p++;
  }
  return -1;
}

void handleLine(char *buf) {
  int cpu = parseCpuPercent(buf);
  if (cpu < 0) return;
  lastDataMs = millis();

  bool warn = cpu > THRESHOLD;
  setLed(warn);

  Serial.print(F("CPU="));
  Serial.print(cpu);
  Serial.println(warn ? F(" WARN") : F(" OK"));
}

void loop() {
  while (Serial.available()) {
    char c = Serial.read();
    if (c == '\n' || c == '\r') {
      if (lineLen > 0) {
        lineBuf[lineLen] = '\0';
        handleLine(lineBuf);
        lineLen = 0;
      }
    } else if (lineLen < sizeof(lineBuf) - 1) {
      lineBuf[lineLen++] = c;
    }
  }

  // Safe fallback: if PC stops sending, LED off after timeout
  if (ledState && millis() - lastDataMs > SERIAL_TIMEOUT_MS) {
    setLed(false);
    Serial.println(F("TIMEOUT LED=OFF"));
  }
}
