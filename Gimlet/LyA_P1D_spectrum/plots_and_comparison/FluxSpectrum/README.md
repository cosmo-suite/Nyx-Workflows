# FluxSpectrum

Scripts and comparison data for the Lyman-alpha 1D flux power spectrum (P1D) from Nyx plotfiles.

## Contents

| Item | Purpose |
|---|---|
| `run_compute_lyap_1d_and_tau_eff.sh` | Batch driver: plotfile -> HDF5 -> `p1d_all_axes.ex` -> summary of `tau_eff` vs z |
| `Plot_P1D_all_vs_DESI.py` | Plots each Nyx P1D against DESI DR1 and literature data |
| `DataForComparison/Irsic_2017/` | Irsic et al. 2017 P1D curves at z = 3.0, 3.6, 4.2 |
| `DataForComparison/Walther_2018/` | Walther et al. 2018 P1D points at z = 2.4, 2.6, 2.8, 3.0, 3.2, 3.4 |

(`.Walther_LyA_P1d_z_eq_3p2.csv.swp` is a stray vim swap file and can be deleted.)

## 1. Computing P1D and tau_eff

```
sh run_compute_lyap_1d_and_tau_eff.sh --reionization-data-file=<file> [plt_filelist.txt]
```

Run inside an allocation. Needs `convert3d.gnu.x86-milan.PROF.MPI.ex` and `p1d_all_axes.ex` in the working directory (512 MPI tasks by default).

- `plt_filelist.txt` has one plotfile path per line. Blank lines and `#` comments are ignored. The default name is `plt_filelist.txt`.
- For each plotfile the script:
  1. converts it to HDF5 in `h5_snapshots/`. It skips this if a complete file exists and redoes truncated ones.
  2. runs `p1d_all_axes.ex`, writing to `p1d_output/p1d_<plotname>_*`, including `*_ave_ps1d.txt`.
  3. appends `z tau_eff` to `tau_eff_vs_z.txt`. The values are unscaled tau, averaged over the x, y and z axes.
- A failure on one plotfile is reported and skipped. The script then continues with the next one.
- `tau_eff_vs_z.txt` is rewritten from scratch each run.

## 2. Plotting against DESI and literature

```
python Plot_P1D_all_vs_DESI.py [--input-dir p1d_output] [--output-dir DIR] \
    [--desi-data FITS] [--comparison-dir DataForComparison] [--legend CDM]
```

- Reads every `*_ave_ps1d.txt` in `--input-dir`. The redshift is parsed from the `z=` value in the header line.
- Files with z > 4.2 are skipped.
- Writes one PNG per file to `<input-dir>/Images/` (default) as `<stem>_P1D_vs_DESI.png`.
- Plotted quantity is kP(k) vs k in s/km. The Nyx curve is multiplied by pi, and only k < 0.1 s/km is shown.
- DESI DR1 (FITS, `P1D_BLIND` table) uses the nearest DESI z bin. The default path is `DESI_data/zenodo_p1d_fft_y1/p1d_fft_y1_measurement_kms_v8_baseline.fits`, relative to the script. Unlike the other inputs, it is not included in this folder.
- Literature overlays are drawn only when the Nyx z is within 0.01 of the comparison z:
  - Irsic 2017: dashed black line.
  - Walther 2018: red square error bars.
- Requires `numpy`, `matplotlib`, `astropy`.

## Comparison data format

- Filenames are `<Author>_LyA_P1d_z_eq_<z>.csv`, with `p` as the decimal point. For example `3p6` means z = 3.6.
- The first line is a header (`z=<value>`) and is skipped when reading. The redshift is taken from the filename.
- Two columns: k [s/km] and P1D value. Values were digitised from the papers' plots and are multiplied by pi when plotted, because the source plots divide by pi.
- Irsic files are a curve, one (k, value) pair per row.
- Walther files are error bars. Each consecutive pair of rows (0-1, 2-3, ...) is the two ends of one bar. The point is placed at the mean k and the centre value, with a half-difference error.
