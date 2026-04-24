python3 Plot_kvsPk_CLASS_Gimlet_Nyx.py \
  --pk-nyx-file /pscratch/sd/n/nataraj2/Nyx/Nyx_MyLightcone/HaloFinder_GPU/spectrum_Nyx_512_LCDM.txt \
  --pk-gimlet-file /pscratch/sd/n/nataraj2/Nyx/Gimlet/gimlet2/apps/matter_pk/spectrum_LCDM_rhom_ps3d.txt \
  --pk-cosmicic-file /pscratch/sd/n/nataraj2/Nyx/cosmo-suite/modcon-cosmology/PythonScripts/PowerSpectrum_CosmicIC/spectrum_CosmicIC_LCDM.txt \
  --labels "Nyx (\$512^3\$)"  "Gimlet" "CosmicIC" \
  --dofz 4.017582e-05 \
  --h 0.675 \
  --output Pk_all_LCDM.png
