#!/usr/bin/env python3

import argparse
import glob
import os


def read_halo_file(filename):
    """Read halo x, y, z coordinates and mvir from a Rockstar ASCII file."""

    halos = []

    with open(filename, "r", encoding="ascii") as f:

        x_idx = y_idx = z_idx = mvir_idx = None

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

            # Extract halo properties
            x = float(values[x_idx])
            y = float(values[y_idx])
            z = float(values[z_idx])
            mvir = float(values[mvir_idx])

            halos.append((x, y, z, mvir))

    return halos


def is_rockstar_ascii_file(filename):
    """
    Check whether a file is an ASCII Rockstar halo catalog.
    """

    try:

        with open(filename, "r", encoding="ascii") as f:

            for line in f:

                if line.startswith("#id "):
                    return True

                # Header should occur near the beginning
                if f.tell() > 10000:
                    break

    except (UnicodeDecodeError, OSError):
        return False

    return False


def write_vtk(filename, halos):
    """Write halo centers and masses to a legacy ASCII VTK file."""

    n = len(halos)

    with open(filename, "w") as f:

        f.write("# vtk DataFile Version 3.0\n")
        f.write("Rockstar Halo Centers\n")
        f.write("ASCII\n")
        f.write("DATASET POLYDATA\n")

        # ---------------------------------------------------------
        # Points
        # ---------------------------------------------------------

        f.write(f"POINTS {n} float\n")

        for x, y, z, mvir in halos:
            f.write(f"{x:.8e} {y:.8e} {z:.8e}\n")

        # ---------------------------------------------------------
        # Vertices
        # ---------------------------------------------------------

        f.write(f"VERTICES {n} {2 * n}\n")

        for i in range(n):
            f.write(f"1 {i}\n")

        # ---------------------------------------------------------
        # Halo mass
        # ---------------------------------------------------------

        f.write(f"\nPOINT_DATA {n}\n")

        f.write("SCALARS mvir float 1\n")
        f.write("LOOKUP_TABLE default\n")

        for x, y, z, mvir in halos:
            f.write(f"{mvir:.8e}\n")


def main():

    parser = argparse.ArgumentParser(
        description="Convert Rockstar ASCII halo catalogs to VTK."
    )

    parser.add_argument(
        "--halos-dir",
        required=True,
        help="Directory containing Rockstar halo files",
    )

    parser.add_argument(
        "--output",
        default="halos.vtk",
        help="Output VTK filename (default: halos.vtk)",
    )

    args = parser.parse_args()

    # -------------------------------------------------------------
    # Find all files
    # -------------------------------------------------------------

    files = sorted(
        f
        for f in glob.glob(os.path.join(args.halos_dir, "*"))
        if os.path.isfile(f)
    )

    if not files:
        raise RuntimeError(
            f"No files found in directory: {args.halos_dir}"
        )

    print(f"Found {len(files)} files")

    # -------------------------------------------------------------
    # Read Rockstar ASCII files
    # -------------------------------------------------------------

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

    # -------------------------------------------------------------
    # Summary
    # -------------------------------------------------------------

    print()
    print(f"Total number of halos: {len(all_halos)}")

    if not all_halos:
        raise RuntimeError(
            "No Rockstar halos were found."
        )

    for x, y, z, mvir in all_halos:
        print(f"Halo mass = {mvir:.6e} Msun/h")

    # -------------------------------------------------------------
    # Write VTK
    # -------------------------------------------------------------

    write_vtk(args.output, all_halos)

    print(f"Wrote: {args.output}")


if __name__ == "__main__":
    main()
