# Rockstar halo finder - Background with 2 halos in parallel mode

This is the same test case in the `Serial/BackgndWithTwoHalo` directory, but run in parallel. This has a uniform background distribution with two higher density, uniform spherical distribution of particles. Rockstar will identify two halos containing the two higher density, uniform spherical particle distribution.  

Rockstar does not use MPI. It has a server-client setup for doing parallel halo finding. Each rank can write its own block of particle data into gadget files, and rockstar can read them in. In this example, the different blocks of particles are written out for the domain to simulate what a parallel MPI run from a code would produce with each rank writing its own set of particles.

# How to run the case on an interactive node on Perlmutter
This section describes how to run rockstar in client-server on Perlmutter (NERSC). The procedure should be similar for other clusters.

1. `cd BackgndWithTwoHalo_Parallel`
2. `mkdir halos`
3. Get an interactive node on a terminal (say terminal 1)
4. Get another terminal (terminal 2) of the interactive node by doing 
`ssh <nodeid>`
5. On terminal 1
`./rockstar-galaxies -c rockstar.cfg`
This will write `cfg` files in the `halos` directory.
6. On terminal 2
`srun -n <nranks> ./rockstar-galaxies -c halos/auto-rockstar.cfg`.
It should be made sure that the `<nranks> = NUM_READERS + NUM_WRITERS` where the `NUM_READERS` and `NUM_WRITERS` are defined in `rockstar.cfg`.
7. This will write the halo output into `.ascii` files and the particles into the `.bin` files
