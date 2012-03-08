/* 
 *
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
 * \file widget_group.h
 * \brief This file contains the declaration of structure type and functions
 *        defining a group widget type.
 */

#ifndef _widget_group_h_
#define _widget_group_h_

///////////////////////////////////////////////////////////////////////////////
/// \defgroup gui2d_group Group widget type
/// \ingroup gui2d
///
/// The \e widget_group inherits from the base widget and implements a
/// container of widgets: it stores, displays and updates a group of
/// widgets. In a GUI tree, \e widget_groups are used as intermediate nodes.\n
/// \n
/// Children of a \e widget_group are stored in a linked list, the widget group
/// containing the head of that list.\n
/// \n
/// The widget system uses a recursive behavior for handling events and displaying
/// widgets. Widgets must be added to a group from back to front to ensure they
/// are handled properly.\n
/// \n
/// Displaying is done by looping the children list in normal order - widgets at
/// the back of the list will be able to cover the ones at the top of the list.
/// A child widget is displayed only if is enabled.\n
/// \n
/// Event forwarding is done by looping the children list in reverse order -
/// widgets covering other widgets will have priority in catching events.
/// An event is propagated to a child only if it is enabled.\n
/// \n
/// A widget group also detects when the mouse pointer leaves or enters one of
/// its children, and send leave/enter events to the involved children widgets.\n
/// \n
/// \note see \ref gui2d (Creating a GUI) for an example code.
///
///@{
// start gui2d_group submodule

/** 
 * \brief Widget Group type.
 *
 * This structure type is used for storing, displaying and updating a group of
 * widgets.\n
 * It does not introduce new pseudo-virtual functions.\n
 * \n
 * \b NOTE: Widget types inheriting widget_group must have it as their first attribute.
 */
struct widget_group {
	/// \name Base type

	/// @{
	struct widget base;          /**< Pseudo-inheritance of the base widget type. */
	/// @}

	/// \name Private internal attributes
	///       Needed for the management of a widget's tree, not meant to changed
	///       by the user.

	/// @{
	struct widget *last_focused; /**< Widget pointer used for handling enter/leave events. */
	struct list_head list;       /**< Head of the linked list used for storing children. */
	/// @}
};

/**
 * \brief Type casting macro.
 *
 * Cast a pointer to a type inheriting from a \e widget_group into a pointer to
 * a \e widget_group.
 */
#define WIDGET_GROUP(x) ((struct widget_group *)x)

struct widget_group *widget_group_create(void);
void widget_group_init(struct widget_group *);
int widget_group_add(struct widget_group *, struct widget *);
int widget_group_handle_event(struct widget *, SDL_Event *);

// end gui2d_group submodule
///@}
///////////////////////////////////////////////////////////////////////////////

#endif // _widget_group_h_
