/* 
 *
 *   Copyright (C) 2006  Pekka Enberg
 *   Copyright (c) 2009 Arthur Huillet 
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
 * This was taken from Jato, and kindly relicensed to GPLv2+ by Pekka Enberg.
 */

#include "system.h"

#include "defs.h"
#include "struct.h"
#include "global.h"
#include "proto.h"

static int autostr_resize(struct auto_string *str, unsigned long capacity)
{
	char *buffer;

	buffer = realloc(str->value, capacity);
	if (!buffer)
		return -ENOMEM;

	str->value = buffer;
	str->capacity = capacity;

	return 0;
}

struct auto_string *alloc_autostr(int capacity)
{
	int err;
	struct auto_string *str = (struct auto_string *)MyMalloc(sizeof(struct auto_string));

	err = autostr_resize(str, capacity);
	if (err) {
		free(str);
		return NULL;
	}
	memset(str->value, 0, str->capacity);

	return str;
}

void free_autostr(struct auto_string *str)
{
	free(str->value);
	free(str);
}

static unsigned long autostr_remaining(struct auto_string *str, int offset)
{
	return str->capacity - offset;
}

static int autostr_vprintf(struct auto_string *str, unsigned long offset,
		       const char *fmt, va_list args)
{
	unsigned long size;
	va_list tmp_args;
	int nr, err = 0;

  retry:
	size = autostr_remaining(str, offset);
	if (size <= 1) { // Not enough room to write anything, resizing
		err = autostr_resize(str, str->capacity * 2);
		if (err)
			goto out;
		goto retry;

	}
	va_copy(tmp_args, args);

	// We expect a C99 compliant implementation on vsnprintf. When built with
	// NLS, we also need an implementation that understands the '$' in the
	// format string.
	// The gnulib implementation has all of this.
	//
	// Issue arises on win32:
	//
	// On non-gnulib-based system, libintl is used, redefining vsnprintf() to
	// libintl_vsnprintf(), which provides its own code to handle the '$' in
	// format string. If no '$' is found, libintl_vsnprintf() calls the libc's
	// vsnprintf().
	//
	// With mingw, depending on how the libintl was built, the vsnprintf() used
	// in libintl_vsnprintf() will be replaced by mingw's own implementation
	// (if build with -D_POSIX_C_SOURCE) or by the MS implementation (which is
	// not C99 compliant).
	//
	// As a result, on Linux or MacOS, vsnprintf() can be used without issues.
	// With mingw, well, it depends... To ensure that it works no matter how
	// mingw was built, we call internal implementations by hand. Hack !!!!
	//
	// Note: with mingw, we *have* to build with _POSIX_C_SOURCE=1

#if defined(ENABLE_NLS) && defined(USE_LIBINTL) && __MINGW32__
	if (strchr(fmt, '$') == NULL)
		nr = __mingw_vsnprintf(str->value + offset, size, fmt, tmp_args);
	else
		nr = libintl_vsnprintf(str->value + offset, size, fmt, tmp_args);
#else
	nr = vsnprintf(str->value + offset, size, fmt, tmp_args);
#endif

	va_end(tmp_args);

	if (nr < 0) {
		error_message(__FUNCTION__, "An error occurred when calling vsnprintf: %s", PLEASE_INFORM, strerror(errno));
		return -1;
	}

	if ((unsigned long)nr >= size) {
		err = autostr_resize(str, str->capacity * 2);
		if (err)
			goto out;
	
		goto retry;
	}
	str->length = offset + nr;

  out:
	return err;
}

int autostr_printf(struct auto_string *str, const char *fmt, ...)
{
	int err;
	va_list args;

	va_start(args, fmt);
	err = autostr_vprintf(str, 0, fmt, args);
	va_end(args);
	return err;
}

int autostr_vappend(struct auto_string *str, const char *fmt, va_list args)
{
	return autostr_vprintf(str, str->length, fmt, args);
}

int autostr_append(struct auto_string *str, const char *fmt, ...)
{
	int err;
	va_list args;

	va_start(args, fmt);
	err = autostr_vappend(str, fmt, args);
	va_end(args);
	return err;
}
