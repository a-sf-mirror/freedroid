/* 
 *
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
 * Desc: the main program
 * ---------------------------------------------------------------------- */

#define _main_c

#include "system.h"

#include "defs.h"
#include "struct.h"
#include "global.h"
#include "proto.h"
#include "vars.h"

#ifdef __OpenBSD__
#include "ieeefp.h"
#else
#include "fenv.h"
#endif

int ThisMessageTime;
float LastGotIntoBlastSound = 2;
float LastRefreshSound = 2;

void UpdateCountersForThisFrame () ;

/* ----------------------------------------------------------------------
 *
 *
 * ---------------------------------------------------------------------- */
void
DoAllMovementAndAnimations ( void )
{
    MoveLevelDoors ( ) ;
    
    WorkLevelGuns( ); // this should fire all autocannons on this level
    
    CheckForTriggeredEventsAndStatements ( ) ;
    
    AnimateCyclingMapTiles (); // this is a pure client issue.  Not dependent upon the players.
    
    animate_blasts ();	// move blasts to the right current "phase" of the blast
    
    MoveActiveSpells (); // move moving spells currently active...
    
    MoveBullets ();
  
}; // void DoAllMovementAndAnimations ( void )

/* -----------------------------------------------------------------
 * This function is the heart of the game.  It contains the main
 * game loop.
 * ----------------------------------------------------------------- */
int
main (int argc, char * argv[])
{
#if ENABLE_NLS
	DIR *tmp_dir;
	setlocale(LC_MESSAGES, "");
	setlocale(LC_CTYPE, "");
	tmp_dir = opendir("../po");
	if(tmp_dir != NULL) 
	{
		bindtextdomain(LOCALE_PACKAGE, "../po");
		closedir(tmp_dir);
	}
	else 
		bindtextdomain(LOCALE_PACKAGE, LOCALE_DIR);
	textdomain(LOCALE_PACKAGE);
#endif

    //--------------------
    // First we issue some message, that should appear in the debug log for
    // windows users.
    //
#ifdef __WIN32__
    fprintf ( stderr , "\n\
Hello!  This window contains the DEBUG OUTPUT of FreedroidRPG.\n\
\n\
Normally you would not see this message or this window, but apparently\n\
FreedroidRPG has terminated because of an error of some sort.\n\
\n\
You might wish to inspect the debug output below.  Maybe sending the\n\
debug output (or at least the lower end of the debug output) to the\n\
FreedroidRPG developers could help them to track down the problem.\n\
\n\
Well, it's no guarantee that we can solve any bug, but it's certainly\n\
better than nothing.  Thanks anyway for your interest in FreedroidRPG.\n\
\n\n\n\
--start of real debug log--\n\n" );
#endif
    
    GameOver = FALSE;
    QuitProgram = FALSE;
    xray_vision_for_tux = FALSE ;
    draw_collision_rectangles = FALSE ;
    draw_grid = TRUE ;

    /*
     *  Parse command line and set global switches 
     *  this function exits program when error, so we don't need to 
     *  check its success  (dunno if that's good design?)
     */
    sound_on = TRUE;	 // default value, can be overridden by command-line 
    use_open_gl = TRUE;	 // default value, can be overridden by command-line 
    debug_level = -1;      // -1: shut up all debug ... 0=no debug 1=first debug level (at the moment=all) 
    GameConfig . fullscreen_on = TRUE;  // use X11-window or full screen 
    joy_sensitivity = 1;
    mouse_control = TRUE;
    classic_user_rect = FALSE;
    
    
    InitFreedroid ( argc, argv);   // Initialisation of global variables and arrays
    

    while (!QuitProgram)
    {
	GameOver = TRUE;
	StartupMenu ( );
	GameOver = FALSE;
	while ( (!GameOver && !QuitProgram))
	{
	    CurLevel = curShip.AllLevels [ Me . pos . z ];
	    
	    StartTakingTimeForFPSCalculation(); 
	    
	    track_last_frame_input_status();
	    keyboard_update();
	    ReactToSpecialKeys();
	    
	    UpdateCountersForThisFrame ( ) ;
	    
	    CollectAutomapData ();
	    
	    DoAllMovementAndAnimations();
	    
	    AssembleCombatPicture ( SHOW_ITEMS | USE_OWN_MOUSE_CURSOR ); 
	    our_SDL_flip_wrapper ( Screen );
	    
	    move_tux ( );	
	
	    HandleInventoryScreen ();
	    HandleCharacterScreen ( );

	    UpdateAllCharacterStats ( 0 );
	    
	    MoveEnemys ();	// move all the enemys:
	    
	    check_tux_enemy_collision ();
	    
	    correct_tux_position_according_to_jump_thresholds ( 0 );
	    
	    CheckIfMissionIsComplete (); 
	    
	    if ( ! GameConfig.hog_CPU ) 
		SDL_Delay (1); // we allow the CPU to also do something else..
	    
	    ComputeFPSForThisFrame();
	    
	} // while !GameOver 
    } // while !QuitProgram 
    Terminate (0);
    return (0);
}; // int main ( void )

/* ----------------------------------------------------------------------
 * Some bots might be frozen and some might be poisoned, some might still 
 * have a 'firewait' or a normal wait or a paralysation.  Other bots have
 * a text, that is being displayed and that will timeout at some point.
 *
 * In any case those counteres must be updated, which is what this 
 * function is supposed to do.
 *
 * NOTE:  This whole updating business is a bit in-efficient.  It might 
 *        be better to use some sort of 'game_time' for this and then
 *        not use 'duration left' but rather 'end time' for all these 
 *        poison, paralysation, etc. effects.  That way, we would be able
 *        be skip this whole counter advancement here...
 *
 *        Maybe later it will finally be implemented this way...
 *
 * ---------------------------------------------------------------------- */
void
update_timeouts_for_bots_on_level ( int level_num , float latest_frame_time ) 
{
    enemy* this_bot = alive_bots_head;

    for ( ; this_bot; this_bot = GETNEXT(this_bot))
    {
	if ( this_bot -> pos . z != level_num )
	    continue;

	if ( this_bot -> pure_wait > 0 ) 
	{
	    this_bot -> pure_wait -= latest_frame_time ;
	    if ( this_bot -> pure_wait < 0 ) this_bot -> pure_wait = 0;
	}
	
	if ( this_bot -> frozen > 0 ) 
	{
	    this_bot -> frozen -= latest_frame_time ;
	    if ( this_bot -> frozen < 0 ) this_bot -> frozen = 0;
	}
	
	if ( this_bot -> poison_duration_left > 0 ) 
	{
	    this_bot -> poison_duration_left -= latest_frame_time ;
	    if ( this_bot -> poison_duration_left < 0 ) this_bot -> poison_duration_left = 0 ;
	    this_bot -> energy -= latest_frame_time * this_bot -> poison_damage_per_sec ;
	    if ( this_bot -> energy < 1 )
	    {
		this_bot -> energy = 1;
		this_bot -> poison_duration_left = 0;
	    }
	}
	
	if ( this_bot -> paralysation_duration_left > 0 ) 
	{
	    this_bot -> paralysation_duration_left -= latest_frame_time ;
	    if ( this_bot -> paralysation_duration_left < 0 ) this_bot -> paralysation_duration_left = 0 ;
	    // this_bot -> energy -= latest_frame_time * this_bot -> paralysation_damage_per_sec ;
	}
	
	if ( this_bot -> firewait > 0 ) 
	{
	    this_bot -> firewait -= latest_frame_time ;
	    if ( this_bot -> firewait <= 0 ) this_bot -> firewait = 0 ;
	}
	
	this_bot -> TextVisibleTime += latest_frame_time;
    } 
    
}; // void update_timeouts_for_bots_on_level ( int level_num ) 

/* -----------------------------------------------------------------
 * This function updates counters and is called ONCE every frame.
 * The counters include timers, but framerate-independence of game speed
 * is preserved because everything is weighted with the Frame_Time()
 * function.
 * ----------------------------------------------------------------- */
void
UpdateCountersForThisFrame ( )
{
    static long Overall_Frames_Displayed=0;
    int i;
    Level item_level = curShip . AllLevels [ Me . pos . z ] ;
    float my_speed ;
    float latest_frame_time = Frame_Time();
    int level_num;

	GameConfig . Mission_Log_Visible_Time += latest_frame_time;
	
	// The next couter counts the frames displayed by freedroid during this
	// whole run!!  DO NOT RESET THIS COUNTER WHEN THE GAME RESTARTS!!
	Overall_Frames_Displayed++;
	Overall_Average = ( Overall_Average * ( Overall_Frames_Displayed - 1 )
			   + latest_frame_time ) / Overall_Frames_Displayed ;
	
	// Here are some things, that were previously done by some periodic */
	// interrupt function
	ThisMessageTime++;
	
	LastGotIntoBlastSound += latest_frame_time ;
	LastRefreshSound += latest_frame_time ;
	
	LevelDoorsNotMovedTime += latest_frame_time;
	LevelGunsNotFiredTime += latest_frame_time;
	if ( SkipAFewFrames ) SkipAFewFrames--;

	//--------------------
	// This is the timeout, that the tux should not start a movement
	// some fraction of a second after an item drop.
	//
	if ( timeout_from_item_drop > 0 )
	{
	    timeout_from_item_drop -= latest_frame_time ;
	    if ( timeout_from_item_drop < 0 ) timeout_from_item_drop = 0 ; 
	}

	
	for ( i = 0 ; i < MAXBULLETS ; i ++ )
	{
	    if ( AllBullets [ i ] . time_to_hide_still > 0 )
	    {
		AllBullets [ i ] . time_to_hide_still -= latest_frame_time;
		if ( AllBullets [ i ] . time_to_hide_still < 0 )
		    AllBullets [ i ] . time_to_hide_still = 0 ; 
	    }
	}
	
	//--------------------
	// Maybe some items are just thrown in the air and still in the air.
	// We need to keep track of the time the item has spent in the air so far.
	//
	for ( i = 0 ; i < MAX_ITEMS_PER_LEVEL ; i ++ )
	{
	    if ( item_level -> ItemList [ i ] . type == ( -1 ) ) continue;
	    if ( item_level -> ItemList [ i ] . throw_time > 0 ) 
		item_level -> ItemList [ i ] . throw_time += latest_frame_time;
	    if ( item_level -> ItemList [ i ] . throw_time > ( M_PI / 3.0 ) ) 
		item_level -> ItemList [ i ] . throw_time = 0 ;
	}

	//--------------------
	// Some bots might be frozen and some might be poisoned, some
	// might still have a 'firewait' or a normal wait or a paralysation.
	// In any case those counteres must be updated, but we'll only to 
	// that for the Tux current level (at present).
	//
	for ( level_num = 0 ; level_num < MAX_LEVELS ; level_num ++ )
	{
	    if ( level_is_partly_visible ( level_num ) )
		update_timeouts_for_bots_on_level ( level_num , latest_frame_time ) ;
	}
    
    //--------------------
    // Now we do all the things, that need to be updated for each connected
    // player separatedly.
    //
    Me . current_game_date += latest_frame_time ;

    Me . FramesOnThisLevel++;
    Me . LastCrysoundTime += latest_frame_time ;
    Me . MissionTimeElapsed += latest_frame_time;
    Me . LastTransferSoundTime += latest_frame_time;
    Me . TextVisibleTime += latest_frame_time;
    
    //--------------------
    // We take care of the running stamina...
    //
    my_speed = sqrt ( Me . speed . x * Me . speed . x +
		      Me . speed . y * Me . speed . y ) ;
    if ( my_speed >= ( TUX_WALKING_SPEED + TUX_RUNNING_SPEED ) * 0.5 )
    {
	Me . running_power -= latest_frame_time * 3.0 ;
    }
    else
    {
	Me . running_power += latest_frame_time * 3.0 ;
	if ( Me . running_power > Me . max_running_power )
	    Me . running_power = Me . max_running_power ;

	if ( Me . running_power >= 20 )
	    Me . running_must_rest = FALSE ;
    }
    

    //-------------------
    // Mana and health recover over time
    Me . energy += Me . health_recovery_rate * latest_frame_time;
    if ( Me . energy > Me . maxenergy )
	    Me . energy = Me . maxenergy;

    Me . temperature -= Me . cooling_rate * latest_frame_time;
    if ( Me . temperature < 0 )
	    Me . temperature = 0;
    
    if ( Me . temperature > Me . max_temperature ) //overheat, lose life
	    {
	    if ( Me . old_temperature < Me . max_temperature ) 
		append_new_game_message(_("Overheating!"));
	    Me . energy -= ( Me . temperature - Me.max_temperature ) * latest_frame_time / 10;
	    }
    
    Me . old_temperature = Me . temperature;

    if ( Me . weapon_swing_time != (-1) ) Me . weapon_swing_time += latest_frame_time;
    if ( Me . got_hit_time != (-1) ) Me . got_hit_time += latest_frame_time;
    
    if ( Me . busy_time > 0 )
    {
	Me . busy_time -= latest_frame_time ;
	if ( Me . busy_time < 0 ) Me . busy_time = 0 ;
    }
    if ( Me . busy_time == 0)
	{
	if ( Me . busy_type == WEAPON_RELOAD )
		append_new_game_message(_("Weapon reloaded"));
	Me . busy_type = NONE;
	}

    if ( Me . paralyze_duration )
	Me . paralyze_duration -= latest_frame_time;  
    if ( Me . slowdown_duration )
	Me . slowdown_duration -= latest_frame_time;    
    if ( Me . paralyze_duration < 0)  Me . paralyze_duration = 0;
    if ( Me . slowdown_duration < 0)  Me . slowdown_duration = 0;

    //--------------------
    // In order to know when a level can finally be respawned with
    // enemies, we keep track to the time spent actually in the game, i.e.
    // time actually spent passing frames...
    //
    for ( i = 0 ; i < MAX_LEVELS ; i ++ )
    {
	if ( Me . pos . z != i )
	{
	    if ( Me . time_since_last_visit_or_respawn [ i ] > (-1) )
	    {
		Me . time_since_last_visit_or_respawn [ i ] += latest_frame_time ;
	    }
	    
	    //--------------------
	    // Now maybe it's time to respawn?  If we really have multiple
	    // players of course, this check would have to look a bit different...
	    //
	    if ( Me . time_since_last_visit_or_respawn [ i ] > 600 )
	    {
		DebugPrintf ( -10 , "\nNow respawning all bots on level : %d. " , i ) ;
		Me . time_since_last_visit_or_respawn [ i ] = 0 ;
		respawn_level ( i ) ;
	    }
	  
	}
	else
	{
	    //--------------------
	    // When the Tux is right on this level, there is absolutely no need 
	    // for respawning anything...
	    //
	    Me . time_since_last_visit_or_respawn [ i ] = 0 ;
	}
    }

    //--------------------
    // On some maps, the Tux will have no enemies.  Therefore it would
    // make sense that (for better gameplay) the running bar does not
    // run out.
    //
    if ( curShip . AllLevels [ Me . pos . z ] -> infinite_running_on_this_level )
    {
	Me . running_power = Me . max_running_power ;
    }

}; // void UpdateCountersForThisFrame(...) 

#undef _main_c
