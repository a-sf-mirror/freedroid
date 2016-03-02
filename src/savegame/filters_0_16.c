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

// In 0.16, quest logs are saved translated, and with the German locale there
// is a mission_diary_text containing double-quotes. Loading it generates a Lua
// error.
// Note: a patch (b8728ec0) fixed this error by saving Lua strings with long
// literals.
int filter_0_16_fix_german(struct savegame_data *savegame, struct auto_string *report)
{
	// First check if this savegame predates the b8728ec0 fix, by searching
	// if savegame_version_string = \"0.16;sizeof...\"

	char *ptr = strstr(savegame->sav_buffer, "savegame_version_string = ");
	if (!ptr) {
		// This should not happen, this filter being called only when that
		// string is found...
		return FILTER_NOT_APPLIED;
	}
	if (strncmp(ptr+strlen("savegame_version_string = "), "\"0.16;sizeof", strlen("\"0.16;sizeof"))) {
		// Post b8728ec0 savegame
		return FILTER_NOT_APPLIED;
	}

	// To avoid a search in the whole buffer, we mark the part of the
	// buffer that can possibly contain the bug

	/* Find the problematic mission */
	ptr = strstr(savegame->sav_buffer, "mission_name = \"An Explosive Situation\",");
	if (!ptr)
		return FILTER_NOT_APPLIED;

	/* Reach the mission_diary_text */
	ptr = strstr(ptr, "mission_diary_texts = {");
	if (!ptr) {
		 // It should be there !
		autostr_append(report,
		               _("Error during savegame filtering (%s:%s): 'mission_diary_texts' data was not found.\n"
		                 "The savegame seems to be corrupted."),
		               savegame->running_converter->id, __FUNCTION__);
		return FILTER_ABORT;
	}

	/* Mark the end of mission_diary_text */
	char *end_ptr = strchr(ptr, '}');
	if (!end_ptr) {
		autostr_append(report,
		               _("Error during savegame filtering (%s:%s): End marker of 'mission_diary_texts' was not found.\n"
		                 "The savegame seems to be corrupted."),
		               savegame->running_converter->id, __FUNCTION__);
		return FILTER_ABORT;
	}
	*end_ptr = '\0';

	// Now we can search the problematic text and replace the double-quotes
	// with single-quotes

	int filter_report = FILTER_NOT_APPLIED;

	ptr = strstr(ptr, "\"Subatomare");
	if (ptr) {
		filter_report = FILTER_APPLIED;
		*ptr = '\'';
		ptr = strstr(ptr, "Teil IV\"");
		if (ptr)
			*(ptr+7) = '\'';
	}

	// Replace back the end-marker

	*end_ptr = '}';

	return filter_report;
}

// Pre 0.16.1 savegames do not have the SAVEGAME data in their comment header
// This filter adds it.
int filter_0_16_add_savegame_version(struct savegame_data *savegame, struct auto_string *report)
{
	// Set new SAVEGAME data
	struct auto_string *version_string = alloc_autostr(256);
	autostr_printf(version_string, "SAVEGAME: XXXXXX XX %s\n", savegame->info.code_signature);

	// A line has to be inserted, so we need to create a new buffer
	char *new_savegame = (char*)MyMalloc(sizeof(char)*(savegame->sav_buffer_size + version_string->length + 1));

	// Insert the version string as first comment line
	sprintf(new_savegame, "--[[\n%s", version_string->value);

	// Copy the rest of the savegame buffer
	memcpy(new_savegame+strlen("--[[\n")+version_string->length, savegame->sav_buffer+strlen("--[[\n"), savegame->sav_buffer_size-strlen("--[[\n"));
	new_savegame[savegame->sav_buffer_size+version_string->length] = '\0';

	// Replace the old buffer by the new one
	free(savegame->sav_buffer);
	savegame->sav_buffer = new_savegame;
	savegame->sav_buffer_size = savegame->sav_buffer_size + version_string->length;
	free_autostr(version_string);

	return FILTER_APPLIED;
}
