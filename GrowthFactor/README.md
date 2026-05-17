# Growth Factor

  Compute the linear growth factor D(z) for a flat wCDM cosmology using a simple ODE solver. The program reads cosmological parameters from a `.par` file, reads a list of
  redshifts from a text file, and writes the corresponding growth factors to `growth_factors.txt`.

  ## Build

  This project uses `mpicxx` and C++17. Build with:

  ```bash
  make
  ```
  The output binary is `growth_factor.ex`.
  ## Run
  ```bash
  ./growth_factor.ex input_20Mpcbyh_1024.par z_vals.txt
  ```
  This writes one growth factor per line to `growth_factors.txt`, in the same order as the input redshifts.
