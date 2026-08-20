# DisPerSE

[**DisPerSE** (Discrete Persistent Structures Extractor)](https://github.com/thierry-sousbie/DisPerSE) is a scientific toolkit designed to analyze the topological structure of multidimensional datasets using Discrete Morse Theory and Persistence Diagrams.


## 🛠️ Prerequisites & Installation

Ensure system dependencies and build utilities are installed before compiling:

```bash
# Install core dependencies
sudo apt update && sudo apt install -y \
  libcfitsio-dev \
  libgmp-dev \
  libmpfr-dev \
  libboost-all-dev \
  libcgal-dev

# Install GSL and development utilities
sudo apt update && sudo apt install -y \
  libgsl-dev \
  pkg-config
```

---

## Clone the repository
```
git clone --recursive https://github.com/thierry-sousbie/DisPerSE
```

## ⚙️ Compilation


Build the toolkit using an out-of-source CMake setup:

```bash
# Navigate to the repository root
cd DisPerSE

# Create and enter the build directory
mkdir -p build && cd build

# Configure and compile
cmake ..
make -j$(nproc)
```

Upon successful compilation, the generated executables will be located in the `bin/` directory:

| Executable | Description |
| :--- | :--- |
| **`mse`** | Extracts the Morse-Smale complex (critical points, filaments, walls) |
| **`skelconv`** | Converts skeleton (`.NDskl`) files into other data formats (e.g., VTK) |
| **`netconv`** | Converts Delaunay/Voronoi networks into internal formats |
| **`fieldconv`** | Converts regular grids and density fields |
| **`delaunay_2D`** | Computes 2D Delaunay tessellations from point samples |
| **`delaunay_3D`** | Computes 3D Delaunay tessellations from point samples |

---

## 🚀 Usage Examples

1. The `Serial` directory has an example of a density field. Do
```
make -j8
./out
```

### 1. Extracting Topological Features (`mse`)
Run `mse` on a density field to extract persistence structures (such as cosmic skeletons):

```bash
./bin/mse random_density.ndfield -nsig 3 -periodicity 111 -nthreads 32 -upSkl
```
* **`-nsig 3`**: Filters out noise below a $3\sigma$ persistence threshold.
* **`-periodicity 111`**: Enforces periodic boundary conditions across all 3 dimensions.
* **`-nthreads 32`**: Parallelizes execution across 32 threads.
* **`-upSkl`**: Generates the ascending persistence skeleton (`.NDskl`).

### 2. Converting Output Skeletons (`skelconv`)
Convert the resulting skeleton file into standard VTK format for visualization in **ParaView**:

```bash
./bin/skelconv random_density.ndfield.up.NDskl -to vtk
```
