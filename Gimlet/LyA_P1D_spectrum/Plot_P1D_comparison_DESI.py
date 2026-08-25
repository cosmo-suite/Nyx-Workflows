import argparse
import numpy as np
import matplotlib.pyplot as plt
from astropy.io import fits

def main():
    parser = argparse.ArgumentParser(
        description="Plot DESI DR1 Ly-alpha 1D Flux Power Spectrum against Nyx data."
    )
    parser.add_argument(
        "--desi-data",
        type=str,
        required=True,
        help="Path to the DESI baseline .fits file",
    )
    parser.add_argument(
        "--nyx-data",
        type=str,
        required=True,
        help="Path to the Nyx .txt data file",
    )
    parser.add_argument(
        "--z-target",
        type=float,
        required=True,
        help="Target redshift to filter",
    )
    parser.add_argument(
        "--output",
        type=str,
        required=True,
        help="Output image filename (e.g., plot.png)",
    )
    args = parser.parse_args()

    fname = args.desi_data

    with fits.open(fname) as hdul:
        data = hdul["P1D_BLIND"].data
        # print(hdul["P1D_BLIND"].header)

    # Select redshift from user input
    z_target = args.z_target
    tol = 1e-6

    mask = np.isclose(data["Z"], z_target, atol=tol)

    k = data["K"][mask]          # s/km
    P1D = data["PLYA"][mask]     # km/s
    err = data["E_PK"][mask]     # km/s

    k = k[0:70]
    P1D = P1D[0:70]
    err = err[0:70]

    # k P_1D
    kP1D = k * P1D

    # Propagated uncertainty:
    # sigma(k P) = k * sigma(P), since k has negligible uncertainty
    kP1D_err = k * err

    plt.figure(figsize=(8, 6))

    plt.errorbar(
        k,
        kP1D,
        yerr=kP1D_err,
        fmt="o",
        markersize=4,
        capsize=2,
        label=rf"DESI DR1, $z={z_target}$"
    )

    data = np.loadtxt(args.nyx_data)
    k_nyx = data[:, 0]
    k_nyx_P1D_nyx = data[:, 1] * 3.1416
    plt.plot(k_nyx, k_nyx_P1D_nyx)

    plt.xscale("log")
    plt.yscale("log")

    plt.ylim([1e-1, 1e0])
    plt.xlim([1e-3, 2e-1])

    plt.xlabel(r"$k\ [{\rm s/km}]$")
    plt.ylabel(r"$kP_{\rm 1D}(k)$")
    plt.title(r"DESI DR1 Ly$\alpha$ 1D Flux Power Spectrum")

    plt.grid(True, which="both", alpha=0.3)
    plt.legend()

    plt.tight_layout()
    plt.savefig(args.output, dpi=300)
    plt.show()

if __name__ == "__main__":
    main()
