# Running Nyx simulations 

This repository contains the details of how to run simulations using [Nyx](https://github.com/AMReX-Astro/Nyx) - DOE's flagship code for 
cosmology. [Nyx](https://github.com/AMReX-Astro/Nyx) is an open-source code based on [AMReX](https://github.com/AMReX-Codes/amrex). 
The different test cases are in the [Exec](https://github.com/AMReX-Astro/Nyx/tree/development/Exec) directory. This repository 
contains some instructions of how to run these test cases. Here is what the individual directories in this repository contain

## Step 1 
Generating initial conditions for the dark matter particles - required by all simulations.  
See the README in [CosmicIC](https://github.com/cosmo-suite/NyxCompilationAndRunning/tree/main/CosmicIC). 

## Step 2
Run the Nyx simulation.   
See the README in [LyA](https://github.com/cosmo-suite/NyxCompilationAndRunning/tree/main/LyA) for how to run [Nyx](https://github.com/AMReX-Astro/Nyx) simulation with dark matter particles with baryonic matter simulations for Lyman alpha test case.

## For runs with halos and lightcone writing
[HaloFinder](https://github.com/cosmo-suite/NyxCompilationAndRunning/tree/main/HaloFinder) - Dark matter particles simulations for writing lightcones and halos for dark matter simulations

## Post-processing Nyx data
[Gimlet](https://github.com/cosmo-suite/NyxCompilationAndRunning/tree/main/Gimlet) - post processing Nyx data

These are not end-to-end details. But inputs, makefiles and scripts on Perlmutter and Frontier.
