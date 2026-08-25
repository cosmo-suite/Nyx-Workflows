#!/bin/bash

# Assuming your snapshots are in folders named after the model
SNAPSHOT="plt_CDM_z_eq_4p2.h5"
OUTPUT="p1d_CDM_z_eq_4p2"
    
# Run the tool
srun -n 512 p1d_all_axes.ex $SNAPSHOT $OUTPUT
