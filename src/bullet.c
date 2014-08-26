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
 */

/**
 * Desc: all Bullet AND Blast - related functions.
 */

#define _bullet_c

#include "system.h"

#include "defs.h"
#include "struct.h"
#include "global.h"
#include "proto.h"

/* Distances for hitting a droid */
#define DROIDHITDIST  (0.25)
#define DROIDHITDIST2 (DROIDHITDIST*DROIDHITDIST)

/**
 *
 *
 */
static void move_this_bullet_and_check_its_collisions(int num)
{
	Bullet CurBullet = &(AllBullets[num]);
	moderately_finepoint bullet_step_vector;
	float whole_step_size;
	int i;
	float number_of_steps;

	// In case of a bullet, which is not a melee weapon, we just move
	// the bullets as specified in it's speed vector.  But of course we
	// must make several stops and check for collisions in case the 
	// planned step would be too big to crash into walls...
	//
	whole_step_size = max(fabsf(CurBullet->speed.x * Frame_Time()), fabsf(CurBullet->speed.y * Frame_Time()));

	// NOTE:  The number 0.25 here is the value of thickness of the collision
	// rectangle of a standard wall.  Since we might not have loaded all wall tiles
	// at every time of the game, also during game, guessing the minimum thickness
	// of walls at runtime is a bit hard and would be inconvenient and complicated,
	// so I leave this with the hard-coded constant for now...
	//
	number_of_steps = rintf(whole_step_size / 0.25) + 1;

	bullet_step_vector.x = 0.5 * CurBullet->speed.x * Frame_Time() / number_of_steps;
	bullet_step_vector.y = 0.5 * CurBullet->speed.y * Frame_Time() / number_of_steps;

	for (i = 0; i < number_of_steps; i++) {
		CurBullet->pos.x += bullet_step_vector.x;
		CurBullet->pos.y += bullet_step_vector.y;

		// The bullet could have traverse a level's boundaries, so
		// retrieve its new level and position, if possible
		int pos_valid = resolve_virtual_position(&CurBullet->pos, &CurBullet->pos);
		if (!pos_valid) {
			DebugPrintf(-1000, "\nBullet outside of map: pos.x=%f, pos.y=%f, pos.z=%d, type=%d\n",
				    CurBullet->pos.x, CurBullet->pos.y, CurBullet->pos.z, CurBullet->type);
			DeleteBullet(num, FALSE);
			return;
		}
		// If the bullet has reached an invisible level, remove it silently
		if (!level_is_visible(CurBullet->pos.z)) {
			DeleteBullet(num, FALSE);
			return;
		}
		// Check collision with the environment

		CheckBulletCollisions(num);
		if (CurBullet->type == INFOUT)
			return;
	}

}				// void move_this_bullet_and_check_its_collisions ( CurBullet )

/**
 * Whenever a new bullet is generated, we need to find a free index in 
 * the array of bullets.  This function automates the process and 
 * also is secure against too many bullets in the game (with a rather
 * ungraceful exit, but that event shouldn't ever occur in a normal game.
 */
int find_free_melee_shot_index(void)
{
	int j;

	for (j = 0; j < MAX_MELEE_SHOTS; j++) {
		if (AllMeleeShots[j].attack_target_type == ATTACK_TARGET_IS_NOTHING) {
			return (j);
		}
	}

	error_message(__FUNCTION__, "\
I seem to have run out of free melee shot entries.", PLEASE_INFORM);

	return (0);

};				// void find_free_bullet_entry_pointer ( void )

void delete_melee_shot(melee_shot * t)
{
	memset(t, 0, sizeof(melee_shot));
	t->attack_target_type = ATTACK_TARGET_IS_NOTHING;
}

/* ------------------------------------------------------------------
 * This function applies melee damage of all attacks that have taken
 * place in the previous cycle
 * ----------------------------------------------------------------- */
void DoMeleeDamage(void)
{
	int i;
	float latest_frame_time = Frame_Time();
	melee_shot *CurMelS;

	/* Browse all melee shots */
	for (i = 0; i < MAX_MELEE_SHOTS; i++) {
		CurMelS = &AllMeleeShots[i];

		if (CurMelS->attack_target_type == ATTACK_TARGET_IS_NOTHING || CurMelS->time_to_hit > 0) {
			CurMelS->time_to_hit -= latest_frame_time;
			continue;
		}

		if (CurMelS->attack_target_type == ATTACK_TARGET_IS_ENEMY) {
			/* Attack an enemy */
			enemy *tg = enemy_resolve_address(CurMelS->bot_target_n, &CurMelS->bot_target_addr);
			if (!tg) {
				error_message(__FUNCTION__,
					     "Melee shot was set to ATTACK_TARGET_IS_ENEMY but had no targeted enemy. Deleting.",
					     NO_REPORT);
				delete_melee_shot(CurMelS);
				continue;
			}

			if (tg->energy <= 0) {
				// our enemy is already dead ! 
				delete_melee_shot(CurMelS);
				continue;
			}

			if (MyRandom(100) < CurMelS->to_hit) {
				hit_enemy(tg, CurMelS->damage, CurMelS->mine ? 1 : 0, CurMelS->owner, CurMelS->mine ? 1 : 0);
			}

			delete_melee_shot(CurMelS);
			continue;

		}

		if (CurMelS->attack_target_type == ATTACK_TARGET_IS_PLAYER) {
			/* hit player */
			if (MyRandom(100) < CurMelS->to_hit) {
				float real_damage = CurMelS->damage * get_player_damage_factor();
				hit_tux(real_damage);
				DamageProtectiveEquipment();
			}
		}

		delete_melee_shot(CurMelS);
	}
}

/**
 * This function moves all the bullets according to their speeds and the
 * current frame rate of course.
 */
void MoveBullets(void)
{
	int i;
	Bullet CurBullet;

	// We move all the bullets
	//
	for (i = 0; i < MAXBULLETS; i++) {
		CurBullet = &AllBullets[i];
		// We need not move any bullets, that are INFOUT already...
		//
		if (CurBullet->type == INFOUT)
			continue;

		if (CurBullet->time_to_hide_still > 0)
			continue;

		move_this_bullet_and_check_its_collisions(i);

		// WARNING!  The bullet collision check might have deleted the bullet, so 
		//           maybe there's nothing sensible at the end of that 'CurBullet'
		//           pointer any more at this point.  So we check AGAIN for 'OUT'
		//           bullets, before we proceed with the safety out-of-map checks...
		//
		if (CurBullet->type == INFOUT)
			continue;

		// Maybe the bullet has a limited lifetime.  In that case we check if the
		// bullet has expired yet or not.
		//
		if ((CurBullet->bullet_lifetime != (-1)) && (CurBullet->time_in_seconds > CurBullet->bullet_lifetime)) {
			DeleteBullet(i, FALSE);
			continue;
		}
		CurBullet->time_in_seconds += Frame_Time();

	}			/* for */
}				// void MoveBullets(void)

/**
 * This function eliminates the bullet with the given number.  As an 
 * additional parameter you can specify if there should be a blast 
 * generated at the location where the bullet died (=TRUE) or not (=FALSE).
 */
void DeleteBullet(int Bulletnumber, int ShallWeStartABlast)
{
	Bullet CurBullet = &(AllBullets[Bulletnumber]);

	// At first we generate the blast at the collision spot of the bullet,
	// cause later, after the bullet is deleted, it will be hard to know
	// the correct location ;)
	//
	if (ShallWeStartABlast) {
		struct bulletspec *bullet_spec = dynarray_member(&bullet_specs, CurBullet->type, sizeof(struct bulletspec));
		StartBlast(CurBullet->pos.x, CurBullet->pos.y, CurBullet->pos.z, bullet_spec->blast_type, CurBullet->damage, CurBullet->faction, NULL);
	}

	CurBullet->type = INFOUT;
	CurBullet->time_in_seconds = 0;
	CurBullet->mine = FALSE;
	CurBullet->owner = -100;
	CurBullet->phase = 0;
	CurBullet->pos.x = 0;
	CurBullet->pos.y = 0;
	CurBullet->pos.z = -1;
	CurBullet->angle = 0;
}

/**
 * This function starts a blast (i.e. an explosion) at the given location
 * in the usual map coordinates of course.  The type of blast must also
 * be specified, where possible values are defined in defs.h as follows:
 *
 * BULLETBLAST = 0 , (explosion of a small bullet hitting the wall)
 * DROIDBLAST,       (explosion of a dying droid)
 * OWNBLAST          (not implemented)
 *
 * StartBlast will either use sound_name if passed, or if NULL will use
 * the default sound for the blast. 
 */
void StartBlast(float x, float y, int level, int type, int dmg, int faction, char *sound_name)
{
	int i;
	blast *NewBlast;

	// Resolve blast real position, if possible
	//
	gps blast_vpos = { x, y, level };
	gps blast_pos;

	if (!resolve_virtual_position(&blast_pos, &blast_vpos)) {
		// The blast position is nowhere....
		error_message(__FUNCTION__, "\
A BLAST VIRTUAL POSITION WAS FOUND TO BE INCONSISTENT.\n\
\n\
However, the error is not fatal and will be silently compensated for now.\n\
When reporting a problem to the FreedroidRPG developers, please note if this\n\
warning message was created prior to the error in your report.\n\
However, it should NOT cause any serious trouble for FreedroidRPG.", NO_REPORT);
		return;
	}
	// If the blast is not on a visible level, we do not take care of it
	if (!level_is_visible(blast_pos.z))
		return;

	// Maybe there is a box under the blast.  In this case, the box will
	// get smashed and perhaps an item will drop.
	// 
	smash_obstacle(blast_pos.x, blast_pos.y, blast_pos.z);

	// find out the position of the next free blast
	for (i = 0; i < MAXBLASTS; i++)
		if (AllBlasts[i].type == INFOUT)
			break;

	// didn't find any --> then take the first one
	if (i >= MAXBLASTS)
		i = 0;

	// get pointer to it: more comfortable 
	NewBlast = &(AllBlasts[i]);

	// create a blast at the specified x/y coordinates
	NewBlast->pos.x = blast_pos.x;
	NewBlast->pos.y = blast_pos.y;
	NewBlast->pos.z = blast_pos.z;

	NewBlast->type = type;
	NewBlast->phase = 0;

	NewBlast->damage_per_second = dmg;

	NewBlast->faction = faction;

	if(sound_name) {
		play_blast_sound(sound_name, &NewBlast->pos);
	} else {
		play_blast_sound(Blastmap[type].sound_file, &NewBlast->pos);
	}
}

/**
 * This function advances the different phases of an explosion according
 * to the current lifetime of each explosion (=blast).
 */
void animate_blasts(void)
{
	int i;
	blast *CurBlast = AllBlasts;

	for (i = 0; i < MAXBLASTS; i++, CurBlast++) {
		if (CurBlast->type != INFOUT) {

			// But maybe the blast is also outside the map already, which would
			// cause a SEGFAULT directly afterwards, when the map is queried.
			// Therefore we introduce some extra security here...
			//
			if (!pos_inside_level(CurBlast->pos.x, CurBlast->pos.y, curShip.AllLevels[CurBlast->pos.z])) {
				error_message(__FUNCTION__, "\
A BLAST WAS FOUND TO EXIST OUTSIDE THE BOUNDS OF THE MAP.\n\
This is an indication of an inconsistency in FreedroidRPG.\n\
\n\
However, the error is not fatal and will be silently compensated for now.\n\
When reporting a problem to the FreedroidRPG developers, please note if this\n\
warning message was created prior to the error in your report.\n\
However, it should NOT cause any serious trouble for FreedroidRPG.", NO_REPORT);
				CurBlast->pos.x = 0;
				CurBlast->pos.y = 0;
				CurBlast->pos.z = 0;
				DeleteBlast(i);
				continue;
			}
			
			if (Blastmap[CurBlast->type].do_damage)
				CheckBlastCollisions(i);

			// And now we advance the phase of the blast according to the
			// time that has passed since the last frame (approximately)
			//
			CurBlast->phase += Frame_Time() * Blastmap[CurBlast->type].phases / Blastmap[CurBlast->type].total_animation_time;

			// Maybe the blast has lived over his normal lifetime already.
			// Then of course it's time to delete the blast, which is done
			// here.
			//
			if ((int)floorf(CurBlast->phase) >= Blastmap[CurBlast->type].phases)
				DeleteBlast(i);
		}		/* if */
	}
};				// void animate_blasts( ... )

/**
 * This function deletes a single blast entry from the list of all blasts
 */
void DeleteBlast(int BlastNum)
{
	AllBlasts[BlastNum].phase = INFOUT;
	AllBlasts[BlastNum].type = INFOUT;
};				// void DeleteBlast( int BlastNum )

/**
 * This function advances the currently active spells.
 */
void MoveActiveSpells(void)
{
	int i;
	float PassedTime;
	float DistanceFromCenter;
	PassedTime = Frame_Time();
	int direction_index;
	moderately_finepoint Displacement;
	gps final_point;
	float Angle;

	for (i = 0; i < MAX_ACTIVE_SPELLS; i++) {
		// We can ignore all unused entries...
		//
		if (AllActiveSpells[i].img_type == (-1))
			continue;

		// All spells should count their lifetime...
		//
		AllActiveSpells[i].spell_age += PassedTime;

		// We hardcode a speed here
		AllActiveSpells[i].spell_radius += 5.0 * PassedTime;

		// We do some collision checking with the obstacles in each
		// 'active_direction' of the spell and deactivate those directions,
		// where some collision with solid material has happened.
		//
		for (direction_index = 0; direction_index < RADIAL_SPELL_DIRECTIONS; direction_index++) {
			if (AllActiveSpells[i].active_directions[direction_index] == FALSE)
				continue;

			Angle = 360.0 * (float)direction_index / RADIAL_SPELL_DIRECTIONS;
			Displacement.x = AllActiveSpells[i].spell_radius;
			Displacement.y = 0;
			RotateVectorByAngle(&Displacement, Angle);
			final_point.x = AllActiveSpells[i].spell_center.x + Displacement.x;
			final_point.y = AllActiveSpells[i].spell_center.y + Displacement.y;
			final_point.z = Me.pos.z;

			// This spell's fraction could have traverse a level's border, so we
			// need to retrieve its actual level, if possible
			gps final_vpoint;
			if (!resolve_virtual_position(&final_vpoint, &final_point)) {
				AllActiveSpells[i].active_directions[direction_index] = FALSE;
				continue;
			}
			// Just discard the spell's fraction if it is on an invisible level
			if (!level_is_visible(final_vpoint.z)) {
				AllActiveSpells[i].active_directions[direction_index] = FALSE;
				continue;
			}
			// if this spell's fraction collides an obstacle, discard it
			if (!SinglePointColldet(final_vpoint.x, final_vpoint.y, final_vpoint.z, &FlyablePassFilter))
				AllActiveSpells[i].active_directions[direction_index] = FALSE;
		}

		// Here we also do the spell damage application here
		//
		float minDist = (0.2 + AllActiveSpells[i].spell_radius) * (0.2 + AllActiveSpells[i].spell_radius);

		struct visible_level *visible_lvl, *n;
		enemy *erot, *nerot;
		BROWSE_NEARBY_VISIBLE_LEVELS(visible_lvl, n, minDist) {
			BROWSE_LEVEL_BOTS_SAFE(erot, nerot, visible_lvl->lvl_pointer->levelnum) {
				update_virtual_position(&erot->virt_pos, &erot->pos, Me.pos.z);
				DistanceFromCenter =
				    (AllActiveSpells[i].spell_center.x - erot->virt_pos.x) * (AllActiveSpells[i].spell_center.x -
											      erot->virt_pos.x) +
				    (AllActiveSpells[i].spell_center.y - erot->virt_pos.y) * (AllActiveSpells[i].spell_center.y -
											      erot->virt_pos.y);

				if (DistanceFromCenter < minDist) {
					if ((AllActiveSpells[i].hit_type == ATTACK_HIT_BOTS && Droidmap[erot->type].is_human) ||
					    (AllActiveSpells[i].hit_type == ATTACK_HIT_HUMANS && !Droidmap[erot->type].is_human))
						continue;

					// Let's see if that enemy has a direction, that is still
					// active for the spell. 
					// We get the angle in radians but with zero at the 'north' direction.
					// And we convert the angle to a normal direction index
					//
					Displacement.x = erot->virt_pos.x - AllActiveSpells[i].spell_center.x;
					Displacement.y = erot->virt_pos.y - AllActiveSpells[i].spell_center.y;
					if (Displacement.x <= 0.01 && Displacement.y <= 0.01) {
						// if enemy is very close, the Angle computation could be inaccurate,
						// so do not check if the spell is active or not
						direction_index = -1;
					} else {
						// nota : Y axis is toward down in fdrpg 
						Angle = atan2(-Displacement.y, Displacement.x);	// -M_PI <= Angle <= M_PI
						if (Angle < 0)
							Angle += 2 * M_PI;	// 0 <= Angle <= 2 * M_PI
						direction_index = (int)((Angle * RADIAL_SPELL_DIRECTIONS) / (2 * M_PI));
						// clamp direction_index to avoid any bug
						if (direction_index < 0)
							direction_index = 0;
						if (direction_index >= RADIAL_SPELL_DIRECTIONS)
							direction_index = RADIAL_SPELL_DIRECTIONS - 1;
					}

					if ((direction_index == -1) || AllActiveSpells[i].active_directions[direction_index]) {
						/* we hit the enemy. the owner is set to NULL because for now we assume it can only be the player. */
						hit_enemy(erot, AllActiveSpells[i].damage * Frame_Time(),
							  AllActiveSpells[i].mine ? 1 : 0 /*givexp */ , -1,
							  AllActiveSpells[i].mine ? 1 : 0);

						if (erot->poison_duration_left < AllActiveSpells[i].poison_duration)
							erot->poison_duration_left = AllActiveSpells[i].poison_duration;
						erot->poison_damage_per_sec = AllActiveSpells[i].damage;

						if (erot->frozen < AllActiveSpells[i].freeze_duration)
							erot->frozen = AllActiveSpells[i].freeze_duration;

						if (erot->paralysation_duration_left < AllActiveSpells[i].paralyze_duration)
							erot->paralysation_duration_left = AllActiveSpells[i].paralyze_duration;
					}
				}
			}
		}

		// Such a spell can not live for longer than 1.0 seconds, say
		//
		if (AllActiveSpells[i].spell_age >= 1.0)
			DeleteSpell(i);

	}

};				// void MoveActiveSpells( ... )

/**
 * This function deletes a single blast entry from the list of all blasts
 */
void DeleteSpell(int SpellNum)
{
	AllActiveSpells[SpellNum].img_type = (-1);
	AllActiveSpells[SpellNum].spell_age = 0;
};				// void DeleteSpell( int SpellNum )

/**
 *
 *
 */
void clear_active_spells(void)
{
	int i;

	for (i = 0; i < MAX_ACTIVE_SPELLS; i++) {
		DeleteSpell(i);
	}

};				// void clear_active_spells ( void )

/**
 *
 *
 */
void clear_active_bullets()
{
	int i;

	for (i = 0; i < MAXBLASTS; i++) {
		DeleteBlast(i);
	}
	for (i = 0; i < MAXBULLETS; i++) {
		DeleteBullet(i, FALSE);
	}
}				// void clear_active_bullets()

/**
 * Whenever a new bullet is generated, we need to find a free index in 
 * the array of bullets.  This function automates the process and 
 * also is secure against too many bullets in the game (with a rather
 * ungraceful exit, but that event shouldn't ever occur in a normal game.
 */
int find_free_bullet_index(void)
{
	int j;

	for (j = 0; j < MAXBULLETS; j++) {
		if (AllBullets[j].type == INFOUT) {
			return (j);
		}
	}

	// If this point is ever reached, there's a severe bug in here...
	//
	error_message(__FUNCTION__, "\
I seem to have run out of free bullet entries.  This can't normally happen.  --> some bug in here, oh no...", PLEASE_INFORM | IS_FATAL);

	return (-1);		// can't happen.  just to make compilers happy (no warnings)

};				// void find_free_bullet_entry_pointer ( void )

/**
 * \brief Initialize a bullet - Common part
 *
 * \param bullet            Pointer to the bullet struct to fill in
 * \param bullet_type       Type of the bullet (i.e. its graphical representation)
 * \param from_pos          Starting position of the bullet
 * \param height            Vertical offset applied when drawing the bullet, to simulate a shot starting from the gun's muzzle
 * \param weapon_item_type  Type of the weapon used to shoot the bullet (defines
 *                          the behavior of the bullet)
 */
static void _bullet_init(struct bullet *bullet, int bullet_type, gps *from_pos, int height, short int weapon_item_type)
{
	memset(bullet, 0, sizeof(struct bullet));

	// Fill in non-zero field
	bullet->type = bullet_type;
	bullet->pos.x = from_pos->x;
	bullet->pos.y = from_pos->y;
	bullet->pos.z = from_pos->z;
	bullet->height = height;

	// Bullet characteristics defined by the weapon used to fire the bullet.
	bullet->bullet_lifetime = ItemMap[weapon_item_type].weapon_bullet_lifetime;
	bullet->pass_through_hit_bodies = ItemMap[weapon_item_type].weapon_bullet_pass_through_hit_bodies;
}


/**
 * \brief Initialize a bullet, shot by the player
 *
 * \param bullet            Pointer to the bullet struct to fill in
 * \param bullet_type       Type of the bullet (i.e. its graphical representation)
 * \param weapon_item_type  Type of the weapon used to shoot the bullet (defines
 *                          the behavior of the bullet)
 */
void bullet_init_for_player(struct bullet *bullet, int bullet_type, short int weapon_item_type)
{
	// Initialize the characteristics that do not depend on the shooter
	_bullet_init(bullet, bullet_type, &Me.pos, tux_rendering.gun_muzzle_height, weapon_item_type);

	// The damage depends on the weapon's type + the player's stats.
	// It is computed and stored in the Me struct.
	bullet->damage = Me.base_damage + MyRandom(Me.damage_modifier);

	// Remember that the bullet was shot by Tux.
	bullet->mine = TRUE;
	bullet->owner = -1;
	bullet->faction = FACTION_SELF;
	bullet->time_to_hide_still = 0.0;
}

/**
 * \brief Initialize a bullet, shot by a bot
 *
 * \param bullet            Pointer to the bullet struct to fill in
 * \param bullet_type       Type of the bullet (i.e. its graphical representation)
 * \param weapon_item_type  Type of the weapon used to shoot the bullet (defines
 *                          the behavior of the bullet)
 * \param bot               Pointer to the bot that fired the bullet
 */
void bullet_init_for_enemy(struct bullet *bullet, int bullet_type, short int weapon_item_type, struct enemy *bot)
{
	// Initialize the characteristics that do not depend on the shooter
	_bullet_init(bullet, bullet_type, &(bot->virt_pos), Droidmap[bot->type].gun_muzzle_height, weapon_item_type);

	// The damage depends on the weapon's type only.
	bullet->damage = ItemMap[weapon_item_type].weapon_base_damage +
	                 MyRandom(ItemMap[weapon_item_type].weapon_damage_modifier);

	// Remember that the bullet was shot by this bot.
	bullet->owner = bot->id;
	bullet->faction = bot->faction;
}

/**
 *
 *
 */
void check_bullet_background_collisions(bullet * CurBullet, int num)
{
	// Check for collision with background
	struct colldet_filter filter = { &FlyablePassFilterCallback, NULL, 0.05, NULL };
	if (!SinglePointColldet(CurBullet->pos.x, CurBullet->pos.y, CurBullet->pos.z, &filter)) {
			DeleteBullet(num, TRUE);
	}
}

/**
 *
 *
 */
void apply_bullet_damage_to_player(int damage, int owner)
{
	float real_damage = damage * get_player_damage_factor();

	hit_tux(real_damage);           // loose some energy
	DamageProtectiveEquipment();    // chance worn items gets damaged on each hit
}

/**
 *
 *
 */
void check_bullet_player_collisions(bullet * CurBullet, int num)
{
	double xdist, ydist;

	// Of course only active player may be checked!
	//
	if (Me.energy <= 0)
		return;

	// A player is supposed not to hit himself with his bullets, so we may
	// check for that case as well....
	//
	if (CurBullet->mine)
		return;

	if (is_friendly(CurBullet->faction, FACTION_SELF))
		return;

	// Now we see if the distance to the bullet is as low as hitting
	// distance or not.
	//

	// We need compatible gps positions to compute a distance
	gps bullet_vpos;
	update_virtual_position(&bullet_vpos, &CurBullet->pos, Me.pos.z);
	if (bullet_vpos.x == -1)
		return;

	xdist = Me.pos.x - bullet_vpos.x;
	ydist = Me.pos.y - bullet_vpos.y;

	if ((xdist * xdist + ydist * ydist) < DROIDHITDIST2) {

		apply_bullet_damage_to_player(CurBullet->damage, CurBullet->owner);
		DeleteBullet(num, TRUE);	// we want a bullet-explosion
		return;		// This bullet was deleted and does not need to be processed any further...
	}
};				// check_bullet_player_collisions ( CurBullet , num )

/**
 *
 *
 */
void check_bullet_enemy_collisions(bullet * CurBullet, int num)
{
	double xdist, ydist;
	int level = CurBullet->pos.z;

	// Check for collision with enemys
	//
	enemy *ThisRobot, *nerot;
	BROWSE_LEVEL_BOTS_SAFE(ThisRobot, nerot, level) {
		// Check several hitting conditions
		//
		xdist = CurBullet->pos.x - ThisRobot->pos.x;
		ydist = CurBullet->pos.y - ThisRobot->pos.y;

		if ((xdist * xdist + ydist * ydist) >= DROIDHITDIST2)
			continue;

		if (CurBullet->hit_type == ATTACK_HIT_BOTS && Droidmap[ThisRobot->type].is_human)
			continue;
		if (CurBullet->hit_type == ATTACK_HIT_HUMANS && !Droidmap[ThisRobot->type].is_human)
			continue;
		if (is_friendly(ThisRobot->faction, CurBullet->faction))
			continue;

		// Hit the ennemy
		//
		hit_enemy(ThisRobot, CurBullet->damage, (CurBullet->mine ? 1 : 0) /*givexp */ , CurBullet->owner,
			  (CurBullet->mine ? 1 : 0));

		ThisRobot->frozen += CurBullet->freezing_level;

		ThisRobot->poison_duration_left += CurBullet->poison_duration;
		ThisRobot->poison_damage_per_sec += CurBullet->poison_damage_per_sec;

		ThisRobot->paralysation_duration_left += CurBullet->paralysation_duration;

		// If the blade can pass through dead and not dead bodies, it will so
		// so and create a small explosion passing by.  But if it can't, it should
		// be completely deleted of course, with the same small explosion as well
		//
		if (CurBullet->pass_through_hit_bodies)
			StartBlast(CurBullet->pos.x, CurBullet->pos.y, CurBullet->pos.z, BULLETBLAST, 0, CurBullet->faction, NULL);
		else
			DeleteBullet(num, TRUE);	// we want a bullet-explosion

		return;
	}
};				// void check_bullet_enemy_collisions ( CurBullet , num )

/**
 * This function checks if there are some collisions of the one bullet
 * with number num with anything else in the game, like blasts, walls,
 * droids, the tux and other bullets.
 */
void CheckBulletCollisions(int num)
{
	Bullet CurBullet = &AllBullets[num];

	switch (CurBullet->type) {
	case INFOUT:
		// Never do any collision checking if the bullet is INFOUT already...
		return;
		break;

	default:
		// If its a "normal" Bullet, several checks have to be
		// done, one for collisions with background, 
		// one for collision with influencer
		// some for collisions with enemies
		// and some for collisions with other bullets
		// and some for collisions with blast
		//
		// A bullet has a non null radius, so it can potentially hit 
		// an object (obstacle, enemy...) on a neighbor level. 
		// We however limit the collision detection to the visible levels
		// in order to lower the CPU cost, mainly because we don't really
		// care of what happens on invisible levels.
		//

		// Bullet/player collision code is level-independent

		check_bullet_player_collisions(CurBullet, num);
		if (CurBullet->type == INFOUT)
			return;

		// Bullet/background and bullet/enemy collision code is level dependent.
		// Store bullet's current position, in order to restore it after level dependent code
		gps bullet_actual_pos = CurBullet->pos;
		struct visible_level *visible_lvl, *next_lvl;

		BROWSE_VISIBLE_LEVELS(visible_lvl, next_lvl) {
			level *lvl = visible_lvl->lvl_pointer;

			// Compute the bullet position according to current visible level, if possible
			update_virtual_position(&CurBullet->pos, &bullet_actual_pos, lvl->levelnum);
			if (CurBullet->pos.x == -1)
				continue;

			// If the bullet is too far from the level's border, no need to check that level
			if (!pos_near_level(CurBullet->pos.x, CurBullet->pos.y, lvl, DROIDHITDIST))
				continue;

			// Now, collision detection...
			check_bullet_background_collisions(CurBullet, num);
			if (CurBullet->type == INFOUT)
				return;

			check_bullet_enemy_collisions(CurBullet, num);
			if (CurBullet->type == INFOUT)
				return;
		}

		// restore bullet's position, if the bullet is always active
		CurBullet->pos.x = bullet_actual_pos.x;
		CurBullet->pos.y = bullet_actual_pos.y;
		CurBullet->pos.z = bullet_actual_pos.z;

		break;
	}			// switch ( Bullet-Type )
};				// CheckBulletCollisions( ... )

/**
 * This function checks for collisions of blasts with bullets and droids
 * and delivers damage to the hit objects according to how long they have
 * been in the blast.
 * 
 * Maybe even some text like 'Ouch, this was hot' might be generated.
 *
 */
void CheckBlastCollisions(int num)
{
	blast *CurBlast = &(AllBlasts[num]);
	static const float Blast_Radius = 1.5;

	// Now we check for enemys, that might have stepped into this
	// one blasts area of effect...
	//
	// The blast is on a visible level, and we are only concerned with the effect
	// of the blast on any potentially visible enemy.
	//
	struct visible_level *visible_lvl, *n;
	enemy *erot, *nerot;
	gps blast_vpos;

	BROWSE_VISIBLE_LEVELS(visible_lvl, n) {
		level *lvl = visible_lvl->lvl_pointer;

		// Compute the blast position according to current visible level, if possible
		update_virtual_position(&blast_vpos, &CurBlast->pos, lvl->levelnum);
		if (blast_vpos.x == -1)
			continue;

		// If the blast is too far from the level's border, no need to check that level
		if (!pos_near_level(blast_vpos.x, blast_vpos.y, lvl, Blast_Radius))
			continue;

		BROWSE_LEVEL_BOTS_SAFE(erot, nerot, lvl->levelnum) {
			if (fabsf(erot->pos.x - blast_vpos.x) >= Blast_Radius)
				continue;
			if (fabsf(erot->pos.y - blast_vpos.y) >= Blast_Radius)
				continue;

			if (is_friendly(CurBlast->faction, erot->faction))
				continue;

			hit_enemy(erot, CurBlast->damage_per_second * Frame_Time(), 0, -1, CurBlast->faction == FACTION_SELF ? 1 : 0);
		}
	}

	// Now we check, if perhaps the influencer has stepped into the area
	// of effect of this one blast.  Then he'll get burnt ;)
	//
	if (!is_friendly(CurBlast->faction, FACTION_SELF)) {	
		update_virtual_position(&blast_vpos, &CurBlast->pos, Me.pos.z);
		if (blast_vpos.z != -1) {
			if ((fabsf(Me.pos.x - blast_vpos.x) < Blast_Radius) && (fabsf(Me.pos.y - blast_vpos.y) < Blast_Radius)) {
				float real_damage = CurBlast->damage_per_second * Frame_Time() * get_player_damage_factor();
				hit_tux(real_damage);
			}
		}
	}
}

/**
 * This function returns the bullet number of a specified bullet string.
 * Bullet strings are defined in map/bullet_archtypes.dat
 *
 * Defaults to 0
 */
int GetBulletByName(const char *bullet_name)
{
	int i;
	for (i=0; i < bullet_specs.size; i++) {
		struct bulletspec *bullet_spec = dynarray_member(&bullet_specs, i, sizeof(struct bulletspec));

		if (!bullet_spec->name)
			return 0;

		if (!strcmp(bullet_name, bullet_spec->name))
			return i;
	}
	error_message(__FUNCTION__, "\
The bullet name \"%s\" lacks a definition.", PLEASE_INFORM, bullet_name);
	return 0;
}

/**
 * This functions resolves the blast name to blast type.
 * If blast type wasn't find for the given name the default blast type is
 * returned.
 */
int get_blast_type_by_name(const char *name)
{
	if (!name)
		return 0;

	int i;
	for (i = 0; i < sizeof(Blastmap) / sizeof(Blastmap[0]); i++) {
		if (!strcmp(name, Blastmap[i].name))
			return i;
	}

	return 0;
}

#undef _bullet_c
