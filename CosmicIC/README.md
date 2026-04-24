# How to generate initial conditions for Nyx using the CosmicIC repo
Dependenices: fftw library

Pre-requisite: The CLASS code (open-source) to generate the transfer function has to be run prior to 
running the CosmicIC code (access needed).

The CosmicIC repository generates the initial dark matter particle positions consistent 
with the overdensity spectrum at a specified redshift. The transfer function used to 
generate the spectrum is obtained by running the CLASS code. The input file `input.par` 
specifies all the required parameters for generating the initial particle locations. 
It runs in parallel using MPI. The steps are as below.


1. Run the CLASS code to get the transfer function for generating the power spectrum. See 
the README in [How to run CLASS](https://github.com/nataraj2/class_public/tree/master/ForNyx). 
2. `git clone https://maheshnatarajan@bitbucket.org/zarija/cosmicic.git` (access needed)  
3. `cd cosmicic`
4. `make -j8`
5. `mpirun -n <nranks> ./init input.par <*tk.dat-from-CLASS> IC_File`
The `<*tk.dat-from-CLASS>` is the file from the `output` directory in CLASS. In this file, 
make sure to delete the first 8 commented lines.

## Inputs 
- `input.par` - the inputs file. 
- `np` is the number of cells in each direction. Note that the dimension in each direction 
   specified by `box_size` is in the units of Mpc/h.  
- `IC_File` is the prefix to the filenames generated. Each rank writes the data into a 
separate file. These files can be read in by Nyx.  

Note: The initial binary files for particles can be created using other codebases such as 
MUSIC and CLASS as well.

# How to run on Frontier
The folder `Frontier` has the modules to be loaded and the batch script for running the 
CosmicIC code.
