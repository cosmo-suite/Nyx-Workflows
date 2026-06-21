# Rockstar for halo identification and mergers

1. Clone the Rockstar repository and compile

```
1. git clone https://bitbucket.org/pbehroozi/rockstar-galaxies.git
2. cd rockstar-galaxies
3. make -j8
```
This will produce the executable named `rockstar-galaxies`.

To run examples of halo finding with Rockstar, go into one of the example folders in 
`rockstar_example` - `Example1`, `Example 2` or `Example3` and do the following.  
4. Compile and run to generate particle snapshot
```
mpicxx -std=c++14 main.cpp -o out
./out
```
This will generate a binary file in gadget format containing the particles.  
5. `./rockstar-galaxies -c rockstar.cfg <particle-gadget-file>`    
where `<particle-gadget-file>` is the output file produced in step 4. This will produce a folder named 
`halos` with 
- `.ascii` file which contains information of the halos,
- `.bin` file which is binary file which contains the particle locations of the halos
- `.rbin` file which contains all the particles in the domain

To visualize all the particles and the halo particles, `.vtk` files can be written
```
7. cd WriteHalosVTK
8. mpicxx -std=c++14 main.cpp -o out
9. ./out <.bin file> <.rbin file> <output-dir-for-vtk-files>
```
where `<.bin file>` and `<.rbin file>` are from Step 5. This will write individual `.vtk` files for each of the halo produced and a `all_particles.vtk` file 
for all the particles in the domain

