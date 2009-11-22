/**********************************************************************

  resamplesubs.c

  Real-time library interface by Dominic Mazzoni

  Based on resample-1.7:
    http://www-ccrma.stanford.edu/~jos/resample/

  License: LGPL - see the file LICENSE.txt for more information

**********************************************************************/

/* Definitions */
#include "resample_defs.h"

/*
 * FilterUp() - Applies a filter to a given sample when up-converting.
 * FilterUD() - Applies a filter to a given sample when up- or down-
 */

float FilterUp(float Imp[], float ImpD[], UWORD Nwing, BOOL Interp,
               float *Xp, float Ph, int Inc);

float FilterUD(float Imp[], float ImpD[], UWORD Nwing, BOOL Interp,
               float *Xp, float Ph, int Inc, float dhb);

void LpFilter(float c[], int N, float frq, float Beta, int Num);
