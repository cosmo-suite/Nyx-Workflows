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
    data = np.loadtxt(tk_file, skiprows=8)
    k_hmpc = data[:, 0]           # k in h/Mpc
    T_tot = np.abs(data[:, 6])    # total matter transfer function

    # Unnormalized P(k)
    Pk_unnorm = T_tot**2 * k_hmpc**n_s

    # Normalize to match sigma8
    s8_current = sigma_r(k_hmpc, Pk_unnorm, R=8.0)
    norm = sigma8_target**2 / s8_current**2
    Pk = norm * Pk_unnorm

    s8_final = sigma_r(k_hmpc, Pk, R=8.0)
    print(f"sigma_8 after normalization: {s8_final:.5f}, target: {sigma8_target}")
    print(f"Normalization factor applied: {norm:.5e}")

    return k_hmpc, Pk

def plot_pk(k, Pk_from_Tk, Pk_CLASS, output_file="Pk_sigma8.png"):
    plt.figure()
    plt.loglog(k, Pk_from_Tk, 'o', color='b', label="Pk from Tk CLASS", markersize=3, fillstyle="none")
    plt.loglog(k, Pk_CLASS, color='k', label="Pk from CLASS")
    plt.xlabel("k [h/Mpc]")
    plt.ylabel("P(k) [(Mpc/h)^3]")
    plt.title("Matter Power Spectrum normalized to sigma8")
    plt.grid(True, which="both")
    plt.legend(loc="lower left")
    plt.savefig(output_file, dpi=150, bbox_inches='tight')
    print(f"Saved plot to {output_file}")

if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("--file-class-tk", type=str, required=True)
    parser.add_argument("--file-class-pk", type=str, required=True)
    parser.add_argument("--n_s", type=float, required=True)
    parser.add_argument("--sigma8", type=float, required=True)
    parser.add_argument("--h", type=float, required=True)
    parser.add_argument("--output", type=str, default="Pk_from_Tk.png")
    args = parser.parse_args()

    k, Pk_from_Tk = compute_pk(args.file_class_tk, args.n_s, args.sigma8, args.h)
    data = np.loadtxt(args.file_class_pk, skiprows=4)
    Pk_CLASS = data[:,1]
    # 1. Define the directory name
    output_dir = "Images"

    # 2. Create the directory if it doesn't already exist
    if not os.path.exists(output_dir):
        os.makedirs(output_dir)

    # 3. Join the directory path with your filename
    # This ensures it works on both Windows and Mac/Linux
    file_path = os.path.join(output_dir, args.output)
    plot_pk(k, Pk_from_Tk, Pk_CLASS, file_path)
    print(f"Saved combined plot to {file_path}")

