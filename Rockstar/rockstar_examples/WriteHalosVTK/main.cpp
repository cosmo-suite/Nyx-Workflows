#include <cstdint>
#include <cstring>
#include <fstream>
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>

static const uint64_t ROCKSTAR_MAGIC = 0xfadedacec0c0d0d0ULL;
static const int64_t PARTICLE_TYPE_IDS  = 1;
static const int64_t PARTICLE_TYPE_FULL = 2;
static const int BINARY_HEADER_SIZE = 256;
static const int VERSION_MAX_SIZE = 12;

struct BinaryHeader {
  uint64_t magic;
  int64_t snap, chunk;
  float scale, Om, Ol, h0;
  float bounds[6];
  int64_t num_halos, num_particles;
  float box_size, particle_mass;
  int64_t particle_type;
  int32_t format_revision;
  char rockstar_version[VERSION_MAX_SIZE];
  char unused[BINARY_HEADER_SIZE
              - (sizeof(char)*VERSION_MAX_SIZE)
              - (sizeof(float)*12)
              - sizeof(int32_t)
              - (sizeof(int64_t)*6)];
};

struct Halo {
  int64_t id;
  float pos[6], corevel[3], bulkvel[3];
  float m, r, child_r, vmax_r, mgrav, vmax, rvmax, rs, klypin_rs, vrms,
        J[3], energy, spin, alt_m[4], Xoff, Voff, b_to_a, c_to_a, A[3],
        b_to_a2, c_to_a2, A2[3],
        bullock_spin, kin_to_pot, m_pe_b, m_pe_d;
  int64_t num_p, num_child_particles, p_start, desc, flags, n_core;
  float min_pos_err, min_vel_err, min_bulkvel_err;
  int32_t type;
  float sm, gas, bh, peak_density, av_density;
};

struct Particle {
  int64_t id;
  float pos[6];
  float mass, energy;
  float softening;
  float metallicity;
  int32_t type;
};

template <typename T>
void read_exact(std::ifstream &in, T *data, size_t count) {
  in.read(reinterpret_cast<char *>(data), sizeof(T) * count);
  if (!in) {
    std::cerr << "Error: failed to read " << count << " items\n";
    std::exit(1);
  }
}

void write_vtk(const std::string &path,
               const std::vector<Particle> &pts,
               const std::vector<size_t> &indices,
               int64_t halo_id) {
  std::ofstream out(path);
  if (!out) {
    std::cerr << "Error: cannot open " << path << " for write\n";
    std::exit(1);
  }

  const size_t n = indices.size();
  out << "# vtk DataFile Version 3.0\n";
  out << "halo " << halo_id << "\n";
  out << "ASCII\n";
  out << "DATASET POLYDATA\n";
  out << "POINTS " << n << " float\n";
  for (size_t idx : indices) {
    const Particle &p = pts[idx];
    out << p.pos[0] << " " << p.pos[1] << " " << p.pos[2] << "\n";
  }

  out << "VERTICES " << n << " " << (n * 2) << "\n";
  for (size_t i = 0; i < n; i++) {
    out << "1 " << i << "\n";
  }

  out << "POINT_DATA " << n << "\n";
  out << "SCALARS id double 1\n";
  out << "LOOKUP_TABLE default\n";
  for (size_t idx : indices) {
    out << static_cast<double>(pts[idx].id) << "\n";
  }
  out << "SCALARS mass float 1\n";
  out << "LOOKUP_TABLE default\n";
  for (size_t idx : indices) {
    out << pts[idx].mass << "\n";
  }
}

int main(int argc, char **argv) {
  if (argc < 4) {
    std::cerr << "Usage: " << argv[0]
              << " <halos.bin> <particles.rbin> <out_dir>\n";
    return 1;
  }
  std::string bin_path  = argv[1];
  std::string rbin_path = argv[2];
  std::string out_dir   = argv[3];

  // Read .bin (halos + particle IDs)
  std::ifstream bin(bin_path, std::ios::binary);
  if (!bin) {
    std::cerr << "Error: cannot open " << bin_path << "\n";
    return 1;
  }
  BinaryHeader bh{};
  read_exact(bin, &bh, 1);
  if (bh.magic != ROCKSTAR_MAGIC) {
    std::cerr << "Error: .bin magic mismatch\n";
    return 1;
  }
  if (bh.particle_type != PARTICLE_TYPE_IDS) {
    std::cerr << "Error: .bin particle_type != IDS\n";
    return 1;
  }

  std::vector<Halo> halos(bh.num_halos);
  read_exact(bin, halos.data(), halos.size());

  std::vector<int64_t> part_ids(bh.num_particles);
  read_exact(bin, part_ids.data(), part_ids.size());

  // Read .rbin (full particles)
  std::ifstream rbin(rbin_path, std::ios::binary);
  if (!rbin) {
    std::cerr << "Error: cannot open " << rbin_path << "\n";
    return 1;
  }
  BinaryHeader rh{};
  read_exact(rbin, &rh, 1);
  if (rh.magic != ROCKSTAR_MAGIC) {
    std::cerr << "Error: .rbin magic mismatch\n";
    return 1;
  }
  if (rh.particle_type != PARTICLE_TYPE_FULL) {
    std::cerr << "Error: .rbin particle_type != FULL\n";
    return 1;
  }

  std::vector<Particle> particles(rh.num_particles);
  read_exact(rbin, particles.data(), particles.size());

  // Build id -> index map
  std::unordered_map<int64_t, size_t> id_to_idx;
  id_to_idx.reserve(particles.size() * 2);
  for (size_t i = 0; i < particles.size(); i++) {
    id_to_idx[particles[i].id] = i;
  }

  // Write one VTK per halo
  for (size_t i = 0; i < halos.size(); i++) {
    const Halo &h = halos[i];
    if (h.p_start < 0 || h.num_p < 0) continue;
    size_t start = static_cast<size_t>(h.p_start);
    size_t count = static_cast<size_t>(h.num_p);
    if (start + count > part_ids.size()) {
      std::cerr << "Warning: halo " << h.id << " out of range\n";
      continue;
    }

    std::vector<size_t> indices;
    indices.reserve(count);
    for (size_t j = 0; j < count; j++) {
      int64_t pid = part_ids[start + j];
      auto it = id_to_idx.find(pid);
      if (it != id_to_idx.end()) {
        indices.push_back(it->second);
      }
    }

    std::string out_path = out_dir + "/halo_" + std::to_string(h.id) + ".vtk";
    write_vtk(out_path, particles, indices, h.id);
  }

  std::cerr << "Done.\n";
  return 0;
}
