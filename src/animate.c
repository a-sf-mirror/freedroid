/*
 *
 *   Copyright (c) 1994, 2002, 2003 Johannes Prix
 *   Copyright (c) 1994, 2002 Reinhard Prix
 *   Copyright (c) 2004-2007 Arthur Huillet
 *   Copyright (c) 2009 Samuel Degrande
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
 * Desc: all animation of obstacles related functions.
 */

#define _animate_c

#include "defs.h"
#include "struct.h"
#include "global.h"
#include "proto.h"

//--------------------
// Distance, where door opens 
//
#define DOOROPENDIST2 (2.0)
#define DOOROPENDIST2_FOR_DROIDS (1.0)

//--------------------
// Animation timelines.
//
static float autogun_timeline = 0.0;
static float door_timeline = 0.0;
static float animation_timeline = 0.0;

void animation_timeline_reset()
{
	autogun_timeline = 0.0;
	door_timeline = 0.0;
	animation_timeline = 0.0;
}

/*
 * This function advances all animation timelines to their next value
 */
void animation_timeline_advance()
{
	autogun_timeline += Frame_Time();
	if (autogun_timeline >= 0.3)
		autogun_timeline = 0.0;
	door_timeline += Frame_Time();
	if (door_timeline >= 0.02)
		door_timeline = 0.0;
	animation_timeline += Frame_Time();
}

/*****************************************************************************
 * Animations callbacks
 * 
 * Functions called to animate one specific type of animated object
 *****************************************************************************/

/*
 * This function animates a door.
 * It opens and closes a door depending on the presence of characters near them
 */
int animate_door(level* door_lvl, int door_idx)
{
	float xdist, ydist;
	float dist2;
	int *Pos;
	int one_player_close_enough = FALSE;
	int some_bot_was_close_to_this_door = FALSE;

	if (door_timeline != 0.0)
		return TRUE;

	// First we see if the Tux is close enough to the door, so that it would get
	// opened.
	//
	if (Me.pos.z == door_lvl->levelnum) {
		xdist = Me.pos.x - door_lvl->obstacle_list[door_idx].pos.x;
		ydist = Me.pos.y - door_lvl->obstacle_list[door_idx].pos.y;
		dist2 = xdist * xdist + ydist * ydist;
		if (dist2 < DOOROPENDIST2) {
			one_player_close_enough = TRUE;
		}
	}
			
	// If the Tux is not close enough, then we must see if perhaps one of the 
	// enemies is close enough, so that the door would still get opened.
	//
	if (!one_player_close_enough) {

		enemy *erot;
		BROWSE_LEVEL_BOTS(erot, door_lvl->levelnum) {
			// We will only consider droids that are at least within a range of
			// say 2 squares in each direction.  Anything beyond that distance
			// can be safely ignored for this door.
			//
			xdist = abs(erot->pos.x - door_lvl->obstacle_list[door_idx].pos.x);
			if (xdist < 2.0) {
				ydist = abs(erot->pos.y - door_lvl->obstacle_list[door_idx].pos.y);
				if (ydist < 2.0) {
	
					// Now that we know, that there is some droid at least halfway
					// close to this door, we can start to go into more details and
					// compute the exact distance from the droid to the door.
					//
					dist2 = xdist * xdist + ydist * ydist;
					if (dist2 < DOOROPENDIST2_FOR_DROIDS) {
						some_bot_was_close_to_this_door = TRUE;
						break;
					}
	
				}	// ydist < 2.0
			}	// xdist < 2.0
	
		}	// bots
	}
	
	// Depending on the presence or not of someone near the door, open it or
	// close it.
	Pos = &(door_lvl->obstacle_list[door_idx].type);

	if (one_player_close_enough || some_bot_was_close_to_this_door) {
		if ( ((*Pos >= ISO_H_DOOR_000_OPEN) && (*Pos < ISO_H_DOOR_100_OPEN)) ||
		     ((*Pos >= ISO_V_DOOR_000_OPEN) && (*Pos < ISO_V_DOOR_100_OPEN)) ||
		     ((*Pos >= ISO_DH_DOOR_000_OPEN) && (*Pos < ISO_DH_DOOR_100_OPEN)) ||
		     ((*Pos >= ISO_DV_DOOR_000_OPEN) && (*Pos < ISO_DV_DOOR_100_OPEN)) ||
		     ((*Pos >= ISO_OUTER_DOOR_H_00) && (*Pos < ISO_OUTER_DOOR_H_100)) ||
		     ((*Pos >= ISO_OUTER_DOOR_V_00) && (*Pos < ISO_OUTER_DOOR_V_100)) )
			*Pos += 1;
	} else {
		if ( ((*Pos > ISO_H_DOOR_000_OPEN) && (*Pos <= ISO_H_DOOR_100_OPEN)) ||
		     ((*Pos > ISO_V_DOOR_000_OPEN) && (*Pos <= ISO_V_DOOR_100_OPEN)) ||
		     ((*Pos > ISO_DH_DOOR_000_OPEN) && (*Pos <= ISO_DH_DOOR_100_OPEN)) ||
		     ((*Pos > ISO_DV_DOOR_000_OPEN) && (*Pos <= ISO_DV_DOOR_100_OPEN)) ||
		     ((*Pos > ISO_OUTER_DOOR_H_00) && (*Pos <= ISO_OUTER_DOOR_H_100)) ||
		     ((*Pos > ISO_OUTER_DOOR_V_00) && (*Pos <= ISO_OUTER_DOOR_V_100)) )
			*Pos -= 1;	
	}

	return TRUE;
}

/*
 * This function animates an autogun.
 * It fires a bullet regularly 
 */
int animate_autogun(level* autogun_lvl, int autogun_idx)
{
	float autogunx, autoguny;
	int *AutogunType;
	int j = 0;
	bullet *CurBullet = NULL;

	// Initialization of the weapon item type associated to the autoguns
	//
	static int static_initialized = FALSE;
	static int weapon_item_type = -1;
	static int bullet_image_type = -1; 
	static float BulletSpeed = 0.0;
	
	if (!static_initialized) {
		weapon_item_type = GetItemIndexByName("Laser pistol");
		bullet_image_type = ItemMap[weapon_item_type].item_gun_bullet_image_type;	// which gun do we have ? 
		BulletSpeed = ItemMap[weapon_item_type].item_gun_speed;
		static_initialized = TRUE;
	}
	
	// Wait until it's time to fire a new bullet
	//
	if (autogun_timeline != 0.0)
		return TRUE;
	
	// Fire a new bullet
	//
	autogunx = (autogun_lvl->obstacle_list[autogun_idx].pos.x);
	autoguny = (autogun_lvl->obstacle_list[autogun_idx].pos.y);
	AutogunType = &(autogun_lvl->obstacle_list[autogun_idx].type);

	/* search for the next free bullet list entry */
	for (j = 0; j < (MAXBULLETS); j++) {
		if (AllBullets[j].type == INFOUT) {
			CurBullet = &AllBullets[j];
			break;
		}
	}

	/* didn't find any free bullet entry? --> take the first */
	if (CurBullet == NULL)
		CurBullet = &AllBullets[0];

	/* create a new bullet */
	CurBullet->type = bullet_image_type;
	CurBullet->damage = 5;
	CurBullet->faction = FACTION_BOTS;
	CurBullet->owner = -3;
	CurBullet->bullet_lifetime = ItemMap[weapon_item_type].item_gun_bullet_lifetime;
	CurBullet->time_in_seconds = 0;
	CurBullet->pass_through_hit_bodies = ItemMap[weapon_item_type].item_gun_bullet_pass_through_hit_bodies;
	CurBullet->freezing_level = 0;
	CurBullet->poison_duration = 0;
	CurBullet->poison_damage_per_sec = 0;
	CurBullet->paralysation_duration = 0;

	/* compute speed and initial position of the bullet */
	CurBullet->speed.x = 0.0;
	CurBullet->speed.y = 0.0;
	CurBullet->pos.x = autogunx;
	CurBullet->pos.y = autoguny;
	CurBullet->pos.z = autogun_lvl->levelnum;
	CurBullet->height = 20;

	switch (*AutogunType) {
	case ISO_AUTOGUN_W:
		CurBullet->speed.x = -BulletSpeed;
		CurBullet->pos.x -= 0.5;
		CurBullet->pos.y -= 0.25;
		CurBullet->angle = 180 + 45;
		break;
	case ISO_AUTOGUN_E:
		CurBullet->speed.x = BulletSpeed;
		CurBullet->pos.x += 0.4;
		CurBullet->angle = 45;
		break;
	case ISO_AUTOGUN_N:
		CurBullet->speed.y = -BulletSpeed;
		CurBullet->pos.x += -0.25;
		CurBullet->pos.y += -0.5;
		CurBullet->angle = -45;
		break;
	case ISO_AUTOGUN_S:
		CurBullet->speed.y = BulletSpeed;
		CurBullet->pos.y += 0.4;
		CurBullet->angle = 180 - 45;
		break;
	default:
		fprintf(stderr, "\n*AutogunType: '%d'.\n", *AutogunType);
		ErrorMessage(__FUNCTION__, "\
There seems to be an autogun in the autogun list of this level, but it\n\
is not really an autogun.  Instead it's something else.", PLEASE_INFORM, IS_FATAL);
		break;
	}
	
	return TRUE;
}

int animate_obstacle(level *obstacle_lvl, int obstacle_idx)
{
	obstacle *obstacle = &obstacle_lvl->obstacle_list[obstacle_idx];
	obstacle_spec *spec = get_obstacle_spec(obstacle->type);
	int num_frames = max(spec->filenames.size, spec->emitted_light_strength.size);
	obstacle->frame_index = (int)rintf(spec->animation_fps * animation_timeline) % num_frames;
	return TRUE;
}

/**
 * This functions returns a pointer to the obstacle animation function for the
 * given animation name.
 */
animation_fptr get_animation_by_name(const char *animation_name)
{
	const struct {
		const char *name;
		animation_fptr animation;
	} animation_map[] = {
		{ "door", animate_door },
		{ "autogun", animate_autogun }
	};

	if (!animation_name)
		return NULL;

	int i;
	for (i = 0; i < sizeof(animation_map) / sizeof(animation_map[0]); i++) {
		if (!strcmp(animation_name, animation_map[i].name))
			return animation_map[i].animation;
	}

	ErrorMessage(__FUNCTION__, "\nUnknown obstacle animation '%s'.\n", PLEASE_INFORM, IS_FATAL, animation_name);
	return NULL;
}
