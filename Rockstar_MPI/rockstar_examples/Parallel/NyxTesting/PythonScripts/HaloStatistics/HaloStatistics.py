#!/usr/bin/env python3

import argparse
import glob
import os
import re
import sys

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
# Check whether file is a Rockstar ASCII halo file
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
# Read all Rockstar files for one snapshot
# =============================================================

def read_halo_snapshot(halos_dir, snapshot_id, redshift):

    pattern = os.path.join(
        halos_dir,
        f"halos_{snapshot_id}.*.ascii"
    )

    files = sorted(glob.glob(pattern))

    if not files:
        raise RuntimeError(
            f"No Rockstar ASCII files found for snapshot {snapshot_id}.\n"
            f"Directory: {halos_dir}\n"
            f"Expected files matching: {pattern}"
        )

    print()
    print("=" * 70)
    print(f"Reading halos for snapshot = {snapshot_id}, z = {redshift}")
    print(f"Directory: {halos_dir}")
    print(f"Pattern: {pattern}")
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

            print(f"Skipping malformed file: {basename}")
            print(f"  Error: {e}")
            continue

        print(
            f"Reading: {basename} "
            f"({len(halos)} halos)"
        )

        all_halos.extend(halos)

    if not all_halos:

        raise RuntimeError(
            f"No Rockstar halos found for snapshot {snapshot_id} "
            f"in {halos_dir}"
        )

    masses = np.array(
        [halo[3] for halo in all_halos],
        dtype=np.float64
    )

    print()
    print(
        f"snapshot = {snapshot_id}, z = {redshift}: "
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
# Find numbered input sets
#
# We allow:
#
#   --halos-dir-1
#   --halos-dir-2
#   ...
#
# and similarly for snapshot IDs and redshifts.
#
# The arguments are dynamically registered based on what
# appears on the command line, so there is no arbitrary
# 50-snapshot limit.
# =============================================================

def add_numbered_arguments(parser, argv):

    patterns = {
        "halos-dir": r"^--halos-dir-(\d+)(?:=.*)?$",
        "snapshot-id": r"^--snapshot-id-(\d+)(?:=.*)?$",
        "redshift": r"^--redshift-(\d+)(?:=.*)?$",
    }

    numbers = {
        "halos-dir": set(),
        "snapshot-id": set(),
        "redshift": set(),
    }

    for arg in argv:

        for prefix, pattern in patterns.items():

            match = re.match(pattern, arg)

            if match:
                numbers[prefix].add(
                    int(match.group(1))
                )

    # Register every numbered argument that appeared.
    for number in sorted(
        set().union(*numbers.values())
    ):

        parser.add_argument(
            f"--halos-dir-{number}",
            dest=f"halos_dir_{number}",
            default=None,
            help=f"Rockstar halo directory #{number}"
        )

        parser.add_argument(
            f"--snapshot-id-{number}",
            dest=f"snapshot_id_{number}",
            default=None,
            help=f"Snapshot ID #{number}"
        )

        parser.add_argument(
            f"--redshift-{number}",
            dest=f"redshift_{number}",
            type=float,
            default=None,
            help=f"Redshift #{number}"
        )


# =============================================================
# Get numbered halo-dir / snapshot-id / redshift sets
# =============================================================

def get_snapshots(args):

    halos_dirs = {}
    snapshot_ids = {}
    redshifts = {}

    # ---------------------------------------------------------
    # Collect all numbered arguments
    # ---------------------------------------------------------

    for key, value in vars(args).items():

        if value is None:
            continue

        match = re.match(
            r"halos_dir_(\d+)$",
            key
        )

        if match:
            halos_dirs[int(match.group(1))] = value
            continue

        match = re.match(
            r"snapshot_id_(\d+)$",
            key
        )

        if match:
            snapshot_ids[int(match.group(1))] = value
            continue

        match = re.match(
            r"redshift_(\d+)$",
            key
        )

        if match:
            redshifts[int(match.group(1))] = value
            continue

    # ---------------------------------------------------------
    # Make sure at least one set was supplied
    # ---------------------------------------------------------

    if not halos_dirs and not snapshot_ids and not redshifts:

        raise RuntimeError(
            "No --halos-dir-N / --snapshot-id-N / "
            "--redshift-N inputs were supplied."
        )

    # ---------------------------------------------------------
    # Check that all three have the same number of values
    # ---------------------------------------------------------

    n_dirs = len(halos_dirs)
    n_snapshots = len(snapshot_ids)
    n_redshifts = len(redshifts)

    if not (
        n_dirs == n_snapshots == n_redshifts
    ):

        raise RuntimeError(
            "\nMismatch between input counts:\n"
            f"  Number of halo directories = {n_dirs}\n"
            f"  Number of snapshot IDs     = {n_snapshots}\n"
            f"  Number of redshifts        = {n_redshifts}\n"
            "\n"
            "Every halo directory must have a corresponding "
            "snapshot ID and redshift."
        )

    # ---------------------------------------------------------
    # Check that the indices match
    # ---------------------------------------------------------

    dir_numbers = set(halos_dirs.keys())
    snapshot_numbers = set(snapshot_ids.keys())
    redshift_numbers = set(redshifts.keys())

    all_numbers = (
        dir_numbers
        | snapshot_numbers
        | redshift_numbers
    )

    for number in sorted(all_numbers):

        missing = []

        if number not in dir_numbers:
            missing.append(
                f"--halos-dir-{number}"
            )

        if number not in snapshot_numbers:
            missing.append(
                f"--snapshot-id-{number}"
            )

        if number not in redshift_numbers:
            missing.append(
                f"--redshift-{number}"
            )

        if missing:

            raise RuntimeError(
                f"\nIncomplete input set #{number}.\n"
                f"Missing: {', '.join(missing)}"
            )

    # ---------------------------------------------------------
    # Require numbering to start at 1 and be contiguous
    # ---------------------------------------------------------

    numbers = sorted(all_numbers)

    expected_numbers = list(
        range(1, len(numbers) + 1)
    )

    if numbers != expected_numbers:

        raise RuntimeError(
            "\nInput numbering must be contiguous "
            "starting from 1.\n"
            f"Specified: {numbers}\n"
            f"Expected:  {expected_numbers}"
        )

    # ---------------------------------------------------------
    # Construct snapshot list
    # ---------------------------------------------------------

    snapshots = []

    for number in numbers:

        snapshots.append(
            (
                number,
                halos_dirs[number],
                snapshot_ids[number],
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
# Plot all HMFs on one figure
# =============================================================

def plot_hmf(
    hmf_data,
    volume,
    bins
):

    plt.figure(
        figsize=(9, 7)
    )

    # ---------------------------------------------------------
    # Evaluate Sheth-Tormen over common mass range
    # ---------------------------------------------------------

    m_eval = np.logspace(
        bins[0],
        bins[-1],
        400
    )

    # ---------------------------------------------------------
    # Plot each Rockstar HMF and corresponding
    # Sheth-Tormen prediction
    # ---------------------------------------------------------

    for (
        number,
        snapshot_id,
        redshift,
        logM,
        hmf
    ) in hmf_data:

        # Rockstar
        plt.loglog(
            10.0 ** logM,
            hmf,
            marker="o",
            markersize=4,
            linewidth=1.5,
            label=f"Rockstar, z = {redshift:g}"
        )

        # Sheth-Tormen
        st = sheth_tormen_hmf(
            m_eval,
            redshift
        )

        plt.loglog(
            m_eval,
            st,
            linestyle="--",
            linewidth=2,
            label=f"Sheth-Tormen, z = {redshift:g}"
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
        "Halo Mass Function"
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
    # One output image
    # ---------------------------------------------------------

    output_filename = "halo_mass_functions.png"

    plt.savefig(
        output_filename,
        dpi=200
    )

    plt.close()

    print()
    print("=" * 70)
    print(f"Saved HMF plot: {output_filename}")
    print("=" * 70)


# =============================================================
# Main
# =============================================================

def main():

    parser = argparse.ArgumentParser(
        description=(
            "Read Rockstar halo catalogs for an arbitrary "
            "number of snapshots and plot all halo mass "
            "functions on a single figure."
        )
    )

    # ---------------------------------------------------------
    # General arguments
    # ---------------------------------------------------------

    parser.add_argument(
        "--box-size",
        type=float,
        required=True,
        help="Box size in Mpc/h (default: 20)"
    )

    # ---------------------------------------------------------
    # Dynamically add numbered arguments based on argv
    # ---------------------------------------------------------

    add_numbered_arguments(
        parser,
        sys.argv[1:]
    )

    args = parser.parse_args()

    # ---------------------------------------------------------
    # Get snapshots
    # ---------------------------------------------------------

    snapshots = get_snapshots(args)

    print()
    print(
        f"Found {len(snapshots)} snapshot(s)."
    )

    # ---------------------------------------------------------
    # Volume
    # ---------------------------------------------------------

    volume = args.box_size ** 3

    # ---------------------------------------------------------
    # Same mass bins for every redshift
    # ---------------------------------------------------------

    bins = np.linspace(
        7.0,
        13.5,
        35
    )

    # ---------------------------------------------------------
    # Read all snapshots
    # ---------------------------------------------------------

    hmf_data = []

    for (
        number,
        halos_dir,
        snapshot_id,
        redshift
    ) in snapshots:

        halos, masses = read_halo_snapshot(
            halos_dir,
            snapshot_id,
            redshift
        )

        # -----------------------------------------------------
        # Compute HMF
        # -----------------------------------------------------

        logM, hmf, counts = compute_hmf(
            masses,
            volume,
            bins
        )

        hmf_data.append(
            (
                number,
                snapshot_id,
                redshift,
                logM,
                hmf
            )
        )

    # ---------------------------------------------------------
    # Plot everything in one figure
    # ---------------------------------------------------------

    plot_hmf(
        hmf_data,
        volume,
        bins
    )


if __name__ == "__main__":
    main()

