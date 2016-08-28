/*
 *
 *   Copyright (c) 2016 Samuel Degrande
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

#include "savegame.h"

// In 0.16.1, melee shots, bullets, blasts and spells were stored in statically
// allocated arrays, and a specific object's type (INFOUT) was used to define
// empty slots in that array.
// Now objects are stored in a sparse dynarray, and the INFOUT object's type
// is no more used.
// This filter removes 'INFOUT objects' from the savegame.
static int convert_to_sparse_array(struct savegame_data *savegame, struct auto_string *report, const char* section, const char* type, const char* infout, int nb_lines)
{
	char *array_start = strstr(savegame->sav_buffer, section);
	if (!array_start) {
		// An empty list in that savegame ? Strange, but let's be silent
		// about that...
		return FILTER_NOT_APPLIED;
	}

	// Loop over the MAXOBJECTS (=100) object definitions to find INFOUT's ones

	char *object_start = array_start + strlen(section) + 1;

	int object_index;
	for (object_index = 0; object_index < 100; object_index++) {

		// Find the end of the object's definition by passing over
		// nb_lines lines in the savegame stream

		char *object_end = object_start;

		int i;
		for (i = 0; i < nb_lines; i++) {
			while (*object_end != '\n' && *object_end != '\0') object_end++;
			if (*object_end == '\0') {
				// Reach the end of savegame before to find the end of the
				// object definition ?
				autostr_append(report,
				               _("Error during savegame filtering (%s:%s): End of savegame reached while "
				                 "reading a object's definition..\n"
				                 "The savegame seems to be corrupted."),
				               savegame->running_converter->id, __FUNCTION__);
				return FILTER_ABORT;
			}
			object_end++;
		}

		// "},\n" expected on the nth line

		if (strncmp(object_end, "},\n", 3)) {
			autostr_append(report,
			               _("Error during savegame filtering (%s:%s): End marker of a object's definition "
			                 "not found where it was expected..\n"
			                 "The savegame seems to be corrupted."),
			               savegame->running_converter->id, __FUNCTION__);
			return FILTER_ABORT;
		}

		// Now check the object's type

		char *type_start = strstr(array_start, type);
		if (type_start) {
			type_start += strlen(type);
			if (!strncmp(type_start, infout, strlen(infout))) {
				// INFOUT object - Remove it by replacing its definition by a series of spaces
				memset(object_start, ' ', (object_end + 2) - object_start + 1);
			}
		}

		// Points to the beginning of the next object

		object_start = object_end + 3;
	}

	return FILTER_APPLIED;
}

int filter_0_16_1_convert_bullets_array(struct savegame_data *savegame, struct auto_string *report)
{
	return convert_to_sparse_array(savegame, report, "bullet_array{", "type = ", "-30,", 26);
}

int filter_0_16_1_convert_melee_shots_array(struct savegame_data *savegame, struct auto_string *report)
{
	return convert_to_sparse_array(savegame, report, "melee_shot_array{", "attack_target_type = ", "103,", 8);
}

int filter_0_16_1_convert_blasts_array(struct savegame_data *savegame, struct auto_string *report)
{
	return convert_to_sparse_array(savegame, report, "blast_array{", "type = ", "-30,", 10);
}

int filter_0_16_1_convert_spellactives_array(struct savegame_data *savegame, struct auto_string *report)
{
	return convert_to_sparse_array(savegame, report, "spell_active_array{", "img_type = ", "-1,", 34);
}
