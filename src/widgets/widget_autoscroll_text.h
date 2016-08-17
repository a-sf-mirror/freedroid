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
 * \file widget_autoscroll_text.h
 * \brief This file contains the declaration of structure type and functions
 *        defining an autoscroll text widget type. Typical usage: title screen.
 */

#ifndef _widget_autoscroll_text_h
#define _widget_autoscroll_text_h

///////////////////////////////////////////////////////////////////////////////
/// \defgroup gui2d_autoscroll_text Autoscroll text widget type
/// \ingroup gui2d
///
/// The \e widget_autoscroll_text inherits from the base widget and implements
/// a widget than can display a multi-lines text and scroll it automatically
/// from under the bottom of the widget to over its top.\n
/// The text is word-wrapped at the right edge of the widget's rectangle.\n
/// \n
/// The text contained in the widget is rendered using the standard fdRPG text
/// engine. Escape codes can thus be used to switch the current font.\n
/// \n
/// The scrolling speed is initialized to 1 line per 2 seconds, in the 'up'
/// direction. The scrolling speed can be sped up or slowed down by user's
/// interaction. Once the speed comes to zero, the scrolling direction is
/// reversed.
/// \n
/// The interaction used to change the scrolling speed is as follows:
///   \li a left mouse click on one of the 'scroll buttons'.
///   \li a use of the 'up' and 'down' keyboard keys.
///   \li the mouse wheel rotation is also used to scroll the text up or down.
///
/// \par Typical usage
///   \code
/// struct widget_autoscroll_text *my_textw = widget_autoscroll_test_create();
/// widget_set_rect(WIDGET(my_textw), x, y, w, h);
/// /* Set content */
/// char *text = "Multine line text....";
/// struct font* font = Para_Font;
/// widget_autoscroll_set_text(my_textw, text, font);
///   \endcode
///
/// \par Special feature: Calling a function when a line scrolls in or out
///   \n
///   This feature can be used, for instance, to change the screen background
///   when a given number of lines are scrolled in from the bottom of
///   the widget.
///   \n
///   The function to register a callback at a given line is
///   \e widget_autoscroll_text_call_at_line(my_textw, line_number, pointer_to_function).
///   \n
///   If \e line_number is a negative value, the callback is called when
///   a given number of lines are still visible before the text scrolls out
///   at the top of the widget (-1 = no more visible lines, -2 = one visible line, ...).
///   \n
///   \note The callback is called only once, when the given line is first triggered.
///   This feature should then only be used when the user's capability to change the
///   scrolling direction is disabled (see widget_autoscroll_text_disable_scroll_interaction()).
///   \n
///   Example:
///   \code
/// void change_background(struct widget_autoscroll_text *w, int at_line)
/// {
///   if (at_line == 5)
///     ... set background to something ...
///   else
///     ... set background to something else ...
/// }
/// ...
/// widget_autoscroll_text_call_at_line(my_textw, 5, change_background);
/// widget_autoscroll_text_call_at_line(my_textw, -5, change_background);
///   \endcode
///
///@{
// start gui2d_text submodule

struct widget_autoscroll_text;

/**
 * \brief Widget autoscroll text type.
 * 
 * This structure is used for displaying and handling autoscrolling text windows.
 */
struct widget_autoscroll_text {
	/// \name Base Type
	/// @{
	struct widget base;        /**< Pseudo-inheritance of the base widget type. */
	/// @}

	/// \name Private attributes
	///       Needed to handle internal state of the widget.
	/// @{
	char *text;                              /**< Text to be displayed. */
	struct font *font;                       /**< Font to be used when displaying text. */
	int new_content;                         /**< New text stored, internal state must be recomputed **/
	float scrolling_speed;                   /**< 'base' scrolling speed **/
	int scrolling_speed_mult;                /**< Multiplier applied to the 'base' scrolling speed. Can be negative, for a scrolling in the 'down' direction **/
	float offset_current;                    /**< Current offset applied to display the text **/
	int offset_start;                        /**< Start offset: text is 'under' the bottom of the widget **/
	int offset_stop;                         /**< Stop offset: text is 'over' the top of the widget **/
	int scroll_interaction_disabled;         /**< Boolean flag to enable/disable the modification of the scrolling speed **/
	struct dynarray line_reached_callbacks;  /**< Functions called when a given line is reached during the scrolling **/
	/// @}
};

/**
 * \brief Type casting macro.
 *
 * Cast a pointer to a type inheriting from a \e base \e widget into a pointer
 * to a \e widget_autoscroll_text.
 */
#define WIDGET_AUTOSCROLL_TEXT(x) ((struct widget_autoscroll_text *)x)

struct widget_autoscroll_text *widget_autoscroll_text_create(void);
void widget_autoscroll_set_text(struct widget_autoscroll_text *, const char *, struct font *font);
void widget_autoscroll_disable_scroll_interaction(struct widget_autoscroll_text *, int);
void widget_autoscroll_call_at_line(struct widget_autoscroll_text *, int, void (*)(struct widget_autoscroll_text *, int));
int widget_autoscroll_text_can_scroll_up(struct widget_autoscroll_text *);
int widget_autoscroll_text_can_scroll_down(struct widget_autoscroll_text *);
void widget_autoscroll_text_scroll_up(struct widget_autoscroll_text *);
void widget_autoscroll_text_scroll_down(struct widget_autoscroll_text *);

// end gui2d_autoscroll_text submodule
///@}
///////////////////////////////////////////////////////////////////////////////

#endif // _widget_autoscroll_text_h_
