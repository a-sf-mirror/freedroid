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
 * \file widget_text_list.h
 * \brief This file contains the declaration of structure type and functions
 *        defining a widget type that contains a scrollable list of text entries,
 *        one being selectable.
 */

#ifndef _widget_text_list_h_
#define _widget_text_list_h_

///////////////////////////////////////////////////////////////////////////////
/// \defgroup gui2d_textlist Text-list widget type
/// \ingroup gui2d
///
/// The \e widget_text_list inherits from the base widget and implements a widget
/// that can display a scrollable list of text entries, and select one of them.\n
/// Only vertical scrolling is implemented. One typical example of its use is the
/// dialog selector subwindow of the chat.\n
/// \n
/// The text is word-wrapped at the right edge of the widget's rectangle. It is
/// rendered using the standard fdRPG text engine. Escape codes can thus be used
/// to switch the current font.\n
/// \n
/// The text entry under the mouse cursor is highlighted, and pressing the
/// up (resp. down) key scroll the highlight up (resp. down) in the list.\n
/// \n
/// An entry of the list is composed of two data: the text to be displayed,
/// and a user data. This user data can be used to store any integer value,
/// such as an index to an other array.\n
/// \n
/// Currently, there are no specific functions to handle the text list, apart
/// from widget_text_list_init(), which replaces all the current content with a
/// new one, and widget_text_list_add() to add one entry to the current list.
/// Removing/changing the content is to be done by the user code.\n
/// \n
/// \note widget_text_list_add() and widget_text_list_init() directly store the
/// string pointers without duplication of the text.\n
/// So the widget's user has to ensure that the strings are available while the
/// widget is used. The widget's user also has to free the current text entries
/// before to call widget_text_list_init().\n
/// \n
/// \note widget_text_list_dupadd() is a special version of widget_text_list_add()
/// that duplicates the string, avoiding  the widget's user to have to handle
/// deallocation when the widget's content is re-initialized.
/// \n
///
/// \par Callback on user's selection
///   \n
///   widget_text_list::process_entry() is called when a text entry is selected,
///   that is, when:
///     - the ENTER or SPACE key is pressed while the mouse cursor is on
///       the highlighted entry.
///     - the left mouse button is pressed and then released on the same text
///       entry (the mouse can move between the press and the release events).
///
///   \par
///   The widget_text_list::selected_entry field contains the index (starting at 0)
///   of the currently highlighted text entry.\n
///
/// \par Typical usage
///   \code
/// void selection_cb(struct widget_text_list *wl)
/// {
///   do_something(wl->selected_entry);
///   // The user data associated with that entry can be retrieved with:
///   int user_data = widget_text_list_get_data(wl, wl->selected_entry);
/// }
/// ...
/// struct widget_text_list *my_w = widget_test_list_create();
/// widget_set_rect(WIDGET(my_w), x, y, w, h);
/// my_w->font = FPS_Display_BFont;
/// my_w->process_entry = selection_cb;
/// /* Initial content */
/// string initial_text[] = { "First entry.", "Second entry.", NULL };
/// int initial_data[] = { 1, 2 };
/// widget_text_list_init(my_w, initial_text, initial_data);
/// ... some time later ...
/// /* Add text */
/// widget_text_list_add(my_w->text, "A third entry.", 10);
///   \endcode
///
/// \par Defining scroll-buttons
///   \n
///   Here is a typical code to define scroll-buttons and associate them with
///   a widget_text_list (only the code for the scroll-up button is shown):
///   \code
/// /* Global variables */
/// struct widget_text_list *option_list;
/// ...
/// void up_button_cb(struct widget_button *wb)
/// {
///   widget_text_list_scroll_up(option_list, FALSE);
/// }
/// ...
/// int main(...)
/// {
///   ...
///   option_list = widget_text_list_create();
///   ... define position and content of the text-list
///   struct widget_button *up_button = widget_button_create();
///   ... define position and content of the button
///   WIDGET(up_button)->update = WIDGET_UPDATE_FLAG_ON_DATA(WIDGET_BUTTON, active, widget_text_can_scroll_down(option_list));
///   up_button->activate_button = up_button_cb;
///   ...
/// }
///   \endcode
///@{
// start gui2d_text_list submodule

/**
 * \brief Widget Text List type.
 *
 * This structure is used for managing a scrollable list of text entries, one
 * being selectable.
 */
struct widget_text_list {
	/// \name Base Type
	/// @{
	struct widget base;  /**< Pseudo-inheritance of the base widget type. */
    /// @}

	/// \name Public attributes
	/// @{
	struct font *font;        /**< The font used for displaying text. */
	struct dynarray entries;  /**< Dynarray used for text and position info for each entry. */
	int selected_entry;       /**< The index of the currently highlighted entry. */
	/// @}

	/// \name Behavior callbacks
	///       Function to call when an entry is \e activated (on a left mouse
	///       click or enter/space key press)
	/// @{
	void (*process_entry)(struct widget_text_list *);  /**< Selection callback */
    /// @}

	/// \name Private attributes
	///       Needed to handle internal state of the widget.
	/// @{
	int pressed_entry;        /**< The index of the last entry on which a mouse press event was received. */
	int first_visible_entry;  /**< The index of the first visible entry. */
	int last_visible_entry;   /**< The index of the last visible entry. */
	int all_entries_visible;  /**< All entries are visible in the widget rect */
	/// @}
};

/**
 * \brief Type casting macro.
 *
 * Cast a pointer to a type inheriting from a \e base \e widget into a pointer to
 * a \e widget_text_list.
 */
#define WIDGET_TEXT_LIST(x) ((struct widget_text_list *)x)

struct widget_text_list *widget_text_list_create();
void widget_text_list_init(struct widget_text_list *, string[], int *);
void widget_text_list_add(struct widget_text_list *, string, int);
void widget_text_list_dupadd(struct widget_text_list *, const char *, int);
int widget_text_list_get_data(struct widget_text_list *, int);
int widget_text_list_can_scroll_up(struct widget_text_list *);
int widget_text_list_can_scroll_down(struct widget_text_list *);
void widget_text_list_scroll_up(struct widget_text_list *);
void widget_text_list_scroll_down(struct widget_text_list *);

// end gui2d_text_list submodule
///@}
///////////////////////////////////////////////////////////////////////////////

#endif // _widget_text_list_h_
