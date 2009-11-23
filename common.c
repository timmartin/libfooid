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

#include "common.h"

const int bitlen(int n)
{
    int res = 0;

    while(n) {
        n = n >> 1;
        res++;
    }

    return res;
}

#if defined(SLOWROUND) || defined(WIN64)
const int round(const float x) {
    assert(x >= INT_MIN-0.5);
    assert(x <= INT_MAX+0.5);

    if (x >= 0) {
        return (int)(x+0.5);
    }
    return (int)(x-0.5);
}
#elsif defined(WIN32)
const int round(const float x)
{
    int a;
    __asm {
        fld x
        fistp a
    }
    return (a);
}
#endif
