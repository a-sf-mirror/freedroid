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

#define _bullet_c 1

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
static int move_this_bullet_and_check_its_collisions(struct bullet *current_bullet)
{
	moderately_finepoint bullet_step_vector;
	float whole_step_size;
	int i;
	float number_of_steps;

	// In case of a bullet, which is not a melee weapon, we just move
	// the bullets as specified in it's speed vector.  But of course we
	// must make several stops and check for collisions in case the 
	// planned step would be too big to crash into walls...
	//
	whole_step_size = max(fabsf(current_bullet->speed.x * Frame_Time()), fabsf(current_bullet->speed.y * Frame_Time()));

	// NOTE:  The number 0.25 here is the value of thickness of the collision
	// rectangle of a standard wall.  Since we might not have yet loaded all
	// the wall tiles (due to lazy loading), guessing the minimum thickness
	// of walls at runtime is a bit hard and would be inconvenient and complicated,
	// so we leave this with the hard-coded constant for now...

	number_of_steps = rintf(whole_step_size / 0.25) + 1;

	bullet_step_vector.x = 0.5 * current_bullet->speed.x * Frame_Time() / number_of_steps;
	bullet_step_vector.y = 0.5 * current_bullet->speed.y * Frame_Time() / number_of_steps;

	for (i = 0; i < number_of_steps; i++) {
		current_bullet->pos.x += bullet_step_vector.x;
		current_bullet->pos.y += bullet_step_vector.y;

		// The bullet could have traverse a level's boundaries, so
		// retrieve its new level and position, if possible
		int pos_valid = resolve_virtual_position(&current_bullet->pos, &current_bullet->pos);
		if (!pos_valid) {
			DebugPrintf(1, "Bullet outside of map: pos.x=%f, pos.y=%f, pos.z=%d, type=%d\n",
			            current_bullet->pos.x, current_bullet->pos.y, current_bullet->pos.z, current_bullet->type);
			return TRUE;
		}

		// If the bullet has reached an invisible level, we will remove it
		if (!level_is_visible(current_bullet->pos.z)) {
			return TRUE;
		}

		// Check collision with the environment
		if (check_bullet_collisions(current_bullet))
			return TRUE;
	}

	// No collision detected
	return FALSE;
}

void delete_melee_shot(int melee_shot_number)
{
	dynarray_del(&all_melee_shots, melee_shot_number, sizeof(struct melee_shot));
}

/* ------------------------------------------------------------------
 * This function applies melee damage of all attacks that have taken
 * place in the previous cycle
 * ----------------------------------------------------------------- */
void do_melee_damage(void)
{
	int i;
	float latest_frame_time = Frame_Time();

	/* Browse all melee shots */
	for (i = 0; i < all_melee_shots.size; i++) {
		// Unused melee shot slot
		if (!sparse_dynarray_member_used(&all_melee_shots, i))
			continue;

		struct melee_shot *current_melee_shot = (struct melee_shot *)dynarray_member(&all_melee_shots, i, sizeof(struct melee_shot));

		// Wait the hit of the melee shot
		if (current_melee_shot->time_to_hit > 0) {
			current_melee_shot->time_to_hit -= latest_frame_time;
			continue;
		}

		if (current_melee_shot->attack_target_type == ATTACK_TARGET_IS_ENEMY) {
			/* Attack an enemy */
			enemy *tg = enemy_resolve_address(current_melee_shot->bot_target_n, &current_melee_shot->bot_target_addr);
			if (!tg) {
				error_message(__FUNCTION__,
				              "Melee shot was set to ATTACK_TARGET_IS_ENEMY but had no targeted enemy. Deleting.",
				              NO_REPORT);
				delete_melee_shot(i);
				continue;
			}

			if (tg->energy <= 0) {
				// our enemy is already dead ! 
				delete_melee_shot(i);
				continue;
			}

			if (MyRandom(100) < current_melee_shot->to_hit) {
				// Slow or paralyze enemies if the player has bonuses with those effects.
				tg->frozen += Me.slowing_melee_targets;
				tg->paralysation_duration_left += Me.paralyzing_melee_targets;

				hit_enemy(tg, current_melee_shot->damage, current_melee_shot->mine ? 1 : 0, current_melee_shot->owner, current_melee_shot->mine ? 1 : 0);
			}

			delete_melee_shot(i);
			continue;

		}

		if (current_melee_shot->attack_target_type == ATTACK_TARGET_IS_PLAYER) {
			/* hit player */
			if (MyRandom(100) < current_melee_shot->to_hit) {
				float real_damage = current_melee_shot->damage * get_player_damage_factor();
				hit_tux(real_damage);
				DamageProtectiveEquipment();
			}
		}

		delete_melee_shot(i);
	}
}

/**
 * This function moves all the bullets according to their speeds and the
 * current frame rate of course.
 */
void move_bullets(void)
{
	int i;

	for (i = 0; i < all_bullets.size; i++) {
		// Unused bullet slot
		if (!sparse_dynarray_member_used(&all_bullets, i))
			continue;

		struct bullet *current_bullet = (struct bullet *)dynarray_member(&all_bullets, i, sizeof(struct bullet));

		// if during its move, a bullet collides something, it has done its job !
		if (move_this_bullet_and_check_its_collisions(current_bullet)) {
			delete_bullet(i);
			continue;
		}

		// Maybe the bullet has a limited lifetime.  In that case we check if the
		// bullet has expired yet or not.
		if ((current_bullet->bullet_lifetime != (-1)) && (current_bullet->time_in_seconds > current_bullet->bullet_lifetime)) {
			delete_bullet(i);
			continue;
		}

		// Store new bullet's age, for next move round
		current_bullet->time_in_seconds += Frame_Time();
	}
}

/**
 * This function eliminates the bullet with the given number.
 */
void delete_bullet(int bullet_number)
{
	dynarray_del(&all_bullets, bullet_number, sizeof(struct bullet));
}

/**
 * This function starts a blast (i.e. an explosion) at the given location
 * in the usual map coordinates of course.  The type of blast must also
 * be specified, where possible values are defined in defs.h as follows:
 *
 * BULLETBLAST = 0 , (explosion of a small bullet hitting the wall)
 * DROIDBLAST,       (explosion of a dying droid)
 * EXTERMINATORBLAST (explosion of an exterminator)
 *
 * start_blast will either use sound_name if passed, or if NULL will use
 * the default sound for the blast. 
 */
void start_blast(float x, float y, int lvl, int type, int dmg, int faction, char *sound_name)
{
	// Resolve blast real position, if possible
	//
	gps blast_vpos = { x, y, lvl };
	gps blast_pos;

	if (!resolve_virtual_position(&blast_pos, &blast_vpos)) {
		// The blast position is nowhere....
		error_message(__FUNCTION__,
		              "A BLAST VIRTUAL POSITION WAS FOUND TO BE INCONSISTENT.\n"
		              "\n"
		              "However, the error is not fatal and will be silently compensated for now.\n"
		              "When reporting a problem to the FreedroidRPG developers, please note if this\n"
		              "warning message was created prior to the error in your report.\n"
		              "However, it should NOT cause any serious trouble for FreedroidRPG.",
		              NO_REPORT);
		return;
	}
	// If the blast is not on a visible level, we do not take care of it
	if (!level_is_visible(blast_pos.z))
		return;

	// Maybe there is a box under the blast.  In this case, the box will
	// get smashed and perhaps an item will drop.

	smash_obstacle(blast_pos.x, blast_pos.y, blast_pos.z);

	// create a new blast at the specified x/y coordinates
	struct blast new_blast;
	new_blast.pos.x = blast_pos.x;
	new_blast.pos.y = blast_pos.y;
	new_blast.pos.z = blast_pos.z;
	new_blast.type = type;
	new_blast.phase = 0;
	new_blast.damage_per_second = dmg;
	new_blast.faction = faction;

	// add the blast to the game
	dynarray_add(&all_blasts, &new_blast, sizeof(struct blast));

	// play the blast sound effect
	if (sound_name) {
		play_blast_sound(sound_name, &new_blast.pos);
	} else {
		play_blast_sound(Blastmap[type].sound_file, &new_blast.pos);
	}
}

/**
 * This function advances the different phases of an explosion according
 * to the current lifetime of each explosion (=blast).
 */
void animate_blasts(void)
{
	int i;

	for (i = 0; i < all_blasts.size; i++) {
		if (sparse_dynarray_member_used(&all_blasts, i)) {

			struct blast *current_blast = (struct blast *)dynarray_member(&all_blasts, i, sizeof(struct blast));

			// But maybe the blast is also outside the map already, which would
			// cause a SEGFAULT directly afterwards, when the map is queried.
			// Therefore we introduce some extra security here...

			if (!pos_inside_level(current_blast->pos.x, current_blast->pos.y, curShip.AllLevels[current_blast->pos.z])) {
				error_message(__FUNCTION__,
				              "A BLAST WAS FOUND TO EXIST OUTSIDE THE BOUNDS OF THE MAP.\n"
				              "This is an indication of an inconsistency in FreedroidRPG.\n"
				              "\n"
				              "However, the error is not fatal and will be silently compensated for now.\n"
				              "When reporting a problem to the FreedroidRPG developers, please note if this\n"
				              "warning message was created prior to the error in your report.\n"
				              "However, it should NOT cause any serious trouble for FreedroidRPG.",
				              NO_REPORT);
				delete_blast(i);
				continue;
			}
			
			if (Blastmap[current_blast->type].do_damage)
				check_blast_collisions(current_blast);

			// And now we advance the phase of the blast according to the
			// time that has passed since the last frame (approximately)

			current_blast->phase += Frame_Time() * Blastmap[current_blast->type].phases / Blastmap[current_blast->type].total_animation_time;

			// Maybe the blast has lived over his normal lifetime already.
			// Then of course it's time to delete the blast, which is done
			// here.

			if ((int)floorf(current_blast->phase) >= Blastmap[current_blast->type].phases)
				delete_blast(i);
		}
	}
}

/**
 * This function deletes a single blast entry from the list of all blasts
 */
void delete_blast(int blast_number)
{
	dynarray_del(&all_blasts, blast_number, sizeof(struct blast));
}

/**
 * This function advances the currently active spells.
 */
void move_active_spells(void)
{
	int i;
	float passed_time = Frame_Time();
	float distance_from_center;
	int direction_index;
	moderately_finepoint displacement;
	gps final_point;
	float angle;

	for (i = 0; i < MAX_ACTIVE_SPELLS; i++) {
		// We can ignore all unused entries...

		if (AllActiveSpells[i].img_type == (-1))
			continue;

		// All spells should count their lifetime...

		AllActiveSpells[i].spell_age += passed_time;

		// We hardcode a speed here

		AllActiveSpells[i].spell_radius += 5.0 * passed_time;

		// We do some collision checking with the obstacles in each
		// 'active_direction' of the spell and deactivate those directions,
		// where some collision with solid material has happened.

		for (direction_index = 0; direction_index < RADIAL_SPELL_DIRECTIONS; direction_index++) {
			if (AllActiveSpells[i].active_directions[direction_index] == FALSE)
				continue;

			angle = 360.0 * (float)direction_index / RADIAL_SPELL_DIRECTIONS;
			displacement.x = AllActiveSpells[i].spell_radius;
			displacement.y = 0;
			RotateVectorByAngle(&displacement, angle);
			final_point.x = AllActiveSpells[i].spell_center.x + displacement.x;
			final_point.y = AllActiveSpells[i].spell_center.y + displacement.y;
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

		float minDist = (0.2 + AllActiveSpells[i].spell_radius) * (0.2 + AllActiveSpells[i].spell_radius);

		struct visible_level *visible_lvl, *n;
		enemy *erot, *nerot;
		BROWSE_NEARBY_VISIBLE_LEVELS(visible_lvl, n, minDist) {
			BROWSE_LEVEL_BOTS_SAFE(erot, nerot, visible_lvl->lvl_pointer->levelnum) {
				update_virtual_position(&erot->virt_pos, &erot->pos, Me.pos.z);
				distance_from_center =
				    (AllActiveSpells[i].spell_center.x - erot->virt_pos.x) * (AllActiveSpells[i].spell_center.x -
											      erot->virt_pos.x) +
				    (AllActiveSpells[i].spell_center.y - erot->virt_pos.y) * (AllActiveSpells[i].spell_center.y -
											      erot->virt_pos.y);

				if (distance_from_center < minDist) {
					if ((AllActiveSpells[i].hit_type == ATTACK_HIT_BOTS && Droidmap[erot->type].is_human) ||
					    (AllActiveSpells[i].hit_type == ATTACK_HIT_HUMANS && !Droidmap[erot->type].is_human))
						continue;

					// Let's see if that enemy has a direction, that is still
					// active for the spell. 
					// We get the angle in radians but with zero at the 'north' direction.
					// And we convert the angle to a normal direction index

					displacement.x = erot->virt_pos.x - AllActiveSpells[i].spell_center.x;
					displacement.y = erot->virt_pos.y - AllActiveSpells[i].spell_center.y;
					if (displacement.x <= 0.01 && displacement.y <= 0.01) {
						// if enemy is very close, the angle computation could be inaccurate,
						// so do not check if the spell is active or not
						direction_index = -1;
					} else {
						// nota : Y axis is toward down in fdrpg 
						angle = atan2(-displacement.y, displacement.x);	// -M_PI <= Angle <= M_PI
						if (angle < 0)
							angle += 2 * M_PI;	// 0 <= Angle <= 2 * M_PI
						direction_index = (int)((angle * RADIAL_SPELL_DIRECTIONS) / (2 * M_PI));
						// clamp direction_index to avoid any bug
						if (direction_index < 0)
							direction_index = 0;
						if (direction_index >= RADIAL_SPELL_DIRECTIONS)
							direction_index = RADIAL_SPELL_DIRECTIONS - 1;
					}

					if ((direction_index == -1) || AllActiveSpells[i].active_directions[direction_index]) {
						if (erot->poison_duration_left < AllActiveSpells[i].poison_duration)
							erot->poison_duration_left = AllActiveSpells[i].poison_duration;
						erot->poison_damage_per_sec = AllActiveSpells[i].damage;

						if (erot->frozen < AllActiveSpells[i].freeze_duration)
							erot->frozen = AllActiveSpells[i].freeze_duration;

						if (erot->paralysation_duration_left < AllActiveSpells[i].paralyze_duration)
							erot->paralysation_duration_left = AllActiveSpells[i].paralyze_duration;

						/* we hit the enemy. the owner is set to NULL because for now we assume it can only be the player. */
						hit_enemy(erot, AllActiveSpells[i].damage * Frame_Time(),
							  AllActiveSpells[i].mine ? 1 : 0 /*givexp */ , -1,
							  AllActiveSpells[i].mine ? 1 : 0);
					}
				}
			}
		}

		// Such a spell can not live for longer than 1.0 seconds, say

		if (AllActiveSpells[i].spell_age >= 1.0)
			DeleteSpell(i);
	}
}

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
}

/**
 *
 *
 */
void clear_active_bullets(void)
{
	int i;

	for (i = 0; i < all_blasts.size; i++) {
		if (sparse_dynarray_member_used(&all_blasts, i))
			delete_blast(i);
	}

	for (i = 0; i < all_bullets.size; i++) {
		if (sparse_dynarray_member_used(&all_bullets, i))
			delete_bullet(i);
	}
}

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
int check_bullet_background_collisions(struct bullet *current_bullet)
{
	// Check for collision with background
	struct colldet_filter filter = { &FlyablePassFilterCallback, NULL, 0.05, NULL };
	if (!SinglePointColldet(current_bullet->pos.x, current_bullet->pos.y, current_bullet->pos.z, &filter)) {
		// We want a bullet-explosion, so we create a blast
		struct bulletspec *bullet_spec = (struct bulletspec *)dynarray_member(&bullet_specs, current_bullet->type, sizeof(struct bulletspec));
		start_blast(current_bullet->pos.x, current_bullet->pos.y, current_bullet->pos.z, bullet_spec->blast_type, current_bullet->damage, current_bullet->faction, NULL);

		return TRUE;
	}

	return FALSE;
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
int check_bullet_player_collisions(struct bullet *current_bullet)
{
	double xdist, ydist;

	// Of course only active player may be checked!

	if (Me.energy <= 0)
		return FALSE;

	// A player is supposed not to hit himself with his bullets, so we may
	// check for that case as well....

	if (current_bullet->mine)
		return FALSE;

	if (is_friendly(current_bullet->faction, FACTION_SELF))
		return FALSE;

	// Now we see if the distance to the bullet is as low as hitting
	// distance or not.

	// We need compatible gps positions to compute a distance
	gps bullet_vpos;
	update_virtual_position(&bullet_vpos, &current_bullet->pos, Me.pos.z);
	if (bullet_vpos.x == -1)
		return FALSE;

	xdist = Me.pos.x - bullet_vpos.x;
	ydist = Me.pos.y - bullet_vpos.y;

	if ((xdist * xdist + ydist * ydist) < DROIDHITDIST2) {
		apply_bullet_damage_to_player(current_bullet->damage, current_bullet->owner);
		// We want a bullet-explosion, so we create a blast
		struct bulletspec *bullet_spec = (struct bulletspec *)dynarray_member(&bullet_specs, current_bullet->type, sizeof(struct bulletspec));
		start_blast(current_bullet->pos.x, current_bullet->pos.y, current_bullet->pos.z, bullet_spec->blast_type, current_bullet->damage, current_bullet->faction, NULL);

		return TRUE;
	}

	return FALSE;
}

/**
 *
 *
 */
int check_bullet_enemy_collisions(struct bullet *current_bullet)
{
	int lvl = current_bullet->pos.z;

	// Check for collision with enemys
	//
	enemy *ThisRobot, *nerot;
	BROWSE_LEVEL_BOTS_SAFE(ThisRobot, nerot, lvl) {
		// Check several hitting conditions
		//
		double xdist = current_bullet->pos.x - ThisRobot->pos.x;
		double ydist = current_bullet->pos.y - ThisRobot->pos.y;

		if ((xdist * xdist + ydist * ydist) >= DROIDHITDIST2)
			continue;

		if (current_bullet->hit_type == ATTACK_HIT_BOTS && Droidmap[ThisRobot->type].is_human)
			continue;
		if (current_bullet->hit_type == ATTACK_HIT_HUMANS && !Droidmap[ThisRobot->type].is_human)
			continue;
		if (is_friendly(ThisRobot->faction, current_bullet->faction))
			continue;

		// Hit the ennemy

		ThisRobot->frozen += current_bullet->freezing_level;

		ThisRobot->poison_duration_left += current_bullet->poison_duration;
		ThisRobot->poison_damage_per_sec += current_bullet->poison_damage_per_sec;

		ThisRobot->paralysation_duration_left += current_bullet->paralysation_duration;

		hit_enemy(ThisRobot, current_bullet->damage, (current_bullet->mine ? 1 : 0) /*givexp */ , current_bullet->owner,
			  (current_bullet->mine ? 1 : 0));

		// If the blade can pass through dead and not dead bodies, it will so
		// so and create a small explosion passing by.  But if it can't, it should
		// be completely deleted of course, with the same small explosion as well

		if (current_bullet->pass_through_hit_bodies) {
			start_blast(current_bullet->pos.x, current_bullet->pos.y, current_bullet->pos.z, BULLETBLAST, 0, current_bullet->faction, NULL);
			return FALSE;
		} else {
			// We want a bullet-explosion
			struct bulletspec *bullet_spec = (struct bulletspec *)dynarray_member(&bullet_specs, current_bullet->type, sizeof(struct bulletspec));
			start_blast(current_bullet->pos.x, current_bullet->pos.y, current_bullet->pos.z, bullet_spec->blast_type, current_bullet->damage, current_bullet->faction, NULL);

			return TRUE;
		}
	}

	return FALSE;
}

/**
 * This function checks if there are some collisions of the one bullet
 * with number num with anything else in the game, like blasts, walls,
 * droids, the tux and other bullets.
 */
int check_bullet_collisions(struct bullet *current_bullet)
{
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

	// Bullet/player collision code is level-independent

	if (check_bullet_player_collisions(current_bullet))
		return TRUE;

	// Bullet/background and bullet/enemy collision code is level dependent.
	// Store bullet's current position, in order to restore it after level dependent code

	gps bullet_actual_pos = current_bullet->pos;
	struct visible_level *visible_lvl, *next_lvl;

	BROWSE_VISIBLE_LEVELS(visible_lvl, next_lvl) {
		level *lvl = visible_lvl->lvl_pointer;

		// Compute the bullet position according to current visible level, if possible
		update_virtual_position(&current_bullet->pos, &bullet_actual_pos, lvl->levelnum);
		if (current_bullet->pos.x == -1)
			goto RESTORE;

		// If the bullet is too far from the level's border, no need to check that level
		if (!pos_near_level(current_bullet->pos.x, current_bullet->pos.y, lvl, DROIDHITDIST))
			goto RESTORE;

		// Now, collision detection...
		if (check_bullet_background_collisions(current_bullet))
			return TRUE;

		if (check_bullet_enemy_collisions(current_bullet))
			return TRUE;

		// restore bullet's position, before to check with the next level
RESTORE:
		current_bullet->pos.x = bullet_actual_pos.x;
		current_bullet->pos.y = bullet_actual_pos.y;
		current_bullet->pos.z = bullet_actual_pos.z;
	}

	// No collision detected
	return FALSE;
}

/**
 * This function checks for collisions of blasts with bullets and droids
 * and delivers damage to the hit objects according to how long they have
 * been in the blast.
 * 
 * Maybe even some text like 'Ouch, this was hot' might be generated.
 */
void check_blast_collisions(struct blast *current_blast)
{
	static const float blast_radius = 1.5;

	// Now we check for enemys, that might have stepped into this
	// one blasts area of effect...
	//
	// The blast is on a visible level, and we are only concerned with the effect
	// of the blast on any potentially visible enemy.

	struct visible_level *visible_lvl, *n;
	enemy *erot, *nerot;
	gps blast_vpos;

	BROWSE_VISIBLE_LEVELS(visible_lvl, n) {
		level *lvl = visible_lvl->lvl_pointer;

		// Compute the blast position according to current visible level, if possible
		update_virtual_position(&blast_vpos, &current_blast->pos, lvl->levelnum);
		if (blast_vpos.x == -1)
			continue;

		// If the blast is too far from the level's border, no need to check that level
		if (!pos_near_level(blast_vpos.x, blast_vpos.y, lvl, blast_radius))
			continue;

		BROWSE_LEVEL_BOTS_SAFE(erot, nerot, lvl->levelnum) {
			if (fabsf(erot->pos.x - blast_vpos.x) >= blast_radius)
				continue;
			if (fabsf(erot->pos.y - blast_vpos.y) >= blast_radius)
				continue;

			if (is_friendly(current_blast->faction, erot->faction))
				continue;

			hit_enemy(erot, current_blast->damage_per_second * Frame_Time(), 0, -1, current_blast->faction == FACTION_SELF ? 1 : 0);
		}
	}

	// Now we check, if perhaps the influencer has stepped into the area
	// of effect of this one blast.  Then he'll get burnt ;)

	if (!is_friendly(current_blast->faction, FACTION_SELF)) {
		update_virtual_position(&blast_vpos, &current_blast->pos, Me.pos.z);
		if (blast_vpos.z != -1) {
			if ((fabsf(Me.pos.x - blast_vpos.x) < blast_radius) && (fabsf(Me.pos.y - blast_vpos.y) < blast_radius)) {
				float real_damage = current_blast->damage_per_second * Frame_Time() * get_player_damage_factor();
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
