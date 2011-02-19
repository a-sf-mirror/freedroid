/* 
 *
 *   Copyright (c) 2008-2009 Samuel Degrande
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
 * This file contains all functions to validate level maps.
 * Used by the level editor.
 */

#ifndef _leveleditor_validator_h_
#define _leveleditor_validator_h_

#include "defs.h"
#include "struct.h"
#include "global.h"
#include "proto.h"
#include "lists.h"

#undef EXTERN
#ifdef _leveleditor_validator_c
#define EXTERN
#else
#define EXTERN extern
#endif

#ifdef _leveleditor_validator_c

struct lvlval_ctx {
	SDL_Rect *report_rect;
	Level this_level;
	int in_report_section;
	int error_caught;
};

struct lvlval_error {
	char *title;
	char *comment;
	char *format;
	int caught;
	int is_error;
};

struct lvlval_excpt_item {
	int rule_id;
	int caught;
	void *opaque_data;
	struct list_head node;
};

struct level_validator {
	char initial;
	struct list_head excpt_list;
	void (*execute) (struct level_validator * this, struct lvlval_ctx * validator_ctx);
	void *(*parse_excpt) (char *string);
	int (*cmp) (void *opaque_data1, void *opaque_data2);
};

#endif

EXTERN void LevelValidation(void);

#endif				// _leveleditor_validator_h_
