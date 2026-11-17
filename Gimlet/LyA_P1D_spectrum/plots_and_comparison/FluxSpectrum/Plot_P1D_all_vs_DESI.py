import argparse
import glob
import os
import re

import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt
import numpy as np
from astropy.io import fits

BASE = os.path.dirname(os.path.abspath(__file__))
Z_MAX = 4.2
# Extra comparison curves (k, value) drawn only at these redshifts.
COMPARISON_FILES = {
    3.0: "Irsic_2017/Irsic_LyA_P1d_z_eq_3p0.csv",
    3.6: "Irsic_2017/Irsic_LyA_P1d_z_eq_3p6.csv",
    4.2: "Irsic_2017/Irsic_LyA_P1d_z_eq_4p2.csv",
}
WALTHER_FILES = {
    2.4: "Walther_2018/Walther_LyA_P1d_z_eq_2p4.csv",
    2.6: "Walther_2018/Walther_LyA_P1d_z_eq_2p6.csv",
    2.8: "Walther_2018/Walther_LyA_P1d_z_eq_2p8.csv",
    3.0: "Walther_2018/Walther_LyA_P1d_z_eq_3p0.csv",
    3.2: "Walther_2018/Walther_LyA_P1d_z_eq_3p2.csv",
    3.4: "Walther_2018/Walther_LyA_P1d_z_eq_3p4.csv",
}


def read_z(filename):
    """Read redshift from the header line '# ... z=8.499945'."""
    with open(filename) as f:
        header = f.readline()
    m = re.search(r"z=([-+0-9.eE]+)", header)
    return float(m.group(1)) if m else None


def main():
    p = argparse.ArgumentParser(
        description="Compare every Nyx P1D file with z<=4.2 to DESI DR1."
    )
    p.add_argument("--input-dir", default=os.path.join(BASE, "p1d_output"))
    p.add_argument("--output-dir", default=None,
                   help="Default: <input-dir>/Images")
    p.add_argument("--desi-data", default=os.path.join(
        BASE, "DESI_data/zenodo_p1d_fft_y1/"
        "p1d_fft_y1_measurement_kms_v8_baseline.fits"))
    p.add_argument("--comparison-dir",
                   default=os.path.join(BASE, "DataForComparison"))
    p.add_argument("--legend", default="CDM")
    args = p.parse_args()

    out_dir = args.output_dir or os.path.join(args.input_dir, "Images")
    os.makedirs(out_dir, exist_ok=True)

    with fits.open(args.desi_data) as hdul:
        desi = hdul["P1D_BLIND"].data
    desi_z = np.unique(desi["Z"])

    files = sorted(glob.glob(os.path.join(args.input_dir, "*_ave_ps1d.txt")))
    nplots = 0
    for fn in files:
        z = read_z(fn)
        name = os.path.basename(fn)
        if z is None:
            print(f"skip {name}: no z in header")
            continue
        if z > Z_MAX:
            print(f"skip {name}: z={z:.4f} > {Z_MAX}")
            continue

        # DESI is binned in discrete z; use the nearest bin.
        z_desi = desi_z[np.argmin(np.abs(desi_z - z))]
        mask = np.isclose(desi["Z"], z_desi, atol=1e-6)
        k = desi["K"][mask][:70]
        kP = k * desi["PLYA"][mask][:70]
        kP_err = k * desi["E_PK"][mask][:70]

        d = np.loadtxt(fn)
        kn, Pn = d[:, 0], d[:, 1] * 3.1416
        sel = kn < 0.1

        fig = plt.figure(figsize=(8, 6))
        plt.errorbar(k, kP, yerr=kP_err, fmt="o", markersize=4, capsize=2,
                     label=rf"DESI DR1, $z={z_desi:.1f}$")
        plt.plot(kn[sel], Pn[sel], label=f"{args.legend}, $z={z:.3f}$")
        for zc, cfile in COMPARISON_FILES.items():
            if abs(z - zc) < 0.01:
                c = np.loadtxt(os.path.join(args.comparison_dir, cfile), skiprows=1)
                # Source plots divide by pi, so multiply it back.
                plt.plot(c[:, 0], c[:, 1] * np.pi, "--", color="k",
                         label=rf"Irsic et al., $z={zc:.1f}$")
        for zc, cfile in WALTHER_FILES.items():
            if abs(z - zc) < 0.01:
                w = np.loadtxt(os.path.join(args.comparison_dir, cfile), skiprows=1)
                # Each consecutive row pair (0-1, 2-3, ...) is the two ends
                # of one error bar; scale by pi like the other comparisons.
                kw = 0.5 * (w[0::2, 0] + w[1::2, 0])
                lo = np.minimum(w[0::2, 1], w[1::2, 1]) * np.pi
                hi = np.maximum(w[0::2, 1], w[1::2, 1]) * np.pi
                plt.errorbar(kw, 0.5 * (lo + hi), yerr=0.5 * (hi - lo),
                             fmt="s", color="tab:red", markersize=4,
                             capsize=2,
                             label=rf"Walther et al., $z={zc:.1f}$")
        plt.xscale("log")
        plt.yscale("log")
        plt.xlabel(r"$k\ [{\rm s/km}]$")
        plt.ylabel(r"$kP_{\rm 1D}(k)$")
        plt.title(rf"Nyx $z={z:.3f}$ vs DESI DR1 $z={z_desi:.1f}$")
        plt.grid(True, which="both", alpha=0.3)
        plt.legend()

        stem = name.replace("_ave_ps1d.txt", "").rstrip("_")
        out = os.path.join(out_dir, f"{stem}_P1D_vs_DESI.png")
        fig.savefig(out, dpi=300)
        plt.close(fig)
        nplots += 1
        print(f"saved {out} (Nyx z={z:.4f}, DESI bin z={z_desi:.1f})")

    print(f"Done: {nplots} plot(s) of {len(files)} file(s).")


if __name__ == "__main__":
    main()
