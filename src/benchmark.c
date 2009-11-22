/* 
 *
 *   Copyright (c) 2009 Arthur Huillet 
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

#include "valgrind/callgrind.h"
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
static void text_bench()
{
    char *str = "abcdefghijklmnopqrstuvwxyz0123456789abcdefghijklmnopqrstuvwxyz";

	// Make sure all glyphs are loaded
	PutString(Screen, 0, 0, str);

	// Display the string many times	
	int nb = 10000;

	CALLGRIND_START_INSTRUMENTATION;
	timer_start();
	while (nb--) {
		PutString(Screen, 0, 0, str);
	}

	our_SDL_update_rect_wrapper(Screen, 0, 0, Screen->w, Screen->h);
	timer_stop();
	CALLGRIND_STOP_INSTRUMENTATION;
}

void benchmark()
{
	struct {
		char *name;
		void (*func)();
	} benchs[] = {
			{ "text", text_bench },
	};

	int i;

	ClearGraphMem();
	PutString(Screen, 10, 100, "Benchmarking...");
	our_SDL_update_rect_wrapper(Screen, 0, 0, Screen->w, Screen->h);


	for (i = 0; i < sizeof(benchs)/sizeof(benchs[0]); i++) {
		if (!strcmp(do_benchmark, benchs[i].name)) {
			benchs[i].func();
			printf("Running benchmark %s took %d milliseconds.\n", do_benchmark, stop_stamp - start_stamp);
			return;
		}
	}
	
	fprintf(stderr, "Unrecognized benchmark %s. Existing tests are:\n", do_benchmark);
	for (i = 0; i < sizeof(benchs)/sizeof(benchs[0]); i++) {
		fprintf(stderr, "\t%s\n", benchs[i].name);
	}
	exit(1);
}


#undef _benchmark_c
