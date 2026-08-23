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
                if (mu < 0.01)
                {
                    secondary =
                        secondary->next_coprog;

                    continue;
                }


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
            if (N_halos[mbin] == 0)
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

