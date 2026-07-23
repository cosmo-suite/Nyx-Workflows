#include <algorithm>
#include <cstdint>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>

#include "GadgetFileReader.H"

namespace fs = std::filesystem;

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

    float halfmass_radius;   // <-- MISSING FIELD

    int64_t num_p;
    int64_t num_child_particles;
    int64_t p_start;
    int64_t desc;
    int64_t flags;
    int64_t n_core;

    float min_pos_err;
    float min_vel_err;
    float min_bulkvel_err;
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

// ------------------------------------------------------------
// Write halo particles to VTK
// ------------------------------------------------------------
void write_vtk(const std::string& filename,
               const std::vector<Particle>& particles,
               const std::vector<size_t>& indices,
               int64_t halo_id)
{
    std::ofstream out(filename);

    if (!out)
    {
        std::cerr
            << "Cannot open "
            << filename
            << " for writing.\n";

        std::exit(1);
    }

    const size_t n = indices.size();

    out << "# vtk DataFile Version 3.0\n";
    out << "halo " << halo_id << "\n";
    out << "ASCII\n";
    out << "DATASET POLYDATA\n";

    out << "POINTS "
        << n
        << " float\n";

    for (size_t idx : indices)
    {
        const Particle& p = particles[idx];

        out << p.x << " "
            << p.y << " "
            << p.z << "\n";
    }

    out << "VERTICES "
        << n
        << " "
        << 2 * n
        << "\n";

    for (size_t i = 0; i < n; i++)
    {
        out << "1 "
            << i
            << "\n";
    }

    out << "POINT_DATA "
        << n
        << "\n";

    out << "SCALARS id long_long 1\n";
    out << "LOOKUP_TABLE default\n";

    for (size_t idx : indices)
    {
        out << particles[idx].id
            << "\n";
    }
}

// ------------------------------------------------------------
// Main
// ------------------------------------------------------------
int main(int argc, char** argv)
{
    std::string halo_bin_file;
    std::string gadget_files_dir;

    // --------------------------------------------------------
    // Parse command line
    // --------------------------------------------------------

    for (int i = 1; i < argc; i++)
    {
        std::string arg = argv[i];

        const std::string halo_prefix =
            "--halo-bin-file=";

        const std::string gadget_prefix =
            "--gadget-files-dir=";

        if (arg.rfind(halo_prefix, 0) == 0)
        {
            halo_bin_file =
                arg.substr(halo_prefix.size());
        }
        else if (arg.rfind(gadget_prefix, 0) == 0)
        {
            gadget_files_dir =
                arg.substr(gadget_prefix.size());
        }
        else
        {
            std::cerr
                << "Unknown argument: "
                << arg
                << "\n";

            return 1;
        }
    }

    if (halo_bin_file.empty() ||
        gadget_files_dir.empty())
    {
        std::cerr
            << "Usage:\n"
            << argv[0]
            << " --halo-bin-file=<halo.bin>"
            << " --gadget-files-dir=<gadget_directory>\n";

        return 1;
    }

    // --------------------------------------------------------
    // Read Rockstar halo catalog
    // --------------------------------------------------------

    std::ifstream bin(halo_bin_file,
                      std::ios::binary);

    if (!bin)
    {
        std::cerr
            << "Cannot open "
            << halo_bin_file
            << "\n";

        return 1;
    }

    BinaryHeader bh{};

    read_exact(bin,
               &bh,
               1);

    if (bh.magic != ROCKSTAR_MAGIC)
    {
        std::cerr
            << "Invalid halo .bin file.\n";

        return 1;
    }

    if (bh.particle_type != PARTICLE_TYPE_IDS)
    {
        std::cerr
            << "Halo file does not contain particle IDs.\n";

        return 1;
    }

    std::vector<Halo> halos(
        static_cast<size_t>(bh.num_halos));

    read_exact(bin,
               halos.data(),
               halos.size());

    std::vector<int64_t> part_ids(
        static_cast<size_t>(bh.num_particles));

    read_exact(bin,
               part_ids.data(),
               part_ids.size());

    std::cout
        << "Read "
        << halos.size()
        << " halos and "
        << part_ids.size()
        << " particle IDs from Rockstar catalog.\n";

            std::cout << "First 20 particle IDs:\n";

for (size_t i = 0;
     i < std::min<size_t>(20, part_ids.size());
     i++)
{
    std::cout
        << i
        << " : "
        << part_ids[i]
        << "\n";
}

    // --------------------------------------------------------
    // Read Gadget particle data
    // --------------------------------------------------------

    std::vector<Particle> particles =
        read_all_gadget_files_data(
            gadget_files_dir);

    std::cout
        << "Loaded "
        << particles.size()
        << " Gadget particles.\n";

    // --------------------------------------------------------
    // Build particle ID lookup
    // --------------------------------------------------------

    std::unordered_map<int64_t, size_t>
        id_to_index;

    id_to_index.reserve(
        particles.size() * 2);

    for (size_t i = 0;
         i < particles.size();
         i++)
    {
        id_to_index[
            particles[i].id] = i;
    }

    std::cout
        << "Built particle lookup table.\n";

    // --------------------------------------------------------
    // Write one VTK per halo
    // --------------------------------------------------------

    for (const Halo& halo : halos)
    {
        if (halo.p_start < 0 ||
            halo.num_p <= 0)
        {
            continue;
        }

        const size_t start =
            static_cast<size_t>(
                halo.p_start);

        const size_t count =
            static_cast<size_t>(
                halo.num_p);

        if (start + count >
            part_ids.size())
        {
            std::cerr
                << "Halo "
                << halo.id
                << " has invalid particle range.\n";

            continue;
        }

        std::vector<size_t> indices;

        indices.reserve(count);



        size_t missing = 0;

        for (size_t j = 0;
             j < count;
             j++)
        {
            const int64_t pid =
                part_ids[start + j];

            auto it =
                id_to_index.find(pid);

            if (it != id_to_index.end())
            {
                indices.push_back(
                    it->second);
            }
            else
            {
                missing++;
            }
        }

        if (missing > 0)
        {
            std::cout
                << "Halo "
                << halo.id
                << ": missing "
                << missing
                << " particles.\n";
        }


        std::vector<int64_t> missing_ids;

for (size_t j = 0;
     j < count;
     j++)
{
    const int64_t pid =
        part_ids[start + j];

    auto it =
        id_to_index.find(pid);

    if (it != id_to_index.end())
    {
        indices.push_back(
            it->second);
    }
    else
    {
        missing_ids.push_back(pid);
    }
}

if (!missing_ids.empty())
{
    std::cout
        << "Halo "
        << halo.id
        << ": missing "
        << missing_ids.size()
        << " particles\n";

    for (const auto& pid : missing_ids)
    {
        std::cout
            << "    Missing particle ID = "
            << pid
            << "\n";
    }
}

        std::string vtk_name =
            "halo_" +
            std::to_string(halo.id) +
            ".vtk";

        write_vtk(vtk_name,
                  particles,
                  indices,
                  halo.id);

        std::cout
            << "Wrote "
            << vtk_name
            << " ("
            << indices.size()
            << " particles)\n";
    }

    std::cout
        << "\nDone.\n";

    return 0;
}
