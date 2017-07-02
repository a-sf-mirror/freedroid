/*
 *
 *   Copyright (c) 2004-2010 Arthur Huillet
 *   Copyright (c) 2011 Samuel Degrande
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
 * \file savestruct_internal.h
 * \brief Lua based save/load subsystem's definitions
 */

/// \defgroup luasaveload Lua based save/load subsystem
///
/// The save/load subsystem is based on a set of functions automatically
/// generated from the struct.h file. The saved data is a Lua script containing
/// a set of Lua tables (one table per 'root' data structure). The Lua parser is
/// used to load the saved file, and calls table constructors written in C.
///
/// A python script (gen_savestruct.py) scans the struct.h file, and creates
/// savestruct.[c,h], for each 'root' data structure (see the 'dump_structs'
/// list), as well as for the data types used by those structures.
///
/// For each structure found in struct.h, the script creates a couple of
/// functions: write_<struct>() and read_<struct>().
///
/// Those functions write or read each field of the structure, using calls to
/// write_<fieldtype>() or read_<fieldtype>().
///
/// If a field is an array, or a dynarray, then calls to
/// write|read_<fieldtype>_array() or write|read_<fieldtype>_dynarray() are
/// used. Those functions are themselves macros using write|read_<fieldtype>().
///
/// So, we end up with calls to write/read simple types (int, float, or 'user
/// defined' types, i.e. any type not defined in struct.h).
/// Those 'terminal' functions are 'hardcoded' in savestruct_internal.[c,h].
///
/// As a consequence, when a new structure is introduced in struct.h, nothing
/// as to be written in order to write or read it, apart from adding it to
/// the script's dump_structs list, in case of a 'root' data structure.
///
/// However, if a new 'terminal' type is used in a structure to be saved, the
/// functions to write/read that type have to be added in savestruct_internal.[c,h].
/// If those functions have to be defined in another file (due to access to
/// 'private' data), then they have to be declared in savestruct_internal.h
///
/// \note
/// For the python script to work, C structures and dynarrays, intended
/// to be saved, have to be typedef-ed (note the mandatory '_dynarray' post-fix).
///
/// Example:
/// \code
/// typedef struct the_struct {
/// 	...
/// } the_struct;
///
/// typedef struct dynarray the_data_dynarray;
///
/// struct struct_to_be_saved {
/// 	...
///		the_struct        my_data;
///		the_data_dynarray my_other_data;
/// }
/// \endcode
///
/// \note
/// The 'char' type must not be used for fields of structures that are to
/// be written in a savegame:
///   - for a single char, use 'uint8_t'.
///   - for an array of char containing a text, use 'string'.
///   - for an array of char containing flags, use 'uint8_t[]'.
/// This rule ensures that arrays are saved using the right encoding. The python
/// script will complain if it finds a 'char' type, and exits with failure (see
/// the 'forbidden_types' list).
/// Special case: If a field is a pointer to a static text, it must not be saved
/// (just like every other kind of pointer). Thus, it cannot be defined using the
/// 'string' type, a 'string' data being saved. It can also not be a 'char *',
/// because this is a 'forbidden' type.
/// As a trick, we introduced the 's_char' type (which is defined to be a 'char').
/// The field containing a pointer to a static text has then to be defined as
/// a 's_char *'.
///
/// \note
/// It is sometimes not possible to rely on the macros that creates
/// the _array or _dynarray write/read functions, mainly when a specific
/// behavior is needed - such as with write_char_array() (removed), for example.
/// If the write/read functions of a type have to be hardcoded, then they have
/// to be flagged as such in the python script (so that the script does not
/// auto-create them), by adding that type to the 'hardcoded_types' list.

#ifndef _savestruct_internal_h
#define _savestruct_internal_h

#include "system.h"

#include "defs.h"
#include "struct.h"
#include "global.h"
#include "proto.h"
#include "lua.h"

#include "savestruct.h"

/// \defgroup helpers Functions or macros helpers
/// \ingroup luasaveload

/**
 * Try to get a named field and output a warning if it fails.
 * \ingroup helpers
 *
 * The macro is implemented with a compound statement expression, so that it can
 * be used as a predicate in a conditional statement.
 * \param L Lua state
 * \param n Index on the stack
 * \param t Name of the field
 */

#define lua_getfield_or_warn(L, n, t)\
({ \
	lua_getfield(L, n, t);\
	int rc = !lua_isnil(L, -1);\
	if (!rc) {\
		error_once_message(ONCE_PER_GAME, __FUNCTION__, "Field \"%s\" not found", PLEASE_INFORM, t);\
		alert_once_window(ONCE_PER_GAME, _("An error occurred when trying to load the savegame.\n"\
		                                   "A common reason for this is that FreedroidRPG has been "\
		                                   "updated to a newer version since the save was made, in "\
		                                   "which case the savegame is very likely not compatible.\n"\
		                                   "If you see this message and you have not updated the game, "\
		                                   "make sure to report this to the developers.\n"\
		                                   "Thanks!"));\
		lua_pop(L, 1);\
	}\
	rc;\
})

/**
 * Check the type of a data on Lua stack, and aborts if type is wrong.
 * \ingroup helpers
 *
 * \param L Lua state
 * \param n Index on the stack
 * \param t Lua type to check against
 */

#define lua_is_of_type_or_abort(L, n, t)\
do {\
	if (lua_type(L, n) != t) {\
		error_message(__FUNCTION__, "Unexpected data type\n", PLEASE_INFORM);\
		alert_window(_("An error occurred when trying to load the savegame.\n"\
		               "A data type was found to be incompatible with the "\
		               "expected one. Your savegame could be corrupted and so "\
		               "its loading is aborted.\n"\
		               "If you see this message and you have not manually modified "\
		               "your savegame, make sure to report this to the developers.\n"\
		               "Thanks!"));\
		longjmp(saveload_jmpbuf, 1);\
	}\
} while(0)

/// \defgroup arraymacros Save/load array macros
/// \ingroup luasaveload
///
/// Functions needed to write/read arrays (or dynarrays) are defined using
/// a set of macros. The gen_savestruct python script adds them to
/// savestruct.c when it detects that the field of a structure is an
/// array (or dynarray).
/// You should thus rarely have to used them directly.

/**
 * Define a function to write an array of type X.
 * \ingroup arraymacros
 *
 * The generated function will write a Lua table containing all elements of
 * a C array.
 * \param X Data type
 */

#define define_write_xxx_array(X)\
void write_##X##_array(struct auto_string *strout, X *data, int size)\
{\
	autostr_append(strout, "{\n");\
	int i;\
	for (i = 0; i < size; i++) {\
		write_##X(strout, &data[i]);\
		autostr_append(strout, ",\n");\
	}\
	autostr_append(strout, "}\n");\
}

/**
 * Define a function to write a dynarray of type X.
 * \ingroup arraymacros
 *
 * The generated function will write a Lua table containing all elements of
 * a C dynarray.
 * \param X Data type
 */

#define define_write_xxx_dynarray(X)\
void write_##X##_dynarray(struct auto_string *strout, X##_dynarray *data)\
{\
	autostr_append(strout, "{\n");\
	int i;\
	for (i = 0; i < data->size; i++) {\
		write_##X(strout, &((X *)data->arr)[i]);\
		autostr_append(strout, ",\n");\
	}\
	autostr_append(strout, "}\n");\
}

/**
 * Define a function to write a sparse dynarray of type X.
 * \ingroup arraymacros
 *
 * The generated function will write a Lua table containing all elements of
 * a C dynarray. Only used slots are written.
 * \param X Data type
 */

#define define_write_xxx_sparse_dynarray(X)\
void write_##X##_sparse_dynarray(struct auto_string *strout, X##_sparse_dynarray *data)\
{\
	autostr_append(strout, "{\n");\
	int i;\
	for (i = 0; i < data->size; i++) {\
		if (data->used_members[i]) {\
			write_##X(strout, &((X *)data->arr)[i]);\
			autostr_append(strout, ",\n");\
		}\
	}\
	autostr_append(strout, "}\n");\
}

/**
 * Define a function to write a list of type X.
 * \ingroup arraymacros
 *
 * The generated function will write a Lua table containing all elements of
 * a C list.
 * \param X Data type
 */

#define define_write_xxx_list(X)\
void write_##X##_list(struct auto_string *strout, X##_list *data)\
{\
	autostr_append(strout, "{\n");\
	X *elt;\
	list_for_each_entry(elt, data, node) {\
		write_##X(strout, elt);\
		autostr_append(strout, ",\n");\
	}\
	autostr_append(strout, "}\n");\
}

/**
 * Define a function to read an array of type X.
 * \ingroup arraymacros
 *
 * The generated function will read a Lua table, and store each read element
 * in a C array.
 * \param X Data type
 */

#define define_read_xxx_array(X)\
void read_##X##_array(lua_State *L, int index, X *result, int array_size)\
{\
	lua_is_of_type_or_abort(L, index, LUA_TTABLE);\
	int i;\
	for (i = 0; i < lua_rawlen(L, -1) && i < array_size; i++) {\
		lua_rawgeti(L, index, i+1);\
		read_##X(L, -1, &result[i]);\
		lua_pop(L, 1);\
	}\
}

/**
 * Define a function to read a dynarray of type X.
 * \ingroup arraymacros
 *
 * The generated function will read a Lua table, and append each read element
 * into a C dynarray.
 * \param X Data type
 */

#define define_read_xxx_dynarray(X)\
void read_##X##_dynarray(lua_State *L, int index, X##_dynarray *result)\
{\
	lua_is_of_type_or_abort(L, index, LUA_TTABLE);\
	int array_size = lua_rawlen(L, index);\
	if (array_size != 0) {\
		dynarray_init((struct dynarray *)result, array_size, sizeof(X));\
		int i;\
		X data;\
		for (i = 0; i < array_size; i++) {\
			lua_rawgeti(L, index, i+1);\
			read_##X(L, -1, &data);\
			dynarray_add((struct dynarray *)result, &data, sizeof(X));\
			lua_pop(L, 1);\
		}\
	} else {\
		dynarray_init((struct dynarray *)result, 0, sizeof(X));\
	}\
}

/**
 * Define a function to read a sparse dynarray of type X.
 * \ingroup arraymacros
 *
 * The generated function will read a Lua table, and append each read element
 * into a C dynarray. The array is initialized as being a sparse dynarray.
 * \param X Data type
 */

#define define_read_xxx_sparse_dynarray(X)\
void read_##X##_sparse_dynarray(lua_State *L, int index, X##_sparse_dynarray *result)\
{\
	lua_is_of_type_or_abort(L, index, LUA_TTABLE);\
	int array_size = lua_rawlen(L, index);\
	if (array_size != 0) {\
		sparse_dynarray_init((struct sparse_dynarray *)result, array_size, sizeof(X));\
		int i;\
		X data;\
		for (i = 0; i < array_size; i++) {\
			lua_rawgeti(L, index, i+1);\
			read_##X(L, -1, &data);\
			sparse_dynarray_add((struct sparse_dynarray *)result, &data, sizeof(X));\
			lua_pop(L, 1);\
		}\
	} else {\
		sparse_dynarray_init((struct sparse_dynarray *)result, 0, sizeof(X));\
	}\
}

/**
 * Define a function to read a list of type X.
 * \ingroup arraymacros
 *
 * The generated function will read a Lua table, and add each read element
 * on the tail of a C list.
 * \param X Data type
 */

#define define_read_xxx_list(X)\
void read_##X##_list(lua_State *L, int index, X##_list *result)\
{\
	lua_is_of_type_or_abort(L, index, LUA_TTABLE);\
	int array_size = lua_rawlen(L, index);\
	INIT_LIST_HEAD(result);\
	if (array_size != 0) {\
		for (int i = 0; i < array_size; i++) {\
			lua_rawgeti(L, index, i+1);\
			X *data = MyMalloc(sizeof(X));\
			read_##X(L, -1, data);\
			list_add_tail(&data->node, result);\
			lua_pop(L, 1);\
		}\
	}\
}

/// \defgroup simplerw Read/write of simple types
/// \ingroup luasaveload
///
/// Functions used to read and write simple types

void read_uint8_t(lua_State *L, int index, uint8_t *data);
void write_uint8_t(struct auto_string *strout, uint8_t *data);
void read_uint16_t(lua_State *L, int index, uint16_t *data);
void write_uint16_t(struct auto_string *strout, uint16_t *data);
void read_int16_t(lua_State *L, int index, int16_t *data);
void write_int16_t(struct auto_string *strout, int16_t *data);
void read_uint32_t(lua_State *L, int index, uint32_t *data);
void write_uint32_t(struct auto_string *strout, uint32_t *data);
void read_int32_t(lua_State *L, int index, int32_t *data);
void write_int32_t(struct auto_string *strout, int32_t *data);
void read_float(lua_State *L, int index, float *data);
void write_float(struct auto_string *strout, float *data);
void read_double(lua_State *L, int index, double *data);
void write_double(struct auto_string *strout, double *data);
void read_string(lua_State *L, int index, string *data);
void write_string(struct auto_string *strout, string *data);

/// \defgroup userrw Read/write of 'user' types
/// \ingroup luasaveload
///
/// Functions used to read and write 'user' types.
/// 'User' types are compound types or C structures not defined in struct.h, and
/// thus needing hardcoded read/write functions.

void read_luacode(lua_State *L, int index, luacode *data);
void write_luacode(struct auto_string *strout, luacode *data);
void read_SDL_Rect(lua_State *L, int index, SDL_Rect *data);
void write_SDL_Rect(struct auto_string *strout, SDL_Rect *data);
void read_automap_data_t(lua_State *L, int index, automap_data_t *data);
void write_automap_data_t(struct auto_string *strout, automap_data_t *data);
void read_list_head_t(lua_State *L, int index, list_head_t *data);
void write_list_head_t(struct auto_string *strout, list_head_t *data);
void read_game_config(lua_State *L, int index);
void write_game_config(struct auto_string *strout);

/// \defgroup overloadrw Overloaded read/write functions
/// \ingroup luasaveload
///
/// Some types can not be saved using the 'standard' scheme, and need specific
/// read/write functions.

void read_keybind_t_array(lua_State *L, int index, keybind_t *result, int size);
void write_keybind_t_array(struct auto_string *strout, keybind_t *data, int size);
void write_event_triggers_dynarray(struct auto_string *);
void read_event_triggers_dynarray(lua_State *, int);

/// \defgroup externalrw Declaration of external read/write functions
/// \ingroup luasaveload
///
/// Some read/write functions use private data defined outside of the saveload
/// subsystem. They however have to be declared here.

/**
 * Read a 'faction'.
 * \ingroup externalrw
 *
 * \param L           Current Lua State
 * \param index       Lua stack index of the data
 * \param faction_idx Pointer to the index of the faction to be read (a pointer is used
 *                    for compatibility with standard read function's signature)
 */
void read_faction(lua_State *L, int index, int *faction_idx);

/**
 * Write a 'faction'.
 * \ingroup externalrw
 *
 * \param strout The auto_string to be filled
 * \param faction_idx Pointer to the index of the faction to be saved (a pointer is used
 *                    for compatibility with standard write function's signature)
 */
void write_faction(struct auto_string *strout, int *faction_idx);

/// \defgroup toprw 'Root' save/load functions
/// \ingroup luasaveload
///
/// Functions used by the game's core to save/load a whole
/// set of data.

void save_game_data(struct auto_string *strout);
void load_game_data(char *strin);
void save_freedroid_configuration(struct auto_string *strout);
void load_freedroid_configuration(char *strin);

#endif // _savestruct_internal_h
