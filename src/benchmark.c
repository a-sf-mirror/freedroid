/* 
 *
 *   Copyright (c) 2009-2010 Arthur Huillet 
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

#define _benchmark_c

#include "system.h"

#include "defs.h"
#include "struct.h"
#include "global.h"
#include "proto.h"
#include "lvledit/lvledit_validator.h"

static int start_stamp;
static int stop_stamp;

static void timer_start()
{
	start_stamp = SDL_GetTicks();
}

static void timer_stop()
{
	stop_stamp = SDL_GetTicks();
}

/* Text rendering performance measurement */
static int text_bench()
{
    char *str = "abcdefghijklmnopqrstuvwxyz0123456789abcdefghijklmnopqrstuvwxyz";

	// Make sure all glyphs are loaded
	put_string(GetCurrentFont(), 0, 0, str);

	// Display the string many times	
	int nb = 10000;

	timer_start();
	while (nb--) {
		put_string(GetCurrentFont(), 0, 0, str);
	}
		
	our_SDL_flip_wrapper();
	timer_stop();

	return 0;
}

/* Dialog validator (not an actual benchmark) */
static int dialog_test()
{
	int failed;

	timer_start();
	failed = validate_dialogs();
	timer_stop();

	return failed;
}

/* LoadShip (level loading) performance test */
static int loadship_bench()
{
	int loop = 10;

	// Find a ship file to load
	char fp[PATH_MAX];
	find_file("levels.dat", MAP_DIR, fp);

	// Load it many times
	timer_start();
	while (loop--) {
		LoadShip(fp, 0);
	}
	timer_stop();

	return 0;
}

/* LoadGame (savegame loading) performance test */
static int loadgame_bench()
{
	int loop = 3;

	// Use MapEd.savegame
	free(Me.character_name);
	Me.character_name = strdup("MapEd");

	// Load it many times
	timer_start();
	while (loop--) {
		if (LoadGame() == ERR) {
			timer_stop();
			return ERR;
		}
	}
	timer_stop();

	return 0;
}

/* SaveGame (savegame writing) performance test */
static int savegame_bench()
{
	int loop = 10;

	// Use MapEd.savegame
	free(Me.character_name);
	Me.character_name = strdup("MapEd");

	// Load it
	if (LoadGame() == ERR) {
		error_message(__FUNCTION__, "Whoops, that failed. Maybe you have to save a game under the name \"MapEd\" to make this work?",
				NO_REPORT);
		return -1;
	}
	
	// Write it many times.
	timer_start();
	while (loop--) {
		SaveGame();
	}
	timer_stop();

	return 0;
}

/* Test of dynamic arrays */
static int dynarray_test()
{
	int loop = 5;
	item dummy;
	memset(&dummy, 0, sizeof(item));
	dummy.pos.x = 1.0;
	dummy.type = 52;

	timer_start();
	while (loop--) {
		int i;
		struct dynarray *dyn = dynarray_alloc(100, sizeof(item));
		for (i = 0; i < 1000000; i++) {
			dynarray_add(dyn, &dummy, sizeof(item));
		}
		for (i = 0; i < 1000000; i++) {
			item *it = dyn->arr;
			if (memcmp(&dummy, it, sizeof(item))) {
				fprintf(stderr, "Error reading out item %d\n", i);
			}
		}
		dynarray_free(dyn);
		free(dyn);
		dyn = NULL;
	}

	timer_stop();

	return 0;
}

static int mapgen_bench()
{
	int loop = 100;
	extern void CreateNewMapLevel(int);
	extern int delete_map_level(int);
	timer_start();
	while (loop--) {
		CreateNewMapLevel(0);
		level *l = curShip.AllLevels[0];
		l->xlen = 90;
		l->ylen = 90;
		l->random_dungeon = 2;
		l->teleport_pair = 0;
		set_dungeon_output(l);
		generate_dungeon(l->xlen, l->ylen, l->random_dungeon, l->teleport_pair);
		delete_map_level(0);
	}
	timer_stop();

	return 0;
}

/* Levels validator (not an actual benchmark) */
static int level_test()
{
	int failed;

	// Load default ship
	char fp[PATH_MAX];
	find_file("levels.dat", MAP_DIR, fp);
	LoadShip(fp, 0);

	timer_start();
	failed = level_validation_on_console_only();
	timer_stop();

	return failed;
}

int benchmark()
{
	struct {
		char *name;
		int (*func)();
	} benchs[] = {
			{ "text", text_bench },
			{ "dialog", dialog_test },
			{ "loadship", loadship_bench },
			{ "loadgame", loadgame_bench },
			{ "savegame", savegame_bench },
			{ "dynarray", dynarray_test },
			{ "mapgen", mapgen_bench },
			{ "leveltest", level_test },
	};

	int i;
	char str[1024];

	clear_screen();
	sprintf(str, "Testing \"%s\"...", do_benchmark);
	put_string(GetCurrentFont(), 10, 100, str);
	our_SDL_flip_wrapper();


	for (i = 0; i < sizeof(benchs)/sizeof(benchs[0]); i++) {
		if (!strcmp(do_benchmark, benchs[i].name)) {
			if (benchs[i].func() == 0) {
				printf("Running test %s took %d milliseconds.\n", do_benchmark, stop_stamp - start_stamp);
				return OK;
			} else {
				if (term_has_color_cap)
					printf("\033[31mTest failed!\033[0m\n");
				else
					printf("Test failed!\n");
				return ERR;
			}
		}
	}
	
	fprintf(stderr, "Unrecognized test %s. Existing tests are:\n", do_benchmark);
	for (i = 0; i < sizeof(benchs)/sizeof(benchs[0]); i++) {
		fprintf(stderr, "\t%s\n", benchs[i].name);
	}
	exit(EXIT_FAILURE);
}


#undef _benchmark_c
