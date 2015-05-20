#ifndef _scandir_h
#define _scandir_h

#include "config.h"

#undef PARAMS
#if defined (__GNUC__) || __STDC__
# define PARAMS(args) args
#else
# define PARAMS(args) ()
#endif

#if ! HAVE_SCANDIR

int scandir(const char *dir,
	    struct dirent ***namelist, int (*select) PARAMS((const struct dirent *)), int (*cmp) PARAMS((const void *, const void *)));

#endif				/* if ! HAVE_SCANDIR */

#if ! HAVE_ALPHASORT

int alphasort(const void *a, const void *b);

#endif				/* if ! HAVE_ALPHASORT */

#endif				/* double-inclusion protection */
