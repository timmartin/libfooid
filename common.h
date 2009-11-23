/*
    libFooID - Free audio fingerprinting library
    Copyright (C) 2006 Gian-Carlo Pascutto, Hogeschool Gent

    Use of this software is allowed under either:

    1) The GNU General Public License (GPL), as described
       in LICENSE.GPL.

    2) A modified BSD License, as described in LICENSE.BSDA.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
*/

#ifndef COMMON_H
#define COMMON_H

#include "fooid.h"

/*
    defines
*/
#if (_MSC_VER >= 1400) /* VC8+ Disable all deprecation warnings */
    #pragma warning(disable : 4996)
#endif /* VC8+ */

#define FALSE       0
#define TRUE        1
#define PI          3.14159265358979323846f
#define EPSILON     1e-15

#define FRAME_LEN                8192
#define SPEC_LEN       (FRAME_LEN / 2)
#define MAX_BARK                   17

/*
    size of total analysis sample data
*/
#define SSIZE       (8000 * 100)

/*
    number of samples of input to resample per time
*/
#define IN_LEN        2048

/*
    fingerprint (version 0)
*/
#define FPVERSION        0
#define FPSIZE         424

/*
    fingerprint storage
*/
struct t_fingerprint
{
    /*
        fingerprint version
    */
    short version;
    /*
        length in centiseconds
    */
    int length;
    /*
        average line fit, times 1000
    */
    short avg_fit;
    /*
        average dominant line, times 100
    */
    short avg_dom;
    /*
        spectral fits, 4 bits times 16 bands = 32 times 87 frames
        -> 348 bytes
    */
    unsigned char r[348];
    /*
        spectral doms, 6 bits times 87 frames = 65.25
    */
    unsigned char dom[66];
};

/*
    processing storage
*/
struct t_fooid
{
    /*  spectral stuff */
    float window[SPEC_LEN];
    int line_to_cb[SPEC_LEN];
    int cb_start[MAX_BARK];
    int cb_size[MAX_BARK];
    int max_sfb;

    /*  settings stuff */
    int channels;
    int samplerate;

    /* buffer stuff */
    float *samples;
    float *sbuffer;
    int soundfound;

    /* resampling stuff */
    float resample_ratio;
    void *resample_h;
    int outpos;

    /* actual fingerprint */
    struct t_fingerprint fp;
};

/*
    functions
*/
const int bitlen(int n);

#if defined(WIN32) || defined(SLOWROUND) || defined(WIN64)
int const round(const float x);
#endif


#endif

