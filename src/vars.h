/* 
 *
 *   Copyright (c) 1994, 2002, 2003 Johannes Prix
 *   Copyright (c) 1994, 2002 Reinhard Prix
 *   Copyright (c) 2004-2010 Arthur Huillet 
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

/* 
 *  _Definitions_ of global variables
 * This file should only be included in main.c, and 
 * the variable _declarations_ should be made in global.h under _main_c
 *
 */

tux_t Me;

char *AllSkillTexts[NUMBER_OF_SKILL_LEVELS] = {
	N_("untrained"),
	N_("novice"),
	N_("apprentice"),
	N_("professional"),
	N_("expert"),
	N_("master"),
	N_("god"),
	N_("haxx0r"),
	N_("l33t haxx0r"),
	N_("cheater")
};

//--------------------
// When a character acquires better melee_weapon_skill for melee
// weapons or better ranged_weapon_skill for ranged weapons, this
// will affect (his chance to hit and also) the damage the player
// does, by applying a multiplier to the normal computed damage.
// These multipliers are given by the table below.
//             0    1     2     3     4     5     6     7     8     9
// 1/1.2^x -> 1.0, 0.83, 0.69, 0.58, 0.48, 0.40, 0.33, 0.28, 0.23, 0.19
// 0,85^x  -> 1.0, 0.85, 0.72, 0.61, 0.52, 0.44, 0.38, 0.32, 0.27, 0.23
// 0.9^x   -> 1.0, 0.9,  0.81, 0.73, 0.66, 0.59, 0.53, 0.48, 0.43, 0.39
// 1.1^x   -> 1.0, 1.1,  1.21, 1.33, 1.46, 1.61, 1.77, 1.95, 2.14, 2.36
// 1.2^x   -> 1.0, 1.2,  1.44, 1.73, 2.07, 2.49, 2.99, 3.58, 4.30, 5.16

float MeleeDamageMultiplierTable[] = { 1.0, 1.1, 1.21, 1.33, 1.46, 1.61, 1.77, 1.95, 2.14, 2.36 };
float MeleeRechargeMultiplierTable[] = { 1.0, 0.9, 0.81, 0.73, 0.66, 0.59, 0.53, 0.48, 0.43, 0.39 };

float RangedDamageMultiplierTable[] = { 1.0, 1.1, 1.21, 1.33, 1.46, 1.61, 1.77, 1.95, 2.14, 2.36 };
float RangedRechargeMultiplierTable[] = { 1.0, 0.9, 0.81, 0.73, 0.66, 0.59, 0.53, 0.48, 0.43, 0.39 };

spell_skill_spec *SpellSkillMap;
int number_of_skills;

droidspec *Droidmap;

blastspec Blastmap[ALLBLASTTYPES];

char font_switchto_red[] = { 1, 0, 'r' };
char font_switchto_blue[] = { 2, 0, 'w' };
char font_switchto_neon[] = { 3, 0, 'n' };
char font_switchto_msgstat[] = { 4, 0, 's' };
char font_switchto_msgvar[] = { 5, 0, 'v' };

int skip_initial_menus = FALSE;

enum { INSIDE_MENU = 0, INSIDE_GAME, INSIDE_LVLEDITOR } game_status;
