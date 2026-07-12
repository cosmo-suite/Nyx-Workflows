#include "VelociraptorOutputReader.H"

ParticleCatalog read_particle_file(
    const std::string& filename)
{
    hid_t file =
        H5Fopen(filename.c_str(),
                H5F_ACC_RDONLY,
                H5P_DEFAULT);

    if (file < 0)
        throw std::runtime_error(
            "Failed to open particle file");

    hid_t dataset =
        H5Dopen2(file,
                 "Particle_IDs",
                 H5P_DEFAULT);

    if (dataset < 0)
        throw std::runtime_error(
            "Failed to open Particle_IDs");

    hid_t dataspace =
        H5Dget_space(dataset);

    hsize_t dims[1];

    H5Sget_simple_extent_dims(
        dataspace,
        dims,
        nullptr);

    size_t nparticles =
        static_cast<size_t>(dims[0]);

    ParticleCatalog catalog;

    catalog.particle_ids.resize(nparticles);

    H5Dread(
        dataset,
        H5T_NATIVE_LLONG,
        H5S_ALL,
        H5S_ALL,
        H5P_DEFAULT,
        catalog.particle_ids.data());

    H5Sclose(dataspace);
    H5Dclose(dataset);
    H5Fclose(file);

    return catalog;
}
