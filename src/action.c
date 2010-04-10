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
	level *chest_level;
	int i;
	int j;
	float item_x, item_y;
	int icnt = 0;

	chest_level = curShip.AllLevels[Me.pos.z];

	DebugPrintf(0, "\nthrow_out_all_chest_content: call confimed.");

	//--------------------
	// First some check if the given obstacle is really a closed chest.
	//
	switch (chest_level->obstacle_list[obst_index].type) {
	case ISO_N_CHEST2_CLOSED:
	case ISO_S_CHEST2_CLOSED:
	case ISO_E_CHEST2_CLOSED:
	case ISO_W_CHEST2_CLOSED:
	case ISO_H_CHEST_CLOSED:
	case ISO_V_CHEST_CLOSED:
		// all is ok in this case.  it's really a chest.  fine.
		break;
	default:
		// no chest handed as the chest obstacle!  Clearly a severe error.!
		ErrorMessage(__FUNCTION__, "Obstacle given to empty is not really a chest!", PLEASE_INFORM, IS_FATAL);
		break;
	}

	//--------------------
	// Now we can throw out all the items from inside the chest and maybe
	// (later) also play a 'chest opening' sound.
	//
	for (i = 0; i < MAX_CHEST_ITEMS_PER_LEVEL; i++) {
		if (chest_level->ChestItemList[i].type == (-1))
			continue;

		// An item is defined to be in a chest if it is at the same position than the chest
		if (fabsf(chest_level->obstacle_list[obst_index].pos.x - chest_level->ChestItemList[i].pos.x) > 0.1)
			continue;
		if (fabsf(chest_level->obstacle_list[obst_index].pos.y - chest_level->ChestItemList[i].pos.y) > 0.1)
			continue;

		//--------------------
		// So this item is one of the right one and will now get thrown out of the chest:
		// 
		// First we find a free items index on this level.
		//
		DebugPrintf(0, "\nOne item now thrown out of the chest...");
		j = find_free_floor_items_index(Me.pos.z);
		MoveItem(&(chest_level->ChestItemList[i]), &(chest_level->ItemList[j]));

		find_dropable_position_near_chest(&item_x, &item_y, obst_index, chest_level);
		chest_level->ItemList[j].pos.x = item_x;
		chest_level->ItemList[j].pos.y = item_y;
		chest_level->ItemList[j].pos.z = chest_level->levelnum;		
		chest_level->ItemList[j].throw_time = 0.01;

		icnt++;
	}

	// If the chest was empty, maybe generate a random item to be dropped
	if (!icnt) {
		find_dropable_position_near_chest(&item_x, &item_y, obst_index, chest_level);
		DropRandomItem(Me.pos.z, item_x, item_y, 0, FALSE);
	}
	//--------------------
	// We play the sound, now that the chest is really opened...
	//
	play_open_chest_sound();
}

/**
 * The function detects, if the current mouse cursor is over a given object.
 * The iso image of the object will be used to check hovering.
 */
static int mouse_cursor_is_on_that_object(level *lvl, int obst_index)
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
 * This function checks if there is a closed chest beneath the current 
 * mouse cursor.  
 * It takes into account the actual size of the barrel/crate
 * graphics and not only the geographic position of the mouse cursor on
 * the floor.
 *
 * If there is a closed chest below the mouse cursor, the function will
 * return the obstacle index of the chest in question.  Else -1 will be
 * returned.
 */
int closed_chest_below_mouse_cursor(level **chest_lvl)
{
	int x, y;
	finepoint MapPositionOfMouse;
	int i;
	int obst_index;

	*chest_lvl = NULL;

	//--------------------
	// Now if the cursor is not inside the user rectangle at all, then
	// there can never be a barrel under the mouse cursor...
	//
	if (!MouseCursorIsInUserRect(GetMousePos_x(), GetMousePos_y()))
		return (-1);

	//--------------------
	// We find the approximate position of the mouse cursor on the floor.
	// We will use that as the basis for our scanning for barrels under the
	// current mouse cursor.
	//
	MapPositionOfMouse.x = translate_pixel_to_map_location((float)input_axis.x, (float)input_axis.y, TRUE);
	MapPositionOfMouse.y = translate_pixel_to_map_location((float)input_axis.x, (float)input_axis.y, FALSE);

	//--------------------
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

				switch (lvl->obstacle_list[obst_index].type) {
				case ISO_N_CHEST2_CLOSED:
				case ISO_S_CHEST2_CLOSED:
				case ISO_E_CHEST2_CLOSED:
				case ISO_W_CHEST2_CLOSED:
				case ISO_H_CHEST_CLOSED:
				case ISO_V_CHEST_CLOSED:
					if (mouse_cursor_is_on_that_object(lvl, obst_index)) {
						DebugPrintf(1, "\n%s(): closed chest under cursor identified.", __FUNCTION__);
						*chest_lvl = lvl;
						return (obst_index);
					}
					break;

				default:
					break;
				}

			}
		}
	}

	return (-1);
}

/**
 * This function checks if there is a barrel beneath the current mouse
 * cursor.  It takes into account the actual size of the barrel/crate
 * graphics and not only the geographic position of the mouse cursor on
 * the floor.
 *
 * If there is a barrel below the mouse cursor, the function will
 * return the obstacle index of the chest in question.  Else -1 will be
 * returned.
 */
int smashable_barrel_below_mouse_cursor(level **barrel_lvl)
{
	int x, y;
	finepoint MapPositionOfMouse;
	int i;
	int obst_index;

	*barrel_lvl = NULL;

	//--------------------
	// Now if the cursor is not inside the user rectangle at all, then
	// there can never be a barrel under the mouse cursor...
	//
	if (!MouseCursorIsInUserRect(GetMousePos_x(), GetMousePos_y()))
		return (-1);

	//--------------------
	// We find the approximate position of the mouse cursor on the floor.
	// We will use that as the basis for our scanning for barrels under the
	// current mouse cursor.
	//
	MapPositionOfMouse.x = translate_pixel_to_map_location((float)input_axis.x, (float)input_axis.y, TRUE);
	MapPositionOfMouse.y = translate_pixel_to_map_location((float)input_axis.x, (float)input_axis.y, FALSE);

	//--------------------
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

			for (i = 0; i < MAX_OBSTACLES_GLUED_TO_ONE_MAP_TILE; i++) {
				obst_index = lvl->map[y][x].obstacles_glued_to_here[i];

				if (obst_index == (-1))
					continue;

				switch (lvl->obstacle_list[obst_index].type) {
				case ISO_BARREL_1:
				case ISO_BARREL_2:
				case ISO_BARREL_3:
				case ISO_BARREL_4:
					if (mouse_cursor_is_on_that_object(lvl, obst_index)) {
						DebugPrintf(1, "\n%s(): barrel under cursor identified.", __FUNCTION__);
						*barrel_lvl = lvl;
						return (obst_index);
					}
					break;

				default:
					break;
				}

			}
		}
	}

	return (-1);
}

/**
 * This function executes a chest's action, if Tux is near enough to activate it.
 * If Tux is too far, a combo action will be started, to first move Tux near
 * the chest.
 */
void check_for_chests_to_open(level *chest_lvl, int chest_index)
{
	if (chest_index == (-1))
		return;

	//--------------------
	// So the player clicked on a chest.  Well, if the chest is already close
	// enough, it should be sufficient to just spill out the contents of the
	// chest and then return.  However, if the player was not yet close enough
	// to the chest, or if the player is on an other level, we need to 
	// SET A COMBINED_ACTION, i.e. first set the walk thowards the chest and 
	// then set the open_chest action, which is more complicated of course.
	//

	// get the chest position relatively to Tux's level, in order to compute a distance
	gps chest_vpos;
	update_virtual_position(&chest_vpos, &(chest_lvl->obstacle_list[chest_index].pos), Me.pos.z);

	if (fabsf(Me.pos.x - chest_vpos.x) + fabsf(Me.pos.y - chest_vpos.y) < 1.1) {
		//--------------------
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
		//--------------------
		// Maybe a combo_action has made us come here and open the chest.  Then of
		// course we can remove the combo action setting now...
		//
		Me.mouse_move_target_combo_action_type = NO_COMBO_ACTION_SET;
		Me.mouse_move_target_combo_action_parameter = -1;

		//--------------------
		// Now that the chest has been possibly opened, we don't need to do anything more
		//
		return;
	} else {
		//--------------------
		// So here we know, that we must set the course thowards the chest.  We
		// do so first.
		// The target's position has to be defined relatively to the chest's level, so that we
		// can retrieve the chest later (at the end the combo action)
		//
		DebugPrintf(2, "\ncheck_for_chests_to_open:  setting up combined mouse move target!");
			
		Me.mouse_move_target.x = chest_lvl->obstacle_list[chest_index].pos.x;
		Me.mouse_move_target.y = chest_lvl->obstacle_list[chest_index].pos.y;
		Me.mouse_move_target.z = chest_lvl->levelnum;
		enemy_set_reference(&Me.current_enemy_target_n, &Me.current_enemy_target_addr, NULL);
		Me.mouse_move_target_combo_action_type = COMBO_ACTION_OPEN_CHEST;
		Me.mouse_move_target_combo_action_parameter = chest_index;

		switch (chest_lvl->obstacle_list[chest_index].type) {
		case ISO_V_CHEST_CLOSED:
		case ISO_V_CHEST_OPEN:
		case ISO_E_CHEST2_CLOSED:
			Me.mouse_move_target.x += 0.8;
			break;
		case ISO_H_CHEST_CLOSED:
		case ISO_H_CHEST_OPEN:
		case ISO_S_CHEST2_CLOSED:
			Me.mouse_move_target.y += 0.8;
			break;
		case ISO_W_CHEST2_CLOSED:
			Me.mouse_move_target.x -= 0.8;
			break;
		case ISO_N_CHEST2_CLOSED:
			Me.mouse_move_target.y -= 0.8;
			break;
		default:
			ErrorMessage(__FUNCTION__, "chest to be approached is not a chest obstacle!!", PLEASE_INFORM, IS_FATAL);
			break;
		}
	}
}

/**
 * This function executes a barrel's action, if Tux is near enough to activate it.
 * If Tux is too far, a combo action will be started, to first move Tux near
 * the barrel.
 */
void check_for_barrels_to_smash(level *barrel_lvl, int barrel_index)
{
	int i;
	moderately_finepoint step_vector;
	float vec_len;
	gps barrel_vpos;

	if (barrel_index == (-1))
		return;

	update_virtual_position(&barrel_vpos, &(barrel_lvl->obstacle_list[barrel_index].pos), Me.pos.z);
	if (barrel_vpos.x == -1)
		return;

	//--------------------
	// If the smash distance for a barrel is not yet reached, then we must set up
	// a course that will lead us to the barrel and also a combo_action specification,
	// that will cause the corresponding barrel to be smashed upon arrival.
	//
	if (calc_euklid_distance(Me.pos.x, Me.pos.y, barrel_vpos.x, barrel_vpos.y)
	    > (obstacle_map[ISO_BARREL_1].block_area_parm_1 * sqrt(2)) / 2.0 + 0.5) {
		//--------------------
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
		step_vector.x = Me.pos.x - barrel_vpos.x;
		step_vector.y = Me.pos.y - barrel_vpos.y;
		vec_len = vect_len(step_vector);

		step_vector.x *= ((obstacle_map[ISO_BARREL_1].block_area_parm_1 * sqrt(2)) / 2.0 + 0.05) / vec_len;
		step_vector.y *= ((obstacle_map[ISO_BARREL_1].block_area_parm_1 * sqrt(2)) / 2.0 + 0.05) / vec_len;

		for (i = 0; i < 8; i++) {
			if (DirectLineColldet(Me.pos.x, Me.pos.y, barrel_vpos.x, barrel_vpos.y, Me.pos.z, &WalkablePassFilter)) {
				//--------------------
				// The obstacle plus the step vector give us the position to move the
				// Tux to for the optimal strike...
				// This position has to be defined relatively to the barrel's level, so that we
				// can retrieve the barrel later (at the end the combo action)
				//
				Me.mouse_move_target.x = step_vector.x + barrel_lvl->obstacle_list[barrel_index].pos.x;
				Me.mouse_move_target.y = step_vector.y + barrel_lvl->obstacle_list[barrel_index].pos.y;
				Me.mouse_move_target.z = barrel_lvl->obstacle_list[barrel_index].pos.z;

				//--------------------
				// We set up the combo_action, so that the barrel can be smashed later...
				//
				enemy_set_reference(&Me.current_enemy_target_n, &Me.current_enemy_target_addr, NULL);
				Me.mouse_move_target_combo_action_type = COMBO_ACTION_SMASH_BARREL;
				Me.mouse_move_target_combo_action_parameter = barrel_index;
				return;
			}
			//--------------------
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
		step_vector.x = Me.pos.x - barrel_vpos.x;
		step_vector.y = Me.pos.y - barrel_vpos.y;
		vec_len = vect_len(step_vector);
		step_vector.x /= vec_len;
		step_vector.y /= vec_len;

		// point_near_barrel is a point very near the barrel in the step_vector's direction,
		// point_away_from_barrel is a point in the same direction but a bit farther
		moderately_finepoint point_near_barrel, point_away_from_barrel;

		// half-size of the barrel
		int barrel_type = barrel_lvl->obstacle_list[barrel_index].type;
		moderately_finepoint half_size = { (obstacle_map[barrel_type].block_area_parm_1 * sqrt(2)) / 2.0,
			(obstacle_map[barrel_type].block_area_parm_2 * sqrt(2)) / 2.0
		};

		for (i = 0; i < 8; i++) {
			// (no need to optimize this, the compiler will do its job...)
			point_near_barrel.x = barrel_vpos.x + step_vector.x * (half_size.x + 0.05);
			point_near_barrel.y = barrel_vpos.y + step_vector.y * (half_size.y + 0.05);
			point_away_from_barrel.x = barrel_vpos.x + step_vector.x * (half_size.x + 0.2);
			point_away_from_barrel.y = barrel_vpos.y + step_vector.y * (half_size.y + 0.2);

			// check if the line from point_near_barrel to point_away_from_barrel is walkable
			if (DirectLineColldet(point_near_barrel.x, point_near_barrel.y,
					      point_away_from_barrel.x, point_away_from_barrel.y, Me.pos.z, &WalkablePassFilter)) {
				//--------------------
				// point_to_barrel seems good, move Tux there
				// This position has to be defined relatively to the barrel's level, so that we
				// can retrieve the barrel later (at the end the combo action)
				//
				Me.mouse_move_target.x =
				    barrel_lvl->obstacle_list[barrel_index].pos.x + step_vector.x * (half_size.x + 0.05);
				Me.mouse_move_target.y =
				    barrel_lvl->obstacle_list[barrel_index].pos.y + step_vector.y * (half_size.y + 0.05);
				Me.mouse_move_target.z = barrel_lvl->obstacle_list[barrel_index].pos.z;

				//--------------------
				// We set up the combo_action, so that the barrel can be smashed later, on the
				// second call (made by move_tux_thowards_intermediate_point)...
				//
				enemy_set_reference(&Me.current_enemy_target_n, &Me.current_enemy_target_addr, NULL);
				Me.mouse_move_target_combo_action_type = COMBO_ACTION_SMASH_BARREL;
				Me.mouse_move_target_combo_action_parameter = barrel_index;

				return;
			}
			// rotate the step_vector to check an other point
			RotateVectorByAngle(&step_vector, 45.0);
		}

		return;
	}

	//===================
	// The character is near enough from the barrel to smash it.
	//

	//--------------------
	// Before the barrel can get destroyed and we loose the position information,
	// we record the vector of the charecter's strike direction...
	//
	step_vector.x = -Me.pos.x + barrel_vpos.x;
	step_vector.y = -Me.pos.y + barrel_vpos.y;

	//--------------------
	// Check that the barrel is really reachable, and not behind an obstacle
	//
	colldet_filter filter = ObstacleByIdPassFilter;
	filter.data = &barrel_index;

	if (DirectLineColldet(Me.pos.x, Me.pos.y, barrel_vpos.x, barrel_vpos.y, Me.pos.z, &filter)) {
		//--------------------
		// We make sure the barrel gets smashed, even if the strike made by the
		// Tux would be otherwise a miss...
		//
		smash_obstacle(barrel_lvl->obstacle_list[barrel_index].pos.x,
			       barrel_lvl->obstacle_list[barrel_index].pos.y, barrel_lvl->obstacle_list[barrel_index].pos.z);

		//--------------------
		// We start an attack motion...
		// Since the character is just aside the barrel, we use a melee shot,
		// in order to avoid the lost of an ammunition.
		//
		int store_weapon_type = Me.weapon_item.type;
		Me.weapon_item.type = -1;
		tux_wants_to_attack_now(FALSE);
		Me.weapon_item.type = store_weapon_type;

		//--------------------
		// We set a direction of facing directly thowards the barrel in question
		// so that the strike motion looks authentic...
		//
		Me.angle = -(atan2(step_vector.y, step_vector.x) * 180 / M_PI - 180 - 45);
		Me.angle += 360 / (2 * MAX_TUX_DIRECTIONS);
		while (Me.angle < 0)
			Me.angle += 360;
	}

	Me.mouse_move_target_combo_action_type = NO_COMBO_ACTION_SET;
	Me.mouse_move_target_combo_action_parameter = (-1);
	DebugPrintf(2, "\ncheck_for_barrels_to_smash(...):  combo_action now unset.");
}

/**
 * This function picks an item, if Tux is near enough to pick it.
 * If Tux is too far, a combo action will be started, to first move Tux near
 * the item.
 */
void check_for_items_to_pickup(level *item_lvl, int item_index)
{
	gps item_vpos;

	if (item_lvl != NULL && item_index != -1) {

		update_virtual_position(&item_vpos, &item_lvl->ItemList[item_index].pos, Me.pos.z);

		//--------------------
		// We only take the item directly into our 'hand' i.e. the mouse cursor,
		// if the item in question can be reached directly and isn't blocked by
		// some walls or something...
		//
		if ((calc_euklid_distance(Me.pos.x, Me.pos.y, item_vpos.x, item_vpos.y) < ITEM_TAKE_DIST)
		    && DirectLineColldet(Me.pos.x, Me.pos.y, item_vpos.x, item_vpos.y, Me.pos.z, NULL)) {
			if (GameConfig.Inventory_Visible == FALSE
			    || MatchItemWithName(item_lvl->ItemList[item_index].type, "Valuable Circuits")) {
				AddFloorItemDirectlyToInventory(&(item_lvl->ItemList[item_index]));
				return;
			} else {
				/* Handled in HandleInventoryScreen(). Dirty and I don't plan on changing that right now.
				 * A.H., 2008-04-06 */
				;
			}
		} else {
			Me.mouse_move_target.x = item_lvl->ItemList[item_index].pos.x;
			Me.mouse_move_target.y = item_lvl->ItemList[item_index].pos.y;
			Me.mouse_move_target.z = item_lvl->levelnum;

			//--------------------
			// We set up the combo_action, so that the item can be picked up later...
			//
			enemy_set_reference(&Me.current_enemy_target_n, &Me.current_enemy_target_addr, NULL);
			Me.mouse_move_target_combo_action_type = COMBO_ACTION_PICK_UP_ITEM;
			Me.mouse_move_target_combo_action_parameter = item_index;
		}
	}
}

#undef _action_c
