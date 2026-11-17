# OpticalDepth

Effective Lyman-alpha optical depth, tau_eff = -ln<F>, versus redshift. Compares the Nyx result against a literature curve.

## Contents

| File | Purpose |
|---|---|
| `plot_tau_eff.py` | Plots tau_eff(z) from Nyx against the comparison curve |
| `tau_eff_vs_z_HarrdtAndMadau.txt` | Comparison data, labelled "Haardt & Madau" in the plot. It has two columns, `z` and `tau_eff`, with no header. It covers z of about 1.71 to 6.48, and tau_eff rises from about 0.09 to 12.8. |

(The comparison filename spells "Haardt" as "Harrdt". The script uses that exact spelling, so don't rename one without the other.)

## Usage

```
python plot_tau_eff.py [tau_eff_vs_z.txt] [output.png] [comparison.txt]
```

Defaults are `tau_eff_vs_z.txt`, `tau_eff_vs_z.png` and `tau_eff_vs_z_HarrdtAndMadau.txt`.

- The input `tau_eff_vs_z.txt` is produced by `../FluxSpectrum/run_compute_lyap_1d_and_tau_eff.sh`. It has `z tau_eff` per line, and the header line starts with `#`. Copy or point to it when running this script.
- The plot has a log y-axis. Haardt & Madau is a black line, and Nyx (LCDM) is a blue line.
- Requires `numpy` and `matplotlib`.
