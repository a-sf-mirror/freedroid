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
 * This file contains functions related to events and event triggers.
 */

#define _event_c
#include "system.h"

#include "defs.h"
#include "struct.h"
#include "global.h"
#include "proto.h"
#include "savestruct.h"

struct event_trigger {
	int level;
	point position;

	luacode lua_code;

	char *name;
	int enabled;		//is the trigger enabled?
	int silent;		//do we have to advertise this trigger to the user? (teleporters..)
};

static struct dynarray event_triggers;

/**
 * Delete all events and event triggers
 */
static void clear_out_event_triggers(void)
{
	// Remove existing triggers
	int i;
	struct event_trigger *arr = event_triggers.arr;
	for (i = 0; i < event_triggers.size; i++) {
		if (arr[i].name)
			free(arr[i].name);

		if (arr[i].lua_code)
			free(arr[i].lua_code);
	}

	dynarray_free(&event_triggers);
	
	dynarray_init(&event_triggers, 16, sizeof(struct event_trigger));
}

#define EVENT_TRIGGER_BEGIN_STRING "* New event trigger *"
#define EVENT_TRIGGER_END_STRING "* End of trigger *"
#define EVENT_TRIGGER_NAME_STRING "Name=_\""

#define EVENT_TRIGGER_IS_SILENT_STRING "Silent="
#define EVENT_TRIGGER_LUACODE_STRING "LuaCode={"
#define EVENT_TRIGGER_LUACODE_END_STRING "}"
#define EVENT_TRIGGER_LABEL_STRING "Trigger at label=\""
#define EVENT_TRIGGER_ENABLED_STRING "Enable this trigger by default="

/** 
 *
 *
 */
static void decode_event_triggers(char *EventSectionPointer)
{
	char *EventPointer;
	char *EndOfEvent;
	char *TempMapLabelName;
	location TempLocation;
	char s;
	struct event_trigger temp;

	EventPointer = EventSectionPointer;
	while ((EventPointer = strstr(EventPointer, EVENT_TRIGGER_BEGIN_STRING)) != NULL) {
		memset(&temp, 0, sizeof(struct event_trigger));

		EventPointer += strlen(EVENT_TRIGGER_BEGIN_STRING) + 1;

		EndOfEvent = LocateStringInData(EventPointer, EVENT_TRIGGER_END_STRING);
		s = EndOfEvent[strlen(EVENT_TRIGGER_END_STRING) - 1];
		EndOfEvent[strlen(EVENT_TRIGGER_END_STRING) - 1] = 0;

		DebugPrintf(1, "\nStarting to read details of this event trigger section\n\n");

		// Now we decode the details of this event trigger section
		//

		// Now we read in the triggering position in x and y and z coordinates
		TempMapLabelName = ReadAndMallocStringFromData(EventPointer, EVENT_TRIGGER_LABEL_STRING, "\"");
		ResolveMapLabelOnShip(TempMapLabelName, &TempLocation);
		temp.position.x = TempLocation.x;
		temp.position.y = TempLocation.y;
		temp.level = TempLocation.level;

		free(TempMapLabelName);

		temp.name = ReadAndMallocStringFromData(EventPointer, EVENT_TRIGGER_NAME_STRING, "\"");

		ReadValueFromStringWithDefault(EventPointer, EVENT_TRIGGER_IS_SILENT_STRING, "%d", "1",
					       &temp.silent, EndOfEvent);

		temp.lua_code =
		    ReadAndMallocStringFromData(EventPointer, EVENT_TRIGGER_LUACODE_STRING, EVENT_TRIGGER_LUACODE_END_STRING);

		ReadValueFromStringWithDefault(EventPointer, EVENT_TRIGGER_ENABLED_STRING, "%d", "1",
					       &temp.enabled, EndOfEvent);

		EndOfEvent[strlen(EVENT_TRIGGER_END_STRING) - 1] = '\0';

		EndOfEvent[strlen(EVENT_TRIGGER_END_STRING) - 1] = s;

		dynarray_add(&event_triggers, &temp, sizeof(struct event_trigger));
	}			// While Event trigger begin string found...

};				// void decode_event_triggers ( char* EventSectionPointer )

/**
 * This function reads in the game events, i.e. the locations and conditions
 * under which some actions are triggered.
 */
void GetEventTriggers(const char *EventsAndEventTriggersFilename)
{
	char *EventSectionPointer;
	char fpath[2048];

	clear_out_event_triggers();

	find_file(EventsAndEventTriggersFilename, MAP_DIR, fpath, 0);
	EventSectionPointer =
	    ReadAndMallocAndTerminateFile(fpath, "*** END OF EVENT ACTION AND EVENT TRIGGER FILE *** LEAVE THIS TERMINATOR IN HERE ***");

	decode_event_triggers(EventSectionPointer);

	free(EventSectionPointer);
};

/**
 *
 * This function checks for triggered events.  Those events are
 * usually entered via the mission file and read into the appropriate
 * structures via the InitNewMission function.  Here we check, whether
 * the necessary conditions for an event are satisfied, and in case that
 * they are, we order the appropriate event to be executed.
 *
 */
void trigger_events(void)
{
	int i;
	struct event_trigger *arr = event_triggers.arr;

	for (i = 0; i < event_triggers.size; i++) {

		if (arr[i].level != Me.pos.z)
			continue;

		if (!arr[i].enabled)
			continue;

		if (arr[i].position.x != (int)(Me.pos.x))
			continue;

		if (arr[i].position.y != (int)(Me.pos.y))
			continue;

		run_lua(arr[i].lua_code);
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
 * Enable or disable the trigger with the given name.
 */
void event_modify_trigger_state(const char *name, int state)
{
	int i;
	struct event_trigger *arr = event_triggers.arr;

	for (i = 0; i < event_triggers.size; i++) {
		if (!arr[i].name)
			break;

		if (!strcmp(arr[i].name, name)) {
			arr[i].enabled = state;
			return;
		}
	}
}

/**
 * Visible event Trigger at a location
 */
struct event_trigger * visible_event_at_location(int x, int y, int z)
{
	int i;
	struct event_trigger *arr = event_triggers.arr;
	for (i = 0; i < event_triggers.size; i++) {
		if (z != arr[i].level)
			continue;

		if (x != arr[i].position.x)
			continue;

		if (y != arr[i].position.y)
			continue;

		if (arr[i].silent)
			continue;

		if (!arr[i].enabled)
			continue;

		return &arr[i];
	}
	return NULL;
}
