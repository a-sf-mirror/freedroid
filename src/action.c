/* 
 *
 *   Copyright (c) 1994, 2002, 2003 Johannes Prix
 *   Copyright (c) 1994, 2002 Reinhard Prix
 *   Copyright (c) 2004-2010 Arthur Huillet
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
 * This file contains most of the activable's object/user interaction framework.
 */

#define _action_c

#include "system.h"

#include "defs.h"
#include "struct.h"
#include "global.h"
#include "proto.h"

/**
 * Find a reachable position to drop an item near a chest.
 *
 * This function looks for a position near a given chest that
 * satisfies : DLC(chest, item) && DLW(Tux, item)
 *
 * The use of the DLC() call has 2 purposes:
 * - ensure that the item will be dropped on a position that is in
 *   line of sight with the chest
 * - ensure that there are no other items already dropped there
 *
 * If no position is found, Tux current position is used as a fallback
 * (we know that Tux is near the chest, because he is opening it)
 *
 * Note: a real random position is not used, in order to minimize the CPU cost
 */
static void find_dropable_position_near_chest(float *item_x, float *item_y, int obst_index, level *obst_level)
{
	float obst_x = obst_level->obstacle_list[obst_index].pos.x;
	float obst_y = obst_level->obstacle_list[obst_index].pos.y;
	moderately_finepoint offset_vector;
	int tries;

	// Initialize the item position with the fallback position
	*item_x = Me.pos.x;
	*item_y = Me.pos.y;

	// Step 1: randomly choose one of the 8 main 45° directions around the chest
	float obs_diag = obstacle_map[obst_level->obstacle_list[obst_index].type].diaglength;
	offset_vector.x = obs_diag + 0.5;
	offset_vector.y = 0.0;
	RotateVectorByAngle(&offset_vector, (float)MyRandom(8) * 45.0);

	// Step 2: rotate offset_vector by 45° until an available start position is found
	colldet_filter filter = ObstacleByIdPassFilter;
	filter.data = &obst_index;

	tries = 0;
	while ((!DirectLineColldet(obst_x, obst_y, obst_x + offset_vector.x, obst_y + offset_vector.y, Me.pos.z, &filter) ||
		!DirectLineColldet(Me.pos.x, Me.pos.y, obst_x + offset_vector.x, obst_y + offset_vector.y, Me.pos.z, &WalkablePassFilter))
	       && (tries < 8)) {
		RotateVectorByAngle(&offset_vector, 45.0);
		++tries;
	}
	if (tries == 8)
		return;		// No start position available : fallback to Tux's feet

	// New fallback position will be that start position
	*item_x = obst_x + offset_vector.x;
	*item_y = obst_y + offset_vector.y;

	// Step 3 : randomly choose an available position around that start position
	// Note: If we were only using the position computed on step 2, then we would
	// limit the potential dropable positions.
	tries = 0;
	float trimmer_x, trimmer_y;
	do {
		trimmer_x = (float)MyRandom(10) / 20.0 - 0.25;
		trimmer_y = (float)MyRandom(10) / 20.0 - 0.25;
		++tries;
	}
	while ((!DirectLineColldet(obst_x, obst_y, *item_x + trimmer_x, *item_y + trimmer_y, Me.pos.z, &filter) ||
		!DirectLineColldet(Me.pos.x, Me.pos.y, *item_x + trimmer_x, *item_y + trimmer_y, Me.pos.z, &WalkablePassFilter))
	       && (tries < 100));
	if (tries == 100)
		return;		// No final position available : fallback to start position

	// Step 4 : position found
	*item_x += trimmer_x;
	*item_y += trimmer_y;
}

/**
 * This function will drop the content of a chest on the floor, around
 * the chest.
 * If the chest does not contain specific items, then eventually drop a
 * random item.
 */
static void throw_out_all_chest_content(int obst_index)
{
	int i = 0;
	int j;
	float item_x, item_y;
	int drop_count = 0;
	level *lvl = CURLEVEL();

	struct dynarray *item_list = get_obstacle_extension(CURLEVEL(), obst_index, OBSTACLE_EXTENSION_CHEST_ITEMS);

	play_open_chest_sound();

	if (item_list) {
		int size = item_list->size;
		for (i = 0; i < size; i++) {

			item *it = &((item *)item_list->arr)[i];
			
			if (it->type == -1)
				continue;

			// Find a free items index on this level.
			j = find_free_floor_items_index(Me.pos.z);
			MoveItem(it, &(lvl->ItemList[j]));

			find_dropable_position_near_chest(&item_x, &item_y, obst_index, lvl);
			lvl->ItemList[j].pos.x = item_x;
			lvl->ItemList[j].pos.y = item_y;
			lvl->ItemList[j].pos.z = lvl->levelnum;		
			lvl->ItemList[j].throw_time = 0.01;

			drop_count++;
		}

		// Empty the item list
		for (i = 0; i < size; i++) {
			dynarray_del(item_list, 0, sizeof(item));
		}
		dynarray_free(item_list);

		// Remove the chest items obstacle extension
		del_obstacle_extension(lvl, obst_index, OBSTACLE_EXTENSION_CHEST_ITEMS);
	}
	
	// If the chest was empty, maybe generate a random item to be dropped
	if (!drop_count) {
		find_dropable_position_near_chest(&item_x, &item_y, obst_index, lvl);
		DropRandomItem(Me.pos.z, item_x, item_y, 0, FALSE);
	}
}

/**
 * The function detects, if the current mouse cursor is over a given obstacle.
 * The iso image of the obstacle will be used to check hovering.
 */
static int mouse_cursor_is_on_that_obstacle(level *lvl, int obst_index)
{
	// mouse_cursor_is_on_that_iso_image() needs a position defined relatively to
	// current level
	gps obs_pos = { lvl->obstacle_list[obst_index].pos.x,
		lvl->obstacle_list[obst_index].pos.y,
		lvl->levelnum
	};
	gps obs_vpos;
	update_virtual_position(&obs_vpos, &obs_pos, Me.pos.z);
	if (obs_vpos.z == -1)
		return FALSE;

	if (mouse_cursor_is_on_that_iso_image(obs_vpos.x, obs_vpos.y, get_obstacle_image(lvl->obstacle_list[obst_index].type))) {
		return (TRUE);
	}
	return (FALSE);
}

/**
 * This function checks if there is an obstacle beneath the current 
 * mouse cursor.  
 * It takes into account the actual size of the 
 * graphics and not only the geographic position of the mouse cursor on
 * the floor.
 *
 * If there is an obstacle below the mouse cursor, the function will
 * return its obstacle index.  Else -1 will be returned.
 */
int clickable_obstacle_below_mouse_cursor(level **obst_lvl)
{
	int x, y;
	finepoint MapPositionOfMouse;
	int i;
	int obst_index;

	*obst_lvl = NULL;

	// Now if the cursor is not inside the user rectangle at all, then
	// there can never be an obstacle under the mouse cursor...
	//
	if (!MouseCursorIsInUserRect(GetMousePos_x(), GetMousePos_y()))
		return (-1);

	// We find the approximate position of the mouse cursor on the floor.
	// We will use that as the basis for our scanning for obstacles under the
	// current mouse cursor.
	//
	MapPositionOfMouse.x = translate_pixel_to_map_location((float)input_axis.x, (float)input_axis.y, TRUE);
	MapPositionOfMouse.y = translate_pixel_to_map_location((float)input_axis.x, (float)input_axis.y, FALSE);

	// We will scan a small area around the mouse cursor, to ease object's selection
	// Each scanned position could be on a neighbor of the current's level,
	// so we should resolve all those virtual positions.
	// But resolving a virtual position is a bit CPU costly, so in order
	// to minimize cost, we only resolve the mouse cursor position.
	//
	gps mouse_target_vpos = { MapPositionOfMouse.x, MapPositionOfMouse.y, Me.pos.z };
	gps mouse_target_pos;
	if (!resolve_virtual_position(&mouse_target_pos, &mouse_target_vpos))
		return -1;
	if (!level_is_visible(mouse_target_pos.z))
		return -1;
	level *lvl = curShip.AllLevels[mouse_target_pos.z];

	for (y = mouse_target_pos.y + 3; y > mouse_target_pos.y - 3; y--) {
		for (x = mouse_target_pos.x + 3; x > mouse_target_pos.x - 3; x--) {
			if (!pos_inside_level(x, y, lvl))
				continue;

			// scan all obstacles of the level targeted by the mouse cursor 
			for (i = 0; i < MAX_OBSTACLES_GLUED_TO_ONE_MAP_TILE; i++) {
				obst_index = lvl->map[y][x].obstacles_glued_to_here[i];
				if (obst_index == (-1))
					continue;
				if (mouse_cursor_is_on_that_obstacle(lvl, obst_index)) {
					DebugPrintf(1, "\n%s(): obstacle under cursor identified.", __FUNCTION__);
					if (obstacle_map[lvl->obstacle_list[obst_index].type].flags & IS_CLICKABLE) {
						*obst_lvl = lvl;
						return obst_index;
					} else {
						continue;
					}
				}
			}
		}
	}

	return (-1);
}				// int obstacle_below_mouse_cursor (level **, int [], int)

/**
 * In order to perform a click-action, tux must first move to the obstacle.
 * He can either approach it from a specific direction or any direction. This 
 * function handles the case where direction doesn't matter, like for barrels
 * and crates. Returns 1 if tux is close enought to the obstacle.
 */
static int reach_obstacle_from_any_direction(level *obst_lvl, int obst_index) {
	gps obst_vpos;
	update_virtual_position(&obst_vpos, &(obst_lvl->obstacle_list[obst_index].pos), Me.pos.z);
	int obst_type = obst_lvl->obstacle_list[obst_index].type;
    if (calc_distance(Me.pos.x, Me.pos.y, obst_vpos.x, obst_vpos.y)
		<= (obstacle_map[obst_type].block_area_parm_1 * sqrt(2)) / 2.0 + 0.5) {
    	// Maybe a combo_action has made us come here and open the chest.  Then of
    	// course we can remove the combo action setting now...
    	//
    	Me.mouse_move_target_combo_action_type = NO_COMBO_ACTION_SET;
    	Me.mouse_move_target_combo_action_parameter = -1;
        return 1;
    }	
    
	// We set up a course, that will lead us directly to the barrel, that we are
	// supposed to smash (upon arrival, later).
	//
	// For this purpose, we take a vector and rotate it around the barrel center to
	// find the 'best' location to go to for the smashing motion...

	// First, if the barrel is in direct line with Tux, we search a position that the
	// Tux can reach.
	// We normalize the distance of the final walk-point to the barrel center just
	// so, that it will be within the 'strike_distance' we have used just above in
	// the 'distance-met' query.
	//

	moderately_finepoint step_vector;
	float vec_len;
	int i;
	step_vector.x = Me.pos.x - obst_vpos.x;
	step_vector.y = Me.pos.y - obst_vpos.y;
	vec_len = vect_len(step_vector);

	step_vector.x *= ((obstacle_map[obst_type].block_area_parm_1 * sqrt(2)) / 2.0 + 0.05) / vec_len;
	step_vector.y *= ((obstacle_map[obst_type].block_area_parm_1 * sqrt(2)) / 2.0 + 0.05) / vec_len;

	for (i = 0; i < 8; i++) {
		if (DirectLineColldet(Me.pos.x, Me.pos.y, obst_vpos.x, obst_vpos.y, Me.pos.z, &WalkablePassFilter)) {
			// The obstacle plus the step vector give us the position to move the
			// Tux to for the optimal strike...
			// This position has to be defined relatively to the barrel's level, so that we
			// can retrieve the barrel later (at the end the combo action)
			//
			Me.mouse_move_target.x = step_vector.x + obst_lvl->obstacle_list[obst_index].pos.x;
			Me.mouse_move_target.y = step_vector.y + obst_lvl->obstacle_list[obst_index].pos.y;
			Me.mouse_move_target.z = obst_lvl->obstacle_list[obst_index].pos.z;

			// We set up the combo_action, so that the barrel can be smashed later...
			//
			enemy_set_reference(&Me.current_enemy_target_n, &Me.current_enemy_target_addr, NULL);
			Me.mouse_move_target_combo_action_type = COMBO_ACTION_OBSTACLE;
			Me.mouse_move_target_combo_action_parameter = obst_index;
			return 0;
		}
		// If this vector didn't bring us any luck, we rotate by 45 degrees and try anew...
		//
		RotateVectorByAngle(&step_vector, 45.0);
	}

	// Second : if the barrel is not in direct line with Tux, we search a position
	// near the barrel that is free of obstacle. However, it could happen that the
	// pathfinder will not be able to find a way to go there, but it would be too
	// costly to launch the pathfinder here. Anyway, if no path exists, Tux will
	// not start to move, so the player will certainly try to get closer to the
	// barrel.
	// We will rotate step_vector around the center of the barrel
	//
	step_vector.x = Me.pos.x - obst_vpos.x;
	step_vector.y = Me.pos.y - obst_vpos.y;
	vec_len = vect_len(step_vector);
	step_vector.x /= vec_len;
	step_vector.y /= vec_len;

	// point_near_barrel is a point very near the barrel in the step_vector's direction,
	// point_away_from_barrel is a point in the same direction but a bit farther
	moderately_finepoint point_near_obst, point_away_from_obst;

	// half-size of the barrel
	moderately_finepoint half_size = { (obstacle_map[obst_type].block_area_parm_1 * sqrt(2)) / 2.0,
		(obstacle_map[obst_type].block_area_parm_2 * sqrt(2)) / 2.0
	};

	for (i = 0; i < 8; i++) {
		// (no need to optimize this, the compiler will do its job...)
		point_near_obst.x = obst_vpos.x + step_vector.x * (half_size.x + 0.05);
		point_near_obst.y = obst_vpos.y + step_vector.y * (half_size.y + 0.05);
		point_away_from_obst.x = obst_vpos.x + step_vector.x * (half_size.x + 0.2);
		point_away_from_obst.y = obst_vpos.y + step_vector.y * (half_size.y + 0.2);

		// check if the line from point_near_barrel to point_away_from_barrel is walkable
		if (DirectLineColldet(point_near_obst.x, point_near_obst.y,
					  point_away_from_obst.x, point_away_from_obst.y, Me.pos.z, &WalkablePassFilter)) {
			// point_to_barrel seems good, move Tux there
			// This position has to be defined relatively to the barrel's level, so that we
			// can retrieve the barrel later (at the end the combo action)
			//
			Me.mouse_move_target.x =
				obst_lvl->obstacle_list[obst_index].pos.x + step_vector.x * (half_size.x + 0.05);
			Me.mouse_move_target.y =
				obst_lvl->obstacle_list[obst_index].pos.y + step_vector.y * (half_size.y + 0.05);
			Me.mouse_move_target.z = obst_lvl->obstacle_list[obst_index].pos.z;

			// We set up the combo_action, so that the barrel can be smashed later, on the
			// second call (made by move_tux_towards_intermediate_point)...
			//
			enemy_set_reference(&Me.current_enemy_target_n, &Me.current_enemy_target_addr, NULL);
			Me.mouse_move_target_combo_action_type = COMBO_ACTION_OBSTACLE;
			Me.mouse_move_target_combo_action_parameter = obst_index;

			return 0;
		}
		// rotate the step_vector to check an other point
		RotateVectorByAngle(&step_vector, 45.0);
	}

	return 0;
};			  // int reach_obstacle_from_any_direction(level *obst_lvl, int obst_index)

/**
 * In order to perform a click-action, tux must first move to the obstacle.
 * He can either approach it from a specific direction or any direction. This 
 * function handles the case where direction does matter, like for chests, 
 * terminals, and signs. Returns 1 if tux is close enought to the obstacle.
 */
static int reach_obstacle_from_specific_direction(level *obst_lvl, int obst_index, int direction) {
	gps obst_vpos;
	update_virtual_position(&obst_vpos, &(obst_lvl->obstacle_list[obst_index].pos), Me.pos.z);
	if (fabsf(Me.pos.x - obst_vpos.x) + fabsf(Me.pos.y - obst_vpos.y) < 1.1) {
    	// Maybe a combo_action has made us come here and open the chest.  Then of
    	// course we can remove the combo action setting now...
    	//
    	Me.mouse_move_target_combo_action_type = NO_COMBO_ACTION_SET;
    	Me.mouse_move_target_combo_action_parameter = -1;
        return 1;
    }

	// So here we know, that we must set the course thowards the chest.  We
	// do so first.
	// The target's position has to be defined relatively to the chest's level, so that we
	// can retrieve the chest later (at the end the combo action)
	//
	DebugPrintf(2, "\nreach_obstacle_from_specific_direction:  setting up combined mouse move target!");
		
	Me.mouse_move_target.x = obst_lvl->obstacle_list[obst_index].pos.x;
	Me.mouse_move_target.y = obst_lvl->obstacle_list[obst_index].pos.y;
	Me.mouse_move_target.z = obst_lvl->levelnum;
	enemy_set_reference(&Me.current_enemy_target_n, &Me.current_enemy_target_addr, NULL);
	Me.mouse_move_target_combo_action_type = COMBO_ACTION_OBSTACLE;
	Me.mouse_move_target_combo_action_parameter = obst_index;
	int obst_type = obst_lvl->obstacle_list[obst_index].type;

	switch (direction) {
	case EAST:
		Me.mouse_move_target.x += obstacle_map[obst_type].block_area_parm_1;
		break;
	case SOUTH:
		Me.mouse_move_target.y += obstacle_map[obst_type].block_area_parm_2;
		break;
	case WEST:
		Me.mouse_move_target.x -= obstacle_map[obst_type].block_area_parm_1;
		break;
	case NORTH:
		Me.mouse_move_target.y -= obstacle_map[obst_type].block_area_parm_2;
		break;
	default:
		ErrorMessage(__FUNCTION__, "Invalid direction!!", PLEASE_INFORM, IS_FATAL);
		break;
	}
    return 0;
};			  // int reach_obstacle_from_specific_direction(level **obst_lvl, int obst_index, int direction)
// ------ INSERT ACTION FUNCTION BELOW ------
/**
 * This function executes a chest's action, if Tux is near enough to activate it.
 * If Tux is too far, a combo action will be started, to first move Tux near
 * the chest.
 */
void chest_open_action(level *chest_lvl, int chest_index)
{
	if (chest_index == (-1))
		return;

	// So the player clicked on a chest.  Well, if the chest is already close
	// enough, it should be sufficient to just spill out the contents of the
	// chest and then return.  However, if the player was not yet close enough
	// to the chest, or if the player is on an other level, we need to 
    // SET A COMBINED_ACTION, i.e. first set the walk thowards the chest and 
    // then set the open_chest action, which is more complicated of course.

	// get the chest position relatively to Tux's level, in order to compute a distance
	gps chest_vpos;
	update_virtual_position(&chest_vpos, &(chest_lvl->obstacle_list[chest_index].pos), Me.pos.z);
	int direction = 0;
	switch (chest_lvl->obstacle_list[chest_index].type) {
    	case ISO_V_CHEST_CLOSED:
    	case ISO_E_CHEST2_CLOSED:
    		direction = EAST;
    		break;
    	case ISO_H_CHEST_CLOSED:
    	case ISO_S_CHEST2_CLOSED:
    		direction = SOUTH;
    		break;
    	case ISO_W_CHEST2_CLOSED:
    		direction = WEST;
    		break;
    	case ISO_N_CHEST2_CLOSED:
    		direction = NORTH;
    		break;
    	default:
    		ErrorMessage(__FUNCTION__, "chest to be approached is not a closed chest obstacle!!", PLEASE_INFORM, IS_FATAL);
    		break;
	}
	if (!reach_obstacle_from_specific_direction(chest_lvl, chest_index, direction)) 
        return;
    else {
		// Check that the chest is really reachable, and not behind an obstacle
		colldet_filter filter = ObstacleByIdPassFilter;
		filter.data = &chest_index;
		if (DirectLineColldet(Me.pos.x, Me.pos.y, chest_vpos.x, chest_vpos.y, Me.pos.z, &filter)) {
			throw_out_all_chest_content(chest_index);

			if (chest_lvl->obstacle_list[chest_index].type == ISO_H_CHEST_CLOSED)
				chest_lvl->obstacle_list[chest_index].type = ISO_H_CHEST_OPEN;
			if (chest_lvl->obstacle_list[chest_index].type == ISO_V_CHEST_CLOSED)
				chest_lvl->obstacle_list[chest_index].type = ISO_V_CHEST_OPEN;
			if (chest_lvl->obstacle_list[chest_index].type == ISO_N_CHEST2_CLOSED)
				chest_lvl->obstacle_list[chest_index].type = ISO_N_CHEST2_OPEN;
			if (chest_lvl->obstacle_list[chest_index].type == ISO_E_CHEST2_CLOSED)
				chest_lvl->obstacle_list[chest_index].type = ISO_E_CHEST2_OPEN;
			if (chest_lvl->obstacle_list[chest_index].type == ISO_S_CHEST2_CLOSED)
				chest_lvl->obstacle_list[chest_index].type = ISO_S_CHEST2_OPEN;
			if (chest_lvl->obstacle_list[chest_index].type == ISO_W_CHEST2_CLOSED)
				chest_lvl->obstacle_list[chest_index].type = ISO_W_CHEST2_OPEN;
		}
		// Now that the chest has been possibly opened, we don't need to do anything more
		//
		return;
	} 
}

/**
 * This function executes a barrel's action, if Tux is near enough to activate it.
 * If Tux is too far, a combo action will be started, to first move Tux near
 * the barrel.
 */
void barrel_action(level *barrel_lvl, int barrel_index)
{
	moderately_finepoint step_vector;
	gps barrel_vpos;

	if (barrel_index == (-1))
		return;

	update_virtual_position(&barrel_vpos, &(barrel_lvl->obstacle_list[barrel_index].pos), Me.pos.z);
	if (barrel_vpos.x == -1)
		return;

	// If the smash distance for a barrel is not yet reached, then we must set up
	// a course that will lead us to the barrel and also a combo_action specification,
	// that will cause the corresponding barrel to be smashed upon arrival.
	//
	if (!reach_obstacle_from_any_direction(barrel_lvl, barrel_index)) 
        return;
	//===================
	// The character is near enough from the barrel to smash it.
	//

	// Before the barrel can get destroyed and we lose the position information,
	// we record the vector of the charecter's strike direction...
	//
	step_vector.x = -Me.pos.x + barrel_vpos.x;
	step_vector.y = -Me.pos.y + barrel_vpos.y;

	// Check that the barrel is really reachable, and not behind an obstacle
	//
	colldet_filter filter = ObstacleByIdPassFilter;
	filter.data = &barrel_index;

	if (DirectLineColldet(Me.pos.x, Me.pos.y, barrel_vpos.x, barrel_vpos.y, Me.pos.z, &filter)) {
		// We make sure the barrel gets smashed, even if the strike made by the
		// Tux would be otherwise a miss...
		//
		smash_obstacle(barrel_lvl->obstacle_list[barrel_index].pos.x,
				   barrel_lvl->obstacle_list[barrel_index].pos.y, barrel_lvl->obstacle_list[barrel_index].pos.z);

		// We start an attack motion...
		// Since the character is just aside the barrel, we use a melee shot,
		// in order to avoid the lost of an ammunition.
		//
		int store_weapon_type = Me.weapon_item.type;
		Me.weapon_item.type = -1;
		tux_wants_to_attack_now(FALSE);
		Me.weapon_item.type = store_weapon_type;

		// We set a direction of facing directly thowards the barrel in question
		// so that the strike motion looks authentic...
		//
		Me.angle = -(atan2(step_vector.y, step_vector.x) * 180 / M_PI - 180 - 45);
		Me.angle += 360 / (2 * MAX_TUX_DIRECTIONS);
		while (Me.angle < 0)
			Me.angle += 360;
	}

	DebugPrintf(2, "\ncheck_for_barrels_to_smash(...):  combo_action now unset.");
}

/**
 * This function connects the player to an interactive computer terminal ingame.
 */
void terminal_connect_action(level *lvl, int terminal_index)
{
	gps terminal_vpos;

	if (terminal_index == (-1))
		return;

	update_virtual_position(&terminal_vpos, &(lvl->obstacle_list[terminal_index].pos), Me.pos.z);
	if (terminal_vpos.x == -1)
		return;

	int direction;
	switch (lvl->obstacle_list[terminal_index].type) {
		// Console directions are backwards...
		case ISO_CONSOLE_N:
    		direction = SOUTH;
    		break;
		case ISO_CONSOLE_S:
			direction = NORTH;
			break;
		case ISO_CONSOLE_E:
			direction = WEST;
			break;
		case ISO_CONSOLE_W:
			direction = EAST;
			break;
    	default:
    		ErrorMessage(__FUNCTION__, "Tried to approach a terminal of type %d, but this type is unknown.", PLEASE_INFORM, IS_WARNING_ONLY, lvl->obstacle_list[terminal_index].type);
			direction = NORTH;
    		break;
	}

	if (!reach_obstacle_from_specific_direction(lvl, terminal_index, direction)) 
        return;

	colldet_filter filter = ObstacleByIdPassFilter;
	filter.data = &terminal_index;

	if (DirectLineColldet(Me.pos.x, Me.pos.y, terminal_vpos.x, terminal_vpos.y, Me.pos.z, &filter)) {
		char *dialog = get_obstacle_extension(lvl, terminal_index, OBSTACLE_EXTENSION_DIALOGFILE);
		if (!dialog) {
			append_new_game_message("Colored patterns appear on the screen, but they do not look like any computer interface you have ever seen. Perhaps is this what they call a screen \"saver\".");
			return;
		}

		enemy dummy_enemy;
		dummy_enemy.dialog_section_name = dialog;
		dummy_enemy.will_rush_tux = 0;
		dummy_enemy.type = 34;
		ChatWithFriendlyDroid(&dummy_enemy);
	}
}

/**
 * Have Tux pick up an item if it is close enough and isn't blocked by any
 * obstacles (e.g. walls).
 *
 * If Tux is too far away, a combo action will be started, which will first make
 * Tux walk towards item. If the action is not cancelled, the item is picked up
 * on arrival.
 */
int check_for_items_to_pickup(level *item_lvl, int item_index)
{
	gps item_vpos;

	if (item_lvl == NULL || item_index == -1)
		return 0;

	update_virtual_position(&item_vpos, &item_lvl->ItemList[item_index].pos, Me.pos.z);

	if ((calc_distance(Me.pos.x, Me.pos.y, item_vpos.x, item_vpos.y) < ITEM_TAKE_DIST)
		&& DirectLineColldet(Me.pos.x, Me.pos.y, item_vpos.x, item_vpos.y, Me.pos.z, NULL))
	{
		AddFloorItemDirectlyToInventory(&(item_lvl->ItemList[item_index]));
	} else {
		Me.mouse_move_target.x = item_lvl->ItemList[item_index].pos.x;
		Me.mouse_move_target.y = item_lvl->ItemList[item_index].pos.y;
		Me.mouse_move_target.z = item_lvl->levelnum;

		// Set up the combo_action
		enemy_set_reference(&Me.current_enemy_target_n, &Me.current_enemy_target_addr, NULL);
		Me.mouse_move_target_combo_action_type = COMBO_ACTION_PICK_UP_ITEM;
		Me.mouse_move_target_combo_action_parameter = item_index;
	}

	return 1;
}

#undef _action_c
