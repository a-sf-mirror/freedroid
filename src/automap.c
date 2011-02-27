
/* 
 *   Copyright (c) 2004-2010 Arthur Huillet
 *   Copyright (c) 2004 Johannes Prix
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
 */

/**
 * This file contains everything that has got to do with the automap.
 */

#include "system.h"

#include "defs.h"
#include "struct.h"
#include "global.h"
#include "proto.h"
#include "map.h"

int AUTOMAP_TEXTURE_WIDTH = 2048;
int AUTOMAP_TEXTURE_HEIGHT = 1024;

#define AUTOMAP_SQUARE_SIZE 3
#define WALL_COLOR 0x00, 0xFF, 0xFF
#define TUX_COLOR 0x00, 0x00, 0xFF
#define FRIEND_COLOR 0x00, 0xFF, 0x00

static struct image compass;

/**
 * This function clears out the Automap data.
 */
void ClearAutomapData(void)
{
	memset(Me.Automap, 0, MAX_LEVELS * 100 * 100);

};				// void ClearAutomapData ( void )

/**
 * This function does a special rounding that is used by the automap
 * to compute the start and end position of an obstacle on the map.
 * Its key property is that it preserves the distance between the two
 * numbers, which is important in order for a 1 tile wall to appear
 * as occupying only one tile on the automap regardless of its exact position.
 */
static void round_automap_pos(float a, float b, int *ra, int *rb)
{
	float delta;
	int dist;
#define AUTOMAP_COLLRECT_SHRINK 0.9

	if (a > b) {
		float c = a;
		a = b;
		b = c;
	}

	delta = (b - a) * AUTOMAP_COLLRECT_SHRINK;
	dist = floor(delta);

	*ra = lrint(a);
	*rb = *ra + dist;
}

/**
 * This function collects the automap data and stores it in the Me data
 * structure.
 */
void CollectAutomapData(void)
{
	int x, y;
	int start_x, start_y, end_x, end_y;
	gps ObjPos;
	static int TimePassed;
	Level automap_level = curShip.AllLevels[Me.pos.z];
	int i;
	obstacle *our_obstacle;
	int level = Me.pos.z;

	ObjPos.z = Me.pos.z;

	// Checking the whole map for passability will surely take a lot
	// of computation.  Therefore we only do this once every second of
	// real time.
	//
	if (TimePassed == (int)Me.MissionTimeElapsed)
		return;
	TimePassed = (int)Me.MissionTimeElapsed;

	// DebugPrintf ( -3 , "\nCollecting Automap data... " );

	// Also if there is no map-maker present in inventory, then we need not
	// do a thing here...
	//
	if (!Me.map_maker_is_present)
		return;

	// Earlier we had
	//
	// start_x = 0 ; start_y = 0 ; end_x = automap_level->xlen ; end_y = automap_level->ylen ;
	//
	// when maximal automap was generated.  Now we only add to the automap what really is on screen...
	//
	start_x = Me.pos.x - FLOOR_TILES_VISIBLE_AROUND_TUX;
	end_x = Me.pos.x + FLOOR_TILES_VISIBLE_AROUND_TUX;
	start_y = Me.pos.y - FLOOR_TILES_VISIBLE_AROUND_TUX;
	end_y = Me.pos.y + FLOOR_TILES_VISIBLE_AROUND_TUX;

	if (start_x < 0)
		start_x = 0;
	if (end_x >= automap_level->xlen)
		end_x = automap_level->xlen;
	if (start_y < 0)
		start_y = 0;
	if (end_y >= automap_level->ylen)
		end_y = automap_level->ylen;
	
	int tstamp = next_glue_timestamp();

	// Now we do the actual checking for visible wall components.
	//
	for (y = start_y; y < end_y; y++) {
		for (x = start_x; x < end_x; x++) {
			if (Me.Automap[level][y][x] & SQUARE_SEEN_AT_ALL_BIT)
				continue;

			for (i = 0; i < automap_level->map[y][x].glued_obstacles.size; i++) {
				int idx = ((int *)(automap_level->map[y][x].glued_obstacles.arr))[i];

				our_obstacle = &(automap_level->obstacle_list[idx]);

				if (our_obstacle->timestamp == tstamp)
					continue;

				our_obstacle->timestamp = tstamp;

				if ((our_obstacle->type >= ISO_H_DOOR_000_OPEN) && (our_obstacle->type <= ISO_V_DOOR_100_OPEN))
					continue;
				if ((our_obstacle->type >= ISO_DH_DOOR_000_OPEN) && (our_obstacle->type <= ISO_DV_DOOR_100_OPEN))
					continue;
				if ((our_obstacle->type >= ISO_OUTER_DOOR_V_00) && (our_obstacle->type <= ISO_OUTER_DOOR_H_100))
					continue;

				int obstacle_start_x;
				int obstacle_end_x;
				int obstacle_start_y;
				int obstacle_end_y;

				round_automap_pos(our_obstacle->pos.x + obstacle_map[our_obstacle->type].left_border, our_obstacle->pos.x + obstacle_map[our_obstacle->type].right_border,
						&obstacle_start_x, &obstacle_end_x);

				round_automap_pos(our_obstacle->pos.y + obstacle_map[our_obstacle->type].upper_border, our_obstacle->pos.y + obstacle_map[our_obstacle->type].lower_border,
						&obstacle_start_y, &obstacle_end_y);

				if (obstacle_start_x < 0)
					obstacle_start_x = 0;
				if (obstacle_start_y < 0)
					obstacle_start_y = 0;
				if (obstacle_end_x < 0)
					obstacle_end_x = 0;
				if (obstacle_end_y < 0)
					obstacle_end_y = 0;
				if (obstacle_start_x >= automap_level->xlen)
					obstacle_start_x = automap_level->xlen - 1;
				if (obstacle_start_y >= automap_level->ylen)
					obstacle_start_y = automap_level->ylen - 1;
				if (obstacle_end_x >= automap_level->xlen)
					obstacle_end_x = automap_level->xlen - 1;
				if (obstacle_end_y >= automap_level->ylen)
					obstacle_end_y = automap_level->ylen - 1;

				//printf("pos %f %f - border %f %f to %f %f - xstart %d xend %d ystart %d yend %d\n", our_obstacle->pos.x, our_obstacle->pos.y, our_obstacle->pos.x + obstacle_map [ our_obstacle -> type ] . left_border, our_obstacle->pos.y + obstacle_map [ our_obstacle -> type ] . upper_border, our_obstacle->pos.x + obstacle_map [ our_obstacle -> type ] . right_border, our_obstacle->pos.y + obstacle_map [ our_obstacle -> type ] . lower_border, obstacle_start_x, obstacle_end_x, obstacle_start_y, obstacle_end_y);

				int a, b;

				for (a = obstacle_start_x; a <=  obstacle_end_x; a++) {
					for (b = obstacle_start_y; b <=  obstacle_end_y; b++) {

						if (obstacle_map[our_obstacle->type].block_area_type == COLLISION_TYPE_RECTANGLE) {

							if (obstacle_map[our_obstacle->type].block_area_parm_1 > 0.80) {
								if (Me.pos.x < our_obstacle->pos.x)
									Me.Automap[level][b][a] |= LEFT_WALL_BIT;
								else
									Me.Automap[level][b][a] |= RIGHT_WALL_BIT;
							}
							if (obstacle_map[our_obstacle->type].block_area_parm_2 > 0.80) {
								if (Me.pos.y < our_obstacle->pos.y)
									Me.Automap[level][b][a] |= UP_WALL_BIT;
								else
									Me.Automap[level][b][a] |= DOWN_WALL_BIT;
							}
						}
					}
				}
			}

			Me.Automap[level][y][x] = Me.Automap[level][y][x] | SQUARE_SEEN_AT_ALL_BIT;
		}
	}

};				// void CollectAutomapData ( void )


/**
 * Use GL_POINTS primitive in OpenGL mode to render automap points for better performance.
 * In SDL mode, fallback to PutPixel.
 */
static inline void PutPixel_automap_wrapper(SDL_Surface * abc, int x, int y, int r, int g, int b)
{
	if (!use_open_gl)
		pixelRGBA(abc, x, y, r, g, b, 255);
#ifdef HAVE_LIBGL
	else {
		glColor3ub(r, g, b);
		glVertex2i(x, y);
		glColor3ub(255, 255, 255);
	}
#endif
}

/**
 * Toggle automap visibility
 */
void toggle_automap(void)
{
	GameConfig.Automap_Visible = !GameConfig.Automap_Visible;

	if (!Me.map_maker_is_present) {
		if (GameConfig.Automap_Visible)
			append_new_game_message(_("Compass on. You need a Map Maker to enable the minimap."));
		else
			append_new_game_message(_("Compass off."));
	}
}

static void display_automap_compass()
{
	//load the compass if necessary
	if (!image_loaded(&compass)) {
		load_image(&compass, "compass.png", FALSE);
	}

	display_image_on_screen(&compass, GameConfig.screen_width - compass.w - 10, 40);
}

/**
 *
 * This function should display the automap.
 * 
 * In this case the function only uses pixel operations with the screen.
 * This is the method of choice when using SDL for graphics output.
 * For the OpenGL case, there is a completely different function...
 *
 */
void display_automap(void)
{
	int x, y;
	Level automap_level = curShip.AllLevels[Me.pos.z];
	int level = Me.pos.z;

	// Of course we only display the automap on demand of the user...
	//
	if (GameConfig.Automap_Visible == FALSE)
		return;

	// Display the compass
	display_automap_compass();

	// Also if there is no map-maker present in inventory, then we need not
	// do a thing here...
	//
	if (!Me.map_maker_is_present)
		return;

#ifdef HAVE_LIBGL
	if (use_open_gl) {
		glDisable(GL_TEXTURE_2D);
		glBegin(GL_POINTS);
	}
#endif

	// At first, we only blit the known data about the pure wall-type
	// obstacles on this level.
	for (y = 0; y < automap_level->ylen; y++) {
		for (x = 0; x < automap_level->xlen; x++) {
			if (Me.Automap[level][y][x] & (RIGHT_WALL_BIT | LEFT_WALL_BIT)) {
				PutPixel_automap_wrapper(Screen,
							 1 + AUTOMAP_SQUARE_SIZE * x + AUTOMAP_SQUARE_SIZE * (automap_level->ylen - y),
							 1 + AUTOMAP_SQUARE_SIZE * x + AUTOMAP_SQUARE_SIZE * y, WALL_COLOR);
				PutPixel_automap_wrapper(Screen,
							 2 + AUTOMAP_SQUARE_SIZE * x + AUTOMAP_SQUARE_SIZE * (automap_level->ylen - y),
							 2 + AUTOMAP_SQUARE_SIZE * x + AUTOMAP_SQUARE_SIZE * y, WALL_COLOR);
			}

			if (Me.Automap[level][y][x] & (UP_WALL_BIT | DOWN_WALL_BIT)) {
				PutPixel_automap_wrapper(Screen,
							 1 + AUTOMAP_SQUARE_SIZE * x + AUTOMAP_SQUARE_SIZE * (automap_level->ylen - y),
							 2 + AUTOMAP_SQUARE_SIZE * x + AUTOMAP_SQUARE_SIZE * y, WALL_COLOR);
				PutPixel_automap_wrapper(Screen,
							 2 + AUTOMAP_SQUARE_SIZE * x + AUTOMAP_SQUARE_SIZE * (automap_level->ylen - y),
							 1 + AUTOMAP_SQUARE_SIZE * x + AUTOMAP_SQUARE_SIZE * y, WALL_COLOR);
			}
		}
	}

	// Display enemies
	enemy *erot;
	BROWSE_LEVEL_BOTS(erot, automap_level->levelnum) {
		if (erot->type == (-1))
			continue;

		for (x = 0; x < AUTOMAP_SQUARE_SIZE; x++) {
			for (y = 0; y < AUTOMAP_SQUARE_SIZE; y++) {
				if (is_friendly(erot->faction, FACTION_SELF)) {
					PutPixel_automap_wrapper(Screen,
								 AUTOMAP_SQUARE_SIZE * erot->pos.x +
								 AUTOMAP_SQUARE_SIZE * (automap_level->ylen - erot->pos.y) + x,
								 AUTOMAP_SQUARE_SIZE * erot->pos.x + AUTOMAP_SQUARE_SIZE * erot->pos.y + y,
								 FRIEND_COLOR);
				}
			}
		}
	}

	// Display tux
	for (x = 0; x < AUTOMAP_SQUARE_SIZE; x++) {
		for (y = 0; y < AUTOMAP_SQUARE_SIZE; y++) {
			PutPixel_automap_wrapper(Screen,
						 AUTOMAP_SQUARE_SIZE * Me.pos.x + AUTOMAP_SQUARE_SIZE * (automap_level->ylen - Me.pos.y) +
						 x, AUTOMAP_SQUARE_SIZE * Me.pos.x + AUTOMAP_SQUARE_SIZE * Me.pos.y + y, TUX_COLOR);
		}
	}

#ifdef HAVE_LIBGL
	if (use_open_gl) {
		glEnd();
		glEnable(GL_TEXTURE_2D);
	}
#endif

};				// void show_automap_data_sdl ( void )
