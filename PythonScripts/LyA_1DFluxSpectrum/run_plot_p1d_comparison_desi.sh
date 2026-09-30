python3 Plot_P1D_comparison_DESI.py \
    --desi-data=/pscratch/sd/n/nataraj2/Nyx/Nyx_MyLightcone/LyA_with_HaloFinder_GPU/LCDM_GadgetFiles/LyA_1DFluxSpectrum/DESI_data/zenodo_p1d_fft_y1/p1d_fft_y1_measurement_kms_v8_baseline.fits \
    --nyx-data-1=/pscratch/sd/n/nataraj2/Nyx/Nyx_MyLightcone/LyA_with_HaloFinder_GPU/LCDM_GadgetFiles/LyA_1DFluxSpectrum/p1d_CDM_z_eq_2p2_ave_ps1d.txt \
    --nyx-data-2=/pscratch/sd/n/nataraj2/Nyx/Nyx_MyLightcone/LyA_with_HaloFinder_GPU/WDM/LyA_1DFluxSpectrum/p1d_WDM_2p2.txt_ave_ps1d.txt \
    --nyx-data-3=/pscratch/sd/n/nataraj2/Nyx/Nyx_MyLightcone/LyA_with_HaloFinder_GPU/FDM/LyA_1DFluxSpectrum/p1d_FDM_2p2.txt_ave_ps1d.txt \
    --legend-1="CDM" \
    --legend-2="WDM (\$m=2.1\$ keV)" \
    --legend-3="FDM (\$m = 1e-22\$ eV)" \
    --z-target=2.2 \
    --output=plot_p1d_comparison.png
