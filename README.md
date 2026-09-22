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

| Command | Action |
|---|---|
| `1` | LED solid ON |
| `0` | LED OFF |
| `b` | Blink mode (500 ms on / 500 ms off) |
| `f` | Fade mode (smooth brightness ramp) |
| `?` | Show help |

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
├── projects/
│   ├── test-blink/      # first program: blink
│   └── serial-led/      # serial-controlled LED
```

## Version history

| Version | Date | Changes |
|---|---|---|
| v1.0 | 2026-09-22 | Added `serial-led` (Serial LED Controller). Repo created on GitHub with `test-blink` and `serial-led` documented. |