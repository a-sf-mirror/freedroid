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

#define _lvledit_widgets_c 1

#include "system.h"

#include "defs.h"
#include "struct.h"
#include "global.h"
#include "proto.h"

#include "lvledit/lvledit.h"
#include "lvledit/lvledit_actions.h"
#include "lvledit/lvledit_tools.h"
#include "lvledit/lvledit_object_lists.h"
#include "lvledit/lvledit_display.h"
#include "lvledit/lvledit_beautify_actions.h"
#include "lvledit/lvledit_map.h"
#include "lvledit/lvledit_menu.h"
#include "lvledit/lvledit_widgets.h"

static struct widget_group *level_editor_widget_group = NULL; 
static int *all_obstacles_list = NULL;

typedef struct {
	char *name;
	int **object_list;
	struct widget_lvledit_categoryselect *cs;
} object_category;

static object_category obstacle_category_list[] = {
	{ N_("wall"), &wall_tiles_list },
	{ N_("furniture"), &furniture_tiles_list },
	{ N_("machinery"), &machinery_tiles_list },
	{ N_("container"), &container_tiles_list },
	{ N_("nature"), &nature_tiles_list },
	{ N_("other"), &misc_tiles_list },
	{ N_("all"), &all_obstacles_list }
};

static object_category floor_category_list[] = {
	{ N_("sidewalk"), &sidewalk_floor_list },
	{ N_("water"), &water_floor_list },
	{ N_("grass"), &grass_floor_list },
	{ N_("square"), &square_floor_list },
	{ N_("sand"), &sand_floor_list },
	{ N_("other"), &other_floor_list },
	{ N_("all"), &floor_tiles_list }
};

static object_category item_category_list[] = {
	{ N_("melee"), &melee_items_list },
	{ N_("gun"), &gun_items_list },
	{ N_("defensive"), &defense_items_list },
	{ N_("usable"), &spell_items_list },
	{ N_("other"), &other_items_list },
	{ N_("all"), &all_items_list }
};

static object_category waypoint_category_list[] = {
	{ N_("all"), &waypoint_list }
};

static object_category map_label_category_list[] = {
	{ N_("all"), &map_label_list }
};

static object_category enemy_category_list[] = {
	{ N_("droids"), &droid_enemies_list },
	{ N_("humans"), &human_enemies_list },
	{ N_("all"), &all_enemies_list },
};

static struct {
	enum lvledit_object_type object_type;
	object_category *categories;
	size_t length;
} category_list[] = {
	{ OBJECT_OBSTACLE, obstacle_category_list, sizeof(obstacle_category_list) / sizeof(obstacle_category_list[0]) },
	{ OBJECT_FLOOR, floor_category_list, sizeof(floor_category_list) / sizeof(floor_category_list[0]) },
	{ OBJECT_ITEM, item_category_list, sizeof(item_category_list) / sizeof(item_category_list[0]) },
	{ OBJECT_ENEMY, enemy_category_list, sizeof(enemy_category_list) / sizeof(enemy_category_list[0]) },
	{ OBJECT_WAYPOINT, waypoint_category_list, sizeof(waypoint_category_list) / sizeof(waypoint_category_list[0]) },
	{ OBJECT_MAP_LABEL, map_label_category_list, sizeof(map_label_category_list) / sizeof(map_label_category_list[0]) },
};

//
// Level editor buttons' primary actions
//

static void undo_button_click(struct widget_button *wb)
{
	level_editor_action_undo();
}

static void redo_button_click(struct widget_button *wb)
{
	level_editor_action_redo();
}

static void save_ship_button_click(struct widget_button *wb)
{
	if (game_root_mode == ROOT_IS_LVLEDIT) {	/*don't allow saving if root mode is GAME */
		save_map();
	}
}

static void beautify_grass_button_click(struct widget_button *wb)
{
	level_editor_beautify_grass_tiles(EditLevel());
}

static void delete_obstacle_button_click(struct widget_button *wb)
{
	level_editor_cut_selection();
}

static void next_object_button_click(struct widget_button *wb)
{
	level_editor_cycle_marked_object();
}

static void new_obstacle_label_button_click(struct widget_button *wb)
{
	if (single_tile_selection(OBJECT_OBSTACLE))
		action_change_obstacle_label_user(EditLevel(), single_tile_selection(OBJECT_OBSTACLE));
}

static void edit_chest_button_click(struct widget_button *wb)
{
	level_editor_edit_chest(single_tile_selection(OBJECT_OBSTACLE));
}

static void toggle_enemies_button_click(struct widget_button *wb)
{
	GameConfig.omit_enemies_in_level_editor = !GameConfig.omit_enemies_in_level_editor;
}

static void toggle_tooltips_button_click(struct widget_button *wb)
{
	GameConfig.show_lvledit_tooltips = !GameConfig.show_lvledit_tooltips;
}

static void toggle_collisions_button_click(struct widget_button *wb)
{
	draw_collision_rectangles = !draw_collision_rectangles;
}

static void toggle_grid_button_click(struct widget_button *wb)
{
	GameConfig.show_grid = !GameConfig.show_grid;
}

static void toggle_grid_button_right_click(struct widget_button *wb)
{
	GameConfig.grid_mode = (GameConfig.grid_mode + 1) % 2;
}

static void toggle_obstacles_button_click(struct widget_button *wb)
{
	GameConfig.omit_obstacles_in_level_editor = !GameConfig.omit_obstacles_in_level_editor;
}

static void toggle_map_labels_button_click(struct widget_button *wb)
{
	GameConfig.omit_map_labels_in_level_editor = !GameConfig.omit_map_labels_in_level_editor;
}

static void zoom_in_button_click(struct widget_button *wb)
{
	GameConfig.zoom_is_on = !GameConfig.zoom_is_on;
}

static void quit_button_click(struct widget_button *wb)
{
	TestMap();
	if (game_root_mode == ROOT_IS_GAME)
		level_editor_done = TRUE;
}

static void editor_right_button_click(struct widget_button *wb)
{
	widget_lvledit_toolbar_scroll_right();
}

static void editor_left_button_click(struct widget_button *wb)
{
	widget_lvledit_toolbar_scroll_left();
}

static void typeselect_obstacle_button_click(struct widget_button *wb)
{
	lvledit_select_type(OBJECT_OBSTACLE);
}

static void typeselect_floor_button_click(struct widget_button *wb)
{
	lvledit_select_type(OBJECT_FLOOR);
}

static void typeselect_item_button_click(struct widget_button *wb)
{
	lvledit_select_type(OBJECT_ITEM);
}

static void typeselect_enemy_button_click(struct widget_button *wb)
{
	lvledit_select_type(OBJECT_ENEMY);
}

static void typeselect_waypoint_button_click(struct widget_button *wb)
{
	lvledit_select_type(OBJECT_WAYPOINT);
}

static void typeselect_map_label_button_click(struct widget_button *wb)
{
	lvledit_select_type(OBJECT_MAP_LABEL);
}

static void toggle_waypoints_button_click(struct widget_button *wb)
{
	GameConfig.show_wp_connections = !GameConfig.show_wp_connections;
}

static void zoom_in_button_right_click(struct widget_button *wb)
{
	float zf = lvledit_zoomfact();
	zf += 3.0;
	if (zf > 9.0)
		zf = 3.0;
	if (!lvledit_set_zoomfact(zf)) {
		sprintf(VanishingMessage, _("Zoom factor set to %f."), zf);
	} else {
		sprintf(VanishingMessage, _("Could not change zoom factor."));
	}
	VanishingMessageEndDate = SDL_GetTicks() + max(1000, 2*(int)(Frame_Time()*1000.0f));
}

//
// Leveleditor floor layers button callbacks
//

static void floor_layers_button_click(struct widget_button *wb)
{
	GameConfig.show_all_floor_layers = !GameConfig.show_all_floor_layers;
}

static void floor_layers_button_right_click(struct widget_button *wb)
{
	int next_layer = current_floor_layer + 1;
	if (next_layer >= EditLevel()->floor_layers)
		next_layer = 0;
	action_change_floor_layer(EditLevel(), next_layer);
}

//
// Widgets update callbacks
//

static void _enable_if_undo_list_not_empty(struct widget *w)
{
	w->enabled = !list_empty(&to_undo);
}

static void _enable_if_redo_list_not_empty(struct widget *w)
{
	w->enabled = !list_empty(&to_redo);
}

static void _enable_is_selection_not_empty(struct widget *w)
{
	w->enabled = !selection_empty();
}

static void _enable_if_can_cycle_marked_object(struct widget *w)
{
	w->enabled = level_editor_can_cycle_marked_object();
}

static void _activate_if_show_wp_connections(struct widget *w)
{
	WIDGET_BUTTON(w)->active = !GameConfig.show_wp_connections;
}

static void _enable_if_single_obstacle_selected(struct widget *w)
{
	w->enabled = single_tile_selection(OBJECT_OBSTACLE) ? 1 : 0;
}

static void _enable_if_single_chest_selected(struct widget *w)
{
	w->enabled = 0;

	obstacle *o = single_tile_selection(OBJECT_OBSTACLE);
	if (o) {
		struct obstacle_spec *obs_spec = get_obstacle_spec(o->type);
		if (obs_spec->action && !strncmp(obs_spec->action, "chest", 5))
			w->enabled = 1;
	}
}

static void _activate_if_enemies_not_shown(struct widget *w)
{
	WIDGET_BUTTON(w)->active = GameConfig.omit_enemies_in_level_editor;
}

static void _activate_if_tooltips_not_shown(struct widget *w)
{
	WIDGET_BUTTON(w)->active = !GameConfig.show_lvledit_tooltips;
}

static void _activate_if_collrects_not_shown(struct widget *w)
{
	WIDGET_BUTTON(w)->active = !draw_collision_rectangles;
}

static void _activate_if_grid_shown(struct widget *w)
{
	WIDGET_BUTTON(w)->active = GameConfig.show_grid;
}

static void _activate_if_obstacles_not_shown(struct widget *w)
{
	WIDGET_BUTTON(w)->active = GameConfig.omit_obstacles_in_level_editor;
}

static void _activate_if_map_labels_not_shown(struct widget *w)
{
	WIDGET_BUTTON(w)->active = GameConfig.omit_map_labels_in_level_editor;
}

static void _activate_if_zoom_off(struct widget *w)
{
	WIDGET_BUTTON(w)->active = !GameConfig.zoom_is_on;
}

static void _activate_if_all_floors_not_shown(struct widget *w)
{
	w->enabled = selection_type() == OBJECT_FLOOR && EditLevel()->floor_layers > 1;
	WIDGET_BUTTON(w)->active = !GameConfig.show_all_floor_layers;
}

static void _activate_if_selection_of_obstacles(struct widget *w)
{
	WIDGET_BUTTON(w)->active = (selection_type() == OBJECT_OBSTACLE);
}

static void _activate_if_selection_of_tiles(struct widget *w)
{
	WIDGET_BUTTON(w)->active = (selection_type() == OBJECT_FLOOR);
}

static void _activate_if_selection_of_items(struct widget *w)
{
	WIDGET_BUTTON(w)->active = (selection_type() == OBJECT_ITEM);
}

static void _activate_if_selection_of_enemies(struct widget *w)
{
	WIDGET_BUTTON(w)->active = (selection_type() == OBJECT_ENEMY);
}

static void _activate_if_selection_of_waypoints(struct widget *w)
{
	WIDGET_BUTTON(w)->active = (selection_type() == OBJECT_WAYPOINT);
}

static void _activate_if_selection_of_map_labels(struct widget *w)
{
	WIDGET_BUTTON(w)->active = (selection_type() == OBJECT_MAP_LABEL);
}

/**
 * This function returns the editor top level widget and creates it if necessary.
 */

struct widget_group *get_lvledit_ui()
{
	if (level_editor_widget_group)
		// Editor UI already initialized.
		return level_editor_widget_group;

	level_editor_widget_group = widget_group_create();
	widget_set_rect(WIDGET(level_editor_widget_group), 0, 0, GameConfig.screen_width, GameConfig.screen_height);
	lvledit_widget_list = &level_editor_widget_group->list;
	
	struct widget *map;

	/* Build our interface */

	/* The map (has to be the first widget in the list) */
	map = widget_lvledit_map_create();
	widget_group_add(level_editor_widget_group, map);	

	/* The toolbar */
	struct widget *widget = widget_lvledit_toolbar_create();
	widget_group_add(level_editor_widget_group, widget);

	struct {
		int btn_index;
		char *text;
		string tooltip_text;
		int  number_of_toggle_states;
		void (*activate_button)(struct widget_button *);
		void (*activate_button_secondary)(struct widget_button *);
		void (*update)(struct widget *);
	} b[] = {
		{LEVEL_EDITOR_UNDO_BUTTON, NULL,
			_("Undo\n\nUse this button to undo your last actions."),
			1,
			undo_button_click,
			NULL,
			_enable_if_undo_list_not_empty
		},
		{LEVEL_EDITOR_REDO_BUTTON, NULL,
			_("Redo\n\nUse this button to redo an action."),
			1,
			redo_button_click,
			NULL,
			_enable_if_redo_list_not_empty
		},
		{LEVEL_EDITOR_SAVE_SHIP_BUTTON, NULL,
			_("Save Map\n\nThis button will save your current map over the file '../map/levels.dat' from your current working directory."),
			2,
			save_ship_button_click,
			NULL,
			NULL
		},
		{LEVEL_EDITOR_BEAUTIFY_GRASS_BUTTON, NULL,
			_("Beautify grass button\n\nUse this button to automatically 'beautify' rough edges of the grass-sand tiles. It will apply to the selected floor or, if not applicable, to the entire level.\n\nYou can also use Ctrl-b for this."),
			1,
			beautify_grass_button_click,
			NULL,
			NULL
		},
		{LEVEL_EDITOR_DELETE_OBSTACLE_BUTTON, NULL,
			_("Delete selected objects\n\nUse this button to delete the currently marked objects.\n\nYou can also use Ctrl-X for this."),
			1,
			delete_obstacle_button_click,
			NULL,
			_enable_is_selection_not_empty
		},
		{LEVEL_EDITOR_NEXT_OBJECT_BUTTON, NULL,
			_("Next object on currently selected tile\n\nUse this button to cycle the currently marked object on this tile.\n\nYou can also use the N key for this."),
			1,
			next_object_button_click,
			NULL,
			_enable_if_can_cycle_marked_object
		},
		{LEVEL_EDITOR_TOGGLE_WAYPOINT_CONNECTIONS_BUTTON, NULL,
			_("Toggle display waypoint connections\n\nUse this button to toggle between waypoint connections displayed on and off."),
			2,
			toggle_waypoints_button_click,
			NULL,
			_activate_if_show_wp_connections
		},
		{LEVEL_EDITOR_NEW_OBSTACLE_LABEL_BUTTON, NULL,
			_("New obstacle label\n\nUse this button to attach a label to the currently marked obstacle.  These obstacle labels can be used to define obstacles to be modified by events.\n"),
			1,
			new_obstacle_label_button_click,
			NULL,
			_enable_if_single_obstacle_selected
		},
		{LEVEL_EDITOR_EDIT_CHEST_BUTTON, NULL,
			_("Edit chests contents\n\nUse this button to change the contents of the chest. If it is empty the game may generate random items at run time."),
			1,
			edit_chest_button_click,
			NULL,
			_enable_if_single_chest_selected
		},
		{LEVEL_EDITOR_TOGGLE_ENEMIES_BUTTON, NULL,
			_("Toggle display enemies\n\nUse this button to toggle between enemies displayed in level editor or enemies hidden in level editor."),
			2,
			toggle_enemies_button_click,
			NULL,
			_activate_if_enemies_not_shown
		},
		{LEVEL_EDITOR_TOGGLE_TOOLTIPS_BUTTON, NULL,
			_("Toggle display tooltips\n\nUse this button to toggle these annoying help windows on and off."),
			2,
			toggle_tooltips_button_click,
			NULL,
			_activate_if_tooltips_not_shown
		},
		{LEVEL_EDITOR_TOGGLE_COLLISION_RECTS_BUTTON, NULL,
			_("Toggle display collision rectangles\n\nUse this button to toggle the visible collision rectangles on and off."),
			2,
			toggle_collisions_button_click,
			NULL,
			_activate_if_collrects_not_shown
		},
		{LEVEL_EDITOR_TOGGLE_GRID_BUTTON_OFF, NULL,
			_("Toggle grid mode.\n\nUse this button to toggle grid displaying on and off.\n\nUse right click to change the grid mode."),
			2,
			toggle_grid_button_click,
			toggle_grid_button_right_click,
			_activate_if_grid_shown
		},
		{LEVEL_EDITOR_TOGGLE_OBSTACLES_BUTTON, NULL,
			_("Toggle display obstacles\n\nUse this button to toggle between obstacles displayed in level editor or obstacles hidden in level editor."),
			2,
			toggle_obstacles_button_click,
			NULL,
			_activate_if_obstacles_not_shown
		},
		{LEVEL_EDITOR_TOGGLE_MAP_LABELS_BUTTON, NULL,
			_("Toggle display map labels name\n\nUse this button to toggle between map labels name displayed in level editor or map labels name hidden in level editor."),
			2,
			toggle_map_labels_button_click,
			NULL,
			_activate_if_map_labels_not_shown
		},
		{LEVEL_EDITOR_ZOOM_IN_BUTTON, NULL,
			_("Zoom in/out\n\nUse this button to zoom INTO or OUT of the level.\n\nUse right click to change the zoom ratio.\n"),
			2,
			zoom_in_button_click,
			zoom_in_button_right_click,
			_activate_if_zoom_off
		},
		{LEVEL_EDITOR_ALL_FLOOR_LAYERS_BUTTON, NULL,
			_("Toggle floor layers\n\nUse this button to toggle between all floor layers displayed or single floor layer displayed.\n\nUse right click to change the current floor layer.\n"),
			2,
			floor_layers_button_click,
			floor_layers_button_right_click,
			_activate_if_all_floors_not_shown
		},
		{LEVEL_EDITOR_QUIT_BUTTON, NULL,
			_("Test Map\n\nThis will save your map and reload it after you finish testing, avoiding saving an unclean world state."),
			1,
			quit_button_click,
			NULL,
			NULL
		},
		{RIGHT_LEVEL_EDITOR_BUTTON, NULL,
			NULL,
			1,
			editor_right_button_click,
			NULL,
			NULL
		},
		{LEFT_LEVEL_EDITOR_BUTTON, NULL,
			NULL,
			1,
			editor_left_button_click,
			NULL,
			NULL
		},
		{LEVEL_EDITOR_TYPESELECT_OBSTACLE_BUTTON, _("Obstacle"),
			NULL,
			2,
			typeselect_obstacle_button_click,
			NULL,
			_activate_if_selection_of_obstacles
		},
		{LEVEL_EDITOR_TYPESELECT_FLOOR_BUTTON, _("Floor"),
			NULL,
			2,
			typeselect_floor_button_click,
			NULL,
			_activate_if_selection_of_tiles
		},
		{LEVEL_EDITOR_TYPESELECT_ITEM_BUTTON, _("Item"),
			NULL,
			2,
			typeselect_item_button_click,
			NULL,
			_activate_if_selection_of_items
		},
		{LEVEL_EDITOR_TYPESELECT_ENEMY_BUTTON, _("Enemy"),
			NULL,
			2,
			typeselect_enemy_button_click,
			NULL,
			_activate_if_selection_of_enemies
		},
		//; TRANSLATORS: Waypoint
		{LEVEL_EDITOR_TYPESELECT_WAYPOINT_BUTTON, _("Wayp."),
			NULL,
			2,
			typeselect_waypoint_button_click,
			NULL,
			_activate_if_selection_of_waypoints
		},
		{LEVEL_EDITOR_TYPESELECT_MAP_LABEL_BUTTON, _("Label"),
			NULL,
			2,
			typeselect_map_label_button_click,
			NULL,
			_activate_if_selection_of_map_labels
		}
	};

	int i, j;

	// Build buttons using the array above.
	for (i = 0; i < sizeof(b) / sizeof(b[0]); i++) {
		struct widget_button *button = widget_button_create();

		// Set text and tooltip.
		button->text = b[i].text; 
		button->tooltip.text = b[i].tooltip_text; 
		
		// Set callbacks.
		button->activate_button = b[i].activate_button;
		button->activate_button_secondary = b[i].activate_button_secondary;
		WIDGET(button)->update = b[i].update;

		// Load images using AllMousePressButtons.
		// Buttons use 3 images for each toggle state. Radio buttons have two toggle states.
		for (j = 0; j < b[i].number_of_toggle_states; j++) {
			// AllMousePressButtons have only two images: normal and pressed. Hovered state will be ignored.
			button->image[j][0] = widget_load_image_resource(AllMousePressButtons[b[i].btn_index + j * 2].button_image_file_name, NO_MOD);		// Normal state
			button->image[j][1] = NULL;												// Hovered state
			button->image[j][2] = widget_load_image_resource(AllMousePressButtons[b[i].btn_index + j * 2 + 1].button_image_file_name, NO_MOD);	// Pressed state
		}
		
		// Set button size.
		SDL_Rect rect = AllMousePressButtons[b[i].btn_index].button_rect;	

		// Size can be specified in AllMousePressButtons or left to 0, meaning the actual image size must be used.
		rect.w = (rect.w) ? rect.w : button->image[0][0]->w;
		rect.h = (rect.h) ? rect.h : button->image[0][0]->h;
		widget_set_rect(WIDGET(button), rect.x, rect.y, rect.w, rect.h);
		
		//Add the button to the level editor top level group.
		widget_group_add(level_editor_widget_group, WIDGET(button));
	}

	lvledit_build_tile_lists();

	// Load categories for obstacles
	char fpath[PATH_MAX];
	find_file(fpath, GUI_DIR, "level_editor/obstacle_categories.lua", NULL, PLEASE_INFORM | IS_FATAL);
	run_lua_file(LUA_CONFIG, fpath);

	// Build list of all obstacles
	all_obstacles_list = MyMalloc(sizeof(int) * (obstacle_map.size + 1));
	for (i = 0, j = 0; i < obstacle_map.size; i++) {
		// Dummy obstacles (obstacles without image) should not be displayed in the
		// level editor obstacle list.
		if (!image_loaded(get_obstacle_image(i, 0)))
			continue;
		all_obstacles_list[j++] = i;
	}
	all_obstacles_list[j] = -1;

	// Create category selectors 
	for (i = 0; i < sizeof(category_list) / sizeof(category_list[0]); i++) {
		enum lvledit_object_type type = category_list[i].object_type;
		object_category *categories = category_list[i].categories;
		for (j = 0; j < category_list[i].length; j++) {
			struct widget *widget = widget_lvledit_categoryselector_create(j, _(categories[j].name), type, *categories[j].object_list);
			categories[j].cs = widget->ext;
			widget_group_add(level_editor_widget_group, widget);
		}
	}

	// Create the minimap
	widget = widget_lvledit_minimap_create();
	widget_group_add(level_editor_widget_group, widget);
	
	// Activate the obstacle type selector
	lvledit_select_type(OBJECT_OBSTACLE);
	
	widget_lvledit_map_init();

	return level_editor_widget_group;
}

void free_lvledit_ui()
{
	if (level_editor_widget_group)
	{
		struct widget *w = WIDGET(level_editor_widget_group);
		w->free(w);
		free(level_editor_widget_group);
	}

	if (all_obstacles_list) {
		free(all_obstacles_list);
	}
}

struct widget *get_active_widget(int x, int y)
{
	struct widget *w;
	list_for_each_entry_reverse(w, lvledit_widget_list, node) {
		if (!w->enabled)
			continue;
		if (MouseCursorIsInRect(&w->rect, x, y)) {
			return w;
		}
	}

	return NULL;
}

void lvledit_select_type(enum lvledit_object_type type)
{
	struct widget *cs_widget;

	if (selection_type() != type) {
		// When the user changes the current object type selected, reset tools
		lvledit_reset_tools();
	}

	// Find the categories which must be enabled
 	list_for_each_entry_reverse(cs_widget, lvledit_widget_list, node) {
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
	if (!categories || length == 0) return;

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
