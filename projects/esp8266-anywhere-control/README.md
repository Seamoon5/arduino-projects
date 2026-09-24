# ESP8266 Anywhere Control — Blynk Cloud (v3.0)

Control the **onboard LED + 5 spare outputs from anywhere in the world** using the
free **Blynk IoT** app, plus a **local fallback page** on your home network.

> Template: **Quickstart Template** · ID: **TMPL6Hkyu5N7f**
> Firmware: `3.1.0`

## 1. One-time setup (auto-connect, no app pairing)

The board reads your WiFi name, WiFi password and Blynk Auth Token from a
local file called **`secrets.h`**. That file is listed in `.gitignore`, so your
password and token **never reach GitHub**.

**Get the Auth Token** (once):
1. https://blynk.cloud → log in
2. **Search → Devices → New Device** → template **Quickstart Template** → name it → **Create**
3. Open the device → **Device Info** tab → copy **Auth Token** (32 characters)

**Then run the helper:**
double-click **`Set WiFi.bat`** on the Desktop (`C:\Users\Alauddin\Desktop\Set WiFi.bat`).
It asks 3 questions, writes `secrets.h`, finds the board, compiles, uploads and
prints the board's output for 30 seconds. It refuses a malformed token or
password *before* flashing.

> The `#define BLYNK_AUTH_TOKEN "Your Auth Token"` line on the **Template** page
> is only a placeholder — a real token only appears after a device exists.

Expected serial output:

```
Firmware is preprovisioned
Using Dynamic IP: 192.168.100.236
Connecting to blynk.cloud:443
Ready (ping: 12ms).
CONNECTING_CLOUD => RUNNING
[local] page ready at http://192.168.100.236
```

**Reset to hotspot provisioning:** hold the FLASH button (GPIO0) for 10 s —
this wipes the saved config and restarts the Blynk hotspot
`Blynk Quickstart Template-W4MK`.

## 2. Build the app dashboard

In the Blynk app → your device → **Developer Zone / Dashboard**:

| Widget | Datastream | Meaning |
|--------|-----------|---------|
| Switch ×6 | `V0` … `V5` | LED, LOAD1 … LOAD5 |
| Button ×1 | `V6` | Emergency **ALL OFF** (set to 1 = press) |

Label them however you like (Fan, Light, Pump…).

## 3. Local fallback page (same home network only)

Once the board is Online, open its IP in a browser:

```
http://192.168.100.236
```

The IP is printed on the Serial monitor at 115200 after it connects (and is
shown at the bottom of the page itself). Page gives the same 6 ON/OFF switches
plus a big red **ALL OFF** button.

Verified working: `GET /` → HTTP 200, `GET /status` → JSON with all channels
plus `"blynk":"online"`.

It only runs while Blynk is RUNNING (port 80 is shared with Blynk's
provisioning portal), so it disappears while the board is in setup mode.

## Channels

| V-pin | # | Name | Pin (NodeMCU/D1 mini) | GPIO |
|-------|---|------|------------------------|------|
| V0 | 0 | LED | onboard | GPIO2 (active-LOW) |
| V1 | 1 | LOAD1 | D1 | GPIO5 |
| V2 | 2 | LOAD2 | D2 | GPIO4 |
| V3 | 3 | LOAD3 | D5 | GPIO14 |
| V4 | 4 | LOAD4 | D6 | GPIO12 |
| V5 | 5 | LOAD5 | D7 | GPIO13 |
| V6 | — | ALL OFF | — | — |

Avoided for safety: GPIO0/GPIO15 (boot straps), GPIO1/GPIO3 (UART).

## Safety rules

- ESP pins are **3.3V logic only** — never drive mains, motors or relays direct.
- Small LED → ~220Ω–1kΩ resistor.
- Relay module → must accept **3.3V logic** (or add a transistor/level shift).
- Keep a **common GND** between ESP and driver.
- Every load defaults **OFF at boot** (safe startup).
- All OFF is available in the app (V6), on the local page, and in Serial.

## Reconnect behaviour

- Blynk auto-reconnects if WiFi or internet drops.
- Channel states are re-sent to the app on every reconnect (`BLYNK_CONNECTED`).
- Local page starts automatically once the board reaches `RUNNING`.

## Files

| File | Purpose |
|------|---------|
| `esp8266-anywhere-control.ino` | Main firmware v3.0 (channels, Blynk, local page) |
| `BlynkEdgent.h`, `BlynkState.h`, `ConfigMode.h`, `ConfigStore.h`, `Console.h`, `Indicator.h`, `OTA.h`, `ResetButton.h`, `Settings.h` | Blynk.Edgent framework (from Blynk library examples) |
| `README.md` | This file |

`Settings.h` uses the generic NodeMCU config: config-reset button on **GPIO0**
(hold 10 s), status LED disabled (GPIO2 belongs to LOAD0).

## Upload (this machine)

Windows copy: `C:\Users\Public\esp8266-anywhere-control\`

```bash
cmd.exe /c "C:\Users\Public\arduino-cli.exe compile -b esp8266:esp8266:nodemcuv2 C:\\Users\\Public\\esp8266-anywhere-control"
cmd.exe /c "C:\Users\Public\arduino-cli.exe upload -p COM9 -b esp8266:esp8266:nodemcuv2 C:\\Users\\Public\\esp8266-anywhere-control"
```

Detect the COM port first after every replug (`board list --discovery-timeout 5s`).

## Version history

### v3.1 — 2026-09-24
- **Auto-connect provisioning** via `secrets.h` (git-ignored) — no app pairing
- `tools/Set-Wifi.ps1` + `tools/Set WiFi.bat`: writes `secrets.h`, detects the
  COM port, compiles, uploads, prints 30 s of serial. Validates the Auth Token
  (32 alphanumeric) and password (8–63 chars) *before* flashing
- WiFi status diagnostic printed every 5 s during connect
- **Verified on hardware:** WiFi → `192.168.100.236`, NTP OK,
  `blynk.cloud:443` → `Ready (ping: 12ms)` → `RUNNING`
- **Verified:** `/` HTTP 200, `/status` JSON reports `"blynk":"online"`,
  ON/OFF + ALL OFF all change state correctly

### v3.0 — 2026-09-24
- First release: worldwide control via **Blynk Cloud** (Blynk.Edgent)
- Dynamic WiFi provisioning (Blynk.Inject) — no WiFi password in source
- 6 independent channels on V0–V5, emergency ALL OFF on V6
- State re-sync to app on every reconnect
- Local fallback web page (port 80) while Blynk is RUNNING
- Safe boot: all loads OFF before setup runs
- OTA firmware updates via Blynk.Air
- Blynk event log on emergency ALL OFF

### v2.0 — 2026-09-24 (previous project, local only)
- Multi-channel SoftAP page at `http://192.168.4.1` — see `esp8266-led-web/`
