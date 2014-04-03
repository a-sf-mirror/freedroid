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
/**
 * Desc: the main program
 */

#define _main_c

#include "system.h"

#include "defs.h"
#include "struct.h"
#include "global.h"
#include "proto.h"
#include "vars.h"
#include "widgets/widgets.h"

#ifdef __OpenBSD__
#include "ieeefp.h"
#else
#include "fenv.h"
#endif

float LastGotIntoBlastSound = 2;
float LastRefreshSound = 2;

void UpdateCountersForThisFrame();

/**
 *
 *
 */
void DoAllMovementAndAnimations(void)
{
	animate_scenery();	// this is a pure client issue.  Not dependent upon the players.

	animate_blasts();	// move blasts to the right current "phase" of the blast

	MoveActiveSpells();	// move moving spells currently active...

	MoveBullets();

	DoMeleeDamage();

};				// void DoAllMovementAndAnimations ( void )

/**
 * Run the game until it is over.
 */
void Game()
{
	GameOver = FALSE;
	reset_visible_levels();
	get_visible_levels();
	animation_timeline_reset();

	// Reset tooltips.
	widget_set_tooltip(NULL, NULL);
	
	while ((!GameOver && !QuitProgram)) {
		game_status = INSIDE_GAME;

		StartTakingTimeForFPSCalculation();

		save_mouse_state();
		input_handle();
		update_widgets();
		if (!world_frozen()) {
			HandleInventoryScreen();
			HandleCharacterScreen();
		}

		CollectAutomapData();
		UpdateAllCharacterStats();

		AssembleCombatPicture(SHOW_ITEMS);
		our_SDL_flip_wrapper();

		if (!world_frozen()) {
			UpdateCountersForThisFrame();

			DoAllMovementAndAnimations();
			move_tux();
			get_visible_levels();

			move_enemies();
			check_tux_enemy_collision();
			correct_tux_position_according_to_jump();

		}

		CheckIfMissionIsComplete();
		ComputeFPSForThisFrame();
	}			// while !GameOver 
}

/* -----------------------------------------------------------------
 * This function is the heart of the game.  It contains the main
 * game loop.
 * ----------------------------------------------------------------- */
int main(int argc, char *argv[])
{

#if defined HAVE_UNISTD_H && defined HAVE_DIRNAME
	// change working directory to the executable's directory
	if (chdir(dirname(argv[0])))
		fprintf(stderr, "Couldn't change working directory to %s.\n", dirname(argv[0]));
#endif

	init_data_dirs_path();
	lang_init();

	QuitProgram = FALSE;
	draw_collision_rectangles = FALSE;
	gps_transform_map_dirty_flag = TRUE;

	/*
	 *  Parse command line and set global switches 
	 *  this function exits program when error, so we don't need to 
	 *  check its success  (dunno if that's good design?)
	 */
	sound_on = TRUE;	// default value, can be overridden by command-line 
	use_open_gl = TRUE;	// default value, can be overridden by command-line 
	start_editor = FALSE;	// default value, can be overridden by command-line 
	load_saved = FALSE;	// default value, can be overridden by command-line
	saved_game_name = NULL;
	debug_level = -1;	// -1: shut up all debug ... 0=no debug 1=first debug level (at the moment=all) 

	InitFreedroid(argc, argv);	// Initialisation of global variables and arrays

	if (do_benchmark) {
		/* Benchmark mode? Do not run the regular game from here. */
		int failed = benchmark();
		Terminate(failed ? EXIT_FAILURE : EXIT_SUCCESS, FALSE);
	}

	int skip_menu = FALSE;
	if (start_editor) {
		// When the user wants to start editor from the command line, skip the
		// startup menu and load the level editor directly
		skip_menu = TRUE;
		prepare_level_editor();
		start_editor = FALSE;
	} else if (load_saved && saved_game_name != NULL) {
		// Skip the startup menu and load saved game directly
		if (load_named_game(saved_game_name) == OK) {
			skip_menu = TRUE;
			game_root_mode = ROOT_IS_GAME;
		}
		load_saved = FALSE;
	}

	while (!QuitProgram) {
		if (!skip_menu) {
			StartupMenu();
			input_handle();
		} else {
			skip_menu = FALSE;
		}
		switch (game_root_mode) {
		case ROOT_IS_GAME:
			Game();
			break;
		case ROOT_IS_LVLEDIT:
			LevelEditor();
			break;
		case ROOT_IS_UNKNOWN:
			ErrorMessage(__FUNCTION__, "Game root mode is unknown - was not properly set to GAME or LVLEDIT.\n", PLEASE_INFORM,
				     IS_FATAL);
			break;
		}
	}			// while !QuitProgram 

	LightRadiusClean();

	Terminate(EXIT_SUCCESS, TRUE);
	return (0);
};				// int main ( void )

/**
 * Some bots might be frozen and some might be poisoned, some might still 
 * have a 'firewait' or a normal wait or a paralysis.  Other bots have
 * a text, that is being displayed and that will timeout at some point.
 *
 * In any case those counteres must be updated, which is what this 
 * function is supposed to do.
 *
 * NOTE:  This whole updating business is a bit in-efficient.  It might 
 *        be better to use some sort of 'game_time' for this and then
 *        not use 'duration left' but rather 'end time' for all these 
 *        poison, paralysis, etc. effects.  That way, we would be able
 *        be skip this whole counter advancement here...
 *
 *        Maybe later it will finally be implemented this way...
 *
 */
void update_timeouts_for_bots_on_level(int level_num, float latest_frame_time)
{

	enemy *this_bot, *nerot;
	BROWSE_LEVEL_BOTS_SAFE(this_bot, nerot, level_num) {
		if (this_bot->pure_wait > 0) {
			this_bot->pure_wait -= latest_frame_time;
			if (this_bot->pure_wait < 0)
				this_bot->pure_wait = 0;
		}

		if (this_bot->frozen > 0) {
			this_bot->frozen -= latest_frame_time;
			if (this_bot->frozen < 0)
				this_bot->frozen = 0;
		}

		if (this_bot->poison_duration_left > 0) {
			this_bot->poison_duration_left -= latest_frame_time;
			if (this_bot->poison_duration_left < 0)
				this_bot->poison_duration_left = 0;
			this_bot->energy -= latest_frame_time * this_bot->poison_damage_per_sec;
			if (this_bot->energy < 1) {
				this_bot->energy = 1;
				this_bot->poison_duration_left = 0;
			}
		}

		if (this_bot->paralysation_duration_left > 0) {
			this_bot->paralysation_duration_left -= latest_frame_time;
			if (this_bot->paralysation_duration_left < 0)
				this_bot->paralysation_duration_left = 0;
		}

		if (this_bot->firewait > 0) {
			this_bot->firewait -= latest_frame_time;
			if (this_bot->firewait <= 0)
				this_bot->firewait = 0;
		}

		this_bot->TextVisibleTime += latest_frame_time;
	}

};				// void update_timeouts_for_bots_on_level ( int level_num ) 

/* -----------------------------------------------------------------
 * This function updates counters and is called ONCE every frame.
 * The counters include timers, but framerate-independence of game speed
 * is preserved because everything is weighted with the Frame_Time()
 * function.
 * ----------------------------------------------------------------- */
void UpdateCountersForThisFrame()
{
	int i;
	float my_speed;
	float latest_frame_time = Frame_Time();
	int level_num;
	float lose_life;

	update_frames_displayed();

	LastGotIntoBlastSound += latest_frame_time;
	LastRefreshSound += latest_frame_time;

	// This is the timeout, that the tux should not start a movement
	// some fraction of a second after an item drop.
	//
	if (timeout_from_item_drop > 0) {
		timeout_from_item_drop -= latest_frame_time;
		if (timeout_from_item_drop < 0)
			timeout_from_item_drop = 0;
	}

	for (i = 0; i < MAXBULLETS; i++) {
		if (AllBullets[i].time_to_hide_still > 0) {
			AllBullets[i].time_to_hide_still -= latest_frame_time;
			if (AllBullets[i].time_to_hide_still < 0)
				AllBullets[i].time_to_hide_still = 0;
		}
	}

	// Maybe some items are just thrown in the air and still in the air.
	// We need to keep track of the time the item has spent in the air so far.
	//
	struct visible_level *vis_lvl, *n;
	
	BROWSE_VISIBLE_LEVELS(vis_lvl, n) {
		level *lvl = vis_lvl->lvl_pointer;
		for (i = 0; i < MAX_ITEMS_PER_LEVEL; i++) {
			if (lvl->ItemList[i].type == (-1))
				continue;
			if (lvl->ItemList[i].throw_time > 0)
				lvl->ItemList[i].throw_time += latest_frame_time;
			if (lvl->ItemList[i].throw_time > (M_PI / 3.0))
				lvl->ItemList[i].throw_time = 0;
		}
	}
	
	// We stop here to prevent animation glitches when Tux is dead
	if (GameOver)
		return;	

	// Some bots might be frozen and some might be poisoned, some
	// might still have a 'firewait' or a normal wait or a paralysis.
	// In any case those counters must be updated, but we'll only to 
	// that for the Tux current level (at present).
	//
	for (level_num = 0; level_num < curShip.num_levels; level_num++) {
		if (level_exists(level_num) && level_is_visible(level_num))
			update_timeouts_for_bots_on_level(level_num, latest_frame_time);
	}

	// Now we do all the things, that need to be updated for each connected
	// player separately.
	//
	Me.current_game_date += latest_frame_time;

	Me.LastCrysoundTime += latest_frame_time;
	Me.MissionTimeElapsed += latest_frame_time;
	Me.TextVisibleTime += latest_frame_time;

	// We take care of the running stamina...
	//
	my_speed = sqrt(Me.speed.x * Me.speed.x + Me.speed.y * Me.speed.y);
	if ((my_speed >= (TUX_WALKING_SPEED + TUX_RUNNING_SPEED) * 0.5) && (!GameConfig.cheat_running_stamina)) {
		Me.running_power -= latest_frame_time * 3.0;
	} else {
		Me.running_power += latest_frame_time * 3.0;
		if (Me.running_power > Me.max_running_power)
			Me.running_power = Me.max_running_power;

		if (Me.running_power >= 20)
			Me.running_must_rest = FALSE;
	}

	// Mana and health recover over time
	Me.energy += Me.health_recovery_rate * latest_frame_time;
	if (Me.energy > Me.maxenergy)
		Me.energy = Me.maxenergy;

	Me.temperature -= Me.cooling_rate * latest_frame_time;
	if (Me.temperature < 0)
		Me.temperature = 0;

	lose_life = (Me.temperature - Me.max_temperature) * latest_frame_time / 10;

	if (Me.temperature > Me.max_temperature)	//overheat, lose life
	{
		if (Me.old_temperature < Me.max_temperature)
			append_new_game_message(_("Overheating!"));
		if (Me.energy - lose_life < 1 && Me.paralyze_duration == 0) {
			DoSkill(get_program_index_with_name("Emergency shutdown"),
				calculate_program_heat_cost(get_program_index_with_name("Emergency shutdown")));
			// TRANSLATORS: Screen message, displayed below 'Automatically shutting down'
			SetNewBigScreenMessage(_("due to near lethal overheating"));
			SetNewBigScreenMessage(_("Automatically shutting down"));
			lose_life = (Me.temperature - Me.max_temperature) * latest_frame_time / 10;
		}
		if (lose_life > 0) {
			hit_tux(lose_life);
		}
	}

	Me.old_temperature = Me.temperature;

	if (Me.weapon_swing_time != (-1))
		Me.weapon_swing_time += latest_frame_time;
	if (Me.got_hit_time != (-1))
		Me.got_hit_time += latest_frame_time;

	if (Me.busy_time > 0) {
		Me.busy_time -= latest_frame_time;
		if (Me.busy_time < 0)
			Me.busy_time = 0;
	}
	if (Me.busy_time == 0) {
		if (Me.busy_type == WEAPON_RELOAD)
			append_new_game_message(_("Weapon reloaded"));
		Me.busy_type = NONE;
	}

	if (Me.paralyze_duration > 0)
		Me.paralyze_duration -= latest_frame_time;
	if (Me.slowdown_duration > 0)
		Me.slowdown_duration -= latest_frame_time;
	if (Me.invisible_duration > 0)
		Me.invisible_duration -= latest_frame_time;
	if (Me.nmap_duration > 0)
		Me.nmap_duration -= latest_frame_time;
	if (Me.paralyze_duration < 0)
		Me.paralyze_duration = 0;
	if (Me.slowdown_duration < 0)
		Me.slowdown_duration = 0;
	if (Me.invisible_duration < 0)
		Me.invisible_duration = 0;
	if (Me.nmap_duration < 0)
		Me.nmap_duration = 0;

	// In order to know when a level can finally be respawned with
	// enemies, we keep track to the time spent actually in the game, i.e.
	// time actually spent passing frames...
	//
	for (i = 0; i < curShip.num_levels; i++) {
		if (!level_exists(i))
			continue;
		if (Me.pos.z != i) {
			if (Me.time_since_last_visit_or_respawn[i] > (-1)) {
				Me.time_since_last_visit_or_respawn[i] += latest_frame_time;
			}
			// Now maybe it's time to respawn?  If we really have multiple
			// players of course, this check would have to look a bit different...
			//
			if (Me.time_since_last_visit_or_respawn[i] > 600 && !(curShip.AllLevels[i]->flags & NO_RESPAWN)) {

				DebugPrintf(-10, "\nNow respawning all bots on level : %d. ", i);
				Me.time_since_last_visit_or_respawn[i] = 0;

				respawn_level(i);
			}

		} else {
			// When the Tux is right on this level, there is absolutely no need 
			// for respawning anything...
			//
			Me.time_since_last_visit_or_respawn[i] = 0;
		}
	}

	// On some maps, the Tux will have no enemies.  Therefore it would
	// make sense that (for better gameplay) the running bar does not
	// run out.
	//
	if (curShip.AllLevels[Me.pos.z]->infinite_running_on_this_level) {
		Me.running_power = Me.max_running_power;
	}

};				// void UpdateCountersForThisFrame(...) 

#undef _main_c
