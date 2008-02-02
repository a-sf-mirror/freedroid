/* 
 *
 *
 *   Copyright (c) 2003 Johannes Prix
 *   Copyright (c) 2004-2007 Arthur Huillet 
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
 * This file contains some text code, that is also needed in the 
 * dialog editor.
 * ---------------------------------------------------------------------- */

#define _text_public_c

#include "system.h"

#include "defs.h"
#include "struct.h"
#include "proto.h"
#include "global.h"

void load_item_surfaces_for_item_type ( int item_type );

extern int Number_Of_Item_Types;
extern int debug_level ;
extern dialogue_option ChatRoster[MAX_DIALOGUE_OPTIONS_IN_ROSTER];

/* ---------------------------------------------------------------------- 
 * This function works a malloc, except that it also checks for
 * success and terminates in case of "out of memory", so we dont
 * need to do this always in the code.
 * ---------------------------------------------------------------------- */
void *
MyMalloc ( long Mamount )
{
    void *Mptr = NULL;
    
    // make Gnu-compatible even if on a broken system:
    if (Mamount == 0)
	Mamount = 1;
  
    if ((Mptr = calloc (1, (size_t) Mamount)) == NULL)
    {
	fprintf (stderr, " MyMalloc(%ld) did not succeed!\n", Mamount);
	fflush (stderr);
	Terminate(ERR);
    }

  
    return Mptr;

}; // void* MyMalloc ( long Mamount )

/* ----------------------------------------------------------------------
 * This function is used for debugging purposes.  It writes the
 * given string on the screen, or simply does
 * nothing according to currently set debug level.
 * ---------------------------------------------------------------------- */
void
DebugPrintf ( int db_level, const char *fmt, ...)
{
    va_list args;
    va_start (args, fmt);
    
    if (db_level <= debug_level)
    {
	vfprintf (stderr, fmt, args);
    }
    
    va_end (args);
}; // void DebugPrintf ( int db_level, char *fmt, ...)

/* ----------------------------------------------------------------------
 * This function should help to simplify and standardize the many error
 * messages possible in Freedroid RPG.
 * ---------------------------------------------------------------------- */
void
ErrorMessage ( const char* FunctionName , const char* fmt, int InformDevelopers , int IsFatal, ... )
{
    va_list args;
    va_start(args, IsFatal);

    fprintf (stderr, "\n----------------------------------------------------------------------\n\
Freedroid has encountered a problem:\n" );
    fprintf (stderr, "In Function: %s.\n" , FunctionName );
    fprintf (stderr, "FreedroidRPG package and version number: %s %s.\n" , PACKAGE , VERSION );
    vfprintf (stderr, fmt, args );
    
    if ( InformDevelopers )
    {
	fprintf (stderr, "\
If you encounter this message, please inform the Freedroid developers\n\
about the problem, by sending e-mail to \n\
\n\
freedroid-discussion@lists.sourceforge.net\n\
\n\
Or you can mention it to someone of the developers on our\n\
IRC channel.  The channel is:\n\
\n\
channel: #freedroid on irc.freenode.net\n\
\n\
Thanks a lot!\n\n" );
    }
    
    if ( IsFatal )
    {
	fprintf (stderr, 
		 "Freedroid will terminate now to draw attention to the problems it could\n\
not resolve.  Sorry if that interrupts a major game of yours...\nBacktrace:\n" );
	print_trace( FREEDROID_INTERNAL_ERROR_SIGNAL );
	
    }
    else
    {
	fprintf (stderr, 
		 "The problem mentionned above is not fatal, we continue the execution.\n" );
    }
    
    fprintf (stderr, "----------------------------------------------------------------------\n" );
    
    va_end(args);

    if ( IsFatal ) Terminate ( ERR );
    
}; // void ErrorMessage ( ... )


/* ----------------------------------------------------------------------
 * This function does something similar to memmem.  Indeed, it would be
 * best perhaps if it did exactly the same as memmem, but since we do not
 * use gnu extensions for compatibility reasons, we go this way.
 *
 * Be careful with using this function for searching just one byte!!
 *
 * Be careful with using this function for non-string NEEDLE!!
 *
 * Haystack can of course have ANY form!!!
 *
 * ---------------------------------------------------------------------- */
void *
MyMemmem ( char *haystack, size_t haystacklen, char *needle, size_t needlelen)
{
    char* NextFoundPointer;
    void* MatchPointer;
    size_t SearchPos=0;
    
    DebugPrintf ( 3 , "%s(): haystack = %d, len = %d, needle=%s\n", __FUNCTION__ , haystack, haystacklen, needle);
    
    while ( haystacklen - SearchPos > 0 )
    {
	//--------------------
	// We search for the first match OF THE FIRST CHARACTER of needle
	//
	NextFoundPointer = memchr ( haystack+SearchPos , needle[0] , haystacklen-SearchPos );
	
	//--------------------
	// if not even that was found, we can immediately return and report our failure to find it
	//
	if ( NextFoundPointer == NULL ) return ( NULL );
	
	//--------------------
	// Otherwise we see, if also the rest of the strings match this time ASSUMING THEY ARE STRINGS!
	// In case of a match, we can return immediately
	//
	DebugPrintf (3, "calling strstr()..");
	MatchPointer = strstr( NextFoundPointer , needle );
	if( MatchPointer) DebugPrintf ( 3 , "..survived. MatchPointer = %d\n", MatchPointer );
	else DebugPrintf ( 3 , "survived, but nothing found\n" );
	if ( MatchPointer != NULL ) return ( MatchPointer );
	
	//--------------------
	// At this point, we know that we had no luck with this one occasion of a first-character-match
	// and must continue after this one occasion with our search
	SearchPos = NextFoundPointer - haystack + 1;
    }

    return( NULL );
}; // void *MyMemmem ( ... );


/* ----------------------------------------------------------------------
 * This function looks for a string begin indicator and takes the string
 * from after there up to a sting end indicator and mallocs memory for
 * it, copys it there and returns it.
 * The original source string specified should in no way be modified.
 * ---------------------------------------------------------------------- */
char*
ReadAndMallocStringFromData ( char* SearchString , const char* StartIndicationString , const char* EndIndicationString ) 
{
  char* SearchPointer;
  char* EndOfStringPointer;
  char* ReturnString = "" ;
  int StringLength;

  if ( (SearchPointer = strstr ( SearchString , StartIndicationString )) == NULL )
    {
      fprintf( stderr, "\n\nStartIndicationString: '%s'\n" , StartIndicationString );
      ErrorMessage ( __FUNCTION__  , "\
The string that is supposed to prefix an entry in a text data file\n\
of Freedroid was not found within this text data file.\n\
This indicates some corruption in the data file in question.",
				 PLEASE_INFORM, IS_FATAL );
    }
  else
    {
      // Now we move to the beginning
      SearchPointer += strlen ( StartIndicationString );

      // Now we move to the end with the end pointer
      if ( (EndOfStringPointer = strstr( SearchPointer , EndIndicationString ) ) == NULL )
	{
	  fprintf( stderr, "\n\nEndIndicationString: '%s'\n" , EndIndicationString );
	  ErrorMessage ( __FUNCTION__  , "\
The string that is supposed to terminate an entry in a text data file\n\
of Freedroid was not found within this text data file.\n\
This indicates some corruption in the data file in question.",
				     PLEASE_INFORM, IS_FATAL );
	}

      // Now we allocate memory and copy the string...
      // delete_one_dialog_option() doesn't free empty strings so don't
      // malloc those.
      if ( ( StringLength = (EndOfStringPointer - SearchPointer) ) )
	{
	  ReturnString = MyMalloc ( StringLength + 1 );
	  strncpy ( ReturnString , SearchPointer , StringLength );
	  ReturnString[ StringLength ] = 0;
	}
      else
	{
	  ReturnString = "" ;
	}

      DebugPrintf( 2 , "\nchar* ReadAndMalocStringFromData (...): Successfully identified string : %s." , ReturnString );
    }
  return ( ReturnString );
}; // char* ReadAndMallocStringFromData ( ... )

/* ----------------------------------------------------------------------
 * This function counts the number of occurences of a string in a given
 * other string.
 * ---------------------------------------------------------------------- */
int
CountStringOccurences ( char* SearchString , const char* TargetString ) 
{
  int Counter=0;
  char* CountPointer;

  CountPointer = SearchString;

  while ( ( CountPointer = strstr ( CountPointer, TargetString ) ) != NULL)
    {
      CountPointer += strlen ( TargetString );
      Counter++;
    }
  return ( Counter );
}; // CountStringOccurences ( char* SearchString , char* TargetString ) 

/* ----------------------------------------------------------------------
 * This function read in a file with the specified name, allocated 
 * memory for it of course, looks for the file end string and then
 * terminates the whole read in file with a 0 character, so that it
 * can easily be treated like a common string.
 * ---------------------------------------------------------------------- */
char* 
ReadAndMallocAndTerminateFile( char* filename , const char* File_End_String ) 
{
    FILE *DataFile;
    char *Data;
    char *ReadPointer;
    long MemoryAmount;

    DebugPrintf ( 1 , "\n%s() : The filename is: %s" , __FUNCTION__ , filename );

    //--------------------
    // Read the whole theme data to memory.  We use binary mode, as we
    // don't want to have to deal with any carriage return/line feed 
    // convention mess on win32 or something...
    // 
    if ( ( DataFile = fopen ( filename , "rb") ) == NULL )
    {
	fprintf( stderr, "\n\nfilename: '%s'\n" , filename );

	ErrorMessage ( __FUNCTION__  , "\
Freedroid was unable to open a given text file,\n\
that should be there and should be accessible.\n\
This indicates a serious bug in this installation of Freedroid.",
				   PLEASE_INFORM, IS_FATAL );
    }
    else
    {
	DebugPrintf ( 1 , "\nchar* ReadAndMallocAndTerminateFile ( char* filename ) : Opening file succeeded...");
    }
    
    MemoryAmount = FS_filelength( DataFile )  + 64*2 + 10000;
    Data = (char *) MyMalloc ( MemoryAmount );
    
    fread ( Data, MemoryAmount, 1, DataFile);

    DebugPrintf ( 1 , "\n%s(): Reading file succeeded..." , __FUNCTION__ );

    if ( fclose ( DataFile ) == EOF)
    {
	fprintf( stderr, "\n\nfilename: '%s'\n" , filename );
	ErrorMessage ( __FUNCTION__  , "\
Freedroid was unable to close a given text file, that should be there and\n\
should be accessible.\n\
This indicates a strange bug in this installation of Freedroid, that is\n\
very likely a problem with the file/directory permissions of the files\n\
belonging to Freedroid.",
				   PLEASE_INFORM, IS_FATAL );
    }
    else
    {
	DebugPrintf( 1 , "\n%s(): file closed successfully...\n" , __FUNCTION__ );
    }
    
    // NOTE: Since we do not assume to always have pure text files here, we switched to
    // MyMemmem, so that we can handle 0 entries in the middle of the file content as well
    //
    // if ( (ReadPointer = strstr( Data , File_End_String ) ) == NULL )
    if ( ( ReadPointer = MyMemmem ( Data, (size_t) MemoryAmount , (char*)File_End_String , (size_t)strlen( File_End_String ))) == NULL)
    {
	fprintf( stderr, "\n\nfilename: '%s'\n" , filename );
	fprintf( stderr, "File_End_String: '%s'\n" , File_End_String );

	ErrorMessage ( __FUNCTION__  , "\
Freedroid was unable to find the string, that should indicate the end of\n\
the given text file within this file.\n\
This indicates a corrupt or outdated data or saved game file.", PLEASE_INFORM, IS_FATAL );
    }
  else
    {
      // ReadPointer+=strlen( File_End_String ) + 1; // no need to destroy the end pointer :-)
      ReadPointer[0]=0; // we want to handle the file like a string, even if it is not zero
                       // terminated by nature.  We just have to add the zero termination.
    }

  // DebugPrintf( 1 , "\nchar* ReadAndMallocAndTerminateFile ( char* filename ) : The content of the read file: \n%s" , Data );

  return ( Data );
}; // char* ReadAndMallocAndTerminateFile( char* filename) 

/* ----------------------------------------------------------------------
 * This function tries to locate a string in some given data string.
 * The data string is assumed to be null terminated.  Otherwise SEGFAULTS
 * might happen.
 * 
 * The return value is a pointer to the first instance where the substring
 * we are searching is found in the main text.
 * ---------------------------------------------------------------------- */
char* 
LocateStringInData ( char* SearchBeginPointer, const char* SearchTextPointer )
{
  char* temp;

  if ( ( temp = strstr ( SearchBeginPointer , SearchTextPointer ) ) == NULL)
    {
      fprintf( stderr, "\n\nSearchTextPointer: '%s'\n" , SearchTextPointer );
      ErrorMessage ( __FUNCTION__  , "\
The string that was supposed to be in the text data file could not be found.\n\
This indicates a corrupted or seriously outdated game data or saved game file.",
				 PLEASE_INFORM, IS_FATAL );
    }

  return ( temp );

}; // char* LocateStringInData ( ... )

/* ----------------------------------------------------------------------
 * This function should analyze a given passage of text, locate an 
 * indicator for a value, and read in the value.
 * ---------------------------------------------------------------------- */
void
ReadValueFromStringWithDefault( char* SearchBeginPointer , const char* ValuePreceedText , const char* FormatString , const char * DefaultValueString, void* TargetValue , char* EndOfSearchSectionPointer )
{
    char OldTerminaterCharValue;
    const char* SourceLocation;

    // We shortly make a termination char into the string.
    OldTerminaterCharValue=EndOfSearchSectionPointer[0];
    EndOfSearchSectionPointer[0]=0;
    
    // Now we locate the spot, where we finally will find our value
    SourceLocation = strstr ( SearchBeginPointer , ValuePreceedText );
    if ( SourceLocation)
	    SourceLocation += strlen ( ValuePreceedText );
    else SourceLocation = DefaultValueString;
    
    //--------------------
    // Attention!!! 
    // Now we try to read in the value!!!
    //
    if ( sscanf ( SourceLocation , FormatString , TargetValue ) == EOF )
    {
	fprintf( stderr, "\n\nFormatString: '%s'\n" , FormatString );
	fprintf( stderr, "ValuePreceedText: '%s'\n" , ValuePreceedText );
	ErrorMessage ( __FUNCTION__  , "\
sscanf using a certain format string failed!\n\
This indicates a corrupted or seriously outdated game data or saved game file.",
				 PLEASE_INFORM, IS_FATAL );
    }
    
    // Now that we are done, we restore the given SearchArea to former glory
    EndOfSearchSectionPointer[0]=OldTerminaterCharValue;

}; // void ReadValueFromString( ... )

/* ----------------------------------------------------------------------
 * This function should analyze a given passage of text, locate an 
 * indicator for a value, and read in the value.
 * ---------------------------------------------------------------------- */
void
ReadValueFromString( char* SearchBeginPointer , const char* ValuePreceedText , const char* FormatString , void* TargetValue , char* EndOfSearchSectionPointer )
{
    char OldTerminaterCharValue;
    char* SourceLocation;

    // We shortly make a termination char into the string.
    OldTerminaterCharValue=EndOfSearchSectionPointer[0];
    EndOfSearchSectionPointer[0]=0;
    
    // Now we locate the spot, where we finally will find our value
    SourceLocation = LocateStringInData ( SearchBeginPointer , ValuePreceedText );
    SourceLocation += strlen ( ValuePreceedText );
    
    //--------------------
    // Attention!!! 
    // Now we try to read in the value!!!
    //
    if ( sscanf ( SourceLocation , FormatString , TargetValue ) == EOF )
    {
	fprintf( stderr, "\n\nFormatString: '%s'\n" , FormatString );
	fprintf( stderr, "ValuePreceedText: '%s'\n" , ValuePreceedText );
	ErrorMessage ( __FUNCTION__  , "\
sscanf using a certain format string failed!\n\
This indicates a corrupted or seriously outdated game data or saved game file.",
				 PLEASE_INFORM, IS_FATAL );
    }
    
    // Now that we are done, we restore the given SearchArea to former glory
    EndOfSearchSectionPointer[0]=OldTerminaterCharValue;

}; // void ReadValueFromString( ... )

/* ----------------------------------------------------------------------
 * This function does the rotation of a given vector by a given angle.
 * The vector is a vector.
 * The angle given is in DEGREE MEASURE, i.e. 0-360 or so, maybe more or
 * less than that, but you get what I mean...
 * ---------------------------------------------------------------------- */
void
RotateVectorByAngle ( moderately_finepoint* vector , float rot_angle )
{
    moderately_finepoint new_vect;
    float rad_angle;
    
    rad_angle = rot_angle * ( M_PI / 180.0 ) ; 
    
    DebugPrintf( 2 , "\n RAD_ANGLE : %f " , rad_angle );
    new_vect.x =  sin( rad_angle ) * vector->y + cos( rad_angle ) * vector->x;
    new_vect.y =  cos( rad_angle ) * vector->y - sin( rad_angle ) * vector->x;
    vector->x = new_vect.x;
    vector->y = new_vect.y;

}; // void RotateVectorByAngle ( ... )

/* ----------------------------------------------------------------------
 *
 * ---------------------------------------------------------------------- */
void
delete_one_dialog_option ( int i , int FirstInitialisation )
{
    int j;

    //--------------------
    // If this is not the first initialisation, we have to free the allocated
    // strings first, or we'll be leaking memory otherwise...
    //
    if ( !FirstInitialisation )
    {
	if ( strlen ( ChatRoster[i].option_text ) ) free ( ChatRoster[i].option_text );
	if ( strlen ( ChatRoster[i].option_sample_file_name ) ) free ( ChatRoster[i].option_sample_file_name );
    }
    ChatRoster[i].option_text="";
    ChatRoster[i].option_sample_file_name="";
    
    //--------------------
    // Now we can set the positions of the dialog boxes within the dialog editor
    // to 'empty' values.  This will remain completely without effect in FreedroidRPG.
    // The only one caring about these positions is the Dialog Editor.
    //
    ChatRoster[i].position_x = -1;
    ChatRoster[i].position_y = -1;
    
    for ( j = 0 ; j < MAX_REPLIES_PER_OPTION ; j++ )
    {
	//--------------------
	// If this is not the first initialisation, we have to free the allocated
	// strings first, or we'll be leaking memory otherwise...
	//
	if ( !FirstInitialisation )
	{
	    if ( strlen ( ChatRoster [ i ] . reply_sample_list [ j ] ) ) 
		free ( ChatRoster [ i ] . reply_sample_list [ j ] );
	    if ( strlen ( ChatRoster [ i ] . reply_subtitle_list [ j ] ) ) 
		free ( ChatRoster [ i ] . reply_subtitle_list [ j ] );
	}
	ChatRoster [ i ] . reply_sample_list [ j ] = "";
	ChatRoster [ i ] . reply_subtitle_list [ j ] = "";
    }
    
    for ( j = 0 ; j < MAX_EXTRAS_PER_OPTION ; j++ )
    {
	//--------------------
	// If this is not the first initialisation, we have to free the allocated
	// strings first, or we'll be leaking memory otherwise...
	//
	if ( !FirstInitialisation )
	{
	    if ( strlen ( ChatRoster [ i ] . extra_list [ j ] ) ) 
		free ( ChatRoster [ i ] . extra_list [ j ] );
	}
	ChatRoster [ i ] . extra_list [ j ] = "";
    }
    
    //--------------------
    // If this is not the first initialisation, we have to free the allocated
    // strings first, or we'll be leaking memory otherwise...
    //
    if ( !FirstInitialisation )
    {
	if ( strlen ( ChatRoster [ i ] . on_goto_condition ) ) 
	    free ( ChatRoster [ i ] . on_goto_condition );
    }
    ChatRoster [ i ] . on_goto_condition = "";
    ChatRoster [ i ] . on_goto_first_target = (-1);
    ChatRoster [ i ] . on_goto_second_target = (-1);
    ChatRoster [ i ] . link_target = (0);
    ChatRoster [ i ] . always_execute_this_option_prior_to_dialog_start = FALSE ;
    
    for ( j = 0 ; j < MAX_DIALOGUE_OPTIONS_IN_ROSTER ; j++ )
    {
	ChatRoster [ i ] . change_option_nr [ j ] = (-1); 
	ChatRoster [ i ] . change_option_to_value [ j ] = (-1); 
    }
}; // void delete_one_dialog_option ( int i , int FirstInitialisation )

/* ----------------------------------------------------------------------
 * This function should init the chat roster with empty values and thereby
 * clean out the remnants of the previous chat dialogue.
 * ---------------------------------------------------------------------- */
void
InitChatRosterForNewDialogue( void )
{
  int i;
  static int FirstInitialisation = TRUE;
  
  for ( i = 0 ; i < MAX_DIALOGUE_OPTIONS_IN_ROSTER ; i ++ )
    {
      delete_one_dialog_option ( i , FirstInitialisation );
    }

  //--------------------
  // Next time, we WILL have to free every used entry before cleaning it
  // out, or we will be leaking memory...
  //
  FirstInitialisation = FALSE ;

}; // void InitChatRosterForNewDialogue( void )

/* ----------------------------------------------------------------------
 * This function should load new chat dialogue information from the 
 * chat info file into the chat roster.
 *
 * ---------------------------------------------------------------------- */
void
LoadChatRosterWithChatSequence ( char* FullPathAndFullFilename )
{
    char *ChatData;
    char *SectionPointer;
    char *EndOfSectionPointer;
    char *NextChatSectionCode;
    int i , j ;
    int OptionSectionsLeft;
    char fpath[2048];
    int OptionIndex;
    int NumberOfOptionsInSection;
    char TempSavedCharacter = 'A' ;
    char *TempEndPointer = NULL ;
    int NumberOfReplySubtitles;
    int NumberOfReplySamples;
    int NumberOfOptionChanges;
    int NumberOfNewOptionValues;
    int NumberOfExtraEntries;
    
    int RestoreTempDamage;
    char* ReplyPointer;
    char* OptionChangePointer;
    char* ExtraPointer;
    char* YesNoString;
    
    sprintf(fpath, "%s", FullPathAndFullFilename);
    
    // #define END_OF_DIALOGUE_FILE_STRING "*** End of Dialogue File Information ***"
#define CHAT_CHARACTER_BEGIN_STRING "Beginning of new chat dialog for character=\""
#define CHAT_CHARACTER_END_STRING "End of chat dialog for character"
#define NEW_OPTION_BEGIN_STRING "New Option Nr="
    
    //--------------------
    // At first we read the whole chat file information into memory
    //
    ChatData = ReadAndMallocAndTerminateFile( fpath , CHAT_CHARACTER_END_STRING ) ;
    SectionPointer = ChatData ;
    
    //--------------------
    // Now we search for the desired chat section, cause most likely
    // there will be more than one person to chat in this chat file soon
    //
#define UNINITIALIZED_SECTION_CODE "NOPERSONATALL"
    NextChatSectionCode = UNINITIALIZED_SECTION_CODE ;
    SectionPointer = ChatData;
    
    //--------------------
    // Now we locate the end of this chat section and put a 
    // termination character there, so we can use string functions
    // conveniently on the given section.
    //
    // EndOfSectionPointer = LocateStringInData ( SectionPointer , CHAT_CHARACTER_END_STRING );
    // *EndOfSectionPointer = 0;
    //
    EndOfSectionPointer = SectionPointer + strlen ( SectionPointer );
    
    //--------------------
    // At first we go take a look on how many options we have
    // to decode from this section.
    //
    NumberOfOptionsInSection = CountStringOccurences ( SectionPointer , NEW_OPTION_BEGIN_STRING ) ;
    DebugPrintf( CHAT_DEBUG_LEVEL , "\nWe have counted %d Option entries in this section." , NumberOfOptionsInSection ) ;
    
    //--------------------
    // Now we see which option index is assigned to this option.
    // It may happen, that some numbers are OMITTED here!  This
    // should be perfectly ok and allowed as far as the code is
    // concerned in order to give the content writers more freedom.
    //
    for ( i = 0 ; i < NumberOfOptionsInSection; i ++ )
    {
	SectionPointer = LocateStringInData ( SectionPointer, NEW_OPTION_BEGIN_STRING );
	ReadValueFromString( SectionPointer , NEW_OPTION_BEGIN_STRING, "%d" , 
			     &OptionIndex , EndOfSectionPointer );
	DebugPrintf( CHAT_DEBUG_LEVEL , "\nFound New Option entry.  Index found is: %d. " , OptionIndex ) ;
	SectionPointer++;
	
	//--------------------
	// Now that we have the actual option index, we can start to
	// fill the roster with real information.  At first, this will be only
	// the Option string and sample
	//
	// Anything that is loaded into the chat roster doesn't need to be freed,
	// cause this will be done by the next 'InitChatRoster' function anyway.
	//
	ChatRoster[ OptionIndex ] . option_text = 
	    ReadAndMallocStringFromData ( SectionPointer , "OptionText=_\"" , "\"" ) ;
	DebugPrintf( CHAT_DEBUG_LEVEL , "\nOptionText found : \"%s\"." , ChatRoster[ OptionIndex ] . option_text );
	ChatRoster[ OptionIndex ] . option_sample_file_name = 
	    ReadAndMallocStringFromData ( SectionPointer , "OptionSample=\"" , "\"" ) ;
	DebugPrintf( CHAT_DEBUG_LEVEL , "\nOptionSample found : \"%s\"." , ChatRoster[ OptionIndex ] . option_sample_file_name );
	
	//--------------------
	// Now we can start to add all given Sample and Subtitle combinations
	// But first we must add a termination character in order to not use
	// the combinations of the next option section.
	// 
	if ( ( OptionSectionsLeft = CountStringOccurences ( SectionPointer , NEW_OPTION_BEGIN_STRING ) ) )
	{
	    DebugPrintf ( CHAT_DEBUG_LEVEL , "\nThere are still %d option sections in the file.  \n\
Therefore we must add a new temporary termination character in between." , OptionSectionsLeft );
	    TempEndPointer = LocateStringInData ( SectionPointer, NEW_OPTION_BEGIN_STRING );
	    TempSavedCharacter = *TempEndPointer;
	    *TempEndPointer = 0 ;
	    RestoreTempDamage = TRUE;
	}
	else
	{
	    DebugPrintf ( CHAT_DEBUG_LEVEL , "\nThere is no more option section left in the file.  \n\
Therefore we need not add an additional termination character now." );
	    RestoreTempDamage = FALSE;
	}
	
	//--------------------
	// Now that the temporary termination character has been inserted, we can 
	// start to hunt for position and other strings in the new terminated area...
	//
	if ( strstr ( SectionPointer , "PositionX=" ) != NULL )
	{
	    ReadValueFromString( SectionPointer , "PositionX=" , "%d" , 
				 & ( ChatRoster[ OptionIndex ] . position_x ) , TempEndPointer );
	    ReadValueFromString( SectionPointer , "PositionY=" , "%d" , 
				 & ( ChatRoster[ OptionIndex ] . position_y ) , TempEndPointer );
	}
	else
	{
	    ChatRoster [ OptionIndex ] . position_x = ( 2 + ( OptionIndex % 10 ) ) * 30 ;
	    ChatRoster [ OptionIndex ] . position_y = ( 2 + ( OptionIndex / 10 ) ) * 30 ;
	}
	
#define NEW_REPLY_SAMPLE_STRING "ReplySample=\""
#define NEW_REPLY_SUBTITLE_STRING "Subtitle=_\""
	
	//--------------------
	// We count the number of Subtitle and Sample combinations and then
	// we will read them out
	//
	NumberOfReplySamples = CountStringOccurences ( SectionPointer , NEW_REPLY_SAMPLE_STRING ) ;
	NumberOfReplySubtitles = CountStringOccurences ( SectionPointer , NEW_REPLY_SUBTITLE_STRING ) ;
	if ( NumberOfReplySamples != NumberOfReplySubtitles )
	{
	    fprintf( stderr, "\n\nNumberOfReplySamples: %d NumberOfReplySubtitles: %d \n" , NumberOfReplySamples , NumberOfReplySubtitles );
	    fprintf( stderr, "The section in question looks like this: \n%s\n\n" , SectionPointer );
	    ErrorMessage ( __FUNCTION__  , "\
There were an unequal number of reply samples and subtitles specified\n\
within a section of the Freedroid.dialogues file.\n\
This is currently not allowed in Freedroid and therefore indicates a\n\
severe error.",
				       PLEASE_INFORM, IS_FATAL );
	}
	else
	{
	    DebugPrintf ( CHAT_DEBUG_LEVEL , "\nThere were %d reply samples and an equal number of subtitles\n\
found in this option of the dialogue, which is fine.", NumberOfReplySamples );
	}
	
	//--------------------
	// Now that we know exactly how many Sample and Subtitle sections 
	// to read out, we can well start reading exactly that many of them.
	// 
	ReplyPointer = SectionPointer;
	for ( j = 0 ; j < NumberOfReplySamples ; j ++ )
	{
	    ChatRoster[ OptionIndex ] . reply_subtitle_list [ j ] =
		ReadAndMallocStringFromData ( ReplyPointer , "Subtitle=_\"" , "\"" ) ;
	    DebugPrintf( CHAT_DEBUG_LEVEL , "\nReplySubtitle found : \"%s\"." , ChatRoster[ OptionIndex ] . reply_subtitle_list [ j ] );
	    ChatRoster[ OptionIndex ] . reply_sample_list [ j ] =
		ReadAndMallocStringFromData ( ReplyPointer , "ReplySample=\"" , "\"" ) ;
	    DebugPrintf( CHAT_DEBUG_LEVEL , "\nReplySample found : \"%s\"." , ChatRoster[ OptionIndex ] . reply_sample_list [ j ] );
	    
	    //--------------------
	    // Now we must move the reply pointer to after the previous combination.
	    //
	    ReplyPointer = LocateStringInData ( ReplyPointer, "ReplySample" );
	    ReplyPointer ++;
	    
	}
	
	//--------------------
	// We count the number of Option changes and new values and then
	// we will read them out
	//
	NumberOfOptionChanges = CountStringOccurences ( SectionPointer , "ChangeOption" ) ;
	NumberOfNewOptionValues = CountStringOccurences ( SectionPointer , "ChangeToValue" ) ;
	if ( NumberOfOptionChanges != NumberOfNewOptionValues )
	{
	    fprintf( stderr, "\n\nNumberOfOptionChanges: %d NumberOfNewOptionValues: %d \n" , NumberOfOptionChanges , NumberOfNewOptionValues );
	    ErrorMessage ( __FUNCTION__  , "\
There was number of option changes but an unequal number of new option\n\
values specified in a section within the Freedroid.dialogues file.\n\
This is currently not allowed in Freedroid and therefore indicates a\n\
severe error.",
				       PLEASE_INFORM, IS_FATAL );
	}
	else
	{
	    DebugPrintf ( CHAT_DEBUG_LEVEL , "\nThere were %d option changes and an equal number of new option values\n\
found in this option of the dialogue, which is fine.", NumberOfOptionChanges );
	}
	
	//--------------------
	// Now that we know exactly how many option changes and new option values 
	// to read out, we can well start reading exactly that many of them.
	// 
	OptionChangePointer = SectionPointer;
	for ( j = 0 ; j < NumberOfOptionChanges ; j ++ )
	{
	    ReadValueFromString( OptionChangePointer , "ChangeOption=" , "%d" , 
				 & ( ChatRoster[ OptionIndex ] . change_option_nr [ j ] ) , TempEndPointer );
	    ReadValueFromString( OptionChangePointer , "ChangeToValue=" , "%d" , 
				 & ( ChatRoster[ OptionIndex ] . change_option_to_value [ j ] ) , TempEndPointer );
	    DebugPrintf( CHAT_DEBUG_LEVEL , "\nOption Nr. %d will change to value %d. " , 
			 ChatRoster[ OptionIndex ] . change_option_nr [ j ] ,
			 ChatRoster[ OptionIndex ] . change_option_to_value [ j ] );
	    
	    //--------------------
	    // Now we must move the option change pointer to after the previous combination.
	    //
	    OptionChangePointer = LocateStringInData ( OptionChangePointer, "ChangeToValue" );
	    OptionChangePointer ++;
	}
	
	//--------------------
	// We count the number of Extras to be done then
	// we will read them out
	//
	NumberOfExtraEntries = CountStringOccurences ( SectionPointer , "DoSomethingExtra" ) ;
	DebugPrintf( CHAT_DEBUG_LEVEL , "\nThere were %d 'Extras' specified in this option." , 
		     NumberOfExtraEntries );
	
	//--------------------
	// Now that we know exactly how many extra entries 
	// to read out, we can well start reading exactly that many of them.
	// 
	ExtraPointer = SectionPointer;
	for ( j = 0 ; j < NumberOfExtraEntries ; j ++ )
	{
	    // ExtraPointer = LocateStringInData ( ExtraPointer, "DoSomethingExtra" );
	    
	    ChatRoster[ OptionIndex ] . extra_list [ j ] =
		ReadAndMallocStringFromData ( ExtraPointer , "DoSomethingExtra=\"" , "\"" ) ;
	    
	    DebugPrintf( CHAT_DEBUG_LEVEL , "\nOption will execute this extra: %s. " , 
			 ChatRoster[ OptionIndex ] . extra_list [ j ] );
	    
	    //--------------------
	    // Now we must move the option change pointer to after the previous combination.
	    //
	    ExtraPointer = LocateStringInData ( ExtraPointer, "DoSomethingExtra" );
	    ExtraPointer ++;
	}
	
	//--------------------
	// Next thing we do will be to look whether there is maybe a on-goto-command
	// included in this option section.  If so, we'll read it out.
	//
	if ( CountStringOccurences ( SectionPointer , "OnCondition" ) ) 
	{
	    DebugPrintf( CHAT_DEBUG_LEVEL , "\nWe've found an ON-GOTO-CONDITION IN THIS OPTION!" );
	    ChatRoster[ OptionIndex ] . on_goto_condition = 
		ReadAndMallocStringFromData ( SectionPointer , "OnCondition=\"" , "\"" ) ;
	    DebugPrintf( CHAT_DEBUG_LEVEL , "\nOnCondition text found : \"%s\"." , ChatRoster[ OptionIndex ] . on_goto_condition );
	    ReadValueFromString( SectionPointer , "JumpToOption=" , "%d" , 
				 & ( ChatRoster[ OptionIndex ] . on_goto_first_target ) , TempEndPointer );
	    ReadValueFromString( SectionPointer , "ElseGoto=" , "%d" , 
				 & ( ChatRoster[ OptionIndex ] . on_goto_second_target ) , TempEndPointer );
	    DebugPrintf( CHAT_DEBUG_LEVEL , "\nOnCondition jump targets: TRUE--> %d FALSE-->%d." , 
			 ChatRoster[ OptionIndex ] . on_goto_first_target ,
			 ChatRoster[ OptionIndex ] . on_goto_second_target  );
	}
	else
	{
	    DebugPrintf( CHAT_DEBUG_LEVEL , "\nThere seems to be NO ON-GOTO-CONDITION AT ALL IN THIS OPTION." );
	}
	if ( CountStringOccurences ( SectionPointer , "LinkedTo:" ) ) 
	{
	    ReadValueFromString( SectionPointer , "LinkedTo:" , "%d" ,  & ( ChatRoster[ OptionIndex ] . link_target ) , TempEndPointer );
	}
	
	//--------------------
	// Next thing we do will be to get the always-on-startup flag status.
	//
	if ( CountStringOccurences ( SectionPointer , "AlwaysExecuteThisOptionPriorToDialogStart" ) ) 
	{
	    DebugPrintf( CHAT_DEBUG_LEVEL , "\nWe've found an ALWAYS-ON-START FLAG IN THIS OPTION!" );
	    
	    // Now we read in if this item can be used by the influ without help
	    YesNoString = ReadAndMallocStringFromData ( SectionPointer , "AlwaysExecuteThisOptionPriorToDialogStart=\"" , "\"" ) ;
	    if ( strcmp( YesNoString , "yes" ) == 0 )
	    {
		ChatRoster[ OptionIndex ] . always_execute_this_option_prior_to_dialog_start = TRUE;
	    }
	    else if ( strcmp( YesNoString , "no" ) == 0 )
	    {
		ChatRoster[ OptionIndex ] . always_execute_this_option_prior_to_dialog_start = FALSE;
	    }
	    else
	    {
		ErrorMessage ( __FUNCTION__  , "\
The text should contain an \n\
answer that is either 'yes' or 'no', but which was neither 'yes' nor 'no'.\n\
This indicated a corrupted FreedroidRPG dialog.",
					   PLEASE_INFORM, IS_FATAL );
	    }
	    free ( YesNoString ) ;
	}
	else
	{
	    DebugPrintf( CHAT_DEBUG_LEVEL , "\nThere seems to be NO ALWAYS-ON-START FLAG AT ALL IN THIS OPTION." );
	}
	
	//--------------------
	// Now that the whole section has been read out into the ChatRoster, we can
	// restore the original form of the Text again and the next option section
	// can be read out in the next run of this loop 
	//
	if ( RestoreTempDamage )
	{
	    DebugPrintf ( CHAT_DEBUG_LEVEL , "\nWe have now restored the damage from the temporary termination character." );
	    *TempEndPointer = TempSavedCharacter ;
	}
	else
	{
	    DebugPrintf ( CHAT_DEBUG_LEVEL , "\nSince we didn't add any temp termination character, there's nothing to restore now." );
	}
    }
    
    //--------------------
    // Now we've got all the information we wanted from the dialogues file.
    // We can now free the loaded file again.  Upon a new character dialogue
    // being initiated, we'll just reload the file.  This is very conveninet,
    // for it allows making and testing changes to the dialogues without even
    // having to restart Freedroid!  Very cool!
    //
    free( ChatData );
    
    //--------------------
    // Some security check against missing '0' dialog node in any given
    // dialog
    //
    if ( strlen ( ChatRoster [ 0 ] . option_text ) <= 0 )
    {
	DebugPrintf ( -4 , "\n%s(): Dialog file in question: %s." , __FUNCTION__ , FullPathAndFullFilename );
	ErrorMessage ( __FUNCTION__  , "\
The '0' dialog node was empty!",
				     PLEASE_INFORM, IS_FATAL );
    }

}; // void LoadChatRosterWithChatSequence ( char* SequenceCode )

/*----------------------------------------------------------------------
 * Copyright (C) 1997-2001 Id Software, Inc., under GPL
 *
 * FS_filelength().. (taken from quake2)
 * 		contrary to stat() this fct is nice and portable, 
 *----------------------------------------------------------------------*/
int
FS_filelength (FILE *f)
{
    int		pos;
    int		end;
    
    pos = ftell (f);
    fseek (f, 0, SEEK_END);
    end = ftell (f);
    fseek (f, pos, SEEK_SET);
    
    // DebugPrintf ( -4 , "\n%s(): file length: %d." , __FUNCTION__ , end );
    
    return end;
}; // int FS_filelength (FILE *f)


#undef _text_public_c

