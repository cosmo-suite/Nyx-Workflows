#include "VelociraptorOutputReader.H"

GroupCatalog read_group_file(
    const std::string& filename,
    size_t num_halos)
{
    hid_t file =
        H5Fopen(filename.c_str(),
                H5F_ACC_RDONLY,
                H5P_DEFAULT);

    if (file < 0)
        throw std::runtime_error(
            "Failed to open group file");

    GroupCatalog groups;

    groups.offsets.resize(num_halos);
    groups.group_sizes.resize(num_halos);

    read_dataset(
        file,
        "Offset",
        H5T_NATIVE_LLONG,
        groups.offsets.data());

    read_dataset(
        file,
        "Group_Size",
        H5T_NATIVE_INT,
        groups.group_sizes.data());

    H5Fclose(file);

    return groups;
}
