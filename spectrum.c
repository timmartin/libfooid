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
#include <string.h>
#include "common.h"
#include "s_fft.h"
#include "spectrum.h"
#include "harmonics.h"
#include "regress.h"

/*
    transform frequency to Bark
*/
static const float toBARK(const float f)
{
    float z;

    z = ((26.81f * f) / (1960.0f + f)) - 0.53f;

    if (z < 2.0f) {
        z = z + 0.15f * (2.0f - z);
    } else if (z > 20.1f) {
        z = z + 0.22f * (z - 20.1f);
    }

    return z;
}

/*
    make a Hann window for the FFT
*/
void init_sine_window(t_fooid *fi)
{
    int i;

    for (i = 0; i < SPEC_LEN; i++) {
        fi->window[i] = (float)sqrt(0.5 - 0.5*cos(2*PI*(float)i/(FRAME_LEN)));
    }
}

/*
    apply a window function to float data
*/
static void windowize(float *window, float *smp)
{
    int i;

    for(i = 0; i < SPEC_LEN; i++) {
        smp[i] *= window[i];
    }
    for(i = SPEC_LEN; i < FRAME_LEN; i++) {
        smp[i] *= window[FRAME_LEN - i - 1];
    }
}

/*
    set lookups from frequency or spectrum line
    to Bark and the reverse
*/
void init_scales(t_fooid *fi)
{
    int i;
    float f;
    int lastcb;
    int cbsize;
    int cb;

    fi->cb_start[0] = 0;
    cbsize = 0;
    lastcb = 0;

    for (i = 0; i < SPEC_LEN; i++) {
        f = ((i * 8000.0f) + SPEC_LEN) / (FRAME_LEN);
        cb = (int)toBARK(f);

        /*
            we limit the number of Bark bands
            specifically, we don't want to start
            a new band at around 3700Hz, but we
            extend the 3100Hz band to the end of
            our spectrum
        */
        if (cb > MAX_BARK - 1) {
            cb = MAX_BARK - 1;
        }

        fi->line_to_cb[i] = cb;

        if (cb != lastcb) {
            fi->cb_start[lastcb + 1] = i;
            fi->cb_size[lastcb] = cbsize;
            lastcb++;
            cbsize = 0;
        }

        cbsize++;
    }

    fi->cb_size[lastcb] = cbsize;
    fi->max_sfb = lastcb + 1;
}

static void get_dbpower(t_complex *work, float *dbpower)
{
    int i;
    float power;

    for (i = 0; i < SPEC_LEN; i++) {
        power = (work[i].re * work[i].re) + (work[i].im * work[i].im);

        if (power <= EPSILON) {
            dbpower[i] = 0.0f;
        } else {
            /* 10 * log(x1/x2) / log(10) */
            dbpower[i] = (float)log(power) * 4.34294480f;
        }
    }
}

static int quantize_r(const float r, const int band)
{
    const static float q1[MAX_BARK] = {
        0.8116f,
        0.4273f,        0.4233f,        0.3827f,        0.3783f,
        0.3848f,        0.3726f,        0.3669f,        0.3500f,
        0.3440f,        0.3281f,        0.3256f,        0.3148f,
        0.3091f,        0.3031f,        0.3340f,        0.5660f
    };

    const static float q2[MAX_BARK] = {
        0.8528f,
        0.5300f,        0.5267f,        0.4800f,        0.4765f,
        0.4853f,        0.4711f,        0.4635f,        0.4430f,
        0.4356f,        0.4147f,        0.4117f,        0.3993f,
        0.3892f,        0.3825f,        0.4151f,        0.6249f
    };
    const static float q3[MAX_BARK] = {
        0.8824f,
        0.6216f,        0.6220f,        0.5754f,        0.5736f,
        0.5838f,        0.5699f,        0.5595f,        0.5374f,
        0.5281f,        0.5032f,        0.4983f,        0.4850f,
        0.4674f,        0.4612f,        0.4929f,        0.6746f
    };

    if (r < q1[band]) {
        return 0;
    }
    if (r < q2[band]) {
        return 1;
    }
    if (r < q3[band]) {
        return 2;
    }

    return 3;
}

void get_params(t_fooid *fi)
{
    t_fft_data *fft_data;
    int i, j;
    int frames;
    int ansize;
    float r[MAX_BARK];
    int qr[MAX_BARK];
    float dbpower[SPEC_LEN];
    int counts[4];
    int doms[88];
    int domidx;
    int idom;
    int total_dom;
    float avg_dom;
    float avg_qr;

    /*
        prepare FFT data and window
    */
    fft_data = fft_init(FRAME_LEN);

    ansize = (8000 * 90);

    frames = ansize / FRAME_LEN;

    counts[0] = 0;
    counts[1] = 0;
    counts[2] = 0;
    counts[3] = 0;
    total_dom = 0;
    memset(doms, 0, sizeof(int) * 88);

    for (i = 0; i < frames; i++) {
        /*
            set up FFT data
        */
        windowize(fi->window, &(fi->samples[i * FRAME_LEN]));

        for (j = 0; j < FRAME_LEN; j++) {
            fft_data->work[j].re = fi->samples[i * FRAME_LEN + j];
            fft_data->work[j].im = 0.0f;
        }

        fft(fft_data, fft_data->work);

        get_dbpower(fft_data->work, dbpower);

        for (j = 1; j < fi->max_sfb; j++) {
            do_linear_regress(&dbpower[fi->cb_start[j]], fi->cb_size[j], &r[j]);
            qr[j] = quantize_r(r[j], j);
        }

        get_dominant_harmonic(fft_data->work, &idom);

        total_dom += idom;

        for (j = 1; j < fi->max_sfb; j++) {
            counts[qr[j]]++;
        }

        /*
            store the r data packed into bytes, 4 bytes per frame
        */
        fi->fp.r[(i*4)]    = (qr[1] << 6)  | (qr[2] << 4)  | (qr[3] << 2)  | qr[4];
        fi->fp.r[(i*4)+1]  = (qr[5] << 6)  | (qr[6] << 4)  | (qr[7] << 2)  | qr[8];
        fi->fp.r[(i*4)+2]  = (qr[9] << 6)  | (qr[10] << 4) | (qr[11] << 2) | qr[12];
        fi->fp.r[(i*4)+3]  = (qr[13] << 6) | (qr[14] << 4) | (qr[15] << 2) | qr[16];
        /*
            store the dom data in a temporary array
        */
        doms[i]   = idom;
    }

    /*
        66 byte for the doms
        they are 6 bits, so we store them 4 at a time, in 3 bytes
    */
    domidx = 0;
    for (i = 0; i < 87; i += 4) {
        fi->fp.dom[domidx++] = (doms[i] << 2)           | (doms[i+1] >> 4);
        fi->fp.dom[domidx++] = (doms[i+1] & 0xF) << 4   | (doms[i+2] >> 2);
        fi->fp.dom[domidx++] = ((doms[i+2] & 0x3) << 6) | (doms[i+3]);

    }

    avg_dom = (float)total_dom / (float)frames;
    avg_qr  = ((1.0f * counts[1]) + (2.0f * counts[2]) + (3.0f * counts[3]))
              / ((float)frames*(float)(fi->max_sfb-1));

    fi->fp.avg_dom = round(avg_dom *  100.0f);
    fi->fp.avg_fit = round(avg_qr  * 1000.0f);

    fft_free(fft_data);
}
