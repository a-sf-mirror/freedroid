/*
 *
 *   Copyright (c) 1994, 2002, 2003 Johannes Prix
 *   Copyright (c) 1994, 2002 Reinhard Prix
 *   Copyright (c) 2004-2010 Arthur Huillet
 *   Copyright (c) 2015 Samuel Degrande
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
 */

/**
 * \file font.c
 * \brief This file contains the functions used to process fonts, independently
 *        of the font implementation by itself.
 *        Currently we use bitmaps fonts (see BFont.c)
 */

#define _font_c

#include "system.h"

#include "defs.h"
#include "struct.h"
#include "global.h"
#include "proto.h"
#include "BFont.h"

static struct {
	struct font **font_ref;
	char *filename;
	struct font font;
} fonts_def[] = {
		{ &Para_BFont,        "parafont.png",     { 0,  0, NULL } },
		{ &Menu_BFont,        "cpuFont.png",      { 0, -4, NULL } },
		{ &Messagevar_BFont,  "small_white.png",  { 0,  0, NULL } },
		{ &Messagestat_BFont, "small_blue.png",   { 0,  0, NULL } },
		{ &Red_BFont,         "font05_red.png",   { 0, -2, NULL } },
		{ &Blue_BFont,        "font05_white.png", { 0, -2, NULL } },
		{ &FPS_Display_BFont, "font05.png",       { 0, -2, NULL } },
		{ &Messagered_BFont,  "small_red.png",    { 0,  0, NULL } }
};

/* Current font */
static struct font *CurrentFont;

/**
 * This function should load all the fonts we'll be using via the SDL
 * BFont library in FreedroidRPG.
 */
void InitOurBFonts(void)
{
	int i;
	char fpath[PATH_MAX];

	for (i = 0; i < sizeof(fonts_def)/sizeof(fonts_def[0]); i++) {
		char constructed_fname[PATH_MAX];
		sprintf(constructed_fname, "font/%s", fonts_def[i].filename);
		find_file(constructed_fname, GRAPHICS_DIR, fpath, PLEASE_INFORM | IS_FATAL);

		int rtn = LoadFont(constructed_fname, &fonts_def[i].font);
		if (rtn == FALSE) {
			error_message(__FUNCTION__, "A font file for the BFont library could not be loaded (%s).",
					PLEASE_INFORM | IS_FATAL, fonts_def[i].filename);
		}

		*fonts_def[i].font_ref = &fonts_def[i].font;
	}

}

/**
 * Set the current font
 */
void SetCurrentFont(struct font* font)
{
	CurrentFont = font;
};				// void SetCurrentFont (BFont_Info * Font)

/**
 * Returns the pointer to the current font strucure in use
 */
struct font *GetCurrentFont(void)
{
	return CurrentFont;
};				// BFont_Info * GetCurrentFont (void)

/**
 * Get letter-spacing for specified font.
 *
 * Letter-spacing refers to the overall spacing of a word or block of text
 * affecting its overall density and texture.
 */
int get_letter_spacing(struct font *font)
{
	return font->letter_spacing;
}

/**
 * Return the font height
 */
int FontHeight(struct font *font)
{
	return font->height;
}

/**
 * Handle font switching on special characters (\x) or formatting tags (aka bbcodes).
 * Returns 1 if the font was changed and 0 if it was not.
 */
int handle_switch_font_char(char **ptr)
{
	if (**ptr == '[' && *(*ptr+2) != ']')
		return FALSE;

	int index = 0;
	int incr = 1;

	if (**ptr == '[') {
		(*ptr)++;
		index = 2;
		incr = 2;
	}

	if (**ptr == font_switchto_red[index]) {
		SetCurrentFont(Red_BFont);
		(*ptr) += incr;
		return TRUE;
	} else if (**ptr == font_switchto_blue[index]) {
		SetCurrentFont(Blue_BFont);
		(*ptr) += incr;
		return TRUE;
	} else if (**ptr == font_switchto_neon[index]) {
		SetCurrentFont(FPS_Display_BFont);
		(*ptr) += incr;
		return TRUE;
	} else if (**ptr == font_switchto_msgstat[index]) {
		SetCurrentFont(Messagestat_BFont);
		(*ptr) += incr;
		return TRUE;
	} else if (**ptr == font_switchto_msgvar[index]) {
		SetCurrentFont(Messagevar_BFont);
		(*ptr) += incr;
		return TRUE;
	}

	return FALSE;
}

void put_string_centered(struct font *font, int y, const char *text)
{
	put_string(font, Screen->w / 2 - text_width(font, text) / 2, y, text);
}

void put_string_right(struct font *font, int y, const char *text)
{
	put_string(font, Screen->w - text_width(font, text) - 13, y, text);
}

void put_string_left(struct font *font, int y, const char *text)
{
	put_string(font, 13, y, text);
}

#undef _font_c
