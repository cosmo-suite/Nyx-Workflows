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


struct Particle
{
    double x;
    double y;
    double z;

    int64_t id;
    int64_t num_prog;
    double redshift;
};

void write_vtk(
    const std::string& filename,
    const std::vector<Particle>& particles,
    double redshift)
{
    std::ofstream out(filename);

    if (!out)
    {
        std::cerr
            << "Cannot open VTK file: "
            << filename
            << "\n";

        return;
    }

    out << "# vtk DataFile Version 3.0\n";
    out << "Halo merger tree\n";
    out << "ASCII\n";
    out << "DATASET POLYDATA\n";

    /*
     * --------------------------------------------------------
     * Points
     * --------------------------------------------------------
     */

    out << "POINTS "
        << particles.size()
        << " double\n";

    for (const auto& p : particles)
    {
        out << std::setprecision(10)
            << p.x << " "
            << p.y << " "
            << p.z << "\n";
    }

    /*
     * --------------------------------------------------------
     * Vertices
     *
     * Make every halo an individual VTK point.
     * --------------------------------------------------------
     */

    out << "\nVERTICES "
        << particles.size()
        << " "
        << particles.size() * 2
        << "\n";

    for (size_t i = 0; i < particles.size(); i++)
    {
        out << "1 " << i << "\n";
    }

    out << "\nFIELD FieldData 1\n";
    out << "Redshift 1 1 double\n";
    out << std::setprecision(10) << redshift << "\n";


    /*
     * --------------------------------------------------------
     * Halo properties
     * --------------------------------------------------------
     */

    out << "\nPOINT_DATA "
        << particles.size()
        << "\n";

    /*
     * Halo ID
     */

    out << "\nSCALARS halo_id long_long 1\n";
    out << "LOOKUP_TABLE default\n";

    for (const auto& p : particles)
        out << p.id << "\n";

    /*
     * Number of progenitors
     */

    out << "\nSCALARS num_prog long_long 1\n";
    out << "LOOKUP_TABLE default\n";

    for (const auto& p : particles)
        out << p.num_prog << "\n";

    /*
     * Redshift
     */

    out << "\nSCALARS redshift double 1\n";
    out << "LOOKUP_TABLE default\n";

    for (const auto& p : particles)
        out << p.redshift << "\n";

 
    out.close();
}

/* ------------------------------------------------------------
   Find a halo by ID in all_halos
   ------------------------------------------------------------ */

struct halo *find_halo(int64_t id)
{
    int64_t i;

    for (i = 0; i < all_halos.num_halos; i++) {
        if (all_halos.halos[i].id == id)
            return &all_halos.halos[i];
    }

    return NULL;
}


/* ------------------------------------------------------------
   Print one halo
   ------------------------------------------------------------ */

void print_halo(struct halo *h)
{
    if (!h) {
        printf("NULL halo\n");
        return;
    }

    double z = 1.0 / h->scale - 1.0;

    printf("%" PRId64
           "  snap=%" PRId32
           "  scale=%.6f"
           "  z=%.6f"
           "  Mvir=%.6e"
           "  mmp=%" PRId64
           "  num_prog=%" PRId64
           "  phantom=%" PRId64
           "\n",
           h->id,
           h->snap_num,
           h->scale,
           z,
           h->mvir,
           h->mmp,
           h->num_prog,
           h->phantom);
}


/* ------------------------------------------------------------
   One stored progenitor
   ------------------------------------------------------------ */

struct progenitor_entry {
    int64_t id;
    double z;
    int64_t num_prog;
};

void build_merger_tree_vtk(struct halo *descendant)
{
    std::vector<Particle> particles;

    /*
     * --------------------------------------------------------
     * Current level
     *
     * Initially this contains only the requested descendant.
     * --------------------------------------------------------
     */

    std::vector<struct halo *> current;
    current.push_back(descendant);

    /*
     * --------------------------------------------------------
     * Add the initial descendant to the cumulative vector.
     * --------------------------------------------------------
     */

    Particle root;

    root.x = descendant->pos[0];
    root.y = descendant->pos[1];
    root.z = descendant->pos[2];

    root.id = descendant->id;
    root.num_prog = descendant->num_prog;

    root.redshift =
        1.0 / descendant->scale - 1.0;

    particles.push_back(root);

    /*
     * --------------------------------------------------------
     * Write halo_merger_0.vtk
     *
     * This contains ONLY the requested descendant.
     * --------------------------------------------------------
     */

    double redshift = 1.0 / descendant->scale - 1.0;

write_vtk(
    "halo_merger_00.vtk",
    particles,
    redshift
);

    printf(
        "Wrote halo_merger_00.vtk  "
        "(z=%.5f, %zu halos)\n",
        root.redshift,
        particles.size()
    );

    /*
     * --------------------------------------------------------
     * Now walk backward through the merger tree.
     * --------------------------------------------------------
     */

    int vtk_index = 1;

    while (!current.empty())
    {
        /*
         * This will contain ALL progenitors of ALL halos
         * in the current level.
         */
        std::vector<struct halo *> next;

        /*
         * ----------------------------------------------------
         * Find all progenitors of all halos at this level.
         * ----------------------------------------------------
         */

        for (struct halo *h : current)
        {
            struct halo *p = h->prog;

            while (p)
            {
                /*
                 * Add progenitor to cumulative particle list.
                 */

                Particle particle;

                particle.x = p->pos[0];
                particle.y = p->pos[1];
                particle.z = p->pos[2];

                particle.id = p->id;
                particle.num_prog = p->num_prog;

                particle.redshift =
                    1.0 / p->scale - 1.0;

                particles.push_back(particle);

                /*
                 * Add progenitor to next level.
                 */
                next.push_back(p);

                p = p->next_coprog;
            }
        }

        /*
         * ----------------------------------------------------
         * If there were no progenitors, we are done.
         * ----------------------------------------------------
         */

        if (next.empty())
            break;

        /*
         * ----------------------------------------------------
         * All progenitors found at this level have now been
         * appended to the cumulative particle vector.
         *
         * Write the entire cumulative tree.
         * ----------------------------------------------------
         */

        char filename[256];

        snprintf(
            filename,
            sizeof(filename),
            "halo_merger_%02d.vtk",
            vtk_index
        );


        double redshift = 1.0 / next[0]->scale - 1.0;

write_vtk(
    filename,
    particles,
    redshift
);

        /*
         * Determine redshift of this level from first halo.
         */
        double z =
            1.0 / next[0]->scale - 1.0;

        printf(
            "Wrote %s  "
            "(z=%.5f, added=%zu, total=%zu)\n",
            filename,
            z,
            next.size(),
            particles.size()
        );

        /*
         * ----------------------------------------------------
         * Move backward one level.
         * ----------------------------------------------------
         */

        current = std::move(next);

        vtk_index++;
    }
}

/* ------------------------------------------------------------
   Print stored progenitors
   ------------------------------------------------------------ */

void print_progenitors(
    struct progenitor_entry *entries,
    int64_t num_entries)
{
    int64_t i;

    printf("\n");
    printf("%-12s %-20s %-12s\n",
           "Redshift",
           "Halo ID",
           "NumProg");

    printf("----------------------------------------------\n");

    for (i = 0; i < num_entries; i++) {

        printf("%-12.5f %-20" PRId64 " %-12" PRId64 "\n",
               entries[i].z,
               entries[i].id,
               entries[i].num_prog);
    }
}

void print_merger_history(struct halo *h)
{
    int step = 0;

    printf("\n");
    printf("MERGER HISTORY\n");
    printf("===============================================================================================\n");

    printf("%-6s %-10s %-18s %-18s %-18s %-12s %-12s\n",
           "Step",
           "z",
           "Desc Mvir",
           "Main Prog Mvir",
           "Secondary Mvir",
           "Ratio",
           "Type");

    printf("-----------------------------------------------------------------------------------------------\n");

    while (h && h->prog) {

        struct halo *main_prog = h->prog;
        struct halo *p = h->prog->next_coprog;

        /*
         * Every progenitor after the first is a secondary progenitor.
         */
        while (p) {

            double ratio = 0.0;

            if (main_prog->mvir > 0.0)
                ratio = p->mvir / main_prog->mvir;

            const char *type;

            if (ratio >= 0.30)
                type = "MAJOR";
            else if (ratio >= 0.10)
                type = "MINOR";
            else if (ratio >= 0.01)
                type = "SMALL";
            else
                type = "TINY";

            printf("%-6d %-10.5f %-18.6e %-18.6e %-18.6e %-12.5f %-12s\n",
                   step,
                   1.0 / h->scale - 1.0,
                   h->mvir,
                   main_prog->mvir,
                   p->mvir,
                   ratio,
                   type);

            p = p->next_coprog;
        }

        h = main_prog;
        step++;
    }

    printf("===============================================================================================\n");
}


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

    /*
     * --------------------------------------------------------
     * Build cumulative merger-tree VTK files.
     * --------------------------------------------------------
     */

    build_merger_tree_vtk(h);

    delete_tree();

    return 0;
}

