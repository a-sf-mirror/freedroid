/* 
 *
 *   Copyright (c) 2010 Arthur Huillet
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
 * This file provides faction handling: tell whether two factions are hostile to each other,
 * change this parameter, etc.
 */

#define _faction_c 1

#include "system.h"

#include "defs.h"
#include "struct.h"
#include "proto.h"
#include "global.h"
#include "savestruct.h"

static int hostility_matrix[FACTION_NUMBER_OF_FACTIONS][FACTION_NUMBER_OF_FACTIONS];

static struct {
	enum faction_id id;
	const char *name;
} factions[] = {
		{ FACTION_SELF, "self" },
		{ FACTION_BOTS, "ms" },
		{ FACTION_REDGUARD, "redguard" },
		{ FACTION_RESISTANCE, "resistance" },
		{ FACTION_CIVILIAN, "civilian" },
		{ FACTION_CRAZY, "crazy" },
		{ FACTION_SINGULARITY, "singularity"},
		{ FACTION_NEUTRAL, "neutral"},
		{ FACTION_TEST, "test"} // extra faction for level 24, behaves neutral
};

/**
  * Returns the numerical ID corresponding to the name of a faction given as parameter.
  */
enum faction_id get_faction_id(const char *name) 
{
	int i;

	for (i = 0; i < sizeof(factions)/sizeof(factions[0]); i++) {
		if (!strcmp(factions[i].name, name))
			return factions[i].id;
	}

	error_message(__FUNCTION__, "Faction name %s does not exist.", PLEASE_INFORM, name);
	return FACTION_SELF;
}

/**
 * Returns the name of a faction based on a given id
 * @param faction_id Position of the faction in the factions enum
 * @return Name of the faction
 */
const char *get_faction_from_id(int faction_id)
{
	if ((faction_id < 0) || (faction_id >= (sizeof(factions) / sizeof(factions[0])))) {
		error_message(__FUNCTION__, "Malformed faction id! Faction %d does not exist.", PLEASE_INFORM, faction_id);
		return factions[FACTION_SELF].name;
	}

	return factions[faction_id].name;
}

/**
  * Set the hostility parameter between two factions. This function takes care of the symmetry: two factions always have
  * the same state towards each other, there can be no friendly one way/hostile the other way situation.
  */
void set_faction_state(enum faction_id fact1, enum faction_id fact2, enum faction_state state)
{
	/* Do not make a faction hostile to itself */
	if (fact1 == fact2 && state == HOSTILE) {
		return;
	}

	hostility_matrix[fact1][fact2] = state;
	hostility_matrix[fact2][fact1] = state;
}

/**
  * Query the hostility matrix: are the two factions passed as parameter friendly?
  * @return 1 if friendly, 0 if hostile
  */
int is_friendly(enum faction_id fact1, enum faction_id fact2)
{
	return (hostility_matrix[fact1][fact2] == FRIENDLY);
}

/**
  * Fill in faction hostility matrix with the game default values.
  */
void init_factions()
{
	int i;

	/* The "selfish human" faction is the player's faction.
	   It is friendly to everyone and hostile towards bots.
	 */ 
	for (i = 0; i < FACTION_NUMBER_OF_FACTIONS; i++) {
		set_faction_state(FACTION_SELF, i, FRIENDLY);
	}
	set_faction_state(FACTION_SELF, FACTION_BOTS, HOSTILE);

	/* The "civilian" faction represents all normally non-fighting
	   characters.
	   It is friendly to everyone and hostile towards bots.
	 */ 
	for (i = 0; i < FACTION_NUMBER_OF_FACTIONS; i++) {
		set_faction_state(FACTION_CIVILIAN, i, FRIENDLY);
	}
	set_faction_state(FACTION_CIVILIAN, FACTION_BOTS, HOSTILE);
	
	/* Crazy faction is for people that were pissed off by tux. They are civilians that are hostile to Tux. */
	for (i = 0; i < FACTION_NUMBER_OF_FACTIONS; i++) {
		set_faction_state(FACTION_CRAZY, i, FRIENDLY);
	}
	set_faction_state(FACTION_CRAZY, FACTION_BOTS, HOSTILE);
	set_faction_state(FACTION_CRAZY, FACTION_SELF, HOSTILE);

	/* The "red guard" faction is hostile to the resistance and bots, friendly towards the rest. */
	for (i = 0; i < FACTION_NUMBER_OF_FACTIONS; i++) {
		set_faction_state(FACTION_REDGUARD, i, FRIENDLY);
	}
	set_faction_state(FACTION_REDGUARD, FACTION_BOTS, HOSTILE);
	set_faction_state(FACTION_REDGUARD, FACTION_RESISTANCE, HOSTILE);

	/* Resistance is similar to red guards. */
	for (i = 0; i < FACTION_NUMBER_OF_FACTIONS; i++) {
		set_faction_state(FACTION_RESISTANCE, i, FRIENDLY);
	}	   
	set_faction_state(FACTION_RESISTANCE, FACTION_BOTS, HOSTILE);
	set_faction_state(FACTION_RESISTANCE, FACTION_REDGUARD, HOSTILE);

	/* MegaSys hate everyone, and everything, except perhaps their shareholders but those are dead. */
	for (i = 0; i < FACTION_NUMBER_OF_FACTIONS; i++) {
		set_faction_state(FACTION_BOTS, i, HOSTILE);
	}

	/* Singularity hates everyone but Tux and bots */
	for (i = 0; i < FACTION_NUMBER_OF_FACTIONS; i++) {
		set_faction_state(FACTION_SINGULARITY, i, HOSTILE);
	}
	set_faction_state(FACTION_SINGULARITY, FACTION_SELF, FRIENDLY);
	set_faction_state(FACTION_SINGULARITY, FACTION_BOTS, FRIENDLY);

	/* Neutral does not attack anyone, neither is it being attacked */
	for (i = 0; i < FACTION_NUMBER_OF_FACTIONS; i++) {
		set_faction_state(FACTION_NEUTRAL, i, FRIENDLY);
	}

	/* Test as well does not attack anyone, neither is it being attacked */
	for (i = 0; i < FACTION_NUMBER_OF_FACTIONS; i++) {
		set_faction_state(FACTION_TEST, i, FRIENDLY);
	}

	/* Each faction is friendly towards itself. Otherwise, its members attack each other and/or commit suicide. */
	for (i = 0; i < FACTION_NUMBER_OF_FACTIONS; i++) {
		set_faction_state(i, i, FRIENDLY);
	}
}

/**
  * Save one faction hostility matrix.
  */

void write_faction(struct auto_string *strout, int *faction_idx)
{
	autostr_append(strout, "%s = ", factions[*faction_idx].name);
	write_int32_t_array(strout, hostility_matrix[*faction_idx], FACTION_NUMBER_OF_FACTIONS);
	autostr_append(strout, ",\n");
}

/**
  * Load one faction hostility matrix.
  */

void read_faction(lua_State *L, int index, int *faction_idx)
{
	lua_getfield(L, index, factions[*faction_idx].name);
	read_int32_t_array(L, -1, hostility_matrix[*faction_idx], FACTION_NUMBER_OF_FACTIONS);
	lua_pop(L, 1);
}

#undef _faction_c
