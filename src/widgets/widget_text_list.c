/*
 *
 *  Copyright (c) 2011 Catalin Badea
 *  Copyright (c) 2012 Samuel Degrande
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
 * \file widget_text_list.c
 * \brief This file contains the implementation of the widget_text_list functions.
 */

#include "system.h"

#include "defs.h"
#include "struct.h"
#include "global.h"
#include "proto.h"

#include "widgets/widgets.h"

/**
 * \brief Structure type used for storing a text entry.
 */
struct text_list_entry {
	char *text;          /**< Pointer to the text. Must be already allocated. No ownership is transferred. */
	int data;            /**< An integer user data (example: Index of this text entry in an other list). */
	int is_allocated;    /**< If true, the memory holding the text is allocated by the widget. */
	SDL_Rect rect;       /**< Outer rectangle used by the text entry. */
	SDL_Rect text_rect;  /**< Inner rectangle used by the text entry. */
};

static void text_list_selected_up(struct widget_text_list *);
static void text_list_selected_down(struct widget_text_list *);
static void widget_text_list_free(struct widget *w);

#define WTL_TEXT_PADDING 5

/**
 * \brief Compute the visible entries.
 *
 * \details This functions computes the last visible option in the list and
 * updates the rectangles of all currently visible entries.
 *
 * \param wl Pointer the the widget_text_list object
 */
static void compute_visible_lines(struct widget_text_list *wl)
{
	struct text_list_entry *list_entries = wl->entries.arr;

	// Compute the number of lines that can be displayed.
	set_current_font(wl->font);

	// Compute the number of lines that can be displayed.
	int font_height = get_font_height(wl->font);
	int visible_lines = WIDGET(wl)->rect.h / font_height;

	SDL_Rect rect = WIDGET(wl)->rect;
	rect.h = 0;

	SDL_Rect text_rect = rect;
	text_rect.x += WTL_TEXT_PADDING;
	text_rect.w -= 2*WTL_TEXT_PADDING;

	// Reset the last visible entry index.
	wl->last_visible_entry = -1;
	wl->all_entries_visible = TRUE;

	int i;
	int line_count = 0;
	for (i = wl->first_visible_entry; i < wl->entries.size; i++) {
		int lines_needed = get_lines_needed(list_entries[i].text, text_rect, 1.0);

		// Check if there's enough space left for a new entry to be displayed.
		if (line_count + lines_needed > visible_lines) {
			wl->all_entries_visible = FALSE;
			break;
		}
		line_count += lines_needed;
		wl->last_visible_entry = i;

		// Update each visible option's rectangle.
		text_rect.y = rect.y += rect.h;
		text_rect.h = rect.h = lines_needed * font_height;
		list_entries[i].rect = rect;
		list_entries[i].text_rect = text_rect;
	}
}

/**
 * \brief Find the entry hovered by the mouse cursor.
 *
 * \details This function determines the currently mouse hovered entry.
 * Keep previous one if none if hovered.
 *
 * \param wl    Pointer to the widget_text_list object
 * \param event Pointer to the propagated event
 *
 * \return TRUE if the mouse is hovering an entry, FALSE otherwise.
 */
static int compute_selected_entry(struct widget_text_list *wl, SDL_Event *event)
{
	int i;
	struct text_list_entry *list_entries = wl->entries.arr;

	for (i = wl->first_visible_entry; i <= wl->last_visible_entry; i++) {
		if (MouseCursorIsInRect(&list_entries[i].rect, event->motion.x, event->motion.y)) {
			wl->selected_entry = i;
			return TRUE;
		}
	}
	return FALSE;
}

//////////////////////////////////////////////////////////////////////
// Overloads of Base widget functions
//////////////////////////////////////////////////////////////////////

/**
 * \brief Display a text-list widget.
 * \relates widget_text_list
 *
 * \details !the text is word-wrapped at the right border of the widget's
 * rectangle. The rendering start with the first_visible_entry, and stop
 * on the last_visible_entry.\n
 * The selected_entry is highlighted.
 *
 * \param w Pointer to the widget_text object
 */
static void text_list_display(struct widget *w)
{
	struct widget_text_list *wl = WIDGET_TEXT_LIST(w);
	struct text_list_entry *list_entries = wl->entries.arr;

	compute_visible_lines(wl);
	set_current_font(wl->font);

	int i;
	for (i = wl->first_visible_entry; i <= wl->last_visible_entry; i++) {
		struct text_list_entry *elt = &list_entries[i];

		// Highlight the current selected entry
		if (wl->selected_entry == i)
			HighlightRectangle(Screen, elt->rect);

		display_text(elt->text, elt->text_rect.x, elt->text_rect.y, &elt->text_rect, 1.0);
	}
}

/**
 * \brief Event handler for the text-list widget.
 * \relates widget_text_list
 *
 * \details Scroll the text entries on mouse wheel events, and call
 * widget_text_list::process_entry() when the left button of the mouse is
 * released on a previously selected entry (by a left button press event).
 * The keyboard can also be used to scroll the list (up and down keys) and
 * process a selected entry (space or return keys).
 *
 * \param w     Pointer to the widget_text object
 * \param event Pointer to the propagated event
 *
 * \return 1 if the event was handled and no further handling is required.
 */
static int text_list_handle_event(struct widget *w, SDL_Event *event)
{
	struct widget_text_list *wl = WIDGET_TEXT_LIST(w);

	switch (event->type) {
		case SDL_MOUSEBUTTONDOWN:
			switch (event->button.button) {
				case SDL_BUTTON_WHEELUP:
					text_list_selected_up(wl);
					return 1;
				case SDL_BUTTON_WHEELDOWN:
					text_list_selected_down(wl);
					return 1;
				case MOUSE_BUTTON_1:
					// Mark the entry as pressed. Used when the mouse button is
					// released, to detect an actual 'click'.
					wl->pressed_entry = -1;
					if (compute_selected_entry(wl, event)) {
						wl->pressed_entry = wl->selected_entry;
						return 1;
					}
					break;
				default:
					break;
			}
			break;

		case SDL_MOUSEBUTTONUP:
			// Check if a valid mouse press has previously occurred on
			// the currently selected entry, and if so call process_entry().
			if (event->button.button == MOUSE_BUTTON_1) {
				if (wl->pressed_entry == wl->selected_entry) {
					if (wl->process_entry)
						wl->process_entry(wl);
					return 1;
				}
			}
			break;

		case SDL_MOUSEMOTION:
			{
				// Remove all motion events from the event stack and use the latest
				// one.
				SDL_Event evt1, evt2 = *event;
				while (SDL_PeepEvents(&evt1, 1, SDL_GETEVENT, SDL_MOUSEMOTIONMASK) > 0)
					evt2 = evt1;
				// The entry hovered by the mouse cursor is the new selected entry.
				compute_selected_entry(wl, &evt2);
			}
			break;

		case SDL_KEYDOWN:
			if (!MouseCursorIsInRect(&WIDGET(wl)->rect, GetMousePos_x(), GetMousePos_y()))
				break;
			switch (event->key.keysym.sym) {
				case SDLK_UP:
					text_list_selected_up(wl);
					return 1;
				case SDLK_DOWN:
					text_list_selected_down(wl);
					return 1;
				case SDLK_RETURN:
				case SDLK_SPACE:
					if (wl->process_entry)
						wl->process_entry(wl);
					return 1;
				default:
					break;
			}
			break;

		case SDL_USEREVENT:
			if (event->user.code == EVENT_MOUSE_LEAVE)
				// Reset the pressed option index.
				wl->pressed_entry = -1;
			break;
	}
	return 0;
}

//////////////////////////////////////////////////////////////////////
// Text-list Widget
//////////////////////////////////////////////////////////////////////

/**
 * \brief Create a text-list widget and initialize it.
 * \ingroup gui2d_textlist
 *
 * \return a pointer to the newly created widget_text_list.
 */
struct widget_text_list *widget_text_list_create()
{
	char *empty_list[] = { NULL };

	struct widget_text_list *wl = MyMalloc(sizeof(struct widget_text_list));
	widget_init(WIDGET(wl));
	WIDGET(wl)->display = text_list_display;
	WIDGET(wl)->handle_event = text_list_handle_event;
	WIDGET(wl)->free = widget_text_list_free;

	wl->font = FPS_Display_Font;

	dynarray_init(&wl->entries, 5, sizeof(struct text_list_entry));
	widget_text_list_init(wl, empty_list, NULL);

	wl->process_entry = NULL;

	return wl;
}

/**
 * \brief Initialize the widget content, and reset the widget state.
 * \ingroup gui2d_textlist
 *
 * \details Replace the current list of text entries with a new one, and reset
 * internal attributes. If some of the current text entries were duplicated,
 * they are freed. Store the string pointers without duplication, so no memory
 * is allocated.
 *
 * \param wl       Pointer to the widget_text_list_object
 * \param text_arr Array of strings that will populate the 'text' array of the
 *                 widget. The last element of this array must be NULL.
 * \param data_arr Array of integers that will populate the 'data' array of
 *                 the widget.
 *                 If 'data_arr' is NULL, the 'data' array is filled with an
 *                 auto-incremented value, starting at 0.
 *                 No check is done on the array size, so the user has to take
 *                 care to provide at least as many values as in the 'text_arr'.
 */
void widget_text_list_init(struct widget_text_list *wl, string text_arr[], int *data_arr)
{
	int i;

	// Reset the entries dynarray, freeing allocated strings if needed
	struct text_list_entry *list_entries = wl->entries.arr;
	for (i = 0; i < wl->entries.size; i++) {
		if (list_entries[i].is_allocated) {
			free(list_entries[i].text);
		}
		list_entries[i].text = NULL;
	}
	wl->entries.size = 0;

	// Fill with new content
	struct text_list_entry one_entry = {NULL, -1, FALSE, {0, 0, 0, 0}, {0, 0, 0, 0}};

	i = 0;
	while (text_arr[i] && strlen(text_arr[i])) {
		one_entry.text = text_arr[i];
		if (data_arr)
			one_entry.data = data_arr[i];
		else
			one_entry.data = i;
		dynarray_add(&wl->entries, &one_entry, sizeof(struct text_list_entry));
		i++;
	}

	// Reset the indexes
	wl->selected_entry = 0;
	wl->pressed_entry = -1;
	wl->first_visible_entry = 0;
	wl->last_visible_entry = -1;
	wl->all_entries_visible = TRUE;
}

static void widget_text_list_free(struct widget *w)
{
	struct widget_text_list *wtl = WIDGET_TEXT_LIST(w);

	dynarray_free(&wtl->entries);

	widget_free(w);
}

/**
 * \brief Add an entry to a widget list, by pointer copy.
 * \ingroup gui2d_textlist
 *
 * \details Add an entry at the end of the current list of text entries. Store
 * the string pointer without duplication, so no memory is allocated.
 *
 * \param wl   Pointer to the widget_text_list_object
 * \param text Pointer to a string to be added to the list of entries.
 * \param data Integer value to be added as the 'data' of the added entry.
 */
void widget_text_list_add(struct widget_text_list *wl, string text, int data)
{
	struct text_list_entry one_entry = {text, data, FALSE, {0, 0, 0, 0}, {0, 0, 0, 0}};
	dynarray_add(&wl->entries, &one_entry, sizeof(struct text_list_entry));
}

/**
 * \brief Add an entry to a widget list, duplicating the string.
 * \ingroup gui2d_textlist
 *
 * \details Add an entry at the end of the current list of text entries. The
 * string is internally duplicated with memory allocation.
 *
 * \param wl   Pointer to the widget_text_list_object
 * \param text Pointer to a string to be added to the list of entries.
 * \param data Integer value to be added as the 'data' of the added entry.
 */
void widget_text_list_dupadd(struct widget_text_list *wl, const char *text, int data)
{
	char *dup_text = strdup(text);
	struct text_list_entry one_entry = {dup_text, data, TRUE, {0, 0, 0, 0}, {0, 0, 0, 0}};
	dynarray_add(&wl->entries, &one_entry, sizeof(struct text_list_entry));
}

/**
 * \brief Return the user data associated to a given entry of a widget list.
 * \ingroup gui2d_textlist

 * \param wl    Pointer to the widget_text_list_object
 * \param index Index of the entry from which to return the user data
 *
 * @return The user data of the list's entry.
 */
int widget_text_list_get_data(struct widget_text_list *wl, int index)
{
	struct text_list_entry *entry = (struct text_list_entry *)dynarray_member(&wl->entries, index, sizeof(struct text_list_entry));
	return entry->data;
}

/**
 * \brief Scroll up the position of the highlighted entry.
 * \relates widget_text_list
 *
 * \details This function decrements the position of the current selected entry
 * and updates the visible lines if necessary.
 * The mouse cursor is moved to be vertically aligned with the selection.
 *
 * \param wl         Pointer to the widget_text_list
 */
static void text_list_selected_up(struct widget_text_list *wl)
{
	if (wl->selected_entry <= 0)
		return;

	// Change selection and play sound.
	wl->selected_entry--;
	MoveMenuPositionSound();

	// Update visible lines if necessary.
	if (wl->first_visible_entry > wl->selected_entry) {
		wl->first_visible_entry = wl->selected_entry;
		compute_visible_lines(wl);
	}

	// Warp the mouse to the new selected option.
	struct text_list_entry *list_entries = wl->entries.arr;
	SDL_Rect rect = list_entries[wl->selected_entry].rect;
	SDL_WarpMouse(GetMousePos_x(), rect.y + rect.h / 2);
}

/**
 * \brief Scroll down the position of the highlighted entry.
 * \relates widget_text_list
 *
 * \details This function increments the position of the current selected entry
 * and updates the visible lines if necessary.
 * The mouse cursor is moved to be vertically aligned with the selection.
 *
 * \param wl         Pointer to the widget_text_list
 */
static void text_list_selected_down(struct widget_text_list *wl)
{
	if (wl->selected_entry >= wl->entries.size - 1)
		return;

	// Change selection and play sound.
	wl->selected_entry++;
	MoveMenuPositionSound();

	// Update visible lines if necessary.
	if (wl->last_visible_entry < wl->selected_entry) {
		wl->first_visible_entry++;
		compute_visible_lines(wl);
	}

	// Warp the mouse to the new selected option.
	struct text_list_entry *list_entries = wl->entries.arr;
	SDL_Rect rect = list_entries[wl->selected_entry].rect;
	SDL_WarpMouse(GetMousePos_x(), rect.y + rect.h / 2);
}

/**
 * \brief Check if a text-list widget can be scrolled up.
 * \ingroup gui2d_textlist
 *
 * \details A text-list widget can be scrolled up as soon as the first visible
 * entry is not the first text entry in the entries list.
 *
 * \param wl Pointer to the widget_text_list object
 *
 * \return TRUE if the text-list can be scrolled up
 */
int widget_text_list_can_scroll_up(struct widget_text_list *wl)
{
	return (wl->first_visible_entry > 0);
}

/**
 * \brief Check if a text-list widget can be scrolled down.
 * \ingroup gui2d_textlist
 *
 * \details A text-list widget can be scrolled down if the last text entry is
 * not the last visible entry.
 *
 * \param wl Pointer to the widget_text_list object
 *
 * \return TRUE if the text-list can be scrolled down
 */
int widget_text_list_can_scroll_down(struct widget_text_list *wl)
{
	return (!wl->all_entries_visible && (wl->last_visible_entry <= wl->entries.size));
}

/**
 * \brief Scroll up the whole list.
 * \ingroup gui2d_textlist
 *
 * \details This function decrements the position of the first visible entry,
 * updates the visible lines, and move the position of the highlighted entry if
 * necessary.
 * The mouse cursor position is not changed.
 *
 * \param wl         Pointer to the widget_text_list
 */
void widget_text_list_scroll_up(struct widget_text_list *wl)
{
	if (!widget_text_list_can_scroll_up(wl))
		return;

	// Scroll the list and play sound.
	wl->first_visible_entry--;
	compute_visible_lines(wl);
	MoveMenuPositionSound();

	// Update position of highlight if necessary.
	if (wl->selected_entry > wl->last_visible_entry) {
		wl->selected_entry = wl->last_visible_entry;
	}
}

/**
 * \brief Scroll down the whole list.
 * \ingroup gui2d_textlist
 *
 * \details This function increments the position of the first visible entry,
 * updates the visible lines, and move the position of the highlighted entry if
 * necessary.
 * The mouse cursor position is not changed.
 *
 * \param wl         Pointer to the widget_text_list
 */
void widget_text_list_scroll_down(struct widget_text_list *wl)
{
	if (!widget_text_list_can_scroll_down(wl))
		return;

	// Scroll the list and play sound.
	wl->first_visible_entry++;
	compute_visible_lines(wl);
	MoveMenuPositionSound();

	// Update position of highlight if necessary.
	if (wl->selected_entry < wl->first_visible_entry) {
		wl->selected_entry = wl->first_visible_entry;
	}
}
