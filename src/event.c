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

#include "system.h"

#include "defs.h"
#include "struct.h"
#include "global.h"
#include "proto.h"
#include "savestruct.h"

#include "../lua/lua.h"
#include "../lua/lauxlib.h"
#include "../lua/lualib.h"

/* Our Lua state for event execution */
lua_State *event_lua_state;

/* Our triggers */
event_trigger AllEventTriggers[ MAX_EVENT_TRIGGERS ];

/**
 * Delete all events and event triggers
 */
static void
clear_out_event_triggers( void )
{
    int i;
    
    for ( i = 0 ; i < MAX_EVENT_TRIGGERS ; i++ )
    {
	AllEventTriggers[i].Influ_Must_Be_At_Level=-1;
	AllEventTriggers[i].Influ_Must_Be_At_Point.x=-1;
	AllEventTriggers[i].Influ_Must_Be_At_Point.y=-1;
	
	AllEventTriggers[i].enabled=1;
	
	if (AllEventTriggers[i].name)
	    free(AllEventTriggers[i].name);
	AllEventTriggers[i].name = NULL;

	if (AllEventTriggers[i].lua_code)
	    free(AllEventTriggers[i].lua_code);	    
	AllEventTriggers[i].lua_code = NULL;
    }
}; // void clear_out_event_triggers(void)

#define EVENT_TRIGGER_BEGIN_STRING "* New event trigger *"
#define EVENT_TRIGGER_END_STRING "* End of trigger *"
#define EVENT_TRIGGER_NAME_STRING "Name=\""

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
static void decode_event_triggers ( char* EventSectionPointer )
{
    char *EventPointer;
    char *EndOfEvent;
    int EventTriggerNumber;
    char* TempMapLabelName;
    location TempLocation;
    char s;

    EventPointer=EventSectionPointer;
    EventTriggerNumber=0;
    while ( ( EventPointer = strstr ( EventPointer , EVENT_TRIGGER_BEGIN_STRING ) ) != NULL)
    {
	EventPointer += strlen( EVENT_TRIGGER_BEGIN_STRING ) + 1;
	
	EndOfEvent = LocateStringInData ( EventPointer , EVENT_TRIGGER_END_STRING );
	s = EndOfEvent[strlen(EVENT_TRIGGER_END_STRING)-1];
        EndOfEvent[strlen(EVENT_TRIGGER_END_STRING)-1]	= 0;

	DebugPrintf ( 1 , "\nStarting to read details of this event trigger section\n\n");
	
	//--------------------
	// Now we decode the details of this event trigger section
	//
	
	// Now we read in the triggering position in x and y and z coordinates
	TempMapLabelName = 
	    ReadAndMallocStringFromData ( EventPointer , EVENT_TRIGGER_LABEL_STRING , "\"" ) ;
	ResolveMapLabelOnShip ( TempMapLabelName , &TempLocation );
	AllEventTriggers [ EventTriggerNumber ] . Influ_Must_Be_At_Point . x = TempLocation . x ;
	AllEventTriggers [ EventTriggerNumber ] . Influ_Must_Be_At_Point . y = TempLocation . y ;
	AllEventTriggers[ EventTriggerNumber ] . Influ_Must_Be_At_Level = TempLocation . level ;

	free ( TempMapLabelName );	

	AllEventTriggers[EventTriggerNumber].name = ReadAndMallocStringFromData(EventPointer, EVENT_TRIGGER_NAME_STRING, "\"");

	ReadValueFromStringWithDefault( EventPointer , EVENT_TRIGGER_DELETED_AFTER_TRIGGERING , "%d" , "0",
			     &AllEventTriggers[ EventTriggerNumber ].DeleteTriggerAfterExecution , EndOfEvent );
	
	ReadValueFromStringWithDefault( EventPointer , EVENT_TRIGGER_IS_SILENT_STRING , "%d" , "1",
			     &AllEventTriggers[ EventTriggerNumber ].silent , EndOfEvent );
	
	AllEventTriggers[ EventTriggerNumber ].lua_code = 
	    ReadAndMallocStringFromData ( EventPointer , EVENT_TRIGGER_LUACODE_STRING , EVENT_TRIGGER_LUACODE_END_STRING ) ;

	ReadValueFromStringWithDefault( EventPointer , EVENT_TRIGGER_ENABLED_STRING , "%d" , "1",
			     &AllEventTriggers[ EventTriggerNumber ].enabled , EndOfEvent );
	
	EventTriggerNumber++;
	        EndOfEvent[strlen(EVENT_TRIGGER_END_STRING)-1] = '\0';

	EndOfEvent[strlen(EVENT_TRIGGER_END_STRING)-1] = s;
    } // While Event trigger begin string found...
    

}; // void decode_event_triggers ( char* EventSectionPointer )

/**
 * This function reads in the game events, i.e. the locations and conditions
 * under which some actions are triggered.
 */
void GetEventTriggers ( const char* EventsAndEventTriggersFilename )
{
  char* EventSectionPointer;
  char fpath[2048];

  clear_out_event_triggers();

  find_file (EventsAndEventTriggersFilename , MAP_DIR , fpath, 0 );
  EventSectionPointer = 
    ReadAndMallocAndTerminateFile( fpath , 
				   "*** END OF EVENT ACTION AND EVENT TRIGGER FILE *** LEAVE THIS TERMINATOR IN HERE ***" 
				   ) ;

  decode_event_triggers ( EventSectionPointer );

  free ( EventSectionPointer ) ;
};

static int lua_event_teleport(lua_State *L)
{
    const char *label = luaL_checkstring(L, 1);
    location TempLocation;
    ResolveMapLabelOnShip ( label, &TempLocation);
    Teleport(TempLocation . level, TempLocation . x + 0.5, TempLocation . y + 0.5, TRUE);
    return 0;
}

static int lua_event_display_big_message(lua_State *L)
{
    const char *msg = luaL_checkstring(L, 1);
    SetNewBigScreenMessage (msg);
    return 0;
}

static int lua_event_display_console_message(lua_State *L)
{
    const char *msg = luaL_checkstring(L, 1);
    append_new_game_message (msg);
    return 0;
}

static void event_modify_trigger(const char * name, int state)
{
    event_trigger * target_event = NULL;
    int i;
    for ( i = 0 ; i < MAX_EVENT_TRIGGERS ; i++ )
	{
	if (!strcmp (AllEventTriggers[i].name, name))
	    target_event = &AllEventTriggers[i];
	}

    target_event -> enabled = state;
}

static int lua_event_enable_trigger(lua_State *L)
{
    const char * name = luaL_checkstring(L, 1);
    event_modify_trigger(name, 1);
    return 0;
}

static int lua_event_disable_trigger(lua_State *L)
{
    const char * name = luaL_checkstring(L, 1);
    event_modify_trigger(name, 0);
    return 0;
}

static int event_change_obstacle_type(const char *obslabel, const char *state)
{
    obstacle *our_obstacle = give_pointer_to_obstacle_with_label (obslabel) ;
    int obstacle_level_num = give_level_of_obstacle_with_label (obslabel) ;
    level *obstacle_level = curShip . AllLevels [ obstacle_level_num ] ;

    if (state != NULL) {
	int j;
	int base  = obstacle_level -> obstacle_statelist_base  [ our_obstacle -> name_index ] ;
	int count = obstacle_level -> obstacle_statelist_count [ our_obstacle -> name_index ] ;
	for (j=0; j<count; j++)
	    if ( !strcmp ( obstacle_level->obstacle_states_names [ j+base ], state) ) {
		our_obstacle -> type = obstacle_level->obstacle_states_values [ j+base ];
		break;
	    }
    } else {
	action_remove_obstacle (obstacle_level, our_obstacle);
    }

    //--------------------
    // Now we make sure the door lists and that are all updated...
    GetAllAnimatedMapTiles (obstacle_level) ;

    //--------------------
    // Also make sure the other maps realize the change too, if it
    // maybe happend in the border area where two maps are glued together
    // only export if the obstacle falls within the interface zone

    if( our_obstacle->pos.x <= obstacle_level->jump_threshold_west ||
	    our_obstacle->pos.x >= obstacle_level->xlen - obstacle_level->jump_threshold_east ||
	    our_obstacle->pos.y <= obstacle_level->jump_threshold_north ||
	    our_obstacle->pos.y >= obstacle_level->ylen - obstacle_level->jump_threshold_south
      ) 
	ExportLevelInterface ( obstacle_level_num ) ;

    return 0;
}

static int lua_event_change_obstacle(lua_State *L) 
{
    const char *obslabel = luaL_checkstring(L, 1);
    const char *type = luaL_checkstring(L, 2);
    event_change_obstacle_type(obslabel, type);
    return 0;
}

static int lua_event_delete_obstacle(lua_State *L)
{
    const char *obslabel = luaL_checkstring(L, 1);
    event_change_obstacle_type(obslabel, NULL);
    return 0;
}

static void init_lua_events()
{
    char fpath[2048];

    event_lua_state = lua_open();
    luaL_openlibs(event_lua_state);

    if (!find_file("script_helpers.lua", MAP_DIR, fpath, 0))
	if (luaL_dofile(event_lua_state, fpath ))
	    ErrorMessage(__FUNCTION__, "Cannot open script helpers file script_helpers.lua: %s.\n", PLEASE_INFORM, IS_FATAL, lua_tostring(event_lua_state, -1));

    lua_pushcfunction(event_lua_state, lua_event_teleport);
    lua_setglobal(event_lua_state, "teleport");
    lua_pushcfunction(event_lua_state, lua_event_display_big_message);
    lua_setglobal(event_lua_state, "display_big_message");
    lua_pushcfunction(event_lua_state, lua_event_display_console_message);
    lua_setglobal(event_lua_state, "display_console_message");
    lua_pushcfunction(event_lua_state, lua_event_enable_trigger);
    lua_setglobal(event_lua_state, "enable_trigger");
    lua_pushcfunction(event_lua_state, lua_event_disable_trigger);
    lua_setglobal(event_lua_state, "disable_trigger");
    lua_pushcfunction(event_lua_state, lua_event_change_obstacle);
    lua_setglobal(event_lua_state, "change_obstacle");
    lua_pushcfunction(event_lua_state, lua_event_delete_obstacle);
    lua_setglobal(event_lua_state, "delete_obstacle");

}

/**
 * Execute an action given the Lua function name from the freedroid events Lua file
 */
void ExecuteActionWithLabel (char * name)
{
    if (!event_lua_state) /* first call ? initialize Lua */
	init_lua_events();

    lua_getglobal(event_lua_state, name);
    if(lua_pcall(event_lua_state, 0, 0, 0)) {
	ErrorMessage(__FUNCTION__, "Error running Lua event %s: %s.\n", PLEASE_INFORM, IS_FATAL, name, lua_tostring(event_lua_state, -1));
    }
}

/**
 * Execute an action given its code as a string
 */
void ExecuteAction (char *code)
{
    if (!event_lua_state) /* first call ? initialize Lua */
	init_lua_events();

    if(luaL_dostring(event_lua_state, code)) {
	ErrorMessage(__FUNCTION__, "Error running Lua event code {%s}: %s.\n", PLEASE_INFORM, IS_FATAL, code, lua_tostring(event_lua_state, -1));
    }
}

/**
 *
 * This function checks for triggered events & statements.  Those events are
 * usually entered via the mission file and read into the apropriate
 * structures via the InitNewMission function.  Here we check, whether
 * the nescessary conditions for an event are satisfied, and in case that
 * they are, we order the apropriate event to be executed.
 *
 * In addition, statements are started, if the influencer is at the 
 * right location for them.
 *
 */
void CheckForTriggeredEventsAndStatements ( )
{
    int i;
    int map_x, map_y;
    Level StatementLevel = curShip.AllLevels[ Me . pos . z ] ;
    
    //--------------------
    // Now we check if some statment location is reached
    //
    map_x = (int) rintf( (float) Me . pos . x ); map_y = (int) rintf( (float)Me . pos . y ) ;
    for ( i = 0 ; i < MAX_STATEMENTS_PER_LEVEL ; i++ )
    {
	if ( ( map_x == StatementLevel -> StatementList [ i ] . x ) &&
	     ( map_y == StatementLevel -> StatementList [ i ] . y ) )
	{
	    Me . TextVisibleTime = 0 ;
	    Me . TextToBeDisplayed = CURLEVEL -> StatementList [ i ] . Statement_Text ;
	}
    }
    
    //--------------------
    // Now we check if some event trigger is fullfilled.
    //
    for ( i = 0 ; i < MAX_EVENT_TRIGGERS ; i++ )
    {
	if ( AllEventTriggers[i].enabled == 0 ) continue;  // this trigger is not enabled
	
	// --------------------
	// So at this point we know, that the event trigger is somehow meaningful. 
	// Fine, so lets check the details, if the event is triggered now
	//
	if ( rintf( AllEventTriggers[i].Influ_Must_Be_At_Level ) != StatementLevel->levelnum ) continue;

	if ( rintf( AllEventTriggers[i].Influ_Must_Be_At_Point.x ) != (int) ( Me . pos.x ) ) continue;

	if ( rintf( AllEventTriggers[i].Influ_Must_Be_At_Point.y ) != (int) ( Me . pos.y ) ) continue;

	ExecuteAction ( AllEventTriggers [ i ] . lua_code ) ;
	
	if ( AllEventTriggers[i].DeleteTriggerAfterExecution == 1 )
	{
	    AllEventTriggers[i].enabled = 0;
	}
    }

}; // CheckForTriggeredEventsAndStatements (void )

/**
 *
 *
 */
int teleporter_square_below_mouse_cursor ( char* ItemDescText )
{
    finepoint MapPositionOfMouse;
    int i;

    if ( MouseCursorIsInUserRect( GetMousePos_x()  , 
		GetMousePos_y()  ) )
	{
	MapPositionOfMouse . x = 
	    translate_pixel_to_map_location ( (float) input_axis.x , 
		    (float) input_axis.y , TRUE ) ;
	MapPositionOfMouse . y = 
	    translate_pixel_to_map_location ( (float) input_axis.x , 
		    (float) input_axis.y , FALSE ) ;

	for ( i = 0 ; i < MAX_EVENT_TRIGGERS ; i++ )
	    {
	    if ( ( ( (int) MapPositionOfMouse . x ) != AllEventTriggers [ i ] . Influ_Must_Be_At_Point . x ) )
		continue;
	    if ( ( ( (int) MapPositionOfMouse . y ) != AllEventTriggers [ i ] . Influ_Must_Be_At_Point . y ) )
		continue;
	    if ( Me . pos . z != AllEventTriggers [ i ] . Influ_Must_Be_At_Level )
		continue;
	    if ( AllEventTriggers[i].silent )
		continue;

	    // DebugPrintf ( -1000 , "\nSome trigger seems to be here..." );

	    //--------------------
	    // Now we know, that the mouse is currently exactly over an event trigger.  The
	    // question to be answered still is whether this trigger also triggers a teleporter
	    // action or not and if yes, where the connection leads to...
	    //
	    sprintf ( ItemDescText , "%s..." , D_(AllEventTriggers[i].name) ) ;
	    return ( TRUE );
	    }
	}
    return ( FALSE );
}; // void teleporter_square_below_mouse_cursor ( char* ItemDescText )


