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
 * \file luaFD.c
 * \brief This file contains the functions needed to create the 'FDrpg' lua library
 *
 */

#include "defs.h"

#include "luaFD_bindings.h"

/**
 * \brief Create the class-type metatables of the Lua bindings
 *
 * \param L Pointer to the Lua state to use
 *
 * \return 1 (with the library on the Lua stack)
 */
static int luaopen_FDrpg(lua_State *L)
{
	// Create all metatables

	const luaL_Reg *initializer;
	for (initializer = luaFD_initializers; initializer->name; initializer++) {
		initializer->func(L);
	}

	// Create the lib containing all the instance getters and put it on the Lua stack

	luaL_newlib(L, luaFD_instances);                                            // stack: (-1) lib (1)

	return 1;
}

/**
 * \brief Create and load the FDrpg library
 *
 * \param L Pointer to the Lua state to use
 *
 * \return TRUE
 */
int luaFD_init(lua_State *L)
{
	luaL_requiref(L, "FDrpg", luaopen_FDrpg, 1);
	lua_pop(L, 1);

	return TRUE;
}
