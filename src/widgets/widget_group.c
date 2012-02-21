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
 * @file widget_group.c
 * @brief Contains callback functions used by the widget group.
 *
 * The widget system uses a recursive behaviour for handling events and displaying widgets.
 * Widgets must be added to a group from back to front to ensure they are handled properly.
 * Displaying is done by looping the children list in normal order - widgets at the back of
 * the list will be able to cover the ones at the top of the list.
 * Event forwarding is done by looping the children list in reverse order - widgets covering
 * other widgets will have priority in catching events.
 */
#define _widget_group_c

#include "system.h"
#include "defs.h"
#include "struct.h"
#include "global.h"
#include "proto.h"
#include "widgets/widgets.h"

/*
 * Display children widgets.
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
 * @brief Handles mouse motion and mouse button clicks events received by widget groups.
 * 
 * This function is used in widget_group_handle_event to handle mouse motion and button events.
 * The widget group handles these events by forwarding them to the first enabled children 
 * under the mouse.
 *
 * Mouse leave/enter events are handled recursively by each widget group.
 * Each widget group uses a last_focused widget pointer to monitor when a new children receives the event
 * and sends leave/enter events accordingly.
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
 * Pass the keyboard event to all enabled children until one returns true.
 */
static int group_keyboard_event(struct widget *wg, SDL_Event *event)
{
	struct widget *w;

	list_for_each_entry_reverse(w, &WIDGET_GROUP(wg)->list, node) {
		if (w->enabled && w->handle_event(w, event)) {
			return 1;
		}
	}
	return 0;
}

/**
 * @brief Handles update calls received by widget groups.
 * 
 * This function will call the group's update callback and forward
 * the update call to all children widgets if the group is enabled.
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

/**
 * Pass the mouse leave event to the last focused children widget if there is one.
 */
static int group_mouse_leave_event(struct widget *wg, SDL_Event *event)
{
	if (WIDGET_GROUP(wg)->last_focused) {
		WIDGET_GROUP(wg)->last_focused->handle_event(WIDGET_GROUP(wg)->last_focused, event);
		WIDGET_GROUP(wg)->last_focused = NULL;
	}
	return 1;
}

/**
 * This function handles all events received by a widget group.
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
 * Initialize widget group properties
 */
void widget_group_init(struct widget_group *wg)
{
	widget_init(WIDGET(wg));
	WIDGET(wg)->update_tree = group_update;
	WIDGET(wg)->display = group_display;
	WIDGET(wg)->handle_event = widget_group_handle_event;
	wg->list = (struct list_head)LIST_HEAD_INIT(wg->list);
	wg->last_focused = NULL;
}

/**
 * @brief Creates a widget_group.
 *
 * This function creates a widget_group using the default callbacks.
 * @return A pointer to the newly created widget_group.
 */
struct widget_group *widget_group_create() 
{
	struct widget_group *wb = (struct widget_group *)MyMalloc(sizeof(struct widget_group));
	widget_group_init(wb);
	return wb;
}

/**
 * This function adds a widget to a widget_group.
 * @param wg a pointer to the widget_group to which a new widget is added.
 * @param w a pointer to widget that is being added to the widget group.
 */
int widget_group_add(struct widget_group *wg, struct widget *w) 
{
	list_add_tail(&w->node, &wg->list);
	return 0;
}

#undef _widget_group_c
