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

#define _widgets_c

#include "system.h"

#include "defs.h"
#include "struct.h"
#include "global.h"
#include "proto.h"

#include "lvledit/lvledit.h"
#include "lvledit/lvledit_actions.h"
#include "lvledit/lvledit_tools.h"
#include "widgets.h"
#include "lvledit/lvledit_object_lists.h"

static int *all_obstacles_list = NULL;

typedef struct {
	char *name;
	int **object_list;
	struct widget_lvledit_categoryselect *cs;
} object_category;

static object_category obstacle_category_list[] = {
	{ "WALL", &wall_tiles_list },
	{ "FURNITURE", &furniture_tiles_list },
	{ "MACHINERY", &machinery_tiles_list },
	{ "CONTAINER", &container_tiles_list },
	{ "PLANT", &plant_tiles_list },
	{ "OTHER", &misc_tiles_list },
	{ "ALL", &all_obstacles_list }
};

static object_category floor_category_list[] = {
	{ "SIDEWALK", &sidewalk_floor_list },
	{ "WATER", &water_floor_list },
	{ "GRASS", &grass_floor_list },
	{ "SQUARE", &square_floor_list },
	{ "OTHER", &other_floor_list },
	{ "ALL", &floor_tiles_list }
};

static object_category item_category_list[] = {
	{ "MELEE", &melee_items_list },
	{ "GUN", &gun_items_list },
	{ "DEFENSIVE", &defense_items_list },
	{ "USEABLE", &spell_items_list },
	{ "OTHER", &other_items_list },
	{ "ALL", &all_items_list }
};

static object_category waypoint_category_list[] = {
	{ "ALL", &waypoint_list }
};

static struct {
	enum lvledit_object_type object_type;
	object_category *categories;
	size_t length;
} category_list[] = {
	{ OBJECT_OBSTACLE, obstacle_category_list, sizeof(obstacle_category_list) / sizeof(obstacle_category_list[0]) },
	{ OBJECT_FLOOR, floor_category_list, sizeof(floor_category_list) / sizeof(floor_category_list[0]) },
	{ OBJECT_ITEM, item_category_list, sizeof(item_category_list) / sizeof(item_category_list[0]) },
	{ OBJECT_WAYPOINT, waypoint_category_list, sizeof(waypoint_category_list) / sizeof(waypoint_category_list[0]) },
};

LIST_HEAD(lvledit_widget_list);

static struct widget *widget_button_create(int btype, char *text, char *tooltip)
{
	struct widget *a = MyMalloc(sizeof(struct widget));
	a->type = WIDGET_BUTTON;
	a->rect = AllMousePressButtons[btype].button_rect;
	a->mouseenter = widget_button_mouseenter;
	a->mouseleave = widget_button_mouseleave;
	a->mouserelease = widget_button_mouserelease;
	a->mousepress = widget_button_mousepress;
	a->mouserightrelease = widget_button_mouserightrelease;
	a->mouserightpress = widget_button_mouserightpress;
	a->mousewheelup = widget_button_mousewheelup;
	a->mousewheeldown = widget_button_mousewheeldown;
	a->enabled = 1;

	struct widget_button *b = MyMalloc(sizeof(struct widget_button));
	b->btn_index = btype;
	b->pressed = 0;
	b->text = text;
	b->tooltip = tooltip;

	a->ext = b;

	return a;
}

static struct widget *widget_lvledit_map_create()
{
	struct widget *a = MyMalloc(sizeof(struct widget));
	a->type = WIDGET_MAP;
	a->rect.x = 0;
	a->rect.y = 68;
	a->rect.w = GameConfig.screen_width;
	a->rect.h = GameConfig.screen_height - 68;
	a->mouseenter = widget_lvledit_map_mouseenter;
	a->mouseleave = widget_lvledit_map_mouseleave;
	a->mouserelease = widget_lvledit_map_mouserelease;
	a->mousepress = widget_lvledit_map_mousepress;
	a->mouserightrelease = widget_lvledit_map_mouserightrelease;
	a->mouserightpress = widget_lvledit_map_mouserightpress;
	a->mousewheelup = widget_lvledit_map_mousewheelup;
	a->mousewheeldown = widget_lvledit_map_mousewheeldown;
	a->mousemove = widget_lvledit_map_mousemove;
	a->keybevent = widget_lvledit_map_keybevent;
	a->enabled = 1;

	struct widget_lvledit_map *m = MyMalloc(sizeof(struct widget_lvledit_map));
	a->ext = m;

	return a;
}

static struct widget *widget_lvledit_toolbar_create()
{
	struct widget *a = MyMalloc(sizeof(struct widget));
	a->type = WIDGET_TOOLBAR;
	a->rect.x = 0;
	a->rect.y = 0;
	a->rect.w = GameConfig.screen_width;
	a->rect.h = 73;
	a->mouseenter = widget_lvledit_toolbar_mouseenter;
	a->mouseleave = widget_lvledit_toolbar_mouseleave;
	a->mouserelease = widget_lvledit_toolbar_mouserelease;
	a->mousepress = widget_lvledit_toolbar_mousepress;
	a->mouserightrelease = widget_lvledit_toolbar_mouserightrelease;
	a->mouserightpress = widget_lvledit_toolbar_mouserightpress;
	a->mousewheelup = widget_lvledit_toolbar_mousewheelup;
	a->mousewheeldown = widget_lvledit_toolbar_mousewheeldown;
	a->enabled = 1;

	struct widget_lvledit_toolbar *t = MyMalloc(sizeof(struct widget_lvledit_toolbar));
	a->ext = t;

	return a;
}

static struct widget *widget_lvledit_categoryselector_create(int x, char *text, enum lvledit_object_type type, int *olist)
{
	struct widget *a = MyMalloc(sizeof(struct widget));
	a->type = WIDGET_CATEGORY_SELECTOR;
	a->rect.x = x * 80;
	a->rect.y = 73;
	a->rect.w = 80;
	a->rect.h = 17;
	a->mouseenter = widget_lvledit_categoryselect_mouseenter;
	a->mouseleave = widget_lvledit_categoryselect_mouseleave;
	a->mouserelease = widget_lvledit_categoryselect_mouserelease;
	a->mousepress = widget_lvledit_categoryselect_mousepress;
	a->mouserightrelease = widget_lvledit_categoryselect_mouserightrelease;
	a->mouserightpress = widget_lvledit_categoryselect_mouserightpress;
	a->mousewheelup = widget_lvledit_categoryselect_mousewheelup;
	a->mousewheeldown = widget_lvledit_categoryselect_mousewheeldown;
	a->enabled = 1;

	struct widget_lvledit_categoryselect *cs = MyMalloc(sizeof(struct widget_lvledit_categoryselect));
	cs->type = type;
	cs->indices = olist;
	cs->title = text;

	a->ext = cs;
	return a;
}

static struct widget *widget_lvledit_minimap_create()
{
	struct widget *a = MyMalloc(sizeof(struct widget));
	a->type = WIDGET_MINIMAP;
	a->rect.w = WIDGET_MINIMAP_WIDTH;
	a->rect.h = WIDGET_MINIMAP_HEIGHT;
	a->rect.x = GameConfig.screen_width - a->rect.w;
	a->rect.y = GameConfig.screen_height - a->rect.h;
	a->mouseenter = widget_lvledit_minimap_mouseenter;
	a->mouseleave = widget_lvledit_minimap_mouseleave;
	a->mouserelease = widget_lvledit_minimap_mouserelease;
	a->mousepress = widget_lvledit_minimap_mousepress;
	a->mouserightrelease = widget_lvledit_minimap_mouserightrelease;
	a->mouserightpress = widget_lvledit_minimap_mouserightpress;
	a->mousewheelup = widget_lvledit_minimap_mousewheelup;
	a->mousewheeldown = widget_lvledit_minimap_mousewheeldown;
	a->enabled = 1;

	struct widget_lvledit_minimap *n = MyMalloc(sizeof(struct widget_lvledit_minimap));	
	a->ext = n;

	return a;
}

void widget_lvledit_init()
{
	struct widget *map;

	if (!list_empty(&lvledit_widget_list)) {
		/* Widgets already initialized, get out */
		return;
	}

	/* Build our interface */
	struct {
		int btn_index;
		char *text;
		char *tooltip;
	} b[] = {
		{LEVEL_EDITOR_UNDO_BUTTON, NULL,
			_("Undo\n\nUse this button to undo your last actions.")},
		{LEVEL_EDITOR_REDO_BUTTON, NULL,
			_("Redo\n\nUse this button to redo an action.")},
		{LEVEL_EDITOR_SAVE_SHIP_BUTTON, NULL,
			_("Save Map\n\nThis button will save your current map over the file '../map/levels.dat' from your current working directory.")},
		{LEVEL_EDITOR_BEAUTIFY_GRASS_BUTTON, NULL,
			_("Beautify grass button\n\nUse this button to automatically 'beautify' rough edges of the grass-sand tiles. It will apply to the selected floor or, if not applicable, to the entire level.\n\nYou can also use Ctrl-b for this.")},
		{LEVEL_EDITOR_DELETE_OBSTACLE_BUTTON, NULL,
			_("Delete selected obstacle\n\nUse this button to delete the currently marked obstacle.\n\nYou can also use Ctrl-X for this.")},
		{LEVEL_EDITOR_NEXT_OBJECT_BUTTON, NULL,
			_("Next object on currently selected tile\n\nUse this button to cycle the currently marked object on this tile.\n\nYou can also use the N key for this.")},
		{LEVEL_EDITOR_NEW_OBSTACLE_LABEL_BUTTON, NULL,
			_("New obstacle label\n\nUse this button to attach a label to the currently marked obstacle.  These obstacle labels can be used to define obstacles to be modified by events.\n")},
		{LEVEL_EDITOR_NEW_MAP_LABEL_BUTTON, NULL,
			_("New map label\n\nUse this button to attach a new map label to the current cursor position.  These map labels can be used to define starting points for bots and characters or also to define locations for events and triggers.")},
		{LEVEL_EDITOR_EDIT_CHEST_BUTTON, NULL,
			_("Edit chests contents\n\nUse this button to change the contents of the chest. If it is empty the game may generate random items at run time.")},
		{LEVEL_EDITOR_ESC_BUTTON, NULL,
			_("Menu\n\nUse this button to enter the main menu of the level editor.\n Note, that you can also use the Escape key to enter the level editor main menu.")},
		{LEVEL_EDITOR_TOGGLE_ENEMIES_BUTTON, NULL,
			_("Toggle display enemies\n\nUse this button to toggle between enemies displayed in level editor or enemies hidden in level editor.")},
		{LEVEL_EDITOR_TOGGLE_TOOLTIPS_BUTTON, NULL,
			_("Toggle display tooltips\n\nUse this button to toggle these annoying help windows on and off.")},
		{LEVEL_EDITOR_TOGGLE_COLLISION_RECTS_BUTTON, NULL,
			_("Toggle display collision rectangles\n\nUse this button to toggle the visible collision rectangles on and off.")},
		{LEVEL_EDITOR_TOGGLE_GRID_BUTTON_OFF, NULL,
			_("Change grid mode ( placement / full / off )")},
		{LEVEL_EDITOR_TOGGLE_OBSTACLES_BUTTON, NULL,
			_("Toggle display obstacles\n\nUse this button to toggle between obstacles displayed in level editor or obstacles hidden in level editor.")},
		{LEVEL_EDITOR_ZOOM_IN_BUTTON, NULL,
			_("Zoom in/out\n\nUse this button to zoom INTO or OUT of the level.\n\nUse right click to change the zoom ratio.\n")},
		{LEVEL_EDITOR_QUIT_BUTTON, NULL,
			_("Test Map\n\nThis will save your map and reload it after you finish testing, avoiding saving an unclean world state.")},
		{RIGHT_LEVEL_EDITOR_BUTTON, NULL, NULL},
		{LEFT_LEVEL_EDITOR_BUTTON, NULL, NULL},
		{LEVEL_EDITOR_TYPESELECT_OBSTACLE_BUTTON, "OBSTACLE", NULL},
		{LEVEL_EDITOR_TYPESELECT_FLOOR_BUTTON, "FLOOR", NULL},
		{LEVEL_EDITOR_TYPESELECT_ITEM_BUTTON, "ITEM", NULL},
		{LEVEL_EDITOR_TYPESELECT_WAYPOINT_BUTTON, "WAYPOINT", NULL},
	};

	int i, j;

	for (i = 0; i < sizeof(b) / sizeof(b[0]); i++) {
		ShowGenericButtonFromList(b[i].btn_index);	//we need that to have .w and .h of the button rect initialized.
		list_add(&widget_button_create(b[i].btn_index, b[i].text, b[i].tooltip)->node, &lvledit_widget_list);
	}

	lvledit_build_tile_lists();

	// Load categories for obstacles
	char fpath[4096];
	find_file("leveleditor_obstacle_categories.lua", MAP_DIR, fpath, 0);
	run_lua_file(fpath);

	// Build list of all obstacles
	all_obstacles_list = MyMalloc(sizeof(int) * (obstacle_map.size + 1));
	for (i = 0; i < obstacle_map.size; i++)
		all_obstacles_list[i] = i;
	all_obstacles_list[i] = -1;

	// Create category selectors 
	for (i = 0; i < sizeof(category_list) / sizeof(category_list[0]); i++) {
		enum lvledit_object_type type = category_list[i].object_type;
		object_category *categories = category_list[i].categories;
		for (j = 0; j < category_list[i].length; j++) {
			struct widget *widget = widget_lvledit_categoryselector_create(j, _(categories[j].name), type, *categories[j].object_list);
			categories[j].cs = widget->ext;
			list_add_tail(&widget->node, &lvledit_widget_list);
		}
	}

	// Create the minimap
	list_add_tail(&widget_lvledit_minimap_create()->node, &lvledit_widget_list);

	/* The toolbar */
	list_add_tail(&widget_lvledit_toolbar_create()->node, &lvledit_widget_list);

	/* The map (has to be the latest widget in the list) */
	map = widget_lvledit_map_create();
	list_add_tail(&map->node, &lvledit_widget_list);

	// Activate the obstacle type selector
	lvledit_select_type(OBJECT_OBSTACLE);

	widget_lvledit_map_init();
}

void leveleditor_update_button_states()
{
	struct widget_button *b;
	struct widget *w;
	obstacle *o;
	list_for_each_entry(w, &lvledit_widget_list, node) {
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
		case LEVEL_EDITOR_SAVE_SHIP_BUTTON:
			w->enabled = (game_root_mode == ROOT_IS_LVLEDIT);
			break;
		case LEVEL_EDITOR_NEXT_OBJECT_BUTTON:
			w->enabled = level_editor_can_cycle_marked_object();
			break;		
		case LEVEL_EDITOR_DELETE_OBSTACLE_BUTTON:
			w->enabled = !selection_empty();
			break;
		case LEVEL_EDITOR_EDIT_CHEST_BUTTON:
			o = single_tile_selection(OBJECT_OBSTACLE);
			if (o)
				switch (o->type) {
				case ISO_H_CHEST_CLOSED:
				case ISO_V_CHEST_CLOSED:
				case ISO_E_CHEST2_CLOSED:
				case ISO_S_CHEST2_CLOSED:
				case ISO_N_CHEST2_CLOSED:
				case ISO_W_CHEST2_CLOSED:
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
		case LEVEL_EDITOR_TYPESELECT_OBSTACLE_BUTTON:
			b->active = 2 * (selection_type() == OBJECT_OBSTACLE);
			break;
		case LEVEL_EDITOR_TYPESELECT_FLOOR_BUTTON:
			b->active = 2 * (selection_type() == OBJECT_FLOOR);
			break;
		case LEVEL_EDITOR_TYPESELECT_ITEM_BUTTON:
			b->active = 2 * (selection_type() == OBJECT_ITEM);
			break;
		case LEVEL_EDITOR_TYPESELECT_WAYPOINT_BUTTON:
			b->active = 2 * (selection_type() == OBJECT_WAYPOINT);
			break;
		}
	}
}

struct widget *get_active_widget(int x, int y)
{
	struct widget *w;
	list_for_each_entry(w, &lvledit_widget_list, node) {
		if (!w->enabled)
			continue;
		if (MouseCursorIsInRect(&w->rect, x, y)) {
			return w;
		}
	}

	return NULL;
}

void widget_display()
{
	struct widget *w;
	list_for_each_entry_reverse(w, &lvledit_widget_list, node) {
		if (!w->enabled)
			continue;

		switch (w->type) {
		case WIDGET_BUTTON:
			widget_button_display(w);
			break;
		case WIDGET_TOOLBAR:
			widget_lvledit_toolbar_display(w);
			break;
		case WIDGET_MAP:
			widget_lvledit_map_display(w);
			break;
		case WIDGET_CATEGORY_SELECTOR:
			widget_lvledit_categoryselect_display(w);
			break;
		case WIDGET_MINIMAP:
			widget_lvledit_minimap_display(w);
			break;
		}
	}
}

void lvledit_select_type(enum lvledit_object_type type)
{
	struct widget *cs_widget;

	if (selection_type() != type) {
		// When the user changes the current object type selected, reset tools
		lvledit_reset_tools();
	}

	// Find the categories which must be enabled
 	list_for_each_entry_reverse(cs_widget, &lvledit_widget_list, node) {
 		if (cs_widget->type != WIDGET_CATEGORY_SELECTOR)
 			continue;

		// By default, desactivate all categories
		cs_widget->enabled = 0;		
		
		struct widget_lvledit_categoryselect *cs = cs_widget->ext;

		// Activate a category if it is of the right type
		if (cs->type == type) {
			cs_widget->enabled = 1;

			widget_lvledit_categoryselect_activate(cs);
		}
	}
}

void lvledit_categoryselect_switch(int direction)
{
	// Find a category list for the current selection type
	object_category *categories = NULL;
	int i, length = 0;
	for (i = 0; i < sizeof(category_list) / sizeof(category_list[0]); i++) {
		if (selection_type() == category_list[i].object_type) {
			categories = category_list[i].categories;
			length = category_list[i].length;
			break;
		}
	}

	// Find the current category
	for (i = 0; i < length; i++) {
		if (get_current_object_type() == categories[i].cs)
			break;
	}

	// Switch the current category according to the direction
	int index = i + direction;
	if (index < 0)
		index = length - 1;
	index %= length;
	widget_lvledit_categoryselect_activate(categories[index].cs);
}
