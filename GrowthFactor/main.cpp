#include <iostream>
#include <fstream>
#include <vector>
#include <string>

#include "Cosmology.h"

// assume real = double or float

int main(int argc, char** argv)
{
    if (argc != 3)
    {
        std::cerr << "Usage: " << argv[0]
                  << " inputs.par z_vals.txt\n";
        return 1;
    }

    std::string param_file = argv[1];
    std::string z_file     = argv[2];

    CosmoClass cosmo;

    // read params
    cosmo.read_inputs(param_file);

    // read redshift list
    std::ifstream fin(z_file);
    if (!fin)
    {
        std::cerr << "Error opening z file\n";
        return 1;
    }

    std::vector<real> z_vals;
    real z;

    while (fin >> z)
        z_vals.push_back(z);

    // output file
    std::ofstream fout("growth_factors.txt");

    if (!fout)
    {
        std::cerr << "Error opening output file\n";
        return 1;
    }

    // compute growth factor
    for (real zi : z_vals)
    {
        real D, dDdz;

        cosmo.GrowthFactor(zi, &D, &dDdz);

        fout << D << "\n";
    }

    return 0;
}

