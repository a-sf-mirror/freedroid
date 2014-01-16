/*
 *
 *  Copyright (c) 2013 Samuel Degrande
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

/**
 * \file luaFD.h
 */

#ifndef _luafd_h
#define _luafd_h

#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"

#if DOXYGEN
	#define LUAFD_DOC(...) __VA_ARGS__;
#else
	#define LUAFD_DOC(...)
#endif

#define LUAFD_CFUN(name) { #name, _##name }

#define GET_SELF_INSTANCE_OF(type, L, class) \
	type ** self = (type **)luaL_testudata(L, 1, class); \
	if (!self) { \
		return luaL_error(L, "%s() called with a userdata of wrong type (%s expected)", __FUNCTION__, class); \
	}

#endif /* _luafd_h */
