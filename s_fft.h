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

#ifndef S_FFT_DEFINED
#define S_FFT_DEFINED
/*
    defs
*/

typedef struct
{
    float re;
    float im;
} t_complex;

typedef struct
{
    float cos_t;
    float sin_t;
    float cos3_t;
    float sin3_t;
} t_twiddle;


typedef struct
{
    int size;

    /*
        trig tables
    */
    t_twiddle *twiddle_tab;
    unsigned *seed_tab;

    /*
        work buffers
    */
    t_complex *work;
}
t_fft_data;

/*
    funcs
*/
t_fft_data* fft_init(const int fftsize);
void fft_free(t_fft_data *tb);
void fft(const t_fft_data *tb, t_complex *x);

/*
    globals
*/
extern t_fft_data *xtb;

#endif


