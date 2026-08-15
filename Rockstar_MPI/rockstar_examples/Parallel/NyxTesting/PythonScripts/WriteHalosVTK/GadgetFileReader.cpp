#include "GadgetFileReader.H"

#include <filesystem>
#include <iomanip>
#include <sstream>
#include <string>

std::string make_vtk_filename(const std::string& input_path)
{
    // Extract filename only
    std::string fname =
        std::filesystem::path(input_path).filename().string();

    // Find last '.'
    std::size_t pos = fname.rfind('.');

    if (pos == std::string::npos)
        throw std::runtime_error(
            "Could not extract file id from " + fname);

    // Get final token ("52")
    int file_id = std::stoi(fname.substr(pos + 1));

    // Format as 3 digits ("052")
    std::ostringstream ss;
    ss << "all_particles_"
       << std::setw(3)
       << std::setfill('0')
       << file_id
       << ".vtk";

    return ss.str();
}

void write_vtk_particles(
    const std::string& filename,
    const std::vector<Particle>& particles)
{
    std::ofstream out(filename, std::ios::binary);

    if (!out)
        throw std::runtime_error("Cannot open output file");

    const uint32_t N = static_cast<uint32_t>(particles.size());

    // Header
    out << "# vtk DataFile Version 3.0\n";
    out << "Particles\n";
    out << "BINARY\n";
    out << "DATASET POLYDATA\n";

    // Particle coordinates
    out << "POINTS " << N << " float\n";

    for (const auto& p : particles)
    {
        write_be(out, p.x);
        write_be(out, p.y);
        write_be(out, p.z);
    }

    out << "\n";

    // One vertex per point
    out << "VERTICES " << N << " " << 2 * N << "\n";

    for (uint32_t i = 0; i < N; ++i)
    {
        write_be(out, uint32_t(1));
        write_be(out, i);
    }

    out << "\n";

    // Particle IDs
    out << "POINT_DATA " << N << "\n";
    out << "SCALARS id long_long 1\n";
    out << "LOOKUP_TABLE default\n";

    for (const auto& p : particles)
    {
        write_be(out, p.id);
    }

    out << "\n";
}

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

// ------------------------------------------------------------
// Read one Gadget file
// ------------------------------------------------------------
void read_gadget_file(const fs::path& filename,
                      std::vector<Particle>& particles)
{
    std::ifstream in(filename,
                     std::ios::binary);

    if (!in)
    {
        throw std::runtime_error(
            "Cannot open " + filename.string());
    }


    // Header
    uint32_t header_size;

    in.read(reinterpret_cast<char*>(&header_size),
            sizeof(uint32_t));


    if (header_size != 256)
    {
        throw std::runtime_error(
            "Invalid Gadget header size");
    }


    GadgetHeader header{};

    in.read(reinterpret_cast<char*>(&header),
            sizeof(GadgetHeader));


    uint32_t header_size_end;

    in.read(reinterpret_cast<char*>(&header_size_end),
            sizeof(uint32_t));


    if (header_size != header_size_end)
    {
        throw std::runtime_error(
            "Header block mismatch");
    }


    // Number of particles of type 1
    uint32_t npart =
        header.num_particles[1];


    std::vector<float> pos;
    std::vector<float> vel;
    std::vector<int64_t> ids;


    // Positions
    read_block(in, pos);


    // Velocities
    read_block(in, vel);
    /*for(auto &tmp_vel: vel){
        std::cout << "Value is " << tmp_vel << std::endl;
    }*/

    // IDs
    read_block(in, ids);



    if (pos.size() != 3*npart)
    {
        throw std::runtime_error(
            "Position count mismatch");
    }


    
    if (ids.size() != npart)
    {
        std::cout << "Valiues are " << ids.size() << " " << npart << std::endl; 
        throw std::runtime_error(
            "ID count mismatch");
    }


    size_t old_size = particles.size();

    particles.resize(old_size + npart);


    for (uint64_t i=0; i<npart; i++)
    {
        particles[old_size+i].id = ids[i];

        particles[old_size+i].x =
            pos[3*i+0];

        particles[old_size+i].y =
            pos[3*i+1];

        particles[old_size+i].z =
            pos[3*i+2];
    }


    std::cout
        << filename.filename()
        << " : "
        << npart
        << " particles\n";
}


std::vector<Particle>
read_all_gadget_files_data(const std::string prefix)
{

    fs::path directory(prefix);


    if (!fs::exists(directory))
    {
        std::cerr
            << "Directory does not exist\n";
        exit(1);
    }

    std::vector<fs::path> files;

    for (auto& entry :
         fs::directory_iterator(directory))
    {
        if (entry.is_regular_file())
        {
            files.push_back(entry.path());
        }
    }


    // Sort for deterministic ordering
    std::sort(files.begin(),
              files.end());
    
    std::cout << "Sorting files done" << std::endl;

    std::vector<Particle> particles;

    for (auto& f : files)
    {
        
        //std::vector<Particle> tmp_particles;
        read_gadget_file(f, particles);
        //read_gadget_file(f, tmp_particles);
         //std::string vtk_name = make_vtk_filename(f);

         //write_vtk_particles(vtk_name, tmp_particles);    
    }

    std::cout
        << "\nTotal particles read = "
        << particles.size()
        << "\n";


    // Example access
    for (size_t i=0;
         i<std::min<size_t>(10,particles.size());
         i++)
    {
        std::cout
            << particles[i].id
            << " "
            << particles[i].x
            << " "
            << particles[i].y
            << " "
            << particles[i].z
            << "\n";
    }
    
    return particles;
}
