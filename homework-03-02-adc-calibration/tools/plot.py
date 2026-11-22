"""Plot data.csv from capture.py.

Usage: python plot.py [data.csv] [out.png]
"""
import csv
import sys

import matplotlib.pyplot as plt

src = sys.argv[1] if len(sys.argv) > 1 else "data.csv"
out = sys.argv[2] if len(sys.argv) > 2 else "plots.png"

rows = list(csv.DictReader(open(src)))
raw = [int(r["raw"]) for r in rows]
u_manual = [float(r["u_manual_mv"]) for r in rows]
u_cali = [int(r["u_cali_mv"]) for r in rows]
err = [(int(r["raw"]), float(r["error_pct"])) for r in rows if r["error_pct"] != "-"]

fig, (ax1, ax2, ax3) = plt.subplots(3, 1, figsize=(9, 12))

ax1.scatter(raw, u_manual, s=8, label="U_manual (VDDA = 3300 mV)")
ax1.scatter(raw, u_cali, s=8, label="U_cali (VDDA via VREFINT)")
ax1.set(title="Voltage vs RAW", xlabel="RAW", ylabel="mV")
ax1.legend()
ax1.grid(True)

ax2.scatter(*zip(*err), s=8, color="tab:red")
ax2.set(title="Error vs RAW", xlabel="RAW", ylabel="Error, %", yscale="log")
ax2.axvspan(0, 100, color="tab:red", alpha=0.1, label="RAW < 100: 1 mV rounding dominates")
ax2.legend()
ax2.grid(True, which="both")

ax3.plot(raw, marker=".", markersize=3, linewidth=0.5)
ax3.set(title="RAW over time (pot rotated 0 -> max)", xlabel="sample (~100 ms)", ylabel="RAW")
ax3.axhline(4095, color="tab:red", linestyle="--", label="4095 = ADC full scale")
ax3.legend()
ax3.grid(True)

fig.tight_layout()
fig.savefig(out, dpi=120)
print(f"-> {out}")
