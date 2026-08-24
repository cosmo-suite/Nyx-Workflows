# mpi-rockstar for halo identification and mergers for Nyx simulations

To use this code, first the Nyx simulation has to be run to obtain the particle snapshots as a collection of Gadget files for various redshifts. These Gadget files are used as the inputs for the Rockstar halo finding code.

Note: The mpi-rockstar code works only with OpenMPI and not MPICH.

## 1. Computing halos for all snapshots at different redshifts

1. Clone the Rockstar repository and compile

```
1. git clone https://github.com/Tomoaki-Ishiyama/mpi-rockstar.git
2. cd mpi-rockstar/src
3. make -j8 mpi-rockstar
```
This will produce the executable named `mpi-rockstar` in the mpi-rockstar directory.

4. To run mpi-rockstar fo Nyx snaphosts, 
`cd rockstar_examples/Parallel/NyxTesting`

5. Create a .cfg file like the ones provided in this folder - `rockstar_allsnapshots.cfg`.
```
INBASE = <directory-containing-gadget-files>
FILENAME = "nyx_snapshot.<snap>.<block>"
NUM_BLOCKS = <number-of-gadget-files-for-each-snapshot>
STARTING_SNAP=0
NUM_SNAPS = <number-of-snapshots>
RESCALE_PARTICLE_MASS = 1
OUTBASE = <output-directory>
```
With the above string for the `FILENAME`, the gadget filenames has to be of the form `nyx_snapshot.005.0`, `nyx_snapshot.005.1` etc., where 5 is the snapshot id and 0, 1 are the block id.

5. Run
`mpirun -np <num_ranks> mpi-rockstar -c <.cfg-file>`
This will write the halo.bin files and halo.ascii files into the `OUTBASE` directory. There will be as many `.bin` and `.ascii` files as the number of MPI ranks for each snapshot. The `.ascii` files contain a summary of the halos, and `.bin` contains the particles that form each of the halos. An example of the `.cfg` file is given in `rockstar_allsnapshots.cfg`. For a $$1024^3$$ particle snapshot, it takes about 5-10 minutes to compute the halos for that snapshot on 256 MPI ranks.

6. To visualize the halos
```
cd WriteHalosVTK
make -j8
./write_halos_vtk --halo-bin-file=<halo.bin> --gadget-files-dir=<dir-containing-the-gadget-files> --desc_snapshot=<snapshot-id>
```
For example `halo_000.10.bin` file will contain the locations of all the particles of all halos for the snapshot 0 with the halo center in the processor with rank 10. The above will write each of those halos into a separte `.vtk` file that can be visualized in ParaView. 

### Nyx-mpi-rockstar halo output

The image below shows the result of executing the above pipeline for a Nyx snapshot with $$256^3$$ particles at a redshift of `z=2.0` on a 20 Mpc/h simulation domain.

<div align="center">

<img src="./Images/Nyx_mpi-rockstar_256_halos.png" width="800"><br>

<i>Figure 3:</i> Halos identified by mpi-rockstar for a redshift of `z=2` snapshot on a 20 Mpc/h simulation domain.

</div>


## 2. Computing the halo mergers

For this, the companion code of Rockstar - [consistent-trees](https://github.com/cosmo-suite/consistent-trees.git) is used. This is a copy of the [original consistent-trees codebase](https://bitbucket.org/pbehroozi/consistent-trees/src/main/). Here are the steps


1. Clone the `consistent-trees` repository
```
git clone --recursive https://github.com/cosmo-suite/consistent-trees.git
```

2. Generate the merger directory and `merger-tree.cfg`. Execute this command from this directory. ie. the directory that contains the `OUTBASE` directory that contains the halos.
```
perl </path/to/mpi-rockstar>/scripts/gen_merger_cfg.pl <path-to-rockstar.cfg>
```
The same configuration script (`.cfg`) should be used in the above command as in Step 5 above. This will create a directory named `outputs` in the `OUTBASE` directory, which will contain the `merger_tree.cfg` and `scales.txt` files. 

3. Two lines in the `OUTBASE/outputs/merger_tree` have to be replaced. See the `merger_tree.cfg` file provided in this directory. Copy the two lines 
```
EXTRA_PARAMS = 37
EXTRA_PARAM_LABELS = "rs_klypin Mvir_all M200b M200c M500c M2500c Xoff Voff spin_bullock b_to_a c_to_a A[x] A[y] A[z] b_to_a(500c) c_to_a(500c) A[x](500c) A[y](500c) A[z](500c) T/|U| M_pe_Behroozi M_pe_Diemer Halfmass_Radius rvmax NFW_chi2 Ixx Iyy Izz Ixy Iyz Izx Ixx(500c) Iyy(500c) Izz(500c) Ixy(500c) Iyz(500c) Izx(500c)"
``` 
and replace the corresponding lines in `OUTBASE/outputs/merger_tree`. This is because the mpi-rockstar output writes a lot more parameters in the `out*.list` files, and these lines will tell the code how many to read. These are extra parameters which do not affect the halo merger, but need to be provided so that the halo merger code knows how to read the `out*.list` files.

3. Compile `consistent-trees` 
```
cd consistent-trees
make -j8
```

4. Run the merger code
```
perl do_merger_tree.pl <path-to-OUTBASE>/outputs/merger_tree.cfg
```
This will generate the trees in the `OUTBASE/trees` directory. The trees are contained in a file named `tree_0_0_0.dat`. This is a serial code. It takes about 1-2 hours to generate the halo merger trees. The information encoded in the `tree_0_0_0.dat` is as follows. Each section is `#tree <descendant-halo-id>` for each of the halos in the very final snapshot. And under each of that section, the progenitors of that descendant halo will be written, and followed by the progenitors of the first progenitor, progenitors of the second progenitor, and so on.

## 3. Post-processing halo mergers
To post-process the halo merger code and visualize the halos
```
cd WriteHaloMergersVTK
./parse_trees.exe <descendant-halo-id>
```
To visualize the halo merging for a given descendant halo for a given halo id


