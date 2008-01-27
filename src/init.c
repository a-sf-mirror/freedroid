/* 
 *
 *   Copyright (c) 2004-2007 Arthur Huillet
 *   Copyright (c) 1994, 2002, 2003 Johannes Prix
 *   Copyright (c) 1994, 2002 Reinhard Prix
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

/* ----------------------------------------------------------------------
 * (Not all of the) Initialisation routines for FreeDroid.
 * ---------------------------------------------------------------------- */

#define _init_c

#include "system.h"

#include "defs.h"
#include "struct.h"
#include "global.h"
#include "proto.h"

#ifdef __OpenBSD__
#include "ieeefp.h"
#else
#include "fenv.h"
#endif

#include "getopt.h"

void Init_Game_Data(void);
void Get_Bullet_Data ( char* DataPointer );


/* ----------------------------------------------------------------------
 * The following function is NOT 'standard' C but rather a GNU extention
 * to the C standards.  We *DON'T* want to use such things, but in this
 * case it helps debugging purposes of floating point operations just so
 * much, that it's really worth using it (in development versions, not in
 * releases).  But to avoid warnings from GCC (which we always set to not
 * allow gnu extentions to the C standard by default), we declare the
 * prototype of this function here.  If you don't use GCC or this 
 * function should give you portability problems, it's ABSOLUTELY SAFE
 * to just remove all instances of it, since it really only helps 
 * debugging.  Proper documentation can be found in the GNU C Library,
 * section about 'Arithmethic', subsection on floating point control
 * functions.
 * ---------------------------------------------------------------------- */
#if ! defined __gnu_linux__
/* turn off these functions where they are not present */
#define feenableexcept(X) {}
#define fedisableexcept(X) {}
#else
extern int feenableexcept (int excepts);
extern int fedisableexcept (int TheExceptionFlags );
#endif

/* ----------------------------------------------------------------------
 *
 *
 * ---------------------------------------------------------------------- */
static void
clear_out_arrays_for_fresh_game ( void )
{
    int i;

    level_editor_marked_obstacle = NULL ;
    for ( i = 0 ; i < MAXBULLETS ; i++ )
    {
	AllBullets [ i ] . Surfaces_were_generated = FALSE;
	DeleteBullet ( i , FALSE );
    }
    for ( i = 0 ; i < MAXBLASTS ; i++ )
    {
	DeleteBlast ( i );
    }
    for ( i = 0 ; i < MAX_ACTIVE_SPELLS ; i++ )
    {
	DeleteSpell ( i );
    }
    ClearEnemys ( ) ;
    clear_active_spells ( ) ;
    ClearAutomapData( );

    for ( i = 0 ; i < MAX_PERSONS ; i++)
	Me . chat_character_initialized [ i ] = 0;

}; // void clear_out_arrays_for_fresh_game ( void )

/* ---------------------------------------------------------------------- 
 * Each character inside the game can have a (lengthly) description of
 * his appearance and the like.  This is somewhat different from the mere
 * description of the character model, since a character model may be 
 * used for several characters inside the game.  Therefore the 
 * descriptions of (town) characters are linked with their dialog section
 * cause those are more unique.  At startup, for every dialog section
 * we set up a character description.  Initializing those needs to be
 * done only once.  We do it here.
 * ---------------------------------------------------------------------- */
static void
init_character_descriptions ( void )
{
    int i;

    for ( i = 0 ; i < MAX_PERSONS ; i ++ )
    {
	character_descriptions [ i ] = _("My name is Bug. Software Bug. Please report me to the developers. Thank you.");
    }

    character_descriptions [ PERSON_CHA ] = _("Hmm... This must be the token town sage.");

    character_descriptions [ PERSON_SORENSON ] = _("You see a legend. Sorenson Clark. One of the best human programmers that have ever lived. Just like Paganini she is said to have sold her soul to the devil for her skill. Whatever is the truth, her code is like magic, and there is no denying that.");

    character_descriptions [ PERSON_KEVIN ] = _("He is either a college student, a drug addict or a hacker. I bet he did not have a warm meal for days, he looks totally malnourished. His body keeps twitching. Must be the caffeine. I hope.");

    character_descriptions [ PERSON_BENDER ] = _("Don't do steroids or you will be just like him.");

    character_descriptions [ PERSON_614 ] = _("There are at least 614 reasons to stay away from the 614 bot battle platform. There are also 614 reasons why a 614 bot battle platform can be your best friend during combat...");

    character_descriptions [ PERSON_FRANCIS ] = _("Francis is not looking so good today. His tattered coat is not looking that much better either. The war is taking its toll on them both.");

    character_descriptions [ PERSON_DUNCAN ] = _("What a strange person. He does not seem to really fit into this reality. There is something very eerie about him. And frightening.");

    character_descriptions [ PERSON_BRUCE ] = _("This person looks totally defeated. Tattered clothes surround his skeletal form. If there was a fifth raider of the apocalypse, it would be him and his name would be Hard Work.");

    character_descriptions [ PERSON_SKIPPY ] = _("It seems that even during the end of the world the species known as 'Salesman' is able to survive. Just his presence makes you want to kill all humans.");
    
    character_descriptions [ PERSON_EWALD ] = _("He looks unexplicably happy.");

    character_descriptions [ PERSON_STONE ] = _("In some places of the world there exists a very interesting saying. 'Tough as nails.' However, in every place where the Stone trader family stays for a while, a new saying emerges. 'Tough as the Stones.'");

    character_descriptions [ PERSON_DIXON ] = _("It is very obvious that this is not a normal Red Guard. Most of them do not have oil stains on their armor and smell of alcohol.");

    character_descriptions [ PERSON_KEVINS_GUARD ] = _("It looks like a 614. It moves like a 614. It even kills like a 614. Must be a 614 then.");

    character_descriptions [ PERSON_RMS ] = _("He must be the resident scientist. The only thing keeping his body alive is the massive coffee overdose that he takes early in the morning every day. The amount of cafeine that he has in his blood right now would be enough to kill ten humans, but since computer scientists are a different species, he is unharmed.");

    character_descriptions [ PERSON_DARWIN ] = _("Something about him makes you think of angels. Except angels are usually sane, do not have an urge to kill everything around them and are much better at singing religious songs.");

    character_descriptions [ PERSON_MELFIS ] = _("As you look at him, you cannot really belive he is there. If you turn your back on him he will vanish like a ghost into thin air and you will never see him again.");

    character_descriptions [ PERSON_TYBALT ] = _("His IQ is sometimes as high as a dog's. Only sometimes though. He smells like a dog too.");

    character_descriptions [ PERSON_PENDRAGON ] = _("He seems to be the chief of the gate guards. Hard to say anything more, the armor hides his body.");

    character_descriptions [ PERSON_BENJAMIN ] = _("Just an another Red Guard. The armor hides his body.");

    character_descriptions [ PERSON_DOC_MOORE ] = _("Some people say that every doctor is addicted to drugs. You estimate that right now this one is high on at least four different narcotics. Doc Moore seems very happy indeed...");

    character_descriptions [ PERSON_BUTCH ] = _("Yet another Red Guard. You cannot say anything more, the armor hides his body.");

    character_descriptions [ PERSON_SPENCER ] = _("He looks taller than most of the Red Guards, but you cannot really say anything more about him, the armor hides his body.");

    character_descriptions [ PERSON_MICHELANGELO ] = _("And now you know who is to be blamed for the truly awful food in here. You are in a killing mood. Oh yeah. The cook must die. The cook must die. Ha. Ha. Ha.");

    character_descriptions [ PERSON_SUBDIALOG_DUMMY ] = _("This is a major bug. Please report this incident to the developers. Thank you.");

    character_descriptions [ PERSON_STANDARD_BOT_AFTER_TAKEOVER ] = _("Blood. Stone. Cog. The Blood has made the Cog from the Stone. Now the Cog is making more Stone from the Blood. I wonder what is the next step of the cycle...");

    character_descriptions [ PERSON_STANDARD_OLD_TOWN_GATE_GUARD ] = _("The armor hides his body.");

    character_descriptions [ PERSON_OLD_TOWN_GATE_GUARD_LEADER ] = _("The armor hides his body.");

    character_descriptions [ PERSON_STANDARD_MS_FACILITY_GATE_GUARD ] = _("The armor hides his body.");

    character_descriptions [ PERSON_MS_FACILITY_GATE_GUARD_LEADER ] = _("The armor hides his body.");

}; // void init_character_descriptions ( void )

/* ---------------------------------------------------------------------- 
 * This function displays a startup status bar that shows a certain
 * percentage of loading done.
 * ---------------------------------------------------------------------- */
void 
ShowStartupPercentage ( int Percentage )
{
    SDL_Rect Bar_Rect;
    Uint32 FillColor = SDL_MapRGB( Screen->format, 150 , 200 , 225 ) ; 
    
#if __WIN32__
    if ( use_open_gl)
	blit_special_background ( FREEDROID_LOADING_PICTURE_CODE );
#endif
    
    Bar_Rect . x = 200 * GameConfig . screen_width / 640 ;
    Bar_Rect . y = 200 * GameConfig . screen_height / 480 ;
    Bar_Rect . w = 2 * Percentage * GameConfig . screen_width / 640 ;
    Bar_Rect . h = 30 * GameConfig . screen_height / 480 ;
    our_SDL_fill_rect_wrapper ( Screen , & Bar_Rect , FillColor ) ;
    
    Bar_Rect . x = ( 200 + 2 * Percentage ) * GameConfig . screen_width / 640 ;
    Bar_Rect . y = 200 * GameConfig . screen_height / 480 ;
    Bar_Rect . w = ( 200 - 2 * Percentage ) * GameConfig . screen_width / 640 ;
    Bar_Rect . h = 30 * GameConfig . screen_height / 480 ;
    our_SDL_fill_rect_wrapper ( Screen , & Bar_Rect , 0 ) ;
    
    SDL_SetClipRect( Screen , NULL );
    
    switch ( GameConfig . screen_width )
    {
	case 640:
	    PrintString ( Screen , ( 290 ) * GameConfig . screen_width / 640 , 
			  ( 205 ) * GameConfig . screen_height / 480 , "%d%%", Percentage ) ;
	    break;
	case 720: 
	    PrintString ( Screen , ( 290 ) * GameConfig . screen_width / 640 + 3, 
			  ( 205 ) * GameConfig . screen_height / 480 + 2, "%d%%", Percentage ) ;
	    break;
	case 800:
	    PrintString ( Screen , ( 290 ) * GameConfig . screen_width / 640 + 5 , 
			  ( 205 ) * GameConfig . screen_height / 480 + 4 , "%d%%", Percentage ) ;
	    break;
	case 1024:
	    PrintString ( Screen , ( 290 ) * GameConfig . screen_width / 640 + 8 , 
			  ( 205 ) * GameConfig . screen_height / 480 + 7 , "%d%%", Percentage ) ;
	    break;
	case 1280:
	    PrintString ( Screen , ( 290 ) * GameConfig . screen_width / 640 + 11 , 
			  ( 205 ) * GameConfig . screen_height / 480 + 10 , "%d%%", Percentage ) ;
	    break;
	default:
	    ErrorMessage ( __FUNCTION__  , "\
The resolution found is none of\n\
     0 = 640 x 480 (default with SDL)\n\
     1 = 800 x 600 (default with OpenGL)\n\
     2 = 1024 x 748 \n\
     3 = 720 x 480 (NTSC) \n\
     4 = 720 x 576 (PAL) \n\
This means a severe bug...",
						       PLEASE_INFORM , IS_FATAL );
	    break;
    }

    our_SDL_update_rect_wrapper ( Screen , 200 , 200 , 200 , 30  ) ;
    
    DebugPrintf ( 1 , "\nNow at percentage: %d." , Percentage );
    
}; // void ShowStartupPercentage ( int Percentage )

/* ----------------------------------------------------------------------
 * This function can be used to play a generic title file, containing 
 * 
 *  1. a background picture name
 *  2. a background music to play
 *  3. some text to display in a scrolling fashion
 *
 * ---------------------------------------------------------------------- */
void
PlayATitleFile ( char* Filename )
{
char fpath[2048];
    char* TitleFilePointer;
    char* NextSubsectionStartPointer;
    char* PreparedBriefingText;
    char* TerminationPointer;
    char* TitleSongName;
    int ThisTextLength;
    extern char * language_dirs[];
    char finaldir[50];
    while ( SpacePressed() || MouseLeftPressed());

    snprintf(finaldir, 50, "%s%s", TITLES_DIR,  language_dirs[GameConfig.language]);
    //--------------------
    // Now its time to start loading the title file...
    //
    find_file (Filename , finaldir , fpath, 0 );
    TitleFilePointer = 
	ReadAndMallocAndTerminateFile( fpath , "*** END OF TITLE FILE *** LEAVE THIS TERMINATOR IN HERE ***" ) ;
    
    TitleSongName = ReadAndMallocStringFromData ( TitleFilePointer, "The title song in the sound subdirectory for this mission is : " , "\n" ) ;
    
    SwitchBackgroundMusicTo ( TitleSongName );
    free ( TitleSongName );
    
    SDL_SetClipRect ( Screen, NULL );
    Me.status=BRIEFING;
    SetCurrentFont( Para_BFont );
    
    NextSubsectionStartPointer = TitleFilePointer;
    while ( ( NextSubsectionStartPointer = strstr ( NextSubsectionStartPointer, "*** START OF PURE SCROLLTEXT DATA ***")) 
	    != NULL )
    {
	NextSubsectionStartPointer += strlen ( "*** START OF PURE SCROLLTEXT DATA ***" );
	if ( (TerminationPointer=strstr ( NextSubsectionStartPointer, "*** END OF PURE SCROLLTEXT DATA ***")) == NULL)
	{
	    DebugPrintf (1, "\n\nvoid PlayATitleFile(...): Unterminated Subsection in Mission briefing....Terminating...");
	    Terminate(ERR);
	}
	ThisTextLength=TerminationPointer-NextSubsectionStartPointer;
	PreparedBriefingText = MyMalloc (ThisTextLength + 10);
	strncpy ( PreparedBriefingText , NextSubsectionStartPointer , ThisTextLength );
	PreparedBriefingText[ThisTextLength]=0;
	fflush(stdout);
	ScrollText ( PreparedBriefingText, SCROLLSTARTX, SCROLLSTARTY, NE_TITLE_PIC_BACKGROUND_CODE );
	free ( PreparedBriefingText );
    }
    
    ClearGraphMem ();
    our_SDL_flip_wrapper( Screen );
   free ( TitleFilePointer ) ;
  
}; // void PlayATitleFile ( char* Filename )

/*----------------------------------------------------------------------
 * This function reads in all the bullet data from the freedroid.ruleset file,
 * but IT DOES NOT LOAD THE FILE, IT ASSUMES IT IS ALREADY LOADED and
 * it only receives a pointer to the start of the bullet section from
 * the calling function.
 ----------------------------------------------------------------------*/
void 
Get_Bullet_Data ( char* DataPointer )
{
    char *BulletPointer;
    char *EndOfBulletData;
    int i;
    int BulletIndex=0;
    
#define BULLET_SECTION_BEGIN_STRING "*** Start of Bullet Data Section: ***" 
#define BULLET_SECTION_END_STRING "*** End of Bullet Data Section: ***" 
#define NEW_BULLET_TYPE_BEGIN_STRING "** Start of new bullet specification subsection **"

#define BULLET_RECHARGE_TIME_BEGIN_STRING "Time is takes to recharge this bullet/weapon in seconds :"
#define BULLET_SPEED_BEGIN_STRING "Flying speed of this bullet type :"
#define BULLET_DAMAGE_BEGIN_STRING "Damage cause by a hit of this bullet type :"
#define BULLET_ONE_SHOT_ONLY_AT_A_TIME "Cannot fire until previous bullet has been deleted : "
#define BULLET_BLAST_TYPE_CAUSED_BEGIN_STRING "Type of blast this bullet causes when crashing e.g. against a wall :"

    BulletPointer = LocateStringInData ( DataPointer , BULLET_SECTION_BEGIN_STRING );
    EndOfBulletData = LocateStringInData ( DataPointer , BULLET_SECTION_END_STRING );
    
    DebugPrintf ( 1 , "\n\nStarting to read bullet data...\n\n");
    //--------------------
    // At first, we must allocate memory for the droid specifications.
    // How much?  That depends on the number of droids defined in freedroid.ruleset.
    // So we have to count those first.  ok.  lets do it.
    
    Number_Of_Bullet_Types = CountStringOccurences ( DataPointer , NEW_BULLET_TYPE_BEGIN_STRING ) ;
    
    // Not that we know how many bullets are defined in freedroid.ruleset, we can allocate
    // a fitting amount of memory, but of course only if the memory hasn't been allocated
    // aready!!!
    //
    // If we would do that in any case, every Init_Game_Data call would destroy the loaded
    // image files AND MOST LIKELY CAUSE A SEGFAULT!!!
    //
    if ( Bulletmap == NULL )
    {
	i = sizeof ( bulletspec ) ;
	Bulletmap = MyMalloc ( i * ( Number_Of_Bullet_Types + 1 ) + 1 );
	DebugPrintf ( 1 , "\nvoid Get_Bullet_Data( char* DatapPointer ) : We have counted %d different bullet types in the game data file." , Number_Of_Bullet_Types );
	// DebugPrintf ( 0 , "\nMEMORY HAS BEEN ALLOCATED.\nTHE READING CAN BEGIN.\n" );
	// getchar();
    }
    
    //--------------------
    // Now we start to read the values for each bullet type:
    // 
    BulletPointer=DataPointer;
    
    while ( (BulletPointer = strstr ( BulletPointer, NEW_BULLET_TYPE_BEGIN_STRING )) != NULL)
    {
	DebugPrintf (1, "\n\nFound another Bullet specification entry!  Lets add that to the others!");
	BulletPointer ++; // to avoid doubly taking this entry
	Bulletmap[BulletIndex].phases = 1;
	Bulletmap[BulletIndex].phase_changes_per_second = 1;
	BulletIndex++;
    }
    
    DebugPrintf (1, "\nEnd of Get_Bullet_Data ( char* DataPointer ) reached.");
} // void Get_Bullet_Data ( char* DataPointer );

/* ----------------------------------------------------------------------
 * Delete all events and event triggers
 * ---------------------------------------------------------------------- */
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
	AllTriggeredActions[i].modify_obstacle_to_type=-1;
	AllTriggeredActions[i].modify_event_trigger_with_action_label="";
	AllTriggeredActions[i].modify_event_trigger_value=-1;
	
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
#define MODIFY_OBSTACLE_TO_TYPE_STRING "modify_obstacle_to_type="

/* ----------------------------------------------------------------------
 *
 *
 * ---------------------------------------------------------------------- */
static void
decode_all_event_actions ( char* EventSectionPointer )
{
    char *EventPointer;
    char *EndOfEvent;
    int EventActionNumber;
    char* TempMapLabelName;
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
	    ReadValueFromString( EventPointer , MODIFY_OBSTACLE_TO_TYPE_STRING , "%d" , 
		    & ( AllTriggeredActions[ EventActionNumber ] . modify_obstacle_to_type ) , EndOfEvent );
	    DebugPrintf ( 1 , "\nObstacle will be modified to type: %d." , AllTriggeredActions[ EventActionNumber ] . modify_obstacle_to_type );
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

/* ---------------------------------------------------------------------- 
 *
 *
 * ---------------------------------------------------------------------- */
static void
decode_all_event_triggers ( char* EventSectionPointer )
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

/* ----------------------------------------------------------------------
 * This function reads in the game events, i.e. the locations and conditions
 * under which some actions are triggered.
 * ---------------------------------------------------------------------- */
void 
GetEventsAndEventTriggers ( const char* EventsAndEventTriggersFilename )
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


/* ----------------------------------------------------------------------
 * This function reads the descriptions of the different programs
 * (source and blobs) that are used for magic
 * ---------------------------------------------------------------------- */
static int Get_Programs_Data ( char * DataPointer )
{
    char *ProgramPointer;
    char *EndOfProgramData;

#define PROGRAM_SECTION_BEGIN_STRING "*** Start of program data section: ***"
#define PROGRAM_SECTION_END_STRING "*** End of program data section ***"
#define NEW_PROGRAM_BEGIN_STRING "** Start of new program specification subsection **"
    
    ProgramPointer = LocateStringInData ( DataPointer , PROGRAM_SECTION_BEGIN_STRING );
    EndOfProgramData = LocateStringInData ( DataPointer , PROGRAM_SECTION_END_STRING );
            
    int Number_Of_Programs = CountStringOccurences ( DataPointer , NEW_PROGRAM_BEGIN_STRING ) ;
    number_of_skills = Number_Of_Programs;            

    SpellSkillMap = (spell_skill_spec *) MyMalloc( sizeof(spell_skill_spec) * (Number_Of_Programs + 1 ));

    if ( Number_Of_Programs >= MAX_NUMBER_OF_PROGRAMS ) 
	{
	ErrorMessage ( __FUNCTION__  , "\
There are more skills defined, than the maximum number specified in the code!",
                                   PLEASE_INFORM, IS_FATAL );
	}

    char * whattogrep = NEW_PROGRAM_BEGIN_STRING;
    
    spell_skill_spec * ProgramToFill = SpellSkillMap;

    while ( (ProgramPointer = strstr ( ProgramPointer, whattogrep )) != NULL)
            {
            ProgramPointer ++;
            char * EndOfThisProgram = strstr ( ProgramPointer, whattogrep );

	    if ( EndOfThisProgram ) EndOfThisProgram [ 0 ] = 0;

	    ProgramToFill -> name = ReadAndMallocStringFromData ( ProgramPointer , "Program name=_\"" , "\"" ) ;
	    ProgramToFill -> description = ReadAndMallocStringFromData ( ProgramPointer , "Program description=_\"" , "\"" ) ;
	    ProgramToFill -> icon_name = ReadAndMallocStringFromData ( ProgramPointer , "Picture=\"" , "\"" ) ;

	    ProgramToFill -> icon_surface . surface = NULL;
	    ProgramToFill -> icon_surface . zoomed_out_surface = NULL;
	    ProgramToFill -> icon_surface . texture_has_been_created = 0;
	    ProgramToFill -> icon_surface . texture_width = 0;

	    ProgramToFill -> effect = ReadAndMallocStringFromData ( ProgramPointer , "Effect=\"" , "\"" ) ;
	    
	    char * pform = ReadAndMallocStringFromData ( ProgramPointer , "Form=\"" , "\"" ) ;
	    if ( !strcmp(pform, "immediate") ) 
		ProgramToFill ->  form = PROGRAM_FORM_IMMEDIATE;
	    if ( !strcmp(pform, "bullet") ) 
		ProgramToFill ->  form = PROGRAM_FORM_BULLET;
	    if ( !strcmp(pform, "radial") ) 
		ProgramToFill ->  form = PROGRAM_FORM_RADIAL;
	    if ( !strcmp(pform, "self") ) 
		ProgramToFill ->  form = PROGRAM_FORM_SELF;

	    free ( pform );

            ReadValueFromStringWithDefault( ProgramPointer , "Base damage=" , "%hd" , "0",
                             & ProgramToFill -> damage_base  , EndOfProgramData );
            ReadValueFromStringWithDefault( ProgramPointer , "Mod damage=" , "%hd" , "0",
                             & ProgramToFill -> damage_mod  , EndOfProgramData );
            ReadValueFromStringWithDefault( ProgramPointer , "Damage per level=" , "%hd" , "0",
                             & ProgramToFill -> damage_per_level  , EndOfProgramData );

            ReadValueFromStringWithDefault( ProgramPointer , "Affect bots=" , "%hhd" , "1",
                             & ProgramToFill -> hurt_bots  , EndOfProgramData );
            ReadValueFromStringWithDefault( ProgramPointer , "Affect humans=" , "%hhd" , "1",
                             & ProgramToFill -> hurt_humans  , EndOfProgramData );


            ReadValueFromStringWithDefault( ProgramPointer , "Cost=" , "%hd" , "0",
                             & ProgramToFill -> heat_cost  , EndOfProgramData );
            ReadValueFromStringWithDefault( ProgramPointer , "Cost per level=" , "%hd" , "0",
                             & ProgramToFill -> heat_cost_per_level  , EndOfProgramData );
            ReadValueFromStringWithDefault( ProgramPointer , "Present at startup=" , "%hhd" , "0",
                             & ProgramToFill -> present_at_startup  , EndOfProgramData );
            ReadValueFromStringWithDefault( ProgramPointer , "Artwork internal code=" , "%hhd" , "-1",
                             & ProgramToFill -> graphics_code  , EndOfProgramData );
            //ReadValueFromStringWithDefault( ProgramPointer , "Bonus to tohit modifier=" , "%d" , "0",
	    
                             
            ProgramToFill ++;
            if ( EndOfThisProgram ) EndOfThisProgram [ 0 ] = '*'; // We put back the star at its place 
            }
                             
return 0;
}

/* ----------------------------------------------------------------------
 * This function loads all the constant concerning robot archetypes
 * from a section in memory to the actual archetype structures.
 * ---------------------------------------------------------------------- */
static void
Get_Robot_Data ( void* DataPointer )
{
  int RobotIndex = 0;
  char *RobotPointer;
  char *EndOfDataPointer;
  int i;

  double maxspeed_calibrator;
  double acceleration_calibrator;
  double maxenergy_calibrator;
  double aggression_calibrator;
  float energyloss_calibrator;
  double experience_reward_calibrator;
  double range_of_vision_calibrator;

#define MAXSPEED_CALIBRATOR_STRING "Common factor for all droids maxspeed values: "
#define ACCELERATION_CALIBRATOR_STRING "Common factor for all droids acceleration values: "
#define MAXENERGY_CALIBRATOR_STRING "Common factor for all droids maximum energy values: "
#define AGGRESSION_CALIBRATOR_STRING "Common factor for all droids aggression values: "
#define ENERGYLOSS_CALIBRATOR_STRING "Common factor for all droids energyloss values: "
#define EXPERIENCE_REWARD_CALIBRATOR_STRING "Common factor for all droids experience_reward values: "
#define RANGE_OF_VISION_CALIBRATOR_STRING "Common factor for all droids range of vision: "

#define ROBOT_SECTION_BEGIN_STRING "*** Start of Robot Data Section: ***" 
#define ROBOT_SECTION_END_STRING "*** End of Robot Data Section: ***" 
#define NEW_ROBOT_BEGIN_STRING "** Start of new Robot: **" 
#define DROIDNAME_BEGIN_STRING "Droidname: "
#define PORTRAIT_FILENAME_WITHOUT_EXT "Droid portrait file name (without extension) to use=\""

#define DROID_PORTRAIT_ROTATION_SERIES_NAME_PREFIX "Droid uses portrait rotation series with prefix=\""

#define MAXSPEED_BEGIN_STRING "Maximum speed of this droid: "
#define CLASS_BEGIN_STRING "Class of this droid: "
#define ACCELERATION_BEGIN_STRING "Maximum acceleration of this droid: "
#define MAXENERGY_BEGIN_STRING "Maximum energy of this droid: "
#define MAXMANA_BEGIN_STRING "Maximum mana of this droid: "
#define LOSEHEALTH_BEGIN_STRING "Rate of healing: "
#define AGGRESSION_BEGIN_STRING "Aggression rate of this droid: "
#define BASE_PHYSICAL_DAMAGE_BEGIN_STRING "Physical (base) damage an attack of this droid will do: "
#define FLASHIMMUNE_BEGIN_STRING "Is this droid immune to disruptor blasts? "
#define EXPERIENCE_REWARD_BEGIN_STRING "Experience_Reward gained for destroying one of this type: "
#define DRIVE_BEGIN_STRING "Drive of this droid : "
#define BRAIN_BEGIN_STRING "Brain of this droid : "
#define SENSOR1_BEGIN_STRING "Sensor 1 of this droid : "
#define SENSOR2_BEGIN_STRING "Sensor 2 of this droid : "
#define SENSOR3_BEGIN_STRING "Sensor 3 of this droid : "
#define ARMAMENT_BEGIN_STRING "Armament of this droid : "
#define DRIVE_ITEM_BEGIN_STRING "Drive item="
#define WEAPON_ITEM_BEGIN_STRING "Weapon item=\""
#define SHIELD_ITEM_BEGIN_STRING "Shield item="
#define ARMOUR_ITEM_BEGIN_STRING "Armour item="
#define SPECIAL_ITEM_BEGIN_STRING "Special item="
#define GREETING_SOUND_STRING "Greeting Sound number="
#define ENEMY_GOT_HIT_SOUND_STRING "Got Hit Sound number="
#define DROID_DEATH_SOUND_FILE_NAME "Death sound file name=\""
#define DROID_ATTACK_ANIMATION_SOUND_FILE_NAME "Attack animation sound file name=\""
#define TO_HIT_STRING "Chance of this robot scoring a hit="
#define GETTING_HIT_MODIFIER_STRING "Chance modifier, that this robot gets hit="
#define IS_HUMAN_SPECIFICATION_STRING "Is this 'droid' a human : "
#define INDIVIDUAL_SHAPE_SPECIFICATION_STRING "Individual shape of this droid or just -1 for classic ball shaped : "
#define NOTES_BEGIN_STRING "Notes concerning this droid=_\""

  
  RobotPointer = LocateStringInData ( DataPointer , ROBOT_SECTION_BEGIN_STRING );
  EndOfDataPointer = LocateStringInData ( DataPointer , ROBOT_SECTION_END_STRING );

  
  DebugPrintf (2, "\n\nStarting to read robot calibration section\n\n");

  // Now we read in the speed calibration factor for all droids
  ReadValueFromString( RobotPointer , MAXSPEED_CALIBRATOR_STRING , "%lf" , 
		       &maxspeed_calibrator , EndOfDataPointer );

  // Now we read in the acceleration calibration factor for all droids
  ReadValueFromString( RobotPointer , ACCELERATION_CALIBRATOR_STRING , "%lf" , 
		       &acceleration_calibrator , EndOfDataPointer );

  // Now we read in the maxenergy calibration factor for all droids
  ReadValueFromString( RobotPointer , MAXENERGY_CALIBRATOR_STRING , "%lf" , 
		       &maxenergy_calibrator , EndOfDataPointer );
  // Now we read in the energy_loss calibration factor for all droids
  ReadValueFromString( RobotPointer , ENERGYLOSS_CALIBRATOR_STRING , "%f" ,
	&energyloss_calibrator , EndOfDataPointer );

  // Now we read in the aggression calibration factor for all droids
  ReadValueFromString( RobotPointer , AGGRESSION_CALIBRATOR_STRING , "%lf" , 
		       &aggression_calibrator , EndOfDataPointer );

  // Now we read in the experience_reward calibration factor for all droids
  ReadValueFromString( RobotPointer , EXPERIENCE_REWARD_CALIBRATOR_STRING , "%lf" , 
		       &experience_reward_calibrator , EndOfDataPointer );

  // Now we read in the range of vision calibration factor for all droids
  ReadValueFromString( RobotPointer , RANGE_OF_VISION_CALIBRATOR_STRING , "%lf" , 
		       &range_of_vision_calibrator , EndOfDataPointer );

  DebugPrintf ( 1 , "\n\nStarting to read Robot data...\n\n" );
  //--------------------
  // At first, we must allocate memory for the droid specifications.
  // How much?  That depends on the number of droids defined in freedroid.ruleset.
  // So we have to count those first.  ok.  lets do it.

  Number_Of_Droid_Types = CountStringOccurences ( DataPointer , NEW_ROBOT_BEGIN_STRING ) ;

  // Not that we know how many robots are defined in freedroid.ruleset, we can allocate
  // a fitting amount of memory.
  i=sizeof(druidspec);
  Druidmap = MyMalloc ( i * (Number_Of_Droid_Types + 1) + 1 );
  DebugPrintf(1, "\nWe have counted %d different druid types in the game data file." , Number_Of_Droid_Types );
  DebugPrintf (2, "\nMEMORY HAS BEEN ALLOCATED.\nTHE READING CAN BEGIN.\n" );

  //--------------------
  //Now we start to read the values for each robot:
  //Of which parts is it composed, which stats does it have?
  while ( (RobotPointer = strstr ( RobotPointer, NEW_ROBOT_BEGIN_STRING )) != NULL)
    {
      DebugPrintf (2, "\n\nFound another Robot specification entry!  Lets add that to the others!");
      RobotPointer ++; // to avoid doubly taking this entry
      char * EndOfThisRobot = strstr ( RobotPointer, NEW_ROBOT_BEGIN_STRING );
      if ( EndOfThisRobot ) EndOfThisRobot [ 0 ] = 0;

      // Now we read in the Name of this droid.  We consider as a name the rest of the
      // line with the DROIDNAME_BEGIN_STRING until the "\n" is found.
      Druidmap[RobotIndex].druidname =
	ReadAndMallocStringFromData ( RobotPointer , DROIDNAME_BEGIN_STRING , "\n" ) ;

      //--------------------
      // Now we read in the prefix of the file names in the rotation series
      // to use for the console droid rotation
      Druidmap [ RobotIndex ] . droid_portrait_rotation_series_prefix =
	ReadAndMallocStringFromData ( RobotPointer , DROID_PORTRAIT_ROTATION_SERIES_NAME_PREFIX , "\"" ) ;

      //--------------------
      // Now we read in the file name of the death sound for this droid.  
      // Is should be enclosed in double-quotes.
      //
      Druidmap [ RobotIndex ] . droid_death_sound_file_name =
	ReadAndMallocStringFromData ( RobotPointer , DROID_DEATH_SOUND_FILE_NAME , "\"" ) ;

      //--------------------
      // Now we read in the file name of the attack animation sound for this droid.  
      // Is should be enclosed in double-quotes.
      //
      Druidmap [ RobotIndex ] . droid_attack_animation_sound_file_name =
	ReadAndMallocStringFromData ( RobotPointer , DROID_ATTACK_ANIMATION_SOUND_FILE_NAME , "\"" ) ;

      // Now we read in the maximal speed this droid can go. 
      ReadValueFromString( RobotPointer , MAXSPEED_BEGIN_STRING , "%f" , 
			   &Druidmap[RobotIndex].maxspeed , EndOfDataPointer );

      // Now we read in the class of this droid.
      ReadValueFromString( RobotPointer , CLASS_BEGIN_STRING , "%d" , 
			   &Druidmap[RobotIndex].class , EndOfDataPointer );

      // Now we read in the maximal acceleration this droid can go. 
      ReadValueFromString( RobotPointer , ACCELERATION_BEGIN_STRING , "%f" , 
			   &Druidmap[RobotIndex].accel , EndOfDataPointer );

      // Now we read in the maximal energy this droid can store. 
      ReadValueFromString( RobotPointer , MAXENERGY_BEGIN_STRING , "%f" , 
			   &Druidmap[RobotIndex].maxenergy , EndOfDataPointer );

      // Now we read in the maximal mana this droid can store. 
      ReadValueFromString( RobotPointer , MAXMANA_BEGIN_STRING , "%f" , 
			   &Druidmap[RobotIndex].max_temperature , EndOfDataPointer );

      // Now we read in the lose_health rate.
      ReadValueFromString( RobotPointer , LOSEHEALTH_BEGIN_STRING , "%f" , 
			   &Druidmap[RobotIndex].lose_health , EndOfDataPointer );

      // Now we read in the aggression rate of this droid.
      ReadValueFromString( RobotPointer , AGGRESSION_BEGIN_STRING , "%d" , 
			   &Druidmap[RobotIndex].aggression , EndOfDataPointer );

      // Now we read in the aggression rate of this droid.
      ReadValueFromString( RobotPointer , BASE_PHYSICAL_DAMAGE_BEGIN_STRING , "%f" , 
			   & Druidmap [ RobotIndex ] . physical_damage , EndOfDataPointer );

      // Now we read in range of vision of this droid
      ReadValueFromString( RobotPointer , "Range of vision of this droid=" , "%f" , 
			   &Druidmap[RobotIndex].range_of_vision , EndOfDataPointer );

      // Now we read in range of vision of this droid
      ReadValueFromString( RobotPointer , "Time spent eyeing Tux=" , "%f" , 
			   &Druidmap[RobotIndex].time_spent_eyeing_tux , EndOfDataPointer );

      // Now we read in range of vision of this droid
      ReadValueFromString( RobotPointer , "Minimal distance hostile bots are tolerated=" , "%f" , 
			   &Druidmap[RobotIndex].minimal_range_hostile_bots_are_ignored , EndOfDataPointer );

      // Now we read in the flash immunity of this droid.
      ReadValueFromStringWithDefault( RobotPointer , FLASHIMMUNE_BEGIN_STRING , "%hhd" , "0",
			   &Druidmap[RobotIndex].flashimmune , EndOfDataPointer );

      // Now we experience_reward to be had for destroying one droid of this type
      ReadValueFromString( RobotPointer , EXPERIENCE_REWARD_BEGIN_STRING , "%hd" , 
			   &Druidmap[RobotIndex].experience_reward, EndOfDataPointer );

      // Now we read in the monster level = maximum treasure chest to pick from
      ReadValueFromString( RobotPointer , "Drops item class=" , "%d" , 
			   &Druidmap[RobotIndex].monster_level , EndOfDataPointer );

      // Now we read in the number of additional magical items this monster type must drop
      ReadValueFromStringWithDefault( RobotPointer , "Force how many additional magic items to be dropped=" , "%d" , "0" ,
			   &Druidmap[RobotIndex].forced_magic_items , EndOfDataPointer );

      // Now we read in the brain of this droid of this type
      ReadValueFromStringWithDefault( RobotPointer , BRAIN_BEGIN_STRING , "%d" , "1",
			   &Druidmap[RobotIndex].brain, EndOfDataPointer );

      // Now we read in the sensor 1, 2 and 3 of this droid type
      ReadValueFromStringWithDefault( RobotPointer , SENSOR1_BEGIN_STRING , "%d" , "1",
			   &Druidmap[RobotIndex].sensor1, EndOfDataPointer );
      ReadValueFromStringWithDefault( RobotPointer , SENSOR2_BEGIN_STRING , "%d" , "5",
			   &Druidmap[RobotIndex].sensor2, EndOfDataPointer );
      ReadValueFromStringWithDefault( RobotPointer , SENSOR3_BEGIN_STRING , "%d" , "0",
			   &Druidmap[RobotIndex].sensor3, EndOfDataPointer );

      char * tmp_item_name = ReadAndMallocStringFromData ( RobotPointer , WEAPON_ITEM_BEGIN_STRING , "\"" ) ;
      Druidmap[RobotIndex].weapon_item.type = GetItemIndexByName ( tmp_item_name );
      free ( tmp_item_name );

      // Now we read in the number of plasma transistors
      ReadValueFromStringWithDefault( RobotPointer , "Number of Plasma Transistors=" , "%hhd" , "0",
			   &Druidmap[RobotIndex].amount_of_plasma_transistors , EndOfDataPointer );

      // Now we read in the number of plasma transistors
      ReadValueFromStringWithDefault( RobotPointer , "Number of Superconductors=" , "%hhd" , "0", 
			   &Druidmap[RobotIndex].amount_of_superconductors , EndOfDataPointer );

      // Now we read in the number of plasma transistors
      ReadValueFromStringWithDefault( RobotPointer , "Number of Antimatter-Matter Converters=" , "%hhd" , "0", 
			   &Druidmap[RobotIndex].amount_of_antimatter_converters , EndOfDataPointer );

      // Now we read in the number of plasma transistors
      ReadValueFromStringWithDefault( RobotPointer , "Number of Entropy Inverters=" , "%hhd" , "0", 
			   &Druidmap[RobotIndex].amount_of_entropy_inverters , EndOfDataPointer );

      // Now we read in the number of plasma transistors
      ReadValueFromStringWithDefault( RobotPointer , "Number of Tach. Condensators=" , "%hhd" , "0",
			   &Druidmap[RobotIndex].amount_of_tachyon_condensators , EndOfDataPointer );

      // Now we read in the greeting sound type of this droid type
      ReadValueFromString( RobotPointer , GREETING_SOUND_STRING , "%hd" , 
			   &Druidmap[RobotIndex].greeting_sound_type , EndOfDataPointer );

      // Now we read in the greeting sound type of this droid type
      ReadValueFromString( RobotPointer , ENEMY_GOT_HIT_SOUND_STRING , "%hd" , 
			   &Druidmap[RobotIndex].got_hit_sound_type , EndOfDataPointer );

      // Now we read in the to-hit chance this robot has in combat against an unarmoured target
      ReadValueFromString( RobotPointer , TO_HIT_STRING , "%hd" , 
			   &Druidmap[RobotIndex].to_hit , EndOfDataPointer );

      // Now we read in the modifier, that increases/decreases the chance of this robot getting hit
      ReadValueFromString( RobotPointer , GETTING_HIT_MODIFIER_STRING , "%hd" , 
			   &Druidmap[RobotIndex].getting_hit_modifier , EndOfDataPointer );

      // Now we read in the modifier, that increases/decreases the chance of this robot getting hit
      ReadValueFromString( RobotPointer , "Time to recover after getting hit=" , "%lf" , 
			   &Druidmap[RobotIndex] . recover_time_after_getting_hit , EndOfDataPointer );

      // Now we read in the is_human flag of this droid type
      ReadValueFromString( RobotPointer , IS_HUMAN_SPECIFICATION_STRING , "%hhd" , 
			   &Druidmap[RobotIndex].is_human , EndOfDataPointer );

      // Now we read in the is_human flag of this droid type
      ReadValueFromString( RobotPointer , INDIVIDUAL_SHAPE_SPECIFICATION_STRING , "%hd" , 
			   &Druidmap[RobotIndex].individual_shape_nr , EndOfDataPointer );

      Druidmap[RobotIndex].notes = 
	ReadAndMallocStringFromData ( RobotPointer , NOTES_BEGIN_STRING , "\"" ) ;

      // Now we're potentially ready to process the next droid.  Therefore we proceed to
      // the next number in the Droidmap array.
      RobotIndex++;
      if ( EndOfThisRobot ) EndOfThisRobot [ 0 ] = '*'; // We put back the star at its place
    }

  DebugPrintf ( 1 , "\n\nThat must have been the last robot.  We're done reading the robot data.");
  DebugPrintf ( 1 , "\n\nApplying the calibration factors to all droids...");

  for ( i = 0 ; i < Number_Of_Droid_Types ; i++ ) 
    {
      Druidmap [ i ] . maxspeed *= maxspeed_calibrator;
      Druidmap [ i ] . maxenergy *= maxenergy_calibrator;
      Druidmap [ i ] . aggression *= aggression_calibrator;
      Druidmap [ i ] . experience_reward *= experience_reward_calibrator;
      Druidmap [ i ] . range_of_vision *= range_of_vision_calibrator;
      Druidmap [ i ] . lose_health *= energyloss_calibrator;
      Druidmap [ i ] . weapon_item . currently_held_in_hand = FALSE ;
    }
}; // int Get_Robot_Data ( void )

/*----------------------------------------------------------------------
 * This function reads in all the item data from the freedroid.ruleset file,
 * but IT DOES NOT LOAD THE FILE, IT ASSUMES IT IS ALREADY LOADED and
 * it only receives a pointer to the start of the bullet section from
 * the calling function.
 ----------------------------------------------------------------------*/
void 
Get_Item_Data ( char* DataPointer )
{
    char *ItemPointer;
    char *EndOfItemData;
    int ItemIndex=0;
    char *YesNoString;
    float ranged_weapon_damage_calibrator;
    float melee_weapon_damage_calibrator;
    float ranged_weapon_speed_calibrator;
    
#define ITEM_SECTION_BEGIN_STRING "*** Start of item data section: ***"
#define ITEM_SECTION_END_STRING "*** End of item data section: ***"
#define NEW_ITEM_TYPE_BEGIN_STRING "** Start of new item specification subsection **"
    
#define ITEM_NAME_INDICATION_STRING "Item name=_\""
#define ITEM_DESCRIPTION_INDICATION_STRING "Item description text=_\""
#define ITEM_CAN_BE_APPLIED_IN_COMBAT "Item can be applied in combat=\""
#define ITEM_CAN_BE_INSTALLED_IN_WEAPON_SLOT "Item can be installed in weapon slot=\""
#define ITEM_CAN_BE_INSTALLED_IN_DRIVE_SLOT "Item can be installed in drive slot=\""
#define ITEM_CAN_BE_INSTALLED_IN_ARMOUR_SLOT "Item can be installed in armour slot=\""
#define ITEM_CAN_BE_INSTALLED_IN_SHIELD_SLOT "Item can be installed in shield slot=\""
#define ITEM_CAN_BE_INSTALLED_IN_SPECIAL_SLOT "Item can be installed in special slot=\""
    
#define ITEM_CAN_BE_INSTALLED_IN_SLOT_WITH_NAME "Item can be installed in slot with name=\""
#define ITEM_ROTATION_SERIES_NAME_PREFIX "Item uses rotation series with prefix=\""
#define ITEM_GROUP_TOGETHER_IN_INVENTORY "Items of this type collect together in inventory=\""
    
#define ITEM_GUN_IGNORE_WALL "Item as gun: ignore collisions with wall=\""
// #define ITEM_DROP_SOUND_FILE_NAME "Item uses drop sound with filename=\""
// #define ITEM_INVENTORY_IMAGE_FILE_NAME "File or directory name for inventory image=\""
    
#define ITEM_RECHARGE_TIME_BEGIN_STRING "Time is takes to recharge this bullet/weapon in seconds :"
#define ITEM_SPEED_BEGIN_STRING "Flying speed of this bullet type :"
#define ITEM_DAMAGE_BEGIN_STRING "Damage cause by a hit of this bullet type :"
#define ITEM_ONE_SHOT_ONLY_AT_A_TIME "Cannot fire until previous bullet has been deleted : "
    
#define ITEM_SPEED_CALIBRATOR_STRING "Common factor for all bullet's speed values: "
#define ITEM_DAMAGE_CALIBRATOR_STRING "Common factor for all bullet's damage values: "
    
    ItemPointer = LocateStringInData ( DataPointer , ITEM_SECTION_BEGIN_STRING );
    EndOfItemData = LocateStringInData ( DataPointer , ITEM_SECTION_END_STRING );
    
    Number_Of_Item_Types = CountStringOccurences ( DataPointer , NEW_ITEM_TYPE_BEGIN_STRING ) ;
    
    //--------------------
    // Now that we know how many item archetypes there are, we can allocate the proper
    // amount of memory for this information.
    //
    ItemMap = (itemspec*) MyMalloc ( sizeof ( itemspec ) * ( Number_Of_Item_Types + 1 ) );
    //--------------------
    // Now we start to read the values for each bullet type:
    // 
    ItemPointer=DataPointer;
    
    // Now we read in the speed calibration factor for all bullets
    ReadValueFromString( DataPointer ,  "Common factor for all ranged weapons bullet speed values:" , "%f" , 
			 &ranged_weapon_speed_calibrator , EndOfItemData );
    
    // Now we read in the damage calibration factor for all bullets
    ReadValueFromString( DataPointer ,  "Common factor for all ranged weapons bullet damage values:" , "%f" , 
			 &ranged_weapon_damage_calibrator , EndOfItemData );
    
    // Now we read in the damage calibration factor for all bullets
    ReadValueFromString( DataPointer ,  "Common factor for all melee weapons damage values:" , "%f" , 
			 &melee_weapon_damage_calibrator , EndOfItemData );
    
    DebugPrintf ( 1 , "\nCommon bullet speed factor: %f.\nCommon bullet damage factor: %f.\nCommon melee damage factor: %f.\n", ranged_weapon_speed_calibrator, ranged_weapon_damage_calibrator , melee_weapon_damage_calibrator );
    
    while ( (ItemPointer = strstr ( ItemPointer, NEW_ITEM_TYPE_BEGIN_STRING )) != NULL)
    {
	DebugPrintf ( 1 , "\n\nFound another Item specification entry!  Lets add that to the others!");
	ItemPointer ++; 
	char * EndOfThisItem = strstr ( ItemPointer, NEW_ITEM_TYPE_BEGIN_STRING );
        if ( EndOfThisItem ) EndOfThisItem [ 0 ] = 0;
	
	// Now we read in the name of this item
	ItemMap[ItemIndex].item_name = ReadAndMallocStringFromData ( ItemPointer , ITEM_NAME_INDICATION_STRING , "\"" ); 
	
	// Now we read in the description string of this item
	ItemMap[ItemIndex].item_description = ReadAndMallocStringFromData ( ItemPointer , ITEM_DESCRIPTION_INDICATION_STRING , "\"" ) ;
	
	// Now we read in if this item can be used by the influ without help
	YesNoString = ReadAndMallocStringFromData ( ItemPointer , ITEM_CAN_BE_APPLIED_IN_COMBAT , "\"" ) ;
	if ( strcmp( YesNoString , "yes" ) == 0 )
	{
	    ItemMap[ItemIndex].item_can_be_applied_in_combat = TRUE;
	}
	else if ( strcmp( YesNoString , "no" ) == 0 )
	{
	    ItemMap[ItemIndex].item_can_be_applied_in_combat = FALSE;
	}
	else
	{
	    ErrorMessage ( __FUNCTION__  , "The item specification of an item in freedroid.item_archetypes should contain an \nanswer that is either 'yes' or 'no', but which was neither 'yes' nor 'no'.\nThis indicated a corrupted freedroid.ruleset file with an error at least in\nthe item specification section.",
				       PLEASE_INFORM, IS_FATAL );
	}
	free ( YesNoString ) ;
	
	// Now we read the label telling us in which slot the item can be installed
	YesNoString = ReadAndMallocStringFromData ( ItemPointer , ITEM_CAN_BE_INSTALLED_IN_SLOT_WITH_NAME , "\"" ) ;
	ItemMap[ItemIndex].item_can_be_installed_in_weapon_slot = FALSE;
	ItemMap[ItemIndex].item_can_be_installed_in_shield_slot = FALSE;
	ItemMap[ItemIndex].item_can_be_installed_in_drive_slot = FALSE;
	ItemMap[ItemIndex].item_can_be_installed_in_armour_slot = FALSE;
	ItemMap[ItemIndex].item_can_be_installed_in_special_slot = FALSE;
	if ( strcmp( YesNoString , "weapon" ) == 0 )
	{
	    ItemMap[ItemIndex].item_can_be_installed_in_weapon_slot = TRUE;
	}
	else if ( strcmp( YesNoString , "drive" ) == 0 )
	{
	    ItemMap[ItemIndex].item_can_be_installed_in_drive_slot = TRUE;
	}
	else if ( strcmp( YesNoString , "shield" ) == 0 )
	{
	    ItemMap[ItemIndex].item_can_be_installed_in_shield_slot = TRUE;
	}
	else if ( strcmp( YesNoString , "armour" ) == 0 )
	{
	    ItemMap[ItemIndex].item_can_be_installed_in_armour_slot = TRUE;
	}
	else if ( strcmp( YesNoString , "special" ) == 0 )
	{
	    ItemMap[ItemIndex].item_can_be_installed_in_special_slot = TRUE;
	}
	else if ( strcmp( YesNoString , "none" ) == 0 )
	{
	    // good.  Everything is ok, as long as at least 'none' was found
	}
	else
	{
	    fprintf(stderr, "\n\nItemIndex: %d.\n" ,ItemIndex ); 
	    ErrorMessage ( __FUNCTION__  , "The item specification of an item in freedroid.ruleset should contain an \nanswer for the slot installation possiblieties, that was neither \n'weapon' nor 'armour' nor 'shield' nor 'special' nor 'drive' nor 'none'.",
				       PLEASE_INFORM, IS_FATAL );
	}
	free ( YesNoString ) ;
	
	//--------------------
	// Next we read in the prefix for the image series in the items browser
	// that this item is going to use.
	//
	ItemMap [ ItemIndex ] . item_rotation_series_prefix = ReadAndMallocStringFromData ( ItemPointer , ITEM_ROTATION_SERIES_NAME_PREFIX , "\"" ) ;
	
	//--------------------
	// Now we read in if this item will group together in inventory
	//
	YesNoString = ReadAndMallocStringFromData ( ItemPointer , ITEM_GROUP_TOGETHER_IN_INVENTORY , "\"" ) ;
	if ( strcmp( YesNoString , "yes" ) == 0 )
	{
	    ItemMap[ItemIndex].item_group_together_in_inventory = TRUE;
	}
	else if ( strcmp( YesNoString , "no" ) == 0 )
	{
	    ItemMap[ItemIndex].item_group_together_in_inventory = FALSE;
	}
	else
	{
	    ErrorMessage ( __FUNCTION__  , "\
The item specification of an item in freedroid.ruleset should contain an \n\
answer that is either 'yes' or 'no', but which was neither 'yes' nor 'no'.",
				       PLEASE_INFORM, IS_FATAL );
	}
	free ( YesNoString ) ;
	
	//--------------------
	// Now we read in minimum strength, dex and magic required to wear/wield this item
	//
	ReadValueFromStringWithDefault( ItemPointer , "Strength minimum required to wear/wield this item=" , "%hd" , "-1",
			     &ItemMap[ItemIndex].item_require_strength , EndOfItemData );
	ReadValueFromStringWithDefault( ItemPointer , "Dexterity minimum required to wear/wield this item=" , "%hd" , "-1", 
			     &ItemMap[ItemIndex].item_require_dexterity , EndOfItemData );
	ReadValueFromStringWithDefault( ItemPointer , "Magic minimum required to wear/wield this item=" , "%hd" , "-1",
			     &ItemMap[ItemIndex].item_require_magic , EndOfItemData );


	ReadValueFromStringWithDefault( ItemPointer, "Item min class for drop=", "%hd", "-1", &ItemMap[ItemIndex].min_drop_class, EndOfItemData);
	ReadValueFromStringWithDefault( ItemPointer, "Item max class for drop=", "%hd", "-1", &ItemMap[ItemIndex].max_drop_class, EndOfItemData);

	if ( ItemMap[ItemIndex].min_drop_class != -1 )
	    {
	    int cc;
	    for ( cc = 0; cc < 10; cc ++ )
		{
		if ( cc > ItemMap[ItemIndex].max_drop_class ) break;
		if ( cc < ItemMap[ItemIndex].min_drop_class ) continue;
		else 
		    item_count_per_class[cc] ++;
		}
	    }

	
	//--------------------
	// If the item is a gun, we read in the weapon specification...
	//
	if ( ItemMap[ItemIndex].item_can_be_installed_in_weapon_slot == TRUE )
	{
	    // Now we read in the damage bullets from this gun will do
	    ReadValueFromStringWithDefault( ItemPointer ,  "Item as gun: damage of bullets=" , "%hd" , "0",
				 &ItemMap[ItemIndex].base_item_gun_damage , EndOfItemData );
	    ReadValueFromStringWithDefault( ItemPointer ,  "Item as gun: modifier for damage of bullets=" , "%hd" , "0",
				 &ItemMap[ItemIndex].item_gun_damage_modifier , EndOfItemData );
	    
	    // Now we read in the speed this bullet will go
	    ReadValueFromStringWithDefault( ItemPointer ,  "Item as gun: speed of bullets=" , "%lf" , "0.000000",
				 &ItemMap[ItemIndex].item_gun_speed , EndOfItemData );
	    
	    // Now we read in speed of melee application and melee offset from influ
	    ReadValueFromStringWithDefault( ItemPointer ,  "Item as gun: angle change of bullets=" , "%lf" , "0.000000", 
				 &ItemMap[ItemIndex].item_gun_angle_change , EndOfItemData );
	    ReadValueFromStringWithDefault( ItemPointer ,  "Item as gun: offset for melee weapon=" , "%lf" , "0.000000",
				 &ItemMap[ItemIndex].item_gun_fixed_offset , EndOfItemData );
	    ReadValueFromStringWithDefault( ItemPointer ,  "Item as gun: modifier for starting angle=" , "%lf" , "0.000000",
				 &ItemMap[ItemIndex].item_gun_start_angle_modifier , EndOfItemData );
	    
	    // Now we read in if this weapon can pass through walls or not...
	    YesNoString = ReadAndMallocStringFromData ( ItemPointer , ITEM_GUN_IGNORE_WALL , "\"" ) ;
	    if ( strcmp( YesNoString , "yes" ) == 0 )
	    {
		ItemMap[ItemIndex].item_gun_bullet_ignore_wall_collisions = TRUE;
	    }
	    else if ( strcmp( YesNoString , "no" ) == 0 )
	    {
		ItemMap[ItemIndex].item_gun_bullet_ignore_wall_collisions = FALSE;
	    }
	    else
	    {
		ErrorMessage ( __FUNCTION__  , "\
The item specification of an item in freedroid.ruleset should contain an \n\
answer that is either 'yes' or 'no', but which was neither 'yes' nor 'no'.",
					   PLEASE_INFORM, IS_FATAL );
	    }; // if ( ItemMap[ItemIndex].item_can_be_installed_in_weapon_slot == TRUE )
            free ( YesNoString ) ;
	    
	    // Now we read in if this weapons bullets will reflect other bullets or not
	    YesNoString = ReadAndMallocStringFromData ( ItemPointer , "Item as gun: reflect other bullets=\"" , "\"" ) ;
	    if ( strcmp( YesNoString , "yes" ) == 0 )
	    {
		ItemMap[ItemIndex].item_gun_bullet_reflect_other_bullets = TRUE;
	    }
	    else if ( strcmp( YesNoString , "no" ) == 0 )
	    {
		ItemMap[ItemIndex].item_gun_bullet_reflect_other_bullets = FALSE;
	    }
	    else
	    {
		ErrorMessage ( __FUNCTION__  , "\
The item specification of an item in freedroid.ruleset should contain an \n\
answer that is either 'yes' or 'no', but which was neither 'yes' nor 'no'.",
					   PLEASE_INFORM, IS_FATAL );
	    }; // if ( ItemMap[ItemIndex].item_can_be_installed_in_weapon_slot == TRUE )
            free ( YesNoString ) ;

	    // Now we read in if this weapons bullets will reflect other bullets or not
	    YesNoString = ReadAndMallocStringFromData ( ItemPointer , "Item as gun: pass through explosions=\"" , "\"" ) ;
	    if ( strcmp( YesNoString , "yes" ) == 0 )
	    {
		ItemMap[ItemIndex].item_gun_bullet_pass_through_explosions = TRUE;
	    }
	    else if ( strcmp( YesNoString , "no" ) == 0 )
	    {
		ItemMap[ItemIndex].item_gun_bullet_pass_through_explosions = FALSE;
	    }
	    else
	    {
		ErrorMessage ( __FUNCTION__  , "\
The item specification of an item in freedroid.ruleset should contain an \n\
answer that is either 'yes' or 'no', but which was neither 'yes' nor 'no'.",
					   PLEASE_INFORM, IS_FATAL );
	    }; // if ( ItemMap[ItemIndex].item_can_be_installed_in_weapon_slot == TRUE )
            free ( YesNoString ) ;
	    
	    // Now we read in if this weapons bullets will reflect other bullets or not
	    YesNoString = ReadAndMallocStringFromData ( ItemPointer , "Item as gun: pass through hit bodies=\"" , "\"" ) ;
	    if ( strcmp( YesNoString , "yes" ) == 0 )
	    {
		ItemMap[ItemIndex].item_gun_bullet_pass_through_hit_bodies = TRUE;
	    }
	    else if ( strcmp( YesNoString , "no" ) == 0 )
	    {
		ItemMap[ItemIndex].item_gun_bullet_pass_through_hit_bodies = FALSE;
	    }
	    else
	    {
		ErrorMessage ( __FUNCTION__  , "\
The item specification of an item in freedroid.ruleset should contain an \n\
answer that is either 'yes' or 'no', but which was neither 'yes' nor 'no'.",
					   PLEASE_INFORM, IS_FATAL );
	    }; // if ( ItemMap[ItemIndex].item_can_be_installed_in_weapon_slot == TRUE )
            free ( YesNoString ) ;
	    
	    // Now we read in the recharging time this weapon will need
	    ReadValueFromString( ItemPointer ,  "Item as gun: recharging time=" , "%lf" , 
				 &ItemMap[ItemIndex].item_gun_recharging_time , EndOfItemData );

	    // Now we read in the reloading time this weapon will need
	    ReadValueFromString( ItemPointer ,  "Item as gun: reloading time=" , "%lf" , 
				 &ItemMap[ItemIndex].item_gun_reloading_time , EndOfItemData );
	    
	    // Now we read in the image type that should be generated for this bullet
	    ReadValueFromString( ItemPointer ,  "Item as gun: bullet_image_type=" , "%hd" , 
				 &ItemMap[ItemIndex].item_gun_bullet_image_type , EndOfItemData );
	    
	    // Now we read in the image type that should be generated for this bullet
	    ReadValueFromString( ItemPointer ,  "Item as gun: bullet_lifetime=" , "%lf" , 
				 &ItemMap[ItemIndex].item_gun_bullet_lifetime , EndOfItemData );

	    // Now we read in the image type that should be generated for this bullet
	    ReadValueFromString( ItemPointer ,  "Item as gun: ammo clip size=" , "%hd" , 
				 &ItemMap[ItemIndex].item_gun_ammo_clip_size , EndOfItemData );
	    
	    //--------------------
	    // Some guns require some ammunition.  This will be read in and
	    // examined next...
	    //
	    YesNoString = ReadAndMallocStringFromData ( ItemPointer , "Item as gun: required ammunition type=\"" , "\"" ) ;
	    if ( strcmp( YesNoString , "none" ) == 0 )
		{
		ItemMap[ItemIndex].item_gun_use_ammunition = 0;
		}
	    else if ( strcmp( YesNoString , "plasma_ammunition" ) == 0 )
		{
		ItemMap[ItemIndex].item_gun_use_ammunition = 2;
		}
	    else if ( strcmp( YesNoString , "laser_ammunition" ) == 0 )
		{
		ItemMap[ItemIndex].item_gun_use_ammunition = 1;
		}
	    else if ( strcmp( YesNoString , "exterminator_ammunition" ) == 0 )
		{
		ItemMap[ItemIndex].item_gun_use_ammunition = 3;
		}
	    else if ( strcmp( YesNoString, "22LR" ) == 0 )
		{
		ItemMap[ItemIndex].item_gun_use_ammunition = 4;
		}
	    else if ( strcmp( YesNoString, "Sshell" ) == 0 )
		{
		ItemMap[ItemIndex].item_gun_use_ammunition = 5;
		}
	    else if ( strcmp( YesNoString, "9mm" ) == 0 )
		{
		ItemMap[ItemIndex].item_gun_use_ammunition = 6;
		}
	    else if ( strcmp( YesNoString, "7.62mm" ) == 0 )
		{
		ItemMap[ItemIndex].item_gun_use_ammunition = 7;
		}
	    else if ( strcmp( YesNoString, "50BMG" ) == 0 )
		{
		ItemMap[ItemIndex].item_gun_use_ammunition = 8;
		}
	    else
	    {
		ErrorMessage ( __FUNCTION__  , "\
The type of ammunition used by an item in freedroid.item_archetypes was not recognized. \n\
This string was: %s\n",
					   PLEASE_INFORM, IS_FATAL, YesNoString );
	    }
            free ( YesNoString ) ;
	    
	    // Now we read in if this weapons (strictly) requires both hands for usage
	    YesNoString = ReadAndMallocStringFromData ( ItemPointer , "Item as gun: weapon requires both hands=\"" , "\"" ) ;
	    if ( strcmp( YesNoString , "yes" ) == 0 )
	    {
		ItemMap [ ItemIndex ] . item_gun_requires_both_hands = TRUE;
	    }
	    else if ( strcmp( YesNoString , "no" ) == 0 )
	    {
		ItemMap [ ItemIndex ] . item_gun_requires_both_hands = FALSE;
	    }
	    else
	    {
		ErrorMessage ( __FUNCTION__  , "\
The item specification of an item in freedroid.item_archetypes should contain an \n\
answer that is either 'yes' or 'no', but which was neither 'yes' nor 'no'.",
					   PLEASE_INFORM, IS_FATAL );
	    }; 
            free ( YesNoString ) ;
	    
	}
	else
	{
	    //--------------------
	    // If it is not a gun, we set the weapon specifications to
	    // empty values...
	    //
	    ItemMap [ ItemIndex ] . base_item_gun_damage = 0 ;
	    ItemMap [ ItemIndex ] . item_gun_damage_modifier = 0 ;
	    ItemMap [ ItemIndex ] . item_gun_speed = 0 ;
	    ItemMap [ ItemIndex ] . item_gun_angle_change = 0 ;
	    ItemMap [ ItemIndex ] . item_gun_fixed_offset = 0 ;
	    ItemMap [ ItemIndex ] . item_gun_start_angle_modifier = 0 ;
	    ItemMap [ ItemIndex ] . item_gun_bullet_ignore_wall_collisions = FALSE ;
	    ItemMap [ ItemIndex ] . item_gun_bullet_reflect_other_bullets = FALSE ;
	    ItemMap [ ItemIndex ] . item_gun_bullet_pass_through_explosions = FALSE ;
	    ItemMap [ ItemIndex ] . item_gun_bullet_pass_through_hit_bodies = FALSE ;
	    ItemMap [ ItemIndex ] . item_gun_recharging_time = 0 ;
	    ItemMap [ ItemIndex ] . item_gun_reloading_time = 0 ;
	    ItemMap [ ItemIndex ] . item_gun_bullet_image_type = 0 ; 
	    ItemMap [ ItemIndex ] . item_gun_bullet_lifetime = 0 ;
	    ItemMap [ ItemIndex ] . item_gun_use_ammunition = 0 ;
	    ItemMap [ ItemIndex ] . item_gun_requires_both_hands = TRUE ;
	}
	
	// Now we read in the armour value of this item as armour or shield or whatever
	ReadValueFromStringWithDefault( ItemPointer ,  "Item as defensive item: base_ac_bonus=" , "%hd" , "0",
			     &ItemMap[ItemIndex].base_ac_bonus , EndOfItemData );
	ReadValueFromStringWithDefault( ItemPointer ,  "Item as defensive item: ac_bonus_modifier=" , "%hd" , "0",
			     &ItemMap[ItemIndex].ac_bonus_modifier , EndOfItemData );
	
	// Now we read in the base item duration and the duration modifier
	ReadValueFromStringWithDefault( ItemPointer ,  "Base item duration=" , "%hd" , "-1", 
			     &ItemMap[ItemIndex].base_item_duration , EndOfItemData );
	ReadValueFromStringWithDefault( ItemPointer ,  "plus duration modifier=" , "%hd" , "0",
			     &ItemMap[ItemIndex].item_duration_modifier , EndOfItemData );
	
	//--------------------
	// Now we read in the name of the inventory item image, that is to be used
	// on the inventory screen.
	//
	ItemMap [ ItemIndex ] . item_inv_file_name = 
	    ReadAndMallocStringFromData ( ItemPointer , ITEM_INVENTORY_IMAGE_FILE_NAME , "\"" ) ;
	// DebugPrintf ( 0 , "\nName of item %d is: '%s'." , ItemIndex , ItemMap [ ItemIndex ] . item_name );
	
	// Now we read in the name of the sound sample to be played when this item is moved
	ItemMap[ItemIndex].item_drop_sound_file_name = 
	    ReadAndMallocStringFromData ( ItemPointer , ITEM_DROP_SOUND_FILE_NAME , "\"" ) ;
	// DebugPrintf ( 0 , "\nName of item %d is: '%s'." , ItemIndex , ItemMap [ ItemIndex ] . item_name );
	
	// Now we read the size of the item in the inventory. 0 equals "figure out automatically".
	ReadValueFromStringWithDefault( ItemPointer ,  "inventory_size_x=" , "%d" , "0",
			     &ItemMap [ ItemIndex ] . inv_image . inv_size . x , EndOfItemData );
	ReadValueFromStringWithDefault( ItemPointer ,  "inventory_size_y=" , "%d" , "0",
			     &ItemMap [ ItemIndex ] . inv_image . inv_size . y , EndOfItemData );


	// Now we read in the base list price for this item
	ReadValueFromString( ItemPointer ,  "Base list price=" , "%hd" , 
			     &ItemMap[ItemIndex].base_list_price , EndOfItemData );
	
	//--------------------
	// Now that the picture name has been loaded, we can already load the
	// surfaces associated with the picture...
	//
	load_item_surfaces_for_item_type ( ItemIndex );
	

	ItemIndex++;
        if ( EndOfThisItem ) EndOfThisItem [ 0 ] = '*'; // We put back the star at its place	
    }
    
    //--------------------
    // Now that all the calibrations factors have been read in, we can start to
    // apply them to all the bullet types
    //
    for ( ItemIndex = 0 ; ItemIndex < Number_Of_Item_Types ; ItemIndex++ )
    {
	if ( ItemMap [ ItemIndex ] . item_gun_angle_change )
	{
	    ItemMap [ ItemIndex ] . base_item_gun_damage *= melee_weapon_damage_calibrator;
	    ItemMap [ ItemIndex ] . item_gun_damage_modifier *= melee_weapon_damage_calibrator;
	}
	else
	{
	    ItemMap [ ItemIndex ] . item_gun_speed *= ranged_weapon_speed_calibrator;
	    ItemMap [ ItemIndex ] . base_item_gun_damage *= ranged_weapon_damage_calibrator;
	    ItemMap [ ItemIndex ] . item_gun_damage_modifier *= ranged_weapon_damage_calibrator;
	}
    }
    
}; // void Get_Item_Data ( char* DataPointer );

/* ----------------------------------------------------------------------
 * This function loads all the constant variables of the game from
 * a data file, using mainly subroutines which do the main work.
 * ---------------------------------------------------------------------- */
void
Init_Game_Data ()
{
char fpath[2048];
  char *Data;

#define INIT_GAME_DATA_DEBUG 1 

  //--------------------
  // Load magical items informations
  //
  find_file ("freedroid.prefix_archetypes" , MAP_DIR , fpath, 0 );
  DebugPrintf ( INIT_GAME_DATA_DEBUG , "\nvoid Init_Game_Data:  Data will be taken from file : %s. Commencing... \n" ,
		fpath );
  Data = ReadAndMallocAndTerminateFile( fpath , "*** End of this Freedroid data File ***" ) ;
  Get_Prefixes_Data ( Data );
  free ( Data );

  //--------------------
  // Load programs (spells) informations
  //
  find_file ("freedroid.program_archetypes" , MAP_DIR , fpath, 0 );
  DebugPrintf ( INIT_GAME_DATA_DEBUG , "\nvoid Init_Game_Data:  Data will be taken from file : %s. Commencing... \n" ,
		fpath );
  Data = ReadAndMallocAndTerminateFile( fpath , "*** End of this Freedroid data File ***" ) ;
  Get_Programs_Data ( Data );
  free ( Data );


  //--------------------
  // Item archetypes must be loaded too
  //
  find_file ("freedroid.item_archetypes" , MAP_DIR , fpath, 0 );
  DebugPrintf ( INIT_GAME_DATA_DEBUG , "\nvoid Init_Game_Data:  Data will be taken from file : %s. Commencing... \n" ,
		fpath );
  Data = ReadAndMallocAndTerminateFile( fpath , "*** End of this Freedroid data File ***" ) ;
  Get_Item_Data ( Data );
  free ( Data );

  //--------------------
  // Time to eat some droid archetypes...
  //
  find_file ("freedroid.droid_archetypes" , MAP_DIR , fpath, 0 );
  DebugPrintf ( INIT_GAME_DATA_DEBUG , "\nvoid Init_Game_Data:  Data will be taken from file : %s. Commencing... \n" ,
		fpath );
  Data = ReadAndMallocAndTerminateFile( fpath , "*** End of this Freedroid data File ***" ) ;
  Get_Robot_Data ( Data );
  free ( Data );

  //--------------------
  // Now finally it's time for all the bullet data...
  //
  find_file ("freedroid.bullet_archetypes" , MAP_DIR, fpath, 0);
  DebugPrintf ( INIT_GAME_DATA_DEBUG , "\nvoid Init_Game_Data:  Data will be taken from file : %s. Commencing... \n" ,
		fpath );
  Data = ReadAndMallocAndTerminateFile( fpath , "*** End of this Freedroid data File ***" ) ;
  Get_Bullet_Data ( Data );
  free ( Data );

}; // int Init_Game_Data ( void )

char copyright[] = "\nCopyright (C) 2004 Johannes Prix, Reinhard Prix\n\
Copyright (C) 2005-2007 Arthur Huillet, Karol Swietlicki\n\
Freedroid comes with NO WARRANTY to the extent permitted by law.\n\
You may redistribute copies of Freedroid\n\
under the terms of the GNU General Public License.\n\
For more information about these matters, see the file named COPYING.\n";


char usage_string[] ="\
Usage: freedroid [-v|--version] \n\
                 [-s|--sound] [-q|--nosound] \n\
                 [-o|--open_gl] [-n|--no_open_gl]\n\
                 [-f|--fullscreen] [-w|--window]\n\
                 [-m|--mapcheck] \n\
                 [-j|--sensitivity]\n\
                 [-d|--debug=LEVEL]\n\
		 [-r|--resolution=CODE]\n\
\n\
Please report bugs either by entering them into the bug-tracking\n\
system on our sourceforge-website via this link:\n\n\
http://sourceforge.net/projects/freedroid/\n\n\
or EVEN BETTER, report them by sending e-mail to:\n\n\
freedroid-discussion@lists.sourceforge.net\n\n\
Thanks a lot in advance, the Freedroid dev team.\n\n";

/* -----------------------------------------------------------------
 *  parse command line arguments and set global switches 
 *  exit on error, so we don't need to return success status
 * -----------------------------------------------------------------*/
void
ParseCommandLine (int argc, char *const argv[])
{
    int c;
    int resolution_code = 1 ;

    static struct option long_options[] = {
	{"version",     0, 0,  'v'},
	{"help",        0, 0,  'h'},
	{"open_gl",     0, 0,  'o'},
	{"no_open_gl",  0, 0,  'n'},
	{"nosound",     0, 0,  'q'},
	{"sound",       0, 0,  's'},
	{"debug",       1, 0,  'd'},
	{"window",      0, 0,  'w'},
	{"fullscreen",  0, 0,  'f'},
	{"mapcheck",    0, 0,  'm'},
	{"sensitivity", 1, 0,  'j'},
	{"resolution",  1, 0,  'r'},	
	{ 0,            0, 0,    0}
    };

    command_line_override_for_screen_resolution = FALSE ;

    while ( 1 )
    {
	c = getopt_long ( argc , argv , "vonqst:h?d::r:wfmj:" , long_options , NULL );
	if ( c == -1 )
	    break;

	switch ( c )
	{
	    /* version statement -v or --version
	     * following gnu-coding standards for command line interfaces */
	    case 'v':
		printf ("\n%s %s  \n", PACKAGE, VERSION);
		printf (copyright);
		exit (0);
		break;
		
	    case 'h':
	    case '?':
		printf ( usage_string );
		exit ( 0 );
		break;
		
	    case 'o':
		use_open_gl = TRUE;
		break;
		
	    case 'n':
		use_open_gl = FALSE;
		break;
		
	    case 'q':
		sound_on = FALSE;
		break;
		
	    case 's':
		sound_on = TRUE;
		break;
		
	    case 'j':
		joy_sensitivity = atoi (optarg);
		if (joy_sensitivity < 0 || joy_sensitivity > 32)
		{
		    printf ("\nJoystick sensitivity must lie in the range [0;32]\n");
		    Terminate(ERR);
		}
		break;
		
	    case 'd':
		if (!optarg) 
		    debug_level = 1;
		else
		    debug_level = atoi (optarg);
		break;
		
	    case 'r':
		if (!optarg) 
		{
		    GameConfig . screen_width = 800; 
		    GameConfig . screen_height = 600 ;
		}
		else
		{
		    resolution_code = atoi (optarg);
		    switch ( resolution_code )
		    {
			case 0:
			    command_line_override_for_screen_resolution = TRUE ;
			    GameConfig . screen_width = 640 ; 
			    GameConfig . screen_height = 480 ;
			    DebugPrintf ( 1 , "\n%s(): Command line argument -r 0 recognized." , __FUNCTION__ );
			    break;
			case 1:
			    command_line_override_for_screen_resolution = TRUE ;
			    GameConfig . screen_width = 800 ; 
			    GameConfig . screen_height = 600 ;
			    DebugPrintf ( 1 , "\n%s(): Command line argument -r 1 recognized." , __FUNCTION__ );
			    break;
			case 2:
			    command_line_override_for_screen_resolution = TRUE ;
			    GameConfig . screen_width = 1024 ; 
			    GameConfig . screen_height = 768 ;
			    DebugPrintf ( 1 , "\n%s(): Command line argument -r 2 recognized." , __FUNCTION__ );
			    break;
			case 3:
	                    command_line_override_for_screen_resolution = TRUE ;
                            GameConfig . screen_width = 1280 ;
                            GameConfig . screen_height = 1024 ;
                            DebugPrintf ( 1 , "\n%s(): Command line argument -r 3 recognized." , __FUNCTION__ );
                            break;
			case 4:
                            command_line_override_for_screen_resolution = TRUE ;
                            GameConfig . screen_width = 1280 ;
                            GameConfig . screen_height = 800 ;
                            DebugPrintf ( 1 , "\n%s(): Command line argument -r 4 recognized." , __FUNCTION__ );
                            break;
#if 0			
			case 5: 
                            command_line_override_for_screen_resolution = TRUE ;
                            GameConfig . screen_width = 1280 ;
                            GameConfig . screen_height = 800 ;
                            DebugPrintf ( 1 , "\n%s(): Command line argument -r 5 recognized." , __FUNCTION__ );
                            break;
			case 6: 
                            command_line_override_for_screen_resolution = TRUE ;
                            GameConfig . screen_width = 1280 ;
                            GameConfig . screen_height =1024 ;
                            DebugPrintf ( 1 , "\n%s(): Command line argument -r 6 recognized." , __FUNCTION__ );
                            break;
#endif


			default:
			    fprintf( stderr, "\nresolution code received: %d" , resolution_code );
			    ErrorMessage ( __FUNCTION__  , "\
The resolution identifier given is not a valid resolution code.\n\
These codes correspond to the following resolutions available:\n\
     0 = 640 x 480 (default with SDL)\n\
     1 = 800 x 600 (default with OpenGL)\n\
     2 = 1024 x 748 \n\
     3 = 1280 x 1024 \n\
Anything else will not be accepted right now, but you can send in\n\
your suggestion to the FreedroidRPG dev team to enable new resolutions.",
						       NO_NEED_TO_INFORM , IS_FATAL );
			    break;
		    }
		}
		break;
		
	    case 'f':
		GameConfig . fullscreen_on = TRUE;
		break;
		
	    case 'w':
		GameConfig . fullscreen_on = FALSE;
		break;
		
	    case 'm':
		skip_initial_menus = TRUE;
		break;
		
	    default:
		printf ("\nOption %c not implemented yet! Ignored.", c);
		break;
	}			/* switch(c) */
    }				/* while(1) */

    //--------------------
    // If the user is using SDL for the graphics output, then no other
    // screen resolutions than 640x480 will be available.
    //
    if ( ( ! use_open_gl ) && ( GameConfig . screen_width != 640 ) )
    {
	GameConfig . screen_width = 640; 
	GameConfig . screen_height = 480 ;
	GameConfig . next_time_width_of_screen = 640; 
	GameConfig . next_time_height_of_screen = 480 ;
	ErrorMessage ( __FUNCTION__  , "\
You are using SDL instead of OpenGL for graphics output.  For this\n\
output method, no other screen resolutions than 640x480 is available.\n\
Therefore your setting will be overridden and 640x480 will be used.\n\
If you want different resolutions, please use OpenGL for graphics\n\
output.",
				   NO_NEED_TO_INFORM , IS_WARNING_ONLY );
    }

    //--------------------
    // By default, after starting up, the current resolution should be
    // the resolution used at the next game startup too, so we preselect
    // that for now.  The user can still change that later inside the
    // game from within the options menu.
    //
    GameConfig . next_time_width_of_screen  = GameConfig . screen_width ;
    GameConfig . next_time_height_of_screen = GameConfig . screen_height ;

}; // ParseCommandLine (int argc, char *const argv[])


/* ----------------------------------------------------------------------
 * Now we initialize the skills of the new hero...
 * ---------------------------------------------------------------------- */
void
InitInfluencerStartupSkills( void )
{
    int i ;
    
    Me.readied_skill = 0;
    for ( i = 0 ; i < number_of_skills ; i ++ ) 
    {
	Me.SkillLevel [ i ] = SpellSkillMap[ i ] . present_at_startup ;
	Me.base_skill_level [ i ] = SpellSkillMap[ i ] . present_at_startup ;
    }
    
    GameConfig.spell_level_visible = 0;
    
    Me . melee_weapon_skill = 0 ;
    Me . ranged_weapon_skill = 0 ;
    Me . spellcasting_skill = 0 ;
    Me . hacking_skill = 0 ;

    Me . running_power_bonus = 0 ;

}; // void InitInfluencerStartupSkills( )

/* ----------------------------------------------------------------------
 * Now we disable all chat flags (i.e. the Tux hasn't spoken to
 * that person at all) for all the non-player-characters in the game,
 * except for the 0-chat alternative, which is always set to open.  WHY???????
 * ---------------------------------------------------------------------- */
void
InitInfluencerChatFlags( void )
{
  int i , j;

  for ( i = 0 ; i < MAX_PERSONS ; i ++ ) 
    {
      for ( j = 0 ; j < MAX_ANSWERS_PER_PERSON ; j ++ )
	{
	  Me . Chat_Flags [ i ] [ j ] = 0 ;
	}
//	  Me . Chat_Flags [ i ] [ END_ANSWER ] = 1 ;
    }

}; // void InitInfluencerChatFlags( )

/* ----------------------------------------------------------------------
 * When a completely fresh and new game is started, some more or less
 * harmless status variables need to be initialized.  This is what is
 * done in here.
 * ---------------------------------------------------------------------- */
void
InitHarmlessTuxStatusVariables( )
{
    int i;
    
    Me . type = DRUID001;
    Me . current_game_date = 0.0 ;
    Me . current_power_bonus = 0 ;
    Me . power_bonus_end_date = (-1); // negative dates are always in the past...
    Me . current_dexterity_bonus = 0 ;
    Me . dexterity_bonus_end_date = (-1); // negative dates are always in the past...
    Me . speed.x = 0;
    Me . speed.y = 0;
    Me . energy = 5 ;
    Me . maxenergy = 10 ;
    Me . temperature = 5 ;
    Me . max_temperature = 10 ;
    Me . health_recovery_rate = 0.2;
    Me . cooling_rate = 0.2;
    Me . status = MOBILE;
    Me . phase = 0;
    Me . MissionTimeElapsed=0;
    Me . Current_Victim_Resistance_Factor=1;
    Me . FramesOnThisLevel=0;
    Me . weapon_swing_time = (-1);  // currently not swinging this means...
    Me . got_hit_time = (-1);  // currently not stunned and needing time to recover...
    Me . points_to_distribute = 0;
    Me . ExpRequired = 1500;
    Me . map_maker_is_present = FALSE ;
    for ( i = 0 ; i < 1000 ; i ++ ) 
    {
	Me . KillRecord [ i ] = 0;
    }
    for ( i = 0 ; i < MAX_LEVELS ; i ++ ) 
    {
	Me . HaveBeenToLevel [ i ] = FALSE ;
	Me . time_since_last_visit_or_respawn [ i ] = (-1) ;
    }
    Me . Experience = 1;
    Me . exp_level = 1;
    Me . Gold = 0 ; 

}; // void InitHarmlessTuxStatusVariables( )

/* -----------------------------------------------------------------
 * This function initializes a completely new game within freedroid.
 * In contrast to InitFreedroid, this function should be called 
 * whenever or better before any new game is started.
 * -----------------------------------------------------------------*/
void
PrepareStartOfNewCharacter ( void )
{
    int i , j ;
    int StartingLevel=0;
    int StartingXPos=0;
    int StartingYPos=0;
    int MissionTargetIndex = 0;
    location StartPosition;
    
    //--------------------
    // At first we do the things that must be done for all
    // missions, regardless of mission file given
    //
    Activate_Conservative_Frame_Computation();
    ThisMessageTime = 0;
    LevelDoorsNotMovedTime = 0.0;
    RespectVisibilityOnMap = TRUE ;
    global_ingame_mode = GLOBAL_INGAME_MODE_NORMAL ;
    
    //--------------------
    // We mark all the big screen messages for this character
    // as out of date, so they can be overwritten with new 
    // messages...
    //
    Me . BigScreenMessageIndex = 0 ;
    for ( i = 0 ; i < MAX_BIG_SCREEN_MESSAGES ; i ++ )
	Me . BigScreenMessageDuration [ i ] = 10000 ;
    
    //--------------------
    // We make sure we don't have garbage in our arrays from a 
    // previous game or failed load-game attempt...
    //
    clear_out_arrays_for_fresh_game ( );
    
    //--------------------
    // Now the mission file is read into memory.  That means we can start to decode the details given
    // in the body of the mission file.  
    //
    GetEventsAndEventTriggers ( "EventsAndEventTriggers" );
    
    if ( !skip_initial_menus )
	PlayATitleFile ( "StartOfGame.title" );
    
    //--------------------
    // We also load the comment for the influencer to say at the beginning of the mission
    //
    Me . TextToBeDisplayed = _("Huh? What?  Where am I?");
    Me . TextVisibleTime = 0;
    
    //--------------------
    // initialize enemys according to crew file */
    // WARNING!! THIS REQUIRES THE freedroid.ruleset FILE TO BE READ ALREADY, BECAUSE
    // ROBOT SPECIFICATIONS ARE ALREADY REQUIRED HERE!!!!!
    //
    GetCrew ( "ReturnOfTux.droids" ) ;
    
    // ResolveMapLabelOnShip ( "TuxStartGameSquare" , &StartPosition );
    ResolveMapLabelOnShip ( "NewTuxStartGameSquare" , &StartPosition );
    Me . pos . x = StartPosition . x ;
    Me . pos . y = StartPosition . y ;
    Me . pos . z = StartPosition . level ;
    
    Me . teleport_anchor . x = 0 ; //no anchor at the beginning
    Me . teleport_anchor . y = 0 ;
    Me . teleport_anchor . z = 0 ;
    
    DebugPrintf ( 1 , "\nFinal starting position: Level=%d XPos=%d YPos=%d." , StartingLevel, StartingXPos, StartingYPos );
    
    //--------------------
    // At this point the position history can be initialized
    //
    InitInfluPositionHistory( );
    
    //--------------------
    // Now we read in the mission targets for this mission
    // Several different targets may be specified simultaneously
    //
    clear_tux_mission_info ( );
    GetQuestList ( "QuestList_archetypes" );
    
    SwitchBackgroundMusicTo ( curShip.AllLevels [ Me . pos . z ] -> Background_Song_Name );
    
    InitHarmlessTuxStatusVariables( );
    
    InitInfluencerStartupSkills( );
    
    UpdateAllCharacterStats( );
    
    InitInfluencerChatFlags( );
    
    clear_out_intermediate_points ( ) ;
    
    for ( j = 0 ; j < MAX_COOKIES ; j ++ )
    {
	strcpy ( Me . cookie_list [ j ] , "" ) ;
    }
    
    //--------------------
    // Now that the prime character stats have been initialized, we can
    // set these much-varying variables too...
    //
    Me . energy = Me . maxenergy;
    Me . temperature = 0;
    Me . running_power = Me . max_running_power ;
    Me . busy_time = 0 ;
    Me . busy_type = NONE ;
    
    Me . TextVisibleTime = 0;
    Me . readied_skill = 0;
    Me . walk_cycle_phase = 0 ;
    Me . TextToBeDisplayed = _("Linux Kernel booted.  001 transfer-tech modules loaded.  System up and running.");
    
    //--------------------
    // None of the inventory slots like currently equipped weapons
    // or the like should be held in hand, like when you take it
    // 'into your hand' by clicking on it with the mouse button in
    // the inventory screen.
    //
    Me . weapon_item  . type = -1;
    Me . armour_item  . type = -1; 
    Me . shield_item  . type = -1; 
    Me . special_item . type = -1; 
    Me . drive_item   . type = -1; 
    Me . weapon_item  . currently_held_in_hand = FALSE;
    Me . armour_item  . currently_held_in_hand = FALSE;
    Me . shield_item  . currently_held_in_hand = FALSE;
    Me . special_item . currently_held_in_hand = FALSE;
    Me . drive_item   . currently_held_in_hand = FALSE;
    Item_Held_In_Hand = ( -1 );
    
    
    DebugPrintf ( 1 , "\n%s():  Shuffling droids on all %d levels!" , __FUNCTION__ , curShip.num_levels );
    for ( i = 0 ; i < curShip.num_levels ; i ++ )
    {
	ShuffleEnemys( i );
    }
    
    
    //--------------------
    // Now we start those missions, that are to be assigned automatically to the
    // player at game start
    //
    for ( MissionTargetIndex = 0 ; MissionTargetIndex < MAX_MISSIONS_IN_GAME ; MissionTargetIndex ++ )
    {
	if ( Me.AllMissions[ MissionTargetIndex ].AutomaticallyAssignThisMissionAtGameStart ) 
	{
	    AssignMission( MissionTargetIndex );
	}
    }
    
    Me . mouse_move_target . x = ( -1 ) ;
    Me . mouse_move_target . y = ( -1 ) ;
    Me . mouse_move_target . z = ( -1 ) ;
    Me . current_enemy_target = NULL ;
    Me . mouse_move_target_combo_action_type = NO_COMBO_ACTION_SET ; // what extra action has to be done upon arrival?
    Me . mouse_move_target_combo_action_parameter = (-1) ; // extra data to use for the combo action
    
    //--------------------
    // Now we know that right after starting a new game, the Tux might have
    // to 'change clothes' i.e. a lot of tux images need to be updated which can
    // take a little time.  Therefore we print some message so the user will not
    // panic and push the reset button :)
    //
    PutStringFont ( Screen , Menu_BFont , ( GameConfig . screen_width / 2 ) - 180 , ( GameConfig . screen_height / 2 ) - 70 , _("Updating Tux images") );
    PutStringFont ( Screen , Menu_BFont , ( GameConfig . screen_width / 2 ) - 250 , ( GameConfig . screen_height / 2 ) - 30 , _("(this may take a little while...)") );
    our_SDL_flip_wrapper ( Screen );
    
    append_new_game_message ( _("Starting new game.") );
}; // void PrepareStartOfNewCharacter ( char* MissionName )

/* ----------------------------------------------------------------------
 * This function sets the GameConfig back to the default values, NOT THE
 * VALUES STORED IN THE USERS CONFIG FILE.  This function is useful if 
 * no config file if found or if the config file turns out to originate
 * from a different version of freedroid, which could be dangerous as
 * well.
 * ---------------------------------------------------------------------- */
void 
ResetGameConfigToDefaultValues ( void )
{
    //--------------------
    // At first we set audio volume to maximum value.
    // This might be replaced later with values from a 
    // private user Freedroid config file.  But for now
    // this code is good enough...
    //
    GameConfig . Current_BG_Music_Volume=0.5;
    GameConfig . Current_Sound_FX_Volume=0.5;
    GameConfig . current_gamma_correction = 1.00 ;
    GameConfig . WantedTextVisibleTime = 3;
    GameConfig . Draw_Framerate=FALSE;
    GameConfig . All_Texts_Switch=TRUE;
    GameConfig . Enemy_Hit_Text=FALSE;
    GameConfig . Enemy_Bump_Text=TRUE;
    GameConfig . Enemy_Aim_Text=TRUE;
    GameConfig . Influencer_Refresh_Text=FALSE;
    GameConfig . Influencer_Blast_Text=TRUE;
    GameConfig . Draw_Framerate=TRUE;
    GameConfig . Draw_Energy=FALSE;
    GameConfig . Draw_Position=FALSE;
    GameConfig . All_Texts_Switch = FALSE;
    GameConfig . terminate_on_missing_speech_sample = FALSE ;
    GameConfig . show_subtitles_in_dialogs = TRUE ;
    GameConfig . enemy_energy_bars_visible = TRUE ;
    GameConfig . hog_CPU = TRUE ;
    GameConfig . highlighting_mode_full = TRUE ;
    GameConfig . skip_light_radius = FALSE ; 
    GameConfig . omit_tux_in_level_editor = TRUE ;
    GameConfig . omit_obstacles_in_level_editor = FALSE ;
    GameConfig . omit_enemies_in_level_editor = TRUE ;
    GameConfig . zoom_is_on = FALSE ;
    GameConfig . show_blood = TRUE ;
    GameConfig . show_tooltips = TRUE;
    GameConfig . tux_image_update_policy = TUX_IMAGE_UPDATE_EVERYTHING_AT_ONCE ;
    GameConfig . number_of_big_screen_messages = 4 ;
    GameConfig . delay_for_big_screen_messages = 6.5 ;
    GameConfig . enable_cheatkeys = FALSE ;
    GameConfig . automap_manual_shift_x = 0 ;
    GameConfig . automap_manual_shift_y = 0 ;
    GameConfig . automap_display_scale = 2.0 ;
    GameConfig . skip_shadow_blitting = FALSE ;
 
    #if ENABLE_NLS
    char *lang = setlocale(LC_MESSAGES, NULL);

    if(lang && strstr(lang, "fr")) 
	GameConfig . language = 2;
    else if(lang && strstr(lang, "de"))
	GameConfig . language = 1;
    else
	GameConfig . language = 0;
    #else
	GameConfig . language = 0;
    #endif

    GameConfig . auto_display_to_help = 1;
}; // void Reset_GameConfig_To_Default_Values ( void )

/* -----------------------------------------------------------------
 * This function initializes the whole Freedroid game.
 * 
 * THIS MUST NOT BE CONFUSED WITH INITNEWGAME, WHICH
 * ONLY INITIALIZES A NEW MISSION FOR THE GAME.
 *  
 * ----------------------------------------------------------------- */
void
InitFreedroid ( int argc, char ** argv )
{
#ifndef USE_SDL_FRAMERATE
    struct timeval timestamp;
#endif
    struct stat statbuf;

    global_ingame_mode = GLOBAL_INGAME_MODE_NORMAL ;

    //--------------------
    // We want DisplayChar to produce visible results by default...
    //
    display_char_disabled = FALSE ;

    //--------------------
    // We mention the version of FreedroidRPG, so that debug reports
    // are easier to assign to the different versions of the game.
    //
    DebugPrintf ( -4 , "\nHello, this is FreedroidRPG, version %s." , VERSION );
		  

#ifndef __WIN32__

    //--------------------
    // Let's see if we're dealing with a real release or rather if
    // we're dealing with a cvs version.  The difference is this:
    // Releases shouldn't terminate upon a floating point exception
    // while the current cvs code (for better debugging) should
    // do so.  Therefore we check for 'cvs' in the current version
    // string and enable/disable the exceptions accordingly...
    //
    if ( strstr ( VERSION , "cvs" ) != NULL )
    {
	DebugPrintf ( -4 , "\nThis seems to be a cvs version, so we'll exit on floating point exceptions." );
	// feenableexcept ( FE_ALL_EXCEPT );
	// feenableexcept ( FE_INEXACT ) ;
	// feenableexcept ( FE_UNDERFLOW ) ;
	// feenableexcept ( FE_OVERFLOW ) ;
	feenableexcept ( FE_INVALID ) ;
	feenableexcept ( FE_DIVBYZERO ) ;
    }
    else
    {
	DebugPrintf ( -4 , "\nThis seems to be a 'stable' release, so no exit on floating point exceptions." );
	fedisableexcept ( FE_INVALID ) ;
	fedisableexcept ( FE_DIVBYZERO ) ;
    }
#endif
    
    /*
      if ( feraiseexcept ( FE_ALL_EXCEPT ) != 0 )
      {
      DebugPrintf ( -100 , "\nCouldn't set floating point exceptions to be raised...\nTerminating..." );
      exit ( 0 );
      }
      else
      {
      DebugPrintf ( -100 , "\nFloating point exceptions to be raised set successfully!\n" );
      }
    */
    /*
      test_float_1 = 3.1 ;
      test_float_2 = 0.0 ; 
      test_float_3 = test_float_1 / test_float_2 ;
    */
    

    // feenableexcept ( FE_ALL_EXCEPT );
    // feenableexcept ( FE_DIVBYZERO | FE_INVALID ); // FE_INEXACT | FE_UNDERFLOW | FE_OVERFLOW 
    // fesetexceptflag (const fexcept_t *flagp, int excepts);


#ifdef ENABLE_NLS
#include <locale.h>
    //--------------------
    // Portable localization
	setlocale(LC_COLLATE, "C");
	setlocale(LC_MONETARY, "C");
	setlocale(LC_NUMERIC, "C");
	setlocale(LC_TIME, "C");
#endif

    //--------------------
    // We hack the default signal handlers to print out a backtrace
    // in case of a fatal error of type 'segmentation fault' or the
    // like...
    //
    implant_backtrace_into_signal_handlers ( ) ;
    
    GameConfig . level_editor_edit_mode = LEVEL_EDITOR_SELECTION_FLOOR ;

    init_character_descriptions ( );
    
    clear_out_arrays_for_fresh_game ();

    RespectVisibilityOnMap = TRUE ; 
    timeout_from_item_drop = 0 ; 
    
    global_ignore_doors_for_collisions_flag = FALSE ;
    
    Overall_Average = 0.041 ;
    SkipAFewFrames = 0;
    Me . TextToBeDisplayed = "";
    
    InventorySize.x = INVENTORY_GRID_WIDTH ;
    InventorySize.y = INVENTORY_GRID_HEIGHT ;
    
    ResetGameConfigToDefaultValues ();

#if __WIN32__
    our_homedir = ".";
#else
    // first we need the user's homedir for loading/saving stuff
    if ( ( our_homedir = getenv("HOME")) == NULL )
    {
	DebugPrintf ( 0 , "WARNING: Environment does not contain HOME variable...\n\
I will try to use local directory instead\n");
	our_homedir = ".";
    }
#endif
    
    our_config_dir = MyMalloc( strlen ( our_homedir ) + 20 );
    sprintf ( our_config_dir , "%s/.freedroid_rpg" , our_homedir);
    
    if ( stat ( our_config_dir , &statbuf) == -1) 
    {
	DebugPrintf ( 0 , "\n----------------------------------------------------------------------\n\
You seem not to have the directory %s in your home directory.\n\
This directory is used by freedroid to store saved games and your personal settings.\n\
So I'll try to create it now...\n\
----------------------------------------------------------------------\n", our_config_dir );
#if __WIN32__
	_mkdir ( our_config_dir );
	DebugPrintf ( 1 , "ok\n" );
#else
	if ( mkdir ( our_config_dir , S_IREAD|S_IWRITE|S_IEXEC) == -1)
	{
	    DebugPrintf ( 0 , "\n----------------------------------------------------------------------\n\
WARNING: Failed to create config-dir: %s. Giving up...\n\
I will not be able to load or save games or configurations\n\
----------------------------------------------------------------------\n", our_config_dir);
	    free ( our_config_dir );
	    our_config_dir = NULL;
	}
	else
	{
	    DebugPrintf ( 1 , "ok\n" );
	}
#endif
    }

    GameConfig . screen_width = 800;
    GameConfig . screen_height = 600;
    LoadGameConfig ();

    ParseCommandLine ( argc, argv );
    //--------------------
    // Adapt button positions for the current screen resolution.  (Note: At this
    // point the command line has been evaluated already, therefore we know if OpenGL
    // is used or not and also which screen resolution to use.
    //
    adapt_button_positions_to_screen_resolution();

    Copy_Rect (Full_User_Rect, User_Rect);
    
    InitTimer ();
    
    InitVideo ();
    
    init_keyboard_input_array();
    ShowStartupPercentage ( 2 ) ; 
    
    InitAudio ();
    
    ShowStartupPercentage ( 8 ) ; 
    
    //--------------------
    // Now that the music files have been loaded successfully, it's time to set
    // the music and sound volumes accoridingly, i.e. as specifies by the users
    // configuration.
    //
    // THIS MUST NOT BE DONE BEFORE THE SOUND SAMPLES HAVE BEEN LOADED!!
    //
    SetSoundFXVolume( GameConfig.Current_Sound_FX_Volume );
    
    Init_Joy ();
    
    ShowStartupPercentage ( 10 ) ; 
    
    //--------------------
    // Now we prepare the automap data for later use
    //
    GameConfig . Automap_Visible = TRUE;
    
    ShowStartupPercentage ( 14 ) ; 
    
    Init_Game_Data(); 
    
    ShowStartupPercentage ( 16 ) ; 
    
    /* 
     * Initialise random-number generator in order to make 
     * level-start etc really different at each program start
     */
    srand(time(NULL));
    
    MinMessageTime = 55;
    MaxMessageTime = 850;
    
    CurLevel = NULL; // please leave this here BEFORE InitPictures
  
    InitPictures ( ) ;
 
    ShowStartupPercentage ( 100 ) ; 

    if(GameConfig . screen_width == 640)
	 GiveMouseAlertWindow ( "\nYou are playing in 640x480.\n\nWhile this resolution works correctly and will\ngive you a great gaming experience, its support is\nin the process of being dropped, therefore you will\nwant to consider using 800x600 or 1024x768.\n\nThank you.\n");
    if ( strstr( VERSION, "rc" ) )
	 GiveMouseAlertWindow ( "\nYou are playing a release candidate version.\nMany strange bugs might still be present in the game.\nPlease report anything you may find to #freedroid at irc.freenode.net, or\n by mail to freedroid-discussion at lists.sourceforge.net\nThank you for helping us test the game.\nGood luck!\n");

    
}; // void InitFreedroid ( void ) 

/* ----------------------------------------------------------------------
 * This function displayes the last seconds of the game when the influencer
 * has actually been killed.  It generates some explosions and waits for
 * some seconds, where the user can reload his latest game, or after that
 * returns to finally quit the inner game loop and the program will 
 * (outside this function) ask for a completely new game or loading a different
 * saved game or quit as in the very beginning of the game.
 * ---------------------------------------------------------------------- */
void
ThouArtDefeated (void)
{
    int j;
    int now;

    DebugPrintf ( 1 , "\n%s(): Real function call confirmed." , __FUNCTION__ );
    Me . status = INFOUT ;
    append_new_game_message ( _("Game over.\n") );
    GameConfig . Inventory_Visible = FALSE;
    GameConfig . CharacterScreen_Visible = FALSE;
    GameConfig . Mission_Log_Visible = FALSE;
    ThouArtDefeatedSound ( ) ;
    start_tux_death_explosions ( ) ;
    now = SDL_GetTicks ( ) ;
    global_ingame_mode = GLOBAL_INGAME_MODE_NORMAL;

    //--------------------
    // Now that the influencer is dead, all this precious items
    // spring off of him...
    //
    if ( Me . weapon_item  . type > 0 )
    {
	DropItemAt ( Me . weapon_item  . type , Me . pos . z , 
		     Me . pos . x - 0.5 , Me . pos . y - 0.5 , -1 , -1 ,  1 );
    }
    if ( Me . drive_item . type > 0 )
    {
	DropItemAt ( Me . drive_item   . type , Me . pos . z , 
		     Me . pos . x + 0.5 , Me . pos . y - 0.5 , -1 , -1 , 1 );
    }
    if ( Me . shield_item . type > 0 )
    {
	DropItemAt ( Me . shield_item  . type , Me . pos . z , 
		     Me . pos . x + 0.5 , Me . pos . y + 0.5 , -1 , -1 , 1 );
    }
    if ( Me . armour_item . type > 0 )
    {
	DropItemAt ( Me . armour_item  . type , Me . pos . z , 
		     Me . pos . x - 0.5 , Me . pos . y + 0.5 , -1 , -1 , 1 );
    }
    if ( Me . special_item . type > 0 )
    {
	DropItemAt ( Me . special_item . type , Me . pos . z , 
		     Me . pos . x - 0.5 , Me . pos . y       , -1 , -1 , 1 );
    }
    if ( Me . Gold > 0 )
    {
	DropItemAt ( GetItemIndexByName("Cyberbucks"), Me . pos . z , 
		     Me . pos . x       , Me . pos . y       , -1 , -1 , 1 );
    }

    
    GameOver = TRUE;
    
    while ( ( SDL_GetTicks() - now < 1000 * WAIT_AFTER_KILLED ) && ( GameOver == TRUE ) )
    {
	StartTakingTimeForFPSCalculation(); 
	
	AssembleCombatPicture ( DO_SCREEN_UPDATE | SHOW_ITEMS | USE_OWN_MOUSE_CURSOR );
	DisplayBanner ( );
	animate_blasts ();
	MoveBullets ();
	MoveEnemys ();
	MoveLevelDoors ( );	
	
	ReactToSpecialKeys();
	
	for (j = 0; j < MAXBULLETS; j++)
	    CheckBulletCollisions (j);
	
	ComputeFPSForThisFrame();
	
    }
    
    //--------------------
    // The automap doesn't need to be shown any more and also when
    // the next game starts up (on the same level as right now) there
    // should not be any automap info remaining...
    //
    clear_automap_texture_completely (  ) ;

    DebugPrintf ( 2 , "\n%s():  Usual end of function reached." , __FUNCTION__ );

}; // void ThouArtDefeated(void)

/* ----------------------------------------------------------------------
 * This function displayes the last seconds of the game when the influencer
 * has actually been killed.  It generates some explosions and waits for
 * some seconds, where the user can reload his latest game, or after that
 * returns to finally quit the inner game loop and the program will 
 * (outside this function) ask for a completely new game or loading a different
 * saved game or quit as in the very beginning of the game.
 * ---------------------------------------------------------------------- */
void
ThouHastWon (void)
{
    int j;
    int now;

    DebugPrintf ( 1 , "\n%s(): Real function call confirmed." , __FUNCTION__ );
    Me . status = INFOUT ;
    append_new_game_message ( _("Game won.\n") );
    GameConfig . Inventory_Visible = FALSE;
    GameConfig . CharacterScreen_Visible = FALSE;
    GameConfig . Mission_Log_Visible = FALSE;
    now = SDL_GetTicks ( ) ;

    GameOver = TRUE;
    
    while ( ( SDL_GetTicks() - now < 1000 * WAIT_AFTER_GAME_WON ) && ( GameOver == TRUE ) )
    {
	StartTakingTimeForFPSCalculation(); 
	
	AssembleCombatPicture ( DO_SCREEN_UPDATE | SHOW_ITEMS | USE_OWN_MOUSE_CURSOR );
	DisplayBanner ( );
	animate_blasts ();
	MoveBullets ();
	MoveEnemys ();
	MoveLevelDoors ( );	
	
	// ReactToSpecialKeys();
	
	for (j = 0; j < MAXBULLETS; j++)
	    CheckBulletCollisions (j);
	
	ComputeFPSForThisFrame();
	
    }

    //--------------------
    // Now it's time for the end game title file...
    //
    PlayATitleFile ( "EndOfGame.title" );

    //--------------------
    // Now it's time for the credits file
    //
    Credits_Menu();

    //--------------------
    // The automap doesn't need to be shown any more and also when
    // the next game starts up (on the same level as right now) there
    // should not be any automap info remaining...
    //
    clear_automap_texture_completely (  ) ;

    DebugPrintf ( 2 , "\n%s():  Usual end of function reached." , __FUNCTION__ );

}; // void ThouHastWon(void)

#undef _init_c
