LABEL0='CDM'
LABEL1='WDM $(m = 2.1$ keV)'
LABEL2='FDM $(m = 1e-22$ eV)'

python3 Plot_Pk_from_Tk_CosmicIC.py \
  --tk-files \
    /pscratch/sd/n/nataraj2/Nyx/Nyx_MyLightcone/HaloFinder_GPU/LCDM/cosmicic/transfer_function.txt \
    /pscratch/sd/n/nataraj2/Nyx/Nyx_MyLightcone/HaloFinder_GPU/WDM/cosmicic/transfer_function.txt \
    /pscratch/sd/n/nataraj2/Nyx/Nyx_MyLightcone/HaloFinder_GPU/FDM/cosmicic/transfer_function.txt \
  --labels "$LABEL0" "$LABEL1" "$LABEL2" \
  --n_s 0.96 \
  --sigma8 0.83 \
  --h 0.675 \
  --output Pk_All.png \
  --output-pk-files \
    spectrum_CosmicIC_All.txt \
    spectrum_CosmicIC_All.txt \
    spectrum_CosmicIC_All.txt
