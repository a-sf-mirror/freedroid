/*
 *
 *   Copyright (c) 1994, 2002, 2003 Johannes Prix
 *   Copyright (c) 1994, 2002 Reinhard Prix
 *   Copyright (c) 2004-2007 Arthur Huillet
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

/* Distances for hitting a druid */
#define DRUIDHITDIST2		(0.0625)

/**
 *
 *
 */
static void move_this_bullet_and_check_its_collisions ( int num )
{
	Bullet CurBullet = &(AllBullets[num]); 
	moderately_finepoint bullet_step_vector;
	float whole_step_size;
	int i;
	float number_of_steps;
	
	//--------------------
	// In case of a bullet, which is not a melee weapon, we just move
	// the bullets as specified in it's speed vector.  But of course we
	// must make several stops and check for collisions in case the 
	// planned step would be too big to crash into walls...
	//
	whole_step_size = max( fabsf( CurBullet->speed.x * Frame_Time() ),
	                       fabsf( CurBullet->speed.y * Frame_Time() ) );
	
	//--------------------
	// NOTE:  The number 0.25 here is the value of thickness of the collision
	// rectangle of a standard wall.  Since we might not have loaded all wall tiles
	// at every time of the game, also during game, guessing the minimum thickness
	// of walls at runtime is a bit hard and would be unconvenient and complicated,
	// so I leave this with the hard-coded constant for now...
	//
	number_of_steps = rintf( whole_step_size / 0.25 ) + 1;
	
	bullet_step_vector.x = 0.5 * CurBullet->speed.x * Frame_Time() / number_of_steps;
	bullet_step_vector.y = 0.5 * CurBullet->speed.y * Frame_Time() / number_of_steps;
	
	for ( i = 0 ; i < number_of_steps ; i ++ )
	{
		CurBullet->pos.x += bullet_step_vector.x;
		CurBullet->pos.y += bullet_step_vector.y;
		
		// The bullet could have traverse a level's boundaries, so
		// get its new level and position, and check if the transformation was possible
		int pos_valid = resolve_virtual_position(&CurBullet->pos, &CurBullet->pos);
		if ( !pos_valid ) {
			DebugPrintf( -1000, "\nBullet outside of map: pos.x=%f, pos.y=%f, pos.z=%d, type=%d\n",
			             CurBullet->pos.x, CurBullet->pos.y, CurBullet->pos.z, CurBullet->type );
			DeleteBullet(num, FALSE);
			return;
		}
		
		CheckBulletCollisions( num );
		if ( CurBullet->type == INFOUT )
			return;
	}

}; // void move_this_bullet_and_check_its_collisions ( CurBullet )

/**
 * Whenever a new bullet is generated, we need to find a free index in 
 * the array of bullets.  This function automates the process and 
 * also is secure against too many bullets in the game (with a rather
 * ungraceful exit, but that event shouldn't ever occur in a normal game.
 */
int
find_free_melee_shot_index ( void )
{
    int j;

    for ( j = 0 ; j < MAX_MELEE_SHOTS ; j ++ )
    {
	if ( AllMeleeShots [ j ] . attack_target_type == ATTACK_TARGET_IS_NOTHING )
	{
	    return ( j ) ;
	    break;
	}
    }
    
    ErrorMessage ( __FUNCTION__  , "\
I seem to have run out of free melee shot entries." ,
			       PLEASE_INFORM, IS_WARNING_ONLY );
    
    return ( 0 );
    
}; // void find_free_bullet_entry_pointer ( void )


void delete_melee_shot(melee_shot * t)
{
    memset(t, 0, sizeof(melee_shot));
    t->attack_target_type = ATTACK_TARGET_IS_NOTHING;
}

/* ------------------------------------------------------------------
 * This function applies melee damage of all attacks that have taken
 * place in the previous cycle
 * ----------------------------------------------------------------- */
void DoMeleeDamage (void)
{
    int i;
    melee_shot * CurMelS;

    /* Browse all melee shots */
    for ( CurMelS = AllMeleeShots, i = 0; i < MAX_MELEE_SHOTS; CurMelS++, i++)	{
	if ( CurMelS -> attack_target_type == ATTACK_TARGET_IS_NOTHING )
	    continue;

	if ( CurMelS -> attack_target_type == ATTACK_TARGET_IS_ENEMY )  { 
	    /* Attack an enemy */
	    enemy * tg = enemy_resolve_address(CurMelS -> bot_target_n, &CurMelS->bot_target_addr);
	    if ( ! tg ) {
		ErrorMessage(__FUNCTION__, "Melee shot was set to ATTACK_TARGET_IS_ENEMY but had no targetted enemy. Deleting.\n", NO_NEED_TO_INFORM, IS_WARNING_ONLY);
		delete_melee_shot(CurMelS);
		continue;
	    }

	    if ( tg -> energy <= 0 ) {
		// our enemy is already dead ! 
		delete_melee_shot(CurMelS);
		continue;
	    }

	    if ( ((float) Druidmap [ tg -> type ] . monster_level * (float)MyRandom ( 100 ) < CurMelS->to_hit )) {
		if (CurMelS->mine) {
		}
		hit_enemy(tg, CurMelS->damage, CurMelS->mine ? 1 : 0, CurMelS->owner, CurMelS->mine ? 1 : 0);
	    } else 
		if (CurMelS->mine) {
		}
	}
	else if ( CurMelS -> attack_target_type == ATTACK_TARGET_IS_PLAYER ) { 
	    /* hit player */
	    if ( MyRandom ( 100 ) <= Me . lv_1_bot_will_hit_percentage * CurMelS->level) {
		Me . energy -= CurMelS -> damage;
		if ( MyRandom ( 100 ) <= 20 ) tux_scream_sound ( );
	    }
	}
	delete_melee_shot(CurMelS);
    }
}


/**
 * This function moves all the bullets according to their speeds and the
 * current frame rate of course.
 */
void
MoveBullets (void)
{
	int i;
	Bullet CurBullet;
	
	//--------------------
	// We move all the bullets
	//
	for ( i = 0; i < MAXBULLETS; i++)
	{
		CurBullet = &AllBullets[i];
		//--------------------
		// We need not move any bullets, that are INFOUT already...
		//
		if ( CurBullet->type == INFOUT )
			continue;
		
		if ( CurBullet->time_to_hide_still > 0 )
			continue;
		
		if ( !level_is_visible(CurBullet->pos.z) )
		{
			// if the bullet is on an inactive level, silently kill it
			DeleteBullet( i, FALSE );
			continue;
		}
		
		move_this_bullet_and_check_its_collisions( i );
		
		//--------------------
		// WARNING!  The bullet collision check might have deleted the bullet, so 
		//           maybe there's nothing sensible at the end of that 'CurBullet'
		//           pointer any more at this point.  So we check AGAIN for 'OUT'
		//           bullets, before we proceed with the safety out-of-map checks...
		//
		if ( CurBullet->type == INFOUT )
			continue;
		
		//--------------------
		// Maybe the bullet has a limited lifetime.  In that case we check if the
		// bullet has expired yet or not.
		//
		if ( ( CurBullet->bullet_lifetime != (-1) ) && 
		     ( CurBullet->time_in_seconds > CurBullet->bullet_lifetime ) )
		{
			DeleteBullet( i, FALSE );
			continue;
		}
		CurBullet->time_in_frames++;
		CurBullet->time_in_seconds += Frame_Time();

	}				/* for */
} // void MoveBullets(void)

/**
 * This function eliminates the bullet with the given number.  As an 
 * additional parameter you can specify if there should be a blast 
 * generated at the location where the bullet died (=TRUE) or not (=FALSE).
 */
void 
DeleteBullet ( int Bulletnumber , int ShallWeStartABlast )
{
  Bullet CurBullet = & ( AllBullets [ Bulletnumber ] ) ;

  //--------------------
  // At first we generate the blast at the collision spot of the bullet,
  // cause later, after the bullet is deleted, it will be hard to know
  // the correct location ;)
  //
  int type = (CurBullet->type == 4) ? OWNBLAST : BULLETBLAST;
  
  if ( ShallWeStartABlast ) StartBlast ( CurBullet->pos.x, CurBullet->pos.y, CurBullet->pos.z , type, (type == OWNBLAST) ? (CurBullet -> damage * 1) : 2.0 );


  CurBullet->type = INFOUT;
  CurBullet->time_in_seconds = 0;
  CurBullet->time_in_frames = 0;
  CurBullet->mine = FALSE;
  CurBullet->owner = -100;
  CurBullet->phase = 0;
  CurBullet->pos.x = 0;
  CurBullet->pos.y = 0;
  CurBullet->pos.z = -1;
  CurBullet->angle = 0;

}; // void DeleteBullet( int Bulletnumber , int StartBlast )

/**
 * This function starts a blast (i.e. an explosion) at the given location
 * in the usual map coordinates of course.  The type of blast must also
 * be specified, where possible values are defined in defs.h as follows:
 *
 * BULLETBLAST = 0 , (explosion of a small bullet hitting the wall)
 * DRUIDBLAST,       (explosion of a dying droid)
 * OWNBLAST          (not implemented)
 *
 */
void StartBlast ( float x, float y, int level, int type, int dmg)
{
	int i;
	Blast NewBlast;
	
	//--------------------
	// Maybe there is a box under the blast.  In this case, the box will
	// get smashed and perhaps an item will drop.
	// 
	smash_obstacle( x, y, level );
	
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
	NewBlast->pos.x = x;
	NewBlast->pos.y = y;
	NewBlast->pos.z = level;
    
	NewBlast->type = type;
	NewBlast->phase = 0;
	
	NewBlast->MessageWasDone = 0;
	NewBlast->damage_per_second = dmg;
	
	if (type == DRUIDBLAST)
	{
		DruidBlastSound();
	}
	
	if (type == OWNBLAST)
	{
		ExterminatorBlastSound();
	}
	
} // void StartBlast( ... )

/**
 * This function advances the different phases of an explosion according
 * to the current lifetime of each explosion (=blast).
 */
void
animate_blasts (void)
{
	int i;
	Blast CurBlast = AllBlasts;
	
	for (i = 0; i < MAXBLASTS; i++, CurBlast++)
		if (CurBlast->type != INFOUT)
		{
			
			//--------------------
			// But maybe the bullet is also outside the map already, which would
			// cause a SEGFAULT directly afterwards, when the map is queried.
			// Therefore we introduce some extra security here...
			//
			if ( !pos_inside_level( CurBlast->pos.x, CurBlast->pos.y, curShip.AllLevels[CurBlast->pos.z] ) )
			{
				ErrorMessage ( __FUNCTION__  , "\
A BLAST WAS FOUND TO EXIST OUTSIDE THE BOUNDS OF THE MAP.\n\
This is an idication for an inconsistency in Freedroid.\n\
\n\
However, the error is not fatal and will be silently compensated for now.\n\
When reporting a problem to the Freedroid developers, please note if this\n\
warning message was created prior to the error in your report.\n\
However, it should NOT cause any serious trouble for Freedroid.",
				       NO_NEED_TO_INFORM, IS_WARNING_ONLY );
				CurBlast->pos.x = 0 ;
				CurBlast->pos.y = 0 ;
				CurBlast->pos.z = 0 ;
				DeleteBlast( i );
				continue;
			}
			
			//--------------------
			// Druid blasts are dangerous, so we check if someone gets
			// hurt by this particular droid explosion
			//
			if (CurBlast->type == DRUIDBLAST || CurBlast->type == OWNBLAST ) CheckBlastCollisions (i);
			
			//--------------------
			// And now we advance the phase of the blast according to the
			// time that has passed since the last frame (approximately)
			//
			// CurBlast->phase += Frame_Time () * Blastmap[ CurBlast->type ].phases / Blastmap[ CurBlast->type ].total_animation_time;
			CurBlast->phase += Frame_Time () * PHASES_OF_EACH_BLAST / Blastmap[ CurBlast->type ].total_animation_time;
			
			//--------------------
			// Maybe the blast has lived over his normal lifetime already.
			// Then of course it's time to delete the blast, which is done
			// here.
			//
			if ( ( (int) floorf (CurBlast->phase)) >= PHASES_OF_EACH_BLAST )
				DeleteBlast (i);
		}				/* if */
}; // void animate_blasts( ... )

/**
 * This function deletes a single blast entry from the list of all blasts
 */
void
DeleteBlast(int BlastNum)
{
	AllBlasts[BlastNum].phase = INFOUT;
	AllBlasts[BlastNum].type = INFOUT;
}; // void DeleteBlast( int BlastNum )

/**
 * This function advances the currently active spells.
 */
void
MoveActiveSpells (void)
{
	int i ;
	float PassedTime;
	float DistanceFromCenter;
	PassedTime = Frame_Time();
	int direction_index;
	moderately_finepoint Displacement;
	moderately_finepoint final_point;
	float Angle;
	
	for ( i = 0; i < MAX_ACTIVE_SPELLS; i++ )
	{
		//--------------------
		// We can ignore all unused entries...
		//
		if ( AllActiveSpells[i].img_type == (-1) ) continue;
				
		//--------------------
		// All spells should count their lifetime...
		//
		AllActiveSpells[i].spell_age += PassedTime;
		
		// We hardcode a speed here
		AllActiveSpells[i].spell_radius += 5.0 * PassedTime;
		
		//--------------------
		// We do some collision checking with the obstacles in each
		// 'active_direction' of the spell and deactivate those directions,
		// where some collision with solid material has happend.
		//
		for ( direction_index = 0; direction_index < RADIAL_SPELL_DIRECTIONS; direction_index++ )
		{
			if ( AllActiveSpells[i].active_directions[direction_index] == FALSE ) continue;

			Angle = 360.0 * (float)direction_index / RADIAL_SPELL_DIRECTIONS;
			Displacement.x = AllActiveSpells[i].spell_radius; 
			Displacement.y = 0;			
			RotateVectorByAngle( &Displacement , Angle );
			final_point.x = AllActiveSpells[i].spell_center.x + Displacement.x;
			final_point.y = AllActiveSpells[i].spell_center.y + Displacement.y;
			// current_active_direction = rintf ( ( Angle  ) * (float) RADIAL_SPELL_DIRECTIONS / 360.0 ); 
			if ( !SinglePointColldet( final_point.x, final_point.y, Me.pos.z, &FlyablePassFilter ) )
				AllActiveSpells[i].active_directions[direction_index] = FALSE;
		}
		
		//--------------------
		// Here we also do the spell damage application here
		//
		float minDist = (0.2 + AllActiveSpells[i].spell_radius) * (0.2 + AllActiveSpells[i].spell_radius);
		
		struct visible_level *l, *n;
		enemy *erot, *nerot;
		BROWSE_NEARBY_VISIBLE_LEVELS( l, n, minDist )
		{			
			BROWSE_LEVEL_BOTS_SAFE(erot, nerot, l->lvl_pointer->levelnum)
			{
				update_virtual_position(&erot->virt_pos, &erot->pos, Me.pos.z);
				DistanceFromCenter = ( AllActiveSpells[i].spell_center.x - erot->virt_pos.x ) * ( AllActiveSpells[i].spell_center.x - erot->virt_pos.x ) +
	    		                     ( AllActiveSpells[i].spell_center.y - erot->virt_pos.y ) * ( AllActiveSpells[i].spell_center.y - erot->virt_pos.y );
			

				if ( DistanceFromCenter < minDist )
				{
					if ( ( AllActiveSpells[i].hit_type == ATTACK_HIT_BOTS   &&  Druidmap[erot->type].is_human ) ||
					     ( AllActiveSpells[i].hit_type == ATTACK_HIT_HUMANS && !Druidmap[erot->type].is_human ) )
						continue;
					
					//--------------------
					// Let's see if that enemy has a direction, that is still
					// active for the spell. 
					// We get the angle in radians but with zero at the 'north' direction.
					// And we convert the angle to a normal direction index
					//
					Displacement.x = erot->virt_pos.x - AllActiveSpells[i].spell_center.x;
					Displacement.y = erot->virt_pos.y - AllActiveSpells[i].spell_center.y;
					if ( Displacement.x <= 0.01 && Displacement.y <= 0.01 ) {
						// if enemy is very close, the Angle computation could be inaccurate,
						// so do not check if the spell is active or not
						direction_index = -1;
					} else {
						// nota : Y axis is toward down in fdrpg 
						Angle = atan2( -Displacement.y, Displacement.x );	// -M_PI <= Angle <= M_PI
						if ( Angle < 0 ) Angle += 2 * M_PI; 				// 0 <= Angle <= 2 * M_PI
						direction_index = (int) ( ( Angle * RADIAL_SPELL_DIRECTIONS ) / ( 2 * M_PI ) );
						// clamp direction_index to avoid any bug
						if ( direction_index < 0 ) direction_index = 0;
						if ( direction_index >= RADIAL_SPELL_DIRECTIONS ) direction_index = RADIAL_SPELL_DIRECTIONS - 1;
					}
					
					if ( (direction_index == -1) || AllActiveSpells[i].active_directions[direction_index] )
					{
						/* we hit the enemy. the owner is set to NULL because for now we assume it can only be the player.*/
						hit_enemy(erot, AllActiveSpells[i].damage * Frame_Time(), AllActiveSpells[i].mine ? 1 : 0 /*givexp*/, -1, AllActiveSpells[i].mine ? 1 : 0); 
						
						if ( erot->poison_duration_left < AllActiveSpells[i].poison_duration )
							erot->poison_duration_left = AllActiveSpells[i].poison_duration;
						erot->poison_damage_per_sec = AllActiveSpells[i].damage;
						
						if ( erot->frozen < AllActiveSpells[i].freeze_duration )
							erot->frozen = AllActiveSpells[i].freeze_duration;
						
						if ( erot->paralysation_duration_left < AllActiveSpells[i].paralyze_duration )
							erot->paralysation_duration_left = AllActiveSpells[i].paralyze_duration;						
					}
				}
			}
		}
		
		//--------------------
		// Such a spell can not live for longer than 1.0 seconds, say
		//
		if ( AllActiveSpells[i].spell_age >= 1.0 ) DeleteSpell( i );
		
	}
	
}; // void MoveActiveSpells( ... )

/**
 * This function deletes a single blast entry from the list of all blasts
 */
void
DeleteSpell (int SpellNum)
{
  AllActiveSpells [ SpellNum ] . img_type = ( -1 );
  AllActiveSpells [ SpellNum ] . spell_age = 0 ;
}; // void DeleteSpell( int SpellNum )

/**
 *
 *
 */
void
clear_active_spells ( void )
{
  int i ;

  for ( i = 0; i < MAX_ACTIVE_SPELLS; i++ )
    {
      DeleteSpell ( i ) ;
    }

}; // void clear_active_spells ( void )

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
		DeleteBullet( i, FALSE ); 
	}	
} // void clear_active_bullets()

/**
 * Whenever a new bullet is generated, we need to find a free index in 
 * the array of bullets.  This function automates the process and 
 * also is secure against too many bullets in the game (with a rather
 * ungraceful exit, but that event shouldn't ever occur in a normal game.
 */
int
find_free_bullet_index ( void )
{
    int j;

    for ( j = 0 ; j < MAXBULLETS ; j ++ )
    {
	if ( AllBullets [ j ] . type == INFOUT )
	{
	    return ( j ) ;
	    break;
	}
    }
    
    //--------------------
    // If this point is ever reached, there's a severe bug in here...
    //
    ErrorMessage ( __FUNCTION__  , "\
I seem to have run out of free bullet entries.  This can't normally happen.  --> some bug in here, oh no..." ,
			       PLEASE_INFORM, IS_FATAL );
    
    return ( -1 ) ; // can't happen.  just to make compilers happy (no warnings)
    
}; // void find_free_bullet_entry_pointer ( void )

/**
 * Bullet collision checks and effect handling for 'flash' bullets is 
 * done here...  Classic bullet collision and effect handling is done
 * somewhere else...
 */
void handle_flash_effects ( bullet* CurBullet )
{
	float my_damage;
	
	//--------------------
	// if the flash is over, just delete it and return
	if ( CurBullet->time_in_seconds > FLASH_DURATION_IN_SECONDS )
	{
		CurBullet->time_in_frames = 0;
		CurBullet->time_in_seconds = 0;
		CurBullet->type = INFOUT;
		CurBullet->mine = FALSE;
		CurBullet->owner = -100;
		return;
	}
	
	//--------------------
	// if the flash is not yet over, do some checking for who gets
	// hurt by it.  
	//
	// Two different methode for doing this are available:
	// The first but less elegant Method is just to check for
	// flash immunity, for distance and visiblity.
	//
	// The second and more elegant method is to recursively fill
	// out the room where the flash-maker is in and to hurt all
	// robots in there except of course for those immune.
	//
	if ( CurBullet->time_in_frames != 1 ) return; // we only do the damage once and thats at frame nr. 1 of the flash
	
	enemy *erot, *nerot;
	BROWSE_LEVEL_BOTS_SAFE(erot, nerot, CurBullet->pos.z)
	{
		if ( erot->type == (-1) ) continue;
		
		if ( IsVisible( &erot->pos ) && ( !Druidmap[erot->type].flashimmune ) ) 
		{
			hit_enemy( erot, CurBullet->damage, CurBullet->mine ? 1 : 0 /*givexp*/, CurBullet->owner, CurBullet->mine );
		}	
	}
	
	//--------------------
	// We do some damage to the Tux, depending on the current
	// disruptor resistance that the Tux might have...
	//
	my_damage = CurBullet->damage * ( 100 - Me.resist_disruptor ) / 100;
	Me.energy -= my_damage;
	append_new_game_message( _("Disruptor blast has hit you for %d damage."), (int)my_damage );

}; // handle_flash_effects ( bullet* CurBullet )

/**
 *
 *
 */  
void
check_bullet_background_collisions ( bullet* CurBullet , int num )
{
  // Check for collision with background
  if ( ! SinglePointColldet ( CurBullet -> pos . x , CurBullet -> pos . y , CurBullet -> pos . z, &FlyablePassFilter ) )
    {
      if ( CurBullet->ignore_wall_collisions )
	{
	  StartBlast ( CurBullet->pos.x , CurBullet->pos.y , CurBullet->pos.z , BULLETBLAST, 0 );
	}
      else
	{
	  DeleteBullet ( num , TRUE ); // we want a bullet-explosion
	  return;
	}
    }
}; // void check_bullet_background_collisions ( CurBullet , num )

/**
 *
 *
 */
void
apply_bullet_damage_to_player ( int damage, int owner ) 
{
    float real_damage = damage;
    
    UpdateAllCharacterStats( );
    
    //--------------------
    // NEW RULE:  Even when the bullet hits, there's still a chance that
    // the armour will compensate the shot
    //
    int monster_level = -1;
    if ( owner >= 0 ) monster_level = Druidmap [ owner ] . monster_level;
    if( ! monster_level || monster_level == -1)
	monster_level = 1;
    if ( MyRandom(100) / monster_level >= Me . lv_1_bot_will_hit_percentage )
    {
	Me . TextVisibleTime = 0 ;
	Me . TextToBeDisplayed = _("That one went into the armor.");
	DamageProtectiveEquipment ( ) ;
	BulletReflectedSound ( ) ;
    }
    else
    {
	
	Me . TextVisibleTime = 0 ;
	Me . TextToBeDisplayed = "Ouch!" ;
	Me . energy -= real_damage ;	// loose some energy
	DebugPrintf ( 1 , "\n%s(): Tux took damage from bullet: %f." , __FUNCTION__ , real_damage );
	
	tux_scream_sound ( );
    }
}; // void apply_bullet_damage_to_player ( int damage ) 

/**
 *
 *
 */
void
check_bullet_player_collisions ( bullet* CurBullet , int num )
{
  double xdist, ydist;

      //--------------------
      // Of course only active players and players on the same level
      // may be checked!
      //
      if ( Me . energy <= 0 || Me . pos . z != CurBullet -> pos . z ) return;
      
      //--------------------
      // A player is supposed not to hit himself with his bullets, so we may
      // check for that case as well....
      //
      if ( CurBullet -> mine ) return;

      if ( CurBullet -> is_friendly ) return;
      
      //--------------------
      // Now we see if the distance to the bullet is as low as hitting
      // distance or not.
      //
      
      xdist = Me . pos . x - CurBullet -> pos . x ;
      ydist = Me . pos . y - CurBullet -> pos . y ;
      if ((xdist * xdist + ydist * ydist) < DRUIDHITDIST2 )
	  {

	  apply_bullet_damage_to_player ( CurBullet-> damage, CurBullet->owner ) ; 
	  DeleteBullet ( num , TRUE ) ; // we want a bullet-explosion
	  return;  // This bullet was deleted and does not need to be processed any further...
	  }
}; // check_bullet_player_collisions ( CurBullet , num )

/**
 *
 *
 */
void
check_bullet_enemy_collisions ( bullet* CurBullet , int num )
{
    double xdist, ydist;
    int level = CurBullet -> pos.z ;
    
    if ( CurBullet -> type == INFOUT ) 
	fprintf(stderr, "Caca\n");
    //--------------------
    // Check for collision with enemys
    //
    enemy *ThisRobot, *nerot;
    BROWSE_LEVEL_BOTS_SAFE(ThisRobot, nerot, level)
	{
	xdist = CurBullet->pos.x - ThisRobot -> pos . x;
	ydist = CurBullet->pos.y - ThisRobot -> pos . y;

	if ( (xdist * xdist + ydist * ydist) < DRUIDHITDIST2 && ((float) Druidmap [ ThisRobot->type ] . monster_level * (float) MyRandom(100) < CurBullet->to_hit )
		&& ThisRobot->is_friendly != CurBullet->is_friendly)
	    {
	    if (( CurBullet->hit_type == ATTACK_HIT_BOTS && Druidmap[ThisRobot->type].is_human ) ||
		    ( CurBullet->hit_type == ATTACK_HIT_HUMANS && !Druidmap[ThisRobot->type].is_human))
		continue;

	    hit_enemy(ThisRobot, CurBullet->damage, (CurBullet->mine ? 1 : 0) /*givexp*/, CurBullet->owner, (CurBullet->mine ? 1 : 0));

	    ThisRobot -> frozen += CurBullet -> freezing_level;

	    ThisRobot -> poison_duration_left += CurBullet->poison_duration;
	    ThisRobot -> poison_damage_per_sec += CurBullet->poison_damage_per_sec;

	    ThisRobot -> paralysation_duration_left += CurBullet->paralysation_duration;

	    //--------------------
	    // If the blade can pass through dead and not dead bodies, it will so
	    // so and create a small explosion passing by.  But if it can't, it should
	    // be completely deleted of course, with the same small explosion as well
	    //
	    if ( CurBullet -> pass_through_hit_bodies )
		StartBlast ( CurBullet -> pos.x , CurBullet -> pos.y , CurBullet -> pos.z , BULLETBLAST, 0 );
	    else DeleteBullet( num , TRUE ); // we want a bullet-explosion

	    return;
	    } // if distance low enough to possibly be at hit
	} 
}; // void check_bullet_enemy_collisions ( CurBullet , num )

/**
 *
 *
 */
void check_bullet_bullet_collisions ( bullet* CurBullet , int num )
{
	int i;
	
	// check for collisions with other bullets
	for (i = 0; i < MAXBULLETS; i++)
	{
		if (i == num) continue;  // never check for collision with youself.. ;)
		if (CurBullet->pos.z != AllBullets[i].pos.z) continue; // not on same level
		if (AllBullets[i].type == INFOUT) continue; // never check for collisions with dead bullets.. 
		if (AllBullets[i].type == FLASH) continue; // never check for collisions with flashes bullets.. 
		
		if ( fabsf(AllBullets[i].pos.x-CurBullet->pos.x) > BULLET_BULLET_COLLISION_DIST ) continue;
		if ( fabsf(AllBullets[i].pos.y-CurBullet->pos.y) > BULLET_BULLET_COLLISION_DIST ) continue;
		// it seems like we have a collision of two bullets!
		// both will be deleted and replaced by blasts..
		DebugPrintf( 1 , "\nBullet-Bullet-Collision detected..." );
		
		//CurBullet->type = INFOUT;
		//AllBullets[num].type = INFOUT;
		
		if ( CurBullet->reflect_other_bullets )
		{
			if ( AllBullets[i].was_reflected )
			{
				// well, if it has been reflected once, we don't do any more
				// reflections after that...
			}
			else
			{
				AllBullets[i].speed.x = - AllBullets[i].speed.x;
				AllBullets[i].speed.y = - AllBullets[i].speed.y;
				AllBullets[i].was_reflected = TRUE;
			}
		}
		
		if ( AllBullets[i].reflect_other_bullets )
		{
			if ( CurBullet->was_reflected )
			{
				// well, if it has been reflected once, we don't do any more
				// reflections after that...
			}
			else
			{
				CurBullet->speed.x = - CurBullet->speed.x;
				CurBullet->speed.y = - CurBullet->speed.y;
				CurBullet->was_reflected = TRUE;
			}
		}
		
	}
}; // void check_bullet_bullet_collisions ( CurBullet , num )

/**
 * This function checks if there are some collisions of the one bullet
 * with number num with anything else in the game, like blasts, walls,
 * droids, the tux and other bullets.
 */
void
CheckBulletCollisions (int num)
{
    Bullet CurBullet = &AllBullets[num];

    switch ( CurBullet -> type )
    {
	case INFOUT:
	    // --------------------
	    // Never do any collision checking if the bullet is INFOUT already...
	    return;
	    break;
	    
	case FLASH:
	    // --------------------
	    // Next we handle the case that the bullet is of type FLASH
	    handle_flash_effects ( CurBullet );
	    return;
	    break;
	    
	default:
	    // --------------------
	    // If its a "normal" Bullet, several checks have to be
	    // done, one for collisions with background, 
	    // one for collision with influencer
	    // some for collisions with enemys
	    // and some for collisions with other bullets
	    // and some for collisions with blast
	    //
	    check_bullet_background_collisions ( CurBullet , num );
	    if ( CurBullet->type == INFOUT )
		return;
	    check_bullet_player_collisions ( CurBullet , num );
	    if  ( CurBullet->type == INFOUT )
		return;
	    check_bullet_enemy_collisions ( CurBullet , num );
	    if ( CurBullet->type == INFOUT )
		return;
	    check_bullet_bullet_collisions ( CurBullet , num );
	    
	    break;
    } // switch ( Bullet-Type )
}; // CheckBulletCollisions( ... )

/**
 * This function checks for collisions of blasts with bullets and druids
 * and delivers damage to the hit objects according to how long they have
 * been in the blast.
 * 
 * Maybe even some text like 'Ouch, this was hot' might be generated.
 *
 */
void
CheckBlastCollisions (int num)
{
    int i;
    Blast CurBlast = &(AllBlasts[num]);
    int level = CurBlast->pos.z;
    static const float Blast_Radius = 0.3;
    //--------------------
    // At first, we check for collisions of this blast with all bullets 
    //
    for (i = 0; i < MAXBULLETS; i++)
    {
	if (AllBullets[i].type == INFOUT)
	    continue;
	if (CurBlast->phase > 4)
	    break;
	
	if (abs (AllBullets[i].pos.x - CurBlast->pos.x ) < Blast_Radius)
	{
	    if (abs (AllBullets[i].pos.y - CurBlast->pos.y ) < Blast_Radius)
	    {
		if ( AllBullets[i].pos.z == CurBlast->pos.z )
		{
		    if ( ! AllBullets[i].pass_through_explosions )
		    {
			DeleteBullet( i , TRUE ); // we want a bullet-explosion
		    }
		}
	    }
	}
    }	
    
    //--------------------
    // Now we check for enemys, that might have stepped into this
    // one blasts area of effect...
    //
    enemy *erot, *nerot;
    
    BROWSE_LEVEL_BOTS_SAFE(erot, nerot, level) 
	{
	if ( ( fabsf (erot->pos.x - CurBlast->pos.x ) < Blast_Radius ) &&
		( fabsf (erot->pos.y - CurBlast->pos.y ) < Blast_Radius ) )
	    {
	    /* we have no support for blast ownership yet, so we give no XP *and* don't know who killed the guy */
	    hit_enemy(erot, CurBlast -> damage_per_second * Frame_Time (), 0, -1, 0);
	    }

	}
    
    //--------------------
    // Now we check, if perhaps the influencer has stepped into the area
    // of effect of this one blast.  Then he'll get burnt ;)
    // 
    if ( (Me.energy > 0) && 
	 ( fabsf (Me.pos.x - CurBlast->pos.x ) < Blast_Radius ) &&
	 ( fabsf (Me.pos.y - CurBlast->pos.y ) < Blast_Radius ) )
    {
	Me.energy -= CurBlast->damage_per_second * Frame_Time ();
	
	// So the influencer got some damage from the hot blast
	// Now most likely, he then will also say so :)
	if ( !CurBlast->MessageWasDone )
	{
	    CurBlast->MessageWasDone=TRUE;
	}
    }
    
}; // CheckBlastCollisions( ... )

#undef _bullet_c
