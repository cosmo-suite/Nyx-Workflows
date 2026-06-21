#include <cassert>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <fstream>
#include <iostream>
#include <random>
#include <vector>

#define GADGET_HEADER_SIZE 256

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
constexpr double BoxSize   = 10.0;
constexpr double HaloRadius = 0.2;
constexpr double Center     = BoxSize / 2.0;

constexpr uint64_t Nhalo = 2000;

// ------------------------------------------------------------
// RNG helper
// ------------------------------------------------------------
static inline double U01(std::mt19937& rng)
{
    return std::generate_canonical<double,64>(rng);
}

// ------------------------------------------------------------
// Sample radius for a uniform-density sphere
//
// p(r) = 3 r^2 / R^3
//
// Therefore:
// r = R * u^(1/3)
// ------------------------------------------------------------
static inline double sample_r(std::mt19937& rng)
{
    return HaloRadius * std::cbrt(U01(rng));
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
    double sintheta = std::sqrt(1.0 - costheta*costheta);

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
    std::vector<float> pos(3 * Nhalo);
    std::vector<float> vel(3 * Nhalo);

    // Rockstar is happy with 32-bit IDs for small tests
    std::vector<int32_t> ids(Nhalo);

    std::mt19937 rng(42);

    // --------------------------------------------------------
    // Generate isolated halo
    // --------------------------------------------------------
    for (uint64_t i = 0; i < Nhalo; ++i)
    {
        double r = sample_r(rng);

        double dx, dy, dz;
        sample_dir(dx, dy, dz, rng);

        pos[3*i+0] = static_cast<float>(Center + r*dx);
        pos[3*i+1] = static_cast<float>(Center + r*dy);
        pos[3*i+2] = static_cast<float>(Center + r*dz);

        // Perfectly cold halo
        vel[3*i+0] = 0.0f;
        vel[3*i+1] = 0.0f;
        vel[3*i+2] = 0.0f;

        ids[i] = static_cast<int32_t>(i + 1);
    }

    // --------------------------------------------------------
    // Header
    // --------------------------------------------------------
    GadgetHeader hdr{};
    std::memset(&hdr, 0, sizeof(hdr));

    hdr.num_particles[1] = static_cast<uint32_t>(Nhalo);
    hdr.num_total_particles[1] = static_cast<uint32_t>(Nhalo);
    hdr.num_total_particles_hw[1] = 0;

    // Constant mass per DM particle stored in header
    hdr.particle_masses[1] = 1.0;

    hdr.scale_factor = 1.0;
    hdr.redshift     = 0.0;

    hdr.flag_sfr      = 0;
    hdr.flag_feedback = 0;
    hdr.flag_cooling  = 0;

    hdr.num_files_per_snapshot = 1;

    hdr.box_size      = BoxSize;
    hdr.omega_0       = 0.3;
    hdr.omega_lambda  = 0.7;
    hdr.h_0           = 0.7;

    hdr.flag_stellarage = 0;
    hdr.flag_metals     = 0;
    hdr.flag_entropy_ics = 0;

    // --------------------------------------------------------
    // Write snapshot
    // --------------------------------------------------------
    std::ofstream out("isolated_halo.0", std::ios::binary);

    if (!out)
    {
        std::cerr << "Failed to open output file\n";
        return 1;
    }

    write_block(out, &hdr, sizeof(GadgetHeader));

    write_block(out,
                pos.data(),
                static_cast<uint32_t>(pos.size() * sizeof(float)));

    write_block(out,
                vel.data(),
                static_cast<uint32_t>(vel.size() * sizeof(float)));

    write_block(out,
                ids.data(),
                static_cast<uint32_t>(ids.size() * sizeof(int32_t)));

    out.close();

    std::cout << "Successfully generated isolated_halo.0\n";
    std::cout << "Particles : " << Nhalo << "\n";
    std::cout << "Box Size  : " << BoxSize << "\n";
    std::cout << "Radius    : " << HaloRadius << "\n";

    return 0;
}
