#include <EEPROM.h>

const uint8_t MAGIC[4] = {0x4B, 0x4C, 0x47, 0x01};
const unsigned int EE_LEN_ADDR = 4;
const unsigned int EE_DATA_ADDR = 6;
const unsigned int MAX_LOG = 1024 - EE_DATA_ADDR;
const int LED_PIN = 13;

uint16_t logLen = 0;
bool paused = false;
bool fullWarned = false;
bool ledFlashOn = false;
bool pauseBlinkState = false;
unsigned long ledFlashAt = 0;
unsigned long pauseBlinkAt = 0;
String lineBuf;

void setup() {
  pinMode(LED_PIN, OUTPUT);
  Serial.begin(9600);
  loadLog();
  Serial.println(F("=== Arduino Key Logger v1.0 ==="));
  Serial.print(F("Stored: "));
  Serial.print(logLen);
  Serial.println(F(" chars"));
  Serial.println(F("Commands: help, read, count, find <text>, clear, pause, resume, status"));
  Serial.println(F("Tip: close this monitor before running Key Logger.bat"));
}

void loop() {
  while (Serial.available() > 0) {
    char c = Serial.read();
    if (c == '\n') {
      handleLine(lineBuf);
      lineBuf = "";
    } else if (c != '\r' && lineBuf.length() < 200) {
      lineBuf += c;
    }
  }
  updateLed();
}

void loadLog() {
  bool ok = true;
  for (uint8_t i = 0; i < 4; i++) {
    if (EEPROM.read(i) != MAGIC[i]) {
      ok = false;
    }
  }
  if (!ok) {
    for (uint8_t i = 0; i < 4; i++) {
      EEPROM.update(i, MAGIC[i]);
    }
    logLen = 0;
    saveLen();
    return;
  }
  logLen = (uint16_t)EEPROM.read(EE_LEN_ADDR) | ((uint16_t)EEPROM.read(EE_LEN_ADDR + 1) << 8);
  if (logLen > MAX_LOG) {
    logLen = 0;
    saveLen();
  }
}

void saveLen() {
  EEPROM.update(EE_LEN_ADDR, logLen & 0xFF);
  EEPROM.update(EE_LEN_ADDR + 1, (logLen >> 8) & 0xFF);
}

void handleLine(const String &line) {
  if (line.length() == 0) {
    return;
  }
  if (line.charAt(0) == '+') {
    if (paused) {
      if (!fullWarned) {
        Serial.println(F("PAUSED"));
        fullWarned = true;
      }
      return;
    }
    appendPayload(line.substring(1));
    return;
  }

  String cmd = line;
  cmd.trim();
  String lower = cmd;
  lower.toLowerCase();

  if (lower == "help" || lower == "?") {
    printHelp();
  } else if (lower == "read") {
    dumpLog();
  } else if (lower == "count") {
    Serial.print(F("COUNT "));
    Serial.println(logLen);
  } else if (lower == "clear") {
    clearLog();
  } else if (lower == "pause") {
    paused = true;
    fullWarned = false;
    Serial.println(F("PAUSED"));
  } else if (lower == "resume") {
    paused = false;
    fullWarned = false;
    Serial.println(F("RESUMED"));
  } else if (lower == "status") {
    printStatus();
  } else if (lower.startsWith("find ")) {
    findInLog(cmd.substring(5));
  } else {
    Serial.print(F("Unknown command: "));
    Serial.println(cmd);
  }
}

void printHelp() {
  Serial.println(F("--- Commands ---"));
  Serial.println(F("  read         show all stored keys"));
  Serial.println(F("  count        number of stored chars"));
  Serial.println(F("  find <text>  search the log"));
  Serial.println(F("  clear        erase the log"));
  Serial.println(F("  pause        stop accepting new keys"));
  Serial.println(F("  resume       start accepting keys"));
  Serial.println(F("  status       paused/full/count"));
  Serial.println(F("  help         this list"));
}

void printStatus() {
  Serial.print(F("STATUS count="));
  Serial.print(logLen);
  Serial.print(F(" max="));
  Serial.print(MAX_LOG);
  Serial.print(F(" state="));
  if (paused) {
    Serial.print(F("PAUSED"));
  } else if (logLen >= MAX_LOG) {
    Serial.print(F("FULL"));
  } else {
    Serial.print(F("LOGGING"));
  }
  Serial.println();
}

void dumpLog() {
  Serial.println(F("--- BEGIN ---"));
  for (unsigned int i = 0; i < logLen; i++) {
    Serial.write(EEPROM.read(EE_DATA_ADDR + i));
  }
  Serial.println();
  Serial.println(F("--- END ---"));
  Serial.print(F("COUNT "));
  Serial.println(logLen);
}

void clearLog() {
  logLen = 0;
  saveLen();
  fullWarned = false;
  Serial.println(F("CLEARED"));
}

void findInLog(const String &needle) {
  if (needle.length() == 0) {
    Serial.println(F("FIND what? usage: find hello"));
    return;
  }
  unsigned int matches = 0;
  int first = -1;
  unsigned int nLen = needle.length();
  if (nLen > logLen) {
    Serial.println(F("NOT FOUND"));
    return;
  }
  for (unsigned int i = 0; i + nLen <= logLen; i++) {
    bool ok = true;
    for (unsigned int j = 0; j < nLen; j++) {
      if (EEPROM.read(EE_DATA_ADDR + i + j) != (uint8_t)needle.charAt(j)) {
        ok = false;
        break;
      }
    }
    if (ok) {
      matches++;
      if (first < 0) {
        first = (int)i;
      }
    }
  }
  if (matches == 0) {
    Serial.println(F("NOT FOUND"));
  } else {
    Serial.print(F("FOUND "));
    Serial.print(matches);
    Serial.print(F(" time(s), first at "));
    Serial.println(first);
  }
}

void appendPayload(const String &payload) {
  for (unsigned int i = 0; i < payload.length(); i++) {
    char c = payload.charAt(i);
    if (c == '\\' && i + 1 < payload.length()) {
      char n = payload.charAt(i + 1);
      if (n == 'n') {
        appendByte('\n');
        i++;
      } else if (n == 't') {
        appendByte('\t');
        i++;
      } else if (n == 'b') {
        backspaceLog();
        i++;
      } else if (n == '\\') {
        appendByte('\\');
        i++;
      } else {
        appendByte(c);
      }
    } else {
      appendByte(c);
    }
  }
}

void backspaceLog() {
  if (logLen == 0) {
    return;
  }
  logLen--;
  saveLen();
  fullWarned = false;
}

void appendByte(uint8_t b) {
  if (logLen >= MAX_LOG) {
    if (!fullWarned) {
      Serial.println(F("FULL"));
      fullWarned = true;
    }
    return;
  }
  EEPROM.update(EE_DATA_ADDR + logLen, b);
  logLen++;
  saveLen();
  flashLed();
}

void flashLed() {
  digitalWrite(LED_PIN, HIGH);
  ledFlashOn = true;
  ledFlashAt = millis();
}

void updateLed() {
  unsigned long now = millis();
  if (logLen >= MAX_LOG) {
    digitalWrite(LED_PIN, HIGH);
    ledFlashOn = false;
    return;
  }
  if (ledFlashOn) {
    if (now - ledFlashAt >= 60) {
      ledFlashOn = false;
      if (!paused) {
        digitalWrite(LED_PIN, LOW);
      }
    }
    return;
  }
  if (paused) {
    if (now - pauseBlinkAt >= 500) {
      pauseBlinkAt = now;
      pauseBlinkState = !pauseBlinkState;
      digitalWrite(LED_PIN, pauseBlinkState ? HIGH : LOW);
    }
  } else {
    digitalWrite(LED_PIN, LOW);
  }
}
