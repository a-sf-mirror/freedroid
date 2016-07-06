/*
 *
 *   Copyright (c) 2014 Samuel Degrande
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

#define _lang_c 1

#include "system.h"

#include "defs.h"
#include "struct.h"
#include "global.h"
#include "proto.h"

static struct codeset _default_codeset = { "_default_", "ASCII" };
#ifdef ENABLE_NLS
static char *_current_encoding = NULL;
#endif

#ifdef ENABLE_NLS
/**
 * Extract the subforms of a locale, by removing the trailing codes one at a time.
 *
 * Called in a loop, this function generates all forms of a locale, from the
 * most specialized to the language only. 'next_tokens' is used internally
 * to know which part to extract on the next call. On the first call, it must
 * contain a NULL pointer. So a typical usage is:
 * \code
 * char *next_tokens = NULL;
 * char *extracted_locale;
 * while ((extracted_locale = lang_extract_next_part("fr_FR.ISO-8859-1@euro", &next_tokens)) != NULL) {
 *   ... do something ...
 * }
 * \endcode
 * This code returns in turn: "fr_FR.ISO-8859-1@euro", "fr_FR.ISO-8859-1", "fr_FR", "fr", NULL.
 *
 * \param locale Pointer to the locale to parse
 * \param next_token Pointer to a char pointer, used internally. Must points to NULL on the first use.
 *
 * \return A pointer to the current subform of the locale. On first call, returns the whole locale.
 *         When no more parts have to be extracted, returns NULL.
 */
static char *lang_extract_next_subform(const char *locale, char **next_tokens)
{
	static char *tokens = "@._";
	static char *extract = NULL;

	if (*next_tokens == NULL) {
		*next_tokens = tokens;
		if (extract)
			free(extract);
		extract = my_strdup(locale);
		return extract;
	} else if (**next_tokens == '\0') {
		free(extract);
		return NULL;
	}

	while (**next_tokens) {
		char *ptr = strchr(extract, **next_tokens);
		if (ptr) {
			*ptr = '\0';
			return extract;
		}
		(*next_tokens)++;
	}
	free(extract);
	return NULL;
}
#endif

/**
 * Set the locale used by the game for text translation.
 *
 * On success, the locale is set into the game config. The charset encoding
 * to use is computed, and stored internally.
 * Note: GameConfig.locale is set to "" to denote the use of the system
 * default locale.
 *
 * \param locale            Pointer to the locale name, or "" to use system default.
 * \param encoding_changed  Pointer to an int that be be set to TRUE if the charset encoding
 *                          is changed. If NULL, no information is returned.
 */

void lang_set(const char *locale, int *encoding_changed)
{
	if (encoding_changed)
		*encoding_changed = FALSE;

#ifdef ENABLE_NLS
	// Usually setlocale() is called to define the locale to use for
	// localization. However, when called with a locale name, setlocale()
	// checks if that locale is configured on the system, and if not it does
	// not change the current locale.
	// Unfortunately, some distros such as Debian and its derivatives, or Arch,
	// only have a few pre-configured locales.
	// So in order to be able to use all our translations, even if the
	// corresponding locales are not installed on the system, we rather rely
	// on the LANGUAGE envvar and ask gettext to use the system default
	// locale. gettext() gives precedence to LANGUAGE over any other envvar
	// (see GNU gettext manual), avoiding to have to install any locale.

	char *applied_locale = NULL;

	// If a specific locale was requested, we set the LANGUAGE envvar.
	if (locale && strlen(locale)) {
		if (fd_setenv("LANGUAGE", locale, TRUE) != 0)
			return;
		setlocale(LC_MESSAGES, ""); // gettext() will use LANGUAGE
		applied_locale = (char *)locale;
	}

	// If we were asked to use the system default locale, LANGUAGE is unset, and
	// we configure gettext() to use the LC_MESSAGES envvar.
	if (!locale || strlen(locale) == 0) {
		if (fd_unsetenv("LANGUAGE") != 0)
			return;
		applied_locale = setlocale(LC_MESSAGES, ""); // gettext() will use envvars
		if (!applied_locale) {
			error_message(__FUNCTION__,
					"You asked to use the system default local, defined by the LC_MESSAGES envvar.\n"
					"But that locale seems to be unknown on your system. Switching to C locale.",
					PLEASE_INFORM);
			setlocale(LC_MESSAGES, "C");
			applied_locale = "";
		}
	}

	// Save the requested locale in GameConfig
	// 'locale' and GameConfig.locale can possibly be the same pointer.
	// In that case, do nothing.
	if (locale != GameConfig.locale) {
		if (GameConfig.locale)
			free(GameConfig.locale);
		if (locale && strlen(locale))
			GameConfig.locale = my_strdup(locale);
		else
			GameConfig.locale = my_strdup("");
	}

	// Search for the charset encoding to use, unless we default to "C" locale
	// in which case we will use the default encoding.
	char *new_encoding = NULL;
	if (applied_locale && strlen(applied_locale) != 0) {
		char *tok = NULL;
		while (!new_encoding) {
			// Remove trailing parts of the locale, to get all the subforms of
			// the locale, one at a time
			char *subform = lang_extract_next_subform(applied_locale, &tok);
			if (!subform) break;

			// Check if the extracted sub-definition of the locale is one of the available
			// codesets
			int i;
			for (i = 0; i < lang_codesets.size; i++) {
				struct codeset *cs = dynarray_member(&lang_codesets, i, sizeof(struct codeset));
				if (!strcmp(subform, cs->language)) {
					new_encoding = cs->encoding;
					break;
				}
			}
		}
		if (!new_encoding) {
			error_message(__FUNCTION__, "The charset encoding of your locale (%s) is not available. Defaulting to %s.",
					NO_REPORT, applied_locale, _default_codeset.encoding);
			new_encoding = _default_codeset.encoding;
		}
	} else {
		new_encoding = _default_codeset.encoding;
	}

	// If a new codeset is to be used, gettext conversion has to be changed and
	// font bitmaps have to be reloaded
	if (strcmp(new_encoding, _current_encoding)) {
		_current_encoding = new_encoding;
		bind_textdomain_codeset("freedroidrpg", _current_encoding);
		bind_textdomain_codeset("freedroidrpg-dialogs", _current_encoding);
		bind_textdomain_codeset("freedroidrpg-data", _current_encoding);
		if (encoding_changed)
			*encoding_changed = TRUE;
	}
#endif
}

/**
 * Return the locale used by the game for text translation.
 *
 * \return Pointer to the locale name, or NULL if localization is disabled.
 */
char *lang_get()
{
#ifdef ENABLE_NLS
	// If GameConfig.locale is not set, then return the system default locale

	if (GameConfig.locale && strlen(GameConfig.locale) != 0) {
		return GameConfig.locale;
	} else {
		return setlocale(LC_MESSAGES, "");
	}
#else
	return NULL;
#endif
}

/**
 * Return the charset encoding to be used to load font bitmaps.
 *
 * \return Pointer to the encoding.
 */
char *lang_get_encoding()
{
#ifdef ENABLE_NLS
	return _current_encoding;
#else
	return _default_codeset.encoding;
#endif
}

/**
 * Initialize localization.
 *
 * The game is started in the system default locale.
 */

void lang_init()
{
#ifdef ENABLE_NLS
	// Only use localization on messages, to avoid issues when
	// reading/writing numerical data.

	setlocale(LC_ALL, "C");
	setlocale(LC_MESSAGES, "");
	if (GameConfig.locale)
		free(GameConfig.locale);
	GameConfig.locale = my_strdup("");

	_current_encoding = _default_codeset.encoding;

	const char *localedir = data_dirs[LOCALE_DIR].path;
	if (strlen(localedir) == 0) {
		error_message(__FUNCTION__, "Locale dir not found. Disabling localization.",
		             NO_REPORT);
		setlocale(LC_MESSAGES, "C");
	} else {
		// i18n text domain declarations.
		// Note: our bitmap fonts are not utf-8 compliant, but contain a subset
		// of the iso-8859 charset encodings. So we enforce a conversion.

		bindtextdomain("freedroidrpg", localedir);
		bind_textdomain_codeset("freedroidrpg", _current_encoding);
		bindtextdomain("freedroidrpg-dialogs", localedir);
		bind_textdomain_codeset("freedroidrpg-dialogs", _current_encoding);
		bindtextdomain("freedroidrpg-data", localedir);
		bind_textdomain_codeset("freedroidrpg-data", _current_encoding);

		// Default domain to use, if none is specified
		textdomain("freedroidrpg");
	}
#endif
}

#undef _lang_c
