# EEPROM Settings Menu for Arduino Uno

A small Arduino Uno project that remembers LED settings after the USB cable is unplugged. It uses the Uno's internal EEPROM and the Serial Monitor as a text menu.

## What it does

The first version uses only the Arduino Uno and its onboard LED:

- LED modes: off, on, blink, and software fade
- Blink speed from 1 (slow) to 9 (fast)
- Brightness from 0 to 100
- Automatic saving after every valid change
- EEPROM validation with a magic number, version, range checks, and checksum
- Factory reset and reload commands
- No blocking `delay()` calls

## Hardware

| Part | Quantity | Purpose |
|---|---:|---|
| Arduino Uno or Uno-compatible board | 1 | Controller and EEPROM storage |
| USB cable | 1 | Power, programming, and Serial Monitor |
| Onboard LED | 1 | Status/output indicator |

No external LED, resistor, sensor, or power supply is required for v1. The onboard LED is connected to `LED_BUILTIN`; the firmware controls brightness with software PWM because the Uno's built-in LED pin does not have hardware PWM.

## Serial commands

Open the Serial Monitor at **9600 baud** and send each command followed by Enter.

| Command | Action |
|---|---|
| `help` | Show the command list |
| `show` | Show the saved settings |
| `mode off` | Turn the LED off |
| `mode on` | Turn the LED on at the saved brightness |
| `mode blink` | Blink at the saved speed |
| `mode fade` | Fade smoothly up and down |
| `speed 1` ... `speed 9` | Set blink speed; 1 is slow and 9 is fast |
| `brightness 0` ... `brightness 100` | Set LED brightness |
| `save` | Save the current settings |
| `load` | Reload the last saved settings from EEPROM |
| `reset` | Restore and save factory defaults |
| `o`, `n`, `b`, `f` | Quick commands for off, on, blink, and fade |
| `h` or `?` | Show help |

The default settings are blink mode, speed 5, and brightness 80.

## EEPROM design

The settings occupy the first eight EEPROM bytes:

```text
Address 0-1  Magic number
Address 2    Format version
Address 3    LED mode
Address 4    Blink speed
Address 5    Brightness
Address 6-7  Checksum
```

The sketch uses `EEPROM.update()` so a byte is written only when its value changes. A blank or damaged record is replaced with safe factory defaults on the next boot. Do not use this project for passwords, encryption keys, or high-frequency sensor logging.

## Build and upload on this WSL2 computer

The Arduino is connected to Windows, not directly to WSL2. Copy the complete project to a Windows-visible folder before compiling:

```bash
mkdir -p /mnt/c/Users/Public/eeprom-settings-menu
cp projects/eeprom-settings-menu/eeprom-settings-menu.ino /mnt/c/Users/Public/eeprom-settings-menu/
cp projects/eeprom-settings-menu/config.h /mnt/c/Users/Public/eeprom-settings-menu/
```

Compile with the Windows-side Arduino CLI:

```bash
cmd.exe /c "C:\\Users\\Public\\arduino-cli.exe compile -b arduino:avr:uno C:\\Users\\Public\\eeprom-settings-menu"
```

Find the current COM port immediately before uploading:

```bash
cmd.exe /c "C:\\Users\\Public\\arduino-cli.exe board list --discovery-timeout 5s"
```

Replace `COMx` with the detected port:

```bash
cmd.exe /c "C:\\Users\\Public\\arduino-cli.exe upload -p COMx -b arduino:avr:uno C:\\Users\\Public\\eeprom-settings-menu"
```

Open the monitor at 9600 baud:

```bash
cmd.exe /c "C:\\Users\\Public\\arduino-cli.exe monitor -p COMx -b arduino:avr:uno --config baudrate=9600"
```

The repository also contains `tools/Arduino Serial Monitor.bat`, which detects the COM port automatically on Windows.

## Persistence test

1. Upload the sketch and open the Serial Monitor.
2. Send `mode fade`, `brightness 60`, and `speed 8`.
3. Send `show` and note the values.
4. Unplug the USB cable for a few seconds.
5. Reconnect the board and open the Serial Monitor again.
6. Send `show`; the same values should be reported.

Send `reset` to verify that the defaults return.

## Troubleshooting

| Problem | Fix |
|---|---|
| No Serial Monitor output | Close and reopen the monitor at 9600 baud, then press the board reset button. |
| Board not listed | Run `board list` again; the COM port can change after every replug. |
| `not in sync: resp=0x20` | Unplug the USB cable, wait two seconds, replug, detect the new COM port, and upload again. |
| LED brightness looks slightly stepped | The onboard LED pin has no hardware PWM, so v1 uses 20-step software PWM. |
| Settings return to defaults | The EEPROM record was blank, damaged, or from a different format; the checksum/version checks intentionally reject it. |

## Proof status

- Firmware design: complete
- Build proof: passed; 6,914 bytes flash and 334 bytes SRAM
- Upload proof: passed on the connected Uno at COM6
- Serial proof: `mode`, `speed`, `brightness`, `show`, and `reset` commands were verified
- Persistence proof: settings survived a board reset and were reloaded from EEPROM
- Visual LED proof: still requires watching the onboard LED during the next test

## Version history

| Version | Date | Changes |
|---|---|---|
| v1.0 | 2026-09-24 | Added the EEPROM-backed LED settings menu, checksum recovery, factory reset, software PWM brightness, non-blocking timing, and Serial Monitor commands. |
