/* 
 *
 *   Copyright (c) 2010, Alexander Solovets
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
 * */
#include "system.h"

#include "defs.h"
#include "struct.h"
#include "../src/global.h"
#include "proto.h"

#include "mapgen/mapgen.h"
#include "mapgen/themes.h"

#define RAND_THEME(t)	t[MyRandom(sizeof(t) / sizeof(t[0]) - 1)]
#define OBSTACLE_DIM_X(x)	ceil(get_obstacle_spec(x)->right_border - get_obstacle_spec(x)->left_border)
#define OBSTACLE_DIM_Y(x)	ceil(get_obstacle_spec(x)->lower_border - get_obstacle_spec(x)->upper_border)

static void apply_default_theme(int, int, int);
static void apply_metal_theme(int, int, int);
static void apply_glass_theme(int, int, int);
static void apply_red_theme(int, int, int);
static void apply_green_theme(int, int, int);
static void apply_flower_theme(int, int, int);

const struct theme_info theme_data[] = {
	{ ISO_V_WALL, ISO_H_WALL, ISO_V_WALL, ISO_H_WALL, ISO_GREY_WALL_END_N, ISO_GREY_WALL_END_W },
	{ ISO_GREY_WALL_END_E, ISO_GREY_WALL_END_S, ISO_GREY_WALL_END_E, ISO_GREY_WALL_END_S, ISO_GREY_WALL_END_N, ISO_GREY_WALL_END_W },
	{ ISO_GLASS_WALL_1, ISO_GLASS_WALL_2, ISO_GLASS_WALL_1, ISO_GLASS_WALL_2 },
	{ ISO_ROOM_WALL_V_RED, ISO_ROOM_WALL_H_RED, ISO_V_WALL, ISO_H_WALL, ISO_RED_WALL_WINDOW_2, ISO_RED_WALL_WINDOW_1,
		{ ISO_FLOOR_HOUSE_FLOOR, ISO_CARPET_TILE_0004 }
	},
	{ ISO_BROKEN_GLASS_WALL_1, ISO_GLASS_WALL_2, ISO_GLASS_WALL_1, ISO_GLASS_WALL_2 },
	{ ISO_LIGHT_GREEN_WALL_1, ISO_LIGHT_GREEN_WALL_2, ISO_V_WALL, ISO_H_WALL, ISO_FLOWER_WALL_WINDOW_2, ISO_FLOWER_WALL_WINDOW_1,
		{ ISO_CARPET_TILE_0002, ISO_COMPLICATED_PMM }
	},
	{ ISO_FUNKY_WALL_1, ISO_FUNKY_WALL_2, ISO_V_WALL, ISO_H_WALL, ISO_FLOWER_WALL_WINDOW_2, ISO_FLOWER_WALL_WINDOW_1,
		{ ISO_TWOSQUARE_0001, ISO_TWOSQUARE_0003 }
	},
};

typedef void (*theme_proc)(int, int, int);
const theme_proc themes[] = {
	apply_default_theme,
	apply_metal_theme,
	apply_glass_theme,
	apply_red_theme,
	apply_glass_theme,
	apply_green_theme,
	apply_flower_theme
};

// A list of themes are used to specify basic parameters for a room
// such as wall and floor tiles that may be partly omitted in others themes.
const enum theme basic_themes[] = {
	THEME_METAL,
	THEME_GRAY,
	THEME_RED,
	THEME_GREEN
};

const enum theme living_themes[]		= { THEME_RED, THEME_GREEN, THEME_FLOWER };
const enum theme industrial_themes[]	= { THEME_METAL, THEME_GRAY };

static int set_generic_wall(int x, int y, int wall, int theme)
{
	// A value '1' of 'processed' means that data was processed, otherwise
	// the caller should process them itself.
	int processed = 1;
	int obstacle;
	int room = mapgen_get_room(x, y);
	int period = room != -1 ? rooms[room].period : 0;
	if (wall & WALL_PART) {
		if (wall & WALL_N) {
			obstacle = theme_data[theme].wall_n;
			if (period && !(x % period))
				obstacle = theme_data[theme].window_wall_h;
			mapgen_add_obstacle(x + 0.5, y, obstacle);
		} else if (wall & WALL_S) {
			obstacle = theme_data[theme].wall_n;
			if (period && !(x % period))
				obstacle = theme_data[theme].window_wall_h;
			mapgen_add_obstacle(x + 0.5, y + 1, obstacle);
		} else if (wall & WALL_W) {
			obstacle = theme_data[theme].wall_w;
			if (period && !(y % period))
				obstacle = theme_data[theme].window_wall_v;
			mapgen_add_obstacle(x, y + 0.5, obstacle);
		} else if (wall & WALL_E) {
			obstacle = theme_data[theme].wall_w;
			if (period && !(y % period))
				obstacle = theme_data[theme].window_wall_v;
			mapgen_add_obstacle(x + 1, y + 0.5, obstacle);
		} else {
			processed = 0;
		}
	} else {
		switch (wall) {
			case 0:
				break;
			case WALL_N:
				mapgen_add_obstacle(x + 0.5, y, theme_data[theme].wall_n);
				break;
			case WALL_S:
				mapgen_add_obstacle(x + 0.5, y + 1, theme_data[theme].wall_s);
				break;
			case WALL_W:
				mapgen_add_obstacle(x, y + 0.5, theme_data[theme].wall_w);
				break;
			case WALL_E:
				mapgen_add_obstacle(x + 1, y + 0.5, theme_data[theme].wall_e);
				break;
			default:
				processed = 0;
		}
	}

	return processed;
}

static void set_simple_wall(int x, int y, int wall, int theme)
{
	if (set_generic_wall(x, y, wall, theme)) return;
	switch (wall) {
		case WALL_NW:
			mapgen_add_obstacle(x + 0.5, y, theme_data[theme].wall_n);
			mapgen_add_obstacle(x, y + 0.5, theme_data[theme].wall_w);
			break;
		case WALL_NE:
			mapgen_add_obstacle(x + 0.5, y, theme_data[theme].wall_n);
			mapgen_add_obstacle(x + 1, y + 0.5, theme_data[theme].wall_e);
			break;
		case WALL_SW:
			mapgen_add_obstacle(x + 0.5, y + 1, theme_data[theme].wall_s);
			mapgen_add_obstacle(x, y + 0.5, theme_data[theme].wall_w);
			break;
		case WALL_SE:
			mapgen_add_obstacle(x + 0.5, y + 1, theme_data[theme].wall_s);
			mapgen_add_obstacle(x + 1, y + 0.5, theme_data[theme].wall_e);
			break;
	}
}

static void apply_default_theme(int x, int y, int object)
{
	set_simple_wall(x, y, object, THEME_METAL);
	mapgen_set_floor(x, y, ISO_FLOOR_ERROR_TILE);
}

static void apply_metal_theme(int x, int y, int object)
{
	set_simple_wall(x, y, object, THEME_GRAY);
	mapgen_set_floor(x, y, ISO_FLOOR_STONE_FLOOR);
}

static void apply_glass_theme(int x, int y, int object)
{
	int theme = object == WALL_W && MyRandom(100) < 15 ? THEME_BROKEN_GLASS : THEME_GLASS;
	set_simple_wall(x, y, object, theme);
	mapgen_set_floor(x, y, ISO_MINI_SQUARE_0003);
}

static void apply_red_theme(int x, int y, int object)
{
	set_simple_wall(x, y, object, THEME_RED);
	mapgen_set_floor(x, y, ISO_CARPET_TILE_0004);
} 

static void apply_green_theme(int x, int y, int object)
{
	set_simple_wall(x, y, object, THEME_GREEN);
	mapgen_set_floor(x, y, ISO_CARPET_TILE_0002);
} 

static void apply_flower_theme(int x, int y, int object)
{
	set_simple_wall(x, y, object, THEME_FLOWER);
	mapgen_set_floor(x, y, ISO_CARPET_TILE_0002);
}

static void fill_armory(int r)
{
	int x, y;
	struct roominfo	*room = &rooms[r];

#define ARMORY_PROB		90

	// Place gifts alongside the wall that has greater dimension
	if (room->w > room->h) {
		for (y = 1; y < 3; y++) {
			for (x = 1; x < room->w; x++) {
				if (mapgen_get_tile(room->x + x, room->y - 1) == TILE_WALL && MyRandom(100) < ARMORY_PROB)
					mapgen_add_obstacle(room->x + x, room->y + y, ISO_BARREL_1 + MyRandom(3));
				if (mapgen_get_tile(room->x + x, room->y + room->h) == TILE_WALL && MyRandom(100) < ARMORY_PROB)
					mapgen_add_obstacle(room->x + x, room->y + room->h - y, ISO_BARREL_1 + MyRandom(3));
			}
		}
	} else {
		for (y = 1; y < room->h; y++) {
			for (x = 1; x < 3; x++) {
				if (mapgen_get_tile(room->x - 1, room->y + y) == TILE_WALL && MyRandom(100) < ARMORY_PROB)
					mapgen_add_obstacle(room->x + x, room->y + y, ISO_BARREL_1 + MyRandom(3));
				if (mapgen_get_tile(room->x + room->w, room->y + y) == TILE_WALL && MyRandom(100) < ARMORY_PROB)
					mapgen_add_obstacle(room->x + room->w - x, room->y + y, ISO_BARREL_1 + MyRandom(3));
			}
		}
	}
}

static void place_library(int room)
{
	struct roominfo *ri = &rooms[room];
	int x1 = ri->x;
	int y1 = ri->y;
	int x2 = x1 + ri->w - 1;
	int y2 = y1 + ri->h - 1;
	int rows, cols;
	int i, j;
	float x, y, dx, dy;
	enum obstacle_types obj;


	// Place shelves vertically or horizontally
	x = x1;
	y = y1;
	if (ri->w < ri->h) {
		dx = OBSTACLE_DIM_X(ISO_SHELF_FULL_V);
		dy = OBSTACLE_DIM_Y(ISO_SHELF_FULL_V);
		rows = ri->h / dy / 2;
		cols = ri->w / dx / 3;
		obj = ISO_SHELF_FULL_V;
		dx *= 2;
		// Put chair at the corner
		x += OBSTACLE_DIM_X(ISO_DESKCHAIR_3);
		y += 0.5;
		mapgen_add_obstacle(x, y, ISO_DESKCHAIR_3);
		// Put library table near the chair
		x = x1 + OBSTACLE_DIM_X(ISO_LIBRARY_FURNITURE_1) / 2;
		y = y1 + OBSTACLE_DIM_Y(ISO_LIBRARY_FURNITURE_1);
		mapgen_add_obstacle(x, y, ISO_LIBRARY_FURNITURE_1);
	} else {
		dx = OBSTACLE_DIM_X(ISO_SHELF_FULL_H);
		dy = OBSTACLE_DIM_Y(ISO_SHELF_FULL_H);
		rows = ri->h / dy / 3;
		cols = ri->w / dx / 2;
		obj = ISO_SHELF_FULL_H;
		dy *= 2;
		// Put chair at the corner
		x += 0.5;
		y += OBSTACLE_DIM_Y(ISO_DESKCHAIR_1);
		mapgen_add_obstacle(x, y, ISO_DESKCHAIR_1);
		// Put library table near the chair
		x = x1 + OBSTACLE_DIM_X(ISO_LIBRARY_FURNITURE_2);
		y = y1 + OBSTACLE_DIM_Y(ISO_LIBRARY_FURNITURE_2) / 2;
		mapgen_add_obstacle(x, y, ISO_LIBRARY_FURNITURE_2);
	}

	for (i = 1; i <= rows; i++) {
		for (j = 1; j <= cols; j++) {
			x = x2 - j * dx;
			y = y2 - i * dy;
			mapgen_add_obstacle(x, y, obj);
		}
	}
}

static void build_garden_path(struct roominfo *ri, struct doorinfo *di)
{
	int x1, x2, y1, y2;
	int i, j;

	x1 = di->x;
	y1 = di->y;
	// Whether we are building vertical or horizontal path?
	if (ri->x < x1 && x1 < ri->x + ri->w) {
		// Build path from door to room height if the door is lower
		if (ri->y < y1) {
			y2 = y1;
			y1 = ri->y;
		} else {
			y2 = ri->y + ri->h;
		}
		x2 = x1;
		x1 -= 2;
	} else {
		// Build path from door to room width if the door is right-hand
		if (ri->x < x1) {
			x2 = x1;
			x1 = ri->x;
		} else {
			x2 = ri->x + ri->w;
		}
		y2 = y1;
		y1 -= 2;
	}

	for (i = y1; i < y2; i++) {
		for (j = x1; j < x2; j++) {
			mapgen_set_floor(j, i, ISO_MISCELLANEOUS_FLOOR_21);
			mapgen_set_floor(j, i, ISO_MISCELLANEOUS_FLOOR_21);
			// Prevent path from placing other obstacles on it
			mapgen_put_tile(j, i, TILE_WALL, -1);
			mapgen_put_tile(j, i, TILE_WALL, -1);
		}
	}
}

static void place_garden(int room)
{
	struct roominfo *ri = &rooms[room];
	int x1 = ri->x;
	int y1 = ri->y;
	int x2 = x1 + ri->w - 1;
	int y2 = y1 + ri->h - 1;
	int i, j;
	int num;

	for (i = y1; i <= y2; i++)
		for (j = x1; j <= x2; j++)
			mapgen_set_floor(j, i, ISO_OVERLAY_FLOOR_SAND_WITH_GRASS_1 + MyRandom(4));

	// Cycle through doors of the given room
	for (i = 0; i < ri->num_doors; i++)
		build_garden_path(ri, &ri->doors[i]);
	// Find the doors of other rooms that lead to the given
	for (i = 0; i < total_rooms; i++) {
		for (j = 0; j < rooms[i].num_doors; j++) {
			if (rooms[i].doors[j].room == room)
				build_garden_path(ri, &rooms[i].doors[j]);
		}
	}
	// Fill garden with trees
	num = sqrt(ri->w * ri->h) / 3;
	while (num--) {
		x1 = ri->x + 1;
		y1 = ri->y + 1;
		x1 += MyRandom(ri->w - 2);
		y1 += MyRandom(ri->h - 2);
		if (mapgen_get_tile(x1, y1) == TILE_FLOOR) {
			mapgen_add_obstacle(x1, y1, ISO_TREE_1 + MyRandom(2));
			mapgen_put_tile(x1, y1, TILE_WALL, -1);
		}
	}
}

// Place a grid of office desks in the given room
static int place_work_office(int room)
{
	// Randomly choose to place single row of desks or double
	int d = 3 + 2 * MyRandom(1);

	int w = rooms[room].w - 3;
	int h = rooms[room].h - 3;
	int num_col = w / 2;
	int num_row = h / d;
	int x0 = rooms[room].x + (rooms[room].w - num_col * 2) / 2;
	int y0 = rooms[room].y + (rooms[room].h - num_row * d) / 2;
	int i, j, k, n, l;
	float x, y;
	int theme = RAND_THEME(living_themes);
	int floor_theme = RAND_THEME(living_themes);
	int obj;
	int plain_wall = MyRandom(5);
	int chair = MyRandom(1) ? ISO_N_CHAIR : ISO_DESKCHAIR_1;

	if (!w || !h)
		return 0;

	// Whether to start build upper cube or lower if the row is double
	l = -2 * MyRandom(1);
	if (l == -2 || d == 5)
		y0 += 2;
	for (i = 0; i < num_row; i++) {
		for (j = 0; j < num_col; j++) {
			n = d / 2;
			k = l;
			while (n--) {
				// Construct office cube
				mapgen_add_obstacle(x0 + j * 2 + 0.5, y0 + i * d, theme_data[theme].wall_n);
				obj = plain_wall ? theme_data[theme].wall_n : theme_data[theme].window_wall_h;
				mapgen_add_obstacle(x0 + j * 2 + 1.5, y0 + i * d, obj);
				// Sometimes don't create cube, instead place sofas and table
				if (MyRandom(5) || !j) {
					mapgen_add_obstacle(x0 + j * 2, y0 + i * d + 0.5 + k, theme_data[theme].wall_w);
					mapgen_add_obstacle(x0 + j * 2, y0 + i * d + 1.5 + k, theme_data[theme].wall_w);
					// Place table and chair
					obj = ISO_N_DESK;
					x = OBSTACLE_DIM_X(obj) / 2;
					y = OBSTACLE_DIM_Y(obj) / 2 + k;
					mapgen_add_obstacle(x0 + j * 2 + x, y0 + i * d + y, obj);
					obj = chair + MyRandom(2);
					mapgen_add_obstacle(x0 + j * 2 + x + OBSTACLE_DIM_X(obj) / 2, y0 + i * d + y, obj);
					// Place a book shelf or a plant
					if (!MyRandom(3))
						obj = ISO_SOFFA_CORNER_PLANT_2 + 2 * MyRandom(1);
					else
						obj = ISO_SHELF_SMALL_FULL_H;
					mapgen_add_obstacle(x0 + j * 2 + x, y0 + i * d + y + 1, obj);
				} else {
					obj = ISO_TABLE_GLASS_2;
					x = OBSTACLE_DIM_X(obj) / 3;
					y = 3 * OBSTACLE_DIM_Y(obj) / 4 + k;
					mapgen_add_obstacle(x0 + j * 2 + x, y0 + i * d + y, obj);
					mapgen_add_obstacle(x0 + j * 2 + x, y0 + i * d + y - OBSTACLE_DIM_Y(obj) / 3, ISO_RED_CHAIR_S);
					if (MyRandom(1))
						mapgen_add_obstacle(x0 + j * 2 + x + OBSTACLE_DIM_X(obj) / 2, y0 + i * d + y, ISO_SOFFA_3);
				}
				k = -2 - k;
			}
		}
	}
	// Fill floor tiles
	if (l == -2 || d == 5)
		y0 -= 2;
	for (i = 0; i < num_row * d - 1; i++) {
		for (j = 0; j < num_col * 2; j++)
			mapgen_set_floor(x0 + j, y0 + i, theme_data[floor_theme].floor[1]);
	}

	return 1;
}

static int place_main_room(int room)
{
	const int projectors[] = { ISO_PROJECTOR_S, ISO_PROJECTOR_W, ISO_PROJECTOR_N, ISO_PROJECTOR_E };
	const int dx[] = { 1, -1, -1,  1 };
	const int dy[] = { 1,  1, -1, -1 };
	const int screen_dx[] = { 1, -3, -1, 3 };
	const int screen_dy[] = { 3,  1, -3, -1 };

	int x = rooms[room].x + rooms[room].w / 2;
	int y = rooms[room].y + rooms[room].h / 2;
	int i = MyRandom(3);
	int obj;
	int w, h;

	if (rooms[room].w < 8 || rooms[room].h < 8)
		return 0;

	// If length of the one of the sides is equal to 8 the others
	// should be greater than 8
	if (rooms[room].w == 8 || rooms[room].h == 8) {
		if (rooms[room].w < rooms[room].h && i % 2)
			i = (i + 1) % 4;
		else if (rooms[room].w > rooms[room].h && !(i % 2))
			i = (i + 1) % 4;
		else
			return 0;
	}
	int n = MyRandom(1) + 3;
	// If we place only 3 tables out of 4, there is a place for projector
	if (n == 3) {
		// Place projector instead of one of the tables
		obj = projectors[i];
		mapgen_add_obstacle(x + dx[i], y + dy[i], obj);
		mapgen_add_obstacle(x + screen_dx[i], y + screen_dy[i], ISO_PROJECTOR_SCREEN_N + i);
	}
	// Place round tables
	while (n--) {
		i = (i + 1) % 4;
		obj = ISO_CONFERENCE_TABLE_N + i;
		w = OBSTACLE_DIM_X(obj) / 2;
		h = OBSTACLE_DIM_Y(obj) / 2;
		mapgen_add_obstacle(x + w * dx[i], y + h * dy[i], obj);
	}

	return 1;
}

static void fill_rooms(int *vis)
{
	int i;
	// Gifts are placed in rooms that have not yet been visited.

	// Armories are for rooms with only one neighbor
	for (i = 0; i < total_rooms; i++) {
		if (!vis[i] && rooms[i].num_neighbors == 1) {
			fill_armory(i);
			vis[i] = 1;
		}
	}

	// Other rooms get regular gifts
	for (i = 0; i < total_rooms; i++) {
		if (!vis[i])
			mapgen_gift(&rooms[i]);
	}
}

// Sets living room theme for the middle room and some its neighbors
static int set_living_theme_recursive(int room, int depth, int *vis)
{
	int i;
	int count = 0;

	if (!depth)
		return 0;

	if (rooms[room].w != 2 && rooms[room].h != 2) {
		rooms[room].theme = RAND_THEME(living_themes);
		vis[room] = 1;
		count = 1;
	}
	for (i = 0; i < rooms[room].num_neighbors; i++)
		count += set_living_theme_recursive(rooms[room].neighbors[i], depth - 1, vis);

	return count;
}

void mapgen_place_obstacles(struct dungeon_info *di, int w, int h, unsigned char *tiles)
{
#define MIN_LIVING_ROOMS	6

#define MAIN_ROOM			2
#define OFFICE_ROOM			3
#define GARDEN_ROOM			4
#define LIBRARY_ROOM		5
#define SPECIAL_ROOM		6

	int i;
	int x, y;
	int wall, room, room2;
	int vis[total_rooms];
	int num;

	for (i = 0; i < total_rooms; i++) {
		rooms[i].theme = RAND_THEME(industrial_themes);
		rooms[i].period = MyRandom(4) + 1;
		vis[i] = 0;
	}
	num = MyRandom(1) + 2;
	while (set_living_theme_recursive(di->middle_room, num, vis) < MIN_LIVING_ROOMS)
		set_living_theme_recursive(di->middle_room, ++num, vis);
	vis[di->enter] = SPECIAL_ROOM;
	vis[di->exit] = SPECIAL_ROOM;
	// Place main room to the biggest among visited
	for (i = 0; i < total_rooms; i++)
		if (vis[di->sorted_square[i]] == 1) {
			place_main_room(di->sorted_square[i]);
			vis[di->sorted_square[i]] = MAIN_ROOM;
			break;
		}

	for (y = 0; y < h; y++) {
		for (x = 0; x < w; x++) {
			room = mapgen_get_room(x, y);
			switch(tiles[y * w + x]) {
				case TILE_PARTITION:
					wall = WALL_PART;
					room2 = room;
					// Tiles oriented to the north and west, must have a theme of a room,
					// which is lower and right of the partition, respectively.
					if (mapgen_get_room(x, y + 1) != room && mapgen_get_tile(x, y + 1) == TILE_FLOOR) {
						wall |= WALL_S;
						room2 = mapgen_get_room(x, y + 1);
					}
					if (mapgen_get_room(x + 1, y) != room && mapgen_get_tile(x + 1, y) == TILE_FLOOR) {
						wall |= WALL_E;
						room2 = mapgen_get_room(x + 1, y);
					}

					if (mapgen_get_room(x - 1, y) != room && mapgen_get_tile(x - 1, y) == TILE_FLOOR)
						wall |= WALL_W;
					if (mapgen_get_room(x, y - 1) != room && mapgen_get_tile(x, y - 1) == TILE_FLOOR)
						wall |= WALL_N;
					themes[rooms[room2].theme](x, y, wall);
				case TILE_FLOOR:
					wall = 0;
					if (tiles[y * w + x - 1] == TILE_WALL)
						wall |= WALL_W;
					if (tiles[y * w + x + 1] == TILE_WALL)
						wall |= WALL_E;
					if (tiles[(y - 1) * w + x] == TILE_WALL)
						wall |= WALL_N;
					if (tiles[(y + 1) * w + x] == TILE_WALL)
						wall |= WALL_S;
					themes[rooms[room].theme](x, y, wall);
					break;	
				case TILE_WALL:
					mapgen_set_floor(x, y, ISO_FLOOR_EMPTY);
					break;
				default:
					mapgen_set_floor(x, y, tiles[y * w + x]);
			}
		}
	} 

	// Place offices
	for (i = 0; i < total_rooms; i++) {
		// vis[i] equal to 1 means that the room is free to decorate
		if (rooms[i].w != 2 && rooms[i].h != 2 && vis[i] == 1) {
			if (MyRandom(1)) {
				place_work_office(i);
				vis[i] = OFFICE_ROOM;
			} else if (MyRandom(1)) {
				place_library(i);
				vis[i] = LIBRARY_ROOM;
			} else if (di->distance[i] < num + 5 && !MyRandom(2)) {
				place_garden(i);
				vis[i] = GARDEN_ROOM;
			}
		}
	}

	fill_rooms(vis);
}
