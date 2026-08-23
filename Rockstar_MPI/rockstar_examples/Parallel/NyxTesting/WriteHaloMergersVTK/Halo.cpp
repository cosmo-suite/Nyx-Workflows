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
