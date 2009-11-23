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
#include <string.h>
#include <stdio.h>
#include <math.h>
#include <limits.h>

#include "common.h"
#include "fooid.h"

#include "spectrum.h"
#include "libresample/resample.h"

/* The original code seemed to assume that min() was a part of the standard library.
   Since it isn't, I've added a simple implementation. */
#if !defined(WIN32) && !defined(WIN64)
inline int min(int a, int b) { return (a < b) ? a : b; }
#endif


FOOIDAPI struct t_fooid* fp_init(int samplerate, int channels)
{
    t_fooid *res = (t_fooid*)malloc(sizeof(t_fooid));

    if (res == NULL) {
        return NULL;
    }

    memset(&(res->fp), 0, sizeof(struct t_fingerprint));
    res->fp.version = FPVERSION;

    /*
        store input settings
    */
    res->channels = channels;
    res->samplerate = samplerate;
    res->soundfound = 0;
    res->outpos = 0;

    /*
        get Bark division & FFT window
    */
    init_sine_window(res);
    init_scales(res);

    /*
        get input buffer
    */
    res->samples = (float *)calloc(SSIZE, sizeof(float));
    res->sbuffer = (float *)malloc(sizeof(float) * IN_LEN);

    if (res->samples == NULL || res->sbuffer == NULL) {
        return NULL;
    }

     /*
        set up resampling to 8000 Hz
    */
    res->resample_ratio = 8000.0f / (float)res->samplerate;
    res->resample_h = resample_open(FALSE, res->resample_ratio, res->resample_ratio);

    if (res->resample_h == NULL) {
        return NULL;
    }

    return res;
}

FOOIDAPI int fp_feed_float(t_fooid * fid, float *data, int len)
{
    int pos;
    int c;
    float accum;
    int inpos;
    int res_out;
    int in_used;

    /*
        check input validity
    */
    if (len % fid->channels != 0) {
        return -1;
    }

    len = len / fid->channels;

    if (!fid->soundfound) {
        pos = 0;

        while (!fid->soundfound && pos < len) {
            for (c = 0; c < fid->channels; c++) {
                if (fabs(data[(pos * fid->channels) + c]) >= (1.0f/32768.0f - EPSILON)) {
                    fid->soundfound = TRUE;
                }
            }
            if (!fid->soundfound) {
                pos++;
            }
        }

        /*
            end without sound?
        */
        if (pos >= len && !fid->soundfound) {
            return TRUE;
        }

        /*
            adjust start
        */
        len  = len - pos;
        data = data + (pos * fid->channels);
    }

    /*
        silly user feeding us more than we need?
    */
    if (fid->outpos >= SSIZE) {
        return FALSE;
    }

    /*
        now we have music data queued up
        process it at most IN_LEN at a time
    */
    do {
        /*
            read samples
        */
        for (pos = 0; pos < len && pos < IN_LEN; pos++) {
            /*
                downmix sample
            */
            accum = 0;
            for (c = 0; c < fid->channels; c++) {
                accum += data[(pos * fid->channels) + c];
            }
            accum /= (float)fid->channels;

            fid->sbuffer[pos] = accum;
        }

        inpos = 0;

        /*
            feed to resampler
        */
        do {
            res_out = resample_process(fid->resample_h, fid->resample_ratio,
                                       &(fid->sbuffer[inpos]), min(IN_LEN - inpos, len), FALSE,
                                       &in_used,
                                       &(fid->samples[fid->outpos]), SSIZE - fid->outpos);
            fid->outpos += res_out;
            inpos       += in_used;
        } while (in_used < min(IN_LEN, len) && fid->outpos < SSIZE);

        /*
            are we done yet?
        */
        if (fid->outpos >= SSIZE) {
            return FALSE;
        }

        /*
            check if there's still input left
        */
        len  = len - in_used;
        data = data + (in_used * fid->channels);
    } while (len > 0);

    return TRUE;
}

FOOIDAPI  int fp_feed_short(t_fooid *fid, short *data, int len)
{
    int res;
    int i;
    float *buff;

    buff = (float*)malloc(sizeof(float) * len * fid->channels);

    if (buff == NULL) {
        return FALSE;
    }

    for (i = 0; i < len * fid->channels; i++) {
        buff[i] = data[i] / 32767.0f;
    }

    res = fp_feed_float(fid, buff, len);

    free(buff);

    return res;
}

FOOIDAPI int fp_getversion(t_fooid *fi)
{
    return FPVERSION;
}

FOOIDAPI int fp_getsize(t_fooid *fi)
{
    return FPSIZE;
}

FOOIDAPI int fp_calculate(t_fooid *fi, int songlen, unsigned char* buff)
{
    /*
        we need at least 10 seconds of usable data or so
    */
    if (songlen < 1000 || !fi->soundfound || fi->outpos < (8000 * 10)) {
        return -1;
    }

    fi->fp.length = songlen;
    get_params(fi);

    /*
        now pack our structure into the minimal space
        possible
    */
    memcpy(buff, &(fi->fp.version), sizeof(short));
    buff += sizeof(short);
    memcpy(buff, &(fi->fp.length), sizeof(int));
    buff += sizeof(int);
    memcpy(buff, &(fi->fp.avg_fit), sizeof(short));
    buff += sizeof(short);
    memcpy(buff, &(fi->fp.avg_dom), sizeof(short));
    buff += sizeof(short);
    memcpy(buff, &(fi->fp.r), sizeof(unsigned char) * 348);
    buff += sizeof(unsigned char) * 348;
    memcpy(buff, &(fi->fp.dom), sizeof(unsigned char) * 66);
    buff += sizeof(unsigned char) * 66;

    return 0;
}


FOOIDAPI void fp_free(t_fooid * fid)
{
    resample_close(fid->resample_h);
    free(fid->sbuffer);
    free(fid->samples);
    free(fid);
}
