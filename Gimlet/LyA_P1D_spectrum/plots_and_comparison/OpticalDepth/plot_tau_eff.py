#!/usr/bin/env python3
"""Plot Ly-alpha effective optical depth vs redshift from tau_eff_vs_z.txt.

Usage: python plot_tau_eff.py [tau_eff_vs_z.txt] [output.png] [comparison.txt]
"""
import sys

import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt
import numpy as np

infile = sys.argv[1] if len(sys.argv) > 1 else "tau_eff_vs_z.txt"
outfile = sys.argv[2] if len(sys.argv) > 2 else "tau_eff_vs_z.png"
compfile = sys.argv[3] if len(sys.argv) > 3 else "tau_eff_vs_z_HarrdtAndMadau.txt"

data = np.atleast_2d(np.loadtxt(infile))
order = np.argsort(data[:, 0])
z, tau = data[order, 0], data[order, 1]

# Comparison data (Haardt & Madau).
comp = np.atleast_2d(np.loadtxt(compfile))
comp = comp[np.argsort(comp[:, 0])]

fig, ax = plt.subplots(figsize=(6, 4.5))
ax.plot(comp[:, 0], comp[:, 1], "o-", color="black", label="Haardt & Madau")
ax.plot(z, tau, "o-", color="blue", label="Nyx (LCDM)")
ax.set_yscale("log")
ax.set_xlabel("Redshift $z$")
ax.set_ylabel(r"$\tau_{\rm eff} = -\ln\langle F\rangle$")
ax.set_title(r"Evolution of the Ly$\alpha$ effective optical depth")
ax.grid(True, which="both", alpha=0.3)
ax.legend()
fig.tight_layout()
fig.savefig(outfile, dpi=200)
print(f"Wrote {outfile}")
