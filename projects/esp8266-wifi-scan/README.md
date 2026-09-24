# ESP8266 Welcome — Blink + WiFi Scan

First program for Salman’s ESP8266 (CP2102, **COM9** on 2026-09-24).

## What it does
1. **Blinks** the built-in LED (board is alive)
2. **Scans WiFi** every 10 seconds (no password required)
3. Prints chip ID, free heap, and nearby networks on the **Serial Monitor @ 115200**

## Files
| File | Purpose |
|------|---------|
| `esp8266-wifi-scan.ino` | The sketch (same name as folder — Arduino rule) |
| `README.md` | This file |

## Upload (this machine)

```bash
# from WSL
mkdir -p /mnt/c/Users/Public/esp8266-wifi-scan
cp /home/salman/arduino-projects/projects/esp8266-wifi-scan/esp8266-wifi-scan.ino \
   /mnt/c/Users/Public/esp8266-wifi-scan/

cmd.exe /c "C:\Users\Public\arduino-cli.exe compile -b esp8266:esp8266:nodemcuv2 C:\\Users\\Public\\esp8266-wifi-scan"
cmd.exe /c "C:\Users\Public\arduino-cli.exe upload -p COM9 -b esp8266:esp8266:nodemcuv2 C:\\Users\\Public\\esp8266-wifi-scan"
```

Replace `COM9` after every replug (board list first).  
If your board is a **Wemos D1 mini**, use `-b esp8266:esp8266:d1_mini` instead.

## Serial Monitor
```bash
cmd.exe /c "C:\Users\Public\arduino-cli.exe monitor -p COM9 -b esp8266:esp8266:nodemcuv2 --config baudrate=115200"
```
Or double-click `arduino-projects/tools/Arduino Serial Monitor.bat`.

## Expected output
```
----------------------------------------
ESP8266 Welcome Sketch v1.0
----------------------------------------
Chip ID: ...
...
----------------------------------------
WiFi networks nearby
----------------------------------------
Found 5 network(s):
1. MyHomeWiFi  |  RSSI -45 dBm  |  ch 6  |  secured
```

## Version history
### v1.0 — 2026-09-24
- Initial sketch: blink + WiFi scan + chip info
- 115200 baud, scan every 10s
