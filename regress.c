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

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "common.h"

/*
    http://mathworld.wolfram.com/LeastSquaresFitting.html
*/

void do_linear_regress(float *dbspec, int len, float *r)
{
    int i;
    float ssx;
    float ssy;
    float sxy;

    float ssxx;
    float ssyy;
    float ssxy;

    float avx;
    float avy;

    float rsq;
    /* float b, a; */

    ssx = 0.0f;
    ssy = 0.0f;
    sxy = 0.0f;
    avx = 0.0f;
    avy = 0.0f;

    for (i = 0; i < len; i++) {
        avx += i;
        avy += dbspec[i];

        ssx  += i * i;
        ssy  += dbspec[i] * dbspec[i];
        sxy  += i * dbspec[i];
    }

    avx /= (float)len;
    avy /= (float)len;

    ssxx = ssx - (float)len * avx * avx;
    ssyy = ssy - (float)len * avy * avy;
    ssxy = sxy - (float)len * avx * avy;

    if (ssyy <= 0.0f + EPSILON) {
        *r = 1.0f;
    }

    /*
        b = ssxy / ssxx;
        a = avy - b * avx;
    */
    rsq = (ssxy * ssxy) / (ssxx * ssyy);

    /*
        It seems that rsq ^ 0.25 follows roughly
        a normal distribution, so thats our preferred
        way of looking at things
    */
    *r = (float)sqrt(sqrt(rsq));
}
