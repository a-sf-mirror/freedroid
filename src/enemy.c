/* 
 *   Copyright (c) 1994, 2002, 2003 Johannes Prix
 *   Copyright (c) 1994, 2002 Reinhard Prix
 *   Copyright (c) 2004-2007 Arthur Huillet 
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
 * This file contains all enemy realted functions.  This includes their 
 * whole behaviour, healing, initialization, shuffling them around after 
 * evevator-transitions, deleting them, collisions of enemys among 
 * themselves, their fireing, animation and such.
 */

#define _enemy_c

#include "system.h"

#include "defs.h"
#include "struct.h"
#include "global.h"
#include "proto.h"

#define COL_SPEED		3

#define IS_FRIENDLY_EYE_DISTANCE (2.0)


void check_if_switching_to_stopandeyetuxmode_makes_sense ( enemy* );
static int TurnABitTowardsPosition ( Enemy, float, float, float);
static void MoveToMeleeCombat ( Enemy, gps*, moderately_finepoint *);
static void MoveAwayFromMeleeCombat ( Enemy, moderately_finepoint *);
static void ReachMeleeCombat( Enemy, gps*, moderately_finepoint*, pathfinder_context* );
static void RawStartEnemysShot( enemy*, float, float);
static int is_potential_target( enemy* this_robot, gps* target_pos, float* squared_best_dist );

LIST_HEAD(alive_bots_head);
LIST_HEAD(dead_bots_head);
list_head_t level_bots_head[MAX_LEVELS]; //THIS IS NOT STATICALLY PROPERLY INITIALIZED, done in init functions


/**
 * In the very beginning of each game, it is not enough to just place the
 * bots onto the right locations.  They must also be integrated into the
 * waypoint system, i.e. current waypoint and next waypoint initialized.
 * This is what this function is supposed to do.
 */
void
TeleportToClosestWaypoint ( Enemy ThisRobot )
{
    int i;
    float BestDistanceSqu = 10000;
    float NewDistance = 10000;
    Level ThisLevel = curShip . AllLevels [ ThisRobot -> pos . z ] ;
    short BestWaypoint = ( -1 );
    
    for ( i = 0 ; i < ThisLevel->num_waypoints ; i ++ )
    {
	if ( ThisLevel -> AllWaypoints [ i ] . x <= 0 ) continue;
	
	NewDistance = ( ThisRobot -> pos . x - ThisLevel -> AllWaypoints [ i ] . x + 0.5 ) *
			     ( ThisRobot -> pos . x - ThisLevel -> AllWaypoints [ i ] . x + 0.5 ) +
			     ( ThisRobot -> pos . y - ThisLevel -> AllWaypoints [ i ] . y + 0.5 ) *
			     ( ThisRobot -> pos . y - ThisLevel -> AllWaypoints [ i ] . y + 0.5 );
	
	if ( NewDistance <= BestDistanceSqu )
	{
	    BestDistanceSqu = NewDistance;
	    BestWaypoint = i ;
	}
    }
    
    //--------------------
    // Now we have found a global minimum.  So we 'teleport' there.
    
    if ( BestWaypoint == - 1 )
    	        ErrorMessage ( __FUNCTION__  , "Found no waypoint to teleport bot to on level %d.",
			                                   PLEASE_INFORM, IS_FATAL, ThisRobot->pos.z );

    ThisRobot -> pos . x = ThisLevel -> AllWaypoints [ BestWaypoint ] . x + 0.5 ;
    ThisRobot -> pos . y = ThisLevel -> AllWaypoints [ BestWaypoint ] . y + 0.5 ;
    ThisRobot -> nextwaypoint = BestWaypoint ;
    ThisRobot -> lastwaypoint = BestWaypoint ;
    DebugPrintf ( 1 , "\n%s(): Final teleport target: Wp no. %d position: %f/%f on level %d.", __FUNCTION__ ,
		  BestWaypoint, ThisRobot -> pos . x , ThisRobot -> pos . y , ThisRobot -> pos . z );
}; // void TeleportToClosestWaypoint ( Enemy ThisRobot )

int TeleportToRandomWaypoint(enemy * erot, level * ShuffleLevel, char *wp_used)
{
    int wp_num = ShuffleLevel->num_waypoints;
    int testwp = MyRandom ( wp_num - 1);
    int wp;

    if ( wp_used [ testwp ] || ShuffleLevel -> AllWaypoints [ testwp ] . suppress_random_spawn) //test a random waypoint
	{
	int found = 0;
	int a;

	for ( a = (testwp == wp_num - 1 ) ? 0 : testwp + 1 ; a != testwp; a = (a >= wp_num - 1) ? 0 : a + 1)
	    { /* Test waypoints starting from the current one to the last, and back from zero */
	    if ( ! wp_used [ a ] && ! ShuffleLevel -> AllWaypoints [ a ] . suppress_random_spawn )
		{
		found = 1;
		testwp = a;
		break;
		}
	    }

	if ( ! found )
	    ErrorMessage(__FUNCTION__, "There was no free waypoint found on level %d to place another random bot.\n", PLEASE_INFORM, IS_WARNING_ONLY, ShuffleLevel->levelnum);
	}

    wp = testwp;
    wp_used [ testwp ] = 1;

    erot->pos.x = ShuffleLevel->AllWaypoints[wp].x + 0.5;
    erot->pos.y = ShuffleLevel->AllWaypoints[wp].y + 0.5;

    erot->lastwaypoint = wp;
    erot->nextwaypoint = wp;

    return wp;
}

/**
 * Enemys recover with time, just so.  This is done in this function, and
 * it is of course independent of the current framerate.
 */
void
PermanentHealRobots (void)
{
  static float time_since_last_heal=0;
#define HEAL_INTERVAL (3.0)

  //--------------------
  // For performance issues, we won't heal each robot every frame.  Instead it
  // will be enough to heal the bots every HEAL_INTERVAL seconds or something like that.
  //
  time_since_last_heal += Frame_Time() ;
  if ( time_since_last_heal < HEAL_INTERVAL ) return;
  time_since_last_heal = 0 ;

  enemy *erot;
  BROWSE_ALIVE_BOTS(erot)
      {
      if ( erot->energy < Druidmap [ erot->type ] . maxenergy )
	  erot->energy += floor(Druidmap[erot->type ] . lose_health * HEAL_INTERVAL) ;
      if ( erot->energy > Druidmap [ erot->type ] . maxenergy )
	  erot->energy = Druidmap [ erot->type ] . maxenergy ;
      }

} // void PermanentHealRobots(void)

void 
InitEnemy ( enemy * our_bot )
{
    memset(our_bot, 0, sizeof(enemy));

    int j;
    our_bot -> type = -1;
    our_bot -> pos . z = our_bot -> virt_pos . z = our_bot -> energy = 0;
    our_bot -> nextwaypoint = our_bot -> lastwaypoint =  our_bot -> homewaypoint = 0;
    our_bot -> max_distance_to_home = 0;
    our_bot -> pure_wait = 0;
    our_bot -> frozen = 0;
    our_bot -> poison_duration_left = 0;
    our_bot -> poison_damage_per_sec = 0;
    our_bot -> firewait = 0;
    our_bot -> energy = 0;
    our_bot -> SpecialForce = 0;
    our_bot -> CompletelyFixed = 0;
    our_bot -> follow_tux = 0;
    our_bot -> marker = 0;
    our_bot -> is_friendly = 0;
    our_bot -> has_been_taken_over = FALSE ;  // has the Tux made this a friendly bot via takeover subgame?
    our_bot -> attack_target_type = ATTACK_TARGET_IS_NOTHING ;
    enemy_set_reference(&our_bot->bot_target_n, &our_bot->bot_target_addr, NULL);
    our_bot -> TextVisibleTime = 0;
    our_bot -> TextToBeDisplayed = "";
    our_bot -> ammo_left = 0;

    our_bot -> animation_type = WALK_ANIMATION ;
    our_bot -> animation_phase = 0.0 ;

    our_bot -> previous_angle = 0 ;         // which angle has this robot been facing the frame before?
    our_bot -> current_angle = 0 ;          // which angle will the robot be facing now?
    our_bot -> last_phase_change = 100 ;      // when did the robot last change his (8-way-)direction of facing
    our_bot -> previous_phase = 0 ;         // which (8-way) direction did the robot face before?
    our_bot -> has_greeted_influencer = FALSE ;   // has this robot issued his first-time-see-the-Tux message?
    our_bot -> will_rush_tux = FALSE ;
    our_bot -> last_combat_step = 100 ;       // when did this robot last make a step to move in closer or farther away from Tux in combat?
    for ( j = 0 ; j < 5 ; j++ )
	{
	our_bot -> PrivatePathway [ j ] . x = -1 ;
	our_bot -> PrivatePathway [ j ] . y = -1 ;
	}

    our_bot -> time_since_previous_stuck_in_wall_check = ( (float) MyRandom ( 1000 ) ) / 1000.1 ;
    our_bot -> bot_stuck_in_wall_at_previous_check = FALSE ;
}

/* -----------------------------------------------------------------
 * This function removes all enemy entries from the list of the
 * enemys.
 * ----------------------------------------------------------------- */
void
ClearEnemys ( void )
{
    enemy * erot, *nerot;
    
    int i;
    for ( i =0 ;  i < MAX_LEVELS; i++)
	{
	INIT_LIST_HEAD(&level_bots_head[i]);
	}

    BROWSE_ALIVE_BOTS_SAFE(erot, nerot)
	{
	list_del( &erot->global_list );
	free ( erot );
	}

    BROWSE_DEAD_BOTS_SAFE(erot,nerot)
	{
	list_del( &erot->global_list );
	free ( erot );
	}

    INIT_LIST_HEAD(&alive_bots_head);
    INIT_LIST_HEAD(&dead_bots_head);
}; // void ClearEnemys ( void ) 


/* -----------------------------------------------------------------
 * Right after loading a ship (load game, start new game, etc.), 
 * we tie every enemy to a given waypoint on the map.
 * -----------------------------------------------------------------*/
void
ShuffleEnemys ( int LevelNum )
{
    int j;
    int wp_num;
    int BestWaypoint;
    Level ShuffleLevel = curShip.AllLevels[ LevelNum ];
    
    wp_num = ShuffleLevel->num_waypoints;
    char wp_used[wp_num]; //is this waypoint used?
    memset(wp_used, 0, wp_num);

    enemy *erot;
    BROWSE_LEVEL_BOTS(erot, LevelNum)
	{

	if ( erot->CompletelyFixed ) 
	    continue;

	//--------------------
	// A special force, that is not completely fixed, needs to be integrated
	// into the waypoint system:  We find the closest waypoint for it and put
	// it simply there.  For simplicity we use sum norm as distance.
	//
	TeleportToClosestWaypoint(erot);
	
	erot->combat_state = SELECT_NEW_WAYPOINT;
	erot->homewaypoint = erot->lastwaypoint;
	if ( erot->SpecialForce )
	    {

	    //--------------------
	    // For every special force, that is exactly positioned in the map anyway,
	    // we find the waypoint he's standing on.  That will be his current target
	    // and source waypoint.  That's it for special forces.
	    //
	    BestWaypoint = 0;
	    for ( j = 0 ; j < wp_num ; j ++ )
		{
		if ( fabsf ( ( ShuffleLevel -> AllWaypoints[j].x  - erot->pos.x ) *
			    ( ShuffleLevel -> AllWaypoints[j].x  - erot->pos.x ) +
			    ( ShuffleLevel -> AllWaypoints[j].y - erot->pos.y ) *
			    ( ShuffleLevel -> AllWaypoints[j].y - erot->pos.y ) ) < 
			fabsf ( ( ShuffleLevel -> AllWaypoints[ BestWaypoint ].x - erot->pos.x ) *
			    ( ShuffleLevel -> AllWaypoints[ BestWaypoint ].x - erot->pos.x ) +
			    ( ShuffleLevel -> AllWaypoints[ BestWaypoint ].y - erot->pos.y ) *
			    ( ShuffleLevel -> AllWaypoints[ BestWaypoint ].y - erot->pos.y ) ) )
		    BestWaypoint = j;
		}

	    erot->nextwaypoint = BestWaypoint;
	    erot->lastwaypoint = BestWaypoint;

	    erot->pos.x = ShuffleLevel->AllWaypoints[ BestWaypoint ].x + 0.5;
	    erot->pos.y = ShuffleLevel->AllWaypoints[ BestWaypoint ].y + 0.5;
	    if ( wp_used [ BestWaypoint ] == 1 )
		ErrorMessage ( __FUNCTION__, "Waypoint %d at %d %d  on level %d is used for a special force but open to random placement of bots. This should be fixed.\n", NO_NEED_TO_INFORM, IS_WARNING_ONLY, BestWaypoint, ShuffleLevel->AllWaypoints[ BestWaypoint ].x, ShuffleLevel->AllWaypoints[ BestWaypoint ].y, LevelNum);

	    wp_used [ BestWaypoint ] = 1;
	    continue;
	    }

	TeleportToRandomWaypoint(erot, ShuffleLevel, wp_used);
	} 

}; // void ShuffleEnemys ( void ) 

static void 
enemy_get_current_walk_target ( enemy *ThisRobot, moderately_finepoint * a )
{
int i = 0;
while ( ThisRobot->PrivatePathway [ i ] . x != (-1) )
    {
    if ( i == 5 )
	break;
    i++;
    }

if ( ! i ) i = 1;

a -> x = ThisRobot -> PrivatePathway [ i - 1 ] . x;
a -> y = ThisRobot -> PrivatePathway [ i - 1 ] . y;

if ( a -> x == - 1)
    {
    a -> x = ThisRobot->pos.x;
    a -> y = ThisRobot->pos.y;
    }
}

/**
 * This function moves one robot in an advanced way, that hasn't been
 * present within the classical paradroid game.
 */
static float 
remaining_distance_to_current_walk_target ( Enemy ThisRobot )
{
    moderately_finepoint remaining_way;

    enemy_get_current_walk_target(ThisRobot, &remaining_way);

    remaining_way.x -= ThisRobot -> pos . x ;
    remaining_way.y -= ThisRobot -> pos . y ;

    return ( sqrt ( remaining_way.x * remaining_way.x + remaining_way.y * remaining_way.y ) ) ;

}; // float remaining_distance_to_current_walk_target ( Enemy ThisRobot )



/**
 *
 *
 */
static void
DetermineAngleOfFacing ( enemy * e )
{
  //--------------------
  // The phase now depends upon the direction this robot
  // is heading.
  //
  // We calsulate the angle of the vector, but only if the robot has at least
  // some minimal speed.  If not, simply the previous angle will be used again.
  //
  if ( ( fabsf ( e->speed.y ) > 0.03 ) || ( fabsf ( e->speed.x ) > 0.03 ) )
    {
      e->current_angle = 180 - ( atan2 ( e->speed.y,  e->speed.x) * 180 / M_PI + 90 );
      e->previous_angle = e->current_angle ;
    }
  else
    {
      e->current_angle = e->previous_angle ;
    }
}; // void DetermineAngleOfFacing ( int EnemyNum )


/**
 * Once the next waypoint or the next private pathway point has been 
 * selected, this generic low_level movement function can be called to
 * actually move the robot towards this spot.
 */
static void move_enemy_to_spot ( Enemy ThisRobot, moderately_finepoint next_target_spot )
{
	moderately_finepoint remaining_way;
	float maxspeed;
	int old_map_level;

	//--------------------
	// According to properties of the robot like being frozen or not,
	// we define the maximum speed of this machine for later use...
	// A frozen robot is slow while a paralyzed robot can do absolutely nothing.
	//
	// if ( ThisRobot -> paralysation_duration_left != 0 ) return;
	
	if ( ThisRobot->frozen == 0 )
		maxspeed = Druidmap[ThisRobot->type].maxspeed;
	else 
		maxspeed = 0.2 * Druidmap[ThisRobot->type].maxspeed;
	
	//--------------------
	// While getting hit, the bot or person shouldn't be running, but
	// when standing, it should move over to the 'walk' animation type...
	//
	if ( ThisRobot->animation_type == GETHIT_ANIMATION ) return;
	if ( ThisRobot->animation_type == STAND_ANIMATION ) 
	{
		ThisRobot->animation_type = WALK_ANIMATION;
		ThisRobot->animation_phase = 0.0;
	}
	
	remaining_way.x = next_target_spot.x - ThisRobot->pos.x;
	remaining_way.y = next_target_spot.y - ThisRobot->pos.y;
	
	float squared_length = remaining_way.x * remaining_way.x + remaining_way.y * remaining_way.y;
	gps newpos = ThisRobot->pos;
	
	if ( squared_length < DIST_TO_INTERM_POINT * DIST_TO_INTERM_POINT )
	{
		ThisRobot->speed.x = 0;
		ThisRobot->speed.y = 0;
		newpos.x = next_target_spot.x;
		newpos.y = next_target_spot.y;
	}
	else
	{
		if ( ( Frame_Time() > 0.001 ) )
		{
			float length = sqrt(squared_length);
			
			ThisRobot->speed.x = maxspeed * remaining_way.x / length;
			ThisRobot->speed.y = maxspeed * remaining_way.y / length;
			if ( fabsf( ThisRobot->speed.x * Frame_Time() ) >= fabsf ( remaining_way.x) )
				ThisRobot->speed.x = remaining_way.x / Frame_Time();
			if ( fabsf( ThisRobot->speed.y * Frame_Time()) >= fabsf ( remaining_way.y ) )
				ThisRobot->speed.y = remaining_way.y / Frame_Time();
			newpos.x = ThisRobot->pos.x + ThisRobot->speed.x * Frame_Time();
			newpos.y = ThisRobot->pos.y + ThisRobot->speed.y * Frame_Time();
		}
	}
	
	//--------------------
	// Now the bot is moving, so maybe it's moving over a jump threshold?
	// In any case, it might be best to check...
	//
	// In case a jump has taken place, we best also reset the current
	// waypointless wandering target.  Otherwise the bot might want to
	// walk to the end of the map before thinking again...
	//
	
	if ( !resolve_virtual_position(&newpos, &newpos) )
		return;

	old_map_level = ThisRobot->pos.z;
	ThisRobot->pos.x = newpos.x;
	ThisRobot->pos.y = newpos.y;
	ThisRobot->pos.z = newpos.z;
	
	if ( ThisRobot->pos.z != old_map_level )
	{ /* if the bot has changed level */
		/* Prevent it from moving this frame */
		clear_out_intermediate_points ( &ThisRobot->pos, &ThisRobot->PrivatePathway[0], 5);
	
		/* Move it to the appropriate level list */
		list_move(&ThisRobot->level_list, &(level_bots_head[ThisRobot->pos.z]));
	}
	
	DetermineAngleOfFacing( ThisRobot );
	
}; // void move_enemy_to_spot ( Enemy ThisRobot , finepoint next_target_spot )

/**
 * This function moves one robot thowards his next waypoint.  If already
 * there, the function does nothing more.
 */
void 
MoveThisRobotThowardsHisCurrentTarget ( enemy * ThisRobot )
{
    if ( ThisRobot -> animation_type == ATTACK_ANIMATION ) return;

    if (( ThisRobot -> PrivatePathway [ 0 ] . x == ThisRobot -> pos . x ) &&
	    ( ThisRobot -> PrivatePathway [ 0 ] . y == ThisRobot -> pos . y ))
	{
	if ( ThisRobot -> PrivatePathway [ 1 ] . x != -1 )
	    {
	    int i;        
	    for ( i = 1; i < 5; i ++ )
		{         
		ThisRobot->PrivatePathway[i - 1].x = ThisRobot->PrivatePathway[i] . x;
		ThisRobot->PrivatePathway[i - 1].y = ThisRobot->PrivatePathway[i] . y;
		}
	    ThisRobot -> PrivatePathway [ 4 ] . x = -1;
	    ThisRobot -> PrivatePathway [ 4 ] . y = -1;
	    }
	return;
	}

    move_enemy_to_spot ( ThisRobot , ThisRobot->PrivatePathway[0] );

    if ((fabsf( ThisRobot -> pos .x - ThisRobot->PrivatePathway[0] . x ) < 0.001)
	&& fabsf( ThisRobot->pos.y - ThisRobot->PrivatePathway[0] . y ) < 0.001)
	{ /* Have we reached our target ?*/
	int i;
	for ( i = 1; i < 5; i ++ )
	    {
	    ThisRobot->PrivatePathway[i - 1].x = ThisRobot->PrivatePathway[i] . x;
	    ThisRobot->PrivatePathway[i - 1].y = ThisRobot->PrivatePathway[i] . y;
	    }
	ThisRobot -> PrivatePathway [ 4 ] . x = -1;
	ThisRobot -> PrivatePathway [ 4 ] . y = -1;
	}

    if ( ThisRobot->PrivatePathway[0] . x == -1 )
	{
	ThisRobot->PrivatePathway[0] . x = ThisRobot->pos.x;
	ThisRobot->PrivatePathway[0] . y = ThisRobot->pos.y;
	}


}; // void MoveThisRobotThowardsHisCurrentTarget ( int EnemyNum )

/**
 * This function sets a new random waypoint to a bot.
 *
 * Returns 0 if everything was OK, 1 if couldn't set a new waypoint.
 */
static int
SetNewRandomWaypoint ( Enemy ThisRobot )
{
    int i;
    Waypoint WpList;	
    int nextwp;
    finepoint nextwp_pos;
    waypoint *this_wp;
    int num_conn;
    int trywp = 0 ;
    int FreeWays[ MAX_WP_CONNECTIONS ];
    int SolutionFound;
    int TestConnection;
    Level WaypointLevel = curShip.AllLevels[ ThisRobot -> pos.z ];
    
    //--------------------
    // We do some definitions to save us some more typing later...
    //
    WpList = WaypointLevel->AllWaypoints;
    nextwp = ThisRobot->nextwaypoint;
    nextwp_pos.x = WpList[nextwp].x + 0.5 ;
    nextwp_pos.y = WpList[nextwp].y + 0.5 ;
    
    ThisRobot->lastwaypoint = ThisRobot->nextwaypoint;
    
    this_wp = &WpList[nextwp];
    num_conn = this_wp->num_connections;
    
    if (  num_conn == 0 ) // no connections found!
    {
        fprintf ( stderr , "\nFound a waypoint without connection\n");
	fprintf ( stderr , "\nThe offending waypoint nr. is: %d at %d, %d.", nextwp, WpList [ nextwp ] . x, WpList [ nextwp ] . y );
	fprintf ( stderr , "\nThe map level in question got nr.: %d.", ThisRobot -> pos . z );
	return 1;
    }
    
    //--------------------
    // At this point, we should check, if there is another waypoint 
    // and also if the way there is free of other droids
    //
    freeway_context frw_ctx = { TRUE, { ThisRobot, NULL } };
    
    for ( i = 0; i < num_conn ; i++ )
    {
	FreeWays [ i ] = CheckIfWayIsFreeOfDroids ( WpList [ ThisRobot -> lastwaypoint ] . x + 0.5 , 
	    WpList [ ThisRobot -> lastwaypoint ] . y + 0.5 , 
	    WpList [ WpList [ ThisRobot -> lastwaypoint ] . connections [ i ] ] . x + 0.5 , 
	    WpList [ WpList [ ThisRobot -> lastwaypoint ] . connections [ i ] ] . y + 0.5 , 
	    ThisRobot->pos.z , &frw_ctx );
    }
    
    //--------------------
    // Now see whether any way point at all is free in that sense
    // otherwise we set this robot to waiting and return;
    //
    for ( i = 0 ; i < num_conn ; i++ )
    {
	if ( FreeWays[i] ) break;
    }
    if ( i == num_conn )
    {
	DebugPrintf( 2 , "\n%s(): Sorry, there seems no free way out.  I'll wait then... , num_conn was : %d ." , __FUNCTION__ , num_conn );
	ThisRobot->pure_wait = 0.5 ; 
	return 1;
    }
    
    //--------------------
    // Now that we know, there is some way out of this, we can test around
    // and around randomly until we finally find some solution.
    //
    // ThisRobot->nextwaypoint = WpList [ nextwp ] . connections [ i ] ;
    // 
    SolutionFound = FALSE;
    while ( !SolutionFound )
    {
	TestConnection = MyRandom ( WpList [ nextwp ] . num_connections - 1 );
	
	if ( WpList[nextwp].connections[ TestConnection ] == (-1) ) continue;
	if ( !FreeWays[TestConnection] ) continue;
	
	trywp = WpList[nextwp].connections[ TestConnection ];
	SolutionFound = TRUE;
    }
    
    // set new waypoint...
    ThisRobot->nextwaypoint = trywp;

    return 0;    
};

/**
 * If the droid in question is currently not following the waypoint system
 * but rather moving around on it's own and without any real destination,
 * this function sets up randomly chosen targets for the droid.
 */
static void 
set_new_waypointless_walk_target ( enemy* ThisRobot, moderately_finepoint * mt)
{
    int i ;
    moderately_finepoint target_candidate;
    int success = FALSE ;

#define MAX_RANDOM_WALK_ATTEMPTS_BEFORE_GIVING_UP 4

    for ( i = 0 ; i < MAX_RANDOM_WALK_ATTEMPTS_BEFORE_GIVING_UP ; i ++ )
	{
	//--------------------
	// We select a possible new walktarget for this bot, not too
	// far away from the current position...
	//
	target_candidate . x = ThisRobot -> pos . x + ( MyRandom ( 600 ) - 300 ) / 100 ; 
	target_candidate . y = ThisRobot -> pos . y + ( MyRandom ( 600 ) - 300 ) / 100 ; 

	}

    if ( DirectLineColldet ( ThisRobot -> pos . x , ThisRobot -> pos . y ,
		                      target_candidate . x , target_candidate . y,
		                     ThisRobot -> pos . z,
		                     &WalkablePassFilter) )
	{
	mt -> x = target_candidate . x ;
	mt -> y = target_candidate . y ;
	success = TRUE ;
	}

    if ( ! success )
	{
    	ThisRobot -> pure_wait = WAIT_COLLISION;
	}
}; // void set_new_waypointless_walk_target ( enemy* ThisRobot )


/**
 * This function tells if a given level is active in the sence that there
 * is one ore more player character on the level, so that need exists to
 * move all the enemies on this level etc.
 */
int 
IsActiveLevel ( int levelnum ) 
{
    return (Me . energy > 0 && Me . pos . z == levelnum );
}; // int IsActiveLevel ( int levelnum ) 

/**
 * When a (hostile) robot is defeated and explodes, it will drop some 
 * treasure, i.e. stuff it had or parts that it consisted of or similar
 * things.  Maybe there will even be some extra magical treasures if the
 * robot in question was a 'boss monster'.  This function does the 
 * treasure dropping.
 */
void
DropEnemyTreasure ( Enemy ThisRobot )
{

    //--------------------
    // If the Tux has the skill to extract certain components from dead bots,
    // these components will be thrown out automatically, when the bot is killed.
    //
    switch ( Me . base_skill_level [ get_program_index_with_name("Extract bot parts") ] )
	{
	case 6:
	case 5:
        if (Druidmap [ ThisRobot->type ] . amount_of_tachyon_condensators && Druidmap[ThisRobot->type] . amount_of_tachyon_condensators > MyRandom(100))
	    DropItemAt( GetItemIndexByName("Tachyon Condensator"), ThisRobot -> pos . z , 
			ThisRobot->virt_pos.x , ThisRobot->virt_pos.y , -1 , -1 , 1 );
	case 4:
        if (Druidmap [ ThisRobot->type ] . amount_of_antimatter_converters && Druidmap[ThisRobot->type] . amount_of_antimatter_converters > MyRandom(100))
	    DropItemAt( GetItemIndexByName("Antimatter-Matter Converter") , ThisRobot -> pos . z , 
			ThisRobot->virt_pos.x , ThisRobot->virt_pos.y , -1 , -1 , 1 );
	case 3:
        if (Druidmap [ ThisRobot->type ] . amount_of_superconductors && Druidmap[ThisRobot->type] . amount_of_superconductors > MyRandom(100))
	    DropItemAt( GetItemIndexByName("Superconducting Relay Unit") , ThisRobot -> pos . z , 
			ThisRobot->virt_pos.x , ThisRobot->virt_pos.y , -1 , -1 , 1 );
	case 2:
        if (Druidmap [ ThisRobot->type ] . amount_of_plasma_transistors && Druidmap[ThisRobot->type] . amount_of_plasma_transistors > MyRandom(100))
	    DropItemAt( GetItemIndexByName("Plasma Transistor") , ThisRobot -> pos . z , 
			ThisRobot->virt_pos.x , ThisRobot->virt_pos.y , -1 , -1 , 1 );
	case 1:
        if (Druidmap [ ThisRobot->type ] . amount_of_entropy_inverters && Druidmap[ThisRobot->type] . amount_of_entropy_inverters > MyRandom(100))
	    DropItemAt( GetItemIndexByName("Entropy Inverter") , ThisRobot -> pos . z , 
			ThisRobot->virt_pos.x , ThisRobot->virt_pos.y , -1 , -1 , 1 );
	case 0: break;
	}

    if ( ThisRobot -> on_death_drop_item_code != (-1) )
    {
	//--------------------
	// We make sure the item created is of a reasonable type
	//
	if ( ( ThisRobot -> on_death_drop_item_code <= 0 ) ||
	     ( ThisRobot -> on_death_drop_item_code >= Number_Of_Item_Types ) )
	{
	    DebugPrintf ( -1000 , "\n%s(): item type found: %d." , ThisRobot -> on_death_drop_item_code );
	    ErrorMessage ( __FUNCTION__  , "\
Item to be dropped (forced for this bot) is of illegal type!" ,
				       PLEASE_INFORM, IS_FATAL );
	}

	DropItemAt( ThisRobot -> on_death_drop_item_code , ThisRobot -> pos . z , 
		    ThisRobot -> virt_pos . x , 
		    ThisRobot -> virt_pos . y , -1 , -1 , 1 );
	ThisRobot -> on_death_drop_item_code = -1;
    }  
    
    //--------------------
    // Apart from the parts, that the Tux might be able to extract from the bot,
    // there is still some chance, that the enemy will have (and drop) some other
    // valuables, that the Tux can then collect afterwards.
    //
    DropRandomItem ( ThisRobot -> pos . z , ThisRobot -> virt_pos . x , ThisRobot -> virt_pos . y , Druidmap [ ThisRobot -> type ] . monster_level , FALSE );
    

}; // void DropEnemyTreasure ( Enemy ThisRobot )

/**
 *
 *
 */
int
MakeSureEnemyIsInsideHisLevel ( Enemy ThisRobot )
{
    //--------------------
    // If the enemy is outside of the current map, 
    // that's an error and needs to be correted.
    //
    if ( ( ThisRobot -> pos . x <= 0 ) || 
	 ( ThisRobot -> pos . x >= curShip . AllLevels [ ThisRobot -> pos . z ] -> xlen ) ||
	 ( ThisRobot -> pos . y <= 0 ) || 
	 ( ThisRobot -> pos . y >= curShip . AllLevels [ ThisRobot -> pos . z ] -> ylen ) )
    {
	
	ErrorMessage ( __FUNCTION__  , "\
There was a droid found outside the bounds of this level (when dying).\n\
This is an error and should not occur, but most likely it does since\n\
the bots are allowed some motion without respect to existing waypoints\n\
in Freedroid RPG.\n",
				   PLEASE_INFORM, IS_FATAL );
	return ( FALSE );
    }

    return ( TRUE );

}; // int MakeSureEnemyIsInsideThisLevel ( int Enum )

/**
 * When an enemy is his, this causes some blood to be sprayed on the floor.
 * The blood is just an obstacle (several types of blood exist) with 
 * preput flag set, so that the Tux and everyone can really step *on* the
 * blood.
 *
 * Blood will always be sprayed, but there is a toggle available for making
 * the blood visible/invisible for more a children-friendly version of the
 * game.
 *
 * This function does the blood spraying (adding of these obstacles).
 */
static void enemy_spray_blood ( enemy *CurEnemy ) 
{
    extern obstacle *action_create_obstacle(level*, double, double, int);
    moderately_finepoint target_pos = { 1.0 , 0 } ;

    RotateVectorByAngle ( & target_pos , MyRandom ( 360 ) );

    target_pos . x += CurEnemy -> virt_pos . x ;
    target_pos . y += CurEnemy -> virt_pos . y ;


    if ( Druidmap [ CurEnemy -> type ] . is_human )
	action_create_obstacle (curShip.AllLevels[CurEnemy->pos.z], target_pos.x, target_pos.y,  ISO_BLOOD_1 + MyRandom (7));
    else  
	action_create_obstacle (curShip.AllLevels[CurEnemy->pos.z], target_pos.x, target_pos.y,  ISO_OIL_STAINS_1 + MyRandom (7));

}; // void enemy_spray_blood ( Enemy CurEnemy ) 




/**
 * When a robot has reached energy <= 1, then this robot will explode and
 * die, lose some treasure and add up to the kill record of the Tux.  All
 * the things that should happen when energy is that low are handled here
 * while the check for low energy is done outside of this function namely
 * somewhere in the movement processing for this enemy.
 */
static int kill_enemy(enemy * target, char givexp, int killertype)
{
    char game_message_text [ 300 ] ;
    int reward = 0;

    /* Give death message */
    if ( givexp ) 
	{
	reward = Druidmap [ target -> type ] . experience_reward;
	Me . Experience += reward;
	}

    if ( target->is_friendly )
	{
	if ( killertype && killertype != -1) //killed by someone else, and we know who it is
	    sprintf ( game_message_text , _("Your friend %s was killed by %s."), Druidmap [ target -> type ] . druidname, Druidmap [ killertype ] . druidname );
	else if ( ! killertype )
	    sprintf ( game_message_text, _("You killed %s."), Druidmap [ target -> type ] . druidname ); 
	else
	    sprintf ( game_message_text, _("%s is dead."), Druidmap [ target -> type ] . druidname ); 
	}
    else
	{
	if ( givexp )
	    sprintf ( game_message_text , _("For defeating %s, you receive %d experience.") , Druidmap [ target -> type ] . druidname,  reward );
	else 
	    if ( killertype && killertype != -1) 
		sprintf(game_message_text , _("%s was killed by %s."), Druidmap [ target -> type ] . druidname, Druidmap [ killertype ] . druidname);
	    else sprintf(game_message_text, _("%s died."), Druidmap [ target -> type ] . druidname);
	}

    append_new_game_message ( game_message_text );

    if ( MakeSureEnemyIsInsideHisLevel ( target ) ) 
    {
	Me . KillRecord [ target -> type ] ++ ;
	
	DropEnemyTreasure ( target ) ;


    //--------------------
    // NOTE:  We reset the animation phase to the first death animation image
    //        here.  But this may be WRONG!  In the case that the enemy graphics
    //        hasn't been loaded yet, this will result in '1' for the animation
    //        phase.  That however is unlikely to happen unless the bot is killed
    //        right now and hasn't been ever visible in the game yet.  Also it
    //        will lead only to minor 'prior animation' before the real death
    //        phase is reached and so serious bugs other than that, so I think it 
    //        will be tolerable this way.
    //
	    target -> animation_phase = ( ( float ) first_death_animation_image [ target -> type ] ) - 1 + 0.1 ;
	    target -> animation_type = DEATH_ANIMATION;
	    play_death_sound_for_bot ( target );
	    DebugPrintf ( 1 , "\n%s(): playing death sound because bot of type %d really died." , 
			  __FUNCTION__ , target -> type );

        if ( MyRandom(15) == 1) 	    
		enemy_spray_blood ( target ) ;
    }

    list_move(&(target->global_list), &dead_bots_head); // bot is dead? move it to dead list
    list_del(&(target->level_list)); // bot is dead? remove it from level list

    return 0;
};

/**
 *
 *
 */
static void start_gethit_animation ( enemy* ThisRobot ) 
{
    //--------------------
    // Maybe this robot is fully animated.  In this case, after getting
    // hit, the gethit animation should be displayed, which we'll initiate here.
    //
    if ( ( last_gethit_animation_image [ ThisRobot -> type ] - first_gethit_animation_image [ ThisRobot -> type ] > 0 ) )
	{
	if ( ( ThisRobot -> animation_type == DEATH_ANIMATION ) )
	    {
	    DebugPrintf ( -4 , "\n%s(): WARNING: animation phase reset for INFOUT bot... " , __FUNCTION__ );
	    }
	ThisRobot -> animation_phase = ((float)first_gethit_animation_image [ ThisRobot -> type ]) + 0.1 ;
	ThisRobot -> animation_type = GETHIT_ANIMATION;
	}

}; // void start_gethit_animation_if_applicable ( enemy* ThisRobot ) 


/*
 *  Hit an enemy for "hit" HP. This is supposed to be the *only* means 
 *  of removing HPs to a bot.
 *
 *  target is a pointer to the bot to hit
 *  hit is the amount of HPs to remove
 *  givexp (0 or 1) indicates whether to give an XP reward to the player or not
 *  killertype is the type of the bot who is responsible of the attack, or -1 if it is unknown
 *  or if it is the player.
 *  mine is 0 or 1 depending on whether it's the player who is responsible for the attack
 */
void
hit_enemy ( enemy * target, float hit, char givexp, short int killertype, char mine)
{
    /*
     * turn group hostile
     * spray blood
     * enter hitstun
     * say a funny message
     * remove hp
     * check if droid is dead
     */
    
    if ( mine ) // tux hit ? we turn the group hostile !
	{
	    robot_group_turn_hostile ( target );
	}

    // no XP is given for killing a friendly bot
    if ( target->is_friendly && givexp )
	givexp = 0;

    // spray blood
    if ( hit > 1 && MyRandom(6) == 1 )
	enemy_spray_blood ( target ) ;
   
    // hitstun
	// a hit that does less than 5% (over max life) damage cannot stun a bot
	if ( hit / Druidmap[target->type].maxenergy >= 0.05 ) {
		start_gethit_animation (target) ;

	    // if the current wait time of the bot is greater than the hitstun duration, we do nothing
		if(target -> firewait < Druidmap [ target -> type ] . recover_time_after_getting_hit) {
			target -> firewait = Druidmap [ target -> type ] . recover_time_after_getting_hit ;
		}
    }

    EnemyHitByBulletText( target );

    PlayEnemyGotHitSound ( Druidmap [ target->type ] . got_hit_sound_type );

    target -> energy -= hit;
    if ( target -> energy <= 0 )
	{
	kill_enemy(target, givexp, killertype);
	}
}


/**
 * This function moves a single enemy.  It is used by MoveEnemys().
 */
void 
MoveThisEnemy( enemy * ThisRobot )
{
    //--------------------
    // robots that still have to wait also do not need to
    // be processed for movement
    //
    if ( ThisRobot -> pure_wait > 0 ) return;
    
    gps oldpos = { ThisRobot->pos.x, ThisRobot->pos.y, ThisRobot->pos.z };

    MoveThisRobotThowardsHisCurrentTarget( ThisRobot );
    
    if ( CheckEnemyEnemyCollision ( ThisRobot ) )
	{
	ThisRobot -> pos . x = oldpos . x ;
	ThisRobot -> pos . y = oldpos . y;
	ThisRobot -> pos . z = oldpos . z;
	}
}; 


/**
 * This function returns a gps (position) for a robot's current target, 
 * or NULL if such target doesn't exist or is dead or is invisible (if target is Tux)
 */
static gps * enemy_get_target_position ( enemy *ThisRobot )
{
	if ( ( ThisRobot -> attack_target_type == ATTACK_TARGET_IS_PLAYER ) )
	{
		if ( Me.invisible_duration > 0 ) return NULL;
		return & Me . pos;
	}
	else if ( ThisRobot -> attack_target_type == ATTACK_TARGET_IS_ENEMY ) 
	{
		enemy* bot_enemy = enemy_resolve_address(ThisRobot->bot_target_n, &ThisRobot->bot_target_addr);
		if ( !bot_enemy || bot_enemy->energy <= 0 ) return NULL;
		return &bot_enemy->pos;
	}

	/* No (more) target */
	return NULL;
}

/**
 * More for debugging purposes, we print out the current state of the
 * robot as his in-game text.
 */
void
enemy_say_current_state_on_screen ( enemy* ThisRobot )
{
    if ( ThisRobot->pure_wait <= 0 )
    {
		switch ( ThisRobot -> combat_state )
		{                  
		case MOVE_ALONG_RANDOM_WAYPOINTS:
			ThisRobot->TextToBeDisplayed = ("state:  Wandering along waypoints.") ;
			break;     
		case SELECT_NEW_WAYPOINT:
			ThisRobot->TextToBeDisplayed = ("state: Select next WP.");
			break;
		case TURN_TOWARDS_NEXT_WAYPOINT:
			ThisRobot->TextToBeDisplayed = ("state:  Turn towards next WP.") ;
			break;     
		case RUSH_TUX_AND_OPEN_TALK:
			ThisRobot->TextToBeDisplayed = ("state:  Rush Tux and open talk.") ;
			break;     
		case STOP_AND_EYE_TARGET:
			ThisRobot->TextToBeDisplayed = ("state:  Stop and eye target.") ;
			break;     
		case ATTACK:
			ThisRobot->TextToBeDisplayed = ("state:  Attack.") ;
			break;     
		case RETURNING_HOME:
			ThisRobot->TextToBeDisplayed = ("state:  Returning home.") ;
			break;
		case WAYPOINTLESS_WANDERING:
			ThisRobot->TextToBeDisplayed = ("state:  Waypointless wandering.") ;
			break;
		case PARALYZED:
			ThisRobot->TextToBeDisplayed = ("state:  Paralyzed.") ;
			break;
		case COMPLETELY_FIXED:
			ThisRobot->TextToBeDisplayed = ("state: Completely fixed.");
			break;
		case FOLLOW_TUX:
			ThisRobot->TextToBeDisplayed = ("state: Follow Tux.") ;
			break;
		case UNDEFINED_STATE:
			ThisRobot->TextToBeDisplayed = ("state: None.") ;
			break;
		default:
			ThisRobot->TextToBeDisplayed = ("state:  UNHANDLED!!") ;
			break;
		}
    }
    else
    {
#if 1    	
    	ThisRobot->TextToBeDisplayed = ("purewait");
#else
    	switch ( ThisRobot -> combat_state )
    	{                  
    	case MOVE_ALONG_RANDOM_WAYPOINTS:
    	    ThisRobot->TextToBeDisplayed = ("purewait (MARW)");
    	    break;     
    	case SELECT_NEW_WAYPOINT:
    	    ThisRobot->TextToBeDisplayed = ("purewait (SNW)");
    	    break;
    	case TURN_TOWARDS_NEXT_WAYPOINT:
    	    ThisRobot->TextToBeDisplayed = ("purewait (TTNW)");
    	    break;     
    	case RUSH_TUX_AND_OPEN_TALK:
    	    ThisRobot->TextToBeDisplayed = ("purewait (RTAOT)");
    	    break;     
    	case STOP_AND_EYE_TARGET:
    	    ThisRobot->TextToBeDisplayed = ("purewait (SAET)");
    	    break;     
    	case ATTACK:
    	    ThisRobot->TextToBeDisplayed = ("purewait (A)");
    	    break;     
    	case RETURNING_HOME:
    	    ThisRobot->TextToBeDisplayed = ("purewait (RH)");
    	    break;
    	case WAYPOINTLESS_WANDERING:
    	    ThisRobot->TextToBeDisplayed = ("purewait (WW)");
    	    break;
    	case PARALYZED:
    	    ThisRobot->TextToBeDisplayed = ("purewait (P)");
    	    break;
    	case COMPLETELY_FIXED:
    	    ThisRobot->TextToBeDisplayed = ("purewait (CF)");
    	    break;
    	case FOLLOW_TUX:
    	    ThisRobot->TextToBeDisplayed = ("purewait (FT)");
    	    break;
    	case UNDEFINED_STATE:
    	    ThisRobot->TextToBeDisplayed = ("purewait (US)");
    	    break;
    	default:
    	    ThisRobot->TextToBeDisplayed = ("purewait (UNH)");
    	    break;
    	}      
#endif
    }

    ThisRobot->TextVisibleTime = 0 ; 
    
}; // void enemy_say_current_state_on_screen ( enemy* ThisRobot )

/**
 * Some robots (currently) tend to get stuck in walls.  This is an 
 * annoying bug case we have not yet been able to eliminate completely.
 * To provide some safety against this case, some extra fallback handling
 * should be introduced, so that the bots can still recover if that 
 * unlucky case really happens, which is what we provide here.
 *
 * Since passability checks usually can become quite costy in terms of 
 * processor time and also because it makes sense to allow for some more
 * 'natural' fallbacks to work, we only check for stuck bots every second
 * or so.  In order to better distribute the checks (and not cause fps
 * glitches by doing them all at once) we use individual timers for this
 * test.
 */
void
enemy_handle_stuck_in_walls ( enemy* ThisRobot )
{
    //--------------------
    // Maybe the time for the next check for this bot has not yet come.
    // in that case we can return right away.
    //
    ThisRobot -> time_since_previous_stuck_in_wall_check += Frame_Time();
    if ( ThisRobot -> time_since_previous_stuck_in_wall_check < 1.0 )
	return;
    ThisRobot -> time_since_previous_stuck_in_wall_check = 0 ;
    
    //--------------------
    // First we take a look if this bot is currently stuck in a
    // wall somewhere.
    //
    if ( !SinglePointColldet ( ThisRobot -> pos . x , ThisRobot -> pos . y , ThisRobot -> pos.z, &WalkablePassFilter ) )
    {
	    //--------------------
	    // So at this point we know, that we have a bot that is stuck right now,
	    // has been stuck one second ago and also is not moving along wapoints, which
	    // would lead to the bot reaching some sensible spot sooner or later anyway.
	    // In one word:  we have arrived in a situation that might make a crude correction
	    // sensible.  We teleport the robot back to the nearest waypoint.  From there, it
	    // might find a suitable way on it's own again.
	    //	    
	    DebugPrintf ( -2 , "\n\nFound robot that seems really stuck on position: %f/%f/%d." ,
			  ThisRobot -> pos . x , ThisRobot -> pos . y , ThisRobot -> pos.z );
	    DebugPrintf ( -2 , "\nMore details on this robot:  Type=%d." ,
			  ThisRobot -> type );
	    DebugPrintf ( -2 , "\nPrivate Pathway[0]: %f/%f." , 
			  ThisRobot -> PrivatePathway [ 0 ] . x ,
			  ThisRobot -> PrivatePathway [ 0 ] . y );
	    DebugPrintf ( -2 , "\nPrivate Pathway[1]: %f/%f." , 
			  ThisRobot -> PrivatePathway [ 1 ] . x ,
			  ThisRobot -> PrivatePathway [ 1 ] . y );
	    DebugPrintf ( -2 , "\nnextwaypoint: %d at %f/%f" , 
			  ThisRobot -> nextwaypoint, curShip.AllLevels[ThisRobot->pos.z] -> AllWaypoints [ ThisRobot->nextwaypoint ] . x + 0.5,
		   curShip.AllLevels[ThisRobot->pos.z] -> AllWaypoints [ ThisRobot->nextwaypoint ] . y + 0.5 );

	    enemy_say_current_state_on_screen ( ThisRobot ); // safety:  init the TextToBeDisplayed 
	    DebugPrintf ( -2 , "\nnextwaypoint=%d. lastwaypoint=%d. combat_%s." ,
			  ThisRobot -> nextwaypoint , ThisRobot -> lastwaypoint , 
			  ThisRobot -> TextToBeDisplayed );
	    
	    if ( !EscapeFromObstacle( &(ThisRobot->pos.x), &(ThisRobot->pos.y), ThisRobot->pos.z, &WalkablePassFilter) )
	    {
	    	// No free position was found outside the obstacle ???
	    	// It should not happen but since we want the bot to escape in any situation, just have a last fallback
	    	//
	    	ThisRobot->pos.x = curShip.AllLevels[ThisRobot->pos.z]->AllWaypoints[ThisRobot->nextwaypoint].x + 0.5;
	    	ThisRobot->pos.y = curShip.AllLevels[ThisRobot->pos.z]->AllWaypoints[ThisRobot->nextwaypoint].y + 0.5;   	
	    }
	    ThisRobot -> combat_state = SELECT_NEW_WAYPOINT;
	    ThisRobot -> bot_stuck_in_wall_at_previous_check = TRUE ; 
	    return;
    }
    else
    {
	// this bot isn't currently stuck.  what more could anybody want?
	ThisRobot -> bot_stuck_in_wall_at_previous_check = FALSE ;
    }
}; // enemy_handle_stuck_in_walls ( enemy* ThisRobot )

/**
 * This function computes the distance a certain robot has with respect
 * to Tux, i.e. player 0 in the game.  If the Tux and the bot in question
 * are on the same level, then everything is pretty simple.  However, if
 * the enemy is on another level that is connected to this level via an
 * interface area, then of course we need to take more care.
 */
float
DistanceToTux ( Enemy ThisRobot )
{
    if ( ThisRobot -> pos . z == Me . pos . z )
    {
	return ( sqrt ( ( ThisRobot -> pos . x - Me . pos . x ) * 
			( ThisRobot -> pos . x - Me . pos . x ) + 
			( ThisRobot -> pos . y - Me . pos . y ) * 
			( ThisRobot -> pos . y - Me . pos . y ) ) );
    }
    else
    {
	update_virtual_position ( & ( ThisRobot -> virt_pos ) ,
				  & ( ThisRobot -> pos ) , Me . pos . z );
	if ( ThisRobot -> virt_pos . z == (-1) ) return ( 10000 );

	return ( sqrt ( ( ThisRobot -> virt_pos . x - Me . pos . x ) * 
			( ThisRobot -> virt_pos . x - Me . pos . x ) + 
			( ThisRobot -> virt_pos . y - Me . pos . y ) * 
			( ThisRobot -> virt_pos . y - Me . pos . y ) ) );
    }
	
}; // float DistanceToTux ( Enemy ThisRobot )

/**
 * This function selects a target for a friendly bot. 
 * It takes closest reachable enemy bot in view range.
 * A new target is selected at each frame. This should prevent a friendly bot to follow
 * an enemy and thus get too far away from its "steady" position.
 *
 * Note : this function does set ThisRobbot->attack_target_type, and calls enemy_set_reference()
 *        accordingly to its finding.
 */
void
update_vector_to_shot_target_for_friend ( enemy* ThisRobot )
{
	float aggression_distance = Druidmap[ThisRobot->type].aggression_distance;
	float squared_aggression_distance = aggression_distance * aggression_distance;
	float squared_best_dist;
	
	/*
	if ( ThisRobot->pure_wait > 0 )
	{	// Target was not reachable
		// We could, for example, try to find an other target
		// But we should not do that as soon as the bot is pure_waiting,
		// because it could be a very temporary state.
		// So, a "pure_waiting" counter would be needed...
	}
	*/
	
	// We set some default values, in case there isn't anything attackable
	// found below...
	ThisRobot->attack_target_type = ATTACK_TARGET_IS_NOTHING ;
	enemy_set_reference(&ThisRobot->bot_target_n, &ThisRobot->bot_target_addr, NULL);
	
	// This initialization ensures that only targets inside aggression_distance are checked.
	squared_best_dist = squared_aggression_distance;
    
	enemy *erot;
	BROWSE_LEVEL_BOTS(erot, ThisRobot->pos.z)
	{
		if ( erot->is_friendly ) continue;
		
		if ( is_potential_target(ThisRobot, &erot->pos, &squared_best_dist) )
		{
			ThisRobot->attack_target_type = ATTACK_TARGET_IS_ENEMY ;
			enemy_set_reference(&ThisRobot->bot_target_n, &ThisRobot->bot_target_addr, erot);
		}
    }
	
} // void update_vector_to_shot_target_for_friend ( ThisRobot )

/**
 * This function selects an attack target for an hostile bot.
 * Selected target is the previous target if it is still valid (see paragraph below),
 * or the closest potential target.
 *
 * For gameplay value purposes, it also performs a little hack : the target of 
 * the previous frame can be selected even if it is "slightly" out of view (2 times the range),
 * in order to simulate "pursuit". Sorry for the mess but there is no other proper place for that.
 *
 * Note : this function does set this_robbot->attack_target_type, and calls enemy_set_reference()
 *        accordingly to its finding.
 */
void
update_vector_to_shot_target_for_enemy ( enemy* this_robot )
{
	int our_level = this_robot->pos.z ;
	float squared_best_dist;
	float xdist, ydist;
	float aggression_distance = Druidmap[this_robot->type].aggression_distance;
	float squared_aggression_distance = aggression_distance * aggression_distance;

	//--------------------
	// Check validity of old target (if any)
	//
	// Logic : 
	// A target is valid if :
	// 1- always available (not dead and not invisible)
	// 2- was reachable (this_robot was not blocked along its way to the target)
	// 3- is not too far
	// 
	gps* tpos = enemy_get_target_position(this_robot);
		
	if ( !tpos ) 
	{	// Old target is no more available
		this_robot->attack_target_type = ATTACK_TARGET_IS_NOTHING;
		enemy_set_reference(&this_robot->bot_target_n, &this_robot->bot_target_addr, NULL);
	}
	/*
	else if ( this_robot->pure_wait > 0 )
	{	// Target was not reachable
		// We could, for example, try to find an other target
		// But we should not do that as soon as the bot is pure_waiting,
		// because it could be a very temporary state.
		// So, a "pure_waiting" counter would be needed...
	}
	*/
	else if ( this_robot->attack_target_type != ATTACK_TARGET_IS_NOTHING )
	{
		// Check virtual distance
		update_virtual_position(&this_robot->virt_pos, &this_robot->pos, tpos->z);
		if ( this_robot->virt_pos . z  == -1 )
		{ // Target is at more than one level away
			this_robot->attack_target_type = ATTACK_TARGET_IS_NOTHING;
			enemy_set_reference(&this_robot->bot_target_n, &this_robot->bot_target_addr, NULL);
		}
		else
		{
			xdist = tpos->x - this_robot->virt_pos.x;
			ydist = tpos->y - this_robot->virt_pos.y;
	
			if ( (xdist * xdist + ydist * ydist) > 4.0 * squared_aggression_distance )
			{	// Old target is too far
				this_robot->attack_target_type = ATTACK_TARGET_IS_NOTHING;
				enemy_set_reference(&this_robot->bot_target_n, &this_robot->bot_target_addr, NULL);
			}
			else
			{
				// All tests passed : Continue with same target
				return;
			}
		}
	}

	//--------------------
	// Search for a new target
	//
	
	// This initialization ensures that only targets inside aggression_distance are checked.
	squared_best_dist = squared_aggression_distance;

	// First check if Tux is a potential target
	if ( Me . invisible_duration <= 0 )
	{
		if ( is_potential_target(this_robot, &Me.pos, &squared_best_dist) )
		{
			this_robot -> attack_target_type = ATTACK_TARGET_IS_PLAYER ;
		}
	}

	// But maybe there is a friend of the Tux also close.  Then maybe we
	// should attack this one instead, since it's much closer anyway.
	enemy *erot;
	BROWSE_LEVEL_BOTS(erot, our_level)
	{
		if ( !erot->is_friendly ) continue;

		if ( is_potential_target(this_robot, &erot->pos, &squared_best_dist) )
		{
			this_robot -> attack_target_type = ATTACK_TARGET_IS_ENEMY ;
			enemy_set_reference(&this_robot->bot_target_n, &this_robot->bot_target_addr, erot);			
		}	
	}

} // void update_vector_to_shot_target_for_enemy ( ThisRobot )

/**
 * This function handles the inconditional updates done to the bots by
 * the automaton powering them. See update_enemy().
 */
static void state_machine_inconditional_updates ( enemy * ThisRobot )
{
    // Robots that are paralyzed are completely stuck and do not 
    // see their state machine running
    
    //--------------------
    // For debugging purposes we display the current state of the robot
    // in game
    enemy_say_current_state_on_screen ( ThisRobot );

    //--------------------
    // we check whether the current robot is 
    // stuck inside a wall or something...
    //
    enemy_handle_stuck_in_walls ( ThisRobot );

    //--------------------
    // determine the distance vector to the target of this shot.  The target
    // depends of course on wheter it's a friendly device or a hostile device.
    //
    if ( ThisRobot->is_friendly )
	{
    	update_vector_to_shot_target_for_friend ( ThisRobot ) ;
	}
    else
	{
    	update_vector_to_shot_target_for_enemy ( ThisRobot ) ;
	}
 
    ThisRobot->speed.x=0;
    ThisRobot->speed.y=0;

}

/**
 * This function handles state transitions based solely (or almost) on 
 * the external situation and not on the current state of the bot. 
 * The purpose is to reduce code duplication in the "big switch" that follows.
 */
static void state_machine_situational_transitions ( enemy * ThisRobot )
{
    /* The various situations are listed in increasing priority order (ie. they may override each other, so the least priority comes first. */
    /* In an ideal world, this would not exist and be done for each state. But we're in reality and have to limit code duplication. */

    /* Rush Tux when he's close */    
    update_virtual_position(&ThisRobot->virt_pos, &ThisRobot->pos, Me . pos . z); 
    if ( ThisRobot -> will_rush_tux && ThisRobot->virt_pos . z != -1 &&  ( powf ( Me . pos . x - ThisRobot -> virt_pos . x , 2 ) + powf ( Me . pos . y - ThisRobot -> virt_pos . y , 2 ) ) < 16 )
	{
	ThisRobot -> combat_state = RUSH_TUX_AND_OPEN_TALK ;
	}

    /* Return home if we're too far away */
    if ( ThisRobot -> max_distance_to_home != 0 &&
	    sqrt( powf( (curShip.AllLevels[ThisRobot->pos.z]->AllWaypoints[ThisRobot->homewaypoint].x + 0.5) - ThisRobot->pos.x, 2) +
		powf( (curShip.AllLevels[ThisRobot->pos.z]->AllWaypoints[ThisRobot->homewaypoint].y + 0.5) - ThisRobot->pos.y, 2))
	    > ThisRobot->max_distance_to_home )
	{
    	ThisRobot->combat_state = RETURNING_HOME;
    	ThisRobot->attack_target_type = ATTACK_TARGET_IS_NOTHING;
    	enemy_set_reference(&ThisRobot->bot_target_n, &ThisRobot->bot_target_addr, NULL);
	}

    /* Paralyze if appropriate */
    if ( ThisRobot -> paralysation_duration_left > 0 )
	{
	ThisRobot -> combat_state = PARALYZED;
	}


    /* Now we will handle changes, which take place for many - but not exactly all - states */
    /* Those are all related to attack behavior */

    switch ( ThisRobot -> combat_state )
	{ /* Get out for all states we don't want to handle attack for */
	case STOP_AND_EYE_TARGET:
	case ATTACK:
	case PARALYZED:
	case RETURNING_HOME:
	case SELECT_NEW_WAYPOINT:
	case RUSH_TUX_AND_OPEN_TALK:
	    return;
	}
    
    /* Fix completely if appropriate */
    if ( ThisRobot -> CompletelyFixed )
	{
	ThisRobot -> combat_state = COMPLETELY_FIXED;
	}
    
    /* Follow Tux if appropriate */
    if ( ThisRobot -> follow_tux )
	{
	ThisRobot -> combat_state = FOLLOW_TUX;
	}

    /* Switch to stop_and_eye_target if appropriate - it's the prelude to any-on-any attacks */
    if ( ThisRobot->attack_target_type != ATTACK_TARGET_IS_NOTHING )
	{
	    ThisRobot -> combat_state = STOP_AND_EYE_TARGET;
	}

    /* If combat_state is always UNDEFINED_STATE, go to SELECT_NEW_WAYPOINT state */
    if ( ThisRobot->combat_state == UNDEFINED_STATE )
    {
    	ThisRobot->combat_state = SELECT_NEW_WAYPOINT;
    }
}

/* ----------------------------------------------------------
 * "stop and eye tux" state handling 
 * ---------------------------------------------------------- */
static void state_machine_stop_and_eye_target ( enemy * ThisRobot, moderately_finepoint * new_move_target )
{
	gps * tpos = enemy_get_target_position( ThisRobot );

	// Target no more available -> going to default state
	if ( !tpos )
	{
		ThisRobot -> state_timeout = 0;
		ThisRobot -> combat_state = UNDEFINED_STATE;
		ThisRobot->attack_target_type = ATTACK_TARGET_IS_NOTHING;
		enemy_set_reference(&ThisRobot->bot_target_n, &ThisRobot->bot_target_addr, NULL);
		return;
	}

	update_virtual_position(&ThisRobot->virt_pos, &ThisRobot->pos, tpos->z);

	/* Make sure we're looking at the target */
	/*XXX won't work if tpos and ThisRobot are not on the same level*/
	TurnABitTowardsPosition ( ThisRobot , tpos -> x , tpos -> y , 120 );

	/* Do greet sound if not already done */
	if ( ThisRobot -> has_greeted_influencer == FALSE )
	{
		ThisRobot->has_greeted_influencer = TRUE;
		if ( Druidmap [ ThisRobot -> type ] . greeting_sound_type != (-1) )
		{
			PlayGreetingSound ( Druidmap[ ThisRobot -> type ] . greeting_sound_type );
		}
	}

	/* Check state timeout */

	//--------------------
	// After some time, we'll no longer eye the Tux but rather do something,
	// like attack the Tux or maybe also return to 'normal' operation and do
	// nothing.  When Tux is still visible at timeout, then it will be attacked... otherwise
	// the robot resumes normal operation...
	//
	ThisRobot -> state_timeout += Frame_Time();
	if ( ThisRobot -> state_timeout > Druidmap [ ThisRobot -> type ] . time_spent_eyeing_tux ) 
	{
		ThisRobot -> state_timeout = 0;
		if ( !ThisRobot -> attack_run_only_when_direct_line || DirectLineColldet ( ThisRobot->pos.x , ThisRobot->pos.y, tpos->x , tpos->y , ThisRobot->pos.z, NULL ) )
		{
			SetRestOfGroupToState ( ThisRobot , ATTACK );
			ThisRobot -> combat_state = ATTACK ;
			ThisRobot->last_combat_step = 1.0; // So that attack will start immediately
			if ( Druidmap [ ThisRobot -> type ] . greeting_sound_type != (-1))
			{
				play_enter_attack_run_state_sound ( Druidmap[ ThisRobot->type ].greeting_sound_type );
			}
		}
		else
		{
			ThisRobot -> combat_state = UNDEFINED_STATE;
			ThisRobot->attack_target_type = ATTACK_TARGET_IS_NOTHING;
			enemy_set_reference(&ThisRobot->bot_target_n, &ThisRobot->bot_target_addr, NULL);
		}
	}
}

/* ---------------------------------
 * "attack tux" state
 * 
 * This fonction will compute the destination position of a bot in order
 * to reach its target. In the callee (update_ennemy), the pathfinder is 
 * called, to define the path of the bot up to its target.
 * 
 * This fonction will also eventually start a shoot.
 * 
 * --------------------------------- */

/*
 * Note concerning the modification of pathfinder_context :
 * 
 * When we ask the pathfinder to find a path between two points, the pathfinder 
 * will, by default, check if no bots are blocking the path.
 * Let consider the following case : an attacking bot (B1) with a melee weapon 
 * is at the entrance of a corridor. Its target (T) is inside the corridor, and an 
 * other bot (B2) is also inside the corridor :
 *       ---------------------
 *   B1                 B2 T
 *       ---------------------
 * In this case, the pathfinder will not be able to find a way between B1 and T.
 * B1 will thus not move.
 * 
 * However, if B1 were able to go just behind B2, it could be near enough to its
 * target to shoot it :
 *       ---------------------
 *                   B1 B2 T
 *       ---------------------
 * Such behavior will make B1 be as aggressive as possible.
 * We will only use it with friendly bots, so that the game is not too hard.
 * 
 * To implement this behavior, we have to tell the pathfinder not to check
 * collisions with bots. This is implemented in ReachMeleeCombat(). 
 */
static void state_machine_attack(enemy * ThisRobot, moderately_finepoint * new_move_target, pathfinder_context *pf_ctx )
{
	// Not yet time to computer a new bot's move, or to start a new shoot
	if ( ThisRobot->firewait && ThisRobot->last_combat_step < ATTACK_MOVE_RATE ) return;
	
	// Get old target's position
	gps * tpos = enemy_get_target_position(ThisRobot);
    
	// Target no more available -> going to default state
	if ( !tpos )
	{
		ThisRobot->combat_state = UNDEFINED_STATE;
		ThisRobot->attack_target_type = ATTACK_TARGET_IS_NOTHING;
		enemy_set_reference(&ThisRobot->bot_target_n, &ThisRobot->bot_target_addr, NULL);
		return;
	}

	// In case the target is on another level, evaluate the virtual position of the bot
	// in the target's level
	update_virtual_position( &ThisRobot->virt_pos, &ThisRobot->pos, tpos->z );

	//--------------------
	// First compute the type of the bot's move, and if the target can be shot,
	// depending on the weapon's type and the distance to the target
	//
	gps move_pos = { tpos->x, tpos->y, tpos->z };
	
	enum {
		NO_MOVE,
		REACH_MELEE,
		MOVE_MELEE,
		MOVE_AWAY
	} move_type = NO_MOVE;
	
	int shoot_target = FALSE;
	
	// The bot and its target are on different levels. 
	if ( ThisRobot->pos.z != move_pos.z )
	{
		// Before to start the attack, the bot has to reach its target's level.
		// Hence, compute virtual position to reach, and wait until the bot is at
		// the right level.
		update_virtual_position( &move_pos, tpos, ThisRobot->pos.z );
		move_type = REACH_MELEE;
		shoot_target = FALSE;
		goto EXECUTE_ATTACK;
	}

	int melee_weapon = ItemMap[ Druidmap[ThisRobot->type].weapon_item.type ].item_weapon_is_melee;

	if ( melee_weapon )
	{
		//-----
		// Check visibility
		int target_visible = DirectLineColldet ( ThisRobot->virt_pos.x, ThisRobot->virt_pos.y, move_pos.x, move_pos.y, move_pos.z, &WalkablePassFilter);
		
		if ( !target_visible ) 
		{
			move_type = REACH_MELEE;
			shoot_target = FALSE;
			goto EXECUTE_ATTACK;
		}

		//-----
		// Check distance		
		float dist2 = (ThisRobot->virt_pos.x - move_pos.x) * (ThisRobot->virt_pos.x - move_pos.x) + (ThisRobot->virt_pos.y - move_pos.y) * (ThisRobot->virt_pos.y - move_pos.y);
		
		if ( dist2 > SQUARED_MELEE_APPROACH_DIST )
		{
			move_type = REACH_MELEE;
			shoot_target = FALSE;
			goto EXECUTE_ATTACK;
		}

		if ( dist2 > SQUARED_MELEE_MAX_DIST ) 
		{   // Approaching the target -> find a place near the bot
			move_type = MOVE_MELEE;
			shoot_target = FALSE;
			goto EXECUTE_ATTACK;
		}
		
		// All tests passed, the bot can shot.
		shoot_target = TRUE;
		goto EXECUTE_ATTACK;
	}
	
	else // Range weapon

	{
		//-----
		// Check visibility			
		int target_visible = DirectLineColldet ( ThisRobot->virt_pos.x, ThisRobot->virt_pos.y, move_pos.x, move_pos.y, move_pos.z, &FlyablePassFilter);
		
		if ( !target_visible ) 
		{
			move_type = REACH_MELEE;
			shoot_target = FALSE;
			goto EXECUTE_ATTACK;
		}
		
		//-----
		// Check distance
		float dist2 = (ThisRobot->virt_pos.x - move_pos.x) * (ThisRobot->virt_pos.x - move_pos.x) + (ThisRobot->virt_pos.y - move_pos.y) * (ThisRobot->virt_pos.y - move_pos.y);
		
		if ( dist2 < SQUARED_RANGE_SHOOT_MIN_DIST )
		{   // Too close -> move away, and start shooting
			move_type = MOVE_AWAY;
			shoot_target = TRUE;
			goto EXECUTE_ATTACK;
		}
		
		// Check if outside of bullet range
		itemspec* bot_weapon = &ItemMap[Druidmap[ThisRobot->type].weapon_item.type];
		float shot_range = bot_weapon->item_gun_bullet_lifetime * bot_weapon->item_gun_speed;
		float squared_shot_range = shot_range * shot_range;
			
		if ( dist2 >= squared_shot_range )
		{
			move_type = REACH_MELEE;
			shoot_target = FALSE;
			goto EXECUTE_ATTACK;
		}
			
		// All tests passed, the bot can shoot.
		shoot_target = TRUE;
		goto EXECUTE_ATTACK;

	}

EXECUTE_ATTACK:

	//--------------------
	// Execute the bot's move
	//
	// We will often have to move towards our target.
	// But this moving around can lead to jittering of droids moving back and 
	// forth between two positions very rapidly.  Therefore we will not do this
	// movement thing every frame, but rather only sometimes
	//

	if ( ThisRobot -> last_combat_step >= ATTACK_MOVE_RATE )
	{
		ThisRobot -> last_combat_step = 0 ; 

		switch (move_type)
		{
		case REACH_MELEE:
			ReachMeleeCombat( ThisRobot, &move_pos, new_move_target, pf_ctx );
			break;
		case MOVE_MELEE:
			MoveToMeleeCombat( ThisRobot , &move_pos, new_move_target );
			break;
		case MOVE_AWAY:
			MoveAwayFromMeleeCombat( ThisRobot , new_move_target );
			break;
		default:
			break;
		}
	}	
	else
	{
		ThisRobot -> last_combat_step += Frame_Time ();
	}
	
	//---------------
	// Execute the bot's shoot
	
	/* Great suggestion of Sarayan : we do not care about friendly fire, and make bullets go through people of the same side. */
	if ( shoot_target && !ThisRobot->firewait )
		RawStartEnemysShot( ThisRobot , move_pos.x - ThisRobot->virt_pos.x, move_pos.y - ThisRobot->virt_pos.y);
	
}


static void state_machine_paralyzed(enemy * ThisRobot, moderately_finepoint * new_move_target)
{
    /* Move target - none */
    new_move_target -> x = ThisRobot -> pos . x ;
    new_move_target -> y = ThisRobot -> pos . y ;

    if ( ThisRobot -> paralysation_duration_left <= 0 )
	ThisRobot -> combat_state = SELECT_NEW_WAYPOINT;
}


static void state_machine_returning_home(enemy * ThisRobot, moderately_finepoint * new_move_target)
{
    /* Bot too far away from home must go back to home waypoint */

    /* Move target */
    new_move_target -> x =  curShip.AllLevels[ThisRobot->pos.z]->AllWaypoints[ThisRobot->homewaypoint].x + 0.5;
    new_move_target -> y =  curShip.AllLevels[ThisRobot->pos.z]->AllWaypoints[ThisRobot->homewaypoint].y + 0.5;

    /* Action */
    if ( remaining_distance_to_current_walk_target ( ThisRobot ) < ThisRobot->max_distance_to_home / 2.0 ) 
	{
	ThisRobot -> combat_state = SELECT_NEW_WAYPOINT;
	return;
	}

    ThisRobot -> combat_state = RETURNING_HOME;
}


static void state_machine_select_new_waypoint(enemy * ThisRobot, moderately_finepoint * new_move_target)
{
    /* Move target - none */
    new_move_target -> x = ThisRobot -> pos . x ;
    new_move_target -> y = ThisRobot -> pos . y ;
    
    /* Bot must select a new waypoint randomly, and turn towards it. No move this step.*/
    if ( SetNewRandomWaypoint( ThisRobot ) )
	{ /* couldn't find a waypoint ? go waypointless  */
	ThisRobot->combat_state = WAYPOINTLESS_WANDERING;
	}

    ThisRobot->combat_state = TURN_TOWARDS_NEXT_WAYPOINT;
}


static void state_machine_turn_towards_next_waypoint(enemy * ThisRobot, moderately_finepoint * new_move_target)
{
    /* Action */
    /* XXX */
    new_move_target -> x = ThisRobot -> pos . x;
    new_move_target -> y = ThisRobot -> pos . y;
    ThisRobot -> last_phase_change = 100 ; 

    if ( 
	    TurnABitTowardsPosition ( ThisRobot , 
		curShip . AllLevels [ ThisRobot-> pos . z ] -> AllWaypoints [ ThisRobot -> nextwaypoint ] . x + 0.5, 
		curShip . AllLevels [ ThisRobot-> pos . z ] -> AllWaypoints [ ThisRobot -> nextwaypoint ] . y + 0.5,
		90 )
       )
	{
	new_move_target -> x = curShip . AllLevels [ ThisRobot-> pos . z ] -> AllWaypoints [ ThisRobot -> nextwaypoint ] . x + 0.5;
	new_move_target -> y = curShip . AllLevels [ ThisRobot-> pos . z ] -> AllWaypoints [ ThisRobot -> nextwaypoint ] . y + 0.5;
	ThisRobot -> combat_state = MOVE_ALONG_RANDOM_WAYPOINTS ;
	}
}

static void state_machine_move_along_random_waypoints(enemy * ThisRobot, moderately_finepoint * new_move_target)
{
    /* The bot moves towards its next waypoint */

    /* Move target */
    new_move_target -> x = curShip . AllLevels [ ThisRobot-> pos . z ] -> AllWaypoints [ ThisRobot -> nextwaypoint ] . x + 0.5;
    new_move_target -> y = curShip . AllLevels [ ThisRobot-> pos . z ] -> AllWaypoints [ ThisRobot -> nextwaypoint ] . y + 0.5;

    /* Action */
    if ((new_move_target -> x - ThisRobot->pos.x) * (new_move_target -> x - ThisRobot->pos.x) + (new_move_target -> y - ThisRobot -> pos.y ) * (new_move_target -> y - ThisRobot->pos.y) < 0.3 )
	{
	ThisRobot -> combat_state = SELECT_NEW_WAYPOINT ;
	return;
	}

    ThisRobot -> combat_state = MOVE_ALONG_RANDOM_WAYPOINTS;
}

static void state_machine_rush_tux_and_open_talk(enemy * ThisRobot, moderately_finepoint * new_move_target)
{
    /* Move target */
    if ( ThisRobot -> pos . z == Me . pos . z)
	{
	new_move_target -> x = Me . pos . x;
	new_move_target -> y = Me . pos . y;
	}
    else 
	{
	new_move_target -> x = ThisRobot -> pos . x;
	new_move_target -> y = ThisRobot -> pos . y;
	}

    /* Action */
    if ( sqrt ( ( ThisRobot -> virt_pos . x - Me . pos . x ) * ( ThisRobot -> virt_pos . x - Me . pos . x ) +
		( ThisRobot -> virt_pos . y - Me . pos . y ) * ( ThisRobot -> virt_pos . y - Me . pos . y ) ) < 1 )
	{ //if we are close enough to tux, we talk
	ChatWithFriendlyDroid ( ThisRobot );
	ThisRobot -> will_rush_tux = FALSE ;
	ThisRobot -> combat_state = SELECT_NEW_WAYPOINT ;
	return; 
	}
}

static void state_machine_follow_tux(enemy * ThisRobot, moderately_finepoint * new_move_target)
{
    if ( ! ThisRobot->follow_tux || Me.invisible_duration > 0 )
	{
	ThisRobot->combat_state = WAYPOINTLESS_WANDERING;
	new_move_target -> x = ThisRobot->pos.x;
	new_move_target -> y = ThisRobot->pos.y;
	return;
	}

    /* Move target - The friendly bot will follow Tux, but with a small delay */
    
    // Compute a FrameRate independent delay
    int delay = 0.5 * 1.0/Frame_Time(); // Number of frame for a 0.5 seconds
    delay <<= 1; // InfluPosition is only stored every two frames
    
    if ( GetInfluPositionHistoryZ(delay) == ThisRobot -> pos . z )
	{
	new_move_target -> x = GetInfluPositionHistoryX(delay);
	new_move_target -> y = GetInfluPositionHistoryY(delay);
	
	moderately_finepoint ab = { ThisRobot -> pos . x - new_move_target -> x, ThisRobot -> pos . y - new_move_target -> y};
	if ( fabsf(ab . x) < 1 && fabsf(ab . y) < 1 )
	    {
	    new_move_target -> x = ThisRobot->pos.x;
	    new_move_target -> y = ThisRobot->pos.y;
	    }
	}
    else
	{
	update_virtual_position(&ThisRobot->virt_pos, &ThisRobot->pos, Me . pos . z);
	if ( ThisRobot -> virt_pos . z != -1 )
	    {
	    new_move_target -> x = ThisRobot -> pos . x + Me . pos . x - ThisRobot -> virt_pos . x;
	    new_move_target -> y = ThisRobot -> pos . y + Me . pos . y - ThisRobot -> virt_pos . y;
	    }
	else 
	    {
	    ThisRobot->combat_state = WAYPOINTLESS_WANDERING;
	    new_move_target -> x = ThisRobot->pos.x;
	    new_move_target -> y = ThisRobot->pos.y;
	    }
	}
}

static void state_machine_completely_fixed ( enemy * ThisRobot, moderately_finepoint * new_move_target )
{
    /* Move target */
    new_move_target -> x = ThisRobot-> pos . x;
    new_move_target -> y = ThisRobot-> pos . y;

    if ( ! ThisRobot->CompletelyFixed ) 
	ThisRobot->combat_state = WAYPOINTLESS_WANDERING;
}

static void state_machine_waypointless_wandering(enemy * ThisRobot, moderately_finepoint * new_move_target)
{
    if ( remaining_distance_to_current_walk_target ( ThisRobot ) < 0.1 ) 
	{
	set_new_waypointless_walk_target ( ThisRobot, new_move_target );
	TurnABitTowardsPosition ( ThisRobot, new_move_target -> x, new_move_target -> y, 90);
	ThisRobot -> combat_state = WAYPOINTLESS_WANDERING ;
	}
}

/**
 * 
 * This function runs the finite state automaton that powers the bots.
 * It handles attack and movement behaviors.
 *
 */
void
update_enemy ( enemy * ThisRobot )
{
    /* New structure :
     *
     * Inconditional updates:
     *	  debug stuff (say state on screen)
     *	  unstick from walls if relevant
     *	  certain switches (cleanup to be made here)
     *	  find an attack target (we consider it state independant)
     *	  reset speed to 0 (for now)
     *
     * Situational state changes (transitions from any state to a given one in certain input conditions)
     *    switch to RUSH TUX
     *    switch to RETURN HOME
     *
     * Per-state actions
     *    for each state:
     *       compute a new moving target (no pathfinding there, just tell where to go)
     *       do actions if appropriate (attack, talk, whatever)
     *       transition to a state
     *
     * Universal actions ("could" be merged with inconditional updates)
     *	  pathfind the new moving target if applicable (it differs from the current moving target)
     *	  move (special case target = cur. position)
     *
     */
 	
    /* Inconditional updates */
    state_machine_inconditional_updates(ThisRobot); 
    
    /* Situational state changes */
    state_machine_situational_transitions(ThisRobot);

    moderately_finepoint new_move_target;
    enemy_get_current_walk_target(ThisRobot, &new_move_target);

    /* Handle per-state switches and actions.
     * Each state much set move_target and combat_state.
     */

    // Default pathfinder execution context
    // Can eventually be changed by a state_machine_xxxx function
    freeway_context frw_ctx = { FALSE, { ThisRobot, NULL } };
	pathfinder_context pf_ctx = { &WalkableWithMarginPassFilter, &frw_ctx };
    
    switch ( ThisRobot -> combat_state )
	{
	case STOP_AND_EYE_TARGET:
	    state_machine_stop_and_eye_target ( ThisRobot, &new_move_target); 
	    break;

	case ATTACK:
	    state_machine_attack ( ThisRobot, &new_move_target, &pf_ctx );
	    break;

	case PARALYZED:
	    state_machine_paralyzed ( ThisRobot, &new_move_target);
	    break;
	
	case COMPLETELY_FIXED:
	    state_machine_completely_fixed ( ThisRobot, &new_move_target);
	    break;

	case FOLLOW_TUX:
	    state_machine_follow_tux ( ThisRobot, &new_move_target );
	    break;

	case RETURNING_HOME:
	    state_machine_returning_home ( ThisRobot, &new_move_target);
	    break;
	
	case SELECT_NEW_WAYPOINT:
	    state_machine_select_new_waypoint ( ThisRobot, &new_move_target);
	    break;

	case TURN_TOWARDS_NEXT_WAYPOINT:
	    state_machine_turn_towards_next_waypoint ( ThisRobot, &new_move_target);
	    break;
	
	case MOVE_ALONG_RANDOM_WAYPOINTS:
	    state_machine_move_along_random_waypoints ( ThisRobot, &new_move_target );
	    break;

	case RUSH_TUX_AND_OPEN_TALK:
	    state_machine_rush_tux_and_open_talk ( ThisRobot, &new_move_target );
	    break;

	case WAYPOINTLESS_WANDERING:
	    state_machine_waypointless_wandering ( ThisRobot, &new_move_target );
	    break;

	}

    /* Pathfind current target */
    /* I am sorry this is a bit dirty, but I've got time and efficiency constraints. If you're not happy please send a patch. No complaints will 
     * be accepted.*/

    /* The basic design is the following :
     * we get the current moving target of the bot (ie. old)
     * we compare the new and current moving target
     *    if they differ : we have to set up a new route (pathfind the route)
     *    if they do not : we move towards our first waypoint
     *
     * special case: 
     *                if first waypoint is -1 -1 we have a bug and do nothing (hack around)
     */
    moderately_finepoint wps[40];
    moderately_finepoint old_move_target;
    enemy_get_current_walk_target(ThisRobot, &old_move_target);

    if ( (new_move_target.x == ThisRobot->pos.x) && (new_move_target.y == ThisRobot->pos.y) )
    {   // If the bot stopped moving, create a void path
    	ThisRobot->PrivatePathway[0].x = ThisRobot->pos.x;
    	ThisRobot->PrivatePathway[0].y = ThisRobot->pos.y;
    	ThisRobot->PrivatePathway[1].x = -1;
    	ThisRobot->PrivatePathway[1].y = -1;    	
    }
    else if (((new_move_target . x != old_move_target . x) || (new_move_target .y != old_move_target . y)))
    {   // If the current move target differs from the old one
    	// This implies we do not re-pathfind every frame, which means we may bump into colleagues. 
    	// This is handled in MoveThisEnemy()
    	if ( set_up_intermediate_course_between_positions ( &ThisRobot->pos, &new_move_target, &wps[0], 40, &pf_ctx ) && wps[5].x == -1)
    	{ /* If position was passable *and* streamline course uses max 4 waypoints */
    		memcpy ( &ThisRobot->PrivatePathway[0], &wps[0], 5 * sizeof(moderately_finepoint));
    	}
    	else
    	{
    		ThisRobot->PrivatePathway[0].x = ThisRobot->pos.x;
    		ThisRobot->PrivatePathway[0].y = ThisRobot->pos.y;
    		ThisRobot->PrivatePathway[1].x = -1;
    		ThisRobot->PrivatePathway[1].y = -1;
    		if ( ThisRobot->pure_wait < WAIT_COLLISION )
    			ThisRobot->pure_wait = WAIT_COLLISION;
    	}
    }
    
    if ( ThisRobot->PrivatePathway[0].x == -1)
    {
    	/* This happens at the very beginning of the game. If it happens afterwards this is a ugly bug. */
    	ThisRobot -> PrivatePathway[0] . x = ThisRobot->pos.x;
    	ThisRobot -> PrivatePathway[0] . y = ThisRobot->pos.y;
    }

    MoveThisEnemy(ThisRobot);
}; // void update_enemy()

/**
 * This function handles all the logic tied to enemies : animation, movement
 * and attack behavior.
 *
 * Note that no enemy must be killed by the logic function. It's a technical limitation
 * and a requirement in freedroidRPG.
 */
void
MoveEnemys ( void )
{
    //--------------------
    // We heal the robots as time passes.
    PermanentHealRobots ();

    //--------------------
    // Now the per-enemy stuff
    //
    enemy *ThisRobot, *nerot;
    int i; // i = 0 mean dead bots, 1 means alive
    for ( i = 0; i < 2; i ++ )
	list_for_each_entry_safe(ThisRobot, nerot, i? &alive_bots_head : &dead_bots_head, global_list)
	    {

	    //--------------------
	    // Ignore robots on other levels, except those 
	    // on levels that can be "seen"
	    // 
	    if ( ( ! level_is_visible ( ThisRobot -> pos . z ) ) ) 
		continue;

	    animate_enemy(ThisRobot);

	    /* If we are processing dead bots, simply do animation stuff.
	     * The rest is for alive bots only. */
	    if ( i == 0 ) 
		continue;

	    //--------------------
	    // Run a new cycle of the bot's state machine
	    update_enemy ( ThisRobot );

	    }

}; // MoveEnemys( void ) 

/**
 * When an enemy is firing a shot, the newly created bullet must be 
 * assigned a speed, that would lead the bullet thowards the intended
 * target, which is done here.
 */
void
set_bullet_speed_to_target_direction ( bullet* NewBullet , float bullet_speed , float xdist , float ydist )
{
  //--------------------
  // determine the direction of the shot, so that it will go into the direction of
  // the target
  //
  if (fabsf (xdist) > fabsf (ydist))
    {
      NewBullet->speed.x = bullet_speed;
      NewBullet->speed.y = ydist * NewBullet->speed.x / xdist;
      if (xdist < 0)
	{
	  NewBullet->speed.x = -NewBullet->speed.x;
	  NewBullet->speed.y = -NewBullet->speed.y;
	}
    }
  
  if (fabsf (xdist) < fabsf (ydist))
    {
      NewBullet->speed.y = bullet_speed;
      NewBullet->speed.x = xdist * NewBullet->speed.y / ydist;
      if (ydist < 0)
	{
	  NewBullet->speed.x = -NewBullet->speed.x;
	  NewBullet->speed.y = -NewBullet->speed.y;
	}
    }
}; // void set_bullet_speed_to_target_direction ( bullet* NewBullet , float bullet_speed , float xdist , float ydist )

/**
 * This function is low-level:  It simply sets off a shot from enemy
 * through the pointer ThisRobot at the target VECTOR xdist ydist, which
 * is a DISTANCE VECTOR, NOT ABSOLUTE COORDINATES OF THE TARGET!!!
 */
static void 
RawStartEnemysShot( enemy* ThisRobot , float xdist , float ydist )
{
    int guntype = ItemMap[ Druidmap[ThisRobot->type].weapon_item.type ].item_gun_bullet_image_type;
    float bullet_speed = (float) ItemMap[ Druidmap[ ThisRobot->type ].weapon_item.type ].item_gun_speed;
    bullet* NewBullet=NULL;
    int bullet_index = 0 ;

    //--------------------
    // If the robot is not in walk or stand animation, i.e. if it's in
    // gethit, death or attack animation, then we can't start another
    // shot/attack right now...
    //
    if ( ( ThisRobot -> animation_type != WALK_ANIMATION ) && 
	    ( ThisRobot -> animation_type != STAND_ANIMATION ) ) return ;

    /* First of all, check what kind of weapon the bot has : ranged or melee */
    if ( ! ItemMap[ Druidmap[ ThisRobot->type ].weapon_item.type ].item_weapon_is_melee )
	{ /* ranged */	
	//--------------------
	// find a bullet entry, that isn't currently used... 
	//
	bullet_index = find_free_bullet_index ();
	NewBullet = & ( AllBullets [ bullet_index ] );

	//--------------------
	// We send the bullet onto it's way thowards the given target
	//
	set_bullet_speed_to_target_direction ( NewBullet , bullet_speed , xdist , ydist );

	//--------------------
	// Newly, also enemys have to respect the angle modifier in their weapons...
	//
	RotateVectorByAngle ( & ( NewBullet->speed ) , ItemMap[ Druidmap[ ThisRobot->type ].weapon_item.type ].item_gun_start_angle_modifier );    
	NewBullet->angle = - ( 90 + 45 + 180 * atan2 ( NewBullet->speed.y,  NewBullet->speed.x ) / M_PI );  

	//--------------------
	// At this point we mention, that when not moving anywhere, the robot should also
	// face into the direction of the shot
	//
	ThisRobot->previous_angle = NewBullet -> angle + 180 ;

	// start all bullets in the center of the shooter first...
	NewBullet -> pos . x = ThisRobot -> virt_pos . x;
	NewBullet -> pos . y = ThisRobot -> virt_pos . y;
	NewBullet -> pos . z = ThisRobot -> virt_pos . z;

	// fire bullets so, that they don't hit the shooter...
	NewBullet->pos.x +=
	    ( NewBullet -> speed.x) / ( bullet_speed ) * 0.5 ;
	NewBullet->pos.y +=
	    ( NewBullet -> speed.y) / ( bullet_speed ) * 0.5 ;

	NewBullet->type = guntype;

	// Now we set the damage of this bullet to the correct value
	NewBullet->damage = ItemMap[ Druidmap[ ThisRobot->type ].weapon_item.type ].base_item_gun_damage + MyRandom( ItemMap[ Druidmap[ ThisRobot->type ].weapon_item.type ].item_gun_damage_modifier);

	NewBullet->time_in_seconds = 0;
	NewBullet->time_in_frames = 0;

	//--------------------
	// Most enemy shots will not have any special 'magic' property...
	//
	NewBullet->poison_duration = 0;
	NewBullet->poison_damage_per_sec = 0;
	NewBullet->freezing_level = 0;
	NewBullet->paralysation_duration = 0;

	NewBullet->bullet_lifetime = ItemMap [ Druidmap[ThisRobot->type].weapon_item.type ].item_gun_bullet_lifetime;

	NewBullet->owner = ThisRobot -> type;
	NewBullet->ignore_wall_collisions = 
	    ItemMap[ Druidmap[ ThisRobot->type].weapon_item.type ].item_gun_bullet_ignore_wall_collisions;
	NewBullet->to_hit = Druidmap [ ThisRobot->type ].to_hit ;
	NewBullet->was_reflected = FALSE;
	NewBullet->pass_through_explosions = 
	    ItemMap[ Druidmap[ ThisRobot->type].weapon_item.type ].item_gun_bullet_pass_through_explosions;
	NewBullet->reflect_other_bullets = 
	    ItemMap[ Druidmap[ ThisRobot->type].weapon_item.type ].item_gun_bullet_reflect_other_bullets;
	NewBullet->pass_through_hit_bodies = 
	    ItemMap[ Druidmap[ ThisRobot->type].weapon_item.type ].item_gun_bullet_pass_through_hit_bodies;
	NewBullet->is_friendly = ThisRobot->is_friendly;
	}
    else  /* melee weapon */
	{
        int shot_index = find_free_melee_shot_index ();
	melee_shot *NewShot = & ( AllMeleeShots [ shot_index ] );

	NewShot -> attack_target_type = ThisRobot -> attack_target_type;
	NewShot -> mine = 0; /* shot comes from a bot not tux */
	
	if ( ThisRobot -> attack_target_type == ATTACK_TARGET_IS_ENEMY) 
	    {
	    NewShot -> bot_target_n = ThisRobot->bot_target_n;
	    NewShot -> bot_target_addr = ThisRobot->bot_target_addr;
	    }
	else
	    { /* enemy bot attacking tux*/
	    enemy_set_reference(& NewShot -> bot_target_n, &NewShot -> bot_target_addr, NULL);
	    }

	NewShot->to_hit = 60 * Druidmap [ ThisRobot->type] .monster_level;
	NewShot->damage = ItemMap[ Druidmap[ ThisRobot->type ].weapon_item.type ].base_item_gun_damage + MyRandom( ItemMap[ Druidmap[ ThisRobot->type ].weapon_item.type ].item_gun_damage_modifier);
	NewShot->owner = ThisRobot->type;
	NewShot->level = Druidmap [ ThisRobot-> type ] . monster_level;
	}


    ThisRobot -> ammo_left --;

    if( ThisRobot -> ammo_left > 0 )
	{
	ThisRobot -> firewait += 
	    ItemMap [ Druidmap [ ThisRobot -> type ] . weapon_item . type ] . item_gun_recharging_time ;
	}
    else 
	{
	ThisRobot -> ammo_left = ItemMap [ Druidmap [ ThisRobot -> type ] . weapon_item . type ] . item_gun_ammo_clip_size ; 
	if(ThisRobot -> firewait < ItemMap [ Druidmap [ ThisRobot -> type ] . weapon_item . type ] . item_gun_reloading_time)
	    ThisRobot -> firewait = ItemMap [ Druidmap [ ThisRobot -> type ] . weapon_item . type ] . item_gun_reloading_time ;
	}

    if(ThisRobot -> firewait < ItemMap [ Druidmap [ ThisRobot -> type ] . weapon_item . type ] . item_gun_recharging_time)
	ThisRobot -> firewait = ItemMap [ Druidmap [ ThisRobot -> type ] . weapon_item . type ] . item_gun_recharging_time;

    if ( last_attack_animation_image [ ThisRobot -> type ] - first_attack_animation_image [ ThisRobot -> type ] > 1 )
	{
	ThisRobot -> animation_phase = ((float)first_attack_animation_image [ ThisRobot -> type ]) + 0.1 ;
	ThisRobot -> animation_type = ATTACK_ANIMATION;
	ThisRobot -> current_angle = - ( - 90 + 180 * atan2 ( ydist ,  xdist ) / M_PI );  
	}

    Fire_Bullet_Sound(guntype);
}; // void RawStartEnemysShot( enemy* ThisRobot , float xdist , float ydist )

/**
 * This function should determine the closest visible player to this 
 * enemy droid.
 */
enemy *
ClosestOtherEnemyDroid ( Enemy ThisRobot ) 
{
  enemy * BestTarget = NULL ;
  float BestDistance = 100000 ;
  float FoundDistance;
 
  enemy *erot, *nerot;
  BROWSE_ALIVE_BOTS_SAFE(erot, nerot)
    {
      if ( ThisRobot -> pos . z != erot->pos . z )
	  {
	  continue;
	  }

      //--------------------
      // If we compare us with ourselves, this is also no good...
      //
      if ( ThisRobot == erot) 
	  {
	  continue;
	  }

      FoundDistance = ( erot->pos . x - ThisRobot -> pos . x ) *
	( erot->pos . x - ThisRobot -> pos . x ) +
	( erot->pos . y - ThisRobot -> pos . y ) * 
	( erot->pos . y - ThisRobot -> pos . y ) ;

      if ( FoundDistance < BestDistance )
	{
	  BestDistance = FoundDistance ;
	  BestTarget = erot;
	}
    }

  return BestTarget ;

};


/**
 * In some of the movement functions for enemy droids, we consider making
 * a step and move a bit into one direction or the other.  But not all
 * moves are really allowed and feasible.  Therefore we need a function
 * to check if a certain step makes sense or not, which is exactly what
 * this function is supposed to do.
 *
 */
int
ConsideredMoveIsFeasible ( Enemy ThisRobot , moderately_finepoint StepVector )
{
	freeway_context frw_ctx = { TRUE, { ThisRobot, NULL } };
	
    if ( ( DirectLineColldet ( ThisRobot -> pos.x, ThisRobot -> pos.y, ThisRobot -> pos.x + StepVector.x ,
			ThisRobot -> pos.y + StepVector.y ,
			ThisRobot -> pos.z, NULL ) ) && 
	     ( CheckIfWayIsFreeOfDroids ( ThisRobot->pos.x , ThisRobot->pos.y , 
			ThisRobot->pos.x + StepVector . x , 
			ThisRobot->pos.y + StepVector . y ,
			ThisRobot->pos.z , &frw_ctx ) )  )
    {
	return TRUE;
    }
    
    return FALSE;

}; // int ConsideredMoveIsFeasible ( Enemy ThisRobot , finepoint StepVector )

/*
 * This function will find a place near target_pos that is free of any bots.
 * 
 * During a melee, several bots will try to shoot a common enemy.
 * We will "distribute" them around the enemy.
 * 
 * The approach is :
 * - compute several positions around the target, by rotating a unit vector
 * - halt as soon as one of those positions is free
 */

static void MoveToMeleeCombat( Enemy ThisRobot, gps* target_pos, moderately_finepoint* set_move_tgt )
{
	freeway_context frw_ctx = { ThisRobot->is_friendly, { ThisRobot->bot_target_addr, NULL } };
	
	// All computations are done in the target's level
	gps bot_vpos;
	update_virtual_position ( &bot_vpos, &ThisRobot->pos, target_pos->z);
	
	// Compute a unit vector from target to ThisRobot
	moderately_finepoint vector_from_target = { bot_vpos.x, bot_vpos.y }; //
	normalize_vect ( target_pos->x , target_pos->y, &(vector_from_target.x), &(vector_from_target.y) );
	vector_from_target.x -= target_pos->x;
	vector_from_target.y -= target_pos->y; // vector_from_target holds the coordinates of normalized target -> bot vector

	// Fallback if there are no free positions -> Rush on Tux
	gps final_pos = { target_pos->x + vector_from_target.x*MELEE_MIN_DIST, target_pos->y + vector_from_target.y*MELEE_MIN_DIST, target_pos->z }; 

	// Rotate the unit vector
	float angles_to_try[8] = { 0, 45, -45, 90, -90, 135, -135, 180 };
	int a;
	for ( a = 0; a < 8; ++a )
	{
		int target_reachable = FALSE;
		moderately_finepoint checked_vector = { vector_from_target.x, vector_from_target.y };
		RotateVectorByAngle ( &checked_vector, angles_to_try[a] );
		moderately_finepoint checked_pos = { target_pos->x + checked_vector.x*MELEE_MIN_DIST, target_pos->y + checked_vector.y*MELEE_MIN_DIST }; 

		// Lower quantization of checked_pos, to limit bot jittering around the target
		checked_pos.x = floorf(checked_pos.x * 10.0) / 10.0;
		checked_pos.y = floorf(checked_pos.y * 10.0) / 10.0;
		
		if ( CheckIfWayIsFreeOfDroids( target_pos->x, target_pos->y, checked_pos.x, checked_pos.y, target_pos->z, &frw_ctx ) )
		{
			// If the checked_pos is free, also check that the target is reachable
			target_reachable = DirectLineColldet( checked_pos.x, checked_pos.y, target_pos->x, target_pos->y, target_pos->z, &WalkablePassFilter);
			if ( target_reachable ) 
			{   // The position has been found
				final_pos.x = checked_pos.x;
				final_pos.y = checked_pos.y;
				break;
			}
		}
	}

	// Transform back into ThisRobot's reference level
	update_virtual_position( &bot_vpos, &final_pos, ThisRobot->pos.z );
	set_move_tgt->x = bot_vpos.x;
	set_move_tgt->y = bot_vpos.y;
}

/**
 *
 */
static void
MoveAwayFromMeleeCombat ( Enemy ThisRobot , moderately_finepoint * set_move_tgt )
{
    finepoint VictimPosition = { 0.0, 0.0};
    finepoint CurrentPosition = { 0.0, 0.0};
    moderately_finepoint StepVector;
    moderately_finepoint RotatedStepVector;
    float StepVectorLen;
    int i ;

#define ANGLES_TO_TRY 7
    float RotationAngleTryList[ ANGLES_TO_TRY ] = { 0 , 30 , 360-30 , 60, 360-60, 90, 360-90 };
    
    VictimPosition . x = enemy_get_target_position(ThisRobot) -> x;
    VictimPosition . y = enemy_get_target_position(ThisRobot) -> y;
    
    CurrentPosition . x = ThisRobot -> virt_pos . x ;
    CurrentPosition . y = ThisRobot -> virt_pos . y ;
    
    StepVector . x = VictimPosition . x - CurrentPosition . x ;
    StepVector . y = VictimPosition . y - CurrentPosition . y ;
    
    //--------------------
    // Now some protection against division by zero when two bots
    // have _exactly_ the same position, i.e. are standing on top
    // of each other:
    //
    if ( ( fabsf ( StepVector . x ) < 0.01 ) &&
	 ( fabsf ( StepVector . y ) < 0.01 ) )
    {
	if ( MyRandom(1) == 1 )
	{
	    StepVector . x = 1.0 ;
	    StepVector . y = 1.0 ;
	}
	else
	{
	    StepVector . x = -1.0 ;
	    StepVector . y = -1.0 ;
	}
    }
     
    StepVectorLen = sqrt ( ( StepVector . x ) * ( StepVector . x ) + ( StepVector . y ) * ( StepVector . y ) );

    StepVector . x /= ( -1 * StepVectorLen ) ;
    StepVector . y /= ( -1 * StepVectorLen ) ;

    for ( i = 0 ; i < ANGLES_TO_TRY ; i ++ )
    {
	RotatedStepVector.x = StepVector.x ;
	RotatedStepVector.y = StepVector.y ;
	RotateVectorByAngle ( & RotatedStepVector , RotationAngleTryList [ i ] ) ;
	
	//--------------------
	// Maybe we've found a solution, then we can take it and quit
	// trying around...
	//
	if ( /*XXX*/ ConsideredMoveIsFeasible ( ThisRobot , RotatedStepVector ) &&
	             DirectLineColldet( ThisRobot->virt_pos.x + RotatedStepVector.x, ThisRobot->virt_pos.y + RotatedStepVector.y,
	                                 VictimPosition.x, VictimPosition.y, ThisRobot->pos.z, NULL )
	   )
	{
	    set_move_tgt -> x = ThisRobot -> pos.x + RotatedStepVector . x ;
	    set_move_tgt -> y = ThisRobot -> pos.y + RotatedStepVector . y ;
	    break;
	}

	//--------------------
	// No solution, don't move
	set_move_tgt->x = ThisRobot->pos.x;
	set_move_tgt->y = ThisRobot->pos.y;

    }
}; // void MoveAwayFromMeleeCombat( Enemy ThisRobot , moderately_finepoint * set_move_tgt )

/**
 * 
 */
static void ReachMeleeCombat( Enemy ThisRobot, gps* tpos, moderately_finepoint* new_move_target, pathfinder_context *pf_ctx )
{
	// Target not reachable -> roughly reach the target.
	// The exact destination will be computed later.
	new_move_target->x = tpos->x;
	new_move_target->y = tpos->y;
	
	// If ThisRobot is a friend, we want him to move as far as possible,
	// so we de-activate the bot-collision test during the pathfinder call.
	// ( see the function's commnent of state_machine_attack() )
	//
	// Else, a classical pathfinder context is used. However, the final position
	// is the target position, and so we have to add the target into the bot-collision
	// exception's list.

	if ( ThisRobot->is_friendly ) pf_ctx->frw_ctx = NULL;
	else pf_ctx->frw_ctx->except_bots[1] = ThisRobot->bot_target_addr;
}

/**
 * At some points it may be necessary, that an enemy turns around to
 * face the Tux.  This function does that, but only so much as can be
 * done in the current frame, i.e. NON-BLOCKING operation.
 *
 * The return value indicates, if the turning has already reached it's
 * target by now or not.
 */
static int
TurnABitTowardsPosition ( Enemy ThisRobot , float x , float y , float TurnSpeed )
{
  float RightAngle;
  float AngleInBetween;
  float TurningDirection;

  if ( ThisRobot -> pos . x == x && ThisRobot -> pos . y == y ) 
      return TRUE;

  //--------------------
  // Now we find out what the final target direction of facing should
  // be.
  //
  // For this we use the atan2, which gives angles from -pi to +pi.
  // 
  // Attention must be paid, since 'y' in our coordinates ascends when
  // moving down and descends when moving 'up' on the scren.  So that
  // one sign must be corrected, so that everything is right again.
  //
  RightAngle = ( atan2 ( - ( y - ThisRobot -> pos . y ) ,  
			 + ( x - ThisRobot -> pos . x ) ) * 180.0 / M_PI ) ;
  //
  // Another thing there is, that must also be corrected:  '0' begins
  // with facing 'down' in the current rotation models.  Therefore angle
  // 0 corresponds to that.  We need to shift again...
  //
  RightAngle += 90 ;

  //--------------------
  // Now it's time do determine which direction to move, i.e. if to 
  // turn to the left or to turn to the right...  For this purpose
  // we convert the current angle, which is between 270 and -90 degrees
  // to one between -180 and +180 degrees...
  //
  if ( RightAngle > 180.0 ) RightAngle -= 360.0 ; 

  //--------------------
  // Having done these preparations, it's now easy to determine the right
  // direction of rotation...
  //
  AngleInBetween = RightAngle - ThisRobot -> current_angle ;
  if ( AngleInBetween > 180 ) AngleInBetween -= 360;
  if ( AngleInBetween <= -180 ) AngleInBetween += 360;

  if ( AngleInBetween > 0 )
    TurningDirection = +1 ; 
  else 
    TurningDirection = -1 ; 

  //--------------------
  // Now we turn and show the image until both chat partners are
  // facing each other, mostly the chat partner is facing the Tux,
  // since the Tux may still turn around to somewhere else all the 
  // while, if the chose so
  //
  ThisRobot -> current_angle += TurningDirection * TurnSpeed * Frame_Time() ;

  //--------------------
  // In case of positive turning direction, we wait, till our angle is greater
  // than the right angle.
  // Otherwise we wait till our angle is lower than the right angle.
  //
  AngleInBetween = RightAngle - ThisRobot -> current_angle ;
  if ( AngleInBetween > 180 ) AngleInBetween -= 360;
  if ( AngleInBetween <= -180 ) AngleInBetween += 360;
      
  if ( ( TurningDirection > 0 ) && ( AngleInBetween < 0 ) ) return ( TRUE );
  if ( ( TurningDirection < 0 ) && ( AngleInBetween > 0 ) ) return ( TRUE );

  return ( FALSE );

}; // int TurnABitThowardsTux ( Enemy ThisRobot , float TurnSpeed )

/**
 * Enemies act as groups.  If one is hit, all will attack and the like.
 * This transmission of information is handled here.
 */
void 
SetRestOfGroupToState ( Enemy ThisRobot , short NewState )
{
  int MarkerCode;

  MarkerCode = ThisRobot -> marker ;

  if ( ( MarkerCode == 0 ) || ( MarkerCode == 101 ) )return ;

  enemy *erot;
  BROWSE_ALIVE_BOTS(erot)
      {
      if ( erot-> marker == MarkerCode )
	  erot->combat_state = NewState ;
      }

}; // void SetRestOfGroupToState ( Enemy ThisRobot , int NewState )

/**
 * Enemies act as groups.  If one is hit, all will attack and the like.
 * Similarly, if you attack one peaceful guard, all other guards will be
 * pissed as well...
 */
void 
robot_group_turn_hostile ( enemy * ThisRobot )
{
    int MarkerCode;

    MarkerCode = ThisRobot -> marker ;
    if (MarkerCode == 9999) /* ugly hack */
	SwitchBackgroundMusicTo(BIGFIGHT_BACKGROUND_MUSIC_SOUND);

    enemy *erot;
    BROWSE_ALIVE_BOTS(erot)
	{
	if ( erot->marker == MarkerCode && erot->has_been_taken_over == FALSE)
	    erot->is_friendly = FALSE ;
	}
}; // void robot_group_turn_hostile ( int enemy_num )


/**
 * This function checks for enemy collsions and returns TRUE if enemy 
 * with number enemynum collided with another enemy from the list.
 */
int
CheckEnemyEnemyCollision ( enemy * OurBot )
{
    float check_x, check_y;
    int swap;
    float xdist, ydist;
    float dist2;
    check_x = OurBot -> pos . x ;
    check_y = OurBot -> pos . y ;

    if ( OurBot -> pure_wait ) 
	return FALSE;
    
    //--------------------
    // Now we check through all the other enemys on this level if 
    // there is perhaps a collision with them...

    enemy *erot;
    BROWSE_LEVEL_BOTS(erot, OurBot->pos.z)
    {
	if (erot == OurBot)
	    continue;

	xdist = check_x - erot->pos.x;
	ydist = check_y - erot->pos.y;
	
	dist2 = sqrt(xdist * xdist + ydist * ydist);
	
	// Is there a Collision?
	if ( dist2 <= 2*DRUIDRADIUSXY )
	{
	    if ( erot -> pure_wait ) 
		continue;

	    erot->pure_wait = WAIT_COLLISION;
	    
	    swap = OurBot->nextwaypoint;
	    OurBot->nextwaypoint = OurBot->lastwaypoint;
	    OurBot->lastwaypoint = swap;
	    
	    return TRUE;
	} // if collision distance reached

    } // for all the bots...
    
    return FALSE;
}; // int CheckEnemyEnemyCollision

/**
 * This function increases the phase counters for animation of a bot.
 */
void
animate_enemy (enemy * our_enemy)
{
    switch ( our_enemy -> animation_type )
	{

	case WALK_ANIMATION:
	    our_enemy -> animation_phase += 
		Frame_Time() * droid_walk_animation_speed_factor [ our_enemy -> type ] ;
	
	    //--------------------
	    // While we're in the walk animation cycle, we have the walk animation
	    // images cycle.
	    //
	    if ( our_enemy -> animation_phase >= last_walk_animation_image [ our_enemy -> type ] )
		{
		our_enemy -> animation_phase = 0 ;
		our_enemy -> animation_type = WALK_ANIMATION;
		}
	    //--------------------
	    // But as soon as the walk stops and the 'bot' is standing still, we switch
	    // to the standing cycle...
	    //
	    if ( ( fabsf ( our_enemy -> speed . x ) < 0.1 ) && ( fabsf ( our_enemy -> speed . y ) < 0.1 ) )
		{
		our_enemy -> animation_type = STAND_ANIMATION ;
		our_enemy -> animation_phase = first_stand_animation_image [ our_enemy -> type ] - 1 ;
		}

	    break;
	case ATTACK_ANIMATION:
	    our_enemy -> animation_phase += 
		Frame_Time() * droid_attack_animation_speed_factor [ our_enemy -> type ] ;

	    if ( our_enemy -> animation_phase >= last_attack_animation_image [ our_enemy -> type ] )
		{
		our_enemy -> animation_phase = 0 ;
		our_enemy -> animation_type = WALK_ANIMATION;
		}

	    break;
	case GETHIT_ANIMATION:
	    our_enemy -> animation_phase += 
		Frame_Time() * droid_gethit_animation_speed_factor [ our_enemy -> type ] ;

	    if ( our_enemy -> animation_phase >= last_gethit_animation_image [ our_enemy -> type ] )
		{
		our_enemy -> animation_phase = 0 ;
		our_enemy -> animation_type = WALK_ANIMATION;
		}

	    break;
	case DEATH_ANIMATION:
	    our_enemy -> animation_phase += 
		Frame_Time() * droid_death_animation_speed_factor [ our_enemy -> type ] ;

	    if ( our_enemy -> animation_phase >= last_death_animation_image [ our_enemy -> type ] - 1 )
		{
		our_enemy -> animation_phase = last_death_animation_image [ our_enemy -> type ] - 1 ;
		our_enemy -> animation_type = DEATH_ANIMATION ;
		}
	    break;
	case STAND_ANIMATION:
	    our_enemy -> animation_phase += 
		Frame_Time() * droid_stand_animation_speed_factor [ our_enemy -> type ] ;

	    if ( our_enemy -> animation_phase >= last_stand_animation_image [ our_enemy -> type ] - 1 )
		{
		our_enemy -> animation_phase = first_stand_animation_image [ our_enemy -> type ] - 1 ;
		our_enemy -> animation_type = STAND_ANIMATION;
		}

	    break;

	default:
	    fprintf ( stderr , "\nThe animation type found is: %d.", our_enemy -> animation_type );
	    ErrorMessage ( __FUNCTION__  , "\
		    There was an animation type encountered that isn't defined in FreedroidRPG.\n\
		    That means:  Something is going *terribly* wrong!" ,
		    PLEASE_INFORM, IS_FATAL );
	    break;
	}

}; // void animate_enemy ( enemy *)


/******************************************************
 * Resolve the address of an enemy, given its number (primary key) and 
 * the cache value of its address. 
 * 1- number == -1 means no enemy targetted, means we return NULL
 * 2- if the cache value is not NULL, we return it
 * 3- if the cache value is NULL, resolve the address by browsing the list and set the cache value
 *********************************************************/
enemy * enemy_resolve_address(short int enemy_number, enemy ** enemy_addr)
{
    if ( enemy_number == -1 )
	{
	*enemy_addr = NULL;
	return NULL;
	}

    if ( ! (* enemy_addr ))
	{
	int i;
	for ( i = 0; i < 2; i ++ )
	    {
	    enemy * erot;
	    list_for_each_entry(erot, i ? &dead_bots_head : &alive_bots_head, global_list)
		if ( enemy_number == erot->id )
		    {
		    *enemy_addr = erot;
		    return *enemy_addr;
		    }
	    }
	return NULL;
	}
    return *enemy_addr;
}


/********************************************
 * Sets a reference to an enemy to the given address
 * (we could use the number too)
 * This sets both the cache value and the number.
 ********************************/
void enemy_set_reference(short int * enemy_number, enemy ** enemy_addr, enemy * addr)
{
    if ( addr == NULL )
	{
	*enemy_addr = NULL;
	*enemy_number = -1;
	}
    else 
	{
	*enemy_addr = addr;
	*enemy_number = addr->id;
	}
}


/***************************************
 * Generate per level alive enemy lists
 *
 * This function assumes level_bots_head lists
 * have been properly initialized and emptied.
 ***************************************/
void enemy_generate_level_lists()
{
enemy * erot;

BROWSE_ALIVE_BOTS(erot)
    {
    list_add(&erot->level_list, &level_bots_head[erot->pos.z]);
    }

}

/**
 * This function checks if the enemy at 'target_pos' is a potential target for
 * 'this_robot'.
 * To be a potential target, the enemy has :
 * 1) to be closer to this_robot than the current best target (defined by its
 *    distance 'squared_best_dist')
 * 2) to be visible
 * 3) to be reachable (definition depends on the robot's weapon)
 * 
 * If the enemy is a potential target, 'squared_best_dist' is changed, and
 * the function returns TRUE.
 * 
 * Note: All operations are executed in this_robot's level
 * 
 */
static int is_potential_target( enemy* this_robot, gps* target_pos, float* squared_best_dist )
{
	// Get target's virtual position in term of this_robot's level
	gps target_vpos;
	update_virtual_position(&target_vpos, target_pos, this_robot->pos.z);
	
	// Potentially closer than current best dist ?
	// We use the direct line distance as the distance to the target.
	// The reason is : if two potential targets are seen through a window,
	// we will attack the one that is visually the closest one (the bots are
	// not omniscient and do not know the real length of the path to their targets)
	float xdist = target_vpos.x - this_robot->pos.x;
	float ydist = target_vpos.y - this_robot->pos.y;
	float squared_target_dist = (xdist * xdist + ydist * ydist);

	if ( squared_target_dist > *squared_best_dist )
	{
		return FALSE;
	}
	
	// If the target is not visible, then it cannot be attacked
	if ( !DirectLineColldet(this_robot->pos.x, this_robot->pos.y, 
	                        target_vpos.x, target_vpos.y, 
	                        this_robot->pos.z, &VisiblePassFilter) )
	{
		return FALSE;
	}

	// For a range weapon, check if the target can be directly shot
	int melee_weapon = ItemMap[Druidmap[this_robot->type].weapon_item.type].item_weapon_is_melee;

	if (!melee_weapon)
	{
		if ( DirectLineColldet(this_robot->pos.x, this_robot->pos.y, 
		                       target_vpos.x, target_vpos.y, 
		                       this_robot->pos.z, &FlyablePassFilter) )
		{
			*squared_best_dist = squared_target_dist;
			return TRUE;
		}
	}

	// Else (if melee_weapon or not shootable for a range_weapon), checks if a path exists to reach the target
	moderately_finepoint mid_pos[40];
	moderately_finepoint to_pos = { target_vpos.x, target_vpos.y };
	pathfinder_context pf_ctx = { &WalkableWithMarginPassFilter, NULL };
	int path_found = set_up_intermediate_course_between_positions( &(this_robot->pos), &to_pos, mid_pos, 40, &pf_ctx) && (mid_pos[5].x == -1);
	if (!path_found) return FALSE;

	*squared_best_dist = squared_target_dist;
	return TRUE;

} // is_potential_target( enemy* this_robot, gps* target_pos, float* squared_best_dist )

#undef _enemy_c
