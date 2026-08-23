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
#include "IO.H"

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
