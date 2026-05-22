import numpy as np
import matplotlib.pyplot as plt
import glob
import os
import re
import argparse

def compute_hmf(masses, volume, bins):
    logM = np.log10(masses)

    counts, edges = np.histogram(logM, bins=bins)

    dlogM = edges[1] - edges[0]

    centers = 0.5 * (edges[:-1] + edges[1:])

    # number density per log10 mass
    hmf = counts / (volume * dlogM)

    return centers, hmf


def read_binary_points(filename):
    try:
        with open(filename, 'rb') as f:
            data = np.fromfile(f, dtype='>f4')
    except Exception as e:
        print(f"[WARNING] Could not read {filename}: {e}")
        return None

    if data.size == 0:
        return None

    if data.size % 5 != 0:
        print(f"[WARNING] {filename} size not multiple of 5. Skipping.")
        return None

    return data.reshape((-1, 5))


def extract_redshift(filename):
    base = os.path.basename(filename)
    match = re.search(r'_(\d+)\.bin', base)
    if not match:
        raise ValueError(f"Cannot parse redshift from {filename}")

    return int(match.group(1)) / 10000.0


def compute_counts(files, mass_thresholds, min_ncells):
    zs = []
    counts = {m: [] for m in mass_thresholds}

    for f in files:
        data = read_binary_points(f)

        if data is None or data.shape[0] == 0:
            continue

        z = extract_redshift(f)

        masses = data[:, 3]
        ncells = data[:, 4]

        # APPLY RESOLUTION CUT HERE
        valid = ncells >= min_ncells

        n_valid = np.sum(valid)
        print(f"z = {z:.5f}, total = {len(masses)}, valid = {n_valid}")

        zs.append(z)

        for mth in mass_thresholds:
            counts[mth].append(np.sum((masses > mth) & valid))

    if len(zs) == 0:
        raise RuntimeError("No valid halo files found.")

    # sort by redshift
    order = np.argsort(zs)
    zs = np.array(zs)[order]

    for m in mass_thresholds:
        counts[m] = np.array(counts[m])[order]

    return zs, counts


def main():
    parser = argparse.ArgumentParser(description="Compute halo counts vs redshift")

    parser.add_argument(
        "--halo-dir",
        required=True,
        help="Directory containing reeber_halos_*.bin files"
    )

    parser.add_argument(
        "--min-ncells",
        type=int,
        default=20,
        help="Minimum number of cells required for a valid halo (default: 20)"
    )

    args = parser.parse_args()

    pattern = os.path.join(args.halo_dir, "reeber_halos_*.bin")
    files = sorted(glob.glob(pattern))

    if len(files) == 0:
        raise RuntimeError(f"No halo files found in {args.halo_dir}")

    mass_thresholds = [1e7, 1e8, 1e9, 1e10, 1e11, 1e12]

    zs, counts = compute_counts(files, mass_thresholds, args.min_ncells)

    plt.figure()

    for mth in mass_thresholds:
        plt.plot(
            zs,
            counts[mth],
            marker='o',
            markersize=1,
            label=f"M > {mth:.0e}"
        )

    plt.xlabel("Redshift z")
    plt.ylabel(r"$N(M > M_{\mathrm{thresh}})$")
    plt.title(f"Evolution of Halo Counts (n_cells ≥ {args.min_ncells})")
    plt.legend()
    plt.grid(True)

    plt.gca().invert_xaxis()
    plt.tight_layout()

    plt.savefig("halo_evolution_count.png", dpi=200)
    plt.close()


if __name__ == "__main__":
    main()
