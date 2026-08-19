#ifndef NDFIELD_H
#define NDFIELD_H

#include <iostream>
#include <fstream>
#include <random>
#include <cstring>
#include <iomanip>
#include <cstdlib>

// ============================================================
// NDfield definitions
// ============================================================

#define NDFIELD_MAX_DIMS 20

// NDfield datatype definitions
#define ND_CHAR    1
#define ND_UCHAR   2
#define ND_SHORT   4
#define ND_USHORT  8
#define ND_INT     16
#define ND_UINT    32
#define ND_LONG    64
#define ND_ULONG   128
#define ND_FLOAT   256
#define ND_DOUBLE  512
#define NDFIELD_TAG "NDFIELD"

typedef struct NDfield_str
{
    char comment[80];

    int dims[NDFIELD_MAX_DIMS];

    int ndims;
    int n_dims;
    int fdims_index;
    int datatype;

    double x0[NDFIELD_MAX_DIMS];
    double delta[NDFIELD_MAX_DIMS];

    char dummy[160];

    void *val;

    long nval;
    int datasize;
} NDfield;

int sizeof_NDfield(int type);

int WriteNDfield(const char *filename, NDfield *field);

void WriteNDfieldToVTK(const char* filename,
                       const NDfield& field,
                       const char* scalar_name);


// ============================================================
// Create clustered filamentary density field
// ============================================================
//
// The field consists of:
//   1. A positive background density
//   2. Several Gaussian filamentary structures
//   3. Small random fluctuations
//
// The filaments are Gaussian tubes around specified line
// segments.
//
// ============================================================

struct Vec3
{
    double x;
    double y;
    double z;
};


// ------------------------------------------------------------
// Squared distance from point p to a line segment a -> b
// ------------------------------------------------------------

inline 
double DistanceSquaredToSegment(const Vec3& p,
                                const Vec3& a,
                                const Vec3& b)
{
    const double abx = b.x - a.x;
    const double aby = b.y - a.y;
    const double abz = b.z - a.z;

    const double apx = p.x - a.x;
    const double apy = p.y - a.y;
    const double apz = p.z - a.z;

    const double ab2 =
        abx * abx +
        aby * aby +
        abz * abz;

    double t = 0.0;

    if (ab2 > 0.0)
    {
        t =
            (apx * abx +
             apy * aby +
             apz * abz) / ab2;
    }

    // Clamp to the line segment
    if (t < 0.0)
        t = 0.0;

    if (t > 1.0)
        t = 1.0;

    const double closest_x = a.x + t * abx;
    const double closest_y = a.y + t * aby;
    const double closest_z = a.z + t * abz;

    const double dx = p.x - closest_x;
    const double dy = p.y - closest_y;
    const double dz = p.z - closest_z;

    return dx * dx + dy * dy + dz * dz;
}


// ============================================================
// Filament definition
// ============================================================

struct Filament
{
    Vec3 start;
    Vec3 end;

    double amplitude;
    double sigma;
};

#endif
