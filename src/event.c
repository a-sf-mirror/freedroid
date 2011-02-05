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

#define MAX_EVENT_TRIGGERS      500	// how many event triggers at most to allow

static struct event_trigger {
	int level;
	point position;

	luacode lua_code;

	char *name;
	int enabled;		//is the trigger enabled?
	int silent;		//do we have to advertise this trigger to the user? (teleporters..)
} event_triggers[MAX_EVENT_TRIGGERS];

/**
 * Delete all events and event triggers
 */
static void clear_out_event_triggers(void)
{
	int i;

	for (i = 0; i < MAX_EVENT_TRIGGERS; i++) {
		event_triggers[i].level = -1;
		event_triggers[i].position.x = -1;
		event_triggers[i].position.y = -1;

		event_triggers[i].enabled = 1;

		if (event_triggers[i].name)
			free(event_triggers[i].name);
		event_triggers[i].name = NULL;

		if (event_triggers[i].lua_code)
			free(event_triggers[i].lua_code);
		event_triggers[i].lua_code = NULL;
	}
};				// void clear_out_event_triggers(void)

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
	int EventTriggerNumber;
	char *TempMapLabelName;
	location TempLocation;
	char s;

	EventPointer = EventSectionPointer;
	EventTriggerNumber = 0;
	while ((EventPointer = strstr(EventPointer, EVENT_TRIGGER_BEGIN_STRING)) != NULL) {
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
		event_triggers[EventTriggerNumber].position.x = TempLocation.x;
		event_triggers[EventTriggerNumber].position.y = TempLocation.y;
		event_triggers[EventTriggerNumber].level = TempLocation.level;

		free(TempMapLabelName);

		event_triggers[EventTriggerNumber].name = ReadAndMallocStringFromData(EventPointer, EVENT_TRIGGER_NAME_STRING, "\"");

		ReadValueFromStringWithDefault(EventPointer, EVENT_TRIGGER_IS_SILENT_STRING, "%d", "1",
					       &event_triggers[EventTriggerNumber].silent, EndOfEvent);

		event_triggers[EventTriggerNumber].lua_code =
		    ReadAndMallocStringFromData(EventPointer, EVENT_TRIGGER_LUACODE_STRING, EVENT_TRIGGER_LUACODE_END_STRING);

		ReadValueFromStringWithDefault(EventPointer, EVENT_TRIGGER_ENABLED_STRING, "%d", "1",
					       &event_triggers[EventTriggerNumber].enabled, EndOfEvent);

		EventTriggerNumber++;
		EndOfEvent[strlen(EVENT_TRIGGER_END_STRING) - 1] = '\0';

		EndOfEvent[strlen(EVENT_TRIGGER_END_STRING) - 1] = s;
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
void CheckForTriggeredEvents()
{
	int i;

	// Now we check if some event trigger is fullfilled.
	//
	for (i = 0; i < MAX_EVENT_TRIGGERS; i++) {
		if (event_triggers[i].enabled == 0)
			continue;	// this trigger is not enabled

		// So at this point we know, that the event trigger is somehow meaningful. 
		// Fine, so lets check the details, if the event is triggered now
		//
		if (rintf(event_triggers[i].level) != CURLEVEL()->levelnum)
			continue;

		if (rintf(event_triggers[i].position.x) != (int)(Me.pos.x))
			continue;

		if (rintf(event_triggers[i].position.y) != (int)(Me.pos.y))
			continue;

		run_lua(event_triggers[i].lua_code);

	}
};				// CheckForTriggeredEvents(void )

/**
 *
 *
 */
const char *teleporter_square_below_mouse_cursor(void)
{
	finepoint MapPositionOfMouse;
	struct event_trigger *t = NULL;
	int i;

	if (MouseCursorIsInUserRect(GetMousePos_x(), GetMousePos_y())) {
		MapPositionOfMouse.x = translate_pixel_to_map_location((float)input_axis.x, (float)input_axis.y, TRUE);
		MapPositionOfMouse.y = translate_pixel_to_map_location((float)input_axis.x, (float)input_axis.y, FALSE);

		for (i = 0; i < MAX_EVENT_TRIGGERS; i++) {
			t = &event_triggers[i];

			if (Me.pos.z != t->level)
				continue;
			if ((((int)MapPositionOfMouse.x) != t->position.x))
				continue;
			if ((((int)MapPositionOfMouse.y) != t->position.y))
				continue;
			if (t->silent)
				continue;
			if (!t->enabled)
				continue;

			return D_(event_triggers[i].name);
		}
	}
	return NULL;
}

/**
 * Enable or disable the trigger with the given name.
 */
void event_modify_trigger_state(const char *name, int state)
{
	struct event_trigger *target_event = NULL;
	int i;
	for (i = 0; i < MAX_EVENT_TRIGGERS; i++) {
		if (!event_triggers[i].name)
			break;

		if (!strcmp(event_triggers[i].name, name)) {
			target_event = &event_triggers[i];
			break;
		}
	}

	if (target_event)
		target_event->enabled = state;
}

