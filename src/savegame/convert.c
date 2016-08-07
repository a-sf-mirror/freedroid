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

/**
 * \file convert.c
 * \brief This file contains code to fix some savegame errors.
 *
 * Sometimes, we produce savegames that contain bugs, possibly preventing to
 * reload them. And those bugs can happen after hours of play...
 * We need to provide a way to fix the savegame while loading it, letting
 * the player to continue to enjoy the game.
 *
 * This file contains an implementation of a savegame converter framework.
 * 'Chains of converters' can be applied to 'upgrade' from one old savegame's
 * version to a newer one.
 *
 * A 'SAVEGAME' header in the .sav file is used to store the savegame's version
 * and revision, using a couple of integers (to have fast comparisons):
 * - SAVEGAME_VERSION is the game's version coded on 6 digits: XXYYZZ, with
 *   XX=major version, YY=minor version, ZZ=patch version.
 *   For instance, for release 0.15.1, SAVEGAME_VERSION = 001501, and for
 *   release 1.0.0, SAVEGAME_VERSION = 010000.
 * - SAVEGAME_REVISION is a two digits integer, defining the latest fix
 *   applied.
 *
 * A converter usually converts a savegame from (VERSION, REVISION) to
 * (VERSION, REVISION+1).
 *
 * Those 2 values are defined in the configure.ac file. They have to be changed
 * when:
 * - a new release of the game is made available.
 *   In this case, SAVEGAME_VERSION is set according to the new release number,
 *   and SAVEGAME_REVISION is set to 00
 * - a converter for a savegame bug is added.
 *   In this case, SAVEGAME_REVISION is incremented.
 *
 * Example:
 * 1) 0.16.0 is released
 *   -> SAVEGAME_VERSION = 001600, SAVEGAME_REVISION = 00
 * 2) A bug is found in that savegame version, and a converter is written to fix it
 *   -> SAVEGAME_VERSION = 001600, SAVEGAME_REVISION = 01
 * 3) 0.16.1, containing that converter, is released
 *   -> SAVEGAME_VERSION = 001601, SAVEGAME_REVISION = 00
 *
 * A savegame converter is composed of a list of piped filter functions.
 * A filter gets a pointer to a buffer containing the savegame data.
 * If possible, a filter should do its job in-place, thus not modifying the
 * pointer, or if it has to create a new buffer, the filter has to free the old
 * buffer, and set the pointer to the new buffer.
 * The last filter in the list should be _change_savegame_version(), which set
 * the SAVEGAME data to the converter's output version and revision.
 *
 * Note: A filter often has to search if a given data is present in the
 * savegame and if found the filter will change it. This search has to be as
 * fast as possible, and the filter should avoid to scan the whole savegame
 * buffer. This can be achieved by first looking for a well known data near
 * the one to fix, and then limit the search to a small part of the savegame
 * (see filter_0_16_fix_german() as an example).
 *
 * Note: Do not expect too much from a savegame converter. It can fix a bad
 * value, add or remove a part of a savegame. But writing a converter to
 * upgrade between two 'savegame-incompatible' versions of the game is an
 * other story.
 */

#include "savegame.h"

// Available filters function

extern int filter_0_16_fix_german(struct savegame_data *, struct auto_string *);
extern int filter_0_16_add_savegame_version(struct savegame_data *, struct auto_string *);
extern int filter_0_16_1_convert_bullets_array(struct savegame_data *, struct auto_string *);
static int _change_savegame_version(struct savegame_data *, struct auto_string *);

// List of available converters
// Keep them ordered so that they form a chain of converters

static struct converter converters[] = {
		{ "fix0_16", 0000, 0, 1600, 1,
		  { filter_0_16_fix_german, filter_0_16_add_savegame_version, _change_savegame_version, NULL }
		},
		{ "update_to_0_16_1", 1600, 1, 1601, 0,
		  { _change_savegame_version, NULL }
		},
		{ "fix0_16_1", 1601, 0, 1601, 1,
		  { filter_0_16_1_convert_bullets_array, _change_savegame_version, NULL }
		}
};

/*
 * Set, in-place, the SAVEGAME data to the converter's (to_version, to_revision)
 */
static int _change_savegame_version(struct savegame_data *savegame, struct auto_string *report)
{
	char version_string[10];

	char *ptr = strchr(savegame->sav_buffer, '\n'); // skip first line (start of comment)
	ptr++;
	if (strncmp(ptr, "SAVEGAME: ", 10)) {
		// We already checked that the SAVEGAME line is present, so we should
		// not get here. However, just in case a filter messed up the data, we
		// avoid to continue to corrupt it.
		return FILTER_NOT_APPLIED;
	}
	snprintf(version_string, 10, "%06d %02d", savegame->running_converter->to_version, savegame->running_converter->to_revision);
	memcpy(ptr+10, version_string, 9);

	return FILTER_APPLIED;
}

/*
 * Extract savegame's info from the SAVEGAME data, if present, or from the
 * savegame_version_string for old savegames
 */
static int _extract_savegame_info(struct savegame_data *savegame, struct auto_string *report)
{
	char *ptr = strchr(savegame->sav_buffer, '\n'); // skip first line (start of comment)
	ptr++;

	// Try to get the savegame info from the SAVEGAME data, which is the first
	// line in the comment header

	if (!strncmp(ptr, "SAVEGAME", 8)) {

		int version, revision;
		char *code_sign = NULL;

		char *marker = strchr(ptr, '\n');
		*marker = '\0';
		int nb = sscanf(ptr, "SAVEGAME: %d %d %ms", &version, &revision, &code_sign);
		*marker = '\n';

		if (nb != 3) {
			autostr_append(report,
			               _("The SAVEGAME info can not be decoded.\n"
			                 "Your savegame is probably corrupted."));
			return FILTER_ABORT;
		}

		savegame->info.version = version;
		savegame->info.revision = revision;
		savegame->info.code_signature = code_sign;

		return FILTER_APPLIED;
	}

	// This seems to be a pre-0.16.1 savegame.
	// Try to extract info from the old 'savegame_version_string'

	ptr = strstr(savegame->sav_buffer, "savegame_version_string = ");
	if (!ptr) {
		autostr_append(report,
		               _("Your savegame seems to be a pre-0.16.1 version, but the savegame_version_string can not be found.\n"
		                 "Your savegame is probably corrupted."));
		return FILTER_ABORT;
	}
	ptr += strlen("savegame_version_string = ");

	char *code_sign = NULL;
	if (*ptr == '\"') {
		char *marker = strchr(ptr+1, '\"');
		*marker = '\0';
		code_sign = strdup(ptr+1);
		*marker = '\"';
	} else if (*ptr == '[') {
		char *marker = strstr(ptr+1, "]=]");
		*marker = '\0';
		code_sign = strdup(ptr+1);
		*marker = ']';
	} else {
		autostr_append(report,
		               _("Your savegame seems to be a pre-0.16.1 version, but the savegame_version_string can not be decoded.\n"
		                 "Your savegame is probably corrupted."));
		return FILTER_ABORT;
	}

	savegame->info.version = 0;
	savegame->info.revision = 0;
	savegame->info.code_signature = code_sign;

	return FILTER_APPLIED;
}

/*
 * Check if the code_signature found in the savegame is the same than the
 * code_signature of the current game code
 */
static int _code_version_mismatch(struct savegame_data *savegame, struct auto_string *report)
{
	struct auto_string *version_string = alloc_autostr(256);
	autostr_printf(version_string,
		"%s;sizeof(tux_t)=%d;sizeof(enemy)=%d;sizeof(bullet)=%d",
		VERSION, (int)sizeof(tux_t), (int)sizeof(enemy), (int)sizeof(bullet));

	int cmp = strcmp(savegame->info.code_signature, version_string->value);
	free_autostr(version_string);

	return (cmp != 0);
}

/**
 * \brief Main function called when a savegame is loaded.
 *
 * \param savegame Pointer to a char buffer containing the savegame.
 * \param size     Pointer to the sze of the savegame buffer.
 *
 * \return FILTER_NOT_NEEDED  - if savegame is recent enough to not needing to be converted
 *         FILTER_NOT_APPLIED - if the savegame is old, but no converter was applied
 *         FILTER_APPLIED     - if a converter was applied
 *         FILTER_ABORT       - if conversion was aborted
 *
 */
int convert_old_savegame(char **sav_buffer, int *sav_buffer_size)
{
	int last_good_version = converters[sizeof(converters)/sizeof(converters[0]) - 1].to_version;
	int last_good_revision = converters[sizeof(converters)/sizeof(converters[0]) - 1].to_revision;
	int conversion_report = FILTER_NOT_NEEDED;
	struct auto_string *report = alloc_autostr(128);

	struct savegame_data savegame = {
		.info = { 0, 0, NULL },
		.sav_buffer = *sav_buffer,
		.sav_buffer_size = *sav_buffer_size,
		.running_converter = NULL
	};

	// Get the savegame info
	if (_extract_savegame_info(&savegame, report) != FILTER_APPLIED) {
		conversion_report = FILTER_ABORT;
		goto CONVERT_END;
	}

	// Check if conversion is needed
	if (savegame.info.version > last_good_version)
		goto CONVERT_END;
	if ((savegame.info.version == last_good_version) && (savegame.info.revision >= last_good_revision))
		goto CONVERT_END;

	// Run converters
	int new_savegame_version = savegame.info.version;
	int new_savegame_revision = savegame.info.revision;
	conversion_report = FILTER_NOT_APPLIED;

	int i;
	for (i = 0; i < sizeof(converters)/sizeof(converters[0]); i++) {
		struct converter *one_converter = &converters[i];
		savegame.running_converter = one_converter;
		if ((one_converter->from_version == new_savegame_version) && (one_converter->from_revision == new_savegame_revision)) {
			filter_t *filter_function = one_converter->filters;
			while ((*filter_function) != NULL) {
				int rtc = (*filter_function)(&savegame, report);
				switch (rtc) {
					case FILTER_ABORT:
						conversion_report = FILTER_ABORT;
						goto CONVERT_END;
					case FILTER_APPLIED:
						conversion_report = FILTER_APPLIED;
						filter_function++;
						break;
					case FILTER_NOT_APPLIED:
					default:
						filter_function++;
						break;
				}
			}
			new_savegame_version = one_converter->to_version;
			new_savegame_revision = one_converter->to_revision;
		}
	}

CONVERT_END:
	switch (conversion_report) {
		case FILTER_NOT_NEEDED:
			// Final check: game code version mismatch
			if (_code_version_mismatch(&savegame, report)) {
				alert_window(_("-=* Savegame loading *=-\n\n"
				               "Your savegame is from an other game version.\n"
				               "If you are using a dev version of the game, this is expected.\n"
				               "We load it as is, but expect some errors or even a crash of the game."));
			}
			break;
		case FILTER_NOT_APPLIED:
			alert_window(_("-=* Savegame loading *=-\n\n"
			               "An old savegame version has been detected (v%d-r%d).\n"
			               "However, no converter was found.\n"
			               "We load it as is, but expect some errors or even a crash of the game."),
			             savegame.info.version, savegame.info.revision);
			break;
		case FILTER_APPLIED:
		default:
			alert_window(_("-=* Savegame loading *=-\n\n"
			               "Your old savegame has been converted from v%d-r%d to v%d-r%d.\n"
			               "Note that we can not guarantee that your savegame is fully playable,\n"
			               "Some errors or even a game crash could possibly happen."),
			             savegame.info.version, savegame.info.revision, new_savegame_version, new_savegame_revision);
			break;
		case FILTER_ABORT:
			if (report->length == 0) {
				autostr_append(report, "None.");
			}
			error_message(__FUNCTION__, "Error during conversion of an old savegame v%d-r%d.\nConverter report:\n%s",
			              PLEASE_INFORM, savegame.info.version, savegame.info.revision, report->value);
			alert_window(_("-=* Savegame loading *=-\n\n"
			               "An old savegame version has been detected (v%d-r%d),\n"
			               "An error was caught during conversion. See your console output.\n"
			               "We will try to load it unconverted, but expect some errors or even a crash of the game."),
			             savegame.info.version, savegame.info.revision);
			break;
	}

	free_autostr(report);
	if (savegame.info.code_signature)
		free(savegame.info.code_signature);

	*sav_buffer = savegame.sav_buffer;
	*sav_buffer_size = savegame.sav_buffer_size;

	return conversion_report;
}
