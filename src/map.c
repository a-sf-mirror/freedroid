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
 * This file contains (all?) map-related functions, which also includes 
 * loading of decks and whole ships, starting the lifts and consoles if 
 * close to the paradroid, refreshes as well as determining the map brick 
 * that contains specified coordinates are done in this file.
 */

#define _map_c

#include "system.h"

#include "defs.h"
#include "struct.h"
#include "proto.h"
#include "global.h"

#include "lvledit/lvledit_actions.h"
#include "lvledit/lvledit_display.h"
#include "map.h"

void TranslateToHumanReadable(Uint16 * HumanReadable, map_tile * MapInfo, int LineLength);
void GetThisLevelsDroids(char *SectionPointer);
Level DecodeLoadedLeveldata(char *data);

struct animated_obstacle {
	int index;
	struct list_head node;
};

int animated_obstacles_lists_level = -1;	//level for our lists, -1 means lists are dirty
struct list_head droid_nests_head = LIST_HEAD_INIT(droid_nests_head);
struct list_head teleporters_head = LIST_HEAD_INIT(teleporters_head);
struct list_head doors_head = LIST_HEAD_INIT(doors_head);
struct list_head autoguns_head = LIST_HEAD_INIT(autoguns_head);
/**
 * If the blood doesn't vanish, then there will be more and more blood,
 * especially after the bots on the level have respawned a few times.
 * Therefore we need this function, which will remove all traces of blood
 * from a given level.
 */
static void remove_blood_obstacles_for_respawning(int level_num)
{
	int i;

	//--------------------
	// We pass through all the obstacles, deleting those
	// that are 'blood'.
	//
	for (i = 0; i < MAX_OBSTACLES_ON_MAP; i++) {
		switch (curShip.AllLevels[level_num]->obstacle_list[i].type) {
			//--------------------
			// In case we encounter the -1 obstacle, we're done, cause 'holes'
			// aren't permitted inside the obstacle list of a level...
			//
		case ISO_BLOOD_1:
		case ISO_BLOOD_2:
		case ISO_BLOOD_3:
		case ISO_BLOOD_4:
		case ISO_BLOOD_5:
		case ISO_BLOOD_6:
		case ISO_BLOOD_7:
		case ISO_BLOOD_8:
		case ISO_OIL_STAINS_1:
		case ISO_OIL_STAINS_2:
		case ISO_OIL_STAINS_3:
		case ISO_OIL_STAINS_4:
		case ISO_OIL_STAINS_5:
		case ISO_OIL_STAINS_6:
		case ISO_OIL_STAINS_7:
		case ISO_OIL_STAINS_8:
			action_remove_obstacle(curShip.AllLevels[level_num], &(curShip.AllLevels[level_num]->obstacle_list[i]));
			//--------------------
			// Now the obstacles have shifted a bit to close the gap from the
			// deletion.  We need to re-process the current index in the next
			// loop of this cycle...
			//
			i--;
			break;
		default:
			break;
		}
	}

};				// void remove_blood_obstacles_for_respawning ( int level_num )

/**
 * This function will make all blood obstacles vanish and all dead bots
 * (and characters) come back to life and resume their previous operation
 * from before thier death.
 */
void respawn_level(int level_num)
{
	int wp_num = curShip.AllLevels[level_num]->num_waypoints;
	char wp_used[wp_num];
	memset(wp_used, 0, wp_num);

	//--------------------
	// First we remove all the blood obstacles...
	//
	remove_blood_obstacles_for_respawning(level_num);

	//--------------------
	// Now we can start to fill the enemies on this level with new life...
	//
	enemy *aerot, *next, *erot, *nerot;
	BROWSE_DEAD_BOTS_SAFE(aerot, next) {
		if (aerot->pos.z != level_num)
			continue;

		if (Druidmap[aerot->type].is_human)
			continue;

		/* Move the bot to the alive list */
		list_move(&(aerot->global_list), &alive_bots_head);

		/* reinsert it into the current level list */
		list_add(&(aerot->level_list), &level_bots_head[level_num]);
	}

	BROWSE_ALIVE_BOTS_SAFE(erot, nerot) {
		if (erot->pos.z != level_num)
			continue;

		erot->energy = Druidmap[erot->type].maxenergy;
		erot->animation_phase = 0;
		erot->animation_type = WALK_ANIMATION;
		erot->follow_tux = 0;
		erot->CompletelyFixed = 0;

		if (erot->has_been_taken_over == TRUE) {
			erot->is_friendly = FALSE;
			erot->has_been_taken_over = FALSE;
		}

		if (!erot->is_friendly) {
			erot->combat_state = SELECT_NEW_WAYPOINT;

			erot->has_greeted_influencer = FALSE;
			erot->state_timeout = 0;
		}

		if (!erot->SpecialForce) {
			int out = TeleportToRandomWaypoint(erot, curShip.AllLevels[level_num], wp_used);
			if (out > 0)
				wp_used[out] = 1;
		}
	}

};				// void respawn_level ( int level_num )

/**
 * Now that we plan not to use hard-coded and explicitly human written 
 * coordinates any more, we need to use some labels instead.  But there
 * should be a function to conveniently resolve a given label within a
 * given map.  That's what this function is supposed to do.
 */
static void ResolveMapLabelOnLevel(const char *MapLabel, location * PositionPointer, int LevelNum)
{
	Level ResolveLevel = curShip.AllLevels[LevelNum];
	int i;

	for (i = 0; i < MAX_MAP_LABELS_PER_LEVEL; i++) {
		if (ResolveLevel->labels[i].pos.x == (-1))
			continue;

		if (!strcmp(ResolveLevel->labels[i].label_name, MapLabel)) {
			PositionPointer->x = ResolveLevel->labels[i].pos.x + 0.5;
			PositionPointer->y = ResolveLevel->labels[i].pos.y + 0.5;
			PositionPointer->level = LevelNum;
			DebugPrintf(1, "\nResolving map label '%s' succeeded: pos.x=%d, pos.y=%d, pos.z=%d.",
				    MapLabel, PositionPointer->x, PositionPointer->y, PositionPointer->level);
			return;
		}
	}

	PositionPointer->x = -1;
	PositionPointer->y = -1;
	PositionPointer->level = -1;
	DebugPrintf(1, "\nResolving map label '%s' failed on level %d.", MapLabel, LevelNum);
};				// void ResolveMapLabel ( char* MapLabel , grob_point* PositionPointer )

/**
 * This is the ultimate function to resolve a given label within a
 * given SHIP.
 */
void ResolveMapLabelOnShip(const char *MapLabel, location * PositionPointer)
{
	int i;

	//--------------------
	// We empty the given target pointer, so that we can tell
	// a successful resolve later...
	//
	PositionPointer->x = -1;
	PositionPointer->y = -1;

	//--------------------
	// Now we check each level of the ship, if it maybe contains this
	// label...
	//
	for (i = 0; i < curShip.num_levels; i++) {
		if (curShip.AllLevels[i] == NULL)
			continue;
		ResolveMapLabelOnLevel(MapLabel, PositionPointer, i);

		if (PositionPointer->x != (-1))
			return;
	}

	ErrorMessage(__FUNCTION__, "\
Resolving map label %s failed on the complete ship!\n\
This is a severe error in the game data of Freedroid.", PLEASE_INFORM, IS_FATAL, MapLabel);

};

/**
 * Next we extract the level interface data from the human-readable data 
 * into the level struct, but WITHOUT destroying or damaging the 
 * human-readable data in the process!
 */
static void DecodeInterfaceDataForThisLevel(Level loadlevel, char *DataPointer)
{
	char *TempSectionPointer;
	char PreservedLetter;

	//--------------------
	// Now we read in the jump points associated with this map
	//

	// We look for the beginning and end of the map statement section
	TempSectionPointer = LocateStringInData(DataPointer, MAP_BEGIN_STRING);

	// We add a terminator at the end, but ONLY TEMPORARY.  The damage will be restored later!
	PreservedLetter = TempSectionPointer[0];
	TempSectionPointer[0] = 0;

#define DEBUG_LEVEL_INTERFACES 1

	ReadValueFromString(DataPointer, "jump threshold north: ", "%d", &(loadlevel->jump_threshold_north), TempSectionPointer);
	DebugPrintf(DEBUG_LEVEL_INTERFACES, "\nSuccessfully read jump theshold north : %d ", loadlevel->jump_threshold_north);
	if (loadlevel->jump_threshold_north < 0)
		loadlevel->jump_threshold_north = 0;
	ReadValueFromString(DataPointer, "jump threshold south: ", "%d", &(loadlevel->jump_threshold_south), TempSectionPointer);
	DebugPrintf(DEBUG_LEVEL_INTERFACES, "\nSuccessfully read jump theshold south : %d ", loadlevel->jump_threshold_south);
	if (loadlevel->jump_threshold_south < 0)
		loadlevel->jump_threshold_south = 0;
	ReadValueFromString(DataPointer, "jump threshold east: ", "%d", &(loadlevel->jump_threshold_east), TempSectionPointer);
	DebugPrintf(DEBUG_LEVEL_INTERFACES, "\nSuccessfully read jump theshold east : %d ", loadlevel->jump_threshold_east);
	if (loadlevel->jump_threshold_east < 0)
		loadlevel->jump_threshold_east = 0;
	ReadValueFromString(DataPointer, "jump threshold west: ", "%d", &(loadlevel->jump_threshold_west), TempSectionPointer);
	DebugPrintf(DEBUG_LEVEL_INTERFACES, "\nSuccessfully read jump theshold west : %d ", loadlevel->jump_threshold_west);
	if (loadlevel->jump_threshold_west < 0)
		loadlevel->jump_threshold_west = 0;

	ReadValueFromString(DataPointer, "jump target north: ", "%d", &(loadlevel->jump_target_north), TempSectionPointer);
	DebugPrintf(DEBUG_LEVEL_INTERFACES, "\nSuccessfully read jump target north : %d ", loadlevel->jump_target_north);
	ReadValueFromString(DataPointer, "jump target south: ", "%d", &(loadlevel->jump_target_south), TempSectionPointer);
	DebugPrintf(DEBUG_LEVEL_INTERFACES, "\nSuccessfully read jump target south : %d ", loadlevel->jump_target_south);
	ReadValueFromString(DataPointer, "jump target east: ", "%d", &(loadlevel->jump_target_east), TempSectionPointer);
	DebugPrintf(DEBUG_LEVEL_INTERFACES, "\nSuccessfully read jump target east : %d ", loadlevel->jump_target_east);
	ReadValueFromString(DataPointer, "jump target west: ", "%d", &(loadlevel->jump_target_west), TempSectionPointer);
	DebugPrintf(DEBUG_LEVEL_INTERFACES, "\nSuccessfully read jump target west : %d ", loadlevel->jump_target_west);
	ReadValueFromString(DataPointer, "use underground lighting: ", "%d", &(loadlevel->use_underground_lighting), TempSectionPointer);
	DebugPrintf(DEBUG_LEVEL_INTERFACES, "\nSuccessfully read use_underground_lighting : %d ", loadlevel->use_underground_lighting);

	TempSectionPointer[0] = PreservedLetter;

};				// void DecodeInterfaceDataForThisLevel ( Level loadlevel , char* data )

/**
 * Next we extract the level parameters from the human-readable data into
 * the level struct, but WITHOUT destroying or damaging the human-readable
 * data in the process!
 */
static void DecodeDimensionsOfThisLevel(Level loadlevel, char *DataPointer)
{

	int off = 0;

	/* Read levelnumber */
	char *fp = DataPointer;	//strstr(DataPointer, "Levelnumber");
	fp += 12;
	while (*(fp + off) != '\n')
		off++;
	fp[off] = 0;
	loadlevel->levelnum = atoi(fp);
	fp[off] = '\n';
	fp += off + 1;
	off = 0;

	/* Read xlen */
	fp += strlen("xlen of this level:");
	while (*(fp + off) != '\n')
		off++;
	fp[off] = 0;
	loadlevel->xlen = atoi(fp);
	fp[off] = '\n';
	fp += off + 1;
	off = 0;

	/* Read ylen */
	fp += strlen("ylen of this level:");
	while (*(fp + off) != '\n')
		off++;
	fp[off] = 0;
	loadlevel->ylen = atoi(fp);
	fp[off] = '\n';
	fp += off + 1;
	off = 0;

	/* Read lrb */
	fp += strlen("light radius bonus of this level:");
	while (*(fp + off) != '\n')
		off++;
	fp[off] = 0;
	loadlevel->light_radius_bonus = atoi(fp);
	fp[off] = '\n';
	fp += off + 1;
	off = 0;

	fp += strlen("minimal light on this level:");
	while (*(fp + off) != '\n')
		off++;
	fp[off] = 0;
	loadlevel->minimum_light_value = atoi(fp);
	fp[off] = '\n';
	fp += off + 1;
	off = 0;

	fp += strlen("infinite_running_on_this_level:");
	while (*(fp + off) != '\n')
		off++;
	fp[off] = 0;
	loadlevel->infinite_running_on_this_level = atoi(fp);
	fp[off] = '\n';
	fp += off + 1;
	off = 0;

	fp += strlen("random dungeon:");
	while (*(fp + off) != '\n')
		off++;
	fp[off] = 0;
	loadlevel->random_dungeon = atoi(fp);
	fp[off] = '\n';
	fp += off + 1;
	off = 0;

	if (loadlevel->ylen >= MAX_MAP_LINES) {
		ErrorMessage(__FUNCTION__, "\
A maplevel Freedroid was supposed to load has more map lines than allowed\n\
for a map level as by the constant MAX_MAP_LINES in defs.h.\n\
Sorry, but unless this constant is raised, Freedroid will refuse to load this map.", PLEASE_INFORM, IS_FATAL);
	}

};				// void DecodeDimensionsOfThisLevel ( Level loadlevel , char* DataPointer );

/**
 * Next we extract the human readable obstacle data into the level struct
 * WITHOUT destroying or damaging the human-readable data in the process!
 * This is an improved parser that is not quite readable but very performant.
 */
static void decode_obstacles_of_this_level(level * loadlevel, char *DataPointer)
{
	int i;
	char *curfield = NULL;
	char *curfieldend = NULL;
	char *obstacle_SectionBegin;

	//--------------------
	// First we initialize the obstacles with 'empty' information
	//
	for (i = 0; i < MAX_OBSTACLES_ON_MAP; i++) {
		loadlevel->obstacle_list[i].type = -1;
		loadlevel->obstacle_list[i].pos.x = -1;
		loadlevel->obstacle_list[i].pos.y = -1;
		loadlevel->obstacle_list[i].pos.z = loadlevel->levelnum;
		loadlevel->obstacle_list[i].name_index = -1;
		loadlevel->obstacle_list[i].description_index = -1;
	}

	if (loadlevel->random_dungeon)
		return;

	//--------------------
	// Now we look for the beginning and end of the obstacle section
	//
	obstacle_SectionBegin = LocateStringInData(DataPointer, OBSTACLE_DATA_BEGIN_STRING) + strlen(OBSTACLE_DATA_BEGIN_STRING) + 1;

	//--------------------
	// Now we decode all the obstacle information
	//
	curfield = obstacle_SectionBegin;
	i = 0;
	while (*curfield != '/') {
		//structure of obstacle entry is :      // t59 x2.50 y63.50 l-1 d-1 
		//we read the type
		curfield++;
		curfieldend = curfield;
		while ((*curfieldend) != ' ')
			curfieldend++;
		(*curfieldend) = 0;
		loadlevel->obstacle_list[i].type = atoi(curfield);

		//we read the X position
		curfield = curfieldend + 2;
		(*curfieldend) = ' ';
		curfieldend += 2;
		while ((*curfieldend) != ' ')
			curfieldend++;
		(*curfieldend) = 0;
		loadlevel->obstacle_list[i].pos.x = atof(curfield);

		//Y position
		curfield = curfieldend + 2;
		(*curfieldend) = ' ';
		curfieldend += 2;
		while ((*curfieldend) != ' ')
			curfieldend++;
		(*curfieldend) = 0;
		loadlevel->obstacle_list[i].pos.y = atof(curfield);

		//label #
		curfield = curfieldend + 2;
		(*curfieldend) = ' ';
		curfieldend += 2;
		while ((*curfieldend) != ' ')
			curfieldend++;
		(*curfieldend) = 0;
		loadlevel->obstacle_list[i].name_index = atoi(curfield);

		//description #
		curfield = curfieldend + 2;
		(*curfieldend) = ' ';
		curfieldend += 2;
		while ((*curfieldend) != ' ')
			curfieldend++;
		(*curfieldend) = 0;
		loadlevel->obstacle_list[i].description_index = atoi(curfield);
		(*curfieldend) = ' ';

		while ((*curfield) != '\n')
			curfield++;
		curfield++;
		//fprintf( stderr , "\nobtacle_type=%d pos.x=%3.2f pos.y=%3.2f\n" , loadlevel -> obstacle_list [ i ] . type ,  loadlevel -> obstacle_list [ i ] . pos . 
//x , loadlevel-> obstacle_list [ i ] . pos . y );
		i++;
	}

};				// void decode_obstacles_of_this_level ( loadlevel , DataPointer )

/**
 * Next we extract the map labels of this level WITHOUT destroying
 * or damaging the data in the process!
 */
static void DecodeMapLabelsOfThisLevel(Level loadlevel, char *DataPointer)
{
	int i;
	char PreservedLetter;
	char *MapLabelPointer;
	char *MapLabelSectionBegin;
	char *MapLabelSectionEnd;
	int NumberOfMapLabelsInThisLevel;

	//--------------------
	// First we initialize the map labels array with 'empty' information
	//
	for (i = 0; i < MAX_MAP_LABELS_PER_LEVEL; i++) {
		loadlevel->labels[i].pos.x = (-1);
		loadlevel->labels[i].pos.y = (-1);
		if (loadlevel->labels[i].label_name != NULL) {
			free(loadlevel->labels[i].label_name);
			loadlevel->labels[i].label_name = NULL;
		}
		loadlevel->labels[i].label_name = "no_label_defined";
	}

	if (loadlevel->random_dungeon)
		return;

	//--------------------
	// Now we look for the beginning and end of the map labels section
	//
	MapLabelSectionBegin = LocateStringInData(DataPointer, MAP_LABEL_BEGIN_STRING) + strlen(MAP_LABEL_BEGIN_STRING) + 1;
	MapLabelSectionEnd = LocateStringInData(MapLabelSectionBegin, MAP_LABEL_END_STRING);

	//--------------------
	// We add a terminator at the end, but ONLY TEMPORARY.  The damage will be restored later!
	//
	PreservedLetter = MapLabelSectionEnd[0];
	MapLabelSectionEnd[0] = 0;
	NumberOfMapLabelsInThisLevel = CountStringOccurences(MapLabelSectionBegin, LABEL_ITSELF_ANNOUNCE_STRING);
	DebugPrintf(1, "\nNumber of map labels found in this level : %d.", NumberOfMapLabelsInThisLevel);

	//--------------------
	// Now we decode all the map label information
	//
	MapLabelPointer = MapLabelSectionBegin;
	for (i = 0; i < NumberOfMapLabelsInThisLevel; i++) {
		if (i)
			MapLabelPointer = strstr(MapLabelPointer + 1, X_POSITION_OF_LABEL_STRING);
		ReadValueFromString(MapLabelPointer, X_POSITION_OF_LABEL_STRING, "%d", &(loadlevel->labels[i].pos.x), MapLabelSectionEnd);
		ReadValueFromString(MapLabelPointer, Y_POSITION_OF_LABEL_STRING, "%d", &(loadlevel->labels[i].pos.y), MapLabelSectionEnd);
		loadlevel->labels[i].label_name = ReadAndMallocStringFromData(MapLabelPointer, LABEL_ITSELF_ANNOUNCE_STRING, "\"");

		DebugPrintf(1, "\npos.x=%d pos.y=%d label_name=\"%s\"", loadlevel->labels[i].pos.x,
			    loadlevel->labels[i].pos.y, loadlevel->labels[i].label_name);
	}

	//--------------------
	// Now we repair the damage done to the loaded level data
	//
	MapLabelSectionEnd[0] = PreservedLetter;

};				// void DecodeMapLabelsOfThisLevel ( Level loadlevel , char* DataPointer );

/**
 * Every map level in a FreedroidRPG 'ship' can have up to 
 * MAX_OBSTACLE_NAMES_PER_LEVEL obstacles, that have a label attached to
 * them.  Such obstacle labels are very useful when modifying obstacles
 * from within game events and triggers.  The obstacle labels are stored
 * in a small subsection of the whole level data.  This function decodes
 * this small subsection and loads all the obstacle data into the ship
 * struct.
 */
static void decode_obstacle_names_of_this_level(Level loadlevel, char *DataPointer)
{
	int i;
	char PreservedLetter;
	char *obstacle_namePointer;
	char *obstacle_nameSectionBegin;
	char *obstacle_nameSectionEnd;
	int NumberOfobstacle_namesInThisLevel;
	int target_index;

	//--------------------
	// At first we set all the obstacle name pointers to NULL in order to
	// mark them as unused.
	//
	for (i = 0; i < MAX_OBSTACLE_NAMES_PER_LEVEL; i++) {
		loadlevel->obstacle_name_list[i] = NULL;
	}

	if (loadlevel->random_dungeon)
		return;

	//--------------------
	// Now we look for the beginning and end of the map labels section
	//
	obstacle_nameSectionBegin = LocateStringInData(DataPointer, OBSTACLE_LABEL_BEGIN_STRING);
	obstacle_nameSectionEnd = LocateStringInData(obstacle_nameSectionBegin, OBSTACLE_LABEL_END_STRING);

	//--------------------
	// We add a terminator at the end, but ONLY TEMPORARY.  The damage will be restored later!
	//
	PreservedLetter = obstacle_nameSectionEnd[0];
	obstacle_nameSectionEnd[0] = 0;
	NumberOfobstacle_namesInThisLevel = CountStringOccurences(obstacle_nameSectionBegin, OBSTACLE_LABEL_ANNOUNCE_STRING);
	DebugPrintf(1, "\nNumber of obstacle labels found in this level : %d.", NumberOfobstacle_namesInThisLevel);

	//--------------------
	// Now we decode all the map label information
	//
	obstacle_namePointer = obstacle_nameSectionBegin;
	for (i = 0; i < NumberOfobstacle_namesInThisLevel; i++) {
		obstacle_namePointer = strstr(obstacle_namePointer + 1, INDEX_OF_OBSTACLE_NAME);
		ReadValueFromString(obstacle_namePointer, INDEX_OF_OBSTACLE_NAME, "%d", &(target_index), obstacle_nameSectionEnd);

		loadlevel->obstacle_name_list[target_index] =
		    ReadAndMallocStringFromData(obstacle_namePointer, OBSTACLE_LABEL_ANNOUNCE_STRING, "\"");

		DebugPrintf(1, "\nobstacle_name_index=%d obstacle_label_name=\"%s\"", target_index,
			    loadlevel->obstacle_name_list[target_index]);
	}
	//--------------------
	// Now we repair the damage done to the loaded level data
	//
	obstacle_nameSectionEnd[0] = PreservedLetter;

};				// void decode_obstacle_names_of_this_level ( loadlevel , DataPointer )

/**
 * Every map level in a FreedroidRPG 'ship' can have up to 
 * MAX_OBSTACLE_DESCRIPTIONS_PER_LEVEL obstacles, that have a label 
 * attached to them.  
 * Such obstacle labels are very useful when modifying obstacles
 * from within game events and triggers.  
 *
 * The obstacle labels are stored in a small subsection of the whole 
 * level data.  This function decodes this small subsection and loads all 
 * the obstacle data into the ship struct.
 */
static void decode_obstacle_descriptions_of_this_level(Level loadlevel, char *DataPointer)
{
	int i;
	char PreservedLetter;
	char *obstacle_descriptionPointer;
	char *obstacle_descriptionSectionBegin;
	char *obstacle_descriptionSectionEnd;
	int NumberOfobstacle_descriptionsInThisLevel;
	int target_index;

	//--------------------
	// At first we set all the obstacle description pointers to NULL in order to
	// mark them as unused.
	//
	for (i = 0; i < MAX_OBSTACLE_DESCRIPTIONS_PER_LEVEL; i++) {
		loadlevel->obstacle_description_list[i] = NULL;
	}

	if (loadlevel->random_dungeon)
		return;

	//--------------------
	// Now we look for the beginning and end of the obstacle descriptions section
	//
	obstacle_descriptionSectionBegin = LocateStringInData(DataPointer, OBSTACLE_DESCRIPTION_BEGIN_STRING);
	obstacle_descriptionSectionEnd = LocateStringInData(obstacle_descriptionSectionBegin, OBSTACLE_DESCRIPTION_END_STRING);

	//--------------------
	// We add a terminator at the end, but ONLY TEMPORARY.  The damage will be restored later!
	//
	PreservedLetter = obstacle_descriptionSectionEnd[0];
	obstacle_descriptionSectionEnd[0] = 0;
	NumberOfobstacle_descriptionsInThisLevel =
	    CountStringOccurences(obstacle_descriptionSectionBegin, OBSTACLE_DESCRIPTION_ANNOUNCE_STRING);
	DebugPrintf(1, "\nNumber of obstacle descriptions found in this level : %d.", NumberOfobstacle_descriptionsInThisLevel);

	//--------------------
	// Now we decode all the map label information
	//
	obstacle_descriptionPointer = obstacle_descriptionSectionBegin;
	for (i = 0; i < NumberOfobstacle_descriptionsInThisLevel; i++) {
		obstacle_descriptionPointer = strstr(obstacle_descriptionPointer + 1, INDEX_OF_OBSTACLE_DESCRIPTION);
		ReadValueFromString(obstacle_descriptionPointer, INDEX_OF_OBSTACLE_DESCRIPTION, "%d",
				    &(target_index), obstacle_descriptionSectionEnd);

		loadlevel->obstacle_description_list[target_index] =
		    ReadAndMallocStringFromData(obstacle_descriptionPointer, OBSTACLE_DESCRIPTION_ANNOUNCE_STRING, "\"");

		DebugPrintf(1, "\nobstacle_description_index=%d obstacle_description=\"%s\"", target_index,
			    loadlevel->obstacle_description_list[target_index]);
	}

	//--------------------
	// Now we repair the damage done to the loaded level data
	//
	obstacle_descriptionSectionEnd[0] = PreservedLetter;

};				// void decode_obstacle_descriptions_of_this_level ( loadlevel , DataPointer )

/**
 *
 *
 */
void glue_obstacles_to_floor_tiles_for_level(int level_num)
{
	level *loadlevel = curShip.AllLevels[level_num];
	int obstacle_counter = 2;
	int x_tile;
	int y_tile;
	int glue_index;
	int next_free_index;

	//--------------------
	// We clean out any obstacle glue information that might be still
	// in this level.
	//
	for (x_tile = 0; x_tile < loadlevel->xlen; x_tile++) {
		for (y_tile = 0; y_tile < loadlevel->ylen; y_tile++) {
			for (glue_index = 0; glue_index < MAX_OBSTACLES_GLUED_TO_ONE_MAP_TILE; glue_index++) {
				loadlevel->map[y_tile][x_tile].obstacles_glued_to_here[glue_index] = (-1);
			}
		}
	}

	//--------------------
	// Each obstacles must to be anchored to exactly one (the closest!)
	// map tile, so that we can find out obstacles 'close' to somewhere
	// more easily...
	//
	for (obstacle_counter = 0; obstacle_counter < MAX_OBSTACLES_ON_MAP; obstacle_counter++) {
		if (loadlevel->obstacle_list[obstacle_counter].type == -1)
			continue;

		//--------------------
		// We need to glue this one and we glue it to the closest map tile center we have...
		// For this we need first to prepare some things...
		//
		x_tile = rintf(loadlevel->obstacle_list[obstacle_counter].pos.x - 0.5);
		y_tile = rintf(loadlevel->obstacle_list[obstacle_counter].pos.y - 0.5);

		if (x_tile < 0)
			x_tile = 0;
		if (y_tile < 0)
			y_tile = 0;
		if (x_tile >= loadlevel->xlen)
			x_tile = loadlevel->xlen - 1;
		if (y_tile >= loadlevel->ylen)
			y_tile = loadlevel->ylen - 1;

		next_free_index = MAX_OBSTACLES_GLUED_TO_ONE_MAP_TILE;
		for (glue_index = 0; glue_index < MAX_OBSTACLES_GLUED_TO_ONE_MAP_TILE; glue_index++) {
			if (loadlevel->map[y_tile][x_tile].obstacles_glued_to_here[glue_index] != (-1)) {
				// DebugPrintf ( 0 , "\nHey, someone's already sitting here... moving to next index...: %d." ,
				// glue_index + 1 );
			} else {
				next_free_index = glue_index;
				break;
			}
		}

		//--------------------
		// some safety check against writing beyond the bonds of the
		// array.
		//
		if (next_free_index >= MAX_OBSTACLES_GLUED_TO_ONE_MAP_TILE) {
			// 
			// We disable this VERY FREQUENT warning now...
			//   
			/*
			   DebugPrintf ( 0 , "The position where the problem occured is: x_tile=%d, y_tile=%d." , x_tile , y_tile );
			   ErrorMessage ( __FUNCTION__  , "\
			   FreedroidRPG was unable to glue a certain obstacle to the nearest map tile.\n\
			   This bug can be resolved by simply raising a contant by one, but it needs to be done :)",
			   PLEASE_INFORM, IS_WARNING_ONLY );
			 */
			continue;
		}
		//--------------------
		// Now it can be glued...
		//
		loadlevel->map[y_tile][x_tile].obstacles_glued_to_here[next_free_index] = obstacle_counter;

	}

};				// glue_obstacles_to_floor_tiles_for_level ( int level_num )

/**
 * This function collects the automap data and stores it in the Me data
 * structure.
 */
void CollectAutomapData(void)
{
	int x, y;
	int start_x, start_y, end_x, end_y;
	gps ObjPos;
	static int TimePassed;
	Level automap_level = curShip.AllLevels[Me.pos.z];
	int i;
	Obstacle our_obstacle;
	int level = Me.pos.z;

	ObjPos.z = Me.pos.z;

	//--------------------
	// Checking the whole map for passablility will surely take a lot
	// of computation.  Therefore we only do this once every second of
	// real time.
	//
	if (TimePassed == (int)Me.MissionTimeElapsed)
		return;
	TimePassed = (int)Me.MissionTimeElapsed;

	// DebugPrintf ( -3 , "\nCollecting Automap data... " );

	//--------------------
	// Also if there is no map-maker present in inventory, then we need not
	// do a thing here...
	//
	if (!Me.map_maker_is_present)
		return;

	//--------------------
	// Earlier we had
	//
	// start_x = 0 ; start_y = 0 ; end_x = automap_level->xlen ; end_y = automap_level->ylen ;
	//
	// when maximal automap was generated.  Now we only add to the automap what really is on screen...
	//
	start_x = Me.pos.x - 9;
	end_x = Me.pos.x + 9;
	start_y = Me.pos.y - 9;
	end_y = Me.pos.y + 9;

	if (start_x < 0)
		start_x = 0;
	if (end_x >= automap_level->xlen)
		end_x = automap_level->xlen - 1;
	if (start_y < 0)
		start_y = 0;
	if (end_y >= automap_level->ylen)
		end_y = automap_level->ylen - 1;

	//--------------------
	// Now we do the actual checking for visible wall components.
	//
	for (y = start_y; y < end_y; y++) {
		for (x = start_x; x < end_x; x++) {
			if (Me.Automap[level][y][x] & SQUARE_SEEN_AT_ALL_BIT)
				continue;

			for (i = 0; i < MAX_OBSTACLES_GLUED_TO_ONE_MAP_TILE; i++) {
				if (automap_level->map[y][x].obstacles_glued_to_here[i] == (-1))
					continue;
				our_obstacle = &(automap_level->obstacle_list[automap_level->map[y][x].obstacles_glued_to_here[i]]);

				if ((our_obstacle->type >= ISO_H_DOOR_000_OPEN) && (our_obstacle->type <= ISO_V_DOOR_100_OPEN))
					continue;
				if ((our_obstacle->type >= ISO_DH_DOOR_000_OPEN) && (our_obstacle->type <= ISO_DV_DOOR_100_OPEN))
					continue;
				if ((our_obstacle->type >= ISO_OUTER_DOOR_V_00) && (our_obstacle->type <= ISO_OUTER_DOOR_H_100))
					continue;

				//printf("pos %f %f - border %f %f to %f %f\n", our_obstacle->pos.x, our_obstacle->pos.y, our_obstacle->pos.x + obstacle_map [ our_obstacle -> type ] . upper_border, our_obstacle->pos.y + obstacle_map [ our_obstacle -> type ] . left_border, our_obstacle->pos.x + obstacle_map [ our_obstacle -> type ] . lower_border, our_obstacle->pos.y + obstacle_map [ our_obstacle -> type ] . right_border);

				int a, b;
				for (a = rintf(our_obstacle->pos.x + obstacle_map[our_obstacle->type].upper_border);
				     (a) < (our_obstacle->pos.x + obstacle_map[our_obstacle->type].lower_border); a++) {
					for (b = rintf(our_obstacle->pos.y + obstacle_map[our_obstacle->type].left_border);
					     (b) < ((our_obstacle->pos.y + obstacle_map[our_obstacle->type].right_border)); b++) {
						if (obstacle_map[our_obstacle->type].block_area_type == COLLISION_TYPE_RECTANGLE) {
							if (obstacle_map[our_obstacle->type].block_area_parm_1 > 0.80) {
								if (Me.pos.x < our_obstacle->pos.x)
									Me.Automap[level][b][a] |= LEFT_WALL_BIT;
								else
									Me.Automap[level][b][a] |= RIGHT_WALL_BIT;
							}
							if (obstacle_map[our_obstacle->type].block_area_parm_2 > 0.80) {
								if (Me.pos.y < our_obstacle->pos.y)
									Me.Automap[level][b][a] |= UP_WALL_BIT;
								else
									Me.Automap[level][b][a] |= DOWN_WALL_BIT;
							}
						}
					}
				}
			}

			Me.Automap[level][y][x] = Me.Automap[level][y][x] | SQUARE_SEEN_AT_ALL_BIT;
		}
	}

};				// void CollectAutomapData ( void )

/**
 * The smash_obstacle function uses this function as a subfunction to 
 * check for exploding obstacles glued to one specific map square.  Of
 * course also the player number (or -1 in case of no check/bullet hit)
 * must be supplied so as to be able to suppress hits through walls or
 * the like.
 */
static int smash_obstacles_only_on_tile(float x, float y, int level, int map_x, int map_y)
{
	Level BoxLevel = curShip.AllLevels[level];
	int i;
	int target_idx;
	Obstacle target_obstacle;
	int smashed_something = FALSE;
	moderately_finepoint blast_start_pos;

	//--------------------
	// First some security checks against touching the outsides of the map...
	//
	if (!pos_inside_level(map_x, map_y, BoxLevel))
		return (FALSE);

	//--------------------
	// We check all the obstacles on this square if they are maybe destructable
	// and if they are, we destruct them, haha
	//
	for (i = 0; i < MAX_OBSTACLES_GLUED_TO_ONE_MAP_TILE; i++) {
		//--------------------
		// First we see if there is something glued to this map tile at all.
		//
		target_idx = BoxLevel->map[map_y][map_x].obstacles_glued_to_here[i];
		if (target_idx == -1)
			continue;

		target_obstacle = &(BoxLevel->obstacle_list[target_idx]);

		if (!(obstacle_map[target_obstacle->type].flags & IS_SMASHABLE))
			continue;

		//--------------------
		// Now we check if the item really was close enough to the strike target.
		// A range of 0.5 should do.
		//
		if (fabsf(x - target_obstacle->pos.x) > 0.4)
			continue;
		if (fabsf(y - target_obstacle->pos.y) > 0.4)
			continue;

		colldet_filter filter = FlyableExceptIdPassFilter;
		filter.data = &target_idx;
		gps smash_pos = { x, y, level };
		gps vsmash_pos;
		update_virtual_position(&vsmash_pos, &smash_pos, Me.pos.z);
		if (vsmash_pos.x == -1)
			continue;
		if (!DirectLineColldet(vsmash_pos.x, vsmash_pos.y, Me.pos.x, Me.pos.y, Me.pos.z, &filter)) {
			continue;
		}

		DebugPrintf(1, "\nObject smashed at: (%f/%f) by hit/explosion at (%f/%f).",
			    target_obstacle->pos.x, target_obstacle->pos.y, x, y);

		smashed_something = TRUE;

		//--------------------
		// Before we destroy the obstacle (and lose the obstacle type) we see if we
		// should maybe drop some item.
		//
		if (obstacle_map[target_obstacle->type].flags & DROPS_RANDOM_TREASURE)
			DropRandomItem(level, target_obstacle->pos.x, target_obstacle->pos.y, 1, FALSE);

		//--------------------
		// Since the obstacle is destroyed, we start a blast at it's position.
		// But here a WARNING WARNING WARNING! is due!  We must not start the
		// blast before the obstacle is removed, because the blast will again
		// cause this very obstacle removal function, so we need to be careful
		// so as not to incide endless recursion.  We memorize the position for
		// later, then delete the obstacle, then we start the blast.
		//
		blast_start_pos.x = target_obstacle->pos.x;
		blast_start_pos.y = target_obstacle->pos.y;

		//--------------------
		// Now we really smash the obstacle, i.e. we can set it's type to the debirs that has
		// been configured for this obstacle type.  In if there is nothing configured (i.e. -1 set)
		// then we'll just delete the obstacle in question entirely.  For this we got a standard function to
		// safely do it and not make some errors into the glue structure or obstacles lists...
		//
		if (obstacle_map[target_obstacle->type].result_type_after_smashing_once == (-1)) {
			action_remove_obstacle(BoxLevel, target_obstacle);
		} else {
			target_obstacle->type = obstacle_map[target_obstacle->type].result_type_after_smashing_once;
		}

		//--------------------
		// Now that the obstacle is removed AND ONLY NOW that the obstacle is
		// removed, we may start a blast at this position.  Otherwise we would
		// run into trouble, see the warning further above.
		//
		StartBlast(blast_start_pos.x, blast_start_pos.y, level, DRUIDBLAST, 2.0);

	}

	return (smashed_something);

}				// int smash_obstacles_only_on_tile ( float x , float y , int map_x , int map_y )

/**
 * When a destructable type of obstacle gets hit, e.g. by a blast 
 * exploding on the tile or a melee hit on the same floor tile, then some
 * of the obstacles (like barrel or crates) might explode, sometimes
 * leaving some treasure behind.
 *
 */
int smash_obstacle(float x, float y, int level)
{
	int map_x, map_y;
	int smash_x, smash_y;
	int smashed_something = FALSE;

	map_x = (int)rintf(x);
	map_y = (int)rintf(y);

	for (smash_y = map_y - 1; smash_y < map_y + 2; smash_y++) {
		for (smash_x = map_x - 1; smash_x < map_x + 2; smash_x++) {
			if (smash_obstacles_only_on_tile(x, y, level, smash_x, smash_y))
				smashed_something = TRUE;
		}
	}

	return (smashed_something);

}				// int smash_obstacle ( float x , float y );

/**
 * This function returns the map brick code of the tile that occupies the
 * given position.
 */
Uint16 GetMapBrick(level * lvl, float x, float y)
{
	Uint16 BrickWanted;
	int RoundX, RoundY;

	gps vpos = { x, y, lvl->levelnum };
	gps rpos;
	if (!resolve_virtual_position(&rpos, &vpos)) {
		return ISO_COMPLETELY_DARK;
	}
	RoundX = (int)rintf(rpos.x);
	RoundY = (int)rintf(rpos.y);

	//--------------------
	// Now we can return the floor tile information, but again we
	// do so with sanitiy check for the range of allowed floor tile
	// types and that...
	//
	BrickWanted = curShip.AllLevels[rpos.z]->map[RoundY][RoundX].floor_value;
	if (BrickWanted >= ALL_ISOMETRIC_FLOOR_TILES) {
		fprintf(stderr, "\nBrickWanted: %d at pos X=%d Y=%d Z=%d.", BrickWanted, RoundX, RoundY, rpos.z);
		ErrorMessage(__FUNCTION__, "\
				A maplevel in Freedroid contained a brick type, that does not have a\n\
				real graphical representation.  This is a severe error, that really \n\
				shouldn't be occuring in normal game, except perhaps if the level editor\n\
				was just used to add/remove some new doors or refreshes or other animated\n\
				map tiles.", PLEASE_INFORM, IS_FATAL);
	}

	return BrickWanted;
}

/**
 * Some structures within Freedroid rpg maps are animated in the sense
 * that the map tiles used on the secected square rotates through a number
 * of different map tile types.
 * We generate lists of them animated tiles in 
 * advance and then update only according to the current list of such
 * map tiles.  
 */
void GetAnimatedMapTiles()
{
	int i;
	int obstacle_index;
	struct list_head *target_list = NULL;
	level *Lev = CURLEVEL();

	/* At first we need to clear out the existing lists */
	struct animated_obstacle *a, *next;

	for (i = 0; i < 4; i++) {
		switch (i) {
		case 0:
			target_list = &droid_nests_head;
			break;
		case 1:
			target_list = &teleporters_head;
			break;
		case 2:
			target_list = &doors_head;
			break;
		case 3:
			target_list = &autoguns_head;
			break;
		}
		list_for_each_entry_safe(a, next, target_list, node) {
			list_del(&a->node);
			free(a);
		}
	}

	/* Now browse obstacles and fill our lists of animated obstacles. */
	for (obstacle_index = 0; obstacle_index < MAX_OBSTACLES_ON_MAP; obstacle_index++) {
		switch (Lev->obstacle_list[obstacle_index].type) {
		case ISO_H_DOOR_000_OPEN:
		case ISO_H_DOOR_025_OPEN:
		case ISO_H_DOOR_050_OPEN:
		case ISO_H_DOOR_075_OPEN:
		case ISO_H_DOOR_100_OPEN:

		case ISO_V_DOOR_000_OPEN:
		case ISO_V_DOOR_025_OPEN:
		case ISO_V_DOOR_050_OPEN:
		case ISO_V_DOOR_075_OPEN:
		case ISO_V_DOOR_100_OPEN:

		case ISO_OUTER_DOOR_V_00:
		case ISO_OUTER_DOOR_V_25:
		case ISO_OUTER_DOOR_V_50:
		case ISO_OUTER_DOOR_V_75:
		case ISO_OUTER_DOOR_V_100:

		case ISO_OUTER_DOOR_H_00:
		case ISO_OUTER_DOOR_H_25:
		case ISO_OUTER_DOOR_H_50:
		case ISO_OUTER_DOOR_H_75:
		case ISO_OUTER_DOOR_H_100:

		case ISO_DH_DOOR_000_OPEN:
		case ISO_DH_DOOR_025_OPEN:
		case ISO_DH_DOOR_050_OPEN:
		case ISO_DH_DOOR_075_OPEN:
		case ISO_DH_DOOR_100_OPEN:

		case ISO_DV_DOOR_000_OPEN:
		case ISO_DV_DOOR_025_OPEN:
		case ISO_DV_DOOR_050_OPEN:
		case ISO_DV_DOOR_075_OPEN:
		case ISO_DV_DOOR_100_OPEN:
			target_list = &doors_head;
			break;

		case ISO_REFRESH_1:
		case ISO_REFRESH_2:
		case ISO_REFRESH_3:
		case ISO_REFRESH_4:
		case ISO_REFRESH_5:
			target_list = &droid_nests_head;
			break;

		case ISO_TELEPORTER_1:
		case ISO_TELEPORTER_2:
		case ISO_TELEPORTER_3:
		case ISO_TELEPORTER_4:
		case ISO_TELEPORTER_5:
			target_list = &teleporters_head;
			break;

		case ISO_AUTOGUN_N:
		case ISO_AUTOGUN_S:
		case ISO_AUTOGUN_E:
		case ISO_AUTOGUN_W:
			target_list = &autoguns_head;
			break;

		default:
			target_list = NULL;
			break;
		}

		if (target_list) {
			struct animated_obstacle *a = MyMalloc(sizeof(struct animated_obstacle));
			a->index = obstacle_index;
			list_add(&a->node, target_list);
		}

	}

	animated_obstacles_lists_level = CURLEVEL()->levelnum;
}

/**
 * This function moves all the refresh fields to their next phase (if
 * it's time already).
 */
static void AnimateRefresh(void)
{
	static float InnerWaitCounter = 0;
	struct animated_obstacle *a;
	level *RefreshLevel = CURLEVEL();

	InnerWaitCounter += Frame_Time() * 3;

	if (RefreshLevel->levelnum != animated_obstacles_lists_level)
		GetAnimatedMapTiles();

	list_for_each_entry(a, &droid_nests_head, node) {

		switch (RefreshLevel->obstacle_list[a->index].type) {
		case ISO_REFRESH_1:
		case ISO_REFRESH_2:
		case ISO_REFRESH_3:
		case ISO_REFRESH_4:
		case ISO_REFRESH_5:
			//--------------------
			// All is well :)
			//
			break;
		default:
			ErrorMessage(__FUNCTION__, "\
			Error:  A refresh index pointing not to a refresh obstacles found.", PLEASE_INFORM, IS_FATAL);
			break;
		}

		RefreshLevel->obstacle_list[a->index].type = (((int)rintf(InnerWaitCounter)) % 5) + ISO_REFRESH_1;

	}
};				// void AnimateRefresh ( void )

/**
 * This function moves all the teleporter fields to their next phase (if
 * it's time already).
 */
static void AnimateTeleports(void)
{
	static float InnerWaitCounter = 0;
	struct animated_obstacle *a;
	level *TeleportLevel = CURLEVEL();

	InnerWaitCounter += Frame_Time() * 10;

	if (TeleportLevel->levelnum != animated_obstacles_lists_level)
		GetAnimatedMapTiles();

	list_for_each_entry(a, &teleporters_head, node) {
		switch (TeleportLevel->obstacle_list[a->index].type) {
		case ISO_TELEPORTER_1:
		case ISO_TELEPORTER_2:
		case ISO_TELEPORTER_3:
		case ISO_TELEPORTER_4:
		case ISO_TELEPORTER_5:
			//--------------------
			// All is well :)
			//
			break;
		default:
			ErrorMessage(__FUNCTION__, "\
			Error:  A teleporter index pointing not to a teleporter obstacle found.", PLEASE_INFORM, IS_FATAL);
			break;
		}

		TeleportLevel->obstacle_list[a->index].type = (((int)rintf(InnerWaitCounter)) % 5) + ISO_TELEPORTER_1;

	}

};				// void AnimateTeleports ( void )

/**
 * This function loads the data for a whole ship
 * Possible return values are : OK and ERR
 */
int LoadShip(char *filename)
{
	char *ShipData = NULL;
	FILE *ShipFile;
	char *endpt;		// Pointer to end-strings
	char *LevelStart[MAX_LEVELS];	// Pointer to a level-start
	int level_anz;
	int i;

#define END_OF_SHIP_DATA_STRING "*** End of Ship Data ***"

	for (i = 0; i < MAX_LEVELS; i++) {
		if (curShip.AllLevels[i] != NULL) {
			int ydim = curShip.AllLevels[i]->ylen;
			int row = 0;
			for (row = 0; row < ydim; row++) {
				free(curShip.AllLevels[i]->map[row]);
				curShip.AllLevels[i]->map[row] = NULL;
			}

			if (curShip.AllLevels[i]->Background_Song_Name)
				free(curShip.AllLevels[i]->Background_Song_Name);
			free(curShip.AllLevels[i]);
			curShip.AllLevels[i] = NULL;
		}
	}

	//--------------------
	// Read the whole ship-data to memory 
	//
	ShipFile = fopen(filename, "rb");
	if (!ShipFile) {
		ErrorMessage(__FUNCTION__, "Unable to open ship file %s: %s.\n", PLEASE_INFORM, IS_FATAL, filename, strerror(errno));
	}

	if (inflate_stream(ShipFile, (unsigned char **)&ShipData, NULL)) {
		ErrorMessage(__FUNCTION__, "Unable to decompress ship file %s.\n", PLEASE_INFORM, IS_FATAL, filename);
	}

	fclose(ShipFile);

	//--------------------
	// Now we count the number of levels and remember their start-addresses.
	// This is done by searching for the LEVEL_END_STRING again and again
	// until it is no longer found in the ship file.  good.
	// Note : this is actually only needed to display the progressbar
	//
	level_anz = 0;
	endpt = ShipData;
	LevelStart[level_anz] = ShipData;
	while ((endpt = strstr(endpt, LEVEL_END_STRING)) != NULL) {
		endpt += strlen(LEVEL_END_STRING);
		level_anz++;
		if (level_anz >= MAX_LEVELS)
			ErrorMessage(__FUNCTION__, "Size mismatch for level array : at least %d in file, %d in game.\n", PLEASE_INFORM,
				     IS_FATAL, level_anz + 1, MAX_LEVELS);
		LevelStart[level_anz] = endpt + 1;
	}

	//--------------------
	// Now we can start to take apart the information about each level...
	//
	for (i = 0; i < level_anz; ++i) {
		Level this_level = DecodeLoadedLeveldata(LevelStart[i]);
		int this_levelnum = this_level->levelnum;
		if (this_levelnum >= MAX_LEVELS)
			ErrorMessage(__FUNCTION__, "One levelnumber in savegame (%d) is bigger than the maximum expected (%d).\n",
				     PLEASE_INFORM, IS_FATAL, this_levelnum, MAX_LEVELS - 1);
		if (curShip.AllLevels[this_levelnum] != NULL)
			ErrorMessage(__FUNCTION__, "Two levels with same levelnumber (%d) found in the savegame.\n", PLEASE_INFORM,
				     IS_FATAL, this_levelnum);

		curShip.AllLevels[this_levelnum] = this_level;
		curShip.num_levels = this_levelnum + 1;	// levels are saved in ascending number, so the last one defines the number of levels in the ship

		//--------------------
		// The level structure contains an array with the locations of all
		// doors that might have to be opened or closed during the game.  This
		// list is prepared in advance, so that we don't have to search for doors
		// on all of the map during program runtime.
		//
		// It requires, that the obstacles have been read in already.
		//
		// Mark the list as dirty so it is regenerated automatically.
		animated_obstacles_lists_level = -1;

		//--------------------
		// We attach each obstacle to a floor tile, just so that we can sort
		// out the obstacles 'close' more easily within an array of literally
		// thousands of obstacles...
		//
		glue_obstacles_to_floor_tiles_for_level(this_levelnum);

	}

	//--------------------
	// Now that all the information has been copied, we can free the loaded data
	// again.
	//
	free(ShipData);

	// Compute the gps transform acceleration data
	gps_transform_map_dirty_flag = TRUE;
	gps_transform_map_init();

	return OK;

};				// int LoadShip ( ... ) 

/**
 * This function is intended to eliminate leading -1 entries before
 * real entries in the waypoint connection structure.
 *
 * Such leading -1 entries might cause problems later, because some
 * Enemy-Movement routines expect that the "real" entries are the
 * first entries in the connection list.
 */
static void CheckWaypointIntegrity(Level Lev)
{
	int i, j, k, l;

	for (i = 0; i < MAXWAYPOINTS; i++) {
		// Search for the first -1 entry:  j contains this number
		for (j = 0; j < MAX_WP_CONNECTIONS; j++) {
			if (Lev->AllWaypoints[i].connections[j] == -1)
				break;
		}

		// have only non-(-1)-entries?  then we needn't do anything.
		if (j == MAX_WP_CONNECTIONS)
			continue;
		// have only one (-1) entry in the last position?  then we needn't do anything.
		if (j == MAX_WP_CONNECTIONS - 1)
			continue;

		// search for the next non-(-1)-entry AFTER the -1 entry fount first
		for (k = j + 1; k < MAX_WP_CONNECTIONS; k++) {
			if (Lev->AllWaypoints[i].connections[k] != -1)
				break;
		}

		// none found? -- that would be good.  no corrections nescessary.  we can go.
		if (k == MAX_WP_CONNECTIONS)
			continue;

		// At this point we have found a non-(-1)-entry after a -1 entry.  that means work!!

		DebugPrintf(0, "\n WARNING!! INCONSISTENSY FOUND ON LEVEL %d!! ", Lev->levelnum);
		DebugPrintf(0, "\n NUMBER OF LEADING -1 ENTRIES: %d!! ", k - j);
		DebugPrintf(0, "\n COMPENSATION ACTIVATED...");

		// So we move the later waypoints just right over the existing leading -1 entries

		for (l = j; l < MAX_WP_CONNECTIONS - (k - j); l++) {
			Lev->AllWaypoints[i].connections[l] = Lev->AllWaypoints[i].connections[l + (k - j)];
		}

		// So the block of leading -1 entries has been eliminated
		// BUT:  This may have introduced double entries of waypoints, e.g. if there was a -1
		// at the start and all other entries filled!  WE DO NOT HANDLE THIS CASE.  SORRY.
		// Also there might be a second sequence of -1 entries followed by another non-(-1)-entry
		// sequence.  SORRY, THAT CASE WILL ALSO NOT BE HANDLES SEPARATELY.  Maybe later.
		// For now this function will do perfectly well as it is now.

	}

};				// void CheckWaypointIntegrity(Level Lev)

/**
 * This should write the obstacle information in human-readable form into
 * a buffer.
 */
static void encode_obstacles_of_this_level(char *LevelMem, Level Lev)
{
	int i;
	strcat(LevelMem, OBSTACLE_DATA_BEGIN_STRING);
	strcat(LevelMem, "\n");
	LevelMem += strlen(LevelMem);

	for (i = 0; i < MAX_OBSTACLES_ON_MAP; i++) {
		if (Lev->obstacle_list[i].type == (-1))
			continue;

		sprintf(LevelMem, "%s%d %s%3.2f %s%3.2f %s%d %s%d \n", OBSTACLE_TYPE_STRING, Lev->obstacle_list[i].type,
			OBSTACLE_X_POSITION_STRING, Lev->obstacle_list[i].pos.x, OBSTACLE_Y_POSITION_STRING,
			Lev->obstacle_list[i].pos.y, OBSTACLE_LABEL_INDEX_STRING, Lev->obstacle_list[i].name_index,
			OBSTACLE_DESCRIPTION_INDEX_STRING, Lev->obstacle_list[i].description_index);
		LevelMem += strlen(LevelMem);

	}

	strcat(LevelMem, OBSTACLE_DATA_END_STRING);
	strcat(LevelMem, "\n");

};				// void encode_obstacles_of_this_level ( LevelMem , Lev )

/**
 * This function adds the statement data of this level to the chunk of 
 * data that will be written out to a file later.
 */
static void EncodeMapLabelsOfThisLevel(char *LevelMem, Level Lev)
{
	int i;
	strcat(LevelMem, MAP_LABEL_BEGIN_STRING);
	strcat(LevelMem, "\n");
	LevelMem += strlen(LevelMem);

	for (i = 0; i < MAX_MAP_LABELS_PER_LEVEL; i++) {
		if (Lev->labels[i].pos.x == (-1))
			continue;

		sprintf(LevelMem, "%s%d %s%d %s%s\"\n", X_POSITION_OF_LABEL_STRING, Lev->labels[i].pos.x, Y_POSITION_OF_LABEL_STRING,
			Lev->labels[i].pos.y, LABEL_ITSELF_ANNOUNCE_STRING, Lev->labels[i].label_name);

		LevelMem += strlen(LevelMem);
	}

	strcat(LevelMem, MAP_LABEL_END_STRING);
	strcat(LevelMem, "\n");

};				// void EncodeMapLabelsOfThisLevel ( char* LevelMem , Level Lev )

/**
 * Every map level in a FreedroidRPG 'ship' can have up to 
 * MAX_OBSTACLE_NAMES_PER_LEVEL obstacles, that have a label attached to
 * them.  Such obstacle labels are very useful when modifying obstacles
 * from within game events and triggers.  The obstacle labels are stored
 * in a small subsection of the whole level data.  This function encodes
 * this small subsection and puts all the obstacle data into a human
 * readable text string for saving with the map file.
 */
static void encode_obstacle_names_of_this_level(char *LevelMem, Level Lev)
{
	int i;

	strcat(LevelMem, OBSTACLE_LABEL_BEGIN_STRING);
	strcat(LevelMem, "\n");
	LevelMem += strlen(LevelMem);

	for (i = 0; i < MAX_OBSTACLE_NAMES_PER_LEVEL; i++) {
		if (Lev->obstacle_name_list[i] == NULL)
			continue;
		LevelMem +=
		    sprintf(LevelMem, "%s%d %s%s\"", INDEX_OF_OBSTACLE_NAME, i, OBSTACLE_LABEL_ANNOUNCE_STRING, Lev->obstacle_name_list[i]);
		*LevelMem++ = '\n';
	}

	strcat(LevelMem, OBSTACLE_LABEL_END_STRING);
	strcat(LevelMem, "\n");

};				// void encode_obstacle_names_of_this_level ( char* LevelMem , Level Lev )

/**
 * Every map level in a FreedroidRPG 'ship' can have up to 
 * MAX_OBSTACLE_DESCRIPTIONS_PER_LEVEL obstacles, that have a label 
 * attached to them.  
 * These obstacle descriptions will be displayed, if present, instead of
 * the default generic obstacle-type specific obstacle explanation.
 * For examply, traffic signs with the same obstacle type being used in
 * multiple places shouldn't read 'this is a traffic sign' all the time,
 * but should have individual route names even though the obstacle type
 * is the same every time.  That's what such individual obstacle 
 * descriptions are for.
 *
 * The obstacle descriptions are stored in a small subsection of the 
 * whole level data.  This function encodes this small subsection and 
 * puts all the obstacle data into a human readable text string for 
 * saving with the map file.
 *
 */
static void encode_obstacle_descriptions_of_this_level(char *LevelMem, Level Lev)
{
	int i;
	strcat(LevelMem, OBSTACLE_DESCRIPTION_BEGIN_STRING);
	strcat(LevelMem, "\n");
	LevelMem += strlen(LevelMem);

	for (i = 0; i < MAX_OBSTACLE_DESCRIPTIONS_PER_LEVEL; i++) {
		if (Lev->obstacle_description_list[i] == NULL)
			continue;
		sprintf(LevelMem, "%s%d %s%s\"\n", INDEX_OF_OBSTACLE_DESCRIPTION, i, OBSTACLE_DESCRIPTION_ANNOUNCE_STRING,
			Lev->obstacle_description_list[i]);
		LevelMem += strlen(LevelMem);
	}

	strcat(LevelMem, OBSTACLE_DESCRIPTION_END_STRING);
	strcat(LevelMem, "\n\n");

};				// void encode_obstacle_descriptions_of_this_level ( char* LevelMem , Level Lev )

/**
 *
 * 
 */
static void WriteOutOneItem(char *LevelMem, Item ItemToWriteOut)
{
	char linebuf[5000];

	strcat(LevelMem, ITEM_NAME_STRING);
	sprintf(linebuf, "%s\" ", ItemMap[ItemToWriteOut->type].item_name);
	strcat(LevelMem, linebuf);

	strcat(LevelMem, ITEM_POS_X_STRING);
	sprintf(linebuf, "%f ", ItemToWriteOut->pos.x);
	strcat(LevelMem, linebuf);

	strcat(LevelMem, ITEM_POS_Y_STRING);
	sprintf(linebuf, "%f ", ItemToWriteOut->pos.y);
	strcat(LevelMem, linebuf);

	if (ItemToWriteOut->ac_bonus) {
		strcat(LevelMem, ITEM_AC_BONUS_STRING);
		sprintf(linebuf, "%d ", ItemToWriteOut->ac_bonus);
		strcat(LevelMem, linebuf);
	}

	strcat(LevelMem, ITEM_DAMAGE_STRING);
	sprintf(linebuf, "%d ", ItemToWriteOut->damage);
	strcat(LevelMem, linebuf);

	strcat(LevelMem, ITEM_DAMAGE_MODIFIER_STRING);
	sprintf(linebuf, "%d ", ItemToWriteOut->damage_modifier);
	strcat(LevelMem, linebuf);

	strcat(LevelMem, ITEM_MAX_DURATION_STRING);
	sprintf(linebuf, "%d ", ItemToWriteOut->max_duration);
	strcat(LevelMem, linebuf);

	strcat(LevelMem, ITEM_CUR_DURATION_STRING);
	sprintf(linebuf, "%f ", ItemToWriteOut->current_duration);
	strcat(LevelMem, linebuf);

	strcat(LevelMem, ITEM_AMMO_CLIP_STRING);
	sprintf(linebuf, "%d ", ItemToWriteOut->ammo_clip);
	strcat(LevelMem, linebuf);

	strcat(LevelMem, ITEM_MULTIPLICITY_STRING);
	sprintf(linebuf, "%d ", ItemToWriteOut->multiplicity);
	strcat(LevelMem, linebuf);

	if (ItemToWriteOut->prefix_code != -1) {
		strcat(LevelMem, ITEM_PREFIX_CODE_STRING);
		sprintf(linebuf, "%d ", ItemToWriteOut->prefix_code);
		strcat(LevelMem, linebuf);
	}

	if (ItemToWriteOut->suffix_code != -1) {
		strcat(LevelMem, ITEM_SUFFIX_CODE_STRING);
		sprintf(linebuf, "%d ", ItemToWriteOut->suffix_code);
		strcat(LevelMem, linebuf);
	}

	if (ItemToWriteOut->bonus_to_str) {
		strcat(LevelMem, ITEM_BONUS_TO_STR_STRING);
		sprintf(linebuf, "%d ", ItemToWriteOut->bonus_to_str);
		strcat(LevelMem, linebuf);
	}
	if (ItemToWriteOut->bonus_to_dex) {
		strcat(LevelMem, ITEM_BONUS_TO_DEX_STRING);
		sprintf(linebuf, "%d ", ItemToWriteOut->bonus_to_dex);
		strcat(LevelMem, linebuf);
	}
	if (ItemToWriteOut->bonus_to_vit) {
		strcat(LevelMem, ITEM_BONUS_TO_VIT_STRING);
		sprintf(linebuf, "%d ", ItemToWriteOut->bonus_to_vit);
		strcat(LevelMem, linebuf);
	}
	if (ItemToWriteOut->bonus_to_mag) {
		strcat(LevelMem, ITEM_BONUS_TO_MAG_STRING);
		sprintf(linebuf, "%d ", ItemToWriteOut->bonus_to_mag);
		strcat(LevelMem, linebuf);
	}
	if (ItemToWriteOut->bonus_to_all_attributes) {
		strcat(LevelMem, ITEM_BONUS_TO_ALLATT_STRING);
		sprintf(linebuf, "%d ", ItemToWriteOut->bonus_to_all_attributes);
		strcat(LevelMem, linebuf);
	}
	// Now we save the secondary stat boni

	if (ItemToWriteOut->bonus_to_life) {
		strcat(LevelMem, ITEM_BONUS_TO_LIFE_STRING);
		sprintf(linebuf, "%d ", ItemToWriteOut->bonus_to_life);
		strcat(LevelMem, linebuf);
	}
	if (ItemToWriteOut->bonus_to_health_recovery) {
		strcat(LevelMem, ITEM_BONUS_TO_HEALTH_RECOVERY_STRING);
		sprintf(linebuf, "%f ", ItemToWriteOut->bonus_to_health_recovery);
		strcat(LevelMem, linebuf);
	}
	if (ItemToWriteOut->bonus_to_force) {
		strcat(LevelMem, ITEM_BONUS_TO_FORCE_STRING);
		sprintf(linebuf, "%d ", ItemToWriteOut->bonus_to_force);
		strcat(LevelMem, linebuf);
	}
	if (ItemToWriteOut->bonus_to_cooling_rate) {
		strcat(LevelMem, ITEM_BONUS_TO_MANA_RECOVERY_STRING);
		sprintf(linebuf, "%f ", ItemToWriteOut->bonus_to_cooling_rate);
		strcat(LevelMem, linebuf);
	}
	if (ItemToWriteOut->bonus_to_tohit) {
		strcat(LevelMem, ITEM_BONUS_TO_TOHIT_STRING);
		sprintf(linebuf, "%d ", ItemToWriteOut->bonus_to_tohit);
		strcat(LevelMem, linebuf);
	}
	if (ItemToWriteOut->bonus_to_ac_or_damage) {
		strcat(LevelMem, ITEM_BONUS_TO_ACDAM_STRING);
		sprintf(linebuf, "%d ", ItemToWriteOut->bonus_to_ac_or_damage);
		strcat(LevelMem, linebuf);
	}
	// Now we save the resistanc boni

	if (ItemToWriteOut->bonus_to_resist_electricity) {
		strcat(LevelMem, ITEM_BONUS_TO_RESELE_STRING);
		sprintf(linebuf, "%d ", ItemToWriteOut->bonus_to_resist_electricity);
		strcat(LevelMem, linebuf);
	}
	if (ItemToWriteOut->bonus_to_resist_disruptor) {
		strcat(LevelMem, ITEM_BONUS_TO_RESFOR_STRING);
		sprintf(linebuf, "%d ", ItemToWriteOut->bonus_to_resist_disruptor);
		strcat(LevelMem, linebuf);
	}
	if (ItemToWriteOut->bonus_to_resist_fire) {
		strcat(LevelMem, ITEM_BONUS_TO_RESFIR_STRING);
		sprintf(linebuf, "%d ", ItemToWriteOut->bonus_to_resist_fire);
		strcat(LevelMem, linebuf);
	}
	strcat(LevelMem, ITEM_IS_IDENTIFIED_STRING);
	sprintf(linebuf, "%d ", ItemToWriteOut->is_identified);
	strcat(LevelMem, linebuf);

	strcat(LevelMem, "\n");

};				// void WriteOutOneItem ( LevelMem , ItemToWriteOut ) 

/**
 *
 */
static void EncodeItemSectionOfThisLevel(char *LevelMem, Level Lev)
{
	// char linebuf[5000];        // Buffer 
	int i;

	//--------------------
	// Now we write out a marker to announce the beginning of the items data
	//
	strcat(LevelMem, ITEMS_SECTION_BEGIN_STRING);
	strcat(LevelMem, "\n");

	//--------------------
	// Now we write out the bulk of items infos
	//
	for (i = 0; i < MAX_ITEMS_PER_LEVEL; i++) {
		if (Lev->ItemList[i].type == (-1))
			continue;

		WriteOutOneItem(LevelMem, &(Lev->ItemList[i]));

	}
	//--------------------
	// Now we write out a marker to announce the end of the items data
	//
	strcat(LevelMem, ITEMS_SECTION_END_STRING);
	strcat(LevelMem, "\n");

};				// void EncodeItemSectionOfThisLevel ( LevelMem , Lev ) 

/**
 *
 */
static void EncodeChestItemSectionOfThisLevel(char *LevelMem, Level Lev)
{
	// char linebuf[5000];        // Buffer 
	int i;

	//--------------------
	// Now we write out a marker to announce the beginning of the items data
	//
	strcat(LevelMem, CHEST_ITEMS_SECTION_BEGIN_STRING);
	strcat(LevelMem, "\n");

	//--------------------
	// Now we write out the bulk of items infos
	//
	for (i = 0; i < MAX_ITEMS_PER_LEVEL; i++) {
		if (Lev->ChestItemList[i].type == (-1))
			continue;

		WriteOutOneItem(LevelMem, &(Lev->ChestItemList[i]));

	}
	//--------------------
	// Now we write out a marker to announce the end of the items data
	//
	strcat(LevelMem, CHEST_ITEMS_SECTION_END_STRING);
	strcat(LevelMem, "\n");

};				// void EncodeChestItemSectionOfThisLevel ( LevelMem , Lev ) 

/**
 * This function generates savable text out of the current lavel data
 */
static char *EncodeLevelForSaving(Level Lev)
{
	char *LevelMem;
	int i, j;
	unsigned int MemAmount = 0;	// the size of the level-data 
	int xlen = Lev->xlen, ylen = Lev->ylen;
	int anz_wp;		// number of Waypoints 
	char linebuf[5000];	// Buffer 
	waypoint *this_wp;
	Uint16 HumanReadableMapLine[10000];

	// Get the number of waypoints 
	anz_wp = 0;
	while (Lev->AllWaypoints[anz_wp++].x != 0) ;
	anz_wp--;		// we counted one too much 

	// estimate the amount of memory needed 
	MemAmount = (xlen + 1) * (ylen + 1);	// Map-memory 
	MemAmount += MAXWAYPOINTS * MAX_WP_CONNECTIONS * 4;
	MemAmount += 1000000;	// add some safety buffer for dimension-strings and marker strings...

	//--------------------
	// We allocate some memory, such that we should be able to fill
	// all the data for the level into it.  If this estimate is wrong,
	// we'll simply panic further below...
	//
	if ((LevelMem = (char *)MyMalloc(MemAmount)) == NULL) {
		ErrorMessage(__FUNCTION__, "Did not get any more memory.", PLEASE_INFORM, IS_FATAL);
		return (NULL);
	}
	//--------------------
	// Write the data to memory:
	// Here the levelnumber and general information about the level is written
	sprintf(linebuf, "Levelnumber: %d\n\
xlen of this level: %d\n\
ylen of this level: %d\n\
light radius bonus of this level: %d\n\
minimal light on this level: %d\n\
infinite_running_on_this_level: %d\n\
random dungeon: %d\n\
jump threshold north: %d\n\
jump threshold south: %d\n\
jump threshold east: %d\n\
jump threshold west: %d\n\
jump target north: %d\n\
jump target south: %d\n\
jump target east: %d\n\
jump target west: %d\n\
use underground lighting: %d\n", Lev->levelnum, Lev->xlen, Lev->ylen, Lev->light_radius_bonus, Lev->minimum_light_value, Lev->infinite_running_on_this_level, Lev->random_dungeon, Lev->jump_threshold_north, Lev->jump_threshold_south, Lev->jump_threshold_east, Lev->jump_threshold_west, Lev->jump_target_north, Lev->jump_target_south, Lev->jump_target_east, Lev->jump_target_west, Lev->use_underground_lighting);
	strcpy(LevelMem, linebuf);
	strcat(LevelMem, LEVEL_NAME_STRING);
	strcat(LevelMem, Lev->Levelname);
	strcat(LevelMem, "\"\n");
	strcat(LevelMem, BACKGROUND_SONG_NAME_STRING);
	strcat(LevelMem, Lev->Background_Song_Name);
	strcat(LevelMem, "\n");

	// Now the beginning of the actual map data is marked:
	strcat(LevelMem, MAP_BEGIN_STRING);
	strcat(LevelMem, "\n");

	// Now in the loop each line of map data should be saved as a whole
	for (i = 0; i < ylen; i++) {
		//--------------------
		// But before we can write this line of the map to the disk, we need to
		// convert is back to human readable format.
		//
		if (!Lev->random_dungeon) {
			TranslateToHumanReadable(HumanReadableMapLine, Lev->map[i], xlen);
			strncat(LevelMem, (char *)HumanReadableMapLine, xlen * 4 * 2);	// We need FOUR , no EIGHT chars per map tile
			strcat(LevelMem, "\n");
		} else {
			int j = xlen;
			while (j--) {
				strcat(LevelMem, "  4 ");
			}
			strcat(LevelMem, "\n");
		}
	}

	//--------------------
	// Now we write out a marker at the end of the map data.  This marker is not really
	// vital for reading in the file again, but it adds clearness to the files structure.
	strcat(LevelMem, MAP_END_STRING);
	strcat(LevelMem, "\n");

	if (!Lev->random_dungeon) {
		encode_obstacles_of_this_level(LevelMem, Lev);

		EncodeMapLabelsOfThisLevel(LevelMem, Lev);

		encode_obstacle_names_of_this_level(LevelMem, Lev);

		encode_obstacle_descriptions_of_this_level(LevelMem, Lev);

		EncodeItemSectionOfThisLevel(LevelMem, Lev);

		EncodeChestItemSectionOfThisLevel(LevelMem, Lev);
	}
	// --------------------  
	// The next thing we must do is write the waypoints of this level also
	// to disk.

	// There might be LEADING -1 entries in front of other connection entries.
	// This is unwanted and shall be corrected here.
	CheckWaypointIntegrity(Lev);

	strcat(LevelMem, WP_SECTION_BEGIN_STRING);
	strcat(LevelMem, "\n");

	for (i = 0; i < Lev->num_waypoints; i++) {
		sprintf(linebuf, "Nr.=%3d x=%4d y=%4d rnd=%1d", i,
			Lev->AllWaypoints[i].x, Lev->AllWaypoints[i].y, Lev->AllWaypoints[i].suppress_random_spawn);
		strcat(LevelMem, linebuf);
		strcat(LevelMem, "\t ");
		strcat(LevelMem, CONNECTION_STRING);

		this_wp = &Lev->AllWaypoints[i];
		for (j = 0; j < this_wp->num_connections; j++) {
			sprintf(linebuf, " %3d", Lev->AllWaypoints[i].connections[j]);
			strcat(LevelMem, linebuf);
		}		// for connections 
		strcat(LevelMem, "\n");
	}			// for waypoints 

	strcat(LevelMem, LEVEL_END_STRING);
	strcat(LevelMem, "\n----------------------------------------------------------------------\n");

	//--------------------
	// So we're done now.  Did the estimate for the amount of memory hit
	// the target or was it at least sufficient? 
	// If not, we're in trouble...
	//
	if (strlen(LevelMem) >= MemAmount) {
		ErrorMessage(__FUNCTION__, "Estimate of needed memory was wrong!  How stupid!", PLEASE_INFORM, IS_FATAL);
		return (NULL);
	}
	//--------------------
	// all ok : 
	//
	return LevelMem;

};				// char *EncodeLevelForSaving ( Level Lev )

/**
 * This function should save a whole ship to disk to the given filename.
 * It is not only used by the level editor, but also by the function that
 * saves whole games.
 */
int SaveShip(const char *filename)
{
	int i;
	char *LevelMem = NULL;
	FILE *ShipFile = NULL;
	struct auto_string *shipstr;

	// Open the ship file 
	if ((ShipFile = fopen(filename, "wb")) == NULL) {
		ErrorMessage(__FUNCTION__, "Error opening ship file %s.", PLEASE_INFORM, IS_FATAL, filename);
		return ERR;
	}

	shipstr	= alloc_autostr(1048576);
	autostr_printf(shipstr, "\n");
	
	// Save all the levels
	for (i = 0; i < curShip.num_levels; i++) {
		if (curShip.AllLevels[i] != NULL) {
			LevelMem = EncodeLevelForSaving(curShip.AllLevels[i]);
			autostr_append(shipstr, "%s", LevelMem);
			free(LevelMem);
		}
	}

	autostr_append(shipstr, "%s\n\n", END_OF_SHIP_DATA_STRING);

	deflate_to_stream((unsigned char *)shipstr->value, shipstr->length+1, ShipFile);

	if (fclose(ShipFile) == EOF) {
		ErrorMessage(__FUNCTION__, "Closing of ship file failed!", PLEASE_INFORM, IS_FATAL);
		return ERR;
	}

	free_autostr(shipstr);
	return OK;
}

/**
 *
 *
 */
static void ReadInOneItem(char *ItemPointer, char *ItemsSectionEnd, Item TargetItem)
{

	char *iname = ReadAndMallocStringFromData(ItemPointer, ITEM_NAME_STRING, "\"");
	TargetItem->type = GetItemIndexByName(iname);
	free(iname);

	ReadValueFromString(ItemPointer, ITEM_POS_X_STRING, "%lf", &(TargetItem->pos.x), ItemsSectionEnd);
	ReadValueFromString(ItemPointer, ITEM_POS_Y_STRING, "%lf", &(TargetItem->pos.y), ItemsSectionEnd);
	ReadValueFromStringWithDefault(ItemPointer, ITEM_AC_BONUS_STRING, "%d", "0", &(TargetItem->ac_bonus), ItemsSectionEnd);
	ReadValueFromString(ItemPointer, ITEM_DAMAGE_STRING, "%d", &(TargetItem->damage), ItemsSectionEnd);
	ReadValueFromString(ItemPointer, ITEM_DAMAGE_MODIFIER_STRING, "%d", &(TargetItem->damage_modifier), ItemsSectionEnd);
	ReadValueFromString(ItemPointer, ITEM_MAX_DURATION_STRING, "%d", &(TargetItem->max_duration), ItemsSectionEnd);
	ReadValueFromString(ItemPointer, ITEM_CUR_DURATION_STRING, "%f", &(TargetItem->current_duration), ItemsSectionEnd);
	ReadValueFromString(ItemPointer, ITEM_AMMO_CLIP_STRING, "%d", &(TargetItem->ammo_clip), ItemsSectionEnd);
	ReadValueFromString(ItemPointer, ITEM_MULTIPLICITY_STRING, "%d", &(TargetItem->multiplicity), ItemsSectionEnd);
	ReadValueFromStringWithDefault(ItemPointer, ITEM_PREFIX_CODE_STRING, "%d", "-1", &(TargetItem->prefix_code), ItemsSectionEnd);
	ReadValueFromStringWithDefault(ItemPointer, ITEM_SUFFIX_CODE_STRING, "%d", "-1", &(TargetItem->suffix_code), ItemsSectionEnd);
	// Now we read in the boni to the primary stats (attributes)
	ReadValueFromStringWithDefault(ItemPointer, ITEM_BONUS_TO_STR_STRING, "%d", "0", &(TargetItem->bonus_to_str), ItemsSectionEnd);
	ReadValueFromStringWithDefault(ItemPointer, ITEM_BONUS_TO_DEX_STRING, "%d", "0", &(TargetItem->bonus_to_dex), ItemsSectionEnd);
	ReadValueFromStringWithDefault(ItemPointer, ITEM_BONUS_TO_MAG_STRING, "%d", "0", &(TargetItem->bonus_to_mag), ItemsSectionEnd);
	ReadValueFromStringWithDefault(ItemPointer, ITEM_BONUS_TO_VIT_STRING, "%d", "0", &(TargetItem->bonus_to_vit), ItemsSectionEnd);
	ReadValueFromStringWithDefault(ItemPointer, ITEM_BONUS_TO_ALLATT_STRING, "%d", "0",
				       &(TargetItem->bonus_to_all_attributes), ItemsSectionEnd);
	// Now we read in the boni for the secondary stats
	ReadValueFromStringWithDefault(ItemPointer, ITEM_BONUS_TO_LIFE_STRING, "%d", "0", &(TargetItem->bonus_to_life), ItemsSectionEnd);
	ReadValueFromStringWithDefault(ItemPointer, ITEM_BONUS_TO_HEALTH_RECOVERY_STRING, "%f", "0.000",
				       &(TargetItem->bonus_to_health_recovery), ItemsSectionEnd);
	ReadValueFromStringWithDefault(ItemPointer, ITEM_BONUS_TO_FORCE_STRING, "%d", "0", &(TargetItem->bonus_to_force), ItemsSectionEnd);
	ReadValueFromStringWithDefault(ItemPointer, ITEM_BONUS_TO_MANA_RECOVERY_STRING, "%f", "0.000",
				       &(TargetItem->bonus_to_cooling_rate), ItemsSectionEnd);
	ReadValueFromStringWithDefault(ItemPointer, ITEM_BONUS_TO_TOHIT_STRING, "%d", "0", &(TargetItem->bonus_to_tohit), ItemsSectionEnd);
	ReadValueFromStringWithDefault(ItemPointer, ITEM_BONUS_TO_ACDAM_STRING, "%d", "0",
				       &(TargetItem->bonus_to_ac_or_damage), ItemsSectionEnd);
	// Now we read in the boni for resistances
	ReadValueFromStringWithDefault(ItemPointer, ITEM_BONUS_TO_RESELE_STRING, "%d", "0",
				       &(TargetItem->bonus_to_resist_electricity), ItemsSectionEnd);
	ReadValueFromStringWithDefault(ItemPointer, ITEM_BONUS_TO_RESFIR_STRING, "%d", "0",
				       &(TargetItem->bonus_to_resist_fire), ItemsSectionEnd);
	ReadValueFromStringWithDefault(ItemPointer, ITEM_BONUS_TO_RESFOR_STRING, "%d", "0",
				       &(TargetItem->bonus_to_resist_disruptor), ItemsSectionEnd);
	// Now we see if the item is identified...
	ReadValueFromString(ItemPointer, ITEM_IS_IDENTIFIED_STRING, "%d", &(TargetItem->is_identified), ItemsSectionEnd);

	DebugPrintf(1, "\nPosX=%f PosY=%f Item=%d", TargetItem->pos.x, TargetItem->pos.y, TargetItem->type);

};				// void ReadInOneItem ( ItemPointer , &(loadlevel->ItemList[ i ]) )

//----------------------------------------------------------------------
// From here on we take apart the items section of the loaded level...
//----------------------------------------------------------------------
static void DecodeItemSectionOfThisLevel(Level loadlevel, char *data)
{
	int i;
	char Preserved_Letter;
	int NumberOfItemsInThisLevel;
	char *ItemPointer;
	char *ItemsSectionBegin;
	char *ItemsSectionEnd;

	//--------------------
	// First we initialize the items arrays with 'empty' information
	//
	for (i = 0; i < MAX_ITEMS_PER_LEVEL; i++) {
		loadlevel->ItemList[i].pos.x = (-1);
		loadlevel->ItemList[i].pos.y = (-1);
		loadlevel->ItemList[i].type = (-1);
		loadlevel->ItemList[i].currently_held_in_hand = FALSE;
	}

	if (loadlevel->random_dungeon)
		return;

	// We look for the beginning and end of the items section
	ItemsSectionBegin = LocateStringInData(data, ITEMS_SECTION_BEGIN_STRING);
	ItemsSectionEnd = LocateStringInData(ItemsSectionBegin, ITEMS_SECTION_END_STRING);

	// We add a terminator at the end of the items section, but ONLY TEMPORARY.  
	// The damage will be restored later!
	Preserved_Letter = ItemsSectionEnd[0];
	ItemsSectionEnd[0] = 0;
	NumberOfItemsInThisLevel = CountStringOccurences(ItemsSectionBegin, ITEM_NAME_STRING);
	DebugPrintf(1, "\nNumber of items found in this level : %d.", NumberOfItemsInThisLevel);

	// Now we decode all the item information
	ItemPointer = ItemsSectionBegin;
	char *NextItemPointer;
	for (i = 0; i < NumberOfItemsInThisLevel; i++) {
		if ((ItemPointer = strstr(ItemPointer + 1, ITEM_NAME_STRING))) {
			NextItemPointer = strstr(ItemPointer + 1, ITEM_NAME_STRING);
			if (NextItemPointer)
				NextItemPointer[0] = 0;
			ReadInOneItem(ItemPointer, ItemsSectionEnd, &(loadlevel->ItemList[i]));
			if (NextItemPointer)
				NextItemPointer[0] = ITEM_NAME_STRING[0];
		}
	}

	// Now we repair the damage done to the loaded level data
	ItemsSectionEnd[0] = Preserved_Letter;
};				// void DecodeItemSectionOfThisLevel ( Level loadlevel , char* data )

//----------------------------------------------------------------------
// From here on we take apart the chest items section of the loaded level...
//----------------------------------------------------------------------
static void DecodeChestItemSectionOfThisLevel(Level loadlevel, char *data)
{
	int i;
	char Preserved_Letter;
	int NumberOfItemsInThisLevel;
	char *ItemPointer;
	char *ItemsSectionBegin;
	char *ItemsSectionEnd;
	char *NextItemPointer;
	//--------------------
	// First we initialize the items arrays with 'empty' information
	//
	for (i = 0; i < MAX_ITEMS_PER_LEVEL; i++) {
		loadlevel->ChestItemList[i].pos.x = (-1);
		loadlevel->ChestItemList[i].pos.y = (-1);
		loadlevel->ChestItemList[i].type = (-1);
		loadlevel->ChestItemList[i].currently_held_in_hand = FALSE;
	}

	if (loadlevel->random_dungeon)
		return;

	// We look for the beginning and end of the items section
	ItemsSectionBegin = LocateStringInData(data, CHEST_ITEMS_SECTION_BEGIN_STRING);
	ItemsSectionEnd = LocateStringInData(ItemsSectionBegin, CHEST_ITEMS_SECTION_END_STRING);

	// We add a terminator at the end of the items section, but ONLY TEMPORARY.  
	// The damage will be restored later!
	Preserved_Letter = ItemsSectionEnd[0];
	ItemsSectionEnd[0] = 0;
	NumberOfItemsInThisLevel = CountStringOccurences(ItemsSectionBegin, ITEM_NAME_STRING);
	DebugPrintf(1, "\nNumber of chest items found in this level : %d.", NumberOfItemsInThisLevel);

	// Now we decode all the item information
	ItemPointer = ItemsSectionBegin;
	for (i = 0; i < NumberOfItemsInThisLevel; i++) {
		if ((ItemPointer = strstr(ItemPointer + 1, ITEM_NAME_STRING))) {
			NextItemPointer = strstr(ItemPointer + 1, ITEM_NAME_STRING);
			if (NextItemPointer)
				NextItemPointer[0] = 0;
			ReadInOneItem(ItemPointer, ItemsSectionEnd, &(loadlevel->ChestItemList[i]));
			if (NextItemPointer)
				NextItemPointer[0] = ITEM_NAME_STRING[0];
		}
	}

	// Now we repair the damage done to the loaded level data
	ItemsSectionEnd[0] = Preserved_Letter;

};				// void DecodeChestItemSectionOfThisLevel ( Level loadlevel , char* data )

/**
 * This function is for LOADING map data!
 * This function extracts the data from *data and writes them 
 * into a Level-struct:
 *
 * NOTE:  Here, the map-data are NOT yet translated to their 
 *        their internal values, like "VOID", "H_OPEN_DOOR" and
 *         all the other values from the defs.h file.
 *
 * Doors and Waypoints Arrays are initialized too
 *
 * @Ret:  Level or NULL
 */
Level DecodeLoadedLeveldata(char *data)
{
	Level loadlevel;
	char *pos;
	char *map_begin, *wp_begin;
	int i;
	int nr, x, y, wp_rnd;
	int k;
	int connection;
	char *this_line;
	char *DataPointer;
	char *level_end;
	int res;

	//--------------------
	// Get the memory for one level 
	//
	loadlevel = (Level) MyMalloc(sizeof(level));

	DebugPrintf(2, "\n-----------------------------------------------------------------");
	DebugPrintf(2, "\nStarting to process information for another level:\n");

	//--------------------
	// Read Header Data: levelnum and x/ylen 
	//
	DataPointer = strstr(data, "Levelnumber:");
	if (DataPointer == NULL) {
		ErrorMessage(__FUNCTION__, "No levelnumber entry found!", PLEASE_INFORM, IS_FATAL);
	}

	DecodeInterfaceDataForThisLevel(loadlevel, DataPointer);

	DecodeDimensionsOfThisLevel(loadlevel, DataPointer);

	//--------------------
	// Read the levelname.
	// Accept legacy ship-files that are not yet marked-up for translation
	if ((loadlevel->Levelname = ReadAndMallocStringFromDataOptional(data, LEVEL_NAME_STRING, "\"", 0)) == NULL) {
		loadlevel->Levelname = ReadAndMallocStringFromData(data, LEVEL_NAME_STRING_LEGACY, "\n");
	}

	loadlevel->Background_Song_Name = ReadAndMallocStringFromData(data, BACKGROUND_SONG_NAME_STRING, "\n");

	decode_obstacles_of_this_level(loadlevel, DataPointer);

	DecodeMapLabelsOfThisLevel(loadlevel, DataPointer);

	decode_obstacle_names_of_this_level(loadlevel, DataPointer);

	decode_obstacle_descriptions_of_this_level(loadlevel, DataPointer);

	//--------------------
	// Next we extract the statments of the influencer on this level WITHOUT destroying
	// or damaging the data in the process!
	//
	DecodeItemSectionOfThisLevel(loadlevel, data);

	DecodeChestItemSectionOfThisLevel(loadlevel, data);

	//--------------------
	// find the map data
	// NOTE, that we here only set up a pointer to the map data
	// as they are stored in the file.  This is NOT the same format
	// as the map data stored internally for the game, but rather
	// an easily human readable format with acceptable ascii 
	// characters.  The transformation into game-usable data is
	// done in a later step outside of this function!
	//
	if ((map_begin = strstr(data, MAP_BEGIN_STRING)) == NULL)
		return NULL;
	map_begin += strlen(MAP_BEGIN_STRING) + 1;

	/* set position to Waypoint-Data */
	if ((wp_begin = strstr(data, WP_SECTION_BEGIN_STRING)) == NULL)
		return NULL;
	wp_begin += strlen(WP_SECTION_BEGIN_STRING) + 1;

	/* now scan the map */
	short int curlinepos = 0;
	this_line = (char *)MyMalloc(4096);

	/* read MapData */
	for (i = 0; i < loadlevel->ylen; i++) {
		int col;
		map_tile *Buffer;
		int tmp;

		/* Select the next line */
		short int nlpos = 0;
		memset(this_line, 0, 4096);
		while (map_begin[curlinepos + nlpos] != '\n')
			nlpos++;
		memcpy(this_line, map_begin + curlinepos, nlpos);
		this_line[nlpos] = '\0';
		nlpos++;

		/* Decode it */
		Buffer = MyMalloc((loadlevel->xlen + 10) * sizeof(map_tile));
		for (col = 0; col < loadlevel->xlen; col++) {
			//sscanf( ( ( (char*)(Lev->map[row]) ) + 4 * col) , "%04d " , &tmp);
			*(this_line + 4 * col + 4) = '0';
			tmp = atoi(this_line + 4 * col);
			*(this_line + 4 * col + 4) = ' ';
			Buffer[col].floor_value = (Uint16) tmp;
			memset(Buffer[col].obstacles_glued_to_here, -1, MAX_OBSTACLES_GLUED_TO_ONE_MAP_TILE);
		}

		//--------------------
		// Now the old text pointer can be replaced with a pointer to the
		// correctly assembled struct...
		//
		loadlevel->map[i] = Buffer;

		curlinepos += nlpos;
	}

	DebugPrintf(2, "\nReached Waypoint-read-routine.");

	level_end = LocateStringInData(wp_begin, LEVEL_END_STRING);

	//--------------------
	// We decode the waypoint data from the data file into the waypoint
	// structs...
	//
	curlinepos = 0;
	for (i = 0; i < MAXWAYPOINTS; i++) {
		/* Select the next line */
		short int nlpos = 0;
		memset(this_line, 0, 4096);
		while (wp_begin[curlinepos + nlpos] != '\n')
			nlpos++;
		memcpy(this_line, wp_begin + curlinepos, nlpos);
		this_line[nlpos] = '\0';
		nlpos++;
		/* Copy it */
		curlinepos += nlpos;

		if (*this_line != 'N')
			if (!strcmp(this_line, LEVEL_END_STRING)) {
				loadlevel->num_waypoints = i;
				break;
			}
		wp_rnd = 0;
		sscanf(this_line, "Nr.=%d \t x=%d \t y=%d   rnd=%d", &nr, &x, &y, &wp_rnd);

		// completely ignore x=0/y=0 entries, which are considered non-waypoints!!
		if (x == 0 && y == 0)
			continue;

		loadlevel->AllWaypoints[i].x = x;
		loadlevel->AllWaypoints[i].y = y;
		// loadlevel -> AllWaypoints [ i ] . suppress_random_spawn = 0 ;
		loadlevel->AllWaypoints[i].suppress_random_spawn = wp_rnd;

		pos = strstr(this_line, CONNECTION_STRING);
		if (pos == NULL) {
			fprintf(stderr, "Unable to find connection string? i is %i, line is %s, level %i\n", i, this_line,
				loadlevel->levelnum);
		}
		pos += strlen(CONNECTION_STRING);	// skip connection-string
		pos += strspn(pos, WHITE_SPACE);	// skip initial whitespace

		for (k = 0; k < MAX_WP_CONNECTIONS; k++) {
			if (*pos == '\0')
				break;
			res = sscanf(pos, "%d", &connection);
			if ((connection == -1) || (res == 0) || (res == EOF))
				break;
			loadlevel->AllWaypoints[i].connections[k] = connection;
			pos += strcspn(pos, WHITE_SPACE);	// skip last token
			pos += strspn(pos, WHITE_SPACE);	// skip initial whitespace for next one

		}		// for k < MAX_WP_CONNECTIONS

		loadlevel->AllWaypoints[i].num_connections = k;

	}			// for i < MAXWAYPOINTS

	if (loadlevel->random_dungeon) {
		// Generate random dungeon now
		set_dungeon_output(loadlevel);
		generate_dungeon(loadlevel->xlen, loadlevel->ylen, loadlevel->random_dungeon);
	}

	free(this_line);
	return (loadlevel);

};				// Level DecodeLoadedLeveldata (char *data)

/**
 * This function translates map data into human readable map code, that
 * can later be written to the map file on disk.
 */
void TranslateToHumanReadable(Uint16 * HumanReadable, map_tile * MapInfo, int LineLength)
{
	int col;

	HumanReadable[0] = 0;	// Add a terminator at the beginning

	HumanReadable += strlen((char *)HumanReadable);

	for (col = 0; col < LineLength; col++) {
		sprintf((char *)HumanReadable, "%3d ", MapInfo[col].floor_value);
		HumanReadable += 2;
	}

};				// void TranslateToHumanReadable( ... )

/**
 * This function is used to calculate the number of the droids on the 
 * ship, which is a global variable.
 */
void CountNumberOfDroidsOnShip(void)
{
	Number_Of_Droids_On_Ship = 0;

	enemy *erot;
	BROWSE_ALIVE_BOTS(erot) {
		Number_Of_Droids_On_Ship++;
	}

	BROWSE_DEAD_BOTS(erot) {
		Number_Of_Droids_On_Ship++;
	}

};				// void CountNumberOfDroidsOnShip ( void )

static void ReviveAllDroidsOnShip(void)
{
	int i;

	for (i = 0; i < curShip.num_levels; i++) {
		if (curShip.AllLevels[i] == NULL)
			continue;
		respawn_level(i);
	}
};				// void ReviveAllDroidsOnShip ( void )

/* -----------------------------------------------------------------
 * This function initializes all enemys, which means that enemys are
 * filled in into the enemy list according to the enemys types that 
 * are to be found on each deck.
 * ----------------------------------------------------------------- */
int GetCrew(char *filename)
{
	char fpath[2048];
	char *MainDroidsFilePointer;
	char *DroidSectionPointer;
	char *EndOfThisDroidSectionPointer;

#define START_OF_DROID_DATA_STRING "*** Beginning of Droid Data ***"
#define END_OF_DROID_DATA_STRING "*** End of Droid Data ***"
#define DROIDS_LEVEL_DESCRIPTION_START_STRING "** Beginning of new Level **"
#define DROIDS_LEVEL_DESCRIPTION_END_STRING "** End of this levels droid data **"

	//--------------------
	//Now its time to start decoding the droids file.
	//For that, we must get it into memory first.
	//The procedure is the same as with LoadShip
	//

	find_file(filename, MAP_DIR, fpath, 0);

	MainDroidsFilePointer = ReadAndMallocAndTerminateFile(fpath, END_OF_DROID_DATA_STRING);

	//--------------------
	// The Droid crew file for this map is now completely read into memory
	// It's now time to decode the file and to fill the array of enemys with
	// new droids of the given types.
	//
	DroidSectionPointer = MainDroidsFilePointer;
	while ((DroidSectionPointer = strstr(DroidSectionPointer, DROIDS_LEVEL_DESCRIPTION_START_STRING)) != NULL) {
		DroidSectionPointer += strlen(DROIDS_LEVEL_DESCRIPTION_START_STRING);
		DebugPrintf(1, "\nFound another levels droids description starting point entry!");
		EndOfThisDroidSectionPointer = strstr(DroidSectionPointer, DROIDS_LEVEL_DESCRIPTION_END_STRING);
		if (EndOfThisDroidSectionPointer == NULL) {
			ErrorMessage(__FUNCTION__, "Unterminated droid section encountered!", PLEASE_INFORM, IS_FATAL);
		}
		// EndOfThisDroidSectionPointer[0]=0;
		GetThisLevelsDroids(DroidSectionPointer);
		DroidSectionPointer = EndOfThisDroidSectionPointer + 2;	// Move past the inserted String terminator
	}

	//--------------------
	// Now that the correct crew types have been filled into the 
	// right structure, it's time to set the energy of the corresponding
	// droids to "full" which means to the maximum of each type.
	//
	CountNumberOfDroidsOnShip();
	ReviveAllDroidsOnShip();

	enemy_generate_level_lists();

	free(MainDroidsFilePointer);
	return (OK);

};				// int GetCrew ( ... ) 

/**
 *
 *
 */
static void GetThisLevelsSpecialForces(char *SearchPointer, int OurLevelNumber, char *EndOfThisLevelData)
{
	char TypeIndicationString[1000];
	short int ListIndex;
	char *StartMapLabel;
	char *YesNoString;
	location StartupLocation;

	enemy newen;
	while ((SearchPointer = strstr(SearchPointer, SPECIAL_FORCE_INDICATION_STRING)) != NULL) {
		InitEnemy(&newen);
		SearchPointer += strlen(SPECIAL_FORCE_INDICATION_STRING);
		strncpy(TypeIndicationString, SearchPointer, 3);	// Every type is 3 characters long
		TypeIndicationString[3] = 0;
		DebugPrintf(1, "\nSpecial Force Type indication found!  It reads: %s.", TypeIndicationString);

		// Now that we have got a type indication string, we only need to translate it
		// into a number corresponding to that droid in the droid list
		for (ListIndex = 0; ListIndex < Number_Of_Droid_Types; ListIndex++) {
			if (!strcmp(Druidmap[ListIndex].druidname, TypeIndicationString))
				break;
		}
		if (ListIndex == Number_Of_Droid_Types) {
			fprintf(stderr, "\n\nTypeIndicationString: '%s' OurLevelNumber: %d .\n", TypeIndicationString, OurLevelNumber);
			ErrorMessage(__FUNCTION__, "\
The function reading and interpreting the crew file stunbled into something:\n\
It was unable to assign the SPECIAL FORCE droid type identification string '%s' found \n\
in the entry of the droid types allowed for level %d to an entry in\n\
the List of droids obtained from the gama data specification\n\
file you use.", PLEASE_INFORM, IS_FATAL);
		} else {
			DebugPrintf(1, "\nSpecial force's Type indication string %s translated to type Nr.%d.",
				    TypeIndicationString, ListIndex);
		}

		ReadValueFromString(SearchPointer, "Fixed=", "%hd", &newen.CompletelyFixed, EndOfThisLevelData);
		ReadValueFromString(SearchPointer, "Marker=", "%d", &newen.marker, EndOfThisLevelData);
		ReadValueFromStringWithDefault(SearchPointer, "MaxDistanceToHome=", "%hd", "0", &newen.max_distance_to_home,
					       EndOfThisLevelData);
		ReadValueFromString(SearchPointer, "Friendly=", "%hd", &newen.is_friendly, EndOfThisLevelData);
		StartMapLabel = ReadAndMallocStringFromData(SearchPointer, "StartLabel=\"", "\"");
		ResolveMapLabelOnShip(StartMapLabel, &StartupLocation);
		newen.pos.x = StartupLocation.x;
		newen.pos.y = StartupLocation.y;

		free(StartMapLabel);

		YesNoString = ReadAndMallocStringFromData(SearchPointer, "RushTux=\"", "\"");
		if (strcmp(YesNoString, "yes") == 0) {
			newen.will_rush_tux = TRUE;
		} else if (strcmp(YesNoString, "no") == 0) {
			newen.will_rush_tux = FALSE;
		} else {
			ErrorMessage(__FUNCTION__, "\
The droid specification of a droid in ReturnOfTux.droids should contain an \n\
answer that is either 'yes' or 'no', but which was neither 'yes' nor 'no'.\n\
This indicated a corrupted freedroid.ruleset file with an error at least in\n\
the item specification section.", PLEASE_INFORM, IS_FATAL);
		}

		free(YesNoString);

		newen.dialog_section_name = ReadAndMallocStringFromData(SearchPointer, "UseDialog=\"", "\"");
		if (strlen(newen.dialog_section_name) >= MAX_LENGTH_FOR_DIALOG_SECTION_NAME - 1) {
			ErrorMessage(__FUNCTION__, "\
The dialog section specification string for a bot was too large.\n\
This indicated a corrupted ReturnOfTux.droids file with an error when specifying\n\
the dialog section name for one special force droid/character.", PLEASE_INFORM, IS_FATAL);
		}

		newen.short_description_text = ReadAndMallocStringFromData(SearchPointer, "ShortLabel=_\"", "\"");
		if (strlen(newen.short_description_text) >= MAX_LENGTH_OF_SHORT_DESCRIPTION_STRING) {
			ErrorMessage(__FUNCTION__, "\
The short description specification string for a bot was too large.\n\
This indicated a corrupted ReturnOfTux.droids file with an error when specifying\n\
the dialog section name for one special force droid/character.", PLEASE_INFORM, IS_FATAL);
		}

		YesNoString = ReadAndMallocStringFromData(SearchPointer, "attack_run_only_when_direct_line=\"", "\"");
		if (strcmp(YesNoString, "yes") == 0) {
			newen.attack_run_only_when_direct_line = TRUE;
		} else if (strcmp(YesNoString, "no") == 0) {
			newen.attack_run_only_when_direct_line = FALSE;
		} else {
			ErrorMessage(__FUNCTION__, "\
The droid specification of a droid in ReturnOfTux.droids should contain an \n\
answer that is either 'yes' or 'no', but which was neither 'yes' nor 'no'.\n\
This indicated a corrupted freedroid.ruleset file with an error at least in\n\
the item specification section.", PLEASE_INFORM, IS_FATAL);
		}

		free(YesNoString);
		if (strstr(SearchPointer, "on_death_drop_item_name")) {
			YesNoString = ReadAndMallocStringFromData(SearchPointer, "on_death_drop_item_name=\"", "\"");
			newen.on_death_drop_item_code = GetItemIndexByName(YesNoString);
			free(YesNoString);
		} else
			newen.on_death_drop_item_code = -1;

		newen.type = ListIndex;
		newen.pos.z = OurLevelNumber;
		newen.SpecialForce = 1;
		newen.id = last_bot_number;
		newen.has_been_taken_over = FALSE;
		enemy *ne = (enemy *) calloc(1, sizeof(enemy));
		memcpy(ne, &newen, sizeof(enemy));
		list_add(&(ne->global_list), &alive_bots_head);
		last_bot_number++;

	}			// while Special force droid found...

	CountNumberOfDroidsOnShip();

};				// void GetThisLevelsSpecialForces ( char* SearchPointer )

/**
 * This function receives a pointer to the already read in crew section
 * in a already read in droids file and decodes all the contents of that
 * droid section to fill the AllEnemys array with droid types accoriding
 * to the specifications made in the file.
 */
void GetThisLevelsDroids(char *SectionPointer)
{
	int OurLevelNumber;
	char *SearchPointer;
	char *EndOfThisLevelData;
	int MaxRand;
	int MinRand;
	int RealNumberOfRandomDroids;
	int DifferentRandomTypes;
	int ListIndex;
	char TypeIndicationString[1000];
	short int ListOfTypesAllowed[1000];
	enemy newen;

#define DROIDS_LEVEL_INDICATION_STRING "Level="
#define DROIDS_LEVEL_END_INDICATION_STRING "** End of this levels droid data **"
#define DROIDS_MAXRAND_INDICATION_STRING "Maximum number of Random Droids="
#define DROIDS_MINRAND_INDICATION_STRING "Minimum number of Random Droids="
#define ALLOWED_TYPE_INDICATION_STRING "Allowed Type of Random Droid for this level: "

	// printf("\nReceived another levels droid section for decoding. It reads: %s " , SectionPointer );

	EndOfThisLevelData = LocateStringInData(SectionPointer, DROIDS_LEVEL_END_INDICATION_STRING);
	EndOfThisLevelData[0] = 0;

	// Now we read in the level number for this level
	ReadValueFromString(SectionPointer, DROIDS_LEVEL_INDICATION_STRING, "%d", &OurLevelNumber, EndOfThisLevelData);

	// Now we read in the maximal number of random droids for this level
	ReadValueFromString(SectionPointer, DROIDS_MAXRAND_INDICATION_STRING, "%d", &MaxRand, EndOfThisLevelData);

	// Now we read in the minimal number of random droids for this level
	ReadValueFromString(SectionPointer, DROIDS_MINRAND_INDICATION_STRING, "%d", &MinRand, EndOfThisLevelData);

	DifferentRandomTypes = 0;
	SearchPointer = SectionPointer;
	while ((SearchPointer = strstr(SearchPointer, ALLOWED_TYPE_INDICATION_STRING)) != NULL) {
		InitEnemy(&newen);
		SearchPointer += strlen(ALLOWED_TYPE_INDICATION_STRING);
		strncpy(TypeIndicationString, SearchPointer, 3);	// Every type is 3 characters long
		TypeIndicationString[3] = 0;
		// printf("\nType indication found!  It reads: %s." , TypeIndicationString );

		// Now that we have got a type indication string, we only need to translate it
		// into a number corresponding to that droid in the droid list
		for (ListIndex = 0; ListIndex < Number_Of_Droid_Types; ListIndex++) {
			if (!strcmp(Druidmap[ListIndex].druidname, TypeIndicationString))
				break;
		}
		if (ListIndex >= Number_Of_Droid_Types) {
			fprintf(stderr, "\n\nTypeIndicationString: '%s' OurLevelNumber: %d .\n", TypeIndicationString, OurLevelNumber);
			ErrorMessage(__FUNCTION__, "\
The function reading and interpreting the crew file stunbled into something:\n\
It was unable to assign the droid type identification string '%s' found \n\
in the entry of the droid types allowed for level %d to an entry in\n\
the List of droids obtained from the gama data specification\n\
file you use.  \n\
Please check that this type really is spelled correctly, that it consists of\n\
only three characters and that it really has a corresponding entry in the\n\
game data file with all droid type specifications.", PLEASE_INFORM, IS_FATAL);
		} else {
			DebugPrintf(1, "\nType indication string %s translated to type Nr.%d.", TypeIndicationString, ListIndex);
		}
		ListOfTypesAllowed[DifferentRandomTypes] = ListIndex;
		DifferentRandomTypes++;
	}
	//fprintf( stderr , "\nFound %d different allowed random types for this level. " , DifferentRandomTypes );

	//--------------------
	// At this point, the List "ListOfTypesAllowed" has been filled with the NUMBERS of
	// the allowed types.  The number of different allowed types found is also available.
	// That means that now we can add the apropriate droid types into the list of existing
	// droids in that mission.

	RealNumberOfRandomDroids = MyRandom(MaxRand - MinRand) + MinRand;

	while (RealNumberOfRandomDroids--) {
		newen.type = ListOfTypesAllowed[MyRandom(DifferentRandomTypes - 1)];
		newen.pos.z = OurLevelNumber;
		newen.on_death_drop_item_code = (-1);
		newen.ammo_left = ItemMap[Druidmap[newen.type].weapon_item.type].item_gun_ammo_clip_size;
		newen.id = last_bot_number + 1;

		newen.dialog_section_name = strdup("StandardBotAfterTakeover");

		switch (atoi(Druidmap[newen.type].druidname)) {
		case 123:
			newen.short_description_text = strdup(_("123 Acolyte"));
			break;
		case 139:
			newen.short_description_text = strdup(_("139 Templar"));
			break;
		case 247:
			newen.short_description_text = strdup(_("247 Banshee"));
			break;
		case 249:
			newen.short_description_text = strdup(_("249 Chicago"));
			break;
		case 296:
			newen.short_description_text = strdup(_("296 Sawmill"));
			break;
		case 302:
			newen.short_description_text = strdup(_("302 Nemesis"));
			break;
		case 329:
			newen.short_description_text = strdup(_("329 Sparkie"));
			break;
		case 420:
			newen.short_description_text = strdup(_("420 Surgeon"));
			break;
		case 476:
			newen.short_description_text = strdup(_("476 Coward"));
			break;
		case 493:
			newen.short_description_text = strdup(_("493 Spinster"));
			break;
		case 516:
			newen.short_description_text = strdup(_("516 Ghoul"));
			break;
		case 543:
			newen.short_description_text = strdup(_("543 Forest Harvester"));
			break;
		case 571:
			newen.short_description_text = strdup(_("571 Apollo"));
			break;
		case 598:
			newen.short_description_text = strdup(_("598 Minister"));
			break;
		case 615:
			newen.short_description_text = strdup(_("615 Firedevil"));
			break;
		case 629:
			newen.short_description_text = strdup(_("629 Spitfire"));
			break;
		case 711:
			newen.short_description_text = strdup(_("711 Grillmeister"));
			break;
		case 742:
			newen.short_description_text = strdup(_("742 Zeus"));
			break;
		case 751:
			newen.short_description_text = strdup(_("751 Soviet"));
			break;
		case 821:
			newen.short_description_text = strdup(_("821 Ufo"));
			break;
		case 834:
			newen.short_description_text = strdup(_("834 Wisp"));
			break;
		case 883:
			newen.short_description_text = strdup(_("883 Dalex"));
			break;
		case 999:
			newen.short_description_text = strdup(_("999 Cerebrum"));
			break;
		default:
			newen.short_description_text = strdup(_("No Description For This One"));
		};

		enemy *ne = (enemy *) calloc(1, sizeof(enemy));
		memcpy(ne, &newen, sizeof(enemy));
		list_add(&(ne->global_list), &alive_bots_head);
		last_bot_number++;
	}			// while (enemy-limit of this level not reached) 

	SearchPointer = SectionPointer;

	GetThisLevelsSpecialForces(SearchPointer, OurLevelNumber, EndOfThisLevelData);

};				// void GetThisLevelsDroids( char* SectionPointer )

/** 
 * This funtion moves the level doors in the sense that they are opened
 * or closed depending on whether there is a robot close to the door or
 * not.  
 */
void MoveLevelDoors()
{
	struct animated_obstacle *a;
	float xdist, ydist;
	float dist2;
	int *Pos;
	level *DoorLevel;
	int one_player_close_enough = FALSE;
	int door_obstacle_index;
	int some_bot_was_close_to_this_door = FALSE;

	DoorLevel = CURLEVEL();

	// if the door list is dirty, regenerate it
	if (DoorLevel->levelnum != animated_obstacles_lists_level)
		GetAnimatedMapTiles();

	//--------------------
	// This prevents animation going too quick.
	// The constant should be replaced by a variable, that can be
	// set from within the theme, but that may be done later...
	//
	if (LevelDoorsNotMovedTime < 0.05)
		return;

	LevelDoorsNotMovedTime = 0;

	list_for_each_entry(a, &doors_head, node) {
		door_obstacle_index = a->index;

		//--------------------
		// We make a convenient pointer to the type of the obstacle, that
		// is supposed to be a door and might need to be changed as far as
		// it's opening status is concerned...
		//
		Pos = &(DoorLevel->obstacle_list[door_obstacle_index].type);

		//--------------------
		// Some security check against changing anything that isn't a door here...
		//
		switch (*Pos) {
		case ISO_H_DOOR_000_OPEN:
		case ISO_H_DOOR_025_OPEN:
		case ISO_H_DOOR_050_OPEN:
		case ISO_H_DOOR_075_OPEN:
		case ISO_H_DOOR_100_OPEN:

		case ISO_V_DOOR_000_OPEN:
		case ISO_V_DOOR_025_OPEN:
		case ISO_V_DOOR_050_OPEN:
		case ISO_V_DOOR_075_OPEN:
		case ISO_V_DOOR_100_OPEN:

		case ISO_OUTER_DOOR_V_00:
		case ISO_OUTER_DOOR_V_25:
		case ISO_OUTER_DOOR_V_50:
		case ISO_OUTER_DOOR_V_75:
		case ISO_OUTER_DOOR_V_100:

		case ISO_OUTER_DOOR_H_00:
		case ISO_OUTER_DOOR_H_25:
		case ISO_OUTER_DOOR_H_50:
		case ISO_OUTER_DOOR_H_75:
		case ISO_OUTER_DOOR_H_100:

		case ISO_DH_DOOR_000_OPEN:
		case ISO_DH_DOOR_025_OPEN:
		case ISO_DH_DOOR_050_OPEN:
		case ISO_DH_DOOR_075_OPEN:
		case ISO_DH_DOOR_100_OPEN:

		case ISO_DV_DOOR_000_OPEN:
		case ISO_DV_DOOR_025_OPEN:
		case ISO_DV_DOOR_050_OPEN:
		case ISO_DV_DOOR_075_OPEN:
		case ISO_DV_DOOR_100_OPEN:

			break;

		default:
			fprintf(stderr, "\n*Pos: '%d'.\nlevelnum: %d\nObstacle index: %d", *Pos, DoorLevel->levelnum, door_obstacle_index);
			ErrorMessage(__FUNCTION__, "\
			Error:  Doors pointing not to door obstacles found.", PLEASE_INFORM, IS_FATAL);
			break;
		}

		//--------------------
		// First we see if one of the players is close enough to the
		// door, so that it would get opened.
		//
		one_player_close_enough = FALSE;
		//--------------------
		// Maybe this player is on a different level, than we are 
		// interested now.
		//
		if (Me.pos.z != DoorLevel->levelnum)
			continue;

		//--------------------
		// But this player is on the right level, we need to check it's distance 
		// to this door.
		//
		xdist = Me.pos.x - DoorLevel->obstacle_list[door_obstacle_index].pos.x;
		ydist = Me.pos.y - DoorLevel->obstacle_list[door_obstacle_index].pos.y;
		dist2 = xdist * xdist + ydist * ydist;
		if (dist2 < DOOROPENDIST2) {
			one_player_close_enough = TRUE;
		}
		// --------------------
		// If one of the players is close enough, the door gets opened
		// and we are done.
		//
		if (one_player_close_enough) {
			if ((*Pos != ISO_H_DOOR_100_OPEN) && (*Pos != ISO_V_DOOR_100_OPEN)
			    && (*Pos != ISO_DH_DOOR_100_OPEN) && (*Pos != ISO_DV_DOOR_100_OPEN)
			    && (*Pos != ISO_OUTER_DOOR_H_100) && (*Pos != ISO_OUTER_DOOR_V_100))
				*Pos += 1;
		} else {
			//--------------------
			// But if the Tux is not close enough, then we must
			// see if perhaps one of the enemys is close enough, so that
			// the door would still get opened instead of closed.
			//
			some_bot_was_close_to_this_door = FALSE;

			enemy *erot;
			BROWSE_LEVEL_BOTS(erot, DoorLevel->levelnum) {
				//--------------------
				// We will only consider droids, that are at least within a range of
				// say 2 squares in each direction.  Anything beyond that distance
				// can be safely ignored for this door.
				//
				xdist = abs(erot->pos.x - DoorLevel->obstacle_list[door_obstacle_index].pos.x);
				if (xdist < 2.0) {
					ydist = abs(erot->pos.y - DoorLevel->obstacle_list[door_obstacle_index].pos.y);
					if (ydist < 2.0) {

						//--------------------
						// Now that we know, that there is some droid at least halfway
						// close to this door, we can start to go into more details and
						// compute the exact distance from the droid to the door.
						//
						dist2 = xdist * xdist + ydist * ydist;
						if (dist2 < DOOROPENDIST2_FOR_DROIDS) {
							if ((*Pos != ISO_H_DOOR_100_OPEN) && (*Pos != ISO_V_DOOR_100_OPEN)
							    && (*Pos != ISO_DH_DOOR_100_OPEN) && (*Pos != ISO_DV_DOOR_100_OPEN)
							    && (*Pos != ISO_OUTER_DOOR_H_100) && (*Pos != ISO_OUTER_DOOR_V_100))
								*Pos += 1;

							//--------------------
							// Just to make sure the last bot doesn't look like the whole loop
							// went through without any bot being close...
							//
							some_bot_was_close_to_this_door = TRUE;
							break;
						}

					}	// ydist < 2.0
				}	// xdist < 2.0

			}	// bots

			//--------------------
			// So if the whole loop when through, that means that no bot was close
			// enough to this door.  That means that we can close this door a bit
			// more...
			//
			if (!some_bot_was_close_to_this_door)
				if ((*Pos != ISO_V_DOOR_000_OPEN) && (*Pos != ISO_H_DOOR_000_OPEN)
				    && (*Pos != ISO_DV_DOOR_000_OPEN) && (*Pos != ISO_DH_DOOR_000_OPEN)
				    && (*Pos != ISO_OUTER_DOOR_V_00) && (*Pos != ISO_OUTER_DOOR_H_00))
					*Pos -= 1;

		}		/* else */
	}			/* for */
};				// void MoveLevelDoors ( void )

/**
 * This function does all the firing for the autocannons installed in
 * the map of this level.
 */
void WorkLevelGuns()
{
	float autogunx, autoguny;
	int *AutogunType;
	level *GunLevel;

	//--------------------
	// The variables for the gun.
	//
	int j = 0;
	/*XXX hardcoded weapon item type */
	int weapon_item_type = GetItemIndexByName("Laser pistol");
	bullet *CurBullet = NULL;	// the bullet we're currentl dealing with
	int bullet_image_type = ItemMap[weapon_item_type].item_gun_bullet_image_type;	// which gun do we have ? 
	float BulletSpeed = ItemMap[weapon_item_type].item_gun_speed;
	double speed_norm;
	moderately_finepoint speed;
	struct animated_obstacle *a;

	GunLevel = CURLEVEL();

	if (GunLevel->levelnum != animated_obstacles_lists_level)
		GetAnimatedMapTiles();

	if (LevelGunsNotFiredTime < 0.2)
		return;
	LevelGunsNotFiredTime = 0;

	list_for_each_entry(a, &autoguns_head, node) {

		autogunx = (GunLevel->obstacle_list[a->index].pos.x);
		autoguny = (GunLevel->obstacle_list[a->index].pos.y);

		AutogunType = &(GunLevel->obstacle_list[a->index].type);

		//--------------------
		// search for the next free bullet list entry
		//
		for (j = 0; j < (MAXBULLETS); j++) {
			if (AllBullets[j].type == INFOUT) {
				CurBullet = &AllBullets[j];
				break;
			}
		}

		// didn't find any free bullet entry? --> take the first
		if (CurBullet == NULL)
			CurBullet = &AllBullets[0];

		CurBullet->type = bullet_image_type;

		CurBullet->damage = 5;
		CurBullet->mine = FALSE;
		CurBullet->owner = -1;
		CurBullet->bullet_lifetime = ItemMap[weapon_item_type].item_gun_bullet_lifetime;
		CurBullet->ignore_wall_collisions = ItemMap[weapon_item_type].item_gun_bullet_ignore_wall_collisions;
		CurBullet->time_in_frames = 0;
		CurBullet->time_in_seconds = 0;
		CurBullet->was_reflected = FALSE;
		CurBullet->reflect_other_bullets = ItemMap[weapon_item_type].item_gun_bullet_reflect_other_bullets;
		CurBullet->pass_through_explosions = ItemMap[weapon_item_type].item_gun_bullet_pass_through_explosions;
		CurBullet->pass_through_hit_bodies = ItemMap[weapon_item_type].item_gun_bullet_pass_through_hit_bodies;

		CurBullet->is_friendly = 0;

		CurBullet->to_hit = 90;

		//--------------------
		// Maybe the bullet has some magic properties.  This is handled here.
		//
		CurBullet->freezing_level = 0;
		CurBullet->poison_duration = 0;
		CurBullet->poison_damage_per_sec = 0;
		CurBullet->paralysation_duration = 0;

		speed.x = 0.0;
		speed.y = 0.0;

		CurBullet->pos.x = autogunx;
		CurBullet->pos.y = autoguny;

		switch (*AutogunType) {
		case ISO_AUTOGUN_W:
			speed.x = -0.2;
			CurBullet->pos.x -= 0.5;
			CurBullet->pos.y -= 0.25;
			break;
		case ISO_AUTOGUN_E:
			speed.x = 0.2;
			CurBullet->pos.x += 0.4;
			break;
		case ISO_AUTOGUN_N:
			speed.y = -0.2;
			CurBullet->pos.x += -0.25;
			CurBullet->pos.y += -0.5;
			break;
		case ISO_AUTOGUN_S:
			speed.y = +0.2;
			CurBullet->pos.y += 0.4;
			break;
		default:
			fprintf(stderr, "\n*AutogunType: '%d'.\n", *AutogunType);
			ErrorMessage(__FUNCTION__, "\
There seems to be an autogun in the autogun list of this level, but it\n\
is not really an autogun.  Instead it's something else.", PLEASE_INFORM, IS_FATAL);
			break;
		}

		CurBullet->pos.z = Me.pos.z;

		speed_norm = sqrt(speed.x * speed.x + speed.y * speed.y);
		CurBullet->speed.x = (speed.x / speed_norm);
		CurBullet->speed.y = (speed.y / speed_norm);

		//--------------------
		// Now we determine the angle of rotation to be used for
		// the picture of the bullet itself
		//

		CurBullet->angle = -(atan2(speed.y, speed.x) * 180 / M_PI + 90 + 45);

		DebugPrintf(1, "\nWorkLevelGuns(...) : Phase of bullet=%d.", CurBullet->phase);
		DebugPrintf(1, "\nWorkLevelGuns(...) : angle of bullet=%f.", CurBullet->angle);

		CurBullet->speed.x *= BulletSpeed;
		CurBullet->speed.y *= BulletSpeed;

	}			// for

};				// void WorkLevelGuns ( void )

/**
 * This function determines wether a given object on x/y is visible to
 * the 001 or not (due to some walls or something in between
 * 
 * Return values are TRUE or FALSE accodinly
 *
 */
int IsVisible(GPS objpos)
{

	//--------------------
	// For the purpose of visibility checking, we might as well exclude objects
	// that are too far away to ever be visible and thereby save some checks of
	// longer lines on the map, that wouldn't be nescessary or helpful anyway.
	//
	if ((fabsf(Me.pos.x - objpos->x) > FLOOR_TILES_VISIBLE_AROUND_TUX) ||
	    (fabsf(Me.pos.y - objpos->y) > FLOOR_TILES_VISIBLE_AROUND_TUX))
		return (FALSE);

	//--------------------
	// So if the object in question is close enough to be visible, we'll do the
	// actual check and see if the line of sight is free or blocked, a rather
	// time-consuming and often re-iterated process.  (Maybe some do-it-every-
	// -10th-frame-only code could be added here later... and in the meantime
	// old values could be used from a stored flag?!
	//
	return (DirectLineColldet(objpos->x, objpos->y, Me.pos.x, Me.pos.y, objpos->z, &VisiblePassFilter));

};				// int IsVisible( Point objpos )

/**
 * This function moves all periodically changing map tiles...
 */
void AnimateCyclingMapTiles(void)
{
	AnimateRefresh();
	// AnimateConsumer();
	AnimateTeleports();
};				// void AnimateCyclingMapTiles (void)

/*----------------------------------------------------------------------
 * delete given waypoint num (and all its connections) on level Lev
 *----------------------------------------------------------------------*/
void DeleteWaypoint(level * Lev, int num)
{
	int i, j;
	waypoint *WpList, *ThisWp;
	int wpmax;

	WpList = Lev->AllWaypoints;
	wpmax = Lev->num_waypoints - 1;

	// is this the last one? then just delete
	if (num == wpmax)
		WpList[num].num_connections = 0;
	else			// otherwise shift down all higher waypoints
		memcpy(&WpList[num], &WpList[num + 1], (wpmax - num) * sizeof(waypoint));

	// now there's one less:
	Lev->num_waypoints--;
	wpmax--;

	// now adjust the remaining wp-list to the changes:
	ThisWp = WpList;
	for (i = 0; i < Lev->num_waypoints; i++, ThisWp++)
		for (j = 0; j < ThisWp->num_connections; j++) {
			// eliminate all references to this waypoint
			if (ThisWp->connections[j] == num) {
				// move all connections after this one down
				memcpy(&(ThisWp->connections[j]), &(ThisWp->connections[j + 1]),
				       (ThisWp->num_connections - 1 - j) * sizeof(int));
				ThisWp->num_connections--;
				j--;	// just to be sure... check the next connection as well...(they have been shifted!)
				continue;
			}
			// adjust all connections to the shifted waypoint-numbers
			else if (ThisWp->connections[j] > num)
				ThisWp->connections[j]--;

		}		// for j < num_connections

	return;

}				// DeleteWaypoint()

/*----------------------------------------------------------------------
 * create a new empty waypoint on position x/y
 *----------------------------------------------------------------------*/
int CreateWaypoint(level * Lev, int x, int y, int *isnew)
{
	int num;

	if (Lev->num_waypoints == MAXWAYPOINTS) {
		ErrorMessage(__FUNCTION__,
			     "Maximal number of waypoints (%d) reached on level %d when trying to add waypoint at x %d y %d\n",
			     PLEASE_INFORM, IS_WARNING_ONLY, MAXWAYPOINTS, Lev->levelnum, x, y);
		return -1;
	}

	int i;
	// find out if there is already a waypoint on the current square
	for (i = 0; i < Lev->num_waypoints; i++) {
		if ((Lev->AllWaypoints[i].x == x) && (Lev->AllWaypoints[i].y == y)) {
			*isnew = 0;
			return i;
		}
	}

	num = Lev->num_waypoints;
	Lev->num_waypoints++;

	Lev->AllWaypoints[num].x = x;
	Lev->AllWaypoints[num].y = y;
	Lev->AllWaypoints[num].num_connections = 0;
	Lev->AllWaypoints[num].num_connections = 0;
	Lev->AllWaypoints[num].suppress_random_spawn = 0;

	*isnew = 1;
	return num;

}				// CreateWaypoint()

/**
 *
 *
 */
inline float translate_pixel_to_map_location(float axis_x, float axis_y, int give_x)
{

	//--------------------
	// NOTE:  This function does not expect absolute screen coordinates but rather coordinates relative
	// to the center of the screen.
	//
	// That's also why it's 'axis' rather than 'pos' or 'point'.
	//
	// That is because mouse clicks can best be analyzed this way.
	//

	if (give_x) {
		return (Me.pos.x + (axis_x / ((float)iso_floor_tile_width)) + (axis_y / ((float)iso_floor_tile_height)));
	} else {
		return (Me.pos.y - (axis_x / ((float)iso_floor_tile_width)) + (axis_y / ((float)iso_floor_tile_height)));
	}

};				// int translate_pixel_to_map_location ( int axis_x , int axis_y , int give_x ) 

/**
 *
 *
 */
float translate_pixel_to_zoomed_map_location(float axis_x, float axis_y, int give_x)
{
	float zf = lvledit_zoomfact();
	if (give_x) {
		return (Me.pos.x + (zf * axis_x / ((float)iso_floor_tile_width)) + (zf * axis_y / ((float)iso_floor_tile_height)));
		// return ( ( axis_x / ISO_WIDTH ) + ( axis_y / ISO_HEIGHT ) ) ;
	} else {
		return (Me.pos.y - (zf * axis_x / ((float)iso_floor_tile_width)) + (zf * axis_y / ((float)iso_floor_tile_height)));
		// return ( - ( axis_x / ISO_WIDTH ) + ( axis_y / ISO_HEIGHT ) ) ;
	}

};				// int translate_pixel_to_zoomed_map_location ( int axis_x , int axis_y , int give_x ) 

/**
 *
 *
 */
moderately_finepoint translate_point_to_map_location(float axis_x, float axis_y, int zoom_is_on)
{
	moderately_finepoint position;
	if (zoom_is_on) {
		position.x = translate_pixel_to_zoomed_map_location(axis_x, axis_y, TRUE);
		position.y = translate_pixel_to_zoomed_map_location(axis_x, axis_y, FALSE);
	} else {
		position.x = translate_pixel_to_map_location(axis_x, axis_y, TRUE);
		position.y = translate_pixel_to_map_location(axis_x, axis_y, FALSE);
	}
	return position;
}

/**
 * This function translates a given map point to screen coordinates.
 *
 * @param x_map_pos X position on map
 * @param y_map_pos Y position on map
 * @param x_res	pointer to the int that will hold the x position on screen
 * @param y_res pointer to the y position on screen
 * @param zoom_factor zoom factor in use
 * 
 */
void translate_map_point_to_screen_pixel_func(float x_map_pos, float y_map_pos, int *x_res, int *y_res, float zoom_factor)
{
#define R ceilf
#define factX iso_floor_tile_width*0.5*zoom_factor
#define factY iso_floor_tile_height*0.5*zoom_factor
	if (x_res != NULL) {
		//obstacles oscillent *x_res = UserCenter_x + R( (x_map_pos - Me.pos.x) * factX) + R((Me . pos . y - y_map_pos) * factX);
		//murs tilent pas -- en fait si
		*x_res = UserCenter_x + R(x_map_pos * factX) - R(y_map_pos * factX) + R(Me.pos.y * factX) - R(factX * Me.pos.x);
		//murs tilent pas ET tux oscille *x_res = UserCenter_x + R( x_map_pos * factX) - R(y_map_pos * factX) + R((Me.pos.y - Me.pos.x) * factX);
		//original "devtrack" - murs tilent pas *x_res = ( UserCenter_x + R ( ( x_map_pos - y_map_pos )  * factX  ) + R ( ( Me . pos . y - Me . pos . x ) * factX ) );

	}
	if (y_res != NULL) {
		//*y_res = UserCenter_y + R( (x_map_pos - Me.pos.x)* factY ) + R((y_map_pos - Me . pos . y)* factY);
		*y_res = UserCenter_y + R(x_map_pos * factY) + R(y_map_pos * factY) - R(Me.pos.x * factY) - R(factY * Me.pos.y);
		//*y_res = UserCenter_y + R( x_map_pos * factY ) + R(y_map_pos * factY) - R((Me.pos.x + Me.pos.y) * factY);
		//*y_res=( UserCenter_y + R ( ( x_map_pos + y_map_pos )  * factY ) - R( (  Me . pos . x + Me . pos . y ) * factY ));
	}
#undef R
#undef factX
#undef factY
}

#undef _map_c
