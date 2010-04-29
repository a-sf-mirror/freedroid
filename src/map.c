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

void GetThisLevelsDroids(char *SectionPointer);

struct animated_obstacle {
	int index;
	int (*animate_fn)(level* obstacle_lvl, int obstacle_idx);
	struct list_head node;
};

/**
 * If the blood doesn't vanish, then there will be more and more blood,
 * especially after the bots on the level have respawned a few times.
 * Therefore we need this function, which will remove all traces of blood
 * from a given level.
 */
static void remove_blood_obstacles_for_respawning(int level_num)
{
	int i;

	// We pass through all the obstacles, deleting those
	// that are 'blood'.
	//
	for (i = 0; i < MAX_OBSTACLES_ON_MAP; i++) {
		switch (curShip.AllLevels[level_num]->obstacle_list[i].type) {
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
 * This function will make all blood obstacles vanish, all dead bots come
 * back to life, and get all bots return to a wandering state.
 */
void respawn_level(int level_num)
{
	enemy *erot, *nerot;

	int wp_num = curShip.AllLevels[level_num]->num_waypoints;
	char wp_used[wp_num]; // is a waypoint already used ?
	memset(wp_used, 0, wp_num);

	// First we remove all the blood obstacles...
	//
	remove_blood_obstacles_for_respawning(level_num);

	// Now we can give new life to dead bots...
	//
	BROWSE_DEAD_BOTS_SAFE(erot, nerot) {
		if (erot->pos.z != level_num || Druidmap[erot->type].is_human)
			continue;
		/* Move the bot to the alive list */
		list_move(&(erot->global_list), &alive_bots_head);
		/* Reinsert it into the current level list */
		list_add(&(erot->level_list), &level_bots_head[level_num]);
	}

	// Finally, we reset the runtime attributes of the bots, place them
	// on a waypoint, and ask them to start wandering...
	//
	BROWSE_LEVEL_BOTS(erot, level_num) {

		// Unconditional reset of the 'transient state' attributes
		enemy_reset(erot);

		// Conditional reset of some 'global state' attributes
		if (erot->has_been_taken_over) {
			erot->is_friendly = FALSE;
			erot->has_been_taken_over = FALSE;
			erot->CompletelyFixed = FALSE;
			erot->follow_tux = FALSE;
		}
		if (!erot->is_friendly) {
			erot->has_greeted_influencer = FALSE;
		}

		// Re-place the bots onto the waypoint system
		if (!erot->SpecialForce) {
			// Standard bots are randomly placed on one waypoint
			int wp = teleport_to_random_waypoint(erot, curShip.AllLevels[level_num], wp_used);
			wp_used[wp] = 1;
			erot->homewaypoint = erot->lastwaypoint;
			erot->combat_state = SELECT_NEW_WAYPOINT;
			erot->state_timeout = 0.0;
		} else {
			if (erot->homewaypoint == -1) {
				// If a special force droid has not yet been integrated onto
				// the waypoint system, place it near its current position.
				int wp = teleport_to_closest_waypoint(erot);
				wp_used[wp] = 1;
				erot->homewaypoint = erot->lastwaypoint;
				erot->combat_state = SELECT_NEW_WAYPOINT;
				erot->state_timeout = 0.0;
			} else {
				// Consider that the nextwaypoint of a special force droid
				// is occupied, so that a standard bot will not be placed here
				if (erot->nextwaypoint != -1)
					wp_used[erot->nextwaypoint] = 1;
			}
		}
	}
}

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

	// We empty the given target pointer, so that we can tell
	// a successful resolve later...
	//
	PositionPointer->x = -1;
	PositionPointer->y = -1;

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
static void decode_interfaces(level *loadlevel, char *DataPointer)
{
	char *TempSectionPointer;
	char PreservedLetter;

	// We look for the beginning and end of the map statement section
	TempSectionPointer = LocateStringInData(DataPointer, MAP_BEGIN_STRING);

	// We add a terminator at the end, but ONLY TEMPORARY.  The damage will be restored later!
	PreservedLetter = TempSectionPointer[0];
	TempSectionPointer[0] = 0;

	ReadValueFromString(DataPointer, "jump target north: ", "%d", &(loadlevel->jump_target_north), TempSectionPointer);
	ReadValueFromString(DataPointer, "jump target south: ", "%d", &(loadlevel->jump_target_south), TempSectionPointer);
	ReadValueFromString(DataPointer, "jump target east: ", "%d", &(loadlevel->jump_target_east), TempSectionPointer);
	ReadValueFromString(DataPointer, "jump target west: ", "%d", &(loadlevel->jump_target_west), TempSectionPointer);
	ReadValueFromString(DataPointer, "use underground lighting: ", "%d", &(loadlevel->use_underground_lighting), TempSectionPointer);

	TempSectionPointer[0] = PreservedLetter;

}

static void decode_dimensions(level *loadlevel, char *DataPointer)
{

	int off = 0;

	/* Read levelnumber */
	char *fp = DataPointer;
	fp += strlen(LEVEL_HEADER_LEVELNUMBER);
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
	loadlevel->light_bonus = atoi(fp);
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

	if (!strncmp(fp, "dungeon generated:", 18)) {
		fp += strlen("dungeon generated:");
		while (*(fp + off) != '\n')
			off++;
		fp[off] = 0;
		loadlevel->dungeon_generated = atoi(fp);
		fp[off] = '\n';
		fp += off + 1;
		off = 0;
	} else {
		loadlevel->dungeon_generated = 0;
	}

	if (loadlevel->ylen >= MAX_MAP_LINES) {
		ErrorMessage(__FUNCTION__, "\
A maplevel Freedroid was supposed to load has more map lines than allowed\n\
for a map level as by the constant MAX_MAP_LINES in defs.h.\n\
Sorry, but unless this constant is raised, Freedroid will refuse to load this map.", PLEASE_INFORM, IS_FATAL);
	}
}

static int decode_header(level *loadlevel, char *data)
{
	data = strstr(data, LEVEL_HEADER_LEVELNUMBER);
	if (!data)
		return 1;

	decode_interfaces(loadlevel, data);
	decode_dimensions(loadlevel, data);
	
	// Read the levelname.
	// Accept legacy ship-files that are not yet marked-up for translation
	if ((loadlevel->Levelname = ReadAndMallocStringFromDataOptional(data, LEVEL_NAME_STRING, "\"")) == NULL) {
		loadlevel->Levelname = ReadAndMallocStringFromData(data, LEVEL_NAME_STRING_LEGACY, "\n");
	}

	loadlevel->Background_Song_Name = ReadAndMallocStringFromData(data, BACKGROUND_SONG_NAME_STRING, "\n");

	return 0;
}

/**
 * Next we extract the human readable obstacle data into the level struct
 * WITHOUT destroying or damaging the human-readable data in the process!
 * This is an improved parser that is not quite readable but very performant.
 */
static char *decode_obstacles(level *loadlevel, char *DataPointer)
{
	int i;
	char *curfield = NULL;
	char *curfieldend = NULL;
	char *obstacle_SectionBegin;

	// First we initialize the obstacles with 'empty' information
	//
	for (i = 0; i < MAX_OBSTACLES_ON_MAP; i++) {
		loadlevel->obstacle_list[i].type = -1;
		loadlevel->obstacle_list[i].pos.x = -1;
		loadlevel->obstacle_list[i].pos.y = -1;
		loadlevel->obstacle_list[i].pos.z = loadlevel->levelnum;
		loadlevel->obstacle_list[i].name_index = -1;
	}

	if (loadlevel->random_dungeon && !loadlevel->dungeon_generated)
		return DataPointer;

	// Now we look for the beginning and end of the obstacle section
	//
	obstacle_SectionBegin = LocateStringInData(DataPointer, OBSTACLE_DATA_BEGIN_STRING) + strlen(OBSTACLE_DATA_BEGIN_STRING) + 1;

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

		(*curfieldend) = ' ';
		while ((*curfield) != '\n')
			curfield++;
		curfield++;
		//fprintf( stderr , "\nobtacle_type=%d pos.x=%3.2f pos.y=%3.2f\n" , loadlevel -> obstacle_list [ i ] . type ,  loadlevel -> obstacle_list [ i ] . pos . 
//x , loadlevel-> obstacle_list [ i ] . pos . y );
		i++;
	}

	return curfield;
}

/**
 * Next we extract the map labels of this level WITHOUT destroying
 * or damaging the data in the process!
 */
static char *decode_map_labels(level *loadlevel, char *DataPointer)
{
	int i;
	char PreservedLetter;
	char *MapLabelPointer;
	char *MapLabelSectionBegin;
	char *MapLabelSectionEnd;
	int NumberOfMapLabelsInThisLevel;

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

	if (loadlevel->random_dungeon && !loadlevel->dungeon_generated)
		return DataPointer;

	// Now we look for the beginning and end of the map labels section
	//
	MapLabelSectionBegin = LocateStringInData(DataPointer, MAP_LABEL_BEGIN_STRING) + strlen(MAP_LABEL_BEGIN_STRING) + 1;
	MapLabelSectionEnd = LocateStringInData(MapLabelSectionBegin, MAP_LABEL_END_STRING);

	// We add a terminator at the end, but ONLY TEMPORARY.  The damage will be restored later!
	//
	PreservedLetter = MapLabelSectionEnd[0];
	MapLabelSectionEnd[0] = 0;
	NumberOfMapLabelsInThisLevel = CountStringOccurences(MapLabelSectionBegin, LABEL_ITSELF_ANNOUNCE_STRING);
	DebugPrintf(1, "\nNumber of map labels found in this level : %d.", NumberOfMapLabelsInThisLevel);

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

	// Now we repair the damage done to the loaded level data
	//
	MapLabelSectionEnd[0] = PreservedLetter;
	return MapLabelSectionEnd;
}

/**
 * Every map level in a FreedroidRPG 'ship' can have up to 
 * MAX_OBSTACLE_NAMES_PER_LEVEL obstacles, that have a label attached to
 * them.  Such obstacle labels are very useful when modifying obstacles
 * from within game events and triggers.  The obstacle labels are stored
 * in a small subsection of the whole level data.  This function decodes
 * this small subsection and loads all the obstacle data into the ship
 * struct.
 */
static char *decode_obstacle_names(Level loadlevel, char *DataPointer)
{
	int i;
	char PreservedLetter;
	char *obstacle_namePointer;
	char *obstacle_nameSectionBegin;
	char *obstacle_nameSectionEnd;
	int NumberOfobstacle_namesInThisLevel;
	int target_index;

	// At first we set all the obstacle name pointers to NULL in order to
	// mark them as unused.
	//
	for (i = 0; i < MAX_OBSTACLE_NAMES_PER_LEVEL; i++) {
		loadlevel->obstacle_name_list[i] = NULL;
	}

	if (loadlevel->random_dungeon && !loadlevel->dungeon_generated)
		return DataPointer;

	// Now we look for the beginning and end of the map labels section
	//
	obstacle_nameSectionBegin = LocateStringInData(DataPointer, OBSTACLE_LABEL_BEGIN_STRING);
	obstacle_nameSectionEnd = LocateStringInData(obstacle_nameSectionBegin, OBSTACLE_LABEL_END_STRING);

	// We add a terminator at the end, but ONLY TEMPORARY.  The damage will be restored later!
	//
	PreservedLetter = obstacle_nameSectionEnd[0];
	obstacle_nameSectionEnd[0] = 0;
	NumberOfobstacle_namesInThisLevel = CountStringOccurences(obstacle_nameSectionBegin, OBSTACLE_LABEL_ANNOUNCE_STRING);
	DebugPrintf(1, "\nNumber of obstacle labels found in this level : %d.", NumberOfobstacle_namesInThisLevel);

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
	// Now we repair the damage done to the loaded level data
	//
	obstacle_nameSectionEnd[0] = PreservedLetter;
	return obstacle_nameSectionEnd;
}

static void ReadInOneItem(char *ItemPointer, char *ItemsSectionEnd, Item TargetItem)
{

	char *iname = ReadAndMallocStringFromData(ItemPointer, ITEM_NAME_STRING, "\"");
	TargetItem->type = GetItemIndexByName(iname);
	free(iname);

	ReadValueFromString(ItemPointer, ITEM_POS_X_STRING, "%f", &(TargetItem->pos.x), ItemsSectionEnd);
	ReadValueFromString(ItemPointer, ITEM_POS_Y_STRING, "%f", &(TargetItem->pos.y), ItemsSectionEnd);
	ReadValueFromStringWithDefault(ItemPointer, ITEM_DAMRED_BONUS_STRING, "%d", "0", &(TargetItem->damred_bonus), ItemsSectionEnd);
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
	ReadValueFromStringWithDefault(ItemPointer, ITEM_BONUS_TO_DAMREDDAM_STRING, "%d", "0",
				       &(TargetItem->bonus_to_damred_or_damage), ItemsSectionEnd);
	// Now we read in the boni for resistances
	ReadValueFromStringWithDefault(ItemPointer, ITEM_BONUS_TO_RESELE_STRING, "%d", "0",
				       &(TargetItem->bonus_to_resist_electricity), ItemsSectionEnd);
	ReadValueFromStringWithDefault(ItemPointer, ITEM_BONUS_TO_RESFIR_STRING, "%d", "0",
				       &(TargetItem->bonus_to_resist_fire), ItemsSectionEnd);
	// Now we see if the item is identified...
	ReadValueFromString(ItemPointer, ITEM_IS_IDENTIFIED_STRING, "%d", &(TargetItem->is_identified), ItemsSectionEnd);

	DebugPrintf(1, "\nPosX=%f PosY=%f Item=%d", TargetItem->pos.x, TargetItem->pos.y, TargetItem->type);

}

static char *decode_item_section(level *loadlevel, char *data)
{
	int i;
	char Preserved_Letter;
	int NumberOfItemsInThisLevel;
	char *ItemPointer;
	char *ItemsSectionBegin;
	char *ItemsSectionEnd;

	// First we initialize the items arrays with 'empty' information
	//
	for (i = 0; i < MAX_ITEMS_PER_LEVEL; i++) {
		loadlevel->ItemList[i].pos.x = (-1);
		loadlevel->ItemList[i].pos.y = (-1);
		loadlevel->ItemList[i].pos.z = (-1);
		loadlevel->ItemList[i].type = (-1);
		loadlevel->ItemList[i].currently_held_in_hand = FALSE;
	}

	if (loadlevel->random_dungeon && !loadlevel->dungeon_generated)
		return data;

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
			loadlevel->ItemList[i].pos.z = loadlevel->levelnum;
			if (NextItemPointer)
				NextItemPointer[0] = ITEM_NAME_STRING[0];
		}
	}

	// Now we repair the damage done to the loaded level data
	ItemsSectionEnd[0] = Preserved_Letter;
	return ItemsSectionEnd;
}

//----------------------------------------------------------------------
// From here on we take apart the chest items section of the loaded level...
//----------------------------------------------------------------------
static char *decode_chest_item_section(level *loadlevel, char *data)
{
	int i;
	char Preserved_Letter;
	int NumberOfItemsInThisLevel;
	char *ItemPointer;
	char *ItemsSectionBegin;
	char *ItemsSectionEnd;
	char *NextItemPointer;
	// First we initialize the items arrays with 'empty' information
	//
	for (i = 0; i < MAX_ITEMS_PER_LEVEL; i++) {
		loadlevel->ChestItemList[i].pos.x = (-1);
		loadlevel->ChestItemList[i].pos.y = (-1);
		loadlevel->ChestItemList[i].pos.z = (-1);
		loadlevel->ChestItemList[i].type = (-1);
		loadlevel->ChestItemList[i].currently_held_in_hand = FALSE;
	}

	if (loadlevel->random_dungeon && !loadlevel->dungeon_generated)
		return data;

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
			loadlevel->ChestItemList[i].pos.z = loadlevel->levelnum;
			if (NextItemPointer)
				NextItemPointer[0] = ITEM_NAME_STRING[0];
		}
	}

	// Now we repair the damage done to the loaded level data
	ItemsSectionEnd[0] = Preserved_Letter;
	return ItemsSectionEnd;
}

static char *decode_map(level *loadlevel, char *data)
{
	char *map_begin, *map_end;
	char *this_line;
	int i;

	if ((map_begin = strstr(data, MAP_BEGIN_STRING)) == NULL)
		return NULL;
	map_begin += strlen(MAP_BEGIN_STRING) + 1;

	if ((map_end = strstr(data, MAP_END_STRING)) == NULL)
		return NULL;

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
			tmp = strtol(this_line + 4 * col, NULL, 10);
			Buffer[col].floor_value = (Uint16) tmp;
			memset(Buffer[col].obstacles_glued_to_here, -1, MAX_OBSTACLES_GLUED_TO_ONE_MAP_TILE);
		}

		// Now the old text pointer can be replaced with a pointer to the
		// correctly assembled struct...
		//
		loadlevel->map[i] = Buffer;

		curlinepos += nlpos;
	}

	free(this_line);
	return map_end;
}

static char *decode_waypoints(level *loadlevel, char *data)
{
	char *wp_begin, *wp_end;
	char *this_line;
	int i, k;
	int nr, x, y, wp_rnd;
	char *pos;

	// Reset all waypoints prior to reading them from the file
	for (i = 0; i < MAXWAYPOINTS; i++) {
		loadlevel->AllWaypoints[i].x = 0;
		loadlevel->AllWaypoints[i].y = 0;

		for (k = 0; k < MAX_WP_CONNECTIONS; k++) {
			loadlevel->AllWaypoints[i].connections[k] = -1;
		}
	}
	loadlevel->num_waypoints = 0;

	// Find the beginning and end of the waypoint list
	if ((wp_begin = strstr(data, WP_BEGIN_STRING)) == NULL)
		return NULL;
	wp_begin += strlen(WP_BEGIN_STRING) + 1;

	if ((wp_end = strstr(data, WP_END_STRING)) == NULL)
		return NULL;

	short int curlinepos = 0;
	this_line = (char *)MyMalloc(4096);
	
	for (i = 0; i < MAXWAYPOINTS; i++) {
		/* Select the next line */
		short int nlpos = 0;
		memset(this_line, 0, 4096);
		while (wp_begin[curlinepos + nlpos] != '\n')
			nlpos++;
		memcpy(this_line, wp_begin + curlinepos, nlpos);
		this_line[nlpos] = '\0';
		nlpos++;
		
		curlinepos += nlpos;

		if (!strncmp(this_line, wp_end, strlen(WP_END_STRING))) {
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
		loadlevel->AllWaypoints[i].suppress_random_spawn = wp_rnd;

		pos = strstr(this_line, CONNECTION_STRING);
		if (pos == NULL) {
			fprintf(stderr, "Unable to find connection string. i is %i, line is %s, level %i\n", i, this_line,
				loadlevel->levelnum);
		}
		pos += strlen(CONNECTION_STRING);	// skip connection-string
		pos += strspn(pos, WHITE_SPACE);	// skip initial whitespace

		for (k = 0; k < MAX_WP_CONNECTIONS; k++) {
			if (*pos == '\0')
				break;
			int connection;
			int res = sscanf(pos, "%d", &connection);
			if ((connection == -1) || (res == 0) || (res == EOF))
				break;
			loadlevel->AllWaypoints[i].connections[k] = connection;
			pos += strcspn(pos, WHITE_SPACE);	// skip last token
			pos += strspn(pos, WHITE_SPACE);	// skip initial whitespace for next one

		}		// for k < MAX_WP_CONNECTIONS

		loadlevel->AllWaypoints[i].num_connections = k;

	}			// for i < MAXWAYPOINTS

	free(this_line);
	return wp_end;
}


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

	// Each obstacles must to be anchored to exactly one (the closest!)
	// map tile, so that we can find out obstacles 'close' to somewhere
	// more easily...
	//
	for (obstacle_counter = 0; obstacle_counter < MAX_OBSTACLES_ON_MAP; obstacle_counter++) {
		if (loadlevel->obstacle_list[obstacle_counter].type == -1)
			continue;

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
		// Now it can be glued...
		//
		loadlevel->map[y_tile][x_tile].obstacles_glued_to_here[next_free_index] = obstacle_counter;

	}

};				// glue_obstacles_to_floor_tiles_for_level ( int level_num )

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

	// First some security checks against touching the outsides of the map...
	//
	if (!pos_inside_level(map_x, map_y, BoxLevel))
		return (FALSE);

	// We check all the obstacles on this square if they are maybe destructable
	// and if they are, we destruct them, haha
	//
	for (i = 0; i < MAX_OBSTACLES_GLUED_TO_ONE_MAP_TILE; i++) {
		// First we see if there is something glued to this map tile at all.
		//
		target_idx = BoxLevel->map[map_y][map_x].obstacles_glued_to_here[i];
		if (target_idx == -1)
			continue;

		target_obstacle = &(BoxLevel->obstacle_list[target_idx]);

		if (!(obstacle_map[target_obstacle->type].flags & IS_SMASHABLE))
			continue;

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

		// Before we destroy the obstacle (and lose the obstacle type) we see if we
		// should maybe drop some item.
		//
		if (obstacle_map[target_obstacle->type].flags & DROPS_RANDOM_TREASURE)
			DropRandomItem(level, target_obstacle->pos.x, target_obstacle->pos.y, 0, FALSE);

		// Since the obstacle is destroyed, we start a blast at it's position.
		// But here a WARNING WARNING WARNING! is due!  We must not start the
		// blast before the obstacle is removed, because the blast will again
		// cause this very obstacle removal function, so we need to be careful
		// so as not to incide endless recursion.  We memorize the position for
		// later, then delete the obstacle, then we start the blast.
		//
		blast_start_pos.x = target_obstacle->pos.x;
		blast_start_pos.y = target_obstacle->pos.y;

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

		// Now that the obstacle is removed AND ONLY NOW that the obstacle is
		// removed, we may start a blast at this position.  Otherwise we would
		// run into trouble, see the warning further above.
		//
		StartBlast(blast_start_pos.x, blast_start_pos.y, level, DROIDBLAST, 2.0);

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
 * Some structures within Freedroid rpg maps are animated in the sense that the
 * iso image used to display them rotates through a number of different iso
 * images.
 * We generate lists of the animated obstacles for a given visible level to 
 * speed-up things during animation and rendering.
 */
void get_animated_obstacle_lists(struct visible_level *vis_lvl)
{
	int obstacle_index;
	level *Lev = vis_lvl->lvl_pointer;

	INIT_LIST_HEAD(&vis_lvl->animated_obstacles_list);

	/* Now browse obstacles and fill our list of animated obstacles. */
	for (obstacle_index = 0; obstacle_index < MAX_OBSTACLES_ON_MAP; obstacle_index++) {
		if (obstacle_map[Lev->obstacle_list[obstacle_index].type].animate_fn != NULL) {
			struct animated_obstacle *a = MyMalloc(sizeof(struct animated_obstacle));
			a->index = obstacle_index;
			a->animate_fn = obstacle_map[Lev->obstacle_list[obstacle_index].type].animate_fn;
			list_add(&a->node, &vis_lvl->animated_obstacles_list);
			continue;
		}
	}
	
	vis_lvl->animated_obstacles_dirty_flag = FALSE;
}

/*
 * This function marks the animated obstacle lists of one visible_level
 * as being dirty, so that they will be re-generated later.
 */
void dirty_animated_obstacle_lists(int lvl_num)
{
	struct visible_level *visible_lvl, *next_lvl;

	BROWSE_VISIBLE_LEVELS(visible_lvl, next_lvl) {
		if (visible_lvl->lvl_pointer->levelnum == lvl_num) {
			visible_lvl->animated_obstacles_dirty_flag = TRUE;
			return;
		}
	}
}

/*
 * This function clean all the animated obstacle lists
 */
void clear_animated_obstacle_lists(struct visible_level *vis_lvl)
{
	struct animated_obstacle *a, *next;

	list_for_each_entry_safe(a, next, &vis_lvl->animated_obstacles_list, node) {
		list_del(&a->node);
		free(a);
	}
	
	vis_lvl->animated_obstacles_dirty_flag = TRUE;
}

/**
 * This functions reads the specification for a level
 * taken from the ship file.
 *
 * @return pointer to the level
 * @param text buffer containing level description
 */
static level *decode_level(char **buffer)
{
	level *loadlevel;
	char *data = *buffer;

	loadlevel = (level *)MyMalloc(sizeof(level));
	
	if (decode_header(loadlevel, data)) {
		ErrorMessage(__FUNCTION__, "Unable to decode level header!", PLEASE_INFORM, IS_FATAL);
	}
	
	// The order of sections in the file has to match this.	
	data = decode_map(loadlevel, data);
	if (!data) {
		ErrorMessage(__FUNCTION__, "Unable to decode the map for level %d\n", PLEASE_INFORM, IS_FATAL, loadlevel->levelnum);
	}
	data = decode_obstacles(loadlevel, data);
	data = decode_map_labels(loadlevel, data);
	data = decode_obstacle_names(loadlevel, data);
	data = decode_item_section(loadlevel, data);
	data = decode_chest_item_section(loadlevel, data);
	data = decode_waypoints(loadlevel, data);

	// Point the buffer to the end of this level, so the next level can be read
	*buffer = data;
	return loadlevel;
}


/** 
 * Call the random dungeon generator on this level  if this level is marked
 * as being randomly generated and if we are not in the "leveleditor" mode
 * in which case random dungeons must not be considered as generated (so that
 * they will be exported as being non-generated random levels).
 */
static void generate_dungeon_if_needed(level *l)
{
	if (!l->random_dungeon || l->dungeon_generated) {
		return;
	}

	// Generate random dungeon now
	set_dungeon_output(l);
	generate_dungeon(l->xlen, l->ylen, l->random_dungeon);
	l->dungeon_generated = 1;
}

/**
 * This function loads the data for a whole ship
 * Possible return values are : OK and ERR
 */
int LoadShip(char *filename, int compressed)
{
	char *ShipData = NULL;
	FILE *ShipFile;
	int i;

#define END_OF_SHIP_DATA_STRING "*** End of Ship Data ***"

	for (i = 0; i < MAX_LEVELS; i++) {
		if (curShip.AllLevels[i] != NULL) {
			int ydim = curShip.AllLevels[i]->ylen;
			int row = 0;
			for (row = 0; row < ydim; row++) {
				if (curShip.AllLevels[i]->map[row]) {
					free(curShip.AllLevels[i]->map[row]);
					curShip.AllLevels[i]->map[row] = NULL;
				}	
			}

			if (curShip.AllLevels[i]->Levelname) {
				free(curShip.AllLevels[i]->Levelname);
				curShip.AllLevels[i]->Levelname = NULL;
			}

			if (curShip.AllLevels[i]->Background_Song_Name) {
				free(curShip.AllLevels[i]->Background_Song_Name);
				curShip.AllLevels[i]->Background_Song_Name = NULL;
			}
			
			free(curShip.AllLevels[i]);
			curShip.AllLevels[i] = NULL;
		}
	}

	// Read the whole ship-data to memory 
	//
	ShipFile = fopen(filename, "rb");
	if (!ShipFile) {
		ErrorMessage(__FUNCTION__, "Unable to open ship file %s: %s.\n", PLEASE_INFORM, IS_FATAL, filename, strerror(errno));
	}

	if (compressed) {
		if (inflate_stream(ShipFile, (unsigned char **)&ShipData, NULL)) {
			ErrorMessage(__FUNCTION__, "Unable to decompress ship file %s.\n", PLEASE_INFORM, IS_FATAL, filename);
		}
	} else {
		int length = FS_filelength(ShipFile);
		ShipData = malloc(length + 1);
		fread(ShipData, length, 1, ShipFile);
	}

	fclose(ShipFile);

	// Read each level
	int done = 0;
	char *pos = ShipData;
	while (!done) {
		level *this_level = decode_level(&pos);
		int this_levelnum = this_level->levelnum;

		if (this_levelnum >= MAX_LEVELS)
			ErrorMessage(__FUNCTION__, "One levelnumber in savegame (%d) is bigger than the maximum allowed (%d).\n",
				     PLEASE_INFORM, IS_FATAL, this_levelnum, MAX_LEVELS - 1);
		if (curShip.AllLevels[this_levelnum] != NULL)
			ErrorMessage(__FUNCTION__, "Two levels with same levelnumber (%d) found in the savegame.\n", PLEASE_INFORM,
				     IS_FATAL, this_levelnum);

		curShip.AllLevels[this_levelnum] = this_level;
		curShip.num_levels = this_levelnum + 1;
		
		generate_dungeon_if_needed(this_level);
		
		// We attach each obstacle to a floor tile, as a help for 
		// collision detection.
		glue_obstacles_to_floor_tiles_for_level(this_levelnum);

		// Move to the level termination marker
		pos = strstr(pos, LEVEL_END_STRING);
		pos += strlen(LEVEL_END_STRING) + 1;

		// Check if there is another level
		if (!strstr(pos, LEVEL_HEADER_LEVELNUMBER)) {
			done = 1;
		} 
	}

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

};				// void CheckWaypointIntegrity(level *Lev)

/**
 * This should write the obstacle information in human-readable form into
 * a buffer.
 */
static void encode_obstacles_of_this_level(struct auto_string *shipstr, level *Lev)
{
	int i;
	autostr_append(shipstr, "%s\n", OBSTACLE_DATA_BEGIN_STRING);

	for (i = 0; i < MAX_OBSTACLES_ON_MAP; i++) {
		if (Lev->obstacle_list[i].type == (-1))
			continue;

		autostr_append(shipstr, "%s%d %s%3.2f %s%3.2f %s%d \n", OBSTACLE_TYPE_STRING, Lev->obstacle_list[i].type,
				OBSTACLE_X_POSITION_STRING, Lev->obstacle_list[i].pos.x, OBSTACLE_Y_POSITION_STRING,
				Lev->obstacle_list[i].pos.y, OBSTACLE_LABEL_INDEX_STRING, Lev->obstacle_list[i].name_index);
	}

	autostr_append(shipstr, "%s\n", OBSTACLE_DATA_END_STRING);
}

static void EncodeMapLabelsOfThisLevel(struct auto_string *shipstr, level *Lev)
{
	int i;
	autostr_append(shipstr, "%s\n", MAP_LABEL_BEGIN_STRING);

	for (i = 0; i < MAX_MAP_LABELS_PER_LEVEL; i++) {
		if (Lev->labels[i].pos.x == (-1))
			continue;

		autostr_append(shipstr, "%s%d %s%d %s%s\"\n", X_POSITION_OF_LABEL_STRING, Lev->labels[i].pos.x, Y_POSITION_OF_LABEL_STRING,
				            Lev->labels[i].pos.y, LABEL_ITSELF_ANNOUNCE_STRING, Lev->labels[i].label_name);
	}

	autostr_append(shipstr, "%s\n", MAP_LABEL_END_STRING);
}

/**
 * Every map level in a FreedroidRPG 'ship' can have up to 
 * MAX_OBSTACLE_NAMES_PER_LEVEL obstacles, that have a label attached to
 * them.  Such obstacle labels are very useful when modifying obstacles
 * from within game events and triggers.  The obstacle labels are stored
 * in a small subsection of the whole level data.  This function encodes
 * this small subsection and puts all the obstacle data into a human
 * readable text string for saving with the map file.
 */
static void encode_obstacle_names_of_this_level(struct auto_string *shipstr, level *Lev)
{
	int i;

	autostr_append(shipstr, "%s\n", OBSTACLE_LABEL_BEGIN_STRING);

	for (i = 0; i < MAX_OBSTACLE_NAMES_PER_LEVEL; i++) {
		if (Lev->obstacle_name_list[i] == NULL)
			continue;
		autostr_append(shipstr, "%s%d %s%s\"\n", INDEX_OF_OBSTACLE_NAME, i, OBSTACLE_LABEL_ANNOUNCE_STRING, Lev->obstacle_name_list[i]);
	}

	autostr_append(shipstr, "%s\n", OBSTACLE_LABEL_END_STRING);
}

/**
 *
 * 
 */
static void WriteOutOneItem(struct auto_string *shipstr, Item ItemToWriteOut)
{

	autostr_append(shipstr, "%s%s\" %s%f %s%f ", ITEM_NAME_STRING, ItemMap[ItemToWriteOut->type].item_name,
			ITEM_POS_X_STRING, ItemToWriteOut->pos.x, ITEM_POS_Y_STRING, ItemToWriteOut->pos.y);

	if (ItemToWriteOut->damred_bonus) {
		autostr_append(shipstr, "%s%d ", ITEM_DAMRED_BONUS_STRING, ItemToWriteOut->damred_bonus);
	}

	autostr_append(shipstr, "%s%d %s%d %s%d %s%f %s%d %s%d ", ITEM_DAMAGE_STRING, ItemToWriteOut->damage,
			ITEM_DAMAGE_MODIFIER_STRING, ItemToWriteOut->damage_modifier,
			ITEM_MAX_DURATION_STRING, ItemToWriteOut->max_duration,
			ITEM_CUR_DURATION_STRING, ItemToWriteOut->current_duration,
			ITEM_AMMO_CLIP_STRING, ItemToWriteOut->ammo_clip,
			ITEM_MULTIPLICITY_STRING, ItemToWriteOut->multiplicity);

	if (ItemToWriteOut->prefix_code != -1) {
		autostr_append(shipstr, "%s%d ", ITEM_PREFIX_CODE_STRING, ItemToWriteOut->prefix_code);
	}

	if (ItemToWriteOut->suffix_code != -1) {
		autostr_append(shipstr, "%s%d ", ITEM_SUFFIX_CODE_STRING, ItemToWriteOut->suffix_code);
	}

	if (ItemToWriteOut->bonus_to_str) {
		autostr_append(shipstr, "%s%d ", ITEM_BONUS_TO_STR_STRING, ItemToWriteOut->bonus_to_str);
	}
	if (ItemToWriteOut->bonus_to_dex) {
		autostr_append(shipstr, "%s%d ", ITEM_BONUS_TO_DEX_STRING, ItemToWriteOut->bonus_to_dex);
	}
	if (ItemToWriteOut->bonus_to_vit) {
		autostr_append(shipstr, "%s%d ", ITEM_BONUS_TO_VIT_STRING, ItemToWriteOut->bonus_to_vit);
	}
	if (ItemToWriteOut->bonus_to_mag) {
		autostr_append(shipstr, "%s%d ", ITEM_BONUS_TO_MAG_STRING, ItemToWriteOut->bonus_to_mag);
	}
	if (ItemToWriteOut->bonus_to_all_attributes) {
		autostr_append(shipstr, "%s%d ", ITEM_BONUS_TO_ALLATT_STRING, ItemToWriteOut->bonus_to_all_attributes);
	}
	// Now we save the secondary stat boni

	if (ItemToWriteOut->bonus_to_life) {
		autostr_append(shipstr, "%s%d ", ITEM_BONUS_TO_LIFE_STRING, ItemToWriteOut->bonus_to_life);
	}
	if (ItemToWriteOut->bonus_to_health_recovery) {
		autostr_append(shipstr, "%s%f ", ITEM_BONUS_TO_HEALTH_RECOVERY_STRING, ItemToWriteOut->bonus_to_health_recovery);
	}
	if (ItemToWriteOut->bonus_to_force) {
		autostr_append(shipstr, "%s%d ", ITEM_BONUS_TO_FORCE_STRING, ItemToWriteOut->bonus_to_force);
	}
	if (ItemToWriteOut->bonus_to_cooling_rate) {
		autostr_append(shipstr, "%s%f ", ITEM_BONUS_TO_MANA_RECOVERY_STRING, ItemToWriteOut->bonus_to_cooling_rate);
	}
	if (ItemToWriteOut->bonus_to_tohit) {
		autostr_append(shipstr, "%s%d ", ITEM_BONUS_TO_TOHIT_STRING, ItemToWriteOut->bonus_to_tohit);
	}
	if (ItemToWriteOut->bonus_to_damred_or_damage) {
		autostr_append(shipstr, "%s%d ", ITEM_BONUS_TO_DAMREDDAM_STRING, ItemToWriteOut->bonus_to_damred_or_damage);
	}
	// Now we save the resistanc boni

	if (ItemToWriteOut->bonus_to_resist_electricity) {
		autostr_append(shipstr, "%s%d ", ITEM_BONUS_TO_RESELE_STRING, ItemToWriteOut->bonus_to_resist_electricity);
	}
	if (ItemToWriteOut->bonus_to_resist_fire) {
		autostr_append(shipstr, "%s%d ", ITEM_BONUS_TO_RESFIR_STRING, ItemToWriteOut->bonus_to_resist_fire);
	}

	autostr_append(shipstr, "%s%d ", ITEM_IS_IDENTIFIED_STRING, ItemToWriteOut->is_identified);

	autostr_append(shipstr, "\n");
}

static void EncodeItemSectionOfThisLevel(struct auto_string *shipstr, level *Lev)
{
	int i;

	autostr_append(shipstr, "%s\n", ITEMS_SECTION_BEGIN_STRING);

	// Now we write out the bulk of items infos
	//
	for (i = 0; i < MAX_ITEMS_PER_LEVEL; i++) {
		if (Lev->ItemList[i].type == (-1))
			continue;

		WriteOutOneItem(shipstr, &(Lev->ItemList[i]));

	}

	autostr_append(shipstr, "%s\n", ITEMS_SECTION_END_STRING);
}

/**
 *
 */
static void EncodeChestItemSectionOfThisLevel(struct auto_string *shipstr, level *Lev)
{
	int i;

	autostr_append(shipstr, "%s\n", CHEST_ITEMS_SECTION_BEGIN_STRING);

	// Now we write out the bulk of items infos
	//
	for (i = 0; i < MAX_ITEMS_PER_LEVEL; i++) {
		if (Lev->ChestItemList[i].type == (-1))
			continue;

		WriteOutOneItem(shipstr, &(Lev->ChestItemList[i]));
	}

	autostr_append(shipstr, "%s\n", CHEST_ITEMS_SECTION_END_STRING);	
}

static void encode_waypoints_of_this_level(struct auto_string *shipstr, level *Lev)
{
	waypoint *this_wp;
	int i,j;

	// There might be LEADING -1 entries in front of other connection entries.
	// This is unwanted and shall be corrected here.
	CheckWaypointIntegrity(Lev);

	autostr_append(shipstr, "%s\n", WP_BEGIN_STRING);

	for (i = 0; i < Lev->num_waypoints; i++) {
		autostr_append(shipstr, "Nr.=%3d x=%4d y=%4d rnd=%1d\t %s", i, Lev->AllWaypoints[i].x, Lev->AllWaypoints[i].y, Lev->AllWaypoints[i].suppress_random_spawn, CONNECTION_STRING);

		this_wp = &Lev->AllWaypoints[i];
		for (j = 0; j < this_wp->num_connections; j++) {
			autostr_append(shipstr, " %3d", Lev->AllWaypoints[i].connections[j]);
		}		// for connections 

		autostr_append(shipstr, "\n");
	}			// for waypoints 
}

/**
 * This function translates map data into human readable map code, that
 * can later be written to the map file on disk.
 */
static void TranslateToHumanReadable(struct auto_string *str, map_tile * MapInfo, int LineLength)
{
	int col;

	for (col = 0; col < LineLength; col++) {
		autostr_append(str, "%3d ", MapInfo[col].floor_value);
	}

	autostr_append(str, "\n");
}

/**
 * This function generates savable text out of the current level data
 *
 * If reset_random_levels is TRUE, then the random levels are saved
 * "un-generated" (typical usage: freedroid.levels).
 *
 */
static void encode_level_for_saving(struct auto_string *shipstr, level *lvl, int reset_random_levels)
{
	int i;
	int xlen = lvl->xlen, ylen = lvl->ylen;

	// Write level header	
	autostr_append(shipstr, "%s %d\n\
xlen of this level: %d\n\
ylen of this level: %d\n\
light radius bonus of this level: %d\n\
minimal light on this level: %d\n\
infinite_running_on_this_level: %d\n\
random dungeon: %d\n\
dungeon generated: %d\n\
jump target north: %d\n\
jump target south: %d\n\
jump target east: %d\n\
jump target west: %d\n\
use underground lighting: %d\n", LEVEL_HEADER_LEVELNUMBER, lvl->levelnum, lvl->xlen, lvl->ylen,
		lvl->light_bonus, lvl->minimum_light_value,
		lvl->infinite_running_on_this_level,
		lvl->random_dungeon,
		(reset_random_levels && lvl->random_dungeon) ? 0 : lvl->dungeon_generated,
		lvl->jump_target_north, lvl->jump_target_south, lvl->jump_target_east,
		lvl->jump_target_west, lvl->use_underground_lighting);

	autostr_append(shipstr, "%s%s\"\n%s%s\n", LEVEL_NAME_STRING, lvl->Levelname,
			BACKGROUND_SONG_NAME_STRING, lvl->Background_Song_Name);

	autostr_append(shipstr, "%s\n", MAP_BEGIN_STRING);

	// Now in the loop each line of map data should be saved as a whole
	for (i = 0; i < ylen; i++) {
		if (!(reset_random_levels && lvl->random_dungeon)) {
			TranslateToHumanReadable(shipstr, lvl->map[i], xlen);
		} else {
			int j = xlen;
			while (j--) {
				autostr_append(shipstr, "  4 ");
			}
			autostr_append(shipstr, "\n");
		}
	}

	autostr_append(shipstr, "%s\n", MAP_END_STRING);

	if (!(reset_random_levels && lvl->random_dungeon)) {
		encode_obstacles_of_this_level(shipstr, lvl);

		EncodeMapLabelsOfThisLevel(shipstr, lvl);

		encode_obstacle_names_of_this_level(shipstr, lvl);

		EncodeItemSectionOfThisLevel(shipstr, lvl);

		EncodeChestItemSectionOfThisLevel(shipstr, lvl);

		encode_waypoints_of_this_level(shipstr, lvl);
	}

	autostr_append(shipstr, "%s\n----------------------------------------------------------------------\n", 
			LEVEL_END_STRING);
}

/**
 * This function should save a whole ship to disk to the given filename.
 * It is not only used by the level editor, but also by the function that
 * saves whole games.
 *
 * If reset_random_levels is TRUE, then the random levels are saved
 * "un-generated" (typical usage: freedroid.levels).
 */
int SaveShip(const char *filename, int reset_random_levels, int compress)
{
	int i;
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
			encode_level_for_saving(shipstr, curShip.AllLevels[i], reset_random_levels);
		}
	}

	autostr_append(shipstr, "%s\n\n", END_OF_SHIP_DATA_STRING);

	if (compress) { 
		deflate_to_stream((unsigned char *)shipstr->value, shipstr->length+1, ShipFile);
	}	else {
		fwrite((unsigned char *)shipstr->value, shipstr->length+1, 1, ShipFile); 
	}

	if (fclose(ShipFile) == EOF) {
		ErrorMessage(__FUNCTION__, "Closing of ship file failed!", PLEASE_INFORM, IS_FATAL);
		return ERR;
	}

	free_autostr(shipstr);
	return OK;
}

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

	//Now its time to start decoding the droids file.
	//For that, we must get it into memory first.
	//The procedure is the same as with LoadShip
	//
	find_file(filename, MAP_DIR, fpath, 0);

	MainDroidsFilePointer = ReadAndMallocAndTerminateFile(fpath, END_OF_DROID_DATA_STRING);

	// The Droid crew file for this map is now completely read into memory
	// It's now time to decode the file and to fill the array of enemys with
	// new droids of the given types.
	//
	enemy_reset_fabric();

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

	while ((SearchPointer = strstr(SearchPointer, SPECIAL_FORCE_INDICATION_STRING)) != NULL) {
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

		// Create a new enemy, and initialize its 'identity' and 'global state'
		// (the enemy will be fully initialized by respawn_level())
		enemy *newen = enemy_new(ListIndex);
		newen->SpecialForce = 1;

		ReadValueFromString(SearchPointer, "Fixed=", "%hd", &(newen->CompletelyFixed), EndOfThisLevelData);
		ReadValueFromString(SearchPointer, "Marker=", "%d", &(newen->marker), EndOfThisLevelData);
		ReadValueFromStringWithDefault(SearchPointer, "MaxDistanceToHome=", "%hd", "0", &(newen->max_distance_to_home),
					       EndOfThisLevelData);
		ReadValueFromString(SearchPointer, "Friendly=", "%hd", &(newen->is_friendly), EndOfThisLevelData);

		StartMapLabel = ReadAndMallocStringFromData(SearchPointer, "StartLabel=\"", "\"");
		ResolveMapLabelOnShip(StartMapLabel, &StartupLocation);
		newen->pos.x = StartupLocation.x;
		newen->pos.y = StartupLocation.y;
		newen->pos.z = OurLevelNumber;
		free(StartMapLabel);

		YesNoString = ReadAndMallocStringFromData(SearchPointer, "RushTux=\"", "\"");
		if (strcmp(YesNoString, "yes") == 0) {
			newen->will_rush_tux = TRUE;
		} else if (strcmp(YesNoString, "no") == 0) {
			newen->will_rush_tux = FALSE;
		} else {
			ErrorMessage(__FUNCTION__, "\
The droid specification of a droid in ReturnOfTux.droids should contain an \n\
answer that is either 'yes' or 'no', but which was neither 'yes' nor 'no'.\n\
This indicated a corrupted freedroid.ruleset file with an error at least in\n\
the item specification section.", PLEASE_INFORM, IS_FATAL);
		}
		free(YesNoString);

		newen->dialog_section_name = ReadAndMallocStringFromData(SearchPointer, "UseDialog=\"", "\"");
		if (strlen(newen->dialog_section_name) >= MAX_LENGTH_FOR_DIALOG_SECTION_NAME - 1) {
			ErrorMessage(__FUNCTION__, "\
The dialog section specification string for a bot was too large.\n\
This indicated a corrupted ReturnOfTux.droids file with an error when specifying\n\
the dialog section name for one special force droid/character.", PLEASE_INFORM, IS_FATAL);
		}

		newen->short_description_text = ReadAndMallocStringFromData(SearchPointer, "ShortLabel=_\"", "\"");
		if (strlen(newen->short_description_text) >= MAX_LENGTH_OF_SHORT_DESCRIPTION_STRING) {
			ErrorMessage(__FUNCTION__, "\
The short description specification string for a bot was too large.\n\
This indicated a corrupted ReturnOfTux.droids file with an error when specifying\n\
the dialog section name for one special force droid/character.", PLEASE_INFORM, IS_FATAL);
		}

		if (strstr(SearchPointer, "on_death_drop_item_name")) {
			YesNoString = ReadAndMallocStringFromData(SearchPointer, "on_death_drop_item_name=\"", "\"");
			newen->on_death_drop_item_code = GetItemIndexByName(YesNoString);
			free(YesNoString);
		} else
			newen->on_death_drop_item_code = -1;

		list_add(&(newen->global_list), &alive_bots_head);
		list_add(&(newen->level_list), &level_bots_head[OurLevelNumber]);

	}			// while Special force droid found...

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

	// At this point, the List "ListOfTypesAllowed" has been filled with the NUMBERS of
	// the allowed types.  The number of different allowed types found is also available.
	// That means that now we can add the appropriate droid types into the list of existing
	// droids in that mission.

	RealNumberOfRandomDroids = MyRandom(MaxRand - MinRand) + MinRand;

	while (RealNumberOfRandomDroids--) {
		// Create a new enemy, and initialize its 'identity' and 'global state'
		// (the enemy will be fully initialized by respawn_level())
		enemy *newen = enemy_new(ListOfTypesAllowed[MyRandom(DifferentRandomTypes - 1)]);
		newen->pos.x = newen->pos.y = -1;
		newen->pos.z = OurLevelNumber;
		newen->on_death_drop_item_code = -1;
		newen->dialog_section_name = strdup("StandardBotAfterTakeover");

		switch (atoi(Druidmap[newen->type].druidname)) {
		case 123:
			newen->short_description_text = strdup(_("123 Acolyte"));
			break;
		case 139:
			newen->short_description_text = strdup(_("139 Templar"));
			break;
		case 247:
			newen->short_description_text = strdup(_("247 Banshee"));
			break;
		case 249:
			newen->short_description_text = strdup(_("249 Chicago"));
			break;
		case 296:
			newen->short_description_text = strdup(_("296 Sawmill"));
			break;
		case 302:
			newen->short_description_text = strdup(_("302 Nemesis"));
			break;
		case 329:
			newen->short_description_text = strdup(_("329 Sparkie"));
			break;
		case 420:
			newen->short_description_text = strdup(_("420 Surgeon"));
			break;
		case 476:
			newen->short_description_text = strdup(_("476 Coward"));
			break;
		case 493:
			newen->short_description_text = strdup(_("493 Spinster"));
			break;
		case 516:
			newen->short_description_text = strdup(_("516 Ghoul"));
			break;
		case 543:
			newen->short_description_text = strdup(_("543 Forest Harvester"));
			break;
		case 571:
			newen->short_description_text = strdup(_("571 Apollo"));
			break;
		case 598:
			newen->short_description_text = strdup(_("598 Minister"));
			break;
		case 615:
			newen->short_description_text = strdup(_("615 Firedevil"));
			break;
		case 629:
			newen->short_description_text = strdup(_("629 Spitfire"));
			break;
		case 711:
			newen->short_description_text = strdup(_("711 Grillmeister"));
			break;
		case 742:
			newen->short_description_text = strdup(_("742 Zeus"));
			break;
		case 751:
			newen->short_description_text = strdup(_("751 Soviet"));
			break;
		case 821:
			newen->short_description_text = strdup(_("821 Ufo"));
			break;
		case 834:
			newen->short_description_text = strdup(_("834 Wisp"));
			break;
		case 883:
			newen->short_description_text = strdup(_("883 Dalex"));
			break;
		case 999:
			newen->short_description_text = strdup(_("999 Cerebrum"));
			break;
		default:
			newen->short_description_text = strdup(_("No Description For This One"));
		};

		list_add(&(newen->global_list), &alive_bots_head);
		list_add(&(newen->level_list), &level_bots_head[OurLevelNumber]);
	}			// while (enemy-limit of this level not reached) 

	SearchPointer = SectionPointer;
	GetThisLevelsSpecialForces(SearchPointer, OurLevelNumber, EndOfThisLevelData);

	// End bot's initialization, and put them onto a waypoint.
	respawn_level(OurLevelNumber);
};				// void GetThisLevelsDroids( char* SectionPointer )

/**
 * This function determines wether a given object on x/y is visible to
 * the 001 or not (due to some walls or something in between
 * 
 * Return values are TRUE or FALSE accodinly
 *
 */
int IsVisible(GPS objpos)
{

	// For the purpose of visibility checking, we might as well exclude objects
	// that are too far away to ever be visible and thereby save some checks of
	// longer lines on the map, that wouldn't be nescessary or helpful anyway.
	//
	if ((fabsf(Me.pos.x - objpos->x) > FLOOR_TILES_VISIBLE_AROUND_TUX) ||
	    (fabsf(Me.pos.y - objpos->y) > FLOOR_TILES_VISIBLE_AROUND_TUX))
		return (FALSE);

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
void animate_obstacles(void)
{
	struct animated_obstacle *a;
	struct visible_level *visible_lvl, *next_lvl;

	animation_timeline_advance();
	
	BROWSE_VISIBLE_LEVELS(visible_lvl, next_lvl) {
		// If animated_obstacles list is dirty, regenerate it
		if (visible_lvl->animated_obstacles_dirty_flag) {
			clear_animated_obstacle_lists(visible_lvl);
			get_animated_obstacle_lists(visible_lvl);
		}
		// Call animation function of each animated object
		list_for_each_entry(a, &visible_lvl->animated_obstacles_list, node) {
			if (a->animate_fn != NULL) {
				a->animate_fn(visible_lvl->lvl_pointer, a->index);
			}
		}
	}
	

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
