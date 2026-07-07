#include <fstream>
#include <string>

void WriteBoxVTK(const std::string& filename,
                 double xmin, double xmax,
                 double ymin, double ymax,
                 double zmin, double zmax)
{
    std::ofstream os(filename);

    os << "# vtk DataFile Version 3.0\n";
    os << "Bounding Box\n";
    os << "ASCII\n";
    os << "DATASET POLYDATA\n\n";

    // 8 corner vertices
    os << "POINTS 8 float\n";
    os << xmin << " " << ymin << " " << zmin << "\n"; // 0
    os << xmax << " " << ymin << " " << zmin << "\n"; // 1
    os << xmax << " " << ymax << " " << zmin << "\n"; // 2
    os << xmin << " " << ymax << " " << zmin << "\n"; // 3
    os << xmin << " " << ymin << " " << zmax << "\n"; // 4
    os << xmax << " " << ymin << " " << zmax << "\n"; // 5
    os << xmax << " " << ymax << " " << zmax << "\n"; // 6
    os << xmin << " " << ymax << " " << zmax << "\n"; // 7

    // 12 edges
    os << "\nLINES 12 36\n";

    int edges[12][2] = {
        {0,1},{1,2},{2,3},{3,0}, // bottom
        {4,5},{5,6},{6,7},{7,4}, // top
        {0,4},{1,5},{2,6},{3,7}  // verticals
    };

    for (auto &e : edges)
        os << "2 " << e[0] << " " << e[1] << "\n";
}

int main()
{
    WriteBoxVTK("box.vtk",
            0.0, 20.0,
            0.0, 20.0,
            0.0, 20.0);
}
