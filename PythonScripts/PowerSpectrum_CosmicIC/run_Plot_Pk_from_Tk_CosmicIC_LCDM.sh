LABEL0='CDM'

python3 Plot_Pk_from_Tk_CosmicIC.py \
  --tk-files \
    /pscratch/sd/n/nataraj2/Nyx/Nyx_MyLightcone/HaloFinder_GPU/LCDM/cosmicic/transfer_function.txt \
  --labels "$LABEL0" \
  --n_s 0.96 \
  --sigma8 0.83 \
  --h 0.675 \
  --output Pk_LCDM.png \
  --output-pk-files \
    spectrum_CosmicIC_LCDM.txt 
