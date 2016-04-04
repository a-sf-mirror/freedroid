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
 * \file widget_text.h
 * \brief This file contains the declaration of structure type and functions
 *        defining a scrollable text widget type.
 */

#ifndef _widget_text_h
#define _widget_text_h

/**
 * \brief Enumeration of the mouse hovering states, relative to the widget's rectangle.
 */
enum mouse_text_hover {
	NOT_HOVERED,
	UPPER_HALF,
	LOWER_HALF
};

///////////////////////////////////////////////////////////////////////////////
/// \defgroup gui2d_text Text widget type
/// \ingroup gui2d
///
/// The \e widget_text inherits from the base widget and implements a widget
/// than can display a multi-lines text and scroll it.\n
/// Only vertical scrolling is implemented. The text is word-wrapped at the
/// right edge of the widget's rectangle.\n
/// \n
/// The text contained in the widget is rendered using the standard fdRPG text
/// engine. Escape codes can thus be used to switch the current font.\n
/// \n
/// The interaction used to scroll the text is as follows:
///   \li a left mouse click on the upper half part of the widget scrolls the text up.
///   \li a left mouse click on the lower half part of the widget scrolls the text down.
///   \li the mouse wheel rotation is also used to scroll the text up or down.
///
/// During the display of the widget, two functions are called to render some
/// additional widget parts: content_below_func, called if there is some
/// scrolled text below the widget's rectangle, and content_above_func, called
/// if there is some scrolled text above the widget's rectangle.\n
/// The use of those functions is deprecated, and is only kept for compatibility
/// with the \e old text widget usage.\n
/// \n
/// \note The code of this widget was already used before the introduction of
/// the GUI subsystem. Its use is thus still very intricate in the user code,
/// until all the game panels are converted to the new GUI subsystem.
///
/// Currently, there are no specific functions to handle the text of the widget,
/// apart from widget_text_init(), which replaces all the current content with
/// a new one. Adding/removing/changing the content is to be done by the user
/// code.\n
///
/// \par Typical usage
///   \code
///   struct widget_text *my_textw = widget_test_create();
///   widget_set_rect(WIDGET(my_textw), x, y, w, h);
///   my_textw->font = FPS_Display_BFont;
///   /* Initial content */
///   string initial_text = "Initial text of the widget.\nSecond line.";
///   widget_text_init(my_textw, initial_text);
///   ... some time later ...
///   /* Add text */
///   autostr_append(my_textw->text, "\nA third line.");
///   \endcode
///
/// \par Localization
/// The text of the widget is displayed as is. However if the text has to be
/// translated when it is displayed, i.e. if it was not already translated
/// when the widget's content was set, use widget_text_l10n_at_display().
///@{
// start gui2d_text submodule

/**
 * \brief Widget text type.
 * 
 * This structure is used for displaying and handling text windows.
 */
struct widget_text {
	/// \name Base Type
	/// @{
	struct widget base;        /**< Pseudo-inheritance of the base widget type. */
	/// @}

	/// \name Public attributes
	/// @{
	struct auto_string *text;  /**< Text to be displayed. */
	int l10n_at_display;       /**< TRUE is the text is to be translated when displaying it (the text is not already translated). */
	struct font *font;         /**< Font to be used when displaying text. */
	float line_height_factor;  /**< Scale factor applied to line spacing. */
	int scroll_offset;         /**< Offset for the text being displayed. 0 means bottom, negative means above bottom. */

	void (*content_below_func)(void);
	void (*content_above_func)(void);
	/// @}

	/// \name Private attributes
	///       Needed to handle internal state of the widget.
	/// @{
	enum mouse_text_hover mouse_hover;  /**< Area hovered by the mouse. */
	int mouse_already_handled;          /**< Flag used for handling input. Deprecated. */
	/// @}
};

/**
 * \brief Type casting macro.
 *
 * Cast a pointer to a type inheriting from a \e base \e widget into a pointer to
 * a \e widget_text.
 */
#define WIDGET_TEXT(x) ((struct widget_text *)x)

struct widget_text *widget_text_create(void);
void widget_text_init(struct widget_text *, const char *);
void widget_text_l10n_at_display(struct widget_text *, int);
int widget_text_can_scroll_up(struct widget_text *);
int widget_text_can_scroll_down(struct widget_text *);
void widget_text_scroll_up(struct widget_text *);
void widget_text_scroll_down(struct widget_text *);
int widget_text_handle_mouse(struct widget_text *);
void widget_text_display(struct widget *);

// end gui2d_text submodule
///@}
///////////////////////////////////////////////////////////////////////////////

#endif // _widget_text_h_
