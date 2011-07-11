/* 
 *
 *   Copyright (c) 2010 Stefan Kangas
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

#ifndef _widget_text_h
#define _widget_text_h

/**
 * @struct widget_type
 * @brief Type used for managing text.
 * 
 * This structure is used for displaying and handling text windows.
 */
struct widget_text {
	SDL_Rect rect;             /* The area in which the text should be displayed */
	struct BFont_Info *font;
	struct auto_string *text;
	float line_height_factor;

	int scroll_offset;         /* 0 means bottom, negative means above bottom. */
	int mouse_already_handled;

	void (*content_below_func)(void);
	void (*content_above_func)(void);
};

void widget_text_init(struct widget_text *, const char *);
int widget_text_handle_mouse(struct widget_text *);
void widget_text_display(struct widget_text *);

#endif
