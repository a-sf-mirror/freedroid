/* 
 *
 *   Copyright (c) 1994, 2002, 2003 Johannes Prix
 *   Copyright (c) 1994, 2002 Reinhard Prix
 *   Copyright (c) 2004-2009 Arthur Huillet 
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
 * This file contains functions related to events and event triggers. These functions 
 * check, whether the necessary conditions for an event are satisfied, and in case that
 * they are, they order the appropriate event to be executed.
 */

#define _event_c
#include "system.h"

#include "defs.h"
#include "struct.h"
#include "global.h"
#include "proto.h"

struct event_trigger {

	luacode lua_code;

	char *name;
	int enabled;		//is the trigger enabled?
	int silent;		//do we have to advertise this trigger to the user? (teleporters..)

	enum {
		POSITION,
		CHANGE_LEVEL,
		ENEMY_DEATH,
		ENEMY_HACK,
		OBSTACLE_ACTION,
	} trigger_type;

	union {
		struct {
			int lvl;
			int x;
			int y;
			int teleported;
		} position;
		struct {
			int exit_level;
			int enter_level;
		} change_level;
		struct {
			int lvl;
			int faction;
			char *dialog_name;
			int marker;
		} enemy_event;
		struct {
			int lvl;
			int type;
			char *label;
		} obstacle_action;
	} trigger;
};

static struct dynarray event_triggers;

/**
 * Delete all events and event triggers
 */
static void clear_out_events(void)
{
	// Remove existing triggers
	delete_events();

	dynarray_init(&event_triggers, 16, sizeof(struct event_trigger));
}

/**
 * Delete all events and event triggers
 */
void delete_events(void)
{
	// Remove existing triggers
	int i;
	struct event_trigger *arr = event_triggers.arr;
	for (i = 0; i < event_triggers.size; i++) {
		if (arr[i].name)
			free(arr[i].name);

		if (arr[i].lua_code)
			free(arr[i].lua_code);

		if (arr[i].trigger_type == OBSTACLE_ACTION) {
			free(arr[i].trigger.obstacle_action.label);
		} else if (arr[i].trigger_type == ENEMY_DEATH || arr[i].trigger_type == ENEMY_HACK) {
			free(arr[i].trigger.enemy_event.dialog_name);
		}
	}

	dynarray_free(&event_triggers);
}

#define EVENT_TRIGGER_BEGIN_STRING "* New event trigger *"
#define EVENT_TRIGGER_END_STRING "* End of trigger *"
#define EVENT_TRIGGER_NAME_MARKED_STRING "Name=_\""
#define EVENT_TRIGGER_NAME_STRING "Name=\""
#define EVENT_TRIGGER_LUACODE_STRING "<LuaCode>"
#define EVENT_TRIGGER_LUACODE_END_STRING "</LuaCode>"
#define EVENT_TRIGGER_ENABLED_STRING "Enable this trigger by default="

// For position-based events
#define EVENT_TRIGGER_LABEL_STRING "Trigger at label=\""
#define EVENT_TRIGGER_TELEPORTED "Teleported="
#define EVENT_TRIGGER_IS_SILENT_STRING "Silent="

// For level-change events
#define LEVEL_CHANGE_TRIGGER "Trigger changing level"
#define LEVEL_CHANGE_EXITING "Exiting level="
#define LEVEL_CHANGE_ENTERING "Entering level="

// For enemy-death events
#define ENEMY_DEATH_TRIGGER "Trigger on enemy death"

// For enemy-hacked events
#define ENEMY_HACK_TRIGGER "Trigger on enemy hack"

// For all enemy events
#define ENEMY_TRIGGER "Trigger on enemy"
#define ENEMY_LVLNUM "Enemy level="
#define ENEMY_FACTION "Enemy faction=\""
#define ENEMY_DIALOG_NAME "Enemy dialog name=\""
#define ENEMY_MARKER "Enemy marker="

// For obstacle events
#define OBSTACLE_ACTION_TRIGGER "Trigger on obstacle"
#define OBSTACLE_ACTION_LVLNUM "Obstacle level number="
#define OBSTACLE_ACTION_TYPE "Obstacle type name=\""
#define OBSTACLE_ACTION_LABEL "Obstacle label=\""

/** 
 *
 *
 */
static void load_events(char *EventSectionPointer)
{
	char *EventPointer;
	char *EndOfEvent;
	char *TempMapLabelName;
	gps pos;
	char *TempEnemyFaction;
	char s;
	struct event_trigger temp;
	char *temp_str;

	EventPointer = EventSectionPointer;
	while ((EventPointer = strstr(EventPointer, EVENT_TRIGGER_BEGIN_STRING)) != NULL) {
		memset(&temp, 0, sizeof(struct event_trigger));

		EventPointer += strlen(EVENT_TRIGGER_BEGIN_STRING) + 1;

		EndOfEvent = LocateStringInData(EventPointer, EVENT_TRIGGER_END_STRING);
		s = EndOfEvent[strlen(EVENT_TRIGGER_END_STRING) - 1];
		EndOfEvent[strlen(EVENT_TRIGGER_END_STRING) - 1] = 0;

		// Determine type of event condition
		if ((TempMapLabelName = strstr(EventPointer, EVENT_TRIGGER_LABEL_STRING))) {
			temp.trigger_type = POSITION;
			TempMapLabelName = ReadAndMallocStringFromData(EventPointer, EVENT_TRIGGER_LABEL_STRING, "\"");
			pos = get_map_label_center(TempMapLabelName);
			temp.trigger.position.x = (int)pos.x;
			temp.trigger.position.y = (int)pos.y;
			temp.trigger.position.lvl = pos.z;
			free(TempMapLabelName);
			ReadValueFromStringWithDefault(EventPointer, EVENT_TRIGGER_IS_SILENT_STRING, "%d", "1",
						&temp.silent, EndOfEvent);
			ReadValueFromStringWithDefault(EventPointer, EVENT_TRIGGER_TELEPORTED, "%d", "-1",
						&temp.trigger.position.teleported, EndOfEvent);
		} else if (strstr(EventPointer, LEVEL_CHANGE_TRIGGER)) {
			temp.trigger_type = CHANGE_LEVEL;
			ReadValueFromStringWithDefault(EventPointer, LEVEL_CHANGE_ENTERING, "%d", "-1",
						&temp.trigger.change_level.enter_level, EndOfEvent);
			ReadValueFromStringWithDefault(EventPointer, LEVEL_CHANGE_EXITING, "%d", "-1",
						&temp.trigger.change_level.exit_level, EndOfEvent);
		} else if (strstr(EventPointer, ENEMY_TRIGGER)) {
			if (strstr(EventPointer, ENEMY_DEATH_TRIGGER)) {
				temp.trigger_type = ENEMY_DEATH;
			} else  {
				temp.trigger_type = ENEMY_HACK;
			}
			ReadValueFromStringWithDefault(EventPointer, ENEMY_LVLNUM, "%d", "-1",
						&temp.trigger.enemy_event.lvl, EndOfEvent);
			if (strstr(EventPointer, ENEMY_FACTION)) {
				TempEnemyFaction = ReadAndMallocStringFromData(EventPointer, ENEMY_FACTION, "\"");
				temp.trigger.enemy_event.faction = get_faction_id(TempEnemyFaction);
				free(TempEnemyFaction);
			} else {
				temp.trigger.enemy_event.faction = -1;
			}
			if (strstr(EventPointer, ENEMY_DIALOG_NAME)) {
				temp.trigger.enemy_event.dialog_name = ReadAndMallocStringFromData(EventPointer, ENEMY_DIALOG_NAME, "\"");
			} else {
				temp.trigger.enemy_event.dialog_name = NULL;
			}
			ReadValueFromStringWithDefault(EventPointer, ENEMY_MARKER, "%d", "-1",
						&temp.trigger.enemy_event.marker, EndOfEvent);
		} else if (strstr(EventPointer, OBSTACLE_ACTION_TRIGGER)) {
			temp.trigger_type = OBSTACLE_ACTION;
			ReadValueFromStringWithDefault(EventPointer, OBSTACLE_ACTION_LVLNUM, "%d", "-1",
						&temp.trigger.obstacle_action.lvl, EndOfEvent);
			temp_str = ReadAndMallocStringFromDataOptional(EventPointer, OBSTACLE_ACTION_TYPE, "\"");
			if (temp_str) {
				temp.trigger.obstacle_action.type = get_obstacle_type_by_name(temp_str);
				free(temp_str);
			} else {
				temp.trigger.obstacle_action.type = -1;
			}
			temp.trigger.obstacle_action.label = ReadAndMallocStringFromDataOptional(EventPointer, OBSTACLE_ACTION_LABEL, "\"");
		}

		temp.name = ReadAndMallocStringFromDataOptional(EventPointer, EVENT_TRIGGER_NAME_MARKED_STRING, "\"");
		if (!temp.name) {
			temp.name = ReadAndMallocStringFromData(EventPointer, EVENT_TRIGGER_NAME_STRING, "\"");
		}

		temp.lua_code =
			ReadAndMallocStringFromData(EventPointer, EVENT_TRIGGER_LUACODE_STRING, EVENT_TRIGGER_LUACODE_END_STRING);

		ReadValueFromStringWithDefault(EventPointer, EVENT_TRIGGER_ENABLED_STRING, "%d", "1",
						&temp.enabled, EndOfEvent);

		EndOfEvent[strlen(EVENT_TRIGGER_END_STRING) - 1] = '\0';

		EndOfEvent[strlen(EVENT_TRIGGER_END_STRING) - 1] = s;

		dynarray_add(&event_triggers, &temp, sizeof(struct event_trigger));
	}			// While Event trigger begin string found...
}

/**
 * This function reads in the game events, i.e. the locations and conditions
 * under which some actions are triggered.
 */
void GetEventTriggers(const char *EventsAndEventTriggersFilename)
{
	char *EventSectionPointer;
	char fpath[PATH_MAX];

	clear_out_events();

	find_file(EventsAndEventTriggersFilename, MAP_DIR, fpath, PLEASE_INFORM | IS_FATAL);
	EventSectionPointer =
		ReadAndMallocAndTerminateFile(fpath, "*** END OF EVENT ACTION AND EVENT TRIGGER FILE *** LEAVE THIS TERMINATOR IN HERE ***");

	load_events(EventSectionPointer);

	free(EventSectionPointer);
};

/**
 * \brief Trigger a position-based events for the given positions. 
 * \param cur_pos The current position.
 * \param teleported TRUE if the event come from a teleportation, FALSE otherwise.
 */
void event_position_changed(gps pos, int teleported)
{
	int i;
	struct event_trigger *arr = event_triggers.arr;

	for (i = 0; i < event_triggers.size; i++) {

		if (arr[i].trigger_type != POSITION)
			continue;

		if (!arr[i].enabled)
			continue;

		if (arr[i].trigger.position.lvl != pos.z)
 			continue;

		if (arr[i].trigger.position.teleported != -1)
			if (arr[i].trigger.position.teleported != teleported)
				continue;

		if (((int)pos.x == arr[i].trigger.position.x) 
				&& ((int)pos.y == arr[i].trigger.position.y)) {

			run_lua(LUA_DIALOG, arr[i].lua_code);
		}
	}
}

/**
 * Trigger level-enter and level-exit events for the given level
 */
void event_level_changed(int past_lvl, int cur_lvl)
{
	int i;
	struct event_trigger *arr = event_triggers.arr;

	for (i = 0; i < event_triggers.size; i++) {
		if (arr[i].trigger_type != CHANGE_LEVEL)
			continue;

		if (!arr[i].enabled)
			continue;

		if (arr[i].trigger.change_level.exit_level != -1)
			if (arr[i].trigger.change_level.exit_level != past_lvl)
				continue;

		if (arr[i].trigger.change_level.enter_level != -1)
			if (arr[i].trigger.change_level.enter_level != cur_lvl)
				continue;

		run_lua(LUA_DIALOG, arr[i].lua_code);
	}
}

/**
 * Trigger on enemy events (ENEMY_DEATH or ENEMY_HACK).
 */
static void event_enemy(enemy *target, int event)
{
	int i;
	struct event_trigger *arr = event_triggers.arr;

	for (i = 0; i < event_triggers.size; i++) {
		if (arr[i].trigger_type != event)
			continue;

		if (!arr[i].enabled)
			continue;

		if (arr[i].trigger.enemy_event.lvl != -1)
			if (arr[i].trigger.enemy_event.lvl != target->pos.z)
				continue;

		if (arr[i].trigger.enemy_event.faction != -1)
			if (arr[i].trigger.enemy_event.faction != target->faction)
				continue;

		if (arr[i].trigger.enemy_event.dialog_name != NULL)
			if (strcmp(arr[i].trigger.enemy_event.dialog_name, target->dialog_section_name))
				continue;

		if (arr[i].trigger.enemy_event.marker != -1)
			if (arr[i].trigger.enemy_event.marker != target->marker)
				continue;

		run_lua(LUA_DIALOG, arr[i].lua_code);
	}
}

void event_enemy_died(enemy *dead)
{
	event_enemy(dead, ENEMY_DEATH);
}

void event_enemy_hacked(enemy *hacked)
{
	event_enemy(hacked, ENEMY_HACK);
}

/**
 * Trigger action event on obstacle.
 */
void event_obstacle_action(obstacle *o)
{
	int i;
	struct event_trigger *arr = event_triggers.arr;

	for (i = 0; i < event_triggers.size; i++) {
		if (arr[i].trigger_type != OBSTACLE_ACTION)
			continue;
			
		if (!arr[i].enabled)
			continue;

		if (arr[i].trigger.obstacle_action.lvl != -1)
			if (arr[i].trigger.obstacle_action.lvl != o->pos.z)
				continue;

		if (arr[i].trigger.obstacle_action.type != -1)
			if (arr[i].trigger.obstacle_action.type != o->type)
				continue;

		if (arr[i].trigger.obstacle_action.label) {
			
			level *lvl = curShip.AllLevels[o->pos.z];
			char *label = (char *) get_obstacle_extension(lvl, o, OBSTACLE_EXTENSION_LABEL);
			
			if (!label || strcmp(arr[i].trigger.obstacle_action.label, label))
				continue;
		}

		run_lua(LUA_DIALOG, arr[i].lua_code);
	}
}

/**
 *
 *
 */
const char *teleporter_square_below_mouse_cursor(void)
{
	finepoint MapPositionOfMouse;
	struct event_trigger *arr;

	if (MouseCursorIsInUserRect(GetMousePos_x(), GetMousePos_y())) {
		MapPositionOfMouse.x = translate_pixel_to_map_location((float)input_axis.x, (float)input_axis.y, TRUE);
		MapPositionOfMouse.y = translate_pixel_to_map_location((float)input_axis.x, (float)input_axis.y, FALSE);
 		arr = visible_event_at_location((int)MapPositionOfMouse.x, (int)MapPositionOfMouse.y, Me.pos.z);
		if (arr)
			return D_(arr->name);
	}
	return NULL;
}


/**
 * Visible event Trigger at a location
 */
struct event_trigger * visible_event_at_location(int x, int y, int z)
{
	int i;
	struct event_trigger *arr = event_triggers.arr;
	for (i = 0; i < event_triggers.size; i++) {
		if (arr[i].trigger_type != POSITION)
			continue;

		if (z != arr[i].trigger.position.lvl)
 			continue;

		if (x != arr[i].trigger.position.x)
			continue;

		if (y != arr[i].trigger.position.y)
			continue;

		if (arr[i].silent)
			continue;

		if (!arr[i].enabled)
			continue;

		return &arr[i];
	}
	return NULL;
}

/**
 * Validate Lua code of events
 * This validator loads the events, and checks for the syntax of their lua code.
 * It tries to compile each Lua code snippet, checking them for syntax as well.
 * It then proceeds to executing each Lua code snippet, which makes it possible to check for some errors
 * such as typos in function calls (calls to non existing functions). However, this runtime check does not cover 100% of
 * the Lua code because of branches (when it encounters a if, it will not test both the "then" and "else" branches).
 *
 * As a result, the fact that the validator finds no error does not imply there are no errors in dialogs.
 * Syntax is checked fully, but runtime validation cannot check all of the code.
 */
int validate_events()
{
	int error_caught = FALSE;

	skip_initial_menus = 1;

	/* Disable sound to speed up validation. */
	int old_sound_on = sound_on;
	sound_on = FALSE;

	char fpath[PATH_MAX];
	find_file("levels.dat", MAP_DIR, fpath, PLEASE_INFORM | IS_FATAL);
	LoadShip(fpath, 0);
	PrepareStartOfNewCharacter("NewTuxStartGameSquare");

	/* Temporarily disable screen fadings to speed up validation. */
	GameConfig.do_fadings = FALSE;

	/* _says functions are not run by the validator, as they display
	   text on screen and wait for clicks */
	run_lua(LUA_DIALOG, "function chat_says(a)\nend\n");
	run_lua(LUA_DIALOG, "function cli_says(a)\nend\n");

	/* Subdialogs currently call run_chat and we cannot do that when validating dialogs */
	run_lua(LUA_DIALOG, "function start_chat(a)\nend\n");

	/* Shops must not be run (display + wait for clicks) */
	run_lua(LUA_DIALOG, "function trade_with(a)\nend\n");

	run_lua(LUA_DIALOG, "function user_input_string(a)\nreturn \"dummy\";\nend\n");

	run_lua(LUA_DIALOG, "function upgrade_items(a)\nend\n");
	run_lua(LUA_DIALOG, "function craft_addons(a)\nend\n");

	/* takeover requires user input - hardcode it to win */
	run_lua(LUA_DIALOG, "function takeover(a)\nreturn true\nend\n");

	/* set_mouse_move_target() breaks validator */
	run_lua(LUA_DIALOG, "function set_mouse_move_target(a)\nend\n");

	/* win_game() causes silly animations and delays the process. */
	run_lua(LUA_DIALOG, "function win_game(a)\nend\n");

	/* We do not want to actually exit the game. */
	run_lua(LUA_DIALOG, "function exit_game(a)\nend\n");

	// Loop on all events
	int i;

	GetEventTriggers("events.dat");
	struct event_trigger *arr = event_triggers.arr;

	for (i = 0; i < event_triggers.size; i++) {
		printf("Testing event \"%s\"...\n", arr[i].name);
		int rtn = run_lua(LUA_DIALOG, arr[i].lua_code);
		if (rtn)
			error_caught = TRUE;
		if (term_has_color_cap)
			printf("Result: %s\n", !rtn ? "\033[32msuccess\033[0m" : "\033[31mfailed\033[0m");
		else
			printf("Result: %s\n", !rtn ? "success" : "failed");
	}

	/* Re-enable sound as needed. */
	sound_on = old_sound_on;

	/* Re-enable screen fadings. */
	GameConfig.do_fadings = TRUE;

	return error_caught;
}
