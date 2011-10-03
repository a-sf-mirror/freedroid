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
 * (Not all of the) Initialization routines for FreeDroid.
 */

#define _init_c

#include "system.h"

#include "defs.h"
#include "struct.h"
#include "global.h"
#include "proto.h"
#include "widgets/widgets.h"

#ifdef __OpenBSD__
#include "ieeefp.h"
#else
#include "fenv.h"
#endif

#include "getopt.h"

void Init_Game_Data(void);
void Get_Bullet_Data(char *DataPointer);
void UpdateCountersForThisFrame();

static struct {
	double maxspeed_calibrator;
	double maxenergy_calibrator;
	float healing_hostile_calibrator;
	float healing_friendly_calibrator;
	double experience_reward_calibrator;
	double aggression_distance_calibrator;
} difficulty_parameters[3];

/**
 * The following function is NOT 'standard' C but rather a GNU extension
 * to the C standards.  We *DON'T* want to use such things, but in this
 * case it helps debugging purposes of floating point operations just so
 * much, that it's really worth using it (in development versions, not in
 * releases).  But to avoid warnings from GCC (which we always set to not
 * allow gnu extensions to the C standard by default), we declare the
 * prototype of this function here.  If you don't use GCC or this 
 * function should give you portability problems, it's ABSOLUTELY SAFE
 * to just remove all instances of it, since it really only helps 
 * debugging.  Proper documentation can be found in the GNU C Library,
 * section about 'Arithmethic', subsection on floating point control
 * functions.
 */
#if ! defined __gnu_linux__
/* turn off these functions where they are not present */
#define feenableexcept(X) {}
#define fedisableexcept(X) {}
#else
extern int feenableexcept(int excepts);
extern int fedisableexcept(int TheExceptionFlags);
#endif

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

	SetCurrentFont(Blue_BFont);
	char percent[10];
	sprintf(percent, "%d%%", startup_percent);
	display_text(percent, UNIVERSAL_COORD_W(310) - 9, UNIVERSAL_COORD_H(301) - 7, NULL);

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
	char fpath[2048];
	char *TitleFilePointer;
	char *NextSubsectionStartPointer;
	char *PreparedBriefingText;
	char *TerminationPointer;
	char *TitleSongName;
	char *background_name;
	int ThisTextLength;
	char finaldir[50];
	while (SpacePressed() || MouseLeftPressed()) ;

	snprintf(finaldir, 50, "%s/", TITLES_DIR);

	// Now its time to start loading the title file...
	//
	if (find_file(Filename, finaldir, fpath, 0))	//if file not found, retry with english version
	{
		snprintf(finaldir, 50, "%s/", TITLES_DIR);
		find_file(Filename, finaldir, fpath, 0);
	}

	TitleFilePointer = ReadAndMallocAndTerminateFile(fpath, "*** END OF TITLE FILE *** LEAVE THIS TERMINATOR IN HERE ***");

	TitleSongName =
	    ReadAndMallocStringFromData(TitleFilePointer, "The title song in the sound subdirectory for this mission is : ", "\n");

	background_name = ReadAndMallocStringFromDataOptional(TitleFilePointer, "Background file: ", "\n");
	if (!background_name)
		background_name = strdup("title.jpg");

	SwitchBackgroundMusicTo(TitleSongName);
	free(TitleSongName);

	SDL_SetClipRect(Screen, NULL);
	SetCurrentFont(Para_BFont);

	NextSubsectionStartPointer = TitleFilePointer;
	while ((NextSubsectionStartPointer = strstr(NextSubsectionStartPointer, "*** START OF PURE SCROLLTEXT DATA ***"))
	       != NULL) {
		NextSubsectionStartPointer += strlen("*** START OF PURE SCROLLTEXT DATA ***\n");
		if ((TerminationPointer = strstr(NextSubsectionStartPointer, "\n*** END OF PURE SCROLLTEXT DATA ***")) == NULL) {
			ErrorMessage(__FUNCTION__, "Unterminated Subsection in Mission briefing....Terminating...\n", PLEASE_INFORM,
				     IS_FATAL);
		}
		ThisTextLength = TerminationPointer - NextSubsectionStartPointer;
		PreparedBriefingText = MyMalloc(ThisTextLength + 10);
		strncpy(PreparedBriefingText, NextSubsectionStartPointer, ThisTextLength);
		PreparedBriefingText[ThisTextLength] = 0;

		ScrollText((PreparedBriefingText), background_name);
		free(PreparedBriefingText);
	}

	clear_screen();
	our_SDL_flip_wrapper();
	free(TitleFilePointer);
	free(background_name);
};				// void PlayATitleFile ( char* Filename )

/*----------------------------------------------------------------------
 * This function reads in all the bullet data from the freedroid.ruleset file,
 * but IT DOES NOT LOAD THE FILE, IT ASSUMES IT IS ALREADY LOADED and
 * it only receives a pointer to the start of the bullet section from
 * the calling function.
 ----------------------------------------------------------------------*/
void Get_Bullet_Data(char *DataPointer)
{
	int i;
	int BulletIndex = 0;

#define NEW_BULLET_TYPE_BEGIN_STRING "** Start of new bullet specification subsection **"
#define NEW_BULLET_TYPE_END_STRING "** End of new bullet specification subsection **"

	DebugPrintf(1, "\n\nStarting to read bullet data...\n\n");
	// At first, we must allocate memory for the droid specifications.
	// How much?  That depends on the number of droids defined in freedroid.ruleset.
	// So we have to count those first.  ok.  lets do it.

	Number_Of_Bullet_Types = CountStringOccurences(DataPointer, NEW_BULLET_TYPE_BEGIN_STRING);

	// Not that we know how many bullets are defined in freedroid.ruleset, we can allocate
	// a fitting amount of memory, but of course only if the memory hasn't been allocated
	// already!!!
	//
	// If we would do that in any case, every Init_Game_Data call would destroy the loaded
	// image files AND MOST LIKELY CAUSE A SEGFAULT!!!
	//
	if (Bulletmap == NULL) {
		i = sizeof(bulletspec);
		Bulletmap = MyMalloc(i * (Number_Of_Bullet_Types + 1) + 1);
		DebugPrintf(1,
			    "\nvoid Get_Bullet_Data( char* DatapPointer ) : We have counted %d different bullet types in the game data file.",
			    Number_Of_Bullet_Types);
		// DebugPrintf ( 0 , "\nMEMORY HAS BEEN ALLOCATED.\nTHE READING CAN BEGIN.\n" );
		// getchar();
	}
	// Now we start to read the values for each bullet type:
	// 
	char *BulletPointer = DataPointer;

	while ((BulletPointer = strstr(BulletPointer, NEW_BULLET_TYPE_BEGIN_STRING)) != NULL) {
		char *EndBulletSection = LocateStringInData(BulletPointer, NEW_BULLET_TYPE_END_STRING);
		DebugPrintf(1, "\n\nFound another Bullet specification entry!  Lets add that to the others!");
		BulletPointer++;	// to avoid doubly taking this entry
		Bulletmap[BulletIndex].name = ReadAndMallocStringFromData(BulletPointer, "Bullet identification: \"", "\"");
		ReadValueFromStringWithDefault(BulletPointer, "Number of phases: ", "%d", "1", &Bulletmap[BulletIndex].phases, EndBulletSection);
		ReadValueFromStringWithDefault(BulletPointer, "Phases per second: ", "%lf", "1", &Bulletmap[BulletIndex].phase_changes_per_second, EndBulletSection);
		Bulletmap[BulletIndex].sound = ReadAndMallocStringFromDataOptional(BulletPointer, "Sound to play: \"", "\"");
		char *blast_name = ReadAndMallocStringFromDataOptional(BulletPointer, "Blast: \"", "\"");
		Bulletmap[BulletIndex].blast_type = get_blast_type_by_name(blast_name);
		free(blast_name);
		BulletIndex++;
	}

	DebugPrintf(1, "\nEnd of Get_Bullet_Data ( char* DataPointer ) reached.");
}

/**
 * This function reads the descriptions of the different programs
 * (source and blobs) that are used for magic
 */
static int Get_Programs_Data(char *DataPointer)
{
	char *ProgramPointer;
	char *EndOfProgramData;

#define PROGRAM_SECTION_BEGIN_STRING "*** Start of program data section: ***"
#define PROGRAM_SECTION_END_STRING "*** End of program data section ***"
#define NEW_PROGRAM_BEGIN_STRING "** Start of new program specification subsection **"

	ProgramPointer = LocateStringInData(DataPointer, PROGRAM_SECTION_BEGIN_STRING);
	EndOfProgramData = LocateStringInData(DataPointer, PROGRAM_SECTION_END_STRING);

	int Number_Of_Programs = CountStringOccurences(DataPointer, NEW_PROGRAM_BEGIN_STRING);
	number_of_skills = Number_Of_Programs;

	SpellSkillMap = (spell_skill_spec *) MyMalloc(sizeof(spell_skill_spec) * (Number_Of_Programs + 1));

	if (Number_Of_Programs >= MAX_NUMBER_OF_PROGRAMS) {
		ErrorMessage(__FUNCTION__, "\
There are more skills defined, than the maximum number specified in the code!", PLEASE_INFORM, IS_FATAL);
	}

	char *whattogrep = NEW_PROGRAM_BEGIN_STRING;

	spell_skill_spec *ProgramToFill = SpellSkillMap;

	while ((ProgramPointer = strstr(ProgramPointer, whattogrep)) != NULL) {
		ProgramPointer++;
		char *EndOfThisProgram = strstr(ProgramPointer, whattogrep);

		if (EndOfThisProgram)
			EndOfThisProgram[0] = 0;

		ProgramToFill->name = ReadAndMallocStringFromData(ProgramPointer, "Program name=_\"", "\"");
		ProgramToFill->description = ReadAndMallocStringFromData(ProgramPointer, "Program description=_\"", "\"");
		ProgramToFill->icon_name = ReadAndMallocStringFromData(ProgramPointer, "Picture=\"", "\"");

		struct image empty = EMPTY_IMAGE;
		ProgramToFill->icon_surface = empty;

		ProgramToFill->effect = ReadAndMallocStringFromData(ProgramPointer, "Effect=\"", "\"");

		char *pform = ReadAndMallocStringFromData(ProgramPointer, "Form=\"", "\"");
		if (!strcmp(pform, "instant"))
			ProgramToFill->form = PROGRAM_FORM_INSTANT;
		if (!strcmp(pform, "bullet"))
			ProgramToFill->form = PROGRAM_FORM_BULLET;
		if (!strcmp(pform, "radial"))
			ProgramToFill->form = PROGRAM_FORM_RADIAL;
		if (!strcmp(pform, "self"))
			ProgramToFill->form = PROGRAM_FORM_SELF;

		free(pform);

		ReadValueFromStringWithDefault(ProgramPointer, "Base damage=", "%hd", "0", &ProgramToFill->damage_base, EndOfProgramData);
		ReadValueFromStringWithDefault(ProgramPointer, "Mod damage=", "%hd", "0", &ProgramToFill->damage_mod, EndOfProgramData);
		ReadValueFromStringWithDefault(ProgramPointer, "Damage per level=", "%hd", "0",
					       &ProgramToFill->damage_per_level, EndOfProgramData);

		ReadValueFromStringWithDefault(ProgramPointer, "Affect bots=", "%hd", "1", &ProgramToFill->hurt_bots, EndOfProgramData);
		ReadValueFromStringWithDefault(ProgramPointer, "Affect humans=", "%hd", "1", &ProgramToFill->hurt_humans, EndOfProgramData);

		ReadValueFromStringWithDefault(ProgramPointer, "Cost=", "%hd", "0", &ProgramToFill->heat_cost, EndOfProgramData);
		ReadValueFromStringWithDefault(ProgramPointer, "Cost per level=", "%hd", "0",
					       &ProgramToFill->heat_cost_per_level, EndOfProgramData);
		ReadValueFromStringWithDefault(ProgramPointer, "Present at startup=", "%hd", "0",
					       &ProgramToFill->present_at_startup, EndOfProgramData);
		ReadValueFromStringWithDefault(ProgramPointer, "Artwork internal code=", "%d", "-1",
					       &ProgramToFill->graphics_code, EndOfProgramData);
		ReadValueFromStringWithDefault(ProgramPointer, "Effect duration=", "%f", "0",
					       &ProgramToFill->effect_duration, EndOfProgramData);
		ReadValueFromStringWithDefault(ProgramPointer, "Effect duration per level=", "%f", "0",
					       &ProgramToFill->effect_duration_per_level, EndOfProgramData);

		//ReadValueFromStringWithDefault( ProgramPointer , "Bonus to tohit modifier=" , "%d" , "0",

		ProgramToFill++;
		if (EndOfThisProgram)
			EndOfThisProgram[0] = '*';	// We put back the star at its place 
	}

	return 0;
}

static void Get_Difficulty_Parameters(void *DataPointer)
{
	char *EndOfDataPointer;
	char *startptr;
	char tmp[256];
	const char *diffstr[] = { "easy", "normal", "hard" };
	int i;

#define MAXSPEED_CALIBRATOR_STRING "Droid maxspeed factor in %s: "
#define MAXENERGY_CALIBRATOR_STRING "Droid maximum energy factor in %s: "
#define HEALING_HOSTILE_CALIBRATOR_STRING "Hostile healing speed factor in %s: "
#define HEALING_FRIENDLY_CALIBRATOR_STRING "Friendly healing speed factor in %s: "
#define EXPERIENCE_REWARD_CALIBRATOR_STRING "Droid experience_reward factor in %s: "
#define AGGRESSION_DISTANCE_CALIBRATOR_STRING "Droid aggression distance factor in %s: "

	startptr = LocateStringInData(DataPointer, "*** Start of difficulty parameters ***");
	EndOfDataPointer = LocateStringInData(DataPointer, "*** End of difficulty parameters ***");

	for (i = 0; i < 3; i++) {

		sprintf(tmp, MAXSPEED_CALIBRATOR_STRING, diffstr[i]);
		ReadValueFromString(startptr, tmp, "%lf", &difficulty_parameters[i].maxspeed_calibrator, EndOfDataPointer);

		sprintf(tmp, MAXENERGY_CALIBRATOR_STRING, diffstr[i]);
		ReadValueFromString(startptr, tmp, "%lf", &difficulty_parameters[i].maxenergy_calibrator, EndOfDataPointer);

		sprintf(tmp, HEALING_HOSTILE_CALIBRATOR_STRING, diffstr[i]);
		ReadValueFromString(startptr, tmp, "%f", &difficulty_parameters[i].healing_hostile_calibrator, EndOfDataPointer);

		sprintf(tmp, HEALING_FRIENDLY_CALIBRATOR_STRING, diffstr[i]);
		ReadValueFromString(startptr, tmp, "%f", &difficulty_parameters[i].healing_friendly_calibrator, EndOfDataPointer);

		sprintf(tmp, EXPERIENCE_REWARD_CALIBRATOR_STRING, diffstr[i]);
		ReadValueFromString(startptr, tmp, "%lf", &difficulty_parameters[i].experience_reward_calibrator, EndOfDataPointer);

		sprintf(tmp, AGGRESSION_DISTANCE_CALIBRATOR_STRING, diffstr[i]);
		ReadValueFromString(startptr, tmp, "%lf", &difficulty_parameters[i].aggression_distance_calibrator, EndOfDataPointer);
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
#define EXPERIENCE_REWARD_BEGIN_STRING "Experience_Reward gained for destroying one of this type: "
#define WEAPON_ITEM_BEGIN_STRING "Weapon item=\""
#define GREETING_SOUND_STRING "Greeting Sound number="
#define ENEMY_GOT_HIT_SOUND_STRING "Got Hit Sound number="
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
		ErrorMessage(__FUNCTION__, "\
The value of %d for \"NB_DROID_TYPES\" defined in struct.h is less than %d\n\
which is \"Number_of_Droid_Types\" + 2. Please increase the value of \"NB_DROID_TYPES\"!",
			PLEASE_INFORM, IS_FATAL, NB_DROID_TYPES, Number_Of_Droid_Types + 2);
	}

	// Not that we know how many robots are defined in freedroid.ruleset, we can allocate
	// a fitting amount of memory.
	i = sizeof(druidspec);
	Druidmap = MyMalloc(i * (Number_Of_Droid_Types + 1) + 1);
	DebugPrintf(1, "\nWe have counted %d different druid types in the game data file.", Number_Of_Droid_Types);
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
		Druidmap[RobotIndex].druidname = ReadAndMallocStringFromData(RobotPointer, DROIDNAME_BEGIN_STRING, "\n");

		// Now we read in the default short_description_text for this droid.
		Druidmap[RobotIndex].default_short_description = ReadAndMallocStringFromDataOptional(RobotPointer, "Default description:_\"", "\"");
		if (!Druidmap[RobotIndex].default_short_description)
			Druidmap[RobotIndex].default_short_description = strdup("");

		// Now we read in the prefix of the file names in the rotation series
		// to use for the console droid rotation
		Druidmap[RobotIndex].droid_portrait_rotation_series_prefix =
		    ReadAndMallocStringFromData(RobotPointer, DROID_PORTRAIT_ROTATION_SERIES_NAME_PREFIX, "\"");

		// Now we read in the file name of the death sound for this droid.  
		// Is should be enclosed in double-quotes.
		//
		Druidmap[RobotIndex].droid_death_sound_file_name =
		    ReadAndMallocStringFromData(RobotPointer, DROID_DEATH_SOUND_FILE_NAME, "\"");

		// Now we read in the file name of the attack animation sound for this droid.  
		// Is should be enclosed in double-quotes.
		//
		Druidmap[RobotIndex].droid_attack_animation_sound_file_name =
		    ReadAndMallocStringFromData(RobotPointer, DROID_ATTACK_ANIMATION_SOUND_FILE_NAME, "\"");

		// Now we read in the maximal speed this droid can go. 
		ReadValueFromString(RobotPointer, MAXSPEED_BEGIN_STRING, "%f", &Druidmap[RobotIndex].maxspeed, EndOfDataPointer);

		// Now we read in the class of this droid.
		ReadValueFromString(RobotPointer, CLASS_BEGIN_STRING, "%d", &Druidmap[RobotIndex].class, EndOfDataPointer);

		// Now we read in the maximal energy this droid can store. 
		ReadValueFromString(RobotPointer, MAXENERGY_BEGIN_STRING, "%f", &Druidmap[RobotIndex].maxenergy, EndOfDataPointer);

		// Now we read in the lose_health rate.
		ReadValueFromString(RobotPointer, BASE_HEALING_BEGIN_STRING, "%f", &Druidmap[RobotIndex].healing_friendly, EndOfDataPointer);
		Druidmap[RobotIndex].healing_hostile = Druidmap[RobotIndex].healing_friendly;

		// Now we read in range of vision of this droid
		ReadValueFromString(RobotPointer, "Aggression distance of this droid=", "%f",
				    &Druidmap[RobotIndex].aggression_distance, EndOfDataPointer);

		// Now we read in range of vision of this droid
		ReadValueFromString(RobotPointer, "Time spent eyeing Tux=", "%f",
				    &Druidmap[RobotIndex].time_spent_eyeing_tux, EndOfDataPointer);

		// Now we experience_reward to be had for destroying one droid of this type
		ReadValueFromString(RobotPointer, EXPERIENCE_REWARD_BEGIN_STRING, "%hd",
				    &Druidmap[RobotIndex].experience_reward, EndOfDataPointer);

		// Now we read in the monster level = maximum treasure chest to pick from
		ReadValueFromString(RobotPointer, "Drops item class=", "%hd", &Druidmap[RobotIndex].drop_class, EndOfDataPointer);

		char *tmp_item_name = ReadAndMallocStringFromData(RobotPointer, WEAPON_ITEM_BEGIN_STRING, "\"");
		Druidmap[RobotIndex].weapon_item.type = GetItemIndexByName(tmp_item_name);
		free(tmp_item_name);

		// Now we read in the % chance for droid to drop botpart
		ReadValueFromStringWithDefault(RobotPointer, "Percent to drop Entropy Inverter=", "%hd", "0",
					       &Druidmap[RobotIndex].amount_of_entropy_inverters, EndOfDataPointer);
		ReadValueFromStringWithDefault(RobotPointer, "Percent to drop Plasma Transistor=", "%hd", "0",
					       &Druidmap[RobotIndex].amount_of_plasma_transistors, EndOfDataPointer);
		ReadValueFromStringWithDefault(RobotPointer, "Percent to drop Superconducting Relay Unit=", "%hd", "0",
					       &Druidmap[RobotIndex].amount_of_superconductors, EndOfDataPointer);
		ReadValueFromStringWithDefault(RobotPointer, "Percent to drop Antimatter-Matter Converter=", "%hd", "0",
					       &Druidmap[RobotIndex].amount_of_antimatter_converters, EndOfDataPointer);
		ReadValueFromStringWithDefault(RobotPointer, "Percent to drop Tachyon Condensator=", "%hd", "0",
					       &Druidmap[RobotIndex].amount_of_tachyon_condensators, EndOfDataPointer);

		// Now we read in the greeting sound type of this droid type
		ReadValueFromString(RobotPointer, GREETING_SOUND_STRING, "%hd",
				    &Druidmap[RobotIndex].greeting_sound_type, EndOfDataPointer);

		// Now we read in the greeting sound type of this droid type
		ReadValueFromString(RobotPointer, ENEMY_GOT_HIT_SOUND_STRING, "%hd",
				    &Druidmap[RobotIndex].got_hit_sound_type, EndOfDataPointer);

		// Now we read in the to-hit chance this robot has in combat against an unarmored target
		ReadValueFromString(RobotPointer, TO_HIT_STRING, "%hd", &Druidmap[RobotIndex].to_hit, EndOfDataPointer);

		// Now we read in the modifier, that increases/decreases the chance of this robot getting hit
		ReadValueFromString(RobotPointer, "Time to recover after getting hit=", "%f",
				    &Druidmap[RobotIndex].recover_time_after_getting_hit, EndOfDataPointer);

		// Now we read in the is_human flag of this droid type
		ReadValueFromString(RobotPointer, IS_HUMAN_SPECIFICATION_STRING, "%hd", &Druidmap[RobotIndex].is_human, EndOfDataPointer);

		// Now we read in the Graphics to associate with this droid type
		char *enemy_surface = ReadAndMallocStringFromData(RobotPointer, "Filename prefix for graphics=\"", "\"");
		Druidmap[RobotIndex].individual_shape_nr = 0;
		for (i=0; i < ENEMY_ROTATION_MODELS_AVAILABLE; i++) {
			if (PrefixToFilename[i] && !strcmp(enemy_surface, PrefixToFilename[i])){
				Druidmap[RobotIndex].individual_shape_nr = i;
				break;
			}
		}
		free(enemy_surface);

		// Now we read in the notes about this droid type
		Druidmap[RobotIndex].notes = ReadAndMallocStringFromData(RobotPointer, NOTES_BEGIN_STRING, "\"");

		// Now we're potentially ready to process the next droid.  Therefore we proceed to
		// the next number in the Droidmap array.
		RobotIndex++;
		if (EndOfThisRobot)
			EndOfThisRobot[0] = '*';	// We put back the star at its place
	}

	for (i = 0; i < Number_Of_Droid_Types; i++) {
		Druidmap[i].maxspeed *= difficulty_parameters[GameConfig.difficulty_level].maxspeed_calibrator;
		Druidmap[i].maxenergy *= difficulty_parameters[GameConfig.difficulty_level].maxenergy_calibrator;
		Druidmap[i].experience_reward *= difficulty_parameters[GameConfig.difficulty_level].experience_reward_calibrator;
		Druidmap[i].aggression_distance *= difficulty_parameters[GameConfig.difficulty_level].aggression_distance_calibrator;
		Druidmap[i].healing_friendly *= difficulty_parameters[GameConfig.difficulty_level].healing_friendly_calibrator;
		Druidmap[i].healing_hostile *= difficulty_parameters[GameConfig.difficulty_level].healing_hostile_calibrator;
	}
};				// int Get_Robot_Data ( void )

/**
 * This function reads in all the item data from the item_archetypes.dat file,
 * but IT DOES NOT LOAD THE FILE, IT ASSUMES IT IS ALREADY LOADED and
 * it only receives a pointer to the start of the bullet section from
 * the calling function.
 */
static void get_item_data(char *DataPointer)
{
	char *ItemPointer;
	char *EndOfItemData;
	int ItemIndex = 0;
	char *YesNoString;
	float ranged_weapon_damage_calibrator;
	float melee_weapon_damage_calibrator;
	float ranged_weapon_speed_calibrator;

#define ITEM_SECTION_BEGIN_STRING "*** Start of item data section: ***"
#define ITEM_SECTION_END_STRING "*** End of item data section: ***"
#define NEW_ITEM_TYPE_BEGIN_STRING "** Start of new item specification subsection **"

#define ITEM_NAME_INDICATION_STRING "Item name=_\""
#define ITEM_DESCRIPTION_INDICATION_STRING "Item description text=_\""

#define ITEM_CAN_BE_INSTALLED_IN_SLOT_WITH_NAME "Item can be installed in slot with name=\""
#define ITEM_TUX_PART_INSTANCE "Tux part instance=\""
#define ITEM_ROTATION_SERIES_NAME_PREFIX "Item uses rotation series with prefix=\""
#define ITEM_GROUP_TOGETHER_IN_INVENTORY "Items of this type collect together in inventory=\""

#define ITEM_INVENTORY_IMAGE_FILE_NAME "File or directory name for inventory image=\""
#define ITEM_DROP_SOUND_FILE_NAME "Item uses drop sound with filename=\""

#define ITEM_RECHARGE_TIME_BEGIN_STRING "Time is takes to recharge this bullet/weapon in seconds :"

	ItemPointer = LocateStringInData(DataPointer, ITEM_SECTION_BEGIN_STRING);
	EndOfItemData = LocateStringInData(DataPointer, ITEM_SECTION_END_STRING);

	Number_Of_Item_Types = CountStringOccurences(DataPointer, NEW_ITEM_TYPE_BEGIN_STRING);

	// Now that we know how many item archetypes there are, we can allocate the proper
	// amount of memory for this information.
	ItemMap = (itemspec *) MyMalloc(sizeof(itemspec) * (Number_Of_Item_Types + 1));

	// Now we start to read the values for each bullet type:
	ItemPointer = DataPointer;

	// Now we read in the speed calibration factor for all bullets
	ReadValueFromString(DataPointer, "Common factor for all ranged weapons bullet speed values:", "%f",
			    &ranged_weapon_speed_calibrator, EndOfItemData);

	// Now we read in the damage calibration factor for all bullets
	ReadValueFromString(DataPointer, "Common factor for all ranged weapons bullet damage values:", "%f",
			    &ranged_weapon_damage_calibrator, EndOfItemData);

	// Now we read in the damage calibration factor for all bullets
	ReadValueFromString(DataPointer, "Common factor for all melee weapons damage values:", "%f",
			    &melee_weapon_damage_calibrator, EndOfItemData);

	while ((ItemPointer = strstr(ItemPointer, NEW_ITEM_TYPE_BEGIN_STRING)) != NULL) {
		ItemPointer++;
		char *EndOfThisItem = strstr(ItemPointer, NEW_ITEM_TYPE_BEGIN_STRING);
		if (EndOfThisItem)
			EndOfThisItem[0] = 0;

		itemspec *item = &ItemMap[ItemIndex];

		// Now we read in the name of this item
		item->item_name = ReadAndMallocStringFromData(ItemPointer, ITEM_NAME_INDICATION_STRING, "\"");

		// Now we read in the description string of this item
		item->item_description = ReadAndMallocStringFromData(ItemPointer, ITEM_DESCRIPTION_INDICATION_STRING, "\"");

		// Now we read in if what this item does if it can be used by the influ without help
		item->item_combat_use_description = ReadAndMallocStringFromDataOptional(ItemPointer, "Item use when applied in combat=_\"", "\"");

		// Now we read the label telling us in which slot the item can be installed
		YesNoString = ReadAndMallocStringFromData(ItemPointer, ITEM_CAN_BE_INSTALLED_IN_SLOT_WITH_NAME, "\"");
		item->item_can_be_installed_in_weapon_slot = FALSE;
		item->item_can_be_installed_in_shield_slot = FALSE;
		item->item_can_be_installed_in_drive_slot = FALSE;
		item->item_can_be_installed_in_armour_slot = FALSE;
		item->item_can_be_installed_in_special_slot = FALSE;
		if (strcmp(YesNoString, "weapon") == 0) {
			item->item_can_be_installed_in_weapon_slot = TRUE;
		} else if (strcmp(YesNoString, "drive") == 0) {
			item->item_can_be_installed_in_drive_slot = TRUE;
		} else if (strcmp(YesNoString, "shield") == 0) {
			item->item_can_be_installed_in_shield_slot = TRUE;
		} else if (strcmp(YesNoString, "armour") == 0) {
			item->item_can_be_installed_in_armour_slot = TRUE;
		} else if (strcmp(YesNoString, "special") == 0) {
			item->item_can_be_installed_in_special_slot = TRUE;
		} else if (strcmp(YesNoString, "none") == 0) {
			// good.  Everything is ok, as long as at least 'none' was found
		} else {
			fprintf(stderr, "\n\nItemIndex: %d.\n", ItemIndex);
			ErrorMessage(__FUNCTION__,
				     "The item specification of an item in freedroid.ruleset should contain an \nanswer for the slot installation possibilities, that was neither \n'weapon' nor 'armour' nor 'shield' nor 'special' nor 'drive' nor 'none'.",
				     PLEASE_INFORM, IS_FATAL);
		}
		free(YesNoString);

		// Filename prefix used when rendering Tux (optional)
		item->tux_part_instance = NULL;
		YesNoString = ReadAndMallocStringFromDataOptional(ItemPointer, ITEM_TUX_PART_INSTANCE, "\"");
		if (YesNoString && strlen(YesNoString))
			item->tux_part_instance = YesNoString;

		// Next we read in the prefix for the image series in the items browser
		item->item_rotation_series_prefix =
		    ReadAndMallocStringFromData(ItemPointer, ITEM_ROTATION_SERIES_NAME_PREFIX, "\"");

		// Now we read in if this item will group together in inventory
		YesNoString = ReadAndMallocStringFromData(ItemPointer, ITEM_GROUP_TOGETHER_IN_INVENTORY, "\"");
		if (strcmp(YesNoString, "yes") == 0) {
			item->item_group_together_in_inventory = TRUE;
		} else if (strcmp(YesNoString, "no") == 0) {
			item->item_group_together_in_inventory = FALSE;
		} else {
			ErrorMessage(__FUNCTION__, "\
The item specification of an item in freedroid.ruleset should contain an \n\
answer that is either 'yes' or 'no', but which was neither 'yes' nor 'no'.", PLEASE_INFORM, IS_FATAL);
		}
		free(YesNoString);

		// Now we read in minimum strength, dex and cooling required to wear/wield this item
		ReadValueFromStringWithDefault(ItemPointer, "Strength minimum required to wear/wield this item=", "%hd", "-1",
					       &item->item_require_strength, EndOfItemData);
		ReadValueFromStringWithDefault(ItemPointer, "Dexterity minimum required to wear/wield this item=", "%hd", "-1",
					       &item->item_require_dexterity, EndOfItemData);
		ReadValueFromStringWithDefault(ItemPointer, "Magic minimum required to wear/wield this item=", "%hd", "-1",
					       &item->item_require_cooling, EndOfItemData);

		ReadValueFromStringWithDefault(ItemPointer, "Item min class for drop=", "%hd", "-1", &item->min_drop_class,
					       EndOfItemData);
		ReadValueFromStringWithDefault(ItemPointer, "Item max class for drop=", "%hd", "-1", &item->max_drop_class,
					       EndOfItemData);

		if (item->min_drop_class != -1) {
			int cc;
			for (cc = 0; cc < 10; cc++) {
				if (cc > item->max_drop_class)
					break;
				if (cc < item->min_drop_class)
					continue;
				else
					item_count_per_class[cc]++;
			}

			// Now we read in the min and max number of items to drop at a time
			if (ReadRangeFromString(ItemPointer, "Item number dropped=", "\n", &item->drop_amount, &item->drop_amount_max, 1) > 1) {
				ErrorMessage(__FUNCTION__, "\
The \"%s\" item (index number %d) has a garbled multiplicity\n\
of random items dropped.", PLEASE_INFORM, IS_WARNING_ONLY, item->item_name, ItemIndex);
			}
		}

		// If the item is a gun, we read in the weapon specification...
		if (item->item_can_be_installed_in_weapon_slot == TRUE) {
			// Now we read in the damage bullets from this gun will do
			ReadValueFromStringWithDefault(ItemPointer, "Item as gun: damage of bullets=", "%hd", "0",
						       &item->base_item_gun_damage, EndOfItemData);
			ReadValueFromStringWithDefault(ItemPointer, "Item as gun: modifier for damage of bullets=", "%hd", "0",
						       &item->item_gun_damage_modifier, EndOfItemData);

			// Now we read in the speed this bullet will go
			ReadValueFromStringWithDefault(ItemPointer, "Item as gun: speed of bullets=", "%f", "0.000000",
						       &item->item_gun_speed, EndOfItemData);

			// Now we read in speed of melee application and melee offset from influ
			ReadValueFromStringWithDefault(ItemPointer, "Item as gun: is melee weapon=", "%hd", "0",
						       &item->item_weapon_is_melee, EndOfItemData);
			ReadValueFromStringWithDefault(ItemPointer, "Item as gun: modifier for starting angle=", "%f", "0.000000",
						       &item->item_gun_start_angle_modifier, EndOfItemData);

			// Now we read in if this weapons bullets will reflect other bullets or not
			YesNoString = ReadAndMallocStringFromData(ItemPointer, "Item as gun: pass through hit bodies=\"", "\"");
			if (strcmp(YesNoString, "yes") == 0) {
				item->item_gun_bullet_pass_through_hit_bodies = TRUE;
			} else if (strcmp(YesNoString, "no") == 0) {
				item->item_gun_bullet_pass_through_hit_bodies = FALSE;
			} else {
				ErrorMessage(__FUNCTION__, "\
The item specification of an item in freedroid.ruleset should contain an \n\
answer that is either 'yes' or 'no', but which was neither 'yes' nor 'no'.", PLEASE_INFORM, IS_FATAL);
			};	// if ( item->item_can_be_installed_in_weapon_slot == TRUE )
			free(YesNoString);

			// Now we read in the recharging time this weapon will need
			ReadValueFromString(ItemPointer, "Item as gun: recharging time=", "%f",
					    &item->item_gun_recharging_time, EndOfItemData);

			// Now we read in the reloading time this weapon will need
			ReadValueFromString(ItemPointer, "Item as gun: reloading time=", "%f",
					    &item->item_gun_reloading_time, EndOfItemData);

			// Now we read in the image type that should be generated for this bullet
			char *bullet_type = ReadAndMallocStringFromData(ItemPointer, "Item as gun: bullet_image_type=\"", "\"");
			item->item_gun_bullet_image_type = GetBulletByName(bullet_type);
			free(bullet_type);

			// Now we read in the image type that should be generated for this bullet
			ReadValueFromString(ItemPointer, "Item as gun: bullet_lifetime=", "%f",
					    &item->item_gun_bullet_lifetime, EndOfItemData);
			if (item->item_gun_bullet_lifetime == -1)
				item->item_gun_bullet_lifetime = DEFAULT_BULLET_LIFETIME;

			// Now we read in the image type that should be generated for this bullet
			ReadValueFromString(ItemPointer, "Item as gun: ammo clip size=", "%hd",
					    &item->item_gun_ammo_clip_size, EndOfItemData);

			// Some guns require some ammunition.  This will be read in and
			// examined next...
			YesNoString = ReadAndMallocStringFromData(ItemPointer, "Item as gun: required ammunition type=\"", "\"");
			if (strcmp(YesNoString, "none") == 0) {
				item->item_gun_use_ammunition = 0;
			} else if (strcmp(YesNoString, "plasma_ammunition") == 0) {
				item->item_gun_use_ammunition = 2;
			} else if (strcmp(YesNoString, "laser_ammunition") == 0) {
				item->item_gun_use_ammunition = 1;
			} else if (strcmp(YesNoString, "exterminator_ammunition") == 0) {
				item->item_gun_use_ammunition = 3;
			} else if (strcmp(YesNoString, "22LR") == 0) {
				item->item_gun_use_ammunition = 4;
			} else if (strcmp(YesNoString, "Sshell") == 0) {
				item->item_gun_use_ammunition = 5;
			} else if (strcmp(YesNoString, "9mm") == 0) {
				item->item_gun_use_ammunition = 6;
			} else if (strcmp(YesNoString, "7.62mm") == 0) {
				item->item_gun_use_ammunition = 7;
			} else if (strcmp(YesNoString, "50BMG") == 0) {
				item->item_gun_use_ammunition = 8;
			} else {
				ErrorMessage(__FUNCTION__, "\
The type of ammunition used by an item in item_archetypes.dat was not recognized. \n\
This string was: %s\n", PLEASE_INFORM, IS_FATAL, YesNoString);
			}
			free(YesNoString);

			// Now we read in if this weapons (strictly) requires both hands for usage
			YesNoString = ReadAndMallocStringFromData(ItemPointer, "Item as gun: weapon requires both hands=\"", "\"");
			if (strcmp(YesNoString, "yes") == 0) {
				item->item_gun_requires_both_hands = TRUE;
			} else if (strcmp(YesNoString, "no") == 0) {
				item->item_gun_requires_both_hands = FALSE;
			} else {
				ErrorMessage(__FUNCTION__, "\
The item specification of an item in item_archetypes.dat should contain an \n\
answer that is either 'yes' or 'no', but which was neither 'yes' nor 'no'.", PLEASE_INFORM, IS_FATAL);
			};
			free(YesNoString);

			// Read the motion_class to use to render Tux when this weapon is used
			YesNoString = ReadAndMallocStringFromData(ItemPointer, "Item motion class=\"", "\"");
			item->motion_class = get_motion_class_id_by_name(YesNoString);
			if (item->motion_class == -1) {
				ErrorMessage(__FUNCTION__,
						"The motion_class of an item in item_archetypes.dat is unknown: %s",
						PLEASE_INFORM, IS_FATAL, YesNoString);
			};
			free(YesNoString);

		} else {
			// If it is not a gun, we set the weapon specifications to
			// empty values...
			item->base_item_gun_damage = 0;
			item->item_gun_damage_modifier = 0;
			item->item_gun_speed = 0;
			item->item_gun_start_angle_modifier = 0;
			item->item_gun_bullet_pass_through_hit_bodies = FALSE;
			item->item_gun_recharging_time = 0;
			item->item_gun_reloading_time = 0;
			item->item_gun_bullet_image_type = 0;
			item->item_gun_bullet_lifetime = 0;
			item->item_gun_use_ammunition = 0;
			item->item_gun_requires_both_hands = TRUE;
			item->motion_class = -1;
		}

		// Now we read in the armour value of this item as armour or shield or whatever
		ReadValueFromStringWithDefault(ItemPointer, "Item as defensive item: base_armor_class=", "%hd", "0",
					       &item->base_armor_class, EndOfItemData);
		ReadValueFromStringWithDefault(ItemPointer, "Item as defensive item: armor_class_modifier=", "%hd", "0",
					       &item->armor_class_modifier, EndOfItemData);

		// Now we read in the base item duration and the duration modifier
		ReadValueFromStringWithDefault(ItemPointer, "Base item duration=", "%hd", "-1",
					       &item->base_item_durability, EndOfItemData);
		ReadValueFromStringWithDefault(ItemPointer, "plus duration modifier=", "%hd", "0",
					       &item->item_durability_modifier, EndOfItemData);

		// Now we read in the name of the inventory item image, that is to be used
		// on the inventory screen.
		item->item_inv_file_name = ReadAndMallocStringFromData(ItemPointer, ITEM_INVENTORY_IMAGE_FILE_NAME, "\"");

		// Now we read in the name of the sound sample to be played when this item is moved
		item->item_drop_sound_file_name = ReadAndMallocStringFromData(ItemPointer, ITEM_DROP_SOUND_FILE_NAME, "\"");

		// Now we read the size of the item in the inventory.
		ReadValueFromStringWithDefault(ItemPointer, "inventory_size_x=", "%d", "0",
					       &item->inv_size.x, EndOfItemData);
		ReadValueFromStringWithDefault(ItemPointer, "inventory_size_y=", "%d", "0",
					       &item->inv_size.y, EndOfItemData);

		if (item->inv_size.x == 0 || item->inv_size.y == 0) {
			ErrorMessage(__FUNCTION__, "Invalid inventory size for item '%s' (%d %d).", PLEASE_INFORM, IS_FATAL, item->item_name, item->inv_size.x, item->inv_size.y);
		}

		// Now we read in the base list price for this item
		ReadValueFromString(ItemPointer, "Base list price=", "%hd", &item->base_list_price, EndOfItemData);

		ItemIndex++;
		if (EndOfThisItem)
			EndOfThisItem[0] = '*';	// We put back the star at its place    
	}

	// Now that all the calibrations factors have been read in, we can start to
	// apply them to all the bullet types
	for (ItemIndex = 0; ItemIndex < Number_Of_Item_Types; ItemIndex++) {
		itemspec *item = &ItemMap[ItemIndex];
		if (item->item_weapon_is_melee) {
			item->base_item_gun_damage *= melee_weapon_damage_calibrator;
			item->item_gun_damage_modifier *= melee_weapon_damage_calibrator;
		} else {
			item->item_gun_speed *= ranged_weapon_speed_calibrator;
			item->base_item_gun_damage *= ranged_weapon_damage_calibrator;
			item->item_gun_damage_modifier *= ranged_weapon_damage_calibrator;
		}
	}
}

/**
 * This function loads all the constant variables of the game from
 * a data file, using mainly subroutines which do the main work.
 */
void Init_Game_Data()
{
	char fpath[2048];
	char *Data;

#define INIT_GAME_DATA_DEBUG 1

	// Load programs (spells) information
	//
	find_file("program_archetypes.dat", MAP_DIR, fpath, 0);
	DebugPrintf(INIT_GAME_DATA_DEBUG, "\nvoid Init_Game_Data:  Data will be taken from file : %s. Commencing... \n", fpath);
	Data = ReadAndMallocAndTerminateFile(fpath, "*** End of this Freedroid data File ***");
	Get_Programs_Data(Data);
	free(Data);

	// Load the blast data (required for the bullets to load)
	find_file("blast_specs.lua", MAP_DIR, fpath, 0);
	run_lua_file(LUA_CONFIG, fpath);

	// Load the bullet data (required for the item archtypes to load)
	//
	find_file("bullet_archetypes.dat", MAP_DIR, fpath, 0);
	Data = ReadAndMallocAndTerminateFile(fpath, "*** End of this Freedroid data File ***");
	Get_Bullet_Data(Data);
	free(Data);

	// Load Tux animation and rendering specifications.
	tux_rendering_load_specs("tuxrender_specs.lua");

	// Load Bot animation definitions and animations
	// must be before Get_Robot_Data()
	Load_Enemy_Surfaces();

	// Item archetypes must be loaded too
	find_file("item_archetypes.dat", MAP_DIR, fpath, 0);
	Data = ReadAndMallocAndTerminateFile(fpath, "*** End of this Freedroid data File ***");
	get_item_data(Data);
	free(Data);

	// Load add-on specifications.
	find_file("addon_specs.lua", MAP_DIR, fpath, 1);
	run_lua_file(LUA_CONFIG, fpath);


	find_file("difficulty_params.dat", MAP_DIR, fpath, 0);
	Data = ReadAndMallocAndTerminateFile(fpath, "*** End of this Freedroid data File ***");
	Get_Difficulty_Parameters(Data);
	free(Data);

	// Time to eat some droid archetypes...
	//
	find_file("droid_archetypes.dat", MAP_DIR, fpath, 0);
	DebugPrintf(INIT_GAME_DATA_DEBUG, "\nvoid Init_Game_Data:  Data will be taken from file : %s. Commencing... \n", fpath);
	Data = ReadAndMallocAndTerminateFile(fpath, "*** End of this Freedroid data File ***");
	Get_Robot_Data(Data);
	free(Data);

	// Load obstacle specifications.
	dynarray_init(&obstacle_map, 512, sizeof(struct obstacle_spec));
	find_file("obstacle_specs.lua", MAP_DIR, fpath, 0);
	run_lua_file(LUA_CONFIG, fpath);

	// Load floor tile specifications.
	find_file("floor_tiles.lua", MAP_DIR, fpath, 0);
	run_lua_file(LUA_CONFIG, fpath);

	next_startup_percentage(1);
}

char copyright[] = "\nFreedroidRPG comes with NO WARRANTY to the extent permitted by law.\n\
You may redistribute copies of FreedroidRPG\n\
under the terms of the GNU General Public License.\n\
For more information about these matters, see the file named COPYING.\n";

char usage_string[] = "\
Usage: freedroidRPG [-h | --help] \n\
                    [-v | --version] \n\
                    [-e | --editor] \n\
                    [-s | --sound] [-q | --nosound] \n\
                    [-o | --open_gl] [-n | --no_open_gl] \n\
                    [-f | --fullscreen] [-w | --window] \n\
                    [-d X | --debug=X]       X = 0-5; default 1 \n\
                    [-l character-name | --load=character-name] \n\
                    [-r Y | --resolution=Y]  Y = 99 lists hardcoded resolutions. \n\
                          Y may also be of the form 'WxH' e.g. '800x600'\n\
\n\
Please report bugs either by entering them into the bug tracker\n\
on our website at:\n\n\
http://bugs.freedroid.org\n\n\
or by sending an e-mail to:\n\n\
freedroid-discussion AT lists.sourceforge.net\n\n\
For more information and known issues please see README.\n\
Thanks a lot in advance.\n\
                          / The FreedroidRPG dev team.\n\n";

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
		{"version", 0, 0, 'v'},
		{"help", 0, 0, 'h'},
		{"editor", 0, 0, 'e'},
		{"load", 1, 0, 'l'},
		{"open_gl", 0, 0, 'o'},
		{"no_open_gl", 0, 0, 'n'},
		{"nosound", 0, 0, 'q'},
		{"sound", 0, 0, 's'},
		{"debug", 1, 0, 'd'},
		{"window", 0, 0, 'w'},
		{"fullscreen", 0, 0, 'f'},
		{"resolution", 1, 0, 'r'},
		{"benchmark", 1, 0, 'b'},
		{0, 0, 0, 0}
	};

	command_line_override_for_screen_resolution = FALSE;

	while (1) {
		c = getopt_long(argc, argv, "vel:onqsb:h?d::r:wf", long_options, NULL);
		if (c == -1)
			break;

		switch (c) {
			/* version statement -v or --version
			 * following gnu-coding standards for command line interfaces */
		case 'v':
			printf("\n%s %s  \n", PACKAGE, VERSION);
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
					break;
				}

				resolution_code = atoi(optarg);

				if (resolution_code >= 0 && resolution_code < nb_res) {
					if (resolution_code != 0)
						command_line_override_for_screen_resolution = TRUE;
					GameConfig.screen_width = hard_resolutions[resolution_code].xres;
					GameConfig.screen_height = hard_resolutions[resolution_code].yres;
				} else {
					fprintf(stderr, "\nresolution code received: %d\n", resolution_code);
					char *txt = (char *)malloc((nb_res * 128 + 1) * sizeof(char));
					txt[0] = '\0';
					int i;
					for (i = 0; i < nb_res; i++) {
						char tmp[129];
						snprintf(tmp, 129, "\t\t%d = %s\n", i, hard_resolutions[i].comment);
						strncat(txt, tmp, 128);
					}
					ErrorMessage(__FUNCTION__, "  %s%s  %s", NO_NEED_TO_INFORM, IS_FATAL,
						     "\tThe resolution identifier given is not a valid resolution code.\n"
						     "\tThese codes correspond to the following hardcoded resolutions available:\n",
						     txt,
						     "\tAdditional resolutions may be specified by the form 'WxH' e.g. '800x600'\n"
						     "\tThe in-game menu automatically detects fullscreen modes supported by your hardware.\n"
						     "----------------------------------------------------------------------\n");
					free(txt);
				}
			}
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

		default:
			printf("\nOption %c not implemented yet! Ignored.", c);
			break;
		}		/* switch(c) */
	}			/* while(1) */

	// By default, after starting up, the current resolution should be
	// the resolution used at the next game startup too, so we preselect
	// that for now.  The user can still change that later inside the
	// game from within the options menu.
	//
	GameConfig.next_time_width_of_screen = GameConfig.screen_width;
	GameConfig.next_time_height_of_screen = GameConfig.screen_height;

};				// ParseCommandLine (int argc, char *const argv[])

/* -----------------------------------------------------------------
 * This function initializes a completely new game within freedroid.
 * In contrast to InitFreedroid, this function should be called 
 * whenever or better before any new game is started.
 * -----------------------------------------------------------------*/
void PrepareStartOfNewCharacter(char *startpos)
{
	location StartPosition;

	Activate_Conservative_Frame_Computation();

	// We make sure we don't have garbage in our arrays from a 
	// previous game or failed load-game attempt...
	//
	clear_out_arrays_for_fresh_game();
	// We do the same as above for lua state
	reset_lua_state();

	GetEventTriggers("events.dat");

	if (!skip_initial_menus)
		PlayATitleFile("StartOfGame.title");

	init_npcs();
	init_factions();

	GetCrew("ReturnOfTux.droids");

	ResolveMapLabelOnShip(startpos, &StartPosition);
	reset_visible_levels();
	if (game_root_mode == ROOT_IS_LVLEDIT && level_exists(GameConfig.last_edited_level))
		teleport_to_level_center(GameConfig.last_edited_level);
	else
		Teleport(StartPosition.level, StartPosition.x, StartPosition.y, FALSE, TRUE);
	clear_active_bullets();

	// At this point the position history can be initialized
	//
	InitInfluPositionHistory();

	// Now we read in the mission targets for this mission
	// Several different targets may be specified simultaneously
	//
	GetQuestList("quests.dat");

	SwitchBackgroundMusicTo(curShip.AllLevels[Me.pos.z]->Background_Song_Name);

	// Now we know that right after starting a new game, the Tux might have
	// to 'change clothes' i.e. a lot of tux images need to be updated which can
	// take a little time.  Therefore we print some message so the user will not
	// panic and push the reset button :)
	//
	our_SDL_flip_wrapper();

	widget_text_init(message_log, _("--- Message Log ---"));
	if (strcmp(startpos, "TutorialTuxStart") == 0)
		append_new_game_message(_("Starting tutorial."));
	else
		append_new_game_message(_("Starting new game."));
}

void prepare_level_editor()
{
	game_root_mode = ROOT_IS_LVLEDIT;
	skip_initial_menus = 1;
	char fp[2048];
	find_file("levels.dat", MAP_DIR, fp, 0);
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
 * from a different version of freedroid, which could be dangerous as
 * well.
 */
void ResetGameConfigToDefaultValues(void)
{
	// At first we set audio volume to maximum value.
	// This might be replaced later with values from a 
	// private user Freedroid config file.  But for now
	// this code is good enough...
	//
	GameConfig.Current_BG_Music_Volume = 0.5;
	GameConfig.Current_Sound_FX_Volume = 0.5;
	GameConfig.current_gamma_correction = 1.00;
	GameConfig.WantedTextVisibleTime = 3;
	GameConfig.Draw_Framerate = TRUE;
	GameConfig.Draw_Position = TRUE;
	GameConfig.All_Texts_Switch = FALSE;
	GameConfig.enemy_energy_bars_visible = TRUE;
	GameConfig.limit_framerate = TRUE;
	GameConfig.skip_light_radius = FALSE;
	GameConfig.omit_obstacles_in_level_editor = FALSE;
	GameConfig.omit_enemies_in_level_editor = TRUE;
	GameConfig.zoom_is_on = FALSE;
	GameConfig.show_blood = TRUE;
	GameConfig.show_lvledit_tooltips = TRUE;
	GameConfig.show_grid = TRUE;
	GameConfig.show_wp_connections = FALSE;
	GameConfig.number_of_big_screen_messages = 4;
	GameConfig.delay_for_big_screen_messages = 6.5;
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
	GameConfig.fullscreen_on = FALSE;
}

/** 
 * Set signal handlers for SIGSEGV, SIGFPE, and enable floating point exceptions.
 */
static void set_signal_handlers(void)
{


#ifndef __WIN32__

	// Let's see if we're dealing with a real release or rather if
	// we're dealing with a svn version.  The difference is this:
	// Releases shouldn't terminate upon a floating point exception
	// while the current svn code (for better debugging) should
	// do so.  Therefore we check for 'svn' in the current version
	// string and enable/disable the exceptions accordingly...
	if (strstr(VERSION, "svn")) {
		DebugPrintf(-4, "\nThis seems to be a development version, so we'll exit on floating point exceptions.");
		// feenableexcept ( FE_ALL_EXCEPT );
		// feenableexcept ( FE_INEXACT ) ;
		// feenableexcept ( FE_UNDERFLOW ) ;
		// feenableexcept ( FE_OVERFLOW ) ;
		feenableexcept(FE_INVALID);
		feenableexcept(FE_DIVBYZERO);
	} else {
		DebugPrintf(-4, "\nThis seems to be a 'stable' release, so no exit on floating point exceptions.");
		fedisableexcept(FE_INVALID);
		fedisableexcept(FE_DIVBYZERO);
	}
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
	screen_resolution resolution_holder;
	int i, j, merged_size;
	char *res[64];

	// Get available fullscreen/hardware modes (reported by SDL)
	modes = SDL_ListModes(NULL, SDL_FULLSCREEN|SDL_HWSURFACE);
	if (modes == (SDL_Rect**) -1) {
		ErrorMessage(__FUNCTION__,
			"SDL reports all resolutions are supported in fullscreen mode.\n"
			"Please use -r WIDTHxHEIGHT to specify any one you like.\n"
			"Defaulting to a sane one for now\n", NO_NEED_TO_INFORM, IS_WARNING_ONLY);
			screen_resolutions[0] =	(screen_resolution) {DEFAULT_SCREEN_WIDTH, DEFAULT_SCREEN_HEIGHT, "", TRUE};
			i = 1;
	} else {
		// Add resolutions to the screen_resolutions array
		for (i = 0; modes[i] && i < MAX_RESOLUTIONS; ++i) {
			res[i] = (char *) malloc(32);
			sprintf(res[i], "%d x %d", modes[i]->w, modes[i]->h);
			screen_resolutions[i] = (screen_resolution) {modes[i]->w, modes[i]->h, res[i], TRUE};
		}
	}

	// Add hard-coded resolutions to the list
	for (j = 0; j < sizeof(hard_resolutions) / sizeof(hard_resolutions[0]) && i + j < MAX_RESOLUTIONS; j++) {
		screen_resolutions[i + j] = hard_resolutions[j];
	}

	// Sort and prune duplicates
	merged_size = i + min(sizeof(hard_resolutions) / sizeof(hard_resolutions[0]), j);
	for (i = 0; i < merged_size; i++) {
		for (j = 0; j < merged_size - 1; j++) {
			// Sort in descending order of xres, then yres
			if (screen_resolutions[j].xres < screen_resolutions[j + 1].xres ||
					(screen_resolutions[j].xres == screen_resolutions[j + 1].xres &&
					screen_resolutions[j].yres < screen_resolutions[j +1].yres)) {
				resolution_holder = screen_resolutions[j + 1];
				screen_resolutions[j + 1] = screen_resolutions[j];
				screen_resolutions[j] = resolution_holder;

			// If it's a duplicate we'll set it to a terminator entry and it will be sorted to the end				
			} else if (screen_resolutions[j].xres == screen_resolutions[j + 1].xres &&
					screen_resolutions[j].yres == screen_resolutions[j + 1].yres)
				screen_resolutions[j + 1] = (screen_resolution) {-1, -1, "", FALSE};
		}
	}

	// Add our terminator on the end, just in case
	screen_resolutions[i - 1] = (screen_resolution) {-1, -1, "", FALSE};
};

/* -----------------------------------------------------------------
 * This function initializes the whole Freedroid game.
 * 
 * THIS MUST NOT BE CONFUSED WITH INITNEWGAME, WHICH
 * ONLY INITIALIZES A NEW MISSION FOR THE GAME.
 *  
 * ----------------------------------------------------------------- */
void InitFreedroid(int argc, char **argv)
{
	struct stat statbuf;

	// We mention the version of FreedroidRPG, so that debug reports
	// are easier to assign to the different versions of the game.
	//
	DebugPrintf(-4, "\nHello, this is FreedroidRPG, version %s.", VERSION);

	set_signal_handlers();

	clear_out_arrays_for_fresh_game();

	timeout_from_item_drop = 0;

	Overall_Average = 0.041;
	SkipAFewFrames = 0;

	ResetGameConfigToDefaultValues();

#if __WIN32__
	our_homedir = ".";
#else
	// first we need the user's homedir for loading/saving stuff
	if ((our_homedir = getenv("HOME")) == NULL) {
		DebugPrintf(0, "WARNING: Environment does not contain HOME variable...\n\
I will try to use local directory instead\n");
		our_homedir = ".";
	}
#endif

	our_config_dir = MyMalloc(strlen(our_homedir) + 20);
	sprintf(our_config_dir, "%s/.freedroid_rpg", our_homedir);

	if (stat(our_config_dir, &statbuf) == -1) {
		DebugPrintf(0, "\n----------------------------------------------------------------------\n\
You seem not to have the directory %s in your home directory.\n\
This directory is used by FreedroidRPG to store saved games and your personal settings.\n\
So I'll try to create it now...\n\
----------------------------------------------------------------------\n", our_config_dir);
#if __WIN32__
		_mkdir(our_config_dir);
		DebugPrintf(1, "ok\n");
#else
		if (mkdir(our_config_dir, S_IREAD | S_IWRITE | S_IEXEC) == -1) {
			DebugPrintf(0, "\n----------------------------------------------------------------------\n\
WARNING: Failed to create config-dir: %s. Giving up...\n\
I will not be able to load or save games or configurations\n\
----------------------------------------------------------------------\n", our_config_dir);
			free(our_config_dir);
			our_config_dir = NULL;
		} else {
			DebugPrintf(1, "ok\n");
		}
#endif
	}

	input_keyboard_init();
	input_set_default();
	init_lua();

	if (SDL_Init(SDL_INIT_VIDEO) == -1)
		ErrorMessage(__FUNCTION__, "Couldn't initialize SDL: %s\n", PLEASE_INFORM, IS_FATAL,  SDL_GetError());
	// So the video library could be initialized.  So it should also be
	// cleaned up and closed once we're done and quit FreedroidRPG.
	atexit(SDL_Quit);

	detect_available_resolutions();

	LoadGameConfig();

	ParseCommandLine(argc, argv);

	LightRadiusInit();

	// Adapt button positions for the current screen resolution.  (Note: At this
	// point the command line has been evaluated already, therefore we know if OpenGL
	// is used or not and also which screen resolution to use.
	//
	adapt_button_positions_to_screen_resolution();

	Copy_Rect(Full_User_Rect, User_Rect);

	init_timer();

	InitVideo();

	next_startup_percentage(0);

	init_keyboard_input_array();
	init_message_log();
	init_luaconfig();

	InitAudio();

	// Now that the music files have been loaded successfully, it's time to set
	// the music and sound volumes accordingly, i.e. as specifies by the users
	// configuration.
	//
	// THIS MUST NOT BE DONE BEFORE THE SOUND SAMPLES HAVE BEEN LOADED!!
	//
	SetSoundFXVolume(GameConfig.Current_Sound_FX_Volume);

	Init_Game_Data();

	/* 
	 * Initialize random-number generator in order to make 
	 * level-start etc really different at each program start
	 */
	srand(time(NULL));

	InitPictures(); //requires game data loaded in Init_Game_Data()

	next_startup_percentage(100);
	if (strstr(VERSION, "rc"))
		alert_window("%s", _("You are playing a Release Candidate.\nStrange bugs might still be present in the game.\nPlease report any issues you find to either of:\n\n#freedroid at irc.freenode.net\nfreedroid-discussion AT lists.sourceforge.net\nhttps://sourceforge.net/apps/phpbb/freedroid\n\nor directly to the bugtracker at http://bugs.freedroid.org\nThank you for helping us test the game.\n\nGood luck!\n"));

};				// void InitFreedroid ( void ) 

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
	int j;
	int now;

	DebugPrintf(1, "\n%s(): Real function call confirmed.", __FUNCTION__);
	append_new_game_message(_("Game over.\n"));
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
		DropItemAt(GetItemIndexByName("Valuable Circuits"), Me.pos.z, Me.pos.x, Me.pos.y, 1);
	}

	GameOver = TRUE;

	while ((SDL_GetTicks() - now < 1000 * WAIT_AFTER_KILLED)) {

		AssembleCombatPicture(DO_SCREEN_UPDATE | SHOW_ITEMS);
		show_texts_and_banner();
		animate_blasts();
		MoveBullets();
		move_enemies();
		animate_obstacles();
		UpdateCountersForThisFrame();
		
		for (j = 0; j < MAXBULLETS; j++)
			CheckBulletCollisions(j);

	}

        do_death_menu();

	input_handle();
};				// void ThouArtDefeated(void)

/**
 * This function displays the last seconds of the game when the influencer
 * has actually been killed.  It generates some explosions and waits for
 * some seconds, where the user can reload his latest game, or after that
 * returns to finally quit the inner game loop and the program will 
 * (outside this function) ask for a completely new game or loading a different
 * saved game or quit as in the very beginning of the game.
 */
void ThouHastWon(void)
{
	int j;
	int now;

	DebugPrintf(1, "\n%s(): Real function call confirmed.", __FUNCTION__);
	append_new_game_message(_("Game won.\n"));
	GameConfig.Inventory_Visible = FALSE;
	GameConfig.CharacterScreen_Visible = FALSE;
	now = SDL_GetTicks();

	GameOver = TRUE;

	while ((SDL_GetTicks() - now < 1000 * WAIT_AFTER_GAME_WON)) {

		AssembleCombatPicture(DO_SCREEN_UPDATE | SHOW_ITEMS);
		show_texts_and_banner();
		animate_blasts();
		MoveBullets();
		move_enemies();
		animate_obstacles();

		// ReactToSpecialKeys();

		for (j = 0; j < MAXBULLETS; j++)
			CheckBulletCollisions(j);

	}

	// Now it's time for the end game title file...
	//
	PlayATitleFile("EndOfGame.title");

	PlayATitleFile("Credits.title");

	input_handle();

};				// void ThouHastWon(void)

#undef _init_c
