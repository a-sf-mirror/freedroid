/* 
 *
 *   Copyright (c) 2010 Arthur Huillet
 *   Copyright (c) 2011 Catalin Badea
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
 * \file widget_button.h
 * \brief This file contains the declaration of structure type and functions
 *        defining a button widget type.
 */

#ifndef _widget_button_h_
#define _widget_button_h_

/**
 * \brief Enumeration of the button's states.
 */
enum state_enum {
	DEFAULT,
	HOVERED,
	PRESSED
};

///////////////////////////////////////////////////////////////////////////////
/// \defgroup gui2d_button Button widget type
/// \ingroup gui2d
///
/// The \e widget_button inherits from the base widget and implements the
/// behavior of a button.\n
/// \n
/// Its visual representation is composed of a background image (selected
/// depending on the current state of the button, see below) and a centered
/// foreground text.\n
/// \n
/// A tooltip can be defined for a button. It is assigned as being the global
/// current tooltip (see \ref gui2d_tooltip) when the mouse enters the
/// button's rectangle.\n
/// \n
/// Two actions (function callbacks) are associated to a button:\n
/// - the \e primary action is called when the left mouse button is released.
/// - the \e secondary action is called when the right mouse button is released.
///
/// \par Button states and background images
///   \n
///   Buttons have 3 primary states: normal, hovered and pressed:
///   \n
///   - \e pressed state is switched on on mouse press event and switched off
///   on mouse release or mouse leave event.
///   - \e hovered state is on when the mouse cursor is hovering the button
///     rectangle while no mouse buttons are pressed.
/// \par
///   A background image can be associated to each state. The image is scaled to
///   fill up the rectangle size of the button. If a specific background image
///   is not set, then the \e normal background image is used.\n
///   \n
///   Buttons can also be \e inactive (default value) or \e active.\n
///   This flag is used to choose between 2 sets of images, it does not denote
///   that a button is clickable or not.\n
///   If no background images are defined for the \e active set, then the images
///   of the \e inactive set are used.\n
///
/// \par Usage Example
///   \code
/// void button_pressed(struct widget_button *w)
/// {
///   w->active = FALSE;
///   ... do something when my_button is pressed ...
/// }
/// ...
/// struct widget_button *my_button = widget_button_create();
/// widget_set_rect(WIDGET(my_button), X, Y, W, H);
/// WIDGET(my_button)->update =  WIDGET_UPDATE_FLAG_ON_DATA(widget_button, active, Game.signal);
/// my_button->image[0][DEFAULT] = pointer_to_default_image;
/// my_button->image[0][HOVERED] = pointer_to_hovered_image;
/// my_button->image[0][PRESSED] = pointer_to_pressed_image;
/// my_button->image[1][DEFAULT] = pointer_to_default_active_image;
/// my_button->image[1][HOVERED] = pointer_to_hovered_active_image;
/// my_button->image[1][PRESSED] = pointer_to_pressed_active_image;
/// my_button->activate_button = button_pressed();
///   \endcode
///
///@{
// start gui2d_button submodule

/**
 * \brief Widget button type.
 *
 * This structure type is used for managing buttons.\n
 * Buttons have 3 main states (normal, hovered and pressed) and two
 * toggle states.
 */
struct widget_button {
	/// \name Base Type
	/// @{
	struct widget base;                       /**< Pseudo-inheritance of the base widget type. */
	/// @}

	/// \name Public attributes
	/// @{
	enum state_enum state;                    /**< Button primary state variable. */
	int active;                               /**< Flag for switching between the two toggle states. */
	struct image *image[2][3];                /**< Image pointers for each toggle state. */
	string text;                              /**< Text displayed in center of the button's rectangle. */
	struct tooltip tooltip;                   /**< Tooltip displayed on mouse hovering. */
	/// @}

	/// \name Behavior callbacks
	///       Functions to call when the button is \e activated (on a left or right mouse click)
	/// @{
	void (*activate_button) (struct widget_button *);           /**< Left click callback. */
	void (*activate_button_secondary) (struct widget_button *); /**< Right click callback. */
	/// @}
};

/**
 * \brief Type casting macro.
 *
 * Cast a pointer to a type inheriting from a \e base \e widget into a pointer to
 * a \e widget_button.
 */
#define WIDGET_BUTTON(x) ((struct widget_button *)x)

struct widget_button *widget_button_create(void);

// end gui2d_button submodule
///@}
///////////////////////////////////////////////////////////////////////////////

#endif // _widget_button_h_
