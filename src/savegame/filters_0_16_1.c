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

// In 0.16.1, bullets were stored in a statically allocated array,
// and a specific bullet's type (INFOUT) was used to define empty
// slots in that array.
// Now bullets are stored in a sparse dynarray, and the INFOUT
// bullet's type is no more used.
// This filter removes 'INFOUT bullets' from the savegame.
int filter_0_16_1_convert_bullets_array(struct savegame_data *savegame, struct auto_string *report)
{
	char *array_start = strstr(savegame->sav_buffer, "bullet_array{");
	if (!array_start) {
		// No bullets stored in that savegame ? Strange, but let's be silent
		// about that...
		return FILTER_NOT_APPLIED;
	}

	// Loop over the MAXBULLETS (=100) bullet definitions to find INFOUT's ones

	char *bullet_start = array_start + strlen("bullet_array{\n");

	int bullet_index;
	for (bullet_index = 0; bullet_index < 100; bullet_index++) {

		// Find the end of the bullet's definition by passing over
		// 26 lines in the savegame stream

		char *bullet_end = bullet_start;

		int i;
		for (i = 0; i < 26; i++) {
			while (*bullet_end != '\n' && *bullet_end != '\0') bullet_end++;
			if (*bullet_end == '\0') {
				// Reach the end of savegame before to find the end of the bullet
				// definition ?
				autostr_append(report,
				               _("Error during savegame filtering (%s:%s): End of savegame reached while "
				                 "reading a bullet's definition..\n"
				                 "The savegame seems to be corrupted."),
				               savegame->running_converter->id, __FUNCTION__);
				return FILTER_ABORT;
			}
			bullet_end++;
		}

		// "},\n" expected on the 26th line

		if (strncmp(bullet_end, "},\n", 3)) {
			autostr_append(report,
			               _("Error during savegame filtering (%s:%s): End marker of a bullet's definition "
			                 "not found where it was expected..\n"
			                 "The savegame seems to be corrupted."),
			               savegame->running_converter->id, __FUNCTION__);
			return FILTER_ABORT;
		}

		// Now check the bullet's type

		if (!strncmp(bullet_start + 2, "type = -30,", 11)) {
			// INFOUT bullet - Remove it by replacing its definition by a series of spaces
			memset(bullet_start, ' ', (bullet_end + 2)- bullet_start + 1);
		}

		// Points to the beginning of the next bullet

		bullet_start = bullet_end + 3;
	}

	return FILTER_APPLIED;
}
