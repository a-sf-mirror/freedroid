/* 
 *
 *   Copyright (c) 2009 Arthur Huillet
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
 * \file widgets.c
 * \brief This file contains the implementation of the base widget functions,
 *        + some misc functions.
 */

#define _widgets_c

#include "system.h"
#include "defs.h"
#include "struct.h"
#include "global.h"
#include "proto.h"
#include "widgets/widgets.h"

////////////////////////////////////////////////////////////////////:
// Misc functions used internally by the widgets
////////////////////////////////////////////////////////////////////:

/**
 * \brief Images cache for widgets
 * \ingroup gui2d_misc
 *
 * \details Linked list used by widget_load_image_resource to store images.
 */
struct image_resource {
	char *name;
	struct image img;
	struct list_head node;
};

LIST_HEAD(image_resource_list);

/**
 * \brief This function is used to load the images used by the widget system.
 * \ingroup gui2d_misc
 *
 * \details A cache mechanism is implemented: if the image name is found in
 * the image_resource_list, then the already loaded image is returned.
 * Else, the image is loaded from disk, and stored in \e image_resource_list.
 */
struct image *widget_load_image_resource(char *name, int mod_flags)
{
	struct image_resource *res;

	if (!name)
		return NULL;
	
	// Check if image is already loaded.
	list_for_each_entry(res, &image_resource_list, node) {
		if (!strcmp(name, res->name))
			return &res->img;
	}

	// Image not found, allocate memory and load it from its file.
	res = MyMalloc(sizeof(struct image_resource));
	res->name = strdup(name);
	load_image(&res->img, res->name, mod_flags);
	list_add(&res->node, &image_resource_list);
	return &res->img;
}

void widget_free_image_resources()
{
	struct image_resource *res, *next;

	// Free loaeded images.
	list_for_each_entry_safe(res, next, &image_resource_list, node) {
		list_del(&res->node);

		if (res->name) {
			free(res->name);
		}

		delete_image(&res->img);

		free(res);
	}

	INIT_LIST_HEAD(&image_resource_list);
}

////////////////////////////////////////////////////////////////////:
// Tooltips handling
////////////////////////////////////////////////////////////////////:

/**
 * \brief Current tooltip
 *
 * \details Contains informations about the current tooltip to be displayed.
 */
static struct _tooltip_info {
	struct tooltip *value;	/**< Pointer to the current widget's tooltip. */
	SDL_Rect widget_rect;	/**< Tooltip owner rectangle. */
} tooltip_info;

/**
 * \brief This function changes the tooltip text to be displayed by display_tooltips.
 * \ingroup gui2d_tooltip
 *
 * \details To stop to display a tooltip, call this function with parameters set to NULL
 *
 * \param new_tooltip The new tooltip to be displayed.
 * \param widget_rect Rectangle of the tooltip owner (used to compute the tooltip's position).
 */
void widget_set_tooltip(struct tooltip *new_tooltip, SDL_Rect *widget_rect)
{
	if (!new_tooltip || !widget_rect) {
		tooltip_info.value = NULL;
		return;
	}

	if ((new_tooltip->get_text && new_tooltip->get_text()) || new_tooltip->text) {
		tooltip_info.value = new_tooltip;
		tooltip_info.widget_rect = *widget_rect;
	} else {
		tooltip_info.value = NULL;
	}
}

//////////////////////////////////////////////////////////////////////
// External entry points
//////////////////////////////////////////////////////////////////////

/**
 * \brief This function returns the current active user interface.
 *
 * \return A pointer to the root widget of the current active GUI.
 */
static struct widget *get_active_ui()
{
	static int old_game_status = -1;
	static struct widget *old_active_ui = NULL;
	static struct widget *new_active_ui = NULL;

	switch (game_status) {
		case INSIDE_LVLEDITOR:
			new_active_ui = WIDGET(get_lvledit_ui());
			break;

		case INSIDE_GAME:
			new_active_ui = WIDGET(get_game_ui());
			break;

		default:
			return NULL;
	}

	// If the current active interface has changed, send a
	// mouse leave event to the previous interface.
	if (old_game_status != game_status) {
		SDL_Event event;
		event.type = SDL_USEREVENT;
		event.user.code = EVENT_MOUSE_LEAVE;

		if (old_active_ui)
			old_active_ui->handle_event(old_active_ui, &event);
	}

	old_active_ui = new_active_ui;
	old_game_status = game_status;

	return new_active_ui;
}

/**
 * \brief Display the current active tooltip.
 *
 * \details A widget can set the current active tooltip by calling widget_set_tooltip().
 * This is typically done when a widget receive a mouse hover event.\n
 * \n
 * TODO: reset the timer when a new tooltip is registered.
 */
static void display_tooltips()
{
	static float time_spent_on_button = 0;
	static float previous_function_call_time = 0;
	int centered = 1;
	SDL_Rect tooltip_rect = { .x = 0, .y = 0, .h = 500 };

	// Update the timer.
	time_spent_on_button += SDL_GetTicks() - previous_function_call_time;
	previous_function_call_time = SDL_GetTicks();

	if (!tooltip_info.value) {
		// No tooltip has been set for displaying.
		time_spent_on_button = 0;
		return;
	}

	string tooltip_text;

	// Get the tooltip text.
	if (tooltip_info.value->get_text)
		tooltip_text = tooltip_info.value->get_text();
	else
		tooltip_text = tooltip_info.value->text;

	// Level editor specific options.
	if (game_status == INSIDE_LVLEDITOR) {
		if (!GameConfig.show_lvledit_tooltips)
			return;

		if (time_spent_on_button <= 1200)
			return;
		
		centered = 0;	// Editor tooltips are not centered.
	}
	
	// Set the correct font before computing text width.
	set_current_font(FPS_Display_Font);

	// Temporary copy required due to longest_line_width() altering the string.
	char buffer[strlen(tooltip_text) + 1];
	strcpy(buffer, tooltip_text);

	// Tooltip width is given by the longest line in the tooltip, with a maximum of 400 pixels
	// after which linebreaks are automatically added.
	tooltip_rect.w = longest_line_width(buffer) + 2 * TEXT_BANNER_HORIZONTAL_MARGIN;
	if (tooltip_rect.w > 400)
		tooltip_rect.w = 400;	

	// Compute height
	int lines_in_text = get_lines_needed(tooltip_text, tooltip_rect, 1.0);
	tooltip_rect.h = lines_in_text * get_font_height(FPS_Display_Font);

	int center_x = tooltip_info.widget_rect.x + tooltip_info.widget_rect.w / 2;	
	int center_y = tooltip_info.widget_rect.y + tooltip_info.widget_rect.h / 2;	

	// The tooltip is positioned to the left or to the right (whichever is closer 
	// to the screen's center) of the widget's center.
	if (center_x < GameConfig.screen_width / 2)
		tooltip_rect.x = center_x;
	else
		tooltip_rect.x = center_x - tooltip_rect.w;

	// The tooltip is positioned above or under the widget (whichever is closer 
	// to the screen's center). A small offset is added for aesthetic reasons.
	if (center_y < GameConfig.screen_height / 2)
		tooltip_rect.y = tooltip_info.widget_rect.y + tooltip_info.widget_rect.h + 4;
	else
		tooltip_rect.y = tooltip_info.widget_rect.y - tooltip_rect.h - 4;
		

	display_tooltip(tooltip_text, centered, tooltip_rect);
}

/**
 * \brief This function pushes events to the currently active top level group.
 * \ingroup gui2d_interface
 */
void handle_widget_event(SDL_Event *event)
{
	struct widget *ui = get_active_ui();

	if (ui)
		ui->handle_event(ui, event);
}

/**
 * \brief This functions calls the update function on the current gui tree.
 * \ingroup gui2d_interface
 *
 * \details This should be called once every few frames to update widgets' state.\n
 * \n
 * \note Update calls are handled by disabled widgets but are not sent to
 * children widgets. This allows for inactive widgets to become active on
 * update.
 */
void update_widgets()
{
	struct widget *ui = get_active_ui();

	if (ui && ui->update_tree)
		ui->update_tree(ui);
}

/**
 * \brief This function displays the currently active top level widget group.
 * \ingroup gui2d_interface
 */
void display_widgets() 
{
	struct widget *ui = get_active_ui();

	if (ui && ui->display) {
		ui->display(ui);
		display_tooltips();
	}
}

//////////////////////////////////////////////////////////////////////
// Base widget
//////////////////////////////////////////////////////////////////////

/**
 * \brief Default event handler implementation for a base widget.
 * \relates widget
 *
 * \details The default event handler does nothing, and does not consumes the
 * event.
 *
 * \param w     Pointer to the widget object
 * \param event Pointer to the propagated event
 *
 * \return 0 (event not consumed)
 */
static int handle_event(struct widget *w, SDL_Event *event)
{
	return 0;
}

void widget_free(struct widget *w)
{
	if (w->ext) {
		free(w->ext);
	}
}

/**
 * \brief update_tree() implementation for a base widget
 * \relates widget
 *
 * \details On a base widget, update_tree() calls the widget's update() function.
 *
 * \param w Pointer to the widget object
 */
static void leaf_update(struct widget *w)
{
	if (w && w->update)
		w->update(w);
}

/**
 * \brief Initialize the properties and functions of a base widget.
 * \ingroup gui2d_widget
 *
 * \param w Pointer to the widget object
 */
void widget_init(struct widget *w)
{
	widget_set_rect(w, 0, 0, 0, 0);
	w->display = NULL;
	w->update = NULL;
	w->free = widget_free;
	w->handle_event = handle_event;
	w->enabled = 1;
	w->update_tree = leaf_update;
}

/**
 * \brief Create a base widget and initialize it.
 * \ingroup gui2d_widget
 *
 * \return A pointer to the newly created base widget.
 */
struct widget *widget_create()
{
	struct widget *w = MyMalloc(sizeof(struct widget));
	widget_init(w);
	return w;
}

/**
 * \brief Set position and size of a widget.
 * \ingroup gui2d_widget
 *
 * \param w       Pointer to the base widget struct
 * \param x       Absolute position of the left side of the widget (defined in pixel, from the top-left of the screen)
 * \param y       Absolute position of the top side of the widget (defined in pixel, from the top-left of the screen)
 * \param width   Width of the widget (defined in pixel)
 * \param height  Height of the widget (defined in pixel)
 */
void widget_set_rect(struct widget *w, int x, int y, int width, int height)
{
	w->rect.x = x;
	w->rect.y = y;
	w->rect.w = width;
	w->rect.h = height;
}

#undef _widgets_c
