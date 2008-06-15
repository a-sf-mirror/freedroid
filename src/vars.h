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

/* 
 *  _Definitions_ of global variables
 * This file should only be included in main.c, and 
 * the variable _declarations_ should be made in global.h under _main_c
 *
 */

char* floor_tile_filenames [ ALL_ISOMETRIC_FLOOR_TILES ] =
  {
    "iso_miscellaneous_floor_0000.png" , 
    "iso_miscellaneous_floor_0001.png" , 
    "iso_miscellaneous_floor_0002.png" , 
    "iso_miscellaneous_floor_0003.png" , 
    "iso_miscellaneous_floor_0004.png" , 
    "iso_miscellaneous_floor_0005.png" , 
    "iso_grass_floor_0000.png" , 
    "iso_grass_floor_0001.png" , 
    "iso_grass_floor_0002.png" , 
    "iso_grass_floor_0003.png" , 
    "iso_grass_floor_0004.png" , 
    "iso_grass_floor_0005.png" , 
    "iso_grass_floor_0006.png" , 
    "iso_grass_floor_0007.png" , 
    "iso_grass_floor_0008.png" , 
    "iso_grass_floor_0009.png" , 
    "iso_grass_floor_0010.png" , 
    "iso_grass_floor_0011.png" , 
    "iso_grass_floor_0012.png" , 
    "iso_grass_floor_0013.png" , 
    "iso_grass_floor_0014.png" , 
    "iso_grass_floor_0015.png" , 
    "iso_grass_floor_0016.png" , 
    "iso_grass_floor_0017.png" , 
    "iso_grass_floor_0018.png" , 
    "iso_grass_floor_0019.png" , 
    "iso_grass_floor_0020.png" , 
    "iso_grass_floor_0021.png" , 
    "iso_grass_floor_0022.png" , 
    "iso_grass_floor_0023.png" , 
    "iso_miscellaneous_floor_0006.png" , 
    "iso_miscellaneous_floor_0007.png" , 
    "iso_miscellaneous_floor_0008.png" , 
    "iso_grass_floor_0024.png" , 
    "iso_grass_floor_0025.png" ,
    "iso_grass_floor_0026.png" , 
    "iso_grass_floor_0027.png" , 
    "iso_grass_floor_0028.png" ,

    "iso_miscellaneous_floor_0009.png" ,
    "iso_miscellaneous_floor_0010.png" ,
    "iso_miscellaneous_floor_0011.png" ,
    "iso_miscellaneous_floor_0012.png" ,
    "iso_miscellaneous_floor_0013.png" ,
    "iso_miscellaneous_floor_0014.png" ,
    "iso_miscellaneous_floor_0015.png" ,
    "iso_miscellaneous_floor_0016.png" ,
    "iso_miscellaneous_floor_0017.png" ,
    "iso_miscellaneous_floor_0018.png" ,
    "iso_miscellaneous_floor_0019.png" ,
    "iso_miscellaneous_floor_0020.png" ,
    "iso_miscellaneous_floor_0021.png" ,
    "iso_miscellaneous_floor_0022.png" , 

    "iso_sidewalk_0001.png" ,
    "iso_sidewalk_0002.png" ,
    "iso_sidewalk_0003.png" ,
    "iso_sidewalk_0004.png" ,
    "iso_sidewalk_0005.png" ,
    "iso_sidewalk_0006.png" ,
    "iso_sidewalk_0007.png" ,
    "iso_sidewalk_0008.png" ,
    "iso_sidewalk_0009.png" ,
    "iso_sidewalk_0010.png" ,
    "iso_sidewalk_0011.png" ,
    "iso_sidewalk_0012.png" ,
    "iso_sidewalk_0013.png" ,
    "iso_sidewalk_0014.png" ,
    "iso_sidewalk_0015.png" ,
    "iso_sidewalk_0016.png" ,
    "iso_sidewalk_0017.png" ,
    "iso_sidewalk_0018.png" ,
    "iso_sidewalk_0019.png" ,
    "iso_sidewalk_0020.png" ,

    "iso_miscellaneous_floor_0023.png" ,
    
    "iso_sand_floor_0001.png" , 
    "iso_sand_floor_0002.png" ,
    "iso_sand_floor_0003.png" ,
    "iso_sand_floor_0004.png" ,
    "iso_sand_floor_0005.png" ,
    "iso_sand_floor_0006.png" ,

    // , "ERROR_UNUSED.png" , 
  };

tux_t Me ;

char* AllSkillTexts [ NUMBER_OF_SKILL_LEVELS ] =
  {
    N_("novice"),
    N_("average"),
    N_("experienced"),
    N_("skilled"),
    N_("adept"),
    N_("masterful"),
    N_("inhuman"),
    N_("god-like"),
    N_("super-god-like"),
    N_("wicked sick")
  };

//--------------------
// When a character aquires better melee_weapon_skill for melee
// weapons or better ranged_weapon_skill for ranged weapons, this
// will affect (his chance to hit and also) the damage the player
// does, by applying a multiplier to the normal computed damage.
// These multipliers are given by the table below.
//
// 0.9^x -> 1.0, 0.9, 0.81, 0.73, 0.66, 0.59, 0.53, 0.48, 0.43, 0.39
// 1.1^x -> 1.0, 1.1, 1.21, 1.33, 1.46, 1.61, 1.77, 1.95, 2.14, 2.36

float MeleeDamageMultiplierTable [  ] =
  {  1.0, 1.1, 1.21, 1.33, 1.46, 1.61, 1.77, 1.95, 2.14, 2.36 } ;
float MeleeRechargeMultiplierTable [  ] =
  {  1.0, 0.9, 0.81, 0.73, 0.66, 0.59, 0.53, 0.48, 0.43, 0.39 } ;

float RangedDamageMultiplierTable [  ] =
  {  1.0, 1.1, 1.21, 1.33, 1.46, 1.61, 1.77, 1.95, 2.14, 2.36 } ;
float RangedRechargeMultiplierTable [  ] =
  {  1.0, 0.9, 0.81, 0.73, 0.66, 0.59, 0.53, 0.48, 0.43, 0.39 } ;

//--------------------
// In the game, many spells are still internally similar
// to bullets flying around.  But what hit chance shall this
// bullet have?  --  That will depend upon the skill level
// of the caster.  The details will be taken from the following
// table:
//
int SpellHitPercentageTable [  ] =
  {  50 , 70 , 90 , 110 , 130 , 150 , 190 , 230 , 270 } ;

spell_skill_spec *SpellSkillMap; 
int number_of_skills;

Druidspec Druidmap;

Bulletspec Bulletmap;

blastspec Blastmap[ALLBLASTTYPES];

char font_switchto_red [ 2 ] = { 1 , 0 };
char font_switchto_blue [ 2 ] = { 2 , 0 };
char font_switchto_neon [ 2 ] = { 3 , 0 };

int skip_initial_menus = FALSE ;

supported_languages_t supported_languages []=  {{ .code="C", .name="English", .font_class="" },
					    	{ .code="fr_FR", .name="French", .font_class="" },
						{ .code="de_DE", .name="Deutsch", .font_class="" },
					    	{ .code="sv_SE", .name="Swedish", .font_class="" },
					    	{ .code="ru_RU.cp1251", .name="Russian", .font_class=".cp1251" },
					    	{ NULL, NULL, NULL}, };

int last_bot_number;

