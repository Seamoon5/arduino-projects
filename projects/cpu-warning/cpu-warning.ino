/*
  CPU Warning Light
  -----------------
  Listens on Serial (9600 baud) for a CPU usage number, e.g. "67" or "CPU 67".

  When CPU spikes above 60%:
    1) LED turns FULL ON immediately
    2) Stays full ON for 5 seconds
    3) Fades OUT smoothly over 1.5 seconds
    4) If CPU is still above 60%, the cycle repeats

  Pin 13 (onboard LED) has NO hardware PWM on Uno, so fade uses software PWM.
  If no data arrives for 3 seconds, LED turns OFF (safe fallback).
*/

const int LED_PIN = LED_BUILTIN; // pin 13 on Uno
const int THRESHOLD = 60;        // percent CPU
const unsigned long HOLD_MS = 5000;         // full-on time after a spike
const unsigned long FADE_MS = 1500;         // fade-out time
const unsigned long SERIAL_TIMEOUT_MS = 3000;
const unsigned long PWM_PERIOD_MS = 20;     // ~50 Hz software PWM
const unsigned long REPEAT_GAP_MS = 150;    // pause before next cycle if still high

char lineBuf[32];
byte lineLen = 0;

enum WarnPhase {
  PHASE_OFF,
  PHASE_SOLID,   // full brightness for HOLD_MS
  PHASE_FADE,    // ramp down over FADE_MS
  PHASE_GAP      // brief pause before repeating while CPU still high
};

WarnPhase phase = PHASE_OFF;
unsigned long phaseStartMs = 0;
unsigned long lastDataMs = 0;
bool prevWarn = false;
bool sawData = false;

void setup() {
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);
  Serial.begin(9600);
  Serial.println(F("CPU-WARNING ready"));
}

void startSolid() {
  phase = PHASE_SOLID;
  phaseStartMs = millis();
  digitalWrite(LED_PIN, HIGH);
}

int parseCpuPercent(const char *buf) {
  const char *p = buf;
  while (*p) {
    if ((*p >= '0' && *p <= '9') || *p == '.') {
      return (int)atof(p);
    }
    p++;
  }
  return -1;
}

void handleLine(char *buf) {
  int cpu = parseCpuPercent(buf);
  if (cpu < 0) return;

  lastDataMs = millis();
  sawData = true;

  bool warn = cpu > THRESHOLD;
  bool rising = warn && !prevWarn;
  prevWarn = warn;

  // New spike (OK -> WARN): always start a fresh 5s + fade cycle
  if (rising) {
    startSolid();
    Serial.print(F("CPU="));
    Serial.print(cpu);
    Serial.println(F(" SPIKE - LED ON 5s then fade"));
    return;
  }

  Serial.print(F("CPU="));
  Serial.print(cpu);
  Serial.println(warn ? F(" WARN") : F(" OK"));
}

void serviceLed(unsigned long now) {
  switch (phase) {
    case PHASE_OFF:
      digitalWrite(LED_PIN, LOW);
      // CPU still high after a finished cycle: repeat warning
      if (prevWarn && sawData && (now - lastDataMs) < SERIAL_TIMEOUT_MS) {
        startSolid();
      }
      break;

    case PHASE_SOLID:
      digitalWrite(LED_PIN, HIGH);
      if (now - phaseStartMs >= HOLD_MS) {
        phase = PHASE_FADE;
        phaseStartMs = now;
        Serial.println(F("Fade out..."));
      }
      break;

    case PHASE_FADE: {
      unsigned long elapsed = now - phaseStartMs;
      if (elapsed >= FADE_MS) {
        digitalWrite(LED_PIN, LOW);
        phase = prevWarn ? PHASE_GAP : PHASE_OFF;
        phaseStartMs = now;
        if (!prevWarn) {
          Serial.println(F("Fade done LED=OFF"));
        }
        break;
      }
      // Brightness 255 -> 0 over FADE_MS
      int bright = 255 - (int)((255UL * elapsed) / FADE_MS);
      unsigned long onTime = ((unsigned long)bright * PWM_PERIOD_MS) / 255;
      unsigned long periodPos = now % PWM_PERIOD_MS;
      digitalWrite(LED_PIN, (periodPos < onTime) ? HIGH : LOW);
      break;
    }

    case PHASE_GAP:
      digitalWrite(LED_PIN, LOW);
      if (!prevWarn) {
        phase = PHASE_OFF;
      } else if (now - phaseStartMs >= REPEAT_GAP_MS) {
        startSolid(); // still spiked: start next 5s cycle
      }
      break;
  }
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

  // Read time AFTER serial so lastDataMs is never "in the future"
  unsigned long now = millis();

  // Safe fallback: PC stopped sending -> LED off
  if (sawData && (now - lastDataMs > SERIAL_TIMEOUT_MS) && phase != PHASE_OFF) {
    phase = PHASE_OFF;
    prevWarn = false;
    digitalWrite(LED_PIN, LOW);
    Serial.println(F("TIMEOUT LED=OFF"));
  }

  serviceLed(now);
}
