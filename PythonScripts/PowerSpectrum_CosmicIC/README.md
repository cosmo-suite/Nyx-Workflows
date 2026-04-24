# Computing power spectrum from the CosmicIC generated transfer functions

Pre-requisite: The CosmicIC code for generating the initial conditions has to be run. See [How to run CosmicIC code](https://github.com/cosmo-suite/Nyx-Workflows/tree/main/CosmicIC).

The python script computes the power spectra from the transfer functions output by the CosmicIC code (`transfer_function.txt` file) for cold, warm, and fuzzy dark matter. The warm and fuzzy dark matter transfer functions are obtained by modification of the cold dark matter function by the CosmicIC code (`Cosmology.cpp` file). There are `.sh` scripts for running. 

1. Run the script
`sh run_Plot_Pk_from_Tk_CosmicIC_LCDM.sh`

## Inputs

- `--tk-files` : One or more CosmicIC transfer function files (`.txt`)
- `--labels` : Labels corresponding to each transfer function (for plotting legend)
- `--n_s` : Scalar spectral index
- `--sigma8` : Power spectrum normalization at 8 Mpc/h
- `--h` : Dimensionless Hubble parameter
- `--output` : Output plot filename (e.g., `.png`)
- `--output-pk-files` : Output file(s) to save computed power spectrum data

## Output

- The images are written into `Images` directory.
- The power spectrum files are written into the output files specified by `output-pk-files`. This can be used to compare these against the output power spectrum from Gimlet and Nyx.
