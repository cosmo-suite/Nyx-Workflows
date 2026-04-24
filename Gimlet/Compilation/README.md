# Gimlet - Post processor for Nyx
Gimlet is a repository that contains functionality for post processing Nyx outputs.  

1. Clone the [Gimlet repository](https://bitbucket.org/zarija/gimlet2/src/master/) (access needed)

# How to run on Perlmutter (NERSC)

2. Make a backup of your current `~/.bash_profile` file on Perlmutter  
`cp ~/.bash_profile ~/.bash_profile.bak`

3. Delete the `~/.bash_profile` and create a new one with the following entries.

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

4. `source ~/.bash_profile`

5. In `gimlet2/platform.make` - make the following change (ie. comment the last line and uncomment `F_LIBS=-lgfortran`)

```
# GNU:
F_LIBS := -lgfortran
# Intel:
#F_LIBS := -lifcore
```

6. `cd gimlet2`
7. `make -j8`
8. `cd gimlet2/apps/lya_fields` (or any of the apps folders)
9. `make -j8`

