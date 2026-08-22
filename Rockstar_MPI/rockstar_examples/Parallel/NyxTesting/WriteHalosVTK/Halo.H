#ifndef HALO_H
#define HALO_H

#include <cassert>
#include <cstdint>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <unordered_map>

#include "Particle.H"

namespace fs = std::filesystem;

// ------------------------------------------------------------
// Rockstar halo structure
// ------------------------------------------------------------

struct Halo
{
    int64_t id;

    float pos[6];
    float corevel[3];
    float bulkvel[3];

    float m;
    float r;
    float child_r;
    float vmax_r;
    float mgrav;
    float vmax;
    float rvmax;
    float rs;
    float klypin_rs;
    float vrms;

    float J[3];

    float energy;
    float spin;

    float alt_m[4];

    float Xoff;
    float Voff;

    float b_to_a;
    float c_to_a;

    float A[3];

    float b_to_a2;
    float c_to_a2;

    float A2[3];

    float bullock_spin;
    float kin_to_pot;
    float m_pe_b;
    float m_pe_d;

    float halfmass_radius;

    int64_t num_p;
    int64_t num_child_particles;
    int64_t p_start;
    int64_t desc;
    int64_t flags;
    int64_t n_core;

    float min_pos_err;
    float min_vel_err;
    float min_bulkvel_err;

    // add_flag & 1
    float chi2;

    // add_flag & 4
    float inertia_tensor[6];
    float inertia_tensor2[6];
};

std::unordered_map<int64_t, size_t>
build_particle_id_lookup(
    const std::vector<Particle>& particles);


static const uint64_t ROCKSTAR_MAGIC = 0xfadedacec0c0d0d0ULL;
static const int64_t PARTICLE_TYPE_IDS = 1;
static const int BINARY_HEADER_SIZE = 256;
static const int VERSION_MAX_SIZE = 12;

struct BinaryHeader
{
    uint64_t magic;

    int64_t snap;
    int64_t chunk;

    float scale;
    float Om;
    float Ol;
    float h0;

    float bounds[6];

    int64_t num_halos;
    int64_t num_particles;

    float box_size;
    float particle_mass;

    int64_t particle_type;

    int32_t format_revision;

    char rockstar_version[VERSION_MAX_SIZE];

    int32_t add_flag;

    char unused[
        BINARY_HEADER_SIZE
        - (sizeof(char) * VERSION_MAX_SIZE)
        - (sizeof(float) * 12)
        - (sizeof(int32_t) * 2)
        - (sizeof(int64_t) * 6)];
};

// ------------------------------------------------------------
// Read exact number of objects
// ------------------------------------------------------------
template <typename T>
void read_exact(std::ifstream& in,
                T* data,
                size_t count)
{
    in.read(reinterpret_cast<char*>(data),
            sizeof(T) * count);

    if (!in)
    {
        std::cerr
            << "Error reading "
            << count
            << " items.\n";

        std::exit(1);
    }
}

struct HaloCatalog
{
    std::vector<Halo> halos;
    std::vector<int64_t> part_ids;
};

HaloCatalog
read_all_halos_in_bin_file(
    const std::string& halo_bin_file);
#endif
