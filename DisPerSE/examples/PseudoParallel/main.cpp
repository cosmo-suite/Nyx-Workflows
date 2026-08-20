#include <iostream>
#include <fstream>
#include <random>
#include <cstring>
#include <iomanip>
#include <cstdlib>
#include <vector>
#include <cmath>
#include <cstdio>
#include <cstdint>

#include "NDfield.h"

// ============================================================
// Main
// ============================================================

int main()
{
    const double Lx = 20.0;
    const double Ly = 20.0;
    const double Lz = 20.0;

    const int nx = 128;
    const int ny = 128;
    const int nz = 128;


    NDfield field;


    // ========================================================
    // Create complete global field
    // ========================================================

    CreateField(
        Lx,
        Ly,
        Lz,
        nx,
        ny,
        nz,
        field);


    // ========================================================
    // Serial block-based NDfield writer
    //
    // 2 x 2 x 2 = 8 blocks
    //
    // Eventually:
    //
    //     block 0 -> MPI rank 0
    //     block 1 -> MPI rank 1
    //     ...
    //     block 7 -> MPI rank 7
    //
    // ========================================================

    const char* filename =
        "random_density_blocked.ndfield";


    WriteNDfieldBlockedSerial(
        filename,
        field,
        2,
        2,
        2);


    // ========================================================
    // Cleanup
    // ========================================================

    delete[] static_cast<double*>(field.val);

    field.val = nullptr;


    std::cout
        << "\nDone.\n";

    return 0;
}
