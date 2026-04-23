# Post processing for Nyx
Gimlet is a repository that contains functionality for post processing Nyx outputs.  
Clone the repository (access needed)
```
git clone https://maheshnatarajan@bitbucket.org/zarija/gimlet2.git
```

# How to run on Perlmutter (NERSC)

1. Make a backup of your current `~/.bash_profile` file on Perlmutter  
`cp ~/.bash_profile ~/.bash_profile.bak`

2. Delete the `~/.bash_profile` and create a new one with the following entries.

```
module load craype-x86-milan
module load libfabric/1.20.1
module load craype-network-ofi
module load PrgEnv-gnu/8.5.0
module load cray-dsmml/0.2.2
module load cray-libsci/23.12.5
module load cray-mpich/8.1.28
module load craype/2.7.30
module load gcc-native/12.3
module load perftools-base/23.12.0
module load cpe/23.12
module load cudatoolkit/12.2
module load craype-accel-nvidia80
module load gpu/1.0
module load darshan/3.4.4
module load cray-hdf5-parallel/1.12.2.9
module load cray-netcdf-hdf5parallel
module load cray-fftw/3.3.10.8
```

3. `source ~/.bash_profile`

4. In `gimlet2/platform.make` - make the following change (ie. comment the last line and uncomment `F_LIBS=-lgfortran`)

```
# GNU:
F_LIBS := -lgfortran
# Intel:
#F_LIBS := -lifcore
```

5. `cd gimlet2`
6. `make -j8`
7. `cd gimlet2/apps/lya_fields` (or any of the apps folders)
8. `make -j8`

