#pragma once

#include <vector>
#include <string>
#include <stdexcept>
#include <cstddef>


#include "hdf5.h"

struct HaloProperties
{
    std::vector<long long> ids;
    std::vector<float> masses;
    std::vector<float> xc;
    std::vector<float> yc;
    std::vector<float> zc;
};

struct GroupCatalog
{
    std::vector<long long> offsets;
    std::vector<int> group_sizes;
};

struct ParticleCatalog
{
    std::vector<long long> particle_ids;
};

template <typename T>
void read_dataset(hid_t file,
                  const char* dataset_name,
                  hid_t datatype,
                  T* buffer)
{
    hid_t dset = H5Dopen2(
        file,
        dataset_name,
        H5P_DEFAULT);

    if (dset < 0)
    {
        throw std::runtime_error(
            std::string("Failed to open dataset: ") +
            dataset_name);
    }

    herr_t status = H5Dread(
        dset,
        datatype,
        H5S_ALL,
        H5S_ALL,
        H5P_DEFAULT,
        buffer);

    H5Dclose(dset);

    if (status < 0)
    {
        throw std::runtime_error(
            std::string("Failed to read dataset: ") +
            dataset_name);
    }
}

HaloProperties read_property_file(const std::string& filename);

GroupCatalog read_group_file(
    const std::string& filename,
    size_t num_halos);

ParticleCatalog read_particle_file(
    const std::string& filename);
