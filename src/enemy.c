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

/* ----------------------------------------------------------------------
 * This file contains all enemy realted functions.  This includes their 
 * whole behaviour, healing, initialization, shuffling them around after 
 * evevator-transitions, deleting them, collisions of enemys among 
 * themselves, their fireing, animation and such.
 * ---------------------------------------------------------------------- */

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
int EnemyOfTuxCloseToThisRobot ( Enemy, moderately_finepoint* );
static void MoveInCloserForOrAwayFromMeleeCombat ( Enemy, int, moderately_finepoint *);
static void RawStartEnemysShot( enemy*, float, float);



LIST_HEAD(alive_bots_head);
LIST_HEAD(dead_bots_head);
list_head_t level_bots_head[MAX_LEVELS]; //THIS IS NOT STATICALLY PROPERLY INITIALIZED, done in init functions


/* ----------------------------------------------------------------------
 * In the very beginning of each game, it is not enough to just place the
 * bots onto the right locations.  They must also be integrated into the
 * waypoint system, i.e. current waypoint and next waypoint initialized.
 * This is what this function is supposed to do.
 * ---------------------------------------------------------------------- */
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

/* ----------------------------------------------------------------------
 * This function tests, if a Robot can go a direct straigt line from
 * x1 y1 to x2 y2 without hitting a wall or another obstacle.
 * 
 * The return value is TRUE or FALSE accoringly.
 * ----------------------------------------------------------------------*/
int 
DirectLineWalkable( float x1 , float y1 , float x2 , float y2 , int z )
{
    float LargerDistance;
    int Steps;
    int i;
    finepoint step;
    finepoint CheckPosition;
    static int first_call = TRUE;
    static int step_multiplier = -1 ; // something completely absurd...
    int steps_for_this_obstacle;
    static int key_obstacle_type = -1 ;

    //--------------------
    // On the very first call of this function, we find out how much of
    // a stepsize will be possible for the purpose of finding out, if any
    // given line is directly walkable or not.  Depending on the collision
    // rectangles defined for each obstacle, we'll find out the maximum
    // step distance we can take.
    //
    if ( first_call )
    {
	first_call = FALSE;
	
	DebugPrintf ( 1 , "\n%s(): Now calibrating passability check maximum stepsize..." , __FUNCTION__ );
	
	for ( i = 0 ; i < NUMBER_OF_OBSTACLE_TYPES ; i ++ )
	{
	    if ( obstacle_map [ i ] . block_area_type == COLLISION_TYPE_RECTANGLE )
	    {
		steps_for_this_obstacle = ( 1.0 / obstacle_map [ i ] . block_area_parm_1 ) + 1 ;
		if ( steps_for_this_obstacle > step_multiplier )
		{
		    step_multiplier = steps_for_this_obstacle;
		    key_obstacle_type = i ;
		}
		
		steps_for_this_obstacle = ( 1.0 / obstacle_map [ i ] . block_area_parm_2 ) + 1 ;
		if ( steps_for_this_obstacle > step_multiplier )
		{
		    step_multiplier = steps_for_this_obstacle;
		    key_obstacle_type = i ;
		}
	    }
	}
	
	DebugPrintf ( 1 , "\n%s(): Final calibration for passablilit check produced multiplier : %d." , 
		      __FUNCTION__ , step_multiplier );
	DebugPrintf ( 1 , "\n%s(): The key obstacle type for this calibration was : %d." , 
		      __FUNCTION__ , key_obstacle_type );
	
    }
  

    //--------------------
    // First we determine the amount of steps we need to take
    // such that we can't oversee any walls or something.
    //
    if ( fabsf(x1-x2) > fabsf (y1-y2) ) LargerDistance = fabsf(x1-x2);
    else LargerDistance=fabsf(y1-y2);
    
    //--------------------
    // The larger distance must be used to compute the steps nescessary
    // for good passability check. i.e. passability check such that no
    // movement completely through an obstacle will be possible.
    //
    // The number of steps must of course be multiplied with the minimum
    // number of steps for one floor tile, which has been calibrated
    // (hopefully sensibly) above.
    //
    Steps = LargerDistance * step_multiplier ; 
    if ( Steps <= 1 ) Steps = 2 ; // return TRUE;
    
    //--------------------
    // We determine the step size when walking from (x1,y1) to (x2,y2) in Steps number of steps
    //
    step.x = (x2 - x1) / ( float ) Steps;
    step.y = (y2 - y1) / ( float ) Steps;
    
    // DebugPrintf( 2 , "\n%s():  step.x=%f step.y=%f." , __FUNCTION__ , step.x , step.y );
    
    //--------------------
    // We start from position (x1, y1)
    //
    CheckPosition . x = x1;
    CheckPosition . y = y1;
    
    for ( i = 0 ; i < Steps + 1 ; i++ )
    {
	if ( ! IsPassable ( CheckPosition . x , CheckPosition . y , z ) ) 
	{
	    DebugPrintf( 1 , "\n%s(): Connection analysis revealed : OBSTACLES!! NO WAY!!!" , __FUNCTION__ );
	    return FALSE;
	}	

	CheckPosition.x += step.x;
	CheckPosition.y += step.y;
    }

    DebugPrintf( 1 , "\n%s(): Connection analysis revealed : FREE!" , __FUNCTION__ );

    return TRUE;

}; // int DirectLineWalkable( float x1 , float y1 , float x2 , float y2 )

/* ----------------------------------------------------------------------
 * Enemys recover with time, just so.  This is done in this function, and
 * it is of course independent of the current framerate.
 * ---------------------------------------------------------------------- */
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
    for ( j = 0 ; j < 2 ; j++ )
	{
	our_bot -> PrivatePathway [ j ] . x = 0 ;
	our_bot -> PrivatePathway [ j ] . y = 0 ;
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
    
    BROWSE_ALIVE_BOTS_SAFE(erot, nerot)
	{
	list_del_init( &erot->global_list );
	list_del_init( &erot->level_list );
	free ( erot );
	}

    BROWSE_DEAD_BOTS_SAFE(erot,nerot)
	{
	list_del_init( &erot->global_list );
	list_del_init( &erot->level_list );
	free ( erot );
	}

}; // void ClearEnemys ( void ) 


/* -----------------------------------------------------------------
 * Right after loading a ship (load game, start new game, etc.), 
 * we tie every enemy to a given waypoint on the map.
 * -----------------------------------------------------------------*/
void
ShuffleEnemys ( int LevelNum )
{
    int i, j;
    int nth_enemy;
    int wp_num;
    int wp = 0;
    int BestWaypoint;
    Level ShuffleLevel = curShip.AllLevels[ LevelNum ];
    
    wp_num = ShuffleLevel->num_waypoints;;
    nth_enemy = 0;

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

	    erot->pos.x = ShuffleLevel->AllWaypoints[ BestWaypoint ].x;
	    erot->pos.y = ShuffleLevel->AllWaypoints[ BestWaypoint ].y;

	    continue;
	    }

	nth_enemy++ ;
	if ( nth_enemy < wp_num )
	    {
	    //--------------------
	    // If we can use this waypoint for random spawning
	    // then we use it
	    // "this waypoint" actually is the bot number in this level. 
	    //
	    if ( ! ShuffleLevel -> AllWaypoints [ nth_enemy ] . suppress_random_spawn )
		{
		wp = nth_enemy;
		}
	    else
		{
		i -- ;
		continue ;
		}
	    }
	else
	    {
	    DebugPrintf (0, "\nNumber of waypoints found: %d." , wp_num );
	    DebugPrintf (0, "\nLess waypoints than enemys on level %d? !", ShuffleLevel->levelnum );
	    Terminate (ERR);
	    }

	erot->pos.x = ShuffleLevel->AllWaypoints[wp].x;
	erot->pos.y = ShuffleLevel->AllWaypoints[wp].y;

	erot->lastwaypoint = wp;
	erot->nextwaypoint = wp;
	} 

}; // void ShuffleEnemys ( void ) 

/* ----------------------------------------------------------------------
 * This function checks if the connection between two points is free of
 * droids.  
 *
 * MAP TILES ARE NOT TAKEN INTO CONSIDERATION, ONLY DROIDS!!!
 *
 * ---------------------------------------------------------------------- */
int 
CheckIfWayIsFreeOfDroids (char test_tux, float x1 , float y1 , float x2 , float y2 , int OurLevel , 
					  Enemy ExceptedRobot ) 
{
    float LargerDistance;
    int Steps;
    int i;
    moderately_finepoint step;
    moderately_finepoint CheckPosition;
    static int first_call = TRUE ;
    static float steps_per_square;

    const float Druid_Radius_X = 0.25;
    const float Druid_Radius_Y = 0.25;
    //--------------------
    // Upon the very first function call, we calibrate the density of steps, so that 
    // we cannot miss out a droid by stepping right over it.
    //
    if ( first_call )
    {
	first_call = FALSE ;
	steps_per_square = 1 / ( 2.0 * sqrt(2.0) * Druid_Radius_X );
    }
    
    // DebugPrintf( 2, "\nint CheckIfWayIsFreeOfDroids (...) : Checking from %d-%d to %d-%d.", (int) x1, (int) y1 , (int) x2, (int) y2 );
    // fflush(stdout);
    
    if ( fabsf ( x1 - x2 ) > fabsf ( y1 - y2 ) ) LargerDistance = fabsf ( x1 - x2 );
    else LargerDistance = fabsf ( y1 - y2 );
    
    Steps = LargerDistance * steps_per_square + 1 ;   // We check four times on each map tile...
    // if ( Steps == 0 ) return TRUE;
    
    // We determine the step size when walking from (x1,y1) to (x2,y2) in Steps number of steps
    step.x = ( x2 - x1 ) / ((float)Steps) ;
    step.y = ( y2 - y1 ) / ((float)Steps) ;
    
    // DebugPrintf( 2 , "\nint CheckIfWayIsFreeOfDroids (...) :  step.x=%f step.y=%f." , step.x , step.y );
    
    // We start from position (x1, y1)
    CheckPosition . x = x1;
    CheckPosition . y = y1;
    
    for ( i = 0 ; i < Steps + 1 ; i++ )
    {
    	enemy *this_enemy;
	BROWSE_LEVEL_BOTS(this_enemy, OurLevel)
	{
	    if (( this_enemy -> pure_wait > 0 ) || ( this_enemy == ExceptedRobot ))
		continue;
	    
	    // so it seems that we need to test this one!!
	    if ( ( fabsf ( this_enemy -> pos . x - CheckPosition . x ) < 2.0 * Druid_Radius_X ) &&
		 ( fabsf ( this_enemy -> pos . y - CheckPosition . y ) < 2.0 * Druid_Radius_Y ) ) 
	    {
		// DebugPrintf( 2, "\nCheckIfWayIsFreeOfDroids (...) : Connection analysis revealed : TRAFFIC-BLOCKED !");
		return FALSE;
	    }
	}
	
	//--------------------
	// Now we check for collisions with the Tux himself
	//
	if ( test_tux && ( fabsf ( Me . pos.x - CheckPosition.x ) < 2 * Druid_Radius_X ) &&
	     ( fabsf ( Me . pos.y - CheckPosition.y ) < 2 * Druid_Radius_Y ) ) 
	{
	    // DebugPrintf( 2 , "\nCheckIfWayIsFreeOfDroids (...) : Connection analysis revealed : TRAFFIC-BLOCKED-INFLUENCER !");
	    return FALSE;
	}
	
	CheckPosition . x += step . x;
	CheckPosition . y += step . y;
    }
    
    return TRUE;

}; // CheckIfWayIsFreeOfDroids ( char test_tux, float x1 , float y1 , float x2 , float y2 , int OurLevel , int ExceptedDroid )

/* ----------------------------------------------------------------------
 * Once the next waypoint or the next private pathway point has been 
 * selected, this generic low_level movement function can be called to
 * actually move the robot towards this spot.
 * ---------------------------------------------------------------------- */
static void
move_enemy_to_spot ( Enemy ThisRobot , finepoint next_target_spot )
{
    finepoint remaining_way;
    float maxspeed;
    int old_map_level;

    //--------------------
    // According to properties of the robot like being frozen or not,
    // we define the maximum speed of this machine for later use...
    // A frozen robot is slow while a paralyzed robot can do absolutely nothing.
    //
    // if ( ThisRobot -> paralysation_duration_left != 0 ) return;
    
    if ( ThisRobot -> frozen == 0 )
	maxspeed = Druidmap [ ThisRobot -> type ] . maxspeed;
    else 
	maxspeed = 0.2 * Druidmap [ ThisRobot->type ] . maxspeed;
    
    //--------------------
    // While getting hit, the bot or person shouldn't be running, but
    // when standing, it should move over to the 'walk' animation type...
    //
    if ( ThisRobot -> animation_type == GETHIT_ANIMATION ) return;
    if ( ThisRobot -> animation_type == STAND_ANIMATION ) 
    {
	ThisRobot -> animation_type = WALK_ANIMATION ;
	ThisRobot -> animation_phase = 0.0 ;
    }
    
    remaining_way . x = next_target_spot . x - ThisRobot -> pos . x ;
    remaining_way . y = next_target_spot . y - ThisRobot -> pos . y ;
    
    //--------------------
    // As long a the distance from the current position of the enemy
    // to its next wp is large, movement is rather simple:
    //
    if ( fabsf ( remaining_way . x ) > Frame_Time() * maxspeed )
    {
	ThisRobot -> speed . x =
	    ( remaining_way . x / fabsf ( remaining_way . x ) ) * maxspeed;
	ThisRobot -> pos . x += ThisRobot -> speed . x * Frame_Time ();
    } 	 
    else
    {
	// --------------------
	// Once this enemy is close to his final destination waypoint, we have
	// to do some fine tuning, and then of course set the next waypoint.
	ThisRobot -> pos . x = next_target_spot . x;
	ThisRobot -> speed . x = 0;
    }
    
    if ( fabsf ( remaining_way . y )  > Frame_Time() * maxspeed )
    {
	ThisRobot -> speed . y =
	    ( remaining_way . y / fabsf ( remaining_way . y ) ) * maxspeed;
	ThisRobot -> pos . y += ThisRobot -> speed . y * Frame_Time ();
    }
    else
    {
	ThisRobot -> pos . y = next_target_spot . y ;
	ThisRobot -> speed . y = 0;
    }
    
    //--------------------
    // Now the bot is moving, so maybe it's moving over a jump threshold?
    // In any case, it might be best to check...
    //
    // In case a jump has taken place, we best also reset the current
    // waypointless wandering target.  Otherwise the bot might want to
    // walk to the end of the map before thinking again...
    //
    old_map_level = ThisRobot -> pos . z ;
    adapt_position_for_jump_thresholds ( & ( ThisRobot -> pos ) , & ( ThisRobot -> pos ) );
    if ( ThisRobot -> pos . z != old_map_level )
    {
	ThisRobot -> PrivatePathway [ 0 ] . x = ThisRobot -> pos . x ;
	ThisRobot -> PrivatePathway [ 0 ] . y = ThisRobot -> pos . y ;
	DebugPrintf ( -1 , "\n%s(): tuncated current waypoinless target because of level jump... bot at %f %f lv %d" , 
		      __FUNCTION__, ThisRobot->pos.x, ThisRobot->pos.y, ThisRobot->pos.z );
    }
    
}; // void move_enemy_to_spot ( Enemy ThisRobot , finepoint next_target_spot )

/* ----------------------------------------------------------------------
 * This function moves one robot thowards his next waypoint.  If already
 * there, the function does nothing more.
 * ---------------------------------------------------------------------- */
void 
MoveThisRobotThowardsHisCurrentTarget ( enemy * ThisRobot )
{
    finepoint nextwp_pos;
    Level WaypointLevel = curShip . AllLevels [ ThisRobot-> pos . z ];

    //--------------------
    // A frozen robot is slow while a paralyzed robot can do absolutely nothing.
    // A waiting robot shouldn't be moved either...
    //
    if ( ThisRobot -> paralysation_duration_left != 0 ) return;
    if ( ThisRobot -> pure_wait > 0 ) return;
    if ( ThisRobot -> animation_type == ATTACK_ANIMATION ) return;

    nextwp_pos . x = ThisRobot -> PrivatePathway [ 0 ] . x ;
    nextwp_pos . y = ThisRobot -> PrivatePathway [ 0 ] . y ;

    move_enemy_to_spot ( ThisRobot , nextwp_pos );

}; // void MoveThisRobotThowardsHisCurrentTarget ( int EnemyNum )

/* ----------------------------------------------------------------------
 * This function sets a new random waypoint to a bot.
 * ---------------------------------------------------------------------- */
void
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
    
    // Of course, only if such connections exist at all, we do the
    // following change of target waypoint procedure
    if (  num_conn == 0 ) // no connections found!
    {
	fprintf ( stderr , "\nThe offending waypoint nr. is: %d at %d, %d.", nextwp, WpList [ nextwp ] . x, WpList [ nextwp ] . y );
	fprintf ( stderr , "\nThe map level in question got nr.: %d.", ThisRobot -> pos . z );
	if ( ThisRobot -> stick_to_waypoint_system_by_default )
	{
	    ErrorMessage ( __FUNCTION__  , "\
There was a droid on a waypoint, that apparently has no connections to other waypoints...\n\
Since it was a waypoint-based bot, this is a fatal message in this case.",
				       NO_NEED_TO_INFORM, IS_FATAL );
	}
	else
	{
	    ErrorMessage ( __FUNCTION__  , "\
There was a droid on a waypoint, that apparently has no connections to other waypoints...\n\
Since it was NOT a waypoint-based bot, this is NOT a fatal message in this case.",
				       NO_NEED_TO_INFORM, IS_WARNING_ONLY );
	}
    }
    
    //--------------------
    // At this point, we should check, if there is another waypoint 
    // and also if the way there is free of other droids
    //
    for ( i = 0; i < num_conn ; i++ )
    {
	FreeWays [ i ] = 1; /*CheckIfWayIsFreeOfDroids ( TRUE, 
	    WpList [ ThisRobot -> lastwaypoint ] . x + 0.5 , 
	    WpList [ ThisRobot -> lastwaypoint ] . y + 0.5 , 
	    WpList [ WpList [ ThisRobot -> lastwaypoint ] . connections [ i ] ] . x + 0.5 , 
	    WpList [ WpList [ ThisRobot -> lastwaypoint ] . connections [ i ] ] . y + 0.5 , 
	    ThisRobot->pos.z , ThisRobot );*/
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
	DebugPrintf( -2 , "\n%s(): Sorry, there seems no free way out.  I'll wait then... , num_conn was : %d ." , __FUNCTION__ , num_conn );
	ThisRobot->pure_wait = 1.5 ; // this makes this droid 'passable' for other droids for now...
	/*if ( ( ThisRobot -> combat_state == MOVE_ALONG_RANDOM_WAYPOINTS ) ||
	     ( ThisRobot -> combat_state == TURN_TOWARDS_NEXT_WAYPOINT ) )
	    ThisRobot -> combat_state = WAIT_AND_TURN_AROUND_AIMLESSLY ;*/
	return;
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
    
    DebugPrintf ( 2 , "\n%s():  A new waypoint has been set." , __FUNCTION__ );
  
}; // void SetNewRandomWaypoint ( Enemy ThisRobot )

/* ----------------------------------------------------------------------
 * This function is supposed to find out if a given line on an 
 * arbitrarily chosen map can be walked by a bot or not.
 * ---------------------------------------------------------------------- */
int
droid_can_walk_this_line ( int level_num , float x1, float y1 , float x2 , float y2 )
{
    global_ignore_doors_for_collisions_flag = TRUE ;
    if ( DirectLineWalkable ( x1 , y1 , x2 , y2 , level_num ) )
	return ( TRUE );
    else
	return ( FALSE ); 
    global_ignore_doors_for_collisions_flag = FALSE ;
}; // int droid_can_walk_this_line ( int level_num , float x1, float y1 , float x2 , float y2 )

/* ----------------------------------------------------------------------
 * If the droid in question is currently not following the waypoint system
 * but rather moving around on it's own and without any real destination,
 * this function sets up randomly chosen targets for the droid.
 * ---------------------------------------------------------------------- */
static void 
set_new_waypointless_walk_target ( enemy* ThisRobot, moderately_finepoint * mt)
{
    int i ;
    moderately_finepoint target_candidate;
    int success = FALSE ;

#define MAX_RANDOM_WALK_ATTEMPTS_BEFORE_GIVING_UP 4

    if ( ThisRobot -> follow_tux )
	{
	moderately_finepoint vect;
	vect . x = ThisRobot -> virt_pos . x - Me . pos . x;
	vect . y = ThisRobot -> virt_pos . y - Me . pos . y;

	target_candidate . x = Me . pos . x;
	target_candidate . y = Me . pos . y;
	}
    else
	{
	for ( i = 0 ; i < MAX_RANDOM_WALK_ATTEMPTS_BEFORE_GIVING_UP ; i ++ )
	    {
	    //--------------------
	    // We select a possible new walktarget for this bot, not too
	    // far away from the current position...
	    //
	    target_candidate . x = ThisRobot -> pos . x + ( MyRandom ( 600 ) - 300 ) / 100 ; 
	    target_candidate . y = ThisRobot -> pos . y + ( MyRandom ( 600 ) - 300 ) / 100 ; 

	    }
	}

    if ( droid_can_walk_this_line ( ThisRobot -> pos . z , ThisRobot -> pos . x , ThisRobot -> pos . y , 
		target_candidate . x , target_candidate . y ) )
	{
	mt -> x = target_candidate . x ;
	mt -> y = target_candidate . y ;
	success = TRUE ;
	}

    if ( ! success )
	{
	DebugPrintf ( -2 , "\n%s():  bad luck with random walk point this time..." , __FUNCTION__ );
	ThisRobot -> pure_wait = 1.6 ;
	}
}; // void set_new_waypointless_walk_target ( enemy* ThisRobot )

/* ----------------------------------------------------------------------
 * This function moves one robot in an advanced way, that hasn't been
 * present within the classical paradroid game.
 * ---------------------------------------------------------------------- */
float 
remaining_distance_to_current_walk_target ( Enemy ThisRobot )
{
    Waypoint WpList;
    int nextwp;
    finepoint nextwp_pos;
    Level WaypointLevel = curShip.AllLevels[ ThisRobot -> pos.z ];
    finepoint remaining_way;

    remaining_way.x = ThisRobot -> PrivatePathway [ 0 ] . x - ThisRobot -> pos . x ;
    remaining_way.y = ThisRobot -> PrivatePathway [ 0 ] . y - ThisRobot -> pos . y ;

  return ( sqrt ( remaining_way.x * remaining_way.x + remaining_way.y * remaining_way.y ) ) ;

}; // float remaining_distance_to_current_walk_target ( Enemy ThisRobot )

/* ----------------------------------------------------------------------
 * This function tells if a given level is active in the sence that there
 * is one ore more player character on the level, so that need exists to
 * move all the enemies on this level etc.
 * ---------------------------------------------------------------------- */
int 
IsActiveLevel ( int levelnum ) 
{
    return (Me . status != INFOUT && Me . pos . z == levelnum );
}; // int IsActiveLevel ( int levelnum ) 

/* ----------------------------------------------------------------------
 * When a (hostile) robot is defeated and explodes, it will drop some 
 * treasure, i.e. stuff it had or parts that it consisted of or similar
 * things.  Maybe there will even be some extra magical treasures if the
 * robot in question was a 'boss monster'.  This function does the 
 * treasure dropping.
 * ---------------------------------------------------------------------- */
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
	if ( Druidmap [ ThisRobot->type ] . amount_of_tachyon_condensators && ( ! MyRandom ( 10 ) == 1 )  )
	    DropItemAt( GetItemIndexByName("Tachyon Condensator"), ThisRobot -> pos . z , 
			ThisRobot->virt_pos.x , ThisRobot->virt_pos.y , -1 , -1 , 1 );
	case 4:
	if ( Druidmap [ ThisRobot->type ] . amount_of_entropy_inverters && ( ! MyRandom ( 10 ) == 1 )  )
	    DropItemAt( GetItemIndexByName("Entropy Inverter") , ThisRobot -> pos . z , 
			ThisRobot->virt_pos.x , ThisRobot->virt_pos.y , -1 , -1 , 1 );
	case 3:
	if ( Druidmap [ ThisRobot->type ] . amount_of_antimatter_converters && ( ! MyRandom ( 10 ) == 1 )  )
	    DropItemAt( GetItemIndexByName("Antimatter-Matter Converter") , ThisRobot -> pos . z , 
			ThisRobot->virt_pos.x , ThisRobot->virt_pos.y , -1 , -1 , 1 );
	case 2:
	if ( Druidmap [ ThisRobot->type ] . amount_of_superconductors && ( ! MyRandom ( 10 ) == 1 )  )
	    DropItemAt( GetItemIndexByName("Superconducting Relay Unit") , ThisRobot -> pos . z , 
			ThisRobot->virt_pos.x , ThisRobot->virt_pos.y , -1 , -1 , 1 );
	case 1:    
	if ( Druidmap [ ThisRobot->type ] . amount_of_plasma_transistors && ( MyRandom ( 10 ) == 1 ) )
	    DropItemAt( GetItemIndexByName("Plasma Transistor") , ThisRobot -> pos . z , 
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

/* ----------------------------------------------------------------------
 *
 *
 * ---------------------------------------------------------------------- */
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

/* ----------------------------------------------------------------------
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
 * ---------------------------------------------------------------------- */
static void
enemy_spray_blood ( enemy *CurEnemy ) 
{
  moderately_finepoint target_pos = { 1.0 , 0 } ;

  DebugPrintf ( 1 , "\nBlood has been sprayed...%d", CurEnemy -> type );

  RotateVectorByAngle ( & target_pos , MyRandom ( 360 ) );

  target_pos . x += CurEnemy -> virt_pos . x ;
  target_pos . y += CurEnemy -> virt_pos . y ;

  
  if ( Druidmap [ CurEnemy -> type ] . is_human )
	  create_new_obstacle_on_level ( curShip . AllLevels [ CurEnemy -> pos . z ] , ISO_BLOOD_1 + MyRandom ( 7 ) , target_pos . x , target_pos . y );
  else  
	  create_new_obstacle_on_level ( curShip . AllLevels [ CurEnemy -> pos . z ] , ISO_OIL_STAINS_1 + MyRandom ( 7 ) , target_pos . x , target_pos . y );

  
}; // void enemy_spray_blood ( Enemy CurEnemy ) 




/* ----------------------------------------------------------------------
 * When a robot has reached energy <= 1, then this robot will explode and
 * die, lose some treasure and add up to the kill record of the Tux.  All
 * the things that should happen when energy is that low are handled here
 * while the check for low energy is done outside of this function namely
 * somewhere in the movement processing for this enemy.
 * ---------------------------------------------------------------------- */
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
    list_del_init(&(target->level_list)); // bot is dead? remove it from level list

    return 0;
};


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
    /* remove hp
     * turn group hostile
     * spray blood
     * enter hitstun
     * say a funny message
     * check if droid is dead
     */
    target -> energy -= hit;
    
    if ( mine ) // tux hit ? we turn the group hostile !
	{
	    robot_group_turn_hostile ( target );
	}

    if ( target->is_friendly && givexp )
	givexp = 0; // force NO XP for killing a friendly bot

    enemy_spray_blood ( target ) ;
    
    if(target -> firewait < Druidmap [ target -> type ] . recover_time_after_getting_hit && MyRandom(100) <= 40)
	target -> firewait = Druidmap [ target -> type ] . recover_time_after_getting_hit ;

    start_gethit_animation_if_applicable ( target) ;

    EnemyHitByBulletText( target );

    PlayEnemyGotHitSound ( Druidmap [ target->type ] . got_hit_sound_type );

    if ( target -> energy <= 0 )
	{
	kill_enemy(target, givexp, killertype);
	}
}

/* ----------------------------------------------------------------------
 *
 *
 * ---------------------------------------------------------------------- */
void
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

/* ----------------------------------------------------------------------
 * This function moves a single enemy.  It is used by MoveEnemys().
 * ---------------------------------------------------------------------- */
void 
MoveThisEnemy( enemy * ThisRobot )
{
    
    //--------------------
    // robots that still have to wait also do not need to
    // be processed for movement
    //
    if ( ThisRobot -> pure_wait > 0 ) return;
    
    CheckEnemyEnemyCollision ( ThisRobot );
    
    MoveThisRobotThowardsHisCurrentTarget( ThisRobot );

    DetermineAngleOfFacing ( ThisRobot );

}; 


/* ----------------------------------------------------------------------
 * This function returns a gps (position) for a robot's current target, 
 * or NULL if such target doesn't exist
 * ---------------------------------------------------------------------- */
static gps * enemy_get_target_position ( enemy *ThisRobot )
{
    if ( ( ThisRobot -> attack_target_type == ATTACK_TARGET_IS_PLAYER ) )
	return & Me . pos;
    else if ( ThisRobot -> attack_target_type == ATTACK_TARGET_IS_ENEMY )
	return & enemy_resolve_address(ThisRobot->bot_target_n, &ThisRobot->bot_target_addr) -> pos;
    else 
	{ /* No (more) target */
	return NULL;
	}
}

/* ----------------------------------------------------------------------
 * More for debugging purposes, we print out the current state of the
 * robot as his in-game text.
 * ---------------------------------------------------------------------- */
    void
enemy_say_current_state_on_screen ( enemy* ThisRobot )
{
    switch ( ThisRobot -> combat_state )
	{                  
	case MOVE_ALONG_RANDOM_WAYPOINTS:
	    ThisRobot->TextToBeDisplayed = _("state:  Wandering along waypoints.") ;
	    break;     
	case SELECT_NEW_WAYPOINT:
	    ThisRobot->TextToBeDisplayed = _("state: Select next WP.");
	    break;
	case TURN_TOWARDS_NEXT_WAYPOINT:
	    ThisRobot->TextToBeDisplayed = _("state:  Turn towards next WP.") ;
	    break;     
	case RUSH_TUX_AND_OPEN_TALK:
	    ThisRobot->TextToBeDisplayed = _("state:  Rush Tux and open talk.") ;
	    break;     
	case STOP_AND_EYE_TARGET:
	    ThisRobot->TextToBeDisplayed = _("state:  Stop and eye target.") ;
	    break;     
	case ATTACK:
	    ThisRobot->TextToBeDisplayed = _("state:  Attack.") ;
	    break;     
	case RETURNING_HOME:
	    ThisRobot->TextToBeDisplayed = _("state:  Returning home.") ;
	    break;
	case WAYPOINTLESS_WANDERING:
	    ThisRobot->TextToBeDisplayed = _("state:  Waypointless wandering.") ;
	    break;
	case TURN_TOWARDS_WAYPOINTLESS_SPOT:
	    ThisRobot->TextToBeDisplayed = _("state:  Waypointless turning.") ;
	    break;
	case PARALYZED:
	    ThisRobot->TextToBeDisplayed = _("state:  Paralyzed.") ;
	    break;
	case COMPLETELY_FIXED:
	    ThisRobot->TextToBeDisplayed = _("state: Completely fixed.");
	    break;
	case FOLLOW_TUX:
	    ThisRobot->TextToBeDisplayed = _("state: Follow Tux.") ;
	    break;
	default:
	    ThisRobot->TextToBeDisplayed = _("state:  UNHANDLED!!") ;
	    break;
	}      
    ThisRobot->TextVisibleTime = 0 ; 
}; // void enemy_say_current_state_on_screen ( enemy* ThisRobot )


/* ----------------------------------------------------------------------
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
 * ---------------------------------------------------------------------- */
void
enemy_handle_stuck_in_walls ( enemy* ThisRobot )
{
return;
#if 0
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
    if ( !IsPassable ( ThisRobot -> pos . x , ThisRobot -> pos . y , ThisRobot -> pos.z ) )
    {
	// so the bot is currently inside of some wall.  hmmm.  best thing to do might
	// be to see if this has been going on for some time.  In that case we would really
	// have to do something about the problem.
	//
	if ( ThisRobot -> bot_stuck_in_wall_at_previous_check )
	{
	    // --------------------
	    // Maybe the robot in question was even sticking to the current 
	    // waypoint system!  That might indicate, that the waypoint system
	    // has pathes, that run too close to some walls or bigger obstacles
	    // This should be fixed inside the map, because upon switching to
	    // wapointless mode, the bot might suddenly be unable to move at all
	    // and stuck in the wall at the same time.
	    //
	    switch ( ThisRobot -> combat_state )
	    {
		case MOVE_ALONG_RANDOM_WAYPOINTS:
		case TURN_TOWARDS_NEXT_WAYPOINT:
		    DebugPrintf ( -2 , "\n\nFound robot, that seems really stuck on position: %f/%f/%d." ,
				  ThisRobot -> pos . x , ThisRobot -> pos . y , ThisRobot -> pos.z );
		    DebugPrintf ( -2 , "\nMore details on this robot:  Type=%d. has_greeted_influencer=%d." ,
				  ThisRobot -> type , ThisRobot -> has_greeted_influencer );
		    enemy_say_current_state_on_screen ( ThisRobot ); // safety:  init the TextToBeDisplayed 
		    DebugPrintf ( -2 , "\nnextwaypoint=%d. lastwaypoint=%d. combat_%s." ,
				  ThisRobot -> nextwaypoint , ThisRobot -> lastwaypoint , 
				  ThisRobot -> TextToBeDisplayed );
		    ErrorMessage ( __FUNCTION__  , "\
There was a bot, that was found to be inside a wall. This may be a bug in waypoint placement\n\
Type=%d. pos =  %f/%f/%d\n\
has_greeted_influencer=%d  combat_%s. \n\
nextwaypoint=%d. lastwaypoint=%d\n" ,
					       NO_NEED_TO_INFORM, IS_WARNING_ONLY, ThisRobot -> type,  ThisRobot -> pos . x , ThisRobot -> pos . y , ThisRobot -> pos.z, ThisRobot -> has_greeted_influencer, ThisRobot -> TextToBeDisplayed, ThisRobot -> nextwaypoint, ThisRobot -> lastwaypoint );
		    ThisRobot -> bot_stuck_in_wall_at_previous_check = TRUE ; 
		    return;
		    break;
		default: 
		    break;
	    }

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
	    DebugPrintf ( -2 , "\nMore details on this robot:  Type=%d. has_greeted_influencer=%d." ,
			  ThisRobot -> type , ThisRobot -> has_greeted_influencer );
	    DebugPrintf ( -2 , "\nPrivate Pathway[0]: %f/%f." , 
			  ThisRobot -> PrivatePathway [ 0 ] . x ,
			  ThisRobot -> PrivatePathway [ 0 ] . y );
	    DebugPrintf ( -2 , "\nPrivate Pathway[1]: %f/%f." , 
			  ThisRobot -> PrivatePathway [ 1 ] . x ,
			  ThisRobot -> PrivatePathway [ 1 ] . y );
	    DebugPrintf ( -2 , "\nnextwaypoint: %d." , 
			  ThisRobot -> nextwaypoint );

	    enemy_say_current_state_on_screen ( ThisRobot ); // safety:  init the TextToBeDisplayed 
	    DebugPrintf ( -2 , "\nnextwaypoint=%d. lastwaypoint=%d. combat_%s." ,
			  ThisRobot -> nextwaypoint , ThisRobot -> lastwaypoint , 
			  ThisRobot -> TextToBeDisplayed );
	    ErrorMessage ( __FUNCTION__  , "\
There was a bot MOVING ON ITS OWN, that was found to be repeatedly inside a wall.\n\
WARNING!  EMERGENCY FALLBACK ENABLED --> Teleporting back to closest waypoint." ,
				       NO_NEED_TO_INFORM, IS_WARNING_ONLY );
	    ThisRobot -> bot_stuck_in_wall_at_previous_check = TRUE ; 
	    TeleportToClosestWaypoint ( ThisRobot );
	    set_new_waypointless_walk_target ( ThisRobot );
	    return;
	}
	ThisRobot -> bot_stuck_in_wall_at_previous_check = TRUE ; 
    }
    else
    {
	// this bot isn't currently stuck.  what more could anybody want?
	ThisRobot -> bot_stuck_in_wall_at_previous_check = FALSE ;
    }
#endif 
}; // enemy_handle_stuck_in_walls ( enemy* ThisRobot )

/* ----------------------------------------------------------------------
 * This function computes the distance a certain robot has with respect
 * to Tux, i.e. player 0 in the game.  If the Tux and the bot in question
 * are on the same level, then everything is pretty simple.  However, if
 * the enemy is on another level that is connected to this level via an
 * interface area, then of course we need to take more care.
 * ---------------------------------------------------------------------- */
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

/* ----------------------------------------------------------------------
 * This function selects a target for a friendly bot. Its simply takes
 * the first enemy bot in view range. This could be improved at a later point.
 *
 * ---------------------------------------------------------------------- */
void
update_vector_to_shot_target_for_friend ( enemy* ThisRobot , moderately_finepoint* vect_to_target )
{
    float IgnoreRange = Druidmap [ ThisRobot -> type ] . range_of_vision;
    int found_some_target = FALSE;


    //--------------------
    // We set some default values, in case there isn't anything attackable
    // found below...
    //
    vect_to_target -> x = -1000;
    vect_to_target -> y = -1000;

        
    enemy *erot;
    BROWSE_LEVEL_BOTS(erot, ThisRobot->pos.z)
    {
	if ( erot->is_friendly )
	    continue;
	
	if ( sqrt ( ( ThisRobot -> pos . x - erot->pos . x ) *
		    ( ThisRobot -> pos . x - erot->pos . x ) +
		    ( ThisRobot -> pos . y - erot->pos . y ) *
		    ( ThisRobot -> pos . y - erot->pos . y ) ) > IgnoreRange ) 
	    continue;

	// At this point we have found our target
	vect_to_target -> x = erot->pos . x - ThisRobot -> pos . x ;
	vect_to_target -> y = erot->pos . y - ThisRobot -> pos . y ;
	DebugPrintf( 0 , "\nPOSSIBLE TARGET FOR FRIENDLY DROID FOUND!!!\n");
	DebugPrintf( 0 , "\nIt is a good target for: %s.\n", ThisRobot -> dialog_section_name );
	found_some_target = TRUE ;
	break;
    }
    
    if ( found_some_target ) 
    {
	ThisRobot -> attack_target_type = ATTACK_TARGET_IS_ENEMY ;
	enemy_set_reference(&ThisRobot -> bot_target_n, &ThisRobot->bot_target_addr, erot);
    }
    else	
	ThisRobot -> attack_target_type = ATTACK_TARGET_IS_NOTHING ;
    
}; // void update_vector_to_shot_target_for_friend ( ThisRobot , moderately_finepoint* vect_to_target )

/* ----------------------------------------------------------------------
 * This function selects an attack target for an hostile bot.
 * Selected target is the previous target if it is still valid (see paragraph bleow),
 * or the closest bot.
 *
 * For gameplay value purposes, it also performs a little hack : the target of 
 * the previous frame can be selected even if it is "slightly" out of view (2 times the range)
 * range, it order to simulate "pursuit". Sorry for the mess but there is no other
 * proper place for that.
 *
 * ---------------------------------------------------------------------- */
void
update_vector_to_shot_target_for_enemy ( enemy* this_robot , moderately_finepoint* vect_to_target )
{
    int our_level = this_robot -> pos . z ;
    float best_dist, our_dist;
    float xdist, ydist;

    gps old_target_pos;
    gps * a = enemy_get_target_position(this_robot);
    if ( !a ) old_target_pos.x = -1000;
    else 
	{
	old_target_pos . x = a->x;
	old_target_pos . y = a->y;
	}

    if ( old_target_pos.x != -1000 && (old_target_pos.x * old_target_pos.x + old_target_pos.y * old_target_pos.y) < 2 *  Druidmap[this_robot->type].range_of_vision )
	{
	vect_to_target -> x = old_target_pos.x - this_robot->pos.x;
	vect_to_target -> y = old_target_pos.y - this_robot->pos.y;
	//attack_target_type is left unmodified
	return;
	}
    
    //--------------------
    // By default, we set the target of this bot to the Tux himself
    // i.e. the closest (visible?) player.
    //
    vect_to_target -> x = Me . pos . x - this_robot -> virt_pos . x ;
    vect_to_target -> y = Me . pos . y - this_robot -> virt_pos . y ;
    this_robot -> attack_target_type = ATTACK_TARGET_IS_PLAYER ;
    
    //--------------------
    // This function is time-critical, so we work with squares in the
    // following and avoid computation of roots entirely
    //
    best_dist = vect_len ( *vect_to_target );
    best_dist = best_dist * best_dist ;
    
    //--------------------
    // But maybe there is a friend of the Tux also close.  Then maybe we
    // should attack this one instead, since it's much closer anyway.
    // Let's see...
    //
    enemy *erot;
    BROWSE_LEVEL_BOTS(erot, our_level)
    {
	if ( ! erot->is_friendly )
	    continue;
	
	xdist = this_robot -> pos . x - erot->pos . x ;
	ydist = this_robot -> pos . y - erot->pos . y ;

	our_dist = xdist * xdist + ydist * ydist ;
	
	if ( our_dist > 25.0 )
	    continue;
	
	if ( our_dist < best_dist )
	{
	    best_dist = our_dist ;
	    
	    vect_to_target -> x = erot->pos . x - this_robot -> pos . x ;
	    vect_to_target -> y = erot->pos . y - this_robot -> pos . y ;
	    this_robot -> attack_target_type = ATTACK_TARGET_IS_ENEMY ;
	    enemy_set_reference(&this_robot->bot_target_n, &this_robot->bot_target_addr, erot);
	}

    }
    
    if ( sqrt(vect_to_target -> x * vect_to_target->x + vect_to_target -> y * vect_to_target->y ) > Druidmap[this_robot->type].range_of_vision )
	{
	vect_to_target->x = -1000;
	vect_to_target->y = -1000;
	this_robot -> attack_target_type = ATTACK_TARGET_IS_NOTHING;
	}
}; // int update_vector_to_shot_target_for_enemy ( ThisRobot , moderately_finepoint* vect_to_target )


/* ----------------------------------------------------------------------
 * This function handles the inconditional updates done to the bots by
 * the automaton powering them. See update_enemy().
 * ---------------------------------------------------------------------- */
static void state_machine_inconditional_updates ( enemy * ThisRobot, moderately_finepoint * vect_to_target)
{
    // Robots that are paralyzed are completely stuck and do not 
    // see their state machine running
    /* XXX paralyzation MUST BE A STATE BY ITSELF
    if ( ThisRobot -> paralysation_duration_left != 0 ) 
	return;*/
    
    //--------------------
    // For debugging purposes we display the current state of the robot
    // in game
    enemy_say_current_state_on_screen ( ThisRobot );

    //--------------------
    // we check whether the current robot is 
    // stuck inside a wall or something...
    //
    // This should go away. ASAP.
    enemy_handle_stuck_in_walls ( ThisRobot );

    //--------------------
    // determine the distance vector to the target of this shot.  The target
    // depends of course on wheter it's a friendly device or a hostile device.
    //
    if ( ThisRobot->is_friendly )
	{
	update_vector_to_shot_target_for_friend ( ThisRobot , vect_to_target ) ;
	}
    else
	{
	update_vector_to_shot_target_for_enemy ( ThisRobot , vect_to_target ) ;
	}
 
    ThisRobot->speed.x=0;
    ThisRobot->speed.y=0;

}

/* ----------------------------------------------------------------------
 * This function handles state transitions based solely (or almost) on 
 * the external situation and not on the current state of the bot. 
 * The purpose is to reduce code duplication in the "big switch" that follows.
 * ---------------------------------------------------------------------- */
static void state_machine_situational_transitions ( enemy * ThisRobot, const moderately_finepoint * vect_to_target )
{
    /* The various situations are listed in increasing priority order (ie. they may override each other, so the least priority comes first. */
    /* In an ideal world, this would not exist and be done for each state. But we're in reality and have to limit code duplication. */

    /* Rush Tux when he's close */     
    if ( ThisRobot -> will_rush_tux  &&  ( powf ( Me . pos . x - ThisRobot -> pos . x , 2 ) + powf ( Me . pos . x - ThisRobot -> pos . x , 2 ) ) < 16 )
	{
	ThisRobot -> combat_state = RUSH_TUX_AND_OPEN_TALK ;
	}

    /* Return home if we're too far away */
    if ( ThisRobot -> max_distance_to_home != 0 &&
	    sqrt(powf(curShip.AllLevels[ThisRobot->pos.z]->AllWaypoints[ThisRobot->homewaypoint].x - ThisRobot->pos.x, 2) +
		powf(curShip.AllLevels[ThisRobot->pos.z]->AllWaypoints[ThisRobot->homewaypoint].y - ThisRobot->pos.y, 2))
	    > ThisRobot->max_distance_to_home )
	{
	ThisRobot->combat_state = RETURNING_HOME;
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
    if ( vect_to_target -> x != - 1000 && droid_can_walk_this_line ( ThisRobot->pos.z , ThisRobot->pos.x + vect_to_target->x , ThisRobot->pos.y + vect_to_target->y, ThisRobot -> pos . x , ThisRobot -> pos . y ))
	{
	    ThisRobot -> combat_state = STOP_AND_EYE_TARGET;
	}

}

/* ----------------------------------------------------------
 * "stop and eye tux" state handling 
 * ---------------------------------------------------------- */
static void state_machine_stop_and_eye_target ( enemy * ThisRobot, moderately_finepoint * new_move_target )
{
    gps * tpos = enemy_get_target_position( ThisRobot );
    
    /* Don't move */
    new_move_target -> x = ThisRobot -> pos . x;
    new_move_target -> y = ThisRobot -> pos . y;

    /* If no more target */ 
    if ( ! tpos ) 
	{
	ThisRobot -> state_timeout = 0;
	ThisRobot -> combat_state = SELECT_NEW_WAYPOINT;
	return;
	}

    /* Make sure we're looking at the target */
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
	if ( !ThisRobot -> attack_run_only_when_direct_line || ( ThisRobot -> attack_run_only_when_direct_line && DirectLineWalkable( ThisRobot -> pos . x , ThisRobot -> pos . y , tpos -> x , tpos -> y , ThisRobot -> pos . z )))
	    {
	    SetRestOfGroupToState ( ThisRobot , ATTACK );
	    ThisRobot -> combat_state = ATTACK ;
	    }

	if ( Druidmap [ ThisRobot -> type ] . greeting_sound_type != (-1))
	    {
	    play_enter_attack_run_state_sound ( Druidmap[ ThisRobot->type ].greeting_sound_type );
	    }

	}
}

/* ---------------------------------
 * "attack tux" state
 * --------------------------------- */
static void state_machine_attack(enemy * ThisRobot, moderately_finepoint * new_move_target)
{
    gps * tpos = enemy_get_target_position(ThisRobot);

    if ( !tpos || ! droid_can_walk_this_line ( ThisRobot->pos. z , ThisRobot -> pos . x , ThisRobot -> pos . y , tpos-> x , tpos->y ) ) 
	{
	//--------------------
	// If a bot is attacking, but has lost eye contact with the target, it should return to waypointless
	// wandering or waypoint based movement.
	ThisRobot -> combat_state = SELECT_NEW_WAYPOINT;
	/*XXX not here set_new_waypointless_walk_target ( ThisRobot );*/
	return;
	}	    
    
    float dist2 = (ThisRobot -> pos . x - tpos -> x) * (ThisRobot -> pos . x - tpos -> x) + (ThisRobot -> pos . y - tpos -> y) * (ThisRobot -> pos . y - tpos -> y);

    //--------------------
    // We will often have to move towards our target.
    //
    // But this moving around can lead to jittering of droids moving back and 
    // forth between two positions very rapidly.  Therefore we will not do this
    // movement thing every frame, but rather only sometimes
    //
    if ( ThisRobot -> last_combat_step > 0.20 )
    {
        /* Depending on the weapon of the bot, we will go *to* melee combat or try and avoid it */
	ThisRobot -> last_combat_step = 0 ; 

    if ( ItemMap [ Druidmap [ ThisRobot -> type ] . weapon_item . type ] . item_weapon_is_melee )
	{
	if ( dist2 > 2.25 )
	    MoveInCloserForOrAwayFromMeleeCombat ( ThisRobot , (+1), new_move_target );
	}
	else if ( dist2 < 1.5 )
	{
	    MoveInCloserForOrAwayFromMeleeCombat ( ThisRobot , (-1), new_move_target );
	} 
    }
    else
	ThisRobot -> last_combat_step += Frame_Time ();
    
    //--------------------
    // Melee weapons have a certain limited range.  If such a weapon is used,
    // don't fire if the influencer is several squares away!
    //
    if ( ( ItemMap [ Druidmap [ ThisRobot -> type ] . weapon_item . type ] . item_weapon_is_melee ) && 
	 ( dist2 > 2.25 ) ) return;
    
    if ( ThisRobot->firewait ) return;
    
    RawStartEnemysShot( ThisRobot , tpos->x - ThisRobot->pos.x, tpos->y - ThisRobot->pos.y);

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
    new_move_target -> x =  curShip.AllLevels[ThisRobot->pos.z]->AllWaypoints[ThisRobot->homewaypoint].x;
    new_move_target -> y =  curShip.AllLevels[ThisRobot->pos.z]->AllWaypoints[ThisRobot->homewaypoint].y;

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
    /* Bot must select a new waypoint randomly, and turn towards it. No move this step.*/
    SetNewRandomWaypoint( ThisRobot );

    /* Move target - none */
    new_move_target -> x = ThisRobot -> pos . x ;
    new_move_target -> y = ThisRobot -> pos . y ;

    ThisRobot->combat_state = TURN_TOWARDS_NEXT_WAYPOINT;
}


static void state_machine_turn_towards_next_waypoint(enemy * ThisRobot, moderately_finepoint * new_move_target)
{
    /* Action */
    /* XXX */
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
    if ( remaining_distance_to_current_walk_target ( ThisRobot ) < 0.1 )
	{
	ThisRobot -> combat_state = SELECT_NEW_WAYPOINT ;
	return;
	}

    ThisRobot -> combat_state = MOVE_ALONG_RANDOM_WAYPOINTS;
}

static void state_machine_rush_tux_and_open_talk(enemy * ThisRobot, moderately_finepoint * new_move_target)
{
    /* Move target */
    new_move_target -> x = Me . pos . x;
    new_move_target -> y = Me . pos . y;

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
    /* Move target */
    new_move_target -> x = Me . pos . x + (ThisRobot->pos.x - Me.pos.x)/fabsf(ThisRobot->pos.x - Me.pos.x);
    new_move_target -> y = Me . pos . y + (ThisRobot->pos.y - Me.pos.y)/fabsf(ThisRobot->pos.y - Me.pos.y);

}

static void state_machine_completely_fixed ( enemy * ThisRobot, moderately_finepoint * new_move_target )
{
    /* Move target */
    new_move_target -> x = ThisRobot-> pos . x;
    new_move_target -> y = ThisRobot-> pos . y;
}

static void state_machine_waypointless_wandering(enemy * ThisRobot, moderately_finepoint * new_move_target)
{
}

static void state_machine_turn_towards_waypointless_spot(enemy * ThisRobot, moderately_finepoint * new_move_target)
{
}

/* ----------------------------------------------------------------------
 * 
 * This function runs the finite state automaton that powers the bots.
 * It handles attack and movement behaviors.
 *
 * ---------------------------------------------------------------------- */
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
    moderately_finepoint vect_to_target;
  
    /* Inconditional updates */
    state_machine_inconditional_updates(ThisRobot, &vect_to_target); 
    
    /* Situational state changes */
    state_machine_situational_transitions(ThisRobot, &vect_to_target);

    moderately_finepoint new_move_target = { ThisRobot->PrivatePathway[0].x, ThisRobot->PrivatePathway[0].y };

    /* Handle per-state switches and actions.
     * Each state much set move_target and combat_state.
     */
    switch ( ThisRobot -> combat_state )
	{
	case STOP_AND_EYE_TARGET:
	    state_machine_stop_and_eye_target ( ThisRobot, &new_move_target); 
	    break;

	case ATTACK:
	    state_machine_attack ( ThisRobot, &new_move_target );
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

	case TURN_TOWARDS_WAYPOINTLESS_SPOT:
	    state_machine_turn_towards_waypointless_spot ( ThisRobot, &new_move_target );
	    break;
	}

    /* Pathfind current target */
    /* XXX non implemented */
    if ( new_move_target . x != ThisRobot -> PrivatePathway[0] . x && new_move_target . y != ThisRobot -> PrivatePathway[0] . y )
	{
	ThisRobot -> PrivatePathway[0] . x = new_move_target . x;
	ThisRobot -> PrivatePathway[0] . y = new_move_target . y;
	}

	
    if ( ThisRobot -> PrivatePathway[0] . x != ThisRobot->pos.x || ThisRobot -> PrivatePathway[0] . y != ThisRobot->pos.y )
	MoveThisEnemy(ThisRobot);
    /* CLEAN UP LIMIT --- anything below this is adults only :) */
#if 0 
    else if ( ThisRobot -> combat_state == WAYPOINTLESS_WANDERING )
    {
	MoveThisRobotThowardsHisCurrentTarget( ThisRobot );
	if ( remaining_distance_to_current_walk_target ( ThisRobot ) < 0.1 ) 
	{
	    set_new_waypointless_walk_target ( ThisRobot );
	    ThisRobot -> combat_state = TURN_TOWARDS_WAYPOINTLESS_SPOT ;
	    return;
	}
	
	
	//--------------------
	// In case that the enemy droid isn't even aware of Tux and
	// does not even see Tux now, there's nothing more to do here...
	// Not even the combat state will change.
	//
	if ( DistanceToTux( ThisRobot ) > Druidmap [ ThisRobot -> type ] . range_of_vision ) return;
	
	//--------------------
	// But if the Tux is now within range of vision, then it will be
	// 'greeted' and also the state will finally move to something more
	// interesting...
	//
	if ( ThisRobot -> has_greeted_influencer == FALSE )
	{
	    ThisRobot->has_greeted_influencer = TRUE;
	    if ( Druidmap[ ThisRobot->type ].greeting_sound_type != (-1) )
	    {
		DebugPrintf ( 1 , "\n%s(): Playing greeting sound for bot of type %d." , 
			      __FUNCTION__ , ThisRobot -> type );
		PlayGreetingSound( Druidmap[ ThisRobot->type ].greeting_sound_type );
	    }
	}
	
	check_if_switching_to_stopandeyetuxmode_makes_sense ( ThisRobot );

	return; 
    }
    else if ( ThisRobot -> combat_state == TURN_TOWARDS_WAYPOINTLESS_SPOT )
    {
	//--------------------
	// We allow arbitrary turning speed in this case... so we disable
	// the turning control in display function...
	//
	ThisRobot -> last_phase_change = 100 ; 
	if ( 
	    TurnABitTowardsPosition ( ThisRobot , 
				       ThisRobot -> PrivatePathway [ 0 ] . x ,
				       ThisRobot -> PrivatePathway [ 0 ] . y ,
				       90 )
	    )
	{
	    //--------------------
	    // Maybe there should be some more waiting here, so that the bots and
	    // characters don't appear so very busy and moving so abruptly, but that
	    // may follow later...
	    //
	    ThisRobot -> combat_state = WAYPOINTLESS_WANDERING ;
	}
	return;
	
    }
    else
    {
	DebugPrintf ( -1000 , "\nWARNING:  Unidentified combat_state encountered: code=%hd!" , ThisRobot -> combat_state );
	MoveThisEnemy ( ThisRobot ); // this will now be done in the attack state machine...
	return;
    }
	
    MoveThisEnemy ( ThisRobot ); // this will now be done in the attack state machine...
    
#endif
}; // void update_enemy()

/* ----------------------------------------------------------------------
 * This function handles all the logic tied to enemies : animation, movement
 * and attack behavior.
 *
 * Note that no enemy must be killed by the logic function. It's a technical limitation
 * and a requirement in freedroidRPG.
 * ---------------------------------------------------------------------- */
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
	    if ( ( ! level_is_partly_visible ( ThisRobot -> pos . z ) ) ) 
		continue;

	    animate_enemy(ThisRobot);

	    /* If we are processing dead bots, simply do animation stuff.
	     * The rest is for alive bots only. */
	    if ( i == 0 ) 
		continue;

	    //--------------------
	    // Ignore robots, that are in the middle of their attack movement,
	    // because during attack motion, the feet of the animation are
	    // usually not moving, therefore it sometimes looks very bad when
	    // bot position is changing during attach cycle.  That's for better
	    // looks only.
	    //
	    // Remainder: animation is done somehere else.
	    /*if ( ThisRobot -> animation_type == ATTACK_ANIMATION )
		continue ;*/

	    //--------------------
	    // Run a new cycle of the bot's state machine
	    update_enemy ( ThisRobot );

	    }

}; // MoveEnemys( void ) 

/* ----------------------------------------------------------------------
 * When an enemy is firing a shot, the newly created bullet must be 
 * assigned a speed, that would lead the bullet thowards the intended
 * target, which is done here.
 * ---------------------------------------------------------------------- */
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

/* ----------------------------------------------------------------------
 * This function is low-level:  It simply sets off a shot from enemy
 * through the pointer ThisRobot at the target VECTOR xdist ydist, which
 * is a DISTANCE VECTOR, NOT ABSOLUTE COORDINATES OF THE TARGET!!!
 * ---------------------------------------------------------------------- */
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

/* ----------------------------------------------------------------------
 * This function should determine the closest visible player to this 
 * enemy droid.
 * ---------------------------------------------------------------------- */
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


/* ----------------------------------------------------------------------
 *
 * ---------------------------------------------------------------------- */
int
EnemyOfTuxCloseToThisRobot ( Enemy ThisRobot , moderately_finepoint* vect_to_target )
{
  float IgnoreRange = Druidmap [ ThisRobot -> type ] . range_of_vision;

  enemy *erot;

  BROWSE_LEVEL_BOTS(erot, ThisRobot->pos.z)
    {
      if ( erot->is_friendly )
      	  continue;
      if ( DirectLineWalkable ( ThisRobot -> pos . x , ThisRobot -> pos . y , 
				erot->pos . x , erot->pos . y , 
				ThisRobot -> pos . z ) != TRUE )
	  continue;
      if ( sqrt ( ( ThisRobot -> pos . x - erot->pos . x ) *
		  ( ThisRobot -> pos . x - erot->pos . x ) +
		  ( ThisRobot -> pos . y - erot->pos . y ) *
		  ( ThisRobot -> pos . y - erot->pos . y ) ) > IgnoreRange )
	  continue;

      // At this point we have found our target
      vect_to_target -> x = erot->pos . x - ThisRobot -> pos . x ;
      vect_to_target -> y = erot->pos . y - ThisRobot -> pos . y ;
      DebugPrintf( 0 , "\nPOSSIBLE TARGET FOR FRIENDLY DROID FOUND!!!\n");
      DebugPrintf( 0 , "\nIt is: %s.\n", ThisRobot -> dialog_section_name );
      return ( TRUE );
    }
  return ( FALSE );

}; // int EnemyOfTuxCloseToThisRobot ( Enemy ThisRobot )

/* ----------------------------------------------------------------------
 * In some of the movement functions for enemy droids, we consider making
 * a step and move a bit into one direction or the other.  But not all
 * moves are really allowed and feasible.  Therefore we need a function
 * to check if a certain step makes sense or not, which is exactly what
 * this function is supposed to do.
 *
 * ---------------------------------------------------------------------- */
int
ConsideredMoveIsFeasible ( Enemy ThisRobot , moderately_finepoint StepVector )
{
    float vec_len;
    
    vec_len = vect_len ( StepVector );

    if ( ( IsPassable ( ThisRobot -> pos.x + StepVector.x , 
			ThisRobot -> pos.y + StepVector.y ,
			ThisRobot -> pos.z ) ) && 
	 ( CheckIfWayIsFreeOfDroids ( TRUE, ThisRobot->pos.x , ThisRobot->pos.y , 
						     ThisRobot->pos.x + StepVector . x , 
						     ThisRobot->pos.y + StepVector . y ,
						     ThisRobot->pos.z , ThisRobot ) ) )
    {
	return TRUE;
    }
    
    return FALSE;

}; // int ConsideredMoveIsFeasible ( Enemy ThisRobot , finepoint StepVector )

/* ----------------------------------------------------------------------
 *
 * ---------------------------------------------------------------------- */
static void
MoveInCloserForOrAwayFromMeleeCombat ( Enemy ThisRobot , int DirectionSign, moderately_finepoint * set_move_tgt )
{
    finepoint VictimPosition = { 0.0, 0.0};
    finepoint CurrentPosition = { 0.0, 0.0};
    moderately_finepoint StepVector;
    moderately_finepoint RotatedStepVector;
    float StepVectorLen;
    int i ;

#define ANGLES_TO_TRY 7
    float RotationAngleTryList[ ANGLES_TO_TRY ] = { 0 , 30 , 360-30 , 60, 360-60, 90, 360-90 };
    
    //--------------------
    // When the robot is just getting hit, then there shouldn't be much
    // of a running motion, especially during the corresponding animaiton
    // phase...
    //
    /* XXX HITSTUN shall be a separate state, similar to paralyzed 
    if ( ThisRobot -> animation_type == GETHIT_ANIMATION ) 
	return;*/
 
    DirectionSign = (DirectionSign < 0 ? (-3) : DirectionSign);   

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
	    StepVector . x = 3.0 ;
	    StepVector . y = 3.0 ;
	}
	else
	{
	    StepVector . x = -3.0 ;
	    StepVector . y = -3.0 ;
	}
    }
     
    StepVectorLen = sqrt ( ( StepVector . x ) * ( StepVector . x ) + ( StepVector . y ) * ( StepVector . y ) );

    StepVector . x /= ( DirectionSign * StepVectorLen ) ;
    StepVector . y /= ( DirectionSign * StepVectorLen ) ;

    for ( i = 0 ; i < ANGLES_TO_TRY ; i ++ )
    {
	RotatedStepVector.x = StepVector.x ;
	RotatedStepVector.y = StepVector.y ;
	RotateVectorByAngle ( & RotatedStepVector , RotationAngleTryList [ i ] ) ;
	
	//--------------------
	// Maybe we've found a solution, then we can take it and quit
	// trying around...
	//
	if ( /*XXX*/ ConsideredMoveIsFeasible ( ThisRobot , RotatedStepVector ) )
	{
	    set_move_tgt -> x = ThisRobot -> pos.x + RotatedStepVector . x ;
	    set_move_tgt -> y = ThisRobot -> pos.y + RotatedStepVector . y ;
	    break;
	}
    }
#if 0  
    /*XXX not the right place for that*/
    if ( i >= ANGLES_TO_TRY ) 
    {
	//--------------------
	// Well, who is the closest (other) robot?
	//
	e = ClosestOtherEnemyDroid ( ThisRobot );
	
	StepVector . x = ThisRobot -> pos . x - e-> pos . x ;
	StepVector . y = ThisRobot -> pos . y - e-> pos . y ;
	StepVectorLen = sqrt ( ( StepVector . x ) * ( StepVector . x ) + ( StepVector . y ) * ( StepVector . y ) );
	
	//--------------------
	// If can happen, that two droids are EXACTLY on top of each other.  This
	// is possible by starting teleportation of a special force right on top
	// of a random bot for example.  But we should not cause a FLOATING POINT
	// EXCEPTION here!  AND we should also do a sensible handling...
	//
	if ( StepVectorLen )
	{
	    StepVector . x /= ( DirectionSign * 2 * StepVectorLen ) ;
	    StepVector . y /= ( DirectionSign * 2 * StepVectorLen ) ;
	}
	else
	{
	    StepVector . x = (float) MyRandom ( 100 ) / 200.0 ;
	    StepVector . y = (float) MyRandom ( 100 ) / 200.0 ;
	}
	
	//--------------------
	// Here, when eventually moving out of a colliding colleague,
	// we must not check for feasibility but only for wall collisions,
	// cause otherwise the move out of the colleague will never
	// be allowed.
	//
	if ( IsPassable ( ThisRobot -> pos.x + StepVector.x , 
			  ThisRobot -> pos.y + StepVector.y ,
			  ThisRobot -> pos.z ) )
	{
	    ThisRobot -> PrivatePathway [ 0 ] . x = ThisRobot -> pos.x + StepVector.x;
	    ThisRobot -> PrivatePathway [ 0 ] . y = ThisRobot -> pos.y + StepVector.y;
	}
	
    }
#endif
}; // void MoveInCloserForOrAwayFromMeleeCombat ( Enemy ThisRobot , int enemynum )

/* ----------------------------------------------------------------------
 * At some points it may be nescessary, that an enemy turns around to
 * face the Tux.  This function does that, but only so much as can be
 * done in the current frame, i.e. NON-BLOCKING operation.
 *
 * The return value indicates, if the turning has already reached it's
 * target by now or not.
 * ---------------------------------------------------------------------- */
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

/* ----------------------------------------------------------------------
 * Enemies act as groups.  If one is hit, all will attack and the like.
 * This transmission of information is handled here.
 * ---------------------------------------------------------------------- */
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

/* ----------------------------------------------------------------------
 * Enemies act as groups.  If one is hit, all will attack and the like.
 * Similarly, if you attack one peaceful guard, all other guards will be
 * pissed as well...
 * ---------------------------------------------------------------------- */
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


/* ----------------------------------------------------------------------
 * This function checks for enemy collsions and returns TRUE if enemy 
 * with number enemynum collided with another enemy from the list.
 * ---------------------------------------------------------------------- */
int
CheckEnemyEnemyCollision ( enemy * OurBot )
{
    float check_x, check_y;
    int swap;
    float xdist, ydist;
    float dist2;
    float speed_x, speed_y;
    
    check_x = OurBot -> pos . x ;
    check_y = OurBot -> pos . y ;
    
    //--------------------
    // Now we check through all the other enemys on this level if 
    // there is perhaps a collision with them...

    /*XXX put back on !*/    
    enemy *erot;
    BROWSE_LEVEL_BOTS(erot, OurBot->pos.z)
    {
	if (erot == OurBot)
	    continue;
	
	// get distance between enemy i and enemynum 
	xdist = check_x - erot->pos.x;
	ydist = check_y - erot->pos.y;
	
	dist2 = sqrt(xdist * xdist + ydist * ydist);
	
	// Is there a Collision?
	if ( dist2 <= 2*DRUIDRADIUSXY )
	{
	    // am I waiting already?  If so, keep waiting... 
	    if ( OurBot->pure_wait)
	    {
		// keep waiting
		OurBot->pure_wait = WAIT_COLLISION;
		continue;
	    }
	    
	    // otherwise: stop this one enemy and go back youself
	    
	    erot->pure_wait = WAIT_COLLISION;
	    
	    swap = OurBot->nextwaypoint;
	    OurBot->nextwaypoint = OurBot->lastwaypoint;
	    OurBot->lastwaypoint = swap;
	    
	    /*
	    if ( erot -> combat_state == MOVE_ALONG_RANDOM_WAYPOINTS )
		erot -> combat_state = SELECT_NEW_WAYPOINT;
	    
	    // push the stopped colleague a little bit backwards...
	    if (xdist)
		erot->pos.x -= xdist / fabsf (xdist) * Frame_Time();
	    if (ydist)
		erot->pos.y -= ydist / fabsf (ydist) * Frame_Time();
	    
	    // Move a little bit out of the colleague yourself...
	    speed_x = OurBot->speed.x;
	    speed_y = OurBot->speed.y;
	    
	    if (speed_x) OurBot->pos.x -= Frame_Time() * COL_SPEED * (speed_x) / fabsf (speed_x);
	    if (speed_y) OurBot->pos.y -= Frame_Time() * COL_SPEED * (speed_y) / fabsf (speed_y);
	    */
	    //DebugPrintf ( -1 , "\n%s(): enemy-enemy collision detected.  moving things..." , __FUNCTION__ );

	    return TRUE;
	} // if collision distance reached

    } // for all the bots...
    
    return FALSE;
}; // int CheckEnemyEnemyCollision

/* ----------------------------------------------------------------------
 *
 *
 * ---------------------------------------------------------------------- */
void
start_gethit_animation_if_applicable ( enemy* ThisRobot ) 
{
    //--------------------
    // Maybe this robot is already fully animated.  In this case, after getting
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

/* ----------------------------------------------------------------------
 * This function increases the phase counters for animation of a bot.
 * ---------------------------------------------------------------------- */
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
#undef _enemy_c
