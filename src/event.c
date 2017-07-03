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

#define _event_c 1

#include "system.h"

#include "defs.h"
#include "struct.h"
#include "global.h"
#include "proto.h"
#include "savestruct.h"

struct event_trigger {

	/* Set 1 - attributes written into savegames */
	/* struct event_trigger_state has to contain the same attributes */

	char *name;
	uint32_t init_state; // initialization state (defined by events.dat)
	uint32_t state;      // current state of the event trigger

	/* Set 2 - attributes not written into savegames */

	luacode lua_code;
	int silent; // do we have to advertise this trigger to the user? (teleporters..)
	int single_activation; // disable the trigger after first activation

	enum {
		CODE_ONLY,
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

// List of event_timer in the game
LIST_HEAD(event_timer_head);

/**
 * Delete all events and initialize
 */
static void clear_out_events(void)
{
	// Remove existing triggers
	delete_events();

	dynarray_init(&event_triggers, 16, sizeof(struct event_trigger));
}

/**
 * Delete all kind of events
 */
void delete_events(void)
{
	// Remove existing event timers
	struct event_timer *n, *next;
	list_for_each_entry_safe(n, next, &event_timer_head, node) {
		list_del(&n->node);
		free(n->trigger_name);
		free(n);
	}
	INIT_LIST_HEAD(&event_timer_head);

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
#define EVENT_TRIGGER_SINGLE_ACTIVATION_STRING "Single activation="

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
static void load_events(char *event_section_pointer)
{
	struct gps pos;
	struct event_trigger temp;

	char *event_pointer = event_section_pointer;
	while ((event_pointer = strstr(event_pointer, EVENT_TRIGGER_BEGIN_STRING)) != NULL) {
		memset(&temp, 0, sizeof(struct event_trigger));

		temp.trigger_type = CODE_ONLY;
		temp.init_state = 0;
		temp.state = 0;

		event_pointer += strlen(EVENT_TRIGGER_BEGIN_STRING) + 1;

		char *end_of_event = LocateStringInData(event_pointer, EVENT_TRIGGER_END_STRING);
		char s = end_of_event[strlen(EVENT_TRIGGER_END_STRING) - 1];
		end_of_event[strlen(EVENT_TRIGGER_END_STRING) - 1] = '\0';

		// Determine type of event condition
		char *temp_map_label_name = strstr(event_pointer, EVENT_TRIGGER_LABEL_STRING);
		if (temp_map_label_name) {
			temp.trigger_type = POSITION;
			temp_map_label_name = ReadAndMallocStringFromData(event_pointer, EVENT_TRIGGER_LABEL_STRING, "\"");
			pos = get_map_label_center(temp_map_label_name);
			temp.trigger.position.x = (int)pos.x;
			temp.trigger.position.y = (int)pos.y;
			temp.trigger.position.lvl = pos.z;
			free(temp_map_label_name);
			ReadValueFromStringWithDefault(event_pointer, EVENT_TRIGGER_IS_SILENT_STRING, "%d", "1",
						&temp.silent, end_of_event);
			ReadValueFromStringWithDefault(event_pointer, EVENT_TRIGGER_TELEPORTED, "%d", "-1",
						&temp.trigger.position.teleported, end_of_event);
		} else if (strstr(event_pointer, LEVEL_CHANGE_TRIGGER)) {
			temp.trigger_type = CHANGE_LEVEL;
			ReadValueFromStringWithDefault(event_pointer, LEVEL_CHANGE_ENTERING, "%d", "-1",
						&temp.trigger.change_level.enter_level, end_of_event);
			ReadValueFromStringWithDefault(event_pointer, LEVEL_CHANGE_EXITING, "%d", "-1",
						&temp.trigger.change_level.exit_level, end_of_event);
		} else if (strstr(event_pointer, ENEMY_TRIGGER)) {
			if (strstr(event_pointer, ENEMY_DEATH_TRIGGER)) {
				temp.trigger_type = ENEMY_DEATH;
			} else  {
				temp.trigger_type = ENEMY_HACK;
			}
			ReadValueFromStringWithDefault(event_pointer, ENEMY_LVLNUM, "%d", "-1",
						&temp.trigger.enemy_event.lvl, end_of_event);
			if (strstr(event_pointer, ENEMY_FACTION)) {
				char *temp_enemy_faction = ReadAndMallocStringFromData(event_pointer, ENEMY_FACTION, "\"");
				temp.trigger.enemy_event.faction = get_faction_id(temp_enemy_faction);
				free(temp_enemy_faction);
			} else {
				temp.trigger.enemy_event.faction = -1;
			}
			if (strstr(event_pointer, ENEMY_DIALOG_NAME)) {
				temp.trigger.enemy_event.dialog_name = ReadAndMallocStringFromData(event_pointer, ENEMY_DIALOG_NAME, "\"");
			} else {
				temp.trigger.enemy_event.dialog_name = NULL;
			}
			ReadValueFromStringWithDefault(event_pointer, ENEMY_MARKER, "%d", "-1",
						&temp.trigger.enemy_event.marker, end_of_event);
		} else if (strstr(event_pointer, OBSTACLE_ACTION_TRIGGER)) {
			temp.trigger_type = OBSTACLE_ACTION;
			ReadValueFromStringWithDefault(event_pointer, OBSTACLE_ACTION_LVLNUM, "%d", "-1",
						&temp.trigger.obstacle_action.lvl, end_of_event);
			char *temp_str = ReadAndMallocStringFromDataOptional(event_pointer, OBSTACLE_ACTION_TYPE, "\"");
			if (temp_str) {
				temp.trigger.obstacle_action.type = get_obstacle_type_by_name(temp_str);
				free(temp_str);
			} else {
				temp.trigger.obstacle_action.type = -1;
			}
			temp.trigger.obstacle_action.label = ReadAndMallocStringFromDataOptional(event_pointer, OBSTACLE_ACTION_LABEL, "\"");
		}

		temp.name = ReadAndMallocStringFromDataOptional(event_pointer, EVENT_TRIGGER_NAME_MARKED_STRING, "\"");
		if (!temp.name) {
			temp.name = ReadAndMallocStringFromData(event_pointer, EVENT_TRIGGER_NAME_STRING, "\"");
		}

		temp.lua_code =
			ReadAndMallocStringFromData(event_pointer, EVENT_TRIGGER_LUACODE_STRING, EVENT_TRIGGER_LUACODE_END_STRING);

		// See the comment about TRIGGER_DEFAULT in defs.h, to get some
		// information on the reason to use (TRIGGER_DEFAULT & TRIGGER_ENABLED)
		int enabled_flag;
		ReadValueFromStringWithDefault(event_pointer, EVENT_TRIGGER_ENABLED_STRING, "%d",
				(TRIGGER_DEFAULT & TRIGGER_ENABLED) ? "1" : "0", &enabled_flag, end_of_event);
		if (enabled_flag) {
			temp.init_state |= TRIGGER_ENABLED;
		} else {
			temp.init_state &= ~TRIGGER_ENABLED;
		}

		ReadValueFromStringWithDefault(event_pointer, EVENT_TRIGGER_SINGLE_ACTIVATION_STRING, "%d", "0",
						&temp.single_activation, end_of_event);

		temp.state = temp.init_state;

		dynarray_add(&event_triggers, &temp, sizeof(struct event_trigger));

		end_of_event[strlen(EVENT_TRIGGER_END_STRING) - 1] = s;
	}
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

	find_file(fpath, MAP_DIR, EventsAndEventTriggersFilename, NULL, PLEASE_INFORM | IS_FATAL);
	EventSectionPointer =
		read_and_malloc_and_terminate_file(fpath, "*** END OF EVENT ACTION AND EVENT TRIGGER FILE *** LEAVE THIS TERMINATOR IN HERE ***");

	load_events(EventSectionPointer);

	free(EventSectionPointer);
};

/**
 * Enable or disable an event's trigger
 *
 * \param name Name of the event's trigger to enable/disable
 * \param flag TRUE to enable the trigger, FALSE to disable it
 *
 * \return FALSE if the event trigger was not found, or TRUE otherwise
 */
int event_trigger_set_enable(const char *name, int flag)
{
	for (int i = 0; i < event_triggers.size; i++) {
		struct event_trigger *evt = (struct event_trigger *)dynarray_member(&event_triggers, i, sizeof(struct event_trigger));
		if (!strcmp(name, evt->name)) {
			if (flag)
				evt->state |= TRIGGER_ENABLED;
			else
				evt->state &= ~TRIGGER_ENABLED;
			return TRUE;
		}
	}
	return FALSE;
}

/**
 * Get the state of an event's trigger
 *
 * \param name  Name of the event's trigger to enable/disable
 * \param state Pointer to the data to fill with the trigger state
 *
 * \return FALSE if the event trigger was not found, or TRUE otherwise
 */
int event_trigger_get_state(const char *name, uint32_t *state)
{
	for (int i = 0; i < event_triggers.size; i++) {
		struct event_trigger *evt = (struct event_trigger *)dynarray_member(&event_triggers, i, sizeof(struct event_trigger));
		if (!strcmp(name, evt->name)) {
			*state = evt->state;
			return TRUE;
		}
	}
	return FALSE;
}

/**
 * Write the sate of event triggers (called when saving a savegame).
 * \ingroup overloadrw
 *
 * Only the triggers with a 'non default' state are written in the
 * savegame, to minimize the savegame's size.
 *
 * \param strout The auto_string to be filled
*/
void write_event_triggers_dynarray(struct auto_string *strout)
{
	// We HAVE to save if the event trigger is not in its initialization
	// state, that is if state != init_state
	// But the init_state (defined in events.dat) may be changed by a level
	// designer. To ensure that a savegame can survive such a change, we
	// also save if init_state != default state.
	// The default state is hardcoded and is not expected to be changed.
	// If, however, a dev was to change the default state, a savegame
	// converter will have to be written.
	for (int i = 0; i < event_triggers.size; i++) {
		struct event_trigger *evt = (struct event_trigger *)dynarray_member(&event_triggers, i, sizeof(struct event_trigger));
		if ((evt->state == evt->init_state) && (evt->init_state == TRIGGER_DEFAULT))
			continue;
		struct event_trigger_state state = { .name = evt->name, .state = evt->state };
		write_event_trigger_state(strout, &state);
		autostr_append(strout, ",\n");
	}
}

/**
 * Read the state of event triggers (called when loading a savegame)
 * \ingroup overloadrw
 *
 * \param L     Current Lua State
 * \param index Lua stack index of the data
 */
void read_event_triggers_dynarray(lua_State *L, int index)
{
	lua_is_of_type_or_abort(L, index, LUA_TTABLE);
	int array_size = lua_rawlen(L, index);
	if (array_size != 0) {
		struct event_trigger_state data;
		for (int i = 0; i < array_size; i++) {
			lua_rawgeti(L, index, i+1);
			read_event_trigger_state(L, -1, &data);
			event_trigger_set_enable(data.name, (data.state & TRIGGER_ENABLED));
			free(data.name);
			lua_pop(L, 1);
		}
	}
}

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

		if (!(arr[i].state & TRIGGER_ENABLED))
			continue;

		if (arr[i].trigger_type != POSITION)
			continue;

		if (arr[i].trigger.position.lvl != pos.z)
 			continue;

		if (arr[i].trigger.position.teleported != -1)
			if (arr[i].trigger.position.teleported != teleported)
				continue;

		if (((int)pos.x != arr[i].trigger.position.x)
				|| ((int)pos.y != arr[i].trigger.position.y))
			continue;

		run_lua(LUA_DIALOG, arr[i].lua_code);
		if (arr[i].single_activation)
			arr[i].state &= ~TRIGGER_ENABLED;
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
		if (!(arr[i].state & TRIGGER_ENABLED))
			continue;

		if (arr[i].trigger_type != CHANGE_LEVEL)
			continue;

		if (arr[i].trigger.change_level.exit_level != -1)
			if (arr[i].trigger.change_level.exit_level != past_lvl)
				continue;

		if (arr[i].trigger.change_level.enter_level != -1)
			if (arr[i].trigger.change_level.enter_level != cur_lvl)
				continue;

		run_lua(LUA_DIALOG, arr[i].lua_code);
		if (arr[i].single_activation)
			arr[i].state &= ~TRIGGER_ENABLED;
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
		if (!(arr[i].state & TRIGGER_ENABLED))
			continue;

		if (arr[i].trigger_type != event)
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
		if (arr[i].single_activation)
			arr[i].state &= ~TRIGGER_ENABLED;
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
		if (!(arr[i].state & TRIGGER_ENABLED))
			continue;

		if (arr[i].trigger_type != OBSTACLE_ACTION)
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
		if (arr[i].single_activation)
			arr[i].state &= ~TRIGGER_ENABLED;
	}
}

/**
 *
 *
 */
const char *teleporter_square_below_mouse_cursor(void)
{
	if (MouseCursorIsInUserRect(GetMousePos_x(), GetMousePos_y())) {
		struct pointf map_position_of_mouse;
		map_position_of_mouse.x = translate_pixel_to_map_location((float)input_axis.x, (float)input_axis.y, TRUE);
		map_position_of_mouse.y = translate_pixel_to_map_location((float)input_axis.x, (float)input_axis.y, FALSE);

		struct event_trigger *arr = visible_event_at_location((int)map_position_of_mouse.x, (int)map_position_of_mouse.y, Me.pos.z);
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
		if (!(arr[i].state & TRIGGER_ENABLED))
			continue;

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

		return &arr[i];
	}
	return NULL;
}

void dispatch_event_timer(const char *trigger_name, float dispatch_time)
{
	struct event_timer *evt = MyMalloc(sizeof(struct event_timer));
	evt->trigger_name = strdup(trigger_name);
	evt->dispatch_time = dispatch_time;

	int added = FALSE;

	// Try to add the timer before a timer that schedules later
	struct event_timer *n, *next;
	list_for_each_entry_safe(n, next, &event_timer_head, node) {
		if (evt->dispatch_time <= n->dispatch_time) {
			list_add_tail(&evt->node, &n->node);
			added = TRUE;
			break;
		}
	}

	// Otherwise, add it at the end of the list.
	if (!added) {
		list_add_tail(&evt->node, &event_timer_head);
	}
}

void execute_event_timers()
{
	struct event_timer *n, *next;

	list_for_each_entry_safe(n, next, &event_timer_head, node) {
		if (n->dispatch_time < Me.current_game_date) {
			for (int i = 0; i < event_triggers.size; i++) {
				struct event_trigger *evt_trigger = (struct event_trigger *)dynarray_member(&event_triggers, i, sizeof(struct event_trigger));
				if (strcmp(evt_trigger->name, n->trigger_name))
					continue;
				if ((evt_trigger->state & TRIGGER_ENABLED) && evt_trigger->lua_code)
					run_lua(LUA_DIALOG, evt_trigger->lua_code);
			}
			list_del(&n->node);
			free(n->trigger_name);
			free(n);
		} else {
			break; // Timer are sorted, so we can stop now.
		}
	}
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

	for (int i = 0; i < game_acts.size; i++) {
		struct game_act *act = (struct game_act *)dynarray_member(&game_acts, i, sizeof(struct game_act));
		game_act_set_current(act);
		prepare_start_of_new_game("NewTuxStartGameSquare", TRUE);

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
		for (int i = 0; i < event_triggers.size; i++) {
			struct event_trigger *evt = (struct event_trigger *)dynarray_member(&event_triggers, i, sizeof(struct event_trigger));
			printf("Testing event \"%s\" from  \"%s\"...\n", evt->name, act->name);
			int rtn = run_lua(LUA_DIALOG, evt->lua_code);
			if (rtn)
				error_caught = TRUE;
			if (term_has_color_cap)
				printf("Result: %s\n", !rtn ? "\033[32msuccess\033[0m" : "\033[31mfailed\033[0m");
			else
				printf("Result: %s\n", !rtn ? "success" : "failed");
		}

		// Prepare the next round
		free_game_data();
	}

	/* Re-enable sound as needed. */
	sound_on = old_sound_on;

	/* Re-enable screen fadings. */
	GameConfig.do_fadings = TRUE;

	return error_caught;
}
