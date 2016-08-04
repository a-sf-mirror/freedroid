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

#define _action_c 1

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

	// Step 1: randomly choose one of 16 main 22.5° directions around the chest
	float obs_diag = get_obstacle_spec(obst_level->obstacle_list[obst_index].type)->diaglength;
	offset_vector.x = obs_diag + 0.5;
	offset_vector.y = 0.0;
	RotateVectorByAngle(&offset_vector, (float)MyRandom(16) * 22.5);

	// Step 2: rotate offset_vector by 45° until an available start position is found

	// Filter by walkability, excluding chest
	colldet_filter item_filter = WalkableWithMarginPassFilter;
	item_filter.extra_margin = COLLDET_DROP_ITEM_MARGIN;

	colldet_filter chest_filter = ObstacleByIdPassFilter;
	chest_filter.data = &obst_index;
	chest_filter.next = &item_filter;

	// Filter by walkability
	colldet_filter tux_filter = WalkableWithMarginPassFilter;

	tries = 0;
	while ((!DirectLineColldet(obst_x, obst_y, obst_x + offset_vector.x, obst_y + offset_vector.y, Me.pos.z, &chest_filter) ||
			!DirectLineColldet(Me.pos.x, Me.pos.y, obst_x + offset_vector.x, obst_y + offset_vector.y, Me.pos.z, &tux_filter))
	       && (tries < 8))
	{
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
	while ((!DirectLineColldet(obst_x, obst_y, *item_x + trimmer_x, *item_y + trimmer_y, Me.pos.z, &chest_filter) ||
		!DirectLineColldet(Me.pos.x, Me.pos.y, *item_x + trimmer_x, *item_y + trimmer_y, Me.pos.z, &tux_filter))
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
	float item_x, item_y;
	int drop_count = 0;
	level *lvl = CURLEVEL();

	struct dynarray *item_list = get_obstacle_extension(CURLEVEL(), &(lvl->obstacle_list[obst_index]), OBSTACLE_EXTENSION_CHEST_ITEMS);

	play_open_chest_sound();

	if (item_list) {
		drop_count = item_list->size;
		int i;
		for (i = 0; i < drop_count; i++) {

			item *it = &((item *)item_list->arr)[i];
			
			if (it->type == -1)
				continue;

			find_dropable_position_near_chest(&item_x, &item_y, obst_index, lvl);
			drop_item(it, item_x, item_y, lvl->levelnum);
		}

		// Empty the item list
		dynarray_free(item_list);

		// Remove the chest items obstacle extension
		del_obstacle_extension(lvl, &(lvl->obstacle_list[obst_index]), OBSTACLE_EXTENSION_CHEST_ITEMS);
	}
	
	// If the chest was empty, maybe generate a random item to be dropped
	if (!drop_count) {
		find_dropable_position_near_chest(&item_x, &item_y, obst_index, lvl);
		drop_random_item(Me.pos.z, item_x, item_y, lvl->drop_class, FALSE);
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

	struct image *img = get_obstacle_image(lvl->obstacle_list[obst_index].type, lvl->obstacle_list[obst_index].frame_index);
	if (mouse_cursor_is_on_that_image(obs_vpos.x, obs_vpos.y, img))
		return TRUE;

	return FALSE;
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
int clickable_obstacle_below_mouse_cursor(level **obst_lvl, int clickable_only)
{
#define HOVER_CHECK_DIST 2
#define SIDE_LENGTH (HOVER_CHECK_DIST * 2 + 1)
	int return_index = -1, obst_index = 0;
	float obst_normal = 0, max_normal = 0;

	if(obst_lvl)
		*obst_lvl = NULL;

	// If the cursor is not inside the user rectangle, 
	// there is no obstacle below it.
	if (!MouseCursorIsInUserRect(GetMousePos_x(), GetMousePos_y()))
		return -1;

	// We find the position of the mouse cursor on the floor.
	struct finepoint map_position_of_mouse;
	map_position_of_mouse.x = translate_pixel_to_map_location((float)input_axis.x, (float)input_axis.y, TRUE);
	map_position_of_mouse.y = translate_pixel_to_map_location((float)input_axis.x, (float)input_axis.y, FALSE);

	gps mouse_target_vpos = { map_position_of_mouse.x, map_position_of_mouse.y, Me.pos.z };
	gps mouse_target_pos;
	if (!resolve_virtual_position(&mouse_target_pos, &mouse_target_vpos))
		return -1;
	if (!level_is_visible(mouse_target_pos.z))
		return -1;

	level *lvl = curShip.AllLevels[mouse_target_pos.z];

	// Iterate through a square area of tiles with sides HOVER_CHECK_DIST * 2 + 1
	// centered on the tile under the mouse
	int i;
	for (i = 0; i < pow(SIDE_LENGTH, 2); i++) {
		int y = i / SIDE_LENGTH - HOVER_CHECK_DIST - 1  + mouse_target_pos.y;
		int x = i % SIDE_LENGTH - HOVER_CHECK_DIST - 1  + mouse_target_pos.x;

		if (!pos_inside_level(x, y, lvl))
			continue;

		// Iterate through all candidate obstacles on this tile
		int j;
		for (j = 0; j < lvl->map[y][x].glued_obstacles.size; j++) {
			obst_index = ((int *)(lvl->map[y][x].glued_obstacles.arr))[j];
			obst_normal = lvl->obstacle_list[obst_index].pos.x + lvl->obstacle_list[obst_index].pos.y;

			if (obst_normal > max_normal &&
				(!clickable_only || (get_obstacle_spec(lvl->obstacle_list[obst_index].type)->flags & IS_CLICKABLE)) &&
				mouse_cursor_is_on_that_obstacle(lvl, obst_index)) {

				max_normal = obst_normal;
				return_index = obst_index;
				if (obst_lvl)
					*obst_lvl = lvl;
			}
		}
	}
	return return_index;
}

/**
 * In order to perform a click-action, tux must first move to the obstacle.
 * He can either approach it from a specific direction or any direction. This 
 * function handles the case where direction doesn't matter, like for barrels
 * and crates. Returns 1 if tux is close enough to the obstacle.
 */
static int reach_obstacle_from_any_direction(level *obst_lvl, int obst_index) {
	gps obst_vpos;
	obstacle_spec *obstacle_spec = get_obstacle_spec(obst_lvl->obstacle_list[obst_index].type);

	update_virtual_position(&obst_vpos, &(obst_lvl->obstacle_list[obst_index].pos), Me.pos.z);
	if (calc_distance(Me.pos.x, Me.pos.y, obst_vpos.x, obst_vpos.y)
		<= (obstacle_spec->block_area_parm_1 * sqrt(2)) / 2.0 + 0.5) {
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

	step_vector.x *= ((obstacle_spec->block_area_parm_1 * sqrt(2)) / 2.0 + 0.05) / vec_len;
	step_vector.y *= ((obstacle_spec->block_area_parm_1 * sqrt(2)) / 2.0 + 0.05) / vec_len;

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
	moderately_finepoint half_size = { (obstacle_spec->block_area_parm_1 * sqrt(2)) / 2.0,
		(obstacle_spec->block_area_parm_2 * sqrt(2)) / 2.0
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
 * terminals, and signs. Returns 1 if tux is close enough to the obstacle.
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

	// So here we know, that we must set the course towards the chest.  We
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

	obstacle_spec *spec = get_obstacle_spec(obst_type);
	switch (direction) {
	case EAST:
		Me.mouse_move_target.x += spec->block_area_parm_1;
		break;
	case SOUTH:
		Me.mouse_move_target.y += spec->block_area_parm_2;
		break;
	case WEST:
		Me.mouse_move_target.x -= spec->block_area_parm_1;
		break;
	case NORTH:
		Me.mouse_move_target.y -= spec->block_area_parm_2;
		break;
	default:
		error_message(__FUNCTION__, "Invalid direction!!", PLEASE_INFORM | IS_FATAL);
		break;
	}
    return 0;
};			  // int reach_obstacle_from_specific_direction(level **obst_lvl, int obst_index, int direction)

enum interactive_obstacle_type {
	ACT_CHEST,
	ACT_TERMINAL,
	ACT_SIGN,
	ACT_BARREL,
};

static int approach(int type)
{
	int i;

	const struct {
		int type;
		int direction;
	} lookup[] = {
			{ ISO_V_CHEST_CLOSED, EAST },
			{ ISO_E_CHEST2_CLOSED, EAST },
			{ ISO_H_CHEST_CLOSED, SOUTH },
			{ ISO_S_CHEST2_CLOSED, SOUTH },
			{ ISO_W_CHEST2_CLOSED, WEST },
			{ ISO_N_CHEST2_CLOSED, NORTH },
			{ ISO_CONSOLE_N, SOUTH },
			{ ISO_CONSOLE_S, NORTH },
			{ ISO_CONSOLE_E, WEST },
			{ ISO_CONSOLE_W, EAST },
		        { ISO_CONSOLE_SECURE_E, EAST },
		        { ISO_CONSOLE_SECURE_S, SOUTH },
		        { ISO_CONSOLE_SECURE_W, WEST },
        		{ ISO_CONSOLE_SECURE_N, NORTH },
			{ ISO_WALL_TERMINAL_S, SOUTH},
			{ ISO_WALL_TERMINAL_E, EAST},
			{ ISO_WALL_TERMINAL_N, NORTH},
			{ ISO_WALL_TERMINAL_W, WEST},
			{ ISO_SIGN_1, EAST },
			{ ISO_SIGN_1_FLASH, EAST },
			{ ISO_SIGN_2, SOUTH },
			{ ISO_SIGN_2_FLASH, SOUTH },
			{ ISO_SIGN_3, EAST },
			{ ISO_SIGN_3_FLASH, EAST },
			{ ISO_VENDING_MACHINE_1_E, EAST },
			{ ISO_VENDING_MACHINE_1_N, NORTH },
			{ ISO_VENDING_MACHINE_1_W, WEST },
			{ ISO_VENDING_MACHINE_1_S, SOUTH },
			{ ISO_VENDING_MACHINE_2_E, EAST },
			{ ISO_VENDING_MACHINE_2_N, NORTH },
			{ ISO_VENDING_MACHINE_2_W, WEST },
			{ ISO_VENDING_MACHINE_2_S, SOUTH },
			{ ISO_VENDING_MACHINE_3_E, EAST },
			{ ISO_VENDING_MACHINE_3_N, NORTH },
			{ ISO_VENDING_MACHINE_3_W, WEST },
			{ ISO_VENDING_MACHINE_3_S, SOUTH },
			{ ISO_BOOKSHELF_LOOTABLE_E, EAST },
			{ ISO_BOOKSHELF_LOOTABLE_S, SOUTH },
			{ ISO_BOOKSHELF_LOOTABLE_W, WEST },
			{ ISO_BOOKSHELF_LOOTABLE_N, NORTH },
			{ ISO_BOOKSHELF_LONG_LOOTABLE_E, EAST },
			{ ISO_BOOKSHELF_LONG_LOOTABLE_S, SOUTH },
			{ ISO_BOOKSHELF_LONG_LOOTABLE_W, WEST },
			{ ISO_BOOKSHELF_LONG_LOOTABLE_N, NORTH },
	};

	for (i = 0; i < sizeof(lookup)/sizeof(lookup[0]); i++) {
		if (lookup[i].type == type)
			return lookup[i].direction;
	}


	error_message(__FUNCTION__, "Tried to approach obstacle type %d, but this is not a known type.", PLEASE_INFORM, type);

	return UNDEFINED;
}

static void act_chest(level *l, obstacle *o)
{
	throw_out_all_chest_content(get_obstacle_index(l, o));

	// After emptying the chest we change it to its corresponding "open" type.
	// If such a type was not defined, the chest will stay "closed" and can be
	// looted indefinitely.
	int new_type = get_obstacle_spec(o->type)->result_type_after_looting;
	if (new_type != -1)
		o->type = new_type;
}

static void act_barrel(level *l, obstacle *o)
{
	smash_obstacle(o->pos.x, o->pos.y, o->pos.z);

	// Do an attack motion with bare hands
	int store_weapon_type = Me.weapon_item.type;
	Me.weapon_item.type = -1;
	tux_wants_to_attack_now(FALSE);
	Me.weapon_item.type = store_weapon_type;
}

static void act_terminal(level *l, obstacle *o)
{
	char *dialog = get_obstacle_extension(l, o, OBSTACLE_EXTENSION_DIALOGFILE);
	if (!dialog) {
		append_new_game_message(_("Colored patterns appear on the screen, but they do not look like any computer interface you have ever seen. Perhaps is this what they call a screen \"saver\"."));
		return;
	}

	enemy dummy_enemy;
	// There are currently two terminal "droids", with different in-dialog images.
	if ((o->type >= ISO_CONSOLE_SECURE_E) && (o->type <= ISO_CONSOLE_SECURE_N))
		dummy_enemy.type = get_droid_type("STM");
	else
		dummy_enemy.type = get_droid_type("TRM");
	enemy_reset(&dummy_enemy);
	dummy_enemy.dialog_section_name = dialog;
	dummy_enemy.will_rush_tux = FALSE;
	chat_with_droid(&dummy_enemy);
}

static void act_sign(level *l, obstacle *o)
{
	const char *message = get_obstacle_extension(l, o, OBSTACLE_EXTENSION_SIGNMESSAGE);
	if (!message) {
		message = _("There is nothing on this sign.");
	}

	append_new_game_message("%s", D_(message));
}

static void __obstacle_action(level *lvl, int index, enum interactive_obstacle_type act)
{
	gps vpos;
	int direction;
	obstacle *o;

	// Retrieve the obstacle
	if (index == -1)
		return;

	o = &lvl->obstacle_list[index];

	// Compute the position of the obtacle
	update_virtual_position(&vpos, &o->pos, Me.pos.z);
	if (vpos.z == -1)
		return;

	// Compute the approach direction
	switch (act) {
		case ACT_CHEST:
		case ACT_TERMINAL:
		case ACT_SIGN:
			direction = approach(o->type);
			break;
		default:
			direction = UNDEFINED; 
	}

	// Approach obstacle
	if (direction == UNDEFINED) {
		int reached = reach_obstacle_from_any_direction(lvl, index);
		
		if (!reached)
			return;
		
		// We set a direction of facing the obstacle
		// so that the further actions look authentic
		moderately_finepoint step_vector;
		step_vector.x = -Me.pos.x + vpos.x;
		step_vector.y = -Me.pos.y + vpos.y;
		Me.angle = -(atan2(step_vector.y, step_vector.x) * 180 / M_PI - 180 - 45);
		Me.angle += 360 / (2 * MAX_TUX_DIRECTIONS);
		while (Me.angle < 0)
			Me.angle += 360;

	} else {
		if (!reach_obstacle_from_specific_direction(lvl, index, direction)) 
			return;
	}

	// Check direct reachability
	colldet_filter filter = ObstacleByIdPassFilter;
	filter.data = &index;

	if (!DirectLineColldet(Me.pos.x, Me.pos.y, vpos.x, vpos.y, Me.pos.z, &filter))
		return;

	event_obstacle_action(o);

	// Do the specific action
	switch (act) {
		case ACT_CHEST:
			act_chest(lvl, o);
			break;
		case ACT_TERMINAL:
			act_terminal(lvl, o);
			break;
		case ACT_SIGN:
			act_sign(lvl, o);
			break;
		case ACT_BARREL:
			act_barrel(lvl, o);
			break;
	}
}

/**
 * This function executes a chest's action, if Tux is near enough to activate it.
 * If Tux is too far, a combo action will be started, to first move Tux near
 * the chest.
 */
void chest_open_action(level *chest_lvl, int chest_index)
{
	__obstacle_action(chest_lvl, chest_index, ACT_CHEST);
}

/**
 * This function executes a barrel's action, if Tux is near enough to activate it.
 * If Tux is too far, a combo action will be started, to first move Tux near
 * the barrel.
 */
void barrel_action(level *barrel_lvl, int barrel_index)
{
	__obstacle_action(barrel_lvl, barrel_index, ACT_BARREL);
}

/**
 * This function connects the player to an interactive computer terminal ingame.
 */
void terminal_connect_action(level *lvl, int terminal_index)
{
	__obstacle_action(lvl, terminal_index, ACT_TERMINAL);
}

void sign_read_action(level *lvl, int index)
{
	__obstacle_action(lvl, index, ACT_SIGN);
}

/**
 * \brief Check if Tux can pick up an item, and if not, let Tux reach the item.
 *
 * Tux can pick up an item if it is close enough and isn't blocked by any
 * obstacles (e.g. walls).
 *
 * If Tux is too far away, a combo action will be started, which will first make
 * Tux walk towards item.
 *
 * \param item_lvl Pointer to the item's level structure
 * \param item_index Index of the item to pick up
 *
 * \return TRUE if the item can be picked up without moving
 */
int check_for_items_to_pickup(level *item_lvl, int item_index)
{
	gps item_vpos;

	if (item_lvl == NULL || item_index == -1)
		return FALSE;

	update_virtual_position(&item_vpos, &item_lvl->ItemList[item_index].pos, Me.pos.z);

	if ((calc_distance(Me.pos.x, Me.pos.y, item_vpos.x, item_vpos.y) < ITEM_TAKE_DIST)
		&& DirectLineColldet(Me.pos.x, Me.pos.y, item_vpos.x, item_vpos.y, Me.pos.z, NULL))
	{
		return TRUE;
	}

	// Set up the combo_action
	Me.mouse_move_target.x = item_lvl->ItemList[item_index].pos.x;
	Me.mouse_move_target.y = item_lvl->ItemList[item_index].pos.y;
	Me.mouse_move_target.z = item_lvl->levelnum;

	enemy_set_reference(&Me.current_enemy_target_n, &Me.current_enemy_target_addr, NULL);
	Me.mouse_move_target_combo_action_type = COMBO_ACTION_PICK_UP_ITEM;
	Me.mouse_move_target_combo_action_parameter = item_index;

	return FALSE;
}

/**
 * This function returns a pointer to the obstacle action function for the given action name.
 * If action with the given name wasn't found it returns NULL.
 */
action_fptr get_action_by_name(const char *action_name)
{
	const struct {
		const char *name;
		action_fptr action;
	} action_map[] = {
		{ "barrel", barrel_action },
		{ "chest", chest_open_action },
		{ "terminal", terminal_connect_action },
		{ "sign", sign_read_action }
	};

	if (!action_name)
		return NULL;

	int i;
	for (i = 0; i < sizeof(action_map) / sizeof(action_map[0]); i++) {
		if (!strcmp(action_name, action_map[i].name))
			return action_map[i].action;
	}

	error_message(__FUNCTION__, "\nUnknown obstacle action '%s'.", PLEASE_INFORM | IS_FATAL, action_name);
	return NULL;
}

#undef _action_c
