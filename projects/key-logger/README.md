# Key Logger (Arduino Uno)

Saves keys typed on your laptop into the Arduino Uno's EEPROM, then lets you
read them back anytime over the Serial Monitor — or with hotkeys while the
logger is running.

Built for Salman's Uno clone (CH340) on this WSL2 + Windows machine.

## How it works

```
You type on laptop
        |
        v
key_logger.py  (Windows, watches keystrokes)
        |
        |  USB serial (COM port, 9600 baud)
        v
Arduino Uno  --->  stores text in EEPROM (about 1000 characters)
```

Your Uno CH340 board cannot act as a fake USB keyboard, so a small Windows
script captures the keys and sends them over the normal serial port instead.

## Files

| File | What it is |
|---|---|
| `key-logger.ino` | Arduino firmware (stores + answers commands) |
| `key_logger.py` | Windows script that captures keys and sends them |
| `Key Logger.bat` | Double-click launcher for Windows |
| `requirements.txt` | Python libraries: `pynput`, `pyserial` |
| `test_serial.py` | Automatic test of the serial protocol |

## Install (once)

Windows copy lives at `C:\Users\Public\key-logger\`.

The `.bat` file installs `pynput` and `pyserial` by itself the first time you
run it. Or manually:

```bat
python -m pip install pynput pyserial
```

## How to use

### 1. Upload the sketch (once)

```bat
C:\Users\Public\arduino-cli.exe board list --discovery-timeout 5s
C:\Users\Public\arduino-cli.exe compile -b arduino:avr:uno C:\Users\Public\key-logger
C:\Users\Public\arduino-cli.exe upload -p COMx -b arduino:avr:uno C:\Users\Public\key-logger
```

### 2. Start capturing

1. **Close** the Arduino Serial Monitor (only one program can use the port).
2. Double-click `Key Logger.bat` on the Desktop (or in
   `C:\Users\Public\key-logger\`).
3. Type normally — every key is sent to the Uno.
4. LED flashes briefly for each saved key.

### 3. Hotkeys while the logger is open

| Key | Action |
|---|---|
| **F6** | Fetch the log from Arduino and print it in this window |
| **F7** | Clear the log on the Arduino |
| **F8** | Pause / resume capturing |
| **F10** | Quit the logger (then you can open Serial Monitor) |

### 4. Or use the Serial Monitor

Stop the logger first (F10), then open the Serial Monitor at **9600 baud**
(easy way: `tools/Arduino Serial Monitor.bat`).

| Command | Action |
|---|---|
| `read` | Show all stored keys |
| `count` | How many characters are stored |
| `find hello` | Search the log (case sensitive) |
| `clear` | Erase the log |
| `pause` | Stop accepting new keys |
| `resume` | Start accepting keys |
| `status` | count / max / logging or paused or full |
| `help` | List commands |

## LED meaning

| LED | Meaning |
|---|---|
| Quick flash | Key saved |
| Slow blink (500 ms) | Logger paused |
| Solid ON | EEPROM log is full (~1000 chars) — type `clear` |

## Limits (honest truth)

- Uno EEPROM holds about **1018 characters**. When full, new keys are refused
  (`FULL`) until you clear.
- Logging works only while `key_logger.py` is running.
- Special keys are stored as readable text: Enter → newline, Tab → tab.
  Backspace removes the last stored character.
- Modifiers alone (Shift/Ctrl/Alt) are not stored.
- This is a **learning project on your own PC** — not a stealth spy tool.
  The logger window stays visible while it runs.

## Windows / WSL paths

| What | Path |
|---|---|
| WSL project | `/home/salman/arduino-projects/projects/key-logger/` |
| Windows copy | `C:\Users\Public\key-logger\` |
| Desktop launcher | `C:\Users\Alauddin\Desktop\Key Logger.bat` |

## Troubleshooting

| Problem | Fix |
|---|---|
| `Access is denied` on COM port | Close Serial Monitor (and any other logger window). |
| No COM port found | Unplug USB, wait 2 s, replug, run `board list` again. |
| Upload `not in sync` | Unplug, wait 2 s, replug, find new COM port, retry. |
| Keys not saving | Press F8 once (might be paused). Check `status` in Serial Monitor. |
| `ModuleNotFoundError: pynput` | `python -m pip install pynput pyserial` |

## Version history

| Version | Date | Changes |
|---|---|---|
| v1.0 | 2026-09-24 | First release: EEPROM store, serial commands (`read`, `count`, `find`, `clear`, `pause`, `resume`, `status`), Windows capture script with F6/F7/F8/F10 hotkeys, auto-installing `.bat` launcher, protocol test script. |
