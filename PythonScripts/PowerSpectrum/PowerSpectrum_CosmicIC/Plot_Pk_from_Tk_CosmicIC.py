import numpy as np
import argparse
import matplotlib.pyplot as plt
import scipy.integrate as integrate
import os

# Top-hat window in Fourier space
def W_tophat(kR):
    kR = np.where(kR == 0, 1e-10, kR)  # avoid divide by zero
    return 3 * (np.sin(kR) - kR * np.cos(kR)) / kR**3

def sigma_r(k, Pk, R):
    """Compute sigma_R from 1D P(k)"""
    W = W_tophat(k * R)
    integrand = k**2 * Pk * W**2
    return np.sqrt(integrate.simpson(integrand, k) / (2*np.pi**2))

def compute_pk(tk_file, n_s, sigma8_target, h):
    # Read CLASS tk.dat (positive k only)
    data = np.loadtxt(tk_file)
    k_hmpc = data[:, 0]           # k in h/Mpc
    T_tot = np.abs(data[:, 1]*data[0,1])    # total matter transfer function

    T_tot[0] = T_tot[0]/data[0,1]
    

    # Unnormalized P(k)
    Pk_unnorm = T_tot**2 * k_hmpc**n_s

    # Normalize to match sigma8
    s8_current = sigma_r(k_hmpc, Pk_unnorm, R=8.0)
    norm = sigma8_target**2 / s8_current**2
    print("Value of norm is ", norm)
    Pk = norm * Pk_unnorm

    s8_final = sigma_r(k_hmpc, Pk, R=8.0)
    print(f"sigma_8 after normalization: {s8_final:.5f}, target: {sigma8_target}")
    print(f"Normalization factor applied: {norm:.5e}")

    return k_hmpc, Pk

def plot_pk(k, Pk, output_file="Pk_sigma8.png"):
    plt.figure()
    plt.loglog(k, Pk)
    plt.xlabel("k [h/Mpc]")
    plt.ylabel("P(k) [(Mpc/h)^3]")
    plt.title("Matter Power Spectrum normalized to sigma8")
    plt.grid(True, which="both")
    plt.savefig(output_file, dpi=150, bbox_inches='tight')
    print(f"Saved plot to {output_file}")


if __name__ == "__main__":
    parser = argparse.ArgumentParser()

    parser.add_argument("--tk-files", nargs="+", required=True,
                        help="List of transfer function files")

    parser.add_argument("--labels", nargs="*",
                        help="Optional labels for each file (same order)")

    parser.add_argument("--n_s", type=float, required=True)
    parser.add_argument("--sigma8", type=float, required=True)
    parser.add_argument("--h", type=float, required=True)
    parser.add_argument("--output", type=str, default="Pk_final.png")
    parser.add_argument("--output-pk-files", nargs="+", required=True,
                    help="Output filename(s) for P(k). Either one base name or one per tk-file")

    args = parser.parse_args()

    if len(args.output_pk_files) != len(args.tk_files):
        raise ValueError("Number of spectrum filenames must match number of tk-files")

    # Safety check
    if args.labels and len(args.labels) != len(args.tk_files):
        raise ValueError("Number of labels must match number of tk-files")

    plt.figure()
    colors = ["black", "blue", "red","green"]

    for i, filepath in enumerate(args.tk_files):
        k, Pk = compute_pk(filepath, args.n_s, args.sigma8, args.h)

        if args.labels:
            label = args.labels[i]
        else:
            label = os.path.splitext(os.path.basename(filepath))[0]

        plt.loglog(k, Pk, label=label, color=colors[i])
        outname = args.output_pk_files[i]

        np.savetxt(outname,
                   np.column_stack([k, Pk]),
                   header="k [h/Mpc]    P(k) [(Mpc/h)^3]")

        print("Saved spectrum to {}".format(outname))

    plt.xlabel(r"$k\ [h\,\mathrm{Mpc}^{-1}]$")
    plt.ylabel(r"$P(k)\ [(\mathrm{Mpc}\,h^{-1})^3]$")
    plt.grid(True, which="both")
    plt.ylim([1e-6, 3e4])
    plt.legend()

# 1. Define the directory name
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

