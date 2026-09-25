# ESP32 Blink Test

A small ESP32 test program that proves the board can boot, run code, and send Serial information. It uses the board's built-in LED when the selected board profile defines one, and otherwise uses a safe Serial heartbeat.

## What it does

- Blinks the board's built-in LED every 1 second when `LED_BUILTIN` is defined
- Sends a heartbeat every second over Serial when no LED pin is defined
- Prints the ESP32 chip model and CPU speed at 115200 baud
- Prints the available heap memory
- Uses no WiFi credentials and no extra libraries

## Files

| File | Purpose |
|------|---------|
| `esp32-blink-test.ino` | ESP32 test sketch |
| `README.md` | This guide |

## Board and upload settings

- Board package: `esp32:esp32`
- Board profile: `esp32:esp32:esp32`
- Serial speed: `115200`
- The sketch uses the board package's `LED_BUILTIN` definition when available; otherwise it does not assume a GPIO pin

The COM port can change after unplugging the board. Detect it before every upload.

## Upload from WSL

1. Detect the current port:

   ```bash
   cmd.exe /c "C:\Users\Public\arduino-cli.exe board list --discovery-timeout 5s"
   ```

2. Copy the sketch to a Windows-visible folder:

   ```bash
   powershell.exe -NoProfile -Command '$source="\\wsl.localhost\Ubuntu\home\salman\arduino-projects\projects\esp32-blink-test"; $target="C:\Users\Public\esp32-blink-test"; New-Item -ItemType Directory -Force -Path $target | Out-Null; Copy-Item "$source\esp32-blink-test.ino" "$target\esp32-blink-test.ino" -Force'
   ```

3. Compile:

   ```bash
   cmd.exe /c "C:\Users\Public\arduino-cli.exe compile -b esp32:esp32:esp32 C:\Users\Public\esp32-blink-test"
   ```

4. Upload using the port from step 1:

   ```bash
   cmd.exe /c "C:\Users\Public\arduino-cli.exe upload -p COMx -b esp32:esp32:esp32 C:\Users\Public\esp32-blink-test"
   ```

5. Open the Serial Monitor at `115200` baud and check the printed chip information.

## Expected result

The Serial Monitor prints an `ESP32 blink test v1.0` banner, the chip model, CPU speed, free heap, and a heartbeat every second. If the selected board profile defines `LED_BUILTIN`, that LED also blinks.

## Verified result

On 2026-09-25, the sketch was compiled and uploaded successfully to `COM9`. The upload tool identified the chip as `ESP32-D0WD-V3` revision `3.1`, and the Serial Monitor received the startup information plus six heartbeat messages at 115200 baud.

The generic `esp32:esp32:esp32` profile does not define `LED_BUILTIN`, so this test did not assume an unknown GPIO pin. The Serial heartbeat is the hardware verification for that profile.

## Version history

### v1.0 — 2026-09-25

- Added the first ESP32 blink and hardware-information test
