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

/**
 * Delete all events and event triggers
 */
static void
clear_out_all_events_and_actions( void )
{
    int i;
    
    for ( i = 0 ; i < MAX_EVENT_TRIGGERS ; i++ )
    {
	AllEventTriggers[i].Influ_Must_Be_At_Level=-1;
	AllEventTriggers[i].Influ_Must_Be_At_Point.x=-1;
	AllEventTriggers[i].Influ_Must_Be_At_Point.y=-1;
	
	// Maybe the event is triggered by time
	AllEventTriggers[i].Mission_Time_Must_Have_Passed=-1;
	AllEventTriggers[i].Mission_Time_Must_Not_Have_Passed=-1;

	AllEventTriggers[i].enabled=1;
		
	AllEventTriggers[i].TargetActionLabel="none";
    }
    for ( i = 0 ; i < MAX_TRIGGERED_ACTIONS_IN_GAME ; i++ )
    {
	AllTriggeredActions[i].ActionLabel="";
	AllTriggeredActions[i].TeleportTarget.x = -1;
	AllTriggeredActions[i].TeleportTarget.y = -1;
	AllTriggeredActions[i].TeleportTargetLevel = -1;
	AllTriggeredActions[i].also_execute_action_label="";
	
	AllTriggeredActions[i].modify_obstacle_with_label="";
	AllTriggeredActions[i].modify_obstacle_to_type=0;
	AllTriggeredActions[i].modify_event_trigger_with_action_label="";
	AllTriggeredActions[i].modify_event_trigger_value=-1;
	AllTriggeredActions[i].show_big_screen_message="";
    }
}; // void clear_out_all_events_and_actions( void )

#define EVENT_TRIGGER_BEGIN_STRING "* New event trigger *"
#define EVENT_TRIGGER_END_STRING "* End of trigger *"
#define EVENT_ACTION_BEGIN_STRING "* New event action *"
#define EVENT_ACTION_END_STRING "* End of action *"


#define EVENT_ACTION_TELEPORT_TARGET_LABEL_STRING "Teleport to=\""

#define EVENT_ACTION_MODIFY_EVENT_TRIGGER_STRING "modify_event_trigger_with_action_label=\""
#define EVENT_ACTION_MODIFY_EVENT_TRIGGER_VALUE_STRING "modify_event_trigger_to="

#define EVENT_ACTION_ALSO_EXECUTE_ACTION_LABEL "Also execute action with label=\""

#define ACTION_LABEL_INDICATION_STRING "Name=\""

#define EVENT_TRIGGER_DELETED_AFTER_TRIGGERING "Delete the event trigger after it has been triggered="
#define TRIGGER_WHICH_TARGET_LABEL "Action=\""
#define EVENT_TRIGGER_LABEL_STRING "Trigger at label=\""
#define EVENT_TRIGGER_ENABLED_STRING "Enable this trigger by default="

#define MODIFY_OBSTACLE_WITH_LABEL_STRING "modify_obstacle_with_label=\""
#define MODIFY_OBSTACLE_TO_TYPE_STRING "modify_obstacle_to_type=\""

#define SHOW_BIG_SCREEN_MESSAGE_STRING "show_big_screen_message=\""

/**
 *
 *
 */
static void decode_all_event_actions ( char* EventSectionPointer )
{
    char *EventPointer;
    char *EndOfEvent;
    int EventActionNumber;
    char* TempMapLabelName;
    char* TempText;
    location TempLocation;

    EventPointer=EventSectionPointer;
    EventActionNumber=0;
    while ( ( EventPointer = strstr ( EventPointer , EVENT_ACTION_BEGIN_STRING ) ) != NULL)
	{
	EventPointer += strlen( EVENT_ACTION_BEGIN_STRING ) + 1;

	EndOfEvent = LocateStringInData ( EventPointer , EVENT_ACTION_END_STRING );
	*EndOfEvent = 0;
	DebugPrintf (1, "\n\nStarting to read details of this event action section\n\n");

	//--------------------
	// Now we decode the details of this event action section
	//
	AllTriggeredActions[ EventActionNumber].ActionLabel =
	    ReadAndMallocStringFromData ( EventPointer , ACTION_LABEL_INDICATION_STRING , "\"" ) ;

	//--------------------
	// Now we see if maybe there was an obstacle label given, that should be used
	// to change an obstacle later.  We take a look if that is the case at all, and
	// if it is, we'll read in the corresponding obstacle label of course.
	//
	if ( CountStringOccurences ( EventPointer , MODIFY_OBSTACLE_WITH_LABEL_STRING ) )
	    {
	    DebugPrintf ( 1 , "\nOBSTACLE LABEL FOUND IN THIS EVENT ACTION!" );
	    TempMapLabelName = 
		ReadAndMallocStringFromData ( EventPointer , MODIFY_OBSTACLE_WITH_LABEL_STRING , "\"" ) ;
	    AllTriggeredActions [ EventActionNumber ] . modify_obstacle_with_label = TempMapLabelName ;
	    DebugPrintf ( 1 , "\nThe label reads: %s." , AllTriggeredActions [ EventActionNumber ] . modify_obstacle_with_label );
	    //--------------------
	    // But if such an obstacle label has been given, we also need to decode the new type that
	    // this obstacle should be made into.  So we do it here:
	    //
	    AllTriggeredActions[ EventActionNumber ] . modify_obstacle_to_type =
		ReadAndMallocStringFromData ( EventPointer , MODIFY_OBSTACLE_TO_TYPE_STRING , "\"" ) ;
	    DebugPrintf ( 1 , "\nObstacle will be modified to state: %s." , AllTriggeredActions[ EventActionNumber ] . modify_obstacle_to_type );
	    }

	//--------------------
	// Now we see if maybe there was a text given, that should be shown to user.
	//
	if ( CountStringOccurences ( EventPointer , SHOW_BIG_SCREEN_MESSAGE_STRING ) )
	    {
	    DebugPrintf ( 1 , "\nTEXT MESSAGE FOUND IN THIS EVENT ACTION!" );
	    TempText = 
		ReadAndMallocStringFromData ( EventPointer , SHOW_BIG_SCREEN_MESSAGE_STRING , "\"" ) ;
	    AllTriggeredActions [ EventActionNumber ] . show_big_screen_message = TempText ;
	    DebugPrintf ( 1 , "\nThe message reads: %s." , AllTriggeredActions [ EventActionNumber ] . show_big_screen_message );
	    }

	//--------------------
	// Now we read in the teleport target position in x and y and level coordinates
	//
	if ( CountStringOccurences ( EventPointer , EVENT_ACTION_TELEPORT_TARGET_LABEL_STRING) )
	    {
	    TempMapLabelName = 
		ReadAndMallocStringFromData ( EventPointer , EVENT_ACTION_TELEPORT_TARGET_LABEL_STRING , "\"" ) ;
	    if ( strcmp ( TempMapLabelName , "NO_LABEL_DEFINED_YET" ) )
		{
		ResolveMapLabelOnShip ( TempMapLabelName , &TempLocation );
		AllTriggeredActions [ EventActionNumber ] . TeleportTarget . x = TempLocation . x ;
		AllTriggeredActions [ EventActionNumber ] . TeleportTarget . y = TempLocation . y ;
		AllTriggeredActions [ EventActionNumber ] . TeleportTargetLevel = TempLocation . level ;
		}
	    else 
		ErrorMessage(__FUNCTION__, "An action used NO_LABEL_DEFINED_YET map label teleport target.\n", PLEASE_INFORM, IS_FATAL);
	    free(TempMapLabelName);
	    }
	else
	    {
	    AllTriggeredActions [ EventActionNumber ] . TeleportTarget . x = -1 ;
	    AllTriggeredActions [ EventActionNumber ] . TeleportTarget . y = -1;
	    }

	if ( ! strstr( EventPointer, EVENT_ACTION_MODIFY_EVENT_TRIGGER_STRING ) ) //if there is no event trigger modified
	    {
	    AllTriggeredActions[ EventActionNumber ].modify_event_trigger_with_action_label = "";
	    }
	else  
	    {
	    AllTriggeredActions[ EventActionNumber ].modify_event_trigger_with_action_label =  	ReadAndMallocStringFromData ( EventPointer , 
		    EVENT_ACTION_MODIFY_EVENT_TRIGGER_STRING , "\"" ) ;
	    ReadValueFromStringWithDefault( EventPointer , EVENT_ACTION_MODIFY_EVENT_TRIGGER_VALUE_STRING , "%d" , "0",
		    &AllTriggeredActions[ EventActionNumber ].modify_event_trigger_value , EndOfEvent );

	    }

	if ( ! strstr( EventPointer, EVENT_ACTION_ALSO_EXECUTE_ACTION_LABEL ) ) //if there is no linked action
	    {
	    AllTriggeredActions[ EventActionNumber ].also_execute_action_label = "";
	    }
	else
	    {
	    AllTriggeredActions[ EventActionNumber ].also_execute_action_label = ReadAndMallocStringFromData ( EventPointer , 
		    EVENT_ACTION_ALSO_EXECUTE_ACTION_LABEL , "\"" ) ;
	    }

	EventActionNumber++;
	*EndOfEvent = EVENT_ACTION_END_STRING[0];
	} // While Event action begin string found...


    DebugPrintf (1, "\nThat must have been the last Event Action section.\nWe can now start with the Triggers. Good.");  

}; // void decode_all_event_actions ( char* EventSectionPointer )

/** 
 *
 *
 */
static void decode_all_event_triggers ( char* EventSectionPointer )
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
	
	ReadValueFromStringWithDefault( EventPointer , EVENT_TRIGGER_DELETED_AFTER_TRIGGERING , "%d" , "0",
			     &AllEventTriggers[ EventTriggerNumber ].DeleteTriggerAfterExecution , EndOfEvent );
	
	AllEventTriggers[ EventTriggerNumber ].TargetActionLabel = 
	    ReadAndMallocStringFromData ( EventPointer , TRIGGER_WHICH_TARGET_LABEL , "\"" ) ;

	ReadValueFromStringWithDefault( EventPointer , EVENT_TRIGGER_ENABLED_STRING , "%d" , "1",
			     &AllEventTriggers[ EventTriggerNumber ].enabled , EndOfEvent );
	
	EventTriggerNumber++;
	        EndOfEvent[strlen(EVENT_TRIGGER_END_STRING)-1] = '\0';

	EndOfEvent[strlen(EVENT_TRIGGER_END_STRING)-1] = s;
    } // While Event trigger begin string found...
    

}; // void decode_all_event_triggers ( char* EventSectionPointer )

/**
 * This function reads in the game events, i.e. the locations and conditions
 * under which some actions are triggered.
 */
void GetEventsAndEventTriggers ( const char* EventsAndEventTriggersFilename )
{
  char* EventSectionPointer;
char fpath[2048];

  //--------------------
  // At first we clear out any garbage that might randomly reside in the current event
  // and action structures...
  //
  clear_out_all_events_and_actions();

  //--------------------
  // Now its time to start loading the event file...
  //
  find_file (EventsAndEventTriggersFilename , MAP_DIR , fpath, 0 );
  EventSectionPointer = 
    ReadAndMallocAndTerminateFile( fpath , 
				   "*** END OF EVENT ACTION AND EVENT TRIGGER FILE *** LEAVE THIS TERMINATOR IN HERE ***" 
				   ) ;

  //--------------------
  // At first we decode ALL THE EVENT ACTIONS not the TRIGGERS!!!!
  //
  decode_all_event_actions ( EventSectionPointer );

  //--------------------
  // Now we decode ALL THE EVENT TRIGGERS not the ACTIONS!!!!
  //
  decode_all_event_triggers ( EventSectionPointer );

  free ( EventSectionPointer ) ;
}; // void Get_Game_Events ( char* EventSectionPointer );

/**
 * 
 */
void ExecuteEvent ( int EventNumber )
{
    obstacle* our_obstacle;
    int obstacle_level_num ;
    Level obstacle_level ;
    
    DebugPrintf( 1 , "\nvoid ExecuteEvent ( int EventNumber ) : executing event Nr.: %d." , EventNumber );

    // Do nothing in case of the empty action (-1) given.
    if ( EventNumber == (-1) ) return;
    
    //--------------------
    // Maybe the action will cause a map obstacle to change type.  If that is so, we
    // do it here...
    //
    if ( strlen ( AllTriggeredActions [ EventNumber ] . modify_obstacle_with_label ) > 0 )
    {
	our_obstacle = give_pointer_to_obstacle_with_label ( AllTriggeredActions [ EventNumber ] . modify_obstacle_with_label ) ;
	obstacle_level_num = give_level_of_obstacle_with_label ( AllTriggeredActions [ EventNumber ] . modify_obstacle_with_label ) ;
	obstacle_level = curShip . AllLevels [ obstacle_level_num ] ;
	if ( AllTriggeredActions [ EventNumber ] . modify_obstacle_to_type ) {
	    int j;
	    int base  = obstacle_level -> obstacle_statelist_base  [ our_obstacle -> name_index ] ;
	    int count = obstacle_level -> obstacle_statelist_count [ our_obstacle -> name_index ] ;
	    for (j=0; j<count; j++)
		if ( !strcmp ( obstacle_level->obstacle_states_names [ j+base ],
			       AllTriggeredActions [ EventNumber ] . modify_obstacle_to_type) ) {
		    our_obstacle -> type = obstacle_level->obstacle_states_values [ j+base ];
		    break;
		}
	} else {
		action_remove_obstacle (  curShip . AllLevels [ obstacle_level_num ], our_obstacle);
	     }

	//--------------------
	// Now we make sure the door lists and that are all updated...
	//
	
	GetAllAnimatedMapTiles ( curShip . AllLevels [ obstacle_level_num ] ) ;
	//--------------------
	// Also make sure the other maps realize the change too, if it
	// maybe happend in the border area where two maps are glued together
	// only export if the obstacle falls within the interface zone
	
	if( our_obstacle->pos.x <= obstacle_level->jump_threshold_west ||
	    our_obstacle->pos.x >= obstacle_level->xlen - obstacle_level->jump_threshold_east ||
	    our_obstacle->pos.y <= obstacle_level->jump_threshold_north ||
	    our_obstacle->pos.y >= obstacle_level->ylen - obstacle_level->jump_threshold_south
	    ) ExportLevelInterface ( obstacle_level_num ) ;
	
    }

    // Does the action include a teleport of the influencer to some other location?
    if ( AllTriggeredActions [ EventNumber ] . TeleportTarget . x != (-1) )
    {
	DebugPrintf( 1 , "\nvoid ExecuteEvent: Now a teleportation should occur!" );
	Teleport ( AllTriggeredActions[ EventNumber ].TeleportTargetLevel ,
		   AllTriggeredActions[ EventNumber ].TeleportTarget.x + 0.5 ,
		   AllTriggeredActions[ EventNumber ].TeleportTarget.y + 0.5 ,
		   TRUE );
    }
   
    // Does the action show text on user screen?
    if ( strlen ( AllTriggeredActions [ EventNumber ] . show_big_screen_message ) > 0 ){
        SetNewBigScreenMessage ( AllTriggeredActions [ EventNumber ] . show_big_screen_message );
	append_new_game_message ( AllTriggeredActions [ EventNumber ] . show_big_screen_message );
    }

    // Does the defined action change another action trigger?
    if ( strlen ( AllTriggeredActions[ EventNumber ] . modify_event_trigger_with_action_label ) > 0 )
	{
	//look for the target event trigger
	event_trigger * target_event = NULL;
	int i;
        for ( i = 0 ; i < MAX_EVENT_TRIGGERS ; i++ )
		{
		if ( ! strcmp (AllEventTriggers[i].TargetActionLabel ,  AllTriggeredActions[ EventNumber ] . modify_event_trigger_with_action_label ) )
			target_event = &AllEventTriggers[i];
		}
	
	//Shall we disable the event trigger?
	if ( AllTriggeredActions[ EventNumber ] . modify_event_trigger_value == 0 )
		{
		target_event -> enabled = 0;
		}

	//Shall we enable the event trigger?
	if ( AllTriggeredActions[ EventNumber ] . modify_event_trigger_value == 1 )
		{
		target_event -> enabled = 1;
		}
	}

   // Does the defined action run another action ?
   if ( strlen ( AllTriggeredActions [ EventNumber ] . also_execute_action_label ) > 0 )
	{
	ExecuteActionWithLabel ( AllTriggeredActions [ EventNumber ] . also_execute_action_label );
	}
}; // void ExecuteEvent ( int EventNumber )

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
	// if ( AllEventTriggers[i].EventNumber == (-1) ) continue;  // thats a sure sign this event doesn't need attention
	if ( strcmp (AllEventTriggers[i].TargetActionLabel , "none" ) == 0 ) continue;  // thats a sure sign this event doesn't need attention
	if ( AllEventTriggers[i].enabled == 0 ) continue;  // this trigger is not enabled
	
	// --------------------
	// So at this point we know, that the event trigger is somehow meaningful. 
	// Fine, so lets check the details, if the event is triggered now
	//
	
	if ( AllEventTriggers[i].Influ_Must_Be_At_Point.x != (-1) )
	{
	    if ( rintf( AllEventTriggers[i].Influ_Must_Be_At_Point.x ) != (int) ( Me . pos.x ) ) continue;
	}
	
	if ( AllEventTriggers[i].Influ_Must_Be_At_Point.y != (-1) )
	{
	    if ( rintf( AllEventTriggers[i].Influ_Must_Be_At_Point.y ) != (int) ( Me . pos.y ) ) continue;
	}
	
	if ( AllEventTriggers[i].Influ_Must_Be_At_Level != (-1) )
	{
	    if ( rintf( AllEventTriggers[i].Influ_Must_Be_At_Level ) != StatementLevel->levelnum ) continue;
	}
	
	DebugPrintf( 2 , "\nWARNING!! INFLU NOW IS AT SOME TRIGGER POINT OF SOME LOCATION-TRIGGERED EVENT!!!");
	// ExecuteEvent( AllEventTriggers[i].EventNumber );
	ExecuteActionWithLabel ( AllEventTriggers [ i ] . TargetActionLabel ) ;
	
	if ( AllEventTriggers[i].DeleteTriggerAfterExecution == 1 )
	{
	    // AllEventTriggers[i].EventNumber = (-1); // That should prevent the event from being executed again.
	    AllEventTriggers[i].TargetActionLabel = "none"; // That should prevent the event from being executed again.
	}
    }

}; // CheckForTriggeredEventsAndStatements (void )


/**
 * Since numbers are not so very telling and can easily get confusing
 * we do not use numbers to reference the action from a trigger but 
 * rather we use labels already in the mission file.  However internally
 * the game needs numbers as a pointer or index in a list and therefore
 * this functions was added to go from a label to the corresponding 
 * number entry.
 */
int GiveNumberToThisActionLabel ( char* ActionLabel )
{
  int i;

  // DebugPrintf( 1 , "\nvoid ExecuteEvent ( int EventNumber ) : real function call confirmed. ");
  // DebugPrintf( 1 , "\nvoid ExecuteEvent ( int EventNumber ) : executing event labeld : %s" , ActionLabel );

  // In case of 'none' as action label, we don't do anything and return;
  if ( strcmp ( ActionLabel , "none" ) == 0 ) return ( -1 );

  //--------------------
  // Now we find out which index the desired action has
  //
  for ( i = 0 ; i < MAX_TRIGGERED_ACTIONS_IN_GAME ; i++ )
    {
      if ( strcmp ( AllTriggeredActions[ i ].ActionLabel , ActionLabel ) == 0 ) break;
    }

  if ( i >= MAX_TRIGGERED_ACTIONS_IN_GAME )
    {
      fprintf( stderr, "\n\nActionLabel: '%s'\n" , ActionLabel );
      ErrorMessage ( __FUNCTION__  , "\
The label that should reference an action for later execution could not\n\
be identified as valid reference to an existing action.",
				 PLEASE_INFORM, IS_FATAL );
    }

  return ( i );
}; // int GiveNumberToThisActionLabel ( char* ActionLabel )

/**
 * This function executes an action with a label.
 */
void ExecuteActionWithLabel ( char* ActionLabel )
{
    ExecuteEvent( GiveNumberToThisActionLabel ( ActionLabel ) );
}; // void ExecuteActionWithLabel ( char* ActionLabel )

/**
 *
 *
 */
int teleporter_square_below_mouse_cursor ( char* ItemDescText )
{
    finepoint MapPositionOfMouse;
    int i;
    int action_number;
    
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
	    
	    // DebugPrintf ( -1000 , "\nSome trigger seems to be here..." );
	    
	    //--------------------
	    // Now we know, that the mouse is currently exactly over an event trigger.  The
	    // question to be answered still is whether this trigger also triggers a teleporter
	    // action or not and if yes, where the connection leads to...
	    //
	    action_number = GiveNumberToThisActionLabel ( AllEventTriggers [ i ] . TargetActionLabel ) ;
	    
	    if ( action_number == -1 ) return FALSE ;
	    
	    if ( AllTriggeredActions [ action_number ] . TeleportTargetLevel != (-1) )
	    {
		sprintf ( ItemDescText , _("To %s....") , D_(curShip . AllLevels [ AllTriggeredActions [ action_number ] . TeleportTargetLevel ] -> Levelname) ) ;
		return ( TRUE );
	    }
	}
    }
    return ( FALSE );
}; // void teleporter_square_below_mouse_cursor ( char* ItemDescText )


