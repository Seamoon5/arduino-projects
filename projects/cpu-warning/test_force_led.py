#!/usr/bin/env python3
"""Force CPU=99 to test if Arduino LED turns on."""
import time
import serial

PORT = "COM6"
BAUD = 9600

print(f"Opening {PORT}...")
ser = serial.Serial(PORT, BAUD, timeout=1)
time.sleep(2)  # wait for Uno auto-reset

print("Sending 99 for 8 seconds - LED should turn ON...")
for i in range(8):
    ser.write(b"99\n")
    ser.flush()
    time.sleep(0.5)
    while ser.in_waiting:
        line = ser.readline().decode("ascii", errors="ignore").strip()
        if line:
            print(f"  Arduino says: {line}")

print("Sending 0 for 3 seconds - LED should turn OFF...")
for i in range(6):
    ser.write(b"0\n")
    ser.flush()
    time.sleep(0.5)
    while ser.in_waiting:
        line = ser.readline().decode("ascii", errors="ignore").strip()
        if line:
            print(f"  Arduino says: {line}")

ser.close()
print("Test finished.")
