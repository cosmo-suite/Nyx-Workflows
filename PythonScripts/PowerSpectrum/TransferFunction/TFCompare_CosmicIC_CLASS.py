#!/usr/bin/env python3

import argparse
import numpy as np
import matplotlib.pyplot as plt


def load_and_normalize(fname):
    # Load 7-column data
    data = np.loadtxt(fname)

    x = data[:, 0]
    col2 = data[:, 1]
    col3 = data[:, 2]

    # Normalize
    col2_norm = col2 / col2[0]
    col3_norm = col3 / col3[0]

    return x, col2_norm, col3_norm


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--file_cosmicic", required=True)
    parser.add_argument("--file_class", required=True)
    args = parser.parse_args()

    x_c, c2_c, c3_c = load_and_normalize(args.file_cosmicic)
    x_cl, c2_cl, c3_cl = load_and_normalize(args.file_class)

    # -------- Figure 1: Column 2 --------
    plt.figure()
    plt.loglog(x_c, c2_c, '-o', label="cosmicic", markersize=3,markerfacecolor="none")
    plt.loglog(x_cl, c2_cl, label="class")
    plt.xlabel("Column 1")
    plt.ylabel("Normalized Column 2")
    plt.legend()
    plt.tight_layout()
    plt.savefig("fig1_col2.png", dpi=300)

    # -------- Figure 2: Column 3 --------
    plt.figure()
    plt.loglog(x_c, c3_c, '-o', label="cosmicic", markersize=3,markerfacecolor="none")
    plt.loglog(x_cl, c3_cl, label="class")
    plt.xlabel("Column 1")
    plt.ylabel("Normalized Column 3")
    plt.legend()
    plt.tight_layout()
    plt.savefig("fig2_col3.png", dpi=300)


if __name__ == "__main__":
    main()

