import argparse
import numpy as np
import matplotlib.pyplot as plt
import os
import re

def extract_index(filename):
    base = os.path.basename(filename)
    return int(re.search(r"(\d+)", base).group(1))


def load_and_filter(file, k_col, p_col):
    data = np.loadtxt(file)
    data = data[1:, :]  # skip header if present

    k = data[:, k_col]
    P = data[:, p_col]

    mask = (k > 0) & (P > 0)
    return k[mask], P[mask]

def extract_redshift(filename):
    """
    Extract redshift from filenames like:
    spectrum_2000000.txt -> 200.0
    spectrum_0199999.txt -> ~20.0
    """
    base = os.path.basename(filename)
    match = re.search(r"(\d+)", base)

    if not match:
        raise ValueError(f"Cannot parse redshift from {filename}")

    num = float(match.group(1))

    return num / 10000.0


def main():
    parser = argparse.ArgumentParser(description="Plot Nyx vs CLASS power spectra")

    # Nyx file list (NOW a single file containing filenames)
    parser.add_argument('--pk-nyx-files', required=True,
                        help='Text file containing list of Nyx spectrum files')

    # CLASS / CosmicIC file
    parser.add_argument('--pk-cosmicic-file', required=True,
                        help='CLASS/CosmicIC power spectrum file')

    # growth factor (1-column, assumed aligned with sorted Nyx files)
    parser.add_argument('--dofz-file', required=True,
                        help='1-column file containing D(z) values')

    parser.add_argument('--h', type=float, required=True,
                        help='Little h')

    parser.add_argument('--output', default="k_vs_Pk.png",
                        help='Output image filename')

    args = parser.parse_args()

    plt.figure()

    # -------------------------
    # Load Nyx file list
    # -------------------------
    with open(args.pk_nyx_files, 'r') as f:
        nyx_files = [
            line.split("#")[0].strip()
            for line in f
            if line.strip()
        ]

    nyx_files = sorted(nyx_files, key=extract_index)

    # -------------------------
    # Load growth factors
    # -------------------------
    D_of_z = np.loadtxt(args.dofz_file)

    if len(D_of_z) != len(nyx_files):
        raise ValueError(
            f"Mismatch: {len(D_of_z)} D(z) values vs {len(nyx_files)} Nyx files"
        )

    # -------------------------
    # Load CLASS spectrum once
    # -------------------------
    data_cl = np.loadtxt(args.pk_cosmicic_file)
    k_cl = data_cl[:, 0]
    Pk_cl = data_cl[:, 1]

    # -------------------------
    # Plot loop
    # -------------------------
    #colors = plt.cm.viridis(np.linspace(0, 1, len(nyx_files)))

    colors = ['k', 'r', 'b', 'g', 'c' ]

    print("The ny_files are", nyx_files);

    for i, nyx_file in enumerate(nyx_files):
        k_nyx, P_nyx = load_and_filter(nyx_file, 0, 1)

        z = extract_redshift(nyx_file)
        print("nyx file and z is ", nyx_file, z)
        D = D_of_z[i]

        print("Value of D is ", D)

        # Nyx (unscaled)
        plt.loglog(
            k_nyx / args.h,
            k_nyx**3*P_nyx/(2.0*np.pi**2),
            color=colors[i],
            linestyle='--',
            label=f"z={z:.2f}"
        )

        # CLASS scaled by D(z)^2
        Pk_scaled = Pk_cl * (D**2)

        plt.loglog(
            k_cl,
            k_cl**3*Pk_scaled/(2*(np.pi**2)),
            color=colors[i],
            linestyle='-'
        )


    # 2. Add a single reference line for the shot noise floor
    P_shot = 0.00002422583
    plt.loglog(
        k_nyx / args.h,
        k_nyx**3 * P_shot / (2.0 * np.pi**2),
        color='gray',
        linestyle=':',
        label='Poisson shot noise $V/N_p$'
    )



    # -------------------------
    # Plot styling
    # -------------------------
    plt.xlabel(r"$k\ [h\,\mathrm{Mpc}^{-1}]$")
    plt.ylabel(r"$k^3 P(k)\ / (2 \pi^2)$")
    plt.legend(loc="upper left")
    plt.grid(True, which="both", ls="--", alpha=0.5)
    plt.ylim([1e-10, 5000.0])

    # -------------------------
    # Save output
    # -------------------------
    os.makedirs("Images", exist_ok=True)
    out_path = os.path.join("Images", args.output)

    plt.savefig(out_path, dpi=150)
    print(f"Saved combined plot to {out_path}")



if __name__ == "__main__":
    main()
