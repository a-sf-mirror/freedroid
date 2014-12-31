/* 
 *
 *   Copyright (c) 1994, 2002, 2003, 2004 Johannes Prix
 *   Copyright (c) 1994, 2002 Reinhard Prix
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

/**
 * This file contains all the functions managing the things one gets to see.
 * That includes assembling of enemies, assembling the currently
 * relevant porting of the map (the bricks I mean), drawing all visible
 * elements like bullets, blasts, enemies or influencer in a not visible
 * place in memory at first, and finally drawing them to the visible
 * screen for the user.
 */

#define _view_c

#include "system.h"

#include "defs.h"
#include "struct.h"
#include "global.h"
#include "map.h"
#include "proto.h"

#include "widgets/widgets.h"

#include "lvledit/lvledit.h"
#include "lvledit/lvledit_display.h"
#include "lvledit/lvledit_tools.h"

#include <zlib.h>
#include <math.h>

#if SDL_BYTEORDER == SDL_LIL_ENDIAN
int rmask = 0x00FF0000;
int gmask = 0x0000FF00;
int bmask = 0x000000FF;
int amask = 0xFF000000;
#else
int rmask = 0x0000FF00;
int gmask = 0x00FF0000;
int bmask = 0xFF000000;
int amask = 0x000000FF;
#endif

static int old_current_level = -1;

void PutRadialBlueSparks(float PosX, float PosY, float Radius, int SparkType, uint8_t active_directions[RADIAL_SPELL_DIRECTIONS], float age);

struct blitting_list_element {
	int element_type;
	void *element_pointer;
	float norm;
	int list_position;
	int code_number;
};

struct dynarray *blitting_list;

enum {
	BLITTING_TYPE_NONE = 0,
	BLITTING_TYPE_OBSTACLE = 1,
	BLITTING_TYPE_ENEMY = 2,
	BLITTING_TYPE_TUX = 3,
	BLITTING_TYPE_BULLET = 4,
	BLITTING_TYPE_BLAST = 5,
	BLITTING_TYPE_THROWN_ITEM = 6,
	BLITTING_TYPE_MOVE_CURSOR = 7
};

LIST_HEAD(visible_level_list);

/**
 * This function displays an item at the current mouse cursor position.
 * The typical crosshair cursor is assumed.  The item is centered around
 * this crosshair cursor, depending on item size.
 */
static void DisplayItemImageAtMouseCursor(int ItemImageCode)
{
	SDL_Rect TargetRect;

	if (ItemImageCode == (-1)) {
		return;
	}
	// We define the target location for the item.  This will be the current
	// mouse cursor position of course, but -16 for the crosshair center, 
	// which is somewhat (16) to the lower right of the cursor top left 
	// corner.
	//
	// And then of course we also have to take into account the size of the
	// item, which is also not always the same.
	//
	TargetRect.x = GetMousePos_x() - ItemMap[ItemImageCode].inv_size.x * 16;
	TargetRect.y = GetMousePos_y() - ItemMap[ItemImageCode].inv_size.y * 16;

	// Do not move an item out of the screen
	if (TargetRect.x < 0)
		TargetRect.x = 0;
	
	if (TargetRect.y < 0)
		TargetRect.y = 0;

	struct image *img = get_item_inventory_image(ItemImageCode);
	display_image_on_screen(img, TargetRect.x, TargetRect.y, IMAGE_NO_TRANSFO);
}

/**
 * Now it's time to blit all the spell effects.
 */
void PutMiscellaneousSpellEffects(void)
{
	int i;

	// Now we put all the spells in the list of active spells
	//
	for (i = 0; i < MAX_ACTIVE_SPELLS; i++) {
		if (AllActiveSpells[i].img_type == (-1))
			continue;
		PutRadialBlueSparks(AllActiveSpells[i].spell_center.x,
				    AllActiveSpells[i].spell_center.y,
				    AllActiveSpells[i].spell_radius, AllActiveSpells[i].img_type,
				    AllActiveSpells[i].active_directions, AllActiveSpells[i].spell_age);
	}

};				// void PutMiscellaneousSpellEffects ( void )

void get_floor_boundaries(int mask, int *LineStart, int *LineEnd, int *ColStart, int *ColEnd)
{
	float zf = lvledit_zoomfact();
	if (mask & ZOOM_OUT) {
		*LineStart = floor(Me.pos.y - (float)(FLOOR_TILES_VISIBLE_AROUND_TUX * zf));
		*LineEnd = floor(Me.pos.y + (float)(FLOOR_TILES_VISIBLE_AROUND_TUX * zf));
		*ColStart = floor(Me.pos.x - (float)(FLOOR_TILES_VISIBLE_AROUND_TUX * zf));
		*ColEnd = floor(Me.pos.x + (float)(FLOOR_TILES_VISIBLE_AROUND_TUX * zf));
	} else {
		*LineStart = floor(translate_pixel_to_map_location(UserCenter_x, -UserCenter_y, FALSE));
		*LineEnd =
		    floor(translate_pixel_to_map_location
			  (-UserCenter_x - iso_floor_tile_width + 1, UserCenter_y + iso_floor_tile_height - 1, FALSE));
		*ColStart = floor(translate_pixel_to_map_location(-UserCenter_x, -UserCenter_y, TRUE));
		*ColEnd =
		    floor(translate_pixel_to_map_location
			  (UserCenter_x + iso_floor_tile_width - 1, UserCenter_y + iso_floor_tile_height - 1, TRUE));
	}
}

void object_vtx_color(void *data, float *r, float *g, float *b)
{
	if (element_in_selection(data)) {
		*r = ((SDL_GetTicks() >> 7) % 3) / 2.0;
		*g = (((SDL_GetTicks() >> 7) + 1) % 3) / 2.0;
		*b = (((SDL_GetTicks() >> 7) + 2) % 3) / 2.0;
	} else {
		*r = 1.0;
		*g = 1.0;
		*b = 1.0;
	}
}

/**
 * This function displays floor on the screen.
 */
static void show_floor(int mask)
{
	int LineStart, LineEnd, ColStart, ColEnd, line, col, MapBrick;
	int layer_start, layer_end, layer;
	float r, g, b;
	float zf = ((mask & ZOOM_OUT) ? lvledit_zoomfact_inv() : 1.0);
	level *lvl = curShip.AllLevels[Me.pos.z];

	get_floor_boundaries(mask, &LineStart, &LineEnd, &ColStart, &ColEnd);

	layer_start = 0;
	layer_end = MAX_FLOOR_LAYERS;
	// Draw only the current floor layer?
	if (game_status == INSIDE_LVLEDITOR) {
		if (!GameConfig.show_all_floor_layers) {
			layer_start = current_floor_layer;
			layer_end = layer_start + 1;
		}
	}

	start_image_batch();

	for (line = LineStart; line < LineEnd; line++) {
		for (col = ColStart; col < ColEnd; col++) {
			for (layer = layer_start; layer < layer_end; layer++) {
				// Retrieve floor tile
				MapBrick = get_map_brick(lvl, col, line, layer);
				if (MapBrick == ISO_FLOOR_EMPTY)
					continue;

				// Compute colorization (in case the floor tile is currently selected in the leveleditor)
				if (pos_inside_level(col, line, lvl)) {
					object_vtx_color(&lvl->map[line][col], &r, &g, &b);
				} else {
					r = g = b = 1.0;
				}

				struct image *img = get_floor_tile_image(MapBrick);
				display_image_on_map(img, (float)col + 0.5, (float)line + 0.5, IMAGE_SCALE_RGB_TRANSFO(zf, r, g, b));
			}
		}
	}

	end_image_batch();
}

void blit_leveleditor_point(int x, int y)
{
	if (use_open_gl) {
#ifdef HAVE_LIBGL
		glDisable(GL_TEXTURE_2D);
		glEnable(GL_POINT_SMOOTH);
		glPointSize(5.0);
		glBegin(GL_POINTS);
		glColor3f(1.0, 0.0, 0.0);
		glVertex2i(x, y);
		glColor3f(1.0, 1.0, 1.0);
		glEnd();
		glDisable(GL_POINT_SMOOTH);
		glEnable(GL_TEXTURE_2D);
		glPointSize(1.0);
#endif
	} else {
		SDL_Rect rect = { .x = x, .y = y, .w = 4, .h = 4 };
		sdl_draw_rectangle(&rect, 255, 0, 0, 255);
	}
}

/**
 * More for debugging purposes than for real gameplay, we add some 
 * function to illustrate the collision rectangle of a certain obstacle
 * on the floor via a bright ugly distorted rectangular shape.
 */
void blit_obstacle_collision_rectangle(obstacle * our_obstacle)
{
	int r1, r2, r3, r4, c1, c2, c3, c4;
	float x1, y1, x2, y2;
	gps vpos;

	if (!draw_collision_rectangles)
		return;

	update_virtual_position(&vpos, &our_obstacle->pos, Me.pos.z);

	// If there is no collision rectangle to draw, we are done
	obstacle_spec *spec = get_obstacle_spec(our_obstacle->type);
	if (spec->block_area_type == COLLISION_TYPE_NONE)
		return;

	x1 = vpos.x + spec->left_border;
	y1 = vpos.y + spec->upper_border;
	x2 = vpos.x + spec->right_border;
	y2 = vpos.y + spec->lower_border;

	translate_map_point_to_screen_pixel(x1, y1, &r1, &c1);
	translate_map_point_to_screen_pixel(x1, y2, &r2, &c2);
	translate_map_point_to_screen_pixel(x2, y2, &r3, &c3);
	translate_map_point_to_screen_pixel(x2, y1, &r4, &c4);

	short x[4] = { r1, r2, r3, r4 };
	short y[4] = { c1, c2, c3, c4 };

	// Now we draw the collision rectangle.  We use the same parameters
	// of the obstacle spec, that are also used for the collision checks.
	draw_quad(x, y, 15, 238, 170, 255);
}

void blit_one_obstacle(obstacle *o, int highlight, int zoom)
{
#define HIGHLIGHT 1
#define NOHIGHLIGHT 0

	float zf = zoom ? lvledit_zoomfact_inv() : 1.0;
	level *lvl = curShip.AllLevels[Me.pos.z];
	float r, g, b, a;
	gps pos;

	if ((o->type <= -1) || (o->type >= obstacle_map.size)) {
		error_message(__FUNCTION__, "The obstacle type %d that was given is incorrect. Resetting type to 1.", PLEASE_INFORM, o->type);
		o->type = 1;
	}

	// Maybe the children friendly version is desired.  Then the blood on the floor
	// will not be blitted to the screen.
	if ((!GameConfig.show_blood) && (o->type >= ISO_BLOOD_1) && (o->type <= ISO_BLOOD_8))
		return;

	update_virtual_position(&pos, &o->pos, lvl->levelnum);

	// Compute colorization (in case the obstacle is currently selected in the leveleditor)
	r = g = b = a = 1.0;
	if (pos_inside_level(pos.x, pos.y, lvl)) {
		object_vtx_color(o, &r, &g, &b);
	}

	if (GameConfig.transparency) { 
		if ((pos.x > Me.pos.x - 1.0) && (pos.y > Me.pos.y - 1.0)
			&& (pos.x < Me.pos.x + 1.5) && (pos.y < Me.pos.y + 1.5)) {
				if (game_status == INSIDE_LVLEDITOR) {
					a = 0.5;
				} else if (get_obstacle_spec(o->type)->transparent == TRANSPARENCY_FOR_WALLS) {
					a = 0.5;
				}
		}
	}

	struct image *img = get_obstacle_image(o->type, o->frame_index);
	display_image_on_map(img, pos.x, pos.y, set_image_transformation(zf, zf, r, g, b, a, highlight));
}

/**
 * Several different things must be inserted into the blitting list.
 * Therefore this function is an abstraction, that will insert a generic
 * object into the blitting list.
 */
static void insert_new_element_into_blitting_list(float new_element_norm, int new_element_type, void *new_element_pointer, int code_number)
{
	struct blitting_list_element elt;

	elt.norm = new_element_norm;
	elt.list_position = blitting_list->size;
	elt.element_type = new_element_type;
	elt.element_pointer = new_element_pointer;
	elt.code_number = code_number;

	dynarray_add(blitting_list, &elt, sizeof(struct blitting_list_element));
}

/**
 * In order for the obstacles to be blitted, they must first be inserted
 * into the correctly ordered list of objects to be blitted this frame.
 */
void insert_obstacles_into_blitting_list(int mask)
{
	int i;
	level *obstacle_level;
	int LineStart, LineEnd, ColStart, ColEnd, line, col;
	int px, py;
	int tstamp = next_glue_timestamp();

	obstacle *OurObstacle;
	gps tile_vpos, tile_rpos;
	gps virtpos, reference;

	get_floor_boundaries(mask, &LineStart, &LineEnd, &ColStart, &ColEnd);

	tile_vpos.z = Me.pos.z;

	for (line = LineStart; line < LineEnd; line++) {
		tile_vpos.y = line;

		for (col = ColStart; col < ColEnd; col++) {
			tile_vpos.x = col;
			if (!resolve_virtual_position(&tile_rpos, &tile_vpos)) {
				continue;
			}
			px = (int)rintf(tile_rpos.x);
			py = (int)rintf(tile_rpos.y);
			obstacle_level = curShip.AllLevels[tile_rpos.z];

			for (i = 0; i < obstacle_level->map[py][px].glued_obstacles.size; i++) {
					// Now we have to insert this obstacle.  We do this of course respecting
					// the blitting order, as always...
					int idx = ((int *)obstacle_level->map[py][px].glued_obstacles.arr)[i];
					OurObstacle = &obstacle_level->obstacle_list[idx];

					if (OurObstacle->timestamp == tstamp) {
						continue;
					}

					reference.x = OurObstacle->pos.x;
					reference.y = OurObstacle->pos.y;
					reference.z = tile_rpos.z;

					update_virtual_position(&virtpos, &reference, Me.pos.z);

					// Could not find virtual position? Give up drawing.
					if (virtpos.z == -1)
						continue;

					if (rintf(virtpos.x - 0.5) != col)
						continue;

					if (rintf(virtpos.y - 0.5) != line)
						continue;

					OurObstacle->timestamp = tstamp;

					insert_new_element_into_blitting_list(virtpos.x + virtpos.y, BLITTING_TYPE_OBSTACLE,
									      OurObstacle,
									      idx);
			}
		}
	}
}

/**
 *
 *
 */
static void insert_tux_into_blitting_list(void)
{
	float tux_norm = Me.pos.x + Me.pos.y;

	insert_new_element_into_blitting_list(tux_norm, BLITTING_TYPE_TUX, NULL, -1);

}

/**
 *
 *
 */
void insert_one_enemy_into_blitting_list(enemy * erot)
{
	float enemy_norm;

	enemy_norm = erot->virt_pos.x + erot->virt_pos.y;

	insert_new_element_into_blitting_list(enemy_norm, BLITTING_TYPE_ENEMY, erot, 0);

};				// void insert_one_enemy_into_blitting_list ( int enemy_num )

/**
 *
 *
 */
static void insert_one_thrown_item_into_blitting_list(item *it, int item_num)
{
	float norm = it->virt_pos.x + it->virt_pos.y;

	insert_new_element_into_blitting_list(norm, BLITTING_TYPE_THROWN_ITEM, it, item_num);
}

/**
 *
 *
 */
static void insert_one_bullet_into_blitting_list(float norm, int bullet_num)
{
	insert_new_element_into_blitting_list(norm, BLITTING_TYPE_BULLET, &AllBullets[bullet_num], bullet_num);
}

/**
 *
 *
 */
void insert_one_blast_into_blitting_list(int blast_num)
{
	gps virtpos;

	// Due to the use of a painter algorithm, we need to sort the objects depending of their 
	// isometric distance on the current level.
	// We thus have to get the blast's position on the current level. 
	update_virtual_position(&virtpos, &AllBlasts[blast_num].pos, Me.pos.z);

	// Could not find virtual position? Give up drawing.
	if (virtpos.z == -1)
		return;

	insert_new_element_into_blitting_list(virtpos.x + virtpos.y, BLITTING_TYPE_BLAST, &(AllBlasts[blast_num]), blast_num);
}				// void insert_one_blast_into_blitting_list ( int enemy_num )

/**
 * 
 * 
 */
void insert_move_cursor_into_blitting_list()
{
	float norm;

	if (game_status == INSIDE_LVLEDITOR) {
		// Do not show move cursors inside the editor.
		return;
	}

	norm = Me.mouse_move_target.x + Me.mouse_move_target.y;

	insert_new_element_into_blitting_list(norm, BLITTING_TYPE_MOVE_CURSOR, NULL, 0);
};

/**
 * We need to display bots, objects, bullets... that are on the current level or on one of the
 * levels glued to this one.
 */
int level_is_visible(int level_num)
{
	// Current level is for sure visible

	if (level_num == Me.pos.z)
		return TRUE;

	struct visible_level *l, *n;
	BROWSE_VISIBLE_LEVELS(l, n) {
		if (l->lvl_pointer->levelnum == level_num)
			return TRUE;
	}

	return FALSE;

}				// int level_is_visible ( int level_num )

/**
 * Construct a linked list of visible levels.
 * Also compute the distance between Tux and each level boundaries, and fill
 * the lists of animated obstacles.
 */
void get_visible_levels()
{
	struct visible_level *e, *n;
	
	// Invalidate all entries in the visible_level list
	
	list_for_each_entry(e, &visible_level_list, node) {
		e->valid = FALSE;
	}

	// Find the 4 visible levels
	//
	// Those 4 levels form a square (eventually a degenerated one), one corner 
	// of the square being the current level.
	// The 2 corners are initialized to be on the current level (idx = 1), and 
	// we will extend one of them, depending on Tux's position.
	// (see gps_transform_map_init() main comment for an explanation about neighbor index)

	int left_idx = 1, right_idx = 1;
	int top_idx = 1, bottom_idx = 1;
	float left_or_right_distance = 0.0;	// distance to the left or right neighbor
	float top_or_bottom_distance = 0.0;	// distance to the top or bottom neighbor

	if (Me.pos.x < FLOOR_TILES_VISIBLE_AROUND_TUX) {
		// left neighbors are potentially visible
		left_idx = 0;
		left_or_right_distance = Me.pos.x;
	} else if (Me.pos.x >= CURLEVEL()->xlen - FLOOR_TILES_VISIBLE_AROUND_TUX) {
		// right neighbors are potentially visible
		right_idx = 2;
		left_or_right_distance = CURLEVEL()->xlen - Me.pos.x;
	}

	if (Me.pos.y < FLOOR_TILES_VISIBLE_AROUND_TUX) {
		// top neighbors are potentially visible
		top_idx = 0;
		top_or_bottom_distance = Me.pos.y;
	} else if (Me.pos.y >= CURLEVEL()->ylen - FLOOR_TILES_VISIBLE_AROUND_TUX) {
		// bottom neighbors are potentially visible
		bottom_idx = 2;
		top_or_bottom_distance = CURLEVEL()->ylen - Me.pos.y;
	}
	
	// Add or re-validate entries in the visible_level list

	int i, j;
	float latitude;		// distance, along Y axis, between Tux and the current neighbor
	float longitude;	// distance, along X axis, between Tux and the current neighbor

	for (j = top_idx; j <= bottom_idx; j++) {
		// if j==1, then current neighbor is at the same 'latitude' than Tux's level,
		// so latitude = 0.0
		latitude = (j == 1) ? 0.0 : top_or_bottom_distance;

		for (i = left_idx; i <= right_idx; i++) {
			// if i==1, then current neighbor is at the same 'longitude' than Tux's level,
			// so longitude = 0.0
			longitude = (i == 1) ? 0.0 : left_or_right_distance;

			if (level_neighbors_map[Me.pos.z][j][i]) {
				// if there is already an entry in the visible_level list for 
				// this level, update the entry and re-validate it
				int in_list = FALSE;
				list_for_each_entry(e, &visible_level_list, node) {
					if (e->lvl_pointer->levelnum == level_neighbors_map[Me.pos.z][j][i]->lvl_idx) {
						e->valid = TRUE;
						e->boundary_squared_dist = longitude * longitude + latitude * latitude;
						in_list = TRUE;
						break;
					}
				}
				// if there was no corresponding entry, create a new one
				if (!in_list) {
					e = MyMalloc(sizeof(struct visible_level));
					e->valid = TRUE;
					e->lvl_pointer = curShip.AllLevels[level_neighbors_map[Me.pos.z][j][i]->lvl_idx];
					e->boundary_squared_dist = longitude * longitude + latitude * latitude;
					e->animated_obstacles_dirty_flag = TRUE;
					INIT_LIST_HEAD(&e->animated_obstacles_list);
					list_add_tail(&e->node, &visible_level_list);
				}
			}
		}
	}
	
	// If the current level changed, remove useless invalid entries.
	// An entry is useless if the associated level is not a neighbor of the
	// current level. To find if two levels are connected, we look at the content
	// of 'gps_transform_matrix' (there is no gps transformation between two
	// unconnected levels).
	
	struct neighbor_data_cell *transform_data;
	
	if (old_current_level != Me.pos.z) {
		old_current_level = Me.pos.z;
		
		list_for_each_entry_safe(e, n, &visible_level_list, node) {
			if (e->valid)
				continue;
			transform_data = &gps_transform_matrix[Me.pos.z][e->lvl_pointer->levelnum];
			if (!transform_data->valid) {
				// useless entry: removing it from the linked list
				clear_animated_obstacle_list(e);
				list_del(&e->node);
				free(e);
			}
		}
	}
}

/*
 * This function resets the visible levels list, as well as all animated
 * obstacle lists.
 */
void reset_visible_levels()
{
	struct visible_level *e, *n;
	
	// Clear current list
	list_for_each_entry_safe(e, n, &visible_level_list, node) {
		clear_animated_obstacle_list(e);
		list_del(&e->node);
		free(e);
	}
	
	old_current_level = -1;
}

/*
 * Initialization of the 2 arrays used to accelerate gps transformations.
 * 
 * gps_transform_matrix[lvl1][lvl2] is a matrix used in update_virtual_position().
 * It contains the data needed to transform a gps position defined relatively to one level
 * into a gps position defined relatively to an other level.
 * gps_transform_matrix[lvl1][lvl2] is used for a transformation from 'lvl1' to 'lvl2'.
 * 
 * level_neighbors_map[lvl][idY][idX] is used to retrieve all the neighbors (and the corresponding 
 * transformation data) of one given level. It is used in resolve_virtual_position().
 * The (idY, idX) pair defines one of the nine neighbors, using the following schema:
 *              N
 *    (0,0) | (0,1) | (0,2)
 *   -------+-------+-------
 *  W (1,0) |       | (1,2) E
 *   -------+-------+-------
 *    (2,0) | (2,1) | (2,2)
 *              S
 */

// Some helper macros
#define NEIGHBOR_TRANSFORM_NW(lvl)   level_neighbors_map[lvl][0][0]
#define NEIGHBOR_TRANSFORM_N(lvl)    level_neighbors_map[lvl][0][1]
#define NEIGHBOR_TRANSFORM_NE(lvl)   level_neighbors_map[lvl][0][2]
#define NEIGHBOR_TRANSFORM_W(lvl)    level_neighbors_map[lvl][1][0]
#define NEIGHBOR_TRANSFORM_SELF(lvl) level_neighbors_map[lvl][1][1]
#define NEIGHBOR_TRANSFORM_E(lvl)    level_neighbors_map[lvl][1][2]
#define NEIGHBOR_TRANSFORM_SW(lvl)   level_neighbors_map[lvl][2][0]
#define NEIGHBOR_TRANSFORM_S(lvl)    level_neighbors_map[lvl][2][1]
#define NEIGHBOR_TRANSFORM_SE(lvl)   level_neighbors_map[lvl][2][2]

void gps_transform_map_init()
{
	int lvl_idx;
	int x, y;
	int ngb_idx, diag_idx;

	if (!gps_transform_map_dirty_flag)
		return;

	// Reset maps
	//

	for (lvl_idx = 0; lvl_idx < MAX_LEVELS; lvl_idx++) {
		for (ngb_idx = 0; ngb_idx < MAX_LEVELS; ngb_idx++) {
			gps_transform_matrix[lvl_idx][ngb_idx].delta_x = 0;
			gps_transform_matrix[lvl_idx][ngb_idx].delta_y = 0;
			gps_transform_matrix[lvl_idx][ngb_idx].lvl_idx = -1;
			gps_transform_matrix[lvl_idx][ngb_idx].valid = FALSE;
		}

		for (y = 0; y < 3; y++) {
			for (x = 0; x < 3; x++) {
				level_neighbors_map[lvl_idx][y][x] = NULL;
			}
		}
	}

	// Scan direct neighbors and fill maps
	//

	for (lvl_idx = 0; lvl_idx < MAX_LEVELS; lvl_idx++) {
		// Undefined level -> continue  
		if (!level_exists(lvl_idx))
			continue;

		// Self
		gps_transform_matrix[lvl_idx][lvl_idx].delta_x = 0;
		gps_transform_matrix[lvl_idx][lvl_idx].delta_y = 0;
		gps_transform_matrix[lvl_idx][lvl_idx].lvl_idx = lvl_idx;
		gps_transform_matrix[lvl_idx][lvl_idx].valid = TRUE;

		NEIGHBOR_TRANSFORM_SELF(lvl_idx) = &gps_transform_matrix[lvl_idx][lvl_idx];

		// North
		ngb_idx = curShip.AllLevels[lvl_idx]->jump_target_north;
		if (ngb_idx != -1) {
			gps_transform_matrix[lvl_idx][ngb_idx].delta_x = 0;
			gps_transform_matrix[lvl_idx][ngb_idx].delta_y = +curShip.AllLevels[ngb_idx]->ylen;
			gps_transform_matrix[lvl_idx][ngb_idx].lvl_idx = ngb_idx;
			gps_transform_matrix[lvl_idx][ngb_idx].valid = TRUE;

			NEIGHBOR_TRANSFORM_N(lvl_idx) = &gps_transform_matrix[lvl_idx][ngb_idx];
		}
		// South
		ngb_idx = curShip.AllLevels[lvl_idx]->jump_target_south;
		if (ngb_idx != -1) {
			gps_transform_matrix[lvl_idx][ngb_idx].delta_x = 0;
			gps_transform_matrix[lvl_idx][ngb_idx].delta_y = -curShip.AllLevels[lvl_idx]->ylen;
			gps_transform_matrix[lvl_idx][ngb_idx].lvl_idx = ngb_idx;
			gps_transform_matrix[lvl_idx][ngb_idx].valid = TRUE;

			NEIGHBOR_TRANSFORM_S(lvl_idx) = &gps_transform_matrix[lvl_idx][ngb_idx];
		}
		// East
		ngb_idx = curShip.AllLevels[lvl_idx]->jump_target_east;
		if (ngb_idx != -1) {
			gps_transform_matrix[lvl_idx][ngb_idx].delta_x = -curShip.AllLevels[lvl_idx]->xlen;
			gps_transform_matrix[lvl_idx][ngb_idx].delta_y = 0;
			gps_transform_matrix[lvl_idx][ngb_idx].lvl_idx = ngb_idx;
			gps_transform_matrix[lvl_idx][ngb_idx].valid = TRUE;

			NEIGHBOR_TRANSFORM_E(lvl_idx) = &gps_transform_matrix[lvl_idx][ngb_idx];
		}
		// West
		ngb_idx = curShip.AllLevels[lvl_idx]->jump_target_west;
		if (ngb_idx != -1) {
			gps_transform_matrix[lvl_idx][ngb_idx].delta_x = +curShip.AllLevels[ngb_idx]->xlen;
			gps_transform_matrix[lvl_idx][ngb_idx].delta_y = 0;
			gps_transform_matrix[lvl_idx][ngb_idx].lvl_idx = ngb_idx;
			gps_transform_matrix[lvl_idx][ngb_idx].valid = TRUE;

			NEIGHBOR_TRANSFORM_W(lvl_idx) = &gps_transform_matrix[lvl_idx][ngb_idx];
		}
	}

	// Scan diagonal levels and fill maps
	//
	// Now that we know the direct neighbors, we can use that knowledge to ease
	// the finding of the diagonal levels.
	// The main difficulty is that the north-west neighbor, for example, can be
	// the west neighbor of the north neighbor or the north neighbor of the west neighbor.
	// On a 4-connected corner (4 levels around a corner), the two cases are equivalent.
	// However, on a 3-connected corner, one of the two cases is invalid. We thus have to
	// try the 2 ways to reach each diagonal levels.
	//

	for (lvl_idx = 0; lvl_idx < MAX_LEVELS; lvl_idx++) {
		// North-West neighbor.
		if (NEIGHBOR_TRANSFORM_N(lvl_idx) && NEIGHBOR_TRANSFORM_W(NEIGHBOR_ID_N(lvl_idx))) {
			diag_idx = NEIGHBOR_ID_W(NEIGHBOR_ID_N(lvl_idx));
			gps_transform_matrix[lvl_idx][diag_idx].delta_x =
			    NEIGHBOR_TRANSFORM_N(lvl_idx)->delta_x + NEIGHBOR_TRANSFORM_W(NEIGHBOR_ID_N(lvl_idx))->delta_x;
			gps_transform_matrix[lvl_idx][diag_idx].delta_y =
			    NEIGHBOR_TRANSFORM_N(lvl_idx)->delta_y + NEIGHBOR_TRANSFORM_W(NEIGHBOR_ID_N(lvl_idx))->delta_y;
			gps_transform_matrix[lvl_idx][diag_idx].lvl_idx = diag_idx;
			gps_transform_matrix[lvl_idx][diag_idx].valid = TRUE;

			NEIGHBOR_TRANSFORM_NW(lvl_idx) = &gps_transform_matrix[lvl_idx][diag_idx];
		}
		// North-East neighbor.
		if (NEIGHBOR_TRANSFORM_N(lvl_idx) && NEIGHBOR_TRANSFORM_E(NEIGHBOR_ID_N(lvl_idx))) {
			diag_idx = NEIGHBOR_ID_E(NEIGHBOR_ID_N(lvl_idx));
			gps_transform_matrix[lvl_idx][diag_idx].delta_x =
			    NEIGHBOR_TRANSFORM_N(lvl_idx)->delta_x + NEIGHBOR_TRANSFORM_E(NEIGHBOR_ID_N(lvl_idx))->delta_x;
			gps_transform_matrix[lvl_idx][diag_idx].delta_y =
			    NEIGHBOR_TRANSFORM_N(lvl_idx)->delta_y + NEIGHBOR_TRANSFORM_E(NEIGHBOR_ID_N(lvl_idx))->delta_y;
			gps_transform_matrix[lvl_idx][diag_idx].lvl_idx = diag_idx;
			gps_transform_matrix[lvl_idx][diag_idx].valid = TRUE;

			NEIGHBOR_TRANSFORM_NE(lvl_idx) = &gps_transform_matrix[lvl_idx][diag_idx];
		}
		// South-West neighbor.
		if (NEIGHBOR_TRANSFORM_S(lvl_idx) && NEIGHBOR_TRANSFORM_W(NEIGHBOR_ID_S(lvl_idx))) {
			diag_idx = NEIGHBOR_ID_W(NEIGHBOR_ID_S(lvl_idx));
			gps_transform_matrix[lvl_idx][diag_idx].delta_x =
			    NEIGHBOR_TRANSFORM_S(lvl_idx)->delta_x + NEIGHBOR_TRANSFORM_W(NEIGHBOR_ID_S(lvl_idx))->delta_x;
			gps_transform_matrix[lvl_idx][diag_idx].delta_y =
			    NEIGHBOR_TRANSFORM_S(lvl_idx)->delta_y + NEIGHBOR_TRANSFORM_W(NEIGHBOR_ID_S(lvl_idx))->delta_y;
			gps_transform_matrix[lvl_idx][diag_idx].lvl_idx = diag_idx;
			gps_transform_matrix[lvl_idx][diag_idx].valid = TRUE;

			NEIGHBOR_TRANSFORM_SW(lvl_idx) = &gps_transform_matrix[lvl_idx][diag_idx];
		}
		// South-East neighbor.
		if (NEIGHBOR_TRANSFORM_S(lvl_idx) && NEIGHBOR_TRANSFORM_E(NEIGHBOR_ID_S(lvl_idx))) {
			diag_idx = NEIGHBOR_ID_E(NEIGHBOR_ID_S(lvl_idx));
			gps_transform_matrix[lvl_idx][diag_idx].delta_x =
			    NEIGHBOR_TRANSFORM_S(lvl_idx)->delta_x + NEIGHBOR_TRANSFORM_E(NEIGHBOR_ID_S(lvl_idx))->delta_x;
			gps_transform_matrix[lvl_idx][diag_idx].delta_y =
			    NEIGHBOR_TRANSFORM_S(lvl_idx)->delta_y + NEIGHBOR_TRANSFORM_E(NEIGHBOR_ID_S(lvl_idx))->delta_y;
			gps_transform_matrix[lvl_idx][diag_idx].lvl_idx = diag_idx;
			gps_transform_matrix[lvl_idx][diag_idx].valid = TRUE;

			NEIGHBOR_TRANSFORM_SE(lvl_idx) = &gps_transform_matrix[lvl_idx][diag_idx];
		}
		// West-North neighbor, if needed (i.e. if north-west neighbor was not found).
		if (!NEIGHBOR_TRANSFORM_NW(lvl_idx)) {
			if (NEIGHBOR_TRANSFORM_W(lvl_idx) && NEIGHBOR_TRANSFORM_N(NEIGHBOR_ID_W(lvl_idx))) {
				diag_idx = NEIGHBOR_ID_N(NEIGHBOR_ID_W(lvl_idx));
				gps_transform_matrix[lvl_idx][diag_idx].delta_x =
				    NEIGHBOR_TRANSFORM_W(lvl_idx)->delta_x + NEIGHBOR_TRANSFORM_N(NEIGHBOR_ID_W(lvl_idx))->delta_x;
				gps_transform_matrix[lvl_idx][diag_idx].delta_y =
				    NEIGHBOR_TRANSFORM_W(lvl_idx)->delta_y + NEIGHBOR_TRANSFORM_N(NEIGHBOR_ID_W(lvl_idx))->delta_y;
				gps_transform_matrix[lvl_idx][diag_idx].lvl_idx = diag_idx;
				gps_transform_matrix[lvl_idx][diag_idx].valid = TRUE;

				NEIGHBOR_TRANSFORM_NW(lvl_idx) = &gps_transform_matrix[lvl_idx][diag_idx];
			}
		}
		// West-South neighbor, if needed (i.e. if south-west neighbor was not found).
		if (!NEIGHBOR_TRANSFORM_SW(lvl_idx)) {
			if (NEIGHBOR_TRANSFORM_W(lvl_idx) && NEIGHBOR_TRANSFORM_S(NEIGHBOR_ID_W(lvl_idx))) {
				diag_idx = NEIGHBOR_ID_S(NEIGHBOR_ID_W(lvl_idx));
				gps_transform_matrix[lvl_idx][diag_idx].delta_x =
				    NEIGHBOR_TRANSFORM_W(lvl_idx)->delta_x + NEIGHBOR_TRANSFORM_S(NEIGHBOR_ID_W(lvl_idx))->delta_x;
				gps_transform_matrix[lvl_idx][diag_idx].delta_y =
				    NEIGHBOR_TRANSFORM_W(lvl_idx)->delta_y + NEIGHBOR_TRANSFORM_S(NEIGHBOR_ID_W(lvl_idx))->delta_y;
				gps_transform_matrix[lvl_idx][diag_idx].lvl_idx = diag_idx;
				gps_transform_matrix[lvl_idx][diag_idx].valid = TRUE;

				NEIGHBOR_TRANSFORM_SW(lvl_idx) = &gps_transform_matrix[lvl_idx][diag_idx];
			}
		}
		// East-North neighbor, if needed (i.e. if north-east neighbor was not found).
		if (!NEIGHBOR_TRANSFORM_NE(lvl_idx)) {
			if (NEIGHBOR_TRANSFORM_E(lvl_idx) && NEIGHBOR_TRANSFORM_N(NEIGHBOR_ID_E(lvl_idx))) {
				diag_idx = NEIGHBOR_ID_N(NEIGHBOR_ID_E(lvl_idx));
				gps_transform_matrix[lvl_idx][diag_idx].delta_x =
				    NEIGHBOR_TRANSFORM_E(lvl_idx)->delta_x + NEIGHBOR_TRANSFORM_N(NEIGHBOR_ID_E(lvl_idx))->delta_x;
				gps_transform_matrix[lvl_idx][diag_idx].delta_y =
				    NEIGHBOR_TRANSFORM_E(lvl_idx)->delta_y + NEIGHBOR_TRANSFORM_N(NEIGHBOR_ID_E(lvl_idx))->delta_y;
				gps_transform_matrix[lvl_idx][diag_idx].lvl_idx = diag_idx;
				gps_transform_matrix[lvl_idx][diag_idx].valid = TRUE;

				NEIGHBOR_TRANSFORM_NE(lvl_idx) = &gps_transform_matrix[lvl_idx][diag_idx];
			}
		}
		// East-South neighbor, if needed (i.e. if south-east neighbor was not found).
		if (!NEIGHBOR_TRANSFORM_SE(lvl_idx)) {
			if (NEIGHBOR_TRANSFORM_E(lvl_idx) && NEIGHBOR_TRANSFORM_S(NEIGHBOR_ID_E(lvl_idx))) {
				diag_idx = NEIGHBOR_ID_S(NEIGHBOR_ID_E(lvl_idx));
				gps_transform_matrix[lvl_idx][diag_idx].delta_x =
				    NEIGHBOR_TRANSFORM_E(lvl_idx)->delta_x + NEIGHBOR_TRANSFORM_S(NEIGHBOR_ID_E(lvl_idx))->delta_x;
				gps_transform_matrix[lvl_idx][diag_idx].delta_y =
				    NEIGHBOR_TRANSFORM_E(lvl_idx)->delta_y + NEIGHBOR_TRANSFORM_S(NEIGHBOR_ID_E(lvl_idx))->delta_y;
				gps_transform_matrix[lvl_idx][diag_idx].lvl_idx = diag_idx;
				gps_transform_matrix[lvl_idx][diag_idx].valid = TRUE;

				NEIGHBOR_TRANSFORM_SE(lvl_idx) = &gps_transform_matrix[lvl_idx][diag_idx];
			}
		}
	}

	gps_transform_map_dirty_flag = FALSE;
}

/**
 * There are several cases where an object or a character (Tux or a bot)
 * could become visible or active when they are technically still not on the
 * current level.
 * 
 * Therefore we introduce 'virtual' positions, i.e. the position the object 
 * would have, if the object were in fact counted as part of a neighboring level,
 * mostly the level of the Tux.  Using this concept, we can more easily compute 
 * distances and compare positions.
 *
 * This function is an abstract approach to this problem, working with
 * the 'gps' notion.
 *
 */
void update_virtual_position(gps * target_pos, gps * source_pos, int level_num)
{
	// The case where the position in question is already directly on 
	// the virtual level, things are really simple and we can quit
	// almost immediately...
	//
	if (source_pos->z == level_num) {
		target_pos->x = source_pos->x;
		target_pos->y = source_pos->y;
		target_pos->z = source_pos->z;
		return;
	}
	// Transform the gps position
	//
	if (source_pos->z < 0 || source_pos->z >= sizeof(gps_transform_matrix)/sizeof(gps_transform_matrix[0]) ||
		level_num < 0 || level_num >= sizeof(gps_transform_matrix[0])/sizeof(gps_transform_matrix[0][0])) {
		error_message(__FUNCTION__, "Virtual position update was required for level %d relative to level %d - one of those are incorrect level numbers.", PLEASE_INFORM, source_pos->z, level_num);
		target_pos->x = (-1);
		target_pos->y = (-1);
		target_pos->z = (-1);
		return;
	}

	struct neighbor_data_cell *ngb_data = &gps_transform_matrix[source_pos->z][level_num];

	if (ngb_data->valid) {
		target_pos->x = source_pos->x + ngb_data->delta_x;
		target_pos->y = source_pos->y + ngb_data->delta_y;
		target_pos->z = level_num;
		return;
	}
	// The gps position cannot be expressed in terms of the virtual level.
	// That means we'll best 'erase' the virtual positions, so that
	// no 'phantoms' will occur...
	//
	target_pos->x = (-1);
	target_pos->y = (-1);
	target_pos->z = (-1);

}				// void update_virtual_position ( gps* target_pos , gps* source_pos , int level_num )

/*
 * Transform a virtual position, defined in the 'lvl' coordinate system, into
 * its real level and position
 * 
 * Note: this function only works if the real position is one of the 8 neighbors of 'lvl'.
 * If not, the function returns FALSE. 
 * (a recursive call could be used to remove this limitation)
 */
int resolve_virtual_position(gps *rpos, gps *vpos)
{
	int valid = FALSE;
	level *lvl;

	// Check pre-conditions

	if (vpos->z == -1) {
		error_message(__FUNCTION__, "Resolve virtual position was called with an invalid virtual position (%f:%f:%d).", PLEASE_INFORM, vpos->x, vpos->y, vpos->z);
		print_trace(0);
		rpos->x = vpos->x;
		rpos->y = vpos->y;
		rpos->z = vpos->z;
		return FALSE;
	}

	// Get the gps transformation data cell, according to virtual position value

	lvl = curShip.AllLevels[vpos->z];
	int idX = NEIGHBOR_IDX(vpos->x, lvl->xlen);
	int idY = NEIGHBOR_IDX(vpos->y, lvl->ylen);

	// If we don't have to transform the position, return immediately

	if (idX == 1 && idY == 1) {
		rpos->x = vpos->x;
		rpos->y = vpos->y;
		rpos->z = vpos->z;
		return TRUE;
	}

	// Do the transformation

	struct neighbor_data_cell *ngb_data = level_neighbors_map[vpos->z][idY][idX];

	if (ngb_data && ngb_data->valid) {
		rpos->x = vpos->x + ngb_data->delta_x;
		rpos->y = vpos->y + ngb_data->delta_y;
		rpos->z = ngb_data->lvl_idx;

		// Check that the transformed position is valid (i.e. inside level boundaries)
		level *rlvl = curShip.AllLevels[rpos->z];
		valid = pos_inside_level(rpos->x, rpos->y, rlvl);
	}

	if (!valid) {
		rpos->x = vpos->x;
		rpos->y = vpos->y;
		rpos->z = vpos->z;
		return FALSE;
	}

	return TRUE;
}

/**
 * Check if a position is inside a level's boundaries
 * 
 * return TRUE if '(x,y)' is inside 'lvl'
 */
int pos_inside_level(float x, float y, level * lvl)
{
	return ((x >= 0) && (x < (float)lvl->xlen) && (y >= 0) && (y < (float)lvl->ylen));
}				// pos_inside_level()

/**
 * Check if a position is inside or near a level's boundaries
 * 
 * return TRUE if (x,y) is inside 'lvl' or at less than 'dist' from
 * 'lvl' borders.
 */
int pos_near_level(float x, float y, level * lvl, float dist)
{
	return ((x >= -dist) && (x < (float)lvl->xlen + dist) && (y >= -dist) && (y < (float)lvl->ylen + dist));
}				// pos_near_level()

/**
 * The blitting list must contain the enemies too.  This function is 
 * responsible for inserting the enemies at the right positions.
 */
static void insert_enemies_into_blitting_list(int mask)
{
	int i;
	int xmin, xmax, ymin, ymax;
	enemy *ThisRobot;

	get_floor_boundaries(mask, &ymin, &ymax, &xmin, &xmax);

	// Now that we plan to also show bots on other levels, we must be
	// a bit more general and proceed through all the levels...
	//
	// Those levels not in question will be filtered out anyway inside
	// the loop...
	//

	for (i = 0; i < 2; i++) {
		list_for_each_entry(ThisRobot, (i) ? &dead_bots_head : &alive_bots_head, global_list) {
			if (!level_is_visible(ThisRobot->pos.z))
				continue;

			// We update the virtual position of this bot, such that we can handle it 
			// with easier expressions later...
			//
			update_virtual_position(&(ThisRobot->virt_pos), &(ThisRobot->pos), Me.pos.z);

			if (ThisRobot->virt_pos.x < xmin || ThisRobot->virt_pos.x > xmax ||
				ThisRobot->virt_pos.y < ymin || ThisRobot->virt_pos.y > ymax)
				continue;

			insert_one_enemy_into_blitting_list(ThisRobot);
		}
	}

}

/**
 *
 *
 */
static void insert_bullets_into_blitting_list(int mask)
{
	int i;
	bullet *b;
	int xmin, xmax, ymin, ymax;
	get_floor_boundaries(mask, &ymin, &ymax, &xmin, &xmax);

	for (i = 0; i < MAXBULLETS; i++) {
		b = &AllBullets[i];
		if (b->type == INFOUT)
			continue;

		gps vpos;
		update_virtual_position(&vpos, &b->pos, Me.pos.z);

		if (vpos.z == -1)
			continue;

		if (vpos.x < xmin || vpos.x > xmax || vpos.y < ymin || vpos.y > ymax)
			continue;

		insert_one_bullet_into_blitting_list(vpos.x + vpos.y, i);
	}
}

/**
 *
 *
 */
static void insert_blasts_into_blitting_list(int mask)
{
	int i;
	blast *b;
	int xmin, xmax, ymin, ymax;
	get_floor_boundaries(mask, &ymin, &ymax, &xmin, &xmax);

	for (i = 0; i < MAXBLASTS; i++) {
		b = &AllBlasts[i];
		if (b->type == INFOUT)
			continue;
		
		gps vpos;
		update_virtual_position(&vpos, &b->pos, Me.pos.z);

		if (vpos.z == -1)
			continue;

		if (vpos.x < xmin || vpos.x > xmax || vpos.y < ymin || vpos.y > ymax)
			continue;

		insert_one_blast_into_blitting_list(i);
	}

}

/**
 *
 *
 */
static void insert_thrown_items_into_blitting_list(int mask)
{
	int i;
	struct visible_level *vis_lvl, *n;
	item *it;

	int xmin, xmax, ymin, ymax;
	get_floor_boundaries(mask, &ymin, &ymax, &xmin, &xmax);
	
	BROWSE_VISIBLE_LEVELS(vis_lvl, n) {
		level *lvl = vis_lvl->lvl_pointer;
		for (i = 0; i < MAX_ITEMS_PER_LEVEL; i++) {
			it = &lvl->ItemList[i];
			
			if (it->type == -1)
				continue;

			update_virtual_position(&it->virt_pos, &it->pos, Me.pos.z);

			if (it->virt_pos.z == -1)
				continue;

			if (it->virt_pos.x < xmin || it->virt_pos.x > xmax || it->virt_pos.y < ymin || it->virt_pos.y > ymax)
				continue;
		
			insert_one_thrown_item_into_blitting_list(it, i);
		}
	}
}

static int blitting_list_compare(const void *elt1, const void *elt2)
{
	const struct blitting_list_element *e1 = elt1, *e2 = elt2;

	if (e1->norm < e2->norm)
		return -1;
	else if (e1->norm > e2->norm)
		return 1;
	else if (e1->list_position < e2->list_position)
		return -1;
	else return 1;
}

static void sort_blitting_list(void)
{
	qsort(blitting_list->arr, blitting_list->size, sizeof(struct blitting_list_element),
			blitting_list_compare);
}

/**
 * In isometric viewpoint setting, we need to respect visibility when
 * considering the order of things to blit.  Therefore we will first set
 * up a list of the things to be blitted for this frame.  Then we can
 * later use this list to fill in objects into the picture, automatically
 * having the right order.
 */
void set_up_ordered_blitting_list(int mask)
{
	if (!blitting_list) {
		blitting_list = dynarray_alloc(100, sizeof(struct blitting_list_element));
	} else {
		dynarray_free(blitting_list);
		dynarray_init(blitting_list, 100, sizeof(struct blitting_list_element));
	}

	// Now we can start to fill in the obstacles around the
	// tux...
	//
	insert_obstacles_into_blitting_list(mask);

	insert_tux_into_blitting_list();

	insert_enemies_into_blitting_list(mask);

	insert_bullets_into_blitting_list(mask);

	insert_blasts_into_blitting_list(mask);

	insert_move_cursor_into_blitting_list();

	insert_thrown_items_into_blitting_list(mask);

	sort_blitting_list();
}

static void show_obstacle(int mask, obstacle * o, int code_number)
{


	// Safety checks
	if ((o->type <= -1) || (o->type >= obstacle_map.size)) {
		error_message(__FUNCTION__, "The blitting list contained an illegal obstacle type %d.", PLEASE_INFORM | IS_FATAL, o->type);
	}

	if (!(mask & OMIT_OBSTACLES)) {
		if (mask & ZOOM_OUT) {
			blit_one_obstacle(o, NOHIGHLIGHT, ZOOM_OUT);
		} else {
			if (code_number == clickable_obstacle_under_cursor) {
				blit_one_obstacle(o, HIGHLIGHT, !ZOOM_OUT);
			} else {
				// Do not blit "transp for water" obstacle when not in leveleditor mode
				if (game_status != INSIDE_LVLEDITOR && o->type == ISO_TRANSP_FOR_WATER)
					return;

				// Normal display
				blit_one_obstacle(o, NOHIGHLIGHT, !ZOOM_OUT);
			}
		}
	}
}

/**
 * Now that the blitting list has finally been assembled, we can start to
 * blit all the objects according to the blitting list set up.
 */
void blit_preput_objects_according_to_blitting_list(int mask)
{
	obstacle *our_obstacle = NULL;
	obstacle_spec *obstacle_spec = NULL;
	int item_under_cursor = -1;
	level *item_under_cursor_lvl = NULL;
	
	start_image_batch();

	int i;
	for (i = 0; i < blitting_list->size; i++) {
		struct blitting_list_element *e = &((struct blitting_list_element *)(blitting_list->arr))[i];

		switch (e->element_type) {
		case BLITTING_TYPE_OBSTACLE:
			// We do some sanity checking for illegal obstacle types.
			// Can't hurt to do that so as to be on the safe side.
			//
			if ((((obstacle *) e->element_pointer)->type <= -1) ||
			    ((obstacle *) e->element_pointer)->type >= obstacle_map.size) {
				error_message(__FUNCTION__,
					     "The blitting list contained an illegal obstacle type %d, for obstacle at coordinates %f %f. Doing nothing.", PLEASE_INFORM, 
						 ((obstacle *) e->element_pointer)->type, ((obstacle *) e->element_pointer)->pos.x, ((obstacle *) e->element_pointer)->pos.y);
				break;

			}

			our_obstacle = e->element_pointer;
			obstacle_spec = get_obstacle_spec(our_obstacle->type);

			// If the obstacle has a shadow, it seems like now would be a good time
			// to blit it.
			// Do not display obstacle shadow when obstacles are omitted
			//
			if (!GameConfig.skip_shadow_blitting && !(mask &OMIT_OBSTACLES)) {
				gps vpos;
				update_virtual_position(&vpos, &our_obstacle->pos, Me.pos.z);
				struct image *shadow_img = get_obstacle_shadow_image(our_obstacle->type, our_obstacle->frame_index);
				display_image_on_map(shadow_img, vpos.x, vpos.y, IMAGE_SCALE_TRANSFO((mask & ZOOM_OUT) ? lvledit_zoomfact_inv() : 1.0));
			}
			
			// If the obstacle in question does have a collision rectangle, then we
			// draw that on the floor now.
			//
			blit_obstacle_collision_rectangle(our_obstacle);

			// Draw the obstacle by itself if it is a preput obstacle
			//
			if (obstacle_spec->flags & NEEDS_PRE_PUT)
				show_obstacle(mask, ((obstacle *) e->element_pointer), e->code_number);
			break;
		
		case BLITTING_TYPE_ENEMY:
			// Enemies, which are dead already become like decoration on the floor.  
			// They should never hide the Tux, so we blit them beforehand and not
			// again later from the list.
			//
			if (((enemy *)e->element_pointer)->animation_type == DEATH_ANIMATION
			    || ((enemy *)e->element_pointer)->animation_type == DEAD_ANIMATION) {
				if (!(mask & OMIT_ENEMIES)) {
					PutEnemy((enemy *) (e->element_pointer), -1, -1, mask, FALSE);
				}
			}
			break;
			
		case BLITTING_TYPE_THROWN_ITEM:
			if (mask & SHOW_ITEMS) {
				// Preput thrown items when they are on the floor (i.e. not during the
				// throwing animation)
				item *the_item = (item*)e->element_pointer;
				item_under_cursor_lvl = NULL;
				if (the_item->throw_time <= 0) {
					item_under_cursor = get_floor_item_index_under_mouse_cursor(&item_under_cursor_lvl);
					if (item_under_cursor != -1 && item_under_cursor_lvl != NULL &&
						the_item->pos.z == item_under_cursor_lvl->levelnum && item_under_cursor == e->code_number)
						PutItem(the_item, mask, PUT_NO_THROWN_ITEMS, TRUE);
					else
						PutItem(the_item, mask, PUT_NO_THROWN_ITEMS, FALSE);				
				}
			}
			break;
			
		case BLITTING_TYPE_MOVE_CURSOR:
			PutMouseMoveCursor();
			break;
		}
	}

	end_image_batch();

}				// void blit_preput_objects_according_to_blitting_list ( ... )

/**
 * Now that the blitting list has finally been assembled, we can start to
 * blit all the objects according to the blitting list set up.
 */
void blit_nonpreput_objects_according_to_blitting_list(int mask)
{
	enemy *enemy_under_cursor = NULL;
	level *item_under_cursor_lvl = NULL;

	// We memorize which 'enemy' is currently under the mouse target, so that we
	// can properly highlight this enemy...
	//
	enemy_under_cursor = GetLivingDroidBelowMouseCursor();
	int item_under_cursor = get_floor_item_index_under_mouse_cursor(&item_under_cursor_lvl);

	// Now it's time to blit all the elements from the list...
	//
	int i;
	for (i = 0; i < blitting_list->size; i++) {
		start_image_batch();
		struct blitting_list_element *e = &((struct blitting_list_element *)(blitting_list->arr))[i];

		if (e->element_type == BLITTING_TYPE_NONE)
			break;
		switch (e->element_type) {
		case BLITTING_TYPE_OBSTACLE:
			// Skip preput obstacles
			if (get_obstacle_spec(((obstacle*)(e->element_pointer))->type)->flags & NEEDS_PRE_PUT)
				continue;
			show_obstacle(mask, e->element_pointer, e->code_number);
			break;
		case BLITTING_TYPE_TUX:
			if (!(mask & OMIT_TUX)) {
				if (Me.energy > 0)
					blit_tux(-1, -1);
			}
			break;
		case BLITTING_TYPE_ENEMY:
			if (!(mask & OMIT_ENEMIES)) {
				if (((enemy *) e->element_pointer)->energy < 0)
					continue;
				if (((enemy *) e->element_pointer)->animation_type == DEATH_ANIMATION)
					continue;
				if (((enemy *) e->element_pointer)->animation_type == DEAD_ANIMATION)
					continue;

				// A droid can either be rendered in normal mode or in highlighted
				// mode, depending in whether the mouse cursor is right over it or not.
				//
				if (e->element_pointer == enemy_under_cursor)
					PutEnemy((enemy *) e->element_pointer, -1, -1, mask, TRUE);
				else
					PutEnemy((enemy *) e->element_pointer, -1, -1, mask, FALSE);
			}
			break;
		case BLITTING_TYPE_BULLET:
			// DebugPrintf ( -1000 , "Bullet code_number: %d. " , blitting_list [ i ] . code_number );
			PutBullet(e->code_number, mask);
			break;
		case BLITTING_TYPE_BLAST:
			if (!(mask & OMIT_BLASTS))
				PutBlast(e->code_number);
			break;
		case BLITTING_TYPE_THROWN_ITEM:
			{
				item *the_item = (item*)e->element_pointer;
				if (the_item->throw_time > 0) {
					if (item_under_cursor != -1 && item_under_cursor == e->code_number
						&& item_under_cursor_lvl->levelnum == the_item->pos.z)
						PutItem(the_item, mask, PUT_ONLY_THROWN_ITEMS, TRUE);
					else
						PutItem(the_item, mask, PUT_ONLY_THROWN_ITEMS, FALSE);
				}
			}
			// DebugPrintf ( -1 , "\nThrown item now blitted..." );
			break;
		case BLITTING_TYPE_MOVE_CURSOR:
			break;
		default:
			error_message(__FUNCTION__, "\
						The blitting list contained an illegal blitting object type.", PLEASE_INFORM | IS_FATAL);
			break;
		}
	}
	end_image_batch();
}

static void show_obstacle_labels(int mask)
{
	int i;
	level *l = CURLEVEL();

	if (game_status != INSIDE_LVLEDITOR) {
		// Don't show obstacles labels when we are not in the editor
		return;
	}

	if (mask & OMIT_OBSTACLES) {
		// Don't show obstacles labels when obstacles are not displayed on the map
		return;
	}

	for (i = 0; i < l->obstacle_extensions.size; i++) {
		struct obstacle_extension *ext = &ACCESS_OBSTACLE_EXTENSION(l->obstacle_extensions, i);
		
		if (ext->type == OBSTACLE_EXTENSION_LABEL) {
			show_backgrounded_label_at_map_position(ext->data,
					0, ext->obs->pos.x,
					ext->obs->pos.y, mask & ZOOM_OUT);
		}
	}
}

/**
 * Each item is lying on the floor.  But that means some of the items,
 * especially the smaller but not necessary less valuable items will not
 * be easy to make out under all the bushed, trees, rubble and stuff.
 * So the solution is to offer a special key that when pressed will make
 * all item names flash up, so that you can't possibly miss an item that
 * you're standing close to.
 *
 * This function blits all the item names to the screen on the exact
 * positions that have been computed before (hopefully!) in other 
 * functions like update_item_text_slot_positions ( ... ) or so.
 */
static void blit_all_item_slots(int mask)
{
	int i;
	struct visible_level *vis_lvl, *n;
	
	if (mask & OMIT_ITEMS_LABEL) {
		// Do not show item slots
		return;
	}

	BROWSE_VISIBLE_LEVELS(vis_lvl, n) {
		
		level *item_level = vis_lvl->lvl_pointer;

		for (i = 0; i < MAX_ITEMS_PER_LEVEL; i++) {
			// We don't work with unused item slots...
			//
			if (item_level->ItemList[i].type == (-1))
				continue;
	
			// Now we check if the cursor is on that slot, because then the
			// background of the slot will be highlighted...
			//
			if (MouseCursorIsInRect(&(item_level->ItemList[i].text_slot_rectangle), GetMousePos_x(), GetMousePos_y()))
				draw_rectangle(&item_level->ItemList[i].text_slot_rectangle, 0, 0, 153, 100);
			else {
				if ((item_level->ItemList[i].text_slot_rectangle.x + item_level->ItemList[i].text_slot_rectangle.w <= 0) ||
					(item_level->ItemList[i].text_slot_rectangle.y + item_level->ItemList[i].text_slot_rectangle.h <= 0) ||
					(item_level->ItemList[i].text_slot_rectangle.x >= GameConfig.screen_width) ||
					(item_level->ItemList[i].text_slot_rectangle.y >= GameConfig.screen_height))
					continue;

				draw_rectangle(&item_level->ItemList[i].text_slot_rectangle, 0, 0, 0, BACKGROUND_TEXT_RECT_ALPHA);
			}
	
			// Finally it's time to insert the font into the item slot.  We
			// use the item name, but currently font color is not adapted for
			// special item properties...
			//
			put_string(FPS_Display_BFont, item_level->ItemList[i].text_slot_rectangle.x,
					  item_level->ItemList[i].text_slot_rectangle.y, D_(item_specs_get_name(item_level->ItemList[i].type)));
	
		}
	}
};				// void blit_all_item_slots ( void )

/**
 *
 *
 */
int item_slot_position_blocked(item * given_item, int item_slot)
{
	int i;
	item *cur_item;
	struct visible_level *vis_lvl, *n;
	int item_level_reached = FALSE;
	int last_slot_to_check;
	
	// We will browse all visible levels, until we reach the item's level.
	// For each browsed level, we check against all items.
	// But, on the item's level, we stop when the current item is reached.
	
	BROWSE_VISIBLE_LEVELS(vis_lvl, n) {
		
		level *item_level = vis_lvl->lvl_pointer;

		if (item_level->levelnum == given_item->pos.z) {
			item_level_reached = TRUE;
			last_slot_to_check = item_slot;
		} else {
			last_slot_to_check = MAX_ITEMS_PER_LEVEL;
		}
		
		for (i = 0; i < last_slot_to_check + 1; i++) {
			cur_item = &(item_level->ItemList[i]);
	
			if (cur_item->type == (-1))
				continue;
	
			if (MouseCursorIsInRect(&(cur_item->text_slot_rectangle),
						given_item->text_slot_rectangle.x, given_item->text_slot_rectangle.y)) {
				return (TRUE);
			}
			if (MouseCursorIsInRect(&(cur_item->text_slot_rectangle),
						given_item->text_slot_rectangle.x,
						given_item->text_slot_rectangle.y + FontHeight(FPS_Display_BFont))) {
				return (TRUE);
			}
			if (MouseCursorIsInRect(&(cur_item->text_slot_rectangle),
						given_item->text_slot_rectangle.x +
						given_item->text_slot_rectangle.w, given_item->text_slot_rectangle.y)) {
				return (TRUE);
			}
			if (MouseCursorIsInRect(&(cur_item->text_slot_rectangle),
						given_item->text_slot_rectangle.x +
						given_item->text_slot_rectangle.w,
						given_item->text_slot_rectangle.y + FontHeight(FPS_Display_BFont))) {
				return (TRUE);
			}
			if (MouseCursorIsInRect(&(cur_item->text_slot_rectangle),
						given_item->text_slot_rectangle.x +
						given_item->text_slot_rectangle.w / 2, given_item->text_slot_rectangle.y)) {
				return (TRUE);
			}
			if (MouseCursorIsInRect(&(cur_item->text_slot_rectangle),
						given_item->text_slot_rectangle.x +
						given_item->text_slot_rectangle.w / 2,
						given_item->text_slot_rectangle.y + FontHeight(FPS_Display_BFont))) {
				return (TRUE);
			}
		}
		
		if (item_level_reached) 
			break;
	}
	
	return (FALSE);
};				// void item_slot_position_blocked ( int x , int y , int last_slot_to_check )

/**
 * Each item is lying on the floor.  But that means some of the items,
 * especially the smaller but not necessarily less valuable items will not
 * be easy to make out under all the bushed, trees, rubble and stuff.
 * So the solution is to offer a special key that when pressed will make
 * all item names flash up, so that you can't possibly miss an item that
 * you're standing close to.
 *
 * This function computes the best rectangles and positions for such 
 * item names to flash up.
 */
void update_item_text_slot_positions(void)
{
	int i;
	BFont_Info *BFont_to_use = FPS_Display_BFont;
	item *cur_item;
	struct visible_level *vis_lvl, *n;
	
	BROWSE_VISIBLE_LEVELS(vis_lvl, n) {
		
		level *item_level = vis_lvl->lvl_pointer;
	
		for (i = 0; i < MAX_ITEMS_PER_LEVEL; i++) {
			cur_item = &(item_level->ItemList[i]);
	
			if (cur_item->type == (-1))
				continue;
	
			// We try to use a text rectangle that is close to the
			// actual item...
			//
			update_virtual_position(&cur_item->virt_pos, &cur_item->pos, Me.pos.z);
			cur_item->text_slot_rectangle.h = FontHeight(BFont_to_use);
			cur_item->text_slot_rectangle.w = text_width(BFont_to_use, D_(item_specs_get_name(cur_item->type)));
			cur_item->text_slot_rectangle.x =
				translate_map_point_to_screen_pixel_x(cur_item->virt_pos.x, cur_item->virt_pos.y) - cur_item->text_slot_rectangle.w / 2;
			cur_item->text_slot_rectangle.y =
				translate_map_point_to_screen_pixel_y(cur_item->virt_pos.x, cur_item->virt_pos.y) - cur_item->text_slot_rectangle.h / 2;
	
			// But maybe the situation is already very crowded, i.e. maybe there are
			// already (a lot of) items there with slot positions conflicting...
			// Well, what to do?  If there is already an item there, we try to escape,
			// that's it.
			// We will however only try a given amount of times, to protect against
			// an "infinite" loop.
			if ((item_slot_position_blocked(cur_item, i - 1))) {
				int max_tries = 10;
				while (max_tries >= 0) {
					if (i % 2)
						cur_item->text_slot_rectangle.y += cur_item->text_slot_rectangle.h + 2;
					else
						cur_item->text_slot_rectangle.y -= cur_item->text_slot_rectangle.h + 2;

					if (!item_slot_position_blocked(cur_item, i - 1))
						break;
				    	
					// Maybe just a hundred left or right would also do...  but if it
					// doesn't, we'll undo the changes made.
					//
					Sint16 tmp = cur_item->text_slot_rectangle.x;
					
					cur_item->text_slot_rectangle.x += 100;
					if (!item_slot_position_blocked(cur_item, i - 1))
						break;
					
					cur_item->text_slot_rectangle.x = tmp - 100;
					if (!item_slot_position_blocked(cur_item, i - 1))
						break;
					
					cur_item->text_slot_rectangle.x = tmp;
					max_tries--;
				}
			}
		}
	}
};				// void update_item_text_slot_positions ( void )

void draw_grid_on_the_floor(int mask)
{
	if (game_status != INSIDE_LVLEDITOR)
		return;

	if (!GameConfig.show_grid)
		return;

	int LineStart, LineEnd, ColStart, ColEnd;
	float x, y;
	Level our_level = curShip.AllLevels[Me.pos.z];

	get_floor_boundaries(mask, &LineStart, &LineEnd, &ColStart, &ColEnd);

	x = rintf(Me.pos.x + 0.5);
	y = rintf(Me.pos.y + 0.5);

	if (x < 1)
		x = 1;
	if (x > our_level->xlen)
		x = our_level->xlen;
	if (y < 1)
		y = 1;
	if (y > our_level->ylen)
		y = our_level->ylen;

	float dd;

	if (GameConfig.grid_mode == 1) {	// large grid

		if (LineStart < 0)
			LineStart = 0;
		if (LineEnd > our_level->ylen)
			LineEnd = our_level->ylen;
		if (ColStart < 0)
			ColStart = 0;
		if (ColEnd > our_level->xlen)
			ColEnd = our_level->xlen;

		// Draw horizontal lines.
		for (dd = LineStart; dd <= LineEnd; dd++) {
			draw_line_on_map(ColStart, dd, ColEnd, dd, 0x99, 0xFF, 0xFF, 1);	// light cyan
		}

		// Draw vertical lines.
		for (dd = ColStart; dd <= ColEnd; dd++) {
			draw_line_on_map(dd, LineStart, dd, LineEnd, 0x99, 0xFF, 0xFF, 1);	// light cyan
		}
	}

	for (dd = 0; dd <= 1; dd += .5)	// quick-placement grid
	{
		draw_line_on_map(x - 1.5, y - dd, x + 0.5, y - dd, 0xFF, 0x00, 0xFF, 1);	// magenta
		draw_line_on_map(x - dd, y - 1.5, x - dd, y + 0.5, 0xFF, 0x00, 0xFF, 1);	// magenta
	}

	// Draw the level borders.
	draw_line_on_map(0, 0, 0, our_level->ylen, 0xFF, 0x00, 0x00, 3);
	draw_line_on_map(our_level->xlen, 0, our_level->xlen, our_level->ylen, 0xFF, 0x00, 0x00, 3);
	draw_line_on_map(0, 0, our_level->xlen, 0, 0xFF, 0x00, 0x00, 3);
	draw_line_on_map(0, our_level->ylen, our_level->xlen, our_level->ylen, 0xFF, 0x00, 0x00, 3);

	// display numbers, corresponding to the numpad keys for quick placing 
	BFont_Info *PreviousFont;
	PreviousFont = GetCurrentFont();
	SetCurrentFont(Messagevar_BFont);
	char *numbers[2][2] = { {"3", "9"}, {"1", "7"} };
	int ii, jj;
	for (ii = 0; ii <= 1; ii++)
		for (jj = 0; jj <= 1; jj++) {
			float xx, yy;
			int r, c;
			xx = x - ii;
			yy = y - jj;
			translate_map_point_to_screen_pixel(xx, yy, &r, &c);
			SDL_Rect tr;
			tr.x = r - 7;
			tr.y = c - 7;
			tr.w = 12;
			tr.h = 14;

			draw_rectangle(&tr, 0, 0, 0, 255);
			display_text(numbers[ii][jj], r - 5, c - 5, &tr);
		}
	SetCurrentFont(PreviousFont);
}

/* -----------------------------------------------------------------
 * This function assembles the contents of the combat window 
 * in Screen.
 *
 * Several FLAGS can be used to control its behavior:
 *
 * (*) ONLY_SHOW_MAP = 1:  This flag indicates not do draw any
 *     game elements but the map blocks
 *
 * (*) DO_SCREEN_UPDATE = 2: This flag indicates for the function
 *     to also cause an SDL_Update of the portion of the screen
 *     that has been modified
 *
 * (*) ONLY_SHOW_MAP_AND_TEXT = 4: This flag indicates, that only
 *     the map and also info like the current coordinate position
 *     should be entered into the Screen.  This flag is mainly
 *     used for the level editor.
 *
 * ----------------------------------------------------------------- */
void AssembleCombatPicture(int mask)
{
	clear_screen();
	clickable_obstacle_under_cursor =  clickable_obstacle_below_mouse_cursor(NULL);
	if ((!GameConfig.skip_light_radius) && (!(mask & SKIP_LIGHT_RADIUS))) {
		// We generate a list of obstacles (and other stuff) that might
		// emit some light.  It should be sufficient to establish this
		// list once in the code and the to use it for all light computations
		// of this frame.
		update_light_list();
	}

	show_floor(mask);

	draw_grid_on_the_floor(mask);

	set_up_ordered_blitting_list(mask);

	blit_preput_objects_according_to_blitting_list(mask);
	
	blit_nonpreput_objects_according_to_blitting_list(mask);

	if ((!GameConfig.skip_light_radius) && (!(mask & SKIP_LIGHT_RADIUS)))
		blit_light_radius();

	PutMiscellaneousSpellEffects();

	if (mask & ONLY_SHOW_MAP) {
		// in case we only draw the map, we are done here.  But
		// of course we must check if we should update the screen too.
		if (mask & DO_SCREEN_UPDATE)
			our_SDL_update_rect_wrapper(Screen, 0, 0, Screen->w, Screen->h);

		return;
	}

	show_obstacle_labels(mask);

	display_automap();

	if (XPressed() || GameConfig.show_item_labels) {
		update_item_text_slot_positions();
		blit_all_item_slots(mask);
	}

	// Here are some more things, that are not needed in the level editor
	// view...
	if (!(mask & ONLY_SHOW_MAP_AND_TEXT)) {
		display_widgets();
		if (!GameOver && !world_frozen())
			show_texts_and_banner();
	}

	if (GameConfig.Inventory_Visible || GameConfig.skill_explanation_screen_visible || addon_crafting_ui_visible()) {
		User_Rect.x = 320;
	} else
		User_Rect.x = 0;

	if (GameConfig.CharacterScreen_Visible || GameConfig.SkillScreen_Visible) {
		User_Rect.w = GameConfig.screen_width - 320 - User_Rect.x;
	} else {
		User_Rect.w = GameConfig.screen_width - User_Rect.x;
	}

#ifdef WITH_RTPROF
	rtprof_display();
#endif

	if (!(mask & NO_CURSOR))
		blit_mouse_cursor();

#if 0
	/* This code displays the player tracks with red dots. */
	glDisable(GL_TEXTURE_2D);
	glPointSize(2.0);
	glBegin(GL_POINTS);
	int i = 0;
	for (; i < MAX_INFLU_POSITION_HISTORY; i++) {
		int x, y;
		translate_map_point_to_screen_pixel(Me.Position_History_Ring_Buffer[i].x, Me.Position_History_Ring_Buffer[i].y, &x, &y);
		glColor3f(1.0, 0.0, 0.0);
		glVertex2i(x, y);
	}

	glEnd();
	glEnable(GL_TEXTURE_2D);
#endif

#if 0
	/* This code displays tux "waypoints" */
	glDisable(GL_TEXTURE_2D);
	glLineWidth(2.0);
	glBegin(GL_LINE_STRIP);
	i = 0;
	int x, y;
	translate_map_point_to_screen_pixel(Me.pos.x, Me.pos.y, &x, &y);
	glColor3f(0.0, 1.0, 0.0);
	glVertex2i(x, y);
	while (Me.next_intermediate_point[i].x != -1) {
		translate_map_point_to_screen_pixel(Me.next_intermediate_point[i].x, Me.next_intermediate_point[i].y, &x, &y);
		glColor3f(0.0, 1.0, 0.0);
		glVertex2i(x, y);
		i++;
	}

	glEnd();
	glEnable(GL_TEXTURE_2D);
#endif

	// At this point we are done with the drawing procedure
	// and all that remains to be done is updating the screen.
	//
	if (mask & DO_SCREEN_UPDATE) {
		our_SDL_update_rect_wrapper(Screen, 0, 0, Screen->w, Screen->h);
	}

	if (GameConfig.limit_framerate)
		SDL_framerateDelay(&SDL_FPSmanager);
}

/* -----------------------------------------------------------------
 * This function draws the mouse move cursor.
 * ----------------------------------------------------------------- */
void PutMouseMoveCursor(void)
{
	SDL_Rect TargetRectangle;

	if ((Me.mouse_move_target.x == (-1)) && (enemy_resolve_address(Me.current_enemy_target_n, &Me.current_enemy_target_addr) == NULL)) {
		return;
	}

	if (Me.mouse_move_target.x != (-1)) {
		TargetRectangle.x = translate_map_point_to_screen_pixel_x(Me.mouse_move_target.x, Me.mouse_move_target.y);
		TargetRectangle.y = translate_map_point_to_screen_pixel_y(Me.mouse_move_target.x, Me.mouse_move_target.y);
		TargetRectangle.x -= MouseCursorImageList[0].w / 2;
		TargetRectangle.y -= MouseCursorImageList[0].h / 2;
		display_image_on_screen(&MouseCursorImageList[0], TargetRectangle.x, TargetRectangle.y, IMAGE_NO_TRANSFO);
	}

	enemy *t = enemy_resolve_address(Me.current_enemy_target_n, &Me.current_enemy_target_addr);
	if (t != NULL) {
		// translate_map_point_to_screen_pixel ( float x_map_pos , float y_map_pos , int give_x )
		update_virtual_position(&t->virt_pos, &t->pos, Me.pos.z);

		TargetRectangle.x = translate_map_point_to_screen_pixel_x(t->virt_pos.x, t->virt_pos.y);
		TargetRectangle.y = translate_map_point_to_screen_pixel_y(t->virt_pos.x, t->virt_pos.y);
		TargetRectangle.x -= MouseCursorImageList[1].w / 2;
		TargetRectangle.y -= MouseCursorImageList[1].h / 2;
		display_image_on_screen(&MouseCursorImageList[1], TargetRectangle.x, TargetRectangle.y, IMAGE_NO_TRANSFO);
	}
}

/**
 *
 *
 */
static void free_one_loaded_tux_image_series(int motion_class, int tux_part_group)
{
	int i, j;

	tux_images[motion_class].part_names[tux_part_group][0] = '\0';

	for (i = 0; i < TUX_TOTAL_PHASES; i++) {
		for (j = 0; j < MAX_TUX_DIRECTIONS; j++) {
			delete_image(&tux_images[motion_class].part_images[tux_part_group][i][j]);
		}
	}
}

/**
 * This function should blit the isometric version of the Tux to the
 * screen.
 */
static void iso_put_tux_part(struct tux_part_render_data *render_data, int x, int y, int motion_class, int our_phase, int rotation_index)
{
	char *part_string = *(render_data->default_part_instance);
	int tux_part_group = render_data->part_group;

	// If there is an equipped item, get its animation prefix name
	if (render_data->wearable_item && render_data->wearable_item->type != -1 &&
			render_data->wearable_item != item_held_in_hand &&
			ItemMap[render_data->wearable_item->type].tux_part_instance) {
		part_string = ItemMap[render_data->wearable_item->type].tux_part_instance;
	}

	// Some parts could be 'empty' (weapon, currently)
	if (!part_string || !strlen(part_string))
		return;

	// If some part string given is unlike the part string we were using so
	// far, then we'll need to free that old part and (later) load the new
	// part.
	//
	if (strcmp(tux_images[motion_class].part_names[tux_part_group], part_string)) {
		free_one_loaded_tux_image_series(motion_class, tux_part_group);
		load_tux_graphics(motion_class, tux_part_group, part_string);
		strcpy(tux_images[motion_class].part_names[tux_part_group], part_string);
		// It can be expected, that this operation HAS TAKEN CONSIDERABLE TIME!
		// Therefore we must activate the conservative frame time compution now,
		// so as to prevent any unwanted jumps right now...
		//
		Activate_Conservative_Frame_Computation();
	}

	float r = 1.0, g = 1.0, b = 1.0, a = 1.0;

	if (Me.paralyze_duration) {	/* Paralyzed ? tux turns red */
		g = 0.2;
		b = 0.2;
	} else if (Me.slowdown_duration) {	/* Slowed down ? tux turns blue */
		r = 0.2;
		g = 0.2;
	} else if (Me.energy < Me.maxenergy * 0.25) {	/* Low energy ? blink red */
		g = b = ((SDL_GetTicks() >> 5) & 31) * 0.03;
	}

	if (Me.invisible_duration) {	/* Invisible? Become transparent */
		a = 0.5;
	}

	struct image *img = &tux_images[motion_class].part_images[tux_part_group][our_phase][rotation_index];

	if (x == -1) {
		display_image_on_map(img, Me.pos.x, Me.pos.y, set_image_transformation(1.0, 1.0, r, g, b, a, 0));
	} else {
		display_image_on_screen(img, x, y, IMAGE_NO_TRANSFO);
	}
}

/**
 * Given the name of a Tux's part, returns the data needed to render it.
 *
 * \param part_name A Tux's part name
 * \return A pointer the static tux_part_render_data which contains the data needed to render the part
 */
struct tux_part_render_data *tux_get_part_render_data(char *part_name)
{
	int i;
	static struct tux_part_render_data render_data[] = {
		{  "head",      &tux_rendering.default_instances.head,      &Me.special_item, PART_GROUP_HEAD },
		{  "torso",     &tux_rendering.default_instances.torso,     &Me.armour_item,  PART_GROUP_TORSO },
		{  "weaponarm", &tux_rendering.default_instances.weaponarm, NULL,             PART_GROUP_WEAPONARM },
		{  "weapon",    &tux_rendering.default_instances.weapon,    &Me.weapon_item,  PART_GROUP_WEAPON },
		{  "shieldarm", &tux_rendering.default_instances.shieldarm, &Me.shield_item,  PART_GROUP_SHIELD },
		{  "feet",      &tux_rendering.default_instances.feet,      &Me.drive_item,   PART_GROUP_FEET }
	};

	if (part_name == NULL)
		return NULL;

	for (i = 0; i < sizeof(render_data)/sizeof(render_data[0]); i++) {
		if (!strcmp(part_name, render_data[i].name)) {
			return &render_data[i];
		}
	}

	return NULL;
}

/**
  * Initialize Tux's part rendering specification's data structures
  */
static void tux_rendering_init()
{
	dynarray_init(&tux_rendering.motion_class_names, 0, 0);
	memset(&tux_rendering.default_instances, 0, sizeof(struct tux_part_instances));
	tux_rendering.render_order = NULL;
}

/**
 * Check mandatory specifications, needed to ensure that Tux can be rendered.
 */
static void tux_rendering_validate()
{
	// At least one motion_class is needed
	if (tux_rendering.motion_class_names.size < 1) {
		error_message(__FUNCTION__,
			"Tux rendering specification is invalid: at least one motion_class is needed",
			PLEASE_INFORM | IS_FATAL);
	}

	// There must be a rendering order defined for each motion class, each rotation
	// and each animation phase
	int i, j;
	for (i = 0; i < tux_rendering.motion_class_names.size; i++) {
		for (j = 0; j < MAX_TUX_DIRECTIONS; j++) {
			if (tux_rendering.render_order[i][j] == NULL) {
				error_message(__FUNCTION__,
					"Tux rendering specification is invalid: no rendering order defined for motion_class \"%s\""
					" and rotation index %d",
					PLEASE_INFORM | IS_FATAL,
					get_motion_class_name_by_id(i), j);
			} else {
				// Check that there is a rendering order for every animation phases
				// Set a flag for each phase found, and then check that all flags are set
				int phase;
				int check_phase[TUX_TOTAL_PHASES];
				memset(check_phase, 0, TUX_TOTAL_PHASES*sizeof(int));

				struct tux_part_render_set *render_order_ptr = tux_rendering.render_order[i][j];
				while (render_order_ptr != NULL) {
					for (phase = render_order_ptr->phase_start; phase <= render_order_ptr->phase_end; phase++) {
						check_phase[phase] = 1;
					}
					render_order_ptr = render_order_ptr->next;
				}

				for (phase = 0; phase < TUX_TOTAL_PHASES; phase++) {
					if (check_phase[phase] == 0) {
						error_message(__FUNCTION__,
							"Tux rendering specification is invalid: no rendering order defined for motion_class \"%s\","
							" rotation index %d and animation phase %d",
							PLEASE_INFORM | IS_FATAL,
							get_motion_class_name_by_id(i), j, phase);
					}
				}
			}
		}
	}
}

/**
 * Load Tux animation and rendering specifications.
 */
void tux_rendering_load_specs(const char *config_filename)
{
	char fpath[PATH_MAX];

	tux_rendering_init();

	find_file(config_filename, MAP_DIR, fpath, PLEASE_INFORM | IS_FATAL);
	run_lua_file(LUA_CONFIG, fpath);
	tux_rendering_validate(); // check mandatory specifications/configurations

	tux_images = MyMalloc(tux_rendering.motion_class_names.size * sizeof(struct tux_motion_class_images));
}

/**
 * Returns the id (index number) of a motion_class, given its name
 *
 * \param name A motion_class name
 * \return Id associated to the motion_class name or -1 if 'name' is invalid or unknown
 */
int get_motion_class_id_by_name(char *name)
{
	// Check pre-condition
	if (name == NULL)
		return -1;

	// Search 'name' in the motion_class_names list
	int i;
	for (i = 0; i < tux_rendering.motion_class_names.size; i++) {
		if (!strcmp(name, ((char **)tux_rendering.motion_class_names.arr)[i]))
			return i;
	}

	// Fall-back if 'name' is not found
	return -1;
}

/**
 * Returns the name of a motion_class, given its id (index number).
 *
 * \param id A motion_class id
 * \return The name of the motion_class associated to 'id', or the first available motion_class if 'id' is invalid (not in range)
 */
char *get_motion_class_name_by_id(int id)
{
	// Check pre-condition. Returns first name if id is invalid
	if ((id < 0) || (id >= tux_rendering.motion_class_names.size))
		return ((char **)tux_rendering.motion_class_names.arr)[0];

	// Return the motion_class name
	return ((char **)tux_rendering.motion_class_names.arr)[id];
}

/**
 * Returns Tux's current motion_class id, which depends on its weapon.
 *
 * \return The current motion_class's id
 */
int get_motion_class_id()
{
	int motion_class = 0;

	// It Tux has a weapon in hand, return its motion class.
	// Otherwise, return the first motion_class.
	if (Me.weapon_item.type != -1)
		motion_class = ItemMap[Me.weapon_item.type].weapon_motion_class;

	return motion_class;
}

/*
 * This function blits the isometric version of the Tux to the screen
 */
void iso_put_tux(int x, int y)
{
	// Compute the 3 parameters defining how to render the animated Tux:
	// - motion_class: type of animation (sword_motion/gun_motion)
	// - our_phase: phase of animation (i.e. "animation keyframe number")
	// - rotation_index: Tux's rotation index
	//
	// When Tux is not animated (in the takeover minigame, for instance), that
	// is when x != -1, then we set those parameters to default values (apart
	// from motion_class.

	int motion_class = get_motion_class_id();
	int our_phase = (int)Me.phase; // Tux current animation keyframe
	int rotation_index = 0; // Facing front

	if (x == -1) {
		// Compute the rotation angle
		// If Tux is walking/running, compute its angle from the direction of movement,
		// else reuse the last computed angle.
		float angle = Me.angle;

		if ((Me.phase < tux_anim.attack.first_keyframe) || (Me.phase > tux_anim.attack.last_keyframe)) {
			if (fabsf(Me.speed.x) + fabsf(Me.speed.y) > 0.1) {
				angle = -(atan2(Me.speed.y, Me.speed.x) * 180 / M_PI - 45 - 180);
				angle += 360 / (2 * MAX_TUX_DIRECTIONS);
				while (angle < 0)
					angle += 360;
				Me.angle = angle; // Store computed angle for later use
			}
		}

		// Compute the rotation index from the rotation angle
		rotation_index = (angle * MAX_TUX_DIRECTIONS) / 360.0 + (MAX_TUX_DIRECTIONS / 2);
		while (rotation_index >= MAX_TUX_DIRECTIONS)
			rotation_index -= MAX_TUX_DIRECTIONS;
		while (rotation_index < 0)
			rotation_index += MAX_TUX_DIRECTIONS;
	}

	// Depending on the rendering parameters, get the right rendering order
	// Loop on all rendering orders associated to the current motion_class and
	// rotation, until one set containing the current animation phase is found
	struct tux_part_render_set *one_render_set = tux_rendering.render_order[motion_class][rotation_index];

	while (one_render_set != NULL) {
		if (our_phase >= one_render_set->phase_start && our_phase <= one_render_set->phase_end) {
			break;
		}
		one_render_set = one_render_set->next;
	}

	// Render all Tux's parts in order
	int i;
	for (i = 0; i < 6; i++) {
		if (one_render_set && one_render_set->part_render_data[i]) {
			iso_put_tux_part(one_render_set->part_render_data[i], x, y, motion_class, our_phase, rotation_index);
		}
	}
}

/* -----------------------------------------------------------------
 * This function draws the influencer to the screen, either
 * to the center of the combat window if (-1,-1) was specified, or
 * to the specified coordinates anywhere on the screen, useful e.g.
 * for drawing the influencer in the takeover minigame.
 *
 * The given coordinates then indicate the UPPER LEFT CORNER for
 * the blit.
 * ----------------------------------------------------------------- */
void blit_tux(int x, int y)
{
	SDL_Rect Text_Rect;

	Text_Rect.x = UserCenter_x + 21;
	Text_Rect.y = UserCenter_y - 32;
	Text_Rect.w = (User_Rect.w / 2) - 21;
	Text_Rect.h = (User_Rect.h / 2);

	// Draw Tux
	iso_put_tux(x, y);

	// Maybe the influencer has something to say :)
	// so let him say it..
	if ((x == -1) && Me.TextToBeDisplayed && (Me.TextVisibleTime < GameConfig.WantedTextVisibleTime) &&
		GameConfig.All_Texts_Switch) {
		SetCurrentFont(FPS_Display_BFont);
		display_text(Me.TextToBeDisplayed, Text_Rect.x, Text_Rect.y, &Text_Rect);
	}
}

/**
 * If the corresponding configuration flag is enabled, enemies might 'say'
 * some text comment on the screen, like 'ouch' or 'i'll getch' or 
 * something else more sensible.  This function is here to blit these
 * comments, that must have been set before, to the screen.
 */
void PrintCommentOfThisEnemy(enemy * e)
{
	int x_pos, y_pos;

	if (game_status != INSIDE_GAME) {
		// Do not print bot states when we are not inside the game.
		return;
	}

	// At this point we can assume, that the enemys has been blittet to the
	// screen, whether it's a friendly enemy or not.
	// 
	// So now we can add some text the enemys says.  That might be fun.
	//
	if (!(e->TextToBeDisplayed))
		return;
	if ((e->TextVisibleTime < GameConfig.WantedTextVisibleTime)
	    && GameConfig.All_Texts_Switch) {
		x_pos = translate_map_point_to_screen_pixel_x(e->virt_pos.x, e->virt_pos.y);
		y_pos = translate_map_point_to_screen_pixel_y(e->virt_pos.x, e->virt_pos.y)
		    - 100;

		// First we display the normal text to be displayed...
		//
#	if 0
		char txt[256];
		sprintf(txt, "%d - %s", e->id, e->TextToBeDisplayed);
		put_string(FPS_Display_BFont, x_pos, y_pos, txt);
#	else
		put_string(FPS_Display_BFont, x_pos, y_pos, e->TextToBeDisplayed);
#	endif
	}

}

/**
 * Not every enemy has to be blitted onto the combat screen every time.
 * This function is here to find out whether this enemy has to be blitted
 * or whether we can skip it.
 */
static int must_blit_enemy(enemy *e)
{
	// if enemy is on other level, return 
	if (e->virt_pos.z != Me.pos.z) {
		return FALSE;
	}
	// if enemy is of type (-1), return 
	if (e->type == (-1)) {
		return FALSE;
	}
	// if the enemy is dead, show him anyways
	if (e->animation_type == DEAD_ANIMATION)
		return TRUE;
	// if the enemy is out of sight, we need not do anything more here
	if (game_status != INSIDE_LVLEDITOR && (!show_all_droids) && (!IsVisible(&e->virt_pos))) {
		return FALSE;
	}

	return TRUE;
}

/**
 *
 *
 */
void PutEnemyEnergyBar(enemy *e, SDL_Rect TargetRectangle)
{
	float Percentage;
	SDL_Rect FillRect;

#define ENEMY_ENERGY_BAR_WIDTH 7

	// If the enemy is dead already, there's nothing to do here...
	//
	if (e->energy <= 0)
		return;

	// work out the percentage health
	//
	Percentage = (e->energy) / Droidmap[e->type].maxenergy;
	if (use_open_gl) {

#ifdef HAVE_LIBGL
		int x, y, w;
		myColor c1 = { 0, 0, 0, 255 };
		myColor c2 = { 0, 0, 0, 255 };
		float PercentageDone = 0;
		int barnum = 0;
		// If Percentage > 100%, several bars are drawn (one bar == 100% of maxenergy)
		for (; Percentage > 0; Percentage -= PercentageDone, barnum++) {
			if (Percentage >= 1)
				PercentageDone = 1;
			else
				PercentageDone = Percentage;
			// draw cool bars here
			x = TargetRectangle.x;
			y = TargetRectangle.y - 10 * barnum;
			w = TargetRectangle.w;

			if (is_friendly(e->faction, FACTION_SELF))
				c1.g = 255;
			else
				c1.r = 255;

			// tweak as needed, this alters the transparency
			c1.a = 140;
			drawIsoEnergyBar(x, y, 1, 5, 5, w, PercentageDone, &c1, &c2);
		}

#endif

	} else {
		//sdl stuff here

		// Calculates the width of the remaining health bar. Rounds the
		// width up to the nearest integer to ensure that at least one
		// pixel of health is always shown.
		//
		int health_pixels = (int) ceil(Percentage * TargetRectangle.w);

		FillRect.x = TargetRectangle.x;
		FillRect.y = TargetRectangle.y - ENEMY_ENERGY_BAR_WIDTH;
		FillRect.h = ENEMY_ENERGY_BAR_WIDTH;
		FillRect.w = health_pixels;

		// The color of the bar depends on the friendly/hostile status
		if (is_friendly(e->faction, FACTION_SELF))
			sdl_draw_rectangle(&FillRect, 0, 255, 0, 140);
		else
			sdl_draw_rectangle(&FillRect, 255, 0, 0, 140);

		// Now after the energy bar has been drawn, we can start to draw the
		// empty part of the energy bar (but only of course, if there is some
		// empty part at all! 
		FillRect.x = TargetRectangle.x + health_pixels;
		FillRect.w = TargetRectangle.w - health_pixels;

		if (Percentage < 1.0)
			sdl_draw_rectangle(&FillRect, 0, 0, 0, 255);
	}
}

/**
 * The direction this robot should be facing right now is determined and
 * properly set in this function.
 */
int set_rotation_index_for_this_robot(enemy * ThisRobot)
{
	int RotationIndex;

	// By now the angle the robot is facing is determined, so we just need to
	// translate this angle into an index within the image series, i.e. into 
	// a 'phase' of rotation. 
	//
	RotationIndex = ((ThisRobot->current_angle - 45.0 + 360.0 + 360 /
			  (2 * ROTATION_ANGLES_PER_ROTATION_MODEL)) * ROTATION_ANGLES_PER_ROTATION_MODEL / 360);

	// But it might happen, that the angle of rotation is 'out of scale' i.e.
	// it's more than 360 degree or less than 0 degree.  Therefore, we need to
	// be especially careful to generate only proper indices for our arrays.
	// Some would say, we identify the remainder classes with integers in the
	// range [ 0 - (rotation_angles-1) ], which is what's happening here.
	//
	while (RotationIndex < 0)
		RotationIndex += ROTATION_ANGLES_PER_ROTATION_MODEL;
	while (RotationIndex >= ROTATION_ANGLES_PER_ROTATION_MODEL)
		RotationIndex -= ROTATION_ANGLES_PER_ROTATION_MODEL;

	// Now to prevent some jittering in some cases, where the droid uses an angle that is
	// right at the borderline between two possible 8-way directions, we introduce some
	// enforced consistency onto the droid...
	//
	if (RotationIndex == ThisRobot->previous_phase) {
		ThisRobot->last_phase_change += Frame_Time();
	} else {
		if (ThisRobot->last_phase_change >= WAIT_BEFORE_ROTATE) {
			ThisRobot->last_phase_change = 0.0;
			ThisRobot->previous_phase = RotationIndex;
		} else {
			// In this case we don't permit to use a new 8-way direction now...
			//
			RotationIndex = ThisRobot->previous_phase;
			ThisRobot->last_phase_change += Frame_Time();
		}
	}

	// DebugPrintf ( 0 , "\nCurrent angle: %f Current RotationIndex: %d. " , angle, RotationIndex );

	return (RotationIndex);

};				// int set_rotation_index_for_this_robot ( enemy* ThisRobot ) 

/**
 *
 *
 */
int set_rotation_model_for_this_robot(enemy * ThisRobot)
{
	int RotationModel = Droidmap[ThisRobot->type].individual_shape_nr;

	// A sanity check for roation model to use can never hurt...
	//
	if ((RotationModel < 0) || (RotationModel >= ENEMY_ROTATION_MODELS_AVAILABLE)) {
		error_message(__FUNCTION__, "\
There was a rotation model type given, that exceeds the number of rotation models allowed and loaded in FreedroidRPG.", PLEASE_INFORM | IS_FATAL);
	}

	return (RotationModel);

};				// int set_rotation_model_for_this_robot ( enemy* ThisRobot ) 

/**
 * This function is here to blit the 'body' of a droid to the screen.
 */
void PutIndividuallyShapedDroidBody(enemy * ThisRobot, SDL_Rect TargetRectangle, int mask, int highlight)
{
	int RotationModel;
	int RotationIndex;
	moderately_finepoint bot_pos;
	float zf = 1.0;
	if (mask & ZOOM_OUT)
		zf = lvledit_zoomfact_inv();

	// if ( ThisRobot -> pos . z != Me . pos . z )
	// DebugPrintf ( -4 , "\n%s(): Now attempting to blit bot on truly virtual position..." , __FUNCTION__ );

	// We properly set the direction this robot is facing.
	//
	RotationIndex = set_rotation_index_for_this_robot(ThisRobot);

	// We properly set the rotation model number for this robot, i.e.
	// which shape (like 302, 247 or proffa) to use for drawing this bot.
	//
	RotationModel = set_rotation_model_for_this_robot(ThisRobot);

	// Maybe the rotation model we're going to use now isn't yet loaded. 
	// Now in this case, we must load it immediately, or a segfault may
	// result...
	//
	LoadAndPrepareEnemyRotationModelNr(RotationModel);

	// Maybe we don't have an enemy here that would really stick to the 
	// exact size of a block but be somewhat bigger or smaller instead.
	// In this case, we'll just adapt the given target rectangle a little
	// bit, cause this rectangle assumes exactly the same size as a map 
	// block and has the origin shifted accordingly.
	//
	if ((TargetRectangle.x != 0) && (TargetRectangle.y != 0)) {
		if (use_open_gl) {
			TargetRectangle.x -= (enemy_images[RotationModel][RotationIndex][0].w) / 2;
			TargetRectangle.y -= (enemy_images[RotationModel][RotationIndex][0].h) / 2;
			TargetRectangle.w = enemy_images[RotationModel][RotationIndex][0].w;
			TargetRectangle.h = enemy_images[RotationModel][RotationIndex][0].h;
		} else {
			TargetRectangle.x -= (enemy_images[RotationModel][RotationIndex][0].surface->w) / 2;
			TargetRectangle.y -= (enemy_images[RotationModel][RotationIndex][0].surface->h) / 2;
			TargetRectangle.w = enemy_images[RotationModel][RotationIndex][0].surface->w;
			TargetRectangle.h = enemy_images[RotationModel][RotationIndex][0].surface->h;
		}
	}
	// Maybe the enemy is desired e.g. for the takeover game, so a pixel position on
	// the screen is given and we blit the enemy to that position, not taking into 
	// account any map coordinates or stuff like that...
	//
	if ((TargetRectangle.x != 0) && (TargetRectangle.y != 0)) {
		RotationIndex = 0;
		display_image_on_screen(&enemy_images[RotationModel][RotationIndex][0],
										  TargetRectangle.x, TargetRectangle.y, IMAGE_NO_TRANSFO);
		return;
	}

	float r, g, b;

	object_vtx_color(ThisRobot, &r, &g, &b);

	if (ThisRobot->paralysation_duration_left != 0) {
		g = 0.2;
		b = 0.2;
	} else if (ThisRobot->poison_duration_left != 0) {
		r = 0.2;
		b = 0.2;
	} else if (ThisRobot->frozen != 0) {
		r = 0.2;
		g = 0.2;
	}

	bot_pos.x = ThisRobot->virt_pos.x;
	bot_pos.y = ThisRobot->virt_pos.y;

	struct image *img = &enemy_images[RotationModel][RotationIndex][(int)ThisRobot->animation_phase];
	display_image_on_map(img, bot_pos.x, bot_pos.y, set_image_transformation(zf, zf, r, g, b, 1.0, highlight));

	if (GameConfig.enemy_energy_bars_visible) {
		int screen_x, screen_y;
		translate_map_point_to_screen_pixel(ThisRobot->virt_pos.x, ThisRobot->virt_pos.y, &screen_x, &screen_y);

		int bar_width = min(enemy_images[RotationModel][RotationIndex][0].w, ENERGYBAR_MAXWIDTH);
		TargetRectangle.x = screen_x - (bar_width * zf) / 2;
		TargetRectangle.y = screen_y - (enemy_images[RotationModel][RotationIndex][0].h * zf) / 1;
		TargetRectangle.w = bar_width * zf;
		TargetRectangle.h = enemy_images[RotationModel][RotationIndex][0].h * zf;
		PutEnemyEnergyBar(ThisRobot, TargetRectangle);
	}
}

/**
 * This function draws an enemy into the combat window.
 * The only parameter given is the number of the enemy within the
 * AllEnemys array. Everything else is computed in here.
 */
void PutEnemy(enemy * e, int x, int y, int mask, int highlight)
{
	SDL_Rect TargetRectangle = { 0, 0, 0, 0 };

	// We check for things like visibility and distance and the like,
	// so that we know whether to consider this enemy for blitting to
	// the screen or not.  Since there are many things to consider, we
	// got a special function for this job.
	//
	if ((!must_blit_enemy(e)) && (!GameConfig.xray_vision_for_tux))
		return;

	// We check for incorrect droid types, which sometimes might occur, especially after
	// heavy editing of the crew initialization functions ;)
	//
	if (e->type >= Number_Of_Droid_Types) {
		error_message(__FUNCTION__, "\
There was a droid type on this level, that does not really exist.", PLEASE_INFORM | IS_FATAL);
		e->type = 0;
	}
	// Since we will need that several times in the sequel, we find out the correct
	// target location on the screen for our surface blit once and remember it for
	// later.  ( THE TARGET RECTANGLE GETS MODIFIED IN THE SDL BLIT!!! )
	//
	if (x == (-1)) {
		TargetRectangle.x = 0;
		TargetRectangle.y = 0;
	} else {
		TargetRectangle.x = x;
		TargetRectangle.y = y;
	}

	PutIndividuallyShapedDroidBody(e, TargetRectangle, mask, highlight);

#if 0
	/* This code displays the pathway of the bots as well as their next waypoint */
	if (e->energy > 0) {
		glDisable(GL_TEXTURE_2D);
		glLineWidth(2.0);
		int a, b;
		glBegin(GL_LINE_STRIP);
		glColor3f(0.0, 1.0, 1.0);
		translate_map_point_to_screen_pixel(e->pos.x, e->pos.y, &a, &b, 1.0);
		glVertex2i(a, b);
		translate_map_point_to_screen_pixel(curShip.AllLevels[e->pos.z]->AllWaypoints[e->nextwaypoint].x + 0.5,
						    curShip.AllLevels[e->pos.z]->AllWaypoints[e->nextwaypoint].y + 0.5, &a, &b, 1.0);
		glVertex2i(a, b);
		glEnd();
		int aue = 0;
		glBegin(GL_LINE_STRIP);
		translate_map_point_to_screen_pixel(e->pos.x, e->pos.y, &a, &b, 1.0);
		glColor3f(0.0, 0.0, 1.0);
		glVertex2i(a, b);
		for (; aue < 5 && e->PrivatePathway[aue].x != -1; aue++) {
			translate_map_point_to_screen_pixel(e->PrivatePathway[aue].x, e->PrivatePathway[aue].y, &a, &b, 1.0);
			glVertex2i(a, b);
		}
		glEnd();
		glEnable(GL_TEXTURE_2D);
	}
#endif

	// Only if this robot is not dead, we consider printing the comments
	// this robot might have to make on the current situation.
	//
	if (e->energy > 0)
		PrintCommentOfThisEnemy(e);

};				// void PutEnemy(int Enum , int x , int y) 

/**
 * This function draws a Bullet into the combat window.  The only 
 * parameter given is the number of the bullet in the AllBullets 
 * array. Everything else is computed in here.
 */
void PutBullet(int bullet_index, int mask)
{
	bullet *CurBullet = &(AllBullets[bullet_index]);
	int PhaseOfBullet;
	int direction_index;

	// DebugPrintf( 0 , "\nBulletType before calculating phase : %d." , CurBullet->type );
	if ((CurBullet->type >= bullet_specs.size) || (CurBullet->type < 0)) {
		fprintf(stderr, "\nPutBullet:  bullet type received: %d.", CurBullet->type);
		fflush(stderr);
		error_message(__FUNCTION__, "\
There was a bullet to be blitted of a type that does not really exist.", PLEASE_INFORM | IS_FATAL);
	}

	struct bulletspec *bullet_spec = dynarray_member(&bullet_specs, CurBullet->type, sizeof(struct bulletspec));

	PhaseOfBullet = CurBullet->time_in_seconds * bullet_spec->phase_changes_per_second;

	PhaseOfBullet = PhaseOfBullet % bullet_spec->phases;
	// DebugPrintf( 0 , "\nPhaseOfBullet: %d.", PhaseOfBullet );

	direction_index = ((CurBullet->angle + 360.0 + 360 / (2 * BULLET_DIRECTIONS)) * BULLET_DIRECTIONS / 360);
	while (direction_index < 0)
		direction_index += BULLET_DIRECTIONS;	// just to make sure... a modulo ROTATION_ANGLES_PER_ROTATION_MODEL operation can't hurt
	while (direction_index >= BULLET_DIRECTIONS)
		direction_index -= BULLET_DIRECTIONS;	// just to make sure... a modulo ROTATION_ANGLES_PER_ROTATION_MODEL operation can't hurt

	// draw position is relative to current level, so compute the appropriate virtual position
	gps vpos;
	update_virtual_position(&vpos, &CurBullet->pos, Me.pos.z);
	if (vpos.x == -1)
		return;

	float scale = 1.0;
	if (mask & ZOOM_OUT)
		scale = lvledit_zoomfact_inv();
	if (IsVisible(&vpos)) {
		struct image *bullet_image = &bullet_spec->image[direction_index][PhaseOfBullet];
		bullet_image->offset_y -= CurBullet->height;
		display_image_on_map(bullet_image, vpos.x, vpos.y, IMAGE_SCALE_TRANSFO(scale));
		bullet_image->offset_y += CurBullet->height;
	}
}

/**
 * This function draws an item into the combat window.
 * The only given parameter is the number of the item within
 * the AllItems array.
 */
void PutItem(item *CurItem, int mask, int put_thrown_items_flag, int highlight_item)
{
	float r, g, b;
	float zf = 1.0;

	if (mask & ZOOM_OUT)
		zf = lvledit_zoomfact_inv();

	// The unwanted cases MUST be handled first...
	//
	if (CurItem->type == (-1)) {
		error_message(__FUNCTION__, "Tried to draw an item of type -1 (item %p on level %d). Ignoring.", PLEASE_INFORM, CurItem, CurItem->pos.z);
		return;
	}

	// We don't blit any item, that we're currently holding in our hand, do we?
	if (item_held_in_hand == CurItem)
		return;

	// In case the flag filters this item, we don't blit it
	//
	if ((put_thrown_items_flag == PUT_ONLY_THROWN_ITEMS) && (CurItem->throw_time <= 0))
		return;
	if ((put_thrown_items_flag == PUT_NO_THROWN_ITEMS) && (CurItem->throw_time > 0))
		return;

	struct image *img = get_item_ingame_image(CurItem->type);

	// Apply disco mode when current item is selected
	object_vtx_color(CurItem, &r, &g, &b);

	float anim_throw = (CurItem->throw_time <= 0) ? 0.0 : (3.0 * sinf(CurItem->throw_time * 3.0));

	display_image_on_map(img, CurItem->virt_pos.x - anim_throw, CurItem->virt_pos.y - anim_throw, set_image_transformation(zf, zf, r, g, b, 1.0, highlight_item));
}

void PutRadialBlueSparks(float PosX, float PosY, float Radius, int SparkType, uint8_t active_direction[RADIAL_SPELL_DIRECTIONS], float age)
{
#define FIXED_NUMBER_OF_SPARK_ANGLES 12
#define FIXED_NUMBER_OF_PROTOTYPES 4
#define NUMBER_OF_SPARK_TYPES 3

	SDL_Rect TargetRectangle;
	static SDL_Surface *SparkPrototypeSurface[NUMBER_OF_SPARK_TYPES][FIXED_NUMBER_OF_PROTOTYPES] =
	    { {NULL, NULL, NULL, NULL}, {NULL, NULL, NULL, NULL} };
	static struct image PrerotatedSparkSurfaces[NUMBER_OF_SPARK_TYPES][FIXED_NUMBER_OF_PROTOTYPES][FIXED_NUMBER_OF_SPARK_ANGLES];
	SDL_Surface *tmp_surf;
	char fpath[PATH_MAX];
	int NumberOfPicturesToUse;
	int i, k;
	float Angle;
	int PrerotationIndex;
	moderately_finepoint Displacement;
	int PictureType;
	char ConstructedFilename[5000];
	int current_active_direction;

	// We do some sanity check against too small a radius
	// given as parameter.  This can be loosened later.
	//
	if (Radius <= 1.0)
		return;

	PictureType = (int)(4 * age) % 4;

	// Now if we do not yet have all the prototype images in memory,
	// we need to load them now and for once...
	//
	if (SparkPrototypeSurface[SparkType][0] == NULL) {
		for (k = 0; k < FIXED_NUMBER_OF_PROTOTYPES; k++) {
			if (SparkType >= NUMBER_OF_SPARK_TYPES) {
				error_message(__FUNCTION__, "FreedroidRPG encountered a radial wave type that exceeds the CONSTANT for wave types (%d).",
						PLEASE_INFORM | IS_FATAL, SparkType);
			}

			switch (SparkType) {
			case 0:
				sprintf(ConstructedFilename, "radial_spells/blue_sparks_%d.png", k);
				break;
			case 1:
				sprintf(ConstructedFilename, "radial_spells/green_mist_%d.png", k);
				break;
			case 2:
				sprintf(ConstructedFilename, "radial_spells/red_fire_%d.png", k);
				break;
			default:
				error_message(__FUNCTION__, "FreedroidRPG encountered a radial wave type that does not exist in FreedroidRPG (%d).",
						PLEASE_INFORM | IS_FATAL, SparkType);
			}

			find_file(ConstructedFilename, GRAPHICS_DIR, fpath, PLEASE_INFORM | IS_FATAL);

			tmp_surf = our_IMG_load_wrapper(fpath);
			if (tmp_surf == NULL) {
				error_message(__FUNCTION__, "FreedroidRPG wanted to load a certain image file into memory, but the SDL\n"
				                            "function used for this did not succeed (%s).",
				              PLEASE_INFORM | IS_FATAL, fpath);
			}
			// SDL_SetColorKey( tmp_surf , 0 , 0 ); 
			SparkPrototypeSurface[SparkType][k] = SDL_DisplayFormatAlpha(tmp_surf);
			SDL_FreeSurface(tmp_surf);

			// Now that the loading is successfully done, we can do the
			// prerotation of the images...using a constant for simplicity...
			//
			for (i = 0; i < FIXED_NUMBER_OF_SPARK_ANGLES; i++) {
				Angle = +45 - 360.0 * (float)i / (float)FIXED_NUMBER_OF_SPARK_ANGLES;

				tmp_surf = rotozoomSurface(SparkPrototypeSurface[SparkType][k], Angle, 1.0, FALSE);

				PrerotatedSparkSurfaces[SparkType][k][i].surface = SDL_DisplayFormatAlpha(tmp_surf);

				// Maybe opengl is in use.  Then we need to prepare some textures too...
				//
				if (use_open_gl) {
					make_texture_out_of_surface(&(PrerotatedSparkSurfaces[SparkType][k][i]));
				}

				SDL_FreeSurface(tmp_surf);
			}
		}

	}

	NumberOfPicturesToUse = 2 * (2 * Radius * 64 * 3.14) / (float)SparkPrototypeSurface[SparkType][PictureType]->w;
	NumberOfPicturesToUse += 3;	// we want some overlap

	// Now we blit all the pictures we like to use...in this case using
	// multiple dynamic rotations (oh god!)...
	//
	for (i = 0; i < NumberOfPicturesToUse; i++) {
		Angle = 360.0 * (float)i / (float)NumberOfPicturesToUse;
		Displacement.x = Radius;
		Displacement.y = 0;
		RotateVectorByAngle(&Displacement, Angle);

		PrerotationIndex = rintf((Angle) * (float)FIXED_NUMBER_OF_SPARK_ANGLES / 360.0);
		if (PrerotationIndex >= FIXED_NUMBER_OF_SPARK_ANGLES)
			PrerotationIndex = 0;

		current_active_direction = rintf((Angle) * (float)RADIAL_SPELL_DIRECTIONS / 360.0);
		if (!active_direction[current_active_direction])
			continue;

		if (use_open_gl) {
			TargetRectangle.x =
			    translate_map_point_to_screen_pixel_x(PosX + Displacement.x,
								  PosY + Displacement.y) -
			    ((PrerotatedSparkSurfaces[SparkType][PictureType][PrerotationIndex].w) / 2);
			TargetRectangle.y =
			    translate_map_point_to_screen_pixel_y(PosX + Displacement.x,
								  PosY + Displacement.y) -
			    ((PrerotatedSparkSurfaces[SparkType][PictureType][PrerotationIndex].h) / 2);
		} else {
			TargetRectangle.x =
			    translate_map_point_to_screen_pixel_x(PosX + Displacement.x,
								  PosY + Displacement.y) -
			    ((PrerotatedSparkSurfaces[SparkType][PictureType][PrerotationIndex].surface->w) / 2);
			TargetRectangle.y =
			    translate_map_point_to_screen_pixel_y(PosX + Displacement.x,
								  PosY + Displacement.y) -
			    ((PrerotatedSparkSurfaces[SparkType][PictureType][PrerotationIndex].surface->h) / 2);
		}

		display_image_on_screen(&PrerotatedSparkSurfaces[SparkType][PictureType][PrerotationIndex],
								 TargetRectangle.x, TargetRectangle.y, IMAGE_NO_TRANSFO);
	}
}

/**
 * This function draws a blast into the combat window.
 * The only given parameter is the number of the blast within
 * the AllBlasts array.
 */
void PutBlast(int Blast_number)
{
	blast *CurBlast = &AllBlasts[Blast_number];

	// If the blast is already long dead, we need not do anything else here
	if (CurBlast->type == INFOUT)
		return;

	int phase = (int)floorf(CurBlast->phase);
	if (phase >= 20) {
		DeleteBlast(Blast_number);
		return;
	}
	// DebugPrintf( 0 , "\nBulletType before calculating phase : %d." , CurBullet->type );
	if (CurBlast->type >= ALLBLASTTYPES) {
		error_message(__FUNCTION__, "\
The PutBlast function should blit a blast of a type that does not\n\
exist at all.", PLEASE_INFORM | IS_FATAL);
	}
	// draw position is relative to current level, so compute the appropriate virtual position
	gps vpos;
	update_virtual_position(&vpos, &CurBlast->pos, Me.pos.z);
	if (vpos.x == -1)
		return;
	if (IsVisible(&vpos))
		display_image_on_map(&Blastmap[CurBlast->type].images[phase], vpos.x, vpos.y, IMAGE_NO_TRANSFO);
}

/**
 * When the inventory screen is visible, we do not only show the items
 * present in inventory, but we also show the inventory squares, that each
 * item in the item pool takes away for storage.  This function blits a
 * part-transparent colored shadow under the item, such that the inventory
 * dimensions become apparent to the player immediately.
 */
void draw_inventory_occupied_rectangle(SDL_Rect TargetRect, int bgcolor)
{
#define REQUIREMENTS_NOT_MET 1
#define IS_MAGICAL 2

	if (!bgcolor)
		draw_rectangle(&TargetRect, 127, 127, 127, 100);
	if (bgcolor & IS_MAGICAL)
		draw_rectangle(&TargetRect, 0, 0, 255, 100);
	if (bgcolor & REQUIREMENTS_NOT_MET)
		draw_rectangle(&TargetRect, 255, 0, 0, 100);
}

/**
 * This function displays the inventory screen and also fills in all the
 * items the influencer is carrying in his inventory and also all the 
 * items the influencer is fitted with.
 */
void show_inventory_screen(void)
{
	SDL_Rect TargetRect;
	int SlotNum;
	int i, j;

	// We define the left side of the user screen as the rectangle
	// for our inventory screen.
	//
	InventoryRect.x = 0;
	InventoryRect.y = User_Rect.y;
	InventoryRect.w = 320;
	InventoryRect.h = 480;

	if (GameConfig.Inventory_Visible == FALSE)
		return;

	// At this point we know, that the inventory screen is desired and must be
	// displayed in-game:
	//
	blit_background("inventory.png");

	// Now we display the item in the influencer drive slot
	//
	TargetRect.x = InventoryRect.x + DRIVE_RECT_X;
	TargetRect.y = InventoryRect.y + DRIVE_RECT_Y;
	if (item_held_in_hand != &Me.drive_item && (Me.drive_item.type != (-1))) {
		struct image *img = get_item_inventory_image(Me.drive_item.type);
		display_image_on_screen(img, TargetRect.x, TargetRect.y, IMAGE_NO_TRANSFO);
	}

	// Now we display the item in the influencer weapon slot
	// At this point we have to pay extra care, cause the weapons in FreedroidRPG
	// really come in many different sizes.
	//
	TargetRect.x = InventoryRect.x + WEAPON_RECT_X;
	TargetRect.y = InventoryRect.y + WEAPON_RECT_Y;
	if (item_held_in_hand != &Me.weapon_item && (Me.weapon_item.type != (-1))) {
		TargetRect.x += INV_SUBSQUARE_WIDTH * 0.5 * (2 - ItemMap[Me.weapon_item.type].inv_size.x);
		TargetRect.y += INV_SUBSQUARE_HEIGHT * 0.5 * (3 - ItemMap[Me.weapon_item.type].inv_size.y);
		struct image *img = get_item_inventory_image(Me.weapon_item.type);
		display_image_on_screen(img, TargetRect.x, TargetRect.y, IMAGE_NO_TRANSFO);

		// Maybe this is also a 2-handed weapon.  In this case we need to blit the
		// weapon a second time, this time in the center of the shield rectangle to
		// visibly reflect the fact, that the shield hand is required too for this
		// weapon.
		//
		if (ItemMap[Me.weapon_item.type].weapon_needs_two_hands) {
			// Display the weapon again
			TargetRect.x = InventoryRect.x + SHIELD_RECT_X;
			TargetRect.y = InventoryRect.y + SHIELD_RECT_Y;
			TargetRect.x += INV_SUBSQUARE_WIDTH * 0.5 * (2 - ItemMap[Me.weapon_item.type].inv_size.x);
			TargetRect.y += INV_SUBSQUARE_HEIGHT * 0.5 * (3 - ItemMap[Me.weapon_item.type].inv_size.y);
			display_image_on_screen(img, TargetRect.x, TargetRect.y, IMAGE_NO_TRANSFO);
		}
	}
	// Now we display the item in the influencer armour slot
	//
	TargetRect.x = InventoryRect.x + ARMOUR_RECT_X;
	TargetRect.y = InventoryRect.y + ARMOUR_RECT_Y;
	if (item_held_in_hand != &Me.armour_item && (Me.armour_item.type != (-1))) {
		struct image *img = get_item_inventory_image(Me.armour_item.type);
		display_image_on_screen(img, TargetRect.x, TargetRect.y, IMAGE_NO_TRANSFO);
	}
	// Now we display the item in the influencer shield slot
	//
	TargetRect.x = InventoryRect.x + SHIELD_RECT_X;
	TargetRect.y = InventoryRect.y + SHIELD_RECT_Y;
	if (item_held_in_hand != &Me.shield_item && (Me.shield_item.type != (-1))) {
		// Not all shield have the same height, therefore we do a little safety
		// correction here, so that the shield will always appear in the center
		// of the shield slot
		//
		TargetRect.y += INV_SUBSQUARE_HEIGHT * 0.5 * (3 - ItemMap[Me.shield_item.type].inv_size.y);
		struct image *img = get_item_inventory_image(Me.shield_item.type);
		display_image_on_screen(img, TargetRect.x, TargetRect.y, IMAGE_NO_TRANSFO);
	}
	// Now we display the item in the influencer special slot
	//
	TargetRect.x = InventoryRect.x + HELMET_RECT_X;
	TargetRect.y = InventoryRect.y + HELMET_RECT_Y;
	if (item_held_in_hand != &Me.special_item && (Me.special_item.type != (-1))) {
		struct image *img = get_item_inventory_image(Me.special_item.type);
		display_image_on_screen(img, TargetRect.x, TargetRect.y, IMAGE_NO_TRANSFO);
	}
	// Now we display all the items the influencer is carrying with him
	//
	for (SlotNum = 0; SlotNum < MAX_ITEMS_IN_INVENTORY - 1; SlotNum++) {
		// In case the item does not exist at all, we need not do anything more...
		if (Me.Inventory[SlotNum].type < 0) {
			// The < 0 test is to handle a case where the item type field gets corrupted and is -256. We could not reproduce the problem so at least try to hide it to players.
			continue;
		}
		// In case the item is currently held in hand, we need not do anything more HERE ...
		if (item_held_in_hand == &Me.Inventory[SlotNum] ) {
			continue;
		}

		for (i = 0; i < ItemMap[Me.Inventory[SlotNum].type].inv_size.y; i++) {
			for (j = 0; j < ItemMap[Me.Inventory[SlotNum].type].inv_size.x; j++) {
				TargetRect.x =
				    INVENTORY_RECT_X - 1 + INV_SUBSQUARE_WIDTH * (Me.Inventory[SlotNum].inventory_position.x + j);
				TargetRect.y =
				    User_Rect.y + INVENTORY_RECT_Y + INV_SUBSQUARE_HEIGHT * (Me.Inventory[SlotNum].inventory_position.y +
											     i);
				TargetRect.w = INV_SUBSQUARE_WIDTH;
				TargetRect.h = INV_SUBSQUARE_HEIGHT;
				if (ItemUsageRequirementsMet(&(Me.Inventory[SlotNum]), FALSE)) {
					// Draw a blue background for items with installed add-ons.
					int socketnum;
					int has_addons = FALSE;
					for (socketnum = 0; socketnum < Me.Inventory[SlotNum].upgrade_sockets.size; socketnum++) {
						if (Me.Inventory[SlotNum].upgrade_sockets.arr[socketnum].addon) {
							has_addons = TRUE;
							break;
						}
					}
					draw_inventory_occupied_rectangle(TargetRect, has_addons? 2 : 0);
				} else {
					draw_inventory_occupied_rectangle(TargetRect, 1);
				}
			}
		}

		TargetRect.x = INVENTORY_RECT_X - 1 + INV_SUBSQUARE_WIDTH * Me.Inventory[SlotNum].inventory_position.x;
		TargetRect.y = User_Rect.y + INVENTORY_RECT_Y + INV_SUBSQUARE_HEIGHT * Me.Inventory[SlotNum].inventory_position.y;

		struct image *img = get_item_inventory_image(Me.Inventory[SlotNum].type);
		display_image_on_screen(img, TargetRect.x, TargetRect.y, IMAGE_NO_TRANSFO);

		// Show amount
		if (ItemMap[Me.Inventory[SlotNum].type].item_group_together_in_inventory) {
			SetCurrentFont(Messagevar_BFont);
			// Only 3 characters fit in one inventory square.
			char amount[4];
			if (Me.Inventory[SlotNum].multiplicity < 999)
				sprintf(amount, "%d", Me.Inventory[SlotNum].multiplicity);
			else
				strcpy(amount, "+++");
			TargetRect.w = INV_SUBSQUARE_WIDTH * ItemMap[Me.Inventory[SlotNum].type].inv_size.x;
			TargetRect.h = INV_SUBSQUARE_HEIGHT * ItemMap[Me.Inventory[SlotNum].type].inv_size.y;
			int xpos = TargetRect.x + TargetRect.w - text_width(GetCurrentFont(), amount) - 2;
			int ypos = TargetRect.y + TargetRect.h - FontHeight(Messagevar_BFont);
			display_text_using_line_height(amount, xpos, ypos, &TargetRect, 1.0);
		}
	}

	if (item_held_in_hand != NULL) {
		DisplayItemImageAtMouseCursor(item_held_in_hand->type);
	} else {
		// In case the player does not have anything in his hand, then of course we need to
		// unset everything as 'not in his hand'.
		//
		// printf("\n Mouse button should cause no image now."); 
	}
}

#undef _view_c
