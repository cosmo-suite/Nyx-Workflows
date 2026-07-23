#include <cassert>
#include <cstdint>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <algorithm>


namespace fs = std::filesystem;


// ------------------------------------------------------------
// Gadget header
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

static_assert(sizeof(GadgetHeader) == 256);

// ------------------------------------------------------------
// Particle structure
// ------------------------------------------------------------
struct Particle
{
    int64_t id;
    float x;
    float y;
    float z;
};

// ------------------------------------------------------------
// Read one Gadget block
// ------------------------------------------------------------
template <typename T>
void read_block(std::ifstream& in,
                std::vector<T>& data)
{
    uint32_t block_size_begin;

    in.read(reinterpret_cast<char*>(&block_size_begin),
            sizeof(uint32_t));


    size_t n = block_size_begin / sizeof(T);

    data.resize(n);

    in.read(reinterpret_cast<char*>(data.data()),
            block_size_begin);


    uint32_t block_size_end;

    in.read(reinterpret_cast<char*>(&block_size_end),
            sizeof(uint32_t));


    if (block_size_begin != block_size_end)
    {
        throw std::runtime_error(
            "Gadget block size mismatch");
    }
}

std::vector<Particle>
read_all_gadget_files_data(const std::string prefix);
