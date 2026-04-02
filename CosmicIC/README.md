# How to generate initial conditions for Nyx using the CosmicIC repo
The CosmicIC repository generates the initial dark matter particle positions consistent 
with the overdensity spectrum at a specified redshift. The transfer function used to 
generate the spectrum is in a file `cmb.tf` in the repo. The input file `input.par` 
specifies all the required parameters for generating the initial particle locations. 
It runs in parallel using MPI. Access is needed. Not open-source. The steps are as below.


1. First, we need to get the transfer function by running the `class` code. See the README in
[ForNyx](https://github.com/nataraj2/class_public/tree/master/ForNyx).

Dependenices: fftw library
 
2. `git clone https://maheshnatarajan@bitbucket.org/zarija/cosmicic.git` (access needed)  
3. `cd cosmicic`
4. `make -j8`
5. `mpirun -n <nranks> ./init input.par cmb.tf IC_File`

`input.par` - the inputs file. `np` is the number of cells in each direction. Note that 
the dimension in each direction specified by `box_size` is in the units of Mpc/h.  
`IC_File` is the prefix to the filenames generated. Each rank writes the data into a 
separate file. These files can be read in by Nyx.  

Note: The initial binary files for particles can be created using other codebases such as 
MUSIC and CLASS as well.

# How to run on Frontier
The folder `Frontier` has the modules to be loaded and the batch script for running the 
CosmicIC code.
