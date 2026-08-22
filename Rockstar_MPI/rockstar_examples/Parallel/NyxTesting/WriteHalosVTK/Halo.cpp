#include "Halo.H"

#include <filesystem>
#include <iomanip>
#include <sstream>
#include <string>
#include <unordered_map>


std::unordered_map<int64_t, size_t>
build_particle_id_lookup(
    const std::vector<Particle>& particles)
{
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

    return id_to_index;
}


HaloCatalog
read_all_halos_in_bin_file(
    const std::string& halo_bin_file)
{
    // --------------------------------------------------------
    // Open Rockstar halo catalog
    // --------------------------------------------------------

    std::ifstream bin(
        halo_bin_file,
        std::ios::binary);

    if (!bin)
    {
        std::cerr
            << "Cannot open "
            << halo_bin_file
            << "\n";

        std::exit(1);
    }

    // --------------------------------------------------------
    // Read binary header
    // --------------------------------------------------------

    BinaryHeader bh{};

    read_exact(
        bin,
        &bh,
        1);

    std::cout
        << "format_revision = "
        << bh.format_revision
        << "\n";

    std::cout
        << "add_flag = "
        << bh.add_flag
        << "\n";

    std::cout
        << "sizeof(BinaryHeader) = "
        << sizeof(BinaryHeader)
        << "\n";

    std::cout
        << "sizeof(Halo) = "
        << sizeof(Halo)
        << "\n";

    // --------------------------------------------------------
    // Validate header
    // --------------------------------------------------------

    if (bh.magic != ROCKSTAR_MAGIC)
    {
        std::cerr
            << "Invalid halo .bin file.\n";

        std::exit(1);
    }

    if (bh.particle_type != PARTICLE_TYPE_IDS)
    {
        std::cerr
            << "Halo file does not contain particle IDs.\n";

        std::exit(1);
    }

    // --------------------------------------------------------
    // Read halos
    // --------------------------------------------------------

    std::vector<Halo> halos(
        static_cast<size_t>(
            bh.num_halos));

    read_exact(
        bin,
        halos.data(),
        halos.size());

    // --------------------------------------------------------
    // Read particle IDs
    // --------------------------------------------------------

    std::vector<int64_t> part_ids(
        static_cast<size_t>(
            bh.num_particles));

    read_exact(
        bin,
        part_ids.data(),
        part_ids.size());

    std::cout
        << "Read "
        << halos.size()
        << " halos and "
        << part_ids.size()
        << " particle IDs from Rockstar catalog.\n";

    // --------------------------------------------------------
    // Print first 20 particle IDs
    // --------------------------------------------------------

    std::cout
        << "First 20 particle IDs:\n";

    for (size_t i = 0;
         i < std::min<size_t>(
             20,
             part_ids.size());
         i++)
    {
        std::cout
            << i
            << " : "
            << part_ids[i]
            << "\n";
    }

    // --------------------------------------------------------
    // Return catalog
    // --------------------------------------------------------

    return HaloCatalog{
        std::move(halos),
        std::move(part_ids)
    };
}
