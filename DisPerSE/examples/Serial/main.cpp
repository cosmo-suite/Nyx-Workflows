#include <iostream>
#include <fstream>
#include <random>
#include <cstring>
#include <iomanip>
#include <cstdlib>

// ============================================================
// NDfield definitions
// ============================================================

#define NDFIELD_MAX_DIMS 20

// NDfield datatype definitions
#define ND_CHAR    1
#define ND_UCHAR   2
#define ND_SHORT   4
#define ND_USHORT  8
#define ND_INT     16
#define ND_UINT    32
#define ND_LONG    64
#define ND_ULONG   128
#define ND_FLOAT   256
#define ND_DOUBLE  512

#include "NDfield.h"

void 
CreateField(const double Lx, const double Ly, const double Lz,
                 const int nx, const int ny, const int nz, NDfield& field) 
{

    // ========================================================
    // Total number of grid cells
    // ========================================================

    const long nval =
        static_cast<long>(nx) *
        static_cast<long>(ny) *
        static_cast<long>(nz);


    // ========================================================
    // Grid spacing
    //
    // This is NOT stored in delta.
    //
    // delta stores the bounding-box size.
    // ========================================================

    const double dx = Lx / nx;
    const double dy = Ly / ny;
    const double dz = Lz / nz;

    // Initialize entire structure to zero.
    std::memset(&field, 0, sizeof(NDfield));


    // ========================================================
    // Comment
    // ========================================================

    std::strncpy(
        field.comment,
        "Random positive density field",
        sizeof(field.comment) - 1
    );


    // ========================================================
    // Number of dimensions
    // ========================================================

    field.ndims = 3;

    field.n_dims = 3;


    // ========================================================
    // Regular grid
    //
    // 0 = regular grid
    // 1 = particle coordinates
    // ========================================================

    field.fdims_index = 0;


    // ========================================================
    // Data type
    //
    // ND_DOUBLE = 512
    // ========================================================

    field.datatype = ND_DOUBLE;

    field.datasize = sizeof(double);


    // ========================================================
    // Grid dimensions
    // ========================================================

    field.dims[0] = nx;
    field.dims[1] = ny;
    field.dims[2] = nz;


    // ========================================================
    // Bounding-box origin
    // ========================================================

    field.x0[0] = 0.0;
    field.x0[1] = 0.0;
    field.x0[2] = 0.0;


    // ========================================================
    // Bounding-box SIZE
    //
    // IMPORTANT:
    //
    // delta is NOT:
    //
    //     (dx, dy, dz)
    //
    // It is:
    //
    //     (Lx, Ly, Lz)
    //
    // ========================================================

    field.delta[0] = Lx;
    field.delta[1] = Ly;
    field.delta[2] = Lz;


    // ========================================================
    // Number of data values
    // ========================================================

    field.nval = nval;


    // ========================================================
    // Allocate field data
    // ========================================================

    double* data = new double[nval];

    field.val = data;

    
// ============================================================
// Define filaments
// ============================================================

std::vector<Filament> filaments =
{
    // Main diagonal filament
    {
        { 1.0,  2.0,  1.0 },
        {19.0, 18.0, 19.0 },
        8.0,
        0.35
    },

    // Crossing filament
    {
        { 2.0, 18.0,  4.0 },
        {18.0,  3.0,  4.0 },
        6.0,
        0.30
    },

    // Another thinner filament
    {
        { 3.0,  4.0, 18.0 },
        {17.0, 16.0,  3.0 },
        5.0,
        0.25
    }
};
    

// ============================================================
// Random number generator
// ============================================================

std::mt19937_64 rng(12345);


// Small fluctuations around the smooth density field
std::normal_distribution<double> noise_distribution(0.0, 1.0e-8);


// ============================================================
// Construct density field
// ============================================================

for (int k = 0; k < nz; ++k)
{
    for (int j = 0; j < ny; ++j)
    {
        for (int i = 0; i < nx; ++i)
        {
            const long index =
                static_cast<long>(i)
                +
                static_cast<long>(nx)
                *
                (
                    static_cast<long>(j)
                    +
                    static_cast<long>(ny)
                    *
                    static_cast<long>(k)
                );


            // ------------------------------------------------
            // Cell/point position
            // ------------------------------------------------

            const double x =
                (static_cast<double>(i) + 0.5) * dx;

            const double y =
                (static_cast<double>(j) + 0.5) * dy;

            const double z =
                (static_cast<double>(k) + 0.5) * dz;

            Vec3 p{x, y, z};


            // ------------------------------------------------
            // Background density
            // ------------------------------------------------

            double density = 1.0;
                
            double eps = 1.0e-8;

            double background =
            eps * (
                std::sin(2.0 * M_PI * x / Lx) +
                std::sin(2.0 * M_PI * y / Ly) +
                std::sin(2.0 * M_PI * z / Lz)
            );

            //density += noise_distribution(rng);
            density += background;
    
                // ------------------------------------------------
            // Add filament contributions
            // ------------------------------------------------

            for (const auto& filament : filaments)
            {
                const double d2 =
                    DistanceSquaredToSegment(
                        p,
                        filament.start,
                        filament.end
                    );

                const double sigma2 =
                    filament.sigma *
                    filament.sigma;

                const double contribution =
                    filament.amplitude *
                    std::exp(
                        -0.5 * d2 / sigma2
                    );

                density += contribution;
            }


            // ------------------------------------------------
            // Add small random fluctuations
            // ------------------------------------------------

            //density += noise_distribution(rng);


            // ------------------------------------------------
            // Make absolutely sure density stays positive
            // ------------------------------------------------

            if (density < 0.01)
                density = 0.01;


            data[index] = density;
        }
    }
}

    // ========================================================
    // Print NDfield information
    // ========================================================

    std::cout << "\n";
    std::cout << "========================================\n";
    std::cout << "NDfield information\n";
    std::cout << "========================================\n";

    std::cout << "Comment     : "
              << field.comment << "\n";

    std::cout << "ndims       : "
              << field.ndims << "\n";

    std::cout << "n_dims      : "
              << field.n_dims << "\n";

    std::cout << "dims        : "
              << field.dims[0] << " "
              << field.dims[1] << " "
              << field.dims[2] << "\n";

    std::cout << "fdims_index : "
              << field.fdims_index << "\n";

    std::cout << "datatype    : "
              << field.datatype
              << " (ND_DOUBLE)\n";

    std::cout << "datasize    : "
              << field.datasize
              << " bytes\n";

    std::cout << "nval        : "
              << field.nval << "\n";

    std::cout << "x0          : "
              << field.x0[0] << " "
              << field.x0[1] << " "
              << field.x0[2] << "\n";

    std::cout << "delta       : "
              << field.delta[0] << " "
              << field.delta[1] << " "
              << field.delta[2] << "\n";

    std::cout << "grid spacing: "
              << dx << " "
              << dy << " "
              << dz << "\n";

    std::cout << "domain      : "
              << Lx << " x "
              << Ly << " x "
              << Lz << "\n";

    std::cout << "total cells : "
              << nval << "\n";

    std::cout << "========================================\n";


    // ========================================================
    // Print first few density values
    // ========================================================

    std::cout << "\nFirst 10 density values:\n";

    for (int i = 0; i < 10; ++i)
    {
        std::cout << std::setprecision(12)
                  << data[i] << "\n";
    }
}
                 

// ============================================================
// Main
// ============================================================

int main()
{

    // ========================================================
    // Physical domain
    // ========================================================

    const double Lx = 20.0;
    const double Ly = 20.0;
    const double Lz = 20.0;


    // ========================================================
    // Grid dimensions
    // ========================================================

    const int nx = 128;
    const int ny = 128;
    const int nz = 128;


    NDfield field;
    CreateField(Lx, Ly, Lz, nx, ny, nz, field);

    // ========================================================
    // Write NDfield file
    // ========================================================

    const char* filename = "random_density.ndfield";

    int err = WriteNDfield(filename, &field);

    WriteNDfieldToVTK("random_density.vtk", field, "density");

    field.val = nullptr;


    std::cout << "\nDone.\n";

    return 0;
}
