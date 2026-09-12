# Halo Statistics

This directory contains tools for analyzing Rockstar halo catalogs. The primary script, `HaloStatistics.py`, processes these catalogs to calculate the Halo Mass Function (HMF) and generate visualization files.

## Overview

The `HaloStatistics.py` script reads Rockstar ASCII halo files, aggregates halo data (positions and virial masses), and performs the following:
- **HMF Calculation**: Computes the empirical halo mass function from the catalog.
- **Theoretical Comparison**: Plots the resulting HMF against the Sheth-Tormen theoretical prediction.
- **Visualization Export**: Generates VTK files containing the centers and masses of the halos for use in visualization software like ParaView.

## Usage

### Running with the Python script
You can run the script directly using `python3`:

```bash
python3 HaloStatistics.py --halos-dir <path_to_halos> --snapshot-id-1 <id> --redshift-1 <z> [--box-size <size>]
```

#### Inputs
- `--halos-dir` (Required): Path to the directory containing the Rockstar ASCII halo files (matching the pattern `halos_<snapshot-id>.*.ascii`).
- `--snapshot-id-N` and `--redshift-N` (Pairs): The snapshot ID and its corresponding redshift. You can specify up to 50 snapshot pairs (e.g., `--snapshot-id-1`, `--redshift-1`, `--snapshot-id-2`, `--redshift-2`, etc.).
- `--box-size` (Optional): The simulation box size in Mpc/h. Defaults to `20.0`.

#### Outputs
For each snapshot provided, the script produces:
- `halos_<snapshot_id>_z_eq_<z>.vtk`: A VTK file containing the 3D positions (x, y, z) and the virial mass (`mvir`) of each halo.
- `hmf_z_eq_<z>.png`: A log-log plot of the Halo Mass Function, comparing the Rockstar results with the Sheth-Tormen model.

### Using the shell script
A wrapper script `run_HaloStatistics.sh` is provided for convenience. It contains a pre-configured command to run the analysis on a specific dataset.

To use it:
1. Open `run_HaloStatistics.sh` and edit the `--halos-dir`, `--snapshot-id`, and `--redshift` arguments to match your data.
2. Run the script from the terminal:
   ```bash
   bash run_HaloStatistics.sh
   ```
