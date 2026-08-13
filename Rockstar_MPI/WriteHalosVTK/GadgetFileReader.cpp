#include "GadgetFileReader.H"

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
    std::vector<int32_t> ids;


    // Positions
    read_block(in, pos);


    // Velocities
    read_block(in, vel);


    // IDs
    read_block(in, ids);



    if (pos.size() != 3*npart)
    {
        throw std::runtime_error(
            "Position count mismatch");
    }


    if (ids.size() != npart)
    {
        throw std::runtime_error(
            "ID count mismatch");
    }



    size_t old_size = particles.size();

    particles.resize(old_size + npart);


    for (uint32_t i=0; i<npart; i++)
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

    std::vector<Particle> particles;

    for (auto& f : files)
    {
        read_gadget_file(f, particles);
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
