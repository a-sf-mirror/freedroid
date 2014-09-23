/* 
 *
 *   Copyright (c) 2002, 2003  Reinhard Prix
 *   Copyright (c) 2004-2007 Arthur Huillet 
 *
 *
 *  This file is part of Freedroid
 *
 *  Freedroid is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  Freedroid is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with Freedroid; see the file COPYING. If not, write to the 
 *  Free Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, 
 *  MA  02111-1307  USA
 *
 */

/* 
 * system.h: here we include all system-wide includes
 *           and take into account the AC-defined conditionals
 */

#ifndef _SYSTEM_H
#define _SYSTEM_H

#include "config.h"

#include <stdio.h>
#include <math.h>

#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif
#ifdef HAVE_STRING_H
#include <string.h>
#endif

#ifdef HAVE_STDINT_H
#include <stdint.h>
#endif

#ifdef HAVE_TIME_H
#include <time.h>
#else
#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>
#endif
#endif

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif

#include <signal.h>

#ifdef HAVE_SYS_SOUNDCARD_H
#ifdef __OpenBSD__
#include <soundcard.h>
#else
#include <sys/soundcard.h>
#endif
#endif
#ifdef HAVE_SYS_IOCTL_H
#include <sys/ioctl.h>
#endif

#ifdef HAVE_FCNTL_H
#include <fcntl.h>
#endif

//#ifdef HAVE_LOCALE_H
//#include <locale.h>
//#endif

#ifdef HAVE_ICONV
#include <iconv.h>
#endif

#ifdef HAVE_LIBGEN_H
#include <libgen.h>
#endif

#include <errno.h>
#include <stdarg.h>
#include <ctype.h>

#include <setjmp.h>

#ifdef HAVE_DIRENT_H
#include <dirent.h>
#endif

#include <SDL.h>
#include <SDL_image.h>

#ifdef HAVE_LIBSDL_GFX
#include <SDL_rotozoom.h>
#include <SDL_framerate.h>
#include <SDL_gfxPrimitives.h>
#endif

#ifdef HAVE_LIBSDL_MIXER
#include <SDL_mixer.h>
#endif

#ifdef HAVE_LIBGL
#include <SDL_opengl.h>
#endif				/* HAVE_LIBGL */

#include "lang.h"

#endif				/* double-inclusion protection */
