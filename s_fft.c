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
#include <assert.h>
#include <math.h>
#include "s_fft.h"
#include "common.h"

t_fft_data* fft_init(const int fftsize)
{
    const int tabsize = fftsize;
    int i, j;
    int powlen, logb_n;
    float e, theta;
    t_fft_data *tb;

    /*
        allocate structure memory
    */
    tb = (t_fft_data*)malloc(sizeof(t_fft_data));

    /*
        init trig tables
    */
    tb->twiddle_tab  = (t_twiddle*)malloc(sizeof(t_twiddle) * tabsize);

    e = (2.0f * PI) / (float)fftsize;
    for (i = 0; i < tabsize; i++) {
        theta = e * (float) i;
        tb->twiddle_tab[i].cos_t = (float)cos(theta);
        tb->twiddle_tab[i].sin_t = (float)sin(theta);
        tb->twiddle_tab[i].cos3_t = (float)cos(theta * 3.0f);
        tb->twiddle_tab[i].sin3_t = (float)sin(theta * 3.0f);
    }

    /*
        init bitrev table
    */
    powlen = bitlen(fftsize - 1);
    logb_n = powlen >> 1;

    if ((powlen & 1) == 1) {
        logb_n++;
    }

    tb->seed_tab = (unsigned*)malloc(sizeof(unsigned) * (1 << logb_n));

    tb->seed_tab[0] = 0;
    tb->seed_tab[1] = 1;

    for (i = 2; i <= logb_n; i++) {
        for (j = 0; j < (1 << (i-1)); j++) {
            tb->seed_tab[j] <<= 1;
            tb->seed_tab[j + (1 << (i-1))] = tb->seed_tab[j] + 1;
        }
    }

    /*
        store size (mostly for error checks)
    */
    tb->size = fftsize;

    /*
        allocate work buffers
    */
    tb->work = (t_complex*)malloc(sizeof(t_complex) * fftsize);

    return tb;
}

void fft_free(t_fft_data *tb)
{
    free(tb->twiddle_tab);
    free(tb->seed_tab);
    free(tb->work);

    tb->twiddle_tab = NULL;
    tb->seed_tab = NULL;
    tb->work = NULL;
    tb->size = 0;

    free(tb);
}


static void fast_reorder(const t_fft_data *tb, t_complex *x)
{
    int i, j, n, firstj, offset, powlen;
    unsigned groupn;
    t_complex tmp;

    powlen = bitlen(tb->size - 1);
    n = 1 << (powlen >> 1);

    for (offset = 1; offset < n; offset++) {
        firstj = n * tb->seed_tab[offset];
        i = offset;
        j = firstj;

        tmp = x[i];
        x[i] = x[j];
        x[j] = tmp;

        for (groupn = 1; groupn < tb->seed_tab[offset]; groupn++) {
            i = i + n;
            j = firstj + tb->seed_tab[groupn];

            tmp = x[i];
            x[i] = x[j];
            x[j] = tmp;
        }
    }
}

static void radix_2_step(const t_fft_data *tb, t_complex *x)
{
    const int size = tb->size;
    int i0, i1, i_s, i_i;
    float R1, R2;

    i_s = 0;
    i_i = 4;

    do {
        for (i0 = i_s; i0 < size; i0 += i_i) {
            i1 = i0 + 1;

            R1 = x[i0].re;
            R2 = x[i0].im;

            x[i0].re = R1 + x[i1].re;
            x[i0].im = R2 + x[i1].im;

            x[i1].re = R1 - x[i1].re;
            x[i1].im = R2 - x[i1].im;
        }
        i_s = (i_i << 1) - 2;
        i_i <<= 2;
    } while (i_s < size);
}

static void L_step(const t_fft_data *tb, t_complex *x)
{
    const int size = tb->size;
    const int halfsize = size >> 1;
    int N2, N4;
    int k, j;
    int i0, i1, i2, i3;
    int i_s, i_i;
    int trig, trigexp;
    float CC1, SS1, CC3, SS3, R1, S1, S2, R2, R3;

    N2 = 4;
    N4 = 1;
    trigexp = halfsize >> 1;

    /*
        do the main loops, except for the
        last one
    */
    for (k = 2; k < halfsize; k <<= 1) {
        /*
            degenerated butterflies with cos(0) and sin(0)
            no multiplies
        */
        i_s = 0;
        i_i = N2 << 1;

        do {
            for (i0 = i_s; i0 < size; i0 += i_i) {
                i1 = i0 +   N4;
                i2 = i0 + 2*N4;
                i3 = i1 + 2*N4;

                R3 = x[i2].re + x[i3].re;
                R2 = x[i2].re - x[i3].re;
                R1 = x[i2].im + x[i3].im;
                S2 = x[i2].im - x[i3].im;

                x[i2].re = x[i0].re - R3;
                x[i2].im = x[i0].im - R1;
                x[i3].re = x[i1].re - S2;
                x[i3].im = x[i1].im + R2;

                x[i0].re += R3;
                x[i0].im += R1;
                x[i1].re += S2;
                x[i1].im -= R2;
            }
            i_s = (i_i << 1) - N2;
            i_i <<= 2;
        } while (i_s < size);

        trig = trigexp;

        for (j = 1; j < N4; j++) {
            i_s = j;
            i_i = N2 << 1;

            /*
                normal butterflies
                2 x 4 multiplies
            */
            CC1 = tb->twiddle_tab[trig].cos_t;
            SS1 = tb->twiddle_tab[trig].sin_t;
            CC3 = tb->twiddle_tab[trig].cos3_t;
            SS3 = tb->twiddle_tab[trig].sin3_t;
            trig += trigexp;

            do {
                for (i0 = i_s; i0 < size; i0 += i_i) {
                    i1 = i0 +   N4;
                    i2 = i0 + 2*N4;
                    i3 = i1 + 2*N4;

                    R1 = x[i2].re * CC1 + x[i2].im * SS1;
                    S1 = x[i2].im * CC1 - x[i2].re * SS1;
                    R2 = x[i3].re * CC3 + x[i3].im * SS3;
                    S2 = x[i3].im * CC3 - x[i3].re * SS3;

                    R3 = R1 + R2;
                    R2 = R1 - R2;
                    R1 = S1 + S2;
                    S2 = S1 - S2;

                    x[i2].re = x[i0].re - R3;
                    x[i2].im = x[i0].im - R1;
                    x[i3].re = x[i1].re - S2;
                    x[i3].im = x[i1].im + R2;

                    x[i0].re += R3;
                    x[i0].im += R1;
                    x[i1].re += S2;
                    x[i1].im -= R2;
                }
                i_s = (i_i << 1) - N2 + j;
                i_i <<= 2;
            } while (i_s < size);
        }

        N2 <<= 1;
        N4   = N2 >> 2;
        trigexp >>= 1;
    }

    /*
        degenerated last loop
        we don't optimize the butterfly (runs only once)
        but the loop structure (runs a lot!)
    */
    i0 =         0;
    i1 =        N4;
    i2 =      2*N4;
    i3 = i1 + 2*N4;

    while (i0 < N4) {
        /*
            normal butterflies
            2 x 4 multiplies
        */
        CC1 = tb->twiddle_tab[i0].cos_t;
        SS1 = tb->twiddle_tab[i0].sin_t;
        CC3 = tb->twiddle_tab[i0].cos3_t;
        SS3 = tb->twiddle_tab[i0].sin3_t;

        R1 = x[i2].re * CC1 + x[i2].im * SS1;
        S1 = x[i2].im * CC1 - x[i2].re * SS1;
        R2 = x[i3].re * CC3 + x[i3].im * SS3;
        S2 = x[i3].im * CC3 - x[i3].re * SS3;

        R3 = R1 + R2;
        R2 = R1 - R2;
        R1 = S1 + S2;
        S2 = S1 - S2;

        x[i2].re = x[i0].re - R3;
        x[i2].im = x[i0].im - R1;
        x[i3].re = x[i1].re - S2;
        x[i3].im = x[i1].im + R2;

        x[i0].re += R3;
        x[i0].im += R1;
        x[i1].re += S2;
        x[i1].im -= R2;

        i0++;
        i1++;
        i2++;
        i3++;
    }
}

/*
    assumes bitreversed input
*/
static void fft_proc_split(const t_fft_data *tb, t_complex *x)
{
    /*
        radix-2 first step
    */
    radix_2_step(tb, x);

    /*
        L-butterflies
    */
    L_step(tb, x);
}

/*
    complex split-radix DIT FFT
*/
void fft(const t_fft_data *tb, t_complex *x)
{
    assert(tb != NULL);

    fast_reorder(tb, x);
    fft_proc_split(tb, x);
}
