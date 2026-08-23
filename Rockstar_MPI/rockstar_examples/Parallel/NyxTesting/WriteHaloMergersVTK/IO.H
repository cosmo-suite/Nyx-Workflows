#ifndef IO_H
#define IO_H

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

struct Particle
{
    double x;
    double y;
    double z;

    int64_t id;
    int64_t num_prog;
    double redshift;
};

/* ------------------------------------------------------------
   One stored progenitor
------------------------------------------------------------ */

struct progenitor_entry {
    int64_t id;
    double z;
    int64_t num_prog;
};

void print_progenitors(
    struct progenitor_entry *entries,
    int64_t num_entries);

void write_vtk(
    const std::string& filename,
    const std::vector<Particle>& particles,
    double redshift);
#endif
