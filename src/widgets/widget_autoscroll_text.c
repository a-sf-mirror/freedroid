/*
 *
 *   Copyright (c) 2016 Samuel Degrande
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
 * \file widget_autoscroll_text.c
 * \brief This file contains the declaration of structure type and functions
 *        defining an autoscroll text widget type. Typical usage: title screen.
 */

#include "system.h"
#include "defs.h"
#include "struct.h"
#include "proto.h"
#include "global.h"
#include "widgets/widgets.h"

static void autoscroll_text_free(struct widget *w);

//////////////////////////////////////////////////////////////////////
// Overloads of Base widget functions
//////////////////////////////////////////////////////////////////////

/**
 * \brief Display an autoscroll text widget.
 * \relates widget_autoscroll_text
 *
 * \details The text is word-wrapped at the right border of the widget's
 * rectangle. The rendering start position of the text is offseted relatively
 * to the top the the widget. The offset is changed at each frame, depending
 * on the current scrolling speed.\n
 * Clipping is used to cut the rendering outside of widget's rectangle.
 *
 * \param w Pointer to the widget_autoscroll_text object
 */
static void autoscroll_text_display(struct widget *w)
{
	struct widget_autoscroll_text *wat = WIDGET_AUTOSCROLL_TEXT(w);

	if (!wat->text)
		return;

	set_current_font(wat->font);
	display_text(wat->text, w->rect.x, floorf((float)w->rect.y + wat->offset_current), &w->rect, 1.0f);
}

/**
 * \brief Update the offset of the text.
 * \relates widget_autoscroll_text
 *
 * \details The current scrolling speed is used to update the offset to apply
 * to the text. The offset is cliped to the [offset_stop, offset_start] range.
 *
 * @param w Pointer to the widget_autoscroll_text object
 */
static void autoscroll_text_update(struct widget *w)
{
	struct widget_autoscroll_text *wat = WIDGET_AUTOSCROLL_TEXT(w);

	if (!wat->text)
		return;

	// If a new text was submitted, recomputes offset limits and scrolling speed.
	if (wat->new_content) {
		set_current_font(wat->font);
		// At start, the text is displayed 'under' the bottom of widget's rect
		wat->offset_start = w->rect.h;
		wat->offset_current = (float)wat->offset_start;
		// Scroll ends when the last line of the text is displayed 'over' the widget's_rect
		int lines_needed = get_lines_needed(wat->text, w->rect, 1.0);
		wat->offset_stop = -lines_needed * get_font_height(get_current_font());
		// Scrolling speed: 1 line per 2 seconds
		wat->scrolling_speed = (float)get_font_height(get_current_font()) / 2.0f;
		wat->scrolling_speed_mult = 1;

		wat->new_content = FALSE;

		return;
	}

	// New text's offset
	wat->offset_current -= (wat->scrolling_speed * (float)wat->scrolling_speed_mult) * Frame_Time();

	// Impose some limit on the amount of scrolling
	if ((wat->offset_current >= wat->offset_start) && (wat->scrolling_speed_mult < 0)) {
		wat->offset_current = wat->offset_start;
		wat->scrolling_speed_mult = 0;
	}
	if ((wat->offset_current <= wat->offset_stop) && (wat->scrolling_speed_mult > 0)) {
		wat->offset_current = wat->offset_stop;
		wat->scrolling_speed_mult = 0;
	}
}

/**
 * \brief Event handler for autoscroll text widget.
 * \relates widget_autoscroll text
 *
 * \details Change scrolling speed and direction using:
 * - 'up' and 'down' keyboard keys.
 * - mouse wheel.
 *
 * \param w     Pointer to the widget_text object
 * \param event Pointer to the propagated event
 *
 * \return 1 if the event was handled and no further handling is required.
 */
static int autoscroll_text_handle_event(struct widget *w, SDL_Event *event)
{
	struct widget_autoscroll_text *wat = WIDGET_AUTOSCROLL_TEXT(w);
	switch (event->type) {
		case SDL_MOUSEBUTTONDOWN:
			switch (event->button.button) {
				case SDL_BUTTON_WHEELDOWN:
					widget_autoscroll_text_scroll_down(wat);
					return 1;
				case SDL_BUTTON_WHEELUP:
					widget_autoscroll_text_scroll_up(wat);
					return 1;
			}
			break;
		case SDL_KEYDOWN:
			switch (event->key.keysym.sym) {
				case SDLK_UP:
					widget_autoscroll_text_scroll_up(wat);
					return 1;
				case SDLK_DOWN:
					widget_autoscroll_text_scroll_down(wat);
					return 1;
				default:
					break;
			}
			break;
	}

	return 0;
}

//////////////////////////////////////////////////////////////////////
// Autoscroll Text Widget
//////////////////////////////////////////////////////////////////////

/**
 * \brief Create an autoscroll text widget and initialize it.
 * \ingroup gui2d_autoscroll_text
 *
 * \return A pointer to the newly created widget_autoscroll_text.
 */
struct widget_autoscroll_text *widget_autoscroll_text_create()
{
	struct widget_autoscroll_text *wat = MyMalloc(sizeof(struct widget_autoscroll_text));
	widget_init(WIDGET(wat));
	WIDGET(wat)->display = autoscroll_text_display;
	WIDGET(wat)->handle_event = autoscroll_text_handle_event;
	WIDGET(wat)->update = autoscroll_text_update;
	WIDGET(wat)->free = autoscroll_text_free;

	wat->text = NULL; // No text currently submitted

	return wat;
}

/**
 * \brief Set the text to be displayed.
 * \ingroup gui2d_autoscroll_text
 *
 * \details Replace the current content with a new one, and reset internal
 * attributes.
 *
 * \param w          Pointer to the widget_autoscroll_text object
 * \param start_text Pointer to the new text content (the text is copied)
 * \param font       Pointer to the font to use
 */
void widget_autoscroll_set_text(struct widget_autoscroll_text *w, const char *text, struct font *font)
{
	if (w->text)
		free(w->text);
	w->text = strdup(text);
	w->font = font;
	w->offset_current = 0.0f;
	w->offset_start = 0;
	w->offset_stop = 1;
	w->new_content = TRUE;
}

static void autoscroll_text_free(struct widget *w)
{
	struct widget_autoscroll_text *wat = WIDGET_AUTOSCROLL_TEXT(w);

	if (wat->text) {
		free(wat->text);
		wat->text = NULL;
	}

	widget_free(w);
}

/**
 * \brief Check if the scrolling speed of the text can be changed in the 'up' direction.
 * \ingroup gui2d_autoscroll_text
 *
 * \details The 'scroll up speed' can be changed as long as the is not totally
 * 'over' the top of the widget.
 *
 * \param w Pointer to the widget_text object.
 *
 * \return TRUE if it is possible to 'scroll up'.
 */
int widget_autoscroll_text_can_scroll_up(struct widget_autoscroll_text *w)
{
	if (w->new_content)
		return FALSE;

	return (w->offset_current > w->offset_stop);
}

/**
 * \brief Check if the scrolling speed of the text can be changed in the 'down' direction.
 * \ingroup gui2d_autoscroll_text
 *
 * \details The 'scroll down speed' can be changed as long as the is not totally
 * 'under' the bottom of the widget.
 *
 * \param w Pointer to the widget_text object.
 *
 * \return TRUE if it is possible to 'scroll up'.
 */
int widget_autoscroll_text_can_scroll_down(struct widget_autoscroll_text *w)
{
	if (w->new_content)
		return FALSE;

	return (w->offset_current < w->offset_start);
}

/**
 * \brief Change scrolling speed in the 'up' direction.
 * \ingroup gui2d_autoscroll_text
 *
 * \details This function increments the scrolling speed multiplier (max value = 5)
 *
 * \param w         Pointer to the widget_text
 */
void widget_autoscroll_text_scroll_up(struct widget_autoscroll_text *w)
{
	if (w->new_content)
		return;

	if (w->scrolling_speed_mult < 5)
		w->scrolling_speed_mult++;
}

/**
 * \brief Change scrolling speed in the 'down' direction.
 * \ingroup gui2d_autoscroll_text
 *
 * \details This function decrements the scrolling speed multiplier (min value = -5)
 *
 * \param w         Pointer to the widget_text
 */
void widget_autoscroll_text_scroll_down(struct widget_autoscroll_text *w)
{
	if (w->new_content)
		return;

	if (w->scrolling_speed_mult > -5)
		w->scrolling_speed_mult--;
}
