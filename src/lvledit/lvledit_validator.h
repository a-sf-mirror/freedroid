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

#undef EXTERN
#ifdef _leveleditor_validator_c
#define EXTERN
#else
#define EXTERN extern
#endif

typedef struct level_validator_ctx_s {
	SDL_Rect* report_rect;
	Level this_level;
} level_validator_ctx;


typedef int (*level_validator)(  level_validator_ctx* ValidatorCtx  );
extern level_validator level_validators[];

EXTERN int chest_reachable_validator( level_validator_ctx* ValidatorCtx );
EXTERN int waypoint_validator(level_validator_ctx *ValidatorCtx);
EXTERN int interface_validator(level_validator_ctx *ValidatorCtx);
EXTERN int jumptarget_validator(level_validator_ctx *ValidatorCtx);
EXTERN void LevelValidation();

#endif // _leveleditor_validator_h_
