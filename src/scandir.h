#ifndef _scandir_h
#define _scandir_h

#include "system.h"

#if HAVE_DIRENT_H

#  include <dirent.h>
#  define NAMLEN(dirent) strlen((dirent)->d_name)

#else

#  define NAMLEN(dirent) (dirent)->d_namlen
#  if HAVE_SYS_NDIR_H
#    include <sys/ndir.h>
#  endif
#  if HAVE_SYS_DIR_H
#    include <sys/dir.h>
#  endif
#  if HAVE_NDIR_H
#    include <ndir.h>
#  endif

#endif  /* if ! HAVE_DIRENT_H */


#undef PARAMS
#if defined (__GNUC__) || __STDC__
# define PARAMS(args) args
#else
# define PARAMS(args) ()
#endif


#if ! HAVE_SCANDIR

int scandir (
	     const char *dir, 
	     struct dirent ***namelist, 
	     int (*select) PARAMS ((const struct dirent *)), 
	     int (*cmp) PARAMS ((const void *, const void *)));

#endif /* if ! HAVE_SCANDIR*/

#if ! HAVE_ALPHASORT

int alphasort (const void *a, const void *b);

#endif /* if ! HAVE_ALPHASORT*/

#endif  /* double-inclusion protection */
