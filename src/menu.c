/* 
 *
 *   Copyright (c) 1994, 2002, 2003 Johannes Prix
 *   Copyright (c) 1994, 2002 Reinhard Prix
 *   Copyright (c) 2004-2007 Arthur Huillet 
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
 * This file contains all menu functions and their subfunctions
 */

#define _menu_c

#include "system.h"

#include "defs.h"
#include "struct.h"
#include "global.h"
#include "proto.h"

#include "scandir.h"

int New_Game_Requested = FALSE ;

int Single_Player_Menu (void);
void Options_Menu (void);

EXTERN void LevelEditor(void);
extern int MyCursorX;
extern int MyCursorY;
extern int load_game_command_came_from_inside_running_game;

#define SELL_PRICE_FACTOR (0.25)
#define REPAIR_PRICE_FACTOR (0.5)

#define SINGLE_PLAYER_STRING "Play"
#define LOAD_EXISTING_HERO_STRING _("Your characters: ")
#define DELETE_EXISTING_HERO_STRING _("Select character to delete: ")

#define MENU_SELECTION_DEBUG 1

#define AUTO_SCROLL_RATE (0.02f)

/**
 * This function tells over which menu item the mouse cursor would be,
 * if there were infinitely many menu items.
 */
int
MouseCursorIsOverMenuItem( int first_menu_item_pos_y , int h )
{
    int PureFraction;

    PureFraction = ( GetMousePos_y ()  - first_menu_item_pos_y ) / h ;

    //--------------------
    // Now it can be that the pure difference is negative or that it is positive.
    // However we should not always round thowards zero here, but rather always to
    // the next LOWER integer!  This will be done here:
    //
    if ( ( GetMousePos_y ()  - first_menu_item_pos_y ) < 0 )
	PureFraction--;
    else
	PureFraction++;
    
    return ( PureFraction );
    
}; // void MouseCursorIsOverMenuItem( first_menu_item_pos_y )

/**
 *
 *
 */
void
print_menu_text ( char* InitialText , char* MenuTexts[] , int first_menu_item_pos_y , int background_code , void* MenuFont ) 
{
    char open_gl_string [ 2000 ];
    int h = FontHeight ( GetCurrentFont ( ) );
    
    //--------------------
    // We need to prepare the background for the menu, so that
    // it can be accessed with proper speed later...
    //
    InitiateMenu ( background_code );
    
    //--------------------
    // Maybe if this is the very first startup menu, we should also print
    // out some status variables like whether using OpenGL or not DIRECTLY
    // ON THE MENU SCREEN...
    //
    if ( ! strcmp ( MenuTexts [ 0 ] , SINGLE_PLAYER_STRING ) )
    {
	SetCurrentFont ( FPS_Display_BFont );
	RightPutString( Screen , GameConfig . screen_height - 1 * FontHeight ( GetCurrentFont() ) , VERSION );
	// printf ("\n%s %s  \n", PACKAGE, VERSION);
	sprintf ( open_gl_string , _("OpenGL support compiled: ") );
#ifdef HAVE_LIBGL
	strcat ( open_gl_string , _(" YES ") ) ;
#else
	strcat ( open_gl_string , _(" NO ") ) ;
#endif
	LeftPutString( Screen , GameConfig . screen_height - 2 * FontHeight ( GetCurrentFont() ) , open_gl_string );
	sprintf ( open_gl_string , _("OpenGL output active: ") );
	if ( use_open_gl )
	    strcat ( open_gl_string , _(" YES ") ) ;
	else
	    strcat ( open_gl_string , _(" NO ") ) ;
	LeftPutString( Screen , GameConfig . screen_height - FontHeight ( GetCurrentFont() ) , open_gl_string );
    }
    
    //--------------------
    // Now that the possible font-changing small info printing is
    // done, we can finally set the right font for the menu itself.
    //
    if ( MenuFont == NULL ) SetCurrentFont ( Menu_BFont );
    else SetCurrentFont ( (BFont_Info*) MenuFont );
    h = FontHeight ( GetCurrentFont() );
    
}; // void print_menu_text ( ... )

/**
 * This function performs a menu for the player to select from, using the
 * keyboard only, currently, sorry.
 *
 * This function EXPECTS, that MenuTexts[] is an array of strings, THE 
 * LAST OF WHICH IS AN EMPTY STRING TO DENOTE THE END OF THE ARRAY!  If 
 * this is not respected, segfault errors are likely.
 * 
 */
int
DoMenuSelection( char* InitialText , char **MenuTexts, int FirstItem , int background_code , void* MenuFont )
{
    int h;
    int i;
    static int MenuPosition = 1;
    int NumberOfOptionsGiven;
    int LongestOption;
    SDL_Rect HighlightRect;
    SDL_Rect BackgroundRect;
    int first_menu_item_pos_y;
    int* MenuTextWidths;
    float auto_scroll_start = 0.0f;
    int auto_scroll_run = TRUE;
    int ret = -1;
    int old_game_status = game_status;
    SDL_Event event;

    game_status = INSIDE_MENU;

    //--------------------
    // At first we hide the system mouse cursor, because we want to use
    // our own creation in the menus too...
    //
    make_sure_system_mouse_cursor_is_turned_off();
    
    //--------------------
    // We set the given font, if appropriate, and set the font height variable...
    //
    if ( MenuFont != NULL )
	SetCurrentFont ( MenuFont );
    h = FontHeight ( GetCurrentFont ( ) );
    
    //--------------------
    // Some menus are intended to start with the default setting of the
    // first menu option selected.  However this is not always desired.
    // It might happen, that a submenu returns to the upper menu and then
    // the upper menu should not be reset to the first position selected.
    // For this case we have some special '-1' entry reserved as the marked
    // menu entry.  This means, taht the menu position from last time will
    // simply be re-used.
    //
    if ( FirstItem != (-1) ) MenuPosition = FirstItem;
    
    //--------------------
    // First thing we do is find out how may options we have
    // been given for the menu
    // For each option, we compute and store its pixel-width
    // Then the longest one is found
    for ( i = 0 ; TRUE ; ++i )
    {
        if ( MenuTexts[ i ][ 0 ] == '\0' ) break;
    }
    NumberOfOptionsGiven = i;
    MenuTextWidths = (int*)malloc(sizeof(int)*(NumberOfOptionsGiven+1));

    LongestOption = 0 ;
    for ( i = 0 ; i < NumberOfOptionsGiven ; ++i )
    {
        MenuTextWidths[ i ] = TextWidth( MenuTexts[ i ] );
        if ( MenuTextWidths[ i ] > LongestOption )
            LongestOption = MenuTextWidths[ i ];
    }
    MenuTextWidths[ NumberOfOptionsGiven ] = 0;

    //--------------------
    // Clamp the longest option to the available width on the screen
    // (50 pixels around the menu's background, and 1.5 fontheigth around the text)
    LongestOption = min( GameConfig . screen_width - 100 - 3 * h, LongestOption );
    
    //--------------------
    // In those cases where we don't reset the menu position upon 
    // initalization of the menu, we must check for menu positions
    // outside the bounds of the current menu.
    //
    if ( MenuPosition > NumberOfOptionsGiven ) MenuPosition = 1 ; 
    
    first_menu_item_pos_y = ( GameConfig . screen_height - NumberOfOptionsGiven * h ) / 2 ;
    
    print_menu_text ( InitialText , MenuTexts , first_menu_item_pos_y , background_code , MenuFont ) ;
    
    StoreMenuBackground ( 0 );
    
    BackgroundRect.x = ( GameConfig . screen_width - LongestOption - 3 * h ) / 2;
    BackgroundRect.y = first_menu_item_pos_y - 50 ;
    BackgroundRect.w = LongestOption + 3 * h ;
    BackgroundRect.h = ( h * NumberOfOptionsGiven ) + 100 ;

    while ( 1 )
    {
    	save_mouse_state();

	//--------------------
	// We write out the normal text of the menu, either by doing it once more
	// in the open_gl case or by restoring what we have saved earlier, in the 
	// SDL output case.
	//
        RestoreMenuBackground ( 0 );
	
	//--------------------
	// Maybe we should display some thumbnails with the saved games entries?
	// But this will only apply for the load_hero and the delete_hero menus...
	//
	if ( ( ( ! strcmp ( InitialText , LOAD_EXISTING_HERO_STRING ) ) ||
	     ( ! strcmp ( InitialText , DELETE_EXISTING_HERO_STRING ) ) ) && 
             strcmp ( MenuTexts [ MenuPosition - 1], _("[down]")) && 
	     strcmp ( MenuTexts [ MenuPosition - 1 ], _("[up]")) && 
	     strcmp ( MenuTexts [ MenuPosition - 1 ], " ") && 
	     MenuPosition < NumberOfOptionsGiven )
	{
	    //--------------------
	    // We load the thumbnail, or at least we try to do it...
	    //
	    LoadAndShowThumbnail ( MenuTexts [ MenuPosition - 1 ] );
	    LoadAndShowStats ( MenuTexts [ MenuPosition - 1 ] );
	}
	
        //--------------------
        // Draw the menu's  background
        //
        ShadowingRectangle ( Screen, BackgroundRect );

        //-------------------
        // Display each option
        //
        for ( i = 0 ; TRUE ; i ++ )
        {
            char* str = NULL;
            int free_needed = FALSE;
            int width = 0;

  	    if ( MenuTextWidths[ i ] == 0 ) break;

            //--------------------
            // Define the actual text to display
            // If the text is too long, handle autoscroll and clip it
            if ( MenuTextWidths[ i ] > LongestOption ) 
            {
                if ( i == MenuPosition - 1 )
                {   // selected option -> autoscroll
                    str = strdup(MenuTexts[i] + (int)auto_scroll_start);
                } 
                else 
                {   // unselected option -> no scroll
                    str = strdup(MenuTexts[i]); 
                }
                free_needed = TRUE;
	        if ( CutDownStringToMaximalSize ( str , LongestOption ) == FALSE ) {
                    // if cutting was not needed, we are at the end of the text,
                    // so stop autoscroll
                     if ( i == MenuPosition - 1 ) auto_scroll_run = FALSE;
                }
                width = LongestOption;
            } 
            else 
            {
                str = MenuTexts[ i ];
                width = MenuTextWidths[ i ];
            }

            //--------------------
            // Depending on what highlight method has been used, we so some highlighting
	    // of the currently selected menu options location on the screen...
	    //
            if ( i == MenuPosition - 1 ) 
            {    
                HighlightRect.x = ( GameConfig . screen_width - width ) / 2 - h ;
	        HighlightRect.y = first_menu_item_pos_y + ( MenuPosition - 1 ) * h ;
	        HighlightRect.w = width + 2 * h ;
	        HighlightRect.h = h;		    
	        HighlightRectangle ( Screen , HighlightRect );

                if ( auto_scroll_run == TRUE ) {
                    auto_scroll_start += AUTO_SCROLL_RATE;
		}
	    }

            //--------------------
            // Draw the option's text
            CenteredPutString ( Screen ,  first_menu_item_pos_y + i * h , str );
          
            if ( free_needed == TRUE ) free(str);
	}
        if ( strlen( InitialText ) > 0 ) 
	    DisplayText ( InitialText , 50 , 50 , NULL , TEXT_STRETCH );
	
	//--------------------
	// Now the mouse cursor must be brought to the screen
	//
	make_sure_system_mouse_cursor_is_turned_off();
	blit_our_own_mouse_cursor();

	//--------------------
	// Image should be ready now, so we can show it...
	//
	our_SDL_flip_wrapper();
	
	//--------------------
	// Now it's time to handle the possible keyboard and mouse 
	// input from the user...
	//
        int old_menu_position = MenuPosition;	

	if (SDL_PollEvent(&event)) {

	    if (event.type == SDL_QUIT) {
		Terminate(0);
	    }

	    //(clever?) hack : mouse wheel up and down behave
	    //exactly like UP and DOWN arrow, so we mangle the event
	    if (event.type == SDL_MOUSEBUTTONDOWN) {
		switch(event.button.button) {
		    case SDL_BUTTON_WHEELUP:
			event.type = SDL_KEYDOWN;
			event.key.keysym.sym = SDLK_UP;
			break;
		    case SDL_BUTTON_WHEELDOWN:
			event.type = SDL_KEYDOWN;
			event.key.keysym.sym = SDLK_DOWN;
			break;
		    default: break;
		}
	    }	

	    if (event.type == SDL_KEYDOWN) {
		switch(event.key.keysym.sym) {
		    case SDLK_ESCAPE:
			MenuItemDeselectedSound();
			ret = -1;
			goto out;
			break;

		    case SDLK_RETURN:
		    case SDLK_SPACE:
		    case SDLK_LEFT:
		    case SDLK_RIGHT:
			//--------------------
			// The space key or enter key or arrow keys all indicate, that
			// the user has made a selection.
			//
			//
			MenuItemSelectedSound();
			ret = MenuPosition;
			goto out;
			break;

		    case SDLK_UP:
			if (MenuPosition > 1) MenuPosition--;
			MoveMenuPositionSound();
			HighlightRect.x = UNIVERSAL_COORD_W(320) ; // ( TextWidth ( MenuTexts [ MenuPosition - 1 ] ) ) / 2 ;
			HighlightRect.y = first_menu_item_pos_y + ( MenuPosition - 1 ) * h ;
			SDL_WarpMouse ( HighlightRect.x , HighlightRect.y );
			break;

		    case SDLK_DOWN:
			if ( MenuPosition < NumberOfOptionsGiven ) MenuPosition++;
			MoveMenuPositionSound();
			HighlightRect.x = UNIVERSAL_COORD_W(320) ; // ( TextWidth ( MenuTexts [ MenuPosition - 1 ] ) ) / 2 ;
			HighlightRect.y = first_menu_item_pos_y + ( MenuPosition - 1 ) * h ;
			SDL_WarpMouse ( HighlightRect.x , HighlightRect.y );
			break;

		    default: break;
		}
	    }

	}

	if ( MouseLeftClicked() )
	    {
	    //--------------------
	    // Only when the mouse click really occured on the menu do we
	    // interpret it as a menu choice.  Otherwise we'll just ignore
	    // it.
	    //
	    if ( MouseCursorIsOverMenuItem( first_menu_item_pos_y , h ) == MenuPosition )
		{
		MenuItemSelectedSound();
		ret = MenuPosition;
		goto out;
		}
	    }

	MenuPosition = MouseCursorIsOverMenuItem( first_menu_item_pos_y , h );
	if ( MenuPosition < 1 ) MenuPosition = 1 ;
	if ( MenuPosition > NumberOfOptionsGiven ) MenuPosition = NumberOfOptionsGiven ;

        //--------------------
        // If the selected option has changed, halt eventual current autoscrolling
	if ( MenuPosition != old_menu_position ) {
            auto_scroll_run = TRUE;
            auto_scroll_start = 0.0f;
        }

	//--------------------
	// At this the while (1) overloop ends.  But for the menu, we really do not
	// need to hog the CPU.  Therefore some waiting should be introduced here.
	//
	SDL_Delay (1);
    }
    
out:
    free(MenuTextWidths);
    game_status = old_game_status;
    return ret;
}; // int DoMenuSelection( ... )

/**
 * This function performs a menu for the player to select from, using the
 * keyboard or mouse wheel.
 */
int
ChatDoMenuSelectionFlagged( char* InitialText , char* MenuTexts[ MAX_ANSWERS_PER_PERSON] , 
			    unsigned char Chat_Flags[ MAX_ANSWERS_PER_PERSON ] , int FirstItem , 
			    int background_code , void* MenuFont , enemy* ChatDroid )
{
  int MenuSelection = (-1) ;
  char* FilteredChatMenuTexts[ MAX_ANSWERS_PER_PERSON ] ;
  int i;
  int use_counter = 0;

  DebugPrintf ( 2 , "\nBEFORE:  First Item now: %d." , FirstItem );

  //--------------------
  // We filter out those answering options that are allowed by the flag mask
  //
  DebugPrintf ( MENU_SELECTION_DEBUG , "\n%s(): %d \n" , __FUNCTION__ , FirstItem ) ; 
  for ( i = 0 ; i < MAX_ANSWERS_PER_PERSON ; i ++ )
    {
      FilteredChatMenuTexts [ i ] = "" ; 
      if ( Chat_Flags[ i ] ) 
	{

	  DebugPrintf ( MENU_SELECTION_DEBUG , "%2d. " , i ) ; 
	  DebugPrintf ( MENU_SELECTION_DEBUG , "%s\n" , MenuTexts [ i ] ) ; 

	  fflush ( stdout );
	  fflush ( stderr );

	  FilteredChatMenuTexts[ use_counter ] = MenuTexts[ i ] ;
	  use_counter++;
	}
    }

  DebugPrintf ( 2 , "\nMIDDLE:  First Item now: %d." , FirstItem );

  //--------------------
  // Now we do the usual menu selection, using only the activated chat alternatives...
  //
  MenuSelection = ChatDoMenuSelection( FilteredChatMenuTexts , 
				       FirstItem , MenuFont , ChatDroid );

  //--------------------
  // Now that we have an answer, we must transpose it back to the original array
  // of all theoretically possible answering possibilities.
  //
  if ( MenuSelection != (-1) )
    {
      use_counter = 0 ;
      for ( i = 0 ; i < MAX_ANSWERS_PER_PERSON ; i ++ )
	{

	  if ( Chat_Flags[ i ] ) 
	    {
	      FilteredChatMenuTexts[ use_counter ] = MenuTexts[ i ] ;
	      use_counter++;
	      if ( MenuSelection == use_counter ) 
		{
		  DebugPrintf( 1 , "\nOriginal MenuSelect: %d. \nTransposed MenuSelect: %d." , 
			       MenuSelection , i+1 );
		  return ( i+1 );
		}
	    }
	}
    }

  return ( MenuSelection );
}; // int ChatDoMenuSelectionFlagged( char* InitialText , char* MenuTexts[ MAX_ANSWERS_PER_PERSON] , ... 

/**
 * Without destroying or changing anything, this function should determine
 * how many lines of text it takes to write a string with the current 
 * font into the given rectangle, provided one would start at the left
 * side as one usually does...
 */
int
GetNumberOfTextLinesNeeded ( char* GivenText, SDL_Rect GivenRectangle , float text_stretch )
{
    int BackupOfMyCursorX, BackupOfMyCursorY;
    int TextLinesNeeded;
    int TestPosition;
    int stored_height ;
    
    //--------------------
    // If we receive an empty string, we print out a warning message and then
    // return one line as the required amount of lines.
    //
    if ( strlen ( GivenText ) <= 1 )
    {
	/*
	ErrorMessage ( __FUNCTION__  , "\
Warning.  Received empty or nearly empty string!",
				   NO_NEED_TO_INFORM, IS_WARNING_ONLY );
	*/
	return ( 1 ) ;
    }
    
    //--------------------
    // First we make a backup of everything, so that we don't destory anything.
    //
    display_char_disabled = TRUE ;
    BackupOfMyCursorX = MyCursorX;
    BackupOfMyCursorY = MyCursorY;
    
    //--------------------
    // Now in our simulated environment, we can blit the Text and see how many lines it takes...
    //
    MyCursorX = GivenRectangle . x ;
    MyCursorY = GivenRectangle . y ;
    TestPosition = MyCursorY ;
    
    stored_height = GivenRectangle.h ;
    GivenRectangle.h = 32000 ;
    TextLinesNeeded = DisplayText ( GivenText , GivenRectangle.x , GivenRectangle.y , &GivenRectangle , text_stretch );
    GivenRectangle.h = stored_height ;

    //--------------------
    // Now that we have found our solution, we can restore everything back to normal
    //
    // RestoreMenuBackground ( 1 ) ;
    display_char_disabled = FALSE ;
    
    MyCursorX = BackupOfMyCursorX;
    MyCursorY = BackupOfMyCursorY;
    
    return ( TextLinesNeeded );
    
}; // int GetNumberOfTextLinesNeeded ( MenuTexts [ i ] , Choice_Window )

/**
 *
 * This function performs a menu for the player to select from, using the
 * keyboard or mouse wheel.
 *
 * The rest of the menu is made particularly unclear by the fact that there
 * can be some multi-line options too and also there is scrolling up and
 * down possible, when there are more menu options than fit onto one
 * dialog options secection window for the player to click from.
 *
 */
int
ChatDoMenuSelection( char* MenuTexts[ MAX_ANSWERS_PER_PERSON ] , 
		     int FirstItem , void* MenuFont , enemy* ChatDroid )
{
    int h = FontHeight (GetCurrentFont());
    int i ;
    static int menu_position_to_remember = 1;
    int NumberOfOptionsGiven;
#define ITEM_DIST 50
    int MenuPosX [ MAX_ANSWERS_PER_PERSON ] ;
    int MenuPosY [ MAX_ANSWERS_PER_PERSON ] ;
    int MenuOptionLineRequirement [ MAX_ANSWERS_PER_PERSON ] ;
    SDL_Rect Choice_Window;
    SDL_Rect HighlightRect;
    int MaxLinesInMenuRectangle;
    int OptionOffset = 0 ;
    int SpaceUsedSoFar;
    int BreakOffCauseAllDisplayed ;
    int BreakOffCauseNoRoom = FALSE ;
    int LastOptionVisible = 0 ;
    int MenuLineOfMouseCursor;
    int ThisOptionEnd;
    int cursors_menu_position = - 1000 ;
    SDL_Event event; 
    int ret = -1;
    int old_game_status = game_status;

    game_status = INSIDE_MENU;
    //--------------------
    // First we initialize the menu positions
    //
    for ( i = 0 ; i < MAX_ANSWERS_PER_PERSON ; i ++ )
    {
	MenuPosX [ i ] = 260 ; 
	MenuPosY [ i ] =  90 + i * ITEM_DIST ; 
	MenuOptionLineRequirement [ i ] = 0;
    }
    
    //--------------------
    // Now we set some viable choice window and we compute the maximum number of lines
    // that will still fit well into the choice window.
    //
    Choice_Window . x = 35*GameConfig . screen_width/640;
    Choice_Window . y = 335*GameConfig . screen_height/480; 
    Choice_Window . w = ( 640 - 70 ) * GameConfig . screen_width / 640 ; 
    Choice_Window . h = 118 * GameConfig . screen_height / 480 ;
    MaxLinesInMenuRectangle = Choice_Window . h / ( FontHeight ( GetCurrentFont() ) * TEXT_STRETCH ) ;
    DebugPrintf ( 1 , "\nComputed number of lines in choice window at most: %d." , MaxLinesInMenuRectangle );
    
    //--------------------
    // We don't need the system mouse cursor, as we do have our own for
    // the same purpose.
    //
    make_sure_system_mouse_cursor_is_turned_off();
    
    if ( FirstItem != (-1) ) menu_position_to_remember = FirstItem;
    
    
    //--------------------
    // First thing we do is find out how may options we have
    // been given for the menu
    //
    DebugPrintf ( MENU_SELECTION_DEBUG , "\n%s(): %d \n" , __FUNCTION__ , FirstItem ) ; 
    for ( i = 0 ; i < MAX_ANSWERS_PER_PERSON ; i ++ )
    {
	DebugPrintf ( MENU_SELECTION_DEBUG , "%2d. " , i ) ; 
	DebugPrintf ( MENU_SELECTION_DEBUG , MenuTexts [ i ] ) ; 
	DebugPrintf ( MENU_SELECTION_DEBUG , "\n" ) ; 
	fflush ( stdout );
	fflush ( stderr );
	
	if ( strlen( MenuTexts[ i ] ) == 0 ) break;
    }
    NumberOfOptionsGiven = i ;
    
    
    //--------------------
    // We need to prepare the background for the menu, so that
    // it can be accessed with proper speed later...
    //
    SDL_SetClipRect( Screen, NULL );
    display_current_chat_protocol ( CHAT_DIALOG_BACKGROUND_PICTURE_CODE , ChatDroid , FALSE );
    StoreMenuBackground ( 0 );
    
    //--------------------
    // Now that the possible font-changing background assembling is
    // done, we can finally set the right font for the menu itself.
    //
    if ( MenuFont == NULL ) SetCurrentFont ( Menu_BFont );
    else SetCurrentFont ( (BFont_Info*) MenuFont );
    h = FontHeight ( GetCurrentFont() );
    
    OptionOffset = 0 ;
    while ( 1 )
	{
	SDL_Delay(1);
	RestoreMenuBackground ( 0 );
	save_mouse_state();

	//--------------------
	// Now that the possible font-changing chat protocol display is
	// done, we can finally set the right font for the menu itself.
	//
	if ( MenuFont == NULL ) SetCurrentFont ( Menu_BFont );
	else SetCurrentFont ( (BFont_Info*) MenuFont );
	h = FontHeight ( GetCurrentFont() );

	//--------------------
	// We blit to the screen all the options that are not empty and that still fit
	// onto the screen
	//
	SpaceUsedSoFar = 0 ;
	for ( i = OptionOffset ; i < MAX_ANSWERS_PER_PERSON ; i ++ )
	    {
	    //--------------------
	    // If all has been displayed already, we quit blitting...
	    //
	    if ( strlen( MenuTexts[ i ] ) == 0 ) 
		{
		BreakOffCauseAllDisplayed = TRUE ;
		BreakOffCauseNoRoom = FALSE ;
		LastOptionVisible = i ;
		break;
		}
	    //--------------------
	    // If there is not enough room any more, we quit blitting...
	    //
	    if ( SpaceUsedSoFar  > MaxLinesInMenuRectangle * FontHeight ( GetCurrentFont () ) ) 
		{
		BreakOffCauseAllDisplayed = FALSE ;
		BreakOffCauseNoRoom = TRUE ;
		LastOptionVisible = i ;
		break;
		}

	    //--------------------
	    // Now that we know, that there is enough room, we can blit the next menu option.
	    //
	    MenuPosX [ i ] = Choice_Window.x;
	    MenuPosY [ i ] = Choice_Window.y + SpaceUsedSoFar;
	    MenuOptionLineRequirement [ i ]  = DisplayText ( MenuTexts [ i ] , Choice_Window.x , Choice_Window.y + SpaceUsedSoFar , &Choice_Window , TEXT_STRETCH );
	    SpaceUsedSoFar += MenuOptionLineRequirement [ i ] * ( FontHeight ( GetCurrentFont() ) * TEXT_STRETCH );
	    }


	//--------------------
	// We highlight the currently selected option with a highlighting rectangle
	//
	// (and we add some security against 'empty' chat selection menus causing
	// some segfaults rather easily...)
	//
	DebugPrintf ( 1 , "\n%s(): menu_position_to_remember: %d." , __FUNCTION__ , menu_position_to_remember );
	DebugPrintf ( 1 , "\n%s(): FirstItem: %d." , __FUNCTION__ , FirstItem );
	if ( menu_position_to_remember <= 0 ) 
	    menu_position_to_remember = 1 ;
	HighlightRect.x = MenuPosX[ menu_position_to_remember -1 ] - 0 * h ;
	HighlightRect.y = MenuPosY[ menu_position_to_remember -1 ] ;
	HighlightRect.w = TextWidth ( MenuTexts [ menu_position_to_remember - 1 ] ) + 0 * h ;
	if ( HighlightRect . w > 580 * GameConfig . screen_width / 640 ) HighlightRect . w = 580 * GameConfig . screen_width / 640 ;
	HighlightRect.h = MenuOptionLineRequirement [ menu_position_to_remember - 1 ] * ( FontHeight ( GetCurrentFont() ) * TEXT_STRETCH ) ;	    
	if ( HighlightRect . h + HighlightRect.y > UNIVERSAL_COORD_H(457) ) HighlightRect . h = UNIVERSAL_COORD_H(457) - HighlightRect.y;
	HighlightRectangle ( Screen , HighlightRect );
	// Display again the highlighted line
	DisplayText ( MenuTexts [ menu_position_to_remember -1 ] , MenuPosX [ menu_position_to_remember - 1 ], MenuPosY [ menu_position_to_remember - 1 ] , &Choice_Window , TEXT_STRETCH);	  

	if ( BreakOffCauseNoRoom ) ShowGenericButtonFromList ( SCROLL_DIALOG_MENU_DOWN_BUTTON );
	if ( OptionOffset ) ShowGenericButtonFromList ( SCROLL_DIALOG_MENU_UP_BUTTON );

	//--------------------
	// Now the mouse cursor must be brought to the screen
	//
	make_sure_system_mouse_cursor_is_turned_off();
	blit_our_own_mouse_cursor();

	//--------------------
	// Now everything should become visible!
	//
	our_SDL_flip_wrapper();

	// Wait for something to happen
	SDL_WaitEvent(&event);      

	if (event.type == SDL_QUIT) {
	    Terminate(0);
	}
	
	//(clever?) hack : mouse wheel up and down behave
	//exactly like UP and DOWN arrow or PAGEUP/PAGEDOWN, so we mangle the event
	if (event.type == SDL_MOUSEBUTTONDOWN) {
	    int in_blabla_screen = 0;
	    if (( GetMousePos_x() > CHAT_SUBDIALOG_WINDOW_X && GetMousePos_x() < CHAT_SUBDIALOG_WINDOW_X + CHAT_SUBDIALOG_WINDOW_W) &&
		( GetMousePos_y() > CHAT_SUBDIALOG_WINDOW_Y && GetMousePos_y() < CHAT_SUBDIALOG_WINDOW_Y + CHAT_SUBDIALOG_WINDOW_H))
		in_blabla_screen = 1;

	    switch(event.button.button) {
		case SDL_BUTTON_WHEELUP:
		    event.type = SDL_KEYDOWN;
		    event.key.keysym.sym = in_blabla_screen ? SDLK_PAGEUP : SDLK_UP;
		    break;
		case SDL_BUTTON_WHEELDOWN:
		    event.type = SDL_KEYDOWN;
		    event.key.keysym.sym = in_blabla_screen ? SDLK_PAGEDOWN : SDLK_DOWN;
		    break;
		default: 
		    break;
	    }
	}	

	if (event.type == SDL_KEYDOWN) {
	    switch (event.key.keysym.sym) {
		case SDLK_UP:
		    if ( menu_position_to_remember > OptionOffset + 1 ) {
			SDL_WarpMouse ( GetMousePos_x () , MenuPosY [ menu_position_to_remember - 2 ] ) ;
			MoveMenuPositionSound();	    
		    }
		    else if ( OptionOffset > 0 ) {
			OptionOffset -- ; 
			MoveMenuPositionSound();	    
		    }
		    break;

		case SDLK_DOWN:
		    if ( menu_position_to_remember < LastOptionVisible ) { 
			SDL_WarpMouse ( GetMousePos_x () , MenuPosY [ menu_position_to_remember ] );
		    }
		    else {
			if ( BreakOffCauseNoRoom ) {
			    OptionOffset++;
			    MoveMenuPositionSound();
			}
			SDL_WarpMouse ( GetMousePos_x () , MenuPosY [ menu_position_to_remember - 1 ] );
		    }
		    break;

		case SDLK_PAGEUP:
		    chat_protocol_scroll_override_from_user -= 3;
		    display_current_chat_protocol ( CHAT_DIALOG_BACKGROUND_PICTURE_CODE , ChatDroid , FALSE );
		    StoreMenuBackground(0);
		    break;
		
		case SDLK_PAGEDOWN:
		    chat_protocol_scroll_override_from_user += 3 ;
		    display_current_chat_protocol ( CHAT_DIALOG_BACKGROUND_PICTURE_CODE , ChatDroid , FALSE );
		    StoreMenuBackground(0);
		    break;

		case SDLK_ESCAPE:
		    RestoreMenuBackground(0);
		    our_SDL_flip_wrapper();
		    ret = -1;
		    goto out;
		    break;

		case SDLK_RETURN:
		case SDLK_SPACE:
		    RestoreMenuBackground(0);
		    our_SDL_flip_wrapper();
		    ret = menu_position_to_remember;
		    goto out;
		    break;

		default: 
		    break;

	    }
	}
	
	if ( MouseLeftClicked() ) {
	    //--------------------
	    // First we see if there was perhaps a click on one of the active scroll buttons
	    //
	    if ( ( MouseCursorIsOnButton ( SCROLL_DIALOG_MENU_DOWN_BUTTON , GetMousePos_x ()  , GetMousePos_y ()  ) ) &&
		    ( BreakOffCauseNoRoom ) )
		{
		OptionOffset ++ ;

		}
	    else if ( ( MouseCursorIsOnButton ( SCROLL_DIALOG_MENU_UP_BUTTON , GetMousePos_x ()  , GetMousePos_y ()  ) ) &&
		    ( OptionOffset ) )
		{
		OptionOffset -- ;
		}
	    else if ( MouseCursorIsOnButton ( CHAT_PROTOCOL_SCROLL_UP_BUTTON , 
			GetMousePos_x ()  , 
			GetMousePos_y ()  ) )
		{
		chat_protocol_scroll_override_from_user -- ;
		display_current_chat_protocol ( CHAT_DIALOG_BACKGROUND_PICTURE_CODE , ChatDroid , FALSE );
		StoreMenuBackground(0);
		}
	    else if ( MouseCursorIsOnButton ( CHAT_PROTOCOL_SCROLL_DOWN_BUTTON , 
			GetMousePos_x ()  , 
			GetMousePos_y ()  ) )
		{
		chat_protocol_scroll_override_from_user ++ ;
		display_current_chat_protocol ( CHAT_DIALOG_BACKGROUND_PICTURE_CODE , ChatDroid , FALSE );
		StoreMenuBackground(0);
		}
	    //--------------------
	    // If not, then maybe it was a click into the options window.  That alone
	    // would be enough to call it a valid user decision.
	    //
	    else
		{
		//--------------------
		// Now if the click has occured within the lines of the menu, we will count
		// this as a valid choice of the user.
		//
		MenuLineOfMouseCursor = 
		    MouseCursorIsOverMenuItem ( MenuPosY [ OptionOffset ] , FontHeight ( GetCurrentFont() ) * TEXT_STRETCH ) ;
		if ( ( MenuLineOfMouseCursor >= 1 ) && ( MenuLineOfMouseCursor <= MaxLinesInMenuRectangle ) )
		    {
		    RestoreMenuBackground ( 0 );
		    our_SDL_flip_wrapper();
		    ret = menu_position_to_remember;
		    goto out;
		    }
		}

	}

	MenuLineOfMouseCursor = 
	    MouseCursorIsOverMenuItem ( MenuPosY [ OptionOffset ] , FontHeight ( GetCurrentFont() ) * TEXT_STRETCH ) ;
	if ( MenuLineOfMouseCursor < 1 ) MenuLineOfMouseCursor = 1 ;

	cursors_menu_position = 1 ;

	ThisOptionEnd = MenuPosY [ 0 ] ;
	for ( i = OptionOffset ; i <= LastOptionVisible ; i ++ )
	    {

	    ThisOptionEnd += MenuOptionLineRequirement [ i ] * ( FontHeight ( GetCurrentFont() ) * TEXT_STRETCH ) ;

	    if ( GetMousePos_y ()  < ThisOptionEnd )
		{
		cursors_menu_position = i + 1 ; // MouseCursorIsOverMenuItem( MenuPosY [ 0 ] , MenuPosY [ 1 ] - MenuPosY [ 0 ] );
		break;
		}
	    }


	if ( cursors_menu_position > LastOptionVisible ) 
	    cursors_menu_position = LastOptionVisible ;

	//--------------------
	// If the mouse cursor was on one of the possible lines, than we can try to translate
	// it into a real menu position
	//
	if ( ( MenuLineOfMouseCursor >= 0 ) && ( MenuLineOfMouseCursor <= MaxLinesInMenuRectangle ) )
	    {
	    ThisOptionEnd = MenuPosY [ 0 ] ;
	    for ( i = OptionOffset ; i <= LastOptionVisible ; i ++ )
		{

		ThisOptionEnd += MenuOptionLineRequirement [ i ] * ( FontHeight ( GetCurrentFont() ) * TEXT_STRETCH ) ;

		if ( GetMousePos_y ()  < ThisOptionEnd )
		    {
		    menu_position_to_remember = i + 1 ; 
		    break;
		    }
		}
	    }

	if ( menu_position_to_remember < OptionOffset + 1 ) menu_position_to_remember = OptionOffset + 1 ;
	if ( menu_position_to_remember > LastOptionVisible ) menu_position_to_remember = LastOptionVisible ;

	}
    RestoreMenuBackground ( 0 );
    our_SDL_flip_wrapper();
out:
    game_status = old_game_status;
    return ret;
}; // int ChatDoMenuSelection( char* InitialText , char* MenuTexts[] , asdfasd .... )

/**
 * This function prepares the screen for the big Escape menu and 
 * its submenus.  This means usual content of the screen, i.e. the 
 * combat screen and top status bar, is "faded out", the rest of 
 * the screen is cleared.  This function resolves some redundance 
 * that occured since there are so many submenus needing this.
 */
void 
InitiateMenu( int background_code )
{
    //--------------------
    // Here comes the standard initializer for all the menus and submenus
    // of the big escape menu.  This prepares the screen, so that we can
    // write on it further down.
    //
    SDL_SetClipRect( Screen, NULL );
    
    if ( background_code == ( -1 ) )
    {
	DisplayBanner ( ) ;
	AssembleCombatPicture ( USE_OWN_MOUSE_CURSOR );
	MakeGridOnScreen( NULL );
    }
    else
    {
	// DisplayImage ( find_file ( BackgroundToUse , GRAPHICS_DIR, FALSE ) );
	blit_special_background ( background_code ) ;
    }
    
    SDL_SetClipRect( Screen, NULL );
}; // void InitiateMenu(void)

/**
 * This function provides a convenient cheat menu, so that any 
 * tester does not have to play all through the game again and again
 * to see if a bug in a certain position has been removed or not.
 */
void
Cheatmenu (void)
{
    char *input;		// string input from user 
    int can_continue;
    int LNum, X, Y;
    int i, l;
    int x0, y0, line;
    int skip_dead = 0;
    Waypoint WpList;      // pointer on current waypoint-list  

    //--------------------
    // Prevent distortion of framerate by the delay coming from 
    // the time spend in the menu.
    //
    Activate_Conservative_Frame_Computation();

    //--------------------
    // Some small font is needed, such that we can get a lot of lines on
    // one single cheat menu page...
    //
    SetCurrentFont ( FPS_Display_BFont ); 
				
    x0 = 50;
    y0 = 20;
    line = 0;

    can_continue = FALSE;
    while (!can_continue)
    {
	ClearGraphMem ();
	printf_SDL (Screen, x0, y0, "Current position: Level=%d, X=%d, Y=%d\n",
		    Me . pos . z, (int)Me.pos.x, (int)Me.pos.y);
	printf_SDL (Screen, -1, -1, " f. xray_vision_for_tux: %s",
		    GameConfig . xray_vision_for_tux ? "ON\n" : "OFF\n");
	printf_SDL (Screen, -1, -1, " g. god mode: %s\n", Me . god_mode? "ON" : "OFF");
	printf_SDL (Screen, -1, -1, " i. Make tux invisible\n");
	printf_SDL (Screen, -1, -1, " l. robot list of current level\n");
	printf_SDL (Screen, -1, -1, " L. alive robot list of current level\n");
	printf_SDL (Screen, -1, -1, " k. dead robot list of current level\n");
	printf_SDL (Screen, -1, -1, " d. destroy robots on current level\n");
	printf_SDL (Screen, -1, -1, " t. Teleportation\n");
	printf_SDL (Screen, -1, -1, " h. Auto-aquire all skills\n" );
	printf_SDL (Screen, -1, -1, " n. No hidden droids: %s",
		    show_all_droids ? "ON\n" : "OFF\n" ); 
	printf_SDL (Screen, -1, -1, " x. Cheatkeys : %s",
		    GameConfig . enable_cheatkeys ? "ON\n" : "OFF\n");
	printf_SDL (Screen, -1, -1, " w. Print current waypoints\n");
	printf_SDL (Screen, -1, -1, " q. RESUME game\n");

	//--------------------
	// Now we show it...
	//
	our_SDL_flip_wrapper();

	switch ( getchar_raw (NULL) )
	{
	    case 'f':
		GameConfig . xray_vision_for_tux = ! GameConfig . xray_vision_for_tux;
		break;
	    
	    case 'g':
		Me . god_mode = ! Me . god_mode;
		break;

	    case 'i':
		Me . invisible_duration += 50000;
		break;

	    case 'k':
		skip_dead = 2;
	    case 'L':
		if ( skip_dead == 0 )
			skip_dead = 1;		
	    case 'l': // robot list of this deck 
		l = 0;  // l is counter for lines of display of enemy output
		for ( i = ((skip_dead == 1)? 1 : 0); i < ((skip_dead == 2) ? 1 : 2) ; i ++ )
		    {
		    enemy * erot;
		    list_for_each_entry(erot, (i) ? &alive_bots_head : &dead_bots_head, global_list)
			{
			if ( erot->pos . z == Me . pos . z ) 
			    {

			    if (l && !(l%((GameConfig.screen_height == 768) ? 25 : 16))) 
				{
				printf_SDL (Screen, -1, -1, " --- MORE --- \n");
				our_SDL_flip_wrapper();
				if( getchar_raw (NULL) == 'q')
				    break;
				}
			    if (!(l % ((GameConfig.screen_height == 768) ? 25 : 16)) )  
				{
				ClearGraphMem ();
				printf_SDL (Screen, 15, y0,
					"ID    X     Y    ENERGY   speedX  Status  Friendly An-type An-Phase \n");
				printf_SDL (Screen, -1, -1,
					"---------------------------------------------\n");
				}

			    l ++;
			    if ( ( erot-> type >= 0 ) &&
				    ( erot-> type <= Number_Of_Droid_Types ) )
				{
				printf_SDL (Screen, 15, -1,
					"%s  %3.1f  %3.1f  %4d  %g ",
					Druidmap[erot->type].druidname,
					erot->pos.x,
					erot->pos.y,
					(int)erot->energy,
					erot->speed.x );
				}
			    else
				{
				printf_SDL ( Screen , 15 , -1 , "SEVERE ERROR: Type=%d. " ,
					erot->type ) ;
				}
			    if ( erot->is_friendly ) printf_SDL (Screen, -1, -1, " YES" );
			    else printf_SDL (Screen, -1, -1, "  NO" );
			    switch ( erot-> animation_type ) 
				{
				case WALK_ANIMATION:
				    printf_SDL (Screen, -1, -1, " Walk" );
				    break;
				case ATTACK_ANIMATION:
				    printf_SDL (Screen, -1, -1, " Atta" );
				    break;
				case GETHIT_ANIMATION:
				    printf_SDL (Screen, -1, -1, " GHit" );
				    break;
				case DEATH_ANIMATION:
				    printf_SDL (Screen, -1, -1, " Deth" );
				    break;
				case STAND_ANIMATION:
				    printf_SDL (Screen, -1, -1, " Stnd" );
				    break;
				default:
				    printf_SDL (Screen, -1, -1, " ERROR!" );
				    break;
				}
			    printf_SDL (Screen, -1, -1, " %4.1f" , erot->animation_phase );
			    printf_SDL (Screen, -1, -1, "\n" );

			    } // if (enemy on current level)
			}}

		printf_SDL (Screen, 15, -1," --- END --- \n");
		CountNumberOfDroidsOnShip ( );
		printf_SDL (Screen, 15, -1," BTW:  Number_Of_Droids_On_Ship: %d \n" , Number_Of_Droids_On_Ship );
		our_SDL_flip_wrapper();
		while ( ( !SpacePressed()) && (!EscapePressed()) && (!MouseLeftPressed()) );
		while ( SpacePressed() || EscapePressed() || MouseLeftPressed());
		break;

	    case 'd': // destroy all robots on this level, very useful
		    {
		    enemy * erot, *nerot;
		    BROWSE_ALIVE_BOTS_SAFE(erot, nerot)
			{
			if ( erot->pos.z == Me . pos . z )
			    hit_enemy(erot, erot->energy + 1, 0, -1, 0);
			}

		    printf_SDL (Screen, -1, -1, "All robots on this deck killed!\n");
		    our_SDL_flip_wrapper();
		    getchar_raw (NULL);
		    }
		break;



	    case 't': // Teleportation 
		ClearGraphMem ();
		input = GetString ( 40 , NE_TITLE_PIC_BACKGROUND_CODE , "\nEnter Level, X, Y\n(and please don't forget the commas...)\n> " );
		if ( input == NULL ) break ; // We take into account the possibility of escape being pressed...
		sscanf (input, "%d, %d, %d\n", &LNum, &X, &Y);
		free (input);
		Teleport ( LNum , X , Y , TRUE ) ;
		break;

	    case 'h': // auto-aquire all skills
		for ( i = 0 ; i < number_of_skills ; i ++ ) Me . base_skill_level [ i ]  ++;
		break;

	    case 'n': // toggle display of all droids
		show_all_droids = !show_all_droids;
		break;


	    case 'x':
		GameConfig . enable_cheatkeys = ! GameConfig . enable_cheatkeys;
		break;

	    case 'w':  /* print waypoint info of current level */
		WpList = CURLEVEL->AllWaypoints;
		for (i=0; i<MAXWAYPOINTS && WpList[i].x; i++)
		{
		    if (i && !(i%20))
		    {
			printf_SDL (Screen, -1, -1, " ---- MORE -----\n");
			if (getchar_raw (NULL) == 'q')
			    break;
		    }
		    if ( !(i%20) )
		    {
			ClearGraphMem ();
			printf_SDL (Screen, x0, y0, "Nr.   X   Y      C1  C2  C3  C4\n");
			printf_SDL (Screen, -1, -1, "------------------------------------\n");
		    }		    printf_SDL (Screen, -1, -1, "%2d   %2d  %2d      %2d  %2d  %2d  %2d\n",
				i, WpList[i].x, WpList[i].y,
				WpList[i].connections[0],
				WpList[i].connections[1],
				WpList[i].connections[2],
				WpList[i].connections[3]);
		} /* for (all waypoints) */
		printf_SDL (Screen, -1, -1, " --- END ---\n");
		getchar_raw (NULL);
		break;

	    case ' ':
	    case 'q':
		while ( QPressed () ) SDL_Delay(1);
		can_continue = 1;
		break;
	} /* switch (getchar_raw()) */
    } /* while (!can_continue) */

    ClearGraphMem ();
    our_SDL_flip_wrapper();

    return;
}; // void Cheatmenu() 
	
 
/***********************
 * Menus : Most of the menus are split into two parts
 * - handle, that handles the user selection
 * - fill, that creates the names of the button.
 *
 * Because there is much code that must be generated for each "menu"
 * like prototypes, enums, I use this trick to do the job, and this way
 * I'm sure there is no mismatch in the orders
 */

#define MENU_LIST MENU(Startup, STARTUP) MENU(Escape, ESCAPE)		\
    MENU(Options, OPTIONS) MENU(Resolution, RESOLUTION)			\
    MENU(Graphics, GRAPHICS) MENU(Sound, SOUND)				\
    MENU(Performance, PERFORMANCE) MENU(Language, LANGUAGE)		\
    MENU(OSD, OSD) MENU(Droid, DROID)



enum {
	EXIT_MENU = -2,
	CONTINUE_MENU = -1,
#define MENU(x, y) MENU_##y, 
	MENU_LIST
#undef MENU
	MENU_NUM
};

#define MENU(x, y) static int x##_handle (int); static void x##_fill (char *[10]);
MENU_LIST
#undef MENU

struct Menu {
    int (*HandleSelection) (int);
    void (*FillText) (char *[10]);
};

struct Menu menus [] = { 
#define MENU(x, y) { x##_handle, x##_fill },
    MENU_LIST
#undef MENU
    { NULL, NULL }
};


void
RunSubMenu (int startup, char buffer[10][1024], int menu_id)
{
    int can_continue = 0;
    char *texts[10];
    int i = 0;
    while ( ! can_continue )
    {
	int pos;
	// We need to fill at each loop because
	// several menus change their contents
	for (i = 0; i < 9; i++) texts[i] = buffer[i];
	menus [ menu_id ] . FillText ( texts );
	
	if (startup)
	    pos = DoMenuSelection ( "" , texts , -1, NE_TITLE_PIC_BACKGROUND_CODE , Menu_BFont);
	else
	    pos = DoMenuSelection ( "", texts , 1 , -1 , Menu_BFont);
	
	int ret = menus [ menu_id ] . HandleSelection (pos);
	
	if (ret == EXIT_MENU) 
	{
	    can_continue = TRUE;
	}
	
	if (ret > 0)
	    RunSubMenu (startup, buffer, ret);
    }
}
void 
RunMenu (int is_startup)
{
	// 1024 should be enough
	char buffer [10][1024]; 
	int start_menu = is_startup ? MENU_STARTUP : MENU_ESCAPE;
	SetCurrentFont ( Menu_BFont );
	Activate_Conservative_Frame_Computation ( ) ;	
	if (is_startup) {
		// Can the music be disabled by a submenu ?
		SwitchBackgroundMusicTo( MENU_BACKGROUND_MUSIC_SOUND );		
		SDL_SetClipRect ( Screen , NULL );
		
		if ( skip_initial_menus && Single_Player_Menu ( ))
			return ;
	} else {
	    while ( EscapePressed() );
	}
	RunSubMenu (is_startup, buffer, start_menu);
	Me.status=MOBILE;
	ClearGraphMem();
}

void StartupMenu (void)
{
    RunMenu (1);
}

void
EscapeMenu (void)
{
    RunMenu (0);
}
static int
Startup_handle (int n)
{
    enum
    { 
	SINGLE_PLAYER_POSITION=1, 
	LVLEDIT_POSITION,
	OPTIONS_POSITION,
	CREDITS_POSITION,
	CONTRIBUTE_POSITION,
	EXIT_FREEDROID_POSITION
    };
    switch (n) 
    {
    case SINGLE_PLAYER_POSITION:
	    if (  Single_Player_Menu ( ) )
		return EXIT_MENU;
	    break;
    case LVLEDIT_POSITION: //allow starting directly in leveleditor - the hack is a little dirty but it does its work.
	skip_initial_menus = 1;
	clear_player_inventory_and_stats ( ) ;
	UpdateAllCharacterStats ( ) ;
	strcpy(Me.character_name, "MapEd");
	char fp[2048];
	find_file ( "freedroid.levels" , MAP_DIR, fp, 0);
	LoadShip ( fp ) ;
	PrepareStartOfNewCharacter ( ) ;
	skip_initial_menus = 0;
	LevelEditor () ;
	return EXIT_MENU;
	break;  
    case OPTIONS_POSITION:
	return MENU_OPTIONS;
    case CREDITS_POSITION:
	PlayATitleFile ( "Credits.title" );
	break;
    case CONTRIBUTE_POSITION:
	PlayATitleFile ( "Contribute.title" );
	break;
    case (-1):
    case EXIT_FREEDROID_POSITION:
	Terminate( OK );
	break;
    default: 
	break;
    } 
    return CONTINUE_MENU;
}

static void
Startup_fill (char *MenuTexts[10])
{
    MenuTexts[0]=_("Play");
    MenuTexts[1]=_("Level Editor");
    MenuTexts[2]=_("Options");
    MenuTexts[3]=_("Credits");
    MenuTexts[4]=_("Contribute");
    MenuTexts[5]=_("Exit FreedroidRPG");
    MenuTexts[6]="";
}


static int
Options_handle (int n)
{
    enum
	{ 
	    GRAPHICS_OPTIONS=1, 
	    SOUND_OPTIONS,
	    LANGUAGE_OPTIONS,
	    DROID_TALK_OPTIONS,
	    ON_SCREEN_DISPLAYS,
	    PERFORMANCE_TWEAKS_OPTIONS,
	    LEAVE_OPTIONS_MENU 
	};
    switch (n) 
    {
    case (-1):
	    return EXIT_MENU;
	    break;
    case GRAPHICS_OPTIONS:
	    return MENU_GRAPHICS;
    case SOUND_OPTIONS:
	    return MENU_SOUND;
    case LANGUAGE_OPTIONS:
#if ENABLE_NLS
#if defined(__WIN32__) || defined(__MACOSX__)
	    GiveMouseAlertWindow( _("\nYou cannot change the language\n"
				    "under Windows or Mac OS X.\n\n"
				    "FreedroidRPG uses your system's language\n"
				    "by default for your convenience.\n\n"
				    "    Thank you\n")); 
#else						
		return MENU_LANGUAGE;
#endif
#else
	    GiveMouseAlertWindow ( _("\nThe game has NOT been\n"
			"compiled with gettext support\n"
			"therefore translations are NOT\n"
			"available.\n"
			"Thanks\n"));
#endif
	    break;
    case DROID_TALK_OPTIONS:
	    return MENU_DROID;
    case ON_SCREEN_DISPLAYS:
	    return MENU_OSD;
    case PERFORMANCE_TWEAKS_OPTIONS:
	    return MENU_PERFORMANCE;
    case LEAVE_OPTIONS_MENU:
	    return EXIT_MENU;
    default: 
	    break;
    }
    return CONTINUE_MENU;
}

static void
Options_fill (char *MenuTexts [10])
{
	MenuTexts[0]=_("Graphics Options");
	MenuTexts[1]=_("Sound Options");
	MenuTexts[2]=_("Language");
	MenuTexts[3]=_("Droid Talk");
	MenuTexts[4]=_("On-Screen Displays");
	MenuTexts[5]=_("Performance Tweaks");
	MenuTexts[6]=_("Back");
	MenuTexts[7]="";
}

static int
Escape_handle (int n)
{
    enum
    { 
	SAVE_GAME_POSITION=1,
	RESUME_GAME_POSITION,
	OPTIONS_POSITION, 
	LOAD_GAME_POSITION,
	LOAD_BACKUP_POSITION,
	NEW_GAME_POSITION,
	QUIT_POSITION
    };
    switch (n)
    {
    case ( -1 ) :
    case ( RESUME_GAME_POSITION ) :
	return EXIT_MENU;
    case OPTIONS_POSITION:
	return MENU_OPTIONS;
    case LOAD_GAME_POSITION:
	  LoadGame ( ) ;
	  return EXIT_MENU;
    case LOAD_BACKUP_POSITION:
	  LoadBackupGame();
	  return EXIT_MENU;
    case NEW_GAME_POSITION:
	GameOver = TRUE ;
	return EXIT_MENU;
    case SAVE_GAME_POSITION:
	SaveGame(  );
	break;
    case QUIT_POSITION:
	  DebugPrintf (2, "\nvoid EscapeMenu( void ): Quit Requested by user.  Terminating...");
	  Terminate(0);
	  break;
	default: 
	  break;
    } 

    return CONTINUE_MENU;
}

static void
Escape_fill (char *MenuTexts [10])
{
      MenuTexts[0]=_("Save Game");
      MenuTexts[1]=_("Resume Game");
      MenuTexts[2]=_("Options");
      MenuTexts[3]=_("Load Game");
      MenuTexts[4]=_("Load Backup");
      MenuTexts[5]=_("Quit Game");
      MenuTexts[6]=_("Exit FreedroidRPG");
      MenuTexts[7]="";
}

static int
Resolution_handle (int n)
{
    enum
	{ 
	    SET_640_480 = 1 , 
	    SET_800_600, 
	    SET_1024_768,
	    SET_1152_864,
	    SET_1280_960,
/*
	    SET_1280_1024,
	    SET_1280_800,
	    SET_1440_900,
	    SET_1680_1050,
*/
	    LEAVE_OPTIONS_MENU 
	};
#define MSG(x, y) "\n"				\
	"You selected "#x "x" #y"pixel.\n\n"	\
	"Change of screen resolution will\n"	\
	"take effect automatically when you next\n"	\
	"start FreedroidRPG.\n"				\
	"\n"						\
	"Thank you.\n"					\
	
#define CASE(x, y) case SET_##x##_##y:		\
    while (EnterPressed ( ) || SpacePressed ( )); \
    GameConfig . next_time_width_of_screen = x ;	\
    GameConfig . next_time_height_of_screen = y ;	\
    GiveMouseAlertWindow (_(MSG(x,y)));			\
    break;

    switch (n) 
    {
    case (-1):
	return EXIT_MENU;
	
    CASE(640,480);
    CASE(800,600);
    CASE(1024,768);
    CASE(1152,864);
    CASE(1280,960);
/*
    CASE(1280,1024);
    CASE(1280,800);
    CASE(1440,900);
    CASE(1680,1050);
*/
    case LEAVE_OPTIONS_MENU:
	while (EnterPressed() || SpacePressed() );
	return EXIT_MENU;
    default: 
	break;
    } 
    return CONTINUE_MENU;
}

static void
Resolution_fill (char *MenuTexts[10])
{
    sprintf( MenuTexts[0] , "%c 640x480", GameConfig . next_time_height_of_screen==480 ? '*' : ' ' );
    sprintf( MenuTexts[1] , "%c 800x600", GameConfig . next_time_height_of_screen==600 ? '*' : ' ' );
    sprintf( MenuTexts[2] , "%c 1024x768", GameConfig . next_time_height_of_screen==768 ? '*' : ' ' );
    sprintf( MenuTexts[3] , "%c 1152x864", GameConfig . next_time_height_of_screen==864 ? '*' : ' ' );
    sprintf( MenuTexts[4] , "%c 1280x960", GameConfig . next_time_height_of_screen==960 ? '*' : ' ' );
    MenuTexts[5]=_("Back");
    MenuTexts[6]="";
/*
    sprintf( MenuTexts[4] , "%c 1280x1024", GameConfig . next_time_height_of_screen==1024 ? '*' : ' ' );
    sprintf( MenuTexts[5] , "%c 1280x800", GameConfig . next_time_height_of_screen==800 ? '*' : ' ' );
    sprintf( MenuTexts[6] , "%c 1440x900", GameConfig . next_time_height_of_screen==900 ? '*' : ' ' );
    sprintf( MenuTexts[7] , "%c 1680x1050", GameConfig . next_time_height_of_screen==1050 ? '*' : ' ' );
    MenuTexts[8]=_("Back");
    MenuTexts[9]="";
*/
}

static int
Graphics_handle (int n)
{
    enum
    { 
	SET_GAMMA_CORRECTION = 1 , 
	SET_FULLSCREEN_FLAG, 
	CHANGE_SCREEN_RESOLUTION,
	SET_SHOW_BLOOD_FLAG,
	SET_AUTOMAP_SCALE,
	LEAVE_OPTIONS_MENU 
    };
    switch (n) 
    {
    case (-1):
	return EXIT_MENU;
    case SET_GAMMA_CORRECTION:
	if ( RightPressed() ) 
	{
	    while ( RightPressed());
	    GameConfig.current_gamma_correction+=0.05;
	    SDL_SetGamma( GameConfig.current_gamma_correction , GameConfig.current_gamma_correction , GameConfig.current_gamma_correction );
	}
	
	if ( LeftPressed() ) 
	{
	    while (LeftPressed());
	    GameConfig.current_gamma_correction-=0.05;
	    SDL_SetGamma( GameConfig.current_gamma_correction , GameConfig.current_gamma_correction , GameConfig.current_gamma_correction );
	}
	
	break;
	
    case SET_FULLSCREEN_FLAG:
	while ( EnterPressed( ) || SpacePressed( ) );
	GameConfig . fullscreen_on = ! GameConfig . fullscreen_on;
#ifndef __WIN32__
	SDL_WM_ToggleFullScreen ( Screen );
#else
	GiveMouseAlertWindow(_("\nThis option will be taken into account after you restart the game.\n\n   Thank you.\n"));
#endif
		break;
		
    case CHANGE_SCREEN_RESOLUTION:
	while ( EnterPressed() || SpacePressed() );
	return MENU_RESOLUTION;
	
    case SET_SHOW_BLOOD_FLAG:
	while (EnterPressed() || SpacePressed() );
	GameConfig . show_blood = !GameConfig . show_blood;
	break;
	
    case SET_AUTOMAP_SCALE:
	if ( RightPressed() ) 
	{
	    while ( RightPressed() );
	    if ( GameConfig . automap_display_scale < 9.1 )
		GameConfig . automap_display_scale += 1.0 ;
	}
	
	if ( LeftPressed() ) 
	{
	    while (LeftPressed());
	    if ( GameConfig . automap_display_scale >= 1.9 )
		GameConfig . automap_display_scale -= 1.0 ;
	}
	
	break;
	
    case LEAVE_OPTIONS_MENU:
	while (EnterPressed() || SpacePressed() );
	return EXIT_MENU;
	break;
	
    default: 
	break;
	
    } 
    return CONTINUE_MENU;
}    
static void
Graphics_fill (char *MenuTexts[10])
{
	sprintf( MenuTexts[0] , _("Gamma Correction: %1.2f"), GameConfig.current_gamma_correction );
	sprintf( MenuTexts[1] , _("Fullscreen Mode: %s"), GameConfig . fullscreen_on ? _("ON") : _("OFF"));
	sprintf( MenuTexts[2] , _("Change Screen Resolution") );
	sprintf( MenuTexts[3] , _("Show Blood: %s"), 
		 GameConfig . show_blood ? _("YES") : _("NO") );
	sprintf( MenuTexts[4] , _("Automap Scale: %2.1f"), 
		 GameConfig . automap_display_scale );
	MenuTexts[5]=_("Back");
	MenuTexts[6]="";
}


static int
Sound_handle (int n)
{
    enum
	{ 
	    SET_BG_MUSIC_VOLUME=1, 
	    SET_SOUND_FX_VOLUME, 
	    LEAVE_OPTIONS_MENU 
	};    
    switch (n) 
    {
    case (-1):
	return EXIT_MENU;
    case SET_BG_MUSIC_VOLUME:
	if ( RightPressed() ) 
	{
	    while ( RightPressed());
	    if ( GameConfig.Current_BG_Music_Volume < 1 ) GameConfig.Current_BG_Music_Volume += 0.05;
	    SetBGMusicVolume( GameConfig.Current_BG_Music_Volume );
	}
	
	
	if ( LeftPressed() ) 
	{
	    while (LeftPressed());
	    if ( GameConfig.Current_BG_Music_Volume > 0 ) GameConfig.Current_BG_Music_Volume -= 0.05;
	    SetBGMusicVolume( GameConfig.Current_BG_Music_Volume );
	}
	
	break;
	
    case SET_SOUND_FX_VOLUME:
	if ( RightPressed() ) 
	{
	    while ( RightPressed());
	    if ( GameConfig.Current_Sound_FX_Volume < 1 ) GameConfig.Current_Sound_FX_Volume += 0.05;
	    SetSoundFXVolume( GameConfig.Current_Sound_FX_Volume );
	}
	
	if ( LeftPressed() ) 
	{
	    while (LeftPressed());
	    if ( GameConfig.Current_Sound_FX_Volume > 0 ) GameConfig.Current_Sound_FX_Volume -= 0.05;
	    SetSoundFXVolume( GameConfig.Current_Sound_FX_Volume );
	}
	break;

    case LEAVE_OPTIONS_MENU:
	while (EnterPressed() || SpacePressed() );
	return EXIT_MENU;
	break;
	
    default: 
	break;
    }
    return CONTINUE_MENU;
}

static void
Sound_fill (char *MenuTexts[10])
{
	sprintf ( MenuTexts[0] , _("Background Music Volume: %1.2f") , GameConfig.Current_BG_Music_Volume );
	sprintf ( MenuTexts[1] , _("Sound Effects Volume: %1.2f"), GameConfig.Current_Sound_FX_Volume );
	MenuTexts [ 2 ] = _("Back");
	MenuTexts [ 3 ] = "";
}


static int
Performance_handle (int n)
{
    enum
	{ 
	    SET_HOG_CPU_FLAG = 1,
	    SET_HIGHLIGHTING_MODE,
	    SKIP_LIGHT_RADIUS_MODE,
	    SKIP_SHADOWS,
	    SKIP_FADINGS,
	    LEAVE_PERFORMANCE_TWEAKS_MENU 
	};
    switch (n)
    {
    case (-1):
	return EXIT_MENU;
	
    case SET_HOG_CPU_FLAG:
	while (EnterPressed() || SpacePressed() || MouseLeftPressed());
	GameConfig . hog_CPU = ! GameConfig . hog_CPU ;
	break;
		
    case SET_HIGHLIGHTING_MODE:
	while (EnterPressed() || SpacePressed() || MouseLeftPressed());
	GameConfig . highlighting_mode_full = ! GameConfig . highlighting_mode_full ;
	break;
	
    case SKIP_LIGHT_RADIUS_MODE:
	while (EnterPressed() || SpacePressed()|| MouseLeftPressed() );
	GameConfig . skip_light_radius = ! GameConfig . skip_light_radius ;
	break;
	
    case SKIP_SHADOWS:
	while (EnterPressed() || SpacePressed()|| MouseLeftPressed() );
	GameConfig . skip_shadow_blitting = ! GameConfig . skip_shadow_blitting ;
	break;
	
    case SKIP_FADINGS:
	while (EnterPressed() || SpacePressed() || MouseLeftPressed() );
	GameConfig . do_fadings = ! GameConfig . do_fadings ;
	break;
	
    case LEAVE_PERFORMANCE_TWEAKS_MENU:
	while (EnterPressed() || SpacePressed()|| MouseLeftPressed() );
	return EXIT_MENU;
	break;
	
    default: 
	break;
	
    } 
    return CONTINUE_MENU;
}
    

static void
Performance_fill (char *MenuTexts[])
{
	sprintf ( MenuTexts[0] , _("Hog CPU for max. performance: %s"), 
		  GameConfig.hog_CPU ? _("YES") : _("NO") );
	sprintf ( MenuTexts[1] , _("Highlighting mode: %s"), GameConfig.highlighting_mode_full ? _("FULL") : _("REDUCED") );
	sprintf ( MenuTexts[2] , _("Skip light radius: %s"), GameConfig . skip_light_radius ? _("YES") : _("NO") );
	sprintf ( MenuTexts[3] , _("Skip shadow blitting: %s"), 
		  GameConfig . skip_shadow_blitting ? _("YES") : _("NO") );
	sprintf( MenuTexts[4] , _("Skip fadings: %s"), 
		 GameConfig . do_fadings ? _("NO") : _("YES") );
	MenuTexts[5]=_("Back");
	MenuTexts[6]="";
}

static int
Language_handle (int n)
{
    int nb_languages = 0;
    while ( supported_languages[nb_languages].code != NULL )
	nb_languages ++;
    if ( n <= nb_languages && n > 0) {
	GameConfig . language = n - 1;
#if ENABLE_NLS
	setlocale(LC_MESSAGES, supported_languages[n - 1] . code);
	setlocale(LC_CTYPE, supported_languages[n - 1] . code);
#endif
	FreeOurBFonts();
	InitOurBFonts();
    }
    return EXIT_MENU;
}

static void
Language_fill (char *MenuTexts[10])
{
    // If there are more than 8 languages supported,
    // it may crash.
    int i = 0;
    while ( supported_languages[i].code != NULL )
	{
	MenuTexts[i] = supported_languages[i].name;
	i ++;
	}
    MenuTexts[i] = "";
}


static int
OSD_handle (int n)
{
  enum
    { 
      SHOW_POSITION=1, 
      SHOW_FRAMERATE, 
      SHOW_ENEMY_ENERGY_BARS,
      PARALLEL_BIG_SCREEN_MESSAGES_AT_MOST_POSITION,
      BIG_SCREEN_MESSAGES_DURATION_POSITION,
      LEAVE_OPTIONS_MENU 
    };

  int *shows[4] = {
	  &GameConfig.Draw_Position, &GameConfig.Draw_Framerate,
	  &GameConfig.enemy_energy_bars_visible
  };

  if (n >= 1 && n < PARALLEL_BIG_SCREEN_MESSAGES_AT_MOST_POSITION) {
	  while (EnterPressed () || SpacePressed ());
	  *shows[n-1] = !*shows[n-1];
  } else {
      switch (n) {
      case -1:
	  return EXIT_MENU;
      case PARALLEL_BIG_SCREEN_MESSAGES_AT_MOST_POSITION:
	  if ( LeftPressed() )
	  {
	      while ( LeftPressed() );
	      if ( GameConfig . number_of_big_screen_messages > 0 )
	      {
		  GameConfig . number_of_big_screen_messages -- ;
	      }
	  }
	  else if ( RightPressed() )
	  {
	      while ( RightPressed() );
	      if ( GameConfig . number_of_big_screen_messages < MAX_BIG_SCREEN_MESSAGES )
	      {
		  GameConfig . number_of_big_screen_messages ++ ;
	      }
	  }
	  break;
	  
      case BIG_SCREEN_MESSAGES_DURATION_POSITION:
	  if ( LeftPressed() )
	  {
	      while ( LeftPressed() );
	      if ( GameConfig . delay_for_big_screen_messages >= 0.5 )
	      {
		  GameConfig . delay_for_big_screen_messages -= 0.5 ;
	      }
	  }
	  else if ( RightPressed() )
	  {
	      while ( RightPressed() );
	      GameConfig . delay_for_big_screen_messages += 0.5 ;
	  }
	  break;
	  
      case LEAVE_OPTIONS_MENU:
	  while (EnterPressed() || SpacePressed() );
	  return EXIT_MENU;
      default: 
	  break;
      } 
  }
  return CONTINUE_MENU;
}

static void
OSD_fill (char *MenuTexts[10])
{
      sprintf( MenuTexts[0] , _("Show Position: %s"), GameConfig.Draw_Position ? _("ON") : _("OFF") );
      sprintf( MenuTexts[1] , _("Show Framerate: %s"), GameConfig.Draw_Framerate? _("ON") : _("OFF") );
      sprintf( MenuTexts[2] , _("Show Enemy Energy Bars: %s"), GameConfig.enemy_energy_bars_visible? _("ON") : _("OFF") );
      sprintf( MenuTexts[3] , _("Screen Messages at most: %d"), GameConfig.number_of_big_screen_messages );
      sprintf( MenuTexts[4] , _("Screen Message time: %3.1f"), GameConfig.delay_for_big_screen_messages );
      strcpy (MenuTexts[5], _("Back"));
      MenuTexts[6][0]='\0';

}

static int
Droid_handle (int n)
{
  enum
    { 
      ENEMY_HIT_TEXT=1,
      ENEMY_BUMP_TEXT,
      ENEMY_AIM_TEXT,
      ALL_TEXTS,
      TALK_AFTER_TO,
      LEAVE_DROID_TALK_OPTIONS_MENU 
    };
  int *ptrs [] = {
      &GameConfig.Enemy_Hit_Text, &GameConfig.Enemy_Bump_Text,
      &GameConfig.Enemy_Aim_Text, &GameConfig.All_Texts_Switch,
      &GameConfig.talk_to_bots_after_takeover
  };

  if (n < LEAVE_DROID_TALK_OPTIONS_MENU && n > 0)
      *ptrs[n-1] = ! *ptrs[n-1];
  else {
      while (EnterPressed () || SpacePressed () || MouseLeftPressed ());
      return EXIT_MENU;
  }
  return CONTINUE_MENU;
}
      

static void
Droid_fill (char *MenuTexts[10])
{
      sprintf( MenuTexts[0] , _("Enemy Hit Texts: %s"), GameConfig.Enemy_Hit_Text ? _("ON") : _("OFF") );
      sprintf( MenuTexts[1] , _("Enemy Bumped Texts: %s"), GameConfig.Enemy_Bump_Text ? _("ON") : _("OFF") );
      sprintf( MenuTexts[2] , _("Enemy Aim Texts: %s"), GameConfig.Enemy_Aim_Text ? _("ON") : _("OFF") );
      sprintf( MenuTexts[3] , _("All in-game Speech: %s"), GameConfig.All_Texts_Switch ? _("ON") : _("OFF") );
      sprintf( MenuTexts[4], _("Reprogram bots after takeover: %s"), GameConfig.talk_to_bots_after_takeover ? _("ON") : _("OFF"));
      MenuTexts[5] = _("Back");
      MenuTexts[6] = "";
}

    
#define FIRST_MIS_SELECT_ITEM_POS_X (0.0)
#define FIRST_MIS_SELECT_ITEM_POS_Y (BANNER_HEIGHT + FontHeight(Menu_BFont))

/**
 * This reads in the new name for the character...
 */
void
Get_New_Character_Name ( void )
{
    char* Temp;
    InitiateMenu( NE_TITLE_PIC_BACKGROUND_CODE );

    if ( ! skip_initial_menus )
	Temp = GetString ( 12 , NE_TITLE_PIC_BACKGROUND_CODE , _("\n\
     Please enter a name\n\
     for the new hero: \n\
     (ESCAPE to cancel.)\n\n\
     > ") );
    else
	Temp = "MapEd" ;

    //--------------------
    // In case 'Escape has been pressed inside GetString, then a NULL pointer
    // will be returned, which means no name has been given to the character
    // yet, so we set an empty string for the character name.
    //
    if ( Temp == NULL )
    {
	//--------------------
	// WARNING:  We should not use "" here, since that is also used inside
	//           the menu code as the termination string for the list of menu
	//           options.  That would cause problems in the load and display
	//           directory content later.
	//           Therefore we supply some default name if empty
	//           string was received...  (some more decent workaround for the
	//           problem might be written some time later...)
	//
	strcpy ( Me . character_name , "HaveNoName" );
	return ;
    }
    
    //--------------------
    // If a real name has been given to the character, we can copy that name into
    // the corresponding structure and return here (not without freeing the string
    // received.  Could be some valuable 20 bytes after all :)
    //
    // Parse Temp for illegal chars
    unsigned int i;
    for ( i = 0 ; i < strlen( Temp ) ; i ++)
	if( ! isalnum ( Temp[i] ) && Temp [ i ] != '-')
		Temp [ i ] = '-';
    strcpy ( Me . character_name , Temp );
    if(!skip_initial_menus && Temp!=NULL)
    free( Temp );
    
}; // void Get_New_Character_Name ( void )

/**
 *
 *
 */
void
clear_player_inventory_and_stats ( void )
{
    int i;

    //--------------------
    // At first we clear the inventory of the new character
    // of any items (or numeric waste) that might be in there
    //
    for ( i = 0 ; i < MAX_ITEMS_IN_INVENTORY ; i ++ )
    {
	Me . Inventory [ i ] . type = (-1);
	Me . Inventory [ i ] . prefix_code = (-1);
	Me . Inventory [ i ] . suffix_code = (-1);
	Me . Inventory [ i ] . currently_held_in_hand = FALSE;
    }
    DebugPrintf ( 1 , "\n%s(): Inventory has been emptied..." , __FUNCTION__ );

    //--------------------
    // Now we add some safety, against 'none present' items
    //
    Me . weapon_item.type = ( -1 ) ;
    Me . drive_item.type = ( -1 ) ;
    Me . armour_item.type = ( -1 ) ;
    Me . shield_item.type = ( -1 ) ;
    Me . special_item.type = ( -1 ) ;
    
    Me . weapon_item.prefix_code = ( -1 ) ;
    Me . drive_item.prefix_code = ( -1 ) ;
    Me . armour_item.prefix_code = ( -1 ) ;
    Me . shield_item.prefix_code = ( -1 ) ;
    Me . special_item.prefix_code = ( -1 ) ;
    
    Me . weapon_item.suffix_code = ( -1 ) ;
    Me . drive_item.suffix_code = ( -1 ) ;
    Me . armour_item.suffix_code = ( -1 ) ;
    Me . shield_item.suffix_code = ( -1 ) ;
    Me . special_item.suffix_code = ( -1 ) ;
    
    Me . weapon_item.currently_held_in_hand = ( -1 ) ;
    Me . drive_item.currently_held_in_hand = ( -1 ) ;
    Me . armour_item.currently_held_in_hand = ( -1 ) ;
    Me . shield_item.currently_held_in_hand = ( -1 ) ;
    Me . special_item.currently_held_in_hand = ( -1 ) ;
    
    Me . base_vitality = 25 ;
    Me . base_strength = 10 ;
    Me . base_dexterity = 15 ;
    Me . base_magic = 25 ;
    Me . exp_level = 1 ;

}; // void clear_player_inventory_and_stats ( void )

/**
 * This function prepares a new hero for adventure...
 */
int
PrepareNewHero (void)
{

    clear_player_inventory_and_stats ( ) ;

    UpdateAllCharacterStats ( ) ;

    Get_New_Character_Name( );
    
    Me . is_town_guard_member = FALSE ;

    //--------------------
    // If the special string "HaveNoName" is being supplied, then
    // we treat this as no name given and will return false.
    //
    if ( ! strcmp ( Me . character_name , "HaveNoName" ) ) return ( FALSE ) ;

    //--------------------
    // If a real name has been given, then we can proceed and start the
    // game.  If no real name has been given or 'Escape' has been pressed,
    // then the calling function will return to the menu and do nothing
    // else.
    //
    if ( strlen ( Me . character_name ) > 0 ) return ( TRUE ); else return ( FALSE );
    
}; // int PrepareNewHero (void)

/**
 * Filter function for scandir calls 
 * This function keeps files with the ".savegame" extension.
 *
 */
static int filename_filter_func ( const struct dirent *file )
{
	char* pos = strstr ( file->d_name , ".savegame" );
	
	if ( pos != NULL ) { // ".savegame" found
	    if ( strlen(pos) == 9 ) {// since strlen(".savegame") is 9, then
		// d_name *ENDS* with ".savegame"

		if ( strstr(file->d_name, ".bkp.savegame") + 4 == pos) {
		    //then we have .bkp.savegame = filter it out
		    return 0;
		}

		return (1);
	    }
	}

	return ( 0 );
}; // static int filename_filter_func (const struct dirent *unused)

enum {
    SAVEGAME_LOAD,
    SAVEGAME_DELETE,
};

static int do_savegame_selection_and_act(int action) 
{

    char Saved_Games_Dir[1000];
    char* MenuTexts[ 10 ] ;
    struct dirent **eps;
    int n;  
    int cnt;
    int MenuPosition;
    int saveoffset = 0;
    char SafetyText[500];
    char *menu_title = NULL;
    
    switch (action) {
	case SAVEGAME_LOAD:
	    menu_title = LOAD_EXISTING_HERO_STRING;
	    break;
	case SAVEGAME_DELETE:
	    menu_title = DELETE_EXISTING_HERO_STRING;
	    break;
    }

    InitiateMenu( NE_TITLE_PIC_BACKGROUND_CODE );

    //--------------------
    // We use empty strings to denote the end of any menu selection, 
    // therefore also for the end of the list of saved characters.
    //
    for ( n = 0 ; n < MAX_SAVED_CHARACTERS_ON_DISK + 1 ; n ++ )
	{
	MenuTexts [ n ] = "";
	}

    //--------------------
    // First we must find the home directory of the user.  From there on
    // we can then construct the full directory path of the saved games directory.
    //

#if __WIN32__
    our_homedir = ".";
#else
    // first we need the user's homedir for loading/saving stuff
    if ( (our_homedir = getenv("HOME")) == NULL )
	{
	DebugPrintf ( 0 , "ERROR: Environment does not contain HOME variable... \n\
		I need to know that for saving. Abort.\n");
	return ( ERR );
	}
#endif

    //--------------------
    // Now we generate the right directory for loading from the home
    // directory.
    //
    sprintf ( Saved_Games_Dir , "%s/.freedroid_rpg" , our_homedir );

    n = scandir ( Saved_Games_Dir , &eps, filename_filter_func , alphasort);
    if (n > 0) {
	
	for (cnt = 0; cnt < n; cnt++) {
	    *strstr(eps[cnt]->d_name, ".savegame") = 0;
	}

	while (1) {

	    if (saveoffset != 0) {
		/* Display "up" */
		MenuTexts [ 0 ] = _("[up]");
	    } else {
		MenuTexts [ 0 ] = " ";
	    }

	    for (cnt = 1; cnt + saveoffset - 1< n && cnt < MAX_SAVED_CHARACTERS_ON_DISK; cnt++) {
		MenuTexts[ cnt ] = eps[cnt + saveoffset - 1]->d_name; 
	    }

	    if (cnt >= 6) {
		/* Display "down" */
		MenuTexts [ cnt++ ] = _("[down]");
	    } else MenuTexts [ cnt++ ] = " ";	

	    MenuTexts [ cnt ] = _("Back");
	    MenuTexts [ cnt + 1 ] = "";

	    MenuPosition = DoMenuSelection( menu_title , MenuTexts , 1 , NE_TITLE_PIC_BACKGROUND_CODE , NULL );

	    if ( MenuPosition == (-1) || MenuPosition == cnt + 1 ) {
		for (cnt = 0; cnt < n; cnt++) {
		    free(eps[cnt]);
		}
		free(eps);
		return ( FALSE );
	    }
	    if ( MenuPosition == cnt ) {
		if (cnt + saveoffset - 1 < n) saveoffset++;
	    } else if ( MenuPosition == 1 ) {
		if (saveoffset > 0) saveoffset--;
	    } else break;
	}

	strcpy ( Me . character_name , MenuTexts [ MenuPosition -1 ] );

	for (cnt = 0; cnt < n; cnt++) {
	    free(eps[cnt]);
	}
	free(eps);

	goto do_action;
    }
    else
	{

	MenuTexts[0]=_("BACK");
	MenuTexts[1]="";

	DoMenuSelection ( _("\n\nNo saved games found!"), MenuTexts , 1 , NE_TITLE_PIC_BACKGROUND_CODE , NULL );

	return FALSE;
	}

    return FALSE;

do_action:    
    switch (action) {
	case SAVEGAME_LOAD:
	    if ( LoadGame ( ) == OK ) {
		GetEventsAndEventTriggers ( "freedroid.events" );
		GetQuestList ( "freedroid.quests" );
		Item_Held_In_Hand = ( -1 );
	    }
	    break;
	case SAVEGAME_DELETE:
	    
	    // We do a final safety check to ask for confirmation.
	    MenuTexts [ 0 ] = _("Sure!") ;
	    MenuTexts [ 1 ] = _("BACK") ;
	    MenuTexts [ 2 ] = "";
	    sprintf( SafetyText , _("Really delete hero '%s'?") , Me.character_name ) ;
	    int FinalDecision = DoMenuSelection( SafetyText , MenuTexts , 1 , NE_TITLE_PIC_BACKGROUND_CODE , NULL );

	    if ( FinalDecision == 1 )
		DeleteGame( );
	    break;
    }

    our_SDL_flip_wrapper();
    return TRUE;
}

/**
 * Load a savegame
 */
int Load_Existing_Hero_Menu ( void )
{
    return do_savegame_selection_and_act(SAVEGAME_LOAD);
}

/** 
 * Delete a savegame
 */
int Delete_Existing_Hero_Menu ( void )
{
    return do_savegame_selection_and_act(SAVEGAME_DELETE);
}

/**
 * This function provides the single player menu.  It offers to start a
 * new hero, to load an old one and to go back.
 *
 * The return value indicates, whether the calling function (StartupMenu)
 * can really enter the new game after this function (cause a new player
 * has been set up properly) or not, cause no player is specified yet and
 * nothing is known.
 *
 */
int
Single_Player_Menu (void)
{
    int can_continue = 0;
    int MenuPosition=1;
    char* MenuTexts[10];
    
    enum
	{ 
	    NEW_HERO_POSITION=1, 
	    LOAD_EXISTING_HERO_POSITION, 
	    DELETE_EXISTING_HERO_POSITION,
	    BACK_POSITION
	};
    
    MenuTexts[0]=_("New Hero");
    MenuTexts[1]=_("Load existing Hero");
    MenuTexts[2]=_("Delete existing Hero");
    MenuTexts[3]=_("Back");
    MenuTexts[4]="";
    
    while (!can_continue)
    {
	
	if ( ! skip_initial_menus )
	    MenuPosition = DoMenuSelection( "" , MenuTexts , 1 , NE_TITLE_PIC_BACKGROUND_CODE , Menu_BFont );
	else
	    MenuPosition = NEW_HERO_POSITION ;
	
	switch (MenuPosition) 
	{
	    case NEW_HERO_POSITION:
		while ( EnterPressed() || SpacePressed() ) ;
		
		if ( PrepareNewHero ( ) == TRUE )
		{
		    char fp[2048];
		    find_file ( "freedroid.levels" , MAP_DIR, fp, 0);
		    LoadShip ( fp ) ;
		    PrepareStartOfNewCharacter ( ) ;
		    can_continue=TRUE;
		    load_game_command_came_from_inside_running_game = TRUE ;
		    return ( TRUE );
		}
		else
		{
		    can_continue=FALSE;
		}
		break;
		
	    case LOAD_EXISTING_HERO_POSITION: 
		while ( EnterPressed() || SpacePressed() ) ;
		
		if ( Load_Existing_Hero_Menu ( ) == TRUE )
		{
		    can_continue = TRUE;
		    return ( TRUE );
		}
		else
		{
		    can_continue = FALSE;
		}
		break;
		
	    case DELETE_EXISTING_HERO_POSITION: 
		while (EnterPressed() || SpacePressed() ) ;
		Delete_Existing_Hero_Menu ( ) ;
		can_continue= FALSE ;
		break;
		
	    case (-1):
	    case BACK_POSITION:
		while (EnterPressed() || SpacePressed() || EscapePressed() ) ;
		can_continue=!can_continue;
		return ( FALSE );
		break;
	    default: 
		break;
	}
    }
    return ( TRUE );
}; // void Single_Player_Menu ( void );

#undef _menu_c
