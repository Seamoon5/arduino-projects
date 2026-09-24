#include <EEPROM.h>
#include <stdlib.h>
#include <string.h>

#include "config.h"

Settings activeSettings = {};
char commandBuffer[COMMAND_BUFFER_SIZE] = {};
uint8_t commandLength = 0;
bool commandOverflow = false;
unsigned long lastCommandByteTime = 0;

unsigned long lastPwmStepTime = 0;
unsigned long pwmElapsed = 0;
uint8_t pwmStep = 0;
uint8_t pwmDutySteps = 0;

unsigned long lastBlinkChange = 0;
unsigned long lastFadeChange = 0;
bool blinkOn = false;
uint8_t fadeValue = 0;
int8_t fadeStep = FADE_VALUE_STEP;

uint16_t mixByte(uint16_t value, uint8_t byte);
uint16_t calculateChecksum(const Settings &settings);
uint16_t readUint16(int address);
void writeUint16(int address, uint16_t value);
bool isValidSettings(const Settings &settings);
Settings makeDefaultSettings();
void persistSettings();
void loadSettings();
void printMode();
void printSettings();
void printHelp();
void setPwmDuty(uint8_t dutySteps);
uint8_t brightnessToPwmSteps(uint8_t brightness);
uint8_t fadeToPwmSteps(uint8_t value);
unsigned long getBlinkInterval();
void applyModeOutput();
void resetAnimation();
void servicePwm(unsigned long now);
void updateLed(unsigned long now);
bool parseBoundedInt(const char *text, int minimum, int maximum, int &value);
void changeMode(LedMode mode);
void changeSpeed(int speed);
void changeBrightness(int brightness);
void factoryReset();
bool applyModeText(const char *text);
void processCommand(const char *command);
void serviceSerial(unsigned long now);

void setup() {
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);
  Serial.begin(SERIAL_BAUD);

  Serial.println(F("=== EEPROM Settings Menu ==="));
  loadSettings();
  printSettings();
  printHelp();
  Serial.println(F("Type a command and press Enter."));
}

void loop() {
  unsigned long now = millis();
  serviceSerial(now);
  updateLed(now);
}

uint16_t mixByte(uint16_t value, uint8_t byte) {
  value ^= byte;
  return static_cast<uint16_t>((value << 1) | (value >> 15));
}

uint16_t calculateChecksum(const Settings &settings) {
  uint16_t checksum = CHECKSUM_SEED;
  checksum = mixByte(checksum, static_cast<uint8_t>(settings.magic & 0xFF));
  checksum = mixByte(checksum, static_cast<uint8_t>(settings.magic >> 8));
  checksum = mixByte(checksum, settings.version);
  checksum = mixByte(checksum, settings.mode);
  checksum = mixByte(checksum, settings.speed);
  checksum = mixByte(checksum, settings.brightness);
  return checksum;
}

uint16_t readUint16(int address) {
  uint16_t low = EEPROM.read(address);
  uint16_t high = EEPROM.read(address + 1);
  return static_cast<uint16_t>(low | (high << 8));
}

void writeUint16(int address, uint16_t value) {
  EEPROM.update(address, static_cast<uint8_t>(value & 0xFF));
  EEPROM.update(address + 1, static_cast<uint8_t>((value >> 8) & 0xFF));
}

bool isValidSettings(const Settings &settings) {
  return settings.magic == EEPROM_MAGIC &&
         settings.version == EEPROM_VERSION &&
         settings.mode <= LED_MODE_FADE &&
         settings.speed >= MIN_SPEED &&
         settings.speed <= MAX_SPEED &&
         settings.brightness <= MAX_BRIGHTNESS &&
         settings.checksum == calculateChecksum(settings);
}

Settings makeDefaultSettings() {
  Settings settings = {};
  settings.magic = EEPROM_MAGIC;
  settings.version = EEPROM_VERSION;
  settings.mode = LED_MODE_BLINK;
  settings.speed = 5;
  settings.brightness = 80;
  settings.checksum = calculateChecksum(settings);
  return settings;
}

void persistSettings() {
  activeSettings.checksum = calculateChecksum(activeSettings);
  writeUint16(EEPROM_MAGIC_ADDRESS, activeSettings.magic);
  EEPROM.update(EEPROM_VERSION_ADDRESS, activeSettings.version);
  EEPROM.update(EEPROM_MODE_ADDRESS, activeSettings.mode);
  EEPROM.update(EEPROM_SPEED_ADDRESS, activeSettings.speed);
  EEPROM.update(EEPROM_BRIGHTNESS_ADDRESS, activeSettings.brightness);
  writeUint16(EEPROM_CHECKSUM_ADDRESS, activeSettings.checksum);
}

void loadSettings() {
  Settings loaded = {};
  loaded.magic = readUint16(EEPROM_MAGIC_ADDRESS);
  loaded.version = EEPROM.read(EEPROM_VERSION_ADDRESS);
  loaded.mode = EEPROM.read(EEPROM_MODE_ADDRESS);
  loaded.speed = EEPROM.read(EEPROM_SPEED_ADDRESS);
  loaded.brightness = EEPROM.read(EEPROM_BRIGHTNESS_ADDRESS);
  loaded.checksum = readUint16(EEPROM_CHECKSUM_ADDRESS);

  if (isValidSettings(loaded)) {
    activeSettings = loaded;
    Serial.println(F("EEPROM: valid settings loaded"));
  } else {
    activeSettings = makeDefaultSettings();
    persistSettings();
    Serial.println(F("EEPROM: blank or invalid; defaults restored"));
  }

  resetAnimation();
}

void printMode() {
  switch (static_cast<LedMode>(activeSettings.mode)) {
    case LED_MODE_OFF:
      Serial.println(F("off"));
      break;
    case LED_MODE_ON:
      Serial.println(F("on"));
      break;
    case LED_MODE_BLINK:
      Serial.println(F("blink"));
      break;
    case LED_MODE_FADE:
      Serial.println(F("fade"));
      break;
    default:
      Serial.println(F("invalid"));
      break;
  }
}

void printSettings() {
  Serial.println(F("--- Saved settings ---"));
  Serial.print(F("mode="));
  printMode();
  Serial.print(F("speed="));
  Serial.println(activeSettings.speed);
  Serial.print(F("brightness="));
  Serial.println(activeSettings.brightness);
  Serial.print(F("checksum=0x"));
  Serial.println(activeSettings.checksum, HEX);
}

void printHelp() {
  Serial.println(F("Commands (press Enter after each command):"));
  Serial.println(F("  help                 Show this help"));
  Serial.println(F("  show                 Show saved settings"));
  Serial.println(F("  mode off|on|blink|fade"));
  Serial.println(F("  speed 1..9           Set blink speed"));
  Serial.println(F("  brightness 0..100    Set LED brightness"));
  Serial.println(F("  save                 Save current settings"));
  Serial.println(F("  load                 Reload settings from EEPROM"));
  Serial.println(F("  reset                Restore factory defaults"));
  Serial.println(F("  o/n/b/f              Quick off/on/blink/fade"));
  Serial.println(F("  h/?                  Show help"));
}

void setPwmDuty(uint8_t dutySteps) {
  if (dutySteps > PWM_STEPS) {
    dutySteps = PWM_STEPS;
  }
  pwmDutySteps = dutySteps;
}

uint8_t brightnessToPwmSteps(uint8_t brightness) {
  if (brightness == 0) {
    return 0;
  }

  uint16_t steps = (static_cast<uint16_t>(brightness) * PWM_STEPS + 50U) / 100U;
  if (steps == 0) {
    steps = 1;
  }
  if (steps > PWM_STEPS) {
    steps = PWM_STEPS;
  }
  return static_cast<uint8_t>(steps);
}

uint8_t fadeToPwmSteps(uint8_t value) {
  uint16_t steps = (static_cast<uint16_t>(value) * PWM_STEPS + 127U) / 255U;
  return static_cast<uint8_t>(steps);
}

unsigned long getBlinkInterval() {
  unsigned long range = BLINK_INTERVAL_MAX_MS - BLINK_INTERVAL_MIN_MS;
  unsigned long speedOffset = activeSettings.speed - MIN_SPEED;
  return BLINK_INTERVAL_MAX_MS - (speedOffset * range) / (MAX_SPEED - MIN_SPEED);
}

void applyModeOutput() {
  switch (static_cast<LedMode>(activeSettings.mode)) {
    case LED_MODE_OFF:
      setPwmDuty(0);
      break;
    case LED_MODE_ON:
      setPwmDuty(brightnessToPwmSteps(activeSettings.brightness));
      break;
    case LED_MODE_BLINK:
      setPwmDuty(blinkOn ? brightnessToPwmSteps(activeSettings.brightness) : 0);
      break;
    case LED_MODE_FADE:
      setPwmDuty(fadeToPwmSteps(fadeValue));
      break;
    default:
      setPwmDuty(0);
      break;
  }
}

void resetAnimation() {
  blinkOn = false;
  fadeValue = 0;
  fadeStep = FADE_VALUE_STEP;
  lastBlinkChange = millis();
  lastFadeChange = millis();
  pwmStep = 0;
  pwmElapsed = 0;
  lastPwmStepTime = millis();
  digitalWrite(LED_PIN, LOW);
  applyModeOutput();
}

void servicePwm(unsigned long now) {
  if (now - lastPwmStepTime < PWM_STEP_MS) {
    return;
  }

  unsigned long elapsed = now - lastPwmStepTime;
  lastPwmStepTime = now;
  pwmElapsed += elapsed;

  while (pwmElapsed >= PWM_STEP_MS) {
    pwmElapsed -= PWM_STEP_MS;
    pwmStep++;
    if (pwmStep >= PWM_STEPS) {
      pwmStep = 0;
    }
    digitalWrite(LED_PIN, pwmStep < pwmDutySteps ? HIGH : LOW);
  }
}

void updateLed(unsigned long now) {
  switch (static_cast<LedMode>(activeSettings.mode)) {
    case LED_MODE_OFF:
      setPwmDuty(0);
      break;
    case LED_MODE_ON:
      setPwmDuty(brightnessToPwmSteps(activeSettings.brightness));
      break;
    case LED_MODE_BLINK:
      if (now - lastBlinkChange >= getBlinkInterval()) {
        lastBlinkChange = now;
        blinkOn = !blinkOn;
        setPwmDuty(blinkOn ? brightnessToPwmSteps(activeSettings.brightness) : 0);
      }
      break;
    case LED_MODE_FADE:
      if (now - lastFadeChange >= FADE_STEP_INTERVAL_MS) {
        lastFadeChange = now;
        if (fadeValue >= 255) {
          fadeValue = 255;
          fadeStep = -FADE_VALUE_STEP;
        } else if (fadeValue <= 0) {
          fadeValue = 0;
          fadeStep = FADE_VALUE_STEP;
        }
        fadeValue = static_cast<uint8_t>(static_cast<int>(fadeValue) + fadeStep);
        setPwmDuty(fadeToPwmSteps(fadeValue));
      }
      break;
    default:
      setPwmDuty(0);
      break;
  }

  servicePwm(now);
}

bool parseBoundedInt(const char *text, int minimum, int maximum, int &value) {
  if (text == nullptr || *text == '\0') {
    return false;
  }

  char *end = nullptr;
  long parsed = strtol(text, &end, 10);
  if (end == text || *end != '\0' || parsed < minimum || parsed > maximum) {
    return false;
  }

  value = static_cast<int>(parsed);
  return true;
}

void changeMode(LedMode mode) {
  activeSettings.mode = static_cast<uint8_t>(mode);
  persistSettings();
  resetAnimation();
  Serial.print(F("Saved mode="));
  printMode();
}

void changeSpeed(int speed) {
  activeSettings.speed = static_cast<uint8_t>(speed);
  persistSettings();
  Serial.print(F("Saved speed="));
  Serial.println(activeSettings.speed);
}

void changeBrightness(int brightness) {
  activeSettings.brightness = static_cast<uint8_t>(brightness);
  persistSettings();
  applyModeOutput();
  Serial.print(F("Saved brightness="));
  Serial.println(activeSettings.brightness);
}

void factoryReset() {
  activeSettings = makeDefaultSettings();
  persistSettings();
  resetAnimation();
  Serial.println(F("Factory defaults restored and saved"));
}

bool applyModeText(const char *text) {
  if (strcmp(text, "off") == 0) {
    changeMode(LED_MODE_OFF);
    return true;
  }
  if (strcmp(text, "on") == 0) {
    changeMode(LED_MODE_ON);
    return true;
  }
  if (strcmp(text, "blink") == 0) {
    changeMode(LED_MODE_BLINK);
    return true;
  }
  if (strcmp(text, "fade") == 0) {
    changeMode(LED_MODE_FADE);
    return true;
  }
  return false;
}

void processCommand(const char *command) {
  if (strcmp(command, "help") == 0 || strcmp(command, "?") == 0 || strcmp(command, "h") == 0) {
    printHelp();
    return;
  }

  if (strcmp(command, "show") == 0 || strcmp(command, "v") == 0) {
    printSettings();
    return;
  }

  if (strcmp(command, "save") == 0 || strcmp(command, "s") == 0) {
    persistSettings();
    Serial.println(F("Settings saved"));
    return;
  }

  if (strcmp(command, "load") == 0 || strcmp(command, "l") == 0) {
    loadSettings();
    printSettings();
    return;
  }

  if (strcmp(command, "reset") == 0 || strcmp(command, "r") == 0) {
    factoryReset();
    printSettings();
    return;
  }

  if (strcmp(command, "o") == 0 || strcmp(command, "off") == 0) {
    changeMode(LED_MODE_OFF);
    return;
  }

  if (strcmp(command, "n") == 0 || strcmp(command, "on") == 0) {
    changeMode(LED_MODE_ON);
    return;
  }

  if (strcmp(command, "b") == 0 || strcmp(command, "blink") == 0) {
    changeMode(LED_MODE_BLINK);
    return;
  }

  if (strcmp(command, "f") == 0 || strcmp(command, "fade") == 0) {
    changeMode(LED_MODE_FADE);
    return;
  }

  if (strncmp(command, "mode ", 5) == 0) {
    if (applyModeText(command + 5)) {
      return;
    }
    Serial.println(F("ERROR: mode must be off, on, blink, or fade"));
    return;
  }

  if (strncmp(command, "speed ", 6) == 0) {
    int speed = 0;
    if (parseBoundedInt(command + 6, MIN_SPEED, MAX_SPEED, speed)) {
      changeSpeed(speed);
      return;
    }
    Serial.println(F("ERROR: speed must be from 1 to 9"));
    return;
  }

  if (strncmp(command, "brightness ", 11) == 0) {
    int brightness = 0;
    if (parseBoundedInt(command + 11, MIN_BRIGHTNESS, MAX_BRIGHTNESS, brightness)) {
      changeBrightness(brightness);
      return;
    }
    Serial.println(F("ERROR: brightness must be from 0 to 100"));
    return;
  }

  Serial.print(F("ERROR: unknown command: "));
  Serial.println(command);
}

void serviceSerial(unsigned long now) {
  while (Serial.available() > 0) {
    char received = static_cast<char>(Serial.read());

    if (received == '\r' || received == '\n') {
      if (commandOverflow) {
        Serial.println(F("ERROR: command is too long"));
      } else if (commandLength > 0) {
        commandBuffer[commandLength] = '\0';
        processCommand(commandBuffer);
      }
      commandLength = 0;
      commandOverflow = false;
      lastCommandByteTime = now;
      continue;
    }

    if (received == '\b' || received == 127) {
      if (commandLength > 0) {
        commandLength--;
      }
      continue;
    }

    if (received >= 32 && received <= 126) {
      if (commandLength < COMMAND_BUFFER_SIZE - 1) {
        commandBuffer[commandLength] = received;
        commandLength++;
        lastCommandByteTime = now;
      } else {
        commandOverflow = true;
      }
    }
  }

  if (commandLength > 0 && now - lastCommandByteTime > COMMAND_TIMEOUT_MS) {
    Serial.println(F("ERROR: command timed out"));
    commandLength = 0;
    commandOverflow = false;
  }
}
