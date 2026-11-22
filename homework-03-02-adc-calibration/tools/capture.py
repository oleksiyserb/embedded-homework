"""Read the ADC table from the serial port for N seconds and save it as CSV.

Usage: python capture.py [port] [seconds] [out.csv]
"""
import csv
import re
import sys
import time

import serial

port = sys.argv[1] if len(sys.argv) > 1 else "COM3"
seconds = float(sys.argv[2]) if len(sys.argv) > 2 else 60
out = sys.argv[3] if len(sys.argv) > 3 else "data.csv"

# " 512   412.6   405   1.88   3281"  (error may be "-" when U_cali == 0)
ROW = re.compile(r"^\s*(\d+)\s+([\d.]+)\s+(\d+)\s+([\d.]+|-)\s+(\d+)\s*$")

rows = []
with serial.Serial(port, 115200, timeout=0.5) as ser:
    end = time.time() + seconds
    while time.time() < end:
        m = ROW.match(ser.readline().decode(errors="ignore"))
        if m:
            rows.append(m.groups())

with open(out, "w", newline="") as f:
    w = csv.writer(f)
    w.writerow(["raw", "u_manual_mv", "u_cali_mv", "error_pct", "vdda_mv"])
    w.writerows(rows)

print(f"{len(rows)} rows -> {out}")
