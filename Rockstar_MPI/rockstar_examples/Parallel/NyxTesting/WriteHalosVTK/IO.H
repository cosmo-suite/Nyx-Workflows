#ifndef IO_H
#define IO_H

#include <cstdint>        // For int64_t
#include <cstddef>        // For size_t
#include <string>         // For std::string
#include <vector>         // For std::vector
#include <unordered_map>  // For std::unordered_map

#include "Halo.H"         // For const Halo& parameter
#include "Particle.H"     // For const Particle& parameter

void
write_vtk(const std::string& filename,
          const std::vector<Particle>& particles,
          const std::vector<size_t>& indices,
          int64_t halo_id);

void 
write_halo_vtk(
    const Halo& halo,
    const std::vector<int64_t>& part_ids,
    const std::unordered_map<int64_t, size_t>& id_to_index,
    const std::vector<Particle>& particles);
#endif
