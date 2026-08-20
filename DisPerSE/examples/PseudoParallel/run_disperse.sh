rm -rf out *.ndfield
make -j8 
./out
rm -rf *.MSC *.NDskl *.NDskl.vtk
./mse random_density.ndfield -nsig 7 -periodicity 111 -nthreads 32 -upSkl
./skelconv random_density.ndfield_s7.up.NDskl -to vtk
