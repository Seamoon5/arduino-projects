# ESP8266 Multi-Load Web Server — phone ON/OFF (v2.0)

Control the **onboard LED + 5 spare outputs** from your phone browser.  
Spare pins are ready for future loads (LEDs, relay modules, etc.).  
**No home WiFi password** — the board creates its own hotspot.

## How to use (phone)

1. On the phone: WiFi → join **`ESP8266-LED`**
2. Password: **`esp8266led`**
3. Browser → **`http://192.168.4.1`**
4. Tap **ON/OFF** on any channel · **stop** = all off

## Channels

| # | Name | Pin (NodeMCU/D1 mini) | GPIO |
|---|------|------------------------|------|
| 0 | LED | onboard | GPIO2 (active-LOW) |
| 1 | LOAD1 | D1 | GPIO5 |
| 2 | LOAD2 | D2 | GPIO4 |
| 3 | LOAD3 | D5 | GPIO14 |
| 4 | LOAD4 | D6 | GPIO12 |
| 5 | LOAD5 | D7 | GPIO13 |

Avoided for safety: GPIO0 / GPIO15 (boot), GPIO1/GPIO3 (UART).

## Wiring future loads (important)

ESP pins are **3.3V logic only** — not for motors/high current.

- Small LED → resistor (~220Ω–1kΩ) to pin, GND common  
- Relay module → use one that accepts **3.3V logic** (or level shift); many 5V relays need a transistor if logic is 5V-only  
- Keep **common GND** between ESP and driver  

## Files

| File | Purpose |
|------|---------|
| `esp8266-led-web.ino` | Full sketch v2.0 |
| `README.md` | This file |

## Upload (this machine)

Windows copy: `C:\Users\Public\esp8266-led-web\`

```bash
cmd.exe /c "C:\Users\Public\arduino-cli.exe compile -b esp8266:esp8266:nodemcuv2 C:\\Users\\Public\\esp8266-led-web"
cmd.exe /c "C:\Users\Public\arduino-cli.exe upload -p COM9 -b esp8266:esp8266:nodemcuv2 C:\\Users\\Public\\esp8266-led-web"
```

Detect COM first after every replug.

## API

- `GET /status` → JSON all channels  
- `GET /set?id=0&state=on` (id 0–5)  
- `GET /all?state=off`  

## Version history

### v2.0 — 2026-09-24
- Multi-channel UI (6 cards: LED + LOAD1–5)
- Spare GPIOs: D1, D2, D5, D6, D7 ready for loads
- `/set`, `/all`, `/status` endpoints
- All outputs default OFF at boot (safe)

### v1.0 — 2026-09-24
- First release: single onboard LED ON/OFF
- SoftAP `ESP8266-LED` / `esp8266led`
- Mobile page at `http://192.168.4.1`
