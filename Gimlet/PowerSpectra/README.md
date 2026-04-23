To use the Gimlet code to compute the power spectrum, first the plotfile has to be converted to hdf5 format using the Nyx ultility.

# Nyx Plotfile to HDF5 Conversion

1. Go to the Plotfile2HDF5 converter: [Plotfile2HDF5_grids](https://github.com/AMReX-Astro/Nyx/tree/development/Util/Converters/Plotfile2HDF5_grids)

2. `make -j8`

## Usage
Run the converter
```
./convert3d.gnu.x86-milan.PROF.MPI.ex input_path=<nyx-plotfile> output_path=<hdf5-filename>.h5
```

## Description

- `input_path` : Path to the Nyx plotfile directory  
- `output_path` : Output HDF5 file name  

This tool reads Nyx plotfile data and writes it into a single HDF5 file for easier post-processing and analysis.

# Compute power spectrum

1. Go to the Gimlet code for power spectrum - `gimlet2/apps/matter_pk`.
2. `make -j8`
3. `mpirun -n <nranks> matter_pk.ex <hdf5-file> <spectrum.txt>` 
