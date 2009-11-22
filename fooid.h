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

#ifndef FOOID_H
#define FOOID_H

#if defined(__cplusplus)
extern "C" {
#endif
#if defined(_WIN32) && defined(DYNAMIC)
#ifdef LIBFOOID_EXPORTS
#define FOOIDAPI _declspec(dllexport)
#else
#define FOOIDAPI _declspec(dllimport)
#endif
#else
#define FOOIDAPI
#endif

typedef struct t_fooid t_fooid;

/*
    Set up library for generating fingerprints for
    file with a given sampling rate and number of
    channels.

    input  * sampling rate in Hz
           * number of channels

    output * handle to fingerprinter
             (NULL on error)

*/
FOOIDAPI t_fooid * fp_init(int samplerate, int channels);

/*
    Free a fingerprinter handle.
*/
FOOIDAPI void fp_free(t_fooid * fid);

/*
    Feed a buffer of samples to the fingerprinting
    generator. The sample buffer size should be a
    multiple of the number of channels indicated
    earlier. Buffer layout is data[length][channels].
    You should keep feeding data as long as this
    function returns TRUE, or until you have no
    more data to feed.

    input  * fingerprinter handle
           * pointer to buffer of 16-bit signed shorts
           * length of buffer

    output *  1  if more data should be fed
              0  if enough is availabled to generate the fingerprint
            < 0  on error
*/
FOOIDAPI int fp_feed_short(t_fooid * fi, short *data, int size);

/*
    As above, but for 32-bit IEEE floats.
*/
FOOIDAPI int fp_feed_float(t_fooid * fi, float *data, int size);

/*
    Returns the size of the fingerprint
    that this library will generate.

    input  * fingerprinter handle

    output *  > 0  size of fingerprint in bytes
              < 0  on error
*/
FOOIDAPI int fp_getsize(t_fooid *fi);

/*
    Returns the fingerprint version number that
    this library will generate

    input  * fingerprinter handle

    output * version number
*/
FOOIDAPI int fp_getversion(t_fooid *fi);

/*
    Calculate the fingerprint.
    You should allocate or provide a buffer
    of suitable length (fg_getsize),
    and provide the real, total length of the song.
    Songs which do not contain enough usable data
    to generate a meaningful fingerprint will cause
    an error to be returned.

    input  * fingerprinter handle
           * total length of song in centiseconds
           * buffer to store fingerprint in

    output *   0 on success
             < 0 on error
*/
FOOIDAPI int fp_calculate(t_fooid *fi, int songlen, unsigned char* buff);


#if defined(__cplusplus)
} // extern "C"
#endif
#endif
