# DESI DR1 & Nyx 1D Flux Power Spectrum Comparison

This Python script plots the **DESI Data Release 1 (DR1) Ly-\alpha 1D Flux Power Spectrum** ($P_{\rm 1D}$) against multiple **Nyx** simulation datasets (e.g., CDM, WDM, FDM) at a target redshift ($z$).

---

## Prerequisites

Make sure you have Python 3 installed along with the required scientific libraries:

```bash
pip install numpy matplotlib astropy
```

---

## Data Preparation

### 1. DESI Baseline Data
The DESI DR1 Ly-$\alpha$ 1D power spectrum baseline FITS file must be downloaded from Zenodo. 

* **Zenodo Record:** [https://zenodo.org/records/17100543](https://zenodo.org/records/17100543)

You can download the baseline file directly using `wget`:

```bash
 wget https://zenodo.org/records/17100543/files/zenodo_p1d_fft_y1.zip
```

### 2. Nyx Simulation Data
Prepare your Nyx 1D power spectrum text files (formatted with wavenumber $k$ and power spectrum values).

---

## Usage

Run the script from the command line by specifying the DESI FITS file, target redshift, output filename, and any number of paired Nyx datasets (`--nyx-data-N`) and corresponding legend labels (`--legend-N`).

### Command-Line Arguments

* `--desi-data`: Path to the DESI baseline `p1d_fft_y1_measurement_kms_v8_baseline.fits` file (**Required**).
* `--z-target`: Target redshift to filter, e.g., `2.2` (**Required**).
* `--output`: Filename for the output plot image, e.g., `plot.png` (**Required**).
* `--nyx-data-N`: Path to the $N$-th Nyx dataset text file (where $N = 1, 2, 3, \dots$).
* `--legend-N`: Legend label corresponding to the $N$-th Nyx dataset.

---

## Example Command

```bash
python3 Plot_P1D_comparison_DESI.py \
    --desi-data=p1d_fft_y1_measurement_kms_v8_baseline.fits \
    --nyx-data-1=path/to/p1d_CDM_z_eq_2p2_ave_ps1d.txt \
    --nyx-data-2=path/to/p1d_WDM_2p2.txt_ave_ps1d.txt \
    --nyx-data-3=path/to/p1d_FDM_2p2.txt_ave_ps1d.txt \
    --legend-1="CDM" \
    --legend-2="WDM ($m=2.1$ keV)" \
    --legend-3="FDM ($m = 10^{-22}$ eV)" \
    --z-target=2.2 \
    --output=plot_p1d_comparison.png
```
