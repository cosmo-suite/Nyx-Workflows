LABEL0='Nyx $(512^3)\, m_{\mathrm{fdm}}=10^{-21}\,\mathrm{eV}$'
python3 Plot_kvsPk_CLASS_Gimlet_Nyx.py \
  --pk-nyx-file /pscratch/sd/n/nataraj2/Nyx/Nyx_MyLightcone/HaloFinder_GPU/spectrum_Nyx_512_FDM.txt \
  --pk-gimlet-file /pscratch/sd/n/nataraj2/Nyx/Gimlet/gimlet2/apps/matter_pk/spectrum_FDM_rhom_ps3d.txt \
  --pk-cosmicic-file /pscratch/sd/n/nataraj2/Nyx/cosmo-suite/modcon-cosmology/PythonScripts/PowerSpectrum_CosmicIC/spectrum_CosmicIC_FDM_10em21.txt \
  --labels "$LABEL0"  "Gimlet" "CosmicIC" \
  --dofz 4.017582e-05 \
  --h 0.675 \
  --output Pk_all_FDM.png
