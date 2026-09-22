const int LED_PIN = 13;

enum Mode { MODE_OFF, MODE_ON, MODE_BLINK, MODE_FADE };

Mode currentMode = MODE_OFF;
int fadeValue = 0;
int fadeStep = 5;
unsigned long lastBlinkTime = 0;
const unsigned long BLINK_INTERVAL = 500;

void setup() {
  pinMode(LED_PIN, OUTPUT);
  Serial.begin(9600);
  Serial.println(F("=== Serial LED Controller ==="));
  Serial.println(F("Commands:"));
  Serial.println(F("  1 = LED ON"));
  Serial.println(F("  0 = LED OFF"));
  Serial.println(F("  b = Blink mode"));
  Serial.println(F("  f = Fade mode"));
  Serial.println(F("  ? = Show this help"));
  Serial.println(F("Waiting for your command..."));
}

void loop() {
  if (Serial.available() > 0) {
    char command = Serial.read();
    handleCommand(command);
  }

  runCurrentMode();
}

void handleCommand(char cmd) {
  switch (cmd) {
    case '1':
      currentMode = MODE_ON;
      Serial.println(F("LED is now ON (solid)"));
      break;
    case '0':
      currentMode = MODE_OFF;
      Serial.println(F("LED is now OFF"));
      break;
    case 'b':
    case 'B':
      currentMode = MODE_BLINK;
      Serial.println(F("Blink mode: 500ms on / 500ms off"));
      break;
    case 'f':
    case 'F':
      currentMode = MODE_FADE;
      fadeValue = 0;
      fadeStep = 5;
      Serial.println(F("Fade mode: smooth brightness ramp"));
      break;
    case '?':
    case 'h':
    case 'H':
      Serial.println(F("--- Commands ---"));
      Serial.println(F("  1 = LED ON"));
      Serial.println(F("  0 = LED OFF"));
      Serial.println(F("  b = Blink mode"));
      Serial.println(F("  f = Fade mode"));
      break;
    case '\n':
    case '\r':
    case ' ':
      break;
    default:
      Serial.print(F("Unknown command: '"));
      Serial.print(cmd);
      Serial.println(F("'  (type ? for help)"));
      break;
  }
}

void runCurrentMode() {
  unsigned long now = millis();

  switch (currentMode) {
    case MODE_OFF:
      digitalWrite(LED_PIN, LOW);
      break;

    case MODE_ON:
      digitalWrite(LED_PIN, HIGH);
      break;

    case MODE_BLINK:
      if (now - lastBlinkTime >= BLINK_INTERVAL) {
        lastBlinkTime = now;
        toggleLed();
      }
      break;

    case MODE_FADE:
      analogWrite(LED_PIN, fadeValue);
      fadeValue += fadeStep;
      if (fadeValue >= 255) {
        fadeValue = 255;
        fadeStep = -5;
      } else if (fadeValue <= 0) {
        fadeValue = 0;
        fadeStep = 5;
      }
      delay(10);
      break;
  }
}

void toggleLed() {
  static bool ledState = false;
  ledState = !ledState;
  digitalWrite(LED_PIN, ledState ? HIGH : LOW);
}
