/* 
 *
 *   Copyright (c) 2002, 2003 Johannes Prix
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
 * This file contains all functions for the heart of the level editor.
 */

#define _leveleditor_c

#include "system.h"

#include "defs.h"
#include "struct.h"
#include "global.h"
#include "proto.h"

#include "SDL_rotozoom.h"

#include "lvledit/lvledit.h"
#include "lvledit/lvledit_validator.h"

#include "lvledit/lvledit_actions.h"
#include "lvledit/lvledit_display.h"
#include "lvledit/lvledit_grass_actions.h"
#include "lvledit/lvledit_map.h"
#include "lvledit/lvledit_menu.h"
#include "lvledit/lvledit_tools.h"
#include "lvledit/lvledit_widgets.h"

int OriginWaypoint = (-1);

char VanishingMessage[10000] = "";
float VanishingMessageEndDate = 0;
int FirstBlock = 0;
int *object_list;
int number_of_walls[NUMBER_OF_LEVEL_EDITOR_GROUPS];
int level_editor_done = FALSE;

LIST_HEAD(quickbar_entries);

/**
 * Return the X coordinate of the block we are on.
 */
int EditX(void)
{
	int BlockX = rintf(Me.pos.x - 0.5);
	if (BlockX < 0) {
		BlockX = 0;
		Me.pos.x = 0.51;
	}
	return BlockX;
}

/**
 * Return the Y coordinate of the block we are on.
 */
int EditY(void)
{
	int BlockY = rintf(Me.pos.y - 0.5);
	if (BlockY < 0) {
		BlockY = 0;
		Me.pos.y = 0.51;
	}
	return BlockY;
}

/**
 * Return a pointer to the level we are currently editing.
 */
level *EditLevel(void)
{
	return CURLEVEL();
}

void leveleditor_print_object_info(enum leveleditor_object_type type, int *array, int idx, char *str)
{
	switch (type) {
	case OBJECT_FLOOR:
		sprintf(str, "Floor tile number %d\n", array[idx]);
		break;
	case OBJECT_OBSTACLE:
		sprintf(str, "Obs. number %d, %s\n", array[idx], obstacle_map[array[idx]].filename);
		break;
	case OBJECT_WAYPOINT:
		sprintf(str, "Waypt %s connection, %s for random spawn\n", "two way", array[idx] ? "NOK" : "OK");
		break;
	default:
		*str = 0;
	}
}

iso_image *leveleditor_get_object_image(enum leveleditor_object_type type, int *array, int idx)
{
	extern iso_image level_editor_waypoint_cursor[];
	switch (type) {
	case OBJECT_FLOOR:
		return &(floor_iso_images[array[idx]]);
	case OBJECT_OBSTACLE:
		return get_obstacle_image(array[idx]);
	case OBJECT_WAYPOINT:
		return &(level_editor_waypoint_cursor[idx]);
	case OBJECT_NPC:
	case OBJECT_ANY:
		ErrorMessage(__FUNCTION__, "Abstract object type %d for leveleditor not supported.\n", PLEASE_INFORM, IS_FATAL, type);
		break;
	}

	return NULL;
}

/* ------------------
 * Quickbar functions
 * ------------------
 */
struct quickbar_entry *quickbar_getentry(int id)
{
	int i = 0;
	struct list_head *node;
	list_for_each(node, &quickbar_entries) {
		if (id == i) {
			struct quickbar_entry *entry = list_entry(node, struct quickbar_entry, node);
			return entry;
		}
		i++;
	}
	return NULL;
}

iso_image *quickbar_getimage(int selected_index, int *placing_floor)
{
/*    struct quickbar_entry *entry = quickbar_getentry ( selected_index );
    if (!entry) 
	return NULL;
    if (entry->obstacle_type == LEVEL_EDITOR_SELECTION_FLOOR) {
	*placing_floor = TRUE;
	return &floor_iso_images  [ entry->id ];
    } else {
	return &obstacle_map [wall_indices [ entry -> obstacle_type ] [ entry->id ] ] . image;
    }*/
	return NULL;
}

/**
 *  @fn void quickbar_additem (struct quickbar_entry *entry)
 * 
 *  @brief Inserts an item in a sorted list 
 */
void quickbar_additem(struct quickbar_entry *entry)
{
	struct quickbar_entry *tmp1, *tmp2;
	struct quickbar_entry *smallest, *biggest;
	struct list_head *node;
	/* The smallest element (if the list is non-empty) is the last element */
	smallest = list_entry(quickbar_entries.prev, struct quickbar_entry, node);
	/* Biggest one */
	biggest = list_entry(quickbar_entries.next, struct quickbar_entry, node);

	/* If the list is empty or if the entry we want to insert is smaller than 
	 * the smallest element, just insert the entry */
	if ((list_empty(&quickbar_entries)) || (entry->used < smallest->used)) {
		list_add_tail(&entry->node, &quickbar_entries);
		/* If it's bigger than the biggest one, let it be the first */
	} else if (entry->used > biggest->used) {
		list_add(&entry->node, &quickbar_entries);
	} else {
		/* We know the element is between two entries, so let's find the place */
		list_for_each(node, &quickbar_entries) {
			tmp1 = list_entry(node, struct quickbar_entry, node);
			tmp2 = list_entry(node->next, struct quickbar_entry, node);
			if (tmp1->used >= entry->used && entry->used >= tmp2->used) {
				list_add(&entry->node, &tmp1->node);
				break;
			}
		}
	}

	int i = 0;
	list_for_each(node, &quickbar_entries) i++;
	number_of_walls[LEVEL_EDITOR_SELECTION_QUICK] = i;
}

void quickbar_use(int obstacle, int id)
{
	struct list_head *node;
	struct quickbar_entry *entry = NULL;;
	list_for_each(node, &quickbar_entries) {
		entry = list_entry(node, struct quickbar_entry, node);
		if (entry->id == id && entry->obstacle_type == obstacle) {
			break;
		}
	}
	if (entry && node != &quickbar_entries) {
		entry->used++;
		list_del(&entry->node);
		quickbar_additem(entry);
	} else {
		entry = MyMalloc(sizeof *entry);
		entry->obstacle_type = obstacle;
		entry->id = id;
		entry->used = 1;
		quickbar_additem(entry);
	}
}

/*
void
quickbar_click (level *level, int id, leveleditor_state *cur_state)
{
    struct quickbar_entry *entry = quickbar_getentry ( id );
    if ( entry ) {
	switch ( entry->obstacle_type )
	{
	    case LEVEL_EDITOR_SELECTION_FLOOR:
		cur_state->r_tile_used = entry->id;
		start_rectangle_mode(cur_state, TRUE);
		break;
	    case LEVEL_EDITOR_SELECTION_WALLS:
		cur_state->l_selected_mode = entry->obstacle_type;
		cur_state->l_id = entry->id;
		start_line_mode(cur_state, TRUE);
		break;
	    default:
	    action_create_obstacle_user (level, 
		    cur_state->TargetSquare . x, cur_state->TargetSquare . y, 
		    wall_indices [ entry -> obstacle_type ] [ entry -> id ]);
	}
	entry->used ++;
    }
}    
*/
/**
 *
 *
 */
void close_all_chests_on_level(int l_num)
{
	int i;

	for (i = 0; i < MAX_OBSTACLES_ON_MAP; i++) {
		switch (curShip.AllLevels[l_num]->obstacle_list[i].type) {
		case ISO_H_CHEST_OPEN:
			curShip.AllLevels[l_num]->obstacle_list[i].type = ISO_H_CHEST_CLOSED;
			break;
		case ISO_V_CHEST_OPEN:
			curShip.AllLevels[l_num]->obstacle_list[i].type = ISO_V_CHEST_CLOSED;
			break;
		default:
			break;
		}
	}

};				// void close_all_chests_on_level ( int l_num ) 

/**
 * This function should associate the current mouse position with an
 * index in the level editor item drop screen.
 * (-1) is returned when cursor is not on any item in the item drop grid.
 */
int level_editor_item_drop_index(int row_len, int line_len)
{
	if ((GetMousePos_x() > UNIVERSAL_COORD_W(55)) && (GetMousePos_x() < UNIVERSAL_COORD_W(55 + 64 * line_len)) &&
	    (GetMousePos_y() > UNIVERSAL_COORD_H(32)) && (GetMousePos_y() < UNIVERSAL_COORD_H(32 + 66 * row_len))) {
		return ((GetMousePos_x() - UNIVERSAL_COORD_W(55)) / (64 * GameConfig.screen_width / 640) +
			((GetMousePos_y() - UNIVERSAL_COORD_H(32)) / (66 * GameConfig.screen_height / 480)) * line_len);
	}
	//--------------------
	// If no level editor item grid index was found under the current
	// mouse cursor position, we just return (-1) to indicate that.
	//
	return (-1);

};				// int level_editor_item_drop_index ( void )

/**
 * This function drops an item onto the floor.  It works with a selection
 * of item images and clicking with the mouse on an item image or on one
 * of the buttons presented to the person editing the level.
 */
item *ItemDropFromLevelEditor(void)
{
	int SelectionDone = FALSE;
	int NewItemCode = (-1);
	int i;
	int j;
	item temp_item;
	int row_len = 5;
	int line_len = 8;
	int our_multiplicity = 1;
	int item_group = 0;
	item *dropped_item;
	static int previous_mouse_position_index = (-1);
	static int previous_suffix_selected = (-1);
	static int previous_prefix_selected = (-1);
	game_status = INSIDE_MENU;

	while (MouseLeftPressed())
		SDL_Delay(1);

	make_sure_system_mouse_cursor_is_turned_on();

	while (!SelectionDone) {
		save_mouse_state();

		our_SDL_fill_rect_wrapper(Screen, NULL, 0);

		for (j = 0; j < row_len; j++) {
			for (i = 0; i < line_len; i++) {
				temp_item.type = i + j * line_len + item_group * line_len * row_len;
				if (temp_item.type >= Number_Of_Item_Types)
					continue;	//temp_item.type = 1 ;
				ShowRescaledItem(i, 32 + (64 * GameConfig.screen_height / 480 + 2) * j, &(temp_item));
			}
		}

		ShowGenericButtonFromList(LEVEL_EDITOR_NEXT_ITEM_GROUP_BUTTON);
		ShowGenericButtonFromList(LEVEL_EDITOR_PREV_ITEM_GROUP_BUTTON);
		ShowGenericButtonFromList(LEVEL_EDITOR_NEXT_PREFIX_BUTTON);
		ShowGenericButtonFromList(LEVEL_EDITOR_PREV_PREFIX_BUTTON);
		ShowGenericButtonFromList(LEVEL_EDITOR_NEXT_SUFFIX_BUTTON);
		ShowGenericButtonFromList(LEVEL_EDITOR_PREV_SUFFIX_BUTTON);
		ShowGenericButtonFromList(LEVEL_EDITOR_CANCEL_ITEM_DROP_BUTTON);

		if (MouseCursorIsOnButton(LEVEL_EDITOR_CANCEL_ITEM_DROP_BUTTON, GetMousePos_x(), GetMousePos_y()))
			PutStringFont(Screen, FPS_Display_BFont, 20, 440 * GameConfig.screen_height / 480, _("Cancel item drop"));
		if (level_editor_item_drop_index(row_len, line_len) != (-1)) {
			previous_mouse_position_index = level_editor_item_drop_index(row_len, line_len) + item_group * line_len * row_len;
			if (previous_mouse_position_index >= Number_Of_Item_Types) {
				previous_mouse_position_index = Number_Of_Item_Types - 1;
			} else
				PutStringFont(Screen, FPS_Display_BFont, 20, 440 * GameConfig.screen_height / 480,
					      D_(ItemMap[previous_mouse_position_index].item_name));
		}

		if (previous_prefix_selected != (-1)) {
			PutStringFont(Screen, FPS_Display_BFont, 300 * GameConfig.screen_width / 640, 370 * GameConfig.screen_height / 480,
				      PrefixList[previous_prefix_selected].bonus_name);
		} else {
			PutStringFont(Screen, FPS_Display_BFont, 300 * GameConfig.screen_width / 640, 370 * GameConfig.screen_height / 480,
				      _("NO PREFIX"));
		}

		if (previous_suffix_selected != (-1)) {
			PutStringFont(Screen, FPS_Display_BFont, 300 * GameConfig.screen_width / 640, 410 * GameConfig.screen_height / 480,
				      SuffixList[previous_suffix_selected].bonus_name);
		} else {
			PutStringFont(Screen, FPS_Display_BFont, 300 * GameConfig.screen_width / 640, 410 * GameConfig.screen_height / 480,
				      _("NO SUFFIX"));
		}

		our_SDL_flip_wrapper();

		if (EscapePressed()) {	//Pressing escape cancels the dropping
			while (EscapePressed()) ;
			return NULL;
		}

		if (MouseLeftClicked()) {
			if (MouseCursorIsOnButton(LEVEL_EDITOR_NEXT_ITEM_GROUP_BUTTON, GetMousePos_x(), GetMousePos_y())) {
				if ((item_group + 1) * line_len * row_len < Number_Of_Item_Types)
					item_group++;
			} else if (MouseCursorIsOnButton(LEVEL_EDITOR_PREV_ITEM_GROUP_BUTTON, GetMousePos_x(), GetMousePos_y())) {
				if (item_group > 0)
					item_group--;
			}

			if (MouseCursorIsOnButton(LEVEL_EDITOR_NEXT_PREFIX_BUTTON, GetMousePos_x(), GetMousePos_y())) {
				if (PrefixList[previous_prefix_selected + 1].bonus_name != NULL)
					previous_prefix_selected++;
			} else if (MouseCursorIsOnButton(LEVEL_EDITOR_PREV_PREFIX_BUTTON, GetMousePos_x(), GetMousePos_y())) {
				if (previous_prefix_selected > (-1))
					previous_prefix_selected--;
			}

			if (MouseCursorIsOnButton(LEVEL_EDITOR_NEXT_SUFFIX_BUTTON, GetMousePos_x(), GetMousePos_y())) {
				if (SuffixList[previous_suffix_selected + 1].bonus_name != NULL)
					previous_suffix_selected++;
			} else if (MouseCursorIsOnButton(LEVEL_EDITOR_PREV_SUFFIX_BUTTON, GetMousePos_x(), GetMousePos_y())) {
				if (previous_suffix_selected > (-1))
					previous_suffix_selected--;
			} else if (MouseCursorIsOnButton(LEVEL_EDITOR_CANCEL_ITEM_DROP_BUTTON, GetMousePos_x(), GetMousePos_y())) {
				return NULL;
			} else if (level_editor_item_drop_index(row_len, line_len) != (-1)) {
				NewItemCode = level_editor_item_drop_index(row_len, line_len) + item_group * line_len * row_len;
				if (NewItemCode < 0)
					NewItemCode = 0;	// just if the mouse has moved away in that little time...
				if (NewItemCode < Number_Of_Item_Types)
					SelectionDone = TRUE;
			}
		}
	}

	if (NewItemCode >= Number_Of_Item_Types) {
		NewItemCode = 0;
	}

	if (ItemMap[NewItemCode].item_group_together_in_inventory) {
		our_multiplicity =
		    do_graphical_number_selection_in_range(1, (!MatchItemWithName(NewItemCode, "Cyberbucks")) ? 100 : 1000, 1);
		if (our_multiplicity == 0)
			our_multiplicity = 1;
	}
	dropped_item = DropItemAt(NewItemCode, Me.pos.z, rintf(Me.pos.x), rintf(Me.pos.y),
				  previous_prefix_selected, previous_suffix_selected, our_multiplicity);

	while (MouseLeftPressed())
		SDL_Delay(1);

	save_mouse_state();
	game_status = INSIDE_LVLEDITOR;

	return dropped_item;
};				// void ItemDropFromLevelEditor( void )

/**
 * There is a 'help' screen for the level editor too.  This help screen
 * is presented as a scrolling text, giving a short introduction and also
 * explaining the keymap to the level editor.  The info for this scrolling
 * text is all in a title file in the maps dir, much like the initial
 * scrolling text at any new game startup.
 */
void ShowLevelEditorKeymap(void)
{
	PlayATitleFile("level_editor_help.title");
};				// void ShowLevelEditorKeymap ( void )

/**
 * The levels in Freedroid may be connected into one big map by simply
 * 'gluing' then together, i.e. we define some interface areas to the
 * sides of a map and when the Tux passes these areas, he'll be silently
 * put into another map without much fuss.  This operation is performed
 * silently and the two maps must be synchronized in this interface area
 * so the map change doesn't become apparend to the player.  Part of this
 * synchronisation, namely copying the map tiles to the other map, is 
 * done automatically, but some inconsistencies like non-matching map
 * sizes or non-symmetric jump directions (i.e. not back and forth but
 * back and forth-to-somewhere else) are not resolved automatically.
 * Instead, a report on inconsistencies will be created and the person
 * editing the map can then resolve the inconsistencies manually in one
 * fashion or the other.
 */
void ReportInconsistenciesForLevel(int LevelNum)
{
	int TargetLevel;
	SDL_Rect ReportRect;

	ReportRect.x = 20;
	ReportRect.y = 20;
	ReportRect.w = 600;
	ReportRect.h = 440;

	AssembleCombatPicture(ONLY_SHOW_MAP_AND_TEXT | SHOW_GRID | SKIP_LIGHT_RADIUS);

	DisplayText(_("\nThe list of inconsistencies of the jump interfaces for this level:\n\n"),
		    ReportRect.x, ReportRect.y + FontHeight(GetCurrentFont()), &ReportRect, 1.0);

	//--------------------
	// First we test for inconsistencies of back-forth ways, i.e. if the transit
	// in one direction will lead back in the right direction when returning.
	//
	if (curShip.AllLevels[LevelNum]->jump_target_north != (-1)) {
		TargetLevel = curShip.AllLevels[LevelNum]->jump_target_north;
		if (curShip.AllLevels[TargetLevel]->jump_target_south != LevelNum) {
			DisplayText(_("BACK-FORTH-MISMATCH: North doesn't lead back here (yet)!\n"), -1, -1, &ReportRect, 1.0);
		}
	}
	if (curShip.AllLevels[LevelNum]->jump_target_south != (-1)) {
		TargetLevel = curShip.AllLevels[LevelNum]->jump_target_south;
		if (curShip.AllLevels[TargetLevel]->jump_target_north != LevelNum) {
			DisplayText(_("BACK-FORTH-MISMATCH: South doesn't lead back here (yet)!\n"), -1, -1, &ReportRect, 1.0);
		}
	}
	if (curShip.AllLevels[LevelNum]->jump_target_east != (-1)) {
		TargetLevel = curShip.AllLevels[LevelNum]->jump_target_east;
		if (curShip.AllLevels[TargetLevel]->jump_target_west != LevelNum) {
			DisplayText(_("BACK-FORTH-MISMATCH: East doesn't lead back here (yet)!\n"), -1, -1, &ReportRect, 1.0);
		}
	}
	if (curShip.AllLevels[LevelNum]->jump_target_west != (-1)) {
		TargetLevel = curShip.AllLevels[LevelNum]->jump_target_west;
		if (curShip.AllLevels[TargetLevel]->jump_target_east != LevelNum) {
			DisplayText(_("BACK-FORTH-MISMATCH: West doesn't lead back here (yet)!\n"), -1, -1, &ReportRect, 1.0);
		}
	}
	DisplayText(_("\nNO OTHER BACK-FORTH-MISMATCH ERRORS other than those listed above\n\n"), -1, -1, &ReportRect, 1.0);

	//--------------------
	// Now we test for inconsistencies of interface sizes, i.e. if the interface source level
	// has an interface as large as the target interface level.
	//
	if (curShip.AllLevels[LevelNum]->jump_target_north != (-1)) {
		TargetLevel = curShip.AllLevels[LevelNum]->jump_target_north;
		if (curShip.AllLevels[TargetLevel]->jump_threshold_south != curShip.AllLevels[LevelNum]->jump_threshold_north) {
			DisplayText(_("INTERFACE SIZE MISMATCH: North doesn't lead so same-sized interface level!!!\n"),
				    -1, -1, &ReportRect, 1.0);
		}
	}
	if (curShip.AllLevels[LevelNum]->jump_target_south != (-1)) {
		TargetLevel = curShip.AllLevels[LevelNum]->jump_target_south;
		if (curShip.AllLevels[TargetLevel]->jump_threshold_north != curShip.AllLevels[LevelNum]->jump_threshold_south) {
			DisplayText(_("INTERFACE SIZE MISMATCH: South doesn't lead so same-sized interface level!!!\n"),
				    -1, -1, &ReportRect, 1.0);
		}
	}
	if (curShip.AllLevels[LevelNum]->jump_target_east != (-1)) {
		TargetLevel = curShip.AllLevels[LevelNum]->jump_target_east;
		if (curShip.AllLevels[TargetLevel]->jump_threshold_west != curShip.AllLevels[LevelNum]->jump_threshold_east) {
			DisplayText(_("INTERFACE SIZE MISMATCH: East doesn't lead so same-sized interface level!!!\n"),
				    -1, -1, &ReportRect, 1.0);
		}
	}
	if (curShip.AllLevels[LevelNum]->jump_target_west != (-1)) {
		TargetLevel = curShip.AllLevels[LevelNum]->jump_target_west;
		if (curShip.AllLevels[TargetLevel]->jump_threshold_east != curShip.AllLevels[LevelNum]->jump_threshold_west) {
			DisplayText(_("INTERFACE SIZE MISMATCH: West doesn't lead so same-sized interface level!!!\n"),
				    -1, -1, &ReportRect, 1.0);
		}
	}
	//--------------------
	// Now we test for inconsistencies of level sizes, i.e. if the interface source level
	// has the same relevant dimension like the target interface level.
	//
	if (curShip.AllLevels[LevelNum]->jump_target_north != (-1)) {
		TargetLevel = curShip.AllLevels[LevelNum]->jump_target_north;
		if (curShip.AllLevels[TargetLevel]->xlen != curShip.AllLevels[LevelNum]->xlen) {
			DisplayText(_
				    ("LEVEL DIMENSION MISMATCH: North doesn't lead so same-sized level (non-fatal, but no good comes from this)!\n"),
				    -1, -1, &ReportRect, 1.0);
		}
	}
	if (curShip.AllLevels[LevelNum]->jump_target_south != (-1)) {
		TargetLevel = curShip.AllLevels[LevelNum]->jump_target_south;
		if (curShip.AllLevels[TargetLevel]->xlen != curShip.AllLevels[LevelNum]->xlen) {
			DisplayText(_
				    ("LEVEL DIMENSION MISMATCH: South doesn't lead so same-sized level (non-fatal, but no good comes from this)!\n"),
				    -1, -1, &ReportRect, 1.0);
		}
	}
	if (curShip.AllLevels[LevelNum]->jump_target_east != (-1)) {
		TargetLevel = curShip.AllLevels[LevelNum]->jump_target_east;
		if (curShip.AllLevels[TargetLevel]->ylen != curShip.AllLevels[LevelNum]->ylen) {
			DisplayText(_
				    ("LEVEL DIMENSION MISMATCH: East doesn't lead so same-sized level (non-fatal, but no good comes from this)!\n"),
				    -1, -1, &ReportRect, 1.0);
		}
	}
	if (curShip.AllLevels[LevelNum]->jump_target_west != (-1)) {
		TargetLevel = curShip.AllLevels[LevelNum]->jump_target_west;
		if (curShip.AllLevels[TargetLevel]->ylen != curShip.AllLevels[LevelNum]->ylen) {
			DisplayText(_
				    ("LEVEL DIMENSION MISMATCH: West doesn't lead so same-sized level (non-fatal, but no good comes from this)!\n"),
				    -1, -1, &ReportRect, 1.0);
		}
	}
	//--------------------
	// This was it.  We can say so and return.
	//
	DisplayText(_("\n\n--- End of List --- Press Space to return to menu ---\n"), -1, -1, &ReportRect, 1.0);

	our_SDL_flip_wrapper();

};				// void ReportInconsistenciesForLevel ( int LevelNum )

/**
 * After exporting a level, there might be some old corpses of 
 * descriptions that were deleted when the target level was partly cleared
 * out and overwritten with the new obstacles that brought their own new
 * obstacle descriptions.
 *
 * In this function, we try to clean out those old corpses to avoid 
 * cluttering in the map file.
 */
void eliminate_dead_obstacle_descriptions(level * target_level)
{
	int i;
	int is_in_use;
	int desc_index;

	//--------------------
	// We proceed through the list of known descriptions.  Some of them
	// might not be in use, but still hold a non-null content string.
	// Such instances will be eliminated.
	//
	for (desc_index = 0; desc_index < MAX_OBSTACLE_DESCRIPTIONS_PER_LEVEL; desc_index++) {
		//--------------------
		// Maybe the description in question is an empty index anyway.
		// Then of course there is no need to eliminate anything and
		// we can proceed right away.
		//
		if (target_level->obstacle_description_list[desc_index] == NULL)
			continue;

		//--------------------
		// So now we've encountered some string.  Let's see if it's really
		// in use.  For that, we need to proceed through all the obstacles
		// of this level and see if one of them has a description index 
		// pointing to this description string.
		//
		is_in_use = FALSE;
		for (i = 0; i < MAX_OBSTACLES_ON_MAP; i++) {
			if (target_level->obstacle_list[i].description_index == desc_index) {
				DebugPrintf(1,
					    "\nvoid eliminate_dead_obstacle_descriptions(...):  This descriptions seems to be in use still.");
				is_in_use = TRUE;
				break;
			}
		}

		if (is_in_use)
			continue;
		target_level->obstacle_description_list[desc_index] = NULL;
		DebugPrintf(1, "\nNOTE: void eliminate_dead_obstacle_descriptions(...):  dead description found.  Eliminated.");
	}

};				// void eliminate_dead_obstacle_descriptions (level *target_level )

/**
 * This function should allow for conveninet duplication of obstacles from
 * one map to the other.  It assumes, that the target area has been cleaned
 * out of obstacles already.
 */
void
duplicate_all_obstacles_in_area(level * source_level,
				float source_start_x, float source_start_y,
				float source_area_width, float source_area_height,
				level * target_level, float target_start_x, float target_start_y)
{
	int i;
	obstacle *new_obstacle;

	for (i = 0; i < MAX_OBSTACLES_ON_MAP; i++) {
		if (source_level->obstacle_list[i].type <= (-1))
			continue;
		if (source_level->obstacle_list[i].pos.x < source_start_x)
			continue;
		if (source_level->obstacle_list[i].pos.y < source_start_y)
			continue;
		if (source_level->obstacle_list[i].pos.x > source_start_x + source_area_width)
			continue;
		if (source_level->obstacle_list[i].pos.y > source_start_y + source_area_height)
			continue;

		new_obstacle =
		    action_create_obstacle(target_level,
					   target_start_x + source_level->obstacle_list[i].pos.x - source_start_x,
					   target_start_y + source_level->obstacle_list[i].pos.y - source_start_y,
					   source_level->obstacle_list[i].type);

		//--------------------
		// Maybe the source obstacle had a label attached to it?  Then
		// We should also duplicate the obstacle label.  Otherwise it
		// might get overwritten when exporting in the other direction.
		//
		if (source_level->obstacle_list[i].name_index != (-1)) {
			action_change_obstacle_label_user(target_level, new_obstacle,
							  source_level->obstacle_name_list[source_level->obstacle_list[i].name_index]);
			DebugPrintf(1, "\nNOTE: void duplicate_all_obstacles_in_area(...):  obstacle name was exported:  %s.",
				    source_level->obstacle_name_list[source_level->obstacle_list[i].name_index]);
		}
		//--------------------
		// Maybe the source obstacle had a description attached to it?  Then
		// We should also duplicate the obstacle description.  Otherwise it
		// might get overwritten when exporting in the other direction.
		//
		if (source_level->obstacle_list[i].description_index != (-1)) {
			action_change_obstacle_description(target_level, new_obstacle,
							   source_level->obstacle_description_list[source_level->obstacle_list[i].
												   description_index]);
			DebugPrintf(-1, "\nNOTE:  obstacle description was exported:  %s.",
				    source_level->obstacle_description_list[source_level->obstacle_list[i].description_index]);
		}
		//action_remove_obstacle ( source_level , & ( source_level -> obstacle_list [ i ] ) );
		// i--; // this is so that this obstacle will be processed AGAIN, since deleting might
		// // have moved a different obstacle to this list position.
	}

	eliminate_dead_obstacle_descriptions(target_level);

};				// void duplicate_all_obstacles_in_area ( ... )

static void leveleditor_init()
{
	level_editor_done = FALSE;

	//--------------------
	// We set the Tux position to something 'round'.
	//
	Me.pos.x = rintf(Me.pos.x) + 0.5;
	Me.pos.y = rintf(Me.pos.y) + 0.5;

	//--------------------
	// We disable all the 'screens' so that we have full view on the
	// map for the purpose of level editing.
	//
	GameConfig.Inventory_Visible = FALSE;
	GameConfig.CharacterScreen_Visible = FALSE;
	GameConfig.SkillScreen_Visible = FALSE;

	strcpy(VanishingMessage, "");
	VanishingMessageEndDate = 0;

	//--------------------
	// For drawing new waypoints, we init this.
	//
	OriginWaypoint = (-1);

	leveleditor_init_widgets();

	global_ingame_mode = GLOBAL_INGAME_MODE_NORMAL;

	SDL_EnableKeyRepeat(SDL_DEFAULT_REPEAT_DELAY, SDL_DEFAULT_REPEAT_INTERVAL);
}

static void leveleditor_cleanup()
{
	Activate_Conservative_Frame_Computation();
	action_freestack();
	clear_selection(-1);

	global_ingame_mode = GLOBAL_INGAME_MODE_NORMAL;

	SDL_EnableKeyRepeat(0, SDL_DEFAULT_REPEAT_INTERVAL);
}

void TestMap(void)
{				/* Keeps World map in a clean state */
	leveleditor_cleanup();	//free current selection and undo stack as a workaround for existing problems
	if (game_root_mode == ROOT_IS_GAME)	/*don't allow map testing if root mode is GAME */
		return;
	SaveGame();
	Game();
	LoadGame();
	leveleditor_init();
	return;
}				// TestMap ( void )

/**
 * This function provides the leveleditor integrated into
 * freedroid.  Actually this function is a submenu of the big
 * Escape Menu.  In here you can edit the level and, upon pressing
 * escape, you can enter a new submenu where you can save the level,
 * change level name and quit from level editing.
 */
void LevelEditor()
{
	leveleditor_init();

	while (!level_editor_done) {
		game_status = INSIDE_LVLEDITOR;

		if (!GameConfig.hog_CPU)
			SDL_Delay(1);

		leveleditor_process_input();

		leveleditor_update_tool();
		leveleditor_display();

	}

	leveleditor_cleanup();
};				// void LevelEditor ( void )

#undef _leveleditor_c
