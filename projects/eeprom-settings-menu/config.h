#pragma once

#include <Arduino.h>

constexpr uint8_t LED_PIN = LED_BUILTIN;
constexpr unsigned long SERIAL_BAUD = 9600UL;
constexpr unsigned long COMMAND_TIMEOUT_MS = 1500UL;
constexpr uint8_t COMMAND_BUFFER_SIZE = 24;

constexpr int EEPROM_MAGIC_ADDRESS = 0;
constexpr int EEPROM_VERSION_ADDRESS = 2;
constexpr int EEPROM_MODE_ADDRESS = 3;
constexpr int EEPROM_SPEED_ADDRESS = 4;
constexpr int EEPROM_BRIGHTNESS_ADDRESS = 5;
constexpr int EEPROM_CHECKSUM_ADDRESS = 6;
constexpr uint16_t EEPROM_MAGIC = 0xE37A;
constexpr uint8_t EEPROM_VERSION = 1;
constexpr uint16_t CHECKSUM_SEED = 0xA55A;

constexpr uint8_t MIN_SPEED = 1;
constexpr uint8_t MAX_SPEED = 9;
constexpr uint8_t MIN_BRIGHTNESS = 0;
constexpr uint8_t MAX_BRIGHTNESS = 100;

constexpr unsigned long BLINK_INTERVAL_MIN_MS = 200UL;
constexpr unsigned long BLINK_INTERVAL_MAX_MS = 2000UL;
constexpr unsigned long FADE_STEP_INTERVAL_MS = 20UL;
constexpr uint8_t FADE_VALUE_STEP = 5;
constexpr uint8_t PWM_STEPS = 20;
constexpr unsigned long PWM_STEP_MS = 2UL;

enum LedMode : uint8_t {
  LED_MODE_OFF = 0,
  LED_MODE_ON = 1,
  LED_MODE_BLINK = 2,
  LED_MODE_FADE = 3
};

struct Settings {
  uint16_t magic;
  uint8_t version;
  uint8_t mode;
  uint8_t speed;
  uint8_t brightness;
  uint16_t checksum;
};
