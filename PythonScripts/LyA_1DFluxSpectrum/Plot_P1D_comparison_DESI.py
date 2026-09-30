import argparse
import re
import numpy as np
import matplotlib.pyplot as plt
from astropy.io import fits


def main():
    parser = argparse.ArgumentParser(
        description=(
            "Plot DESI DR1 Ly-alpha 1D Flux Power Spectrum "
            "against multiple Nyx datasets."
        )
    )

    parser.add_argument(
        "--desi-data",
        type=str,
        required=True,
        help="Path to the DESI baseline .fits file",
    )

    parser.add_argument(
        "--z-target",
        type=float,
        required=True,
        help="Target redshift to filter",
    )

    parser.add_argument(
        "--output",
        type=str,
        required=True,
        help="Output image filename (e.g., plot.png)",
    )

    # Parse fixed arguments first.
    args, unknown = parser.parse_known_args()

    # Dictionaries indexed by the number in --nyx-data-N
    # and --legend-N.
    nyx_data = {}
    legends = {}

    i = 0

    while i < len(unknown):
        arg = unknown[i]

        # --------------------------------------------------------
        # --nyx-data-N=<filename>
        # --------------------------------------------------------
        match = re.fullmatch(r"--nyx-data-(\d+)=(.*)", arg)

        if match:
            num = int(match.group(1))
            nyx_data[num] = match.group(2)
            i += 1
            continue

        # --------------------------------------------------------
        # --legend-N=<legend>
        # --------------------------------------------------------
        match = re.fullmatch(r"--legend-(\d+)=(.*)", arg)

        if match:
            num = int(match.group(1))
            legends[num] = match.group(2)
            i += 1
            continue

        # --------------------------------------------------------
        # --nyx-data-N <filename>
        # --------------------------------------------------------
        match = re.fullmatch(r"--nyx-data-(\d+)", arg)

        if match:
            num = int(match.group(1))

            if i + 1 >= len(unknown):
                parser.error(f"Missing value for {arg}")

            nyx_data[num] = unknown[i + 1]
            i += 2
            continue

        # --------------------------------------------------------
        # --legend-N <legend>
        # --------------------------------------------------------
        match = re.fullmatch(r"--legend-N", arg)

        if match:
            parser.error(
                "Invalid legend argument. Use --legend-1, "
                "--legend-2, etc."
            )

        match = re.fullmatch(r"--legend-(\d+)", arg)

        if match:
            num = int(match.group(1))

            if i + 1 >= len(unknown):
                parser.error(f"Missing value for {arg}")

            legends[num] = unknown[i + 1]
            i += 2
            continue

        parser.error(f"Unrecognized argument: {arg}")

    # ------------------------------------------------------------
    # Check that Nyx data and legend entries match
    # ------------------------------------------------------------

    nyx_numbers = set(nyx_data.keys())
    legend_numbers = set(legends.keys())

    if nyx_numbers != legend_numbers:

        missing_legends = sorted(nyx_numbers - legend_numbers)
        missing_data = sorted(legend_numbers - nyx_numbers)

        message = "Nyx data and legend entries do not match."

        if missing_legends:
            message += (
                "\nMissing legend entries for: "
                + ", ".join(
                    f"--legend-{n}" for n in missing_legends
                )
            )

        if missing_data:
            message += (
                "\nMissing Nyx data entries for: "
                + ", ".join(
                    f"--nyx-data-{n}" for n in missing_data
                )
            )

        parser.error(message)

    if not nyx_data:
        parser.error(
            "At least one --nyx-data-N and corresponding "
            "--legend-N is required."
        )

    # ------------------------------------------------------------
    # Read DESI data
    # ------------------------------------------------------------

    with fits.open(args.desi_data) as hdul:
        data = hdul["P1D_BLIND"].data

    z_target = args.z_target
    tol = 1e-6

    mask = np.isclose(
        data["Z"],
        z_target,
        atol=tol,
    )

    k = data["K"][mask]
    P1D = data["PLYA"][mask]
    err = data["E_PK"][mask]

    if len(k) == 0:
        parser.error(
            f"No DESI data found for z = {z_target}"
        )

    # Keep first 70 DESI points.
    k = k[:70]
    P1D = P1D[:70]
    err = err[:70]

    # ------------------------------------------------------------
    # Calculate k P_1D for DESI
    # ------------------------------------------------------------

    kP1D = k * P1D

    # Propagated uncertainty:
    # sigma(k P) = k * sigma(P)
    kP1D_err = k * err

    # ------------------------------------------------------------
    # Create plot
    # ------------------------------------------------------------

    plt.figure(figsize=(8, 6))

    # ------------------------------------------------------------
    # Plot DESI
    #
    # DESI is plotted exactly once.
    # ------------------------------------------------------------

    plt.errorbar(
        k,
        kP1D,
        yerr=kP1D_err,
        fmt="o",
        markersize=4,
        capsize=2,
        label=rf"DESI DR1, $z={z_target}$",
    )

    # ------------------------------------------------------------
    # Plot all Nyx datasets
    #
    # Only k < 0.04 s/km is plotted.
    # ------------------------------------------------------------

    for num in sorted(nyx_data.keys()):

        filename = nyx_data[num]
        legend = legends[num]

        data_nyx = np.loadtxt(filename)

        k_nyx = data_nyx[:, 0]

        # Nyx P1D multiplied by pi.
        k_nyx_P1D = data_nyx[:, 1] * 3.1416

        # Restrict Nyx data to k < 0.04 s/km.
        nyx_mask = k_nyx < 0.04

        plt.plot(
            k_nyx[nyx_mask],
            k_nyx_P1D[nyx_mask],
            label=legend,
        )

    # ------------------------------------------------------------
    # Plot formatting
    # ------------------------------------------------------------

    plt.xscale("log")
    plt.yscale("log")


    plt.xlabel(r"$k\ [{\rm s/km}]$")
    plt.ylabel(r"$kP_{\rm 1D}(k)$")

    plt.grid(
        True,
        which="both",
        alpha=0.3,
    )

    plt.legend()

    #plt.xlim([1e-3, 1e-1])
    #plt.ylim([1e-1, 1e0])

    plt.savefig(
        args.output,
        dpi=300,
    )

    plt.show()


if __name__ == "__main__":
    main()
