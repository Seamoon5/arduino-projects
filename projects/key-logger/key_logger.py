#!/usr/bin/env python3
"""
Arduino Key Logger — Windows capture script
-------------------------------------------
Watches keys you type on this PC and sends them to the Arduino Uno over USB
serial. The Arduino stores them in EEPROM.

Install (once, on Windows):
    pip install pynput pyserial

Run:
    python key_logger.py
    python key_logger.py --port COM6

Hotkeys (while this window is open):
    F6   fetch log from Arduino and print it here
    F7   clear log on Arduino
    F8   pause / resume capturing
    F10   quit
"""

import argparse
import sys
import time

try:
    from pynput import keyboard
except ImportError:
    print("Missing library: pynput")
    print("Install with:  pip install pynput pyserial")
    sys.exit(1)

try:
    import serial
    from serial.tools import list_ports
except ImportError:
    print("Missing library: pyserial")
    print("Install with:  pip install pynput pyserial")
    sys.exit(1)


def find_arduino_port(preferred=None):
    if preferred:
        return preferred
    preferred_ids = (("1A86", "7523"), ("2341", "0043"), ("2341", "0041"))
    candidates = list(list_ports.comports())
    for p in candidates:
        vid = f"{p.vid:04X}" if p.vid is not None else ""
        pid = f"{p.pid:04X}" if p.pid is not None else ""
        if (vid, pid) in preferred_ids:
            return p.device
    for p in candidates:
        if p.vid is not None:
            return p.device
    return None


def escape_char(ch):
    if ch == "\\":
        return "\\\\"
    return ch


def key_to_payload(key):
    if hasattr(key, "char") and key.char is not None:
        o = ord(key.char)
        if 32 <= o < 127:
            return escape_char(key.char)
        return None
    if key == keyboard.Key.enter:
        return "\\n"
    if key == keyboard.Key.tab:
        return "\\t"
    if key == keyboard.Key.backspace:
        return "\\b"
    if key == keyboard.Key.space:
        return " "
    return None


def drain_banner(ser):
    time.sleep(2)
    lines = []
    deadline = time.time() + 1.0
    while time.time() < deadline:
        if ser.in_waiting:
            raw = ser.readline()
            text = raw.decode("ascii", errors="ignore").rstrip("\r\n")
            if text:
                lines.append(text)
        else:
            time.sleep(0.05)
    for line in lines:
        print(f"  {line}")


def send_line(ser, text):
    ser.write((text + "\n").encode("ascii"))


def fetch_until(ser, end_prefix, timeout=3.0):
    lines = []
    deadline = time.time() + timeout
    while time.time() < deadline:
        raw = ser.readline()
        if not raw:
            continue
        text = raw.decode("ascii", errors="ignore").rstrip("\r\n")
        if text:
            lines.append(text)
            if text.startswith(end_prefix):
                break
    return lines


def main():
    parser = argparse.ArgumentParser(description="Send typed keys to Arduino over COM port")
    parser.add_argument("--port", help="COM port, e.g. COM6 (auto-detect if omitted)")
    args = parser.parse_args()

    port_name = find_arduino_port(args.port)
    if not port_name:
        print("No Arduino COM port found.")
        print("Plug in the board, then run again, or pass --port COM6")
        sys.exit(1)

    print(f"Opening {port_name} at 9600 baud...")
    try:
        ser = serial.Serial(port_name, 9600, timeout=0.2)
    except serial.SerialException as e:
        print(f"Could not open {port_name}: {e}")
        print("If access is denied, close the Arduino Serial Monitor first.")
        sys.exit(1)

    print("Arduino says:")
    drain_banner(ser)

    state = {"paused": False, "sent": 0}

    def on_press(key):
        if key == keyboard.Key.f10:
            return False
        if key == keyboard.Key.f8:
            state["paused"] = not state["paused"]
            mode = "PAUSED" if state["paused"] else "LOGGING"
            print(f"\n  [{mode}] F8 toggles, F6 read, F7 clear, F10 quit")
            return
        if key == keyboard.Key.f6:
            send_line(ser, "read")
            print("\n  --- Arduino log ---")
            for line in fetch_until(ser, "--- END"):
                print(f"  {line}")
            return
        if key == keyboard.Key.f7:
            send_line(ser, "clear")
            print("\n  Arduino: CLEARED")
            return
        if state["paused"]:
            return
        payload = key_to_payload(key)
        if payload is None:
            return
        try:
            ser.write(("+" + payload + "\n").encode("ascii"))
            state["sent"] += 1
        except serial.SerialException as e:
            print(f"\n  Serial error: {e}")
            return False

    print("\n=== Arduino Key Logger ===")
    print(f"Port: {port_name}")
    print("Capturing keys... keep this window open.")
    print("F6 = read log   F7 = clear log   F8 = pause/resume   F10 = quit")
    print("Close Serial Monitor first if it is open.\n")

    with keyboard.Listener(on_press=on_press) as listener:
        try:
            while listener.running:
                time.sleep(0.2)
        except KeyboardInterrupt:
            pass
        listener.stop()

    print(f"\nStopped. Keys sent this session: {state['sent']}")
    try:
        ser.close()
    except serial.SerialException:
        pass
    print("Serial port closed. Open Serial Monitor to review the log.")


if __name__ == "__main__":
    main()
