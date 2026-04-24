#!/usr/bin/env python3

import argparse
import numpy as np
import matplotlib.pyplot as plt
import sys


def load_file(filename):
    try:
        data = np.loadtxt(filename)
    except Exception as e:
        print(f"Error loading file {filename}: {e}")
        sys.exit(1)

    if data.ndim != 2 or data.shape[1] < 2:
        print(f"File {filename} must contain at least two columns.")
        sys.exit(1)

    k = data[:, 0]
    T = data[:, 1]
    return k, T


def plot_ratio(ax, lcdm_k, lcdm_T, filename, label, color, linestyle):
    k, T = load_file(filename)

    if not np.allclose(k, lcdm_k):
        print(f"Warning: k values in {filename} do not exactly match LCDM.")

    ratio = T / lcdm_T
    ax.semilogx(k, ratio, color=color, linestyle=linestyle, label=label)


def main():
    parser = argparse.ArgumentParser(
        description="Plot transfer function ratios relative to LCDM."
    )

    parser.add_argument("--file-wdm1", type=str, default=None)
    parser.add_argument("--file-wdm2", type=str, default=None)
    parser.add_argument("--file-fdm1", type=str, default=None)
    parser.add_argument("--file-fdm2", type=str, default=None)
    parser.add_argument("--file-lcdm", type=str, default=None)

    args = parser.parse_args()

    if args.file_lcdm is None:
        print("You must provide --file-lcdm to compute ratios.")
        sys.exit(1)

    lcdm_k, lcdm_T = load_file(args.file_lcdm)

    fig, ax = plt.subplots()

    # WDM (blue)
    if args.file_wdm1:
        plot_ratio(
            ax, lcdm_k, lcdm_T,
            args.file_wdm1,
            r"$m_{\mathrm{WDM}} = 0.85\,\mathrm{keV}$",
            "blue", "-"
        )

    if args.file_wdm2:
        plot_ratio(
            ax, lcdm_k, lcdm_T,
            args.file_wdm2,
            r"$m_{\mathrm{WDM}} = 2.1\,\mathrm{keV}$",
            "blue", "--"
        )

    # FDM (red)
    if args.file_fdm1:
        plot_ratio(
            ax, lcdm_k, lcdm_T,
            args.file_fdm1,
            r"$m_{\mathrm{FDM}} = 10^{-22}\,\mathrm{eV}$",
            "red", "-"
        )

    if args.file_fdm2:
        plot_ratio(
            ax, lcdm_k, lcdm_T,
            args.file_fdm2,
            r"$m_{\mathrm{FDM}} = 10^{-21}\,\mathrm{eV}$",
            "red", "--"
        )

    ax.set_xlabel(r"Wavenumber $k$")
    ax.set_ylabel(r"$T(k) / T_{\Lambda\mathrm{CDM}}(k)$")
    ax.set_title(r"Transfer Function Ratios Relative to $\Lambda$CDM")

    ax.legend()
    ax.grid(True)

    plt.tight_layout()
    plt.xlim([0.01, 100.0])

    output_file = "TF_Compare_LCDM_WDM_FDM.png"
    plt.savefig(output_file, dpi=300)
    print(f"Figure saved to {output_file}")


if __name__ == "__main__":
    main()

