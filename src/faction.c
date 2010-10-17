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

#define _faction_c

#include "system.h"

#include "defs.h"
#include "struct.h"
#include "proto.h"
#include "global.h"

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

	ErrorMessage(__FUNCTION__, "Faction name %s does not exist.", PLEASE_INFORM, IS_FATAL, name);
	return FACTION_SELF;
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

	/* Each faction is friendly towards itself. Otherwise, its members attack each other and/or commit suicide. */
	for (i = 0; i < FACTION_NUMBER_OF_FACTIONS; i++) {
		set_faction_state(i, i, FRIENDLY);
	}
}

/**
  * Save the faction hostility matrix.
  */
void save_factions(struct auto_string *s)
{
	int i;

	autostr_append(s, "<factions>\n");
	for (i = 0; i < FACTION_NUMBER_OF_FACTIONS; i++) {
		save_int32_t_array(factions[i].name, hostility_matrix[i], FACTION_NUMBER_OF_FACTIONS);
	}
	autostr_append(s, "</factions>\n");
}

/**
  * Load the faction hostility matrix.
  */
void load_factions(char *s)
{
	char *end;
	int i;

	s = strstr(s, "<factions>");
	if (!s) {
		// Reset factions to default
		ErrorMessage(__FUNCTION__, "Could not find factions in savegame. Using default values for faction hostility matrix.", NO_NEED_TO_INFORM, IS_WARNING_ONLY);
		init_factions();
		return;
	}

	end = strstr(s, "</factions>");
	if (!end) {
		return;
	}
	*end = 0;

	for (i = 0; i < FACTION_NUMBER_OF_FACTIONS; i++) {
		read_int32_t_array(s, factions[i].name, hostility_matrix[i], FACTION_NUMBER_OF_FACTIONS);
	}	

	*end = '<';
}
#undef _faction_c
