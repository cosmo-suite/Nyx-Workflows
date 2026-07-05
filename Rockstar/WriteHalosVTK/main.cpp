#include <algorithm>
#include <cstdint>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>

namespace fs = std::filesystem;

static const uint64_t ROCKSTAR_MAGIC = 0xfadedacec0c0d0d0ULL;
static const int64_t PARTICLE_TYPE_IDS  = 1;
static const int64_t PARTICLE_TYPE_FULL = 2;
static const int BINARY_HEADER_SIZE = 256;
static const int VERSION_MAX_SIZE = 12;

struct BinaryHeader {
    uint64_t magic;
    int64_t snap, chunk;
    float scale, Om, Ol, h0;
    float bounds[6];
    int64_t num_halos, num_particles;
    float box_size, particle_mass;
    int64_t particle_type;
    int32_t format_revision;
    char rockstar_version[VERSION_MAX_SIZE];
    char unused[BINARY_HEADER_SIZE
                - (sizeof(char) * VERSION_MAX_SIZE)
                - (sizeof(float) * 12)
                - sizeof(int32_t)
                - (sizeof(int64_t) * 6)];
};

struct Halo {
    int64_t id;
    float pos[6], corevel[3], bulkvel[3];
    float m, r, child_r, vmax_r, mgrav, vmax, rvmax, rs, klypin_rs, vrms,
          J[3], energy, spin, alt_m[4], Xoff, Voff, b_to_a, c_to_a, A[3],
          b_to_a2, c_to_a2, A2[3],
          bullock_spin, kin_to_pot, m_pe_b, m_pe_d;
    int64_t num_p, num_child_particles, p_start, desc, flags, n_core;
    float min_pos_err, min_vel_err, min_bulkvel_err;
    int32_t type;
    float sm, gas, bh, peak_density, av_density;
};

struct Particle {
    int64_t id;
    float pos[6];
    float mass, energy;
    float softening;
    float metallicity;
    int32_t type;
};

template <typename T>
void read_exact(std::ifstream &in, T *data, size_t count)
{
    in.read(reinterpret_cast<char *>(data), sizeof(T) * count);

    if (!in) {
        std::cerr << "Error reading " << count << " items.\n";
        std::exit(1);
    }
}

void write_vtk(const std::string &path,
               const std::vector<Particle> &particles,
               const std::vector<size_t> &indices,
               int64_t halo_id)
{
    std::ofstream out(path);

    if (!out) {
        std::cerr << "Cannot open " << path << " for writing.\n";
        std::exit(1);
    }

    size_t n = indices.size();

    out << "# vtk DataFile Version 3.0\n";
    out << "halo " << halo_id << "\n";
    out << "ASCII\n";
    out << "DATASET POLYDATA\n";

    out << "POINTS " << n << " float\n";
    for (size_t idx : indices) {
        const Particle &p = particles[idx];
        out << p.pos[0] << " "
            << p.pos[1] << " "
            << p.pos[2] << "\n";
    }

    out << "VERTICES " << n << " " << 2 * n << "\n";
    for (size_t i = 0; i < n; i++) {
        out << "1 " << i << "\n";
    }

    out << "POINT_DATA " << n << "\n";

    out << "SCALARS id double 1\n";
    out << "LOOKUP_TABLE default\n";
    for (size_t idx : indices)
        out << static_cast<double>(particles[idx].id) << "\n";

    out << "SCALARS mass float 1\n";
    out << "LOOKUP_TABLE default\n";
    for (size_t idx : indices)
        out << particles[idx].mass << "\n";
}

int main(int argc, char **argv)
{
    if (argc != 3) {
        std::cerr << "Usage:\n"
                  << argv[0]
                  << " <rbin-directory> <halo-bin-file>\n";
        return 1;
    }

    std::string rbin_dir = argv[1];
    std::string bin_path = argv[2];

    //------------------------------------------------------------
    // Read halo .bin file
    //------------------------------------------------------------

    std::ifstream bin(bin_path, std::ios::binary);

    if (!bin) {
        std::cerr << "Cannot open " << bin_path << "\n";
        return 1;
    }

    BinaryHeader bh{};
    read_exact(bin, &bh, 1);

    if (bh.magic != ROCKSTAR_MAGIC) {
        std::cerr << "Invalid halo .bin file.\n";
        return 1;
    }

    if (bh.particle_type != PARTICLE_TYPE_IDS) {
        std::cerr << "Halo file does not contain particle IDs.\n";
        return 1;
    }

    std::vector<Halo> halos(bh.num_halos);
    read_exact(bin, halos.data(), halos.size());

    std::vector<int64_t> part_ids(bh.num_particles);
    read_exact(bin, part_ids.data(), part_ids.size());

    //------------------------------------------------------------
    // Find all .rbin files
    //------------------------------------------------------------

    std::vector<fs::path> rbin_files;

    for (const auto &entry : fs::directory_iterator(rbin_dir)) {

        if (!entry.is_regular_file())
            continue;

        if (entry.path().extension() == ".rbin")
            rbin_files.push_back(entry.path());
    }

    std::sort(rbin_files.begin(), rbin_files.end());

    if (rbin_files.empty()) {
        std::cerr << "No .rbin files found in "
                  << rbin_dir << "\n";
        return 1;
    }

    std::cout << "Found "
              << rbin_files.size()
              << " .rbin files.\n";

    //------------------------------------------------------------
    // Read all particles
    //------------------------------------------------------------

    std::vector<Particle> particles;

    for (const auto &file : rbin_files) {

        std::cout << "Reading "
                  << file.filename().string()
                  << std::endl;

        std::ifstream rbin(file, std::ios::binary);

        if (!rbin) {
            std::cerr << "Cannot open "
                      << file << "\n";
            return 1;
        }

        BinaryHeader rh{};
        read_exact(rbin, &rh, 1);

        if (rh.magic != ROCKSTAR_MAGIC) {
            std::cerr << "Magic mismatch in "
                      << file << "\n";
            return 1;
        }

        if (rh.particle_type != PARTICLE_TYPE_FULL) {
            std::cerr << file
                      << " is not a FULL particle file.\n";
            return 1;
        }

        size_t old_size = particles.size();

        particles.resize(old_size + rh.num_particles);

        read_exact(rbin,
                   particles.data() + old_size,
                   static_cast<size_t>(rh.num_particles));

        std::cout << "    Added "
                  << rh.num_particles
                  << " particles.\n";
    }

    std::cout << "\nTotal particles read = "
              << particles.size()
              << "\n";

    /*std::vector<size_t> all_indices(particles.size());
    for (size_t i = 0; i < particles.size(); i++) {
        all_indices[i] = i;
    }

    write_vtk("all_particles.vtk",
          particles,
          all_indices,
          -1);*/

    //------------------------------------------------------------
    // Build ID lookup
    //------------------------------------------------------------

    std::unordered_map<int64_t, size_t> id_to_index;
    id_to_index.reserve(particles.size() * 2);

    for (size_t i = 0; i < particles.size(); i++) {
        id_to_index[particles[i].id] = i;
    }

    std::cout << "Built particle lookup table.\n";

    //------------------------------------------------------------
    // Write one VTK per halo
    //------------------------------------------------------------

    for (const Halo &halo : halos) {

        if (halo.p_start < 0 || halo.num_p <= 0)
            continue;

        size_t start = static_cast<size_t>(halo.p_start);
        size_t count = static_cast<size_t>(halo.num_p);

        if (start + count > part_ids.size()) {
            std::cerr << "Halo "
                      << halo.id
                      << " has invalid particle range.\n";
            continue;
        }

        std::vector<size_t> indices;
        indices.reserve(count);

        size_t missing = 0;

        for (size_t j = 0; j < count; j++) {

            int64_t pid = part_ids[start + j];

            auto it = id_to_index.find(pid);

            if (it != id_to_index.end()) {
                indices.push_back(it->second);
            } else {
                missing++;
            }
        }

        if (missing > 0) {
            std::cout << "Halo "
                      << halo.id
                      << ": missing "
                      << missing
                      << " particles.\n";
        }

        std::string vtk_name =
            "halo_" + std::to_string(halo.id) + ".vtk";

        write_vtk(vtk_name,
                  particles,
                  indices,
                  halo.id);

        std::cout << "Wrote "
                  << vtk_name
                  << " ("
                  << indices.size()
                  << " particles)\n";
    }

    std::cout << "\nDone.\n";

    return 0;
}
