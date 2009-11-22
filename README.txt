libFooID audio fingerprinting library
-------------------------------------

The libFooID library allows you to make a fingerprint of a music piece
or song. This fingerprint contains information about the defining musical
characteristics of the song in a very compact manner.

The fingerprint will change only very little after encoding
(with MP3, AAC, Vorbis, ...), changing volume, adding noise,
or any other processing of the song, and can therefore be used
(with fuzzy matching) as a robust and compact way to identify it.


What do I do with this fingerprint?
-----------------------------------

One application for audio fingerprinting is the possibility
to retrieve metadata (such as tags) about the song in question.

We provide a free server for this at http://www.foosic.org
You can visit the website for more information about how to
use the fingerprint to retrieve tag information.

You are free to come up with other applications.


Usage
-----

The API of libFooID is described in the fooid.h header file.

In general, you will first call fp_init to set up the library.
Afterwards, you feed audio data to the library via the
fp_feed_short or fp_feed_float library calls. Next, you
call fp_getsize, allocate a structure suitable to hold the
fingerprint, and call fp_calculate. Lastly, you call fp_free.


Compiling
---------

libFooID has been tested to work correctly with both the
Microsoft Visual C++ 6.0 and Microsoft Visual Studio 2005 compilers.

It does not depend on any platform-specific functions and can
be trivially ported to other operating systems (FreeBSD, Linux, ...)
using the GCC compiler.

A precompiled statically linked lib and a DLL version are
included in the package for the users' convenience.


License
-------

The libFooID library is dual-licensed under both:

	1) the GPL
	2) a kind of BSD-with-advertising-clause license

Please see LICENSE.GPL and LICENSE.BSDA, respectively.

We hope that this makes the library suitable for use in all
kinds of software, be it free, shareware, freeware or commercial.

libFooID currently depends on libresample, which is licensed
under the LGPL. See LICENSE.LGPL in the libresample subdirectory.


Version history
---------------

* 2006-5-15: Initial release (1.0)


Authors & Contact
-----------------

libFooID was developed by Gian-Carlo Pascutto as a research project
at the Hogeschool Gent, Belgium. He can be contacted at <gcp@sjeng.org>
