# ESP8266 Anywhere Control — Blynk Cloud (v3.0)

Control the **onboard LED + 5 spare outputs from anywhere in the world** using the
free **Blynk IoT** app, plus a **local fallback page** on your home network.

> Template: **Quickstart Template** · ID: **TMPL6Hkyu5N7f**
> Firmware: `3.0.0`

## 1. Install + provision (one time, ~3 minutes)

1. Phone → install **Blynk IoT** (iOS/Android) → create a free account.
2. Power the board. It creates a WiFi hotspot named:
   **`Blynk Quickstart Template-W4MK`**
3. In the Blynk app: **+ New Device → Find devices nearby** → select it.
4. Pick your home WiFi `zeeshsn.optix-network`, type the password, tap **Connect**.
   (The password goes phone → board directly. It is never in the code.)
5. Board reboots and shows **Online** in the app.

Serial monitor (115200) prints the same steps if anything is unclear.

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
http://192.168.1.xxx
```

The IP is printed on the Serial monitor at 115200 after it connects.
Page gives the same 6 ON/OFF switches + a big red **ALL OFF** button.

It only runs while Blynk is connected (port 80 is shared with Blynk's
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
