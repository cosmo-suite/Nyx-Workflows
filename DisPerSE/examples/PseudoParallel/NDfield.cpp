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
// Write NDfield header
//
// This writes EVERYTHING up to the beginning of the data.
//
// The returned value is the byte offset at which the data
// record marker is written.
//
// ============================================================

std::streamoff WriteNDfieldHeader(
    const char* filename,
    const NDfield& field)
{
    char tag[16];
    char dummy[160];

    std::memset(tag, 0, sizeof(tag));
    std::memset(dummy, 0, sizeof(dummy));

    std::strcpy(tag, NDFIELD_TAG);

    FILE* f = std::fopen(filename, "wb");

    if (!f)
    {
        std::cerr
            << "ERROR: could not open "
            << filename
            << " for writing\n";

        std::exit(EXIT_FAILURE);
    }

    // --------------------------------------------------------
    // Fortran-style record marker
    // --------------------------------------------------------

    int i = 16;

    std::fwrite(&i, sizeof(int), 1, f);

    // --------------------------------------------------------
    // Tag
    // --------------------------------------------------------

    std::fwrite(tag, sizeof(char), 16, f);

    // --------------------------------------------------------
    // Second record marker
    // --------------------------------------------------------

    std::fwrite(&i, sizeof(int), 1, f);


    // --------------------------------------------------------
    // Size of NDfield header record
    // --------------------------------------------------------

    i =
        sizeof(int) *
        (NDFIELD_MAX_DIMS + 3)
        +
        sizeof(double) *
        (2 * NDFIELD_MAX_DIMS)
        +
        (160 + 80) * sizeof(char);

    std::fwrite(&i, sizeof(int), 1, f);

  // --------------------------------------------------------
    // Comment
    // --------------------------------------------------------

    std::fwrite(
        field.comment,
        sizeof(char),
        80,
        f);


    // --------------------------------------------------------
    // ndims
    // --------------------------------------------------------

    std::fwrite(
        &field.ndims,
        sizeof(int),
        1,
        f);


    // --------------------------------------------------------
    // dims
    // --------------------------------------------------------

    std::fwrite(
        field.dims,
        sizeof(int),
        field.ndims,
        f);

    if (field.ndims < NDFIELD_MAX_DIMS)
    {
        std::fwrite(
            dummy,
            sizeof(int),
            NDFIELD_MAX_DIMS - field.ndims,
            f);
    }


    // --------------------------------------------------------
    // fdims_index
    // --------------------------------------------------------

    std::fwrite(
        &field.fdims_index,
        sizeof(int),
        1,
        f);


    // --------------------------------------------------------
    // datatype
    // --------------------------------------------------------

    std::fwrite(
        &field.datatype,
        sizeof(int),
        1,
        f);


    // --------------------------------------------------------
    // x0
    // --------------------------------------------------------

    std::fwrite(
        field.x0,
        sizeof(double),
        field.ndims,
        f);

    if (field.ndims < NDFIELD_MAX_DIMS)
    {
        std::fwrite(
            dummy,
            sizeof(double),
            NDFIELD_MAX_DIMS - field.ndims,
            f);
    }


    // --------------------------------------------------------
    // delta
    //
    // IMPORTANT:
    // These are the bounding-box sizes, not dx/dy/dz.
    // --------------------------------------------------------

    std::fwrite(
        field.delta,
        sizeof(double),
        field.ndims,
        f);

    if (field.ndims < NDFIELD_MAX_DIMS)
    {
        std::fwrite(
            dummy,
            sizeof(double),
            NDFIELD_MAX_DIMS - field.ndims,
            f);
    }


    // --------------------------------------------------------
    // Extension/dummy
    // --------------------------------------------------------

    std::fwrite(
        field.dummy,
        sizeof(char),
        160,
        f);


    // --------------------------------------------------------
    // End header record
    // --------------------------------------------------------

    std::fwrite(
        &i,
        sizeof(int),
        1,
        f);


    // --------------------------------------------------------
    // This is where the DATA record marker starts.
    // --------------------------------------------------------

    const std::streamoff data_record_offset =
        static_cast<std::streamoff>(std::ftell(f));

    std::fclose(f);

    return data_record_offset;
}

// ============================================================
// Write one block.
//
// The block is:
//     [i0,i1)
//     [j0,j1)
//     [k0,k1)
//
// IMPORTANT:
// Global data ordering is:
//
//     i + nx * (j + ny * k)
//
// Therefore a 3D block is not contiguous in the file.
// We write one contiguous x-row at a time.
// ============================================================

void WriteBlock(
    FILE* f,
    const double* global_data,
    const NDfield& field,
    int i0,
    int i1,
    int j0,
    int j1,
    int k0,
    int k1,
    std::streamoff data_start)
{
    const int nx = field.dims[0];
    const int ny = field.dims[1];

    for (int k = k0; k < k1; ++k)
    {
        for (int j = j0; j < j1; ++j)
        {
            // ------------------------------------------------
            // Global linear index of first cell in this row
            // ------------------------------------------------

            const long global_index =
                static_cast<long>(i0)
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
            // Byte offset in NDfield data section
            // ------------------------------------------------

            const std::streamoff offset =
                data_start
                +
                static_cast<std::streamoff>(
                    global_index *
                    sizeof(double));


            // ------------------------------------------------
            // Number of contiguous values in this row
            // ------------------------------------------------

            const int count = i1 - i0;


            // ------------------------------------------------
            // Seek to correct global position
            // ------------------------------------------------

            if (std::fseek(
                    f,
                    static_cast<long>(offset),
                    SEEK_SET) != 0)
            {
                std::cerr
                    << "ERROR: fseek failed\n";

                std::exit(EXIT_FAILURE);
            }


            // ------------------------------------------------
            // Write this x-row
            // ------------------------------------------------

            std::fwrite(
                global_data + global_index,
                sizeof(double),
                count,
                f);
        }
    }
}

// ============================================================
// Block-based serial NDfield writer
// ============================================================

void WriteNDfieldBlockedSerial(
    const char* filename,
    const NDfield& field,
    int nbx,
    int nby,
    int nbz)
{
    // --------------------------------------------------------
    // Open file for binary read/write
    // --------------------------------------------------------

    FILE* f = fopen(filename, "wb");

    if (f == nullptr)
    {
        std::cerr
            << "ERROR: cannot open file "
            << filename << std::endl;
        return;
    }

    // --------------------------------------------------------
    // Write header
    // --------------------------------------------------------

    WriteNDfieldHeader(filename, field);

    // --------------------------------------------------------
    // Write data-size marker and get start of data
    // --------------------------------------------------------

    const std::streamoff data_start =
        WriteNDfieldDataMarker(filename, field);

    if (data_start < 0)
    {
        fclose(f);
        return;
    }

    // --------------------------------------------------------
    // Dimensions
    // --------------------------------------------------------

    const int nx = field.dims[0];
    const int ny = field.dims[1];
    const int nz = field.dims[2];

    if (nx % nbx != 0 ||
        ny % nby != 0 ||
        nz % nbz != 0)
    {
        std::cerr
            << "ERROR: grid dimensions must be divisible "
            << "by number of blocks."
            << std::endl;

        fclose(f);
        return;
    }

    const int sx = nx / nbx;
    const int sy = ny / nby;
    const int sz = nz / nbz;

    const double* data =
        static_cast<const double*>(field.val);

    // --------------------------------------------------------
    // Loop over blocks
    // --------------------------------------------------------

    for (int bz = 0; bz < nbz; ++bz)
    {
        for (int by = 0; by < nby; ++by)
        {
            for (int bx = 0; bx < nbx; ++bx)
            {
                const int bx0 = bx * sx;
                const int bx1 = bx0 + sx;

                const int by0 = by * sy;
                const int by1 = by0 + sy;

                const int bz0 = bz * sz;
                const int bz1 = bz0 + sz;

                std::cout
                    << "Writing block ("
                    << bx << ", "
                    << by << ", "
                    << bz << ")"
                    << std::endl;

                WriteBlock(
                    f,
                    data,
                    field,
                    bx0, bx1,
                    by0, by1,
                    bz0, bz1,
                    data_start
                );
            }
        }
    }

    // --------------------------------------------------------
    // Write trailing data marker
    // --------------------------------------------------------

    const long long data_bytes =
        static_cast<long long>(field.nval) *
        static_cast<long long>(field.datasize);

    const int marker =
        static_cast<int>(data_bytes);

    const std::streamoff end_marker =
        data_start + data_bytes;

    if (fseek(
            f,
            static_cast<long>(end_marker),
            SEEK_SET) != 0)
    {
        std::cerr
            << "ERROR: failed to seek to trailing marker."
            << std::endl;
        fclose(f);
        return;
    }

    fwrite(
        &marker,
        sizeof(int),
        1,
        f
    );

    fclose(f);

    std::cout
        << "Finished writing blocked NDfield: "
        << filename
        << std::endl;
}


// ============================================================
// Create density field
// ============================================================

void CreateField(
    const double Lx,
    const double Ly,
    const double Lz,
    const int nx,
    const int ny,
    const int nz,
    NDfield& field)
{
    const long nval =
        static_cast<long>(nx) *
        static_cast<long>(ny) *
        static_cast<long>(nz);


    const double dx = Lx / nx;
    const double dy = Ly / ny;
    const double dz = Lz / nz;


    std::memset(
        &field,
        0,
        sizeof(NDfield));


    std::strncpy(
        field.comment,
        "Blocked synthetic density field",
        sizeof(field.comment) - 1);


    field.ndims = 3;
    field.n_dims = 3;

    field.fdims_index = 0;

    field.datatype = ND_DOUBLE;
    field.datasize = sizeof(double);

    field.dims[0] = nx;
    field.dims[1] = ny;
    field.dims[2] = nz;

    field.x0[0] = 0.0;
    field.x0[1] = 0.0;
    field.x0[2] = 0.0;

    field.delta[0] = Lx;
    field.delta[1] = Ly;
    field.delta[2] = Lz;

    field.nval = nval;


    double* data =
        new double[nval];

    field.val = data;


    // ========================================================
    // Filaments
    // ========================================================

    std::vector<Filament> filaments =
    {
        {
            { 1.0,  2.0,  1.0 },
            {19.0, 18.0, 19.0 },
            8.0,
            0.35
        },

        {
            { 2.0, 18.0,  4.0 },
            {18.0,  3.0,  4.0 },
            6.0,
            0.30
        },

        {
            { 3.0,  4.0, 18.0 },
            {17.0, 16.0,  3.0 },
            5.0,
            0.25
        }
    };


    // ========================================================
    // Construct density field
    // ========================================================

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


                const double x =
                    (static_cast<double>(i) + 0.5) * dx;

                const double y =
                    (static_cast<double>(j) + 0.5) * dy;

                const double z =
                    (static_cast<double>(k) + 0.5) * dz;


                Vec3 p{x, y, z};


                // ------------------------------------------------
                // Background
                // ------------------------------------------------

                double density = 1.0;


                // Very small smooth perturbation to avoid
                // exact degeneracy.
                const double eps = 1.0e-8;

                density +=
                    eps *
                    (
                        std::sin(2.0 * M_PI * x / Lx)
                        +
                        std::sin(2.0 * M_PI * y / Ly)
                        +
                        std::sin(2.0 * M_PI * z / Lz)
                    );


                // ------------------------------------------------
                // Filaments
                // ------------------------------------------------

                for (const auto& filament : filaments)
                {
                    const double d2 =
                        DistanceSquaredToSegment(
                            p,
                            filament.start,
                            filament.end);


                    const double sigma2 =
                        filament.sigma *
                        filament.sigma;


                    density +=
                        filament.amplitude *
                        std::exp(
                            -0.5 * d2 / sigma2);
                }


                if (density < 0.01)
                    density = 0.01;


                data[index] = density;
            }
        }
    }


    // ========================================================
    // Information
    // ========================================================

    std::cout
        << "\n========================================\n"
        << "NDfield information\n"
        << "========================================\n";

    std::cout
        << "dims        : "
        << nx << " "
        << ny << " "
        << nz << "\n";

    std::cout
        << "nval        : "
        << nval << "\n";

    std::cout
        << "x0          : "
        << field.x0[0] << " "
        << field.x0[1] << " "
        << field.x0[2] << "\n";

    std::cout
        << "delta       : "
        << field.delta[0] << " "
        << field.delta[1] << " "
        << field.delta[2] << "\n";

    std::cout
        << "grid spacing: "
        << dx << " "
        << dy << " "
        << dz << "\n";

    std::cout
        << "========================================\n";
}

std::streamoff WriteNDfieldDataMarker(
    const char* filename,
    const NDfield& field)
{
    std::ofstream f(
        filename,
        std::ios::binary |
        std::ios::in |
        std::ios::out
    );

    if (!f)
    {
        std::cerr
            << "ERROR: cannot open file "
            << filename << std::endl;
        return -1;
    }

    // --------------------------------------------------------
    // Size of the actual data block in bytes
    // --------------------------------------------------------

    const long long data_bytes =
        static_cast<long long>(field.nval) *
        static_cast<long long>(field.datasize);

    // --------------------------------------------------------
    // Move to the end of the header
    //
    // The header size is:
    //
    //   4                 dummy
    //   16                tag
    //   4                 dummy
    //   4                 header size
    //   80                comment
    //   4                 ndims
    //   20*4              dims
    //   4                 fdims_index
    //   4                 datatype
    //   20*8              x0
    //   20*8              delta
    //   160               dummy
    //   4                 dummy
    // --------------------------------------------------------

    const std::streamoff header_size =
          sizeof(int)                         // dummy
        + 16                                  // tag
        + sizeof(int)                         // dummy
        + sizeof(int)                         // header block size
        + 80                                  // comment
        + sizeof(int)                         // ndims
        + NDFIELD_MAX_DIMS * sizeof(int)      // dims
        + sizeof(int)                         // fdims_index
        + sizeof(int)                         // datatype
        + NDFIELD_MAX_DIMS * sizeof(double)   // x0
        + NDFIELD_MAX_DIMS * sizeof(double)   // delta
        + 160                                 // dummy
        + sizeof(int);                        // dummy

    // --------------------------------------------------------
    // Position at beginning of data-size marker
    // --------------------------------------------------------

    f.seekp(header_size, std::ios::beg);

    if (!f)
    {
        std::cerr
            << "ERROR: failed to seek to data marker"
            << std::endl;
        return -1;
    }

    // --------------------------------------------------------
    // Write data block size
    // --------------------------------------------------------

    const int marker = static_cast<int>(data_bytes);

    f.write(
        reinterpret_cast<const char*>(&marker),
        sizeof(int)
    );

    if (!f)
    {
        std::cerr
            << "ERROR: failed to write data marker"
            << std::endl;
        return -1;
    }

    // --------------------------------------------------------
    // This is where the actual NDfield data starts.
    // --------------------------------------------------------

    const std::streamoff data_start =
        header_size + sizeof(int);

    f.close();

    return data_start;
}
