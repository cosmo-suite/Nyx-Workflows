#include <cassert>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <fstream>
#include <iostream>
#include <vector>
#include <random>

#define GADGET_HEADER_SIZE 256

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

// Verify every field lands at the exact byte the reader expects
static_assert(sizeof(GadgetHeader)                                  == 256, "");
static_assert(offsetof(GadgetHeader, num_particles)                 ==   0, "");
static_assert(offsetof(GadgetHeader, particle_masses)               ==  24, "");
static_assert(offsetof(GadgetHeader, scale_factor)                  ==  72, "");
static_assert(offsetof(GadgetHeader, redshift)                      ==  80, "");
static_assert(offsetof(GadgetHeader, flag_sfr)                      ==  88, "");
static_assert(offsetof(GadgetHeader, flag_feedback)                 ==  92, "");
static_assert(offsetof(GadgetHeader, num_total_particles)           ==  96, "");
static_assert(offsetof(GadgetHeader, flag_cooling)                  == 120, "");
static_assert(offsetof(GadgetHeader, num_files_per_snapshot)        == 124, "");
static_assert(offsetof(GadgetHeader, box_size)                      == 128, "");
static_assert(offsetof(GadgetHeader, omega_0)                       == 136, "");
static_assert(offsetof(GadgetHeader, omega_lambda)                  == 144, "");
static_assert(offsetof(GadgetHeader, h_0)                           == 152, "");
static_assert(offsetof(GadgetHeader, flag_stellarage)               == 160, "");
static_assert(offsetof(GadgetHeader, flag_metals)                   == 164, "");
static_assert(offsetof(GadgetHeader, num_total_particles_hw)        == 168, "");
static_assert(offsetof(GadgetHeader, flag_entropy_ics)              == 192, "");
static_assert(offsetof(GadgetHeader, unused)                        == 196, "");

// Fortran-style record: [4-byte size][data][4-byte size]
void write_block(std::ofstream& out,
                 const void*   data,
                 uint32_t      nbytes)
{
    out.write(reinterpret_cast<const char*>(&nbytes), sizeof(uint32_t));
    out.write(reinterpret_cast<const char*>(data),    nbytes);
    out.write(reinterpret_cast<const char*>(&nbytes), sizeof(uint32_t));
}

int main()
{
    //--------------------------------------------------
    // Problem setup
    //--------------------------------------------------

    constexpr int Nbg   = 64;

// Number of particles in overdensity
constexpr int Nhalo = 500000;

constexpr double BoxSize = 20.0;
constexpr double HaloRad = 0.5;

    constexpr double OmegaM = 0.30;
    constexpr double h      = 0.70;
    constexpr double zinit  = 0.0;

    const uint64_t N_background =
    uint64_t(Nbg) * Nbg * Nbg;

const uint64_t N_total =
    N_background + Nhalo;

    std::cout << "N_background = " << N_background << "\n"
              << "N_halo       = " << Nhalo       << "\n"
              << "N_total      = " << N_total      << "\n";

    //--------------------------------------------------
    // Particle storage
    //--------------------------------------------------

    std::vector<float>    pos(3 * N_total);
    std::vector<float>    vel(3 * N_total, 0.0f);
    // Reader detects ID width from block size.
    // Use uint64_t so GADGET_ID_BYTES is set to 8 by the reader.
    std::vector<uint64_t> ids(N_total);
    // Per-particle masses (needed because background and halo have different masses)
    std::vector<float>    masses(N_total);

    //--------------------------------------------------
    // Background particles
    //--------------------------------------------------

    uint64_t p = 0;
    const double dx = BoxSize / double(Nbg);

    // Reader uses: PARTICLE_MASS = Om * CRITICAL_DENSITY * BOX_SIZE^3 / N_total
    // For the writer we compute mass consistently with how the reader will
    // interpret it. Since we have a mass block, reader will use per-particle
    // values directly (after GADGET_MASS_CONVERSION scaling, typically 1e10 Msun/h).
    // Here masses are stored as the raw Gadget unit values.

    const double vol_bg =
    std::pow(BoxSize / double(Nbg), 3);

const double rho_mean =
    27.75197 * OmegaM;

const float m_part =
    float(rho_mean * vol_bg);

    for (int k = 0; k < Nbg; ++k)
    for (int j = 0; j < Nbg; ++j)
    for (int i = 0; i < Nbg; ++i)
    {
        pos[3*p+0] = float((i + 0.5) * dx);
        pos[3*p+1] = float((j + 0.5) * dx);
        pos[3*p+2] = float((k + 0.5) * dx);
        masses[p] = m_part;
        ids[p]     = p + 1;
        ++p;
    }

   //--------------------------------------------------
// Spherical overdensity
//--------------------------------------------------

std::mt19937 rng(1234);

const double center = 0.5 * BoxSize;

std::uniform_real_distribution<float>
    uni(center - HaloRad,
        center + HaloRad);

uint64_t halo_count = 0;

while (halo_count < Nhalo)
{
    float x = uni(rng);
    float y = uni(rng);
    float z = uni(rng);

    float dxh = x - center;
    float dyh = y - center;
    float dzh = z - center;

    float r =
        std::sqrt(dxh*dxh +
                  dyh*dyh +
                  dzh*dzh);

    if (r > HaloRad)
        continue;

    pos[3*p+0] = x;
    pos[3*p+1] = y;
    pos[3*p+2] = z;

    masses[p] = m_part;

    ids[p] = p + 1;

    ++p;
    ++halo_count;
}
 
    assert(p == N_total);

    // Give particles random velocities
std::mt19937 gen(1234);
std::normal_distribution<float> gauss(0.0f, 10.0f);

for (uint64_t p = 0; p < N_total; ++p)
{
    vel[3*p+0] = gauss(gen);
    vel[3*p+1] = gauss(gen);
    vel[3*p+2] = gauss(gen);
}

    std::cout << "\n=== Snapshot Summary ===\n";
std::cout << "Background particles = "
          << N_background << "\n";

std::cout << "Halo particles = "
          << Nhalo << "\n";

std::cout << "Total particles = "
          << N_total << "\n";

std::cout << "Particle mass = "
          << m_part << "\n";

double xmin=1e30, xmax=-1e30;
double ymin=1e30, ymax=-1e30;
double zmin=1e30, zmax=-1e30;

for (uint64_t i=0; i<N_total; i++)
{
    xmin = std::min(xmin,
                    double(pos[3*i+0]));
    xmax = std::max(xmax,
                    double(pos[3*i+0]));

    ymin = std::min(ymin,
                    double(pos[3*i+1]));
    ymax = std::max(ymax,
                    double(pos[3*i+1]));

    zmin = std::min(zmin,
                    double(pos[3*i+2]));
    zmax = std::max(zmax,
                    double(pos[3*i+2]));
}

std::cout << "\n=== Position Range ===\n";
std::cout << "X: "
          << xmin << " "
          << xmax << "\n";

std::cout << "Y: "
          << ymin << " "
          << ymax << "\n";

std::cout << "Z: "
          << zmin << " "
          << zmax << "\n";


uint64_t inside = 0;

for (uint64_t i=0; i<N_total; i++)
{
    double dxh =
        pos[3*i+0] - center;

    double dyh =
        pos[3*i+1] - center;

    double dzh =
        pos[3*i+2] - center;

    double r =
        std::sqrt(dxh*dxh +
                  dyh*dyh +
                  dzh*dzh);

    if (r < HaloRad)
        inside++;
}

std::cout
    << "\nParticles inside halo radius = "
    << inside
    << "\n";



double vmean = 0.0;

for (uint64_t i=0; i<N_total; i++)
{
    double vx = vel[3*i+0];
    double vy = vel[3*i+1];
    double vz = vel[3*i+2];

    vmean +=
        std::sqrt(vx*vx +
                  vy*vy +
                  vz*vz);
}

vmean /= double(N_total);

std::cout
    << "Mean particle speed = "
    << vmean
    << " km/s\n";


std::cout << "\n=== First 10 particles ===\n";

for (int i=0; i<10; i++)
{
    std::cout
        << i
        << " pos=("
        << pos[3*i+0] << ","
        << pos[3*i+1] << ","
        << pos[3*i+2] << ") "
        << "vel=("
        << vel[3*i+0] << ","
        << vel[3*i+1] << ","
        << vel[3*i+2] << ") "
        << "id="
        << ids[i]
        << "\n";
}






    //--------------------------------------------------
    // Build header
    // All particles are type 1 (dark matter / halo type).
    // Set particle_masses[1] = 0 to signal that a MASS block follows.
    //--------------------------------------------------

    GadgetHeader hdr{};
    std::memset(&hdr, 0, sizeof(hdr));

    hdr.num_particles[1]         = static_cast<uint32_t>(N_total);
    hdr.num_total_particles[1]   = static_cast<uint32_t>(N_total & 0xFFFFFFFF);
    // High word: must be int32_t to match reader struct
    hdr.num_total_particles_hw[1] = static_cast<int32_t>(N_total >> 32);

    // particle_masses[1] = 0 tells the reader a per-particle MASS block exists
    hdr.particle_masses[1] = 0.0;

    hdr.scale_factor = 1.0 / (1.0 + zinit);
    hdr.redshift     = zinit;

    hdr.flag_sfr             = 0;
    hdr.flag_feedback        = 0;
    hdr.flag_cooling         = 0;
    hdr.num_files_per_snapshot = 1;

    hdr.box_size     = BoxSize;
    hdr.omega_0      = OmegaM;
    hdr.omega_lambda = 1.0 - OmegaM;
    hdr.h_0          = h;

    hdr.flag_stellarage      = 0;
    hdr.flag_metals          = 0;
    hdr.flag_entropy_ics     = 0;

    //--------------------------------------------------
    // Apply Gadget velocity convention: v_internal = v_peculiar / sqrt(a)
    // At z=0, sqrt(a)=1, so no change; kept for correctness at other redshifts.
    //--------------------------------------------------

    const float inv_sqrt_a = float(std::sqrt(1.0 + zinit)); // = 1/sqrt(a)
    for (size_t i = 0; i < vel.size(); ++i)
        vel[i] *= inv_sqrt_a;

    //--------------------------------------------------
    // Write snapshot
    //--------------------------------------------------

    std::ofstream out("snapshot_000", std::ios::binary);
    if (!out) {
        std::cerr << "Cannot open snapshot_000\n";
        return 1;
    }

    // HEAD block
    write_block(out, &hdr,
                static_cast<uint32_t>(sizeof(GadgetHeader)));

    // POS block
    write_block(out, pos.data(),
                static_cast<uint32_t>(pos.size() * sizeof(float)));

    // VEL block
    write_block(out, vel.data(),
                static_cast<uint32_t>(vel.size() * sizeof(float)));

    // ID block (8-byte; reader will set GADGET_ID_BYTES=8 from block size check)
    write_block(out, ids.data(),
                static_cast<uint32_t>(ids.size() * sizeof(uint64_t)));

    // MASS block (because particle_masses[1]==0 in header)
    write_block(out, masses.data(),
                static_cast<uint32_t>(masses.size() * sizeof(float)));

    out.close();

    std::cout << "Wrote snapshot_000\n"
              << "Header size = " << sizeof(GadgetHeader) << " bytes\n";


    std::cout << "\n=== Header Values ===\n";

std::cout << "npart[1] = "
          << hdr.num_particles[1]
          << "\n";

std::cout << "box_size = "
          << hdr.box_size
          << "\n";

std::cout << "OmegaM = "
          << hdr.omega_0
          << "\n";

std::cout << "OmegaL = "
          << hdr.omega_lambda
          << "\n";

std::cout << "h = "
          << hdr.h_0
          << "\n";

std::cout << "a = "
          << hdr.scale_factor
          << "\n";

std::cout << "particle_mass = "
          << m_part
          << "\n";


    return 0;
}

