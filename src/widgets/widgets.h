/* 
 *
 *   Copyright (c) 2009-2010 Arthur Huillet
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
 * @file widgets.h
 * This file contains structure types and functions used by the widget system.
 */
#ifndef _widgets_h
#define _widgets_h
#undef EXTERN
#ifndef _widgets_c
#define EXTERN extern
#else
#define EXTERN
#endif

enum widget_type {
	WIDGET_BUTTON,
	WIDGET_TOOLBAR,
	WIDGET_MAP,
	WIDGET_CATEGORY_SELECTOR,
	WIDGET_MINIMAP,
};

/**
 * @struct widget
 * @brief Base widget type.
 *
 * This is the base type used by the widget system. Contains basic information and callbacks
 * used by all widget types.
 * NOTE: Widget types inheriting this type must have it as their first attribute.   
 */
struct widget {
	enum widget_type type;	/**< Enum representing the widget's type. Deprecated. */
	SDL_Rect rect;		/**< Rectangle containing widget's size and position. */
	uint8_t enabled;	/**< Boolean flag used for enabling/disabling the widget. */
	void (*display) (struct widget *); /**< Display callback. */
	void (*update) (struct widget *);  /**< Callback triggered by update events. */
	int (*handle_event) (struct widget *, SDL_Event *);	/**< General event handler. */
	void *ext;		/**< Pointer to type specific data. Deprecated. */
	struct list_head node;	/**< Linked list node used for storing sibling widgets in a widget_group. */
};

/**
 * This structure is used for storing and displaying tooltips.
 * The tooltip text will be retrieved using the get_text function pointer. If this pointer is NULL,
 * the text field will be used instead.
 * NOTE: get_text should be used for dynamic tooltips while the text field should be used for static tooltips.
 */
struct tooltip {
	string (*get_text)(void);	/**< Returns the text of a dynamic tooltip. */
	string text;		/**< String used for static tooltip texts. */
};

void display_widgets(void);
void update_widgets(void);
struct widget *widget_create(void);
void handle_widget_event(SDL_Event *);
struct image *widget_load_image_resource(char *, int);
void widget_set_rect(struct widget *, int, int, int, int);
void widget_set_tooltip(struct tooltip *, SDL_Rect *);

/**
 * This macro is used for creating callbacks that updates
 * a widget flag using an external variable.
 *
 * @param widget_type the type of the widget for which the callback is set.
 * @param flag the widget flag to be updated.
 * @param data the external data used for updating.
 */
#define WIDGET_UPDATE_FLAG_ON_DATA(widget_type, flag, data) \
({ \
  void anonymous_func(struct widget *w) \
  { \
    widget_type(w)->flag = data; \
  } \
  anonymous_func; \
})

#undef EXTERN

#define WIDGET(x) ((struct widget *)x)

#define EVENT_UPDATE 0		/**< User event code used for signaling the update event. Sent by update_widgets. */
#define EVENT_MOUSE_ENTER 1	/**< User event code used for signaling mouse entering a widget. */
#define EVENT_MOUSE_LEAVE 2	/**< USer event code used for signaling mouse leaving a widget. */

#define MOUSE_BUTTON_1 1
#define MOUSE_BUTTON_2 2
#define MOUSE_BUTTON_3 3

#include "widgets/widget_group.h"
#include "widgets/widget_button.h"
#include "widgets/widget_text.h"
#include "widgets/widget_background.h"

#endif
