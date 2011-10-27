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
	struct widget base;		/**< Base widget type containing callbacks and position info. */
	struct BFont_Info *font;	/**< Font to be used when displaying text. */
	struct auto_string *text;	/**< Text to be displayed. */
	float line_height_factor;
	int scroll_offset;         	/**< Offset for the text being displayed. 0 means bottom, negative means above bottom. */
	int mouse_already_handled;	/**< Flag used for handling input. Deprecated. */
	enum mouse_hover { NOT_HOVERED, UPPER_HALF, LOWER_HALF } mouse_hover;	/**< Area hovered by the mouse. */
	
	void (*content_below_func)(void);
	void (*content_above_func)(void);
};

struct widget_text *widget_text_create(void);
void widget_text_init(struct widget_text *, const char *);
int widget_text_handle_mouse(struct widget_text *);
void widget_text_display(struct widget *);

#define WIDGET_TEXT(x) ((struct widget_text *)x)

#endif
