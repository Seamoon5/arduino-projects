# AGENTS.md

Personal Arduino / ESP32 / ESP8266 firmware workspace. Anything that helps an AI
coding assistant work here without repeating mistakes.

## User's standing rules
- User has strong hands-on electronics knowledge but is learning software. Explain
  important programming concepts simply when they matter; do not dumb everything down.
- Prefer reliable, maintainable, production-quality code over clever shortcuts.
- Do NOT assume hardware specs (pins, voltage, pull-ups, module variants, motor
  ratings, etc.). Ask when a hardware detail is unknown or ambiguous.
- Before major changes, state the plan first. When modifying code, preserve working
  functionality unless the change was explicitly requested.
- Help select Arduino libraries; prefer well-maintained ones. Compile and diagnose
  compiler errors for the user. Upload when a board is connected.
- Never claim hardware works until it is actually tested. Distinguish software
  problems from hardware problems when troubleshooting.
- Keep secrets, API keys, passwords, and WiFi credentials OUT of source code.
  If a sketch needs credentials, put them in an untracked file (e.g. `secrets.h`
  listed in `.gitignore`) and `#include` it.
- Clean up failed/unnecessary files as work proceeds. If something does not work
  (wrong platform binary, failed download, temp file), delete it right away.

## This machine is WSL2 — CRITICAL
- The Arduino plugs into **Windows**, NOT Linux. WSL2 cannot see Windows COM
  ports (`/dev/ttyS*` do NOT work for uploads — no linux-side serial driver).
- Therefore the **Windows-side arduino-cli** is used for ALL uploads:
  `C:\Users\Public\arduino-cli.exe`
- Data/config lives on Windows at `C:\Users\Alauddin\AppData\Local\Arduino15`
  (the CLI uses the logged-in Windows user's AppData by default).
- A copy of the Windows CLI was at `C:\Users\Public\arduino-cli.exe`; the AVR core
  is installed there.

## Board: Arduino Uno clone with CH340 USB chip
- Appears on Windows as `USB-SERIAL CH340`, VID_1A86&PID_7523.
- The COM port **changes between COM4/COM5/COM6/COM7 after every replug**.
  ALWAYS detect first:
  - `cmd.exe /c "C:\Users\Public\arduino-cli.exe board list --discovery-timeout 5s"`
- Upload "not in sync: resp=0x20" fix: **unplug USB, wait 2 s, replug** (power-cycles
  the CH340), then re-detect the new COM port and retry. Backup: press the onboard
  RESET button the moment avrdude prints "attempt 1 of 10".
- Classic issue seen: COM7 failed mid-session; replug moved board to COM6 and upload
  succeeded instantly.

## Day-to-day commands (run from WSL, target Windows CLI)
```bash
# detect the board (shows which COM port it is on RIGHT NOW)
cmd.exe /c "C:\Users\Public\arduino-cli.exe board list --discovery-timeout 5s"

# compile on Windows (needs a Windows-visible copy of the sketch)
cmd.exe /c "C:\Users\Public\arduino-cli.exe compile -b arduino:avr:uno C:\\Users\\Public\\<name>"

# upload (replace COMx with the detected port)
cmd.exe /c "C:\Users\Public\arduino-cli.exe upload -p COMx -b arduino:avr:uno C:\\Users\\Public\\<name>"

# open the serial monitor in a friendly way (Windows double-click launcher)
# tools/Arduino Serial Monitor.bat auto-detects the COM port, then:
cmd.exe /c "C:\Users\Public\arduino-cli.exe monitor -p COMx -b arduino:avr:uno --config baudrate=9600"
```

## Sketch workflow on this machine (important)
arduino-cli cannot compile from the WSL path `\\wsl.localhost\...` reliably.
So the reliable flow is:
1. Code the sketch in `projects/<name>/<name>.ino` under this repo.
2. Copy it to Windows: `cp` into `C:\Users\Public\<name>\<name>.ino` (mkchdir first).
3. Compile + upload with the Windows CLI using the Windows path.

## Project layout convention
- One folder per project: `projects/<name>/<name>.ino`.
  Sketch file must have the same name as its folder (Arduino requirement).
- Extra files (.cpp/.h, notes, schematics) go inside the project folder.
- The `.ino` file is the entry point; `setup()` runs once, `loop()` runs forever.

## Gotchas that cost real time here
- WSL is invisible for USB — never diagnose a "dead board" from inside WSL using
  the Linux arduino-cli (deleted for this reason; the Linux CLI could not talk to
  the CH340 over /dev/ttyS*).
- If compile/upload errors mention `resp=0x20` or `not in sync`, see the replug fix above.
- Every replug moves the COM port — never assume COM7.
- AVR (Uno/Nano) Serial default is usually 9600 baud unless the sketch says otherwise.

## Hardware reporting rule
Report only what was verified. If firmware compiled but was never uploaded and
observed, say so explicitly. Ask before ordering hardware assumptions.