#include <algorithm>

#include "Halo.H"
#include "IO.H"

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

std::vector<merger_event> compute_merger_rate(
    struct halo *descendant)
{
    std::vector<merger_event> mergers;

    /*
     * Current level of the merger tree.
     */
    std::vector<struct halo *> current;

    current.push_back(descendant);

    /*
     * Walk backward through the tree.
     */
    while (!current.empty())
    {
        std::vector<struct halo *> next;

        /*
         * Examine every halo at this redshift.
         */
        for (struct halo *h : current)
        {
            /*
             * No progenitors -> nothing more to do.
             */
            if (!h->prog)
                continue;

            /*
             * First progenitor is the main progenitor.
             */
            struct halo *main_prog = h->prog;

            /*
             * Remaining progenitors are secondary progenitors.
             */
            struct halo *p = main_prog->next_coprog;

            while (p)
            {
                merger_event event;

                event.descendant_id = h->id;

                event.main_prog_id = main_prog->id;
                event.secondary_id = p->id;

                /*
                 * The merger happens between this progenitor
                 * level and the descendant level.
                 */
                event.redshift =
                    1.0 / h->scale - 1.0;

                event.descendant_mass = h->mvir;

                event.main_prog_mass =
                    main_prog->mvir;

                event.secondary_mass =
                    p->mvir;

                /*
                 * Merger mass ratio.
                 */
                if (main_prog->mvir > 0.0)
                {
                    event.mass_ratio =
                        p->mvir / main_prog->mvir;
                }
                else
                {
                    event.mass_ratio = 0.0;
                }

                mergers.push_back(event);

                /*
                 * Move to the next secondary progenitor.
                 */
                p = p->next_coprog;
            }

            /*
             * Add ALL progenitors to the next level.
             */
            struct halo *p2 = h->prog;

            while (p2)
            {
                next.push_back(p2);
                p2 = p2->next_coprog;
            }
        }

        /*
         * Move one timestep backward.
         */
        current = std::move(next);
    }

    return mergers;
}

void print_merger_events(
    const std::vector<merger_event>& mergers)
{
    printf("\n");
    printf("MERGER EVENTS\n");

    printf("==========================================================================\n");

    printf("%-10s %-15s %-15s %-15s %-15s\n",
           "z",
           "Descendant",
           "MainProg",
           "Secondary",
           "MassRatio");

    printf("--------------------------------------------------------------------------\n");

    for (const auto& m : mergers)
    {
        printf("%-10.5f %-15" PRId64 " %-15" PRId64
               " %-15" PRId64 " %-15.5f\n",
               m.redshift,
               m.descendant_id,
               m.main_prog_id,
               m.secondary_id,
               m.mass_ratio);
    }

    printf("==========================================================================\n");
}



/* ------------------------------------------------------------
   Population halo merger rate

   Calculates:

       dN_merger / dz

   as a function of:

       - descendant halo mass
       - progenitor mass ratio

   where

       mu = M_secondary / M_main

   The rate is normalized by the number of descendant halos:

       dN/dz = N_mergers / (N_halos * dz)

   and

       dN/(dz dmu) = dN/dz / Delta_mu

   ------------------------------------------------------------ */

void compute_population_merger_rate()
{
    /*
     * --------------------------------------------------------
     * Descendant halo mass bins.
     * --------------------------------------------------------
     */

    const double mass_bins[] =
    {
        6.0e8,
        1.0e9,
        3.16227766e9,
        1.0e10,
        3.16227766e10,
        1.0e11,
        3.16227766e11,
        1.0e12,
        3.16227766e12,
        1.0e13
    };

    const int n_mass_bins =
        sizeof(mass_bins) / sizeof(mass_bins[0]) - 1;


    /*
     * --------------------------------------------------------
     * Mass-ratio bins.
     *
     * mu = M_secondary / M_main
     * --------------------------------------------------------
     */



    const double ratio_bins[] =
    {
        0.01,
        0.03,
        0.10,
        0.25,
        0.50,
        1.00
    };

    const int n_ratio_bins =
        sizeof(ratio_bins) / sizeof(ratio_bins[0]) - 1;


    /*
     * --------------------------------------------------------
     * Collect unique scale factors.
     * --------------------------------------------------------
     */

    std::vector<double> scales;

    for (int64_t i = 0;
         i < all_halos.num_halos;
         i++)
    {
        double a =
            all_halos.halos[i].scale;

        bool found = false;

        for (double s : scales)
        {
            if (fabs(s - a) < 1.0e-6)
            {
                found = true;
                break;
            }
        }

        if (!found)
            scales.push_back(a);
    }


    /*
     * --------------------------------------------------------
     * Sort scale factors from small -> large.
     *
     * Small scale factor = high redshift
     * Large scale factor = low redshift
     *
     * Example:
     *
     *     a(z=15) < a(z=2.0)
     *
     * Therefore:
     *
     *     scales[i]     = progenitor level
     *     scales[i + 1] = descendant level
     *
     * --------------------------------------------------------
     */

    std::sort(
        scales.begin(),
        scales.end()
    );


    /*
     * --------------------------------------------------------
     * Print some diagnostic information.
     * --------------------------------------------------------
     */

    printf("\n");
    printf("Found %zu unique scale factors\n", scales.size());

    for (size_t i = 0; i < scales.size(); i++)
    {
        printf("  level %zu: a = %.8f   z = %.5f\n",
               i,
               scales[i],
               1.0 / scales[i] - 1.0);
    }


    /*
     * --------------------------------------------------------
     * Header.
     * --------------------------------------------------------
     */

    printf("\n");
    printf("POPULATION HALO MERGER RATE\n");

    printf("============================================================================================================================\n");

    printf("%-10s %-10s %-10s %-15s %-15s "
           "%-15s %-15s %-15s %-15s %-15s\n",

           "z_desc",
           "z_prog",
           "dz",
           "M_desc",
           "N_halos",
           "mu_min",
           "mu_max",
           "N_mergers",
           "dN/dz",
           "dN/dz/dmu");

    printf("----------------------------------------------------------------------------------------------------------------------------\n");


    /*
     * --------------------------------------------------------
     * Loop over adjacent redshift levels.
     *
     * scales:
     *
     *   high z ------------------> low z
     *
     *   progenitor                descendant
     *
     * Example:
     *
     *   z=3.19992  --->  z=3.00000
     *
     * --------------------------------------------------------
     */

    for (size_t level = 0;
         level + 1 < scales.size();
         level++)
    {
        double a_prog =
            scales[level];

        double a_desc =
            scales[level + 1];


        double z_prog =
            1.0 / a_prog - 1.0;

        double z_desc =
            1.0 / a_desc - 1.0;


        double dz =
            z_prog - z_desc;


        if (dz <= 0.0)
            continue;


        /*
         * ----------------------------------------------------
         * Counters.
         * ----------------------------------------------------
         */

        int64_t N_halos[10] = {0};

        int64_t N_mergers[10][5] = {{0}};


        /*
         * ----------------------------------------------------
         * Loop through all halos.
         *
         * Select halos belonging to the DESCENDANT level.
         * ----------------------------------------------------
         */

        for (int64_t i = 0;
             i < all_halos.num_halos;
             i++)
        {
            struct halo *h =
                &all_halos.halos[i];


            /*
             * Only descendant halos at this scale.
             */
            if (fabs(h->scale - a_desc) > 1.0e-6)
                continue;


            /*
             * Ignore halos below the reliable mass range.
             */
            if (h->mvir < mass_bins[0])
                continue;


            /*
             * Find mass bin.
             */
            int mbin = -1;

            for (int m = 0;
                 m < n_mass_bins;
                 m++)
            {
                if (h->mvir >= mass_bins[m] &&
                    h->mvir < mass_bins[m + 1])
                {
                    mbin = m;
                    break;
                }
            }


            if (mbin < 0)
                continue;


            /*
             * This is one halo in the population.
             */
            N_halos[mbin]++;


            /*
             * ------------------------------------------------
             * No progenitors.
             * ------------------------------------------------
             */

            if (!h->prog)
                continue;


            /*
             * First progenitor = main progenitor.
             */
            struct halo *main_prog =
                h->prog;


            if (main_prog->mvir <= 0.0)
                continue;


            /*
             * ------------------------------------------------
             * Remaining progenitors are secondary progenitors.
             * ------------------------------------------------
             */

            struct halo *secondary =
                main_prog->next_coprog;


            while (secondary)
            {
                /*
                 * Mass ratio.
                 */
                double mu =
                    secondary->mvir /
                    main_prog->mvir;


                /*
                 * Ignore mergers below mu = 0.01.
                 */
                /*if (mu < 0.01)
                {
                    secondary =
                        secondary->next_coprog;

                    continue;
                }*/


                /*
                 * Ignore pathological ratios.
                 */
                if (mu > 1.0)
                {
                    secondary =
                        secondary->next_coprog;

                    continue;
                }


                /*
                 * Find mass-ratio bin.
                 */
                int rbin = -1;

                for (int r = 0;
                     r < n_ratio_bins;
                     r++)
                {
                    if (mu >= ratio_bins[r] &&
                        mu < ratio_bins[r + 1])
                    {
                        rbin = r;
                        break;
                    }
                }


                /*
                 * Exactly mu = 1 belongs to final bin.
                 */
                if (mu == 1.0)
                    rbin = n_ratio_bins - 1;


                if (rbin >= 0)
                    N_mergers[mbin][rbin]++;


                secondary =
                    secondary->next_coprog;
            }
        }


        /*
         * ----------------------------------------------------
         * Print results.
         * ----------------------------------------------------
         */

        for (int mbin = 0;
             mbin < n_mass_bins;
             mbin++)
        {
            /*
             * No halos in this mass bin.
             */
            if (N_halos[mbin] < 100)
                continue;


            /*
             * Geometric center of mass bin.
             */
            double logM =
                0.5 *
                (
                    log10(mass_bins[mbin]) +
                    log10(mass_bins[mbin + 1])
                );

            double Mrep =
                pow(10.0, logM);


            for (int rbin = 0;
                 rbin < n_ratio_bins;
                 rbin++)
            {
                int64_t Nm =
                    N_mergers[mbin][rbin];


                /*
                 * ------------------------------------------------
                 * Per-halo merger rate per unit redshift.
                 *
                 * dN/dz
                 * ------------------------------------------------
                 */

                double rate_dz =
                    (double) Nm /
                    (
                        (double) N_halos[mbin] *
                        dz
                    );


                /*
                 * ------------------------------------------------
                 * Differential rate per unit mass ratio.
                 * ------------------------------------------------
                 */

                double dmu =
                    ratio_bins[rbin + 1] -
                    ratio_bins[rbin];


                double rate_dz_dmu =
                    rate_dz / dmu;


                printf(
                    "%-10.5f %-10.5f %-10.5f "
                    "%-15.3e %-15" PRId64 " "
                    "%-15.3f %-15.3f "
                    "%-15" PRId64 " "
                    "%-15.8f %-15.8f\n",

                    z_desc,
                    z_prog,
                    dz,

                    Mrep,
                    N_halos[mbin],

                    ratio_bins[rbin],
                    ratio_bins[rbin + 1],

                    Nm,

                    rate_dz,
                    rate_dz_dmu
                );
            }

            printf("\n");
        }
    }

    printf("============================================================================================================================\n");
}


#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <inttypes.h>
#include <math.h>
#include <vector>
#include <algorithm>

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <inttypes.h>
#include <math.h>
#include <vector>
#include <algorithm>

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <inttypes.h>
#include <math.h>
#include <vector>
#include <algorithm>

/*
 * Helper: Recursively collect all secondary progenitors for a given descendant,
 * including multi-level sub-branches (sub-subhalos) that merged during dz.
 */
void collect_all_progenitors(struct halo *curr, struct halo *main_prog, std::vector<struct halo*> &secondaries)
{
    if (!curr) return;

    if (curr != main_prog)
    {
        secondaries.push_back(curr);
    }

    // Traverse coprogenitor linked-list
    collect_all_progenitors(curr->next_coprog, main_prog, secondaries);
}

void compute_mean_merger_rate()
{
    /*
     * --------------------------------------------------------
     * Simulation Resolution & Completeness Cutoffs
     * --------------------------------------------------------
     */
    const double particle_mass = 6.5e5;                      // Particle mass (M_sun / h)
    const double min_prog_mass = 40.0 * particle_mass;       // Hard 40-particle detection limit
    const double completeness_mass = 500.0 * particle_mass;  // 500-particle threshold for complete subhalo detection

    /*
     * --------------------------------------------------------
     * Descendant halo mass bins
     * --------------------------------------------------------
     */
    const double mass_bins[] =
    {
        6.0e8,
        1.0e9,
        3.16227766e9,
        1.0e10,
        3.16227766e10,
        1.0e11,
        3.16227766e11,
        1.0e12,
        3.16227766e12,
        1.0e13
    };

    const int n_mass_bins = sizeof(mass_bins) / sizeof(mass_bins[0]) - 1;
    const int64_t MIN_HALOS = 100;

    /*
     * --------------------------------------------------------
     * Mass-ratio bins (xi = M_i / M_1)
     * Fine logarithmic grid (50 bins) spanning [10^-4, 1.0]
     * --------------------------------------------------------
     */
    const int n_xi_bins = 50;
    double xi_bins[n_xi_bins + 1];

    double log_xi_min = log10(1.0e-4);
    double log_xi_max = log10(1.0);
    double dlog_xi = (log_xi_max - log_xi_min) / n_xi_bins;

    for (int i = 0; i <= n_xi_bins; i++)
    {
        xi_bins[i] = pow(10.0, log_xi_min + i * dlog_xi);
    }

    /*
     * --------------------------------------------------------
     * Collect and sort unique scale factors
     * --------------------------------------------------------
     */
    std::vector<double> scales;

    for (int64_t i = 0; i < all_halos.num_halos; i++)
    {
        double a = all_halos.halos[i].scale;
        bool found = false;

        for (double s : scales)
        {
            if (fabs(s - a) < 1.0e-6)
            {
                found = true;
                break;
            }
        }

        if (!found)
            scales.push_back(a);
    }

    std::sort(scales.begin(), scales.end());

    printf("\nMEAN HALO MERGER RATE -- FAKHOURI STYLE\n");
    printf("============================================================\n");
    printf("Found %zu unique scale factors\n", scales.size());

    /*
     * --------------------------------------------------------
     * Loop over adjacent snapshot pairs
     * --------------------------------------------------------
     */
    for (size_t level = 0; level + 1 < scales.size(); level++)
    {
        double a_prog = scales[level];
        double a_desc = scales[level + 1];

        double z_prog = 1.0 / a_prog - 1.0;
        double z_desc = 1.0 / a_desc - 1.0;

        double dz = z_prog - z_desc;

        if (dz <= 0.0)
            continue;

        int64_t N_halos[n_mass_bins] = {0};
        int64_t N_mergers[n_mass_bins][n_xi_bins] = {{0}};

        /*
         * ----------------------------------------------------
         * Loop over descendant halos
         * ----------------------------------------------------
         */
        for (int64_t i = 0; i < all_halos.num_halos; i++)
        {
            struct halo *h = &all_halos.halos[i];

            if (fabs(h->scale - a_desc) > 1.0e-6)
                continue;

            if (h->mvir < mass_bins[0])
                continue;

            int mbin = -1;
            for (int m = 0; m < n_mass_bins; m++)
            {
                if (h->mvir >= mass_bins[m] && h->mvir < mass_bins[m + 1])
                {
                    mbin = m;
                    break;
                }
            }

            if (mbin < 0)
                continue;

            N_halos[mbin]++;

            if (!h->prog)
                continue;

            /*
             * FIX 1: Scan all coprogenitors to guarantee M_1 is strictly 
             * the MOST MASSIVE progenitor at z_prog.
             */
            struct halo *main_prog = h->prog;
            struct halo *curr = h->prog->next_coprog;

            while (curr)
            {
                if (curr->mvir > main_prog->mvir)
                {
                    main_prog = curr;
                }
                curr = curr->next_coprog;
            }

            double m1_mass = main_prog->mvir;
            if (m1_mass <= 0.0)
                continue;

            /*
             * Gather all secondary progenitors attached to descendant h
             */
            std::vector<struct halo*> secondaries;
            collect_all_progenitors(h->prog, main_prog, secondaries);

            for (struct halo *secondary : secondaries)
            {
                double m2_mass = secondary->mvir;

                // Resolution check on secondary progenitor
                if (m2_mass < min_prog_mass)
                    continue;

                // Mass ratio xi = M_2 / M_1
                double xi = m2_mass / m1_mass;

                if (xi < xi_bins[0] || xi > 1.0)
                    continue;

                int xbin = -1;
                for (int x = 0; x < n_xi_bins; x++)
                {
                    if (xi >= xi_bins[x] && xi < xi_bins[x + 1])
                    {
                        xbin = x;
                        break;
                    }
                }

                if (xi == 1.0)
                    xbin = n_xi_bins - 1;

                if (xbin >= 0)
                    N_mergers[mbin][xbin]++;
            }
        }

        /*
         * ----------------------------------------------------
         * Output File Handling
         * ----------------------------------------------------
         */
        char filename[256];
        snprintf(filename, sizeof(filename), "merger_rate_z%.2f.dat", z_desc);

        FILE *fp = fopen(filename, "w");
        if (!fp)
        {
            fprintf(stderr, "ERROR: cannot open %s\n", filename);
            continue;
        }

        fprintf(fp, "# Mean merger rate per halo\n");
        fprintf(fp, "# z_desc = %.8f\n", z_desc);
        fprintf(fp, "# z_prog = %.8f\n", z_prog);
        fprintf(fp, "# dz = %.8f\n", dz);
        fprintf(fp, "# xi    dNm/dxi/dz\n");

        /*
         * ----------------------------------------------------
         * Calculate dN_m / dxi / dz
         * ----------------------------------------------------
         */
        for (int x = 0; x < n_xi_bins; x++)
        {
            int64_t total_mergers = 0;
            int64_t valid_halos = 0;

            double xi_center = sqrt(xi_bins[x] * xi_bins[x + 1]);
            double dxi = xi_bins[x + 1] - xi_bins[x];

            for (int m = 0; m < n_mass_bins; m++)
            {
                if (N_halos[m] < MIN_HALOS)
                    continue;

                /*
                 * FIX 2: Evaluate resolution against the LOWER mass bound of the bin (mass_bins[m]) 
                 * to guarantee that every single halo contributing to the sum meets the 
                 * 500-particle completeness criterion.
                 */
                double M_min_bin = mass_bins[m];

                if (xi_center * M_min_bin >= completeness_mass)
                {
                    total_mergers += N_mergers[m][x];
                    valid_halos += N_halos[m];
                }
            }

            if (valid_halos == 0 || total_mergers == 0)
                continue;

            double rate = (double)total_mergers / ((double)valid_halos * dz * dxi);

            fprintf(fp, "%.8e %.8e\n", xi_center, rate);
        }

        fclose(fp);
        printf("z = %.5f  ->  %s\n", z_desc, filename);
    }

    printf("============================================================\n");
}

