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

/**
 * Delete all events and event triggers
 */
static void clear_out_event_triggers(void)
{
	int i;

	for (i = 0; i < MAX_EVENT_TRIGGERS; i++) {
		AllEventTriggers[i].Influ_Must_Be_At_Level = -1;
		AllEventTriggers[i].Influ_Must_Be_At_Point.x = -1;
		AllEventTriggers[i].Influ_Must_Be_At_Point.y = -1;

		AllEventTriggers[i].enabled = 1;

		if (AllEventTriggers[i].name)
			free(AllEventTriggers[i].name);
		AllEventTriggers[i].name = NULL;

		if (AllEventTriggers[i].lua_code)
			free(AllEventTriggers[i].lua_code);
		AllEventTriggers[i].lua_code = NULL;
	}
};				// void clear_out_event_triggers(void)

#define EVENT_TRIGGER_BEGIN_STRING "* New event trigger *"
#define EVENT_TRIGGER_END_STRING "* End of trigger *"
#define EVENT_TRIGGER_NAME_STRING "Name=_\""

#define EVENT_TRIGGER_IS_SILENT_STRING "Silent="
#define EVENT_TRIGGER_DELETED_AFTER_TRIGGERING "Delete the event trigger after it has been triggered="
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

		//--------------------
		// Now we decode the details of this event trigger section
		//

		// Now we read in the triggering position in x and y and z coordinates
		TempMapLabelName = ReadAndMallocStringFromData(EventPointer, EVENT_TRIGGER_LABEL_STRING, "\"");
		ResolveMapLabelOnShip(TempMapLabelName, &TempLocation);
		AllEventTriggers[EventTriggerNumber].Influ_Must_Be_At_Point.x = TempLocation.x;
		AllEventTriggers[EventTriggerNumber].Influ_Must_Be_At_Point.y = TempLocation.y;
		AllEventTriggers[EventTriggerNumber].Influ_Must_Be_At_Level = TempLocation.level;

		free(TempMapLabelName);

		AllEventTriggers[EventTriggerNumber].name = ReadAndMallocStringFromData(EventPointer, EVENT_TRIGGER_NAME_STRING, "\"");

		ReadValueFromStringWithDefault(EventPointer, EVENT_TRIGGER_DELETED_AFTER_TRIGGERING, "%d", "0",
					       &AllEventTriggers[EventTriggerNumber].DeleteTriggerAfterExecution, EndOfEvent);

		ReadValueFromStringWithDefault(EventPointer, EVENT_TRIGGER_IS_SILENT_STRING, "%d", "1",
					       &AllEventTriggers[EventTriggerNumber].silent, EndOfEvent);

		AllEventTriggers[EventTriggerNumber].lua_code =
		    ReadAndMallocStringFromData(EventPointer, EVENT_TRIGGER_LUACODE_STRING, EVENT_TRIGGER_LUACODE_END_STRING);

		ReadValueFromStringWithDefault(EventPointer, EVENT_TRIGGER_ENABLED_STRING, "%d", "1",
					       &AllEventTriggers[EventTriggerNumber].enabled, EndOfEvent);

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
 * usually entered via the mission file and read into the apropriate
 * structures via the InitNewMission function.  Here we check, whether
 * the nescessary conditions for an event are satisfied, and in case that
 * they are, we order the apropriate event to be executed.
 *
 */
void CheckForTriggeredEvents()
{
	int i;

	//--------------------
	// Now we check if some event trigger is fullfilled.
	//
	for (i = 0; i < MAX_EVENT_TRIGGERS; i++) {
		if (AllEventTriggers[i].enabled == 0)
			continue;	// this trigger is not enabled

		// --------------------
		// So at this point we know, that the event trigger is somehow meaningful. 
		// Fine, so lets check the details, if the event is triggered now
		//
		if (rintf(AllEventTriggers[i].Influ_Must_Be_At_Level) != CURLEVEL()->levelnum)
			continue;

		if (rintf(AllEventTriggers[i].Influ_Must_Be_At_Point.x) != (int)(Me.pos.x))
			continue;

		if (rintf(AllEventTriggers[i].Influ_Must_Be_At_Point.y) != (int)(Me.pos.y))
			continue;

		run_lua(AllEventTriggers[i].lua_code);

		if (AllEventTriggers[i].DeleteTriggerAfterExecution == 1) {
			AllEventTriggers[i].enabled = 0;
		}
	}

};				// CheckForTriggeredEvents(void )

/**
 *
 *
 */
int teleporter_square_below_mouse_cursor(char *ItemDescText)
{
	finepoint MapPositionOfMouse;
	int i;

	if (MouseCursorIsInUserRect(GetMousePos_x(), GetMousePos_y())) {
		MapPositionOfMouse.x = translate_pixel_to_map_location((float)input_axis.x, (float)input_axis.y, TRUE);
		MapPositionOfMouse.y = translate_pixel_to_map_location((float)input_axis.x, (float)input_axis.y, FALSE);

		for (i = 0; i < MAX_EVENT_TRIGGERS; i++) {
			if ((((int)MapPositionOfMouse.x) != AllEventTriggers[i].Influ_Must_Be_At_Point.x))
				continue;
			if ((((int)MapPositionOfMouse.y) != AllEventTriggers[i].Influ_Must_Be_At_Point.y))
				continue;
			if (Me.pos.z != AllEventTriggers[i].Influ_Must_Be_At_Level)
				continue;
			if (AllEventTriggers[i].silent)
				continue;

			// DebugPrintf ( -1000 , "\nSome trigger seems to be here..." );

			//--------------------
			// Now we know, that the mouse is currently exactly over an event trigger.  The
			// question to be answered still is whether this trigger also triggers a teleporter
			// action or not and if yes, where the connection leads to...
			//
			sprintf(ItemDescText, "%s", D_(AllEventTriggers[i].name));
			return (TRUE);
		}
	}
	return (FALSE);
};				// void teleporter_square_below_mouse_cursor ( char* ItemDescText )
