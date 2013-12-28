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
 * \file luaFD_tux.c
 * \brief This file contains the definition of the Tux Lua binding.
 *
 */

#include "system.h"

#include "defs.h"
#include "struct.h"
#include "global.h"
#include "proto.h"

#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"

//////////////////////////////////////////////////////////////////////
// FDtux cfuns
//////////////////////////////////////////////////////////////////////

static int get_hp(lua_State *L)
{
	/* Check that 'self' is of the right type */
	struct tux** self = (struct tux**)luaL_testudata(L, 1, "FDtux");
	if (!self) {
		return luaL_error(L, "%s() %s", __FUNCTION__, "called with a userdata of wrong type (FDtux expected)");
	}

	lua_pushinteger(L, (int)(*self)->energy);
	return 1;
}

//////////////////////////////////////////////////////////////////////
// FDtux binding
//////////////////////////////////////////////////////////////////////

/**
 * FDtux cfuns
 */
static const luaL_Reg tux_cfuns[] = {
		{ "get_hp", get_hp },
		{ NULL, NULL }
};

/**
 * \brief Create class-type metatable for Tux lua binding
 *
 * \details This function creates and stores in the C registry a FDtux metatable
 * containing cfuns and lfuns to act on the Tux C struct.
 * cfuns are defined in the tux_cfuns array.
 * lfuns are defined in the FDtux_lfuns.lua file.
 *
 * \param L Pointer to the Lua state to use
 *
 * \return TRUE
 */
int FDtux_init(lua_State *L)
{
	// Create a metatable that will contain the Tux cfuncs and lfuncs.
	// The metatable is stored in the C registry.

	luaL_newmetatable(L, "FDtux");                                              // stack: (-1) metatable (1)
	lua_setfield(L, -1, "__index");                                             // empty stack

	// First load lfuns.
	// We use a simple trick: The metatable is, temporally, made available as a
	// global table called 'FDtux'. We then load a script that will add entries
	// into that table, using standard table access such as 'FDtux.key = value'.
	// Finally, we remove the table from the global name space.
	//
	// Note: Loading lfuns first will ensure that a lfun can not override a cfun
	// (because a cfun will overwrite a lfun with the same name).

	luaL_getmetatable(L, "FDtux");                                              // stack: (-1) metatable (1)
	lua_setglobal(L, "FDtux");                                                  // empty stack

	char fpath[2048];
	if (!find_file("FDtux_lfuns.lua", LUA_MOD_DIR, fpath, 1)) {
		if (luaL_loadfile(L, fpath)) {
			DebugPrintf(-1, "Error while loading ’%s’: %s", "FDtux_lfuns.lua", lua_tostring(L, -1));
			lua_pop(L, 1);
			ErrorMessage(__FUNCTION__, "Aborting loading FDtux lfuns.\n", NO_NEED_TO_INFORM, IS_WARNING_ONLY);
			return FALSE;
		}

		if (lua_pcall(L, 0, LUA_MULTRET, 0) != LUA_OK) {
			DebugPrintf(-1, "Error while running ’%s’: %s", "FDtux_lfuns.lua", lua_tostring(L, -1));
			lua_pop(L, 1);
			ErrorMessage(__FUNCTION__, "Aborting loading FDtux lfuns.\n", NO_NEED_TO_INFORM, IS_WARNING_ONLY);
			return FALSE;
		}
	}

	lua_pushnil(L);                                                             // stack: (-1) nil  (1)
	lua_setglobal(L, "FDtux");                                                  // empty stack

	// Now register the cfuns

	luaL_getmetatable(L, "FDtux");                                              // stack: (-1) metatable (1)
	luaL_setfuncs(L, tux_cfuns, 0);                                             // stack: (-1) metatable (1)
	lua_pop(L, 1);                                                              // stack: nil

	return TRUE;
}

/**
 * \brief Get a FDtux instance (singleton)
 *
 * \details On first call, this function creates a Lua userdata which acts as
 *  a FDtux instance: the userdata is actually a pointer to the 'Me' C struct,
 *  associated to a FDtux_class metatable.
 *  The userdata is stored in the C registry, to be returned on next calls of
 *  this function.
 *  The created FDtux instance is thus a singleton.
 *
 * \param L Pointer to the Lua state to use
 *
 * \return 1 (userdata on the Lua stack)
 */
int FDtux_get_instance(lua_State *L)
{
	// Try to get the userdata from the C registry.
	// If it does not exist, create it.

	lua_rawgetp(L, LUA_REGISTRYINDEX, (void *)&Me);

	if (lua_isnil(L, -1)) {

		/* Create a userdata which is a pointer to the 'Me' C struct, and make it be a FDtux instance */
		struct tux** userdata = (struct tux **)lua_newuserdata(L, sizeof(struct tux*));    // stack: (-1) userdata (1)
		*userdata = &Me;
		luaL_setmetatable(L, "FDtux");                                                     // stack: (-1) userdata (1)

		/* Register the userdata, using the value of the pointer to create a lightuserdata key */
		lua_pushvalue(L, -1);                                                              // stack: (-1) userdata < userdata (1)
		lua_rawsetp(L, LUA_REGISTRYINDEX, (void *)&Me);                                    // stack: (-1) userdata (1)
    }

	// Return the userdata on the Lua stack

	return 1;
}
