#!/usr/bin/env python3

import argparse
import glob
import os
import re

import numpy as np
import matplotlib.pyplot as plt


# =============================================================
# Read Rockstar ASCII halo file
# =============================================================

def read_halo_file(filename):

    halos = []

    with open(filename, "r", encoding="ascii") as f:

        x_idx = None
        y_idx = None
        z_idx = None
        mvir_idx = None

        for line in f:

            # Rockstar column header
            if line.startswith("#id "):

                columns = line[1:].split()

                x_idx = columns.index("x")
                y_idx = columns.index("y")
                z_idx = columns.index("z")
                mvir_idx = columns.index("mvir")

                continue

            # Skip comments and blank lines
            if line.startswith("#") or not line.strip():
                continue

            values = line.split()

            x = float(values[x_idx])
            y = float(values[y_idx])
            z = float(values[z_idx])
            mvir = float(values[mvir_idx])

            halos.append((x, y, z, mvir))

    return halos


# =============================================================
# Check whether file is a Rockstar ASCII halo catalog
# =============================================================

def is_rockstar_ascii_file(filename):

    try:

        with open(filename, "r", encoding="ascii") as f:

            for line in f:

                if line.startswith("#id "):
                    return True

                if f.tell() > 10000:
                    break

    except (UnicodeDecodeError, OSError):

        return False

    return False


# =============================================================
# Read all halo files in one directory
# =============================================================

def read_halo_directory(directory, redshift):

    files = sorted(
        f
        for f in glob.glob(
            os.path.join(directory, "*")
        )
        if os.path.isfile(f)
    )

    if not files:

        raise RuntimeError(
            f"No files found in directory: {directory}"
        )

    print()
    print("=" * 70)
    print(f"Reading halos at z = {redshift}")
    print(f"Directory: {directory}")
    print("=" * 70)

    all_halos = []

    for filename in files:

        basename = os.path.basename(filename)

        if not is_rockstar_ascii_file(filename):

            print(f"Skipping: {basename}")
            continue

        try:

            halos = read_halo_file(filename)

        except (ValueError, IndexError) as e:

            print(
                f"Skipping malformed file: {basename}"
            )

            print(f"  Error: {e}")

            continue

        print(
            f"Reading: {basename} "
            f"({len(halos)} halos)"
        )

        all_halos.extend(halos)

    if not all_halos:

        raise RuntimeError(
            f"No Rockstar halos found in {directory}"
        )

    masses = np.array(
        [halo[3] for halo in all_halos],
        dtype=np.float64
    )

    print()
    print(
        f"z = {redshift}: "
        f"{len(all_halos)} halos"
    )

    print(
        f"Minimum mass = "
        f"{masses.min():.6e} Msun/h"
    )

    print(
        f"Maximum mass = "
        f"{masses.max():.6e} Msun/h"
    )

    return all_halos, masses


# =============================================================
# Write legacy ASCII VTK
# =============================================================

def write_vtk(filename, halos):

    n = len(halos)

    with open(filename, "w") as f:

        f.write("# vtk DataFile Version 3.0\n")
        f.write("Rockstar Halo Centers\n")
        f.write("ASCII\n")
        f.write("DATASET POLYDATA\n")

        # -----------------------------------------------------
        # Points
        # -----------------------------------------------------

        f.write(f"POINTS {n} float\n")

        for x, y, z, mvir in halos:

            f.write(
                f"{x:.8e} "
                f"{y:.8e} "
                f"{z:.8e}\n"
            )

        # -----------------------------------------------------
        # Vertices
        # -----------------------------------------------------

        f.write(
            f"VERTICES {n} {2 * n}\n"
        )

        for i in range(n):

            f.write(
                f"1 {i}\n"
            )

        # -----------------------------------------------------
        # Halo mass scalar
        # -----------------------------------------------------

        f.write(
            f"\nPOINT_DATA {n}\n"
        )

        f.write(
            "SCALARS mvir float 1\n"
        )

        f.write(
            "LOOKUP_TABLE default\n"
        )

        for x, y, z, mvir in halos:

            f.write(
                f"{mvir:.8e}\n"
            )


# =============================================================
# Compute halo mass function
# =============================================================

def compute_hmf(masses, volume, bins):

    log_mass = np.log10(masses)

    counts, edges = np.histogram(
        log_mass,
        bins=bins
    )

    dlogM = edges[1] - edges[0]

    centers = 0.5 * (
        edges[:-1] + edges[1:]
    )

    hmf = counts / (
        volume * dlogM
    )

    mask = counts > 0

    return (
        centers[mask],
        hmf[mask],
        counts[mask]
    )


# =============================================================
# Growth factor
# =============================================================

def growth_factor(z):

    return 1.0 / (1.0 + z)


# =============================================================
# Toy sigma(M)
# =============================================================

def sigma_M(
    mass,
    z,
    sigma8=0.8,
    M8=1.0e14,
    alpha=0.3
):

    D = growth_factor(z)

    return (
        sigma8
        * D
        * (mass / M8) ** (-alpha)
    )


# =============================================================
# Sheth-Tormen halo mass function
# =============================================================

def sheth_tormen_hmf(mass, z):

    A = 0.3222
    a = 0.707
    p = 0.3

    sig = sigma_M(
        mass,
        z
    )

    dlns_dlnm = -0.3

    nu = np.sqrt(a) / sig

    f_nu = (
        A
        * np.sqrt(2.0 / np.pi)
        * nu
        * np.exp(-0.5 * nu**2)
        * (
            1.0
            + nu ** (-2.0 * p)
        )
    )

    # Cosmology
    Omega_m = 0.31

    # rho_m:
    #
    # (Msun/h) / (Mpc/h)^3
    #

    rho_m = Omega_m * 2.775e11

    dn_dlnM = (
        rho_m / mass
        * f_nu
        * abs(dlns_dlnm)
    )

    # Convert dn/dlnM to dn/dlog10(M)

    dn_dlogM = (
        dn_dlnM
        * np.log(10.0)
    )

    return dn_dlogM


# =============================================================
# Get numbered directory/redshift pairs
# =============================================================

def get_snapshots(args):

    halo_dirs = {}
    redshifts = {}

    # ---------------------------------------------------------
    # Collect halo directories
    # ---------------------------------------------------------

    for key, value in vars(args).items():

        match = re.match(
            r"halos_dir_(\d+)$",
            key
        )

        if match is not None and value is not None:

            number = int(match.group(1))

            halo_dirs[number] = value

    # ---------------------------------------------------------
    # Collect redshifts
    # ---------------------------------------------------------

    for key, value in vars(args).items():

        match = re.match(
            r"redshift_(\d+)$",
            key
        )

        if match is not None and value is not None:

            number = int(match.group(1))

            redshifts[number] = value

    # ---------------------------------------------------------
    # Check at least one pair
    # ---------------------------------------------------------

    if not halo_dirs and not redshifts:

        raise RuntimeError(
            "No --halos-dir-N / --redshift-N pairs were supplied."
        )

    # ---------------------------------------------------------
    # Check number of directories and redshifts
    # ---------------------------------------------------------

    if len(halo_dirs) != len(redshifts):

        raise RuntimeError(
            "\nMismatch between halo directories and redshifts:\n"
            f"  Number of halo directories = {len(halo_dirs)}\n"
            f"  Number of redshifts        = {len(redshifts)}\n"
            "\n"
            "Every --halos-dir-N must have a corresponding "
            "--redshift-N."
        )

    # ---------------------------------------------------------
    # Check matching indices
    # ---------------------------------------------------------

    halo_numbers = set(halo_dirs.keys())
    redshift_numbers = set(redshifts.keys())

    missing_redshifts = (
        halo_numbers - redshift_numbers
    )

    missing_directories = (
        redshift_numbers - halo_numbers
    )

    if missing_redshifts:

        raise RuntimeError(
            "Missing redshift(s) for: "
            + ", ".join(
                f"--halos-dir-{n}"
                for n in sorted(missing_redshifts)
            )
        )

    if missing_directories:

        raise RuntimeError(
            "Missing halo director(y/ies) for: "
            + ", ".join(
                f"--redshift-{n}"
                for n in sorted(missing_directories)
            )
        )

    # ---------------------------------------------------------
    # Check contiguous numbering
    # ---------------------------------------------------------

    numbers = sorted(halo_numbers)

    expected_numbers = list(
        range(1, len(numbers) + 1)
    )

    if numbers != expected_numbers:

        raise RuntimeError(
            "\nSnapshot numbering must be contiguous "
            "starting from 1.\n"
            f"Specified: {numbers}\n"
            f"Expected:  {expected_numbers}\n"
        )

    # ---------------------------------------------------------
    # Construct snapshot list
    # ---------------------------------------------------------

    snapshots = []

    for number in numbers:

        snapshots.append(
            (
                number,
                halo_dirs[number],
                float(redshifts[number])
            )
        )

    return snapshots


# =============================================================
# Format redshift for filenames
# =============================================================

def redshift_string(redshift):

    if redshift.is_integer():

        return str(int(redshift))

    return str(redshift)


# =============================================================
# Plot HMF for one redshift
# =============================================================

def plot_hmf(
    masses,
    redshift,
    volume,
    bins
):

    logM, hmf, counts = compute_hmf(
        masses,
        volume,
        bins
    )

    # ---------------------------------------------------------
    # Sheth-Tormen
    # ---------------------------------------------------------

    m_eval = np.logspace(
        bins[0],
        bins[-1],
        400
    )

    st = sheth_tormen_hmf(
        m_eval,
        redshift
    )

    # ---------------------------------------------------------
    # Plot
    # ---------------------------------------------------------

    plt.figure(
        figsize=(9, 7)
    )

    plt.loglog(
        10.0 ** logM,
        hmf,
        marker="o",
        markersize=4,
        linewidth=1.5,
        label="Rockstar"
    )

    plt.loglog(
        m_eval,
        st,
        linestyle="--",
        linewidth=2,
        label="Sheth-Tormen"
    )

    # ---------------------------------------------------------
    # Formatting
    # ---------------------------------------------------------

    plt.xlabel(
        r"Halo mass [$M_\odot/h$]"
    )

    plt.ylabel(
        r"$dn/d\log_{10}M$ "
        r"[$(h^{-1}{\rm Mpc})^{-3}$]"
    )

    plt.title(
        f"Halo Mass Function — z = {redshift:g}"
    )

    plt.xlim(
        10.0 ** bins[0],
        10.0 ** bins[-1]
    )

    plt.ylim(
        1e-4,
        None
    )

    plt.grid(
        True,
        which="both",
        linestyle="--",
        alpha=0.5
    )

    plt.legend()

    plt.tight_layout()

    # ---------------------------------------------------------
    # Output filename
    # ---------------------------------------------------------

    z_string = redshift_string(
        redshift
    )

    output_filename = (
        f"hmf_z_eq_{z_string}.png"
    )

    plt.savefig(
        output_filename,
        dpi=200
    )

    plt.close()

    print(
        f"Saved HMF plot: {output_filename}"
    )


# =============================================================
# Main
# =============================================================

def main():

    parser = argparse.ArgumentParser(
        description=(
            "Read Rockstar halo catalogs from an arbitrary "
            "number of directories, write VTK files, and "
            "generate one HMF plot per redshift."
        )
    )

    # ---------------------------------------------------------
    # Allow up to 50 snapshots
    # ---------------------------------------------------------

    for i in range(1, 51):

        parser.add_argument(
            f"--halos-dir-{i}",
            dest=f"halos_dir_{i}",
            default=None,
            help=f"Rockstar halo directory #{i}"
        )

        parser.add_argument(
            f"--redshift-{i}",
            dest=f"redshift_{i}",
            type=float,
            default=None,
            help=f"Redshift corresponding to directory #{i}"
        )

    parser.add_argument(
        "--box-size",
        type=float,
        default=20.0,
        help="Box size in Mpc/h (default: 20)"
    )

    args = parser.parse_args()

    # =========================================================
    # Get snapshots
    # =========================================================

    snapshots = get_snapshots(args)

    print()
    print(
        f"Found {len(snapshots)} halo directories."
    )

    # =========================================================
    # Volume
    # =========================================================

    volume = args.box_size ** 3

    # =========================================================
    # Same mass bins for every redshift
    # =========================================================

    bins = np.linspace(
        7.0,
        13.5,
        35
    )

    # =========================================================
    # Process snapshots
    # =========================================================

    for number, directory, redshift in snapshots:

        # -----------------------------------------------------
        # Read halos
        # -----------------------------------------------------

        halos, masses = read_halo_directory(
            directory,
            redshift
        )

        # -----------------------------------------------------
        # Write VTK
        # -----------------------------------------------------

        z_string = redshift_string(
            redshift
        )

        vtk_filename = (
            f"halos_z_eq_{z_string}.vtk"
        )

        write_vtk(
            vtk_filename,
            halos
        )

        print(
            f"Wrote: {vtk_filename}"
        )

        # -----------------------------------------------------
        # Plot HMF
        # -----------------------------------------------------

        plot_hmf(
            masses,
            redshift,
            volume,
            bins
        )


if __name__ == "__main__":
    main()

