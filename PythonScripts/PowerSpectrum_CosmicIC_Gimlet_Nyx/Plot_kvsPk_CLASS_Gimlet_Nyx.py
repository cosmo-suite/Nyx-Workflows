import argparse
import numpy as np
import matplotlib.pyplot as plt
import os

def load_and_filter(file, k_col, p_col):
    data = np.loadtxt(file)
    data = data[1:, :]  # skip header

    k = data[:, k_col]
    P = data[:, p_col]

    mask = (k > 0) & (P > 0)
    return k[mask], P[mask]


def main():
    parser = argparse.ArgumentParser(description="Plot power spectra")

    # Files
    parser.add_argument('--pk-nyx-file', nargs='+', help='Nyx spectrum files')
    parser.add_argument('--pk-gimlet-file', help='Gimlet spectrum file')
    parser.add_argument('--pk-cosmicic-file', help='CosmicIC spectrum file')

    # Labels (order: Nyx..., Gimlet, CLASS)
    parser.add_argument('--labels', nargs='+', required=True,
                        help='Labels for plots (Nyx..., Gimlet, CosmicIC)')

    # Parameters
    parser.add_argument('--dofz', type=float, required=True, help='Growth factor D(z)')
    parser.add_argument('--h', type=float, required=True, help='Little h')

    parser.add_argument('--output', default="k_vs_Pk.png")

    args = parser.parse_args()

    plt.figure()

    label_idx = 0

    colors = ['b', 'g', 'c']
    # ---- Nyx files ----
    if args.pk_nyx_file:
        for nyx_file in args.pk_nyx_file:
            k, P = load_and_filter(nyx_file, 0, 1)
            plt.loglog(k / args.h, P * args.h**3,
                       label=args.labels[label_idx],color=colors[label_idx])
            label_idx += 1

    # ---- Gimlet ----
    if args.pk_gimlet_file:
        k, P = load_and_filter(args.pk_gimlet_file, 0, 3)
        plt.loglog(k, P,
                   label=args.labels[label_idx], color='r')
        label_idx += 1

    # ---- CLASS ----
    if args.pk_cosmicic_file:
        data = np.loadtxt(args.pk_cosmicic_file)
        k_cl = data[:, 0]
        Pk_cl = data[:, 1]

        plt.loglog(k_cl, args.dofz * Pk_cl,
                   label=args.labels[label_idx], color='k')
        label_idx += 1

    # ---- Plot styling ----
    plt.xlabel(r"$k\ [h\,\mathrm{Mpc}^{-1}]$")
    plt.ylabel(r"$P(k)\ [(\mathrm{Mpc}\,h^{-1})^3]$")
    plt.legend(loc="lower left")
    plt.grid(True, which="both", ls="--", alpha=0.5)
    plt.ylim([1e-10, 2.0])
    
    output_dir = "Images"

    # 2. Create the directory if it doesn't already exist
    if not os.path.exists(output_dir):
        os.makedirs(output_dir)

    # 3. Join the directory path with your filename
    # This ensures it works on both Windows and Mac/Linux
    file_path = os.path.join(output_dir, args.output)

    # 4. Save the figure
    plt.savefig(file_path, dpi=150)
    print(f"Saved combined plot to {file_path}")

if __name__ == "__main__":
    main()

