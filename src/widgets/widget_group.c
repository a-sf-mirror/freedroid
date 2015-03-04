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
 * \file widget_group.c
 * \brief This file contains the implementation of the widget_group functions.
 *
 */

#define _widget_group_c

#include "system.h"
#include "defs.h"
#include "struct.h"
#include "global.h"
#include "proto.h"
#include "widgets/widgets.h"

static void widget_group_free(struct widget *w);

//////////////////////////////////////////////////////////////////////
// Specific event handlers
//////////////////////////////////////////////////////////////////////

/**
 * \brief Default handling of mouse motion and mouse button clicks events for a widget_group.
 * \relates widget_group
 * 
 * \details This function is used in widget_group_handle_event() to handle mouse motion
 * and button events. The widget group handles these events by forwarding them
 * to the first enabled children under the mouse.\n
 * \n
 * Mouse leave/enter events are handled recursively by each widget group.\n
 * Each widget group uses a last_focused widget pointer to monitor when a new
 * children receives the event and sends leave/enter events accordingly.
 *
 * \param wg    Pointer to the widget_group object
 * \param event Pointer to the caught event
 *
 * \return 1 if the event is consumed (stop the event propagation) or 0 if not.
 */
static int group_mouse_event(struct widget *wg, SDL_Event *event)
{	
	struct widget *w, *current_widget = NULL;
	SDL_Event enter_event;
	enter_event.type = SDL_USEREVENT;
	enter_event.user.code = EVENT_MOUSE_ENTER;
	SDL_Event leave_event;
	leave_event.type = SDL_USEREVENT;
	leave_event.user.code = EVENT_MOUSE_LEAVE;

	// Get the widget under the cursor.
	// The loop is done in reverse to ensure that widgets covering other widgets catch the event first.
	list_for_each_entry_reverse(w, &WIDGET_GROUP(wg)->list, node) {
		if (w->enabled && MouseCursorIsInRect(&w->rect, event->button.x, event->button.y)) {
			current_widget = w;
			break;
		}
	}

	// Send enter/leave events if the focused widget has changed.
	if (current_widget != WIDGET_GROUP(wg)->last_focused) {
		if (WIDGET_GROUP(wg)->last_focused)
			WIDGET_GROUP(wg)->last_focused->handle_event(WIDGET_GROUP(wg)->last_focused, &leave_event);
		if (current_widget)
			current_widget->handle_event(current_widget, &enter_event);
		WIDGET_GROUP(wg)->last_focused = current_widget;
	}

	// Let the child widget handle the event.
	if (current_widget)
		return current_widget->handle_event(current_widget, event);

	return 0;
}

/**
 * \brief Default handling of keyboard events for a widget_group.
 * \relates widget_group
 *
 * \details This function is used in widget_group_handle_event() to handle keyboard events.
 * It passes the keyboard event to all enabled children until one returns true
 * (i.e. one child consumed the event).
 *
 * \param wg    Pointer to the widget_group object
 * \param event Pointer to the caught event
 *
 * \return 1 if the event is consumed (stop the event propagation) or 0 if not.
 */
static int group_keyboard_event(struct widget *wg, SDL_Event *event)
{
	struct widget *w;

	// The loop is done in reverse to ensure that widgets covering other widgets catch the event first.
	list_for_each_entry_reverse(w, &WIDGET_GROUP(wg)->list, node) {
		if (w->enabled && w->handle_event(w, event)) {
			return 1;
		}
	}
	return 0;
}

/**
 * \brief Default handling of mouse_leave events for a widget_group.
 * \relates widget_group
 *
 * \details This function is used in widget_group_handle_event() to handle mouse leave
 * events.\n
 * It passes the mouse leave event to the last_focused child widget (if any),
 * and then resets the value of the last_focused pointer.
 *
 * \param wg    Pointer to the widget_group object
 * \param event Pointer to the caught event
 *
 * \return 1 (i.e the event is consumed)
 */
static int group_mouse_leave_event(struct widget *wg, SDL_Event *event)
{
	if (WIDGET_GROUP(wg)->last_focused) {
		WIDGET_GROUP(wg)->last_focused->handle_event(WIDGET_GROUP(wg)->last_focused, event);
		WIDGET_GROUP(wg)->last_focused = NULL;
	}
	return 1;
}

//////////////////////////////////////////////////////////////////////
// Overloads of Base widget functions
//////////////////////////////////////////////////////////////////////

/**
 * \brief Display the children of a widget_group.
 * \relates widget_group
 *
 * \details Loop on each child of the widget_group, and call their display function is
 * the child is enabled.
 *
 * \param w Pointer to the widget_group object
 */
static void group_display(struct widget *w)
{
	struct widget *widget;
	list_for_each_entry(widget, &WIDGET_GROUP(w)->list, node) {
		if (widget->enabled && widget->display)
			widget->display(widget);
	}
}

/**
 * \brief Event handler for a widget group.
 * \relates widget_group
 *
 * \details Calls specific functions to handle mouse events, keyboard events and
 * mouse leave event.
 *
 * \param wg    Pointer to the widget_group object
 * \param event Pointer to the caught event
 *
 * \return 1 if the event is consumed (stop the event propagation) or 0 if not.
 */
int widget_group_handle_event(struct widget *wg, SDL_Event *event)
{
	switch (event->type) {
		case SDL_MOUSEMOTION:
		case SDL_MOUSEBUTTONDOWN:
		case SDL_MOUSEBUTTONUP:
			return group_mouse_event(wg, event);
	
		case SDL_KEYDOWN:
			return group_keyboard_event(wg, event);

		case SDL_USEREVENT:
			switch(event->user.code) {
				case EVENT_MOUSE_LEAVE:
					return group_mouse_leave_event(wg, event);
				default:
					break;
			}
			break;

		default:
			break;
	}
	return 0;
}

/**
 * \brief update_tree() implementation for a widget_group
 * \relates widget_group
 *
 * \details On a widget_group, update_tree() calls the group's update() function, and forwards
 * the update call to all children widgets if the group is enabled.
 *
 * \param wg Pointer to the widget_group object
 */
static void group_update(struct widget *wg)
{
	struct widget *w;

	// Update the widget group.
	if (wg->update)
		wg->update(wg);

	// Propagate on all children
	if (wg->enabled) {
		list_for_each_entry(w, &WIDGET_GROUP(wg)->list, node) {
			w->update_tree(w);
		}
	}
}

//////////////////////////////////////////////////////////////////////
// Group Widget
//////////////////////////////////////////////////////////////////////

/**
 * \brief Initialize the properties and functions of a widget_group.
 * \ingroup gui2d_group
 *
 * \param wg Pointer to the widget group object
 */
void widget_group_init(struct widget_group *wg)
{
	widget_init(WIDGET(wg));
	WIDGET(wg)->display = group_display;
	WIDGET(wg)->handle_event = widget_group_handle_event;
	WIDGET(wg)->free = widget_group_free;
	wg->list = (struct list_head)LIST_HEAD_INIT(wg->list);
	wg->last_focused = NULL;
	WIDGET(wg)->update_tree = group_update;
}

/**
 * \brief Create a widget_group and initialize it.
 * \ingroup gui2d_group
 *
 * \return A pointer to the newly created widget_group.
 */
struct widget_group *widget_group_create() 
{
	struct widget_group *wb = (struct widget_group *)MyMalloc(sizeof(struct widget_group));
	widget_group_init(wb);
	return wb;
}

static void widget_group_free(struct widget *w)
{
	struct widget_group *wg = WIDGET_GROUP(w);
	struct widget *entry, *next;

	list_for_each_entry_safe(entry, next, &wg->list, node) {
		list_del(&entry->node);

		entry->free(entry);

		free(entry);
	}

	widget_free(w);
}

/**
 * \brief Add a widget to a widget_group.
 * \ingroup gui2d_group
 *
 * \details Any type deriving from \e base \e widget can be added as a child to a
 * widget_group.\n
 * \n
 * The widget is added at the end of the children linked list.
 *
 * \param wg Pointer to the widget_group to which a new widget is added.
 * \param w  Pointer to widget that is being added to the widget group.
 *
 * \return 0 (reserved for future use)
 */
int widget_group_add(struct widget_group *wg, struct widget *w) 
{
	list_add_tail(&w->node, &wg->list);
	return 0;
}

#undef _widget_group_c
