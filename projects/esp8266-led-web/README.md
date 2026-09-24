# ESP8266 LED Web Server — ON/OFF from phone

Control the ESP8266 built-in LED from your phone browser.  
**No home WiFi password required** — the board creates its own hotspot.

## How to use (phone)

1. Upload the sketch (once) — already done on COM9 if you ran the AI session
2. On the phone: WiFi settings → join **`ESP8266-LED`**
3. Password: **`esp8266led`**
4. Open browser → **`http://192.168.4.1`**
5. Tap **ON** / **OFF**

## How it works

| Piece | Meaning |
|-------|---------|
| Access point (AP) | ESP8266 acts like a tiny router |
| `ESP8266-LED` | WiFi name your phone joins |
| `http://192.168.4.1` | Web page served by the ESP8266 |
| Built-in LED | GPIO2 / `LED_BUILTIN` (onboard) |

## Files

| File | Purpose |
|------|---------|
| `esp8266-led-web.ino` | Full sketch (web server + LED control) |
| `README.md` | This file |

## Windows copy / upload

```
C:\Users\Public\esp8266-led-web\esp8266-led-web.ino
```

```bash
cmd.exe /c "C:\Users\Public\arduino-cli.exe compile -b esp8266:esp8266:nodemcuv2 C:\\Users\\Public\\esp8266-led-web"
cmd.exe /c "C:\Users\Public\arduino-cli.exe upload -p COM9 -b esp8266:esp8266:nodemcuv2 C:\\Users\\Public\\esp8266-led-web"
```

## API (optional)

- `GET /status` → `{"led":"on","ip":"192.168.4.1"}`
- `GET /led?state=on` or `GET /led?state=off`

## Security note

The hotspot password is in the sketch (required for WPA2).  
Change `AP_SSID` / `AP_PASS` at the top of the `.ino` if you want.  
This network is only for LED control — don’t reuse a personal password.

## Version history

### v1.0 — 2026-09-24
- First release
- SoftAP `ESP8266-LED` / `esp8266led`
- Mobile-friendly ON/OFF page at `http://192.168.4.1`
- `/status` JSON endpoint
- 303 redirect back to UI after button press
