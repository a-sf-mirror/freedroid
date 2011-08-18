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
			if (!strcmp(Druidmap[i].druidname, *droid_type))
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
		sprintf(suggested_val, "%s%s, ", suggested_val, Druidmap[type_index[i]].druidname);

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

out:	free_autostr(displayed_text);
}
