#include <iostream>
#include <string>
#include <algorithm>

#include "VelociraptorOutputReader.H"
#include "GadgetFileReader.H"

int main(int argc, char* argv[])
{
    std::string prop_filename;
    std::string group_filename;
    std::string part_filename;
    std::string gadget_files_dir;

    // --------------------------------------------------------
    // Parse command line arguments
    // --------------------------------------------------------
    for (int i = 1; i < argc; ++i)
    {
        std::string arg = argv[i];

        if (arg.rfind("--properties=", 0) == 0)
        {
            prop_filename =
                arg.substr(std::string("--properties=").size());
        }
        else if (arg.rfind("--catalog-groups=", 0) == 0)
        {
            group_filename =
                arg.substr(std::string("--catalog-groups=").size());
        }
        else if (arg.rfind("--catalog-particles=", 0) == 0)
        {
            part_filename =
                arg.substr(std::string("--catalog-particles=").size());
        }
        else if (arg.rfind("--gadget-files-dir=", 0) == 0)
        {
            gadget_files_dir =
                arg.substr(std::string("--gadget-files-dir=").size());
        }
        else
        {
            std::cerr
                << "Unknown argument: "
                << arg
                << "\n";

            return EXIT_FAILURE;
        }
    }

    // --------------------------------------------------------
    // Validate arguments
    // --------------------------------------------------------
    if (prop_filename.empty() ||
        group_filename.empty() ||
        part_filename.empty() ||
        gadget_files_dir.empty())
    {
        std::cerr
            << "Usage:\n"
            << argv[0]
            << " --properties=<properties.hdf5>\n"
            << " --catalog-groups=<catalog_groups.hdf5>\n"
            << " --catalog-particles=<catalog_particles.hdf5>\n"
            << " --gadget-files-dir=<snapshot_directory>\n";

        return EXIT_FAILURE;
    }

    try
    {
        // Disable HDF5 error spam
        H5Eset_auto(H5E_DEFAULT, nullptr, nullptr);

        // ----------------------------------------------------
        // Read halo catalogs
        // ----------------------------------------------------
        HaloProperties properties =
            read_property_file(prop_filename);

        std::cout
            << "Loaded "
            << properties.ids.size()
            << " halos from property file.\n";

        GroupCatalog groups =
            read_group_file(
                group_filename,
                properties.ids.size());

        std::cout
            << "Loaded "
            << groups.offsets.size()
            << " group entries.\n";

        ParticleCatalog catalog_particles =
            read_particle_file(part_filename);

        std::cout
            << "Loaded "
            << catalog_particles.particle_ids.size()
            << " particle IDs from catalog.\n";

        // ----------------------------------------------------
        // Read Gadget snapshots
        // ----------------------------------------------------
        std::vector<Particle> particles =
               read_all_gadget_files_data(gadget_files_dir);

        std::cout
            << "Loaded "
            << particles.size()
            << " Gadget particles.\n";

        // ----------------------------------------------------
        // Example inspection
        // ----------------------------------------------------
        constexpr size_t target_halo_idx = 42;

        if (target_halo_idx < properties.ids.size())
        {
            std::cout
                << "\n=== Halo "
                << target_halo_idx
                << " ===\n";

            std::cout
                << "ID   : "
                << properties.ids[target_halo_idx]
                << "\n";

            std::cout
                << "Mass : "
                << properties.masses[target_halo_idx]
                << "\n";

            std::cout
                << "Pos  : ("
                << properties.xc[target_halo_idx] << ", "
                << properties.yc[target_halo_idx] << ", "
                << properties.zc[target_halo_idx]
                << ")\n";

            std::cout
                << "Npart: "
                << groups.group_sizes[target_halo_idx]
                << "\n";

            long long start =
                groups.offsets[target_halo_idx];

            int npart =
                groups.group_sizes[target_halo_idx];

            std::cout
                << "First particle IDs: ";

            for (int i = 0;
                 i < std::min(npart, 5);
                 ++i)
            {
                std::cout
                    << catalog_particles.particle_ids[start + i]
                    << " ";
            }

            std::cout << "\n";
        }

        std::cout
            << "\nAll input files loaded successfully.\n";
    }
    catch (const std::exception& e)
    {
        std::cerr
            << "Error: "
            << e.what()
            << "\n";

        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
