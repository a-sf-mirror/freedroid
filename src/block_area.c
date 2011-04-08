/*
 *
 *   Copyright (c) 2004-2010 Arthur Huillet
 *   Copyright (c) 1994, 2002, 2003 Johannes Prix
 *   Copyright (c) 1994, 2002 Reinhard Prix
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

#include "system.h"

#include "defs.h"
#include "struct.h"
#include "global.h"
#include "proto.h"

static void block_2param(int i, float ew, float ns)
{
	obstacle_map[i].left_border = -ew / 2.0;
	obstacle_map[i].right_border = ew / 2.0;
	obstacle_map[i].upper_border = -ns / 2.0;
	obstacle_map[i].lower_border = ns / 2.0;

	obstacle_map[i].block_area_parm_1 = ew;
	obstacle_map[i].block_area_parm_2 = ns;
	obstacle_map[i].block_area_type = COLLISION_TYPE_RECTANGLE;
}

static void block_4param(int i, float e, float s, float w, float n)
{
	obstacle_map[i].left_border = -w;
	obstacle_map[i].right_border = e;
	obstacle_map[i].upper_border = -n;
	obstacle_map[i].lower_border = s;

	obstacle_map[i].block_area_parm_1 = e + w;
	obstacle_map[i].block_area_parm_2 = n + s;
	obstacle_map[i].block_area_type = COLLISION_TYPE_RECTANGLE;
}

/**
 *
 *
 */
void init_obstacle_data(void)
{
	int i;
	float standard_wall_thickness = 0.4;
	float standard_wall_width = 1.1;
	float standard_door_width = 1.0;
// Extreme values for outer wall, to slightly lower problem of tux walking through outer wall door post
	float outer_wall_4_width = 0.55;
	float outer_wall_4_thickness = 0.6;
	float outer_wall_4_backside = 0.05;
	float outer_door_4_width_long = 1.55;
	float outer_door_4_width_short = 0.55;

	struct image empty_image = EMPTY_IMAGE;

	// First we enter some default values.  The exceptions from the default values
	// can be added after that.
	//
	for (i = 0; i < NUMBER_OF_OBSTACLE_TYPES; i++) {
		// In addition to the pure image information, we'll also need some
		// collision information for obstacles...
		//
		memcpy(&(obstacle_map[i].image), &(empty_image), sizeof(struct image));

		obstacle_map[i].flags |= BLOCKS_VISION_TOO;
		block_2param(i, 1.2, 1.2);	// standard_wall_thickness 
		obstacle_map[i].flags &= ~IS_SMASHABLE;
		obstacle_map[i].result_type_after_smashing_once = (-1);
		obstacle_map[i].flags &= ~DROPS_RANDOM_TREASURE;
		obstacle_map[i].flags &= ~NEEDS_PRE_PUT;
		obstacle_map[i].flags &= ~GROUND_LEVEL;
		obstacle_map[i].flags &= ~IS_WALKABLE;
        obstacle_map[i].flags &= ~IS_CLICKABLE;
		obstacle_map[i].transparent = FALSE;
		obstacle_map[i].emitted_light_strength = 0;	// how much light emitted from here...
		obstacle_map[i].action = NULL; // no action 
		obstacle_map[i].animate_fn = NULL; // no animation
	}
	// Now we define all exceptions from the default values
	//
	block_2param(ISO_V_WALL, standard_wall_thickness, standard_wall_width);
	obstacle_map[ISO_V_WALL].filename = "iso_walls_0001.png";
	obstacle_map[ISO_V_WALL].transparent = TRANSPARENCY_FOR_WALLS;
	obstacle_map[ISO_V_WALL].flags |= IS_VERTICAL;
	block_2param(ISO_H_WALL, standard_wall_width, standard_wall_thickness);
	obstacle_map[ISO_H_WALL].filename = "iso_walls_0002.png";
	obstacle_map[ISO_H_WALL].transparent = TRANSPARENCY_FOR_WALLS;
	obstacle_map[ISO_H_WALL].flags |= IS_HORIZONTAL;

	block_2param(ISO_V_WALL_WITH_DOT, standard_wall_thickness, standard_wall_width);
	obstacle_map[ISO_V_WALL_WITH_DOT].filename = "iso_walls_0003.png";
	obstacle_map[ISO_V_WALL_WITH_DOT].transparent = TRANSPARENCY_FOR_WALLS;
	obstacle_map[ISO_V_WALL_WITH_DOT].flags |= IS_VERTICAL;
	block_2param(ISO_H_WALL_WITH_DOT, standard_wall_width, standard_wall_thickness);
	obstacle_map[ISO_H_WALL_WITH_DOT].filename = "iso_walls_0004.png";
	obstacle_map[ISO_H_WALL_WITH_DOT].transparent = TRANSPARENCY_FOR_WALLS;
	obstacle_map[ISO_H_WALL_WITH_DOT].flags |= IS_HORIZONTAL;

	block_2param(ISO_GLASS_WALL_1, standard_wall_thickness, standard_wall_width);
	obstacle_map[ISO_GLASS_WALL_1].flags &= ~BLOCKS_VISION_TOO;
	obstacle_map[ISO_GLASS_WALL_1].filename = "iso_walls_0020.png";
	obstacle_map[ISO_GLASS_WALL_1].flags |= IS_VERTICAL;
	obstacle_map[ISO_GLASS_WALL_1].flags |= IS_SMASHABLE | IS_CLICKABLE;
	obstacle_map[ISO_GLASS_WALL_1].label = "";
	obstacle_map[ISO_GLASS_WALL_1].action = &barrel_action;
	obstacle_map[ISO_GLASS_WALL_1].result_type_after_smashing_once = ISO_BROKEN_GLASS_WALL_1;

	block_2param(ISO_GLASS_WALL_2, standard_wall_width, standard_wall_thickness);
	obstacle_map[ISO_GLASS_WALL_2].flags &= ~BLOCKS_VISION_TOO;
	obstacle_map[ISO_GLASS_WALL_2].filename = "iso_walls_0021.png";
	obstacle_map[ISO_GLASS_WALL_2].flags |= IS_HORIZONTAL;
	obstacle_map[ISO_GLASS_WALL_2].flags |= IS_SMASHABLE | IS_CLICKABLE;
	obstacle_map[ISO_GLASS_WALL_2].label = "";
	obstacle_map[ISO_GLASS_WALL_2].action = &barrel_action;

	obstacle_map[ISO_BROKEN_GLASS_WALL_1].block_area_type = COLLISION_TYPE_NONE;
	obstacle_map[ISO_BROKEN_GLASS_WALL_1].flags &= ~BLOCKS_VISION_TOO;
	obstacle_map[ISO_BROKEN_GLASS_WALL_1].flags |= IS_WALKABLE;
	obstacle_map[ISO_BROKEN_GLASS_WALL_1].filename = "iso_walls_0030.png";

	block_2param(ISO_CYAN_WALL_WINDOW_1, standard_wall_thickness, standard_wall_width);
	obstacle_map[ISO_CYAN_WALL_WINDOW_1].flags &= ~BLOCKS_VISION_TOO;
	obstacle_map[ISO_CYAN_WALL_WINDOW_1].filename = "iso_walls_0022.png";
	obstacle_map[ISO_CYAN_WALL_WINDOW_1].transparent = TRANSPARENCY_FOR_WALLS;
	obstacle_map[ISO_CYAN_WALL_WINDOW_1].flags |= IS_VERTICAL;
	block_2param(ISO_CYAN_WALL_WINDOW_2, standard_wall_width, standard_wall_thickness);
	obstacle_map[ISO_CYAN_WALL_WINDOW_2].flags &= ~BLOCKS_VISION_TOO;
	obstacle_map[ISO_CYAN_WALL_WINDOW_2].filename = "iso_walls_0023.png";
	obstacle_map[ISO_CYAN_WALL_WINDOW_2].transparent = TRANSPARENCY_FOR_WALLS;
	obstacle_map[ISO_CYAN_WALL_WINDOW_2].flags |= IS_HORIZONTAL;

	block_2param(ISO_RED_WALL_WINDOW_1, standard_wall_thickness, standard_wall_width);
	obstacle_map[ISO_RED_WALL_WINDOW_1].flags &= ~BLOCKS_VISION_TOO;
	obstacle_map[ISO_RED_WALL_WINDOW_1].filename = "iso_walls_0024.png";
	obstacle_map[ISO_RED_WALL_WINDOW_1].transparent = TRANSPARENCY_FOR_WALLS;
	obstacle_map[ISO_RED_WALL_WINDOW_1].flags |= IS_VERTICAL;
	block_2param(ISO_RED_WALL_WINDOW_2, standard_wall_width, standard_wall_thickness);
	obstacle_map[ISO_RED_WALL_WINDOW_2].flags &= ~BLOCKS_VISION_TOO;
	obstacle_map[ISO_RED_WALL_WINDOW_2].filename = "iso_walls_0025.png";
	obstacle_map[ISO_RED_WALL_WINDOW_2].transparent = TRANSPARENCY_FOR_WALLS;
	obstacle_map[ISO_RED_WALL_WINDOW_2].flags |= IS_HORIZONTAL;

	block_2param(ISO_FLOWER_WALL_WINDOW_1, standard_wall_thickness, standard_wall_width);
	obstacle_map[ISO_FLOWER_WALL_WINDOW_1].flags &= ~BLOCKS_VISION_TOO;
	obstacle_map[ISO_FLOWER_WALL_WINDOW_1].filename = "iso_walls_0026.png";
	obstacle_map[ISO_FLOWER_WALL_WINDOW_1].transparent = TRANSPARENCY_FOR_WALLS;
	obstacle_map[ISO_FLOWER_WALL_WINDOW_1].flags |= IS_VERTICAL;
	block_2param(ISO_FLOWER_WALL_WINDOW_2, standard_wall_width, standard_wall_thickness);
	obstacle_map[ISO_FLOWER_WALL_WINDOW_2].flags &= ~BLOCKS_VISION_TOO;
	obstacle_map[ISO_FLOWER_WALL_WINDOW_2].filename = "iso_walls_0027.png";
	obstacle_map[ISO_FLOWER_WALL_WINDOW_2].transparent = TRANSPARENCY_FOR_WALLS;
	obstacle_map[ISO_FLOWER_WALL_WINDOW_2].flags |= IS_HORIZONTAL;

	block_2param(ISO_FUNKY_WALL_WINDOW_1, standard_wall_thickness, standard_wall_width);
	obstacle_map[ISO_FUNKY_WALL_WINDOW_1].flags &= ~BLOCKS_VISION_TOO;
	obstacle_map[ISO_FUNKY_WALL_WINDOW_1].filename = "iso_walls_0028.png";
	obstacle_map[ISO_FUNKY_WALL_WINDOW_1].transparent = TRANSPARENCY_FOR_WALLS;
	obstacle_map[ISO_FUNKY_WALL_WINDOW_1].flags |= IS_VERTICAL;
	block_2param(ISO_FUNKY_WALL_WINDOW_2, standard_wall_width, standard_wall_thickness);
	obstacle_map[ISO_FUNKY_WALL_WINDOW_2].flags &= ~BLOCKS_VISION_TOO;
	obstacle_map[ISO_FUNKY_WALL_WINDOW_2].filename = "iso_walls_0029.png";
	obstacle_map[ISO_FUNKY_WALL_WINDOW_2].transparent = TRANSPARENCY_FOR_WALLS;
	obstacle_map[ISO_FUNKY_WALL_WINDOW_2].flags |= IS_HORIZONTAL;

	block_2param(ISO_V_DOOR_000_OPEN, standard_wall_thickness, standard_door_width);
	obstacle_map[ISO_V_DOOR_000_OPEN].flags &= ~BLOCKS_VISION_TOO;
	obstacle_map[ISO_V_DOOR_000_OPEN].flags |= IS_WALKABLE;
	obstacle_map[ISO_V_DOOR_000_OPEN].filename = "iso_doors_0006.png";
	obstacle_map[ISO_V_DOOR_000_OPEN].animate_fn = animate_door;
	obstacle_map[ISO_V_DOOR_000_OPEN].flags |= IS_VERTICAL;
	block_2param(ISO_V_DOOR_025_OPEN, standard_wall_thickness, standard_door_width);
	obstacle_map[ISO_V_DOOR_025_OPEN].flags &= ~BLOCKS_VISION_TOO;
	obstacle_map[ISO_V_DOOR_025_OPEN].flags |= IS_WALKABLE;
	obstacle_map[ISO_V_DOOR_025_OPEN].filename = "iso_doors_0007.png";
	obstacle_map[ISO_V_DOOR_025_OPEN].animate_fn = animate_door;
	block_2param(ISO_V_DOOR_050_OPEN, standard_wall_thickness, standard_door_width);
	obstacle_map[ISO_V_DOOR_050_OPEN].flags &= ~BLOCKS_VISION_TOO;
	obstacle_map[ISO_V_DOOR_050_OPEN].flags |= IS_WALKABLE;
	obstacle_map[ISO_V_DOOR_050_OPEN].filename = "iso_doors_0008.png";
	obstacle_map[ISO_V_DOOR_050_OPEN].animate_fn = animate_door;
	block_2param(ISO_V_DOOR_075_OPEN, standard_wall_thickness, standard_door_width);
	obstacle_map[ISO_V_DOOR_075_OPEN].flags &= ~BLOCKS_VISION_TOO;
	obstacle_map[ISO_V_DOOR_075_OPEN].flags |= IS_WALKABLE;
	obstacle_map[ISO_V_DOOR_075_OPEN].filename = "iso_doors_0009.png";
	obstacle_map[ISO_V_DOOR_075_OPEN].animate_fn = animate_door;
	obstacle_map[ISO_V_DOOR_100_OPEN].block_area_type = COLLISION_TYPE_NONE;
	obstacle_map[ISO_V_DOOR_100_OPEN].filename = "iso_doors_0010.png";
	obstacle_map[ISO_V_DOOR_100_OPEN].animate_fn = animate_door;

	block_2param(ISO_V_DOOR_LOCKED, standard_wall_thickness, standard_wall_width);
	obstacle_map[ISO_V_DOOR_LOCKED].filename = "iso_doors_0012.png";
	obstacle_map[ISO_V_DOOR_LOCKED].flags &= ~BLOCKS_VISION_TOO;
	obstacle_map[ISO_V_DOOR_LOCKED].flags |= IS_VERTICAL;

	obstacle_map[ISO_H_DOOR_LOCKED].filename = "iso_doors_0011.png";
	block_2param(ISO_H_DOOR_LOCKED, standard_wall_width, standard_wall_thickness);
	obstacle_map[ISO_H_DOOR_LOCKED].flags &= ~BLOCKS_VISION_TOO;
	obstacle_map[ISO_H_DOOR_LOCKED].flags |= IS_HORIZONTAL;

	block_2param(ISO_H_DOOR_000_OPEN, standard_door_width, standard_wall_thickness);
	obstacle_map[ISO_H_DOOR_000_OPEN].flags &= ~BLOCKS_VISION_TOO;
	obstacle_map[ISO_H_DOOR_000_OPEN].flags |= IS_WALKABLE;
	obstacle_map[ISO_H_DOOR_000_OPEN].filename = "iso_doors_0001.png";
	obstacle_map[ISO_H_DOOR_000_OPEN].animate_fn = animate_door;
	obstacle_map[ISO_H_DOOR_000_OPEN].flags |= IS_HORIZONTAL;
	block_2param(ISO_H_DOOR_025_OPEN, standard_door_width, standard_wall_thickness);
	obstacle_map[ISO_H_DOOR_025_OPEN].flags &= ~BLOCKS_VISION_TOO;
	obstacle_map[ISO_H_DOOR_025_OPEN].flags |= IS_WALKABLE;
	obstacle_map[ISO_H_DOOR_025_OPEN].filename = "iso_doors_0002.png";
	obstacle_map[ISO_H_DOOR_025_OPEN].animate_fn = animate_door;
	block_2param(ISO_H_DOOR_050_OPEN, standard_door_width, standard_wall_thickness);
	obstacle_map[ISO_H_DOOR_050_OPEN].flags &= ~BLOCKS_VISION_TOO;
	obstacle_map[ISO_H_DOOR_050_OPEN].flags |= IS_WALKABLE;
	obstacle_map[ISO_H_DOOR_050_OPEN].filename = "iso_doors_0003.png";
	obstacle_map[ISO_H_DOOR_050_OPEN].animate_fn = animate_door;
	block_2param(ISO_H_DOOR_075_OPEN, standard_door_width, standard_wall_thickness);
	obstacle_map[ISO_H_DOOR_075_OPEN].flags &= ~BLOCKS_VISION_TOO;
	obstacle_map[ISO_H_DOOR_075_OPEN].flags |= IS_WALKABLE;
	obstacle_map[ISO_H_DOOR_075_OPEN].filename = "iso_doors_0004.png";
	obstacle_map[ISO_H_DOOR_075_OPEN].animate_fn = animate_door;
	obstacle_map[ISO_H_DOOR_100_OPEN].block_area_type = COLLISION_TYPE_NONE;
	obstacle_map[ISO_H_DOOR_100_OPEN].filename = "iso_doors_0005.png";
	obstacle_map[ISO_H_DOOR_100_OPEN].animate_fn = animate_door;

	block_4param(ISO_DH_DOOR_000_OPEN, outer_door_4_width_long, standard_wall_thickness / 2, outer_door_4_width_short, standard_wall_thickness / 2);
	obstacle_map[ISO_DH_DOOR_000_OPEN].flags &= ~BLOCKS_VISION_TOO;
	obstacle_map[ISO_DH_DOOR_000_OPEN].flags |= IS_WALKABLE;
	obstacle_map[ISO_DH_DOOR_000_OPEN].filename = "iso_doubledoors_0001.png";
	obstacle_map[ISO_DH_DOOR_000_OPEN].animate_fn = animate_door;
	block_4param(ISO_DH_DOOR_025_OPEN, outer_door_4_width_long, standard_wall_thickness / 2, outer_door_4_width_short, standard_wall_thickness / 2);
	obstacle_map[ISO_DH_DOOR_025_OPEN].flags &= ~BLOCKS_VISION_TOO;
	obstacle_map[ISO_DH_DOOR_025_OPEN].flags |= IS_WALKABLE;
	obstacle_map[ISO_DH_DOOR_025_OPEN].filename = "iso_doubledoors_0002.png";
	obstacle_map[ISO_DH_DOOR_025_OPEN].animate_fn = animate_door;
	block_4param(ISO_DH_DOOR_050_OPEN, outer_door_4_width_long, standard_wall_thickness / 2, outer_door_4_width_short, standard_wall_thickness / 2);
	obstacle_map[ISO_DH_DOOR_050_OPEN].flags &= ~BLOCKS_VISION_TOO;
	obstacle_map[ISO_DH_DOOR_050_OPEN].flags |= IS_WALKABLE;
	obstacle_map[ISO_DH_DOOR_050_OPEN].filename = "iso_doubledoors_0003.png";
	obstacle_map[ISO_DH_DOOR_050_OPEN].animate_fn = animate_door;
	block_4param(ISO_DH_DOOR_075_OPEN, outer_door_4_width_long, standard_wall_thickness / 2, outer_door_4_width_short, standard_wall_thickness / 2);
	obstacle_map[ISO_DH_DOOR_075_OPEN].flags &= ~BLOCKS_VISION_TOO;
	obstacle_map[ISO_DH_DOOR_075_OPEN].flags |= IS_WALKABLE;
	obstacle_map[ISO_DH_DOOR_075_OPEN].filename = "iso_doubledoors_0004.png";
	obstacle_map[ISO_DH_DOOR_075_OPEN].animate_fn = animate_door;
	obstacle_map[ISO_DH_DOOR_100_OPEN].block_area_type = COLLISION_TYPE_NONE;
	obstacle_map[ISO_DH_DOOR_100_OPEN].filename = "iso_doubledoors_0005.png";
	obstacle_map[ISO_DH_DOOR_100_OPEN].animate_fn = animate_door;

	block_4param(ISO_DV_DOOR_000_OPEN, standard_wall_thickness / 2, outer_door_4_width_long, standard_wall_thickness / 2, outer_door_4_width_short);
	obstacle_map[ISO_DV_DOOR_000_OPEN].flags &= ~BLOCKS_VISION_TOO;
	obstacle_map[ISO_DV_DOOR_000_OPEN].flags |= IS_WALKABLE;
	obstacle_map[ISO_DV_DOOR_000_OPEN].filename = "iso_doubledoors_0006.png";
	obstacle_map[ISO_DV_DOOR_000_OPEN].animate_fn = animate_door;
	block_4param(ISO_DV_DOOR_025_OPEN, standard_wall_thickness / 2, outer_door_4_width_long, standard_wall_thickness / 2, outer_door_4_width_short);
	obstacle_map[ISO_DV_DOOR_025_OPEN].flags &= ~BLOCKS_VISION_TOO;
	obstacle_map[ISO_DV_DOOR_025_OPEN].flags |= IS_WALKABLE;
	obstacle_map[ISO_DV_DOOR_025_OPEN].filename = "iso_doubledoors_0007.png";
	obstacle_map[ISO_DV_DOOR_025_OPEN].animate_fn = animate_door;
	block_4param(ISO_DV_DOOR_050_OPEN, standard_wall_thickness / 2, outer_door_4_width_long, standard_wall_thickness / 2, outer_door_4_width_short);
	obstacle_map[ISO_DV_DOOR_050_OPEN].flags &= ~BLOCKS_VISION_TOO;
	obstacle_map[ISO_DV_DOOR_050_OPEN].flags |= IS_WALKABLE;
	obstacle_map[ISO_DV_DOOR_050_OPEN].filename = "iso_doubledoors_0008.png";
	obstacle_map[ISO_DV_DOOR_050_OPEN].animate_fn = animate_door;
	block_4param(ISO_DV_DOOR_075_OPEN, standard_wall_thickness / 2, outer_door_4_width_long, standard_wall_thickness / 2, outer_door_4_width_short);
	obstacle_map[ISO_DV_DOOR_075_OPEN].flags &= ~BLOCKS_VISION_TOO;
	obstacle_map[ISO_DV_DOOR_075_OPEN].flags |= IS_WALKABLE;
	obstacle_map[ISO_DV_DOOR_075_OPEN].filename = "iso_doubledoors_0009.png";
	obstacle_map[ISO_DV_DOOR_075_OPEN].animate_fn = animate_door;
	obstacle_map[ISO_DV_DOOR_100_OPEN].block_area_type = COLLISION_TYPE_NONE;
	obstacle_map[ISO_DV_DOOR_100_OPEN].filename = "iso_doubledoors_0010.png";
	obstacle_map[ISO_DV_DOOR_100_OPEN].animate_fn = animate_door;

	obstacle_map[ISO_DH_DOOR_LOCKED].filename = "iso_doubledoors_0011.png";
	block_4param(ISO_DH_DOOR_LOCKED, outer_door_4_width_long, standard_wall_thickness / 2, outer_door_4_width_short, standard_wall_thickness * 2);
	obstacle_map[ISO_DH_DOOR_LOCKED].flags &= ~BLOCKS_VISION_TOO;

	obstacle_map[ISO_DV_DOOR_LOCKED].filename = "iso_doubledoors_0012.png";
	block_4param(ISO_DV_DOOR_LOCKED, standard_wall_thickness / 2, outer_door_4_width_long, standard_wall_thickness * 2, outer_door_4_width_short);
	obstacle_map[ISO_DV_DOOR_LOCKED].flags &= ~BLOCKS_VISION_TOO;


	// These are the normal pillars, that appear here and there in the game.
	// 4param is used to allow Tux to walk onto the pillar base in S&E yet
	// not appear as walking under the pillar base N&W of the pillar.

	block_4param(ISO_PILLAR_TALL, 0.25, 0.25, 0.5, 0.5);
	obstacle_map[ISO_PILLAR_TALL].filename = "iso_obstacle_0047.png";
	obstacle_map[ISO_PILLAR_TALL].flags &= ~BLOCKS_VISION_TOO;
	block_4param(ISO_PILLAR_SHORT, 0.25, 0.25, 0.5, 0.5);
	obstacle_map[ISO_PILLAR_SHORT].filename = "iso_obstacle_0048.png";
	obstacle_map[ISO_PILLAR_SHORT].flags &= ~BLOCKS_VISION_TOO;



	block_2param(ISO_TV_PILLAR_W, 1.0, 1.0);
	obstacle_map[ISO_TV_PILLAR_W].filename = "iso_machinery_0001.png";
	obstacle_map[ISO_TV_PILLAR_W].flags &= ~BLOCKS_VISION_TOO;
	block_2param(ISO_TV_PILLAR_N, 1.0, 1.0);
	obstacle_map[ISO_TV_PILLAR_N].filename = "iso_machinery_0002.png";
	obstacle_map[ISO_TV_PILLAR_N].flags &= ~BLOCKS_VISION_TOO;
	block_2param(ISO_TV_PILLAR_E, 1.0, 1.0);
	obstacle_map[ISO_TV_PILLAR_E].filename = "iso_machinery_0003.png";
	obstacle_map[ISO_TV_PILLAR_E].flags &= ~BLOCKS_VISION_TOO;
	block_2param(ISO_TV_PILLAR_S, 1.0, 1.0);
	obstacle_map[ISO_TV_PILLAR_S].filename = "iso_machinery_0004.png";
	obstacle_map[ISO_TV_PILLAR_S].flags &= ~BLOCKS_VISION_TOO;

	block_4param(ISO_ENHANCER_LD, 0.55, 0.5, 0.6, 0.6);
	obstacle_map[ISO_ENHANCER_LD].filename = "iso_machinery_0005.png";
	obstacle_map[ISO_ENHANCER_LD].flags &= ~BLOCKS_VISION_TOO;
	block_4param(ISO_ENHANCER_LU, 0.55, 0.55, 0.6, 0.6);
	obstacle_map[ISO_ENHANCER_LU].filename = "iso_machinery_0006.png";
	obstacle_map[ISO_ENHANCER_LU].flags &= ~BLOCKS_VISION_TOO;
	block_4param(ISO_ENHANCER_RU, 0.5, 0.55, 0.6, 0.6);
	obstacle_map[ISO_ENHANCER_RU].filename = "iso_machinery_0007.png";
	obstacle_map[ISO_ENHANCER_RU].flags &= ~BLOCKS_VISION_TOO;
	block_4param(ISO_ENHANCER_RD, 0.5, 0.5, 0.6, 0.6);
	obstacle_map[ISO_ENHANCER_RD].filename = "iso_machinery_0008.png";
	obstacle_map[ISO_ENHANCER_RD].flags &= ~BLOCKS_VISION_TOO;

	obstacle_map[ISO_REFRESH_1].block_area_type = COLLISION_TYPE_NONE;
	obstacle_map[ISO_REFRESH_1].flags &= ~BLOCKS_VISION_TOO;
	obstacle_map[ISO_REFRESH_1].flags |= NEEDS_PRE_PUT;
	obstacle_map[ISO_REFRESH_1].filename = "iso_machinery_0009.png";
	obstacle_map[ISO_REFRESH_1].emitted_light_strength = 10;
	obstacle_map[ISO_REFRESH_1].animate_fn = animate_refresh;
	obstacle_map[ISO_REFRESH_2].block_area_type = COLLISION_TYPE_NONE;
	obstacle_map[ISO_REFRESH_2].flags &= ~BLOCKS_VISION_TOO;
	obstacle_map[ISO_REFRESH_2].flags |= NEEDS_PRE_PUT;
	obstacle_map[ISO_REFRESH_2].filename = "iso_machinery_0010.png";
	obstacle_map[ISO_REFRESH_2].emitted_light_strength = 10;
	obstacle_map[ISO_REFRESH_2].animate_fn = animate_refresh;
	obstacle_map[ISO_REFRESH_3].block_area_type = COLLISION_TYPE_NONE;
	obstacle_map[ISO_REFRESH_3].flags &= ~BLOCKS_VISION_TOO;
	obstacle_map[ISO_REFRESH_3].flags |= NEEDS_PRE_PUT;
	obstacle_map[ISO_REFRESH_3].filename = "iso_machinery_0011.png";
	obstacle_map[ISO_REFRESH_3].emitted_light_strength = 10;
	obstacle_map[ISO_REFRESH_3].animate_fn = animate_refresh;
	obstacle_map[ISO_REFRESH_4].block_area_type = COLLISION_TYPE_NONE;
	obstacle_map[ISO_REFRESH_4].flags &= ~BLOCKS_VISION_TOO;
	obstacle_map[ISO_REFRESH_4].flags |= NEEDS_PRE_PUT;
	obstacle_map[ISO_REFRESH_4].filename = "iso_machinery_0012.png";
	obstacle_map[ISO_REFRESH_4].emitted_light_strength = 10;
	obstacle_map[ISO_REFRESH_4].animate_fn = animate_refresh;
	obstacle_map[ISO_REFRESH_5].block_area_type = COLLISION_TYPE_NONE;
	obstacle_map[ISO_REFRESH_5].flags &= ~BLOCKS_VISION_TOO;
	obstacle_map[ISO_REFRESH_5].flags |= NEEDS_PRE_PUT;
	obstacle_map[ISO_REFRESH_5].filename = "iso_machinery_0013.png";
	obstacle_map[ISO_REFRESH_5].emitted_light_strength = 10;
	obstacle_map[ISO_REFRESH_5].animate_fn = animate_refresh;

	obstacle_map[ISO_TELEPORTER_1].block_area_type = COLLISION_TYPE_NONE;
	obstacle_map[ISO_TELEPORTER_1].flags &= ~BLOCKS_VISION_TOO;
	obstacle_map[ISO_TELEPORTER_1].emitted_light_strength = 20;
	obstacle_map[ISO_TELEPORTER_1].animate_fn = animate_teleporter;
	obstacle_map[ISO_TELEPORTER_2].block_area_type = COLLISION_TYPE_NONE;
	obstacle_map[ISO_TELEPORTER_2].flags &= ~BLOCKS_VISION_TOO;
	obstacle_map[ISO_TELEPORTER_2].emitted_light_strength = 19;
	obstacle_map[ISO_TELEPORTER_2].animate_fn = animate_teleporter;
	obstacle_map[ISO_TELEPORTER_3].block_area_type = COLLISION_TYPE_NONE;
	obstacle_map[ISO_TELEPORTER_3].flags &= ~BLOCKS_VISION_TOO;
	obstacle_map[ISO_TELEPORTER_3].emitted_light_strength = 18;
	obstacle_map[ISO_TELEPORTER_3].animate_fn = animate_teleporter;
	obstacle_map[ISO_TELEPORTER_4].block_area_type = COLLISION_TYPE_NONE;
	obstacle_map[ISO_TELEPORTER_4].flags &= ~BLOCKS_VISION_TOO;
	obstacle_map[ISO_TELEPORTER_4].emitted_light_strength = 19;
	obstacle_map[ISO_TELEPORTER_4].animate_fn = animate_teleporter;
	obstacle_map[ISO_TELEPORTER_5].block_area_type = COLLISION_TYPE_NONE;
	obstacle_map[ISO_TELEPORTER_5].flags &= ~BLOCKS_VISION_TOO;
	obstacle_map[ISO_TELEPORTER_5].emitted_light_strength = 20;
	obstacle_map[ISO_TELEPORTER_5].animate_fn = animate_teleporter;

	block_2param(ISO_V_CHEST_OPEN, 0.6, 0.8);
	obstacle_map[ISO_V_CHEST_OPEN].filename = "iso_container_0004.png";
	obstacle_map[ISO_V_CHEST_OPEN].flags &= ~BLOCKS_VISION_TOO;
	obstacle_map[ISO_V_CHEST_OPEN].flags |= GROUND_LEVEL;

	block_2param(ISO_V_CHEST_CLOSED, 0.6, 0.8);
	obstacle_map[ISO_V_CHEST_CLOSED].filename = "iso_container_0002.png";
	obstacle_map[ISO_V_CHEST_CLOSED].flags &= ~BLOCKS_VISION_TOO;
	obstacle_map[ISO_V_CHEST_CLOSED].flags |= GROUND_LEVEL | IS_CLICKABLE;
	obstacle_map[ISO_V_CHEST_CLOSED].action = &chest_open_action;
	obstacle_map[ISO_V_CHEST_CLOSED].label = _("Chest");

	block_2param(ISO_H_CHEST_OPEN, 0.8, 0.6);
	obstacle_map[ISO_H_CHEST_OPEN].filename = "iso_container_0003.png";
	obstacle_map[ISO_H_CHEST_OPEN].flags &= ~BLOCKS_VISION_TOO;
	obstacle_map[ISO_H_CHEST_OPEN].flags |= GROUND_LEVEL;

	block_2param(ISO_H_CHEST_CLOSED, 0.8, 0.6);
	obstacle_map[ISO_H_CHEST_CLOSED].filename = "iso_container_0001.png";
	obstacle_map[ISO_H_CHEST_CLOSED].flags &= ~BLOCKS_VISION_TOO;
	obstacle_map[ISO_H_CHEST_CLOSED].flags |= GROUND_LEVEL | IS_CLICKABLE;
	obstacle_map[ISO_H_CHEST_CLOSED].action = &chest_open_action;
	obstacle_map[ISO_H_CHEST_CLOSED].label = _("Chest");

	block_2param(ISO_E_CHEST2_CLOSED, 0.6, 0.8);
	obstacle_map[ISO_E_CHEST2_CLOSED].filename = "iso_container_0006.png";
	obstacle_map[ISO_E_CHEST2_CLOSED].flags &= ~BLOCKS_VISION_TOO;
	obstacle_map[ISO_E_CHEST2_CLOSED].flags |= GROUND_LEVEL | IS_CLICKABLE;
	obstacle_map[ISO_E_CHEST2_CLOSED].action = &chest_open_action;
	obstacle_map[ISO_E_CHEST2_CLOSED].label = _("Chest");
	
	block_2param(ISO_W_CHEST2_CLOSED, 0.6, 0.8);
	obstacle_map[ISO_W_CHEST2_CLOSED].filename = "iso_container_0010.png";
	obstacle_map[ISO_W_CHEST2_CLOSED].flags &= ~BLOCKS_VISION_TOO;
	obstacle_map[ISO_W_CHEST2_CLOSED].flags |= GROUND_LEVEL | IS_CLICKABLE;
	obstacle_map[ISO_W_CHEST2_CLOSED].action = &chest_open_action;
	obstacle_map[ISO_W_CHEST2_CLOSED].label = _("Chest");
	
	block_2param(ISO_S_CHEST2_CLOSED, 0.8, 0.6);
	obstacle_map[ISO_S_CHEST2_CLOSED].filename = "iso_container_0005.png";
	obstacle_map[ISO_S_CHEST2_CLOSED].flags &= ~BLOCKS_VISION_TOO;
	obstacle_map[ISO_S_CHEST2_CLOSED].flags |= GROUND_LEVEL | IS_CLICKABLE;
	obstacle_map[ISO_S_CHEST2_CLOSED].action = &chest_open_action;
	obstacle_map[ISO_S_CHEST2_CLOSED].label = _("Chest");
	
	block_2param(ISO_N_CHEST2_CLOSED, 0.8, 0.6);
	obstacle_map[ISO_N_CHEST2_CLOSED].filename = "iso_container_0009.png";
	obstacle_map[ISO_N_CHEST2_CLOSED].flags &= ~BLOCKS_VISION_TOO;
	obstacle_map[ISO_N_CHEST2_CLOSED].flags |= GROUND_LEVEL | IS_CLICKABLE;
	obstacle_map[ISO_N_CHEST2_CLOSED].action = &chest_open_action;
	obstacle_map[ISO_N_CHEST2_CLOSED].label = _("Chest");
	
	block_2param(ISO_E_CHEST2_OPEN, 0.6, 0.8);
	obstacle_map[ISO_E_CHEST2_OPEN].filename = "iso_container_0008.png";
	obstacle_map[ISO_E_CHEST2_OPEN].flags &= ~BLOCKS_VISION_TOO;
	obstacle_map[ISO_E_CHEST2_OPEN].flags |= GROUND_LEVEL;
	
	block_2param(ISO_W_CHEST2_OPEN, 0.6, 0.8);
	obstacle_map[ISO_W_CHEST2_OPEN].filename = "iso_container_0012.png";
	obstacle_map[ISO_W_CHEST2_OPEN].flags &= ~BLOCKS_VISION_TOO;
	obstacle_map[ISO_W_CHEST2_OPEN].flags |= GROUND_LEVEL;
	
	block_2param(ISO_S_CHEST2_OPEN, 0.8, 0.6);
	obstacle_map[ISO_S_CHEST2_OPEN].filename = "iso_container_0007.png";
	obstacle_map[ISO_S_CHEST2_OPEN].flags &= ~BLOCKS_VISION_TOO;
	obstacle_map[ISO_S_CHEST2_OPEN].flags |= GROUND_LEVEL;
	
	block_2param(ISO_N_CHEST2_OPEN, 0.8, 0.6);
	obstacle_map[ISO_N_CHEST2_OPEN].filename = "iso_container_0011.png";
	obstacle_map[ISO_N_CHEST2_OPEN].flags &= ~BLOCKS_VISION_TOO;
	obstacle_map[ISO_N_CHEST2_OPEN].flags |= GROUND_LEVEL;

	block_2param(ISO_AUTOGUN_N, 0.7, 0.7);
	obstacle_map[ISO_AUTOGUN_N].filename = "iso_autogun_act_0002.png";
	obstacle_map[ISO_AUTOGUN_N].flags &= ~BLOCKS_VISION_TOO;
	obstacle_map[ISO_AUTOGUN_N].animate_fn = animate_autogun;
	block_2param(ISO_AUTOGUN_S, 0.7, 0.7);
	obstacle_map[ISO_AUTOGUN_S].filename = "iso_autogun_act_0004.png";
	obstacle_map[ISO_AUTOGUN_S].flags &= ~BLOCKS_VISION_TOO;
	obstacle_map[ISO_AUTOGUN_S].animate_fn = animate_autogun;
	block_2param(ISO_AUTOGUN_E, 0.7, 0.7);
	obstacle_map[ISO_AUTOGUN_E].filename = "iso_autogun_act_0003.png";
	obstacle_map[ISO_AUTOGUN_E].flags &= ~BLOCKS_VISION_TOO;
	obstacle_map[ISO_AUTOGUN_E].animate_fn = animate_autogun;
	block_2param(ISO_AUTOGUN_W, 0.7, 0.7);
	obstacle_map[ISO_AUTOGUN_W].filename = "iso_autogun_act_0001.png";
	obstacle_map[ISO_AUTOGUN_W].flags &= ~BLOCKS_VISION_TOO;
	obstacle_map[ISO_AUTOGUN_W].animate_fn = animate_autogun;

	block_2param(ISO_DIS_AUTOGUN_N, 0.7, 0.7);
	obstacle_map[ISO_DIS_AUTOGUN_N].filename = "iso_autogun_0002.png";
	obstacle_map[ISO_DIS_AUTOGUN_N].flags &= ~BLOCKS_VISION_TOO;
	block_2param(ISO_DIS_AUTOGUN_S, 0.7, 0.7);
	obstacle_map[ISO_DIS_AUTOGUN_S].filename = "iso_autogun_0004.png";
	obstacle_map[ISO_DIS_AUTOGUN_S].flags &= ~BLOCKS_VISION_TOO;
	block_2param(ISO_DIS_AUTOGUN_E, 0.7, 0.7);
	obstacle_map[ISO_DIS_AUTOGUN_E].filename = "iso_autogun_0003.png";
	obstacle_map[ISO_DIS_AUTOGUN_E].flags &= ~BLOCKS_VISION_TOO;
	block_2param(ISO_DIS_AUTOGUN_W, 0.7, 0.7);
	obstacle_map[ISO_DIS_AUTOGUN_W].filename = "iso_autogun_0001.png";
	obstacle_map[ISO_DIS_AUTOGUN_W].flags &= ~BLOCKS_VISION_TOO;

	block_2param(ISO_CAVE_WALL_H, 1.5, 1.0);
	obstacle_map[ISO_CAVE_WALL_H].filename = "iso_cave_wall_0001.png";
	obstacle_map[ISO_CAVE_WALL_H].flags |= IS_HORIZONTAL;
	block_2param(ISO_CAVE_WALL_V, 1.0, 1.5);
	obstacle_map[ISO_CAVE_WALL_V].filename = "iso_cave_wall_0002.png";
	obstacle_map[ISO_CAVE_WALL_V].flags |= IS_VERTICAL;
	block_2param(ISO_CAVE_CORNER_NE, 1.0, 1.0);
	obstacle_map[ISO_CAVE_CORNER_NE].filename = "iso_cave_wall_0003.png";
	block_2param(ISO_CAVE_CORNER_SE, 1.0, 1.0);
	obstacle_map[ISO_CAVE_CORNER_SE].filename = "iso_cave_wall_0004.png";
	block_2param(ISO_CAVE_CORNER_NW, 1.0, 1.0);
	obstacle_map[ISO_CAVE_CORNER_NW].filename = "iso_cave_wall_0005.png";
	block_2param(ISO_CAVE_CORNER_SW, 1.0, 1.0);
	obstacle_map[ISO_CAVE_CORNER_SW].filename = "iso_cave_wall_0006.png";

	block_2param(ISO_COOKING_POT, 0.5, 0.5);
	obstacle_map[ISO_COOKING_POT].flags |= IS_SMASHABLE | GROUND_LEVEL;
	obstacle_map[ISO_COOKING_POT].flags &= ~BLOCKS_VISION_TOO;

	block_2param(ISO_CONSOLE_S, 0.8, 0.8);
	obstacle_map[ISO_CONSOLE_S].flags |= IS_CLICKABLE;
	obstacle_map[ISO_CONSOLE_S].flags &= ~BLOCKS_VISION_TOO;
	obstacle_map[ISO_CONSOLE_S].label = _("Terminal");
	obstacle_map[ISO_CONSOLE_S].action = &terminal_connect_action;
	obstacle_map[ISO_CONSOLE_S].filename = "iso_obstacle_0043.png";

	block_2param(ISO_CONSOLE_E, 0.8, 0.8);
	obstacle_map[ISO_CONSOLE_E].flags |= IS_CLICKABLE;
	obstacle_map[ISO_CONSOLE_E].flags &= ~BLOCKS_VISION_TOO;
	obstacle_map[ISO_CONSOLE_E].label = _("Terminal");
	obstacle_map[ISO_CONSOLE_E].action = &terminal_connect_action;
	obstacle_map[ISO_CONSOLE_E].filename = "iso_obstacle_0044.png";

	block_2param(ISO_CONSOLE_N, 0.8, 0.8);
	obstacle_map[ISO_CONSOLE_N].flags |= IS_CLICKABLE;
	obstacle_map[ISO_CONSOLE_N].flags &= ~BLOCKS_VISION_TOO;
	obstacle_map[ISO_CONSOLE_N].label = _("Terminal");
	obstacle_map[ISO_CONSOLE_N].action = &terminal_connect_action;
	obstacle_map[ISO_CONSOLE_N].filename = "iso_obstacle_0045.png";

	block_2param(ISO_CONSOLE_W, 0.8, 0.8);
	obstacle_map[ISO_CONSOLE_W].flags |= IS_CLICKABLE;
	obstacle_map[ISO_CONSOLE_W].flags &= ~BLOCKS_VISION_TOO;
	obstacle_map[ISO_CONSOLE_W].label = _("Terminal");
	obstacle_map[ISO_CONSOLE_W].action = &terminal_connect_action;
	obstacle_map[ISO_CONSOLE_W].filename = "iso_obstacle_0046.png";

	obstacle_map[ISO_BARREL_1].flags |= DROPS_RANDOM_TREASURE;
	block_2param(ISO_BARREL_1, 0.7, 0.7);
	obstacle_map[ISO_BARREL_1].flags |= IS_SMASHABLE | IS_CLICKABLE;
	obstacle_map[ISO_BARREL_1].action = &barrel_action;
	obstacle_map[ISO_BARREL_1].label = _("Barrel");
	obstacle_map[ISO_BARREL_1].filename = "iso_barrel_1.png";
	obstacle_map[ISO_BARREL_2].flags |= DROPS_RANDOM_TREASURE;
	block_2param(ISO_BARREL_2, 0.7, 0.7);
	obstacle_map[ISO_BARREL_2].flags |= IS_SMASHABLE | IS_CLICKABLE;
	obstacle_map[ISO_BARREL_2].action = &barrel_action;
	obstacle_map[ISO_BARREL_2].label = _("Barrel");
	obstacle_map[ISO_BARREL_2].filename = "iso_barrel_2.png";
	//wood crates
	obstacle_map[ISO_BARREL_3].flags |= DROPS_RANDOM_TREASURE;
	block_2param(ISO_BARREL_3, 0.80, 0.95);
	obstacle_map[ISO_BARREL_3].flags |= IS_SMASHABLE | IS_CLICKABLE;
	obstacle_map[ISO_BARREL_3].action = &barrel_action;
	obstacle_map[ISO_BARREL_3].label = _("Crate");
	obstacle_map[ISO_BARREL_3].filename = "iso_barrel_3.png";
	obstacle_map[ISO_BARREL_4].flags |= DROPS_RANDOM_TREASURE;
	block_2param(ISO_BARREL_4, 0.8, 0.75);
	obstacle_map[ISO_BARREL_4].flags |= IS_SMASHABLE | IS_CLICKABLE;
	obstacle_map[ISO_BARREL_4].action = &barrel_action;
	obstacle_map[ISO_BARREL_4].label = _("Crate");
	obstacle_map[ISO_BARREL_4].filename = "iso_barrel_4.png";
	obstacle_map[ISO_BARREL_5].flags |= DROPS_RANDOM_TREASURE;
	block_2param(ISO_BARREL_5, 0.8, 0.95);
	obstacle_map[ISO_BARREL_5].flags |= IS_SMASHABLE | IS_CLICKABLE;
	obstacle_map[ISO_BARREL_5].action = &barrel_action;
	obstacle_map[ISO_BARREL_5].label = _("Crate");
	obstacle_map[ISO_BARREL_5].filename = "iso_barrel_5.png";

	block_2param(ISO_LAMP_N, 0.5, 0.5);
	obstacle_map[ISO_LAMP_N].flags &= ~BLOCKS_VISION_TOO;
	obstacle_map[ISO_LAMP_N].flags |= GROUND_LEVEL;
	obstacle_map[ISO_LAMP_N].emitted_light_strength = 24;	// how much light emitted from here...
	obstacle_map[ISO_LAMP_N].filename = "iso_obstacle_0055.png";

	block_2param(ISO_LAMP_E, 0.5, 0.5);
	obstacle_map[ISO_LAMP_E].flags &= ~BLOCKS_VISION_TOO;
	obstacle_map[ISO_LAMP_E].flags |= GROUND_LEVEL;
	obstacle_map[ISO_LAMP_E].emitted_light_strength = 24;
	obstacle_map[ISO_LAMP_E].filename = "iso_obstacle_0056.png";

	block_2param(ISO_LAMP_S, 0.5, 0.5);
	obstacle_map[ISO_LAMP_S].flags &= ~BLOCKS_VISION_TOO;
	obstacle_map[ISO_LAMP_S].flags |= GROUND_LEVEL;
	obstacle_map[ISO_LAMP_S].emitted_light_strength = 24;
	obstacle_map[ISO_LAMP_S].filename = "iso_obstacle_0054.png";

	block_2param(ISO_LAMP_W, 0.5, 0.5);
	obstacle_map[ISO_LAMP_W].flags &= ~BLOCKS_VISION_TOO;
	obstacle_map[ISO_LAMP_W].flags |= GROUND_LEVEL;
	obstacle_map[ISO_LAMP_W].emitted_light_strength = 24;
	obstacle_map[ISO_LAMP_W].filename = "iso_obstacle_0057.png";

	// We have several types of fences.  These are typically rather
	// long and slender obstacles, which is a case that our method
	// of planting obstacles so that the visibility properties are
	// more or less correct can not so easily handle.  A feasible
	// solution is to increase the thickness of the long and slender
	// obstacles, setting e.g. the thickness to 1.1 should provide
	// some protection against small errors in the visibility on screen.
	// Maybe we will need even a bit more.  It's a fine-tuning thing.
	//
	block_2param(ISO_V_WOOD_FENCE, 1.1, 2.2);
	obstacle_map[ISO_V_WOOD_FENCE].flags &= ~BLOCKS_VISION_TOO;
	block_2param(ISO_V_DENSE_FENCE, 1.1, 2.2);
	block_2param(ISO_V_MESH_FENCE, 0.8, 2.2);
	obstacle_map[ISO_V_MESH_FENCE].flags &= ~BLOCKS_VISION_TOO;
	obstacle_map[ISO_V_MESH_FENCE].flags |= GROUND_LEVEL;
	block_2param(ISO_V_WIRE_FENCE, 0.8, 2.2);
	obstacle_map[ISO_V_WIRE_FENCE].flags &= ~BLOCKS_VISION_TOO;
	obstacle_map[ISO_V_WIRE_FENCE].flags |= GROUND_LEVEL;
	block_2param(ISO_H_WOOD_FENCE, 2.2, 1.1);
	obstacle_map[ISO_H_WOOD_FENCE].flags &= ~BLOCKS_VISION_TOO;
	block_2param(ISO_H_DENSE_FENCE, 2.2, 1.10);
	block_2param(ISO_H_MESH_FENCE, 2.2, 0.8);
	obstacle_map[ISO_H_MESH_FENCE].flags &= ~BLOCKS_VISION_TOO;
	obstacle_map[ISO_H_MESH_FENCE].flags |= GROUND_LEVEL;
	block_2param(ISO_H_WIRE_FENCE, 2.2, 0.8);
	obstacle_map[ISO_H_WIRE_FENCE].flags &= ~BLOCKS_VISION_TOO;
	obstacle_map[ISO_H_WIRE_FENCE].flags |= GROUND_LEVEL;

	block_2param(ISO_N_TOILET_SMALL, 0.4, 0.4);
	obstacle_map[ISO_N_TOILET_SMALL].flags |= IS_SMASHABLE | GROUND_LEVEL;
	obstacle_map[ISO_N_TOILET_SMALL].filename = "iso_bathroom_furniture_0008.png";
	obstacle_map[ISO_N_TOILET_SMALL].flags &= ~BLOCKS_VISION_TOO;

	block_2param(ISO_E_TOILET_SMALL, 0.4, 0.4);
	obstacle_map[ISO_E_TOILET_SMALL].flags |= IS_SMASHABLE | GROUND_LEVEL;
	obstacle_map[ISO_E_TOILET_SMALL].flags &= ~BLOCKS_VISION_TOO;
	obstacle_map[ISO_E_TOILET_SMALL].filename = "iso_bathroom_furniture_0009.png";
	block_2param(ISO_S_TOILET_SMALL, 0.4, 0.4);
	obstacle_map[ISO_S_TOILET_SMALL].flags |= IS_SMASHABLE | GROUND_LEVEL;
	obstacle_map[ISO_S_TOILET_SMALL].flags &= ~BLOCKS_VISION_TOO;
	block_2param(ISO_W_TOILET_SMALL, 0.4, 0.4);
	obstacle_map[ISO_W_TOILET_SMALL].flags |= IS_SMASHABLE | GROUND_LEVEL;
	obstacle_map[ISO_W_TOILET_SMALL].flags &= ~BLOCKS_VISION_TOO;

	block_2param(ISO_N_TOILET_BIG, 0.4, 0.4);
	obstacle_map[ISO_N_TOILET_BIG].flags |= IS_SMASHABLE | GROUND_LEVEL;
	obstacle_map[ISO_N_TOILET_BIG].flags &= ~BLOCKS_VISION_TOO;
	obstacle_map[ISO_N_TOILET_BIG].filename = "iso_bathroom_furniture_0004.png";
	block_2param(ISO_E_TOILET_BIG, 0.4, 0.4);
	obstacle_map[ISO_E_TOILET_BIG].flags |= IS_SMASHABLE | GROUND_LEVEL;
	obstacle_map[ISO_E_TOILET_BIG].flags &= ~BLOCKS_VISION_TOO;
	obstacle_map[ISO_E_TOILET_BIG].filename = "iso_bathroom_furniture_0005.png";
	block_2param(ISO_S_TOILET_BIG, 0.4, 0.4);
	obstacle_map[ISO_S_TOILET_BIG].flags |= IS_SMASHABLE | GROUND_LEVEL;
	obstacle_map[ISO_S_TOILET_BIG].flags &= ~BLOCKS_VISION_TOO;
	obstacle_map[ISO_S_TOILET_BIG].filename = "iso_bathroom_furniture_0006.png";
	block_2param(ISO_W_TOILET_BIG, 0.4, 0.4);
	obstacle_map[ISO_W_TOILET_BIG].flags |= IS_SMASHABLE | GROUND_LEVEL;
	obstacle_map[ISO_W_TOILET_BIG].flags &= ~BLOCKS_VISION_TOO;
	obstacle_map[ISO_W_TOILET_BIG].filename = "iso_bathroom_furniture_0007.png";

	block_2param(ISO_N_CHAIR, 0.4, 0.4);
	obstacle_map[ISO_N_CHAIR].flags |= IS_SMASHABLE;
	obstacle_map[ISO_N_CHAIR].filename = "iso_chairs_0009.png";
	obstacle_map[ISO_N_CHAIR].flags &= ~BLOCKS_VISION_TOO;
	block_2param(ISO_E_CHAIR, 0.4, 0.4);
	obstacle_map[ISO_E_CHAIR].flags |= IS_SMASHABLE;
	obstacle_map[ISO_E_CHAIR].filename = "iso_chairs_0010.png";
	obstacle_map[ISO_E_CHAIR].flags &= ~BLOCKS_VISION_TOO;
	block_2param(ISO_S_CHAIR, 0.4, 0.4);
	obstacle_map[ISO_S_CHAIR].flags |= IS_SMASHABLE;
	obstacle_map[ISO_S_CHAIR].filename = "iso_chairs_0011.png";
	obstacle_map[ISO_S_CHAIR].flags &= ~BLOCKS_VISION_TOO;
	block_2param(ISO_W_CHAIR, 0.4, 0.4);
	obstacle_map[ISO_W_CHAIR].flags |= IS_SMASHABLE;
	obstacle_map[ISO_W_CHAIR].flags &= ~BLOCKS_VISION_TOO;
	obstacle_map[ISO_W_CHAIR].filename = "iso_chairs_0012.png";

	block_2param(ISO_SOFFA_1, 0.6, 1.2);
	obstacle_map[ISO_SOFFA_1].flags &= ~BLOCKS_VISION_TOO;
	obstacle_map[ISO_SOFFA_1].flags |= GROUND_LEVEL;
	obstacle_map[ISO_SOFFA_1].filename = "iso_chairs_0013.png";

	block_2param(ISO_SOFFA_2, 1.2, 0.6);
	obstacle_map[ISO_SOFFA_2].flags &= ~BLOCKS_VISION_TOO;
	obstacle_map[ISO_SOFFA_2].flags |= GROUND_LEVEL;
	obstacle_map[ISO_SOFFA_2].filename = "iso_chairs_0014.png";

	block_2param(ISO_SOFFA_3, 0.6, 1.2);
	obstacle_map[ISO_SOFFA_3].flags &= ~BLOCKS_VISION_TOO;
	obstacle_map[ISO_SOFFA_3].flags |= GROUND_LEVEL;
	obstacle_map[ISO_SOFFA_3].filename = "iso_chairs_0015.png";

	block_2param(ISO_SOFFA_4, 1.2, 0.6);
	obstacle_map[ISO_SOFFA_4].flags &= ~BLOCKS_VISION_TOO;
	obstacle_map[ISO_SOFFA_4].flags |= GROUND_LEVEL;
	obstacle_map[ISO_SOFFA_4].filename = "iso_chairs_0016.png";

	block_2param(ISO_SOFFA_CORNER_1, 0.6, 0.6);
	obstacle_map[ISO_SOFFA_CORNER_1].flags &= ~BLOCKS_VISION_TOO;
	obstacle_map[ISO_SOFFA_CORNER_1].flags |= GROUND_LEVEL;
	obstacle_map[ISO_SOFFA_CORNER_1].filename = "iso_chairs_0017.png";

	block_2param(ISO_SOFFA_CORNER_2, 0.6, 0.6);
	obstacle_map[ISO_SOFFA_CORNER_2].flags &= ~BLOCKS_VISION_TOO;
	obstacle_map[ISO_SOFFA_CORNER_2].flags |= GROUND_LEVEL;
	obstacle_map[ISO_SOFFA_CORNER_2].filename = "iso_chairs_0018.png";

	block_2param(ISO_SOFFA_CORNER_3, 0.6, 0.6);
	obstacle_map[ISO_SOFFA_CORNER_3].flags &= ~BLOCKS_VISION_TOO;
	obstacle_map[ISO_SOFFA_CORNER_3].flags |= GROUND_LEVEL;
	obstacle_map[ISO_SOFFA_CORNER_3].filename = "iso_chairs_0019.png";

	block_2param(ISO_SOFFA_CORNER_4, 0.6, 0.6);
	obstacle_map[ISO_SOFFA_CORNER_4].flags &= ~BLOCKS_VISION_TOO;
	obstacle_map[ISO_SOFFA_CORNER_4].flags |= GROUND_LEVEL;
	obstacle_map[ISO_SOFFA_CORNER_4].filename = "iso_chairs_0020.png";

	block_2param(ISO_SOFFA_CORNER_PLANT_1, 0.6, 0.6);
	obstacle_map[ISO_SOFFA_CORNER_PLANT_1].flags &= ~BLOCKS_VISION_TOO;
	obstacle_map[ISO_SOFFA_CORNER_PLANT_1].flags |= GROUND_LEVEL;
	obstacle_map[ISO_SOFFA_CORNER_PLANT_1].filename = "iso_chairs_0021.png";

	block_2param(ISO_SOFFA_CORNER_PLANT_2, 0.6, 0.6);
	obstacle_map[ISO_SOFFA_CORNER_PLANT_2].flags &= ~BLOCKS_VISION_TOO;
	obstacle_map[ISO_SOFFA_CORNER_PLANT_2].flags |= GROUND_LEVEL;
	obstacle_map[ISO_SOFFA_CORNER_PLANT_2].filename = "iso_chairs_0022.png";

	block_2param(ISO_SOFFA_CORNER_PLANT_3, 0.6, 0.6);
	obstacle_map[ISO_SOFFA_CORNER_PLANT_3].flags &= ~BLOCKS_VISION_TOO;
	obstacle_map[ISO_SOFFA_CORNER_PLANT_3].flags |= GROUND_LEVEL;
	obstacle_map[ISO_SOFFA_CORNER_PLANT_3].filename = "iso_chairs_0023.png";

	block_2param(ISO_SOFFA_CORNER_PLANT_4, 0.6, 0.6);
	obstacle_map[ISO_SOFFA_CORNER_PLANT_4].flags &= ~BLOCKS_VISION_TOO;
	obstacle_map[ISO_SOFFA_CORNER_PLANT_4].flags |= GROUND_LEVEL;
	obstacle_map[ISO_SOFFA_CORNER_PLANT_4].filename = "iso_chairs_0024.png";

	block_2param(ISO_N_DESK, 0.4, 1.0);
	obstacle_map[ISO_N_DESK].flags &= ~BLOCKS_VISION_TOO;
	obstacle_map[ISO_N_DESK].flags |= IS_SMASHABLE | GROUND_LEVEL;
	obstacle_map[ISO_N_DESK].filename = "iso_tables_0001.png";
	block_2param(ISO_E_DESK, 1.0, 0.4);
	obstacle_map[ISO_E_DESK].flags &= ~BLOCKS_VISION_TOO;
	obstacle_map[ISO_E_DESK].flags |= IS_SMASHABLE | GROUND_LEVEL;
	obstacle_map[ISO_E_DESK].filename = "iso_tables_0002.png";
	block_2param(ISO_S_DESK, 0.4, 1.0);
	obstacle_map[ISO_S_DESK].flags &= ~BLOCKS_VISION_TOO;
	obstacle_map[ISO_S_DESK].flags |= IS_SMASHABLE | GROUND_LEVEL;
	obstacle_map[ISO_S_DESK].filename = "iso_tables_0003.png";
	block_2param(ISO_W_DESK, 1.0, 0.4);
	obstacle_map[ISO_W_DESK].flags &= ~BLOCKS_VISION_TOO;
	obstacle_map[ISO_W_DESK].flags |= IS_SMASHABLE | GROUND_LEVEL;
	obstacle_map[ISO_W_DESK].filename = "iso_tables_0004.png";

	block_2param(ISO_N_SCHOOL_CHAIR, 0.4, 0.4);
	obstacle_map[ISO_N_SCHOOL_CHAIR].flags |= IS_SMASHABLE | GROUND_LEVEL;
	obstacle_map[ISO_N_SCHOOL_CHAIR].flags &= ~BLOCKS_VISION_TOO;
	block_2param(ISO_E_SCHOOL_CHAIR, 0.4, 0.4);
	obstacle_map[ISO_E_SCHOOL_CHAIR].flags |= IS_SMASHABLE | GROUND_LEVEL;
	obstacle_map[ISO_E_SCHOOL_CHAIR].flags &= ~BLOCKS_VISION_TOO;
	block_2param(ISO_S_SCHOOL_CHAIR, 0.4, 0.4);
	obstacle_map[ISO_S_SCHOOL_CHAIR].flags |= IS_SMASHABLE | GROUND_LEVEL;
	obstacle_map[ISO_S_SCHOOL_CHAIR].flags &= ~BLOCKS_VISION_TOO;
	block_2param(ISO_W_SCHOOL_CHAIR, 0.4, 0.4);
	obstacle_map[ISO_W_SCHOOL_CHAIR].flags |= IS_SMASHABLE | GROUND_LEVEL;
	obstacle_map[ISO_W_SCHOOL_CHAIR].flags &= ~BLOCKS_VISION_TOO;

	block_2param(ISO_N_BED, 1.1, 0.7);
	obstacle_map[ISO_N_BED].flags |= IS_SMASHABLE | GROUND_LEVEL;
	obstacle_map[ISO_N_BED].flags &= ~BLOCKS_VISION_TOO;
	block_2param(ISO_E_BED, 0.7, 1.1);
	obstacle_map[ISO_E_BED].flags |= IS_SMASHABLE | GROUND_LEVEL;
	obstacle_map[ISO_E_BED].flags &= ~BLOCKS_VISION_TOO;
	block_2param(ISO_S_BED, 1.1, 0.7);
	obstacle_map[ISO_S_BED].flags |= IS_SMASHABLE | GROUND_LEVEL;
	obstacle_map[ISO_S_BED].flags &= ~BLOCKS_VISION_TOO;
	block_2param(ISO_W_BED, 0.7, 1.1);
	obstacle_map[ISO_W_BED].flags |= IS_SMASHABLE | GROUND_LEVEL;
	obstacle_map[ISO_W_BED].flags &= ~BLOCKS_VISION_TOO;

	block_2param(ISO_N_FULL_PARK_BENCH, 1, 1.3);
	obstacle_map[ISO_N_FULL_PARK_BENCH].flags |= IS_SMASHABLE | GROUND_LEVEL;
	obstacle_map[ISO_N_FULL_PARK_BENCH].flags &= ~BLOCKS_VISION_TOO;
	block_2param(ISO_E_FULL_PARK_BENCH, 1.3, 1);
	obstacle_map[ISO_E_FULL_PARK_BENCH].flags |= IS_SMASHABLE | GROUND_LEVEL;
	obstacle_map[ISO_E_FULL_PARK_BENCH].flags &= ~BLOCKS_VISION_TOO;
	block_2param(ISO_S_FULL_PARK_BENCH, 1.3, 1);
	obstacle_map[ISO_S_FULL_PARK_BENCH].flags |= IS_SMASHABLE | GROUND_LEVEL;
	obstacle_map[ISO_S_FULL_PARK_BENCH].flags &= ~BLOCKS_VISION_TOO;
	block_2param(ISO_W_FULL_PARK_BENCH, 1, 1.3);
	obstacle_map[ISO_W_FULL_PARK_BENCH].flags |= IS_SMASHABLE | GROUND_LEVEL;
	obstacle_map[ISO_W_FULL_PARK_BENCH].flags &= ~BLOCKS_VISION_TOO;

	block_2param(ISO_H_BATHTUB, 1.5, 1.0);
	obstacle_map[ISO_H_BATHTUB].flags |= IS_SMASHABLE | GROUND_LEVEL;
	obstacle_map[ISO_H_BATHTUB].flags &= ~BLOCKS_VISION_TOO;
	obstacle_map[ISO_H_BATHTUB].filename = "iso_bathroom_furniture_0000.png";
	block_2param(ISO_V_BATHTUB, 1.0, 1.5);
	obstacle_map[ISO_V_BATHTUB].flags |= IS_SMASHABLE | GROUND_LEVEL;
	obstacle_map[ISO_V_BATHTUB].filename = "iso_bathroom_furniture_0001.png";
	obstacle_map[ISO_V_BATHTUB].flags &= ~BLOCKS_VISION_TOO;

	block_2param(ISO_3_BATHTUB, 1.5, 1.0);
	obstacle_map[ISO_3_BATHTUB].flags |= IS_SMASHABLE | GROUND_LEVEL;
	obstacle_map[ISO_3_BATHTUB].flags &= ~BLOCKS_VISION_TOO;
	obstacle_map[ISO_3_BATHTUB].filename = "iso_bathroom_furniture_0002.png";

	block_2param(ISO_4_BATHTUB, 1.0, 1.5);
	obstacle_map[ISO_4_BATHTUB].flags |= IS_SMASHABLE | GROUND_LEVEL;
	obstacle_map[ISO_4_BATHTUB].filename = "iso_bathroom_furniture_0003.png";
	obstacle_map[ISO_4_BATHTUB].flags &= ~BLOCKS_VISION_TOO;

	block_2param(ISO_H_WASHTUB, 0.5, 0.4);
	obstacle_map[ISO_H_WASHTUB].flags |= IS_SMASHABLE | GROUND_LEVEL;
	obstacle_map[ISO_H_WASHTUB].flags &= ~BLOCKS_VISION_TOO;

	block_2param(ISO_V_WASHTUB, 0.4, 0.5);
	obstacle_map[ISO_V_WASHTUB].flags |= IS_SMASHABLE | GROUND_LEVEL;
	obstacle_map[ISO_V_WASHTUB].flags &= ~BLOCKS_VISION_TOO;

	obstacle_map[ISO_V_CURTAIN].block_area_type = COLLISION_TYPE_NONE;
	obstacle_map[ISO_V_CURTAIN].flags |= IS_VERTICAL;
	obstacle_map[ISO_H_CURTAIN].block_area_type = COLLISION_TYPE_NONE;
	obstacle_map[ISO_H_CURTAIN].flags |= IS_HORIZONTAL;

	block_2param(ISO_N_SOFA, 1.0, 0.5);
	obstacle_map[ISO_N_SOFA].flags |= GROUND_LEVEL;
	obstacle_map[ISO_N_SOFA].flags &= ~BLOCKS_VISION_TOO;
	block_2param(ISO_S_SOFA, 1.0, 0.5);
	obstacle_map[ISO_S_SOFA].flags |= GROUND_LEVEL;
	obstacle_map[ISO_S_SOFA].flags &= ~BLOCKS_VISION_TOO;
	block_2param(ISO_E_SOFA, 0.5, 1.0);
	obstacle_map[ISO_E_SOFA].flags |= GROUND_LEVEL;
	obstacle_map[ISO_E_SOFA].flags &= ~BLOCKS_VISION_TOO;
	block_2param(ISO_W_SOFA, 0.5, 1.0);
	obstacle_map[ISO_W_SOFA].flags |= GROUND_LEVEL;
	obstacle_map[ISO_W_SOFA].flags &= ~BLOCKS_VISION_TOO;

	block_2param(ISO_TREE_1, 0.6, 0.6);
	obstacle_map[ISO_TREE_1].flags &= ~BLOCKS_VISION_TOO;
	obstacle_map[ISO_TREE_1].filename = "iso_obstacle_0113.png";
	block_2param(ISO_TREE_2, 0.6, 0.6);
	obstacle_map[ISO_TREE_2].flags &= ~BLOCKS_VISION_TOO;
	obstacle_map[ISO_TREE_2].filename = "iso_obstacle_0114.png";
	block_2param(ISO_TREE_3, 0.6, 0.8);
	obstacle_map[ISO_TREE_3].flags &= ~BLOCKS_VISION_TOO;
	obstacle_map[ISO_TREE_3].filename = "iso_obstacle_0115.png";
	block_2param(ISO_TREE_4, 0.6, 0.6);
	obstacle_map[ISO_TREE_4].filename = "iso_tree_0000.png";
	obstacle_map[ISO_TREE_4].flags &= ~BLOCKS_VISION_TOO;
	block_2param(ISO_TREE_5, 1.3, 1.3);
	obstacle_map[ISO_TREE_5].filename = "iso_tree_0001.png";
	obstacle_map[ISO_TREE_5].flags &= ~BLOCKS_VISION_TOO;

	for (i = ISO_THICK_WALL_H; i <= ISO_THICK_WALL_T_W; i++) {
		obstacle_map[i].transparent = TRANSPARENCY_FOR_WALLS;
	}

	block_2param(ISO_THICK_WALL_H, standard_wall_width, standard_wall_thickness);
	obstacle_map[ISO_THICK_WALL_H].filename = "iso_thick_wall_0001.png";
	obstacle_map[ISO_THICK_WALL_H].flags |= IS_HORIZONTAL;
	block_2param(ISO_THICK_WALL_V, standard_wall_thickness, standard_wall_width);
	obstacle_map[ISO_THICK_WALL_V].filename = "iso_thick_wall_0002.png";
	obstacle_map[ISO_THICK_WALL_V].flags |= IS_VERTICAL;
	block_2param(ISO_THICK_WALL_CORNER_NE, standard_wall_width, standard_wall_width);
	obstacle_map[ISO_THICK_WALL_CORNER_NE].filename = "iso_thick_wall_0003.png";
	block_2param(ISO_THICK_WALL_CORNER_SE, standard_wall_width, standard_wall_width);
	obstacle_map[ISO_THICK_WALL_CORNER_SE].filename = "iso_thick_wall_0004.png";
	block_2param(ISO_THICK_WALL_CORNER_NW, standard_wall_width, standard_wall_width);
	obstacle_map[ISO_THICK_WALL_CORNER_NW].filename = "iso_thick_wall_0005.png";
	block_2param(ISO_THICK_WALL_CORNER_SW, standard_wall_width, standard_wall_width);
	obstacle_map[ISO_THICK_WALL_CORNER_SW].filename = "iso_thick_wall_0006.png";
	block_2param(ISO_THICK_WALL_T_N, standard_wall_width, standard_wall_width);
	obstacle_map[ISO_THICK_WALL_T_N].filename = "iso_thick_wall_0007.png";
	block_2param(ISO_THICK_WALL_T_E, standard_wall_width, standard_wall_width);
	obstacle_map[ISO_THICK_WALL_T_E].filename = "iso_thick_wall_0008.png";
	block_2param(ISO_THICK_WALL_T_S, standard_wall_width, standard_wall_width);
	obstacle_map[ISO_THICK_WALL_T_S].filename = "iso_thick_wall_0009.png";
	block_2param(ISO_THICK_WALL_T_W, standard_wall_width, standard_wall_width);
	obstacle_map[ISO_THICK_WALL_T_W].filename = "iso_thick_wall_0010.png";

	// restaurant stuff

	block_2param(ISO_RESTAURANT_SHELVES_1, 0.6, 0.6);
	obstacle_map[ISO_RESTAURANT_SHELVES_1].filename = "iso_restaurant_furniture_0001.png";
	block_2param(ISO_RESTAURANT_SHELVES_2, 0.6, 0.6);
	obstacle_map[ISO_RESTAURANT_SHELVES_2].filename = "iso_restaurant_furniture_0002.png";
	block_2param(ISO_RESTAURANT_SHELVES_3, 0.6, 0.6);
	obstacle_map[ISO_RESTAURANT_SHELVES_3].filename = "iso_restaurant_furniture_0003.png";
	block_2param(ISO_RESTAURANT_SHELVES_4, 0.6, 0.6);
	obstacle_map[ISO_RESTAURANT_SHELVES_4].filename = "iso_restaurant_furniture_0004.png";
	block_2param(ISO_RESTAURANT_SHELVES_5, 0.6, 0.6);
	obstacle_map[ISO_RESTAURANT_SHELVES_5].filename = "iso_restaurant_furniture_0005.png";
	block_2param(ISO_RESTAURANT_SHELVES_6, 0.6, 0.6);
	obstacle_map[ISO_RESTAURANT_SHELVES_6].filename = "iso_restaurant_furniture_0006.png";
	block_2param(ISO_RESTAURANT_SHELVES_7, 0.6, 0.6);
	obstacle_map[ISO_RESTAURANT_SHELVES_7].filename = "iso_restaurant_furniture_0007.png";
	block_2param(ISO_RESTAURANT_SHELVES_8, 0.6, 0.6);
	obstacle_map[ISO_RESTAURANT_SHELVES_8].filename = "iso_restaurant_furniture_0008.png";
	block_2param(ISO_RESTAURANT_SHELVES_9, 0.6, 0.6);
	obstacle_map[ISO_RESTAURANT_SHELVES_9].filename = "iso_restaurant_furniture_0009.png";
	block_2param(ISO_RESTAURANT_SHELVES_10, 0.6, 0.6);
	obstacle_map[ISO_RESTAURANT_SHELVES_10].filename = "iso_restaurant_furniture_0010.png";

	int ahrot = 0;
	for (ahrot = ISO_RESTAURANT_SHELVES_1; ahrot <= ISO_RESTAURANT_SHELVES_10; ahrot++) {
	}

	block_2param(ISO_CAVE_WALL_END_W, 1.0, 1.0);
	obstacle_map[ISO_CAVE_WALL_END_W].filename = "iso_cave_wall_0007.png";
	block_2param(ISO_CAVE_WALL_END_N, 1.0, 1.0);
	obstacle_map[ISO_CAVE_WALL_END_N].filename = "iso_cave_wall_0008.png";
	block_2param(ISO_CAVE_WALL_END_E, 1.0, 1.0);
	obstacle_map[ISO_CAVE_WALL_END_E].filename = "iso_cave_wall_0009.png";
	block_2param(ISO_CAVE_WALL_END_S, 1.0, 1.0);
	obstacle_map[ISO_CAVE_WALL_END_S].filename = "iso_cave_wall_0010.png";

	block_2param(ISO_GREY_WALL_END_W, standard_wall_thickness, standard_wall_width);
	obstacle_map[ISO_GREY_WALL_END_W].filename = "iso_walls_0005.png";
	obstacle_map[ISO_GREY_WALL_END_W].transparent = TRANSPARENCY_FOR_WALLS;
	obstacle_map[ISO_GREY_WALL_END_W].flags &= ~BLOCKS_VISION_TOO;
	obstacle_map[ISO_GREY_WALL_END_W].flags |= IS_VERTICAL;

	block_2param(ISO_GREY_WALL_END_N, standard_wall_width, standard_wall_thickness);
	obstacle_map[ISO_GREY_WALL_END_N].flags &= ~BLOCKS_VISION_TOO;
	obstacle_map[ISO_GREY_WALL_END_N].flags |= IS_HORIZONTAL;
	obstacle_map[ISO_GREY_WALL_END_N].filename = "iso_walls_0006.png";
	obstacle_map[ISO_GREY_WALL_END_N].transparent = TRANSPARENCY_FOR_WALLS;

	/*description for all ISO_GREY_WALL_END_ */
  /**************************************/
	block_2param(ISO_GREY_WALL_END_E, standard_wall_thickness, standard_wall_width);
	obstacle_map[ISO_GREY_WALL_END_E].filename = "iso_walls_0007.png";
	obstacle_map[ISO_GREY_WALL_END_E].flags |= IS_VERTICAL;
	obstacle_map[ISO_GREY_WALL_END_E].transparent = TRANSPARENCY_FOR_WALLS;

	block_2param(ISO_GREY_WALL_END_S, standard_wall_width, standard_wall_thickness);
	obstacle_map[ISO_GREY_WALL_END_S].flags |= IS_HORIZONTAL;
	obstacle_map[ISO_GREY_WALL_END_S].filename = "iso_walls_0008.png";
	obstacle_map[ISO_GREY_WALL_END_S].transparent = TRANSPARENCY_FOR_WALLS;

	block_2param(ISO_GREY_WALL_CORNER_1, 1.1, 1.0);
	obstacle_map[ISO_GREY_WALL_CORNER_1].filename = "iso_wall_corners_0001.png";
	obstacle_map[ISO_GREY_WALL_CORNER_1].transparent = TRANSPARENCY_FOR_WALLS;

	block_2param(ISO_GREY_WALL_CORNER_2, 1.1, 1.0);
	obstacle_map[ISO_GREY_WALL_CORNER_2].filename = "iso_wall_corners_0002.png";
	obstacle_map[ISO_GREY_WALL_CORNER_2].transparent = TRANSPARENCY_FOR_WALLS;

	block_2param(ISO_GREY_WALL_CORNER_3, 1.1, 1.0);
	obstacle_map[ISO_GREY_WALL_CORNER_3].filename = "iso_wall_corners_0003.png";
	obstacle_map[ISO_GREY_WALL_CORNER_3].transparent = TRANSPARENCY_FOR_WALLS;

	block_2param(ISO_GREY_WALL_CORNER_4, 1.1, 1.0);
	obstacle_map[ISO_GREY_WALL_CORNER_4].filename = "iso_wall_corners_0004.png";
	obstacle_map[ISO_GREY_WALL_CORNER_4].transparent = TRANSPARENCY_FOR_WALLS;

	for (i = ISO_LIGHT_GREEN_WALL_1; i <= ISO_FUNKY_WALL_4; i++) {
		obstacle_map[i].transparent = TRANSPARENCY_FOR_WALLS;
	}

	block_2param(ISO_LIGHT_GREEN_WALL_1, standard_wall_thickness, standard_wall_width);
	obstacle_map[ISO_LIGHT_GREEN_WALL_1].filename = "iso_walls_0010.png";
	obstacle_map[ISO_LIGHT_GREEN_WALL_1].flags |= IS_VERTICAL;

	block_2param(ISO_LIGHT_GREEN_WALL_2, standard_wall_width, standard_wall_thickness);
	obstacle_map[ISO_LIGHT_GREEN_WALL_2].filename = "iso_walls_0011.png";
	obstacle_map[ISO_LIGHT_GREEN_WALL_2].flags |= IS_HORIZONTAL;

	block_2param(ISO_FUNKY_WALL_1, standard_wall_thickness, standard_wall_width);
	obstacle_map[ISO_FUNKY_WALL_1].filename = "iso_walls_0012.png";
	obstacle_map[ISO_FUNKY_WALL_1].flags |= IS_VERTICAL;

	block_2param(ISO_FUNKY_WALL_2, standard_wall_width, standard_wall_thickness);
	obstacle_map[ISO_FUNKY_WALL_2].filename = "iso_walls_0013.png";
	obstacle_map[ISO_FUNKY_WALL_2].flags |= IS_HORIZONTAL;

	block_2param(ISO_FUNKY_WALL_3, standard_wall_thickness, standard_wall_width);
	obstacle_map[ISO_FUNKY_WALL_3].filename = "iso_walls_0014.png";
	obstacle_map[ISO_FUNKY_WALL_3].flags |= IS_VERTICAL;

	block_2param(ISO_FUNKY_WALL_4, standard_wall_width, standard_wall_thickness);
	obstacle_map[ISO_FUNKY_WALL_4].filename = "iso_walls_0015.png";
	obstacle_map[ISO_FUNKY_WALL_4].flags |= IS_HORIZONTAL;

	for (i = ISO_BRICK_WALL_H; i <= ISO_BRICK_WALL_CORNER_SE; i++) {
		obstacle_map[i].transparent = TRANSPARENCY_FOR_WALLS;
	}

	// Brick walls are smashable.  When you smash them, there should be
	// first a cracked brick wall, then when smashing again, there will
	// be only some rubble left, and that should be *passable*.
	block_2param(ISO_BRICK_WALL_H, 1.2, 0.8);
	obstacle_map[ISO_BRICK_WALL_H].filename = "iso_brick_wall_0002.png";
	obstacle_map[ISO_BRICK_WALL_H].result_type_after_smashing_once = ISO_BRICK_WALL_CRACKED_1;
	obstacle_map[ISO_BRICK_WALL_H].flags |= IS_HORIZONTAL;
	block_2param(ISO_BRICK_WALL_V, 0.8, 1.2);
	obstacle_map[ISO_BRICK_WALL_V].filename = "iso_brick_wall_0001.png";
	obstacle_map[ISO_BRICK_WALL_V].result_type_after_smashing_once = ISO_BRICK_WALL_CRACKED_2;
	obstacle_map[ISO_BRICK_WALL_V].flags |= IS_VERTICAL;

	block_2param(ISO_BRICK_WALL_END, 1.0, 1.0);
	obstacle_map[ISO_BRICK_WALL_END].filename = "iso_brick_wall_0003.png";

	block_2param(ISO_BRICK_WALL_CABLES_H, 1.2, 0.8);
	obstacle_map[ISO_BRICK_WALL_CABLES_H].filename = "iso_brick_wall_cables_0001.png";
	obstacle_map[ISO_BRICK_WALL_CABLES_H].transparent = TRANSPARENCY_FOR_WALLS;
	obstacle_map[ISO_BRICK_WALL_CABLES_H].flags |= IS_HORIZONTAL;
	block_2param(ISO_BRICK_WALL_CABLES_V, 0.8, 1.2);
	obstacle_map[ISO_BRICK_WALL_CABLES_V].filename = "iso_brick_wall_cables_0002.png";
	obstacle_map[ISO_BRICK_WALL_CABLES_V].transparent = TRANSPARENCY_FOR_WALLS;
	obstacle_map[ISO_BRICK_WALL_CABLES_V].flags |= IS_VERTICAL;

	block_4param(ISO_BRICK_WALL_CORNER_NE, 0.3, 0.6, 0.6, 0.6);
	obstacle_map[ISO_BRICK_WALL_CORNER_NE].filename = "iso_brick_wall_0004.png";
	block_4param(ISO_BRICK_WALL_CORNER_SW, 0.65, 0.3, 0.6, 0.6);
	obstacle_map[ISO_BRICK_WALL_CORNER_SW].filename = "iso_brick_wall_0005.png";
	block_4param(ISO_BRICK_WALL_CORNER_NW, 0.6, 0.6, 0.3, 0.3);
	obstacle_map[ISO_BRICK_WALL_CORNER_NW].filename = "iso_brick_wall_0006.png";
	block_4param(ISO_BRICK_WALL_CORNER_SE, 0.3, 0.3, 0.6, 0.6);
	obstacle_map[ISO_BRICK_WALL_CORNER_SE].filename = "iso_brick_wall_0007.png";

	block_4param(ISO_BRICK_WALL_CABLES_CORNER_NE, 0.3, 0.6, 0.6, 0.6);
	obstacle_map[ISO_BRICK_WALL_CABLES_CORNER_NE].filename = "iso_brick_wall_cables_0004.png";
	obstacle_map[ISO_BRICK_WALL_CABLES_CORNER_NE].transparent = TRANSPARENCY_FOR_WALLS;
	block_4param(ISO_BRICK_WALL_CABLES_CORNER_SW, 0.65, 0.3, 0.6, 0.6);
	obstacle_map[ISO_BRICK_WALL_CABLES_CORNER_SW].filename = "iso_brick_wall_cables_0005.png";
	obstacle_map[ISO_BRICK_WALL_CABLES_CORNER_SW].transparent = TRANSPARENCY_FOR_WALLS;
	block_4param(ISO_BRICK_WALL_CABLES_CORNER_NW, 0.6, 0.6, 0.3, 0.3);
	obstacle_map[ISO_BRICK_WALL_CABLES_CORNER_NW].filename = "iso_brick_wall_cables_0006.png";
	obstacle_map[ISO_BRICK_WALL_CABLES_CORNER_NW].transparent = TRANSPARENCY_FOR_WALLS;
	block_4param(ISO_BRICK_WALL_CABLES_CORNER_SE, 0.3, 0.3, 0.6, 0.6);
	obstacle_map[ISO_BRICK_WALL_CABLES_CORNER_SE].filename = "iso_brick_wall_cables_0007.png";
	obstacle_map[ISO_BRICK_WALL_CABLES_CORNER_SE].transparent = TRANSPARENCY_FOR_WALLS;

	//all cabled brick walls above share the same description

	for (i = ISO_BRICK_WALL_JUNCTION_1; i <= ISO_BRICK_WALL_RUBBLE_2; i++) {
		obstacle_map[i].transparent = TRANSPARENCY_FOR_WALLS;
	}

	block_2param(ISO_BRICK_WALL_JUNCTION_1, 1.2, 1.2);
	obstacle_map[ISO_BRICK_WALL_JUNCTION_1].filename = "iso_brick_wall_0008.png";
	block_2param(ISO_BRICK_WALL_JUNCTION_2, 1.2, 1.2);
	obstacle_map[ISO_BRICK_WALL_JUNCTION_2].filename = "iso_brick_wall_0009.png";
	block_2param(ISO_BRICK_WALL_JUNCTION_3, 1.2, 1.2);
	obstacle_map[ISO_BRICK_WALL_JUNCTION_3].filename = "iso_brick_wall_0010.png";
	block_2param(ISO_BRICK_WALL_JUNCTION_4, 1.2, 1.2);
	obstacle_map[ISO_BRICK_WALL_JUNCTION_4].filename = "iso_brick_wall_0011.png";

	//all brick walls above share the same description

	// Brick walls are smashable.  When you smash them, there should be
	// first a cracked brick wall, then when smashing again, there will
	// be only some rubble left, and that should be *passable*.
	//
	block_2param(ISO_BRICK_WALL_CRACKED_1, 0.5, 1.2);
	obstacle_map[ISO_BRICK_WALL_CRACKED_1].filename = "iso_brick_wall_0012.png";
	obstacle_map[ISO_BRICK_WALL_CRACKED_1].flags |= IS_SMASHABLE | IS_CLICKABLE;
	obstacle_map[ISO_BRICK_WALL_CRACKED_1].label = "";
	obstacle_map[ISO_BRICK_WALL_CRACKED_1].action = &barrel_action;
	obstacle_map[ISO_BRICK_WALL_CRACKED_1].result_type_after_smashing_once = ISO_BRICK_WALL_RUBBLE_1;
	obstacle_map[ISO_BRICK_WALL_CRACKED_1].flags |= IS_VERTICAL;
	block_2param(ISO_BRICK_WALL_CRACKED_2, 1.2, 0.5);
	obstacle_map[ISO_BRICK_WALL_CRACKED_2].filename = "iso_brick_wall_0013.png";
	obstacle_map[ISO_BRICK_WALL_CRACKED_2].result_type_after_smashing_once = ISO_BRICK_WALL_RUBBLE_2;
	obstacle_map[ISO_BRICK_WALL_CRACKED_2].flags |= IS_SMASHABLE | IS_CLICKABLE;
	obstacle_map[ISO_BRICK_WALL_CRACKED_2].label = "";
	obstacle_map[ISO_BRICK_WALL_CRACKED_2].action = &barrel_action;
	obstacle_map[ISO_BRICK_WALL_CRACKED_2].flags |= IS_HORIZONTAL;

	obstacle_map[ISO_BRICK_WALL_RUBBLE_1].block_area_type = COLLISION_TYPE_NONE;
	obstacle_map[ISO_BRICK_WALL_RUBBLE_1].filename = "iso_brick_wall_0014.png";
	obstacle_map[ISO_BRICK_WALL_RUBBLE_1].flags |= IS_VERTICAL;
	obstacle_map[ISO_BRICK_WALL_RUBBLE_2].block_area_type = COLLISION_TYPE_NONE;
	obstacle_map[ISO_BRICK_WALL_RUBBLE_2].filename = "iso_brick_wall_0015.png";
	obstacle_map[ISO_BRICK_WALL_RUBBLE_2].flags |= IS_HORIZONTAL;

	block_2param(ISO_BRICK_WALL_EH, 1.2, 0.8);
	obstacle_map[ISO_BRICK_WALL_EH].filename = "iso_brick_wall_0017.png";
	obstacle_map[ISO_BRICK_WALL_EH].flags |= IS_HORIZONTAL;
	obstacle_map[ISO_BRICK_WALL_EH].transparent = TRANSPARENCY_FOR_WALLS;
	obstacle_map[ISO_BRICK_WALL_EH].result_type_after_smashing_once = ISO_BRICK_WALL_CRACKED_1;
	block_2param(ISO_BRICK_WALL_EV, 0.8, 1.2);
	obstacle_map[ISO_BRICK_WALL_EV].filename = "iso_brick_wall_0016.png";
	obstacle_map[ISO_BRICK_WALL_EV].flags |= IS_VERTICAL;
	obstacle_map[ISO_BRICK_WALL_EV].transparent = TRANSPARENCY_FOR_WALLS;
	obstacle_map[ISO_BRICK_WALL_EV].result_type_after_smashing_once = ISO_BRICK_WALL_CRACKED_2;

	obstacle_map[ISO_BLOOD_1].block_area_type = COLLISION_TYPE_NONE;
	obstacle_map[ISO_BLOOD_1].filename = "iso_blood_0001.png";
	obstacle_map[ISO_BLOOD_1].flags |= NEEDS_PRE_PUT;
	obstacle_map[ISO_BLOOD_2].block_area_type = COLLISION_TYPE_NONE;
	obstacle_map[ISO_BLOOD_2].filename = "iso_blood_0002.png";
	obstacle_map[ISO_BLOOD_2].flags |= NEEDS_PRE_PUT;
	obstacle_map[ISO_BLOOD_3].block_area_type = COLLISION_TYPE_NONE;
	obstacle_map[ISO_BLOOD_3].filename = "iso_blood_0003.png";
	obstacle_map[ISO_BLOOD_3].flags |= NEEDS_PRE_PUT;
	obstacle_map[ISO_BLOOD_4].block_area_type = COLLISION_TYPE_NONE;
	obstacle_map[ISO_BLOOD_4].filename = "iso_blood_0004.png";
	obstacle_map[ISO_BLOOD_4].flags |= NEEDS_PRE_PUT;
	obstacle_map[ISO_BLOOD_5].block_area_type = COLLISION_TYPE_NONE;
	obstacle_map[ISO_BLOOD_5].filename = "iso_blood_0005.png";
	obstacle_map[ISO_BLOOD_5].flags |= NEEDS_PRE_PUT;
	obstacle_map[ISO_BLOOD_6].block_area_type = COLLISION_TYPE_NONE;
	obstacle_map[ISO_BLOOD_6].filename = "iso_blood_0006.png";
	obstacle_map[ISO_BLOOD_6].flags |= NEEDS_PRE_PUT;
	obstacle_map[ISO_BLOOD_7].block_area_type = COLLISION_TYPE_NONE;
	obstacle_map[ISO_BLOOD_7].filename = "iso_blood_0007.png";
	obstacle_map[ISO_BLOOD_7].flags |= NEEDS_PRE_PUT;
	obstacle_map[ISO_BLOOD_8].block_area_type = COLLISION_TYPE_NONE;
	obstacle_map[ISO_BLOOD_8].filename = "iso_blood_0008.png";
	obstacle_map[ISO_BLOOD_8].flags |= NEEDS_PRE_PUT;

	obstacle_map[ISO_OIL_STAINS_1].block_area_type = COLLISION_TYPE_NONE;
	obstacle_map[ISO_OIL_STAINS_1].filename = "iso_oil_stains_0001.png";
	obstacle_map[ISO_OIL_STAINS_1].flags |= NEEDS_PRE_PUT;
	obstacle_map[ISO_OIL_STAINS_2].block_area_type = COLLISION_TYPE_NONE;
	obstacle_map[ISO_OIL_STAINS_2].filename = "iso_oil_stains_0002.png";
	obstacle_map[ISO_OIL_STAINS_2].flags |= NEEDS_PRE_PUT;
	obstacle_map[ISO_OIL_STAINS_3].block_area_type = COLLISION_TYPE_NONE;
	obstacle_map[ISO_OIL_STAINS_3].filename = "iso_oil_stains_0003.png";
	obstacle_map[ISO_OIL_STAINS_3].flags |= NEEDS_PRE_PUT;
	obstacle_map[ISO_OIL_STAINS_4].block_area_type = COLLISION_TYPE_NONE;
	obstacle_map[ISO_OIL_STAINS_4].filename = "iso_oil_stains_0004.png";
	obstacle_map[ISO_OIL_STAINS_4].flags |= NEEDS_PRE_PUT;
	obstacle_map[ISO_OIL_STAINS_5].block_area_type = COLLISION_TYPE_NONE;
	obstacle_map[ISO_OIL_STAINS_5].filename = "iso_oil_stains_0005.png";
	obstacle_map[ISO_OIL_STAINS_5].flags |= NEEDS_PRE_PUT;
	obstacle_map[ISO_OIL_STAINS_6].block_area_type = COLLISION_TYPE_NONE;
	obstacle_map[ISO_OIL_STAINS_6].filename = "iso_oil_stains_0006.png";
	obstacle_map[ISO_OIL_STAINS_6].flags |= NEEDS_PRE_PUT;
	obstacle_map[ISO_OIL_STAINS_7].block_area_type = COLLISION_TYPE_NONE;
	obstacle_map[ISO_OIL_STAINS_7].filename = "iso_oil_stains_0007.png";
	obstacle_map[ISO_OIL_STAINS_7].flags |= NEEDS_PRE_PUT;
	obstacle_map[ISO_OIL_STAINS_8].block_area_type = COLLISION_TYPE_NONE;
	obstacle_map[ISO_OIL_STAINS_8].filename = "iso_oil_stains_0008.png";
	obstacle_map[ISO_OIL_STAINS_8].flags |= NEEDS_PRE_PUT;

	obstacle_map[ISO_EXIT_1].block_area_type = COLLISION_TYPE_NONE;
	obstacle_map[ISO_EXIT_1].filename = "iso_exits_0001.png";
	obstacle_map[ISO_EXIT_1].flags &= ~NEEDS_PRE_PUT;

	obstacle_map[ISO_EXIT_2].block_area_type = COLLISION_TYPE_NONE;
	obstacle_map[ISO_EXIT_2].filename = "iso_exits_0002.png";
	obstacle_map[ISO_EXIT_2].flags &= ~NEEDS_PRE_PUT;

	obstacle_map[ISO_EXIT_3].block_area_type = COLLISION_TYPE_NONE;
	obstacle_map[ISO_EXIT_3].filename = "iso_exits_0003.png";
	obstacle_map[ISO_EXIT_3].flags &= ~NEEDS_PRE_PUT;
	obstacle_map[ISO_EXIT_3].emitted_light_strength = 29;

	obstacle_map[ISO_EXIT_4].block_area_type = COLLISION_TYPE_NONE;
	obstacle_map[ISO_EXIT_4].filename = "iso_exits_0004.png";
	obstacle_map[ISO_EXIT_4].flags &= ~NEEDS_PRE_PUT;
	obstacle_map[ISO_EXIT_4].emitted_light_strength = 29;

	obstacle_map[ISO_EXIT_5].block_area_type = COLLISION_TYPE_NONE;
	obstacle_map[ISO_EXIT_5].filename = "iso_exits_0005.png";
	obstacle_map[ISO_EXIT_5].flags &= ~NEEDS_PRE_PUT;

	obstacle_map[ISO_EXIT_6].block_area_type = COLLISION_TYPE_NONE;
	obstacle_map[ISO_EXIT_6].filename = "iso_exits_0006.png";
	obstacle_map[ISO_EXIT_6].flags &= ~NEEDS_PRE_PUT;
	// This is the wonderful littel exotic plant provided by Basse.
	// It will block the Tux movement but vision should pass through
	// as it's not a particularly high object, so you can see over it.
	//
	block_2param(ISO_ROCKS_N_PLANTS_1, 0.4, 0.4);
	obstacle_map[ISO_ROCKS_N_PLANTS_1].filename = "iso_rocks_n_plants_0000.png";
	obstacle_map[ISO_ROCKS_N_PLANTS_1].flags &= ~BLOCKS_VISION_TOO;
	obstacle_map[ISO_ROCKS_N_PLANTS_1].flags |= GROUND_LEVEL;
	obstacle_map[ISO_ROCKS_N_PLANTS_1].emitted_light_strength = 10;

	block_2param(ISO_ROCKS_N_PLANTS_2, 1.5, 1.5);
	obstacle_map[ISO_ROCKS_N_PLANTS_2].filename = "iso_rocks_n_plants_0001.png";
	obstacle_map[ISO_ROCKS_N_PLANTS_2].flags &= ~BLOCKS_VISION_TOO;
	obstacle_map[ISO_ROCKS_N_PLANTS_2].flags |= GROUND_LEVEL;

	obstacle_map[ISO_ROCKS_N_PLANTS_3].block_area_type = COLLISION_TYPE_NONE;
	obstacle_map[ISO_ROCKS_N_PLANTS_3].filename = "iso_rocks_n_plants_0002.png";
	obstacle_map[ISO_ROCKS_N_PLANTS_3].flags |= NEEDS_PRE_PUT;

	block_2param(ISO_ROCKS_N_PLANTS_4, 1.0, 1.0);
	obstacle_map[ISO_ROCKS_N_PLANTS_4].filename = "iso_rocks_n_plants_0003.png";
	obstacle_map[ISO_ROCKS_N_PLANTS_4].flags &= ~BLOCKS_VISION_TOO;

	//the three rocks above have the same description for the moment
	//inspired by fallout1

	block_2param(ISO_ROCKS_N_PLANTS_5, 1.0, 1.0);
	obstacle_map[ISO_ROCKS_N_PLANTS_5].filename = "iso_rocks_n_plants_0004.png";
	obstacle_map[ISO_ROCKS_N_PLANTS_5].flags &= ~BLOCKS_VISION_TOO;
	obstacle_map[ISO_ROCKS_N_PLANTS_5].flags |= GROUND_LEVEL;
	obstacle_map[ISO_ROCKS_N_PLANTS_5].emitted_light_strength = 7;

	block_2param(ISO_ROCKS_N_PLANTS_6, 1.0, 1.0);
	obstacle_map[ISO_ROCKS_N_PLANTS_6].filename = "iso_rocks_n_plants_0005.png";
	obstacle_map[ISO_ROCKS_N_PLANTS_6].flags &= ~BLOCKS_VISION_TOO;
	obstacle_map[ISO_ROCKS_N_PLANTS_6].flags |= GROUND_LEVEL;
	obstacle_map[ISO_ROCKS_N_PLANTS_6].emitted_light_strength = 9;

	block_2param(ISO_ROCKS_N_PLANTS_7, 0.9, 0.9);
	obstacle_map[ISO_ROCKS_N_PLANTS_7].filename = "iso_rocks_n_plants_0006.png";
	obstacle_map[ISO_ROCKS_N_PLANTS_7].flags &= ~BLOCKS_VISION_TOO;
	obstacle_map[ISO_ROCKS_N_PLANTS_7].flags |= GROUND_LEVEL;
	obstacle_map[ISO_ROCKS_N_PLANTS_7].emitted_light_strength = 8;

	block_2param(ISO_ROCKS_N_PLANTS_8, 0.9, 0.9);
	obstacle_map[ISO_ROCKS_N_PLANTS_8].filename = "iso_rocks_n_plants_0007.png";
	obstacle_map[ISO_ROCKS_N_PLANTS_8].flags &= ~BLOCKS_VISION_TOO;
	obstacle_map[ISO_ROCKS_N_PLANTS_8].flags |= GROUND_LEVEL;
	obstacle_map[ISO_ROCKS_N_PLANTS_8].emitted_light_strength = 11;


	for (i = ISO_ROOM_WALL_V_RED; i <= ISO_ROOM_WALL_H_GREEN; i++) {
		obstacle_map[i].transparent = TRANSPARENCY_FOR_WALLS;
	}

	block_2param(ISO_ROOM_WALL_V_RED, standard_wall_thickness, standard_wall_width);
	obstacle_map[ISO_ROOM_WALL_V_RED].filename = "iso_walls_0016.png";
	obstacle_map[ISO_ROOM_WALL_V_RED].flags |= IS_VERTICAL;
	block_2param(ISO_ROOM_WALL_H_RED, standard_wall_width, standard_wall_thickness);
	obstacle_map[ISO_ROOM_WALL_H_RED].filename = "iso_walls_0017.png";
	obstacle_map[ISO_ROOM_WALL_H_RED].flags |= IS_HORIZONTAL;
	block_2param(ISO_ROOM_WALL_V_GREEN, standard_wall_thickness, standard_wall_width);
	obstacle_map[ISO_ROOM_WALL_V_GREEN].filename = "iso_walls_0018.png";
	obstacle_map[ISO_ROOM_WALL_V_GREEN].flags |= IS_VERTICAL;
	block_2param(ISO_ROOM_WALL_H_GREEN, standard_wall_width, standard_wall_thickness);
	obstacle_map[ISO_ROOM_WALL_H_GREEN].filename = "iso_walls_0019.png";
	obstacle_map[ISO_ROOM_WALL_H_GREEN].flags |= IS_HORIZONTAL;
	// These two are for the big long shop counter.  It has a suitable
	// collision rectangle, but light may pass through, so you can see
	// the person behind the counter
	//
	block_2param(ISO_SHOP_FURNITURE_1, 3.5, 1.5);
	obstacle_map[ISO_SHOP_FURNITURE_1].filename = "iso_shop_furniture_0001.png";
	obstacle_map[ISO_SHOP_FURNITURE_1].flags &= ~BLOCKS_VISION_TOO;
	obstacle_map[ISO_SHOP_FURNITURE_1].flags |= GROUND_LEVEL;
	block_2param(ISO_SHOP_FURNITURE_2, 1.5, 3.5);
	obstacle_map[ISO_SHOP_FURNITURE_2].filename = "iso_shop_furniture_0002.png";
	obstacle_map[ISO_SHOP_FURNITURE_2].flags &= ~BLOCKS_VISION_TOO;
	obstacle_map[ISO_SHOP_FURNITURE_2].flags |= GROUND_LEVEL;

	block_2param(ISO_SHOP_FURNITURE_3, 2.2, 0.6);
	obstacle_map[ISO_SHOP_FURNITURE_3].filename = "iso_shop_furniture_0003.png";
	block_2param(ISO_SHOP_FURNITURE_4, 0.6, 2.2);
	obstacle_map[ISO_SHOP_FURNITURE_4].filename = "iso_shop_furniture_0004.png";

	block_2param(ISO_SHOP_FURNITURE_5, 2.2, 0.6);
	obstacle_map[ISO_SHOP_FURNITURE_5].filename = "iso_shop_furniture_0005.png";
	block_2param(ISO_SHOP_FURNITURE_6, 0.6, 2.2);
	obstacle_map[ISO_SHOP_FURNITURE_6].filename = "iso_shop_furniture_0006.png";

	block_2param(ISO_LIBRARY_FURNITURE_1, 3.5, 1.5);
	obstacle_map[ISO_LIBRARY_FURNITURE_1].filename = "iso_library_furniture_0001.png";
	obstacle_map[ISO_LIBRARY_FURNITURE_1].flags &= ~BLOCKS_VISION_TOO;
	obstacle_map[ISO_LIBRARY_FURNITURE_1].flags |= GROUND_LEVEL;
	block_2param(ISO_LIBRARY_FURNITURE_2, 1.5, 3.5);
	obstacle_map[ISO_LIBRARY_FURNITURE_2].filename = "iso_library_furniture_0002.png";
	obstacle_map[ISO_LIBRARY_FURNITURE_2].flags &= ~BLOCKS_VISION_TOO;
	obstacle_map[ISO_LIBRARY_FURNITURE_2].flags |= GROUND_LEVEL;

	for (i = ISO_OUTER_WALL_N1; i <= ISO_OUTER_DOOR_H_LOCKED; i++) {
		obstacle_map[i].transparent = TRANSPARENCY_FOR_WALLS;
	}

	block_4param(ISO_OUTER_WALL_N1, outer_wall_4_width, outer_wall_4_thickness, outer_wall_4_width, outer_wall_4_backside);
	obstacle_map[ISO_OUTER_WALL_N1].filename = "iso_outer_walls_0002.png";
// HORIZONTAL and VERTICAL, WTF does that mean in our 45degree rotated iso world?
// Instead we should use e.g.
// IS_EAST_TO_WEST
// IS_NORTH_TO_SOUTH
	obstacle_map[ISO_OUTER_WALL_N1].flags |= IS_HORIZONTAL;
	block_4param(ISO_OUTER_WALL_N2, outer_wall_4_width, outer_wall_4_thickness, outer_wall_4_width, outer_wall_4_backside);
	obstacle_map[ISO_OUTER_WALL_N2].filename = "iso_outer_walls_0006.png";
	obstacle_map[ISO_OUTER_WALL_N2].flags |= IS_HORIZONTAL;
	block_4param(ISO_OUTER_WALL_N3, outer_wall_4_width, outer_wall_4_thickness, outer_wall_4_width, outer_wall_4_backside);
	obstacle_map[ISO_OUTER_WALL_N3].filename = "iso_outer_walls_0010.png";
	obstacle_map[ISO_OUTER_WALL_N3].flags |= IS_HORIZONTAL;
	block_4param(ISO_OUTER_WALL_S1, outer_wall_4_width, outer_wall_4_thickness, outer_wall_4_width, outer_wall_4_backside);
	obstacle_map[ISO_OUTER_WALL_S1].filename = "iso_outer_walls_0004.png";
	obstacle_map[ISO_OUTER_WALL_S1].flags |= IS_HORIZONTAL;
	block_4param(ISO_OUTER_WALL_S2, outer_wall_4_width, outer_wall_4_thickness, outer_wall_4_width, outer_wall_4_backside);
	obstacle_map[ISO_OUTER_WALL_S2].filename = "iso_outer_walls_0008.png";
	obstacle_map[ISO_OUTER_WALL_S2].flags |= IS_HORIZONTAL;
	block_4param(ISO_OUTER_WALL_S3, outer_wall_4_width, outer_wall_4_thickness, outer_wall_4_width, outer_wall_4_backside);
	obstacle_map[ISO_OUTER_WALL_S3].filename = "iso_outer_walls_0012.png";
	obstacle_map[ISO_OUTER_WALL_S3].flags |= IS_HORIZONTAL;
	block_4param(ISO_OUTER_WALL_E1, outer_wall_4_thickness, outer_wall_4_width, outer_wall_4_backside, outer_wall_4_width);
	obstacle_map[ISO_OUTER_WALL_E1].filename = "iso_outer_walls_0003.png";
	obstacle_map[ISO_OUTER_WALL_E1].flags |= IS_VERTICAL;
	block_4param(ISO_OUTER_WALL_E2, outer_wall_4_thickness, outer_wall_4_width, outer_wall_4_backside, outer_wall_4_width);
	obstacle_map[ISO_OUTER_WALL_E2].filename = "iso_outer_walls_0007.png";
	obstacle_map[ISO_OUTER_WALL_E2].flags |= IS_VERTICAL;
	block_4param(ISO_OUTER_WALL_E3, outer_wall_4_thickness, outer_wall_4_width, outer_wall_4_backside, outer_wall_4_width);
	obstacle_map[ISO_OUTER_WALL_E3].filename = "iso_outer_walls_0011.png";
	obstacle_map[ISO_OUTER_WALL_E3].flags |= IS_VERTICAL;
	block_4param(ISO_OUTER_WALL_W1, outer_wall_4_thickness, outer_wall_4_width, outer_wall_4_backside, outer_wall_4_width);
	obstacle_map[ISO_OUTER_WALL_W1].filename = "iso_outer_walls_0001.png";
	obstacle_map[ISO_OUTER_WALL_W1].flags |= IS_VERTICAL;
	block_4param(ISO_OUTER_WALL_W2, outer_wall_4_thickness, outer_wall_4_width, outer_wall_4_backside, outer_wall_4_width);
	obstacle_map[ISO_OUTER_WALL_W2].filename = "iso_outer_walls_0005.png";
	obstacle_map[ISO_OUTER_WALL_W2].flags |= IS_VERTICAL;
	block_4param(ISO_OUTER_WALL_W3, outer_wall_4_thickness, outer_wall_4_width, outer_wall_4_backside, outer_wall_4_width);
	obstacle_map[ISO_OUTER_WALL_W3].filename = "iso_outer_walls_0009.png";
	obstacle_map[ISO_OUTER_WALL_W3].flags |= IS_VERTICAL;
	block_2param(ISO_OUTER_WALL_CORNER_NW, 1.1, 1.1);
	obstacle_map[ISO_OUTER_WALL_CORNER_NW].filename = "iso_outer_walls_0013.png";
	block_2param(ISO_OUTER_WALL_CORNER_SW, 1.1, 1.1);
	obstacle_map[ISO_OUTER_WALL_CORNER_SW].filename = "iso_outer_walls_0014.png";
	block_2param(ISO_OUTER_WALL_CORNER_SE, 1.1, 1.1);
	obstacle_map[ISO_OUTER_WALL_CORNER_SE].filename = "iso_outer_walls_0015.png";
	block_2param(ISO_OUTER_WALL_CORNER_NE, 1.1, 1.1);
	obstacle_map[ISO_OUTER_WALL_CORNER_NE].filename = "iso_outer_walls_0016.png";

	block_2param(ISO_OUTER_WALL_SMALL_CORNER_1, 1.0, 1.0);
	obstacle_map[ISO_OUTER_WALL_SMALL_CORNER_1].filename = "iso_outer_walls_0017.png";
	obstacle_map[ISO_OUTER_WALL_SMALL_CORNER_1].transparent = TRANSPARENCY_FOR_WALLS;
	block_2param(ISO_OUTER_WALL_SMALL_CORNER_2, 1.0, 1.0);
	obstacle_map[ISO_OUTER_WALL_SMALL_CORNER_2].filename = "iso_outer_walls_0018.png";
	obstacle_map[ISO_OUTER_WALL_SMALL_CORNER_2].transparent = TRANSPARENCY_FOR_WALLS;
	block_2param(ISO_OUTER_WALL_SMALL_CORNER_3, 1.0, 1.0);
	obstacle_map[ISO_OUTER_WALL_SMALL_CORNER_3].filename = "iso_outer_walls_0019.png";
	obstacle_map[ISO_OUTER_WALL_SMALL_CORNER_3].transparent = TRANSPARENCY_FOR_WALLS;
	block_2param(ISO_OUTER_WALL_SMALL_CORNER_4, 1.0, 1.0);
	obstacle_map[ISO_OUTER_WALL_SMALL_CORNER_4].filename = "iso_outer_walls_0020.png";
	obstacle_map[ISO_OUTER_WALL_SMALL_CORNER_4].transparent = TRANSPARENCY_FOR_WALLS;

	block_4param(ISO_OUTER_DOOR_V_00, outer_wall_4_thickness, outer_door_4_width_short, outer_wall_4_backside, outer_door_4_width_long);
	obstacle_map[ISO_OUTER_DOOR_V_00].flags &= ~BLOCKS_VISION_TOO;
	obstacle_map[ISO_OUTER_DOOR_V_00].flags |= IS_WALKABLE;
	obstacle_map[ISO_OUTER_DOOR_V_00].filename = "iso_doors_0018.png";
	obstacle_map[ISO_OUTER_DOOR_V_00].animate_fn = animate_door;
	obstacle_map[ISO_OUTER_DOOR_V_25].filename = "iso_doors_0019.png";
	obstacle_map[ISO_OUTER_DOOR_V_25].flags &= ~BLOCKS_VISION_TOO;
	obstacle_map[ISO_OUTER_DOOR_V_25].flags |= IS_WALKABLE;
	obstacle_map[ISO_OUTER_DOOR_V_25].animate_fn = animate_door;
	obstacle_map[ISO_OUTER_DOOR_V_50].filename = "iso_doors_0020.png";
	obstacle_map[ISO_OUTER_DOOR_V_50].flags &= ~BLOCKS_VISION_TOO;
	obstacle_map[ISO_OUTER_DOOR_V_50].flags |= IS_WALKABLE;
	obstacle_map[ISO_OUTER_DOOR_V_50].animate_fn = animate_door;
	obstacle_map[ISO_OUTER_DOOR_V_75].filename = "iso_doors_0021.png";
	obstacle_map[ISO_OUTER_DOOR_V_75].flags &= ~BLOCKS_VISION_TOO;
	obstacle_map[ISO_OUTER_DOOR_V_75].flags |= IS_WALKABLE;
	obstacle_map[ISO_OUTER_DOOR_V_75].animate_fn = animate_door;
	obstacle_map[ISO_OUTER_DOOR_V_100].filename = "iso_doors_0022.png";
	obstacle_map[ISO_OUTER_DOOR_V_100].block_area_type = COLLISION_TYPE_NONE;
	obstacle_map[ISO_OUTER_DOOR_V_100].animate_fn = animate_door;

	block_4param(ISO_OUTER_DOOR_H_00, outer_door_4_width_short, outer_wall_4_thickness, outer_door_4_width_long, outer_wall_4_backside);
	obstacle_map[ISO_OUTER_DOOR_H_00].flags &= ~BLOCKS_VISION_TOO;
	obstacle_map[ISO_OUTER_DOOR_H_00].flags |= IS_WALKABLE;
	obstacle_map[ISO_OUTER_DOOR_H_00].filename = "iso_doors_0013.png";
	obstacle_map[ISO_OUTER_DOOR_H_00].animate_fn = animate_door;
	obstacle_map[ISO_OUTER_DOOR_H_25].filename = "iso_doors_0014.png";
	obstacle_map[ISO_OUTER_DOOR_H_25].flags &= ~BLOCKS_VISION_TOO;
	obstacle_map[ISO_OUTER_DOOR_H_25].flags |= IS_WALKABLE;
	obstacle_map[ISO_OUTER_DOOR_H_25].animate_fn = animate_door;
	obstacle_map[ISO_OUTER_DOOR_H_50].filename = "iso_doors_0015.png";
	obstacle_map[ISO_OUTER_DOOR_H_50].flags &= ~BLOCKS_VISION_TOO;
	obstacle_map[ISO_OUTER_DOOR_H_50].flags |= IS_WALKABLE;
	obstacle_map[ISO_OUTER_DOOR_H_50].animate_fn = animate_door;
	obstacle_map[ISO_OUTER_DOOR_H_75].filename = "iso_doors_0016.png";
	obstacle_map[ISO_OUTER_DOOR_H_75].flags &= ~BLOCKS_VISION_TOO;
	obstacle_map[ISO_OUTER_DOOR_H_75].flags |= IS_WALKABLE;
	obstacle_map[ISO_OUTER_DOOR_H_75].animate_fn = animate_door;
	obstacle_map[ISO_OUTER_DOOR_H_100].filename = "iso_doors_0017.png";
	obstacle_map[ISO_OUTER_DOOR_H_100].block_area_type = COLLISION_TYPE_NONE;
	obstacle_map[ISO_OUTER_DOOR_H_100].animate_fn = animate_door;

	block_4param(ISO_OUTER_DOOR_V_LOCKED, outer_wall_4_thickness, outer_door_4_width_short, outer_wall_4_backside, outer_door_4_width_long);
	obstacle_map[ISO_OUTER_DOOR_V_LOCKED].filename = "iso_doors_0024.png";
	obstacle_map[ISO_OUTER_DOOR_V_LOCKED].flags &= ~BLOCKS_VISION_TOO;

	block_4param(ISO_OUTER_DOOR_H_LOCKED, outer_door_4_width_short, outer_wall_4_thickness, outer_door_4_width_long, outer_wall_4_backside);
	obstacle_map[ISO_OUTER_DOOR_H_LOCKED].filename = "iso_doors_0023.png";
	obstacle_map[ISO_OUTER_DOOR_H_LOCKED].flags &= ~BLOCKS_VISION_TOO;

	block_4param(ISO_OUTER_DOOR_V_OFFLINE, outer_wall_4_thickness, outer_door_4_width_short, outer_wall_4_backside, outer_door_4_width_long);
	obstacle_map[ISO_OUTER_DOOR_V_OFFLINE].filename = "iso_doors_0022.png";
	obstacle_map[ISO_OUTER_DOOR_V_OFFLINE].block_area_type = COLLISION_TYPE_NONE;

	block_4param(ISO_OUTER_DOOR_H_OFFLINE, outer_door_4_width_short, outer_wall_4_thickness, outer_door_4_width_long, outer_wall_4_backside);
	obstacle_map[ISO_OUTER_DOOR_H_OFFLINE].filename = "iso_doors_0017.png";
	obstacle_map[ISO_OUTER_DOOR_H_OFFLINE].block_area_type = COLLISION_TYPE_NONE;

	block_2param(ISO_YELLOW_CHAIR_N, 0.8, 0.8);
	obstacle_map[ISO_YELLOW_CHAIR_N].flags &= ~BLOCKS_VISION_TOO;
	obstacle_map[ISO_YELLOW_CHAIR_N].filename = "iso_chairs_0004.png";
	block_2param(ISO_YELLOW_CHAIR_E, 0.8, 0.8);
	obstacle_map[ISO_YELLOW_CHAIR_E].flags &= ~BLOCKS_VISION_TOO;
	obstacle_map[ISO_YELLOW_CHAIR_E].filename = "iso_chairs_0001.png";
	block_2param(ISO_YELLOW_CHAIR_S, 0.8, 0.8);
	obstacle_map[ISO_YELLOW_CHAIR_S].flags &= ~BLOCKS_VISION_TOO;
	obstacle_map[ISO_YELLOW_CHAIR_S].filename = "iso_chairs_0002.png";
	block_2param(ISO_YELLOW_CHAIR_W, 0.8, 0.8);
	obstacle_map[ISO_YELLOW_CHAIR_W].flags &= ~BLOCKS_VISION_TOO;
	obstacle_map[ISO_YELLOW_CHAIR_W].filename = "iso_chairs_0003.png";

	block_2param(ISO_RED_CHAIR_N, 1.6, 0.8);
	obstacle_map[ISO_RED_CHAIR_N].flags &= ~BLOCKS_VISION_TOO;
	obstacle_map[ISO_RED_CHAIR_N].flags |= GROUND_LEVEL;
	obstacle_map[ISO_RED_CHAIR_N].filename = "iso_chairs_0008.png";
	block_2param(ISO_RED_CHAIR_E, 0.8, 1.6);
	obstacle_map[ISO_RED_CHAIR_E].flags &= ~BLOCKS_VISION_TOO;
	obstacle_map[ISO_RED_CHAIR_E].flags |= GROUND_LEVEL;
	obstacle_map[ISO_RED_CHAIR_E].filename = "iso_chairs_0005.png";
	block_2param(ISO_RED_CHAIR_S, 1.6, 0.8);
	obstacle_map[ISO_RED_CHAIR_S].flags &= ~BLOCKS_VISION_TOO;
	obstacle_map[ISO_RED_CHAIR_S].flags |= GROUND_LEVEL;
	obstacle_map[ISO_RED_CHAIR_S].filename = "iso_chairs_0006.png";
	block_2param(ISO_RED_CHAIR_W, 0.8, 1.6);
	obstacle_map[ISO_RED_CHAIR_W].flags &= ~BLOCKS_VISION_TOO;
	obstacle_map[ISO_RED_CHAIR_W].flags |= GROUND_LEVEL;
	obstacle_map[ISO_RED_CHAIR_W].filename = "iso_chairs_0007.png";

	// bodies
	obstacle_map[ISO_BODY_RED_GUARD_N].block_area_type = COLLISION_TYPE_NONE;
	obstacle_map[ISO_BODY_RED_GUARD_N].filename = "iso_body_0001.png";
	obstacle_map[ISO_BODY_RED_GUARD_N].flags |= NEEDS_PRE_PUT;
	obstacle_map[ISO_BODY_RED_GUARD_E].block_area_type = COLLISION_TYPE_NONE;
	obstacle_map[ISO_BODY_RED_GUARD_E].filename = "iso_body_0002.png";
	obstacle_map[ISO_BODY_RED_GUARD_E].flags |= NEEDS_PRE_PUT;
	obstacle_map[ISO_BODY_RED_GUARD_S].block_area_type = COLLISION_TYPE_NONE;
	obstacle_map[ISO_BODY_RED_GUARD_S].filename = "iso_body_0003.png";
	obstacle_map[ISO_BODY_RED_GUARD_S].flags |= NEEDS_PRE_PUT;
	obstacle_map[ISO_BODY_RED_GUARD_W].block_area_type = COLLISION_TYPE_NONE;
	obstacle_map[ISO_BODY_RED_GUARD_W].filename = "iso_body_0004.png";
	obstacle_map[ISO_BODY_RED_GUARD_W].flags |= NEEDS_PRE_PUT;

	block_2param(ISO_CONFERENCE_TABLE_N, 2.0, 2.0);
	obstacle_map[ISO_CONFERENCE_TABLE_N].flags &= ~BLOCKS_VISION_TOO;
	obstacle_map[ISO_CONFERENCE_TABLE_N].flags |= GROUND_LEVEL;
	obstacle_map[ISO_CONFERENCE_TABLE_N].filename = "iso_conference_furniture_0001.png";
	block_2param(ISO_CONFERENCE_TABLE_E, 2.0, 2.0);
	obstacle_map[ISO_CONFERENCE_TABLE_E].flags &= ~BLOCKS_VISION_TOO;
	obstacle_map[ISO_CONFERENCE_TABLE_E].flags |= GROUND_LEVEL;
	obstacle_map[ISO_CONFERENCE_TABLE_E].filename = "iso_conference_furniture_0000.png";
	block_2param(ISO_CONFERENCE_TABLE_S, 2.0, 2.0);
	obstacle_map[ISO_CONFERENCE_TABLE_S].flags &= ~BLOCKS_VISION_TOO;
	obstacle_map[ISO_CONFERENCE_TABLE_S].flags |= GROUND_LEVEL;
	obstacle_map[ISO_CONFERENCE_TABLE_S].filename = "iso_conference_furniture_0003.png";
	block_2param(ISO_CONFERENCE_TABLE_W, 2.0, 2.0);
	obstacle_map[ISO_CONFERENCE_TABLE_W].flags &= ~BLOCKS_VISION_TOO;
	obstacle_map[ISO_CONFERENCE_TABLE_W].flags |= GROUND_LEVEL;
	obstacle_map[ISO_CONFERENCE_TABLE_W].filename = "iso_conference_furniture_0002.png";

	block_2param(ISO_RED_FENCE_H, 2.3, 0.80);
	obstacle_map[ISO_RED_FENCE_H].flags &= ~BLOCKS_VISION_TOO;
	obstacle_map[ISO_RED_FENCE_H].filename = "iso_fence_0002.png";
	obstacle_map[ISO_RED_FENCE_H].transparent = TRANSPARENCY_FOR_WALLS;

	block_2param(ISO_RED_FENCE_V, 0.80, 2.3);
	obstacle_map[ISO_RED_FENCE_V].flags &= ~BLOCKS_VISION_TOO;
	obstacle_map[ISO_RED_FENCE_V].filename = "iso_fence_0001.png";
	obstacle_map[ISO_RED_FENCE_V].transparent = TRANSPARENCY_FOR_WALLS;

	block_2param(ISO_BED_1, 1.2, 2.0);
	obstacle_map[ISO_BED_1].flags &= ~BLOCKS_VISION_TOO;
	obstacle_map[ISO_BED_1].flags |= GROUND_LEVEL;
	obstacle_map[ISO_BED_1].filename = "iso_beds_0000.png";
	block_2param(ISO_BED_2, 2.0, 1.2);
	obstacle_map[ISO_BED_2].flags &= ~BLOCKS_VISION_TOO;
	obstacle_map[ISO_BED_2].flags |= GROUND_LEVEL;
	obstacle_map[ISO_BED_2].filename = "iso_beds_0001.png";
	block_2param(ISO_BED_3, 1.2, 2.0);
	obstacle_map[ISO_BED_3].flags &= ~BLOCKS_VISION_TOO;
	obstacle_map[ISO_BED_3].flags |= GROUND_LEVEL;
	obstacle_map[ISO_BED_3].filename = "iso_beds_0002.png";
	block_2param(ISO_BED_4, 2.0, 1.2);
	obstacle_map[ISO_BED_4].flags &= ~BLOCKS_VISION_TOO;
	obstacle_map[ISO_BED_4].flags |= GROUND_LEVEL;
	obstacle_map[ISO_BED_4].filename = "iso_beds_0003.png";
	block_2param(ISO_BED_5, 1.2, 2.0);
	obstacle_map[ISO_BED_5].flags &= ~BLOCKS_VISION_TOO;
	obstacle_map[ISO_BED_5].filename = "iso_beds_0004.png";
	block_2param(ISO_BED_6, 2.0, 1.2);
	obstacle_map[ISO_BED_6].flags &= ~BLOCKS_VISION_TOO;
	obstacle_map[ISO_BED_6].filename = "iso_beds_0005.png";
	block_2param(ISO_BED_7, 1.2, 2.0);
	obstacle_map[ISO_BED_7].flags &= ~BLOCKS_VISION_TOO;
	obstacle_map[ISO_BED_7].filename = "iso_beds_0006.png";
	block_2param(ISO_BED_8, 2.0, 1.2);
	obstacle_map[ISO_BED_8].flags &= ~BLOCKS_VISION_TOO;
	obstacle_map[ISO_BED_8].filename = "iso_beds_0007.png";

	block_2param(ISO_PROJECTOR_E, 0.50, 0.5);
	obstacle_map[ISO_PROJECTOR_E].flags &= ~BLOCKS_VISION_TOO;
	obstacle_map[ISO_PROJECTOR_E].filename = "iso_conference_furniture_0004.png";
	block_2param(ISO_PROJECTOR_W, 0.5, 0.5);
	obstacle_map[ISO_PROJECTOR_W].flags &= ~BLOCKS_VISION_TOO;
	obstacle_map[ISO_PROJECTOR_W].filename = "iso_conference_furniture_0006.png";
	block_2param(ISO_PROJECTOR_N, 0.50, 0.5);
	obstacle_map[ISO_PROJECTOR_N].flags &= ~BLOCKS_VISION_TOO;
	obstacle_map[ISO_PROJECTOR_N].filename = "iso_conference_furniture_0007.png";
	block_2param(ISO_PROJECTOR_S, 0.5, 0.5);
	obstacle_map[ISO_PROJECTOR_S].flags &= ~BLOCKS_VISION_TOO;
	obstacle_map[ISO_PROJECTOR_S].filename = "iso_conference_furniture_0005.png";

	block_2param(ISO_PROJECTOR_SCREEN_N, 2.2, 1.0);
	obstacle_map[ISO_PROJECTOR_SCREEN_N].filename = "iso_conference_furniture_0011.png";
	block_2param(ISO_PROJECTOR_SCREEN_E, 1.0, 2.2);
	obstacle_map[ISO_PROJECTOR_SCREEN_E].filename = "iso_conference_furniture_0008.png";
	block_2param(ISO_PROJECTOR_SCREEN_S, 2.0, 1.0);
	obstacle_map[ISO_PROJECTOR_SCREEN_S].filename = "iso_conference_furniture_0009.png";
	block_2param(ISO_PROJECTOR_SCREEN_W, 1.0, 2.2);
	obstacle_map[ISO_PROJECTOR_SCREEN_W].filename = "iso_conference_furniture_0010.png";

	block_2param(ISO_SHELF_FULL_V, 0.6, 2.2);
	obstacle_map[ISO_SHELF_FULL_V].filename = "iso_obstacle_0091.png";
	block_2param(ISO_SHELF_FULL_H, 2.2, 0.6);
	obstacle_map[ISO_SHELF_FULL_H].filename = "iso_obstacle_0092.png";

	block_2param(ISO_SHELF_EMPTY_V, 0.6, 2.2);
	obstacle_map[ISO_SHELF_EMPTY_V].filename = "iso_obstacle_0093.png";
	block_2param(ISO_SHELF_EMPTY_H, 2.2, 0.6);
	obstacle_map[ISO_SHELF_EMPTY_H].filename = "iso_obstacle_0094.png";

	block_2param(ISO_SHELF_SMALL_FULL_V, 1.1, 0.6);
	obstacle_map[ISO_SHELF_SMALL_FULL_V].filename = "iso_obstacle_0095.png";
	block_2param(ISO_SHELF_SMALL_FULL_H, 0.6, 1.1);
	obstacle_map[ISO_SHELF_SMALL_FULL_H].filename = "iso_obstacle_0096.png";

	block_2param(ISO_SHELF_SMALL_EMPTY_V, 0.6, 1.1);
	obstacle_map[ISO_SHELF_SMALL_EMPTY_V].filename = "iso_obstacle_0097.png";
	block_2param(ISO_SHELF_SMALL_EMPTY_H, 1.1, 0.6);
	obstacle_map[ISO_SHELF_SMALL_EMPTY_H].filename = "iso_obstacle_0098.png";

	block_2param(ISO_SIGN_1, 0.5, 0.6);
	obstacle_map[ISO_SIGN_1].filename = "iso_signs_0000.png";
	obstacle_map[ISO_SIGN_1].flags &= ~BLOCKS_VISION_TOO;
	obstacle_map[ISO_SIGN_1].flags |= GROUND_LEVEL | IS_CLICKABLE;
	obstacle_map[ISO_SIGN_1].label = _("Sign");
	obstacle_map[ISO_SIGN_1].action = &sign_read_action;
	block_2param(ISO_SIGN_2, 0.6, 0.5);
	obstacle_map[ISO_SIGN_2].filename = "iso_signs_0001.png";
	obstacle_map[ISO_SIGN_2].flags &= ~BLOCKS_VISION_TOO;
	obstacle_map[ISO_SIGN_2].flags |= GROUND_LEVEL | IS_CLICKABLE;
	obstacle_map[ISO_SIGN_2].label = _("Sign");
	obstacle_map[ISO_SIGN_2].action = &sign_read_action;
	block_2param(ISO_SIGN_3, 0.5, 0.6);
	obstacle_map[ISO_SIGN_3].filename = "iso_signs_0002.png";
	obstacle_map[ISO_SIGN_3].flags &= ~BLOCKS_VISION_TOO;
	obstacle_map[ISO_SIGN_3].flags |= GROUND_LEVEL | IS_CLICKABLE;
	obstacle_map[ISO_SIGN_3].label = _("Sign");
	obstacle_map[ISO_SIGN_3].action = &sign_read_action;

	block_2param(ISO_COUNTER_MIDDLE_1, 0.8, 1.05);
	obstacle_map[ISO_COUNTER_MIDDLE_1].filename = "iso_counter_0001.png";
	obstacle_map[ISO_COUNTER_MIDDLE_1].flags &= ~BLOCKS_VISION_TOO;
	obstacle_map[ISO_COUNTER_MIDDLE_1].flags |= GROUND_LEVEL;
	block_2param(ISO_COUNTER_MIDDLE_2, 1.05, 0.8);
	obstacle_map[ISO_COUNTER_MIDDLE_2].filename = "iso_counter_0002.png";
	obstacle_map[ISO_COUNTER_MIDDLE_2].flags &= ~BLOCKS_VISION_TOO;
	obstacle_map[ISO_COUNTER_MIDDLE_2].flags |= GROUND_LEVEL;
	block_2param(ISO_COUNTER_MIDDLE_3, 0.8, 1.05);
	obstacle_map[ISO_COUNTER_MIDDLE_3].filename = "iso_counter_0003.png";
	obstacle_map[ISO_COUNTER_MIDDLE_3].flags &= ~BLOCKS_VISION_TOO;
	obstacle_map[ISO_COUNTER_MIDDLE_3].flags |= GROUND_LEVEL;
	block_2param(ISO_COUNTER_MIDDLE_4, 1.05, 0.8);
	obstacle_map[ISO_COUNTER_MIDDLE_4].filename = "iso_counter_0004.png";
	obstacle_map[ISO_COUNTER_MIDDLE_4].flags &= ~BLOCKS_VISION_TOO;
	obstacle_map[ISO_COUNTER_MIDDLE_4].flags |= GROUND_LEVEL;

	block_2param(ISO_COUNTER_CORNER_ROUND_1, 1.1, 1.1);
	obstacle_map[ISO_COUNTER_CORNER_ROUND_1].filename = "iso_counter_0005.png";
	obstacle_map[ISO_COUNTER_CORNER_ROUND_1].flags &= ~BLOCKS_VISION_TOO;
	obstacle_map[ISO_COUNTER_CORNER_ROUND_1].flags |= GROUND_LEVEL;
	block_2param(ISO_COUNTER_CORNER_ROUND_2, 1.1, 1.1);
	obstacle_map[ISO_COUNTER_CORNER_ROUND_2].filename = "iso_counter_0006.png";
	obstacle_map[ISO_COUNTER_CORNER_ROUND_2].flags &= ~BLOCKS_VISION_TOO;
	obstacle_map[ISO_COUNTER_CORNER_ROUND_2].flags |= GROUND_LEVEL;
	block_2param(ISO_COUNTER_CORNER_ROUND_3, 1.1, 1.1);
	obstacle_map[ISO_COUNTER_CORNER_ROUND_3].filename = "iso_counter_0007.png";
	obstacle_map[ISO_COUNTER_CORNER_ROUND_3].flags &= ~BLOCKS_VISION_TOO;
	obstacle_map[ISO_COUNTER_CORNER_ROUND_3].flags |= GROUND_LEVEL;
	block_2param(ISO_COUNTER_CORNER_ROUND_4, 1.1, 1.1);
	obstacle_map[ISO_COUNTER_CORNER_ROUND_4].filename = "iso_counter_0008.png";
	obstacle_map[ISO_COUNTER_CORNER_ROUND_4].flags &= ~BLOCKS_VISION_TOO;
	obstacle_map[ISO_COUNTER_CORNER_ROUND_4].flags |= GROUND_LEVEL;

	block_2param(ISO_COUNTER_CORNER_SHARP_1, 1.1, 1.1);
	obstacle_map[ISO_COUNTER_CORNER_SHARP_1].filename = "iso_counter_0009.png";
	obstacle_map[ISO_COUNTER_CORNER_SHARP_1].flags &= ~BLOCKS_VISION_TOO;
	obstacle_map[ISO_COUNTER_CORNER_SHARP_1].flags |= GROUND_LEVEL;
	block_2param(ISO_COUNTER_CORNER_SHARP_2, 1.1, 1.1);
	obstacle_map[ISO_COUNTER_CORNER_SHARP_2].filename = "iso_counter_0010.png";
	obstacle_map[ISO_COUNTER_CORNER_SHARP_2].flags &= ~BLOCKS_VISION_TOO;
	obstacle_map[ISO_COUNTER_CORNER_SHARP_2].flags |= GROUND_LEVEL;
	block_2param(ISO_COUNTER_CORNER_SHARP_3, 1.1, 1.1);
	obstacle_map[ISO_COUNTER_CORNER_SHARP_3].filename = "iso_counter_0011.png";
	obstacle_map[ISO_COUNTER_CORNER_SHARP_3].flags &= ~BLOCKS_VISION_TOO;
	obstacle_map[ISO_COUNTER_CORNER_SHARP_3].flags |= GROUND_LEVEL;
	block_2param(ISO_COUNTER_CORNER_SHARP_4, 1.1, 1.1);
	obstacle_map[ISO_COUNTER_CORNER_SHARP_4].filename = "iso_counter_0012.png";
	obstacle_map[ISO_COUNTER_CORNER_SHARP_4].flags &= ~BLOCKS_VISION_TOO;
	obstacle_map[ISO_COUNTER_CORNER_SHARP_4].flags |= GROUND_LEVEL;

	block_2param(ISO_BAR_TABLE, 0.8, 0.8);
	obstacle_map[ISO_BAR_TABLE].filename = "iso_tables_0005.png";
	obstacle_map[ISO_BAR_TABLE].flags &= ~BLOCKS_VISION_TOO;
	obstacle_map[ISO_BAR_TABLE].flags |= GROUND_LEVEL;

	block_2param(ISO_TABLE_OVAL_1, 1.1, 1.3);
	obstacle_map[ISO_TABLE_OVAL_1].filename = "iso_tables_0006.png";
	obstacle_map[ISO_TABLE_OVAL_1].flags &= ~BLOCKS_VISION_TOO;
	obstacle_map[ISO_TABLE_OVAL_1].flags |= GROUND_LEVEL;

	block_2param(ISO_TABLE_OVAL_2, 1.1, 1.3);
	obstacle_map[ISO_TABLE_OVAL_2].filename = "iso_tables_0007.png";
	obstacle_map[ISO_TABLE_OVAL_2].flags &= ~BLOCKS_VISION_TOO;
	obstacle_map[ISO_TABLE_OVAL_2].flags |= GROUND_LEVEL;

	block_2param(ISO_TABLE_GLASS_1, 1.1, 1.3);
	obstacle_map[ISO_TABLE_GLASS_1].filename = "iso_tables_0008.png";
	obstacle_map[ISO_TABLE_GLASS_1].flags &= ~BLOCKS_VISION_TOO;
	obstacle_map[ISO_TABLE_GLASS_1].flags |= GROUND_LEVEL;
	block_2param(ISO_TABLE_GLASS_2, 1.1, 1.3);
	obstacle_map[ISO_TABLE_GLASS_2].filename = "iso_tables_0009.png";
	obstacle_map[ISO_TABLE_GLASS_2].flags &= ~BLOCKS_VISION_TOO;
	obstacle_map[ISO_TABLE_GLASS_2].flags |= GROUND_LEVEL;

	block_2param(ISO_TRANSP_FOR_WATER, 1, 1);
	obstacle_map[ISO_TRANSP_FOR_WATER].filename = "iso_transp_for_water.png";
	obstacle_map[ISO_TRANSP_FOR_WATER].flags &= ~BLOCKS_VISION_TOO;
	obstacle_map[ISO_TRANSP_FOR_WATER].flags |= GROUND_LEVEL;

	block_2param(ISO_RESTAURANT_DESK_1, 1.5, 5);
	obstacle_map[ISO_RESTAURANT_DESK_1].flags &= ~BLOCKS_VISION_TOO;
	obstacle_map[ISO_RESTAURANT_DESK_1].flags |= GROUND_LEVEL;
	obstacle_map[ISO_RESTAURANT_DESK_1].filename = "iso_restaurant_desk_0001.png";

	block_2param(ISO_RESTAURANT_DESK_2, 5, 1.5);
	obstacle_map[ISO_RESTAURANT_DESK_2].flags &= ~BLOCKS_VISION_TOO;
	obstacle_map[ISO_RESTAURANT_DESK_2].flags |= GROUND_LEVEL;
	obstacle_map[ISO_RESTAURANT_DESK_2].filename = "iso_restaurant_desk_0002.png";

	block_2param(ISO_RESTAURANT_BIGSHELF_1, 0.65, 5.5);
	obstacle_map[ISO_RESTAURANT_BIGSHELF_1].filename = "iso_restaurant_desk_0003.png";

	block_2param(ISO_RESTAURANT_BIGSHELF_2, 5.5, 0.65);
	obstacle_map[ISO_RESTAURANT_BIGSHELF_2].filename = "iso_restaurant_desk_0004.png";

	// crystals
	block_2param(ISO_CRYSTALS_1, 0.5, 0.5);
	obstacle_map[ISO_CRYSTALS_1].flags &= ~BLOCKS_VISION_TOO;
	obstacle_map[ISO_CRYSTALS_1].filename = "iso_crystal_fields_0001.png";

	block_2param(ISO_CRYSTALS_2, 1.15, 1.15);
	obstacle_map[ISO_CRYSTALS_2].flags &= ~BLOCKS_VISION_TOO;
	obstacle_map[ISO_CRYSTALS_2].filename = "iso_crystal_fields_0002.png";

	block_2param(ISO_CRYSTALS_3, 0.95, 0.95);
	obstacle_map[ISO_CRYSTALS_3].flags &= ~BLOCKS_VISION_TOO;
	obstacle_map[ISO_CRYSTALS_3].filename = "iso_crystal_fields_0003.png";

	block_2param(ISO_CRYSTALS_4, 1.25, 1.05);
	obstacle_map[ISO_CRYSTALS_4].flags &= ~BLOCKS_VISION_TOO;
	obstacle_map[ISO_CRYSTALS_4].filename = "iso_crystal_fields_0004.png";

	block_2param(ISO_CRYSTALS_5, 1.20, 1.05);
	obstacle_map[ISO_CRYSTALS_5].flags &= ~BLOCKS_VISION_TOO;
	obstacle_map[ISO_CRYSTALS_5].filename = "iso_crystal_fields_0005.png";

	block_2param(ISO_CRYSTALS_6, 1.1, 1.1);
	obstacle_map[ISO_CRYSTALS_6].flags &= ~BLOCKS_VISION_TOO;
	obstacle_map[ISO_CRYSTALS_6].filename = "iso_crystal_fields_0006.png";

	block_2param(ISO_BASIN_1, 1.05, 0.95);
	obstacle_map[ISO_BASIN_1].flags &= ~BLOCKS_VISION_TOO;
	obstacle_map[ISO_BASIN_1].filename = "iso_basin_0001.png";

	block_2param(ISO_BASIN_2, 0.95, 1.05);
	obstacle_map[ISO_BASIN_2].flags &= ~BLOCKS_VISION_TOO;
	obstacle_map[ISO_BASIN_2].filename = "iso_basin_0002.png";

	block_2param(ISO_BASIN_3, 1.05, 0.95);
	obstacle_map[ISO_BASIN_3].flags &= ~BLOCKS_VISION_TOO;
	obstacle_map[ISO_BASIN_3].filename = "iso_basin_0003.png";

	block_2param(ISO_BASIN_4, 0.95, 1.05);
	obstacle_map[ISO_BASIN_4].flags &= ~BLOCKS_VISION_TOO;
	obstacle_map[ISO_BASIN_4].filename = "iso_basin_0004.png";

	block_2param(ISO_DESKCHAIR_1, 0.90, 0.90);
	obstacle_map[ISO_DESKCHAIR_1].flags &= ~BLOCKS_VISION_TOO;
	obstacle_map[ISO_DESKCHAIR_1].filename = "iso_deskchair_0001.png";

	block_2param(ISO_DESKCHAIR_2, 0.90, 0.90);
	obstacle_map[ISO_DESKCHAIR_2].flags &= ~BLOCKS_VISION_TOO;
	obstacle_map[ISO_DESKCHAIR_2].filename = "iso_deskchair_0002.png";

	block_2param(ISO_DESKCHAIR_3, 0.90, 0.90);
	obstacle_map[ISO_DESKCHAIR_3].flags &= ~BLOCKS_VISION_TOO;
	obstacle_map[ISO_DESKCHAIR_3].filename = "iso_deskchair_0003.png";

	block_2param(ISO_SECURITY_GATE_GREEN_E, 0.95, 1.05);
	obstacle_map[ISO_SECURITY_GATE_GREEN_E].flags &= ~BLOCKS_VISION_TOO;
	obstacle_map[ISO_SECURITY_GATE_GREEN_E].filename = "iso_security_gate_0001.png";

	block_2param(ISO_SECURITY_GATE_GREEN_S, 0.95, 1.05);
	obstacle_map[ISO_SECURITY_GATE_GREEN_S].flags &= ~BLOCKS_VISION_TOO;
	obstacle_map[ISO_SECURITY_GATE_GREEN_S].filename = "iso_security_gate_0002.png";

	block_2param(ISO_SECURITY_GATE_RED_E, 0.95, 1.05);
	obstacle_map[ISO_SECURITY_GATE_RED_E].flags &= ~BLOCKS_VISION_TOO;
	obstacle_map[ISO_SECURITY_GATE_RED_E].filename = "iso_security_gate_0003.png";

	block_2param(ISO_SECURITY_GATE_RED_S, 0.95, 1.05);
	obstacle_map[ISO_SECURITY_GATE_RED_S].flags &= ~BLOCKS_VISION_TOO;
	obstacle_map[ISO_SECURITY_GATE_RED_S].filename = "iso_security_gate_0004.png";
	
	block_2param(ISO_SECURITY_GATE_OPEN_E, 0.95, 1.05);
	obstacle_map[ISO_SECURITY_GATE_OPEN_E].flags &= ~BLOCKS_VISION_TOO;
	obstacle_map[ISO_SECURITY_GATE_OPEN_E].filename = "iso_security_gate_0005.png";
	
	block_2param(ISO_SECURITY_GATE_OPEN_S, 0.95, 1.05);
	obstacle_map[ISO_SECURITY_GATE_OPEN_S].flags &= ~BLOCKS_VISION_TOO;
	obstacle_map[ISO_SECURITY_GATE_OPEN_S].filename = "iso_security_gate_0006.png";
	
	block_2param(ISO_SECURITY_GATE_CLOSED_E, 0.95, 1.05);
	obstacle_map[ISO_SECURITY_GATE_CLOSED_E].flags &= ~BLOCKS_VISION_TOO;
	obstacle_map[ISO_SECURITY_GATE_CLOSED_E].filename = "iso_security_gate_0007.png";
	
	block_2param(ISO_SECURITY_GATE_CLOSED_S, 0.95, 1.05);
	obstacle_map[ISO_SECURITY_GATE_CLOSED_S].flags &= ~BLOCKS_VISION_TOO;
	obstacle_map[ISO_SECURITY_GATE_CLOSED_S].filename = "iso_security_gate_0008.png";

	block_2param(ISO_SOLAR_PANEL_BROKEN, 0.95, 1.05);
	obstacle_map[ISO_SOLAR_PANEL_BROKEN].flags &= ~BLOCKS_VISION_TOO;
	obstacle_map[ISO_SOLAR_PANEL_BROKEN].filename = "iso_solar_panel_0000.png";

	block_2param(ISO_SOLAR_PANEL_E, 0.95, 1.05);
	obstacle_map[ISO_SOLAR_PANEL_E].flags &= ~BLOCKS_VISION_TOO;
	obstacle_map[ISO_SOLAR_PANEL_E].flags |= IS_SMASHABLE | IS_CLICKABLE;
	obstacle_map[ISO_SOLAR_PANEL_E].label = _("Solar Panel");
	obstacle_map[ISO_SOLAR_PANEL_E].action = &barrel_action;
	obstacle_map[ISO_SOLAR_PANEL_E].filename = "iso_solar_panel_0001.png";
	obstacle_map[ISO_SOLAR_PANEL_E].result_type_after_smashing_once = ISO_SOLAR_PANEL_BROKEN;

	block_2param(ISO_BOTLINE_01_N, 3, 2);
	obstacle_map[ISO_BOTLINE_01_N].flags &= ~BLOCKS_VISION_TOO;
	obstacle_map[ISO_BOTLINE_01_N].filename = "iso_botline_0000.png";
	
	block_2param(ISO_BOTLINE_01_E, 2, 3);
	obstacle_map[ISO_BOTLINE_01_E].flags &= ~BLOCKS_VISION_TOO;
	obstacle_map[ISO_BOTLINE_01_E].filename = "iso_botline_0001.png";
	
	block_2param(ISO_BOTLINE_02_E, 2.46, 1.94);
	obstacle_map[ISO_BOTLINE_02_E].flags &= ~BLOCKS_VISION_TOO;
	obstacle_map[ISO_BOTLINE_02_E].filename = "iso_botline_0002.png";
	
	block_2param(ISO_BOTLINE_02_N, 1.94, 2.46);
	obstacle_map[ISO_BOTLINE_02_N].flags &= ~BLOCKS_VISION_TOO;
	obstacle_map[ISO_BOTLINE_02_N].filename = "iso_botline_0003.png";
	
	block_2param(ISO_BOTLINE_02_W, 2.46, 1.94);
	obstacle_map[ISO_BOTLINE_02_W].flags &= ~BLOCKS_VISION_TOO;
	obstacle_map[ISO_BOTLINE_02_W].filename = "iso_botline_0004.png";
	
	block_2param(ISO_BOTLINE_02_S, 1.94, 2.46);
	obstacle_map[ISO_BOTLINE_02_S].flags &= ~BLOCKS_VISION_TOO;
	obstacle_map[ISO_BOTLINE_02_S].filename = "iso_botline_0005.png";
	
	block_2param(ISO_BOTLINE_03_N, 4.2, 1.70);
	obstacle_map[ISO_BOTLINE_03_N].flags &= ~BLOCKS_VISION_TOO;
	obstacle_map[ISO_BOTLINE_03_N].filename = "iso_botline_0006.png";
	
	block_2param(ISO_BOTLINE_03_E, 1.70, 4.2);
	obstacle_map[ISO_BOTLINE_03_E].flags &= ~BLOCKS_VISION_TOO;
	obstacle_map[ISO_BOTLINE_03_E].filename = "iso_botline_0007.png";
	
	block_2param(ISO_BOTLINE_04_N, 4.5, 1.70);
	obstacle_map[ISO_BOTLINE_04_N].flags &= ~BLOCKS_VISION_TOO;
	obstacle_map[ISO_BOTLINE_04_N].filename = "iso_botline_0008.png";
	
	block_2param(ISO_BOTLINE_04_E, 1.70, 4.5);
	obstacle_map[ISO_BOTLINE_04_E].flags &= ~BLOCKS_VISION_TOO;
	obstacle_map[ISO_BOTLINE_04_E].filename = "iso_botline_0009.png";
	
	block_2param(ISO_BOTLINE_05_N, 4.5, 1.70);
	obstacle_map[ISO_BOTLINE_05_N].flags &= ~BLOCKS_VISION_TOO;
	obstacle_map[ISO_BOTLINE_05_N].filename = "iso_botline_0010.png";
	
	block_2param(ISO_BOTLINE_05_E, 1.70, 4.5);
	obstacle_map[ISO_BOTLINE_05_E].flags &= ~BLOCKS_VISION_TOO;
	obstacle_map[ISO_BOTLINE_05_E].filename = "iso_botline_0011.png";

	block_2param(ISO_FREIGHTER_RAILWAY_01_N, 3, 3);
	obstacle_map[ISO_FREIGHTER_RAILWAY_01_N].flags &= ~BLOCKS_VISION_TOO;
	obstacle_map[ISO_FREIGHTER_RAILWAY_01_N].filename = "iso_freighter_railway_0000.png";

	block_2param(ISO_FREIGHTER_RAILWAY_01_E, 3, 3);
	obstacle_map[ISO_FREIGHTER_RAILWAY_01_E].flags &= ~BLOCKS_VISION_TOO;
	obstacle_map[ISO_FREIGHTER_RAILWAY_01_E].filename = "iso_freighter_railway_0001.png";

	block_2param(ISO_FREIGHTER_RAILWAY_02_S, 3, 3);
	obstacle_map[ISO_FREIGHTER_RAILWAY_02_S].flags &= ~BLOCKS_VISION_TOO;
	obstacle_map[ISO_FREIGHTER_RAILWAY_02_S].filename = "iso_freighter_railway_0002.png";

	block_2param(ISO_FREIGHTER_RAILWAY_02_E, 3, 3);
	obstacle_map[ISO_FREIGHTER_RAILWAY_02_E].flags &= ~BLOCKS_VISION_TOO;
	obstacle_map[ISO_FREIGHTER_RAILWAY_02_E].filename = "iso_freighter_railway_0003.png";

	block_2param(ISO_FREIGHTER_RAILWAY_02_N, 3, 3);
	obstacle_map[ISO_FREIGHTER_RAILWAY_02_N].flags &= ~BLOCKS_VISION_TOO;
	obstacle_map[ISO_FREIGHTER_RAILWAY_02_N].filename = "iso_freighter_railway_0004.png";

	block_2param(ISO_FREIGHTER_RAILWAY_02_W, 3, 3);
	obstacle_map[ISO_FREIGHTER_RAILWAY_02_W].flags &= ~BLOCKS_VISION_TOO;
	obstacle_map[ISO_FREIGHTER_RAILWAY_02_W].filename = "iso_freighter_railway_0005.png";

	block_2param(ISO_REACTOR_S, 4.5, 4);
	obstacle_map[ISO_REACTOR_S].flags &= ~BLOCKS_VISION_TOO;
	obstacle_map[ISO_REACTOR_S].filename = "iso_reactor_1_0000.png";
	
	block_2param(ISO_REACTOR_E, 4, 4.5);
	obstacle_map[ISO_REACTOR_E].flags &= ~BLOCKS_VISION_TOO;
	obstacle_map[ISO_REACTOR_E].filename = "iso_reactor_1_0001.png";
	
	block_2param(ISO_REACTOR_N, 4.5, 4);
	obstacle_map[ISO_REACTOR_N].flags &= ~BLOCKS_VISION_TOO;
	obstacle_map[ISO_REACTOR_N].filename = "iso_reactor_1_0002.png";
	
	block_2param(ISO_REACTOR_W, 4, 4.5);
	obstacle_map[ISO_REACTOR_W].flags &= ~BLOCKS_VISION_TOO;
	obstacle_map[ISO_REACTOR_W].filename = "iso_reactor_1_0003.png";
	
	block_2param(ISO_WALL_TERMINAL_S, 0.6, 0.4);
	obstacle_map[ISO_WALL_TERMINAL_S].flags |= IS_CLICKABLE;
	obstacle_map[ISO_WALL_TERMINAL_S].filename = "iso_wall_terminal_0000.png";
	obstacle_map[ISO_WALL_TERMINAL_S].flags &= ~BLOCKS_VISION_TOO;
	obstacle_map[ISO_WALL_TERMINAL_S].label = _("Terminal");
	obstacle_map[ISO_WALL_TERMINAL_S].action = &terminal_connect_action;

	block_2param(ISO_WALL_TERMINAL_E, 0.4, 0.6);
	obstacle_map[ISO_WALL_TERMINAL_E].flags |= IS_CLICKABLE;
	obstacle_map[ISO_WALL_TERMINAL_E].filename = "iso_wall_terminal_0002.png";
	obstacle_map[ISO_WALL_TERMINAL_E].flags &= ~BLOCKS_VISION_TOO;
	obstacle_map[ISO_WALL_TERMINAL_E].label = _("Terminal");
	obstacle_map[ISO_WALL_TERMINAL_E].action = &terminal_connect_action;
	
	block_2param(ISO_WALL_TERMINAL_N, 0.6, 0.4);
	obstacle_map[ISO_WALL_TERMINAL_N].flags |= IS_CLICKABLE;
	obstacle_map[ISO_WALL_TERMINAL_N].filename = "iso_wall_terminal_0004.png";
	obstacle_map[ISO_WALL_TERMINAL_N].flags &= ~BLOCKS_VISION_TOO;
	obstacle_map[ISO_WALL_TERMINAL_N].label = _("Terminal");
	obstacle_map[ISO_WALL_TERMINAL_N].action = &terminal_connect_action;
	
	block_2param(ISO_WALL_TERMINAL_W, 0.4, 0.6);
	obstacle_map[ISO_WALL_TERMINAL_W].flags |= IS_CLICKABLE;
	obstacle_map[ISO_WALL_TERMINAL_W].filename = "iso_wall_terminal_0006.png";
	obstacle_map[ISO_WALL_TERMINAL_W].flags &= ~BLOCKS_VISION_TOO;
	obstacle_map[ISO_WALL_TERMINAL_W].label = _("Terminal");
	obstacle_map[ISO_WALL_TERMINAL_W].action = &terminal_connect_action;
	
	block_2param(ISO_TURBINES_SMALL_W, 1.1, 1.8);
	obstacle_map[ISO_TURBINES_SMALL_W].flags &= ~BLOCKS_VISION_TOO;
	obstacle_map[ISO_TURBINES_SMALL_W].filename = "iso_turbines_small_0000.png";
	
	block_2param(ISO_TURBINES_SMALL_N, 1.8, 1.05);
	obstacle_map[ISO_TURBINES_SMALL_N].flags &= ~BLOCKS_VISION_TOO;
	obstacle_map[ISO_TURBINES_SMALL_N].filename = "iso_turbines_small_0001.png";
	
	block_2param(ISO_TURBINES_SMALL_E, 1.1, 1.8);
	obstacle_map[ISO_TURBINES_SMALL_E].flags &= ~BLOCKS_VISION_TOO;
	obstacle_map[ISO_TURBINES_SMALL_E].filename = "iso_turbines_small_0002.png";
	
	block_2param(ISO_TURBINES_SMALL_S, 1.8, 1.05);
	obstacle_map[ISO_TURBINES_SMALL_S].flags &= ~BLOCKS_VISION_TOO;
	obstacle_map[ISO_TURBINES_SMALL_S].filename = "iso_turbines_small_0003.png";
	
	obstacle_map[ISO_WEAPON_CRATE].flags |= DROPS_RANDOM_TREASURE;
	block_2param(ISO_WEAPON_CRATE, 1.3, 1.3);
	obstacle_map[ISO_WEAPON_CRATE].flags |= IS_SMASHABLE | IS_CLICKABLE;
	obstacle_map[ISO_WEAPON_CRATE].action = &barrel_action;
	obstacle_map[ISO_WEAPON_CRATE].label = "Weapon Crate";
	obstacle_map[ISO_WEAPON_CRATE].filename = "iso_weapon_crate.png";
	
	// dead bodies
	obstacle_map[ISO_DEFAULT_DEAD_BODY_0_1].block_area_type = COLLISION_TYPE_NONE;
	obstacle_map[ISO_DEFAULT_DEAD_BODY_0_1].filename = "default_dead_body_00_0001.png";
	obstacle_map[ISO_DEFAULT_DEAD_BODY_0_1].flags &= ~NEEDS_PRE_PUT;
	
	obstacle_map[ISO_DEFAULT_DEAD_BODY_1_1].block_area_type = COLLISION_TYPE_NONE;
	obstacle_map[ISO_DEFAULT_DEAD_BODY_1_1].filename = "default_dead_body_02_0001.png";
	obstacle_map[ISO_DEFAULT_DEAD_BODY_1_1].flags &= ~NEEDS_PRE_PUT;
	
	obstacle_map[ISO_DEFAULT_DEAD_BODY_2_1].block_area_type = COLLISION_TYPE_NONE;
	obstacle_map[ISO_DEFAULT_DEAD_BODY_2_1].filename = "default_dead_body_04_0001.png";
	obstacle_map[ISO_DEFAULT_DEAD_BODY_2_1].flags &= ~NEEDS_PRE_PUT;
	
	obstacle_map[ISO_DEFAULT_DEAD_BODY_3_1].block_area_type = COLLISION_TYPE_NONE;
	obstacle_map[ISO_DEFAULT_DEAD_BODY_3_1].filename = "default_dead_body_06_0001.png";
	obstacle_map[ISO_DEFAULT_DEAD_BODY_3_1].flags &= ~NEEDS_PRE_PUT;
	
	obstacle_map[ISO_DEFAULT_DEAD_BODY_4_1].block_area_type = COLLISION_TYPE_NONE;
	obstacle_map[ISO_DEFAULT_DEAD_BODY_4_1].filename = "default_dead_body_08_0001.png";
	obstacle_map[ISO_DEFAULT_DEAD_BODY_4_1].flags &= ~NEEDS_PRE_PUT;
	
	obstacle_map[ISO_DEFAULT_DEAD_BODY_5_1].block_area_type = COLLISION_TYPE_NONE;
	obstacle_map[ISO_DEFAULT_DEAD_BODY_5_1].filename = "default_dead_body_10_0001.png";
	obstacle_map[ISO_DEFAULT_DEAD_BODY_5_1].flags &= ~NEEDS_PRE_PUT;
	
	obstacle_map[ISO_DEFAULT_DEAD_BODY_6_1].block_area_type = COLLISION_TYPE_NONE;
	obstacle_map[ISO_DEFAULT_DEAD_BODY_6_1].filename = "default_dead_body_12_0001.png";
	obstacle_map[ISO_DEFAULT_DEAD_BODY_6_1].flags &= ~NEEDS_PRE_PUT;
	
	obstacle_map[ISO_DEFAULT_DEAD_BODY_7_1].block_area_type = COLLISION_TYPE_NONE;
	obstacle_map[ISO_DEFAULT_DEAD_BODY_7_1].filename = "default_dead_body_14_0001.png";
	obstacle_map[ISO_DEFAULT_DEAD_BODY_7_1].flags &= ~NEEDS_PRE_PUT;
	
	obstacle_map[ISO_DEFAULT_DEAD_BODY_0_2].block_area_type = COLLISION_TYPE_NONE;
	obstacle_map[ISO_DEFAULT_DEAD_BODY_0_2].filename = "default_dead_human_00_0001.png";
	obstacle_map[ISO_DEFAULT_DEAD_BODY_0_2].flags &= ~NEEDS_PRE_PUT;

	obstacle_map[ISO_LADDER_1].block_area_type = COLLISION_TYPE_NONE;
	obstacle_map[ISO_LADDER_1].filename = "iso_ladder_0001.png";
	obstacle_map[ISO_LADDER_1].flags &= ~NEEDS_PRE_PUT;
	
	obstacle_map[ISO_LADDER_2].block_area_type = COLLISION_TYPE_NONE;
	obstacle_map[ISO_LADDER_2].filename = "iso_ladder_0002.png";
	obstacle_map[ISO_LADDER_2].flags &= ~NEEDS_PRE_PUT;



	block_2param(ISO_WRECKED_CAR_1, 1.4, 2.8);
	obstacle_map[ISO_WRECKED_CAR_1].filename = "iso_wrecked_car_0001.png";
	obstacle_map[ISO_WRECKED_CAR_1].flags &= ~BLOCKS_VISION_TOO;


	block_2param(ISO_WRECKED_CAR_2, 2.8, 1.4);
	obstacle_map[ISO_WRECKED_CAR_2].filename = "iso_wrecked_car_0002.png";
	obstacle_map[ISO_WRECKED_CAR_2].flags &= ~BLOCKS_VISION_TOO;


	block_2param(ISO_WRECKED_CAR_3, 1.4, 2.8);
	obstacle_map[ISO_WRECKED_CAR_3].filename = "iso_wrecked_car_0003.png";
	obstacle_map[ISO_WRECKED_CAR_3].flags &= ~BLOCKS_VISION_TOO;



	block_2param(ISO_WRECKED_CAR_4, 2.8, 1.4);
	obstacle_map[ISO_WRECKED_CAR_4].filename = "iso_wrecked_car_0004.png";
	obstacle_map[ISO_WRECKED_CAR_4].flags &= ~BLOCKS_VISION_TOO;






	for (i = 0; i < NUMBER_OF_OBSTACLE_TYPES; i++) {
		if (!obstacle_map[i].filename) {
			obstacle_map[i].filename = MyMalloc(100);	// that should be sufficient for file names...
			sprintf(obstacle_map[i].filename, "iso_obstacle_%04d.png", i);
		}

	}

	// corrections for corner and T walls
	obstacle_map[ISO_THICK_WALL_T_E].left_border = obstacle_map[ISO_THICK_WALL_V].left_border;
	obstacle_map[ISO_THICK_WALL_T_W].right_border = obstacle_map[ISO_THICK_WALL_V].right_border;
	obstacle_map[ISO_THICK_WALL_T_N].lower_border = obstacle_map[ISO_THICK_WALL_H].lower_border;
	obstacle_map[ISO_THICK_WALL_T_S].upper_border = obstacle_map[ISO_THICK_WALL_H].upper_border;

	obstacle_map[ISO_THICK_WALL_CORNER_NE].upper_border = obstacle_map[ISO_THICK_WALL_H].upper_border;
	obstacle_map[ISO_THICK_WALL_CORNER_NE].right_border = obstacle_map[ISO_THICK_WALL_V].right_border;

	obstacle_map[ISO_THICK_WALL_CORNER_SE].lower_border = obstacle_map[ISO_THICK_WALL_H].lower_border;
	obstacle_map[ISO_THICK_WALL_CORNER_SE].right_border = obstacle_map[ISO_THICK_WALL_V].right_border;

	obstacle_map[ISO_THICK_WALL_CORNER_NW].upper_border = obstacle_map[ISO_THICK_WALL_H].upper_border;
	obstacle_map[ISO_THICK_WALL_CORNER_NW].left_border = obstacle_map[ISO_THICK_WALL_V].left_border;

	obstacle_map[ISO_THICK_WALL_CORNER_SW].lower_border = obstacle_map[ISO_THICK_WALL_H].lower_border;
	obstacle_map[ISO_THICK_WALL_CORNER_SW].left_border = obstacle_map[ISO_THICK_WALL_V].left_border;

	for (i = 0; i < NUMBER_OF_OBSTACLE_TYPES; i++) {	//compute the diagonal length for colldet
		obstacle_map[i].diaglength =
		    sqrt((obstacle_map[i].left_border * obstacle_map[i].left_border) +
			 (obstacle_map[i].upper_border * obstacle_map[i].upper_border));
	}
};				// void init_obstacle_data( void )

