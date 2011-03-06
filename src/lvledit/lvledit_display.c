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
	display_text_using_line_height(gps_text, User_Rect.x + 1, GameConfig.screen_height - 1 * FontHeight(GetCurrentFont()), NULL /*&User_Rect */ , 1.0);
}

/**
 * Now we print out the map label information about this map location.
 */
static void print_label_information(level *EditLevel)
{
	char PanelText[5000] = "";
	struct map_label *map_label;
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
			display_text_using_line_height(PanelText, User_Rect.x, GameConfig.screen_height - 5 * FontHeight(GetCurrentFont()), NULL, 1.0);

			return;
		}
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
		load_image(&level_editor_cursor, "level_editor_floor_cursor.png", TRUE);
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
void draw_connection_between_tiles(float x1, float y1, float x2, float y2, int mask)
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
		load_image(&level_editor_dot_cursor, "level_editor_waypoint_dot.png", TRUE);
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

		display_image_on_map(&level_editor_dot_cursor, x, y, IMAGE_SCALE_TRANSFO(scale));
	}

};				// void draw_connection_between_tiles ( .... )

/**
 * This function is used by thelevel *Editor integrated into 
 * freedroid.  It marks all waypoints with a cross.
 */
static void show_waypoints(int mask)
{
	float zf = (mask & ZOOM_OUT) ? lvledit_zoomfact_inv() : 1.0;
	waypoint *wpts = EditLevel()->waypoints.arr;
	float r, g, b;
	float x, y;
	int i, j;

	// Maybe, if the level editor floor cursor has not yet been loaded,
	// we need to load it.
	if (!image_loaded(&level_editor_waypoint_cursor[0])) {
		load_image(&level_editor_waypoint_cursor[0], "level_editor_waypoint_cursor.png", TRUE);
	}

	if (!image_loaded(&level_editor_waypoint_cursor[1])) {
		load_image(&level_editor_waypoint_cursor[1], "level_editor_norand_waypoint_cursor.png", TRUE);
	}

	for (i = 0; i < EditLevel()->waypoints.size; i++) {
		// Calculate the position of the waypoint
		x = wpts[i].x + 0.5;
		y = wpts[i].y + 0.5;

		// Apply disco mode when current waypoint is selected
		object_vtx_color(&wpts[i], &r, &g, &b);

		struct image *img = &level_editor_waypoint_cursor[wpts[i].suppress_random_spawn];
		display_image_on_map(img, x, y, set_image_transformation(zf, r, g, b, 1.0, 0));

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
static void show_map_labels(int must_zoom)
{
	static struct image map_label_indicator = EMPTY_IMAGE;
	level *EditLevel = curShip.AllLevels[Me.pos.z];
	struct map_label *map_label;
	int i;
	float scale = must_zoom ? lvledit_zoomfact_inv() : 1.0;

	// On the first function call to this function, we must load the map label indicator
	// iso image from the disk to memory and keep it there as static.  That should be
	// it for here.
	if (!image_loaded(&map_label_indicator)) {
		load_image(&map_label_indicator, "level_editor_map_label_indicator.png", TRUE);
	}

	// Now we can draw a fine indicator at all the necessary positions ...
	for (i = 0; i < EditLevel->map_labels.size; i++) {	
		// Get the map label
		map_label = &ACCESS_MAP_LABEL(EditLevel->map_labels, i);

		display_image_on_map(&map_label_indicator, map_label->pos.x + 0.5, map_label->pos.y + 0.5, IMAGE_SCALE_TRANSFO(scale));
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
 * Display the tooltips of the buttons on the screen
 */
static void show_tooltips()
{
	static float time_spent_on_some_button = 0;
	static float previous_function_call_time = 0;

	time_spent_on_some_button += SDL_GetTicks() - previous_function_call_time;

	previous_function_call_time = SDL_GetTicks();

	if (!GameConfig.show_tooltips)
		return;

#define TICKS_UNTIL_TOOLTIP 1200
	if (time_spent_on_some_button <= TICKS_UNTIL_TOOLTIP)
		return;

	struct leveleditor_widget *w = get_active_widget(GetMousePos_x(), GetMousePos_y());
	if (!w)
		return;

	if (w->type != WIDGET_BUTTON) {
		// If the active widget is not a button,
		// reset the tooltip timer
		time_spent_on_some_button = 0;
		return;
	}

	struct leveleditor_button *b = w->ext;
	if (b->tooltip) {
		// The button has a tooltip, display it
		show_button_tooltip(b->tooltip);
	}
}

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
			blit_mouse_cursor();
		else
			leveleditor_map_display_cursor();
	}
}

void leveleditor_display()
{
	char linebuf[1000];

	AssembleCombatPicture(ONLY_SHOW_MAP_AND_TEXT | SHOW_GRID | SHOW_ITEMS | OMIT_TUX | GameConfig.omit_obstacles_in_level_editor *
			      OMIT_OBSTACLES | GameConfig.omit_enemies_in_level_editor * OMIT_ENEMIES | ZOOM_OUT *
			      GameConfig.zoom_is_on | OMIT_BLASTS | SKIP_LIGHT_RADIUS | NO_CURSOR | OMIT_ITEMS_LABEL);

	show_waypoints(ZOOM_OUT * GameConfig.zoom_is_on);
	show_map_labels(ZOOM_OUT * GameConfig.zoom_is_on);
	show_cursor(ZOOM_OUT * GameConfig.zoom_is_on);
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
		display_text_using_line_height(VanishingMessage, 1, GameConfig.screen_height - 8 * FontHeight(GetCurrentFont()), NULL, 1.0);
	}

	show_tooltips();

	// Construct the linked list of visible levels.
	get_visible_levels();

	leveleditor_display_widgets();

	if (EditLevel()->random_dungeon) {
		sprintf(VanishingMessage, " This level is automatically generated. \n Editing will have no effect.");
		VanishingMessageEndDate = SDL_GetTicks() + 100;
	}

	display_cursor();
	// Now that everything is blitted and printed, we may update the screen again...
	//
	our_SDL_flip_wrapper();

}
