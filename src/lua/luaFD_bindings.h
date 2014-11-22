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
 * \file luaFD_bindings.h
 * \brief This file contains the declarations needed to create Lua bindings of
 *  some C structs.
 *
 */

/// \defgroup luaFD_bindings Lua bindings
///
/// Lua scripts (in dialogs, for instance) need a mean to interact with the
/// game's content. They need to set/get properties of game's \e entities, or
/// perform actions on them.
///
/// A game's \e entity can be related to one single C data structure. For
/// instance, the Tux \e entity is defined by the \b struct \b tux.\n
/// But it can also be a compound of several data structures. For instance,
/// the Lua NPC \e entity is associtated to a \b struct \b enemy and a
/// \b struct \b partner.
///
/// The generic name of a Lua \e entity is FD<entity>. For instance: FDtux, FDnpc...
///
/// In the following text, \b FDnpc, the Lua binding to an NPC entity is used as
/// an example.
///
/// \par Accessing a game's entity from Lua
///   \n
///   Example:
///   \code
/// local tania_npc = FDrpg.get_npc("Tania")
/// tania_npc:set_rush_tux(true)
///   \endcode
///   \n
///   On the first line, a call to \b get_npc() is used to get a reference
///   (\e 'tania_npc')to a game's \e entity (the Npc called Tania). This
///   reference is a Lua \e object, an instance of the FDnpc class.\n
///   On the second line, a property of Tania is set by calling one of the FDnpc
///   functions.\n
///   Note the use of a Lua object's notation: \b entity_reference:function_name().
///
/// \par Composition of a Lua binding
///   \n
///   Definitions:
///   - A Lua \e entity (\b FDnpc) is a Lua class, containing 2 set of functions:
///     - \b cfuns: Those functions are defined in C. They are \e low-level
///       functions, tightly coupled with the C data structures of the \e entity.
///     - \b lfuns: Those functions are defined in Lua. They are \e high-level
///       functions. They use cfuns to access the C data structures.
///   - An instance of a Lua \e entity (the 'Tania' FDnpc) is an instance of the
///     Lua class associated to one C data structure defining the object (we call
///     it the entity's \e internal \e data).
/// \par
///   The definition of the FDnpc Lua binding is split in 2 files:
///   - src/lua/luaFD_npc.c, containing:
///     - \b struct \b luaFD_npc - this structure holds the entity's \e internal
///       \e data.
///     - the \b cfuns.
///     - \b luaFD_npc_get_instance() - this function creates a FDnpc instance
///       and associate it with a \b struct \b luaFD_npc.
///     - \b luaFD_npc_init() - this function creates the FDnpc Lua class, and
///       registers all its cfuns. It also registers \b luaFD_npc_get_instance()
///       to \b FDrpg.get_npc(). It is called by the game's code when a Lua VM
///       is initialized.
///   - lua_modules/FDnpc_lfuns.lua, containing:
///     - the \b lfuns.
/// \par
///   The reference to the 2 management functions, luaFD_npc_get_instance() and
///   luaFD_npc_init(), is added into lua/luaFD_bindings.h.
///
/// \par Implementing a \e cfun
///   \n
///   - Let's start with a simple cfun, without parameters and returning nothing:
///     \b FDnpc:heal().\n
///     Its C implementation has the same name, prefixed by a '_', that is:
///     \b _heal().\n
///     \n
///     \code
/// static int _heal(lua_State * L)
/// {
///	  GET_SELF_INSTANCE_OF(struct luaFD_npc, L, "FDnpc");
///
///	  self->some_data = ...;
///
///	  return 0;
/// }
///
/// static const luaL_Reg tux_cfuns[] = {
///   ...
///   // This will bind _heal() (the C func) to FDnpc.heal() (in Lua)
///   LUAFD_CFUN(heal),
///   ...
/// };
///     \endcode
///     \n
///     The most important line is:
///     \code
/// GET_SELF_INSTANCE_OF(struct luaFD_npc, L, "FDnpc");
///     \endcode
///     \n
///     Our cfun is called using, for instance, \b tania_npc:heal().\n
///     The \b GET_SELF_INSTANCE macro first checks that 'tania_npc' is actually
///     an instance of a FDnpc (macro's third parameter), and if so it defines
///     a local variable (\b struct \b luaFD_npc \b self* - its type is the
///     macro's first parameter), which points to the \e entity's \e internal
///     \e data.\n
///     \e 'self' can then be used to set/get properties of the entity.\n
///     \n
///   - The next example is a cfun that gets one parameter: \b FDnpc:set_name(new_name).\n
///     \n
///     \code
/// static int _set_name(lua_State *L)
/// {
///   GET_SELF_INSTANCE_OF(struct luaFD_npc, L, "FDnpc");
///
///   // Get the parameter from the Lua stack
///   const char *new_name = luaL_checkstring(L, 2);
///
///   // Set NPC's new name (This is not the actual implementation !!!)
///   self->name = new_name;
///
///   return 0;
/// }
///     \endcode
///     \n
///     Function's parameters are, as usual, on the Lua stack. The first data
///     on the stack is the FDnpc instance (due to the use of the Lua object's
///     notation). Hence, we get the \e 'new_name' value from the second slot:
///     \code
/// const char *new_name = luaL_checkstring(L, 2);
///     \endcode
///     If the function has a second parameter, it will be on the third slot of
///     the Lua stack, and so on.\n
///     \n
///   - As a last example, let's see how to implement a cfun returning a value:
///     \b FDnpc:get_name().\n
///     \n
///     \code
/// static int _get_name(lua_State *L)
/// {
///   GET_SELF_INSTANCE_OF(struct luaFD_npc, L, "FDnpc");
///
///   // Get the NPC's name (This is not the actual implementation !!!)
///   char *name = self->name;
///
///   // Push the name on the Lua stack
///   lua_pushstring(L, name);
///
///   // Tell Lua that we return one single value
///   return 1;
/// }
///     \endcode
///     \n
///     That's the standard way to interface C with Lua, so it does not need any
///     special comment.
///
/// \par Implementing a \e lfun
///   \n
///   We will use, as an example, an hypothetical function that computes the
///   ratio of an Npc's health, and multiply it by a \e 'factor': \b FDnpc:get_health_ratio(factor).\n
///   \n
///   \code
/// function FDnpc.get_health_ratio(self, factor)
///   ratio = self:get_health()/self:get_max_health()
///   return ratio * factor
/// end
///   \endcode
///   \n
///   The function is defined using the standard Lua notation. As you may know,
///   the following line (using the Lua object notation)
///   \code
/// tania_health = tania_npc:get_health_ratio(100)
///   \endcode
///   is a syntactic sugar transposed to
///   \code
/// tania_health = FDnpc.get_health_ratio(tania_npc, 100)
///   \endcode
///   In this case, 'self' is set to 'tania_npc'.\n
///   \n
///   Using 'self', the function then calls some cfuns to do its job.

#ifndef _luafd_bindings_h
#define _luafd_bindings_h

#include "luaFD.h"
#include "lauxlib.h"

extern int luaFD_tux_init(lua_State *);
extern int luaFD_tux_get_instance(lua_State *);

extern int luaFD_npc_init(lua_State *);
extern int luaFD_npc_get_instance(lua_State *);

/**
 * List of binding initializers (functions creating class-type metatables)
 */
const luaL_Reg luaFD_initializers[] = {
	{ "FDtux", luaFD_tux_init },
	{ "FDnpc", luaFD_npc_init },
	{ NULL, NULL }
};

/**
 * List of functions available in the 'FDrpg' library (instance getters)
 */
const luaL_Reg luaFD_instances[] = {
		{ "get_tux", luaFD_tux_get_instance },
		{ "get_npc", luaFD_npc_get_instance },
		{ NULL, NULL }
};

#endif /* _luafd_bindings_h */
