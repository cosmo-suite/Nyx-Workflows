import numpy as np
import matplotlib.pyplot as plt
import argparse
import os
import re


# -----------------------------
# Read halo file
# -----------------------------
def read_binary_points(filename):
    try:
        with open(filename, 'rb') as f:
            data = np.fromfile(f, dtype='>f4')
    except Exception as e:
        raise RuntimeError(f"Could not read file: {e}")

    if data.size == 0:
        raise RuntimeError(f"Empty halo file: {filename}")

    if data.size % 5 != 0:
        raise RuntimeError(f"Corrupted halo file: {filename}")

    return data.reshape((-1, 5))


# -----------------------------
# Extract redshift from filename
# -----------------------------
def extract_redshift(filename):
    base = os.path.basename(filename)

    match = re.search(r'_(\d+)\.bin', base)

    if not match:
        return None

    return int(match.group(1)) / 10000.0


# =========================================================
# Growth factor (simple LCDM approx, normalized at z=0)
# =========================================================
def growth_factor(z):
    return 1.0 / (1.0 + z)


# =========================================================
# Toy sigma(M) model
# =========================================================
def sigma_M(mass, z, sigma8=0.8, M8=1e14, alpha=0.3):
    D = growth_factor(z)

    return sigma8 * D * (mass / M8) ** (-alpha)


# =========================================================
# Sheth–Tormen mass function
# =========================================================
def sheth_tormen_hmf(mass, z):
    A = 0.3222
    a = 0.707
    p = 0.3

    sig = sigma_M(mass, z)

    dlns_dlnm = -0.3

    nu = np.sqrt(a) * (1.0 / sig)

    f_nu = (
        A
        * np.sqrt(2.0 / np.pi)
        * nu
        * np.exp(-0.5 * nu**2)
        * (1 + (nu ** (-2 * p)))
    )

    Omega_m = 0.3
    h = 0.7

    rho_m = Omega_m * 2.775e11 * h**2

    dn_dlnM = (rho_m / mass) * f_nu * np.abs(dlns_dlnm)

    return dn_dlnM


# -----------------------------
# HMF computation
# -----------------------------
def compute_hmf(masses, volume, bins):

    logM = np.log10(masses)

    counts, edges = np.histogram(logM, bins=bins)

    dlogM = edges[1] - edges[0]

    centers = 0.5 * (edges[:-1] + edges[1:])

    hmf = counts / (volume * dlogM)

    # Remove zero-count bins
    mask = counts > 0

    return centers[mask], hmf[mask]


# -----------------------------
# Main
# -----------------------------
def main():

    parser = argparse.ArgumentParser(
        description="Multi-snapshot Halo Mass Function"
    )

    parser.add_argument(
        "--halo-files",
        nargs='+',
        required=True,
        help="List of halo files"
    )

    parser.add_argument(
        "--legend",
        nargs='+',
        required=True,
        help="Legend labels corresponding to halo files"
    )

    parser.add_argument(
        "--min-ncells",
        type=int,
        default=20,
        help="Minimum number of cells per halo"
    )

    parser.add_argument(
        "--box-size",
        type=float,
        default=20.0,
        help="Box size in Mpc/h"
    )

    args = parser.parse_args()

    if len(args.halo_files) != len(args.legend):
        raise RuntimeError(
            "Number of halo files must match number of legend labels"
        )

    volume = args.box_size ** 3

    bins = np.linspace(7, 13.5, 30)

    plt.figure()

    st_plotted = False

    z = 0;

    colors = ['r', 'b', 'g', 'c'];
    count = 0;

    for halo_file, legend_label in zip(args.halo_files, args.legend):

        z = extract_redshift(halo_file)
        print(f"[INFO] Reading {halo_file}")

        data = read_binary_points(halo_file)

        masses = data[:, 3]
        ncells = data[:, 4]


        mask = ncells >= args.min_ncells

        masses = masses[mask]
        print("Number of halos is ", np.size(masses))

        if len(masses) == 0:
            print(f"[WARNING] No halos remaining after cut for {halo_file}")
            continue

        logM, hmf = compute_hmf(masses, volume, bins)

        plt.loglog(
            10**logM,
            hmf,
            marker='o',
            markersize=3,
            color=colors[count],
            linewidth=1.5,
            label=legend_label
        )
        count += 1


    if z is not None:

        m_eval = np.logspace(7, 13.5, 200)

        st = sheth_tormen_hmf(m_eval, z)

        plt.loglog(m_eval,
                   st,
                   linestyle='-',
                   linewidth=2,
                   color='k', 
                   label=f"Sheth-Tormen (z={z:.2f})")

    plt.xscale("log")
    plt.yscale("log")

    plt.xlabel(r"Halo mass [$M_\odot$]")

    plt.ylabel(r"$dn/d\log M$ [(Mpc/$h$)$^{-3}$]")

    plt.grid(True, which="both", ls="--", alpha=0.5)

    plt.legend()

    #plt.tight_layout()

    outname = "hmf.png"

    plt.savefig(outname, dpi=200)

    plt.close()

    print(f"[OK] Saved plot → {outname}")


if __name__ == "__main__":
    main()

