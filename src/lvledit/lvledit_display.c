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

#define _leveleditor_display_c 1

#include "system.h"

#include "defs.h"
#include "struct.h"
#include "global.h"
#include "proto.h"

#include "lvledit/lvledit.h"
#include "lvledit/lvledit_actions.h"
#include "lvledit/lvledit_widgets.h"

struct image level_editor_waypoint_cursor[2] = { EMPTY_IMAGE, EMPTY_IMAGE };

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
	lvledit_zoom_factor = zf;
	lvledit_zoom_factor_inv = 1.0 / zf;
	return 0;
}

/**
 * Displays the GPS in the editor
 */
static void gps_show() {
	static char gps_text[200];
	// TRANSLATORS: Used to display a GPS position (X=10.5 Y=34.3 L=12 layer=1)
	snprintf(gps_text, sizeof(gps_text) - 1, _(" X=%3.1f Y=%3.1f L=%d layer=%d\n"), Me.pos.x, Me.pos.y, Me.pos.z, current_floor_layer);
	display_text(gps_text, User_Rect.x + 1, GameConfig.screen_height - 1 * get_font_height(get_current_font()), NULL /*&User_Rect */ , 1.0);
}

/**
 * Displays counted FPS right above GPS
 */
void lvledit_display_fps(void) {
	static char fps_text[200];
	sprintf(fps_text, _(" FPS=%d\n"), get_current_fps());
	display_text(fps_text, User_Rect.x + 1,	
		GameConfig.screen_height - 2 * get_font_height(get_current_font()), NULL, 1.0);
}

/**
 * Now we print out the map label information about this map location.
 */
static void print_label_information(struct level *edit_level)
{
	struct map_label *m = get_map_label_from_coords(edit_level, Me.pos.x, Me.pos.y);

	if (m) {
		char panel_text[5000] = "";
		// Create the map label information
		sprintf(panel_text, _("\n Map label information: \n label_name=\"%s\"."), m->label_name);

		// Display the map label information on the screen
		display_text(panel_text, User_Rect.x, GameConfig.screen_height - 5 * get_font_height(get_current_font()), NULL, 1.0);

		return;
	}
}

static void show_cursor(int must_zoom)
{
	static struct image level_editor_cursor = EMPTY_IMAGE;

#define HIGHLIGHTCOLOR 255

	// Maybe, if the level editor floor cursor has not yet been loaded,
	// we need to load it.
	//
	if (!image_loaded(&level_editor_cursor)) {
		load_image(&level_editor_cursor, GUI_DIR, "level_editor/floor_cursor.png", USE_OFFSET);
	}

	float scale = must_zoom ? lvledit_zoomfact_inv() : 1.0;

	display_image_on_map(&level_editor_cursor, Me.pos.x, Me.pos.y, IMAGE_SCALE_TRANSFO(scale)); 
	print_label_information(EditLevel());
}

/**
 * This function is used to draw a line between given map tiles.  It is
 * mainly used for the map editor to highlight connections and the 
 * current map tile target.
 */
void draw_connection_between_tiles(float x1, float y1, float x2, float y2, int mask, int rspawn)
{
	float steps;
	float dist;
	int i;
	static struct image level_editor_dot_cursor = EMPTY_IMAGE;

	float scale = (mask & ZOOM_OUT) ? lvledit_zoomfact_inv() : 1.0;

	// Maybe, if the level editor dot cursor has not yet been loaded,
	// we need to load it.
	//
	if (!image_loaded(&level_editor_dot_cursor)) {
		load_image(&level_editor_dot_cursor, GUI_DIR, "level_editor/waypoint_dot.png", USE_OFFSET);
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
		float r, g, b;
		x = (((float)i) / steps) * x1 + x2 * (steps - i) / steps;
		y = (((float)i) / steps) * y1 + y2 * (steps - i) / steps;

		r = 1.0;
		if (rspawn)
			g = b = 0;
		else
			g = b = 1.0;

		display_image_on_map(&level_editor_dot_cursor, x, y, IMAGE_SCALE_RGB_TRANSFO(scale, r, g, b));

	}

};				// void draw_connection_between_tiles ( .... )

/**
 * This function is used by thelevel *Editor integrated into 
 * FreedroidRPG.  It marks all waypoints with a cross.
 */
static void show_waypoints(int mask)
{
	float zf = (mask & ZOOM_OUT) ? lvledit_zoomfact_inv() : 1.0;
	waypoint *wpts = EditLevel()->waypoints.arr;
	float r, g, b;
	float x, y;
	int i, j;
	int x_min, x_max, y_min, y_max;

	get_floor_boundaries(mask, &y_min, &y_max, &x_min, &x_max);

	// Maybe, if the level editor floor cursor has not yet been loaded,
	// we need to load it.
	if (!image_loaded(&level_editor_waypoint_cursor[0])) {
		load_image(&level_editor_waypoint_cursor[0], GUI_DIR, "level_editor/waypoint_cursor.png", USE_OFFSET);
	}

	if (!image_loaded(&level_editor_waypoint_cursor[1])) {
		load_image(&level_editor_waypoint_cursor[1], GUI_DIR, "level_editor/norand_waypoint_cursor.png", USE_OFFSET);
	}

	for (i = 0; i < EditLevel()->waypoints.size; i++) {
		// Skip waypoints that are not visible
		if (wpts[i].x <= x_min || wpts[i].x >= x_max)
			continue;
		if (wpts[i].y <= y_min || wpts[i].y >= y_max)
			continue;

		// Calculate the position of the waypoint
		x = wpts[i].x + 0.5;
		y = wpts[i].y + 0.5;

		// Apply disco mode when current waypoint is selected
		object_vtx_color(&wpts[i], &r, &g, &b);

		struct image *img = &level_editor_waypoint_cursor[wpts[i].suppress_random_spawn];
		display_image_on_map(img, x, y, IMAGE_SCALE_RGB_TRANSFO(zf, r, g, b));

		// Get the connections of the waypoint
		int *connections = wpts[i].connections.arr;

		for (j = 0; j < wpts[i].connections.size; j++) {
			// Check validity of the connected waypoint
			int connected_waypoint = connections[j];
			if (connected_waypoint < 0 || connected_waypoint >= EditLevel()->waypoints.size)
				continue;

			waypoint *to_wp = &wpts[connected_waypoint];

			if (((EditX() == wpts[i].x) && (EditY() == wpts[i].y)) || GameConfig.show_wp_connections) {
				draw_connection_between_tiles(x, y, to_wp->x + 0.5, to_wp->y + 0.5, mask, wpts[i].suppress_random_spawn);
			}
		}
	}

	// Now we do something extra:  If there is a connection attempt currently
	// going on, then we also draw a connection from the origin point to the
	// current cursor (i.e. 'me') position.
	if (OriginWaypoint != (-1)) {
		// Draw the connection between the origin waypoint and the cursor (ie. 'me')
		draw_connection_between_tiles(wpts[OriginWaypoint].x + 0.5, wpts[OriginWaypoint].y + 0.5, Me.pos.x, Me.pos.y,
											mask, wpts[OriginWaypoint].suppress_random_spawn);
	}
}

/**
 * This function is used by thelevel *Editor integrated into 
 * FreedroidRPG.  It marks all places that have a label attached to them.
 */
static void show_map_labels(int must_zoom)
{
	struct level *edit_level = curShip.AllLevels[Me.pos.z];
	int i;
	float r, g, b;
	float scale = must_zoom ? lvledit_zoomfact_inv() : 1.0;
	int x_min, x_max, y_min, y_max;

	get_floor_boundaries(must_zoom, &y_min, &y_max, &x_min, &x_max);

	// Now we can draw a fine indicator at all the necessary positions ...
	for (i = 0; i < edit_level->map_labels.size; i++) {
		// Get the map label
		struct map_label *map_label = &ACCESS_MAP_LABEL(edit_level->map_labels, i);

		// Apply disco mode when the current map label is selected
		object_vtx_color(map_label, &r, &g, &b);

		// Skip map labels that are not visible
		if (map_label->pos.x < x_min || map_label->pos.x > x_max)
			continue;
		if (map_label->pos.y < y_min || map_label->pos.y > y_max)
			continue;

		struct image *img = get_map_label_image();
		display_image_on_map(img, map_label->pos.x + 0.5, map_label->pos.y + 0.5, IMAGE_SCALE_RGB_TRANSFO(scale, r, g, b));

                if (!GameConfig.omit_map_labels_in_level_editor) {
                    show_backgrounded_label_at_map_position(map_label->label_name,
                                                            0, map_label->pos.x,
                                                            map_label->pos.y, must_zoom);
                }
	}
}

/**
 * Display the cursor in the leveleditor according to the currently
 * selected tool etc.
 */
static void display_cursor()
{
	struct widget *w;
	w = get_active_widget(GetMousePos_x(), GetMousePos_y());

	if (w) {
		if (w->type != WIDGET_MAP)
			blit_mouse_cursor();
		else
			widget_lvledit_map_display_cursor();
	}
}

void leveleditor_display()
{
	AssembleCombatPicture(ONLY_SHOW_MAP_AND_TEXT | SHOW_ITEMS | OMIT_TUX | GameConfig.omit_obstacles_in_level_editor *
			      OMIT_OBSTACLES | GameConfig.omit_enemies_in_level_editor * OMIT_ENEMIES | ZOOM_OUT *
			      GameConfig.zoom_is_on | OMIT_BLASTS | SKIP_LIGHT_RADIUS | NO_CURSOR | OMIT_ITEMS_LABEL);

	start_image_batch();
	show_waypoints(ZOOM_OUT * GameConfig.zoom_is_on);
	show_map_labels(ZOOM_OUT * GameConfig.zoom_is_on);
	end_image_batch();
	show_cursor(ZOOM_OUT * GameConfig.zoom_is_on);
	gps_show();

	if (GameConfig.Draw_Framerate)
		lvledit_display_fps();

	set_current_font(FPS_Display_Font);

	// Now we print out the current status directly onto the window:
	//
	if (OriginWaypoint != -1) {
		waypoint *wpts = EditLevel()->waypoints.arr;
		char linebuf[1000];
		sprintf(linebuf, _(" Source waypoint selected : X=%d Y=%d. "), wpts[OriginWaypoint].x, wpts[OriginWaypoint].y);
		put_string_left(FPS_Display_Font, GameConfig.screen_height - 2 * get_font_height(get_current_font()), linebuf);
	}
	// Now we print out the latest connection operation success or failure...
	//
	if (VanishingMessageEndDate > SDL_GetTicks()) {
		display_text(VanishingMessage, 1, GameConfig.screen_height - 8 * get_font_height(get_current_font()), NULL, 1.0);
	} else if (EditLevel()->random_dungeon) {
		display_text(_(" This level is automatically generated. \n Editing will have no effect."), 1, GameConfig.screen_height - 8 * get_font_height(get_current_font()), NULL, 1.0);
	}

	// Construct the linked list of visible levels.
	get_visible_levels();

	display_widgets();

	display_cursor();
	// Now that everything is blitted and printed, we may update the screen again...
	//
	our_SDL_flip_wrapper();

}
