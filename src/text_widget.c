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

/**
 * This is a text widget that is suitable for a text area with scrolling
 * capabilites.
 */

#include "system.h"

#include "defs.h"
#include "struct.h"
#include "proto.h"
#include "global.h"

/**
 * Initialize or reset a text widget.
 */
void init_text_widget(text_widget *w, const char *start_text) {
	if (w->text == NULL)
		w->text = alloc_autostr(10000);
	w->text->length = 0;
	autostr_printf(w->text, start_text);

	w->mouse_already_handled = 0;
	w->scroll_offset = 0;
	w->text_stretch = 1.0;
	w->content_above_func = NULL;
	w->content_below_func = NULL;
}

/**
 * Handle mouse clicks.
 *
 * @return TRUE if a mouse click was handled, and no further handling should be
 * done by the caller
 */
int widget_handle_mouse(text_widget *w) {
	int mouse_over_widget = MouseCursorIsInRect(&w->rect, GetMousePos_x(), GetMousePos_y());
	int mouse_over_upper_half = (GetMousePos_y() - w->rect.y) < (w->rect.h / 2);

	/* Change the mouse cursor as needed. */
	if (mouse_over_widget) {
		if (mouse_over_upper_half)
			global_ingame_mode = GLOBAL_INGAME_MODE_SCROLL_UP;
		else
			global_ingame_mode = GLOBAL_INGAME_MODE_SCROLL_DOWN;
	} else {
		global_ingame_mode = GLOBAL_INGAME_MODE_NORMAL;
	}

	/* We handle a mouse click under two conditions:
	 * - LMB was clicked this frame, inside the widget.
	 * - LMB was clicked in a previous frame, inside the widget, and we have
	 *   not registered a mouse release since. */
	if (!mouse_over_widget && !w->mouse_already_handled)
		return 0;

	int mwheel_up = MouseWheelUpPressed();
	int mwheel_down = MouseWheelDownPressed();
	if (!MouseLeftPressed() && !mwheel_up && !mwheel_down) {
		w->mouse_already_handled = FALSE;
		return 0;
	}

	/* Do not handle left button click more than once.  However, we always want
	 * to handle mouse wheel actions, since we can get several of those per frame. */
	if (w->mouse_already_handled && !mwheel_up && !mwheel_down) {
		return 1;
	} else {
		w->mouse_already_handled = TRUE;
	}

	/* Handle scrolling. */
	if (MouseLeftPressed()) {
		if (mouse_over_upper_half)
			w->scroll_offset--;
		else
			w->scroll_offset++;
	} else {
		if (mwheel_up)
			w->scroll_offset--;
		if (mwheel_down)
			w->scroll_offset++;
	}

	return 1;
}

/**
 * Show the specified text widget.
 */
void show_text_widget(text_widget *w) {
	SetCurrentFont(w->font);
	int lines_needed = get_lines_needed(w->text->value, w->rect, w->text_stretch);
	int offset = 0;
	const int font_size = FontHeight(w->font) * w->text_stretch;

	float visible_lines = (float) w->rect.h / (float) font_size;
	/* Disallow scrolling too far up or down. */
	if (lines_needed + w->scroll_offset < floorf(visible_lines))
		w->scroll_offset = floorf(visible_lines) - lines_needed;
	else if (w->scroll_offset > 0)
		w->scroll_offset = 0;

	/* Set the offset to display the relevant portions of the text, taking
	 * scrolling into account. */
	if (lines_needed > visible_lines)
		offset = font_size * (lines_needed - visible_lines + w->scroll_offset);
	else
		w->scroll_offset = 0;

	/* Ensure we do not show empty space before first line. */
	if (offset < 0)
		offset = 0;

	SDL_SetClipRect(Screen, NULL);
	DisplayText(w->text->value, w->rect.x, w->rect.y - offset, &w->rect, w->text_stretch);

	/* If we have more content above or below the currently visible text, we call the
	 * functions that may have been specified for this event. */
	if (lines_needed > visible_lines) {
		if (w->content_below_func != NULL && w->scroll_offset != 0)
			w->content_below_func();
		if (w->content_above_func != NULL && w->scroll_offset != (int)visible_lines - lines_needed)
			w->content_above_func();
	}
}
