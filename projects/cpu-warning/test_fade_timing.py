#!/usr/bin/env python3
"""Verify 5s hold + fade while continuously sending CPU samples."""
import time
import serial

PORT = "COM6"
print(f"Opening {PORT}...")
ser = serial.Serial(PORT, 9600, timeout=1)
time.sleep(2)

while ser.in_waiting:
    print("  boot:", ser.readline().decode(errors="ignore").strip())

print("Sending 0 then continuous 99 for 9 seconds...")
ser.write(b"0\n")
time.sleep(0.4)

start = time.time()
next_send = start
while time.time() - start < 9:
    now = time.time()
    if now >= next_send:
        ser.write(b"99\n")
        next_send = now + 0.5
    if ser.in_waiting:
        line = ser.readline().decode(errors="ignore").strip()
        if line:
            print(f"  +{time.time()-start:4.1f}s  {line}")
    else:
        time.sleep(0.05)

print("Sending 0 (CPU recovered)...")
ser.write(b"0\n")
# keep sending 0 for a bit
end = time.time() + 4
next_send = time.time()
while time.time() < end:
    now = time.time()
    if now >= next_send:
        ser.write(b"0\n")
        next_send = now + 0.5
    if ser.in_waiting:
        line = ser.readline().decode(errors="ignore").strip()
        if line:
            print(" ", line)
    else:
        time.sleep(0.05)

ser.close()
print("Done.")
