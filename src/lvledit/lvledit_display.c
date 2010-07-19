/* 
 *
 *   Copyright (c) 2002, 2003 Johannes Prix
 *   Copyright (c) 2004-2010 Arthur Huillet
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

#define _leveleditor_display_c

#include "system.h"

#include "defs.h"
#include "struct.h"
#include "global.h"
#include "proto.h"

#include "SDL_rotozoom.h"

#include "lvledit/lvledit.h"
#include "lvledit/lvledit_actions.h"
#include "lvledit/lvledit_widgets.h"

iso_image level_editor_waypoint_cursor[2] = { UNLOADED_ISO_IMAGE, UNLOADED_ISO_IMAGE };

#define DEFAULT_ZOOM_FACTOR 3.0
static float lvledit_zoom_factor = DEFAULT_ZOOM_FACTOR;
static float lvledit_zoom_factor_inv = 1.0 / DEFAULT_ZOOM_FACTOR;

float lvledit_zoomfact()
{
	return lvledit_zoom_factor;
}

float lvledit_zoomfact_inv()
{
	return lvledit_zoom_factor_inv;
}

int lvledit_set_zoomfact(float zf)
{
	if (!use_open_gl) {
		/* Zoom factor cannot be changed in SDL mode. */
		return -1;
	}

	lvledit_zoom_factor = zf;
	lvledit_zoom_factor_inv = 1.0 / zf;
	return 0;
}

/**
 * Displays the GPS in the editor
 */
static void gps_show() {
	static char gps_text[200];
	snprintf(gps_text, sizeof(gps_text) - 1, _(" X=%3.1f Y=%3.1f L=%d\n"), Me.pos.x, Me.pos.y, Me.pos.z);
	DisplayText(gps_text, User_Rect.x + 1, GameConfig.screen_height - 1 * FontHeight(GetCurrentFont()), NULL /*&User_Rect */ , 1.0);
}

/**
 * Now we print out the map label information about this map location.
 */
static void print_label_information(level *EditLevel)
{
	char PanelText[5000] = "";
	struct map_label_s *map_label;
	int i;

	for (i = 0; i < EditLevel->map_labels.size; i++) {
		// Get the map label
		map_label = &ACCESS_MAP_LABEL(EditLevel->map_labels, i);

		if ((fabsf(Me.pos.x - (map_label->pos.x + 0.5)) <= 0.5) && 
			 (fabsf(Me.pos.y - (map_label->pos.y + 0.5)) <= 0.5)) {
			// When a map label is located at the same position than the cursor,
			// we must print the map label information

			// Create the map label information
			sprintf(PanelText, _("\n Map label information: \n label_name=\"%s\"."), map_label->label_name);

			// Display the map label information on the screen
			DisplayText(PanelText, User_Rect.x, GameConfig.screen_height - 5 * FontHeight(GetCurrentFont()), NULL, 1.0);

			return;
		}
	}
}

/**
 * This function is used by thelevel *Editor integrated into 
 * freedroid.  It highlights the map position that is currently 
 * edited or would be edited, if the user pressed something.  I.e. 
 * it provides a "cursor" for thelevel *Editor.
 */
static void Highlight_Current_Block(int mask)
{
	level *EditLevel;
	static iso_image level_editor_cursor = { NULL, 0, 0 };
	char fpath[2048];

	EditLevel = curShip.AllLevels[Me.pos.z];
#define HIGHLIGHTCOLOR 255

	// Maybe, if the level editor floor cursor has not yet been loaded,
	// we need to load it.
	//
	if (level_editor_cursor.surface == NULL && !level_editor_cursor.texture_has_been_created) {
		find_file("level_editor_floor_cursor.png", GRAPHICS_DIR, fpath, 0);
		get_iso_image_from_file_and_path(fpath, &(level_editor_cursor), TRUE);
		if (level_editor_cursor.surface == NULL) {
			ErrorMessage(__FUNCTION__, "\
		  Unable to load the level editor floor cursor.", PLEASE_INFORM, IS_FATAL);
		}
		if (use_open_gl)
			make_texture_out_of_surface(&level_editor_cursor);
	}

	if (mask & ZOOM_OUT) {
		if (use_open_gl)
			draw_gl_textured_quad_at_map_position(&level_editor_cursor,
							      Me.pos.x, Me.pos.y, 1.0, 1.0, 1.0, 0, FALSE, lvledit_zoomfact_inv());
		else
			blit_zoomed_iso_image_to_map_position(&level_editor_cursor, Me.pos.x, Me.pos.y);
	} else {
		if (use_open_gl)
			draw_gl_textured_quad_at_map_position(&level_editor_cursor, Me.pos.x, Me.pos.y, 1.0, 1.0, 1.0, 0, FALSE, 1.0);
		else
			blit_iso_image_to_map_position(&level_editor_cursor, Me.pos.x, Me.pos.y);
	}

	print_label_information(EditLevel);

}				// void Highlight_Current_Block(void)

/**
 * This function is used to draw a line between given map tiles.  It is
 * mainly used for the map editor to highlight connections and the 
 * current map tile target.
 */
void draw_connection_between_tiles(float x1, float y1, float x2, float y2, int mask)
{
	float steps;
	float dist;
	int i;
	static iso_image level_editor_dot_cursor = UNLOADED_ISO_IMAGE;
	char fpath[2048];

	// Maybe, if the level editor dot cursor has not yet been loaded,
	// we need to load it.
	//
	if ((level_editor_dot_cursor.surface == NULL) && (!level_editor_dot_cursor.texture_has_been_created)) {
		find_file("level_editor_waypoint_dot.png", GRAPHICS_DIR, fpath, 0);
		get_iso_image_from_file_and_path(fpath, &(level_editor_dot_cursor), TRUE);
		if (level_editor_dot_cursor.surface == NULL) {
			ErrorMessage(__FUNCTION__, "\
		    Unable to load the level editor waypoint dot cursor.", PLEASE_INFORM, IS_FATAL);
		}

		if (use_open_gl)
			make_texture_out_of_surface(&level_editor_dot_cursor);
	}
	// So now that the dot cursor has been loaded, we can start to
	// actually draw the dots.
	//

	// We measure the distance that we have to go and then we draw some
	// dots at some convex combinations of our two vectors.  Very fine.
	//
	dist = sqrt((x1 - x2) * (x1 - x2) + (y1 - y2) * (y1 - y2));

	steps = dist * 4;	// let's say 4 dots per square to mark the line, ok?

	if (!steps)		// Oh, noh, the points are at the same place
		return;
	for (i = 0; i < steps + 1; i++) {
		float x, y;
		x = (((float)i) / steps) * x1 + x2 * (steps - i) / steps;
		y = (((float)i) / steps) * y1 + y2 * (steps - i) / steps;
		if (mask & ZOOM_OUT) {
			if (use_open_gl)
				draw_gl_textured_quad_at_map_position(&level_editor_dot_cursor, x, y,
								      1.0, 1.0, 1.0, 0.25, FALSE, lvledit_zoomfact_inv());
			else
				blit_zoomed_iso_image_to_map_position(&level_editor_dot_cursor, x, y);
		} else {
			if (use_open_gl)
				draw_gl_textured_quad_at_map_position(&level_editor_dot_cursor, x, y, 1.0, 1.0, 1.0, TRUE, FALSE, 1.0);
			else
				blit_iso_image_to_map_position(&level_editor_dot_cursor, x, y);
		}
	}

};				// void draw_connection_between_tiles ( .... )

/**
 * This function is used by thelevel *Editor integrated into 
 * freedroid.  It marks all waypoints with a cross.
 */
static void show_waypoints(int mask)
{
	waypoint *wpts = EditLevel()->waypoints.arr;
	char fpath[2048];
	float r, g, b;
	float x, y;
	int i, j;

	// Maybe, if the level editor floor cursor has not yet been loaded,
	// we need to load it.
	for (i = 0; i < 2; i++) {
		if ((level_editor_waypoint_cursor[i].surface == NULL) && (!level_editor_waypoint_cursor[i].texture_has_been_created)) {
			if (i == 0)
				find_file("level_editor_waypoint_cursor.png", GRAPHICS_DIR, fpath, 0);
			else
				find_file("level_editor_norand_waypoint_cursor.png", GRAPHICS_DIR, fpath, 0);
			get_iso_image_from_file_and_path(fpath, &(level_editor_waypoint_cursor[i]), TRUE);

			if (level_editor_waypoint_cursor[i].surface == NULL) {
				ErrorMessage(__FUNCTION__, "\
		        Unable to load the level editor waypoint cursor.", PLEASE_INFORM, IS_FATAL);
			}

			if (use_open_gl)
				make_texture_out_of_surface(&(level_editor_waypoint_cursor[i]));
		}
	}

	for (i = 0; i < EditLevel()->waypoints.size; i++) {
		// Calculate the position of the waypoint
		x = wpts[i].x + 0.5;
		y = wpts[i].y + 0.5;

		// Apply disco mode when current waypoint is selected
		object_vtx_color(&wpts[i], &r, &g, &b);

		// Draw the waypoint on the map
		if (mask && ZOOM_OUT) {
			if (use_open_gl) {
				draw_gl_textured_quad_at_map_position(&level_editor_waypoint_cursor[wpts[i].suppress_random_spawn],
								      x, y, r, g, b, 0.25, FALSE, lvledit_zoomfact_inv());
			} else {
				blit_zoomed_iso_image_to_map_position(&level_editor_waypoint_cursor[wpts[i].suppress_random_spawn], x, y);
			}
		} else {
			if (use_open_gl) {
				draw_gl_textured_quad_at_map_position(&level_editor_waypoint_cursor[wpts[i].suppress_random_spawn],
								      x, y, r, g, b, 0, FALSE, 1.0);
			}
			else {
				blit_iso_image_to_map_position(&level_editor_waypoint_cursor[wpts[i].suppress_random_spawn], x, y);
			}
		}

		// Get the connections of the waypoint
		int *connections = wpts[i].connections.arr;

		for (j = 0; j < wpts[i].connections.size; j++) {
			waypoint *to_wp = &wpts[connections[j]];

			if ((EditX() == wpts[i].x) && (EditY() == wpts[i].y)) {
				draw_connection_between_tiles(x, y, to_wp->x + 0.5, to_wp->y + 0.5, mask);
			}
		}
	}

	// Now we do something extra:  If there is a connection attempt currently
	// going on, then we also draw a connection from the origin point to the
	// current cursor (i.e. 'me') position.
	if (OriginWaypoint != (-1)) {
		// Draw the connection between the origin waypoint and the cursor (ie. 'me')
		draw_connection_between_tiles(wpts[OriginWaypoint].x + 0.5, wpts[OriginWaypoint].y + 0.5, Me.pos.x, Me.pos.y, mask);
	}
}

/**
 * This function is used by thelevel *Editor integrated into 
 * freedroid.  It marks all places that have a label attached to them.
 */
static void show_map_labels(int mask)
{
	static iso_image map_label_indicator = UNLOADED_ISO_IMAGE;
	static int first_function_call = TRUE;
	char fpath[2048];
	level *EditLevel = curShip.AllLevels[Me.pos.z];
	struct map_label_s *map_label;
	int i;

	// On the first function call to this function, we must load the map label indicator
	// iso image from the disk to memory and keep it there as static.  That should be
	// it for here.
	if (first_function_call) {
		first_function_call = FALSE;
		find_file("level_editor_map_label_indicator.png", GRAPHICS_DIR, fpath, 0);
		get_iso_image_from_file_and_path(fpath, &(map_label_indicator), TRUE);

		if (use_open_gl)
			make_texture_out_of_surface(&map_label_indicator);
	}

	// Now we can draw a fine indicator at all the position nescessary...
	for (i = 0; i < EditLevel->map_labels.size; i++) {	
		// Get the map label
		map_label = &ACCESS_MAP_LABEL(EditLevel->map_labels, i);

		if (!(mask && ZOOM_OUT)) {
			if (use_open_gl)
				draw_gl_textured_quad_at_map_position(&map_label_indicator, map_label->pos.x + 0.5,
								      map_label->pos.y + 0.5, 1.0, 1.0, 1.0, FALSE, FALSE,
								      1.);
			else
				blit_iso_image_to_map_position(&map_label_indicator, map_label->pos.x + 0.5,
							       map_label->pos.y + 0.5);
		} else {
			if (use_open_gl)
				draw_gl_textured_quad_at_map_position(&map_label_indicator, map_label->pos.x + 0.5,
								      map_label->pos.y + 0.5, 1.0, 1.0, 1.0, 0.25, FALSE,
								      lvledit_zoomfact_inv());
			else
				blit_zoomed_iso_image_to_map_position(&(map_label_indicator), map_label->pos.x + 0.5,
								      map_label->pos.y + 0.5);
		}
	}
}

/**
 * When the mouse has rested idle on some mouse button in the level 
 * editor (and also tooltips are enabled) then some small window (a 
 * tooltip) will appear and describe the purpose of the button under the
 * mouse cursor.  Bringing up this tooltip window is the purpose of this
 * function.
 */
static void show_button_tooltip(const char *tooltip_text)
{
	float w = 400;
	float h = 20; // will be expanded by show_backgrounded_text_rectangle()
	float x = 120;
	float y = 160;

	show_backgrounded_text_rectangle(tooltip_text, FPS_Display_BFont, x, y, w, h);
}

/**
 *
 *
 */
static void show_level_editor_tooltips(void)
{
	static float time_spent_on_some_button = 0;
	static float previous_function_call_time = 0;

	time_spent_on_some_button += SDL_GetTicks() - previous_function_call_time;

	previous_function_call_time = SDL_GetTicks();

	if (!GameConfig.show_tooltips)
		return;

#define TICKS_UNTIL_TOOLTIP 1200

	if (MouseCursorIsOnButton(GO_LEVEL_NORTH_BUTTON, GetMousePos_x(), GetMousePos_y())) {
		if (time_spent_on_some_button > TICKS_UNTIL_TOOLTIP && EditLevel()->jump_target_north > 0)
			show_button_tooltip(_
					    ("Go level north\n\nUse this button to move one level north, i.e. to the level that is glued to the northern side of this level."));
	} else if (MouseCursorIsOnButton(GO_LEVEL_SOUTH_BUTTON, GetMousePos_x(), GetMousePos_y())) {
		if (time_spent_on_some_button > TICKS_UNTIL_TOOLTIP && EditLevel()->jump_target_south > 0)
			show_button_tooltip(_
					    ("Go level south\n\nUse this button to move one level south, i.e. to the level that is glued to the southern side of this level."));
	} else if (MouseCursorIsOnButton(GO_LEVEL_EAST_BUTTON, GetMousePos_x(), GetMousePos_y())) {
		if (time_spent_on_some_button > TICKS_UNTIL_TOOLTIP && EditLevel()->jump_target_east > 0)
			show_button_tooltip(_
					    ("Go level east\n\nUse this button to move one level east, i.e. to the level that is glued to the eastern side of this level."));
	} else if (MouseCursorIsOnButton(GO_LEVEL_WEST_BUTTON, GetMousePos_x(), GetMousePos_y())) {
		if (time_spent_on_some_button > TICKS_UNTIL_TOOLTIP && EditLevel()->jump_target_west > 0)
			show_button_tooltip(_
					    ("Go level west\n\nUse this button to move one level west, i.e. to the level that is glued to the western side of this level."));
	} else if (MouseCursorIsOnButton(LEVEL_EDITOR_SAVE_SHIP_BUTTON, GetMousePos_x(), GetMousePos_y())
		   || MouseCursorIsOnButton(LEVEL_EDITOR_SAVE_SHIP_BUTTON_OFF, GetMousePos_x(), GetMousePos_y())) {
		if (time_spent_on_some_button > TICKS_UNTIL_TOOLTIP) {
			if (game_root_mode == ROOT_IS_GAME)
				show_button_tooltip(_
						    ("Save Impossible!\n\nPlaying on a map leaves the world in an unclean state not suitable for saving. Enter the editor directly from the main menu to be able to save any changes to the map file."));
			if (game_root_mode == ROOT_IS_LVLEDIT)
				show_button_tooltip(_
						    ("Save Map\n\nThis button will save your current map over the file '../map/freedroid.levels' from your current working directory."));
		}
	} else if (MouseCursorIsOnButton(LEVEL_EDITOR_DELETE_OBSTACLE_BUTTON, GetMousePos_x(), GetMousePos_y())) {
		if (time_spent_on_some_button > TICKS_UNTIL_TOOLTIP)
			show_button_tooltip(_
					    ("Delete selected obstacle\n\nUse this button to delete the currently marked obstacle.\n\nYou can also use Ctrl-X for this."));
	} else if (MouseCursorIsOnButton(LEVEL_EDITOR_NEXT_OBSTACLE_BUTTON, GetMousePos_x(), GetMousePos_y())) {
		if (time_spent_on_some_button > TICKS_UNTIL_TOOLTIP)
			show_button_tooltip(_
					    ("Next obstacle on currently selected tile\n\nUse this button to cycle the currently marked obstacle on this tile.\n\nYou can also use the N key for this."));
	} else if (MouseCursorIsOnButton(LEVEL_EDITOR_BEAUTIFY_GRASS_BUTTON, GetMousePos_x(), GetMousePos_y())) {
		if (time_spent_on_some_button > TICKS_UNTIL_TOOLTIP)
			show_button_tooltip(_
			    ("Beautify grass button\n\nUse this button to automatically 'beautify' rough edges of the grass-sand tiles. It will apply to the selected floor or, if not applicable, to the entire level.\n\nYou can also use Ctrl-b for this."));
	} else if (MouseCursorIsOnButton(LEVEL_EDITOR_ZOOM_OUT_BUTTON, GetMousePos_x(), GetMousePos_y())
		   || MouseCursorIsOnButton(LEVEL_EDITOR_ZOOM_IN_BUTTON, GetMousePos_x(), GetMousePos_y())) {
		if (time_spent_on_some_button > TICKS_UNTIL_TOOLTIP)
			show_button_tooltip(_
					    ("Zoom in/out\n\nUse this button to zoom INTO or OUT of the level.\n\nUse right click to change the zoom ratio (OpenGL only).\n"));
	} else if (MouseCursorIsOnButton(LEVEL_EDITOR_NEW_OBSTACLE_LABEL_BUTTON, GetMousePos_x(), GetMousePos_y())) {
		if (time_spent_on_some_button > TICKS_UNTIL_TOOLTIP)
			show_button_tooltip(_
					    ("New obstacle label\n\nUse this button to attach a label to the currently marked obstacle.  These obstacle labels can be used to define obstacles to be modified by events.\n Note that you can also use the hotkey 'h' for this."));
	} else if (MouseCursorIsOnButton(LEVEL_EDITOR_NEW_MAP_LABEL_BUTTON, GetMousePos_x(), GetMousePos_y())) {
		if (time_spent_on_some_button > TICKS_UNTIL_TOOLTIP)
			show_button_tooltip(_
					    ("New map label\n\nUse this button to attach a new map label to the current cursor position.  These map labels can be used to define starting points for bots and characters or also to define locations for events and triggers."));
	} else if (MouseCursorIsOnButton(LEVEL_EDITOR_EDIT_CHEST_BUTTON, GetMousePos_x(), GetMousePos_y())) {
		if (time_spent_on_some_button > TICKS_UNTIL_TOOLTIP)
			show_button_tooltip(_
					    ("Edit chests contents\n\nUse this button to change the contents of the chest. If it is empty the game may generate random items at run time."));
	} else if (MouseCursorIsOnButton(LEVEL_EDITOR_ESC_BUTTON, GetMousePos_x(), GetMousePos_y())) {
		if (time_spent_on_some_button > TICKS_UNTIL_TOOLTIP)
			show_button_tooltip(_
					    ("Menu\n\nUse this button to enter the main menu of the level editor.\n Note, that you can also use the Escape key to enter the level editor main menu."));
	} else if (MouseCursorIsOnButton(LEVEL_EDITOR_QUIT_BUTTON, GetMousePos_x(), GetMousePos_y())) {
		if (time_spent_on_some_button > TICKS_UNTIL_TOOLTIP) {
			if (game_root_mode == ROOT_IS_GAME)
				show_button_tooltip(_
						    ("Back to game\n\nChanges made to the world will be permanent for current Hero. To avoid, load a previous savegame."));
			if (game_root_mode == ROOT_IS_LVLEDIT)
				show_button_tooltip(_
						    ("Test Map\n\nThis will save your map and reload it after you finish testing, avoiding saving an unclean world state."));
		}
	} else if (MouseCursorIsOnButton(LEVEL_EDITOR_TOGGLE_ENEMIES_BUTTON, GetMousePos_x(), GetMousePos_y())
		   || MouseCursorIsOnButton(LEVEL_EDITOR_TOGGLE_ENEMIES_BUTTON_OFF, GetMousePos_x(), GetMousePos_y())) {
		if (time_spent_on_some_button > TICKS_UNTIL_TOOLTIP)
			show_button_tooltip(_
					    ("Toggle display enemies\n\nUse this button to toggle between enemies displayed in level editor or enemies hidden in level editor."));
	} else if (MouseCursorIsOnButton(LEVEL_EDITOR_TOGGLE_OBSTACLES_BUTTON, GetMousePos_x(), GetMousePos_y())
		   || MouseCursorIsOnButton(LEVEL_EDITOR_TOGGLE_OBSTACLES_BUTTON_OFF, GetMousePos_x(), GetMousePos_y())) {
		if (time_spent_on_some_button > TICKS_UNTIL_TOOLTIP)
			show_button_tooltip(_
					    ("Toggle display obstacles\n\nUse this button to toggle between obstacles displayed in level editor or obstacles hidden in level editor."));
	} else if (MouseCursorIsOnButton(LEVEL_EDITOR_UNDO_BUTTON, GetMousePos_x(), GetMousePos_y())
		   || MouseCursorIsOnButton(LEVEL_EDITOR_UNDO_BUTTON_PUSHED, GetMousePos_x(), GetMousePos_y())) {
		if (time_spent_on_some_button > TICKS_UNTIL_TOOLTIP && !list_empty(&to_undo))
			show_button_tooltip(_("Undo\n\nUse this button to undo your last actions."));
	} else if (MouseCursorIsOnButton(LEVEL_EDITOR_REDO_BUTTON, GetMousePos_x(), GetMousePos_y()) ||
		   MouseCursorIsOnButton(LEVEL_EDITOR_REDO_BUTTON_PUSHED, GetMousePos_x(), GetMousePos_y())) {
		if (time_spent_on_some_button > TICKS_UNTIL_TOOLTIP && !list_empty(&to_redo))
			show_button_tooltip(_("Redo\n\nUse this button to redo an action."));
	} else if (MouseCursorIsOnButton(LEVEL_EDITOR_TOGGLE_TOOLTIPS_BUTTON, GetMousePos_x(), GetMousePos_y()) ||
		   MouseCursorIsOnButton(LEVEL_EDITOR_TOGGLE_TOOLTIPS_BUTTON_OFF, GetMousePos_x(), GetMousePos_y())) {
		if (time_spent_on_some_button > TICKS_UNTIL_TOOLTIP)
			show_button_tooltip(_
					    ("Toggle display tooltips\n\nUse this button to toggle these annoying help windows on and off."));
	} else if (MouseCursorIsOnButton(LEVEL_EDITOR_TOGGLE_COLLISION_RECTS_BUTTON, GetMousePos_x(), GetMousePos_y())
		   || MouseCursorIsOnButton(LEVEL_EDITOR_TOGGLE_COLLISION_RECTS_BUTTON_OFF, GetMousePos_x(), GetMousePos_y())) {
		if (time_spent_on_some_button > TICKS_UNTIL_TOOLTIP)
			show_button_tooltip(_
					    ("Toggle display collision rectangles\n\nUse this button to toggle the visible collision rectangles on and off.\nThis only works using OpenGL mode!"));
	} else if (MouseCursorIsOnButton(LEVEL_EDITOR_TOGGLE_GRID_BUTTON, GetMousePos_x(), GetMousePos_y())
		   || MouseCursorIsOnButton(LEVEL_EDITOR_TOGGLE_GRID_BUTTON_FULL, GetMousePos_x(), GetMousePos_y())
		   || MouseCursorIsOnButton(LEVEL_EDITOR_TOGGLE_GRID_BUTTON_OFF, GetMousePos_x(), GetMousePos_y())) {
		if (time_spent_on_some_button > TICKS_UNTIL_TOOLTIP)
			show_button_tooltip(_("Change grid mode ( placement / full / off )"));
	} else {
		time_spent_on_some_button = 0;
	}

};				// void show_level_editor_tooltips ( void )

/**
 * Display the cursor in the leveleditor according to the currently
 * selected tool etc.
 */
static void display_cursor()
{
	struct leveleditor_widget *w;
	w = get_active_widget(GetMousePos_x(), GetMousePos_y());

	if (w) {
		if (w->type != WIDGET_MAP)
			blit_our_own_mouse_cursor();
		else
			leveleditor_map_display_cursor();
	}
}

void leveleditor_display()
{
	char linebuf[1000];

	AssembleCombatPicture(ONLY_SHOW_MAP_AND_TEXT | SHOW_GRID | SHOW_ITEMS | OMIT_TUX | GameConfig.omit_obstacles_in_level_editor *
			      OMIT_OBSTACLES | GameConfig.omit_enemies_in_level_editor * OMIT_ENEMIES | SHOW_OBSTACLE_NAMES | ZOOM_OUT *
			      GameConfig.zoom_is_on | OMIT_BLASTS | SKIP_LIGHT_RADIUS | NO_CURSOR);

	show_waypoints(ZOOM_OUT * GameConfig.zoom_is_on);
	show_map_labels(ZOOM_OUT * GameConfig.zoom_is_on);
	Highlight_Current_Block(ZOOM_OUT * GameConfig.zoom_is_on);
	gps_show();

	SetCurrentFont(FPS_Display_BFont);

	// Now we print out the current status directly onto the window:
	//
	if (OriginWaypoint != -1) {
		waypoint *wpts = EditLevel()->waypoints.arr;

		sprintf(linebuf, _(" Source waypoint selected : X=%d Y=%d. "), wpts[OriginWaypoint].x, wpts[OriginWaypoint].y);
		LeftPutString(Screen, GameConfig.screen_height - 2 * FontHeight(GetCurrentFont()), linebuf);
	}
	// Now we print out the latest connection operation success or failure...
	//
	if (VanishingMessageEndDate > SDL_GetTicks()) {
		DisplayText(VanishingMessage, 1, GameConfig.screen_height - 8 * FontHeight(GetCurrentFont()), NULL, 1.0);
	}

	show_level_editor_tooltips();

	leveleditor_display_widgets();

	if (EditLevel()->random_dungeon)
		DisplayText("This level is automatically generated. Editing will have no effect.", 10, GameConfig.screen_height / 2, NULL,
			    1.0);

	display_cursor();
	// Now that everything is blitted and printed, we may update the screen again...
	//
	our_SDL_flip_wrapper();

}
