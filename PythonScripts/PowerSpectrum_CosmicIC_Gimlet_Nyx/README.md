# Compare the power spectrum output by CosmicIC, Gimlet and Nyx

Pre-requisite: The [CosmicIC](https://github.com/cosmo-suite/modcon-cosmology/tree/main/PythonScripts/PowerSpectrum_CosmicIC) and [Gimlet](https://github.com/cosmo-suite/Nyx-Workflows/tree/main/Gimlet/PowerSpectra) have to be run.

This script plots and compares power spectra across CosmicIC, Gimlet, and Nyx spectra outputs.

1. Run the script

`sh run_PlotkvsPk_CLASS_Gimlet_Nyx_LCDM.sh`

## Inputs

- `--pk-nyx-file` : Path to Nyx power spectrum file (text file containing k and P(k))
- `--pk-gimlet-file` : Path to Gimlet power spectrum file
- `--pk-cosmicic-file` : Path to CosmicIC power spectrum file
- `--labels` : Legend labels for each dataset (Nyx, Gimlet, CosmicIC, etc.)
- `--dofz` : Scale factor or redshift parameter used for comparison
- `--h` : Dimensionless Hubble parameter
- `--output` : Output filename for the comparison plot (e.g., `.png`) 

