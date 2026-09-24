# Arduino Uno Projects

A personal collection of Arduino Uno sketches, built and uploaded with
[Arduino CLI](https://arduino.github.io/arduino-cli/).

Every sketch here is **complete and tested** on a real Arduino Uno clone (CH340 USB
chip) connected through Windows from WSL2.

## Projects

| Project | What it does | Status |
|---|---|---|
| [test-blink](projects/test-blink/) | The classic first program — blinks the onboard LED (pin 13) every second. | Working |
| [serial-led](projects/serial-led/) | Serial LED Controller — type `1`, `0`, `b`, `f` in the Serial Monitor to control the onboard LED (on / off / blink / fade). | Working |
| [eeprom-settings-menu](projects/eeprom-settings-menu/) | EEPROM Settings Menu — remembers LED mode, speed, and brightness after power-off. | Working |

## What you need

- Arduino Uno (or any Uno-compatible board)
- USB cable (Type A to B)
- Arduino CLI running on **Windows**

## How to upload (this machine)

This laptop uses **WSL2**, so the board is plugged into Windows. The Windows-side
Arduino CLI does the upload — the Linux one cannot see Windows COM ports.

```bat
:: 1. Find which COM port the board is on (changes after every replug!)
C:\Users\Public\arduino-cli.exe board list --discovery-timeout 5s

:: 2. Compile (from the project folder on Windows)
C:\Users\Public\arduino-cli.exe compile -b arduino:avr:uno C:\Users\Public\serial-led

:: 3. Upload (replace COM6 with the detected port)
C:\Users\Public\arduino-cli.exe upload -p COM6 -b arduino:avr:uno C:\Users\Public\serial-led
```

> Tip: after editing a sketch here, copy it to Windows before compiling:
> `mkdir -p /mnt/c/Users/Public/<name> && cp projects/<name>/<name>.ino /mnt/c/Users/Public/<name>/`

## Using the sketches

### test-blink
No setup needed. Upload, and the onboard LED blinks on/off every 500 ms.

### serial-led
Upload, then open a serial monitor on the Arduino's COM port at **9600 baud** and type:

**Easy way:** double-click `tools/Arduino Serial Monitor.bat` on Windows — it detects your port automatically and opens the monitor. Then type:

| Command | Action |
|---|---|
| `1` | LED solid ON |
| `0` | LED OFF |
| `b` | Blink mode (500 ms on / 500 ms off) |
| `f` | Fade mode (smooth brightness ramp — software PWM, pin 13 has no PWM hardware) |
| `?` | Show help |

### eeprom-settings-menu
Open the Serial Monitor at **9600 baud** and send a command followed by Enter. Valid changes are saved to EEPROM automatically.

| Command | Action |
|---|---|
| `mode off\|on\|blink\|fade` | Select the LED mode |
| `speed 1..9` | Set blink speed |
| `brightness 0..100` | Set LED brightness |
| `show` | Show saved settings |
| `load` | Reload settings from EEPROM |
| `reset` | Restore factory defaults |
| `help` | Show all commands |

The first version uses only the Uno onboard LED. See `projects/eeprom-settings-menu/README.md` for the EEPROM layout, persistence test, and troubleshooting.

## Troubleshooting

| Problem | Fix |
|---|---|
| `stk500_getsync(): not in sync: resp=0x20` | Unplug the USB cable, wait 2 seconds, replug. Re-detect the COM port, then retry. |
| Board not found | Run `board list` again — replugging changes the COM port (COM4–COM7). |
| Upload "programmer is not responding" | Same replug fix; if it persists, press the onboard RESET button the moment upload starts. |

## Project structure

```
arduino-projects/
├── README.md            # you are here
├── AGENTS.md            # assistant setup notes
├── tools/
│   └── Arduino Serial Monitor.bat   # double-click serial monitor launcher
└── projects/
    ├── test-blink/      # first program: blink
    ├── serial-led/      # serial-controlled LED
    └── eeprom-settings-menu/ # persistent EEPROM settings menu
```

## Version history

| Version | Date | Changes |
|---|---|---|
| v1.3 | 2026-09-24 | Added and upload/Serial-Monitor-tested `eeprom-settings-menu` with checksum recovery and persistent LED settings. |
| v1.2 | 2026-09-22 | Fixed `fade` mode in `serial-led` (pin 13 has no PWM hardware, so fade now uses software PWM). |
| v1.1 | 2026-09-22 | Added `tools/Arduino Serial Monitor.bat` — double-click Windows launcher that auto-detects the COM port and opens the serial monitor. |
| v1.0 | 2026-09-22 | Added `serial-led` (Serial LED Controller). Repo created on GitHub with `test-blink` and `serial-led` documented. |
