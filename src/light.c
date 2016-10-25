/* 
 *
 *   Copyright (c) 1994, 2002, 2003 Johannes Prix
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
 * This file contains all the functions managing the lighting inside
 * the FreedroidRPG game window, i.e. the 'light radius' of the Tux and
 * of other light emanating objects.
 */

#define _light_c 1

#include "struct.h"
#include "global.h"
#include "proto.h"

/* Position and strength of a light source */
struct light_source {
	gps pos;
	gps vpos;
	int strength;
};

static struct dynarray light_sources;

light_radius_config LightRadiusConfig = { -1, -1, -1, -1, 0.0 };

int *light_strength_buffer = NULL;
#define LIGHT_RADIUS_TEXTURE_DIVISOR 10	// Default number of screen pixels per cell
#define LIGHT_RADIUS_TEXTURE_MAX_SIZE 64	// texture max size is 64x64
#define LIGHT_STRENGTH_CELL(x,y) *(int*)(light_strength_buffer + (y)*LightRadiusConfig.cells_w + (x))

#define UNIVERSAL_TUX_HEIGHT 100.0	// Empirical Tux height, in the universal coordinate system

/*
 * ------------------------------------------------
 * Foreword to the 'light interpolation' algorithm
 * ------------------------------------------------
 * 
 * Some data (such as minimum_light_value) used to create the 'darkness map'
 * are level dependants. If several levels are displayed (i.e. when Tux is
 * near a level's border), we want to avoid abrupt light changes at level
 * boundaries. The call to soften_light_distribution() is not enough to smooth
 * those changes in a correct way. A better method is to interpolate the level
 * dependent values near the level boundaries.
 * 
 * For each tile of the darkness map there are several cases :
 * 1) the darkness tile is near a boundary's corner, which can be connected to 
 *    0, 1, 2 or 3 neighbors, needing no interpolation or an interpolation
 *    between up to 4 values (barycentric interpolation).
 * 2) the tile is near a boundary's line, with 0 or 1 neighbor, needing no
 *    no interpolation or an interpolation between 2 values.
 * 3) the tile is far from any level boundary, so no interpolation is needed.
 *    
 * Those different cases thus depend on where the darkness tile is, relatively
 * to the levels boundaries, but they also depend on the levels neighborhood.
 * Many darkness tiles thus share some properties and interpolation parameters.
 * The idea is to prepare and store as much knowledge as possible, so that
 * for each darkness tile we will use a simple switch with 3 cases :
 * - no interpolation
 * - interpolation between 2 pre-computed values
 * - interpolation between 4 pre-computed values 
 * 
 * ==================
 * Interpolation Mesh
 * ==================
 * 
 * We thus have a given DATA (such as 'minimum_light_value'), which has a
 * specific VALUE on each level. Let's use the following diagram, showing
 * the 8 potential neighbors around the current level (denoted 'C'):
 * 
 * +----+----+----+  We call 'c_val', the VALUE of the DATA on the Current level.
 * | NW | N  | NE |  We call 'nw_val', the VALUE of the DATA on the North-West
 * +----+----+----+  neighbor. And so on...
 * | W  | C  | E  |
 * +----+----+----+  Conceptually, this defines a "quad-mesh", with a VALUE for
 * | SW | S  | SE |  each quad. Our aim is to interpolate those VALUES on the
 * +----+----+----+  quad-mesh, to have a smooth variation of the DATA.
 * 
 * However, given the semantic of the DATAs, we only want to interpolate on a
 * small area near the levels' boundaries. So, we refine *each* quad into a
 * set of quads (that we will call AREAs):
 * 
 *   |     (vc)   |(vd)   
 * --+--+------+--+--    
 *   |  |      |  |      In the central AREA there will be no interpolation.
 *   +--+------+--+       
 *   |  |  (va)|  |(vb)  In the side AREAs, there will be an interpolation with
 *   |  |      |  |      the corresponding direct neighbor.
 *   |  |      |  |      
 *   |  |  (ve)|  |(vf)  In the corner AREAs, there will be an interpolation
 *   +--+------+--+      with all the surrounding neighbors.
 *   |  |      |  |      
 * --+--+------+--+--    
 *   |            |
 * 
 * We now need to define the VALUE of the DATA at each vertex of the AREAs. 
 * 
 * Let's take the north-east corner AREA and the east side AREA as an example.
 * We call 'A', the VALUE at vertex 'va'.
 * We call 'B', the VALUE at vertex 'vb', and so on.
 *   
 * The vertex 'va' is at the boundary of the central AREA. So we have:
 *      A = c_val
 * 
 * The vertex 'vb' is at a level boundary, that is at *the middle* of the current
 * level and the eastern neighbor. The VALUE at 'vb' is thus the *middle* of 
 * the VALUE at current level (c_val) and the VALUE at eastern neighbor (e_val).
 * So we have:
 *     B = 1/2 * (c_val + e_val)
 * 
 * In the same way, we have:
 *     C = 1/2 * (c_val + n_val)
 *
 * The vertex 'vd' is at *the center* of 4 levels, so at the *center* of the 
 * VALUEs of the DATA in those 4 levels. So we have:
 *     D = 1/4 * (c_val + n_val + ne_val + e_val)
 *
 * The vertices 've' and 'vf' are equivalent to 'va' and 'vb', so:
 *     E = A = c_val
 *     F = B = 1/2 * (c_val + e_val)
 *     
 * =============================
 * Application to "darkness map"
 * =============================
 * 
 * To compute the VALUE of the DATA on a specific darkness tile, we need to find
 * in which AREA the tile is lying. If the tile is:
 * 
 * - in the central AREA, no interpolation is needed,
 * 
 * - in a side AREA, we can easily see that we only need an interpolation along
 *   one axis between 2 values (in our east side AREA example, the interpolation
 *   is between the 'A' and 'B' VALUES),
 *   
 * - in a corner AREA, a 4-values interpolation is needed (in our north-east
 *   corner AREA example, the interpolation is between 'A', 'B', 'C' and 'D').
 * 
 * As a result, a given level can thus be defined by a 3x3 matrix of data
 * structures (one per AREA), and for each AREA we have to store the kind of 
 * interpolation to apply, and the VALUEs needed to compute this interpolation.
 * This data structure is called "Interpolation Data Storage".
 * 
 * ================================
 * Interpolation Data Storage (IDS)
 * ================================
 * 
 * If we look at our example (east side AREA and north-east corner AREA), 
 * we easily see that the 'B' value is in common to the two AREAs. 
 * In the same way, the 'C' value is in common to the north side AREA and the 
 * north-east corner AREA. And so on...
 * 
 * The 'A' VALUE (this is the value inside the current level) is common to
 * every AREA, and will be stored in the central IDS (that is the IDS 
 * associated to the central AREA).
 *
 * If we store 'B' in the eastern IDS, and 'C' in the northern IDS, then 
 * only the 'D' VALUE (that is the 'diagonal' value) has to be stored in the 
 * north-eastern IDS. Other values can be retrieved from the other IDSs.
 * 
 * As a conclusion, for each of the 9 AREAs, we only need to store one VALUE 
 * along with the kind of interpolation to apply.
 * 
 * Hence the following definitions:
 * 
 */

/* Interpolation type: Defines the axis along which an interpolation is needed */
enum interpolation_methods { 
	NO_INTERP = 0,	 // No interpolation is needed
	ALONG_X   = 1,	 // Interpolation with the neighbor along X axis is needed
	ALONG_Y   = 2,	 // Interpolation with the neighbor along Y axis is needed
	ALONG_XY  = 3, 	 // ALONG_X | ALONG_Y
};

/* Definition of the Interpolation Data Storage for an interpolation area */
struct interpolation_data_cell
{
	/* Interpolation values */
	float minimum_light_value;
	float light_bonus;
	
	/* Interpolation type */
	enum interpolation_methods interpolation_method;
	
	/* prepare_light_interpolation() Internal use */
	float sum;
};

/* Interpolation Data Storage for all interpolation areas */
/* (neighbors are defined by "N/C/S" and "W/C/E" indices) */
static struct interpolation_data_cell interpolation_data[MAX_LEVELS][3][3];

/*
 * This function adds interpolation values to some interpolation
 * areas.
 */
static void add_interpolation_values(int curr_id, int ngb_id, 
		int starty, int endy, int startx, int endx, enum interpolation_methods method)
{
	if (ngb_id != -1) {
		struct level *ngb_lvl = curShip.AllLevels[ngb_id];
		int x, y;
		for (y=starty; y<=endy; y++) {
			for (x=startx; x<=endx; x++) {
				interpolation_data[curr_id][y][x].light_bonus += ngb_lvl->light_bonus;
				interpolation_data[curr_id][y][x].minimum_light_value += ngb_lvl->minimum_light_value;
				interpolation_data[curr_id][y][x].sum += 1;
				interpolation_data[curr_id][y][x].interpolation_method |= method;
			}
		}
	}
}

/*
 * This function prepares some data structures used later to accelerate the
 * creation of the 'darkness map'. 
 * This function is called only once per frame.
 */
static void prepare_light_interpolation()
{
	struct visible_level *visible_lvl, *next_lvl;
	
	// We only care of the visible levels 

	BROWSE_VISIBLE_LEVELS(visible_lvl, next_lvl) {
		
		struct level *curr_lvl = visible_lvl->lvl_pointer;
		int curr_id  = curr_lvl->levelnum;
		
		// Step 1
		//   Initialize all interpolation areas with the 'c_val'

		int x, y;
		for (y = 0; y < 3; y++) {
			for (x = 0; x < 3; x++) {
				interpolation_data[curr_id][y][x].light_bonus = curr_lvl->light_bonus; 
				interpolation_data[curr_id][y][x].minimum_light_value = curr_lvl->minimum_light_value;
				interpolation_data[curr_id][y][x].sum = 1;
				interpolation_data[curr_id][y][x].interpolation_method = NO_INTERP;
			}
		}
		
		// Step 2
		//   For each existing neighbor, add its data to the related areas

		add_interpolation_values(curr_id, NEIGHBOR_ID_N(curr_id), 0, 0, 0, 2, ALONG_Y);
		add_interpolation_values(curr_id, NEIGHBOR_ID_S(curr_id), 2, 2, 0, 2, ALONG_Y);
		add_interpolation_values(curr_id, NEIGHBOR_ID_W(curr_id), 0, 2, 0, 0, ALONG_X);
		add_interpolation_values(curr_id, NEIGHBOR_ID_E(curr_id), 0, 2, 2, 2, ALONG_X);
		add_interpolation_values(curr_id, NEIGHBOR_ID_NW(curr_id), 0, 0, 0, 0, ALONG_XY);
		add_interpolation_values(curr_id, NEIGHBOR_ID_NE(curr_id), 0, 0, 2, 2, ALONG_XY);
		add_interpolation_values(curr_id, NEIGHBOR_ID_SW(curr_id), 2, 2, 0, 0, ALONG_XY);
		add_interpolation_values(curr_id, NEIGHBOR_ID_SE(curr_id), 2, 2, 2, 2, ALONG_XY);
		
		// Step 3
		//   Compute the mean of each value

		for (y = 0; y < 3; y++) {
			for (x = 0; x < 3; x++) {
				interpolation_data[curr_id][y][x].light_bonus /= interpolation_data[curr_id][y][x].sum; 
				interpolation_data[curr_id][y][x].minimum_light_value /= interpolation_data[curr_id][y][x].sum;
				interpolation_data[curr_id][y][x].sum = 1;
			}
		}
	}
}

/*
 * This functions compute the interpolation of the level dependent light values,
 * at a given gps position.
 * (for a global comment on light interpolation see 'Foreword to light interpolation
 *  algorithm')
 * 
 * Some Notes: 
 * 1) The interpolation is computed in an area inside the level's boundaries. 
 *    The width of this area is INTERP_WIDTH. If the current point is inside such 
 *    an area, an interpolation factor is computed. 
 *    The interpolation factor is defined as : 
 *              distance_to_level_boundary / INTERP_WIDTH.
 *    The range of this factor is thus [0, 1] ('O' if the point is at the level 
 *    boundary).
 * 2) We assume that the level's size, along one given axis, is always greater
 *    than 2*INTER_WIDTH. We thus never have to interpolate the current level
 *    together with its eastern *and* western neighbors, for example.
 */

static void interpolate_light_data(gps *pos, struct interpolation_data_cell *data)
{
#	define INTERP_WIDTH 3.0
#   define INTERP_FACTOR(x) ((x)/INTERP_WIDTH)

	float X = pos->x;
	float Y = pos->y;
	int lvl_id = pos->z;
	level *lvl = curShip.AllLevels[pos->z];
	
	// Interpolation macros
	
#	define INTERP_2_VALUES(dir, field) \
	(                                                  \
	       interp_factor_##dir  * center_area->field + \
	  (1.0-interp_factor_##dir) *   curr_area->field   \
	) 

#	define INTERP_4_VALUES(field) \
	(                                                                       \
	       interp_factor_x  *      interp_factor_y  *  center_area->field + \
	  (1.0-interp_factor_x) *      interp_factor_y  * xborder_area->field + \
	       interp_factor_x  * (1.0-interp_factor_y) * yborder_area->field + \
	  (1.0-interp_factor_x) * (1.0-interp_factor_y) *    curr_area->field   \
	)

	// Compute the 'N/C/S' and 'W/C/E' indices of the area containing
	// the current position, compute the interpolation factors,
	// and get the associated interpolation data cell.
	
	int area_x = 1; // default value : center
	int area_y = 1; // default value : center
	float interp_factor_x = 1.0; // Interpolation factor along X axis
	float interp_factor_y = 1.0; // Interpolation factor along Y axis
	struct interpolation_data_cell *curr_area; 
	
	if (X < INTERP_WIDTH) {
		area_x = 0;
		interp_factor_x = INTERP_FACTOR(X);
	} else if (X > (lvl->xlen - INTERP_WIDTH)) {
		area_x = 2;
		interp_factor_x = INTERP_FACTOR(lvl->xlen - X);
	}

	if (Y < INTERP_WIDTH) {
		area_y = 0;
		interp_factor_y = INTERP_FACTOR(Y);
	} else if (Y > (lvl->ylen - INTERP_WIDTH)) {
		area_y = 2;
		interp_factor_y = INTERP_FACTOR(lvl->ylen - Y);
	}

	curr_area = &interpolation_data[lvl_id][area_y][area_x]; 

	// And now, the interpolation
	
	switch (curr_area->interpolation_method) {
	case NO_INTERP:
		data->minimum_light_value = curr_area->minimum_light_value;
		data->light_bonus = curr_area->light_bonus;
		break;
	case ALONG_X:
		{
			struct interpolation_data_cell *center_area = &interpolation_data[lvl_id][1][1]; 
			data->minimum_light_value = INTERP_2_VALUES(x, minimum_light_value);
			data->light_bonus = INTERP_2_VALUES(x, light_bonus);
		}
		break;
	case ALONG_Y:
		{
			struct interpolation_data_cell *center_area = &interpolation_data[lvl_id][1][1]; 
			data->minimum_light_value = INTERP_2_VALUES(y, minimum_light_value);
			data->light_bonus = INTERP_2_VALUES(y, light_bonus);
		}
		break;
	default:
		{	// In any other case, a 4-values interpolation was prepared
			struct interpolation_data_cell *center_area  = &interpolation_data[lvl_id][1][1]; 
			struct interpolation_data_cell *xborder_area = &interpolation_data[lvl_id][1][area_x]; 
			struct interpolation_data_cell *yborder_area = &interpolation_data[lvl_id][area_y][1]; 
			data->minimum_light_value = INTERP_4_VALUES(minimum_light_value);
			data->light_bonus = INTERP_4_VALUES(light_bonus);
		}
		break;
	}

#	undef INTERP_2_VALUES
#	undef INTERP_4_VALUES
#	undef INTERP_FACTOR
#	undef INTERP_WIDTH
	
} // static void interpolate_light_data(...)

/**
 * Compute the light_radius texture size, and allocate it
 */
void LightRadiusInit()
{
	// Divide to screen dimensions by default divisor, to compute the default
	// number of cells in the light_radius texture
	LightRadiusConfig.cells_w = round((float)GameConfig.screen_width / (float)LIGHT_RADIUS_TEXTURE_DIVISOR);
	LightRadiusConfig.cells_h = round((float)GameConfig.screen_height / (float)LIGHT_RADIUS_TEXTURE_DIVISOR);

	// This will prevent a SIGFPE from happening below
	if(LightRadiusConfig.cells_w <= 1) LightRadiusConfig.cells_w = 2;
	if(LightRadiusConfig.cells_h <= 1) LightRadiusConfig.cells_h = 2;

	// Find the the nearest power-of-two greater than of equal to the number of cells
	// and limit the texture's size
	int pot_w = pot_gte(LightRadiusConfig.cells_w);
	if (pot_w > LIGHT_RADIUS_TEXTURE_MAX_SIZE)
		pot_w = LIGHT_RADIUS_TEXTURE_MAX_SIZE;
	int pot_h = pot_gte(LightRadiusConfig.cells_h);
	if (pot_h > LIGHT_RADIUS_TEXTURE_MAX_SIZE)
		pot_h = LIGHT_RADIUS_TEXTURE_MAX_SIZE;

	if (GameConfig.screen_width >= GameConfig.screen_height) {
		// Use the whole texture width
		LightRadiusConfig.cells_w = pot_w;
		LightRadiusConfig.texture_w = pot_w;
		// Compute the actual scale factor when using the whole texture width
		// Note : The flicker-free code will translate the darkness texture.
		//        The translation can be up to one whole cell, so the scale factor
		//        is computed to be one cell wider than needed.
		LightRadiusConfig.scale_factor = (float)GameConfig.screen_width / (float)(LightRadiusConfig.cells_w - 1);
		// Since we have an homogeneous scale factor in X and Y,
		// compute the new number of cells in Y.
		// Note : Here again, we take into account the translation applied by the
		//        flicker-free code, by adding one more cell.
		LightRadiusConfig.cells_h = (int)ceilf((float)GameConfig.screen_height / LightRadiusConfig.scale_factor) + 1;
		// Check if the texture's height is now big enough. If not, take the next power-of-two
		if (LightRadiusConfig.cells_h <= pot_h)
			LightRadiusConfig.texture_h = pot_h;
		else
			LightRadiusConfig.texture_h = pot_h << 1;
	} else {
		// Same thing, if screen's height > screen's width
		// Is it ever a used screen ratio ? Well, when fdrpg will run on smartphones,
		// it could be :-)
		LightRadiusConfig.cells_h = pot_h;
		LightRadiusConfig.texture_h = pot_h;
		LightRadiusConfig.scale_factor = (float)GameConfig.screen_height / (float)(LightRadiusConfig.cells_h - 1);
		LightRadiusConfig.cells_w = (int)ceilf((float)GameConfig.screen_width / LightRadiusConfig.scale_factor) + 1;
		if (LightRadiusConfig.cells_w <= pot_w)
			LightRadiusConfig.texture_w = pot_w;
		else
			LightRadiusConfig.texture_w = pot_w << 1;
	}

	// The scale factor has to be an integer (due to a modulo operation in the
	// flicker-free code).
	LightRadiusConfig.scale_factor = ceilf(LightRadiusConfig.scale_factor);

	// The center of the light_radius texture will be translated along the Y axe,
	// to simulate a light coming from bot/tux heads, instead of their feet.

	// First: convert a universal unit into a screen height. 
	// But because the UNIVERSAL_COORD macros currently works only for 4:3 screen,
	// we take the real screen ration into account
	//
	// We should use UNIVERSAL_COORD_H( (GameConfig.screen_width/GameConfig.screen_height) / (4.0/3.0) ),
	// however it returns 0 instead of 1 for a 640x480 resolution, due to floating point approximations.
	float unit_screen_height =
	    (((float)GameConfig.screen_width / (float)GameConfig.screen_height) / (4.0 / 3.0)) * ((float)(GameConfig.screen_height) /
												  480.0);

	// Second : transform Tux universal height into screen height
	LightRadiusConfig.translate_y = (int)(UNIVERSAL_TUX_HEIGHT / unit_screen_height);

	// Allocate the light_radius buffer
	light_strength_buffer = MyMalloc(LightRadiusConfig.cells_w * LightRadiusConfig.cells_h * sizeof(int));

}				// void LightRadiusInit();

/**
 * Clean and deallocate light_radius texture
 */
void LightRadiusClean()
{
	if (light_strength_buffer) {
		free(light_strength_buffer);
		light_strength_buffer = NULL;
	}
}

static void add_light_source(gps pos, gps vpos, int strength)
{
	struct light_source src;

	src.pos      = pos;
	src.vpos     = vpos;
	src.strength = strength;

	dynarray_add(&light_sources, &src, sizeof(src));
}

/**
 * There might be some obstacles that emit some light.  Yet, we can't
 * go through all the obstacle list of a level every frame at sufficiently
 * low cost.  Therefore we create a reduced list of light emitters for
 * usage in the light computation code.
 */
void update_light_list()
{
	struct level *light_level = curShip.AllLevels[Me.pos.z];
	struct visible_level *visible_lvl, *next_lvl;
	struct level *curr_lvl;
	int curr_id;
	int glue_index;
	int light_strength;
	int obs_index;
	int map_x, map_y;	
	struct obstacle *emitter;
	int blast_idx;
	struct gps me_vpos;

	dynarray_init(&light_sources, 10, sizeof(struct light_source));

	// Now we fill in the Tux position as the very first light source, that will
	// always be present.

	light_strength = light_level->light_bonus + Me.light_bonus_from_tux;

	// We must not in any case tear a hole into the beginning of the list though...
	if (light_strength <= 0)
		light_strength = 1;

	add_light_source(Me.pos, Me.pos, light_strength);

	// Now we can fill in any explosions, that are currently going on.
	// These will typically emanate a lot of light.

	for (blast_idx = 0; blast_idx < all_blasts.size; blast_idx++) {
		// Unused blast slot
		if (!sparse_dynarray_member_used(&all_blasts, blast_idx))
			continue;

		struct blast *current_blast = (struct blast *)sparse_dynarray_member(&all_blasts, blast_idx, sizeof(struct blast));

		if (current_blast->type != DROIDBLAST)
			continue;

		// We add some light strength according to the phase of the blast

		int light_strength = 10 + current_blast->phase / 2;
		if (light_strength < 0) continue;

		struct gps vpos;
		update_virtual_position(&vpos, &current_blast->pos, Me.pos.z);
		if (vpos.x == -1)
			continue;

		add_light_source(current_blast->pos, vpos, light_strength);
	}

	// Now we can fill in the remaining light sources of this level.
	// First we scan all the obstacles around Tux

	// Scanned area
	struct gps area_start = { Me.pos.x - 1.5 * FLOOR_TILES_VISIBLE_AROUND_TUX,
	                          Me.pos.y - 1.5 * FLOOR_TILES_VISIBLE_AROUND_TUX,
	                          Me.pos.z
	};
	struct gps area_end = { Me.pos.x + 1.5 * FLOOR_TILES_VISIBLE_AROUND_TUX,
	                        Me.pos.y + 1.5 * FLOOR_TILES_VISIBLE_AROUND_TUX,
	                        Me.pos.z
	};
	struct gps intersection_start, intersection_end;

	// For each visible level, compute the intersection between the scanned area
	// and the level's limits, and search light emitting obstacles inside this
	// intersection
	BROWSE_VISIBLE_LEVELS(visible_lvl, next_lvl) {	
		curr_lvl = visible_lvl->lvl_pointer;
		curr_id  = curr_lvl->levelnum;

		// transform area gps relatively to current level
		update_virtual_position(&intersection_start, &area_start, curr_id);
		update_virtual_position(&intersection_end, &area_end, curr_id);
		
		// intersect with level's limits
		intersection_start.x = max(intersection_start.x, 0);
		intersection_start.y = max(intersection_start.y, 0);
		intersection_end.x = min(intersection_end.x, curr_lvl->xlen - 1);
		intersection_end.y = min(intersection_end.y, curr_lvl->ylen - 1);

		// scan all obstacles inside the intersected area
		int tstamp = next_glue_timestamp();
		for (map_y = intersection_start.y; map_y < intersection_end.y; map_y++) {
			for (map_x = intersection_start.x; map_x < intersection_end.x; map_x++) {
				for (glue_index = 0; glue_index < curr_lvl->map[map_y][map_x].glued_obstacles.size; glue_index++) {

					obs_index = ((int *)(curr_lvl->map[map_y][map_x].glued_obstacles.arr))[glue_index];
					emitter = &(curr_lvl->obstacle_list[obs_index]);

					if (emitter->timestamp == tstamp)
						continue;

					emitter->timestamp = tstamp;

					struct dynarray *animated_light_strengths = &(get_obstacle_spec(emitter->type)->emitted_light_strength);
					int emitted_light_strength = *(int *)dynarray_member(animated_light_strengths, emitter->frame_index % animated_light_strengths->size, sizeof(int));
					if (!emitted_light_strength)
						continue;

					struct gps vpos;
					update_virtual_position(&vpos, &emitter->pos, Me.pos.z);
					if (vpos.x == -1)
						continue;

					add_light_source(emitter->pos, vpos, emitted_light_strength);
				}
			}
		}
	}

	// Second, we add the potentially visible bots

	struct enemy *erot;
	
	BROWSE_VISIBLE_LEVELS(visible_lvl, next_lvl) {	
		curr_lvl = visible_lvl->lvl_pointer;
		curr_id  = curr_lvl->levelnum;
		update_virtual_position(&me_vpos, &Me.pos, curr_id);
		
		BROWSE_LEVEL_BOTS(erot, curr_id) {
			if (fabsf(me_vpos.x - erot->pos.x) >= FLOOR_TILES_VISIBLE_AROUND_TUX)
				continue;

			if (fabsf(me_vpos.y - erot->pos.y) >= FLOOR_TILES_VISIBLE_AROUND_TUX)
				continue;

			struct gps vpos;
			update_virtual_position(&vpos, &erot->pos, Me.pos.z);
			if (vpos.x == -1)
				continue;

			add_light_source(erot->pos, vpos, 5);
		}
	}
}

/**
 * This function is used to find the light intensity at any given point
 * on the map.
 */
static int calculate_light_strength(gps *cell_vpos)
{
	int i;
	float xdist;
	float ydist;
	float squared_dist;
	gps cell_rpos;
	struct interpolation_data_cell ilights;
	struct light_source *lights = light_sources.arr;

	// 1. Light interpolation
	//
	if (!resolve_virtual_position(&cell_rpos, cell_vpos))
		return 0;

	interpolate_light_data(&cell_rpos, &ilights);
	
	// Interpolated ambient light
	// Full dark:  final_light_strength = 0
	// Max bright: final_light_strength = minimum light value
	int final_light_strength = max(0, ilights.minimum_light_value);

	// Interpolated light emitted from Tux
	lights[0].strength = ilights.light_bonus + Me.light_bonus_from_tux;

	// 2. Compute the light strength value at "cell_vpos"
	// Nota: the following code being quite obscure, some explanations are quite
	//       mandatory
	//
	// 1) The light_strength value of a light source is a relative value, to
	//    be added to the level's ambient light strength. We thus define:
	//        Absolute_intensity = level_ambient + source_strength
	//    Moreover, the light emitted from a light source decreases linearly 
	//    with the distance. So, at a given target position, the perceived 
	//    intensity of a light is: 
	//        I = Absolute_intensity - 4.0 * distance(Light_pos, Cell_vpos)
	//
	//    Note on the 4.0 factor :
	//      This factor is used to reduce the size of the radius of the "light
	//      circle" around the light source.
	//
	// 2) The code loops on each light source, and keeps the maximum light 
	//    strength. It does not accumulate light intensity.
	//
	// So the initial code is :
	//
	// 1: foreach this_light in (set of lights) {
	// 2:   if (this_light is visible from the target) {
	// 3:     this_light_strength = Absolute_intensity - 4.0 * distance(this_light.pos, cell.pos);
	// 4:     if (this_ligh_strength > final_strength) final_strength = this_light_strength;
	// 5:   }
	// 6: }
	//
	// However, this function is time-critical, simply because it is called many
	// times at every frame. Therefore we want to avoid the sqrt() needed to compute 
	// a distance, if the test fails, and instead use squared values as much as 
	// possible.
	//
	// So, lines 3 and 4 are transformed into:
	//       if (Abs_intensity - 4.0 * distance > final_strength) final_strength = Abs_intensity - 4.0 * distance;
	// which are then transformed into:
	//       if (4.0 * distance < Abs_intensity - final_strength) final_strength = Abs_intensity - 4.0 * distance;
	// and finally, in order to remove sqrt(), into:
	//       if ((4.0 * distance)^2 < (Abs_intensity - final_strength)^2) final_strength = Abs_intensity - 4.0 * distance;
	// (note: we will ensure that (Abs_intensity - final_strength) > 0, so there is no problem
	//  with the sign of the inequality. see optimization 1 in the code)
	//
	// Visibility test (line 2) being far more costly than the light strength test, we revert the 2 tests :
	//
	// 1: foreach this_light in (set of lights) {
	// 2:   if ((4.0 * distance)^2 < (Abs_intensity - final_strength)^2) {
	// 3:     if (this_light is visible from the target) 
	// 4:       final_strength = Abs_intensity - 4.0 * distance;
	// 5:   }
	// 6: }

	// Now, here is the final algorithm
	//
	for (i = 0; i < light_sources.size; i++) {
		float absolute_intensity = ilights.minimum_light_value + lights[i].strength;
		
		// Optimization 1:
		// If absolute_intensity is lower than current intensity, we do not take
		// the light source into account. It means that we do not accumulate
		// light sources.
		if ( absolute_intensity - final_light_strength < 0 )
			continue;

		// Some pre-computations
		// First transform light source's position into virtual position, related to Tux's current level
		xdist = lights[i].vpos.x - cell_vpos->x;
		ydist = lights[i].vpos.y - cell_vpos->y;
		squared_dist = xdist * xdist + ydist * ydist;

		// Comparison between current light strength and the light source's strength (line 2 of the pseudo-code)
		if ((squared_dist * 4.0 * 4.0) >=
		    (absolute_intensity - final_light_strength) * (absolute_intensity - final_light_strength))
			continue;

		// Visibility check (line 3 of pseudo_code)
		// with a small optimization : no visibility check if the target is very closed to the light
		if (squared_dist > (0.5*0.5)) {
			if (!DirectLineColldet(lights[i].vpos.x, lights[i].vpos.y, cell_vpos->x, cell_vpos->y, cell_vpos->z, &VisiblePassFilter))
				continue;
		}

		// New final_light_strength
		final_light_strength = absolute_intensity - 4.0 * sqrt(squared_dist); 

		// Full bright, no need to test any other light source
		if (final_light_strength >= (NUMBER_OF_SHADOW_IMAGES - 1))
			return (NUMBER_OF_SHADOW_IMAGES - 1);
	}

	return (final_light_strength);

}				// int calculate_light_strength(gps *cell_vpos)

/**
 * When the light radius (i.e. the shadow values for the floor) has been
 * set up, the shadows usually are very 'hard' in the sense that extreme
 * darkness can be right next to very bright light.  This does not look
 * very real.  Therefore we 'soften' the shadow a bit, by allowing only
 * a limited step size from one shadow square to the next.  Of course 
 * these changes have to be propagated, so we run through the whole
 * shadow grid twice and propagate in 'both' directions.  
 * The hardness of the shadow can be controlled in the definition of
 * MAX_LIGHT_STEP.  Higher values will lead to harder shadows, while 
 * lower values will give very smooth and fluorescent shadows propagating
 * even a bit under walls (which doesn't look too good).  3 seems like
 * a reasonable setting.
 */
static void soften_light_distribution(void)
{
#define MAX_LIGHT_STEP 3
	uint32_t x, y;

	// Now that the light buffer has been set up properly, we can start to
	// smooth it out a bit.  We do so in the direction of more light.
	// Propagate from top-left to bottom-right
	//
	for (y = 0; y < (LightRadiusConfig.cells_h - 1); y++) {
		for (x = 0; x < (LightRadiusConfig.cells_w - 1); x++) {
			if (LIGHT_STRENGTH_CELL(x + 1, y) < LIGHT_STRENGTH_CELL(x, y) - MAX_LIGHT_STEP)
				LIGHT_STRENGTH_CELL(x + 1, y) = LIGHT_STRENGTH_CELL(x, y) - MAX_LIGHT_STEP;
			if (LIGHT_STRENGTH_CELL(x, y + 1) < LIGHT_STRENGTH_CELL(x, y) - MAX_LIGHT_STEP)
				LIGHT_STRENGTH_CELL(x, y + 1) = LIGHT_STRENGTH_CELL(x, y) - MAX_LIGHT_STEP;
			if (LIGHT_STRENGTH_CELL(x + 1, y + 1) < LIGHT_STRENGTH_CELL(x, y) - MAX_LIGHT_STEP)
				LIGHT_STRENGTH_CELL(x + 1, y + 1) = LIGHT_STRENGTH_CELL(x, y) - MAX_LIGHT_STEP;
		}
	}
	// now the same again, this time from bottom-right to top-left
	for (y = (LightRadiusConfig.cells_h - 1); y > 0; y--) {
		for (x = (LightRadiusConfig.cells_w - 1); x > 0; x--) {
			if (LIGHT_STRENGTH_CELL(x - 1, y) < LIGHT_STRENGTH_CELL(x, y) - MAX_LIGHT_STEP)
				LIGHT_STRENGTH_CELL(x - 1, y) = LIGHT_STRENGTH_CELL(x, y) - MAX_LIGHT_STEP;
			if (LIGHT_STRENGTH_CELL(x, y - 1) < LIGHT_STRENGTH_CELL(x, y) - MAX_LIGHT_STEP)
				LIGHT_STRENGTH_CELL(x, y - 1) = LIGHT_STRENGTH_CELL(x, y) - MAX_LIGHT_STEP;
			if (LIGHT_STRENGTH_CELL(x - 1, y - 1) < LIGHT_STRENGTH_CELL(x, y) - MAX_LIGHT_STEP)
				LIGHT_STRENGTH_CELL(x - 1, y - 1) = LIGHT_STRENGTH_CELL(x, y) - MAX_LIGHT_STEP;
		}
	}

}				// void soften_light_distribution ( void )

/**
 * This function is used to find the light intensity at any given point
 * on the map.
 */
void set_up_light_strength_buffer(int *decay_x, int *decay_y)
{
	uint32_t x;
	uint32_t y;
	gps cell_vpos;
	int screen_x;
	int screen_y;

	// Flicker-free code :
	//
	// To avoid flickering, the darkness map is 'stuck' to the floor, that is :
	// when Tux moves, the darkness's textured rectangle is translated in the
	// opposite direction, to follow the apparent move of the floor.
	// The same translation has thus to be applied when generating the light
	// strength buffer.
	//
	// However, the darkness's textured rectangle does only cover the screen,
	// we will thus apply a modulo to the translation. The value of the modulo
	// is "scale_factor", that is the width (and height) of a darkness cell.
	// This ensures that a given point on the floor is always inside the "same"
	// (relative) darkness cell.
	//
	// Finally, we only keep negative translations, so that the darkness's
	// textured rectangle top-left corner is always outside the screen.
	// (note: the bottom-right corner is also always outside of the screen, due
	// to the large enough value of scale_factor (see: LightRadiusInit())
	//
	*decay_x = - (ceilf(Me.pos.x * FLOOR_TILE_WIDTH*0.5) - ceilf(Me.pos.y * FLOOR_TILE_WIDTH*0.5));
	*decay_x = *decay_x % (int)LightRadiusConfig.scale_factor;
	if (*decay_x > 0) *decay_x -= (int)LightRadiusConfig.scale_factor;

	*decay_y = - (ceilf(Me.pos.x * FLOOR_TILE_HEIGHT*0.5) + ceilf(Me.pos.y * FLOOR_TILE_HEIGHT*0.5));
	*decay_y = *decay_y % (int)LightRadiusConfig.scale_factor;
	if (*decay_y > 0) *decay_y -= (int)LightRadiusConfig.scale_factor;

	prepare_light_interpolation();
	
	for (y = 0; y < LightRadiusConfig.cells_h; y++) {
		for (x = 0; x < LightRadiusConfig.cells_w; x++) {
			screen_x = (int)(x * LightRadiusConfig.scale_factor) - UserCenter_x + *decay_x;
			// Apply a translation to Y coordinate, to simulate a light coming from bot/tux heads, instead
			// of their feet.
			screen_y = (int)(y * LightRadiusConfig.scale_factor) - UserCenter_y + LightRadiusConfig.translate_y + *decay_y;

			// Transform the screen coordinates into a virtual point on the map, relatively to
			// Tux's current level.
			cell_vpos.x = translate_pixel_to_map_location(screen_x, screen_y, TRUE);
			cell_vpos.y = translate_pixel_to_map_location(screen_x, screen_y, FALSE);
			cell_vpos.z = Me.pos.z;

			LIGHT_STRENGTH_CELL(x,y) = calculate_light_strength(&cell_vpos);
		}
	}

	soften_light_distribution();

}				// void set_up_light_strength_buffer ( void )

/**
 * This function is used to find the light intensity at any given point
 * on the screen.
 */
int get_light_strength_cell(uint32_t x, uint32_t y)
{
	return LIGHT_STRENGTH_CELL(x, y);
}

/**
 * This function is used to find the light intensity at any given point
 * on the screen.
 */
int get_light_strength_screen(int x, int y)
{
	x = x / LightRadiusConfig.scale_factor;
	y = y / LightRadiusConfig.scale_factor;

	if ((x >= 0) && (x < LightRadiusConfig.cells_w) && (y >= 0) && (y < LightRadiusConfig.cells_h)) {
		return (LIGHT_STRENGTH_CELL(x, y));
	} else {
		// If a request reaches outside the prepared buffer, we use the
		// nearest point, if we can get it easily, otherwise we'll use
		// blackness by default.

		if ((x >= 0) && (x < LightRadiusConfig.cells_w)) {
			if (y >= LightRadiusConfig.cells_h)
				return (LIGHT_STRENGTH_CELL(x, LightRadiusConfig.cells_h - 1));
			else if (y < 0)
				return (LIGHT_STRENGTH_CELL(x, 0));
		} else if ((y >= 0) && (y < LightRadiusConfig.cells_h)) {
			if (x >= LightRadiusConfig.cells_w)
				return (LIGHT_STRENGTH_CELL(LightRadiusConfig.cells_w - 1, y));
			else if (x < 0)
				return (LIGHT_STRENGTH_CELL(0, y));
		}

		return (NUMBER_OF_SHADOW_IMAGES - 1);
	}

}				// int get_light_screen_strength ( pointf target_pos )

/**
 * This function should blit the shadows on the floor, that are used to
 * generate the impression of a 'light radius' around the players 
 * character.
 *
 * decay_x and decay_y does translate the textured rectangle to avoid
 * darkness flickering. See flicker-free code's note in set_up_light_strength_buffer()
 *
 * Note : the flicker-free light_strength_buffer is not really compatible with this function.
 * The light_strength_buffer is indeed based on an orthogonal grid, but in SDL mode the darkness
 * map does use an isometric grid.
 * However the use of the flicker-free light_strength_buffer does reduce flickering even in
 * SDL mode. So, we use it.
 */
void blit_classic_SDL_light_radius(int decay_x, int decay_y)
{
	// Number of tiles along X and Y
	static int lrc_nb_columns = 0;
	static int lrc_nb_lines = 0;

	/* The screen is covered with small light_radius_chunks images, placed on a regular orthogonal grid.
	 *
	 * Each line is composed of two half-lines :
	 *  /\/\/\  First half of first line
	 *  \/\/\/\
	 *   \/\/\/ Second half of first line
	 *   
	 *  The ligth_strength value to apply is computed at the center of each tile. 
	 */

	// Width and height of the tiles
#	define LRC_ISO_WIDTH 26
#	define LRC_ISO_HEIGHT 14
	// The design of the tiles implies a 2 pixels gap along X, to avoid tiles overlapping
#	define LRC_ISO_GAP_X 2

	// Load of light_radius_chunk images, and initializes some variables, during the first call
	//
	static int first_call = TRUE;

	if (first_call) {
		first_call = FALSE;

		int i;
		char constructed_file_name[2000];

		for (i = 0; i < NUMBER_OF_SHADOW_IMAGES; i++) {
			sprintf(constructed_file_name, "light_radius_chunks/iso_light_radius_darkness_%04d.png", i);
			load_image(&light_radius_chunk[i], GRAPHICS_DIR, constructed_file_name, NO_MOD);
		}

		lrc_nb_columns = (int)ceilf((float)GameConfig.screen_width / (float)(LRC_ISO_WIDTH + LRC_ISO_GAP_X)) + 1;
		lrc_nb_lines = (int)ceilf((float)GameConfig.screen_height / (float)(LRC_ISO_HEIGHT)) + 1;
	}
	// Fill the screen with the tiles
	//

	int l, c;
	SDL_Rect target_rectangle;	// Used to store the position of the blitted tile, centered on (center_x,center_y)

	// Note : target_rectangle is modified by the SDL_BlitSurface call, 
	// so it has to be reinitialized after each call

	// First line
	int center_y = 0;
	target_rectangle.y = center_y - (LRC_ISO_HEIGHT) / 2 + decay_y;

	for (l = 0; l < lrc_nb_lines; ++l) {
		// First column of the first half-line
		int center_x = 0;
		target_rectangle.x = center_x - (LRC_ISO_WIDTH + LRC_ISO_GAP_X) / 2 + decay_x;

		for (c = 0; c <= lrc_nb_columns; ++c) {
			int light_strength;
			light_strength = get_light_strength_screen((uint32_t) center_x, (uint32_t) center_y);
			if (light_strength < (NUMBER_OF_SHADOW_IMAGES-1))
				SDL_BlitSurface(light_radius_chunk[light_strength].surface, NULL, Screen, &target_rectangle);

			// Next tile along X
			center_x += (LRC_ISO_WIDTH + LRC_ISO_GAP_X);
			target_rectangle.x = center_x - (LRC_ISO_WIDTH + LRC_ISO_GAP_X) / 2 + decay_x;
			target_rectangle.y = center_y - (LRC_ISO_HEIGHT) / 2 + decay_y;
		}

		// Second half-line, translated by a tile's half-height
		center_y += (LRC_ISO_HEIGHT) / 2;
		target_rectangle.y = center_y - (LRC_ISO_HEIGHT) / 2 + decay_y;

		// First column of the second half-line, translated by a tile's half-width
		center_x = (LRC_ISO_WIDTH + LRC_ISO_GAP_X) / 2;
		target_rectangle.x = center_x - (LRC_ISO_WIDTH + LRC_ISO_GAP_X) / 2 + decay_x;

		for (c = 0; c <= lrc_nb_columns; ++c) {
			int light_strength;
			light_strength = get_light_strength_screen((uint32_t) center_x, (uint32_t) center_y);
			if (light_strength < (NUMBER_OF_SHADOW_IMAGES-1))
				SDL_BlitSurface(light_radius_chunk[light_strength].surface, NULL, Screen, &target_rectangle);

			// Next tile along X
			center_x += LRC_ISO_WIDTH + LRC_ISO_GAP_X;
			target_rectangle.x = center_x - (LRC_ISO_WIDTH + LRC_ISO_GAP_X) / 2 + decay_x;
			target_rectangle.y = center_y - (LRC_ISO_HEIGHT) / 2 + decay_y;
		}

		// Next line
		center_y += (LRC_ISO_HEIGHT) / 2;
		target_rectangle.y = center_y - (LRC_ISO_HEIGHT) / 2 +decay_y;
	}
}				// void blit_classic_SDL_light_radius( void )

/**
 * FreedroidRPG does some light/shadow computations and then draws some
 * shadows/light on the floor.  There is a certain amount of light around
 * the Tux.  This light is called the 'light radius'.  
 * 
 * This function is supposed to compute all light/shadow values for the
 * the floor and then draw the light radius on the floor.  Note, that 
 * this is something completely different from the prerendered shadows
 * that are to be blitted for obstacles, when they are exposed to 
 * daylight.
 */
void blit_light_radius(void)
{
	int decay_x, decay_y;

	// Before making any reference to the light, it's best to 
	// first calculate the values in the light buffer, because
	// those will be used when drawing the light radius.
	//
	set_up_light_strength_buffer(&decay_x, &decay_y);

	if (use_open_gl) {
		// blit_open_gl_light_radius ();
		// blit_open_gl_cheap_light_radius ();
		blit_open_gl_stretched_texture_light_radius(decay_x, decay_y);
	} else {
		blit_classic_SDL_light_radius(decay_x, decay_y);
	}

	dynarray_free(&light_sources);
}	// void blit_light_radius ( void )

#undef _light_c
