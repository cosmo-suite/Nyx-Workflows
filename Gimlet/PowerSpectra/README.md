# Compute matter power spectrum using the Gimlet code

Pre-requisite: The Gimlet code has to be compiled. See [Compilation](https://github.com/cosmo-suite/Nyx-Workflows/tree/main/Gimlet/Compilation).

To use the Gimlet code to compute the power spectrum, first the Nyx plotfile has to be converted to hdf5 format using the Nyx ultility.

# 1. Nyx Plotfile to HDF5 Conversion

1. Go to the Plotfile2HDF5 converter: [Plotfile2HDF5_grids](https://github.com/AMReX-Astro/Nyx/tree/development/Util/Converters/Plotfile2HDF5_grids)
2. `make -j8`
3. Run the converter
```
./convert3d.gnu.x86-milan.PROF.MPI.ex input_path=<nyx-plotfile> output_path=<hdf5-filename>.h5
```
# 2. Compute Power Spectrum

1. Go to the Gimlet code for power spectrum - `gimlet2/apps/matter_pk`.
2. `make -j8`
3. `mpirun -n <nranks> matter_pk.ex <hdf5-file> <spectrum.txt>`

The spectrum.txt file contains 4 colmuns - the first is the wavenumber in units `h/Mpc` and the fourth is the power spectra in $(Mpc/h)^3$.
