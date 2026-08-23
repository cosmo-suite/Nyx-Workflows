#ifndef HALO_H
#define HALO_H

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

struct halo *find_halo(int64_t id);

void build_merger_tree_vtk(struct halo *descendant);

struct merger_event
{
    int64_t descendant_id;

    int64_t main_prog_id;
    int64_t secondary_id;

    double redshift;

    double descendant_mass;
    double main_prog_mass;
    double secondary_mass;

    double mass_ratio;
};

struct merger_rate_bin
{
    double redshift;

    double mass_min;
    double mass_max;

    double ratio_min;
    double ratio_max;

    int64_t num_halos;
    int64_t num_mergers;

    double rate;
};

std::vector<merger_event> compute_merger_rate(
    struct halo *descendant);

void print_merger_events(
    const std::vector<merger_event>& mergers);

void compute_population_merger_rate();
#endif
