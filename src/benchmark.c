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
	PutString(Screen, 0, 0, str);

	// Display the string many times	
	int nb = 10000;

	timer_start();
	while (nb--) {
		PutString(Screen, 0, 0, str);
	}
		
	our_SDL_flip_wrapper();
	timer_stop();

	return 0;
}

/* Dialog validator (not an actual benchmark) */
static int dialog_test()
{
	timer_start();
	validate_dialogs();
	timer_stop();

	return 0;
}

/* LoadShip (level loading) performance test */
static int loadship_bench()
{
	int loop = 10;

	// Find a ship file to load
	char fp[2048];
	find_file("freedroid.levels", MAP_DIR, fp, 0);

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
	sprintf(Me.character_name, "MapEd");

	// Load it many times
	timer_start();
	while (loop--) {
		LoadGame();
	}
	timer_stop();

	return 0;
}

/* Test of dynamic arrays */
static int dynarray_test()
{
	int loop = 10;
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
	}

	timer_stop();

	return 0;
}

static int mapgen_bench()
{
	int loop = 100;
	extern void CreateNewMapLevel(int);
	CreateNewMapLevel(0);
	level *l = curShip.AllLevels[0];
	l->xlen = 90;
	l->ylen = 90;
	l->random_dungeon = 2;
	l->teleport_pair = 0;

	timer_start();
	while (loop--) {
		set_dungeon_output(l);
		generate_dungeon(l->xlen, l->ylen, l->random_dungeon, l->teleport_pair);
	}
	timer_stop();

	return 0;
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
			{ "dynarray", dynarray_test },
			{ "mapgen", mapgen_bench },
	};

	int i;
	char str[1024];
	int failed = 0;

	ClearGraphMem();
	sprintf(str, "Testing \"%s\"...", do_benchmark);
	PutString(Screen, 10, 100, str);
	our_SDL_flip_wrapper();


	for (i = 0; i < sizeof(benchs)/sizeof(benchs[0]); i++) {
		if (!strcmp(do_benchmark, benchs[i].name)) {
			failed |= benchs[i].func();
			printf("Running test %s took %d milliseconds.\n", do_benchmark, stop_stamp - start_stamp);
			return failed;
		}
	}
	
	fprintf(stderr, "Unrecognized test %s. Existing tests are:\n", do_benchmark);
	for (i = 0; i < sizeof(benchs)/sizeof(benchs[0]); i++) {
		fprintf(stderr, "\t%s\n", benchs[i].name);
	}
	exit(EXIT_FAILURE);
}


#undef _benchmark_c
