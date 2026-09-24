# CPU Warning Light

A tiny CPU-load warning system:

1. **Python script** on your PC reads CPU usage every second.
2. It sends the number over the USB **COM port**.
3. **Arduino** turns the onboard LED **ON** when CPU usage is **above 60%**, and OFF when it drops back down.

No extra wires or parts — just the Arduino Uno + USB cable.

## Files

| File | Purpose |
|---|---|
| `cpu-warning.ino` | Arduino sketch (receives CPU %, controls LED) |
| `cpu_sender.py` | Python script (reads CPU %, sends to COM port) |
| `requirements.txt` | Python libraries needed (`psutil`, `pyserial`) |
| `CPU Warning.bat` | Double-click Windows launcher (auto-installs libs if missing) |

## Upload the Arduino sketch

On this machine (WSL2 → Windows CLI):

```bash
# 1. Find the current COM port (it moves after every replug!)
cmd.exe /c "C:\Users\Public\arduino-cli.exe board list --discovery-timeout 5s"

# 2. Copy sketch to Windows
mkdir -p /mnt/c/Users/Public/cpu-warning
cp projects/cpu-warning/cpu-warning.ino /mnt/c/Users/Public/cpu-warning/

# 3. Compile
cmd.exe /c "C:\Users\Public\arduino-cli.exe compile -b arduino:avr:uno C:\\Users\\Public\\cpu-warning"

# 4. Upload (replace COMx with your port)
cmd.exe /c "C:\Users\Public\arduino-cli.exe upload -p COMx -b arduino:avr:uno C:\\Users\\Public\\cpu-warning"
```

## Run the PC sender

**Easy way (Windows):** double-click `CPU Warning.bat`.

**Manual way:**

```bat
pip install psutil pyserial
python cpu_sender.py
python cpu_sender.py --port COM6
python cpu_sender.py --threshold 80 --interval 0.5
```

| Option | Meaning | Default |
|---|---|---|
| `--port` | COM port | auto-detect |
| `--threshold` | CPU % that turns LED on | `60` |
| `--interval` | seconds between readings | `1.0` |

## How to test

1. Upload the sketch.
2. Run `CPU Warning.bat`.
3. Open Task Manager → Performance → CPU (watch the graph).
4. Run something heavy (large build, video export, many Chrome tabs) until CPU goes above 60%.
5. The **onboard LED (pin 13) should light up**.
6. Stop the heavy task — LED should turn off after CPU drops below 60%.

Serial Monitor at **9600 baud** will show lines like:

```text
CPU-WARNING ready
CPU=67 WARN
CPU=24 OK
```

> Close the Serial Monitor before running `cpu_sender.py` — only one program can hold the COM port at a time.

## Safety / fallback

- If the Python script stops or the USB is unplugged, the Arduino turns the LED **OFF after 3 seconds** (no stuck-on warning light).
- Pressing `Ctrl+C` in Python sends `0` so the LED turns off before the script exits.

## Troubleshooting

| Problem | Fix |
|---|---|
| `Access is denied` on COM port | Close the Serial Monitor / other serial tools first. |
| Port not found | Re-run `board list` — replug moves the port (COM4–COM7). |
| Upload `not in sync` / `resp=0x20` | Unplug USB, wait 2 seconds, replug, re-detect COM port, retry. |
| LED never turns on | Check `--threshold`, and make sure the Python script is actually running. |

## Version history

| Version | Date | Changes |
|---|---|---|
| v1.0 | 2026-09-24 | Initial release: Python CPU sender + Arduino LED warning at >60% CPU, with 3-second serial timeout safety. |
