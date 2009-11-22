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

#include <stdlib.h>
#include <stdio.h>
#include "common.h"
#include "spectrum.h"
#include "harmonics.h"

static const int quantize_harmonic(const float dom)
{
    int i;

    static float quantbord[63] = {
         15.63f,         40.04f,         44.92f,         48.83f,
         50.78f,         52.73f,         54.69f,         57.62f,
         59.57f,         62.50f,         64.45f,         66.41f,
         69.34f,         71.29f,         73.24f,         74.22f,
         77.15f,         79.10f,         82.03f,         83.01f,
         83.98f,         86.91f,         87.89f,         91.80f,
         93.75f,         97.66f,         98.63f,         99.61f,
        103.52f,        105.47f,        109.38f,        110.35f,
        111.33f,        116.21f,        119.14f,        123.05f,
        125.00f,        130.86f,        134.77f,        140.63f,
        146.48f,        151.37f,        159.18f,        165.04f,
        171.88f,        182.62f,        195.31f,        201.17f,
        218.75f,        228.52f,        247.07f,        262.70f,
        291.02f,        310.61f,        333.98f,        372.07f,
        414.06f,        461.91f,        523.44f,        592.77f,
        699.22f,        868.16f,        1176.76
    };

    for (i = 0; i < 63; i++) {
        if (dom < quantbord[i]) {
            return i;
        }
    }

    return 63;
}

void get_dominant_harmonic(const t_complex *data, int *idom)
{
    int i;
    int maxid = 0;
    float pwr;
    float maxpwr = 0.0f;
    float dom;

    for (i = 0; i < SPEC_LEN; i++) {
        pwr = data[i].re * data[i].re + data[i].im * data[i].im;

        if (pwr > maxpwr) {
            maxpwr = pwr;
            maxid = i;
        }
    }

    dom = 4000.0f * ((float)maxid / SPEC_LEN);
    *idom = quantize_harmonic(dom);
}
