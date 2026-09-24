#!/usr/bin/env python3
"""Non-interactive protocol test for Arduino Key Logger."""

import sys
import time

try:
    import serial
    from serial.tools import list_ports
except ImportError:
    print("Missing pyserial. Install: pip install pyserial")
    sys.exit(1)


def find_port():
    preferred_ids = (("1A86", "7523"), ("2341", "0043"), ("2341", "0041"))
    for p in list(list_ports.comports()):
        vid = f"{p.vid:04X}" if p.vid is not None else ""
        pid = f"{p.pid:04X}" if p.pid is not None else ""
        if (vid, pid) in preferred_ids:
            return p.device
    for p in list(list_ports.comports()):
        if p.vid is not None:
            return p.device
    return None


def read_until(ser, end_prefix, timeout=3.0):
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


def cmd(ser, text, end_prefix=None, timeout=3.0):
    ser.reset_input_buffer()
    ser.write((text + "\n").encode("ascii"))
    if end_prefix is None:
        deadline = time.time() + timeout
        while time.time() < deadline:
            raw = ser.readline()
            if raw:
                return raw.decode("ascii", errors="ignore").rstrip("\r\n")
        return ""
    return read_until(ser, end_prefix, timeout)


def main():
    port = find_port()
    if not port:
        print("FAIL: no Arduino port")
        sys.exit(1)
    print(f"Port: {port}")
    ser = serial.Serial(port, 9600, timeout=0.3)
    time.sleep(2)
    banner = read_until(ser, "Tip:", timeout=2.0)
    if not banner:
        print("FAIL: no Arduino banner")
        sys.exit(1)
    print("Banner OK")

    reply = cmd(ser, "clear")
    if "CLEARED" not in reply:
        print(f"FAIL clear: {reply}")
        sys.exit(1)
    print("clear OK")

    reply = cmd(ser, "count")
    if not reply.startswith("COUNT 0"):
        print(f"FAIL count0: {reply}")
        sys.exit(1)
    print("count=0 OK")

    ser.write(b"+Hello\n")
    time.sleep(0.3)
    reply = cmd(ser, "count")
    if not reply.startswith("COUNT 5"):
        print(f"FAIL count5: {reply}")
        sys.exit(1)
    print("count=5 OK")

    ser.write(b"+ World\\n\n")
    time.sleep(0.3)
    lines = cmd(ser, "read", end_prefix="--- END")
    blob = "\n".join(lines)
    if "Hello World" not in blob:
        print(f"FAIL read: {blob}")
        sys.exit(1)
    reply = cmd(ser, "count")
    if not reply.startswith("COUNT 12"):
        print(f"FAIL read count: {reply}")
        sys.exit(1)
    print("read OK")

    reply = cmd(ser, "find World")
    if "FOUND" not in reply:
        print(f"FAIL find: {reply}")
        sys.exit(1)
    print("find OK")

    reply = cmd(ser, "find ZZZNOPE")
    if "NOT FOUND" not in reply:
        print(f"FAIL find-neg: {reply}")
        sys.exit(1)
    print("find-miss OK")

    reply = cmd(ser, "pause")
    if "PAUSED" not in reply:
        print(f"FAIL pause: {reply}")
        sys.exit(1)
    ser.write(b"+X\n")
    time.sleep(0.2)
    reply = cmd(ser, "count")
    expected = None
    for line in reply.splitlines():
        if line.startswith("COUNT "):
            expected = int(line.split()[1])
    if expected is None:
        print(f"FAIL paused count: {reply}")
        sys.exit(1)
    if expected >= 14:
        print(f"FAIL paused accepted data: {reply}")
        sys.exit(1)
    print("pause OK")

    reply = cmd(ser, "resume")
    if "RESUMED" not in reply:
        print(f"FAIL resume: {reply}")
        sys.exit(1)
    print("resume OK")

    reply = cmd(ser, "status")
    if "STATUS" not in reply:
        print(f"FAIL status: {reply}")
        sys.exit(1)
    print("status OK")

    reply = cmd(ser, "clear")
    if "CLEARED" not in reply:
        print(f"FAIL final clear: {reply}")
        sys.exit(1)
    ser.close()
    print("ALL TESTS PASSED")


if __name__ == "__main__":
    main()
