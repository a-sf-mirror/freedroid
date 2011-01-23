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
#include "lvledit/lvledit_object_lists.h"

static int all_obstacles_list[NUMBER_OF_OBSTACLE_TYPES + 1];

LIST_HEAD(leveleditor_widget_list);

static struct leveleditor_widget *create_button(int btype, char *text, char *tooltip)
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
	b->text = text;
	b->tooltip = tooltip;

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

static struct leveleditor_widget *create_categoryselector(int x, char *text, enum leveleditor_object_type type, int *olist)
{
	struct leveleditor_widget *a = MyMalloc(sizeof(struct leveleditor_widget));
	a->type = WIDGET_CATEGORY_SELECTOR;
	a->rect.x = x * 80;
	a->rect.y = 73;
	a->rect.w = 80;
	a->rect.h = 17;
	a->mouseenter = leveleditor_categoryselect_mouseenter;
	a->mouseleave = leveleditor_categoryselect_mouseleave;
	a->mouserelease = leveleditor_categoryselect_mouserelease;
	a->mousepress = leveleditor_categoryselect_mousepress;
	a->mouserightrelease = leveleditor_categoryselect_mouserightrelease;
	a->mouserightpress = leveleditor_categoryselect_mouserightpress;
	a->mousewheelup = leveleditor_categoryselect_mousewheelup;
	a->mousewheeldown = leveleditor_categoryselect_mousewheeldown;
	a->enabled = 1;

	struct leveleditor_categoryselect *cs = MyMalloc(sizeof(struct leveleditor_categoryselect));
	cs->type = type;
	cs->indices = olist;
	cs->title = text;

	a->ext = cs;
	return a;
}

static struct leveleditor_widget *create_minimap()
{
	struct leveleditor_widget *a = MyMalloc(sizeof(struct leveleditor_widget));
	a->type = WIDGET_MINIMAP;
	a->rect.w = WIDGET_MINIMAP_WIDTH;
	a->rect.h = WIDGET_MINIMAP_HEIGHT;
	a->rect.x = GameConfig.screen_width - a->rect.w;
	a->rect.y = GameConfig.screen_height - a->rect.h;
	a->mouseenter = leveleditor_minimap_mouseenter;
	a->mouseleave = leveleditor_minimap_mouseleave;
	a->mouserelease = leveleditor_minimap_mouserelease;
	a->mousepress = leveleditor_minimap_mousepress;
	a->mouserightrelease = leveleditor_minimap_mouserightrelease;
	a->mouserightpress = leveleditor_minimap_mouserightpress;
	a->mousewheelup = leveleditor_minimap_mousewheelup;
	a->mousewheeldown = leveleditor_minimap_mousewheeldown;
	a->enabled = 1;

	struct leveleditor_minimap *n = MyMalloc(sizeof(struct leveleditor_minimap));	
	a->ext = n;

	return a;
}

void leveleditor_init_widgets()
{
	struct leveleditor_widget *map;

	if (!list_empty(&leveleditor_widget_list)) {
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
			_("Save Map\n\nThis button will save your current map over the file '../map/freedroid.levels' from your current working directory.")},
		{LEVEL_EDITOR_BEAUTIFY_GRASS_BUTTON, NULL,
			_("Beautify grass button\n\nUse this button to automatically 'beautify' rough edges of the grass-sand tiles. It will apply to the selected floor or, if not applicable, to the entire level.\n\nYou can also use Ctrl-b for this.")},
		{LEVEL_EDITOR_DELETE_OBSTACLE_BUTTON, NULL,
			_("Delete selected obstacle\n\nUse this button to delete the currently marked obstacle.\n\nYou can also use Ctrl-X for this.")},
		{LEVEL_EDITOR_NEXT_OBSTACLE_BUTTON, NULL,
			_("Next obstacle on currently selected tile\n\nUse this button to cycle the currently marked obstacle on this tile.\n\nYou can also use the N key for this.")},
		{LEVEL_EDITOR_NEW_OBSTACLE_LABEL_BUTTON, NULL,
			_("New obstacle label\n\nUse this button to attach a label to the currently marked obstacle.  These obstacle labels can be used to define obstacles to be modified by events.\n Note that you can also use the hotkey 'h' for this.")},
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
			_("Toggle display collision rectangles\n\nUse this button to toggle the visible collision rectangles on and off.\nThis only works using OpenGL mode!")},
		{LEVEL_EDITOR_TOGGLE_GRID_BUTTON_OFF, NULL,
			_("Change grid mode ( placement / full / off )")},
		{LEVEL_EDITOR_TOGGLE_OBSTACLES_BUTTON, NULL,
			_("Toggle display obstacles\n\nUse this button to toggle between obstacles displayed in level editor or obstacles hidden in level editor.")},
		{LEVEL_EDITOR_ZOOM_IN_BUTTON, NULL,
			_("Zoom in/out\n\nUse this button to zoom INTO or OUT of the level.\n\nUse right click to change the zoom ratio (OpenGL only).\n")},
		{LEVEL_EDITOR_QUIT_BUTTON, NULL,
			_("Test Map\n\nThis will save your map and reload it after you finish testing, avoiding saving an unclean world state.")},
		{RIGHT_LEVEL_EDITOR_BUTTON, NULL, NULL},
		{LEFT_LEVEL_EDITOR_BUTTON, NULL, NULL},
		{LEVEL_EDITOR_TYPESELECT_OBSTACLE_BUTTON, "OBSTACLE", NULL},
		{LEVEL_EDITOR_TYPESELECT_FLOOR_BUTTON, "FLOOR", NULL},
		{LEVEL_EDITOR_TYPESELECT_ITEM_BUTTON, "ITEM", NULL},
		{LEVEL_EDITOR_TYPESELECT_WAYPOINT_BUTTON, "WAYPOINT", NULL},
	};

	int i;

	for (i = 0; i < sizeof(b) / sizeof(b[0]); i++) {
		ShowGenericButtonFromList(b[i].btn_index);	//we need that to have .w and .h of the button rect initialized.
		list_add(&create_button(b[i].btn_index, b[i].text, b[i].tooltip)->node, &leveleditor_widget_list);
	}

	// Create category selectors
	build_leveleditor_tile_lists();

	// Obstacles
	for (i = 0; i < NUMBER_OF_OBSTACLE_TYPES; i++)
		all_obstacles_list[i] = i;
	all_obstacles_list[i] = -1;
	list_add_tail(&create_categoryselector(0, _("WALL"), OBJECT_OBSTACLE, wall_tiles_list)->node, &leveleditor_widget_list);
	list_add_tail(&create_categoryselector(1, _("FURNITURE"), OBJECT_OBSTACLE, furniture_tiles_list)->node, &leveleditor_widget_list);
	list_add_tail(&create_categoryselector(2, _("MACHINERY"), OBJECT_OBSTACLE, machinery_tiles_list)->node, &leveleditor_widget_list);
	list_add_tail(&create_categoryselector(3, _("CONTAINER"), OBJECT_OBSTACLE, container_tiles_list)->node, &leveleditor_widget_list);
	list_add_tail(&create_categoryselector(4, _("PLANT"), OBJECT_OBSTACLE, plant_tiles_list)->node, &leveleditor_widget_list);
	list_add_tail(&create_categoryselector(5, _("ALLOBS."), OBJECT_OBSTACLE, all_obstacles_list)->node, &leveleditor_widget_list);

	// Floor
	list_add_tail(&create_categoryselector(0, _("SIDEWALK"), OBJECT_FLOOR, sidewalk_floor_list)->node, &leveleditor_widget_list);
	list_add_tail(&create_categoryselector(1, _("WATER"), OBJECT_FLOOR, water_floor_list)->node, &leveleditor_widget_list);
	list_add_tail(&create_categoryselector(2, _("GRASS"), OBJECT_FLOOR, grass_floor_list)->node, &leveleditor_widget_list);
	list_add_tail(&create_categoryselector(3, _("SQUARE"), OBJECT_FLOOR, square_floor_list)->node, &leveleditor_widget_list);
	list_add_tail(&create_categoryselector(4, _("OTHER"), OBJECT_FLOOR, other_floor_list)->node, &leveleditor_widget_list);

	list_add_tail(&create_categoryselector(5, _("ALL"), OBJECT_FLOOR, floor_tiles_list)->node, &leveleditor_widget_list);

	// Items
	list_add_tail(&create_categoryselector(0, _("MELEE"), OBJECT_ITEM, melee_items_list)->node, &leveleditor_widget_list);
	list_add_tail(&create_categoryselector(1, _("GUN"), OBJECT_ITEM, gun_items_list)->node, &leveleditor_widget_list);
	list_add_tail(&create_categoryselector(2, _("DEFENSIVE"), OBJECT_ITEM, defense_items_list)->node, &leveleditor_widget_list);
	list_add_tail(&create_categoryselector(3, _("USEABLE"), OBJECT_ITEM, spell_items_list)->node, &leveleditor_widget_list);
	list_add_tail(&create_categoryselector(4, _("OTHER"), OBJECT_ITEM, other_items_list)->node, &leveleditor_widget_list);
	list_add_tail(&create_categoryselector(5, _("ALL"), OBJECT_ITEM, all_items_list)->node, &leveleditor_widget_list);

	// Waypoints
	list_add_tail(&create_categoryselector(0, _("ALL"), OBJECT_WAYPOINT, waypoint_list)->node, &leveleditor_widget_list);

	// Create the minimap
	list_add_tail(&create_minimap()->node, &leveleditor_widget_list);

	/* The toolbar */
	list_add_tail(&create_toolbar()->node, &leveleditor_widget_list);

	/* The map (has to be the latest widget in the list) */
	map = create_map();
	list_add_tail(&map->node, &leveleditor_widget_list);

	// Activate the obstacle type selector
	leveleditor_select_type(OBJECT_OBSTACLE);

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
		case WIDGET_CATEGORY_SELECTOR:
			leveleditor_categoryselect_display(w);
			break;
		case WIDGET_MINIMAP:
			leveleditor_minimap_display(w);
			break;
		}
	}
}

void leveleditor_select_type(enum leveleditor_object_type type)
{
	struct leveleditor_widget *cs_widget;

	if (selection_type() != type) {
		// When the user changes the current object type selected, reset tools
		leveleditor_reset_tools();
	}

	// Find the categories which must be enabled
 	list_for_each_entry_reverse(cs_widget, &leveleditor_widget_list, node) {
 		if (cs_widget->type != WIDGET_CATEGORY_SELECTOR)
 			continue;

		// By default, desactivate all categories
		cs_widget->enabled = 0;		
		
		struct leveleditor_categoryselect *cs = cs_widget->ext;

		// Activate a category if it is of the right type
		if (cs->type == type) {
			cs_widget->enabled = 1;

			leveleditor_categoryselect_activate(cs);
		}
	}
}
