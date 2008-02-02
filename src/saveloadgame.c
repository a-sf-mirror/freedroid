/* 
 *
 *   Copyright (c) 2002, 2003 Johannes Prix
 *   Copyright (c) 2004-2008 Arthur Huillet 
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
 * All functions that have to do with loading and saving of games.
 * ---------------------------------------------------------------------- */


#define _saveloadgame_c

#include "system.h"

#include "defs.h"
#include "struct.h"
#include "global.h"
#include "proto.h"
#include "SDL_rotozoom.h"
#include "sys/stat.h"
#include "savestruct.h"

#define LEVELNUM_EXPL_STRING "Explicit number of the level the influ currently is in="

#define SAVEDGAME_EXT ".savegame"
#define SAVE_GAME_THUMBNAIL_EXT ".thumbnail.bmp"

int load_game_command_came_from_inside_running_game = FALSE ;

FILE *SaveGameFile;  // to this file we will save all the ship data...

void
ShowSaveLoadGameProgressMeter( int Percentage , int IsSavegame ) 
{
    SDL_Rect TargetRect;

    TargetRect.x = AllMousePressButtons[ SAVE_GAME_BANNER ] . button_rect . x + ( AllMousePressButtons[ SAVE_GAME_BANNER ] . button_rect . w - 100 ) / 2 + 2 ;
    TargetRect.y = AllMousePressButtons[ SAVE_GAME_BANNER ] . button_rect . y + 20 ;
    TargetRect.w = Percentage ;
    TargetRect.h = 20 ;
    
    if ( IsSavegame) 
	ShowGenericButtonFromList ( SAVE_GAME_BANNER );
    else
	ShowGenericButtonFromList ( LOAD_GAME_BANNER );
    our_SDL_fill_rect_wrapper ( Screen , &TargetRect , SDL_MapRGB ( Screen->format , 0x0FF , 0x0FF , 0x0FF ) ) ;
    UpdateScreenOverButtonFromList ( SAVE_GAME_BANNER );

}; // void ShowSaveGameProgressMeter( int Percentage ) 

void
LoadAndShowThumbnail ( char* CoreFilename )
{
    char filename[1000];
    SDL_Surface* NewThumbnail;
    SDL_Rect TargetRectangle;
    
    if ( ! our_config_dir )
	return;
    
    sprintf( filename , "%s/%s%s", our_config_dir , CoreFilename , SAVE_GAME_THUMBNAIL_EXT );
    
    NewThumbnail = our_IMG_load_wrapper ( filename );
    if ( NewThumbnail == NULL ) return;
    
    TargetRectangle.x = 10 ;
    TargetRectangle.y = GameConfig . screen_height - NewThumbnail ->h - 10 ;
    
    if ( use_open_gl ) swap_red_and_blue_for_open_gl ( NewThumbnail );  
    our_SDL_blit_surface_wrapper ( NewThumbnail , NULL , Screen , &TargetRectangle );
    
    SDL_FreeSurface( NewThumbnail );

}; // void LoadAndShowThumbnail ( char* CoreFilename )

/* ----------------------------------------------------------------------
 * 
 *
 * ---------------------------------------------------------------------- */
void
LoadAndShowStats ( char* CoreFilename )
{
    char filename[1000];
    struct stat FileInfoBuffer;
    char InfoString[5000];
    struct tm *LocalTimeSplitup;
    long int FileSize;
    char* month_names[] = { "Jan" , "Feb" , "Mar" , "Apr" , "May" , "Jun" , "Jul" , "Aug" , "Sep" , "Oct" , "Nov" , "Dec" };

    if ( ! our_config_dir )
	return;
    
    DebugPrintf ( 2 , "\nTrying to get file stats for character '%s'. " , CoreFilename );
    
    //--------------------
    // First we save the full ship information, same as with the level editor
    //
    
    sprintf( filename , "%s/%s%s", our_config_dir , CoreFilename , SAVEDGAME_EXT );
    
    if ( stat ( filename , & ( FileInfoBuffer) ) )
    {
	fprintf( stderr, "\n\nfilename: %s. \n" , filename );
	ErrorMessage ( __FUNCTION__  , "\
Freedroid was unable to determine the time of the last modification on\n\
your saved game file.\n\
This is either a bug in Freedroid or an indication, that the directory\n\
or file permissions of ~/.freedroid_rpg are somehow not right.",
				   NO_NEED_TO_INFORM, IS_FATAL );
    };
    
    LocalTimeSplitup = localtime ( & ( FileInfoBuffer.st_mtime ) ) ;
    sprintf( InfoString , "%d %s %02d %02d:%02d" , 
	     1900 + LocalTimeSplitup->tm_year ,
	     month_names [ LocalTimeSplitup->tm_mon ] ,
	     LocalTimeSplitup->tm_mday ,
	     LocalTimeSplitup->tm_hour ,
	     LocalTimeSplitup->tm_min );
    
    PutString ( Screen , 240 , GameConfig . screen_height - 3 * FontHeight ( GetCurrentFont () ) , "Last Modified:" );
    PutString ( Screen , 240 , GameConfig . screen_height - 2 * FontHeight ( GetCurrentFont () ) , InfoString );
    
    //--------------------
    // Now that the modification time has been set up, we can start to compute
    // the overall disk space of all files in question.
    //
    FileSize = FileInfoBuffer.st_size;
    
    //--------------------
    // The saved ship must exist.  On not, it's a sever error!
    //    
    sprintf( filename , "%s/%s%s", our_config_dir , CoreFilename , ".shp" );
    if ( stat ( filename , & ( FileInfoBuffer) ) )
    {
	fprintf( stderr, "\n\nfilename: %s. \n" , filename );
	ErrorMessage ( __FUNCTION__  , "\
Freedroid was unable to determine the time of the last modification on\n\
your saved game file.\n\
This is either a bug in Freedroid or an indication, that the directory\n\
or file permissions of ~/.freedroid_rpg are somehow not right.",
				   NO_NEED_TO_INFORM, IS_FATAL );
    }
    FileSize += FileInfoBuffer.st_size;
    
    //--------------------
    // A thumbnail may not yet exist.  We won't make much fuss if it doesn't.
    //
    sprintf( filename , "%s/%s%s", our_config_dir , CoreFilename , SAVE_GAME_THUMBNAIL_EXT );
    if ( ! stat ( filename , & ( FileInfoBuffer) ) )
    {
        FileSize += FileInfoBuffer.st_size;
    }
    
    sprintf( InfoString , "File Size: %2.3f MB" , 
	     ((float)FileSize) / ( 1024.0 * 1024.0 ) );
    
    // PutString ( Screen , 240 , GameConfig . screen_height - 2 * FontHeight ( GetCurrentFont () ) , "File Size:" );
    PutString ( Screen , 240 , GameConfig . screen_height - 1 * FontHeight ( GetCurrentFont () ) , InfoString );
    
}; // void LoadAndShowStats ( char* filename );

/* ----------------------------------------------------------------------
 * This function stores a thumbnail of the currently running game, so that
 * these thumbnails can be browsed when choosing which game to load.
 * ---------------------------------------------------------------------- */
void
SaveThumbnailOfGame ( void )
{
    char filename[1000];
    SDL_Surface* NewThumbnail = NULL;
    SDL_Surface* FullView;
    
    if ( ! our_config_dir )
	return;
    
    //--------------------
    // First we save the full ship information, same as with the level editor
    //
    sprintf( filename , "%s/%s%s", our_config_dir, Me . character_name , SAVE_GAME_THUMBNAIL_EXT );
    
    AssembleCombatPicture ( SHOW_ITEMS );
    
    if ( use_open_gl )
    {
#ifdef HAVE_LIBGL
	//--------------------
	// We need to make a copy in processor memory. 
        GLvoid * imgdata = malloc ( ( GameConfig . screen_width + 2 ) * ( GameConfig . screen_height + 2 ) * 4 );
	glReadPixels( 0 , 1, GameConfig . screen_width , GameConfig . screen_height-1 , GL_RGB, GL_UNSIGNED_BYTE, imgdata );	

	//--------------------
	// Now we need to make a real SDL surface from the raw image data we
	// have just extracted.
	//
	FullView = SDL_CreateRGBSurfaceFrom( imgdata , GameConfig . screen_width , GameConfig . screen_height, 24, 3 * GameConfig . screen_width, 0x0FF0000, 0x0FF00, 0x0FF , 0 );
	
	NewThumbnail = zoomSurface( FullView , 0.32 * 640.0f / GameConfig . screen_width , 0.32 * 640.0f / GameConfig . screen_width , 0 );
	
	free ( imgdata ) ;
	if ( NewThumbnail == NULL ) 
		return;		

	//--------------------
	// Of course, since we used OpenGL for generating the raw image data, the data is
	// upside down again.  Now that won't be much of a problem, since we've already
	// dealt with is several times, using the following flipping code.
	//
	flip_image_vertically ( NewThumbnail );
	swap_red_and_blue_for_open_gl ( NewThumbnail );
	
	SDL_FreeSurface ( FullView ) ;
#endif
    }
    else
    {
	NewThumbnail = zoomSurface( Screen , 0.32 , 0.32 , 0 );
    }
    
    SDL_SaveBMP( NewThumbnail , filename );
    
    SDL_FreeSurface( NewThumbnail );
    
}; // void SaveThumbnailOfGame ( void )

/* ----------------------------------------------------------------------
 * This function saves the current game of Freedroid to a file.
 * ---------------------------------------------------------------------- */
int 
SaveGame( void )
{
    char filename[1000];
    
    if ( ! our_config_dir )
	return (OK);
    
    Activate_Conservative_Frame_Computation();
    
    ShowSaveLoadGameProgressMeter( 0 , TRUE ) ;
    
    sprintf ( Me . freedroid_version_string , 
	      "%s;sizeof(tux_t)=%d;sizeof(enemy)=%d;sizeof(bullet)=%d;MAXBULLETS=%d\n", 
	      VERSION , 
	      (int) sizeof(tux_t) , 
	      (int) sizeof(enemy) ,
	      (int) sizeof(bullet) ,
	      (int) MAXBULLETS );
    
    sprintf( filename , "%s/%s%s", our_config_dir, Me.character_name, ".shp" );

    if ( SaveShip( filename ) != OK )
    {
	ErrorMessage ( __FUNCTION__  , "\
The SAVING OF THE SHIP DATA FOR THE SAVED GAME FAILED!\n\
This is either a bug in Freedroid or an indication, that the directory\n\
or file permissions of ~/.freedroid_rpg are somehow not right.",
				   PLEASE_INFORM, IS_FATAL );
    } 
    else
    {
	DebugPrintf( SAVE_LOAD_GAME_DEBUG , "\nShip data for saved game seems to have been saved correctly.\n");
    }
    
    sprintf ( filename , "%s/%s%s", our_config_dir , Me.character_name, ".savegame");
    
    if( ( SaveGameFile = fopen(filename, "wb")) == NULL) {
	printf("\n\nError opening save game file for writing...\n\nTerminating...\n\n");
	Terminate(ERR);
    }
    
    /* XXX write level number  */
    fprintf(SaveGameFile, "%s%d\n", LEVELNUM_EXPL_STRING, CurLevel->levelnum);
    
    /* Write the version string */
    fprintf(SaveGameFile, "Version string: %s\n\n", Me . freedroid_version_string);
    
    /* Save tux*/
    save_tux_t(NULL, &Me);

    /* Save all enemies */
    int i = 0;
    for ( ; i < 2; i++ )
	{
	enemy * erot = i ? dead_bots_head : alive_bots_head;
	for ( ; erot; erot = GETNEXT(erot) )
	    save_enemy(NULL, erot);
	}

    /* Save all bullets */
    for ( i = 0; i < MAXBULLETS; i ++)
	save_bullet(NULL, &AllBullets[i]);
    
    fclose( SaveGameFile );
    
    ShowSaveLoadGameProgressMeter( 99 , TRUE ); 
    
    SaveThumbnailOfGame ( );
    
    ShowSaveLoadGameProgressMeter( 100 , TRUE ) ;
    
    append_new_game_message ( _("Game saved.") );

    DebugPrintf ( SAVE_LOAD_GAME_DEBUG , "\nint SaveGame( void ): end of function reached.");
    
    return OK;

}; // int SaveGame( void )


/* ----------------------------------------------------------------------
 * This function loads an old saved game of Freedroid from a file.
 * ---------------------------------------------------------------------- */
int 
DeleteGame( void )
{
    char filename[1000];

    if ( ! our_config_dir )
	return (OK);
    
    //--------------------
    // First we save the full ship information, same as with the level editor
    //
    sprintf( filename , "%s/%s%s", our_config_dir , Me.character_name, ".shp");
    
    remove ( filename ) ;
    
    //--------------------
    // First, we must determine the savedgame data file name
    //
    sprintf (filename, "%s/%s%s", our_config_dir, Me.character_name, SAVEDGAME_EXT);
    
    remove ( filename );
    
    sprintf( filename , "%s/%s%s", our_config_dir, Me.character_name , SAVE_GAME_THUMBNAIL_EXT );
    
    remove ( filename );
    
    return ( OK );

}; // int DeleteGame( void )

/* ----------------------------------------------------------------------
 * This function loads an old saved game of Freedroid from a file.
 * ---------------------------------------------------------------------- */
int 
LoadGame( void )
{
#if 0
    char version_check_string[1000];
    char *LoadGameData;
    char filename[1000];
    unsigned char* InfluencerRawDataPointer;
    unsigned char* EnemyRawDataPointer;
    unsigned char* BulletRawDataPointer;
    int i;
    int current_geographics_levelnum;
    FILE *DataFile;
    
    if ( ! our_config_dir )
    {
	DebugPrintf (0, "No Config-directory, cannot load any games\n");
	return (OK);
    }
    
    DebugPrintf ( SAVE_LOAD_GAME_DEBUG , "\n%s(): function call confirmed...." , __FUNCTION__ );
    
    //--------------------
    // Loading might take a while, therefore we activate the conservative
    // frame rate, just to be sure, so that no sudden jumps occur, perhaps
    // placing the influencer or some bullets outside the map even!
    //
    Activate_Conservative_Frame_Computation();
    global_ingame_mode = GLOBAL_INGAME_MODE_NORMAL ;
    
    ShowSaveLoadGameProgressMeter( 0 , FALSE )  ;
 
    //--------------------
    // First we load the full ship information, same as with the level editor
    
    sprintf( filename , "%s/%s%s", our_config_dir, Me . character_name, ".shp");
    
    //--------------------
    // Maybe there isn't any saved game by that name.  This case must be checked for
    // and handled...
    //
    if ((DataFile = fopen ( filename , "rb")) == NULL )
    {
	GiveMouseAlertWindow ( "\nW A R N I N G !\n\nFreedroidRPG was unable to locate the saved game file you requested to load.\nThis might mean that it really isn't there cause you tried to load a game without ever having saved the game before.  \nThe other explanation of this error might be a severe error in FreedroidRPG.\nNothing will be done about it." );
	append_new_game_message ( _("Failed to load old game.") );
	return ( ERR ) ;
    }
    else
    {
	DebugPrintf ( 1 , "\nThe saved game file (.shp file) seems to be there at least.....");
    }
    
    LoadShip( filename );
    
    //--------------------
    // Now we must determine the savedgame data file name
    //
    sprintf (filename, "%s/%s%s", our_config_dir, Me.character_name, SAVEDGAME_EXT);
    
    DebugPrintf ( SAVE_LOAD_GAME_DEBUG , "\n%s(): starting to read savegame data...." , __FUNCTION__ );
    
    //--------------------
    // Now we can read the whole savegame data into memory with one big flush
    //
    LoadGameData = ReadAndMallocAndTerminateFile( filename , END_OF_SAVEDGAME_DATA_STRING ) ;
    
    DebugPrintf ( SAVE_LOAD_GAME_DEBUG , "\n%s(): starting to decode savegame data...." , __FUNCTION__ );
    
    //--------------------
    // Before we start decoding the details, we get the former level-number where the
    // influencer was walking and construct a new 'CurLevel' out of it.
    //
    ReadValueFromString( LoadGameData ,  LEVELNUM_EXPL_STRING , "%d" , 
			 &current_geographics_levelnum , LoadGameData + 30000 );
    CurLevel = curShip.AllLevels[ current_geographics_levelnum ];
    
    
    //--------------------
    // Now we start decoding our new game information and fill it into the apropriate structs
    // We assume, that our target strings will be found, so we give 3000000 as the search area
    // length, since we do not know it exactly
    //
    InfluencerRawDataPointer = MyMemmem( LoadGameData , 3000000 , INFLUENCER_STRUCTURE_RAW_DATA_STRING , 
					 strlen ( INFLUENCER_STRUCTURE_RAW_DATA_STRING ) );
    InfluencerRawDataPointer += strlen ( INFLUENCER_STRUCTURE_RAW_DATA_STRING ) ;
    memcpy( &Me , InfluencerRawDataPointer , sizeof ( tux_t ) );
    InfluencerRawDataPointer += sizeof ( tux_t );
    
    //--------------------
    // Now we decode the enemy information.
    // We assume, that our target strings will be found, so we give 3000000 as the search area
    // length, since we do not know it exactly
    //
    /*XXX*/
    /*
    EnemyRawDataPointer = MyMemmem( LoadGameData , 30000000 , ALLENEMYS_RAW_DATA_STRING , 
				    strlen ( ALLENEMYS_RAW_DATA_STRING ) );
    EnemyRawDataPointer += strlen ( ALLENEMYS_RAW_DATA_STRING ) ;
    memcpy( &(AllEnemys) , EnemyRawDataPointer , sizeof ( enemy ) * MAX_ENEMYS_ON_SHIP );
    */

    //--------------------
    // Now we decode the bullet information.
    // We assume, that our target strings will be found, so we give 10000000 as the search area
    // length, since we do not know it exactly
    //
    BulletRawDataPointer = MyMemmem( LoadGameData , 30000000 , ALLBULLETS_RAW_DATA_STRING , 
				     strlen ( ALLBULLETS_RAW_DATA_STRING ) );
    BulletRawDataPointer += strlen ( ALLBULLETS_RAW_DATA_STRING ) ;
    memcpy( &(AllBullets) , BulletRawDataPointer , sizeof ( bullet ) * MAXBULLETS );
    
    //--------------------
    // When the original game was still going on, some dynamic things like pointers to some
    // constructed text field might have been used.  Now these dynamic things somewhere in 
    // memory of course do not exist, so any pointer previously refering to them, must be
    // set to acceptable values before an accident (SEGFAULT) occurs!
    //
    DebugPrintf ( SAVE_LOAD_GAME_DEBUG , "\n%s(): now correcting dangerous pointers...." , __FUNCTION__ );
    Me . TextToBeDisplayed = "";
    enemy * erot = alive_bots_head;
    while ( erot )
    {
	erot->TextToBeDisplayed = "" ;
	erot->TextVisibleTime = 0;
	erot = GETNEXT(erot);
    }
    for ( i = 0 ; i < MAXBULLETS ; i++ )
    {
	//--------------------
	// This might mean a slight memory loss, but I guess we can live with that...
	//
	/* XXX no we can't*/
	AllBullets [ i ] . Surfaces_were_generated = FALSE;
	if ( AllBullets[ i ].angle_change_rate != 0 ) DeleteBullet( i , FALSE );
    }
    
    //--------------------
    // Now we check if the loaded game is from a compatible version of FreedroidRPG, or
    // if we maybe have a saved game from a different version, which would mean trouble
    //
    // NOTE:  WE MUST DO THIS BEFORE ANY REFERENCE TO THE LOADED GAME IS MADE, EVEN IF
    //        THAT REFERENCE IS SO SMALL AS TO JUST SET BACKGROUND MUSIC ACCORDING TO
    //        CURRENT TUX POSITION IN THE MAPS AS FOUND BELOW
    //
    //
    //
    sprintf ( version_check_string , "%s;sizeof(tux_t)=%d;sizeof(enemy)=%d;sizeof(bullet)=%d;MAXBULLETS=%d;MAX_ENEMYS_ON_SHIP=%d\n", 
	      VERSION , 
	      (int) sizeof(tux_t) , 
	      (int) sizeof(enemy) ,
	      (int) sizeof(bullet) ,
	      (int) MAXBULLETS ,
	      (int) 1200 );
    
    if ( strcmp ( Me . freedroid_version_string , version_check_string ) != 0 )
    {
	show_button_tooltip ( _("Error: Version or structsize mismatch! The saved game in question appears to be from a (slightly?) different version of FreedroidRPG.\nSorry, but I refuse to load it for safety/stability reasons...\nFor Recovery, a blank game will be loaded...(please disregard)\n") );
	our_SDL_flip_wrapper( Screen );
	while ( SpacePressed() || MouseLeftPressed() ) SDL_Delay ( 3 );
	while ( !SpacePressed() && ! MouseLeftPressed() ) SDL_Delay ( 3 );
	while ( SpacePressed() || MouseLeftPressed()) SDL_Delay ( 3 );

	//--------------------
	// Now at this point the current Tux data has been junked
	// around with from the failed loading attempt.  We must
	// clear out the data and then safely exit out of the current
	// game!
	//
	// WARNING!  If the game is already running, this really
	// dangerous.  In that case the damaged data should best be
	// completely overwritten with something sensible...
	// (Otherwise Floating Point Exceptions and the like are likely...)
	//
	if ( load_game_command_came_from_inside_running_game )
	{
	    clear_player_inventory_and_stats ( ) ;
	    UpdateAllCharacterStats ( ) ;
	    char fp[2048];
	    find_file ( "Asteroid.maps", MAP_DIR, fp, 0 );
	    LoadShip ( fp ) ;
	    PrepareStartOfNewCharacter (  ) ;
	}
	return ( ERR ) ;
    }
    
    //--------------------
    // To prevent cheating, we remove all active spells, that might still be there
    // from other games just played before.
    //
    clear_active_spells();
    
    //--------------------
    // Now that we have loaded the game, we must count and initialize the number
    // of droids used in this ship.  Otherwise we might ignore some robots.
    //
    CountNumberOfDroidsOnShip (  ) ;
    
    SwitchBackgroundMusicTo( curShip.AllLevels[ Me.pos.z ]->Background_Song_Name );
    
    free ( LoadGameData );
    
    //--------------------
    // Maybe someone just lost in the game and has then pressed the load
    // button.  Then a new game is loaded and the game-over status has
    // to be restored as well of course.
    //
    GameOver = FALSE; 
    
    DebugPrintf ( SAVE_LOAD_GAME_DEBUG , "\n%s(): end of function reached." , __FUNCTION__ );
    
    //--------------------
    // Now we know that right after loading an old saved game, the Tux might have
    // to 'change clothes' i.e. a lot of tux images need to be updated which can
    // take a little time.  Therefore we print some message so the user will not
    // panic and push the reset button :)
    //
    PutStringFont ( Screen , FPS_Display_BFont , 75 , 150 , _("Updating Tux images (this may take a little while...)") );
    our_SDL_flip_wrapper ( Screen );
    
    //--------------------
    // Now that the whole character information (and that included the automap
    // information) has been restored, we can use the old automap data to rebuild
    // the automap texture in the open_gl case...
    //
    insert_old_map_info_into_texture (  );

    load_game_command_came_from_inside_running_game = TRUE ;

    append_new_game_message ( _("Game loaded.") );
#endif
    return OK;
}; // int LoadGame ( void ) 

/* Save complex simple types. Structured types are defined in savestruct.c, and
 * primitive types are macros in proto.h
 */
void save_enemy_ptr(char * tag, enemy ** val)
{
/* Do nothing, at least for now*/
fprintf(SaveGameFile, "%s: @%p\n", tag, (void *)(*val));
}

void read_enemy_ptr(char * buffer, char * tag, enemy ** val)
{
}

/* Save arrays of simple types */
#define define_save_xxx_array(X) void save_##X##_array(char * tag, X * val_ar, int size)\
{\
fprintf(SaveGameFile, "<%s array n=%d>\n", tag, size);\
int i;\
for ( i = 0; i < size; i ++)\
	{\
	char str[10];\
	sprintf(str, "%i", i);\
	fprintf(SaveGameFile, "\t");\
	save_##X(str, &val_ar[i]);\
	}\
fprintf(SaveGameFile, "</%s>\n", tag);\
}

define_save_xxx_array(int32_t);
define_save_xxx_array(uint32_t);
define_save_xxx_array(int16_t);
define_save_xxx_array(uint16_t);
define_save_xxx_array(mission);
define_save_xxx_array(item);
define_save_xxx_array(gps);
define_save_xxx_array(moderately_finepoint);

void save_chatflags_t_array(char * tag, chatflags_t * chatflags, int size)
{
fprintf(SaveGameFile, "<ChatFlags n=%d>\n", MAX_PERSONS);
int i, j;
for ( i = 0; i < MAX_PERSONS; i ++)
    {
    for ( j = 0; j < MAX_ANSWERS_PER_PERSON; j ++)
	{
	fprintf(SaveGameFile, "%hhd ", (chatflags)[i][j]);
	}
    fprintf(SaveGameFile, "\n");
    }
fprintf(SaveGameFile, "</ChatFlags>\n");
}

void save_cookielist_t_array(char * tag, cookielist_t * cookielist, int size)
{
fprintf(SaveGameFile, "<cookielist>\n");
int i;
for ( i = 0; i < MAX_COOKIES; i ++)
    {
    if ( strlen ( cookielist[i] ) )
	    fprintf(SaveGameFile, "%s\n", cookielist[i]);
    }
fprintf(SaveGameFile, "</cookielist>\n");
}

void save_automap_data(char * tag, automap_data_t * automapdata, int size)
{
fprintf(SaveGameFile, "<automap>\n");
int i,j,k;
for ( i = 0; i < MAX_LEVELS; i ++)
   {
   fprintf(SaveGameFile, "%d\n", i);
   for ( j = 0; j < 100; j ++ )
       {
       for ( k = 0; k < 100; k ++)
	       fprintf(SaveGameFile, "%hhd ", automapdata[i][j][k]);
       fprintf(SaveGameFile, "\n");
       }
   }
fprintf(SaveGameFile, "</automap>\n");
}

#undef _saveloadgame_c
