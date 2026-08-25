# Compute Lyman alpha P1D spectrum using the Gimlet code

Pre-requisite: The Gimlet code has to be compiled. See [Compilation](https://github.com/cosmo-suite/Nyx-Workflows/tree/main/Gimlet/Compilation).

To use the Gimlet code to compute the power spectrum, first the Nyx plotfile has to be converted to hdf5 format using the Nyx ultility.

# 1. Nyx Plotfile to HDF5 Conversion

1. Go to the Nyx Plotfile2HDF5 converter: [Plotfile2HDF5_grids](https://github.com/AMReX-Astro/Nyx/tree/development/Util/Converters/Plotfile2HDF5_grids)
2. `make -j8`
3. Run the converter
```
./convert3d.gnu.x86-milan.PROF.MPI.ex input_path=<nyx-plotfile> output_path=<hdf5-filename>.h5
```
# 2. Compute the Lyman alpha P1D spectrum

1. Go to the Gimlet code for Lyman alpha P1D spectrum - `gimlet2/apps/p1d_all_axes`.
2. `make -j8`
3. `srun -n <num_ranks> p1d_all_axes.ex <.h5-file> <output_spectrum.txt>`
 
This will write the spectrum as a two column file with $k$ (s/km) and $kP(k)/\pi$. Note the factor of $\pi$ in the denominator. 

# 3. Comparison with DESI data

1. Get the DESI data
```
wget https://zenodo.org/records/17100543/files/zenodo_p1d_fft_y1.zip
```
The DESI data contains a `.fits` file with the data.
2. Plot the computed spectrum and DESI data.
```
Plot_P1D_comparison_DESI.py --desi-data=<path-to-DESI-fits-file> --nyx-data=<path-to-txt-file-from-Step2> --z-target=<redshift-value> --output=<plotfilename.png>
```
The redshift value of the file that we are comparing should be specified using `--z-target`.
