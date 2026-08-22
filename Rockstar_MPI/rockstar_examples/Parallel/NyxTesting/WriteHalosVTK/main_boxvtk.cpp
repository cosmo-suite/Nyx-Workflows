#include <fstream>
#include <iostream>
#include <map>
#include <tuple>
#include <vector>

struct Point {
    double x, y, z;
};

int main()
{
    const double xmin = 0.0, xmax = 20.0;
    const double ymin = 0.0, ymax = 20.0;
    const double zmin = 0.0, zmax = 20.0;

    const int nx = 1;
    const int ny = 1;
    const int nz = 1;

    std::vector<Point> points;
    std::map<std::tuple<int,int,int>, int> point_id;

    // ------------------------------------------------------------
    // Create the (nx+1)x(ny+1)x(nz+1) grid vertices
    // ------------------------------------------------------------
    for (int k = 0; k <= nz; ++k) {
        double z = zmin + (zmax-zmin)*k/nz;
        for (int j = 0; j <= ny; ++j) {
            double y = ymin + (ymax-ymin)*j/ny;
            for (int i = 0; i <= nx; ++i) {
                double x = xmin + (xmax-xmin)*i/nx;

                point_id[{i,j,k}] = points.size();
                points.push_back({x,y,z});
            }
        }
    }

    std::vector<std::pair<int,int>> lines;

    // Lines along x
    for (int k = 0; k <= nz; ++k)
        for (int j = 0; j <= ny; ++j)
            for (int i = 0; i < nx; ++i)
                lines.push_back({
                    point_id[{i,j,k}],
                    point_id[{i+1,j,k}]
                });

    // Lines along y
    for (int k = 0; k <= nz; ++k)
        for (int j = 0; j < ny; ++j)
            for (int i = 0; i <= nx; ++i)
                lines.push_back({
                    point_id[{i,j,k}],
                    point_id[{i,j+1,k}]
                });

    // Lines along z
    for (int k = 0; k < nz; ++k)
        for (int j = 0; j <= ny; ++j)
            for (int i = 0; i <= nx; ++i)
                lines.push_back({
                    point_id[{i,j,k}],
                    point_id[{i,j,k+1}]
                });

    std::ofstream os("boxes.vtk");

    os << "# vtk DataFile Version 3.0\n";
    os << "2x2x2 box decomposition\n";
    os << "ASCII\n";
    os << "DATASET POLYDATA\n";

    // Points
    os << "POINTS " << points.size() << " float\n";
    for (const auto &p : points)
        os << p.x << " " << p.y << " " << p.z << "\n";

    // Lines
    os << "LINES " << lines.size() << " " << 3*lines.size() << "\n";
    for (const auto &l : lines)
        os << "2 " << l.first << " " << l.second << "\n";

    os.close();

    std::cout << "Wrote boxes.vtk\n";
}
