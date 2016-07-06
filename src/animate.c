/*
 *
 *   Copyright (c) 1994, 2002, 2003 Johannes Prix
 *   Copyright (c) 1994, 2002 Reinhard Prix
 *   Copyright (c) 2004-2007 Arthur Huillet
 *   Copyright (c) 2009-2013 Samuel Degrande
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
 * Desc: Scenery (obstacle, floor tile and all other non-actor objects) animation.
 */

#define _animate_c 1

#include "defs.h"
#include "struct.h"
#include "global.h"
#include "proto.h"

struct animated_scenery_piece {
	void *scenery_piece;
	animation_fptr animation_fn;
	struct list_head node;
};

static struct list_head animated_floor_tile_list = LIST_HEAD_INIT(animated_floor_tile_list);
static int animated_floor_tiles_dirty_flag;

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

/*****************************************************************************
 * Animation callbacks for obstacles
 * 
 * Functions called to animate one specific type of animated obstacle
 *****************************************************************************/

/*
 * This function animates a door.
 * It opens and closes a door depending on the presence of characters near them
 */
static int animate_door(level* door_lvl, void *scenery_piece)
{
	struct obstacle *obs = (struct obstacle *)scenery_piece;
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
		xdist = Me.pos.x - obs->pos.x;
		ydist = Me.pos.y - obs->pos.y;
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
			xdist = fabsf(erot->pos.x - obs->pos.x);
			if (xdist < 2.0) {
				ydist = fabsf(erot->pos.y - obs->pos.y);
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
	Pos = &obs->type;

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
static int animate_autogun(level* autogun_lvl, void *scenery_piece)
{
	struct obstacle *obs = (struct obstacle *)scenery_piece;
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
		weapon_item_type = get_item_type_by_id("Laser pistol");
		bullet_image_type = ItemMap[weapon_item_type].weapon_bullet_type;	// which gun do we have ?
		BulletSpeed = ItemMap[weapon_item_type].weapon_bullet_speed;
		static_initialized = TRUE;
	}
	
	// Wait until it's time to fire a new bullet
	//
	if (autogun_timeline != 0.0)
		return TRUE;
	
	// Fire a new bullet
	//
	autogunx = obs->pos.x;
	autoguny = obs->pos.y;
	AutogunType = &obs->type;

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
	CurBullet->bullet_lifetime = ItemMap[weapon_item_type].weapon_bullet_lifetime;
	CurBullet->time_in_seconds = 0;
	CurBullet->pass_through_hit_bodies = ItemMap[weapon_item_type].weapon_bullet_pass_through_hit_bodies;
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
		error_message(__FUNCTION__, "\
There seems to be an autogun in the autogun list of this level, but it\n\
is not really an autogun.  Instead it's something else.", PLEASE_INFORM | IS_FATAL);
		break;
	}
	
	return TRUE;
}

/*
 * This function animates an animated obstacle.
 * It can be a 'frame animated' obstacle, i.e. an obstacle associated to
 * several images, and/or an obstacle emitting an animated glowing light.
 */
static int animate_obstacle(level *obstacle_lvl, void *scenery_piece)
{
	struct obstacle *obs = (struct obstacle *)scenery_piece;
	obstacle_spec *spec = get_obstacle_spec(obs->type);
	int num_frames = max(spec->filenames.size, spec->emitted_light_strength.size);
	obs->frame_index = (int)rintf(spec->animation_fps * animation_timeline) % num_frames;
	return TRUE;
}

/*****************************************************************************
 * Animation callbacks for floor tiles
 *
 * Functions called to animate one specific type of floor tile
 *****************************************************************************/

/*
 * This function animates a dynamic floor tile, defined by several images
 * displayed in turn.
 */
static int animate_floor_tile(struct level *lvl, void *scenery_piece)
{
	struct floor_tile_spec *floor_tile = (struct floor_tile_spec *)scenery_piece;

	int frame = (int)rintf(floor_tile->animation_fps * animation_timeline) % floor_tile->frames;
	floor_tile->current_image = &floor_tile->images[frame];
	return TRUE;
}

/*****************************************************************************
 * Handle lists of animated obstacles
 *****************************************************************************/

/*
 * Generate list of the animated obstacles for a given visible level to
 * speed-up things during animation and rendering.
 */
static void generate_animated_obstacle_list(struct visible_level *vis_lvl)
{
	int obstacle_index;
	level *Lev = vis_lvl->lvl_pointer;

	INIT_LIST_HEAD(&vis_lvl->animated_obstacles_list);

	/* Now browse obstacles and fill our list of animated obstacles. */
	for (obstacle_index = 0; obstacle_index < MAX_OBSTACLES_ON_MAP; obstacle_index++) {
		if (Lev->obstacle_list[obstacle_index].type == -1)
			continue;
		animation_fptr animation_fn = get_obstacle_spec(Lev->obstacle_list[obstacle_index].type)->animation_fn;
		if (animation_fn != NULL) {
			struct animated_scenery_piece *a = MyMalloc(sizeof(struct animated_scenery_piece));
			a->scenery_piece = (void *)&Lev->obstacle_list[obstacle_index];
			a->animation_fn = animation_fn;
			list_add(&a->node, &vis_lvl->animated_obstacles_list);
			continue;
		}
	}

	vis_lvl->animated_obstacles_dirty_flag = FALSE;
}

/**
 * This function marks the animated obstacle list of one visible_level
 * as being dirty, so that they will be re-generated later.
 */
void dirty_animated_obstacle_list(int lvl_num)
{
	struct visible_level *lvl;

	list_for_each_entry(lvl, &visible_level_list, node) {
		if (lvl->lvl_pointer->levelnum == lvl_num) {
			lvl->animated_obstacles_dirty_flag = TRUE;
			return;
		}
	}
}

/**
 * This function clean all the animated obstacle list of a given visible level
 */
void clear_animated_obstacle_list(struct visible_level *vis_lvl)
{
	struct animated_scenery_piece *a, *next;

	list_for_each_entry_safe(a, next, &vis_lvl->animated_obstacles_list, node) {
		list_del(&a->node);
		free(a);
	}

	vis_lvl->animated_obstacles_dirty_flag = TRUE;
}

/*****************************************************************************
 * Handle lists of animated floor tiles
 *****************************************************************************/

 /**
  * Generate a list of the animated floor tiles.
  */
static void generate_animated_floor_tile_list(void)
{
	int i;

	INIT_LIST_HEAD(&animated_floor_tile_list);

	/* Now browse floor tiles and fill our list of animated floor tiles. */
	for (i = 0; i < underlay_floor_tiles.size; i++) {
		struct floor_tile_spec *floor_tile = dynarray_member(&underlay_floor_tiles, i, sizeof(struct floor_tile_spec));
		floor_tile->current_image = &floor_tile->images[0];
		if (floor_tile->animation_fn != NULL) {
			struct animated_scenery_piece *a = MyMalloc(sizeof(struct animated_scenery_piece));
			a->scenery_piece = (void *)floor_tile;
			a->animation_fn = floor_tile->animation_fn;
			list_add(&a->node, &animated_floor_tile_list);
		}
	}
	for (i = 0; i < overlay_floor_tiles.size; i++) {
		struct floor_tile_spec *floor_tile = dynarray_member(&overlay_floor_tiles, i, sizeof(struct floor_tile_spec));
		floor_tile->current_image = &floor_tile->images[0];
		if (floor_tile->animation_fn != NULL) {
			struct animated_scenery_piece *a = MyMalloc(sizeof(struct animated_scenery_piece));
			a->scenery_piece = (void *)floor_tile;
			a->animation_fn = floor_tile->animation_fn;
			list_add(&a->node, &animated_floor_tile_list);
		}
	}

	animated_floor_tiles_dirty_flag = FALSE;
}

/**
 * This function clean the animated floor tiles list
 */
void clear_animated_floor_tile_list()
{
	struct animated_scenery_piece *a, *next;

	list_for_each_entry_safe(a, next, &animated_floor_tile_list, node) {
		list_del(&a->node);
		free(a);
	}
	INIT_LIST_HEAD(&animated_floor_tile_list);

	animated_floor_tiles_dirty_flag = TRUE;
}

/**
 * This function marks the animated floor tile list as being dirty, so that it
 * will be re-generated later.
 */
void dirty_animated_floor_tile_list()
{
	animated_floor_tiles_dirty_flag = TRUE;
}

/*****************************************************************************
 * Scenery animation external API
 *****************************************************************************/

/**
 * This functions returns a pointer to a scenery piece animation function for the
 * given animation name.
 */
animation_fptr get_animation_by_name(const char *animation_name)
{
	const struct {
		const char *name;
		animation_fptr animation;
	} animation_map[] = {
		{ "obstacle", animate_obstacle },
		{ "door", animate_door },
		{ "autogun", animate_autogun },
		{ "floor_tile", animate_floor_tile }
	};

	if (!animation_name)
		return NULL;

	int i;
	for (i = 0; i < sizeof(animation_map) / sizeof(animation_map[0]); i++) {
		if (!strcmp(animation_name, animation_map[i].name))
			return animation_map[i].animation;
	}

	error_message(__FUNCTION__, "\nUnknown scenery piece animation '%s'.", PLEASE_INFORM | IS_FATAL, animation_name);
	return NULL;
}

/**
 * Resets all animation timelines
 */
void animation_timeline_reset()
{
	autogun_timeline = 0.0;
	door_timeline = 0.0;
	animation_timeline = 0.0;
}

/**
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

/**
 * Call animation function on all animated scenery pieces
 */
void animate_scenery(void)
{
	struct animated_scenery_piece *a;
	struct visible_level *visible_lvl, *next_lvl;

	animation_timeline_advance();

	// Animated obstacles

	BROWSE_VISIBLE_LEVELS(visible_lvl, next_lvl) {
		// If animated_obstacles list is dirty, regenerate it
		if (visible_lvl->animated_obstacles_dirty_flag) {
			clear_animated_obstacle_list(visible_lvl);
			generate_animated_obstacle_list(visible_lvl);
		}
		// Call animation function of each animated object
		list_for_each_entry(a, &visible_lvl->animated_obstacles_list, node) {
			if (a->animation_fn != NULL) {
				a->animation_fn(visible_lvl->lvl_pointer, a->scenery_piece);
			}
		}
	}

	// Animated floor tiles

	if (animated_floor_tiles_dirty_flag) {
		clear_animated_floor_tile_list();
		generate_animated_floor_tile_list();
	}
	list_for_each_entry(a, &animated_floor_tile_list, node) {
		if (a->animation_fn != NULL) {
			a->animation_fn(NULL, a->scenery_piece);
		}
	}
}
