/* 
 *
 *   Copyright (c) 2004-2010 Arthur Huillet
 *   Copyright (c) 1994, 2002, 2003 Johannes Prix
 *   Copyright (c) 1994, 2002 Reinhard Prix
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
 * (Not all of the) Initialization routines for FreedroidRPG.
 */

#define _init_c

#include "system.h"

#include "defs.h"
#include "struct.h"
#include "global.h"
#include "proto.h"
#include "widgets/widgets.h"

#include "getopt.h"

#ifdef __WIN32__
// For _mkdir()
#include <direct.h>
// For SHGetSpecialFolderPath()
#ifndef _WIN32_IE
// From MSDN: The Microsoft Internet Explorer 4.0 Desktop Update must be installed for this function to be available.
#define _WIN32_IE 0x0400
#endif
#include <shlobj.h>
#endif

#ifdef ENABLE_NLS
#include <iconv.h>
#endif

void Init_Game_Data(void);
void UpdateCountersForThisFrame();
void DoAllMovementAndAnimations(void);

struct dynarray difficulties;

int term_has_color_cap = FALSE;
int run_from_term = FALSE;

/**
 *
 *
 */
void clear_out_arrays_for_fresh_game(void)
{
	int i;

	for (i = 0; i < MAX_MELEE_SHOTS; i++)
		delete_melee_shot(&AllMeleeShots[i]);

	for (i = 0; i < MAXBULLETS; i++) {
		DeleteBullet(i, FALSE);
	}
	for (i = 0; i < MAXBLASTS; i++) {
		DeleteBlast(i);
	}
	clear_active_spells();

	clear_enemies();
	clear_volatile_obstacles();
	ClearAutomapData();

	clear_npcs();

	init_tux();
}

/** 
 * This function displays a startup status bar that shows a certain
 * percentage of loading done.
 */
void next_startup_percentage(int done)
{
	static int startup_percent = 0;
	SDL_Rect Bar_Rect;

	/* Define STARTUP_PERCENTAGE_COMPUTE to get an estimate
	   of the real percentage associated to each step during the
	   game startup.
	   */
#ifdef STARTUP_PERCENTAGE_COMPUTE
	static long load_start_time;

	struct load_step {
		long elapsed;
		int percent;
	};

	static struct dynarray *step_times = NULL;
	if (!step_times)
		step_times = dynarray_alloc(10, sizeof(struct load_step));

	if (!startup_percent)
		load_start_time = SDL_GetTicks();
#endif

	startup_percent += done;
	if (startup_percent > 100)
		startup_percent = 100;

#ifdef STARTUP_PERCENTAGE_COMPUTE
	long elapsed = SDL_GetTicks() - load_start_time;

	printf("Step %d: %ld ms have elapsed (%d%%), now at %d%%\n", step_times->size, elapsed, done, startup_percent);

	struct load_step step = { elapsed, done };
	dynarray_add(step_times, &step, sizeof(struct load_step));

	if (startup_percent >= 100) {
		float ms_to_percent = 100.0 / elapsed;
		printf("Loading took %ld ms, 1 ms = %f%%\n", elapsed, ms_to_percent);

		int i;
		for (i = 1; i < step_times->size; i++) {
#define STEP(X) ((struct load_step *)step_times->arr)[X]
			int step_time = STEP(i).elapsed - STEP(i-1).elapsed;
			int step_percent = step_time * ms_to_percent;
			printf("Step %d: %d ms -> %d%%\n", i, step_time, step_percent);
		}
	}
#endif

	if (use_open_gl)
		blit_background("startup1.jpg");

	Bar_Rect.x = 160 * GameConfig.screen_width / 640;
	Bar_Rect.y = 288 * GameConfig.screen_height / 480;
	Bar_Rect.w = 3 * startup_percent * GameConfig.screen_width / 640;
	Bar_Rect.h = 30 * GameConfig.screen_height / 480;
	draw_rectangle(&Bar_Rect, 150, 200, 225, 255);

	Bar_Rect.x = (160 + 3 * startup_percent) * GameConfig.screen_width / 640;
	Bar_Rect.y = 288 * GameConfig.screen_height / 480;
	Bar_Rect.w = (300 - 3 * startup_percent) * GameConfig.screen_width / 640;
	Bar_Rect.h = 30 * GameConfig.screen_height / 480;
	draw_rectangle(&Bar_Rect, 0, 0, 0, 255);

	SDL_SetClipRect(Screen, NULL);

	set_current_font(Blue_Font);
	char percent[10];
	sprintf(percent, "%d%%", startup_percent);
	display_text(percent, UNIVERSAL_COORD_W(310) - 9, UNIVERSAL_COORD_H(301) - 7, NULL, 1.0);

	our_SDL_update_rect_wrapper(Screen, 0, 0, Screen->w, Screen->h);

}

/**
 * This function can be used to play a generic title file, containing 
 * 
 *  1. a background picture name
 *  2. a background music to play
 *  3. some text to display in a scrolling fashion
 *
 */
void PlayATitleFile(char *Filename)
{
	struct title_screen screen = { NULL, NULL, NULL };
	char fpath[PATH_MAX];

	while (SpacePressed() || MouseLeftPressed()) ;

	if (find_localized_file(Filename, TITLES_DIR, fpath, PLEASE_INFORM)) {
		set_lua_ctor_upvalue(LUA_CONFIG, "title_screen", &screen);
		run_lua_file(LUA_CONFIG, fpath);

#ifdef ENABLE_NLS
		// Convert the title_screen(s text to selected charset encoding
		struct auto_string *tocode = alloc_autostr(64);
		autostr_printf(tocode, "%s//TRANSLIT", lang_get_encoding());
		iconv_t converter = iconv_open(tocode->value, "UTF-8");
		free_autostr(tocode);
		if (converter == (iconv_t)-1) {
			error_once_message(ONCE_PER_GAME, __FUNCTION__,
			                   "Error on iconv_open() (encoding: %s): %s",
			                   NO_REPORT, lang_get_encoding(), strerror(errno));
		} else {
#if __FreeBSD__
			// FreeBSD changed to its own iconv starting with 10-CURRENT
			// Future ref: check for OS version < 10-CURRENT would allow non-const
			const char *in_ptr = screen.text;
#else
			char *in_ptr = screen.text;
#endif	//__FreeBSD__
			size_t in_len = strlen(screen.text);
			// We currently only have 8 bits encodings, so we should not need
			// an output buffer larger than the input one.
			char *converted_text = MyMalloc(in_len+1);
			char *out_ptr = converted_text;
			size_t out_len = in_len;

			size_t nb = iconv(converter, &in_ptr, &in_len, &out_ptr, &out_len);

			// In case of error, use the un-converted text
			if (nb == (size_t)-1) {
				if (errno == EILSEQ || errno == EINVAL) {
					error_once_message(ONCE_PER_GAME, __FUNCTION__,
				                       "Error during Title text conversion (title: %s - encoding: %s): %s\n"
					                   "Invalid sequence:\n--->%.20s...<---",
									   PLEASE_INFORM, Filename, lang_get_encoding(), strerror(errno), in_ptr);
				} else {
					error_once_message(ONCE_PER_GAME, __FUNCTION__,
				                       "Error during Title text conversion (title: %s - encoding: %s): %s",
									   NO_REPORT, Filename, lang_get_encoding(), strerror(errno));
				}
				free(converted_text);
			} else {
				// Replace the title's text by the converted one
				free(screen.text);
				screen.text = converted_text;
			}
		}
		iconv_close(converter);
#endif

		// Remove trailing whitespaces and carriage returns.
		char *ptr = screen.text + strlen(screen.text) - 1;
		while (*ptr != '\0' && (*ptr == ' ' || *ptr == '\t' || *ptr == '\n')) *(ptr--) = '\0';

		switch_background_music(screen.song);

		SDL_SetClipRect(Screen, NULL);
		set_current_font(Para_Font);

		ScrollText(screen.text,screen.background);

		clear_screen();
		our_SDL_flip_wrapper();

		free(screen.background);
		free(screen.song);
		free(screen.text);
	}
}

/**
 * This function loads all the constant concerning robot archetypes
 * from a section in memory to the actual archetype structures.
 */
static void Get_Robot_Data(void *DataPointer)
{
	int RobotIndex = 0;
	char *RobotPointer;
	char *EndOfDataPointer;
	int i;

#define ROBOT_SECTION_BEGIN_STRING "*** Start of Robot Data Section: ***"
#define ROBOT_SECTION_END_STRING "*** End of Robot Data Section: ***"
#define NEW_ROBOT_BEGIN_STRING "** Start of new Robot: **"
#define DROIDNAME_BEGIN_STRING "Droidname: "
#define PORTRAIT_FILENAME_WITHOUT_EXT "Droid portrait file name (without extension) to use=\""

#define DROID_PORTRAIT_ROTATION_SERIES_NAME_PREFIX "Droid uses portrait rotation series with prefix=\""

#define MAXSPEED_BEGIN_STRING "Maximum speed of this droid: "
#define CLASS_BEGIN_STRING "Class of this droid: "
#define MAXENERGY_BEGIN_STRING "Maximum energy of this droid: "
#define BASE_HEALING_BEGIN_STRING "Rate of healing: "
#define SENSOR_ID_BEGIN_STRING "Sensor ID=\""
#define EXPERIENCE_REWARD_BEGIN_STRING "Experience_Reward gained for destroying one of this type: "
#define WEAPON_ITEM_BEGIN_STRING "Weapon item=\""
#define GREETING_SOUND_STRING "Greeting Sound number="
#define DROID_DEATH_SOUND_FILE_NAME "Death sound file name=\""
#define DROID_ATTACK_ANIMATION_SOUND_FILE_NAME "Attack animation sound file name=\""
#define TO_HIT_STRING "Chance of this robot scoring a hit="
#define GETTING_HIT_MODIFIER_STRING "Chance modifier, that this robot gets hit="
#define IS_HUMAN_SPECIFICATION_STRING "Is this 'droid' a human : "
#define INDIVIDUAL_SHAPE_SPECIFICATION_STRING "Individual shape of this droid or just -1 for classic ball shaped : "
#define NOTES_BEGIN_STRING "Notes concerning this droid=_\""

	RobotPointer = LocateStringInData(DataPointer, ROBOT_SECTION_BEGIN_STRING);
	EndOfDataPointer = LocateStringInData(DataPointer, ROBOT_SECTION_END_STRING);

	DebugPrintf(1, "\n\nStarting to read Robot data...\n\n");
	// At first, we must allocate memory for the droid specifications.
	// How much?  That depends on the number of droids defined in freedroid.ruleset.
	// So we have to count those first.  ok.  lets do it.

	Number_Of_Droid_Types = CountStringOccurences(DataPointer, NEW_ROBOT_BEGIN_STRING);
	if (NB_DROID_TYPES < Number_Of_Droid_Types + 2) {
		error_message(__FUNCTION__, "\
The value of %d for \"NB_DROID_TYPES\" defined in struct.h is less than %d\n\
which is \"Number_of_Droid_Types\" + 2. Please increase the value of \"NB_DROID_TYPES\"!",
			PLEASE_INFORM | IS_FATAL, NB_DROID_TYPES, Number_Of_Droid_Types + 2);
	}

	// Not that we know how many robots are defined in freedroid.ruleset, we can allocate
	// a fitting amount of memory.
	i = sizeof(droidspec);
	Droidmap = MyMalloc(i * (Number_Of_Droid_Types + 1) + 1);
	DebugPrintf(1, "\nWe have counted %d different droid types in the game data file.", Number_Of_Droid_Types);
	DebugPrintf(2, "\nMEMORY HAS BEEN ALLOCATED.\nTHE READING CAN BEGIN.\n");

	//Now we start to read the values for each robot:
	//Of which parts is it composed, which stats does it have?
	while ((RobotPointer = strstr(RobotPointer, NEW_ROBOT_BEGIN_STRING)) != NULL) {
		DebugPrintf(2, "\n\nFound another Robot specification entry!  Lets add that to the others!");
		RobotPointer++;	// to avoid doubly taking this entry
		char *EndOfThisRobot = strstr(RobotPointer, NEW_ROBOT_BEGIN_STRING);
		if (EndOfThisRobot)
			EndOfThisRobot[0] = 0;

		// Now we read in the Name of this droid.  We consider as a name the rest of the
		// line with the DROIDNAME_BEGIN_STRING until the "\n" is found.
		Droidmap[RobotIndex].droidname = ReadAndMallocStringFromData(RobotPointer, DROIDNAME_BEGIN_STRING, "\n");

		// Now we read in the default short_description_text for this droid.
		Droidmap[RobotIndex].default_short_description = ReadAndMallocStringFromDataOptional(RobotPointer, "Default description:_\"", "\"");
		if (!Droidmap[RobotIndex].default_short_description)
			Droidmap[RobotIndex].default_short_description = strdup("");

		// Now we read in the prefix of the file names in the rotation series
		// to use for the console droid rotation
		Droidmap[RobotIndex].droid_portrait_rotation_series_prefix =
		    ReadAndMallocStringFromData(RobotPointer, DROID_PORTRAIT_ROTATION_SERIES_NAME_PREFIX, "\"");

		// Now we read in the file name of the death sound for this droid.  
		// Is should be enclosed in double-quotes.
		//
		Droidmap[RobotIndex].droid_death_sound_file_name =
		    ReadAndMallocStringFromData(RobotPointer, DROID_DEATH_SOUND_FILE_NAME, "\"");

		// Now we read in the file name of the attack animation sound for this droid.  
		// Is should be enclosed in double-quotes.
		//
		Droidmap[RobotIndex].droid_attack_animation_sound_file_name =
		    ReadAndMallocStringFromData(RobotPointer, DROID_ATTACK_ANIMATION_SOUND_FILE_NAME, "\"");

		// Now we read in the maximal speed this droid can go. 
		ReadValueFromString(RobotPointer, MAXSPEED_BEGIN_STRING, "%f", &Droidmap[RobotIndex].maxspeed, EndOfDataPointer);

		// Now we read in the class of this droid.
		ReadValueFromString(RobotPointer, CLASS_BEGIN_STRING, "%d", &Droidmap[RobotIndex].class, EndOfDataPointer);

		// Now we read in the maximal energy this droid can store. 
		ReadValueFromString(RobotPointer, MAXENERGY_BEGIN_STRING, "%f", &Droidmap[RobotIndex].maxenergy, EndOfDataPointer);

		// Now we read in the lose_health rate.
		ReadValueFromString(RobotPointer, BASE_HEALING_BEGIN_STRING, "%f", &Droidmap[RobotIndex].healing_friendly, EndOfDataPointer);
		Droidmap[RobotIndex].healing_hostile = Droidmap[RobotIndex].healing_friendly;

		// Now we read what sensor this robot use.
		char *tmp_sensor_ID = ReadAndMallocStringFromDataOptional(RobotPointer, SENSOR_ID_BEGIN_STRING, "\"");
		if (!tmp_sensor_ID)
			tmp_sensor_ID = strdup("spectral"); // Default value if no sensor defined.
		Droidmap[RobotIndex].sensor_id = get_sensor_id_by_name(tmp_sensor_ID);
		free(tmp_sensor_ID);

		// Now we read in range of vision of this droid
		ReadValueFromString(RobotPointer, "Aggression distance of this droid=", "%f",
				    &Droidmap[RobotIndex].aggression_distance, EndOfDataPointer);

		// Now we read in range of vision of this droid
		ReadValueFromString(RobotPointer, "Time spent eyeing Tux=", "%f",
				    &Droidmap[RobotIndex].time_spent_eyeing_tux, EndOfDataPointer);

		// Now we experience_reward to be had for destroying one droid of this type
		ReadValueFromString(RobotPointer, EXPERIENCE_REWARD_BEGIN_STRING, "%hd",
				    &Droidmap[RobotIndex].experience_reward, EndOfDataPointer);

		// Now we read in the monster level = maximum treasure chest to pick from
		ReadValueFromStringWithDefault(RobotPointer, "Drops item class=", "%hd", "-1", 
					       &Droidmap[RobotIndex].drop_class, EndOfDataPointer);

		char *tmp_item_id = ReadAndMallocStringFromData(RobotPointer, WEAPON_ITEM_BEGIN_STRING, "\"");
		Droidmap[RobotIndex].weapon_item.type = get_item_type_by_id(tmp_item_id);
		free(tmp_item_id);
		ReadValueFromStringWithDefault(RobotPointer, "Gun muzzle height=", "%d", "30", &Droidmap[RobotIndex].gun_muzzle_height, EndOfDataPointer);

		// Now we read in the % chance for droid to drop botpart
		ReadValueFromStringWithDefault(RobotPointer, "Percent to drop Entropy Inverter=", "%hd", "0",
					       &Droidmap[RobotIndex].amount_of_entropy_inverters, EndOfDataPointer);
		ReadValueFromStringWithDefault(RobotPointer, "Percent to drop Plasma Transistor=", "%hd", "0",
					       &Droidmap[RobotIndex].amount_of_plasma_transistors, EndOfDataPointer);
		ReadValueFromStringWithDefault(RobotPointer, "Percent to drop Superconducting Relay Unit=", "%hd", "0",
					       &Droidmap[RobotIndex].amount_of_superconductors, EndOfDataPointer);
		ReadValueFromStringWithDefault(RobotPointer, "Percent to drop Antimatter-Matter Converter=", "%hd", "0",
					       &Droidmap[RobotIndex].amount_of_antimatter_converters, EndOfDataPointer);
		ReadValueFromStringWithDefault(RobotPointer, "Percent to drop Tachyon Condensator=", "%hd", "0",
					       &Droidmap[RobotIndex].amount_of_tachyon_condensators, EndOfDataPointer);

		// Now we read in the greeting sound type of this droid type
		ReadValueFromString(RobotPointer, GREETING_SOUND_STRING, "%hd",
				    &Droidmap[RobotIndex].greeting_sound_type, EndOfDataPointer);

		// Now we read in the to-hit chance this robot has in combat against an unarmored target
		ReadValueFromString(RobotPointer, TO_HIT_STRING, "%hd", &Droidmap[RobotIndex].to_hit, EndOfDataPointer);

		// Now we read in the modifier, that increases/decreases the chance of this robot getting hit
		ReadValueFromString(RobotPointer, "Time to recover after getting hit=", "%f",
				    &Droidmap[RobotIndex].recover_time_after_getting_hit, EndOfDataPointer);

		// Now we read in the is_human flag of this droid type
		ReadValueFromString(RobotPointer, IS_HUMAN_SPECIFICATION_STRING, "%hd", &Droidmap[RobotIndex].is_human, EndOfDataPointer);

		// Now we read in the Graphics to associate with this droid type
		char *enemy_surface = ReadAndMallocStringFromData(RobotPointer, "Filename prefix for graphics=\"", "\"");
		Droidmap[RobotIndex].individual_shape_nr = 0;
		for (i=0; i < ENEMY_ROTATION_MODELS_AVAILABLE; i++) {
			if (PrefixToFilename[i] && !strcmp(enemy_surface, PrefixToFilename[i])){
				Droidmap[RobotIndex].individual_shape_nr = i;
				break;
			}
		}
		free(enemy_surface);

		// Now we read in the notes about this droid type
		Droidmap[RobotIndex].notes = ReadAndMallocStringFromData(RobotPointer, NOTES_BEGIN_STRING, "\"");

		// Now we're potentially ready to process the next droid.  Therefore we proceed to
		// the next number in the Droidmap array.
		RobotIndex++;
		if (EndOfThisRobot)
			EndOfThisRobot[0] = '*';	// We put back the star at its place
	}

	struct difficulty *diff = dynarray_member(&difficulties, GameConfig.difficulty_level, sizeof(struct difficulty));

	for (i = 0; i < Number_Of_Droid_Types; i++) {
		Droidmap[i].maxspeed *= diff->droid_max_speed;
		Droidmap[i].maxenergy *= diff->droid_hpmax;
		Droidmap[i].experience_reward *= diff->droid_experience_reward;
		Droidmap[i].aggression_distance *= diff->droid_aggression_distance;
		Droidmap[i].healing_friendly *= diff->droid_friendly_healing;
		Droidmap[i].healing_hostile *= diff->droid_hostile_healing;
	}
};				// int Get_Robot_Data ( void )

/**
 * Load the configuration of the fdrpg "engine", that is the game independent data
 */
static void load_fdrpg_config()
{
	char fpath[PATH_MAX];

	// Load the languages specs
	dynarray_free(&lang_specs);
	dynarray_free(&lang_codesets);
	if (find_file("languages.lua", BASE_DIR, fpath, PLEASE_INFORM)) {
		run_lua_file(LUA_CONFIG, fpath);
	}
}

/**
 * This function loads all the constant variables of the game from
 * a data file, using mainly subroutines which do the main work.
 */
void Init_Game_Data()
{
	char fpath[PATH_MAX];
	char *Data;

	// Load difficulties.
	find_file("difficulties.lua", BASE_DIR, fpath, PLEASE_INFORM | IS_FATAL);
	run_lua_file(LUA_CONFIG, fpath);

	// Load skills and programs (spells) information
	find_file("skill_specs.lua", BASE_DIR, fpath, PLEASE_INFORM | IS_FATAL);
	run_lua_file(LUA_CONFIG, fpath);

	// Load the blast data (required for the bullets to load)
	find_file("blast_specs.lua", BASE_DIR, fpath, PLEASE_INFORM | IS_FATAL);
	run_lua_file(LUA_CONFIG, fpath);

	// Load the bullet data (required for the item archetypes to load)
	//
	dynarray_free(&bullet_specs);
	find_file("bullet_specs.lua", BASE_DIR, fpath, PLEASE_INFORM | IS_FATAL);
	run_lua_file(LUA_CONFIG, fpath);

	// Load Tux animation and rendering specifications.
	tux_rendering_load_specs("tuxrender_specs.lua");

	// Load Bot animation definitions and animations
	// must be before Get_Robot_Data()
	Load_Enemy_Surfaces();

	// Item archetypes must be loaded too
	find_file("item_specs.lua", BASE_DIR, fpath, PLEASE_INFORM | IS_FATAL);
	run_lua_file(LUA_CONFIG, fpath);

	// Load add-on specifications.
	find_file("addon_specs.lua", BASE_DIR, fpath, PLEASE_INFORM | IS_FATAL);
	run_lua_file(LUA_CONFIG, fpath);

	// Time to eat some droid archetypes...
	find_file("droid_archetypes.dat", BASE_DIR, fpath, PLEASE_INFORM | IS_FATAL);
	Data = ReadAndMallocAndTerminateFile(fpath, "*** End of this Freedroid data File ***");
	Get_Robot_Data(Data);
	free(Data);

	// Load obstacle specifications.
	dynarray_init(&obstacle_map, 512, sizeof(struct obstacle_spec));
	find_file("obstacle_specs.lua", BASE_DIR, fpath, PLEASE_INFORM | IS_FATAL);
	run_lua_file(LUA_CONFIG, fpath);

	// Load floor tile specifications.
	find_file("floor_tiles.lua", BASE_DIR, fpath, PLEASE_INFORM | IS_FATAL);
	run_lua_file(LUA_CONFIG, fpath);
	dirty_animated_floor_tile_list();

	next_startup_percentage(1);
}

char copyright[] = "\nFreedroidRPG comes with NO WARRANTY to the extent permitted by law.\n\
You may redistribute copies of FreedroidRPG\n\
under the terms of the GNU General Public License.\n\
For more information about these matters, see the file named COPYING.\n";

char usage_string[] = ""
"Usage: freedroidRPG [-h | --help]\n"
"                    [-v | --version]\n"
"                    [-e | --editor]\n"
"                    [-s | --sound]        [-q | --nosound]\n"
"                    [-o | --open_gl]      [-n | --no_open_gl]\n"
"                    [-f | --fullscreen]   [-w | --window]\n"
"                    [-t | --system_lang]\n"
"                    [-l character-name | --load=character-name]\n"
"                    [-r Y | --resolution=Y]  Y = 99 lists hardcoded resolutions.\n"
"                                             Y may also be of the form 'WxH' e.g. '800x600'\n"
"                    [-d X | --debug=X]       X = 0-5; default 1\n"
"                    [-b Z | --benchmark=Z]   Z = text | dialog | loadship | loadgame | savegame | dynarray | mapgen | leveltest\n"
"\n"
"Please report bugs either by entering them into the bug tracker on our website at:\n\n"
"http://bugs.freedroid.org\n\n"
"or by sending an e-mail to:\n\n"
"freedroid-discussion AT lists.sourceforge.net\n\n"
"For more information and known issues please see README.\n"
"Thanks a lot in advance.\n"
"                          / The FreedroidRPG dev team.\n\n";

#define MAX_RESOLUTIONS 32
screen_resolution screen_resolutions[MAX_RESOLUTIONS];
screen_resolution hard_resolutions[] = {
	{640, 480, "640 x 480", TRUE},
	{800, 600, "800 x 600", TRUE},
	{1024, 768, "1024 x 768", TRUE},
	{1152, 864, "1152 x 864", TRUE},
	{1280, 960, "1280 x 960", TRUE},
	{1400, 1050, "1400 x 1050", TRUE},
	{1600, 1200, "1600 x 1200", TRUE}
};

/* -----------------------------------------------------------------
 *  parse command line arguments and set global switches 
 *  exit on error, so we don't need to return success status
 * -----------------------------------------------------------------*/
void ParseCommandLine(int argc, char *const argv[])
{
	int c;
	int resolution_code = 1;

	static struct option long_options[] = {
		{"version",     0, 0, 'v'},
		{"help",        0, 0, 'h'},
		{"editor",      0, 0, 'e'},
		{"load",        1, 0, 'l'},
		{"open_gl",     0, 0, 'o'},
		{"no_open_gl",  0, 0, 'n'},
		{"nosound",     0, 0, 'q'},
		{"sound",       0, 0, 's'},
		{"debug",       1, 0, 'd'},
		{"window",      0, 0, 'w'},
		{"fullscreen",  0, 0, 'f'},
		{"resolution",  1, 0, 'r'},
		{"system_lang", 0, 0, 't'},
		{"benchmark",   1, 0, 'b'},
		{0, 0, 0, 0}
	};

	while (1) {
		c = getopt_long(argc, argv, "vel:onqsb:h?d::r:wft", long_options, NULL);
		if (c == -1)
			break;

		switch (c) {
			/* version statement -v or --version
			 * following gnu-coding standards for command line interfaces */
		case 'v':
			printf("\n%s %s  \n", PACKAGE, freedroid_version);
			printf("%s",copyright);
			exit(0);
			break;

		case 'h':
		case '?':
			printf("%s",usage_string);
			exit(0);
			break;

		case 'e':
			start_editor = TRUE;
			break;

		case 'l':
			if (!optarg) {
				fprintf(stderr, "Please provide saved game name.\n");
				exit(1);
			}

			load_saved = TRUE;
			saved_game_name = optarg;
			break;

		case 'o':
			use_open_gl = TRUE;
			break;

		case 'n':
			use_open_gl = FALSE;
			break;

		case 'q':
			sound_on = FALSE;
			break;

		case 's':
			sound_on = TRUE;
			break;

		case 'd':
			if (!optarg)
				debug_level = 1;
			else
				debug_level = atoi(optarg);
			break;

		case 'r':
			if (!optarg) {
				GameConfig.screen_width = DEFAULT_SCREEN_WIDTH;
				GameConfig.screen_height = DEFAULT_SCREEN_HEIGHT;
			} else {
				int nb_res = sizeof(hard_resolutions) / sizeof(hard_resolutions[0]);
				char *x = strtok(optarg, "x");
				char *y = strtok(NULL, "x");

				// User input a resolution
				if (y != NULL) {
					GameConfig.screen_width = atoi(x);
					GameConfig.screen_height = atoi(y);
				} else {

					resolution_code = atoi(optarg);

					if (resolution_code >= 0 && resolution_code < nb_res) {
						GameConfig.screen_width = hard_resolutions[resolution_code].xres;
						GameConfig.screen_height = hard_resolutions[resolution_code].yres;
					} else {
						fprintf(stderr, "\nresolution code received: %d\n", resolution_code);
						char *txt = (char *)malloc(nb_res * 128 + 1);
						txt[0] = '\0';
						int i;
						for (i = 0; i < nb_res; i++) {
							char tmp[129];
							snprintf(tmp, 129, "\t\t%d = %s\n", i, hard_resolutions[i].comment);
							strncat(txt, tmp, 128);
						}
						error_message(__FUNCTION__, "  %s%s  %s", IS_FATAL,
								 "\tThe resolution identifier given is not a valid resolution code.\n"
								 "\tThese codes correspond to the following hardcoded resolutions available:\n",
								 txt,
								 "\tAdditional resolutions may be specified by the form 'WxH' e.g. '800x600'\n"
								 "\tThe in-game menu automatically detects fullscreen modes supported by your hardware.");
						free(txt);
					}
				}
			}
			// By default, after starting up, the current resolution should be
			// the resolution used at the next game startup too, so we preselect
			// that for now.  The user can still change that later inside the
			// game from within the options menu.
			//
			GameConfig.next_time_width_of_screen = GameConfig.screen_width;
			GameConfig.next_time_height_of_screen = GameConfig.screen_height;
			break;
		case 'b':
			if (!optarg) {
				fprintf(stderr, "Please specify what to benchmark.\n");
				exit(1);
			}

			do_benchmark = strdup(optarg);
			break;
		case 'f':
			GameConfig.fullscreen_on = TRUE;
			break;

		case 'w':
			GameConfig.fullscreen_on = FALSE;
			break;

		case 't':
			lang_set("", NULL);
			break;

		default:
			printf("\nOption %c not implemented yet! Ignored.", c);
			break;
		}		/* switch(c) */
	}			/* while(1) */
}

/* -----------------------------------------------------------------
 * This function initializes a completely new game within FreedroidRPG.
 * In contrast to InitFreedroid, this function should be called 
 * whenever or better before any new game is started.
 * -----------------------------------------------------------------*/
void PrepareStartOfNewCharacter(char *start_label)
{
	gps start_pos;

	Activate_Conservative_Frame_Computation();

	// We make sure we don't have garbage in our arrays from a 
	// previous game or failed load-game attempt...
	//
	clear_out_arrays_for_fresh_game();
	// We do the same as above for lua state
	reset_lua_state();

	GetEventTriggers("events.dat");

	if (!skip_initial_menus)
		PlayATitleFile("StartOfGame.lua");

	init_npcs();
	init_factions();

	GetCrew("ReturnOfTux.droids");

	start_pos = get_map_label_center(start_label);
	
	reset_visible_levels();
	if (game_root_mode == ROOT_IS_LVLEDIT && level_exists(GameConfig.last_edited_level))
		teleport_to_level_center(GameConfig.last_edited_level);
	else
		Teleport(start_pos.z, start_pos.x, start_pos.y, FALSE, TRUE);
	clear_active_bullets();

	// At this point the position history can be initialized
	//
	InitInfluPositionHistory();

	// Now we read in the mission targets for this mission
	// Several different targets may be specified simultaneously
	//
	GetQuestList("quests.dat");

	switch_background_music(curShip.AllLevels[Me.pos.z]->Background_Song_Name);

	// Now we know that right after starting a new game, the Tux might have
	// to 'change clothes' i.e. a lot of tux images need to be updated which can
	// take a little time.  Therefore we print some message so the user will not
	// panic and push the reset button :)
	//
	our_SDL_flip_wrapper();

	widget_text_init(message_log, _("--- Message Log ---"));
	if (strcmp(start_label, "TutorialTuxStart") == 0)
		append_new_game_message(_("Starting tutorial."));
	else
		append_new_game_message(_("Starting new game."));
}

void prepare_level_editor()
{
	game_root_mode = ROOT_IS_LVLEDIT;
	game_status = INSIDE_LVLEDITOR;
	skip_initial_menus = 1;
	char fp[PATH_MAX];
	find_file("levels.dat", MAP_DIR, fp, PLEASE_INFORM | IS_FATAL);
	LoadShip(fp, 0);
	PrepareStartOfNewCharacter("NewTuxStartGameSquare");
	skip_initial_menus = 0;
	free(Me.character_name);
	Me.character_name = strdup("MapEd");
}

/**
 * This function sets the GameConfig back to the default values, NOT THE
 * VALUES STORED IN THE USERS CONFIG FILE.  This function is useful if 
 * no config file if found or if the config file turns out to originate
 * from a different version of FreedroidRPG, which could be dangerous as
 * well.
 */
void ResetGameConfigToDefaultValues(void)
{
	// At first we set audio volume to maximum value.
	// This might be replaced later with values from a 
	// private user FreedroidRPG config file.  But for now
	// this code is good enough...
	//
	GameConfig.Current_BG_Music_Volume = 0.5;
	GameConfig.Current_Sound_FX_Volume = 0.5;
	GameConfig.Current_Sound_Output_Fmt = SOUND_OUTPUT_FMT_STEREO;
	GameConfig.current_gamma_correction = 1.00;
	GameConfig.WantedTextVisibleTime = 3;
	GameConfig.Draw_Framerate = TRUE;
	GameConfig.Draw_Position = TRUE;
	GameConfig.All_Texts_Switch = FALSE;
	GameConfig.enemy_energy_bars_visible = TRUE;
	GameConfig.limit_framerate = TRUE;
	GameConfig.skip_light_radius = FALSE;
	GameConfig.omit_obstacles_in_level_editor = FALSE;
	GameConfig.omit_map_labels_in_level_editor = TRUE;
	GameConfig.omit_enemies_in_level_editor = TRUE;
	GameConfig.zoom_is_on = FALSE;
	GameConfig.show_blood = TRUE;
	GameConfig.show_lvledit_tooltips = TRUE;
	GameConfig.show_grid = TRUE;
	GameConfig.show_wp_connections = FALSE;
	GameConfig.number_of_big_screen_messages = 4;
	GameConfig.delay_for_big_screen_messages = 3;
	GameConfig.enable_cheatkeys = FALSE;
	GameConfig.skip_shadow_blitting = FALSE;
	GameConfig.do_fadings = TRUE;
	GameConfig.xray_vision_for_tux = FALSE;
	GameConfig.cheat_running_stamina = FALSE;
	GameConfig.lazyload = 1;
	GameConfig.show_item_labels = 0;
	GameConfig.last_edited_level = -1;
	GameConfig.show_all_floor_layers = 1;
	GameConfig.screen_width = DEFAULT_SCREEN_WIDTH;
	GameConfig.screen_height = DEFAULT_SCREEN_HEIGHT;
	GameConfig.next_time_width_of_screen = GameConfig.screen_width;
	GameConfig.next_time_height_of_screen = GameConfig.screen_height;
	GameConfig.fullscreen_on = FALSE;
	GameConfig.difficulty_level = DIFFICULTY_NORMAL;
	if (GameConfig.locale) {
		free(GameConfig.locale);
	}
	GameConfig.locale = my_strdup("");
}

/**
 * Release all allocated memory stored in GameConfig
 */
void gameconfig_clean()
{
	if (GameConfig.locale) {
		free(GameConfig.locale);
	}
}

/** 
 * Set signal handlers for SIGSEGV and SIGFPE.
 */
static void set_signal_handlers(void)
{
#ifndef __WIN32__
#ifndef __APPLE_CC__

	struct sigaction action;

	// We set up the structure for the new signal handling
	// to give to the operating system
	//
	action.sa_handler = print_trace;
	sigemptyset(&action.sa_mask);
	action.sa_flags = 0;

	sigaction(SIGSEGV, &action, NULL);
	sigaction(SIGFPE, &action, NULL);

#endif
#endif
}

/**
 * Enumerates resolutions supported by SDL
 */
static void detect_available_resolutions(void)
{
	SDL_Rect **modes;
	int size = 0;

	// Get available fullscreen/hardware modes (reported by SDL)
	modes = SDL_ListModes(NULL, SDL_FULLSCREEN|SDL_HWSURFACE);
	if (modes == (SDL_Rect**) -1) {
		error_message(__FUNCTION__,
			"SDL reports all resolutions are supported in fullscreen mode.\n"
			"Please use -r WIDTHxHEIGHT to specify any one you like.\n"
			"Defaulting to a sane one for now", NO_REPORT);
	} else {
		// Add resolutions to the screen_resolutions array
		for (size = 0; size < MAX_RESOLUTIONS && modes[size]; ++size)
			screen_resolutions[size] = (screen_resolution) {modes[size]->w, modes[size]->h, "", TRUE};
	}

	if (size == 0) {
		screen_resolutions[0] =	(screen_resolution) {DEFAULT_SCREEN_WIDTH, DEFAULT_SCREEN_HEIGHT, "", TRUE};
		screen_resolutions[1] = (screen_resolution) {-1, -1, "", FALSE};
		return;
	}

	// Sort
	int i, j;
	screen_resolution resolution_holder;

	for (i = 0; i < size; i++) {
		for (j = 0; j < size - 1; j++) {
			// Sort in descending order of xres, then yres
			if (screen_resolutions[j].xres < screen_resolutions[j + 1].xres ||
					(screen_resolutions[j].xres == screen_resolutions[j + 1].xres &&
					screen_resolutions[j].yres < screen_resolutions[j +1].yres)) {
				resolution_holder = screen_resolutions[j + 1];
				screen_resolutions[j + 1] = screen_resolutions[j];
				screen_resolutions[j] = resolution_holder;
			}
		}
	}

	// Add our terminator on the end, just in case
	screen_resolutions[size] = (screen_resolution) {-1, -1, "", FALSE};
}

void prepare_execution(int argc, char *argv[])
{
#if defined HAVE_UNISTD_H && defined HAVE_DIRNAME
	// change working directory to the executable's directory
	if (chdir(dirname(argv[0])))
		fprintf(stderr, "Couldn't change working directory to %s.\n", dirname(argv[0]));
#endif

	// Get color capability of current output stream.
	// Real code to get such a capability has to use setupterm() and
	// tigetnum() from the ncurses lib.
	// In order to avoid a dependency to ncurses, we use here a simple trick.
	run_from_term = FALSE;
	term_has_color_cap = FALSE;
#ifndef __WIN32__
	run_from_term = isatty(STDOUT_FILENO);
	if (run_from_term && !strncmp(getenv("TERM"), "xterm", 5))
		term_has_color_cap = TRUE;
#endif

	// Get the homedir, and define the directory where the config file and
	// the savegames will be stored
#if __WIN32__

	our_homedir = ".";
	// There is no real homedir on Windows.
	// So we use the user's "My Documents" as a home directory.
	char mydocuments_path[MAX_PATH];
	if (SHGetSpecialFolderPath(0, mydocuments_path, CSIDL_PERSONAL, FALSE)) {
		our_homedir = strdup(mydocuments_path);
	}
	our_config_dir = MyMalloc(strlen(our_homedir) + 20);
	sprintf(our_config_dir, "%s/FreedroidRPG", our_homedir);

#else

	// first we need the user's homedir for loading/saving stuff
	if ((our_homedir = getenv("HOME")) == NULL) {
		our_homedir = ".";
	}
	our_config_dir = MyMalloc(strlen(our_homedir) + 20);
	sprintf(our_config_dir, "%s/.freedroid_rpg", our_homedir);

#endif

	struct stat statbuf;
	int rtn;

	if (stat(our_config_dir, &statbuf) == -1) {
#		if __WIN32__
		rtn = _mkdir(our_config_dir);
#		else
		rtn = mkdir(our_config_dir, S_IREAD | S_IWRITE | S_IEXEC);
#		endif
		if (rtn == -1) {
			free(our_config_dir);
			our_config_dir = NULL;
			// If not run from term, we currently have no way to inform the
			// user that we were not able to create the subdir.
			// TODO: Use SDL2 SDL_ShowSimpleMessageBox()
			// Until the SDL2 port is done, well, we just abort somehow silently...
			DebugPrintf(-4, "Was not able to create a subdir to store the configuration and the savegames.\n");
			Terminate(EXIT_FAILURE);
		}
	}

	// If not run from a terminal, stdout and stderr are redirect to a text file
	// written in the config dir (fdrpg_out.txt).
	// Note: On Windows, in SDL_WinMain(), stdout and stderr are redirected to
	// files, before to call our main(). Those files are automatically created
	// in the directory of the current process. They will be removed when the process
	// ends, because they are empty.
	if (!run_from_term) {
		char *filename = MyMalloc(strlen(our_config_dir) + 15);
		sprintf(filename, "%s/fdrpg_out.txt", our_config_dir);
		if (!freopen(filename, "w", stdout)) {
			DebugPrintf(-4, "Was not able to redirect stdout to %s/fdrpg_out.txt", our_config_dir);
		}
		free(filename);
		if (dup2(fileno(stdout), fileno(stderr)) == -1) {
			DebugPrintf(-4, "Was not able to redirect stderr to stdout. Errno: %d", errno);
		}

		fprintf(stderr, "Hello!  This window contains the DEBUG OUTPUT of FreedroidRPG.\n"
		                "\n"
		                "Normally you would not see this message or this window, but apparently\n"
		                "FreedroidRPG has terminated because of an error of some sort.\n"
		                "\n"
		                "You might wish to inspect the debug output below.  Maybe sending the\n"
		                "debug output (or at least the lower end of the debug output) to the\n"
		                "FreedroidRPG developers could help them to track down the problem.\n"
		                "\n"
		                "Well, it's no guarantee that we can solve any bug, but it's certainly\n"
		                "better than nothing.  Thanks anyway for your interest in FreedroidRPG.\n"
		                "\n\n"
		                "--start of real debug log--\n\n");
	}

	// We mention the version of FreedroidRPG, so that debug reports
	// are easier to assign to the different versions of the game.

	DebugPrintf(-4, "\nHello, this is FreedroidRPG, version %s.\n", freedroid_version);
}

/* -----------------------------------------------------------------
 * This function initializes the whole FreedroidRPG game.
 *
 * THIS MUST NOT BE CONFUSED WITH INITNEWGAME, WHICH
 * ONLY INITIALIZES A NEW MISSION FOR THE GAME.
 *
 * ----------------------------------------------------------------- */
void InitFreedroid(int argc, char **argv)
{
	set_signal_handlers();

	clear_out_arrays_for_fresh_game();

	timeout_from_item_drop = 0;

	Overall_Average = 0.041;
	SkipAFewFrames = 0;

	ResetGameConfigToDefaultValues();

	input_keyboard_init();
	init_lua();
	init_luaconfig();

	load_fdrpg_config();

	LoadGameConfig();

	if (SDL_Init(SDL_INIT_VIDEO) == -1)
		error_message(__FUNCTION__, "Couldn't initialize SDL: %s", PLEASE_INFORM | IS_FATAL, SDL_GetError());

	// So the video library could be initialized.  So it should also be
	// cleaned up and closed once we're done and quit FreedroidRPG.
	atexit(SDL_Quit);

	// Enable to compute the unicode value of a pressed key.
	// Needed to use the numkeys which are on top of the alphakeys on keyboards.
	SDL_EnableUNICODE(1);

	detect_available_resolutions();

	ParseCommandLine(argc, argv);

	LightRadiusInit();

	init_timer();

	InitVideo();

	// Adapt button positions for the current screen resolution. Note: At this
	// point the video mode was already initialized, therefore we know if OpenGL
	// is used or not and also which screen resolution is used.
	//
	adapt_button_positions_to_screen_resolution();

	Copy_Rect(Full_User_Rect, User_Rect);

	next_startup_percentage(0);

	init_keyboard_input_array();
	init_message_log();

	init_audio();

	Init_Game_Data();

	/* 
	 * Initialize random-number generator in order to make 
	 * level-start etc really different at each program start
	 */
	srand(time(NULL));

	InitPictures(); //requires game data loaded in Init_Game_Data()

	next_startup_percentage(100);
	if (strstr(VERSION, "rc") && !do_benchmark) {
		blit_background("startup1.jpg");
		alert_window(_("You are playing a Release Candidate.\n"
		               "Strange bugs may still be present in the game.\n"
		               "Please report any issues you find to our bugtracker at http://bugs.freedroid.org/\n"
		               "Thank you for helping us test the game.\n\nGood luck!\n"));
	}
}
/**
 * This function displayes the last seconds of the game when the influencer
 * has actually been killed.  It generates some explosions and waits for
 * some seconds, where the user can reload his latest game, or after that
 * returns to finally quit the inner game loop and the program will 
 * (outside this function) ask for a completely new game or loading a different
 * saved game or quit as in the very beginning of the game.
 */
void ThouArtDefeated(void)
{
	int now;

	DebugPrintf(1, "\n%s(): Real function call confirmed.", __FUNCTION__);
	append_new_game_message(_("Game over.\n"));
	SetNewBigScreenMessage(_("Game over.\n"));
	GameConfig.Inventory_Visible = FALSE;
	GameConfig.CharacterScreen_Visible = FALSE;
	ThouArtDefeatedSound();
	start_tux_death_explosions();
	now = SDL_GetTicks();

	GameConfig.SkillScreen_Visible = FALSE;
	GameConfig.CharacterScreen_Visible = FALSE;
	GameConfig.Inventory_Visible = FALSE;

	// Now that the influencer is dead, all this precious items
	// spring off of him...
	//
	if (Me.weapon_item.type > 0) {
		DropItemAt(Me.weapon_item.type, Me.pos.z, Me.pos.x - 0.5, Me.pos.y - 0.5, 1);
	}
	if (Me.drive_item.type > 0) {
		DropItemAt(Me.drive_item.type, Me.pos.z, Me.pos.x + 0.5, Me.pos.y - 0.5, 1);
	}
	if (Me.shield_item.type > 0) {
		DropItemAt(Me.shield_item.type, Me.pos.z, Me.pos.x + 0.5, Me.pos.y + 0.5, 1);
	}
	if (Me.armour_item.type > 0) {
		DropItemAt(Me.armour_item.type, Me.pos.z, Me.pos.x - 0.5, Me.pos.y + 0.5, 1);
	}
	if (Me.special_item.type > 0) {
		DropItemAt(Me.special_item.type, Me.pos.z, Me.pos.x - 0.5, Me.pos.y, 1);
	}
	if (Me.Gold > 0) {
		DropItemAt(get_item_type_by_id("Valuable Circuits"), Me.pos.z, Me.pos.x, Me.pos.y, 1);
	}

	GameOver = TRUE;

	while ((SDL_GetTicks() - now < 1000 * WAIT_AFTER_KILLED)) {
		StartTakingTimeForFPSCalculation();

		AssembleCombatPicture(DO_SCREEN_UPDATE | SHOW_ITEMS);

		UpdateCountersForThisFrame();
		
		DoAllMovementAndAnimations();
		move_enemies();

		ComputeFPSForThisFrame();
	}
	input_handle();
	if (!skip_initial_menus && (game_root_mode == ROOT_IS_GAME))
		PlayATitleFile("GameLost.lua");

	do_death_menu();

};				// void ThouArtDefeated(void)

/**
 * This function displays the last seconds of the game when the influencer
 * has actually won the game.  It anims the game while it's waits for
 * some seconds, after printed a winning message. After that, Credits.title
 * can be displayed, or the game reboots to main.
 */
void ThouHastWon(void)
{
	int now;

	DebugPrintf(1, "\n%s(): Real function call confirmed.", __FUNCTION__);
	append_new_game_message(_("Game won.\n"));
	SetNewBigScreenMessage(_("Game won.\n"));
	GameConfig.Inventory_Visible = FALSE;
	GameConfig.CharacterScreen_Visible = FALSE;
	now = SDL_GetTicks();

	GameOver = TRUE;

	while ((SDL_GetTicks() - now < 1000 * WAIT_AFTER_GAME_WON)) {
		StartTakingTimeForFPSCalculation();

		AssembleCombatPicture(DO_SCREEN_UPDATE | SHOW_ITEMS);

		UpdateCountersForThisFrame();
		
		DoAllMovementAndAnimations();
		move_enemies();

		ComputeFPSForThisFrame();
	}

	// Now it's time for the end game title file...
	//
	//PlayATitleFile("EndOfGame.title");
	if (!skip_initial_menus)
		PlayATitleFile("Credits.lua");

	input_handle();

};				// void ThouHastWon(void)

#undef _init_c
