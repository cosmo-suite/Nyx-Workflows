#ifndef NDFIELD_H
#define NDFIELD_H

#include <fstream>
#include <random>
#include <cstring>
#include <iomanip>
#include <cstdlib>
#include <vector>
#include <cmath>
#include <cstdio>
#include <cstdint>

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

std::streamoff WriteNDfieldHeader(
    const char* filename,
    const NDfield& field);

void WriteBlock(
    FILE* f,
    const double* global_data,
    const NDfield& field,
    int i0,
    int i1,
    int j0,
    int j1,
    int k0,
    int k1,
    std::streamoff data_start);

void WriteNDfieldBlockedSerial(
    const char* filename,
    const NDfield& field,
    int nblocks_x,
    int nblocks_y,
    int nblocks_z);

void CreateField(
    const double Lx,
    const double Ly,
    const double Lz,
    const int nx,
    const int ny,
    const int nz,
    NDfield& field);

std::streamoff WriteNDfieldDataMarker(
    const char* filename,
    const NDfield& field);

// ============================================================
// Simple 3D vector
// ============================================================

struct Vec3
{
    double x;
    double y;
    double z;
};


// ============================================================
// Filament
// ============================================================

struct Filament
{
    Vec3 start;
    Vec3 end;

    double amplitude;
    double sigma;
};


// ============================================================
// Distance squared from point to line segment
// ============================================================

inline double DistanceSquaredToSegment(
    const Vec3& p,
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
        abx*abx +
        aby*aby +
        abz*abz;

    double t = 0.0;

    if (ab2 > 0.0)
    {
        t =
            (apx*abx +
             apy*aby +
             apz*abz) / ab2;
    }

    // Clamp to segment
    if (t < 0.0)
        t = 0.0;

    if (t > 1.0)
        t = 1.0;

    const double cx = a.x + t * abx;
    const double cy = a.y + t * aby;
    const double cz = a.z + t * abz;

    const double dx = p.x - cx;
    const double dy = p.y - cy;
    const double dz = p.z - cz;

    return dx*dx + dy*dy + dz*dz;
}
#endif
