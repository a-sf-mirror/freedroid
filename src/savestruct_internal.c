/*
 *
 *   Copyright (c) 2009-2010 Arthur Huillet
 *   Copyright (c) 2011 Samuel Degrande
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
/**
 * \file savestruct_internal.c
 * \brief Lua based save/load subsystem's definitions
 */

#include "savestruct_internal.h"
#include "lauxlib.h"

#include "build_flags.h"

/**
 * Macro used to define the write function of a numeric data type.
 * \ingroup simplerw
 *
 * \param X The data type
 */
#define define_read_xxx(X)\
void read_##X(lua_State *L, int index, X *data)\
{\
	lua_is_of_type_or_abort(L, index, LUA_TNUMBER);\
	*((X *)data) = (X)lua_tonumber(L, index);\
}

/**
 * Macro used to define the read function of a numeric data type.
 * \ingroup simplerw
 *
 * \param X The data type
 * \param Y The 'printf' format of the data type
 */
#define define_write_xxx(X, Y)\
void write_##X(struct auto_string *strout, X *data)\
{\
	autostr_append(strout, Y, *(data));\
}

/**
 * \fn void read_uint8_t(lua_State *L, int index, uint8_t *data)
 *  Read a uint8_t data from the Lua stack.
 * \ingroup simplerw
 *
 * \param L     Current Lua State
 * \param index Lua stack index of the data
 * \param data  Pointer to the resulting data storage
 */
define_read_xxx(uint8_t);

 /**
  * \fn void write_uint8_t(struct auto_string *strout, uint8_t *data)
  * Write a uint8_t data into an auto_string.
  * \ingroup simplerw
  *
  * \param strout The auto_string to be filled
  * \param data   Pointer to the data to write
  */
define_write_xxx(uint8_t, "%hhu");

/**
 * \fn void read_uint16_t(lua_State *L, int index, uint16_t *data)
 * Read a uint16_t data from the Lua stack.
 * \ingroup simplerw
 *
 * \param L     Current Lua State
 * \param index Lua stack index of the data
 * \param data  Pointer to the resulting data storage
 */
define_read_xxx(uint16_t);

/**
 * \fn void write_uint16_t(struct auto_string *strout, uint16_t *data)
 * Write a uint16_t data into an auto_string.
 * \ingroup simplerw
 *
 * \param strout The auto_string to be filled
 * \param data   Pointer to the data to write
 */
define_write_xxx(uint16_t, "%hu");

/**
 * \fn void read_int16_t(lua_State *L, int index, int16_t *data)
 * Read a int16_t data from the Lua stack.
 * \ingroup simplerw
 *
 * \param L     Current Lua State
 * \param index Lua stack index of the data
 * \param data  Pointer to the resulting data storage
 */
define_read_xxx(int16_t);

/**
 * \fn void write_int16_t(struct auto_string *strout, int16_t *data)
 * Write a int16_t data into an auto_string.
 * \ingroup simplerw
 *
 * \param strout The auto_string to be filled
 * \param data   Pointer to the data to write
 */
define_write_xxx(int16_t, "%hd");

/**
 * \fn void read_uint32_t(lua_State *L, int index, uint32_t *data)
 * Read a uint32_t data from the Lua stack.
 * \ingroup simplerw
 *
 * \param L     Current Lua State
 * \param index Lua stack index of the data
 * \param data  Pointer to the resulting data storage
 */

define_read_xxx(uint32_t);

/**
 * \fn void write_uint32_t(struct auto_string *strout, uint32_t *data)
 * Write a uint32_t data into an auto_string.
 * \ingroup simplerw
 *
 * \param strout The auto_string to be filled
 * \param data   Pointer to the data to write
 */
define_write_xxx(uint32_t, "%u");

/**
 * \fn void read_int32_t(lua_State *L, int index, int32_t *data)
 * Read a int32_t data from the Lua stack.
 * \ingroup simplerw
 *
 * \param L     Current Lua State
 * \param index Lua stack index of the data
 * \param data  Pointer to the resulting data storage
 */
define_read_xxx(int32_t);

/**
 * \fn void write_int32_t(struct auto_string *strout, int32_t *data)
 * Write a int32_t data into an auto_string.
 * \ingroup simplerw
 *
 * \param strout The auto_string to be filled
 * \param data   Pointer to the data to write
 */
define_write_xxx(int32_t, "%d");

/**
 * \fn void read_float(lua_State *L, int index, float *data)
 * Read a float data from the Lua stack.
 * \ingroup simplerw
 *
 * \param L     Current Lua State
 * \param index Lua stack index of the data
 * \param data  Pointer to the resulting data storage
 */
define_read_xxx(float);

/**
 * \fn void write_float(struct auto_string *strout, float *data)
 * Write a float data into an auto_string.
 * \ingroup simplerw
 *
 * \param strout The auto_string to be filled
 * \param data   Pointer to the data to write
 */
define_write_xxx(float, "%f");

/**
 * \fn void read_double(lua_State *L, int index, double *data)
 * Read a double data from the Lua stack.
 * \ingroup simplerw
 *
 * \param L     Current Lua State
 * \param index Lua stack index of the data
 * \param data  Pointer to the resulting data storage
 */
define_read_xxx(double);

/**
 * \fn void write_double(struct auto_string *strout, double *data)
 * Write a double data into an auto_string.
 * \ingroup simplerw
 *
 * \param strout The auto_string to be filled
 * \param data   Pointer to the data to write
 */
define_write_xxx(double, "%lf");

/**
 * Read a string (i.e a null terminated array of char) data from the Lua stack.
 * \ingroup simplerw
 *
 * \param L     Current Lua State
 * \param index Lua stack index of the data
 * \param data  Pointer to the resulting data storage
 */
void read_string(lua_State *L, int index, string *data)
{
	lua_is_of_type_or_abort(L, index, LUA_TSTRING);
	char *str = (char *)lua_tostring(L, index);
	*data = NULL;
	if (strlen(str)) {
		*data = strdup(lua_tostring(L, index));
	}
}

/**
 * Write a string (i.e a null terminated array of char) data into an auto_string.
 * \ingroup simplerw
 *
 * \param strout The auto_string to be filled
 * \param data   Pointer to the data to write
 */
void write_string(struct auto_string *strout, string *data)
{
	autostr_append(strout, "\"%s\"", (*data) ? *data : "");
}

/**
 * Read a lua code.
 * \ingroup userrw
 *
 * A Lua code is a string, saved using a 'long string' format due to presence
 * of quotes in the string.
 * \param L     Current Lua State
 * \param index Lua stack index of the data
 * \param data  Pointer to the resulting data storage
 */
void read_luacode(lua_State *L, int index, luacode *data)
{
	read_string(L, index, data);
}

/**
 * Write a lua code.
 * \ingroup userrw
 *
 * A Lua code is a string, saved using a 'long string' format due to presence
 * of quotes in the string.
 * \param strout The auto_string to be filled
 * \param data   Pointer to the data to write
 */
void write_luacode(struct auto_string *strout, luacode *data)
{
	autostr_append(strout, "[=[ %s ]=]", (*data) ? *data : "");
}

/**
 * Read a SDL Rect.
 * \ingroup userrw
 *
 * \param L     Current Lua State
 * \param index Lua stack index of the data
 * \param data  Pointer to the resulting data storage
 */
void read_SDL_Rect(lua_State *L, int index, SDL_Rect *data)
{
	if (lua_getfield_or_warn(L, index, "x")) {
		read_int16_t(L, -1, &data->x);
		lua_pop(L, 1);
	}
	if (lua_getfield_or_warn(L, index, "y")) {
		read_int16_t(L, -1, &data->y);
		lua_pop(L, 1);
	}
	if (lua_getfield_or_warn(L, index, "w")) {
		read_uint16_t(L, -1, &data->w);
		lua_pop(L, 1);
	}
	if (lua_getfield_or_warn(L, index, "h")) {
		read_uint16_t(L, -1, &data->h);
		lua_pop(L, 1);
	}
}

/**
 * Write a SDL Rect.
 * \ingroup userrw
 *
 * \param strout The auto_string to be filled
 * \param data   Pointer to the data to write
 */
void write_SDL_Rect(struct auto_string *strout, SDL_Rect *data)
{
    autostr_append(strout, "{\n");
    autostr_append(strout, "x = ");
    write_int16_t(strout, &data->x);
    autostr_append(strout, ",\n");
    autostr_append(strout, "y = ");
    write_int16_t(strout, &data->y);
    autostr_append(strout, ",\n");
    autostr_append(strout, "w = ");
    write_uint16_t(strout, &data->w);
    autostr_append(strout, ",\n");
    autostr_append(strout, "h = ");
    write_uint16_t(strout, &data->h);
    autostr_append(strout, ",\n");
    autostr_append(strout, "}");
}

/**
 * Read an automap.
 * \ingroup userrw
 *
 * An automap is a 100x100 array of bytes (actually, these are 'chars' used to
 * contain byte values). To speed-up the reading of an automap, it is saved as
 * 100 strings of chars.
 * \param L     Current Lua State
 * \param index Lua stack index of the data
 * \param data  Pointer to the resulting data storage
 */
void read_automap_data_t(lua_State *L, int index, automap_data_t *data)
{
	// See write_automap_data_t for more information about the conversion
	// from ascii char value to byte value.
	lua_is_of_type_or_abort(L, index, LUA_TTABLE);
	int i, j;
	for (i = 0; i < 100; i++) {
		lua_rawgeti(L, index, i+1);
		lua_is_of_type_or_abort(L, -1, LUA_TSTRING);
		char *line = (char *)lua_tostring(L, -1);
		for (j = 0; j < 100; j++) {
			(*data)[i][j] = (char)(line[j] - '0');
		}
		lua_pop(L, 1);
	}
}

/**
 * Write an automap.
 * \ingroup userrw
 *
 * An automap is a 100x100 array of bytes (actually, these are 'chars' used to
 * contain byte values). To speed-up the reading of an automap, it is saved as
 * 100 strings of chars.
 * \param strout The auto_string to be filled
 * \param data   Pointer to the data to write
 */
void write_automap_data_t(struct auto_string *strout, automap_data_t *data)
{
	// Each byte value is converted to a printable ascii char by adding a '0'
	// ascii value (48). This only works as long as the resulting value is a
	// printable one (< 127). The byte value must then be less than 79.
	// TODO : do we abort or do we clamp, in such a case ??? Or do we use
	// a base64-like representation ?

	int i, j;
	autostr_append(savestruct_autostr, "{\n");
	for (i = 0; i < 100; i++) {
		char line[101];
		for (j = 0; j < 100; j++)
			line[j] = (char)((*data)[i][j] + '0');
		line[100] = '\0';
		autostr_append(strout, "\"%s\",\n", line);
	}
	autostr_append(strout, "}");
}

/**
 * Read a list head.
 * \ingroup userrw
 *
 * Lists are not saved, so this function is void.
 * \param L     Current Lua State
 * \param index Lua stack index of the data
 * \param data  Pointer to the resulting data storage
 */
void read_list_head_t(lua_State *L, int index, list_head_t *data)
{
	return;
}

/**
 * Write a list head.
 * \ingroup userrw
 *
 * Lists are not saved, so this function is void.
 * \param strout The auto_string to be filled
 * \param data   Pointer to the data to write
 */
void write_list_head_t(struct auto_string *strout, list_head_t *data)
{
	// Even if we do not read list_head_t data, the save subsystem will
	// output a "tag_name =" before to call this function.
	// So in order to follow Lua grammar, we have to also output a data
	autostr_append(strout, "0");
}

/**
 * Read a char array.
 * \ingroup overloadrw
 *
 * In order to improve read performances, a char array is saved as a Lua string.
 * \param L     Current Lua State
 * \param index Lua stack index of the data
 * \param data  Pointer to the resulting data storage
 * \param size  Size of the array (max number of value to read)
 */


/**
 * Read a keybind_t array.
 * \ingroup overloadrw
 *
 * The keybind array has to be NULL terminated.
 * \param L     Current Lua State
 * \param index Lua stack index of the data
 * \param data  Pointer to the resulting data storage
 * \param size  Size of the array (max number of value to read)
 */
void read_keybind_t_array(lua_State *L, int index, keybind_t *data, int size)
{
	lua_is_of_type_or_abort(L, index, LUA_TTABLE);
	keybind_t keybind;
	int i;
	for (i = 0; i < lua_rawlen(L, -1); i++) {
		lua_rawgeti(L, index, i+1);
		read_keybind_t(L, -1, &keybind);
		input_set_keybind(keybind.name, keybind.key, keybind.mod);
		free(keybind.name);
		lua_pop(L, 1);
	}
}

/**
 * Write a keybind_t array.
 * \ingroup overloadrw
 *
 * The array is NULL terminated, so there is no need to save the whole array.
 * \param strout The auto_string to be filled
 * \param data   Pointer to the data to write
 * \param size   Size of the array
 */
void write_keybind_t_array(struct auto_string *strout, keybind_t *data, int size)
{
	autostr_append(strout, "{\n");
	int i;
	for (i = 0; i < size; i++) {
		if (data[i].name == NULL)
			break;
		write_keybind_t(strout, &data[i]);
		autostr_append(strout, ",\n");
	}
	autostr_append(strout, "}");
}

/**
 * Save game data.
 * \ingroup toprw
 *
 * Save current state of the game (apart from level maps) in a Lua script
 * file.
 * \param strout The auto_string to be filled
 */
void save_game_data(struct auto_string *strout)
{
	enemy *erot;
	npc *n;
	int i;

	autostr_append(strout, "--[[\n");
	autostr_append(strout, "BUILD_CFLAGS: %s\n", BUILD_CFLAGS);
	autostr_append(strout, "BUILD_LDFLAGS: %s\n", BUILD_LDFLAGS);
	autostr_append(strout, "VERSION: %s\n", freedroid_version);
	autostr_append(strout, "--]]\n");

	write_lua_variables(strout);

	autostr_append(strout, "tux_t");
	write_tux_t(strout, &Me);
	autostr_append(strout, "\n");

	list_for_each_entry_reverse(erot, &dead_bots_head, global_list) {
		autostr_append(strout, "dead_enemy");
		write_enemy(strout, erot);
		autostr_append(strout, "\n");
	}

	list_for_each_entry_reverse(erot, &alive_bots_head, global_list) {
		autostr_append(strout, "alive_enemy");
		write_enemy(strout, erot);
		autostr_append(strout, "\n");
	}

	list_for_each_entry(n, &npc_head, node) {
		autostr_append(strout, "npc");
		write_npc(strout, n);
		autostr_append(strout, "\n");
	}

	autostr_append(strout, "bullet_array");
	write_bullet_array(strout, AllBullets, MAXBULLETS);
	autostr_append(strout, "\n");

	autostr_append(strout, "blast_array");
	write_blast_array(strout, AllBlasts, MAXBLASTS);
	autostr_append(strout, "\n");

	autostr_append(strout, "spell_active_array");
	write_spell_active_array(strout, AllActiveSpells, MAX_ACTIVE_SPELLS);
	autostr_append(strout, "\n");

	autostr_append(strout, "melee_shot_array");
	write_melee_shot_array(strout, AllMeleeShots, MAX_MELEE_SHOTS);
	autostr_append(strout, "\n");

	autostr_append(strout, "factions{\n");
	for (i = 0; i < FACTION_NUMBER_OF_FACTIONS; i++)
		write_faction(strout, &i);
	autostr_append(strout, "}\n");
}

/*
 * Private table constructors, used when loading a savegame.
 */

static int tux_ctor(lua_State *L)
{
	read_tux_t(L, 1, &Me);
	return 0;
}

static int alive_enemy_ctor(lua_State *L)
{
	enemy *newen = enemy_new(0);
	free(newen->short_description_text);
	read_enemy(L, 1, newen);
	enemy_insert_into_lists(newen, TRUE);

	return 0;
}

static int dead_enemy_ctor(lua_State *L)
{
	enemy *newen = enemy_new(0);
	free(newen->short_description_text);
	read_enemy(L, 1, newen);
	enemy_insert_into_lists(newen, FALSE);

	return 0;
}

static int npc_ctor(lua_State *L)
{
	struct npc *newnpc = (struct npc *)MyMalloc(sizeof(struct npc));
	read_npc(L, 1, newnpc);
	npc_insert(newnpc);

	return 0;
}

static int bullet_array_ctor(lua_State *L)
{
	int i;
	for (i = 0; i < lua_rawlen(L, 1) && i < MAXBULLETS; i++) {
		lua_rawgeti(L, 1, i+1);
		read_bullet(L, -1, &AllBullets[i]);
		lua_pop(L, 1);
	}

	return 0;
}

static int blast_array_ctor(lua_State *L)
{
	int i;
	for (i = 0; i < lua_rawlen(L, 1) && i < MAXBLASTS; i++) {
		lua_rawgeti(L, 1, i+1);
		read_blast(L, -1, &AllBlasts[i]);
		lua_pop(L, 1);
	}

	return 0;
}

static int spell_active_array_ctor(lua_State *L)
{
	int i;
	for (i = 0; i < lua_rawlen(L, 1) && i < MAX_ACTIVE_SPELLS; i++) {
		lua_rawgeti(L, 1, i+1);
		read_spell_active(L, -1, &AllActiveSpells[i]);
		lua_pop(L, 1);
	}

	return 0;
}


static int melee_shot_array_ctor(lua_State *L)
{
	int i;
	for (i = 0; i < lua_rawlen(L, 1) && i < MAX_MELEE_SHOTS; i++) {
		lua_rawgeti(L, 1, i+1);
		read_melee_shot(L, -1, &AllMeleeShots[i]);
		lua_pop(L, 1);
	}

	return 0;
}

static int factions_ctor(lua_State *L)
{
	int i;
	for (i = 0; i < FACTION_NUMBER_OF_FACTIONS; i++) {
		read_faction(L, 1, &i);
	}

	return 0;
}

/**
 * Load game data.
 * \ingroup toprw
 *
 * Load all data defining the current state of a game (apart from level maps),
 * stored in a Lua script file.
 * \param strin Char array containing the data to load
 */
void load_game_data(char *strin)
{
	// Add the table constructors called by Lua when parsing a savegame

	luaL_Reg lfuncs[] = {
		{"tux_t", tux_ctor},
		{"dead_enemy", dead_enemy_ctor},
		{"alive_enemy", alive_enemy_ctor},
		{"npc", npc_ctor},
		{"bullet_array", bullet_array_ctor},
		{"blast_array", blast_array_ctor},
		{"spell_active_array", spell_active_array_ctor},
		{"melee_shot_array", melee_shot_array_ctor},
		{"factions", factions_ctor},
		{NULL, NULL}
	};

	lua_State *L = get_lua_state(LUA_DIALOG);

	int i;
	for (i = 0; lfuncs[i].name != NULL; i++) {
		lua_pushcfunction(L, lfuncs[i].func);
		lua_setglobal(L, lfuncs[i].name);
	}

	// Parse the configuration file, calling table constructors to create the
        // associated C data structures

	run_lua(LUA_DIALOG, strin);
}

/**
 * Save configuration data.
 * \ingroup toprw
 *
 * Save a FreedroidRPG configuration in a Lua script file.
 * \param strout The auto_string to be filled
 */
void save_freedroid_configuration(struct auto_string *strout)
{
	autostr_append(strout, "configuration");
	write_configuration_for_freedroid(strout, &GameConfig);
	autostr_append(strout, "\n");
}

/*
 * Private table constructors, used when loading a FreedroidRPG configuration.
 */

static int configuration_ctor(lua_State *L)
{
	read_configuration_for_freedroid(L, 1, &GameConfig);

	return 0;
}

/**
 * Load configuration data.
 * \ingroup toprw
 *
 * Load a FreedroidRPG configuration stored in a Lua script file.
 * \param strin Char array containing the data to load
 */
void load_freedroid_configuration(char *strin)
{
	// Add the table constructors called by Lua when parsing the configuration file

	luaL_Reg lfuncs[] = {
		{"configuration", configuration_ctor},
		{NULL, NULL}
	};
	
	lua_State *L = get_lua_state(LUA_CONFIG);

	int i;
	for (i = 0; lfuncs[i].name != NULL; i++) {
		lua_pushcfunction(L, lfuncs[i].func);
		lua_setglobal(L, lfuncs[i].name);
	}

	// Parse the configuration file, calling table constructors to create the
        // associated C data structures

	run_lua(LUA_CONFIG, strin);
}
