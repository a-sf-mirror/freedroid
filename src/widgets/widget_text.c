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
 * \file widget_text.c
 * \brief This file contains the implementation of the widget_text functions.
 */

#include "system.h"
#include "defs.h"
#include "struct.h"
#include "proto.h"
#include "global.h"
#include "widgets/widgets.h"

/**
 * \brief Handles mouse button events received by a text widget.
 *
 * \details This function is a helper function, used by text_handle_event()
 *
 * \param wt    Pointer to the widget_text object
 * \param event Pointer to the propagated event
 *
 * \return 1 if the event was handled and no further handling is required.
 */
static int text_handle_mouse_down(struct widget_text *wt, SDL_Event *event)
{
	switch (event->button.button) {
		case MOUSE_BUTTON_1:
			// LMB in the upper half, scroll text up if possible.
			if (wt->mouse_hover == UPPER_HALF && widget_text_can_scroll_up(wt))
				wt->scroll_offset--;
			// LMB in the lower half, scroll text down if possible..
			if (wt->mouse_hover == LOWER_HALF && widget_text_can_scroll_down(wt))
				wt->scroll_offset++;
			return 1;

		case SDL_BUTTON_WHEELDOWN:
			if (widget_text_can_scroll_down(wt))
				wt->scroll_offset++;
			return 1;

		case SDL_BUTTON_WHEELUP:
			if (widget_text_can_scroll_up(wt))
				wt->scroll_offset--;
			return 1;
	}

	return 0;
}

//////////////////////////////////////////////////////////////////////
// Overloads of Base widget functions
//////////////////////////////////////////////////////////////////////

/**
 * \brief Display a text widget.
 * \relates widget_text
 *
 * \details The text is word-wrapped at the right border of the widget's
 * rectangle. The rendering start position is offseted to take the scrolling
 * value into account, and clipping is used to cut the rendering outside of
 * widget's rectangle.\n
 * If needed, content_above_func() and content_below_func() functions are called,
 * to display some additional UI parts (deprecated).
 *
 * \param w Pointer to the widget_text object
 *
 * TODO: This function has to be made static, when all panels will be converted
 * to the new GUI subsystem.
 */
void widget_text_display(struct widget *w)
{
	int lines_needed;
	int font_size;
	float visible_lines;
	int offset = 0;
	struct widget_text *wt = WIDGET_TEXT(w);

	// Set font before computing number of lines required.
	SetCurrentFont(wt->font);

	// Compute the number of lines required.
	lines_needed = get_lines_needed(wt->text->value, w->rect, wt->line_height_factor);

	// Get number of visible lines.
	font_size = FontHeight(wt->font) * wt->line_height_factor;
	visible_lines = (float) w->rect.h / (float) font_size;

	/* Disallow scrolling too far up or down. */
	if (lines_needed + wt->scroll_offset < floorf(visible_lines))
		wt->scroll_offset = floorf(visible_lines) - lines_needed;
	else if (wt->scroll_offset > 0)
		wt->scroll_offset = 0;

	/* Set the offset to display the relevant portions of the text, taking
	 * scrolling into account. */
	if (lines_needed > visible_lines)
		offset = font_size * (lines_needed - visible_lines + wt->scroll_offset);
	else
		wt->scroll_offset = 0;

	/* Ensure we do not show empty space before first line. */
	if (offset < 0)
		offset = 0;

	SDL_SetClipRect(Screen, NULL);
	display_text_using_line_height(wt->text->value, w->rect.x, w->rect.y - offset,
                                       &w->rect, wt->line_height_factor);

	/* If we have more content above or below the currently visible text, we call the
	 * functions that may have been specified for this event.
	 * TODO: Those functions are kept for compatibility with the old text widget, and
	 * should be removed as soon as all panels are converted to the new GUI subsystem:
	 * those additional widgets should be registered to the widget_text, and one of
	 * there attribute changed when the text is scrolled - for example, change the
	 * 'active' state of a widget_button */
	if (lines_needed > visible_lines) {
		if (wt->content_below_func != NULL && wt->scroll_offset != 0)
			wt->content_below_func();
		if (wt->content_above_func != NULL && wt->scroll_offset != (int)visible_lines - lines_needed)
			wt->content_above_func();
	}

	// Change mouse cursor as needed.
	if (widget_text_can_scroll_up(wt) && wt->mouse_hover == UPPER_HALF)
		mouse_cursor = MOUSE_CURSOR_SCROLL_UP;
	else if (widget_text_can_scroll_down(wt) && wt->mouse_hover == LOWER_HALF)
		mouse_cursor = MOUSE_CURSOR_SCROLL_DOWN;
}

/**
 * \brief Event handler for text widget.
 * \relates widget_text
 *
 * \details Depending on the position of the mouse pointer, a mouse
 * click will scroll up or down the widget's text: if the pointer is
 * over the upper half part of the widget's rectangle, it will scroll
 * up the text - on the lower half part, it will scroll down.\n
 * Mouse wheel event are also analyzed and used to scroll the text.
 *
 * \param w     Pointer to the widget_text object
 * \param event Pointer to the propagated event
 *
 * \return 1 if the event was handled and no further handling is required.
 */
static int text_handle_event(struct widget *w, SDL_Event *event)
{
	struct widget_text *wt = WIDGET_TEXT(w);
	switch (event->type) {
		case SDL_MOUSEBUTTONDOWN:
			return text_handle_mouse_down(wt, event);

		case SDL_MOUSEMOTION:
			// Keep track of the area being hovered.
			if (event->motion.y < w->rect.y + w->rect.h / 2)
				WIDGET_TEXT(w)->mouse_hover = UPPER_HALF;
			else
				WIDGET_TEXT(w)->mouse_hover = LOWER_HALF;
			return 1;

		case SDL_USEREVENT:
			// Reset mouse hover flag.
			if (event->user.code == EVENT_MOUSE_LEAVE)
				WIDGET_TEXT(w)->mouse_hover = NOT_HOVERED;
			return 1;
	}

	return 0;
}

/**
 * Handle mouse clicks.
 *
 * \deprecated Only used by addon_crafting_ui.c - Should be removed
 *
 * \return TRUE if a mouse click was handled, and no further handling should be
 * done by the caller
 */
int widget_text_handle_mouse(struct widget_text *w)
{
	int mouse_over_widget = MouseCursorIsInRect(&WIDGET(w)->rect, GetMousePos_x(), GetMousePos_y());
	int mouse_over_upper_half = (GetMousePos_y() - WIDGET(w)->rect.y) < (WIDGET(w)->rect.h / 2);
	int mouse_over_lower_half = (GetMousePos_y() - WIDGET(w)->rect.y) >= (WIDGET(w)->rect.h / 2);

	/* Change the mouse cursor as needed. */
	if (mouse_over_widget) {
		if (widget_text_can_scroll_up(w) && mouse_over_upper_half)
			mouse_cursor = MOUSE_CURSOR_SCROLL_UP;
		else if (widget_text_can_scroll_down(w) && mouse_over_lower_half)
			mouse_cursor = MOUSE_CURSOR_SCROLL_DOWN;
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

//////////////////////////////////////////////////////////////////////
// Text Widget
//////////////////////////////////////////////////////////////////////

/**
 * \brief Create a text widget and initialize it.
 * \ingroup guid2_text
 *
 * \return A pointer to the newly created widget_text.
 */
struct widget_text *widget_text_create()
{
	struct widget_text *wt = MyMalloc(sizeof(struct widget_text));
	widget_init(WIDGET(wt));
	WIDGET(wt)->display = widget_text_display;
	WIDGET(wt)->handle_event = text_handle_event;
	
	widget_text_init(wt, "");
	
	return wt;
}

/**
 * \brief Initialize or reset a text widget.
 * \ingroup gui2d_text
 *
 * \details Replace the current content with a new one, and reset internal
 * attributes.
 *
 * \param w          Pointer to the widget_text object
 * \param start_text Pointer to the new text content (the text is copied)
 */
void widget_text_init(struct widget_text *w, const char *start_text)
{
	if (w->text == NULL)
		w->text = alloc_autostr(10000);
	w->text->length = 0;
	autostr_printf(w->text, "%s", start_text);

	w->mouse_hover = NOT_HOVERED;
	w->mouse_already_handled = 0;
	w->scroll_offset = 0;
	w->line_height_factor = 1.0;
	w->content_above_func = NULL;
	w->content_below_func = NULL;
}

/**
 * \brief Check if the content text can be scrolled up.
 * \ingroup gui2d_text
 *
 * \details A text can be scrolled up if it is longer than the widget's height
 * \b and if it is not yet already scrolled up to its maximum.
 *
 * \param w Pointer to the widget_text object.
 *
 * \return TRUE if it is possible to scroll up.
 */
int widget_text_can_scroll_up(struct widget_text *w)
{
	SetCurrentFont(w->font);
	int lines_needed = get_lines_needed(w->text->value, WIDGET(w)->rect, w->line_height_factor);
	const int font_size = FontHeight(w->font) * w->line_height_factor;
	float visible_lines = (float) WIDGET(w)->rect.h / (float) font_size;

	return lines_needed > visible_lines &&
		w->scroll_offset != ((int)visible_lines - lines_needed);
}

/**
 * \brief Check if the context text can be scrolled down.
 * \ingroup gui2d_text
 *
 * \details A text can be scrolled down if it has already been scrolled up.
 *
 * \param w Pointer to the widget_text object.
 *
 * @return TRUE if it is possible to scroll down.
 */
int widget_text_can_scroll_down(struct widget_text *w)
{
	return w->scroll_offset != 0;
}
