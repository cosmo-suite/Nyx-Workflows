#include <cstdint>        // For int64_t
#include <cstddef>        // For size_t
#include <string>         // For std::string
#include <vector>         // For std::vector
#include <unordered_map>  // For std::unordered_map

#include "Halo.H"         // For const Halo& parameter
#include "Particle.H"     // For const Particle& parameter

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

void write_halo_vtk(
    const Halo& halo,
    const std::vector<int64_t>& part_ids,
    const std::unordered_map<int64_t, size_t>& id_to_index,
    const std::vector<Particle>& particles)
{
    if (halo.p_start < 0 ||
        halo.num_p <= 0)
    {
        return;
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

        return;
    }

    std::vector<size_t> indices;

    indices.reserve(count);

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

    write_vtk(
        vtk_name,
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
