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
 * This file contains all features, movement, firing, collision and
 * extras of the influencer.
 */

#define _influ_c

#include "system.h"

#include "defs.h"
#include "struct.h"
#include "global.h"
#include "proto.h"
#include "widgets/widgets.h"

#define BEST_MELEE_DISTANCE (0.8)
#define BEST_CHAT_DISTANCE (BEST_MELEE_DISTANCE+0.2)
#define DISTANCE_TOLERANCE (0.00002)

#define LEVEL_JUMP_DEBUG 1

static void CheckForTuxOutOfMap();
static void AnalyzePlayersMouseClick();

static int no_left_button_press_in_previous_analyze_mouse_click = FALSE;

/**
 *
 *
 */
float calc_distance(float pos1_x, float pos1_y, float pos2_x, float pos2_y)
{
	return sqrt((pos1_x - pos2_x) * (pos1_x - pos2_x) + (pos1_y - pos2_y) * (pos1_y - pos2_y));
};

/**
 *
 *
 */
float vect_len(moderately_finepoint our_vector)
{
	return (sqrt(powf(our_vector.x, 2) + powf(our_vector.y, 2)));
};				// float vect_len ( moderately_finepoint our_vector )

static float get_tux_running_speed(void)
{
	if (GameConfig.cheat_double_speed)
		return 2 * TUX_RUNNING_SPEED;

	return TUX_RUNNING_SPEED;
}

/**
 * This function adapts the influencers current speed to the maximal speed
 * possible for the influencer.
 *
 * This function also stops Tux from moving while fighting.
 */
static void limit_tux_speed()
{
	/* Stop all movement if Tux is currently inside attack animation.  This is
	 * to stop Tux from moving without his legs animating.
	 *
	 * NOTE REGARDING MELEE ATTACK.  Tux starts to swing before he has reached
	 * BEST_MELEE_DISTANCE.  If we stop before reaching this distance, Tux will
	 * continue to try to move to this distance between swings, which causes a
	 * jerking motion towards the enemy.  To stop this from happening, we force
	 * Tux NOT to stand still when he is first charging a new target.  He stands
	 * still ONLY when he changes target during a swing. */
	static enemy *previous_target;
	enemy *current_target = enemy_resolve_address(Me.current_enemy_target_n,
												  &Me.current_enemy_target_addr);

	if (Me.weapon_item.type >= 0) {
		int has_melee = ItemMap[Me.weapon_item.type].weapon_is_melee;
		if (Me.weapon_swing_time != -1 && (!has_melee
		   || (has_melee && (previous_target != current_target || current_target == NULL))))
		{
			Me.speed.x = 0;
			Me.speed.y = 0;
			return;
		} else {
			previous_target = current_target;
		}
	}

	/* Limit the speed when Tux is not attacking. */
	float speed = Me.speed.x * Me.speed.x + Me.speed.y * Me.speed.y;
	float running_speed = get_tux_running_speed();
	if (speed > running_speed * running_speed) {
		float ratio = (running_speed * running_speed) / speed;
		Me.speed.x *= ratio;
		Me.speed.y *= ratio;
	}
}

/**
 * When the player has requested an attack motion, we start the 
 * corresponding code, that should try to attack, if that's currently
 * possible.
 */
void tux_wants_to_attack_now(int use_mouse_cursor_for_targeting)
{
	// Maybe the player requested an attack before the reload/retract
	// phase is completed.  In that case, we don't attack.

	if (Me.busy_time > 0) {
		return;
	}

	// If the Tux has a weapon and this weapon requires some ammunition, then
	// we have to check for enough ammunition first...

	if (Me.weapon_item.type >= 0 && ItemMap[Me.weapon_item.type].weapon_ammo_type) {
		if (Me.weapon_item.ammo_clip <= 0) {
			// So no ammunition... We should say so and reload...
			No_Ammo_Sound();
			// TRANSLATORS: Console msg when a weapon is empty
			append_new_game_message(_("%s empty, reloading..."), D_(item_specs_get_name(Me.weapon_item.type)));
			TuxReloadWeapon();

			return;
		}
	}

	if (!perform_tux_attack(use_mouse_cursor_for_targeting)) {	// If attack has failed
		return;
	}

	// The weapon was used and therefore looses some of it's durability
	// Also if it uses ammunition, one charge is to be removed

	if (Me.weapon_item.type >= 0) {
		DamageWeapon(&(Me.weapon_item));
		if (ItemMap[Me.weapon_item.type].weapon_ammo_type) {
			Me.weapon_item.ammo_clip--;
		}
	}
}

/**
 * The Tux might have cross a level's boundary. In that case, we 
 * must move the Tux silently to the corresponding other level.
 */
void correct_tux_position_according_to_jump()
{
	gps old_mouse_move_target;
	gps oldpos = { Me.pos.x, Me.pos.y, Me.pos.z };
	gps newpos;

	// If current Tux position is inside current level, there's nothing to change
	//
	if (pos_inside_level(Me.pos.x, Me.pos.y, CURLEVEL()))
		return;

	// Else, try to retrieve the actual position
	//
	int pos_valid = resolve_virtual_position(&newpos, &oldpos);

	if (!pos_valid) {
		// We were not able to compute the actual position...
		CheckForTuxOutOfMap();
		return;
	}
	// Tux is on another level, teleport it
	// (note: Teleport() resets Me.mouse_move_target, so we have to restore it)
	//
	old_mouse_move_target.x = Me.mouse_move_target.x;
	old_mouse_move_target.y = Me.mouse_move_target.y;
	old_mouse_move_target.z = Me.mouse_move_target.z;

	Teleport(newpos.z, newpos.x, newpos.y, FALSE, FALSE);

	Me.mouse_move_target.x = old_mouse_move_target.x;
	Me.mouse_move_target.y = old_mouse_move_target.y;
	Me.mouse_move_target.z = old_mouse_move_target.z;

	// Update the mouse target position, if needed
	//
	// The purpose is to define mouse_move_target relatively to new Tux's level.
	// However, if Tux had to move from one level to one of its diagonal neighbor, it has
	// eventually not yet reach the final destination level.
	// So we first have to get the real mouse_target position, and then transform it into a
	// virtual position according to Tux's new level.
	//
	if (old_mouse_move_target.z != -1) {
		int rtn = resolve_virtual_position(&Me.mouse_move_target, &old_mouse_move_target);
		if (rtn)
			update_virtual_position(&Me.mouse_move_target, &Me.mouse_move_target, newpos.z);
		if (!rtn || (Me.mouse_move_target.x == -1)
				 || !SinglePointColldet(Me.mouse_move_target.x, Me.mouse_move_target.y, Me.mouse_move_target.z, NULL)) {
			Me.mouse_move_target.x = (-1);
			Me.mouse_move_target.y = (-1);
			Me.mouse_move_target.z = (-1);
		}
	}

	// Update the intermediate waypoints
	//
	// Intermediate waypoints are defined relatively to Tux's current level.
	// They thus also have to be updated<Fluzz>
	//
	int i;
	for (i = 0; i < MAX_INTERMEDIATE_WAYPOINTS_FOR_TUX; i++)
	{
		if (Me.next_intermediate_point[i].x == -1)
			break;

		gps old_point = { Me.next_intermediate_point[i].x, Me.next_intermediate_point[i].y, oldpos.z };
		gps new_point;

		int rtn = resolve_virtual_position(&new_point, &old_point);
		if (rtn)
			update_virtual_position(&new_point, &new_point, newpos.z);

		if (!rtn || new_point.x == -1) {
			clear_out_intermediate_points(&newpos, Me.next_intermediate_point, MAX_INTERMEDIATE_WAYPOINTS_FOR_TUX);
			break;
		}

		Me.next_intermediate_point[i].x = new_point.x;
		Me.next_intermediate_point[i].y = new_point.y;
	}

	// Even the Tux must not leave the map!  A sanity check is done
	// here...
	//
	CheckForTuxOutOfMap();

}				// correct_tux_position_according_to_jump ( )

/**
 * This function initializes the influencers position history, which is
 * a ring buffer and is needed for throwing the influencer back (only one
 * or two positions would be needed for that) and for influencers followers
 * to be able to track the influencers path (10000 or so positions are used
 * for that, and that's why it is a ring buffer).
 */
void InitInfluPositionHistory()
{
	int RingPosition;

	for (RingPosition = 0; RingPosition < MAX_INFLU_POSITION_HISTORY; RingPosition++) {
		Me.Position_History_Ring_Buffer[RingPosition].x = Me.pos.x;
		Me.Position_History_Ring_Buffer[RingPosition].y = Me.pos.y;
		Me.Position_History_Ring_Buffer[RingPosition].z = Me.pos.z;
	}
}				// void InitInfluPositionHistory( void )

float GetInfluPositionHistoryX(int HowLongPast)
{
	int RingPosition;

	HowLongPast >>= 1;

	RingPosition = Me.current_zero_ring_index - HowLongPast;

	RingPosition += MAX_INFLU_POSITION_HISTORY;	// We don't want any negative values, for safety

	RingPosition %= MAX_INFLU_POSITION_HISTORY;	// We do MODULO for the Ring buffer length 

	return Me.Position_History_Ring_Buffer[RingPosition].x;
}

float GetInfluPositionHistoryY(int HowLongPast)
{
	int RingPosition;

	HowLongPast >>= 1;
	RingPosition = Me.current_zero_ring_index - HowLongPast;

	RingPosition += MAX_INFLU_POSITION_HISTORY;	// We don't want any negative values, for safety

	RingPosition %= MAX_INFLU_POSITION_HISTORY;	// We do MODULO for the Ring buffer length 

	return Me.Position_History_Ring_Buffer[RingPosition].y;
}

float GetInfluPositionHistoryZ(int HowLongPast)
{
	int RingPosition;

	HowLongPast >>= 1;
	RingPosition = Me.current_zero_ring_index - HowLongPast;

	RingPosition += MAX_INFLU_POSITION_HISTORY;	// We don't want any negative values, for safety

	RingPosition %= MAX_INFLU_POSITION_HISTORY;	// We do MODULO for the Ring buffer length 

	return Me.Position_History_Ring_Buffer[RingPosition].z;
}

/**
 * This function should check if the Tux is still ok, i.e. if he is still
 * alive or if the death sequence should be initiated.
 */
void CheckIfCharacterIsStillOk()
{

	// Now we check if the main character is really still ok.
	//
	if (Me.energy <= 0) {
		ThouArtDefeated();

		DebugPrintf(1, "\n%s():  Alternate end of function reached.", __FUNCTION__);
		return;
	}

};				// void CheckIfCharacterIsStillOk ( )

/**
 * Even the Tux must not leave the map!  A sanity check is done here...
 */
static void CheckForTuxOutOfMap()
{
	level *MoveLevel = curShip.AllLevels[Me.pos.z];

	// Now perhaps the influencer is out of bounds, i.e. outside of the map.
	//
	if (!pos_inside_level(Me.pos.x, Me.pos.y, MoveLevel)) {
		fprintf(stderr, "\n\nplayer's last position: X=%f, Y=%f, Z=%d.\n", Me.pos.x, Me.pos.y, Me.pos.z);
		error_message(__FUNCTION__, "\
A player's Tux was found outside the map.\n\
This indicates either a bug in the FreedroidRPG code or\n\
a bug in the currently used map system of FreedroidRPG.", PLEASE_INFORM | IS_FATAL);
	}
};				// void CheckForTuxOutOfMap ( )

/**
 * If an enemy was specified as the mouse move target, this enemy will
 * maybe move here and there.  But this means that also the mouse move
 * target of the influencer must adapt, which is done in this function.
 */
void tux_get_move_target_and_attack(gps * movetgt)
{
	moderately_finepoint RemainingWay;
	float RemainingWayLength;

	// if there is a mouse move target, we are not going to move towards the enemy
	if (Me.mouse_move_target.x != (-1)) {
		// If a combo action is pending, the mouse_move_target was defined relatively
		// to the item's level. We need here to define the position relatively to Tux's level.
		update_virtual_position(movetgt, &Me.mouse_move_target, Me.pos.z);
		return;
	}

	enemy *t = enemy_resolve_address(Me.current_enemy_target_n, &Me.current_enemy_target_addr);

	if (!t || (t->energy <= 0))	//No enemy or dead enemy, remove enemy
	{
		enemy_set_reference(&Me.current_enemy_target_n, &Me.current_enemy_target_addr, NULL);
		movetgt->x = -1;
		movetgt->y = -1;
		movetgt->z = -1;
		return;
	}

	update_virtual_position(&t->virt_pos, &t->pos, Me.pos.z);

	// If we have a ranged weapon in hand, there is no need to approach the
	// enemy in question.  We just try to fire a shot, and return.
	//
	if (Me.weapon_item.type != (-1)) {
		if (!ItemMap[Me.weapon_item.type].weapon_is_melee) {	//ranged weapon
			if (!is_friendly(t->faction, FACTION_SELF))
				tux_wants_to_attack_now(FALSE);
			movetgt->x = -1;
			movetgt->y = -1;
			movetgt->z = -1;
			return;
		}
	}
	// Move to melee distance
	//
	RemainingWay.x = t->virt_pos.x - Me.pos.x;
	RemainingWay.y = t->virt_pos.y - Me.pos.y;

	RemainingWayLength = sqrt(RemainingWay.x * RemainingWay.x + RemainingWay.y * RemainingWay.y);

	if (RemainingWayLength > 0.05) {
		RemainingWay.x = (RemainingWay.x / RemainingWayLength) * (RemainingWayLength - (BEST_MELEE_DISTANCE - 0.1));
		RemainingWay.y = (RemainingWay.y / RemainingWayLength) * (RemainingWayLength - (BEST_MELEE_DISTANCE - 0.1));
	}

	if ((RemainingWayLength <= BEST_MELEE_DISTANCE * sqrt(2) + 0.01) && (!is_friendly(t->faction, FACTION_SELF))) {
		tux_wants_to_attack_now(FALSE);
	}
	// New move target.
	movetgt->x = Me.pos.x + RemainingWay.x;
	movetgt->y = Me.pos.y + RemainingWay.y;
	movetgt->z = Me.pos.z;

	return;
}				// void tux_get_move_target_and_attack( )

/**
 * Actually move Tux towards the target.
 */
static void move_tux_according_to_his_speed()
{
	
	float planned_step_x;
	float planned_step_y;

	// Now we move influence according to current speed.  But there has been a problem
	// reported from people, that the influencer would (*very* rarely) jump through walls
	// and even out of the ship.  This has *never* occurred on my fast machine.  Therefore
	// I assume that the problem is related to sometimes very low frame rates on these machines.
	// So, we do a sanity check not to make steps too big.
	//
	// And on machines with FPS << 20, it will certainly alter the game behavior, so people
	// should really start using a Pentium or better machine.
	//
	planned_step_x = Me.speed.x * Frame_Time();
	planned_step_y = Me.speed.y * Frame_Time();

	Me.pos.x += planned_step_x;
	Me.pos.y += planned_step_y;

	if (((int)GetInfluPositionHistoryX(0) != (int)Me.pos.x || ((int)GetInfluPositionHistoryY(0) != (int)Me.pos.y))) {
		event_position_changed(Me.pos, FALSE);
	}

	// If the Tux got stuck, i.e. if he got no speed at all and still is 
	// currently not in a 'passable' position, the fallback handling needs
	// to be applied to move the Tux out of the offending obstacle (i.e. 
	// simply away from the offending obstacles center)
	//
	if ((fabsf(Me.speed.x) < 0.1) && (fabsf(Me.speed.y) < 0.1)) {
		// So there is no speed, so we check for passability...
		//
		if (!SinglePointColldet(Me.pos.x, Me.pos.y, Me.pos.z, &WalkablePassFilter)) {
			// Now it's time to launch the stuck-fallback handling...
			//
			DebugPrintf(-3, "\nTux looks stuck...ESCAPING just for this frame...");
			float new_x = Me.pos.x;
			float new_y = Me.pos.y;
			int rtn = EscapeFromObstacle(&new_x, &new_y, Me.pos.z, &WalkablePassFilter);
			if (!rtn) {
				DebugPrintf(-3, "\nNo escape position found around Tux... Looking in position history...");
				// First : look for a suitable position in Tux's position_history
				int i;
				float old_x;
				float old_y;
				for (i = 1; i < 10; i++) {
					if (GetInfluPositionHistoryZ(10 * i) == Me.pos.z) {
						old_x = GetInfluPositionHistoryX(10 * i);
						old_y = GetInfluPositionHistoryY(10 * i);
						if (old_x != Me.pos.x && old_y != Me.pos.y
						    && SinglePointColldet(old_x, old_y, Me.pos.z, &WalkablePassFilter)) {
							// Found...
							Me.pos.x = old_x;
							Me.pos.y = old_y;
							break;
						}
					}
				}
				// If no luck, last fallback
				if (i == 10) {
					DebugPrintf(-3, "\nNo luck with position_history, last fallback...");
					// Get a random direction, and move by random length from 0.5 to 1.5.
					// With some luck, Tux will escape now, or in the future tries
					Me.pos.x += ((0.5 + (float)MyRandom(10) / 10.0) * (MyRandom(10) < 5)) ? 1 : -1;
					Me.pos.y += ((0.5 + (float)MyRandom(10) / 10.0) * (MyRandom(10) < 5)) ? 1 : -1;
				}
			} else {
				Me.pos.x = new_x;
				Me.pos.y = new_y;
			}
		}
	}
}

/**
 * This function contains dumb movement code that, without any checks nor
 * refinement, calculates the direction and speed in which Tux has to move to
 * reach the target position.
 *
 * Returns TRUE if the target has been sufficiently approximated.
 */
static int move_tux_towards_raw_position(float x, float y)
{
	moderately_finepoint RemainingWay;
	moderately_finepoint planned_step;
	float squared_length, length;

	if (Me.energy <= 0)
		return FALSE;

	RemainingWay.x = -Me.pos.x + x;
	RemainingWay.y = -Me.pos.y + y;

	squared_length = RemainingWay.x * RemainingWay.x + RemainingWay.y * RemainingWay.y;

	// Maybe the remaining way is VERY small!  Then we must not do
	// a division at all.  We also need not do any movement, so the
	// speed can be eliminated and we're done here.
	//
	if (squared_length < DIST_TO_INTERM_POINT * DIST_TO_INTERM_POINT) {
		Me.speed.x = 0;
		Me.speed.y = 0;
		return (TRUE);
	}
	// Now depending on whether the running key is pressed or not,
	// we have the Tux go on running speed or on walking speed.
	//
	length = sqrt(squared_length);

	if (Me.running_power <= 0) {
		Me.running_must_rest = TRUE;
	}

	if (Me.running_must_rest) {
		GameConfig.autorun_activated = 0;
		planned_step.x = RemainingWay.x * TUX_WALKING_SPEED / length;
		planned_step.y = RemainingWay.y * TUX_WALKING_SPEED / length;
	}

	if ((LeftCtrlPressed() || GameConfig.autorun_activated) &&
		!(LeftCtrlPressed() && GameConfig.autorun_activated) && (!Me.running_must_rest)) {
		float running_speed = get_tux_running_speed();
		planned_step.x = RemainingWay.x * running_speed / length;
		planned_step.y = RemainingWay.y * running_speed / length;
	} else {
		planned_step.x = RemainingWay.x * TUX_WALKING_SPEED / length;
		planned_step.y = RemainingWay.y * TUX_WALKING_SPEED / length;
	}

	// Now that the speed is set, we can start to make the step
	//
	Me.speed.x = planned_step.x;
	Me.speed.y = planned_step.y;
	Me.meters_traveled += 0.1;
	// If we might step over the target,
	// we reduce the speed.
	//
	if (fabsf(planned_step.x * Frame_Time()) >= fabsf(RemainingWay.x))
		Me.speed.x = RemainingWay.x / Frame_Time();
	if (fabsf(planned_step.y * Frame_Time()) >= fabsf(RemainingWay.y))
		Me.speed.y = RemainingWay.y / Frame_Time();

	// In case we have reached our target, we can remove this mouse_move_target again,
	// but also if we have been thrown onto a different level, we cancel our current
	// mouse move target...
	//
	if (((fabsf(RemainingWay.y) <= DISTANCE_TOLERANCE) && (fabsf(RemainingWay.x) <= DISTANCE_TOLERANCE))) {
		return (TRUE);
	}

	return (FALSE);
}

/**
 *
 *
 */
static void move_tux_towards_intermediate_point(void)
{
	int i;

	/* If we have no intermediate point, Tux has arrived at the target. */
	if (Me.next_intermediate_point[0].x == -1) {
		/* First, stop Tux. */
		Me.speed.x = 0;
		Me.speed.y = 0;

		/* We might have a combo_action, that can occur on the end of any
		 * course, like e.g. open a chest or pick up some item. */
		level *lvl = curShip.AllLevels[Me.mouse_move_target.z];
		switch (Me.mouse_move_target_combo_action_type) {
		case NO_COMBO_ACTION_SET:
			break;
		case COMBO_ACTION_OBSTACLE:
			get_obstacle_spec(lvl->obstacle_list[Me.mouse_move_target_combo_action_parameter].type)->action_fn(
                    lvl,
                    Me.mouse_move_target_combo_action_parameter);
			break;
		case COMBO_ACTION_PICK_UP_ITEM:
			// If Tux arrived at destination, pick up the item and give it to the player
			if (check_for_items_to_pickup(lvl, Me.mouse_move_target_combo_action_parameter)) {
				item *it = &lvl->ItemList[Me.mouse_move_target_combo_action_parameter];

				if (GameConfig.Inventory_Visible) {
					// Special case: when the inventory screen is open, and there
					// is no enough room to place the item, put it in player's hand
					if (!try_give_item(it)) {
						item_held_in_hand = it;
					}
				} else {
					// Put the item into player's inventory, or drop it on the floor
					// if there is no enough room.
					give_item(it);
				}
			}

			break;
		default:
			error_message(__FUNCTION__, "Unhandled combo action for intermediate course encountered!", PLEASE_INFORM | IS_FATAL);
			break;
		}

		Me.mouse_move_target.x = -1;
		Me.mouse_move_target.y = -1;
		Me.mouse_move_target.z = -1;
		Me.mouse_move_target_combo_action_type = NO_COMBO_ACTION_SET;
		return;
	}

	/* Stop Tux when he's close enough to an item to pick it up. */
	if (Me.mouse_move_target_combo_action_type == COMBO_ACTION_PICK_UP_ITEM) {
		float distance = calc_distance(Me.pos.x, Me.pos.y,
									   Me.mouse_move_target.x, Me.mouse_move_target.y);
		if (distance < ITEM_TAKE_DIST &&
			DirectLineColldet(Me.pos.x, Me.pos.y, Me.mouse_move_target.x,
							  Me.mouse_move_target.y, Me.pos.z, NULL))
		{
			for (i = 0; i < MAX_INTERMEDIATE_WAYPOINTS_FOR_TUX; i++) {
				Me.next_intermediate_point[i].x = -1;
				Me.next_intermediate_point[i].y = -1;
			}
			return;
		}
	}

	// Move Tux towards the next intermediate course point
	if (move_tux_towards_raw_position(Me.next_intermediate_point[0].x, Me.next_intermediate_point[0].y)) {
		DebugPrintf(DEBUG_TUX_PATHFINDING, "\nMOVING ON TO NEXT INTERMEDIATE WAYPOINT! ");
		for (i = 1; i < MAX_INTERMEDIATE_WAYPOINTS_FOR_TUX; i++) {
			Me.next_intermediate_point[i - 1].x = Me.next_intermediate_point[i].x;
			Me.next_intermediate_point[i - 1].y = Me.next_intermediate_point[i].y;
		}
	}
}

/**
 * This function moves the influencer, adjusts his speed according to
 * keys pressed and also adjusts his status and current "phase" of his 
 * rotation.
 */
void move_tux()
{
	static gps last_given_course_target = { -2, -2, -2 };

	// check, if the influencer is still ok
	CheckIfCharacterIsStillOk();

	// We store the influencers position for the history record and so that others
	// can follow his trail.
	//
	Me.current_zero_ring_index++;
	Me.current_zero_ring_index %= MAX_INFLU_POSITION_HISTORY;
	Me.Position_History_Ring_Buffer[Me.current_zero_ring_index].x = Me.pos.x;
	Me.Position_History_Ring_Buffer[Me.current_zero_ring_index].y = Me.pos.y;
	Me.Position_History_Ring_Buffer[Me.current_zero_ring_index].z = Me.pos.z;


	if (Me.paralyze_duration) {
		Me.speed.x = 0;
		Me.speed.y = 0;
		return;		//If tux is paralyzed, we do nothing more
	}
	// As a preparation for the later operations, we see if there is
	// a living droid set as a target, and if yes, we correct the move
	// target to something suiting that new droids position.
	//
	gps move_target;

	tux_get_move_target_and_attack(&move_target);

	if (move_target.x != -1) {
		// For optimisation purposes, we'll not do anything unless a new target
		// has been given.
		//
		if (!((fabsf(move_target.x - last_given_course_target.x) < 0.3) &&
		      (fabsf(move_target.y - last_given_course_target.y) < 0.3))) {
			freeway_context frw_ctx = { FALSE, {NULL, NULL} };
			pathfinder_context pf_ctx = { &WalkableWithMarginPassFilter, &frw_ctx };
			moderately_finepoint target_point = { move_target.x, move_target.y };
			if (!set_up_intermediate_course_between_positions
			    (&Me.pos, &target_point, &Me.next_intermediate_point[0], MAX_INTERMEDIATE_WAYPOINTS_FOR_TUX, &pf_ctx)) {
				// A path was not found.
				// If a combo action was set, halt it.
				if (Me.mouse_move_target_combo_action_type != NO_COMBO_ACTION_SET) {
					Me.mouse_move_target_combo_action_type = NO_COMBO_ACTION_SET;
					Me.mouse_move_target.x = Me.pos.x;
					Me.mouse_move_target.y = Me.pos.y;
					Me.mouse_move_target.z = Me.pos.z;
				}
			} else {
				last_given_course_target.x = move_target.x;
				last_given_course_target.y = move_target.y;
				last_given_course_target.z = move_target.z;
			}
		}
	}

	// If there is a mouse move target present, we move towards that.
	move_tux_towards_intermediate_point();

	// Perhaps the player has pressed the right mouse button, indicating the use
	// of the currently selected special function or spell.
	//
	HandleCurrentlyActivatedSkill();

	// Maybe we need to fire a bullet or set a new mouse move target
	// for the new move-to location
	// There are currently two different input systms in use - event based and state based.
	// In order to maintain compatibility between the two, a game_map widget is added on the
	// game main widget to detect how user input should be handled. Therefore, when the mouse
	// is over the game_map widget (the widget is not in its DEFAULT state), no further event handling
	// is done by the widget system and AnalyzePlayersMouseClick is called.
	if (game_map->state != DEFAULT)
		AnalyzePlayersMouseClick();

	if (MouseLeftPressed())
		no_left_button_press_in_previous_analyze_mouse_click = FALSE;
	else
		no_left_button_press_in_previous_analyze_mouse_click = TRUE;

	// During inventory operations, there should not be any (new) movement
	//
	if (item_held_in_hand != NULL) {
		Me.mouse_move_target.x = Me.pos.x;
		Me.mouse_move_target.y = Me.pos.y;
		Me.mouse_move_target.z = Me.pos.z;
		enemy_set_reference(&Me.current_enemy_target_n, &Me.current_enemy_target_addr, NULL);
		return;
	}

	limit_tux_speed();

	move_tux_according_to_his_speed();

	animate_tux();
}

/**
 * This function decrements Tux's health and increments the relevant statistic
 * variable.
 */
void hit_tux(float damage)
{
	if (Me.god_mode)
		return;

	if (Me.energy < 0)
		return;

	Me.energy -= damage;

	if (damage > Me.energy / 10)
		tux_scream_sound();
}


/**
 * This function computes to current animation keyframe of Tux (Me.phase).
 *
 * Depending of Tux's current state (attacking, standing, walking, running),
 * an animation is chosen, defined by a keyframed specification and an "animation
 * progress cursor". This cursor progresses from 0 to 1 (possibly looping for
 * some animations).
 *
 * The current keyframe is then: first_keyframe + cursor * nb_keyframes
 *
 * The way a cursor progresses from 0 to 1 depends on the kind of the animation.
 * For example, the attack animation is a duration-based animation, so its cursor
 * progresses as time progresses. The walk animation is based on the distance
 * covered by Tux, so its cursor is a function of distance.
 *
 * Note on walking/running animations:
 * Those animations are running in loop, and we can change from walk to run
 * in the middle of the animation. In order to have a seamless transition
 * between the animations, we do not reset the progress cursor's value.
 * So those animations share a common animation progress cursor (Me.walk_cycle_phase).
 */
void animate_tux()
{
#define STEP_TIME (0.28)
	static float step_countdown = 0;

	// If Tux is paralyzed, show him as standing still.
	if (Me.paralyze_duration) {
		Me.walk_cycle_phase = 0.0;
		Me.phase = tux_anim.standing_keyframe;

		return;
	}

	// Handle the case of Tux just getting hit
	//
	// Note: We do not yet have keyframes for such an animation, so those
	// commented lines are only a place-holder for future extension.

	/*
	if (Me.got_hit_time != -1) {
		Me.phase = .....
		return;
	}
	*/

	// Attack animation
	// (duration-based animation, no loop)

	if (Me.weapon_swing_time != -1) {
		if (tux_anim.attack.nb_keyframes != 0) {
			float progress_cursor = (float)Me.weapon_swing_time / tux_anim.attack.duration;
			Me.phase = tux_anim.attack.first_keyframe + progress_cursor * (float)(tux_anim.attack.nb_keyframes);

			// Clamp to maximum keyframe value
			if ((int)Me.phase > tux_anim.attack.last_keyframe) {
				Me.phase = tux_anim.attack.last_keyframe;
			}
		} else {
			Me.phase = 0;
		}

		// Reset walk/run animation progress cursor
		Me.walk_cycle_phase = 0.0;

		// Stop the time counter when the end of the animation is reached
		if (Me.weapon_swing_time > tux_anim.attack.duration)
			Me.weapon_swing_time = -1;

		return;
	}

	// Breathe animation (launched when Tux's speed is very low).
	// Currently, there is no such animation, so we only display Tux at its
	// standing position.

	if (fabsf(Me.speed.x) + fabsf(Me.speed.y) < 0.3) {
		Me.walk_cycle_phase = 0.0;
		Me.phase = tux_anim.standing_keyframe;

		return;
	}

	// Walk/run animation
	// (distance-based animation, loop)
	// The progress cursor is: distance_covered_by_tux / distance_covered_during_one_sequence
	//
	// There is no way to 'statically' compute the distance covered by Tux,
	// so it is computed 'dynamically', i.e. incrementally, by adding the
	// distance covered since last frame.
	// The progress cursor thus has to be stored (Me.walk_cycle_phase)

	/* Choose animation spec depending on Tux speed */
	struct distancebased_animation *anim_spec;

	float my_speed = sqrt(Me.speed.x * Me.speed.x + Me.speed.y * Me.speed.y);

	if (my_speed <= (TUX_WALKING_SPEED + TUX_RUNNING_SPEED) * 0.5) {
		anim_spec = &(tux_anim.walk);
	} else {
		anim_spec = &(tux_anim.run);
		step_countdown += Frame_Time();
		if (step_countdown > STEP_TIME) {
			play_sound_cached_v("effects/tux_footstep.ogg", 0.2);
			step_countdown -= STEP_TIME;
		}
	}

	if (anim_spec->nb_keyframes != 0) {
		/* Set the progress cursor */
		Me.walk_cycle_phase += (Frame_Time() * my_speed) / anim_spec->distance;

		/* Loop of progress cursor */
		while (Me.walk_cycle_phase > 1.0) {
			Me.walk_cycle_phase -= 1.0;
		}

		/* Set current animation keyframe */
		Me.phase = anim_spec->first_keyframe + Me.walk_cycle_phase * anim_spec->nb_keyframes;
		if ((int)Me.phase > anim_spec->last_keyframe) {
			Me.phase = anim_spec->last_keyframe;
		}
	} else {
		Me.walk_cycle_phase = 0.0;
		Me.phase = tux_anim.standing_keyframe;
	}
}

/**
 * This function creates several explosions around the location where the
 * influencer is (was) positioned.  It is used after the influencers 
 * death to make his death more spectacular.
 */
void start_tux_death_explosions(void)
{
	int i;
	int counter;

	DebugPrintf(1, "\n%s(): Real function call confirmed.", __FUNCTION__);

	// create a few shifted explosions...
	for (i = 0; i < 10; i++) {

		// find a free blast
		counter = 0;
		while (AllBlasts[counter++].type != INFOUT) ;
		counter -= 1;
		if (counter >= MAXBLASTS) {
			error_message(__FUNCTION__, "Ran out of blasts!!!", PLEASE_INFORM | IS_FATAL);
		}
		AllBlasts[counter].type = DROIDBLAST;
		AllBlasts[counter].pos.x = Me.pos.x - 0.125 / 2 + MyRandom(10) * 0.05;
		AllBlasts[counter].pos.y = Me.pos.y - 0.125 / 2 + MyRandom(10) * 0.05;
		AllBlasts[counter].phase = i;
	}

	DebugPrintf(1, "\n%s(): Usual end of function reached.", __FUNCTION__);

};				// void start_tux_death_explosions ( void )

/** 
 * This function opens a menu when tux dies, asking the
 * player if he/she wants to load latest or backup game, 
 * quit to main menu or quit the game.
 */
void do_death_menu()
{
        char *MenuTexts[100];
        int done = FALSE;
        int MenuPosition = 1;
        int i;

        game_status = INSIDE_MENU;

        input_handle();

        enum {
                LOAD_LATEST_POSITION = 1,
                LOAD_BACKUP_POSITION,
                QUIT_TO_MAIN_POSITION,
                QUIT_POSITION
        };

        while (!done) {
                i = 0;
                MenuTexts[i++] = _("Load Latest");
                MenuTexts[i++] = _("Load Backup");
                MenuTexts[i++] = _("Quit to Main Menu");
                MenuTexts[i++] = _("Exit FreedroidRPG");
                MenuTexts[i++] = "";

                MenuPosition = DoMenuSelection("", MenuTexts, 1, "--GAME_BACKGROUND--", Menu_BFont);

                switch (MenuPosition) {

                case LOAD_LATEST_POSITION:
                        LoadGame();
                        done = !done;
                        break;
                case LOAD_BACKUP_POSITION:
                        LoadBackupGame();
                        done = !done;
                        break;
                case QUIT_TO_MAIN_POSITION:
                        if (game_root_mode == ROOT_IS_GAME) {
                                GameOver = TRUE;
                        }
                        done = !done;
                        break;
                case QUIT_POSITION:
                        Terminate(EXIT_SUCCESS);
                        break;
                default:
                        break;
                }
	}
}

/**
 * This function checks if the influencer is currently colliding with an
 * enemy and throws him back in that case.
 */
void check_tux_enemy_collision(void)
{
	float xdist;
	float ydist;

	enemy *erot, *nerot;
	BROWSE_LEVEL_BOTS_SAFE(erot, nerot, Me.pos.z) {

		if (erot->type == (-1) || erot->pure_wait)
			continue;

		// We determine the distance and back out immediately if there
		// is still one whole square distance or even more...
		//
		xdist = Me.pos.x - erot->pos.x;
		if (abs(xdist) > 1)
			continue;
		ydist = Me.pos.y - erot->pos.y;
		if (abs(ydist) > 1)
			continue;

		// Now at this point we know, that we are pretty close.  It is time
		// to calculate the exact distance and to see if the exact distance
		// indicates a collision or not, in which case we can again back out
		//
		//
		if ((fabsf(xdist) >= 2.0 * 0.25) || (fabsf(ydist) >= 2.0 * 0.25))
			continue;

		erot->pure_wait = WAIT_COLLISION;

		short int swap = erot->nextwaypoint;
		erot->nextwaypoint = erot->lastwaypoint;
		erot->lastwaypoint = swap;

	}

};				// void check_tux_enemy_collision( void )

/**
 * This function checks if there is some living droid below the current
 * mouse cursor and returns the index number of this droid in the array.
 * 
 * Earlier we did this by computing the map location the mouse was pointing
 * to and using that for the computation of the distance to droid coordinates.
 * The problem with this method is, that some droids are very 'high' in
 * the sense that the graphics (e.g. 302 body) is very far away from the
 * 'foot' point, where the droid is in X-Y coordinates on the map.  Therefore
 * some correction has to be done to fix this.  We can't use the map position
 * of the mouse any more... except maybe to exclude some bots from the start.
 *
 */
enemy *GetLivingDroidBelowMouseCursor()
{
	gps mouse_vpos, mouse_pos;
	int RotationModel, RotationIndex;
	struct image *our_image;
	enemy *this_bot;

	// no interaction with the game when the world is frozen
	if (world_frozen())
		return NULL;

	mouse_vpos.x = translate_pixel_to_map_location((float)input_axis.x, (float)input_axis.y, TRUE);
	mouse_vpos.y = translate_pixel_to_map_location((float)input_axis.x, (float)input_axis.y, FALSE);
	mouse_vpos.z = Me.pos.z;

	// Find the actual level (and related position) where the mouse cursor is pointing at.
	if (!resolve_virtual_position(&mouse_pos, &mouse_vpos))
		return NULL;

	// Browse all bots on that actual level, to find if the mouse is hovering one of them
	BROWSE_LEVEL_BOTS(this_bot, mouse_pos.z) {
		if (fabsf(this_bot->pos.x - mouse_pos.x) >= 5.0)
			continue;
		if (fabsf(this_bot->pos.y - mouse_pos.y) >= 5.0)
			continue;

		// We properly set the direction this robot is facing.
		//
		RotationIndex = set_rotation_index_for_this_robot(this_bot);

		// We properly set the rotation model number for this robot, i.e.
		// which shape (like 302, 247 or proffa) to use for drawing this bot.
		//
		RotationModel = set_rotation_model_for_this_robot(this_bot);

		our_image = &(enemy_images[RotationModel][RotationIndex][(int)this_bot->animation_phase]);

		update_virtual_position(&this_bot->virt_pos, &this_bot->pos, Me.pos.z);
		if (mouse_cursor_is_on_that_image(this_bot->virt_pos.x, this_bot->virt_pos.y, our_image)) {
			return this_bot;
		}
	}

	return (NULL);

};				// int GetLivingDroidBelowMouseCursor ( )

/**
 * This function fires a bullet from the influencer in some direction,
 * no matter whether this is 'allowed' or not, not questioning anything
 * and SILENTLY TRUSTING THAT THIS TUX HAS A RANGED WEAPON EQUIPPED.
 */
void perform_tux_ranged_attack(short int weapon_type, bullet *bullet_parameters,
		                       moderately_finepoint target_location)
{
	// Standard ranged attack process is to fire a bullet in the direction
	// of the target, and have it advance step by step until it reaches 'something'.
	// The starting position of the bullet is the gun's muzzle (note that we do not
	// know exactly the length of a gun, so we will use an 'average' constant value of
	// 'muzzle_offset_factor').
	//
	// But, sometime the targeted object can be so near Tux that it is actually
	// between Tux and the gun's muzzle. In "real life", this should not even
	// create a ranged attack, but for simplicity we will fire a bullet in that case.
	// The bullet being already at its target position, we immediately call the code
	// that detects bullet collisions and create bullet's explosion and damages.
	// And only if no collision is detected do we fire the bullet.

	float muzzle_offset_factor = 0.5;

	// Search for the first free bullet entry in the bullet list and initialize
	// with default values

	int bullet_index = find_free_bullet_index();
	struct bullet *new_bullet = &(AllBullets[bullet_index]);
	if (bullet_parameters)
		memcpy(new_bullet, bullet_parameters, sizeof(struct bullet));
	else
		bullet_init_for_player(new_bullet, ItemMap[weapon_type].weapon_bullet_type, weapon_type);

	// Set up recharging time for the Tux...
	// The firewait time is modified by the ranged weapon skill

	Me.busy_time = ItemMap[weapon_type].weapon_attack_time;
	Me.busy_time *= RangedRechargeMultiplierTable[Me.ranged_weapon_skill];
	Me.busy_type = WEAPON_FIREWAIT;
	Me.weapon_swing_time = 0; // restart swing animation

	// First case: The target is too near for an actual shot.
	// We try an immediate hit. If nothing is hurt, or if the bullet traversed
	// the target, then we will fire a bullet using the 'second case'.

	float const dist_epsilon = 0.05;

	if ( (fabs(target_location.x - Me.pos.x) <= muzzle_offset_factor + dist_epsilon) &&
	     (fabs(target_location.y - Me.pos.y) <= muzzle_offset_factor + dist_epsilon) ) {

		new_bullet->pos.x = target_location.x;
		new_bullet->pos.y = target_location.y;
		CheckBulletCollisions(bullet_index);

		/* If the bullet exploded, we're done */
		if (new_bullet->type == INFOUT)
			return;
	}

	// Second case: The target is not near Tux (or a first-case bullet traversed
	// the target)
	// We fire a bullet into the direction of the target.

	// Compute bullet's attack vector.
	// This is a vector from Tux's position to the target's position.

	moderately_finepoint attack_vector = { target_location.x - Me.pos.x,
	                                       target_location.y - Me.pos.y };
	double attack_norm = sqrt(attack_vector.x * attack_vector.x + attack_vector.y *attack_vector.y);
	attack_vector.x /= attack_norm;
	attack_vector.y /= attack_norm;

	// The bullet starts from the muzzle's position.
	// As said in the heading comment, we do not have enough informations to
	// compute it, so we just use a small offset in the attack direction.

	moderately_finepoint muzzle_position = { Me.pos.x + muzzle_offset_factor * attack_vector.x,
	                                         Me.pos.y + muzzle_offset_factor * attack_vector.y };

	// Set the bullet parameters

	new_bullet->pos.x = muzzle_position.x;
	new_bullet->pos.y = muzzle_position.y;
	new_bullet->speed.x = attack_vector.x * ItemMap[weapon_type].weapon_bullet_speed;
	new_bullet->speed.y = attack_vector.y * ItemMap[weapon_type].weapon_bullet_speed;
	new_bullet->angle = -(atan2(attack_vector.y, attack_vector.x) * 180 / M_PI + 90 + 45);
}

/**
 * In some cases, the mouse button will be pressed, but still some signs
 * might tell us, that this mouse button press was not intended as a move
 * or fire command to the Tux.  This function checks for these cases.
 */
int ButtonPressWasNotMeantAsFire()
{
	// If the influencer is holding something from the inventory
	// menu via the mouse, also just return
	//
	if (item_held_in_hand != NULL)
		return (TRUE);
	if (timeout_from_item_drop > 0)
		return (TRUE);

	// Maybe the player just pressed the mouse button but INSIDE one of the character/skills/inventory
	// screens.  Then of course we will not interpret the intention to fire the weapon but rather 
	// return from here immediately.
	//
	if (MouseLeftPressed() &&
	    (GameConfig.Inventory_Visible || GameConfig.CharacterScreen_Visible || GameConfig.SkillScreen_Visible
	     || GameConfig.skill_explanation_screen_visible)
	    && !MouseCursorIsInUserRect(GetMousePos_x(), GetMousePos_y())) {
		DebugPrintf(0, "\nCursor outside user-rect:\n  User_Rect.x=%d, User_Rect.w=%d, User_Rect.y=%d, User_Rect.h=%d.",
			    User_Rect.x, User_Rect.w, User_Rect.y, User_Rect.h);
		DebugPrintf(0, "\nCursor position: X=%d, Y=%d.", input_axis.x, input_axis.y);
		return (TRUE);
	}

	return (FALSE);

};				// int ButtonPressWasNotMeantAsFire ( )

/**
 * At some point in the analysis of the users mouse click, we'll be 
 * certain, that a fireing/weapon swing was meant with the click.  Once
 * this is knows, this function can be called to do the mechanics of the
 * weapon use.
 *
 * Return 0 if attack failed.
 */
int perform_tux_attack(int use_mouse_cursor_for_targeting)
{
	moderately_finepoint target_location;
	float target_angle;

	// The attack target location can be the targeted enemy (if one was set), or an
	// enemy below the mouse cursor, or by default the current mouse position.
	// In case of an 'un-targeted' attack (A-pressed), the mouse position defines the
	// target location.

	if (!APressed()) {

		enemy *targeted_enemy = enemy_resolve_address(Me.current_enemy_target_n, &Me.current_enemy_target_addr);
		if (!targeted_enemy) {
			targeted_enemy = GetLivingDroidBelowMouseCursor();
		}
		if (targeted_enemy) {
			// Use enemy's position to compute target location
			update_virtual_position(&targeted_enemy->virt_pos, &targeted_enemy->pos, Me.pos.z);
			target_location.x = targeted_enemy->virt_pos.x;
			target_location.y = targeted_enemy->virt_pos.y;
		} else {
			// Use cursor position to compute target location
			target_location.x = translate_pixel_to_map_location((float)input_axis.x, (float)input_axis.y, TRUE);
			target_location.y = translate_pixel_to_map_location((float)input_axis.x, (float)input_axis.y, FALSE);
		}

	} else {

		// Un-targeted attack. Target location defined by the mouse position.
		target_location.x = translate_pixel_to_map_location((float)input_axis.x, (float)input_axis.y, TRUE);
		target_location.y = translate_pixel_to_map_location((float)input_axis.x, (float)input_axis.y, FALSE);
	}

	// Turn Tux to face its target

	target_angle = -(atan2(Me.pos.y - target_location.y, Me.pos.x - target_location.x) * 180/M_PI - 90 + 22.5);
	Me.angle = target_angle;

	/**
	 * First case: Attack with a melee weapon (or with fists).
	 */

	if (Me.weapon_item.type == -1 || ItemMap[Me.weapon_item.type].weapon_is_melee) {

		int hit_something = FALSE;

		// A melee attack is a swing gesture, the impact point is thus a bit in front of
		// Tux. We have no idea of the size of the weapon, so we use an 'average' constant
		// value of 0.8 (impact_offset_y).

		const float impact_offset_y = 0.8;

		moderately_finepoint impact_point = { 0, -impact_offset_y };
		RotateVectorByAngle(&impact_point, Me.angle);
		impact_point.x += Me.pos.x;
		impact_point.y += Me.pos.y;

		// Find all enemies which are in a small area around the impact point
		// and setup up a melee shot.
		// TODO: also look for enemies on neighbor levels.

		const float impact_area_size = 0.5;

		enemy *erot, *nerot;
		BROWSE_LEVEL_BOTS_SAFE(erot, nerot, Me.pos.z) {
			if ( (fabsf(erot->pos.x - impact_point.x) > impact_area_size) ||
			     (fabsf(erot->pos.y - impact_point.y) > impact_area_size) ||
			     !DirectLineColldet(Me.pos.x, Me.pos.y, erot->pos.x, erot->pos.y, Me.pos.z, NULL)
			   )
				continue;

			// Set up a melee attack
			int shot_index = find_free_melee_shot_index();
			melee_shot *new_shot = &(AllMeleeShots[shot_index]);

			new_shot->attack_target_type = ATTACK_TARGET_IS_ENEMY;
			new_shot->mine = TRUE;
			new_shot->bot_target_n = erot->id;
			new_shot->bot_target_addr = erot;
			new_shot->to_hit = Me.to_hit;
			new_shot->damage = Me.base_damage + MyRandom(Me.damage_modifier);
			new_shot->owner = -1;	//no "bot class number" owner
			new_shot->time_to_hit = tux_anim.attack.duration / 2;

			// Slow or paralyze enemies if the player has bonuses with those effects.
			erot->frozen += Me.slowing_melee_targets;
			erot->paralysation_duration_left += Me.paralyzing_melee_targets;

			hit_something = TRUE;
		}

		// Also, we should check if there was perhaps a chest or box
		// or something that can be smashed up, cause in this case, we
		// must open Pandora's box now.

		if (smash_obstacle(impact_point.x, impact_point.y, Me.pos.z))
			hit_something = TRUE;

		// Finally we add a new wait-counter, so that swings cannot be started
		// in too rapid succession. Adjust that delay to take player's skill
		// into account.

		Me.busy_type = WEAPON_FIREWAIT;
		Me.busy_time = 0.5; // default value
		if (Me.weapon_item.type != -1)
			Me.busy_time = ItemMap[Me.weapon_item.type].weapon_attack_time;
		Me.busy_time *= MeleeRechargeMultiplierTable[Me.melee_weapon_skill];
		Me.weapon_swing_time = 0; // restart swing animation

		// Play a sound feedback

		if (hit_something)
			play_melee_weapon_hit_something_sound();
		else
			play_melee_weapon_missed_sound(&Me.pos);

		return hit_something;
	}

	/**
	 * Second case: Attack with a ranged weapon.
	 * Note: we know that weapon_item.type != -1
	 */

	perform_tux_ranged_attack(Me.weapon_item.type, NULL, target_location);

	fire_bullet_sound(ItemMap[Me.weapon_item.type].weapon_bullet_type, &Me.pos);

	// We do not know if the bullet will hit something, but the bullet was fired,
	// so that's a success...

	return TRUE;
}

/**
 * Reload the ammo clip
 *
 *
 */

void TuxReloadWeapon()
{
	if (Me.weapon_item.type == -1)
		return;		// Do not reload Tux's fists.

	if (Me.paralyze_duration)
		return;		// Do not reload when paralyzed.

	if (ItemMap[Me.weapon_item.type].weapon_ammo_clip_size == Me.weapon_item.ammo_clip)
		return;		// Clip full, return without reloading.

	int ammo_type = get_item_type_by_id(ItemMap[Me.weapon_item.type].weapon_ammo_type);

	int count = CountItemtypeInInventory(ammo_type);
	if (count > ItemMap[Me.weapon_item.type].weapon_ammo_clip_size - Me.weapon_item.ammo_clip)
		count = ItemMap[Me.weapon_item.type].weapon_ammo_clip_size - Me.weapon_item.ammo_clip;

	if (!count)		//no ammo found, tell the player that he "has it in the baba"
	{
		No_Ammo_Sound();
		No_Ammo_Sound();
		// TRANSLATORS: Out of <ammo type>
		append_new_game_message(_("Out of [s]%s[v]!"), _(item_specs_get_name(ammo_type)));
		return;
	}
	int i;
	for (i = 0; i < count; i++)
		DeleteOneInventoryItemsOfType(ammo_type);
	Me.weapon_item.ammo_clip += count;
	Me.busy_time = ItemMap[Me.weapon_item.type].weapon_reloading_time;
	Me.busy_time *= RangedRechargeMultiplierTable[Me.ranged_weapon_skill];
	Me.busy_type = WEAPON_RELOAD;
}

/**
 * When the player has left-clicked into the game area (i.e. the isometric
 * display of the game world), we need to check if maybe the click was
 * targeted on a droid.  
 * In case that was so, we need to start a dialog or maybe launch an 
 * attack movement.
 */
void check_for_droids_to_attack_or_talk_with()
{
	/* NOTA : the call to GetLivingDroidBelowMouseCursor() does set the virt_pos attribute
	 * of the found droid to be the bot's position relatively to Tux current level
	 */
	enemy *droid_below_mouse_cursor = GetLivingDroidBelowMouseCursor();

	// Set a move action unless there's a droid below the cursor,
	// 'A' is pressed,
	// or the player has clicked to pickup an item and hasn't released the LMB
	if (droid_below_mouse_cursor == NULL && (!APressed()) &&
		(no_left_button_press_in_previous_analyze_mouse_click ||
		Me.mouse_move_target_combo_action_type != COMBO_ACTION_PICK_UP_ITEM)) {

		Me.mouse_move_target.x = translate_pixel_to_map_location(input_axis.x, input_axis.y, TRUE);
		Me.mouse_move_target.y = translate_pixel_to_map_location(input_axis.x, input_axis.y, FALSE);
		Me.mouse_move_target.z = Me.pos.z;
		if (!ShiftPressed()) {
			enemy_set_reference(&Me.current_enemy_target_n, &Me.current_enemy_target_addr, NULL);
		}

		return;
	} else if (APressed()) {
		tux_wants_to_attack_now(TRUE);
		return;
	}

	if (droid_below_mouse_cursor != NULL &&
	    DirectLineColldet(Me.pos.x, Me.pos.y, droid_below_mouse_cursor->virt_pos.x, droid_below_mouse_cursor->virt_pos.y, Me.pos.z,
			      &VisiblePassFilter)) {

		enemy_set_reference(&Me.current_enemy_target_n, &Me.current_enemy_target_addr, droid_below_mouse_cursor);

		enemy *e = enemy_resolve_address(Me.current_enemy_target_n, &Me.current_enemy_target_addr);
		if (is_friendly(e->faction, FACTION_SELF)) {
			if (no_left_button_press_in_previous_analyze_mouse_click) {
				chat_with_droid(enemy_resolve_address(Me.current_enemy_target_n, &Me.current_enemy_target_addr));
				enemy_set_reference(&Me.current_enemy_target_n, &Me.current_enemy_target_addr, NULL);
			}

			return;
		}

		if (!ShiftPressed()) {
			Me.mouse_move_target.x = -1;
			Me.mouse_move_target.y = -1;
		}

		if (Me.weapon_item.type >= 0) {
			if ((ItemMap[Me.weapon_item.type].weapon_is_melee) &&
			    (calc_distance(Me.pos.x, Me.pos.y, droid_below_mouse_cursor->virt_pos.x, droid_below_mouse_cursor->virt_pos.y)
			     > BEST_MELEE_DISTANCE + 0.1)) {

				return;
			}
		} else if (calc_distance(Me.pos.x, Me.pos.y, droid_below_mouse_cursor->virt_pos.x, droid_below_mouse_cursor->virt_pos.y)
			   > BEST_MELEE_DISTANCE + 0.1) {

			return;
		}
		// But if we're close enough or there is a ranged weapon in Tux hands,
		// then we can finally start the attack motion right away...
		//
		tux_wants_to_attack_now(TRUE);
	}
};				// void check_for_droids_to_attack ( ) 

/**
 * If the user clicked his mouse, this might have several reasons.  It 
 * might happen to open some windows, pick up some stuff, smash a box,
 * move somewhere or fire a shot or make a weapon swing.
 * 
 * Therefore it is not so easy to decide what to do upon a users mouse
 * click and so this function analyzes the situation and decides what to
 * do.
 */
static void AnalyzePlayersMouseClick()
{
	int tmp;

	// This flag avoids the mouse_move_target to change while the user presses
	// LMB to start a combo action.
	static int wait_mouseleft_release = FALSE;

	// No action is associated to MouseLeftRelease event or state.
	//

	if (!MouseLeftPressed()) {
		wait_mouseleft_release = FALSE;
		return;
	}

	if (ButtonPressWasNotMeantAsFire())
		return;
	if (no_left_button_press_in_previous_analyze_mouse_click) {
		level *obj_lvl = NULL;

		Me.mouse_move_target_combo_action_type = NO_COMBO_ACTION_SET;

		if ((tmp = clickable_obstacle_below_mouse_cursor(&obj_lvl)) != -1) {
			get_obstacle_spec(obj_lvl->obstacle_list[tmp].type)->action_fn(obj_lvl, tmp);
			if (Me.mouse_move_target_combo_action_type != NO_COMBO_ACTION_SET)
				wait_mouseleft_release = TRUE;
			return;
		}

		// If the inventory screen is open, let it manage any possibly picked item.
		// Else, if the player left-clicked on an item, check if the item can be
		// picked up. If so, get it and give it to the player.
		// Note: if the item is too far away from Tux, check_for_items_to_pickup()
		// creates a combo action to reach the item.
		if (GameConfig.Inventory_Visible == FALSE) {
			if ((tmp = get_floor_item_index_under_mouse_cursor(&obj_lvl)) != -1) {
				if (check_for_items_to_pickup(obj_lvl, tmp)) {
					// The item can be picked up immediately , so give it to the player
					give_item(&obj_lvl->ItemList[tmp]);
					wait_mouseleft_release = TRUE;
				}
				return;
			}
		}
	}
	// Just after the beginning of a combo action, and while LMB is
	// always pressed, mouse_move_target must not be changed (so that
	// the player's character will actually move to the combo action's target)

	if (!wait_mouseleft_release)
		check_for_droids_to_attack_or_talk_with();
}


/**
 *  Handles arrow key based movements
 */
void set_movement_with_keys(int move_x, int move_y)
{
	float floor_center = 0.5;
	int move_amplitude = 1 + GameConfig.autorun_activated;

	// Center the target waypoint, or establish a new target waypoint if there isn't one already
	if (Me.mouse_move_target.z != -1) {
		Me.mouse_move_target.x = floor(Me.mouse_move_target.x) + floor_center;
		Me.mouse_move_target.y = floor(Me.mouse_move_target.y) + floor_center;
	} else {
		Me.mouse_move_target.z = Me.pos.z;
		Me.mouse_move_target.x = floor(Me.pos.x) + floor_center;
		Me.mouse_move_target.y = floor(Me.pos.y) + floor_center;
	}

	//Restricts moving target to within 2 units from current position
	if (abs(Me.pos.x - (Me.mouse_move_target.x + move_x * move_amplitude)) <= 2 &&
			abs(Me.pos.y - (Me.mouse_move_target.y + move_y * move_amplitude)) <= 2) {
		// Determine move amount
		Me.mouse_move_target.x += move_x * move_amplitude;
		Me.mouse_move_target.y += move_y * move_amplitude;
	}
}

static void free_tux()
{
	int i;

	free(Me.character_name);
	Me.character_name = NULL;
	free(Me.savegame_version_string);
	Me.savegame_version_string = NULL;

	clear_tux_mission_info();

	// We mark all the big screen messages for this character
	// as out of date, so they can be overwritten with new 
	// messages...
	//
	Me.BigScreenMessageIndex = 0;
	for (i = 0; i < MAX_BIG_SCREEN_MESSAGES; i++) {
		if (Me.BigScreenMessage[i]) {
			free(Me.BigScreenMessage[i]);
			Me.BigScreenMessage[i] = NULL;
		}
	}

}

/**
 * Reset the data in struct tux, in order to start a new game/load a game.
 */
void init_tux()
{
	int i;

	free_tux();

	memset(&Me, 0, sizeof(struct tux));

	Me.current_game_date = 0.0;
	Me.power_bonus_end_date = -1;	// negative dates are always in the past...
	Me.dexterity_bonus_end_date = -1;

	Me.speed.x = 0;
	Me.speed.y = 0;

	Me.pos.x = 0;
	Me.pos.y = 0;
	Me.pos.z = -1;
	
	Me.mouse_move_target.x = -1;
	Me.mouse_move_target.y = -1;
	Me.mouse_move_target.z = -1;
	
	Me.teleport_anchor.x = 0;
	Me.teleport_anchor.y = 0;
	Me.teleport_anchor.z = -1;
	
	enemy_set_reference(&Me.current_enemy_target_n, &Me.current_enemy_target_addr, NULL);
	
	Me.god_mode = FALSE;
	
	Me.mouse_move_target_combo_action_type = NO_COMBO_ACTION_SET;
	Me.mouse_move_target_combo_action_parameter = -1;
	
	Me.map_maker_is_present = FALSE;

	Me.temperature = 0.0;

	Me.health_recovery_rate = 0.2;
	Me.cooling_rate = 0.2;
	
	Me.busy_time = 0;
	Me.busy_type = NONE;

	Me.phase = 0;
	Me.angle = 180;
	Me.walk_cycle_phase = 0;
	Me.weapon_swing_time = -1;
	Me.MissionTimeElapsed = 0;
	Me.got_hit_time = -1;

	Me.points_to_distribute = 0;
	
	// reset statistics	
	Me.meters_traveled = 0;
	for (i = 0; i < Number_Of_Droid_Types + 1; i++) {
		Me.destroyed_bots[i]    = 0;
		Me.damage_dealt[i]      = 0;
		Me.TakeoverSuccesses[i] = 0;
		Me.TakeoverFailures[i]  = 0;
	}

	for (i = 0; i < MAX_LEVELS; i++) {
		Me.HaveBeenToLevel[i] = FALSE;
		Me.time_since_last_visit_or_respawn[i] = -1;
	}

	Me.Experience = 0;
	Me.exp_level = 0;
	Me.Gold = 0;
	
	Me.readied_skill = 0;
	for (i = 0; i < number_of_skills; i++) {
		Me.skill_level[i] = SpellSkillMap[i].present_at_startup;
	}

	GameConfig.spell_level_visible = 0;

	Me.melee_weapon_skill = 0;
	Me.ranged_weapon_skill = 0;
	Me.spellcasting_skill = 0;

	Me.running_power_bonus = 0;

	for (i = 0; i < 10; i++) {
		Me.program_shortcuts[i] = -1;
	}

	Me.paralyze_duration = 0;
	Me.invisible_duration = 0;
	Me.nmap_duration = 0;

	Me.readied_skill = 0;

	Me.quest_browser_changed = 0;

	for (i = 0; i < MAX_ITEMS_IN_INVENTORY; i++) {
		init_item(&Me.Inventory[i]);
	}
	init_item(&Me.weapon_item);
	init_item(&Me.armour_item);
	init_item(&Me.shield_item);
	init_item(&Me.special_item);
	init_item(&Me.drive_item);
	item_held_in_hand = NULL;

	clear_out_intermediate_points(&Me.pos, Me.next_intermediate_point, MAX_INTERMEDIATE_WAYPOINTS_FOR_TUX);
	Me.next_intermediate_point[0].x = -1;
	Me.next_intermediate_point[0].y = -1;
	
	Me.TextToBeDisplayed = "";
	Me.TextVisibleTime = 0;

	Me.base_physique = 20;
	Me.base_strength = 10;
	Me.base_dexterity = 15;
	Me.base_cooling = 25;
	
	UpdateAllCharacterStats();
	
	Me.energy = Me.maxenergy;
	Me.running_power = Me.max_running_power;
}
#undef _influ_c
