#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <inttypes.h>
#include <math.h>
#include <string>
#include <vector>
#include <fstream>
#include <iostream>
#include <iomanip>

#include "read_tree.h"
#include "Halo.H"
#include "IO.H"

/* ------------------------------------------------------------
   Main
   ------------------------------------------------------------ */

int main(int argc, char **argv)
{
    int64_t halo_id;

    if (argc != 2)
    {
        fprintf(
            stderr,
            "Usage: %s HALO_ID\n",
            argv[0]
        );

        return 1;
    }

    halo_id =
        strtoll(argv[1], NULL, 10);

    /*
     * --------------------------------------------------------
     * Read Consistent Trees file
     * --------------------------------------------------------
     */

    read_tree(
        "/pscratch/sd/n/nataraj2/Nyx/cosmo-suite/"
        "Nyx-Workflows/Rockstar_MPI/rockstar_examples/Parallel/NyxTesting/"
        "halos/trees/tree_0_0_0.dat"
    );

    printf(
        "Total halos: %" PRId64 "\n",
        all_halos.num_halos
    );

    printf(
        "Number of scale lists: %" PRId64 "\n",
        halo_tree.num_lists
    );

    /*
     * --------------------------------------------------------
     * Find requested halo
     * --------------------------------------------------------
     */

    struct halo *h =
        find_halo(halo_id);

    if (!h)
    {
        fprintf(
            stderr,
            "Halo %" PRId64 " not found.\n",
            halo_id
        );

        delete_tree();

        return 1;
    }

    printf(
        "\nStarting halo:\n"
        "ID = %" PRId64 "\n"
        "z  = %.5f\n"
        "x  = %.8f\n"
        "y  = %.8f\n"
        "z  = %.8f\n"
        "num_prog = %" PRId64 "\n\n",
        h->id,
        1.0 / h->scale - 1.0,
        h->pos[0],
        h->pos[1],
        h->pos[2],
        h->num_prog
    );

    std::vector<merger_event> mergers =
    compute_merger_rate(h);

    //print_merger_events(mergers);

    compute_population_merger_rate();    

    /*
     * --------------------------------------------------------
     * Build cumulative merger-tree VTK files.
     * --------------------------------------------------------
     */

    //build_merger_tree_vtk(h);


    compute_mean_merger_rate();

    delete_tree();

    return 0;
}

