# Power spectrum from CLASS

Pre-requisite - Run the CLASS code to generate the transfer function and power spectrum files. See [How to run CLASS](https://github.com/nataraj2/class_public/tree/master/ForNyx).

This script computes the power spectrum from the transfer function data (`tk.dat` file) in CLASS and then compares with the power spectrum (`pk.dat`) output by CLASS. This requires the CLASS code to be run to generate the required files.

1. Run the the script
`sh run_Plot_Pk_from_Tk_CLASS.sh`.

## Inputs

- `--file-class-tk` : Path to the CLASS transfer function file (`tk.dat`)
- `--file-class-pk` : Path to the CLASS linear power spectrum file (`pk.dat`)
- `--n_s` : Scalar spectral index of primordial fluctuations
- `--sigma8` : Normalization of the matter power spectrum at 8 Mpc/h
- `--h` : Dimensionless Hubble parameter
