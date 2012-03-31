/*
 *   Copyright (c) 2011 Matei Pavaluca 
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
 * @file lvledit_enemy.c
 * This file contains various functions needed for handling enemies in the level editor
 */

#include "system.h"

#include "defs.h"
#include "struct.h"
#include "global.h"
#include "proto.h"

#include "lvledit/lvledit.h"
#include "lvledit/lvledit_menu.h"

/**
 * Sets the random droid types of a level, if they are valid. 
 * @param lvl Currently edited level
 * @param input String containing the droid types.
 * @param droid_type If the input is invalid, this string will contain the first malformed droid type encountered.
 * @return 0 on success, 1 on failure.
 */
static int set_random_droid_types(level *lvl, const char *input, char **droid_type)
{
	char delim[] = ", ";
	char *buff = strdup(input);
	int i;

	lvl->random_droids.types_size = 0;
	*droid_type = strtok(buff, delim);

	while (*droid_type) {
		for (i = 0; i < Number_Of_Droid_Types; i++)
			if (!strcmp(Droidmap[i].droidname, *droid_type))
				break;

		if (i == Number_Of_Droid_Types) {
			*droid_type = strdup(*droid_type);
			free(buff);
			return 1;
		}

		if (lvl->random_droids.types_size < (sizeof(lvl->random_droids.types) / sizeof(int)))
			lvl->random_droids.types[lvl->random_droids.types_size++] = get_droid_type(*droid_type);

		*droid_type = strtok(NULL, delim);
	}
	free(buff);

	return 0;
}

void get_random_droids_from_user()
{
	char *user_input;
	char suggested_val[200];
	level *lvl = EditLevel();
	struct auto_string *displayed_text = alloc_autostr(64);
	int numb;
	char *droid_type = NULL;

	InitiateMenu("--EDITOR_BACKGROUND--");

	autostr_printf(displayed_text, "Number of random droids:");

	sprintf(suggested_val, "%d", lvl->random_droids.nr);
 
	while (1) {
		numb = get_number_popup(displayed_text->value, suggested_val);
		// If there was no input the user wants to exit this popup
		if (numb == -2)
			goto out;

		if (numb != -1)
			break;

		alert_window("%s", _("Invalid number! The number must be natural."));
		RestoreMenuBackground(0);
	}

	lvl->random_droids.nr = numb;

	autostr_append(displayed_text, " %d\nDroid types:", lvl->random_droids.nr);

	// Fill the suggested_val with the current droid types
	sprintf(suggested_val, "%s", "");
	int i;
	int *type_index = lvl->random_droids.types;
	for (i = 0; i < lvl->random_droids.types_size; i++)
		sprintf(suggested_val, "%s%s, ", suggested_val, Droidmap[type_index[i]].droidname);

	// Get the droid types
	while (1) {
		user_input = GetEditableStringInPopupWindow(sizeof(suggested_val) - 1, _(displayed_text->value), suggested_val);
		if (!user_input)
			goto out;

		if (!set_random_droid_types(lvl, user_input, &droid_type)) {
			free(user_input);
			free(droid_type);
			break;
		}

		alert_window("%s is not a droid type!", _(droid_type));
		RestoreMenuBackground(0);
		sprintf(suggested_val, "%s", user_input);
		free(user_input);
		free(droid_type);
	}

out:
	free_autostr(displayed_text);
}

/**
 * Edits information related to Special Forces such as marker, faction, dialogue etc
 * @param en Enemy to be edited
 */
static void edit_special_force_info(enemy *en)
{
	char *user_input;
	char suggested_val[200];
	int numb;
	struct auto_string *displayed_text = alloc_autostr(64);

	StoreMenuBackground(0);

	autostr_printf(displayed_text, "New enemy info:\n Marker: ");
	sprintf(suggested_val, "%d", en->marker);

	// Edit the marker
	while (1) {
		numb = get_number_popup(displayed_text->value, suggested_val);
		if (numb == -2)
			goto out;

		if (numb != -1)
			break;

		alert_window("%s", _("Invalid number! The number must be natural."));
		RestoreMenuBackground(0);
	}

	en->marker = numb;
	autostr_append(displayed_text, "%d\n Faction: ", numb);
	sprintf(suggested_val, "%s", get_faction_from_id(en->faction));

	// Edit the faction
	user_input = GetEditableStringInPopupWindow(sizeof(suggested_val) - 1, _(displayed_text->value), suggested_val);
	if (!user_input)
		return;

	en->faction = get_faction_id(user_input);

	autostr_append(displayed_text, "%s\n Dialog name: ", get_faction_from_id(en->faction));
	sprintf(suggested_val, "%s", en->dialog_section_name);
	free(user_input);

	// Change the dialog
	while (1) {
		user_input = GetEditableStringInPopupWindow(sizeof(suggested_val) - 1, _(displayed_text->value), suggested_val);
		if (!user_input)
			goto out;

		if (npc_get(user_input))
			break;

		alert_window("Dialog \"%s\" not found!", _(user_input));
		RestoreMenuBackground(0);
		sprintf(suggested_val, "%s", user_input);
	}

	free(en->dialog_section_name);
	en->dialog_section_name = strdup(user_input);

	autostr_append(displayed_text, "%s\n Short description: ", user_input);
	sprintf(suggested_val, "%s", en->short_description_text);
	free(user_input);

	// Edit the Short description
	user_input = GetEditableStringInPopupWindow(sizeof(suggested_val) - 1, _(displayed_text->value), suggested_val);
	if (!user_input)
		goto out;

	free(en->short_description_text);
	en->short_description_text = strdup(user_input);

	autostr_append(displayed_text, "%s\n Completely fixed: ", user_input);
	sprintf(suggested_val, "%s", en->CompletelyFixed ? "yes" : "no");
	free(user_input);

	// Completely fixed
	while (1) {
		user_input = GetEditableStringInPopupWindow(sizeof(suggested_val) - 1, _(displayed_text->value), suggested_val);
		if (!user_input)
			goto out;

		if (!strcmp(user_input, "yes")) {
			en->CompletelyFixed = 1;
			break;

		} else if (!strcmp(user_input, "no")) {
			en->CompletelyFixed = 0;
			break;
		}
	}

	autostr_append(displayed_text, "%s\n Max distance from home: ", en->CompletelyFixed ? "yes" : "no");
	sprintf(suggested_val, "%d", en->max_distance_to_home);
	free(user_input);

	// Edit max distance to home
	while(1) {
		numb = get_number_popup(displayed_text->value, suggested_val);
		if (numb == -2)
			return;

		if (numb != -1)
			break;

		alert_window("%s", _("Invalid number! The number must be natural."));
		RestoreMenuBackground(0);
		sprintf(suggested_val, "%d", numb);
	}

	en->max_distance_to_home = numb;

	autostr_append(displayed_text, "%d\n Item dropped on death: ", numb);
	sprintf(suggested_val, "%s", (en->on_death_drop_item_code == -1) ? "none" : ItemMap[en->on_death_drop_item_code].item_name);

	// Change the item dropped on death
	while (1) {
		user_input = GetEditableStringInPopupWindow(sizeof(suggested_val) - 1, _(displayed_text->value), suggested_val);
		if (!user_input)
			goto out;

		if (!strcmp(user_input, "none")) {
			en->on_death_drop_item_code = -1;
			break;

		} else if (GetItemIndexByName(user_input) != -1) {
			en->on_death_drop_item_code = GetItemIndexByName(user_input);
			break;
		}

		alert_window("Item \"%s\" not found!", _(user_input));
		RestoreMenuBackground(0);
		sprintf(suggested_val, "%s", user_input);
	}

	autostr_append(displayed_text, "%s\n Rushes Tux: ",
				(en->on_death_drop_item_code == -1) ? "none" : ItemMap[en->on_death_drop_item_code].item_name);
	sprintf(suggested_val, "%s", en->will_rush_tux ? "yes" : "no");

	// Rush Tux
	while (1) {
		user_input = GetEditableStringInPopupWindow(sizeof(suggested_val) - 1, _(displayed_text->value), suggested_val);
		if (!user_input)
			goto out;

		if (!strcmp(user_input, "yes")) {
			en->will_rush_tux = 1;
			break;

		} else if (!strcmp(user_input, "no")) {
			en->will_rush_tux = 0;
			break;
		}
	}

out:
	free_autostr(displayed_text);
}

/**
 * Creates an enemy basing on user input
 * @param droid_pos Coordinates of the droid
 * @param enemy_type Type of enemy to be inserted
 */
enemy *create_special_force(gps droid_pos, int enemy_type)
{
	enemy *en = enemy_new(enemy_type);
	enemy_reset(en);

	// Initialise some of the enemy fields to their default
	en->SpecialForce = TRUE;
	en->pos = droid_pos;

	// Each enemy needs a default dialog
	en->dialog_section_name = strdup("AfterTakeover");

	// Let the user edit the info
	edit_special_force_info(en);

	return en;
}
