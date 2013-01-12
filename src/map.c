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

#define 	TELEPORT_PAIR_STRING	"teleport pair:"

void GetThisLevelsDroids(char *section_pointer);

/**
 * This function removes all volatile obstacles from a given level.
 * An example of a volatile obstacle is the blood.
 * If the blood doesn't vanish, then there will be more and more blood,
 * especially after the bots on the level have respawned a few times.
 * Therefore we need this function, which will remove all traces of blood
 * from a given level.
 */
static void remove_volatile_obstacles(int level_num)
{
	int i;

	// We pass through all the obstacles, deleting those
	// that are 'blood'.
	//
	for (i = 0; i < MAX_OBSTACLES_ON_MAP; i++) {
		int obstacle_type = curShip.AllLevels[level_num]->obstacle_list[i].type;
		if (obstacle_type == -1)
			continue;
		if (get_obstacle_spec(obstacle_type)->flags & IS_VOLATILE)
			del_obstacle(&curShip.AllLevels[level_num]->obstacle_list[i]);
	}
}

/**
 * This function will make all blood obstacles vanish, all dead bots come
 * back to life, and get all bots return to a wandering state.
 */
void respawn_level(int level_num)
{
	enemy *erot, *nerot;

	int wp_num = curShip.AllLevels[level_num]->waypoints.size;
	char wp_used[wp_num]; // is a waypoint already used ?
	memset(wp_used, 0, wp_num);

	// First we remove all the volatile obstacles...
	//
	remove_volatile_obstacles(level_num);

	// Now we can give new life to dead bots...
	//
	BROWSE_DEAD_BOTS_SAFE(erot, nerot) {
		if (erot->pos.z != level_num || Droidmap[erot->type].is_human)
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
			erot->faction = FACTION_BOTS;
			erot->has_been_taken_over = FALSE;
			erot->CompletelyFixed = FALSE;
			erot->follow_tux = FALSE;
		}
		
		erot->has_greeted_influencer = FALSE;

		if (wp_num) {
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
		} else {
			ErrorMessage(__FUNCTION__, "There is no waypoint on level %d - unable to place random bots.\n",
					PLEASE_INFORM, IS_WARNING_ONLY, level_num);
		}
	}
}

/**
 * This is the ultimate function to resolve a given label within a
 * given SHIP.
 */
void ResolveMapLabelOnShip(const char *MapLabel, location * PositionPointer)
{
	map_label *m;
	int i;

	for (i = 0; i < curShip.num_levels; i++) {
		if (!level_exists(i))
			continue;

		m = get_map_label(curShip.AllLevels[i], MapLabel);
		if (m) {
			PositionPointer->x = m->pos.x + 0.5;
			PositionPointer->y = m->pos.y + 0.5;
			PositionPointer->level = i;
			return;
		}
	}

	ErrorMessage(__FUNCTION__, "\
Resolving map label %s failed on the complete ship!\n\
This is a severe error in the game data of FreedroidRPG.", PLEASE_INFORM, IS_FATAL, MapLabel);

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

	/* Read floor_layers */
	fp += strlen("floor layers:");
	while (*(fp + off) != '\n')
		off++;
	fp[off] = 0;
	loadlevel->floor_layers = atoi(fp);
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

	if (!strncmp(fp, TELEPORT_PAIR_STRING, strlen(TELEPORT_PAIR_STRING))) {
		fp += strlen(TELEPORT_PAIR_STRING);
		while (*(fp + off) != '\n')
			off++;
		fp[off] = 0;
		loadlevel->teleport_pair = atoi(fp);
		fp[off] = '\n';
		fp += off + 1;
		off = 0;
	} else {
		loadlevel->teleport_pair = 0;
	}

	if (!strncmp(fp, "dungeon generated:", 18)) {
		fp += strlen("dungeon generated:");
		while (*(fp + off) != '\n')
			off++;
		fp[off] = 0;
		loadlevel->dungeon_generated = atoi(fp);
		fp[off] = '\n';
		fp += off + 1;
	} else {
		loadlevel->dungeon_generated = 0;
	}

	if (!strncmp(fp, "environmental flags:", 20)) {
		fp += strlen("environmental flags:");
		while (*(fp + off) != '\n')
			off++;
		fp[off] = 0;
		loadlevel->flags = atoi(fp);
		fp[off] = '\n';
		fp += off + 1;
	} else {
		loadlevel->flags = 0;
	}

	if (loadlevel->ylen >= MAX_MAP_LINES) {
		ErrorMessage(__FUNCTION__, "\
A map/level in FreedroidRPG which was supposed to load has more map lines than allowed\n\
for a map/level as by the constant MAX_MAP_LINES in defs.h.\n\
Sorry, but unless this constant is raised, FreedroidRPG will refuse to load this map.", PLEASE_INFORM, IS_FATAL);
	}
}

static void decode_random_droids(level *loadlevel, char *data)
{
	char *search_ptr;
	char *end_ptr = data;

#define DROIDS_NUMBER_INDICATION_STRING "number of random droids: "
#define ALLOWED_TYPE_INDICATION_STRING "random droid types: "

	// Read the number of random droids for this level
	end_ptr = strstr(data, ALLOWED_TYPE_INDICATION_STRING);
	ReadValueFromString(data, DROIDS_NUMBER_INDICATION_STRING, "%d", &loadlevel->random_droids.nr, end_ptr);

	data = strstr(data, ALLOWED_TYPE_INDICATION_STRING);

	// Now we read in the type(s) of random droids for this level
	search_ptr = ReadAndMallocStringFromDataOptional(data, ALLOWED_TYPE_INDICATION_STRING, "\n");
	if (search_ptr && (loadlevel->random_droids.nr > 0)) {
		char *droid_type_ptr = search_ptr;
		while (*droid_type_ptr) {
			while (*droid_type_ptr && isspace(*droid_type_ptr)) {
				droid_type_ptr++;
			}
			int droid_type_length = 0;
			char *ptr = droid_type_ptr;
			while (isalnum(*ptr)) {
				ptr++;
				droid_type_length++;
			}
			if (!droid_type_length)
				break;

			char type_indication_string[droid_type_length + 1];
			strncpy(type_indication_string, droid_type_ptr, droid_type_length);
			type_indication_string[droid_type_length] = 0;

			int droid_type = get_droid_type(type_indication_string);

			loadlevel->random_droids.types[loadlevel->random_droids.types_size++] = droid_type;

			droid_type_ptr += droid_type_length;
			if (*droid_type_ptr)
				droid_type_ptr++; //skip the comma
			}
			free(search_ptr);
	}
}

static int decode_header(level *loadlevel, char *data)
{
	data = strstr(data, LEVEL_HEADER_LEVELNUMBER);
	if (!data)
		return 1;

	decode_interfaces(loadlevel, data);
	decode_dimensions(loadlevel, data);
	decode_random_droids(loadlevel, data);

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
	float x, y;
	int type;

	// First we initialize the obstacles with 'empty' information
	//
	for (i = 0; i < MAX_OBSTACLES_ON_MAP; i++) {
		loadlevel->obstacle_list[i].type = -1;
		loadlevel->obstacle_list[i].pos.x = -1;
		loadlevel->obstacle_list[i].pos.y = -1;
		loadlevel->obstacle_list[i].pos.z = loadlevel->levelnum;
		loadlevel->obstacle_list[i].timestamp = 0;
		loadlevel->obstacle_list[i].frame_index = 0;
	}

	if (loadlevel->random_dungeon && !loadlevel->dungeon_generated)
		return DataPointer;

	// Now we look for the beginning and end of the obstacle section
	//
	obstacle_SectionBegin = LocateStringInData(DataPointer, OBSTACLE_DATA_BEGIN_STRING) + strlen(OBSTACLE_DATA_BEGIN_STRING) + 1;

	// Now we decode all the obstacle information
	//
	curfield = obstacle_SectionBegin;
	while (*curfield != '/') {
		//structure of obstacle entry is :      // t59 x2.50 y63.50 l-1 d-1 
		//we read the type
		curfield++;
		curfieldend = curfield;
		while ((*curfieldend) != ' ')
			curfieldend++;
		(*curfieldend) = 0;
		type = atoi(curfield);
		(*curfieldend) = ' ';

		//we read the X position
		curfield = curfieldend + 2;
		curfieldend += 2;
		while ((*curfieldend) != ' ')
			curfieldend++;
		(*curfieldend) = 0;
		x = atof(curfield);
		(*curfieldend) = ' ';

		//Y position
		curfield = curfieldend + 2;
		curfieldend += 2;
		while ((*curfieldend) != ' ')
			curfieldend++;
		(*curfieldend) = 0;
		y = atof(curfield);
		(*curfieldend) = ' ';

		while ((*curfield) != '\n')
			curfield++;
		curfield++;

		add_obstacle(loadlevel, x, y, type);
	}

	return curfield;
}

/**
 * Next we extract the map labels of this level WITHOUT destroying
 * or damaging the data in the process!
 */
static char *decode_map_labels(level *loadlevel, char *data)
{
	char *label_name;
	int i, x, y;

	// Initialize map labels
	dynarray_init(&loadlevel->map_labels, 10, sizeof(struct map_label));

	if (loadlevel->random_dungeon && !loadlevel->dungeon_generated)
		return data;

	// Now we look for the beginning and end of the map labels section
	char *map_label_begin = LocateStringInData(data, MAP_LABEL_BEGIN_STRING) + strlen(MAP_LABEL_BEGIN_STRING) + 1;
	char *map_label_end = LocateStringInData(map_label_begin, MAP_LABEL_END_STRING);
	*map_label_end = '\0';

	// Get the number of map labels in this level
	int nb_map_labels_in_level = CountStringOccurences(map_label_begin, LABEL_ITSELF_ANNOUNCE_STRING);
	DebugPrintf(1, "\nNumber of map labels found in this level : %d.", nb_map_labels_in_level);

	// Now we decode all the map label information
	for (i = 0; i < nb_map_labels_in_level ; i++) {
		if (i)
			map_label_begin = strstr(map_label_begin + 1, X_POSITION_OF_LABEL_STRING);

		// Get the position of the map label
		ReadValueFromString(map_label_begin, X_POSITION_OF_LABEL_STRING, "%d", &x, map_label_end);
		ReadValueFromString(map_label_begin, Y_POSITION_OF_LABEL_STRING, "%d", &y, map_label_end);

		// Get the name of the map label
		label_name = ReadAndMallocStringFromData(map_label_begin, LABEL_ITSELF_ANNOUNCE_STRING, "\"");

		// Add the map label on the level
		add_map_label(loadlevel, x, y, label_name);

		DebugPrintf(1, "\npos.x=%d pos.y=%d label_name=\"%s\"", x, y, label_name);
	}

	*map_label_end = MAP_LABEL_END_STRING[0];
	return map_label_end;
}

static void ReadInOneItem(char *ItemPointer, char *ItemsSectionEnd, item *TargetItem)
{
	init_item(TargetItem);

	char *iname = ReadAndMallocStringFromData(ItemPointer, ITEM_NAME_STRING, "\"");
	TargetItem->type = GetItemIndexByName(iname);
	free(iname);

	ReadValueFromString(ItemPointer, ITEM_POS_X_STRING, "%f", &(TargetItem->pos.x), ItemsSectionEnd);
	ReadValueFromString(ItemPointer, ITEM_POS_Y_STRING, "%f", &(TargetItem->pos.y), ItemsSectionEnd);
	ReadValueFromStringWithDefault(ItemPointer, ITEM_ARMOR_CLASS_BASE_STRING, "%d", "0", &(TargetItem->armor_class), ItemsSectionEnd);
	ReadValueFromString(ItemPointer, ITEM_MAX_DURABILITY_STRING, "%d", &(TargetItem->max_durability), ItemsSectionEnd);
	ReadValueFromString(ItemPointer, ITEM_CUR_DURABILITY_STRING, "%f", &(TargetItem->current_durability), ItemsSectionEnd);
	ReadValueFromString(ItemPointer, ITEM_AMMO_CLIP_STRING, "%d", &(TargetItem->ammo_clip), ItemsSectionEnd);
	ReadValueFromString(ItemPointer, ITEM_MULTIPLICITY_STRING, "%d", &(TargetItem->multiplicity), ItemsSectionEnd);

	// Read the socket data of the item and calculate bonuses using it.
	int i;
	int socket_count;
	ReadValueFromStringWithDefault(ItemPointer, ITEM_SOCKETS_SIZE_STRING, "%d", "0", &socket_count, ItemsSectionEnd);
	for (i = 0; i < socket_count; i++) {
		char type_string[32];
		char addon_string[32];
		struct upgrade_socket socket;
		sprintf(type_string, "%s%d=", ITEM_SOCKET_TYPE_STRING, i);
		sprintf(addon_string, "%s%d=", ITEM_SOCKET_ADDON_STRING, i);
		ReadValueFromString(ItemPointer, type_string, "%d", &socket.type, ItemsSectionEnd);
		socket.addon = ReadAndMallocStringFromDataOptional(ItemPointer, addon_string, "\"");
		create_upgrade_socket(TargetItem, socket.type, socket.addon);
		free(socket.addon);
	}
	calculate_item_bonuses(TargetItem);

	DebugPrintf(1, "\nPosX=%f PosY=%f Item=%d", TargetItem->pos.x, TargetItem->pos.y, TargetItem->type);

}

static char *decode_extension_chest(char *ext, void **data)
{
	struct dynarray *chest = dynarray_alloc(1, sizeof(item));
	char *item_str, *item_end;
	
	item_str = ext;

	while (*item_str != '}') {
		// Find end of this item (beginning of next item)
		item_end = item_str;
		while (*item_end != '\n')
			item_end++;
		while (isspace(*item_end))
			item_end++;

		// Read the item on this line
		item new_item;
		ReadInOneItem(item_str, item_end, &new_item);

		// Add the item to the dynarray
		dynarray_add(chest, &new_item, sizeof(item));

		// Move to the next item definition
		item_str = item_end;
	}


	*data = chest;
	return item_str;
}

static char *decode_extension_label(char *ext, void **data)
{
	char *end = ext;
	while (*end != '\n')
		end++;

	*end = '\0';
	*data = strdup(ext);
	*end = '\n';

	while (*end != '}')
		end++;

	return end;
}

static char *decode_extension_dialog(char *ext, void **data)
{
	// dialog and label extensions are both a string
	return decode_extension_label(ext, data);
}

static char *decode_obstacle_extensions(level *loadlevel, char *data)
{
	dynarray_init(&loadlevel->obstacle_extensions, 10, sizeof(struct obstacle_extension));

	if (loadlevel->random_dungeon && !loadlevel->dungeon_generated)
		return data;

	char *ext_begin = LocateStringInData(data, OBSTACLE_EXTENSIONS_BEGIN_STRING);
	char *ext_end = LocateStringInData(ext_begin, OBSTACLE_EXTENSIONS_END_STRING);
	*ext_end = '\0';

	while (1) {
		// Look for the next extension
		ext_begin = strstr(ext_begin, "idx=");
		if (!ext_begin)
			break;

		// Read extension information
		int index;
		int type;
		void *ext_data = NULL;
		sscanf(ext_begin, "idx=%d type=%d", &index, &type);

		// Move to the extension data definition
		ext_begin = strstr(ext_begin, "data={\n");
		while (*ext_begin != '\n')
			ext_begin++;
		while (isspace(*ext_begin))
			ext_begin++;

		// Read the extension data
		switch (type) {
			case OBSTACLE_EXTENSION_CHEST_ITEMS:
				ext_begin = decode_extension_chest(ext_begin, &ext_data);
				break;
			case OBSTACLE_EXTENSION_LABEL:
				ext_begin = decode_extension_label(ext_begin, &ext_data);
				break;
			case OBSTACLE_EXTENSION_DIALOGFILE:
				ext_begin = decode_extension_dialog(ext_begin, &ext_data);
				break;
		}

		// Add the obstacle extension on the level
		add_obstacle_extension(loadlevel, &(loadlevel->obstacle_list[index]), type, ext_data);
	}

	*ext_end = OBSTACLE_EXTENSIONS_END_STRING[0];
	return ext_end;
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
		init_item(&loadlevel->ItemList[i]);
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
	unsigned int curlinepos = 0;
	this_line = (char *)MyMalloc(4096);

	/* read MapData */
	for (i = 0; i < loadlevel->ylen; i++) {
		int col;
		int layer;
		map_tile *Buffer;
		int tmp;

		/* Select the next line */
		unsigned int nlpos = 0;
		memset(this_line, 0, 4096);
		while (map_begin[curlinepos + nlpos] != '\n')
			nlpos++;
		memcpy(this_line, map_begin + curlinepos, nlpos);
		this_line[nlpos] = '\0';
		nlpos++;

		/* Decode it */
		Buffer = MyMalloc((loadlevel->xlen + 10) * sizeof(map_tile));
		for (col = 0; col < loadlevel->xlen; col++) {
			for (layer = 0; layer < loadlevel->floor_layers; layer++) {
				tmp = strtol(this_line + 4 * (loadlevel->floor_layers * col + layer), NULL, 10);
				Buffer[col].floor_values[layer] = (Uint16) tmp;
			}
			// Make sure that all floor layers are always initialized properly.
			for ( ; layer < MAX_FLOOR_LAYERS; layer++)
				Buffer[col].floor_values[layer] = ISO_FLOOR_EMPTY;
			dynarray_init(&Buffer[col].glued_obstacles, 0, sizeof(int));
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
	int nr, x, y, wp_rnd;
	char *pos;

	// Initialize waypoints
	dynarray_init(&loadlevel->waypoints, 2, sizeof(struct waypoint));

	if (loadlevel->random_dungeon && !loadlevel->dungeon_generated)
		return data;

	// Find the beginning and end of the waypoint list
	if ((wp_begin = strstr(data, WP_BEGIN_STRING)) == NULL)
		return NULL;
	wp_begin += strlen(WP_BEGIN_STRING) + 1;

	if ((wp_end = strstr(data, WP_END_STRING)) == NULL)
		return NULL;

	int curlinepos = 0;
	this_line = (char *)MyMalloc(4096);
	
	while (1) {
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
			break;
		}
		wp_rnd = 0;
		sscanf(this_line, "Nr.=%d \t x=%d \t y=%d   rnd=%d", &nr, &x, &y, &wp_rnd);

		// Create a new waypoint
		waypoint new_wp;
		new_wp.x = x;
		new_wp.y = y;
		new_wp.suppress_random_spawn = wp_rnd;

		// Initalize the connections of the new waypoint
		dynarray_init(&new_wp.connections, 2, sizeof(int));

		pos = strstr(this_line, CONNECTION_STRING);
		if (pos == NULL) {
			fprintf(stderr, "Unable to find connection string. line is %s, level %i\n", this_line,
				loadlevel->levelnum);
		}
		pos += strlen(CONNECTION_STRING);	// skip connection-string
		pos += strspn(pos, WHITE_SPACE);	// skip initial whitespace

		while (1) {
			if (*pos == '\0')
				break;
			int connection;
			int res = sscanf(pos, "%d", &connection);
			if ((connection == -1) || (res == 0) || (res == EOF))
				break;

			// Add the connection on this waypoint
			dynarray_add(&new_wp.connections, &connection, sizeof(int));

			pos += strcspn(pos, WHITE_SPACE);	// skip last token
			pos += strspn(pos, WHITE_SPACE);	// skip initial whitespace for next one
		}

		// Add the waypoint on the level
		dynarray_add(&loadlevel->waypoints, &new_wp, sizeof(struct waypoint));
	}

	free(this_line);
	return wp_end;
}

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
	obstacle *target_obstacle;
	int smashed_something = FALSE;
	moderately_finepoint blast_start_pos;

	// First some security checks against touching the outsides of the map...
	//
	if (!pos_inside_level(map_x, map_y, BoxLevel))
		return (FALSE);

	// We check all the obstacles on this square if they are maybe destructible
	// and if they are, we destruct them, haha
	//
	for (i = 0; i < BoxLevel->map[map_y][map_x].glued_obstacles.size; i++) {
		// First we see if there is something glued to this map tile at all.
		//
		target_idx = ((int *)(BoxLevel->map[map_y][map_x].glued_obstacles.arr))[i];

		target_obstacle = &(BoxLevel->obstacle_list[target_idx]);

		obstacle_spec *obstacle_spec = get_obstacle_spec(target_obstacle->type);
		if (!(obstacle_spec->flags & IS_SMASHABLE))
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

		// Since the obstacle is destroyed, we start a blast at it's position.
		// But here a WARNING WARNING WARNING! is due!  We must not start the
		// blast before the obstacle is removed, because the blast will again
		// cause this very obstacle removal function, so we need to be careful
		// so as not to incite endless recursion.  We memorize the position for
		// later, then delete the obstacle, then we start the blast.
		//
		blast_start_pos.x = target_obstacle->pos.x;
		blast_start_pos.y = target_obstacle->pos.y;

		int obstacle_drops_treasure
			= obstacle_spec->flags & DROPS_RANDOM_TREASURE;

		// Let the automap know that we've updated things
		update_obstacle_automap(level, target_obstacle);

		// Now we really smash the obstacle, i.e. we can set it's type to the debris that has
		// been configured for this obstacle type.  In if there is nothing configured (i.e. -1 set)
		// then we'll just delete the obstacle in question entirely.  For this we got a standard function to
		// safely do it and not make some errors into the glue structure or obstacles lists...
		//
		if (obstacle_spec->result_type_after_smashing_once == (-1)) {
			del_obstacle(target_obstacle);
		} else {
			target_obstacle->type = obstacle_spec->result_type_after_smashing_once;
		}

		// Drop items after destroying the obstacle, in order to avoid collisions
		if (obstacle_drops_treasure)
			DropRandomItem(level, target_obstacle->pos.x, target_obstacle->pos.y, 0, FALSE);

		// Now that the obstacle is removed AND ONLY NOW that the obstacle is
		// removed, we may start a blast at this position.  Otherwise we would
		// run into trouble, see the warning further above.
		StartBlast(blast_start_pos.x, blast_start_pos.y, 
				level, obstacle_spec->blast_type, 0.0, FACTION_SELF, obstacle_spec->smashed_sound);
	}

	return smashed_something;
}

/**
 * When a destructible type of obstacle gets hit, e.g. by a blast 
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
 * given position in the given layer.
 * Floor layers are indexed from 0 to lvl->floor_layers - 1. The lowest
 * floor layer is #0. Every map has at least one layer.
 */
Uint16 get_map_brick(level *lvl, float x, float y, int layer)
{
	Uint16 BrickWanted;
	int RoundX, RoundY;

	gps vpos = { x, y, lvl->levelnum };
	gps rpos;
	if (!resolve_virtual_position(&rpos, &vpos)) {
		return ISO_FLOOR_EMPTY;
	}
	RoundX = (int)rintf(rpos.x);
	RoundY = (int)rintf(rpos.y);

	BrickWanted = curShip.AllLevels[rpos.z]->map[RoundY][RoundX].floor_values[layer];

	if (BrickWanted >= underlay_floor_tile_filenames.size) {
		if (BrickWanted < MAX_UNDERLAY_FLOOR_TILES || (BrickWanted - MAX_UNDERLAY_FLOOR_TILES) >= overlay_floor_tile_filenames.size) {
			ErrorMessage(__FUNCTION__, "Level %d at %d %d in %d layer uses an unknown floor tile: %d.\n", PLEASE_INFORM, IS_WARNING_ONLY,
				lvl->levelnum, RoundX, RoundY, layer, BrickWanted);
			return ISO_FLOOR_EMPTY;
		}
	}

	return BrickWanted;
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
	data = decode_item_section(loadlevel, data);
	data = decode_obstacle_extensions(loadlevel, data);
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
	generate_dungeon(l->xlen, l->ylen, l->random_dungeon, l->teleport_pair);
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

	// Free existing level data
	for (i = 0; i < MAX_LEVELS; i++) {
		if (level_exists(i)) {
			level *lvl = curShip.AllLevels[i];
			int row = 0;
			int col = 0;

			// Map tiles
			for (row = 0; row < lvl->ylen; row++) {
				if (lvl->map[row]) {
					for (col = 0; col < lvl->xlen; col++) {
						dynarray_free(&lvl->map[row][col].glued_obstacles);
					}

					free(curShip.AllLevels[i]->map[row]);
					curShip.AllLevels[i]->map[row] = NULL;
				}	
			}

			// Level strings
			if (lvl->Levelname) {
				free(lvl->Levelname);
				lvl->Levelname = NULL;
			}

			if (lvl->Background_Song_Name) {
				free(lvl->Background_Song_Name);
				lvl->Background_Song_Name = NULL;
			}

			// Waypoints
			int w;
			for (w = 0; w < lvl->waypoints.size; w++) {
				struct waypoint *wpts = lvl->waypoints.arr;
				dynarray_free(&wpts[w].connections);
			}

			dynarray_free(&lvl->waypoints);

			// Obstacle extensions
			free_obstacle_extensions(lvl);
	
			// Map labels
			free_map_labels(lvl);

			// Random droids
			lvl->random_droids.types_size = 0;

			free(lvl);
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
		if (!fread(ShipData, length, 1, ShipFile))
			ErrorMessage(__FUNCTION__, "Reading ship file %s failed with fread().\n", PLEASE_INFORM, IS_FATAL, filename);
		ShipData[length] = 0;
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
		if (level_exists(this_levelnum))
			ErrorMessage(__FUNCTION__, "Two levels with same levelnumber (%d) found in the savegame.\n", PLEASE_INFORM,
				     IS_FATAL, this_levelnum);

		curShip.AllLevels[this_levelnum] = this_level;
		curShip.num_levels = this_levelnum + 1;
		
		generate_dungeon_if_needed(this_level);

		// Move to the level termination marker
		pos = strstr(pos, LEVEL_END_STRING);
		pos += strlen(LEVEL_END_STRING) + 1;

		// Check if there is another level
		if (!strstr(pos, LEVEL_HEADER_LEVELNUMBER)) {
			done = 1;
		} 
	}

	// Check for consistency of levels
	int check_level = curShip.num_levels;
	while (check_level--) {
		if (!level_exists(check_level)) {
			ErrorMessage(__FUNCTION__, "Level number %d should exist but is NULL.", PLEASE_INFORM, IS_FATAL, check_level);
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
 * This should write the obstacle information in human-readable form into
 * a buffer.
 */
static void encode_obstacles_of_this_level(struct auto_string *shipstr, level *Lev)
{
	int i;
	autostr_append(shipstr, "%s\n", OBSTACLE_DATA_BEGIN_STRING);

	defrag_obstacle_array(Lev);

	for (i = 0; i < MAX_OBSTACLES_ON_MAP; i++) {
		if (Lev->obstacle_list[i].type == (-1))
			continue;

		autostr_append(shipstr, "%s%d %s%3.2f %s%3.2f\n", OBSTACLE_TYPE_STRING, Lev->obstacle_list[i].type,
				OBSTACLE_X_POSITION_STRING, Lev->obstacle_list[i].pos.x, OBSTACLE_Y_POSITION_STRING,
				Lev->obstacle_list[i].pos.y);
	}

	autostr_append(shipstr, "%s\n", OBSTACLE_DATA_END_STRING);
}

static void encode_map_labels(struct auto_string *shipstr, level *lvl)
{
	int i;
	struct map_label *map_label;

	autostr_append(shipstr, "%s\n", MAP_LABEL_BEGIN_STRING);

	for (i = 0; i < lvl->map_labels.size; i++) {
		// Get the map label
		map_label = &ACCESS_MAP_LABEL(lvl->map_labels, i);

		// Encode map label
		autostr_append(shipstr, "%s%d %s%d %s%s\"\n", X_POSITION_OF_LABEL_STRING, map_label->pos.x, Y_POSITION_OF_LABEL_STRING,
				            map_label->pos.y, LABEL_ITSELF_ANNOUNCE_STRING, map_label->label_name);
	}

	autostr_append(shipstr, "%s\n", MAP_LABEL_END_STRING);
}

/**
 *
 * 
 */
static void WriteOutOneItem(struct auto_string *shipstr, item *ItemToWriteOut)
{

	autostr_append(shipstr, "%s%s\" %s%f %s%f ", ITEM_NAME_STRING, ItemMap[ItemToWriteOut->type].item_name,
			ITEM_POS_X_STRING, ItemToWriteOut->pos.x, ITEM_POS_Y_STRING, ItemToWriteOut->pos.y);

	if (ItemToWriteOut->armor_class) {
		autostr_append(shipstr, "%s%d ", ITEM_ARMOR_CLASS_BASE_STRING, ItemToWriteOut->armor_class);
	}

	autostr_append(shipstr, "%s%d %s%f %s%d %s%d",
			ITEM_MAX_DURABILITY_STRING, ItemToWriteOut->max_durability,
			ITEM_CUR_DURABILITY_STRING, ItemToWriteOut->current_durability,
			ITEM_AMMO_CLIP_STRING, ItemToWriteOut->ammo_clip,
			ITEM_MULTIPLICITY_STRING, ItemToWriteOut->multiplicity);

	// Write the sockets of the item. The bonuses can be reconstructed from
	// these easily so we don't need to write them at all.
	if (ItemToWriteOut->upgrade_sockets.size) {
		int i;
		autostr_append(shipstr, "%s%d ", ITEM_SOCKETS_SIZE_STRING, ItemToWriteOut->upgrade_sockets.size);
		for (i = 0; i < ItemToWriteOut->upgrade_sockets.size; i++) {
			struct upgrade_socket *socket = &ItemToWriteOut->upgrade_sockets.arr[i];
			autostr_append(shipstr, "%s%d=%d ", ITEM_SOCKET_TYPE_STRING, i, socket->type);
			if (socket->addon) {
				autostr_append(shipstr, "%s%d=%s\" ", ITEM_SOCKET_ADDON_STRING, i, socket->addon);
			}
		}
	}

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

static void encode_extension_chest(struct auto_string *shipstr, struct obstacle_extension *ext)
{
	int i;
	struct dynarray *da = ext->data;

	for (i = 0; i < da->size; i++) {
		item *it = &((item *)da->arr)[i];
		if (it->type == -1)
			continue;

		autostr_append(shipstr, "\t");
		WriteOutOneItem(shipstr, it);
	}
}

static void encode_extension_label(struct auto_string *shipstr, struct obstacle_extension *ext)
{
	const char *label = ext->data;

	autostr_append(shipstr, "\t%s\n", label);
}

static void encode_extension_dialog(struct auto_string *shipstr, struct obstacle_extension *ext)
{
	// dialog and label extensions are both a string
	encode_extension_label(shipstr, ext);
}

static void encode_obstacle_extensions(struct auto_string *shipstr, level *l)
{
	int i;
	autostr_append(shipstr, "%s\n", OBSTACLE_EXTENSIONS_BEGIN_STRING);
	for (i = 0; i < l->obstacle_extensions.size; i++) {
		struct obstacle_extension *ext = &ACCESS_OBSTACLE_EXTENSION(l->obstacle_extensions, i);

		if (ext->type == 0)
			continue;

		autostr_append(shipstr, "idx=%d type=%d data={\n", get_obstacle_index(l, ext->obs), ext->type);

		switch ((enum obstacle_extension_type)(ext->type)) {
			case OBSTACLE_EXTENSION_CHEST_ITEMS:
				encode_extension_chest(shipstr, ext);
				break;
			case OBSTACLE_EXTENSION_LABEL:
				encode_extension_label(shipstr, ext);
				break;
			case OBSTACLE_EXTENSION_DIALOGFILE:
				encode_extension_dialog(shipstr, ext);
				break;
		}

		autostr_append(shipstr, "}\n");
	}
	autostr_append(shipstr, "%s\n", OBSTACLE_EXTENSIONS_END_STRING);
}

static void encode_waypoints(struct auto_string *shipstr, level *lvl)
{
	waypoint *wpts = lvl->waypoints.arr;
	int *connections;
	int i, j;

	autostr_append(shipstr, "%s\n", WP_BEGIN_STRING);

	for (i = 0; i < lvl->waypoints.size; i++) {
		// Encode the waypoint
		autostr_append(shipstr, "Nr.=%3d x=%4d y=%4d rnd=%1d\t %s", i, wpts[i].x, wpts[i].y, wpts[i].suppress_random_spawn, CONNECTION_STRING);

		// Get the connections of the waypoint
		connections = wpts[i].connections.arr;

		for (j = 0; j < wpts[i].connections.size; j++) {
			// Encode the connection of the waypoint
			autostr_append(shipstr, " %3d", connections[j]);
		}

		autostr_append(shipstr, "\n");
	}
}

/**
 * This function translates map data into human readable map code, that
 * can later be written to the map file on disk.
 */
static void TranslateToHumanReadable(struct auto_string *str, map_tile * MapInfo, int LineLength, int layers)
{
	int col;
	int layer;

	for (col = 0; col < LineLength; col++) {
		for (layer = 0; layer < layers; layer++)
			autostr_append(str, "%3d ", MapInfo[col].floor_values[layer]);
	}

	autostr_append(str, "\n");
}

/**
 * This function generates savable text out of the current level data
 *
 * If reset_random_levels is TRUE, then the random levels are saved
 * "un-generated" (typical usage: levels.dat).
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
floor layers: %d\n\
light radius bonus of this level: %d\n\
minimal light on this level: %d\n\
infinite_running_on_this_level: %d\n\
random dungeon: %d\n\
teleport pair: %d\n\
dungeon generated: %d\n\
environmental flags: %d\n\
jump target north: %d\n\
jump target south: %d\n\
jump target east: %d\n\
jump target west: %d\n", LEVEL_HEADER_LEVELNUMBER, lvl->levelnum,
		lvl->xlen, lvl->ylen, lvl->floor_layers,
		lvl->light_bonus, lvl->minimum_light_value,
		lvl->infinite_running_on_this_level,
		lvl->random_dungeon,
		lvl->teleport_pair,
		(reset_random_levels && lvl->random_dungeon) ? 0 : lvl->dungeon_generated,
		lvl->flags,
		lvl->jump_target_north,
		lvl->jump_target_south,
		lvl->jump_target_east,
		lvl->jump_target_west);

	autostr_append(shipstr, "number of random droids: %d\n", lvl->random_droids.nr);
	autostr_append(shipstr, "random droid types: ");

	for (i = 0; i < lvl->random_droids.types_size; i++) {
		if (i)
			autostr_append(shipstr, ", ");
		autostr_append(shipstr, "%s", Droidmap[lvl->random_droids.types[i]].droidname);
	}

	autostr_append(shipstr, "\n%s%s\"\n%s%s\n", LEVEL_NAME_STRING, lvl->Levelname,
			BACKGROUND_SONG_NAME_STRING, lvl->Background_Song_Name);

	autostr_append(shipstr, "%s\n", MAP_BEGIN_STRING);

	// Now in the loop each line of map data should be saved as a whole
	for (i = 0; i < ylen; i++) {
		if (!(reset_random_levels && lvl->random_dungeon)) {
			TranslateToHumanReadable(shipstr, lvl->map[i], xlen, lvl->floor_layers);
		} else {
			int j = xlen;
			while (j--) {
				autostr_append(shipstr, "  0 ");
			}
			autostr_append(shipstr, "\n");
		}
	}

	autostr_append(shipstr, "%s\n", MAP_END_STRING);

	if (!(reset_random_levels && lvl->random_dungeon)) {
		encode_obstacles_of_this_level(shipstr, lvl);

		encode_map_labels(shipstr, lvl);

		EncodeItemSectionOfThisLevel(shipstr, lvl);

		encode_obstacle_extensions(shipstr, lvl);

		encode_waypoints(shipstr, lvl);
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
 * "un-generated" (typical usage: levels.dat).
 * @return 0 if OK, 1 on error
 */
int SaveShip(const char *filename, int reset_random_levels, int compress)
{
	int i;
	FILE *ShipFile = NULL;
	struct auto_string *shipstr;

	// Open the ship file 
	if ((ShipFile = fopen(filename, "wb")) == NULL) {
		ErrorMessage(__FUNCTION__, "Error opening ship file %s for writing.", NO_NEED_TO_INFORM, IS_WARNING_ONLY, filename);
		return ERR;
	}

	shipstr	= alloc_autostr(1048576);
	autostr_printf(shipstr, "\n");
	
	// Save all the levels
	for (i = 0; i < curShip.num_levels; i++) {
		if (level_exists(i)) {
			encode_level_for_saving(shipstr, curShip.AllLevels[i], reset_random_levels);
		}
	}

	autostr_append(shipstr, "%s\n\n", END_OF_SHIP_DATA_STRING);

	if (compress) { 
		deflate_to_stream((unsigned char *)shipstr->value, shipstr->length, ShipFile);
	}	else {
		if (fwrite((unsigned char *)shipstr->value, shipstr->length, 1, ShipFile) != 1) {
			ErrorMessage(__FUNCTION__, "Error writing ship file %s.", PLEASE_INFORM, IS_WARNING_ONLY, filename);
			fclose(ShipFile);
			free_autostr(shipstr);
			return ERR;
		}
	}

	if (fclose(ShipFile) == EOF) {
		ErrorMessage(__FUNCTION__, "Closing of ship file failed!", PLEASE_INFORM, IS_WARNING_ONLY);
		free_autostr(shipstr);
		return ERR;
	}

	free_autostr(shipstr);
	return OK;
}

int save_special_forces(const char *filename)
{
	FILE *s_forces_file = NULL;
	struct auto_string *s_forces_str;
	level *lvl;
	int i;

	if ((s_forces_file = fopen(filename, "wb")) == NULL) {
		ErrorMessage(__FUNCTION__, "Error opening Special Forces file %s for writing.", NO_NEED_TO_INFORM, IS_WARNING_ONLY, filename);
		return ERR;
	}

	s_forces_str = alloc_autostr(64);

	for (i = 0; i < curShip.num_levels; i++) {
		if (!level_exists(i))
			continue;

		lvl = curShip.AllLevels[i];
		autostr_append(s_forces_str, "** Beginning of new Level **\n");
		autostr_append(s_forces_str, "Level=%d\n\n",  lvl->levelnum);

		enemy *en;

		list_for_each_entry_reverse(en, &level_bots_head[lvl->levelnum], level_list) {
			if (!en->SpecialForce)
				continue;

			autostr_append(s_forces_str, "T=%s: ", Droidmap[en->type].droidname);
			autostr_append(s_forces_str, "PosX=%d PosY=%d ", (int)en->pos.x, (int)en->pos.y);
			autostr_append(s_forces_str, "Faction=\"%s\" ", get_faction_from_id(en->faction));
			autostr_append(s_forces_str, "UseDialog=\"%s\" ", en->dialog_section_name);

			autostr_append(s_forces_str, "ShortLabel=\"%s\" ", en->short_description_text);
			autostr_append(s_forces_str, "Marker=%d ", en->marker);
			autostr_append(s_forces_str, "RushTux=%d ", en->will_rush_tux);

			autostr_append(s_forces_str, "Fixed=%hi ", en->CompletelyFixed);
			autostr_append(s_forces_str, "DropItemName=\"%s\" ",
						(en->on_death_drop_item_code == -1) ? "none" : ItemMap[en->on_death_drop_item_code].item_name);
			autostr_append(s_forces_str, "MaxDistanceToHome=%hd\n", en->max_distance_to_home);
		}

		autostr_append(s_forces_str, "** End of this levels Special Forces data **\n");
		autostr_append(s_forces_str, "---------------------------------------------------------\n");
	}

	autostr_append(s_forces_str, "*** End of Droid Data ***");

	if (fwrite((unsigned char *)s_forces_str->value, s_forces_str->length, 1, s_forces_file) != 1) {
		ErrorMessage(__FUNCTION__, "Error writing SpecialForces file %s.", PLEASE_INFORM, IS_WARNING_ONLY, filename);
		fclose(s_forces_file);
		goto out;
	}

	if (fclose(s_forces_file) == EOF) {
		ErrorMessage(__FUNCTION__, "Closing of Special Forces file failed!", PLEASE_INFORM, IS_WARNING_ONLY);
		goto out;
	}

out:	free_autostr(s_forces_str);
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
 * This function initializes all enemies, which means that enemies are
 * filled in into the enemy list according to the enemies types that 
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
#define DROIDS_LEVEL_DESCRIPTION_END_STRING "** End of this levels Special Forces data **"

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
static void GetThisLevelsSpecialForces(char *search_pointer, int our_level_number, char *lvl_end_location)
{
	int droid_type;
#define SPECIAL_FORCE_INDICATION_STRING "T="

	while ((search_pointer = strstr(search_pointer, SPECIAL_FORCE_INDICATION_STRING)) != NULL) {
		char *special_droid = ReadAndMallocStringFromData(search_pointer, SPECIAL_FORCE_INDICATION_STRING, "\n");
		char *special_droid_end = special_droid + strlen(special_droid);
		search_pointer += strlen(SPECIAL_FORCE_INDICATION_STRING);
		//identify what model of droid to display:
		char *ptr = special_droid;
		int droid_type_length = 0;
		while (isalnum(*ptr)) {
			ptr++;
			droid_type_length++;
		}
		char type_indication_string[droid_type_length + 1];
		strncpy(type_indication_string, special_droid, droid_type_length);
		type_indication_string[droid_type_length] = 0;

		droid_type = get_droid_type(type_indication_string);

		// Create a new enemy, and initialize its 'identity' and 'global state'
		// (the enemy will be fully initialized by respawn_level())
		enemy *newen = enemy_new(droid_type);
		newen->SpecialForce = TRUE;

		ReadValueFromStringWithDefault(special_droid, "Fixed=", "%hd", "0", &(newen->CompletelyFixed), special_droid_end);
		ReadValueFromStringWithDefault(special_droid, "Marker=", "%d", "0000", &(newen->marker), special_droid_end);
		ReadValueFromStringWithDefault(special_droid, "MaxDistanceToHome=", "%hd", "0", &(newen->max_distance_to_home),
					       special_droid_end);

		char *faction = ReadAndMallocStringFromData(special_droid, "Faction=\"", "\"");
		newen->faction = get_faction_id(faction);
		free(faction);

		char *x, *y;
		x = ReadAndMallocStringFromData(special_droid, "PosX=", " ");
		y = ReadAndMallocStringFromData(special_droid, "PosY=", " ");

		newen->pos.x = strtof(x, NULL);
		newen->pos.y = strtof(y, NULL);
		newen->pos.z = our_level_number;
		free(x);
		free(y);

		ReadValueFromStringWithDefault(special_droid, "RushTux=", "%hu", "0", &(newen->will_rush_tux), special_droid_end);

		newen->dialog_section_name = ReadAndMallocStringFromData(special_droid, "UseDialog=\"", "\"");
		npc_get(newen->dialog_section_name); // Check that we have a valid dialog.

		if (newen->short_description_text)
			free(newen->short_description_text);

		newen->short_description_text = ReadAndMallocStringFromData(special_droid, "ShortLabel=\"", "\"");;

		char *death_drop;
		death_drop = ReadAndMallocStringFromData(special_droid, "DropItemName=\"", "\"");
		if (strcmp(death_drop, "none")) {
			newen->on_death_drop_item_code = GetItemIndexByName(death_drop);
		} else {
			newen->on_death_drop_item_code = -1;
		}
		free(death_drop);
		free(special_droid);
		enemy_insert_into_lists(newen, TRUE);
	}

};

/**
 * This function receives a pointer to the already read in crew section
 * in a already read in droids file and decodes all the contents of that
 * droid section to fill the AllEnemys array with droid types according
 * to the specifications made in the file.
 */
void GetThisLevelsDroids(char *section_pointer)
{
	int our_level_number;
	char *search_ptr;
	char *lvl_end_location;
	int random_droids;
	int *allowed_type_list;
	level *lvl;

#define DROIDS_LEVEL_INDICATION_STRING "Level="
#define DROIDS_LEVEL_END_INDICATION_STRING "** End of this levels Special Forces data **"

	lvl_end_location = LocateStringInData(section_pointer, DROIDS_LEVEL_END_INDICATION_STRING);
	lvl_end_location[0] = 0;

	// Now we read in the level number for this level
	ReadValueFromString(section_pointer, DROIDS_LEVEL_INDICATION_STRING, "%d", &our_level_number, lvl_end_location);

	lvl = curShip.AllLevels[our_level_number];

	// At this point, the List "allowed_type_list" has been filled with the NUMBERS of
	// the allowed types.  The number of different allowed types found is also available.
	// That means that now we can add the appropriate droid types into the list of existing
	// droids in that mission.

	random_droids = lvl->random_droids.nr;
	allowed_type_list = lvl->random_droids.types;

	while (random_droids--) {
		// Create a new enemy, and initialize its 'identity' and 'global state'
		// (the enemy will be fully initialized by respawn_level())
		enemy *newen = enemy_new(allowed_type_list[MyRandom(lvl->random_droids.types_size - 1)]);
		newen->pos.x = newen->pos.y = -1;
		newen->pos.z = our_level_number;
		newen->on_death_drop_item_code = -1;
		newen->dialog_section_name = strdup("AfterTakeover");
		newen->faction = FACTION_BOTS;

		enemy_insert_into_lists(newen, TRUE);
	}			// while (enemy-limit of this level not reached) 

	search_ptr = section_pointer;
	GetThisLevelsSpecialForces(search_ptr, our_level_number, lvl_end_location);

	// End bot's initialization, and put them onto a waypoint.
	respawn_level(our_level_number);
};

/**
 * This function determines whether a given object on x/y is visible to
 * the 001 or not (due to some walls or something in between
 * 
 * Return values are TRUE or FALSE accordingly
 *
 */
int IsVisible(gps *objpos)
{

	// For the purpose of visibility checking, we might as well exclude objects
	// that are too far away to ever be visible and thereby save some checks of
	// longer lines on the map, that wouldn't be necessary or helpful anyway.
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
void translate_map_point_to_screen_pixel_func(float x_map_pos, float y_map_pos, int *x_res, int *y_res)
{
	float zoom_factor = 1.0;

	if (game_status == INSIDE_LVLEDITOR && GameConfig.zoom_is_on) {
		zoom_factor = lvledit_zoomfact_inv();
	}
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
