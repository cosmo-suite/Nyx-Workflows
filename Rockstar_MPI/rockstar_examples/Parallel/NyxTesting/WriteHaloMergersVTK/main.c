#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <inttypes.h>
#include <math.h>

#include "read_tree.h"


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
   Print all progenitors of a halo
   ------------------------------------------------------------ */

void print_progenitors(struct halo *h)
{
    struct halo *p;

    if (!h)
        return;

    printf("\nProgenitors of halo %" PRId64 "\n", h->id);
    printf("------------------------------------------------------------\n");

    p = h->prog;

    while (p) {

        printf("ID=%" PRId64
               "  z=%.4f"
               "  Mvir=%.6e"
               "  mmp=%" PRId64
               "  phantom=%" PRId64
               "\n",
               p->id,
               1.0 / p->scale - 1.0,
               p->mvir,
               p->mmp,
               p->phantom);

        p = p->next_coprog;
    }
}


/* ------------------------------------------------------------
   Print main progenitor history
   ------------------------------------------------------------ */

void print_main_progenitor_history(struct halo *h)
{
    int step = 0;

    printf("\n");
    printf("MAIN PROGENITOR HISTORY\n");
    printf("================================================================================\n");

    printf("%-6s %-8s %-10s %-18s %-6s %-8s %-8s\n",
           "Step",
           "ID",
           "z",
           "Mvir [Msun/h]",
           "MMP",
           "NumProg",
           "Phantom");

    printf("--------------------------------------------------------------------------------\n");

    while (h) {

        printf("%-6d %-8" PRId64 " %-10.5f %-18.6e %-6" PRId64
               " %-8" PRId64 " %-8" PRId64 "\n",
               step,
               h->id,
               1.0 / h->scale - 1.0,
               h->mvir,
               h->mmp,
               h->num_prog,
               h->phantom);

        /*
         * Follow the main progenitor backward in time.
         */
        h = h->prog;

        step++;
    }

    printf("================================================================================\n");
}


/* ------------------------------------------------------------
   Print merger history along main progenitor branch
   ------------------------------------------------------------ */

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

    if (argc != 2) {
        fprintf(stderr,
                "Usage: %s HALO_ID\n",
                argv[0]);
        return 1;
    }

    halo_id = strtoll(argv[1], NULL, 10);

    /*
     * Change this to your actual tree file.
     */
    read_tree(
        "/pscratch/sd/n/nataraj2/Nyx/cosmo-suite/"
        "Nyx-Workflows/Rockstar_MPI/rockstar_examples/Parallel/NyxTesting/"
        "halos/trees/tree_0_0_0.dat"
    );

    printf("Total halos: %" PRId64 "\n",
           all_halos.num_halos);

    printf("Number of scale lists: %" PRId64 "\n",
           halo_tree.num_lists);

    struct halo *h = find_halo(halo_id);

    if (!h) {
        fprintf(stderr,
                "Halo %" PRId64 " not found.\n",
                halo_id);
        delete_tree();
        return 1;
    }

    printf("\n");
    printf("Selected halo:\n");
    print_halo(h);

    print_progenitors(h);

    print_main_progenitor_history(h);

    print_merger_history(h);

    delete_tree();

    return 0;
}
