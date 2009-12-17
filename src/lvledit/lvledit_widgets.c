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

#define _leveleditor_widgets_c

#include "system.h"

#include "defs.h"
#include "struct.h"
#include "global.h"
#include "proto.h"

#include "SDL_rotozoom.h"

#include "lvledit/lvledit.h"
#include "lvledit/lvledit_actions.h"
#include "lvledit/lvledit_tools.h"
#include "lvledit/lvledit_widgets.h"

LIST_HEAD(leveleditor_widget_list);

struct leveleditor_widget *create_button(int btype)
{
	struct leveleditor_widget *a = MyMalloc(sizeof(struct leveleditor_widget));
	a->type = WIDGET_BUTTON;
	a->rect = AllMousePressButtons[btype].button_rect;
	a->mouseenter = leveleditor_button_mouseenter;
	a->mouseleave = leveleditor_button_mouseleave;
	a->mouserelease = leveleditor_button_mouserelease;
	a->mousepress = leveleditor_button_mousepress;
	a->mouserightrelease = leveleditor_button_mouserightrelease;
	a->mouserightpress = leveleditor_button_mouserightpress;
	a->mousewheelup = leveleditor_button_mousewheelup;
	a->mousewheeldown = leveleditor_button_mousewheeldown;
	a->enabled = 1;

	struct leveleditor_button *b = MyMalloc(sizeof(struct leveleditor_button));
	b->btn_index = btype;
	b->pressed = 0;

	a->ext = b;

	return a;
}

struct leveleditor_widget *create_menu()
{
	struct leveleditor_widget *a = MyMalloc(sizeof(struct leveleditor_widget));
	a->type = WIDGET_MENU;
	a->mouseenter = leveleditor_menu_mouseenter;
	a->mouseleave = leveleditor_menu_mouseleave;
	a->mouserelease = leveleditor_menu_mouserelease;
	a->mousepress = leveleditor_menu_mousepress;
	a->mouserightrelease = leveleditor_menu_mouserightrelease;
	a->mouserightpress = leveleditor_menu_mouserightpress;
	a->mousewheelup = leveleditor_menu_mousewheelup;
	a->mousewheeldown = leveleditor_menu_mousewheeldown;
	a->mousemove = leveleditor_menu_mousemove;
	a->keybevent = NULL;	//leveleditor_menu_keybevent;
	a->enabled = 0;

	struct leveleditor_menu *b = MyMalloc(sizeof(struct leveleditor_menu));
	a->ext = b;

	return a;
}

static struct leveleditor_widget *create_map()
{
	struct leveleditor_widget *a = MyMalloc(sizeof(struct leveleditor_widget));
	a->type = WIDGET_MAP;
	a->rect.x = 0;
	a->rect.y = 68;
	a->rect.w = GameConfig.screen_width;
	a->rect.h = GameConfig.screen_height - 68;
	a->mouseenter = leveleditor_map_mouseenter;
	a->mouseleave = leveleditor_map_mouseleave;
	a->mouserelease = leveleditor_map_mouserelease;
	a->mousepress = leveleditor_map_mousepress;
	a->mouserightrelease = leveleditor_map_mouserightrelease;
	a->mouserightpress = leveleditor_map_mouserightpress;
	a->mousewheelup = leveleditor_map_mousewheelup;
	a->mousewheeldown = leveleditor_map_mousewheeldown;
	a->mousemove = leveleditor_map_mousemove;
	a->keybevent = leveleditor_map_keybevent;
	a->enabled = 1;

	struct leveleditor_mapwidget *m = MyMalloc(sizeof(struct leveleditor_mapwidget));
	a->ext = m;

	return a;
}

static struct leveleditor_widget *create_toolbar()
{
	struct leveleditor_widget *a = MyMalloc(sizeof(struct leveleditor_widget));
	a->type = WIDGET_TOOLBAR;
	a->rect.x = 0;
	a->rect.y = 0;
	a->rect.w = GameConfig.screen_width;
	a->rect.h = 73;
	a->mouseenter = leveleditor_toolbar_mouseenter;
	a->mouseleave = leveleditor_toolbar_mouseleave;
	a->mouserelease = leveleditor_toolbar_mouserelease;
	a->mousepress = leveleditor_toolbar_mousepress;
	a->mouserightrelease = leveleditor_toolbar_mouserightrelease;
	a->mouserightpress = leveleditor_toolbar_mouserightpress;
	a->mousewheelup = leveleditor_toolbar_mousewheelup;
	a->mousewheeldown = leveleditor_toolbar_mousewheeldown;
	a->enabled = 1;

	struct leveleditor_toolbar *t = MyMalloc(sizeof(struct leveleditor_toolbar));
	a->ext = t;

	return a;
}

static struct leveleditor_widget *create_objectselector(int x, char *text, enum leveleditor_object_type type, int *olist)
{
	struct leveleditor_widget *a = MyMalloc(sizeof(struct leveleditor_widget));
	a->type = WIDGET_OBJECTTYPESELECTORBUTTON;
	a->rect.x = x * 80;
	a->rect.y = 73;
	a->rect.w = 80;
	a->rect.h = 17;
	a->mouseenter = leveleditor_typeselect_mouseenter;
	a->mouseleave = leveleditor_typeselect_mouseleave;
	a->mouserelease = leveleditor_typeselect_mouserelease;
	a->mousepress = leveleditor_typeselect_mousepress;
	a->mouserightrelease = leveleditor_typeselect_mouserightrelease;
	a->mouserightpress = leveleditor_typeselect_mouserightpress;
	a->mousewheelup = leveleditor_typeselect_mousewheelup;
	a->mousewheeldown = leveleditor_typeselect_mousewheeldown;
	a->enabled = 1;

	struct leveleditor_typeselect *o = MyMalloc(sizeof(struct leveleditor_typeselect));

	o->type = type;
	o->indices = olist;
	o->title = text;

	a->ext = o;
	return a;
}

void leveleditor_destroy_widget(struct leveleditor_widget *w)
{
	free(w->ext);

	list_del(&w->node);

	free(w);

	if (previously_active_widget == w)
		previously_active_widget = NULL;
}

void leveleditor_init_widgets()
{
	struct leveleditor_widget *map;
	struct leveleditor_widget *wall_selector;

	if (!list_empty(&leveleditor_widget_list)) {
		/* Widgets already initialized, get out */
		return;
	}

	/* Build our interface */
	int t[] = {
		LEVEL_EDITOR_UNDERGROUND_LIGHT_ON_BUTTON,
		LEVEL_EDITOR_UNDO_BUTTON,
		LEVEL_EDITOR_REDO_BUTTON,
		LEVEL_EDITOR_SAVE_SHIP_BUTTON,
		LEVEL_EDITOR_TOGGLE_WAYPOINT_BUTTON,
		LEVEL_EDITOR_TOGGLE_CONNECTION_BLUE_BUTTON,
		LEVEL_EDITOR_BEAUTIFY_GRASS_BUTTON,
		LEVEL_EDITOR_DELETE_OBSTACLE_BUTTON,
		LEVEL_EDITOR_NEXT_OBSTACLE_BUTTON,
		LEVEL_EDITOR_RECURSIVE_FILL_BUTTON,
		LEVEL_EDITOR_NEW_OBSTACLE_LABEL_BUTTON,
		LEVEL_EDITOR_NEW_MAP_LABEL_BUTTON,
		LEVEL_EDITOR_EDIT_CHEST_BUTTON,
		LEVEL_EDITOR_NEW_ITEM_BUTTON,
		LEVEL_EDITOR_ESC_BUTTON,
		LEVEL_EDITOR_LEVEL_RESIZE_BUTTON,
		LEVEL_EDITOR_TOGGLE_ENEMIES_BUTTON,
		LEVEL_EDITOR_TOGGLE_TOOLTIPS_BUTTON,
		LEVEL_EDITOR_TOGGLE_COLLISION_RECTS_BUTTON,
		LEVEL_EDITOR_TOGGLE_GRID_BUTTON_OFF,
		LEVEL_EDITOR_TOGGLE_OBSTACLES_BUTTON,
		LEVEL_EDITOR_ZOOM_IN_BUTTON,
		LEVEL_EDITOR_QUIT_BUTTON,
		GO_LEVEL_NORTH_BUTTON,
		GO_LEVEL_SOUTH_BUTTON,
		GO_LEVEL_EAST_BUTTON,
		GO_LEVEL_WEST_BUTTON,
		RIGHT_LEVEL_EDITOR_BUTTON,
		LEFT_LEVEL_EDITOR_BUTTON,
	};

	int i;

	for (i = 0; i < sizeof(t) / sizeof(t[0]); i++) {
		ShowGenericButtonFromList(t[i]);	//we need that to have .w and .h of the button rect initialized.
		list_add(&create_button(t[i])->node, &leveleditor_widget_list);
	}

	/* The object type selectors */
	wall_selector = create_objectselector(0, _("WALL"), OBJECT_OBSTACLE, wall_tiles_list);
	list_add_tail(&wall_selector->node, &leveleditor_widget_list);
	list_add_tail(&create_objectselector(1, _("FURNITURE"), OBJECT_OBSTACLE, furniture_tiles_list)->node, &leveleditor_widget_list);
	list_add_tail(&create_objectselector(2, _("MACHINERY"), OBJECT_OBSTACLE, machinery_tiles_list)->node, &leveleditor_widget_list);
	list_add_tail(&create_objectselector(3, _("CONTAINER"), OBJECT_OBSTACLE, container_tiles_list)->node, &leveleditor_widget_list);
	list_add_tail(&create_objectselector(4, _("PLANT"), OBJECT_OBSTACLE, plant_tiles_list)->node, &leveleditor_widget_list);
	for (i = 0; i < NUMBER_OF_OBSTACLE_TYPES; i++)
		all_obstacles_list[i] = i;
	all_obstacles_list[i] = -1;
	list_add_tail(&create_objectselector(5, _("ALLOBS."), OBJECT_OBSTACLE, all_obstacles_list)->node, &leveleditor_widget_list);
	list_add_tail(&create_objectselector(6, _("FLOOR"), OBJECT_FLOOR, floor_tiles_list)->node, &leveleditor_widget_list);

	list_add_tail(&create_objectselector(7, _("WAYPT"), OBJECT_WAYPOINT, waypoint_list)->node, &leveleditor_widget_list);

	/* The toolbar */
	list_add_tail(&create_toolbar()->node, &leveleditor_widget_list);

	/* The map (has to be the latest widget in the list) */
	map = create_map();
	list_add_tail(&map->node, &leveleditor_widget_list);

	/* Initialize elements of the interface */
	leveleditor_typeselect_init_selected_list(wall_selector->ext);

	leveleditor_map_init();

}

void leveleditor_update_button_states()
{
	struct leveleditor_button *b;
	struct leveleditor_widget *w;
	obstacle *o;
	list_for_each_entry(w, &leveleditor_widget_list, node) {
		if (w->type != WIDGET_BUTTON)
			continue;

		b = w->ext;
		switch (b->btn_index) {
		case LEVEL_EDITOR_TOGGLE_ENEMIES_BUTTON:
			b->active = 2 * GameConfig.omit_enemies_in_level_editor;
			break;
		case LEVEL_EDITOR_TOGGLE_TOOLTIPS_BUTTON:
			b->active = 2 * !GameConfig.show_tooltips;
			break;
		case LEVEL_EDITOR_TOGGLE_COLLISION_RECTS_BUTTON:
			b->active = 2 * !draw_collision_rectangles;
			break;
		case LEVEL_EDITOR_ZOOM_IN_BUTTON:
			b->active = 2 * !GameConfig.zoom_is_on;
			break;
		case LEVEL_EDITOR_TOGGLE_OBSTACLES_BUTTON:
			b->active = 2 * GameConfig.omit_obstacles_in_level_editor;
			break;
		case LEVEL_EDITOR_TOGGLE_GRID_BUTTON_OFF:
			b->active = 2 * (draw_grid);
			break;
		case LEVEL_EDITOR_UNDERGROUND_LIGHT_ON_BUTTON:
			b->active = 2 * EditLevel()->use_underground_lighting;
			break;
		case GO_LEVEL_NORTH_BUTTON:
			w->enabled = (EditLevel()->jump_target_north >= 0);
			break;
		case GO_LEVEL_WEST_BUTTON:
			w->enabled = (EditLevel()->jump_target_west >= 0);
			break;
		case GO_LEVEL_SOUTH_BUTTON:
			w->enabled = (EditLevel()->jump_target_south >= 0);
			break;
		case GO_LEVEL_EAST_BUTTON:
			w->enabled = (EditLevel()->jump_target_east >= 0);
			break;
		case LEVEL_EDITOR_SAVE_SHIP_BUTTON:
			w->enabled = (game_root_mode == ROOT_IS_LVLEDIT);
			break;
		case LEVEL_EDITOR_NEXT_OBSTACLE_BUTTON:
			w->enabled = level_editor_can_cycle_obs();
			break;
		case LEVEL_EDITOR_DELETE_OBSTACLE_BUTTON:
			w->enabled = !selection_empty();
			break;
		case LEVEL_EDITOR_EDIT_CHEST_BUTTON:
			o = single_tile_selection(OBJECT_OBSTACLE);
			if (o)
				switch (o->type) {
				case ISO_H_CHEST_CLOSED:
				case ISO_H_CHEST_OPEN:
				case ISO_V_CHEST_CLOSED:
				case ISO_V_CHEST_OPEN:
					w->enabled = 1;
					break;
				default:
					w->enabled = 0;
			} else
				w->enabled = 0;
			break;
		case LEVEL_EDITOR_NEW_OBSTACLE_LABEL_BUTTON:
			w->enabled = single_tile_selection(OBJECT_OBSTACLE) ? 1 : 0;
			break;
		case LEVEL_EDITOR_UNDO_BUTTON:
			w->enabled = !list_empty(&to_undo);
			break;
		case LEVEL_EDITOR_REDO_BUTTON:
			w->enabled = !list_empty(&to_redo);
			break;
		}
	}
}

struct leveleditor_widget *get_active_widget(int x, int y)
{
	struct leveleditor_widget *w;
	list_for_each_entry(w, &leveleditor_widget_list, node) {
		if (!w->enabled)
			continue;
		if (MouseCursorIsInRect(&w->rect, x, y)) {
			return w;
		}
	}

	return NULL;
}

void leveleditor_display_widgets()
{
	struct leveleditor_widget *w;
	list_for_each_entry_reverse(w, &leveleditor_widget_list, node) {
		if (!w->enabled)
			continue;

		switch (w->type) {
		case WIDGET_BUTTON:
			leveleditor_button_display(w);
			break;
		case WIDGET_TOOLBAR:
			leveleditor_toolbar_display(w);
			break;
		case WIDGET_MAP:
			leveleditor_map_display(w);
			break;
		case WIDGET_OBJECTTYPESELECTORBUTTON:
			leveleditor_typeselect_display(w);
			break;
		case WIDGET_MENU:
			leveleditor_menu_display(w);
			break;
		}
	}
}
