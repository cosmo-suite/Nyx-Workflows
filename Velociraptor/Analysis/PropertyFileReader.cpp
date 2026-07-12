#include "VelociraptorOutputReader.H"

HaloProperties read_property_file(const std::string& filename)
{
    hid_t file = H5Fopen(
        filename.c_str(),
        H5F_ACC_RDONLY,
        H5P_DEFAULT);

    if (file < 0)
        throw std::runtime_error(
            "Failed to open property file");

    hid_t dataset_mass =
        H5Dopen2(file,
                 "Mass_200crit",
                 H5P_DEFAULT);

    if (dataset_mass < 0)
        throw std::runtime_error(
            "Failed to open Mass_200crit");

    hid_t dataspace =
        H5Dget_space(dataset_mass);

    hsize_t dims[1];

    H5Sget_simple_extent_dims(
        dataspace,
        dims,
        nullptr);

    size_t num_halos =
        static_cast<size_t>(dims[0]);

    HaloProperties props;

    props.masses.resize(num_halos);
    props.ids.resize(num_halos);
    props.xc.resize(num_halos);
    props.yc.resize(num_halos);
    props.zc.resize(num_halos);

    H5Dread(
        dataset_mass,
        H5T_NATIVE_FLOAT,
        H5S_ALL,
        H5S_ALL,
        H5P_DEFAULT,
        props.masses.data());

    H5Sclose(dataspace);
    H5Dclose(dataset_mass);

    read_dataset(
        file,
        "ID",
        H5T_NATIVE_LLONG,
        props.ids.data());

    read_dataset(
        file,
        "Xc",
        H5T_NATIVE_FLOAT,
        props.xc.data());

    read_dataset(
        file,
        "Yc",
        H5T_NATIVE_FLOAT,
        props.yc.data());

    read_dataset(
        file,
        "Zc",
        H5T_NATIVE_FLOAT,
        props.zc.data());

    H5Fclose(file);

    return props;
}
