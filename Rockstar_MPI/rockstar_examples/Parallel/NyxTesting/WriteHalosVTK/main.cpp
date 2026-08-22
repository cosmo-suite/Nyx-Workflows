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
#include "Halo.H"
#include "IO.H"

namespace fs = std::filesystem;

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

    int64_t desc_snapshot = -1;

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
        else if (arg.rfind("--desc_snapshot=", 0) == 0)
        {
            desc_snapshot =
                std::stoll(
                arg.substr(
                std::string("--desc_snapshot=").size()));
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
    gadget_files_dir.empty() ||
    desc_snapshot < 0)
    {
        std::cerr
        << "Usage:\n"
        << argv[0]
        << " --halo-bin-file=<halo.bin>"
        << " --gadget-files-dir=<gadget_directory>"
        << " --desc_snapshot=<id>\n";

        return 1;
    }

    // --------------------------------------------------------
    // Read Rockstar halo catalog
    // --------------------------------------------------------

    HaloCatalog catalog =
        read_all_halos_in_bin_file(
            halo_bin_file);

    std::vector<Halo> halos =
        std::move(catalog.halos);

    std::vector<int64_t> part_ids =
        std::move(catalog.part_ids);

    // --------------------------------------------------------
    // Read Gadget particle data
    // --------------------------------------------------------

    std::vector<Particle> particles =
    read_gadget_particles(
        gadget_files_dir,
        desc_snapshot);

    // --------------------------------------------------------
    // Build particle ID lookup
    // --------------------------------------------------------

    std::unordered_map<int64_t, size_t>
        id_to_index =
            build_particle_id_lookup(particles);

    std::cout
        << "Built particle lookup table.\n";

    for (const Halo& halo : halos)
    {
        write_halo_vtk(
            halo,
            part_ids,
            id_to_index,
            particles);
    }
    std::cout
        << "\nDone.\n";

    return 0;
}
