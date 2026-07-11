#include <iostream>
#include <vector>
#include <string>
#include <stdexcept>
#include <algorithm>

#include "hdf5.h"

struct Halo {
    long long id;
    float mass;
    float pos[3];
};

// Helper function to read a 1D dataset
template <typename T>
void read_dataset(hid_t file,
                  const char* dataset_name,
                  hid_t datatype,
                  T* buffer)
{
    hid_t dset = H5Dopen2(file, dataset_name, H5P_DEFAULT);

    if (dset < 0) {
        throw std::runtime_error(
            std::string("Failed to open dataset: ") + dataset_name);
    }

    herr_t status = H5Dread(
        dset,
        datatype,
        H5S_ALL,
        H5S_ALL,
        H5P_DEFAULT,
        buffer);

    H5Dclose(dset);

    if (status < 0) {
        throw std::runtime_error(
            std::string("Failed to read dataset: ") + dataset_name);
    }
}

int main(int argc, char* argv[])
{

    std::string prop_filename;
std::string group_filename;
std::string part_filename;
std::string gadget_files_dir;

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
            std::cerr << "Unknown argument: " << arg << "\n";
            return 1;
        }
    }

    if (prop_filename.empty() ||
        group_filename.empty() ||
        part_filename.empty() || 
        gadget_files_dir.empty()) {
        std::cerr
            << "Usage: " << argv[0]
            << " --properties=<prop-file>"
            << " --catalog-groups=<catalog_groups-file>"
            << " --catalog-particles=<catalog_particles-file>\n"
            << " --gadget-files-dir=<gadget-files-dir>\n";
        return EXIT_FAILURE;
    }

    try {

        // Disable HDF5 error printing
        H5Eset_auto(H5E_DEFAULT, nullptr, nullptr);

        // =====================================================
        // 1. READ HALO PROPERTIES
        // =====================================================

        hid_t file_prop = H5Fopen(
            prop_filename.c_str(),
            H5F_ACC_RDONLY,
            H5P_DEFAULT);

        if (file_prop < 0)
            throw std::runtime_error("Failed to open property file");


        hid_t dataset_mass = H5Dopen2(
            file_prop,
            "Mass_200crit",
            H5P_DEFAULT);

        if (dataset_mass < 0)
            throw std::runtime_error("Failed to open Mass_200crit");


        hid_t dataspace_mass = H5Dget_space(dataset_mass);

        hsize_t dims[1];

        H5Sget_simple_extent_dims(
            dataspace_mass,
            dims,
            nullptr);

        size_t num_halos = dims[0];

        std::cout
            << "Found "
            << num_halos
            << " halos in property file.\n";


        std::vector<float> masses(num_halos);
        std::vector<long long> halo_ids(num_halos);


        H5Dread(
            dataset_mass,
            H5T_NATIVE_FLOAT,
            H5S_ALL,
            H5S_ALL,
            H5P_DEFAULT,
            masses.data());


        H5Sclose(dataspace_mass);
        H5Dclose(dataset_mass);


        read_dataset(
            file_prop,
            "ID",
            H5T_NATIVE_LLONG,
            halo_ids.data());


        std::vector<float> xc(num_halos);
        std::vector<float> yc(num_halos);
        std::vector<float> zc(num_halos);


        read_dataset(
            file_prop,
            "Xc",
            H5T_NATIVE_FLOAT,
            xc.data());

        read_dataset(
            file_prop,
            "Yc",
            H5T_NATIVE_FLOAT,
            yc.data());

        read_dataset(
            file_prop,
            "Zc",
            H5T_NATIVE_FLOAT,
            zc.data());


        H5Fclose(file_prop);


        // =====================================================
        // 2. READ GROUP OFFSETS
        // =====================================================

        hid_t file_group = H5Fopen(
            group_filename.c_str(),
            H5F_ACC_RDONLY,
            H5P_DEFAULT);

        if (file_group < 0)
            throw std::runtime_error("Failed to open group file");


        std::vector<long long> offsets(num_halos);
        std::vector<int> group_sizes(num_halos);


        read_dataset(
            file_group,
            "Offset",
            H5T_NATIVE_LLONG,
            offsets.data());


        read_dataset(
            file_group,
            "Group_Size",
            H5T_NATIVE_INT,
            group_sizes.data());


        H5Fclose(file_group);


        // =====================================================
        // 3. READ PARTICLE IDS
        // =====================================================

        hid_t file_part = H5Fopen(
            part_filename.c_str(),
            H5F_ACC_RDONLY,
            H5P_DEFAULT);

        if (file_part < 0)
            throw std::runtime_error("Failed to open particle file");


        hid_t dataset_parts = H5Dopen2(
            file_part,
            "Particle_IDs",
            H5P_DEFAULT);

        if (dataset_parts < 0)
            throw std::runtime_error("Failed to open Particle_IDs");


        hid_t dataspace_parts = H5Dget_space(dataset_parts);


        hsize_t part_dims[1];

        H5Sget_simple_extent_dims(
            dataspace_parts,
            part_dims,
            nullptr);


        size_t total_particles =
            static_cast<size_t>(part_dims[0]);


        std::vector<long long> all_particle_ids(total_particles);


        H5Dread(
            dataset_parts,
            H5T_NATIVE_LLONG,
            H5S_ALL,
            H5S_ALL,
            H5P_DEFAULT,
            all_particle_ids.data());


        H5Sclose(dataspace_parts);
        H5Dclose(dataset_parts);
        H5Fclose(file_part);


        // =====================================================
        // 4. INSPECT ONE HALO
        // =====================================================

        size_t target_halo_idx = 42;


        if (target_halo_idx < num_halos)
        {
            std::cout
                << "\n--- Halo Index "
                << target_halo_idx
                << " ---\n";


            std::cout
                << "ID: "
                << halo_ids[target_halo_idx]
                << "\n";


            std::cout
                << "Mass: "
                << masses[target_halo_idx]
                << "\n";


            std::cout
                << "Position: ("
                << xc[target_halo_idx] << ", "
                << yc[target_halo_idx] << ", "
                << zc[target_halo_idx]
                << ")\n";


            std::cout
                << "Number of particles: "
                << group_sizes[target_halo_idx]
                << "\n";


            long long start_offset =
                offsets[target_halo_idx];


            int num_parts =
                group_sizes[target_halo_idx];


            std::cout
                << "First few Particle IDs: ";


            for (int i = 0;
                 i < std::min(num_parts, 5);
                 ++i)
            {
                std::cout
                    << all_particle_ids[start_offset + i]
                    << " ";
            }


            std::cout
                << "\n---------------------\n";
        }

    }
    catch (const std::exception& e)
    {
        std::cerr
            << "Error: "
            << e.what()
            << std::endl;

        return EXIT_FAILURE;
    }


    return EXIT_SUCCESS;
}

