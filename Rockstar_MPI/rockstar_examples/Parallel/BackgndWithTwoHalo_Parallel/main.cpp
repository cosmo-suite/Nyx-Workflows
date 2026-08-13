#include <cassert>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <fstream>
#include <iostream>
#include <random>
#include <array>
#include <vector>

#define GADGET_HEADER_SIZE 256

#include <fstream>
#include <iostream>
#include <vector>
#include <bit> // Required for std::byteswap (C++20)


#include <fstream>
#include <iostream>
#include <vector>
#include <iomanip>
#include <sstream>

#include <filesystem>


// Runtime check for little endianness
static inline bool is_little_endian() 
{
    uint16_t number = 0x1;
    char* numPtr = reinterpret_cast<char*>(&number);
    return (numPtr[0] == 1);
}

// Manual byte-swapping that safely handles both integers and floats
template <typename T>
static inline T to_big_endian(T val) 
{
    if (!is_little_endian()) {
        return val;
    }

    T swapped;
    const char* src = reinterpret_cast<const char*>(&val);
    char* dest = reinterpret_cast<char*>(&swapped);
    size_t size = sizeof(T);

    for (size_t i = 0; i < size; ++i) {
        dest[i] = src[size - 1 - i];
    }

    return swapped;
}


void write_vtk_binary(const std::string& filename, 
                      const std::vector<float>& pos, 
                      const std::vector<float>& vel, 
                      const std::vector<int64_t>& ids) 
{
    std::ofstream out(filename, std::ios::binary);
    if (!out) {
        std::cerr << "Failed to open VTK output file: " << filename << "\n";
        return;
    }

    uint64_t ntot = ids.size();

    // 1. Write the ASCII Header text
    out << "# vtk DataFile Version 3.0\n";
    out << "Gadget ICs Snapshot Visualization\n";
    out << "BINARY\n";
    out << "DATASET UNSTRUCTURED_GRID\n";

    // 2. Write Points (Positions)
    out << "POINTS " << ntot << " float\n";
    for (size_t i = 0; i < pos.size(); ++i) {
        float swapped = to_big_endian(pos[i]);
        out.write(reinterpret_cast<const char*>(&swapped), sizeof(float));
    }
    out << "\n"; // VTK requires a newline after binary blocks

    // 3. Write Topology (Treating every particle as a VTK_VERTEX / Cell Type 1)
    // VTK expects: CELLS [num_cells] [size of cell list array]
    // Each cell list entries are: [num_points_in_cell, point_id_0, point_id_1...]
    out << "CELLS " << ntot << " " << (ntot * 2) << "\n";
    for (uint32_t i = 0; i < ntot; ++i) {
        int32_t cell_size = to_big_endian(static_cast<int32_t>(1));
        int32_t pt_idx = to_big_endian(static_cast<int32_t>(i));
        
        out.write(reinterpret_cast<const char*>(&cell_size), sizeof(int32_t));
        out.write(reinterpret_cast<const char*>(&pt_idx), sizeof(int32_t));
    }
    out << "\n";

    // Cell Types (1 = VTK_VERTEX)
    out << "CELL_TYPES " << ntot << "\n";
    int32_t cell_type = to_big_endian(static_cast<int32_t>(1)); 
    for (uint32_t i = 0; i < ntot; ++i) {
        out.write(reinterpret_cast<const char*>(&cell_type), sizeof(int32_t));
    }
    out << "\n";

    // 4. Write Point Data (Velocities and Particle IDs)
    out << "POINT_DATA " << ntot << "\n";
    
    // Velocity vectors
    out << "VECTORS velocity float\n";
    for (size_t i = 0; i < vel.size(); ++i) {
        float swapped = to_big_endian(vel[i]);
        out.write(reinterpret_cast<const char*>(&swapped), sizeof(float));
    }
    out << "\n";

    // ID scalars
    out << "SCALARS particle_id int 1\n";
    out << "LOOKUP_TABLE default\n";
    for (size_t i = 0; i < ids.size(); ++i) {
        int32_t swapped = to_big_endian(ids[i]);
        out.write(reinterpret_cast<const char*>(&swapped), sizeof(int32_t));
    }
    out << "\n";

    out.close();
    std::cout << "Successfully generated VTK file: " << filename << "\n";
}

// ------------------------------------------------------------
// Gadget-2 header
// ------------------------------------------------------------
struct GadgetHeader
{
    uint32_t num_particles[6];
    double   particle_masses[6];
    double   scale_factor;
    double   redshift;
    int32_t  flag_sfr;
    int32_t  flag_feedback;
    uint32_t num_total_particles[6];
    int32_t  flag_cooling;
    int32_t  num_files_per_snapshot;
    double   box_size;
    double   omega_0;
    double   omega_lambda;
    double   h_0;
    int32_t  flag_stellarage;
    int32_t  flag_metals;
    int32_t  num_total_particles_hw[6];
    int32_t  flag_entropy_ics;
    char     unused[60];
};

static_assert(sizeof(GadgetHeader) == 256, "");

// ------------------------------------------------------------
// Write a Gadget block
// ------------------------------------------------------------
void write_block(std::ofstream& out,
                 const void* data,
                 uint32_t nbytes)
{
    out.write(reinterpret_cast<const char*>(&nbytes), sizeof(uint32_t));
    out.write(reinterpret_cast<const char*>(data), nbytes);
    out.write(reinterpret_cast<const char*>(&nbytes), sizeof(uint32_t));
}

// ------------------------------------------------------------
// Parameters
// ------------------------------------------------------------
constexpr double BoxSize = 10.0;

constexpr int fac = 1;

// Background
constexpr uint64_t Nbg = 100e3*fac;

// Halo #1
constexpr uint64_t Nhalo1      = 20e3*fac;
constexpr double   HaloRadius1 = 0.5;
constexpr double   Center1X    = 2.0;
constexpr double   Center1Y    = 2.0;
constexpr double   Center1Z    = 2.0;

// Halo #2
constexpr uint64_t Nhalo2      = 5e3*fac;
constexpr double   HaloRadius2 = 0.2;

constexpr double   Center2X    = 7.0;
constexpr double   Center2Y    = 7.0;
constexpr double   Center2Z    = 7.0;

constexpr uint64_t Ntot =
    Nbg + Nhalo1 + Nhalo2;

// ------------------------------------------------------------
// Parallel output parameters
// ------------------------------------------------------------
constexpr int NumBlocks = 8;
constexpr int SnapshotNum = 0;
constexpr const char* BaseName = "my_sim";

// ------------------------------------------------------------
// RNG helper
// ------------------------------------------------------------
static inline double U01(std::mt19937& rng)
{
    return std::generate_canonical<double,64>(rng);
}

// ------------------------------------------------------------
// Uniform sphere sampler
// p(r)=3r²/R³
// r=R*u^(1/3)
// ------------------------------------------------------------
static inline double sample_r(double radius,
                              std::mt19937& rng)
{
    return radius * std::cbrt(U01(rng));
}

// ------------------------------------------------------------
// Isotropic direction
// ------------------------------------------------------------
static inline void sample_dir(double& x,
                              double& y,
                              double& z,
                              std::mt19937& rng)
{
    double u = U01(rng);
    double v = U01(rng);

    double costheta = 2.0*u - 1.0;
    double sintheta =
        std::sqrt(1.0 - costheta*costheta);

    double phi = 2.0 * M_PI * v;

    x = sintheta * std::cos(phi);
    y = sintheta * std::sin(phi);
    z = costheta;
}

// ------------------------------------------------------------
// Main
// ------------------------------------------------------------
int main()
{
    std::vector<float> pos(3 * Ntot);
    std::vector<float> vel(3 * Ntot);
    std::vector<int64_t> ids(Ntot);

    std::mt19937 rng(42);

    uint64_t p = 0;

    // ========================================================
    // Background particles
    // ========================================================
    for (; p < Nbg; ++p)
    {
        pos[3*p+0] =
            static_cast<float>(BoxSize * U01(rng));

        pos[3*p+1] =
            static_cast<float>(BoxSize * U01(rng));

        pos[3*p+2] =
            static_cast<float>(BoxSize * U01(rng));

        vel[3*p+0] = 0.0f;
        vel[3*p+1] = 0.0f;
        vel[3*p+2] = 0.0f;

        ids[p] = static_cast<int64_t>(p + 1);
    }

    // ========================================================
    // Halo #1
    // ========================================================
    for (uint64_t i = 0; i < Nhalo1; ++i, ++p)
    {
        double r =
            sample_r(HaloRadius1, rng);

        double dx, dy, dz;
        sample_dir(dx, dy, dz, rng);

        pos[3*p+0] =
            static_cast<float>(Center1X + r*dx);

        pos[3*p+1] =
            static_cast<float>(Center1Y + r*dy);

        pos[3*p+2] =
            static_cast<float>(Center1Z + r*dz);

        vel[3*p+0] = 0.0f;
        vel[3*p+1] = 0.0f;
        vel[3*p+2] = 0.0f;

        ids[p] = static_cast<int64_t>(p + 1);
    }

    // ========================================================
    // Halo #2
    // ========================================================
    for (uint64_t i = 0; i < Nhalo2; ++i, ++p)
    {
        double r =
            sample_r(HaloRadius2, rng);

        double dx, dy, dz;
        sample_dir(dx, dy, dz, rng);

        pos[3*p+0] =
            static_cast<float>(Center2X + r*dx);

        pos[3*p+1] =
            static_cast<float>(Center2Y + r*dy);

        pos[3*p+2] =
            static_cast<float>(Center2Z + r*dz);

        vel[3*p+0] = 0.0f;
        vel[3*p+1] = 0.0f;
        vel[3*p+2] = 0.0f;

        ids[p] = static_cast<int64_t>(p + 1);
    }

    write_vtk_binary("snapshot_000.vtk", pos, vel, ids);

    // --------------------------------------------------------
    // Partition particles into 2x2x2 blocks
    std::array<std::vector<float>, NumBlocks> pos_blocks;
    std::array<std::vector<float>, NumBlocks> vel_blocks;
    std::array<std::vector<int64_t>, NumBlocks> id_blocks;

    for (uint64_t i = 0; i < Ntot; ++i)
    {
        float x = pos[3*i+0];
        float y = pos[3*i+1];
        float z = pos[3*i+2];

        int bx = (x < 0.5f * static_cast<float>(BoxSize)) ? 0 : 1;
        int by = (y < 0.5f * static_cast<float>(BoxSize)) ? 0 : 1;
        int bz = (z < 0.5f * static_cast<float>(BoxSize)) ? 0 : 1;

        int block = bx + 2*by + 4*bz;

        pos_blocks[block].push_back(x);
        pos_blocks[block].push_back(y);
        pos_blocks[block].push_back(z);

        vel_blocks[block].push_back(vel[3*i+0]);
        vel_blocks[block].push_back(vel[3*i+1]);
        vel_blocks[block].push_back(vel[3*i+2]);

        id_blocks[block].push_back(ids[i]);
    }

    // --------------------------------------------------------
    // Write snapshot blocks
    // --------------------------------------------------------
    for (int b = 0; b < NumBlocks; ++b)
    {
        uint64_t nblock = id_blocks[b].size();

        GadgetHeader hdr{};
        std::memset(&hdr, 0, sizeof(hdr));

        hdr.num_particles[1] =
            static_cast<uint32_t>(nblock);

        hdr.num_total_particles[1] =
            static_cast<uint32_t>(Ntot);

        hdr.num_total_particles_hw[1] = 0;

        // Constant particle mass
        hdr.particle_masses[1] = 1.0;

        hdr.scale_factor = 1.0;
        hdr.redshift     = 0.0;

        hdr.flag_sfr      = 0;
        hdr.flag_feedback = 0;
        hdr.flag_cooling  = 0;

        hdr.num_files_per_snapshot = NumBlocks;

        hdr.box_size      = BoxSize;
        hdr.omega_0       = 0.3;
        hdr.omega_lambda  = 0.7;
        hdr.h_0           = 0.7;

        hdr.flag_stellarage   = 0;
        hdr.flag_metals       = 0;
        hdr.flag_entropy_ics  = 0;

        std::ostringstream filename;
        std::filesystem::create_directories("GadgetFilesForSnapshot");
        filename << "GadgetFilesForSnapshot/" << BaseName << "." << std::setw(3) << std::setfill('0') << SnapshotNum << "." << b;

        std::ofstream out(filename.str(), std::ios::binary);

        if (!out)
        {
            std::cerr << "Failed to open output file "
                      << filename.str() << "\n";
            return 1;
        }

        write_block(out,
                    &hdr,
                    sizeof(GadgetHeader));

        write_block(out,
                    pos_blocks[b].data(),
                    static_cast<uint32_t>(
                        pos_blocks[b].size()*sizeof(float)));

        write_block(out,
                    vel_blocks[b].data(),
                    static_cast<uint32_t>(
                        vel_blocks[b].size()*sizeof(float)));

        write_block(out,
                    id_blocks[b].data(),
                    static_cast<uint64_t>(
                        id_blocks[b].size()*sizeof(int64_t)));

        out.close();
    }

    std::cout
        << "Successfully generated "
        << NumBlocks << " block files named "
        << BaseName << "." << SnapshotNum
        << ".<block>\n";

    std::cout
        << "Background particles : "
        << Nbg << "\n";

    std::cout
        << "Halo #1 particles    : "
        << Nhalo1 << "\n";

    std::cout
        << "Halo #2 particles    : "
        << Nhalo2 << "\n";

    std::cout
        << "Total particles      : "
        << Ntot << "\n";

    return 0;
}
