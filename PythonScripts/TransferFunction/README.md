# Python scripts for transfer function comparison


## 1. ```TFCompare_CosmicIC_CLASS.py```
This script compares and visualizes normalized data from two different sources (e.g., `cosmicic` and `class` simulation outputs). It generates log-log plots for the transfer function.

### Usage
The script requires two input files via command-line arguments. Each file is expected to be a text-based format with at least 3 columns.
```
python3 plot_comparison.py --file_cosmicic path/to/cosmicic_data.txt --file_class path/to/class_data.txt
```

### Arguments
* ```--file_cosmicic```: Path to the first data file (rendered with circular markers).
* ```--file_class```: Path to the second data file (rendered as a solid line).

### Outputs
Upon successful execution, the script saves two images in the current directory:
1.  ```fig1_col2.png```: Comparison of the normalized second column.
2.  ```fig2_col3.png```: Comparison of the normalized third column.

## 2. ```TF_Compare_LCDM_WDM_FDM.py```
This script calculates and plots the ratio of transfer functions for alternative dark matter models (Warm Dark Matter and Fuzzy Dark Matter) relative to a baseline $\Lambda$CDM model.

### Usage
The script requires the $\Lambda\mathrm{CDM}$ baseline file and at least one alternative model file to generate the comparison.
```
python3 TF_Compare_LCDM_WDM_FDM.py --file-lcdm lcdm.txt --file-wdm1 wdm_085.txt --file-fdm1 fdm_1022.txt
```

### Arguments
* `--file-lcdm`: **(Required)** Path to the $\Lambda$CDM baseline transfer function file.
* `--file-wdm1`: Path to WDM data ($m_{\mathrm{WDM}} = 0.85$ keV).
* `--file-wdm2`: Path to WDM data ($m_{\mathrm{WDM}} = 2.1$ keV).
* `--file-fdm1`: Path to FDM data ($m_{\mathrm{FDM}} = 10^{-22}$ eV).
* `--file-fdm2`: Path to FDM data ($m_{\mathrm{FDM}} = 10^{-21}$ eV).

### Outputs
Upon successful execution, the script saves one high-resolution image in the current directory:
1. **```TF_Compare_LCDM_WDM_FDM.png```**: A semi-log plot showing the ratio $T(k) / T_{\Lambda\mathrm{CDM}}(k)$ across the wavenumber range $[0.01, 100.0]$.
