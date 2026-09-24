#!/usr/bin/env python3
"""
CPU Warning sender
------------------
Reads this PC's CPU usage every second and sends it over the Arduino's
COM (USB serial) port. The Arduino turns its onboard LED ON when CPU > 60%.

Install (once, on Windows):
    pip install psutil pyserial

Run:
    python cpu_sender.py
    python cpu_sender.py --port COM6
    python cpu_sender.py --threshold 80 --interval 0.5
"""

import argparse
import sys
import time

try:
    import psutil
except ImportError:
    print("Missing library: psutil")
    print('Install with:  pip install psutil pyserial')
    sys.exit(1)

try:
    import serial
    from serial.tools import list_ports
except ImportError:
    print("Missing library: pyserial")
    print('Install with:  pip install psutil pyserial')
    sys.exit(1)


def find_arduino_port(preferred=None):
    """Return the first USB serial port that looks like the Arduino."""
    if preferred:
        return preferred

    preferred_ids = (("1A86", "7523"), ("2341", "0043"), ("2341", "0041"))
    candidates = list(list_ports.comports())

    # Prefer CH340 (Salman's Uno clone) or official Arduino USB IDs
    for p in candidates:
        vid = f"{p.vid:04X}" if p.vid is not None else ""
        pid = f"{p.pid:04X}" if p.pid is not None else ""
        if (vid, pid) in preferred_ids:
            return p.device

    # Fallback: any USB serial port
    for p in candidates:
        if p.vid is not None:
            return p.device

    return None


def main():
    parser = argparse.ArgumentParser(description="Send CPU usage to Arduino over COM port")
    parser.add_argument("--port", help="COM port, e.g. COM6 (auto-detect if omitted)")
    parser.add_argument("--threshold", type=int, default=60, help="LED warn threshold percent (default 60)")
    parser.add_argument("--interval", type=float, default=1.0, help="Seconds between readings (default 1.0)")
    args = parser.parse_args()

    port_name = find_arduino_port(args.port)
    if not port_name:
        print("No Arduino COM port found.")
        print("Plug in the board, then run again, or pass --port COM6")
        print("Tip: find the port with:")
        print(r'  C:\Users\Public\arduino-cli.exe board list --discovery-timeout 5s')
        sys.exit(1)

    print(f"Opening {port_name} at 9600 baud...")
    try:
        ser = serial.Serial(port_name, 9600, timeout=1)
    except serial.SerialException as e:
        print(f"Could not open {port_name}: {e}")
        print("If access is denied, close the Arduino Serial Monitor first.")
        sys.exit(1)

    time.sleep(2)  # wait for Arduino auto-reset after opening the port
    print(f"Sending CPU usage every {args.interval}s  (LED ON if CPU > {args.threshold}%)")
    print("Press Ctrl+C to stop.\n")

    try:
        while True:
            cpu = psutil.cpu_percent(interval=None)
            line = f"{int(round(cpu))}\n"
            ser.write(line.encode("ascii", errors="ignore"))

            status = "WARN (LED ON)" if cpu > args.threshold else "OK"
            print(f"  CPU {cpu:5.1f}%  ->  {line.strip():>3}  {status}", end="\r")

            # Read Arduino reply without blocking long
            time.sleep(min(args.interval, 0.05))
            deadline = time.time() + max(args.interval - 0.05, 0)
            while time.time() < deadline:
                if ser.in_waiting:
                    raw = ser.readline().decode("ascii", errors="ignore").strip()
                    if raw:
                        print(f"\n  Arduino: {raw}")
                else:
                    time.sleep(0.05)

    except KeyboardInterrupt:
        print("\nStopping... sending LED off command.")
        try:
            ser.write(b"0\n")
        except serial.SerialException:
            pass
        ser.close()
        print("Done. Arduino LED should be OFF.")


if __name__ == "__main__":
    main()
