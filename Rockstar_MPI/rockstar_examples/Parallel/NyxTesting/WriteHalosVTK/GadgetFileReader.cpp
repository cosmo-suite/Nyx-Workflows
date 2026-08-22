#include "GadgetFileReader.H"

#include <filesystem>
#include <iomanip>
#include <sstream>
#include <string>
#include <unordered_map>

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
read_all_gadget_files_data(
    const std::vector<std::string>& files)
{
    std::vector<Particle> particles;

    // --------------------------------------------------------
    // Read each supplied Gadget file
    // --------------------------------------------------------

    for (const auto& filename : files)
    {
        fs::path f(filename);

        std::cout
            << "Reading "
            << f
            << "\n";

        read_gadget_file(f, particles);
    }

    std::cout
        << "\nTotal particles read = "
        << particles.size()
        << "\n";

    // Example access
    for (size_t i = 0;
         i < std::min<size_t>(10, particles.size());
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

// ------------------------------------------------------------
// Read Gadget particle data for a specific snapshot
// ------------------------------------------------------------
std::vector<Particle>
read_gadget_particles(
    const std::string& gadget_files_dir,
    int64_t snapshot_id)
{
    // --------------------------------------------------------
    // Construct zero-padded snapshot ID
    //
    // Example:
    //   snapshot_id = 3
    //   snapshot_tag = "003"
    // --------------------------------------------------------

    std::ostringstream snapshot_stream;

    snapshot_stream
        << std::setw(3)
        << std::setfill('0')
        << snapshot_id;

    const std::string snapshot_tag =
        snapshot_stream.str();

    const std::string prefix =
        "nyx_snapshot." +
        snapshot_tag +
        ".";

    std::cout
        << "Reading Gadget files for snapshot "
        << snapshot_id
        << " ("
        << prefix
        << "*)\n";

    // --------------------------------------------------------
    // Find all matching Gadget files
    // --------------------------------------------------------

    std::vector<std::string> gadget_files;

    for (const auto& entry :
         fs::directory_iterator(gadget_files_dir))
    {
        if (!entry.is_regular_file())
        {
            continue;
        }

        const std::string filename =
            entry.path().filename().string();

        if (filename.rfind(prefix, 0) == 0)
        {
            gadget_files.push_back(
                entry.path().string());
        }
    }

    if (gadget_files.empty())
    {
        std::cerr
            << "Error: no Gadget files found for snapshot "
            << snapshot_id
            << " using prefix "
            << prefix
            << "\n";

        std::exit(1);
    }

    // --------------------------------------------------------
    // Sort files so that .0, .1, .2, ... are read in order
    // --------------------------------------------------------

    std::sort(
        gadget_files.begin(),
        gadget_files.end());

    std::cout
        << "Found "
        << gadget_files.size()
        << " Gadget files:\n";

    for (const auto& file : gadget_files)
    {
        std::cout
            << "    "
            << file
            << "\n";
    }

    // --------------------------------------------------------
    // Read the matching Gadget files
    //
    // NOTE:
    // This assumes GadgetFileReader provides a function that
    // can read a specific list of files. If the existing
    // read_all_gadget_files_data() only accepts a directory,
    // we will need to modify that function as well.
    // --------------------------------------------------------

    std::vector<Particle> particles =
        read_all_gadget_files_data(
            gadget_files);

    std::cout
        << "Loaded "
        << particles.size()
        << " Gadget particles.\n";

    // --------------------------------------------------------
    // Particle ID diagnostics
    // --------------------------------------------------------

    if (!particles.empty())
    {
        auto [min_it, max_it] =
            std::minmax_element(
                particles.begin(),
                particles.end(),
                [](const Particle& a,
                   const Particle& b)
                {
                    return a.id < b.id;
                });

        std::cout
            << "Min particle ID: "
            << min_it->id
            << "\n";

        std::cout
            << "Max particle ID: "
            << max_it->id
            << "\n";
    }

    std::unordered_map<uint64_t, size_t> counts;

    counts.reserve(
        particles.size());

    for (const auto& p : particles)
    {
        ++counts[p.id];
    }

    size_t duplicate_particles = 0;
    size_t max_count = 0;

    for (const auto& [id, count] : counts)
    {
        if (count > 1)
        {
            duplicate_particles +=
                count - 1;

            max_count =
                std::max(max_count, count);
        }
    }

    std::cout
        << "Unique IDs: "
        << counts.size()
        << "\n";

    std::cout
        << "Duplicate particle entries: "
        << duplicate_particles
        << "\n";

    std::cout
        << "Maximum occurrences of one ID: "
        << max_count
        << "\n";

    return particles;
}
