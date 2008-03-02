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

int SaveGame( void )
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
    
    
    /* Write the version string */
    fprintf(SaveGameFile, "Version string: %s\n\n", Me . freedroid_version_string);
    
    /* Save tux*/
    save_tux_t("player", &Me);

    /* Save all enemies */
    int i ;
    int a = 0;
    for ( i = 0; i < 2; i++ )
	{
	enemy * erot = i ? dead_bots_head : alive_bots_head;
	for ( ; erot; erot = GETNEXT(erot) )
	    {
	    char str[20];
	    sprintf(str, "%s enemy %d", i ? "dead" : "alive", a);
	    save_enemy(str, erot);
	    a ++;
	    }
	}

    /* Save all bullets */
    for ( i = 0; i < MAXBULLETS; i ++)
	{
	char str[20];
	sprintf(str, "blt%d", i);
	save_bullet(str, &AllBullets[i]);
	}
    
    fprintf ( SaveGameFile, "End of freedroidRPG savefile\n");
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
    char version_check_string[1000];
    char *LoadGameData;
    char filename[1000];
    int i;
    FILE *DataFile;
    
    if ( ! our_config_dir )
    {
	DebugPrintf (0, "No Config-directory, cannot load any games\n");
	return (OK);
    }
    
    DebugPrintf ( SAVE_LOAD_GAME_DEBUG , "\n%s(): function call confirmed...." , __FUNCTION__ );
    
    Activate_Conservative_Frame_Computation();
    global_ingame_mode = GLOBAL_INGAME_MODE_NORMAL ;
    
    ShowSaveLoadGameProgressMeter( 0 , FALSE )  ;
 
    sprintf( filename , "%s/%s%s", our_config_dir, Me . character_name, ".shp");
    
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

    sprintf (filename, "%s/%s%s", our_config_dir, Me.character_name, SAVEDGAME_EXT);
    
    LoadGameData = ReadAndMallocAndTerminateFile( filename , "End of freedroidRPG savefile\n" ) ;
    
    memset(&Me, 0, sizeof(tux_t));
    read_tux_t(LoadGameData, "player", &Me);

    /* read enemies */
    ClearEnemys();

    enemy newen;
    int done;
    int a = 0;
    for ( i = 0 ; i < 2; i ++ )
	{
	enemy ** ce = i ? &dead_bots_head : &alive_bots_head;
	char * cpos = LoadGameData;
	done = 0;
	while ( ! done ) 
	    {
	    memset(&newen, 0, sizeof(enemy));
	    char str[25];
	    sprintf(str, "%s enemy %d", i ? "dead" : "alive", a);
	    cpos = strstr(cpos, str);
	    if ( ! cpos )
		{
		done = 1;
		break;
		}
	    cpos -= 5;
	    if ( read_enemy(cpos, str, &newen ) )
		done = 1;
	    else *ce = add_enemy_head(*ce, &newen);
	    a++;
	    }
	}


    /* read bullets */
    char * cpos = LoadGameData;
    done = 0;
    for ( i = 0; i < MAXBULLETS && ! done ; i ++)
	{
	char str[20];
	sprintf(str, "blt%d", i);
	cpos = strstr(cpos, str);
	if ( ! cpos )
	    {
	    done = 1;
	    break;
	    }
	cpos -= 5;
	read_bullet(cpos, str, &AllBullets[i]);
	}
    
    /* properly restore pointers and references*/
    DebugPrintf ( SAVE_LOAD_GAME_DEBUG , "\n%s(): now correcting dangerous pointers...." , __FUNCTION__ );
    Me . TextToBeDisplayed = "";
    for ( i = 0; i < 2; i ++)
	{
	enemy * erot = i ? alive_bots_head : dead_bots_head;
	while ( erot )
	    {
	    erot->TextToBeDisplayed = "" ;
	    erot->TextVisibleTime = 0;
	    erot = GETNEXT(erot);
	    }
	}

    for ( i = 0 ; i < MAXBULLETS ; i++ )
    {
	DeleteBullet( i , FALSE );
    }
    sprintf ( version_check_string , "%s;sizeof(tux_t)=%d;sizeof(enemy)=%d;sizeof(bullet)=%d;MAXBULLETS=%d", 
	      VERSION , 
	      (int) sizeof(tux_t) , 
	      (int) sizeof(enemy) ,
	      (int) sizeof(bullet) ,
	      (int) MAXBULLETS );
    
    if ( strcmp ( Me . freedroid_version_string , version_check_string ) != 0 )
    {
	show_button_tooltip ( _("Version or structsize mismatch! The savegame is not from the same version of freedroidRPG... possible breakage.\n") );
	our_SDL_flip_wrapper( Screen );
	while ( ! MouseLeftPressed() ) SDL_Delay(1);
    }
    
    CurLevel = curShip.AllLevels[ Me . pos . z ];
    
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
    return OK;
}; // int LoadGame ( void ) 

void read_enemy_ptr(const char * buffer, const char * tag, enemy ** val)
{
}

void read_int32_t(const char * buffer, const char * tag, int32_t * val)
{
    char search[strlen(tag) + 3];
    sprintf(search, "\n%s:", tag);
    char * pos = strstr(buffer, search);
    if ( ! pos ) return;
    pos += 3 + strlen(tag);
    char * epos = pos;
    while ( *epos != '\n' ) epos++;
    *epos = '\0';
    while ( ! isdigit(*pos) && *pos != '-') pos++;
    *val = strtol(pos, NULL, 10);
    *epos = '\n';
}

void read_int16_t(const char * buffer, const char * tag, int16_t * val)
{
    int32_t valt;
    read_int32_t(buffer, tag, &valt);
    *val = valt;
}

void read_char(const char * buffer, const char * tag, char * val)
{
    int32_t valt;
    read_int32_t(buffer, tag, &valt);
    *val = valt;
}

void read_uint32_t(const char * buffer, const char * tag, uint32_t * val)
{
    int32_t valt;
    read_int32_t(buffer, tag, &valt);
    *val = valt;
}

void read_uint16_t(const char * buffer, const char * tag, uint16_t * val)
{
    uint32_t valt;
    read_uint32_t(buffer, tag, &valt);
    *val = valt;
}

void read_uchar(const char * buffer, const char * tag, unsigned char * val)
{
    uint32_t valt;
    read_uint32_t(buffer, tag, &valt);
    *val = valt;
}



void read_double(const char * buffer, const char * tag, double * val)
{
    char search[strlen(tag) + 3];
    sprintf(search, "\n%s:", tag);
    char * pos = strstr(buffer, search);
    if ( ! pos ) return;
    pos += 3 + strlen(tag);
    char * epos = pos;
    while ( *epos != '\n' ) epos++;
    *epos = '\0';
    while ( ! isdigit(*pos) && *pos != '-') pos++;
    *val = strtod(pos, NULL);
    *epos = '\n';
}

void read_float(const char * buffer, const char * tag, float * val)
{
    double valt;
    read_double(buffer, tag, &valt);
    *val = valt;
}


void read_string(const char * buffer, const char * tag, char * val)
{
    char search[strlen(tag) + 3];
    sprintf(search, "\n%s:", tag);
    char * pos = strstr(buffer, search);
    if ( ! pos ) return;
    pos += 3 + strlen(tag);
    char * epos = pos;
    while ( *epos != '\n' ) epos++;
    *epos = '\0';
    strcpy ( val, pos );
    *epos = '\n';
}


/* Save arrays of simple types */
#define define_save_xxx_array(X) void save_##X##_array(const char * tag, X * val_ar, int size)\
{\
fprintf(SaveGameFile, "<%s array n=%d>\n", tag, size);\
int i;\
for ( i = 0; i < size; i ++)\
	{\
	char str[10];\
	sprintf(str, "%i", i);\
	save_##X(str, &val_ar[i]);\
	}\
fprintf(SaveGameFile, "</%s>\n", tag);\
}

define_save_xxx_array(int32_t);
define_save_xxx_array(uint32_t);
define_save_xxx_array(int16_t);
define_save_xxx_array(uint16_t);
typedef unsigned char uchar;
define_save_xxx_array(float);
define_save_xxx_array(uchar);
define_save_xxx_array(mission);
define_save_xxx_array(item);
define_save_xxx_array(gps);
define_save_xxx_array(moderately_finepoint);

/* Read arrays of simple types */
#define define_read_xxx_array(X) void read_##X##_array(const char * buffer, const char * tag, X * val_ar, int size)\
{\
int i;\
char search[strlen(tag) + 20];\
sprintf(search, "<%s array", tag);\
char * pos = strstr(buffer, search);\
if ( ! pos ) ErrorMessage ( __FUNCTION__, "Unable to find array %s\n", PLEASE_INFORM, IS_FATAL, tag);\
sprintf(search, "</%s>", tag);\
char * epos = strstr(pos, search);\
if ( ! epos ) ErrorMessage ( __FUNCTION__, "Unable to find array end %s\n", PLEASE_INFORM, IS_FATAL, tag);\
epos += strlen(search);\
char savechar = *(epos + 1);\
*(epos+1) = '\0';\
int nb = 0;\
char *runp = pos + 1;\
runp += strlen(tag) + 1 + strlen("array") + 1;\
while ( ! isdigit(*runp) ) runp++;\
char * erunp = runp;\
while ( *erunp != '>' ) erunp ++;\
*erunp = '\0';\
nb = atoi(runp);\
*erunp = '>';\
if ( nb != size )  ErrorMessage ( __FUNCTION__, "Size mismatch for array %s, %d in file, %d in game\n", PLEASE_INFORM, IS_FATAL, tag, nb, size);\
for ( i = 0; i < (nb <= size ? nb : size); i ++)\
	{\
	char str[10];\
	sprintf(str, "%d", i);\
	pos = strstr(pos, str);\
	pos -= 5;\
	read_##X(pos, str, &val_ar[i]);\
	}\
*(epos+1) = savechar;\
}

define_read_xxx_array(int32_t);
define_read_xxx_array(uint32_t);
define_read_xxx_array(int16_t);
define_read_xxx_array(uint16_t);
define_read_xxx_array(float);
define_read_xxx_array(uchar);
define_read_xxx_array(mission);
define_read_xxx_array(item);
define_read_xxx_array(gps);
define_read_xxx_array(moderately_finepoint);

void save_sdl_rect(const char * tag, SDL_Rect * target)
{
    fprintf(SaveGameFile, "<%s>\n",tag);
    save_int16_t("x", &(target->x));
    save_int16_t("y", &(target->y));
    save_uint16_t("w", &(target->w));
    save_uint16_t("h", &(target->h));
    fprintf(SaveGameFile, "</%s>\n", tag);
}

int read_sdl_rect(const char * buffer, const char * tag, SDL_Rect * target)
{
    char search[strlen(tag) + 5];
    sprintf(search, "<%s>", tag);
    char * pos = strstr(buffer, search);
    if ( ! pos ) return 1;
    pos += 1 + strlen(tag);
    sprintf(search, "</%s>", tag);
    char * epos = strstr(buffer, search);
    if ( ! epos ) return 2;
    *epos = 0;
    read_int16_t(pos, "x", &(target->x));
    read_int16_t(pos, "y", &(target->y));
    read_uint16_t(pos, "w", &(target->w));
    read_uint16_t(pos, "h", &(target->h));
    *epos = '>';
    return 0;
}

void save_chatflags_t_array(const char * tag, chatflags_t * chatflags, int size)
{
fprintf(SaveGameFile, "<ChatFlags mpeople=%d manswers=%d>\n", MAX_PERSONS, MAX_ANSWERS_PER_PERSON);
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

void read_chatflags_t_array(const char * buffer, const char * tag, chatflags_t * chatflags, int size)
{
char * pos = strstr(buffer, "<ChatFlags mpeople=");
if ( ! pos ) ErrorMessage ( __FUNCTION__, "Unable to find ChatFlags array\n", PLEASE_INFORM, IS_FATAL);
char * epos = strstr(pos, "</ChatFlags>\n");
if ( ! epos ) ErrorMessage ( __FUNCTION__, "Unable to find ChatFlags array end\n", PLEASE_INFORM, IS_FATAL);
epos += strlen("</ChatFlags>\n");
char savechar = *(epos + 1);
*(epos+1) = '\0';
int mp = 0;
int ma = 0;
char *runp = pos + 1;
runp += strlen("ChatFlags n=");
while ( ! isdigit(*runp) ) runp++;
char * erunp = runp;
while ( (*erunp) != ' ') erunp ++;
*erunp = '\0';
mp = atoi(runp);
*erunp = ' ';
runp = erunp;
while ( ! isdigit(*runp) ) runp++;
erunp = runp;
while ( *erunp != '>' ) erunp ++;
*erunp = '\0';
ma = atoi(runp);
*erunp = '>';

if ( mp != MAX_PERSONS )  
    ErrorMessage ( __FUNCTION__, "MAX_PERSONS mismatch for array ChatFlags, %d in file, %d in game\n", PLEASE_INFORM, IS_FATAL, mp, MAX_PERSONS);
if ( ma != MAX_ANSWERS_PER_PERSON )
    ErrorMessage ( __FUNCTION__, "MAX_ANSWERS_PER_PERSON mismatch for array ChatFlags, %d in file, %d in game\n", PLEASE_INFORM, IS_FATAL, ma, MAX_ANSWERS_PER_PERSON);

while ( *runp != '\n' ) runp++;
runp++;

int i = 0, j;
while ( i < MAX_PERSONS )
	{
	j = 0;
	while ( j < MAX_ANSWERS_PER_PERSON )
	    {
	    while ( ! isdigit(*runp) ) runp++;
	    erunp = runp + 1;
	    while ( *erunp != ' ' ) erunp++;
	    *erunp = '\0';
	    
	    (chatflags)[i][j] = atoi(runp);
	    *erunp = ' ';
	    runp = erunp;
	    j++;
	    }

	while ( *runp != '\n' ) runp++;
	i++;
	}
*(epos+1) = savechar;
}


void save_cookielist_t_array(const char * tag, cookielist_t * cookielist, int size)
{
fprintf(SaveGameFile, "<cookielist mc=%d>\n", MAX_COOKIES);
int i;
for ( i = 0; i < MAX_COOKIES; i ++)
    {
    if ( strlen ( cookielist[i] ) )
	    fprintf(SaveGameFile, "%s\n", cookielist[i]);
    }
fprintf(SaveGameFile, "</cookielist>\n");
}

void read_cookielist_t_array(const char * buffer, const char * tag, cookielist_t * cookielist, int size)
{
char * pos = strstr(buffer, "<cookielist mc=");
if ( ! pos ) ErrorMessage ( __FUNCTION__, "Unable to find cookielist.\n", PLEASE_INFORM, IS_FATAL);
char * epos = strstr(pos, "</cookielist>\n");
if ( ! epos ) ErrorMessage ( __FUNCTION__, "Unable to find cookielist end.\n", PLEASE_INFORM, IS_FATAL);
char savechar = *epos;
*epos = '\0';
int mc = 0;

pos += strlen("<cookielist mc=");
while ( ! isdigit(*pos) ) pos++;
char * erunp = pos;
while ( (*erunp) != '>') erunp ++;
*erunp = '\0';
mc = atoi(pos);
*erunp = '>';
pos = erunp;

if ( mc != MAX_COOKIES )
    ErrorMessage(__FUNCTION__, "Size mismatch for max number of cookies, file %d vs. game %d.\n", PLEASE_INFORM, IS_FATAL, mc, MAX_COOKIES);

while ( *pos != '\n' ) pos ++;
pos++;

int i = 0;
while ( pos < epos )
    {
    int j = 0;
    while ( *pos != '\n' ) 
	{
	cookielist[i][j] = *pos;
	pos++;
	j++;
	}
    cookielist[i][j] = '\0';
    pos ++;
    i ++;
    }

*epos = savechar;
}

void save_bigscrmsg_t(const char *tag, bigscrmsg_t * data)
{
/* bigscrmsg_t is an array (size MAX_BIG_SCREEN_MESSAGES) of strings */
/* those messages are of very little interest which is why we will not save them.*/
#if 0
fprintf(SaveGameFile, "<bigscreenmessages m=%d>\n", MAX_BIG_SCREEN_MESSAGES);
int i = 0;
while ( (*data)[i] != NULL && strlen ( (* data) [ i ] ) )
    {
    fprintf(SaveGameFile, "%s", (*data) [ i ] );
    }

fprintf(SaveGameFile, "</bigscreenmessages>\n");
#endif
}

void read_bigscrmsg_t(const char * buffer, const char * tag, bigscrmsg_t * data)
{
/* This function isn't even finished */
#if 0
char * pos = strstr(buffer, "<bigscreenmessages m=");
if ( ! pos ) ErrorMessage ( __FUNCTION__, "Unable to find big screen messages.\n", PLEASE_INFORM, IS_FATAL);
char * epos = strstr(pos, "</bigscreenmessages>\n");
if ( ! epos ) ErrorMessage ( __FUNCTION__, "Unable to find big screen messages end.\n", PLEASE_INFORM, IS_FATAL);
char savechar = *epos;
*epos = '\0';
int mc = 0;

pos += strlen("<bigscreenmessages m=");
while ( ! isdigit(*pos) ) pos++;
char * erunp = pos;
while ( (*erunp) != '>') erunp ++;
*erunp = '\0';
mc = atoi(pos);
*erunp = '>';
pos = erunp;

if ( mc != MAX_BIG_SCREEN_MESSAGES )
        ErrorMessage(__FUNCTION__, "Size mismatch for max number of big screen messages, file %d vs. game %d.\n", PLEASE_INFORM, IS_FATAL, mc, MAX_BIG_SCREEN_MESSAGES);

while ( *pos != '\n' ) pos ++;
pos++;
#endif
}

void save_automap_data(const char * tag, automap_data_t * automapdata, int size)
{
fprintf(SaveGameFile, "<automap nl=%d sx=%d sy=%d>\n", MAX_LEVELS, 100, 100);
int i,j,k;
for ( i = 0; i < MAX_LEVELS; i ++)
   {
   fprintf(SaveGameFile, "%d\n", i);
   for ( j = 0; j < 100; j ++ )
       {
       for ( k = 0; k < 100; k += 5)
	       fprintf(SaveGameFile, "%hhd %hhd %hhd %hhd %hhd ", automapdata[i][j][k], automapdata[i][j][k+1], automapdata[i][j][k+2], automapdata[i][j][k+3],
		       automapdata[i][j][k+4]);
       fprintf(SaveGameFile, "\n");
       }
   }
fprintf(SaveGameFile, "</automap>\n");
}

void read_automap_data_t_array(char * buffer, char * tag, automap_data_t * automapdata, int size)
{
    char * pos = strstr(buffer, "<automap nl=");
    if ( ! pos ) ErrorMessage ( __FUNCTION__, "Unable to find automap data\n", PLEASE_INFORM, IS_FATAL);
    char * epos = strstr(pos, "</automap>\n");
    if ( ! epos ) ErrorMessage ( __FUNCTION__, "Unable to find automap data end\n", PLEASE_INFORM, IS_FATAL);
    epos += strlen("</automap>\n");
    char savechar = *(epos + 1);
    *(epos+1) = '\0';
    char *runp = pos + 1;
    runp += strlen("<automap");


    while ( *runp != '\n' ) runp++;
    *runp = '\0';
    int nl, sx, sy;
    sscanf(pos, "<automap nl=%d sx=%d sy=%d>", &nl, &sx, &sy);
  
    if ( nl != MAX_LEVELS )
	ErrorMessage(__FUNCTION__, "Number of levels mismatch when reading automap data : file %d, game %d\n", PLEASE_INFORM, IS_FATAL, nl, MAX_LEVELS);
    if ( sx != 100 || sy != 100 )
	ErrorMessage(__FUNCTION__, "Size mismatch when reading automap data.\n", PLEASE_INFORM, IS_FATAL);

    *runp = '\n';
    runp++;
    char * erunp;
    int i = 0, j = 0, k = 0;
    while ( i < MAX_LEVELS )
	{
	/* we're on level number */
	while ( *runp != '\n' ) runp++;
	runp++;
	/*now on the actual data*/
	j = 0;
	while ( j < 100 )
	    {
	    k = 0;
	    while ( k < 100 )
		{
		while ( ! isdigit(*runp) ) runp++;
		erunp = runp + 1;
		while ( *erunp != ' ' ) erunp++;
		*erunp = '\0';

		(automapdata)[i][j][k] = atoi(runp);
		*erunp = ' ';
		runp = erunp;
		k++;
		}
	    while ( *runp != '\n' ) runp++;
	    runp ++;
	    j ++;
	    }

	while ( *runp != '\n' ) runp++;
	i++;
	}
    *(epos+1) = savechar;

}

#undef _saveloadgame_c
