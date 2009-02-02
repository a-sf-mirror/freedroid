/* 
 *
 *   Copyright (c) 2002, 2003 Johannes Prix
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

#define _leveleditor_input_c


#include "system.h"

#include "defs.h"
#include "struct.h"
#include "global.h"
#include "proto.h"

#include "SDL_rotozoom.h"

#include "leveleditor.h"
#include "leveleditor_actions.h"
#include "leveleditor_map.h"
#include "leveleditor_widgets.h"

static struct leveleditor_widget * previously_active_widget = NULL;

/**
 * This function shall determine, whether a given left mouse click was in 
 * given rect or not.
 */
static int ClickWasInRect ( SDL_Rect TargetRect )
{
    if ( GetMousePos_x()  > TargetRect.x + TargetRect.w ) return FALSE;
    if ( GetMousePos_x()  < TargetRect.x ) return FALSE;
    if ( GetMousePos_y()  > TargetRect.y + TargetRect.h ) return FALSE;
    if ( GetMousePos_y()  < TargetRect.y ) return FALSE;
    
    return ( TRUE );
}; // int ClickWasInRect ( SDL_Rect TargetRect )

static int ClickWasInEditorBannerRect( void )
{/*
    extern SDL_Rect EditorBannerRect; //XXX drop that
    return ( ClickWasInRect ( EditorBannerRect ) );*/
}; // int ClickWasInEditorBannerRect( void )

static void HandleBannerMouseClick( void )
{
    SDL_Rect TargetRect;
    int i;
    extern int FirstBlock; //XXX 
    {
	// could be a click on a block
        unsigned int num_blocks = GameConfig.screen_width / INITIAL_BLOCK_WIDTH - 1;
	for ( i = 0 ; i < num_blocks ; i ++ ) 
        {
	    TargetRect.x = INITIAL_BLOCK_WIDTH/2 + INITIAL_BLOCK_WIDTH * i; 
	    TargetRect.y = INITIAL_BLOCK_HEIGHT/3;
	    TargetRect.w = INITIAL_BLOCK_WIDTH;
	    TargetRect.h = INITIAL_BLOCK_HEIGHT;
//	    if ( ClickWasInRect ( TargetRect ) )
//		selected_tile_nb = FirstBlock + i;
        }
    }
    
    // check limits
  /*  if ( FirstBlock + 9 >= number_of_walls [ GameConfig . level_editor_edit_mode ] )
	FirstBlock = number_of_walls [ GameConfig . level_editor_edit_mode ] - 9 ;
    
    if ( FirstBlock < 0 )
	FirstBlock = 0;
    
    //--------------------
    // Now some extra security against selecting indices that would point to
    // undefined objects (floor tiles or obstacles) later
    // The following should never occur now - SN
    //
    if ( selected_tile_nb >= number_of_walls [ GameConfig . level_editor_edit_mode ] )
	selected_tile_nb = number_of_walls [ GameConfig . level_editor_edit_mode ] -1 ;
    */
}; // void HandleBannerMouseClick( void )


static void HandleLevelEditorCursorKeys ( leveleditor_state *cur_state )
{
   level *EditLevel;

    EditLevel = curShip.AllLevels [ Me . pos . z ] ;
    static int PressedSince[4] = { 0, 0, 0, 0 };
    int DoAct[4] = { 0, 0, 0, 0};
    int i;

    // Keys that are released have to be marked as such
    if (!LeftPressed())
	PressedSince[0] = 0;
    if (!RightPressed())
	PressedSince[1] = 0;
    if (!DownPressed())
	PressedSince[2] = 0;
    if (!UpPressed())
	PressedSince[3] = 0;

    if (LeftPressed() && PressedSince[0] == 0) {
	PressedSince[0] = SDL_GetTicks();
	DoAct[0] = 1;
    }
    if (RightPressed() && PressedSince[1] == 0) {
	PressedSince[1] = SDL_GetTicks();
	DoAct[1] = 1;
    }
    if (DownPressed() && PressedSince[2] == 0) {
	PressedSince[2] = SDL_GetTicks();
	DoAct[2] = 1;
    } 
    if (UpPressed() && PressedSince[3] == 0) {
	PressedSince[3] = SDL_GetTicks();
	DoAct[3] = 1;
    }

    for (i = 0; i < 4; i++) {
	if (PressedSince[i] && SDL_GetTicks() - PressedSince[i] > 500) {
	    DoAct[i] = 1;
	    PressedSince[i] = SDL_GetTicks() - 450;
	}
    }
    
    if (level_editor_marked_obstacle)
    {
	if ( CtrlPressed() )
	{
#if 0
	    //Uncomment to be able to change the borders of the currently marked obstacle 
	    if (DoAct[0])
		{
		obstacle_map[level_editor_marked_obstacle->type] . left_border -= 0.05;
		obstacle_map[level_editor_marked_obstacle->type] . right_border -= 0.05;
		}
	    if (DoAct[1])
		{
		obstacle_map[level_editor_marked_obstacle->type] . left_border += 0.05;
		obstacle_map[level_editor_marked_obstacle->type] . right_border += 0.05;
		}
	    if (DoAct[2])
		{
		obstacle_map[level_editor_marked_obstacle->type] . upper_border += 0.05;
		obstacle_map[level_editor_marked_obstacle->type] . lower_border += 0.05;
		}
	    if (DoAct[3])
		{
		obstacle_map[level_editor_marked_obstacle->type] . upper_border -= 0.05;
		obstacle_map[level_editor_marked_obstacle->type] . lower_border -= 0.05;
		}
#endif

#if 0
	    //Uncomment to be able to change the offset of the currently marked obstacle 

	    if (DoAct[0])
		{
		obstacle_map[level_editor_marked_obstacle->type] . image . offset_x -= 1;
		}
	    if (DoAct[1])
		{
		obstacle_map[level_editor_marked_obstacle->type] . image . offset_x += 1;
		}
	    if (DoAct[2])
		{
		obstacle_map[level_editor_marked_obstacle->type] . image . offset_y -= 1;
		}
	    if (DoAct[3])
		{
		obstacle_map[level_editor_marked_obstacle->type] . image . offset_y += 1;
		}
	    printf(_("Offset x %hd y %hd\n"), obstacle_map[level_editor_marked_obstacle->type] . image . offset_x, obstacle_map[level_editor_marked_obstacle->type] . image . offset_y);
#endif
	}
	else if ( MPressed() )
	{
	    if (DoAct[0])
	    {
		level_editor_marked_obstacle-> pos. x -= 0.015;
	    }
	    if (DoAct[1])
	    {
		level_editor_marked_obstacle-> pos .x  += 0.015;
	    }
	    if (DoAct[2])
	    {
		level_editor_marked_obstacle-> pos . y += 0.015;
	    }
	    if (DoAct[3])
	    {
		level_editor_marked_obstacle-> pos . y -= 0.015;
	    }
	    glue_obstacles_to_floor_tiles_for_level ( EditLevel -> levelnum );
	}
    }
#if 1
    if ( cur_state -> mode == NORMAL_MODE  && ! MPressed() )
    {
	if (DoAct[0]) 
	{
	    if ( rintf(Me.pos.x) > 0 ) Me.pos.x-=1;
	}
	if (DoAct[1]) 
	{ 
	    if ( rintf(Me.pos.x) < EditLevel->xlen-1 ) Me.pos.x+=1;
	}
	if (DoAct[2]) 
	{
	    if ( rintf(Me.pos.y) < EditLevel->ylen-1 ) Me.pos.y+=1;
	}
	if (DoAct[3]) 
	{
	    if ( rintf(Me.pos.y) > 0 ) Me.pos.y-=1;
	}
    }
#endif
}; // void HandleLevelEditorCursorKeys ( void )

static void level_editor_handle_left_mouse_button (leveleditor_state *cur_state )
{
    int new_x, new_y;
    moderately_finepoint pos;

    if ( MouseLeftClicked() && cur_state->mode == NORMAL_MODE )
    {
	if ( ClickWasInEditorBannerRect() )
	    HandleBannerMouseClick();
//	else
//	{
//	    //--------------------
//	    // With the left mouse button, it should be possible to actually 'draw'
//	    // something into the level.  This seems to work so far.  Caution is nescessary
//	    // to prevent segfault due to writing outside the level, but that's easily
//	    // accomplished.
//	    if ( ( (int)cur_state->TargetSquare . x >= 0 ) &&
//		    ( (int)cur_state->TargetSquare . x <= EditLevel()->xlen-1 ) &&
//		    ( (int)cur_state->TargetSquare . y >= 0 ) &&
//		    ( (int)cur_state->TargetSquare . y <= EditLevel()->ylen-1 ) )
//	    {
//		switch ( GameConfig . level_editor_edit_mode )
//		{
//		    case LEVEL_EDITOR_SELECTION_FLOOR:
//			start_rectangle_mode( cur_state , FALSE );
//			quickbar_use( GameConfig . level_editor_edit_mode , selected_tile_nb );
//			break;
//		    case LEVEL_EDITOR_SELECTION_QUICK:
//			quickbar_click ( EditLevel() , selected_tile_nb , cur_state);
//			break;
//		    case LEVEL_EDITOR_SELECTION_WALLS:
//			/* If the obstacle can be part of a line */
//			if ( (obstacle_map [ wall_indices [ GameConfig . level_editor_edit_mode ] [ selected_tile_nb ] ] . flags & IS_VERTICAL) ||
//				(obstacle_map [ wall_indices [ GameConfig . level_editor_edit_mode ] [ selected_tile_nb ] ] . flags & IS_HORIZONTAL) )
//			{
//			    /* Let's start the line (FALSE because the function will
//			     * find the tile by itself) */
//			    start_line_mode( cur_state , FALSE);
//			    quickbar_use ( cur_state->l_selected_mode , cur_state -> l_id );
//			}
//			break;
//		    default:
//			pos . x = cur_state -> TargetSquare . x;
//			pos . y = cur_state -> TargetSquare . y;
//			/* Completely disallow unaligned placement of walls, with tile granularity, using left click */
//			if (GameConfig . level_editor_edit_mode == LEVEL_EDITOR_SELECTION_WALLS)
//			{
//			    pos . x = (int)pos.x;
//			    pos . y = (int)pos.y;
//			}
//			action_create_obstacle_user ( EditLevel() , pos . x , pos . y , wall_indices [ GameConfig . level_editor_edit_mode ] [ selected_tile_nb ] );
//			quickbar_use ( GameConfig . level_editor_edit_mode, selected_tile_nb );
//		}
//	    }
//	}
    }

    if ( MouseLeftUnclicked() )
    {
	switch ( cur_state->mode )
	{
	    case LINE_MODE:
		/* Mouse right released ? terminate line of wall */
		end_line_mode(cur_state, TRUE);
		break;
	    case RECTANGLE_MODE:
		end_rectangle_mode(cur_state, TRUE);
		break;
	}
    }

}; // void level_editor_handle_left_mouse_button ( void )
/**
 * This function automatically scrolls the leveleditor window when the
 * mouse reaches an edge 
 */
static void level_editor_auto_scroll()
{
float chx = 0, chy = 0; /*Value of the change to player position*/

if ( GameConfig . screen_width - GetMousePos_x() < 5 )
	{ // scroll to the right
	chx += 0.05;
	chy -= 0.05;
	}

if ( GetMousePos_x() < 5 )
	{ // scroll to the left
	chx -= 0.05;
	chy += 0.05;
	}

if ( GameConfig . screen_height - GetMousePos_y() < 5 )
	{ // scroll down
	chx += 0.05;
	chy += 0.05;
	}

if ( GetMousePos_y() < 5 )
	{ //scroll up
	chx -= 0.05;
	chy -= 0.05;
	}


Me . pos . x += chx;
Me . pos . y += chy;

if ( Me . pos . x >= curShip.AllLevels[Me.pos.z]->xlen-1 )
	Me . pos . x = curShip.AllLevels[Me.pos.z]->xlen-1 ;
if ( Me . pos . x <= 0 ) Me . pos . x = 0;
if ( Me . pos . y >= curShip.AllLevels[Me.pos.z]->ylen-1 )
        Me . pos . y = curShip.AllLevels[Me.pos.z]->ylen-1 ;
if ( Me . pos . y <= 0 ) Me . pos . y = 0;
}	

void leveleditor_input_mouse_motion(SDL_Event *event)
{
    struct leveleditor_widget *w;

    w = get_active_widget(event->motion.x, event->motion.y);

    if (previously_active_widget != w) {
	if (w) 
	    w->mouseenter(w->ext);
	if (previously_active_widget)
	    previously_active_widget->mouseleave(previously_active_widget->ext);
	previously_active_widget = w;
    }
}

void leveleditor_input_mouse_button(SDL_Event *event)
{
    struct leveleditor_widget *w;

    w = get_active_widget(event->button.x, event->button.y);

    if (w) {
	switch (event->type) {
	    case SDL_MOUSEBUTTONUP:
		switch(event->button.button) {
		    case 1:
			w->mouserelease(w->ext);
			break;
		    case 3:
			w->mouserightrelease(w->ext);
			break;
		    case 4:
		    case 5:
			break;
		    default:
			ErrorMessage(__FUNCTION__, "Mouse button index %hd unhandled by leveleditor widgets.\n", PLEASE_INFORM, IS_WARNING_ONLY, event->button.button);
		}
		break;
	    case SDL_MOUSEBUTTONDOWN:
		switch(event->button.button) {
		    case 1:
			w->mousepress(w->ext);
			break;
		    case 3:
			w->mouserightpress(w->ext);
			break;
		    case 4:
			w->mousewheelup(w->ext);
			break;
		    case 5:
			w->mousewheeldown(w->ext);
			break;
		    default:
			ErrorMessage(__FUNCTION__, "Mouse button index %hd unhandled by leveleditor widgets.\n", PLEASE_INFORM, IS_WARNING_ONLY, event->button.button);
		}
		break;
	    default:
		ErrorMessage(__FUNCTION__, "Event type %d sent to leveleditor as a mouse button event is not recognized.\n", PLEASE_INFORM, IS_FATAL, event->type);
	}
    }
}

void leveleditor_process_input()
{
    int i;
    leveleditor_update_button_states();
    save_mouse_state();
    input_handle();
    
    HandleLevelEditorCursorKeys( cur_state );
    //--------------------
    // With the 'S' key, you can attach a statement for the influencer to 
    // say to a given location, i.e. the location the map editor cursor
    // currently is on.
    //
    if ( SPressed () )
	{
	while (SPressed());
	SetCurrentFont( FPS_Display_BFont );
	char *NewCommentOnThisSquare = 
	    GetEditableStringInPopupWindow ( 1000 , _("\n Please enter new statement for this tile: \n\n") ,
		    "");
	for ( i = 0 ; i < MAX_STATEMENTS_PER_LEVEL ; i ++ )
	    {
	    if ( EditLevel()->StatementList[ i ].x == (-1) ) break;
	    }
	if ( i == MAX_STATEMENTS_PER_LEVEL ) 
	    {
	    DisplayText ( _("\nNo more free comment position.  Using first. ") , -1 , -1 , &User_Rect , 1.0 );
	    i=0;
	    our_SDL_flip_wrapper();
	    getchar_raw(NULL);
	    // Terminate( ERR );
	    }

	EditLevel()->StatementList[ i ].Statement_Text = NewCommentOnThisSquare;
	EditLevel()->StatementList[ i ].x = rintf( Me.pos.x );
	EditLevel()->StatementList[ i ].y = rintf( Me.pos.y );
	}

    if ( level_editor_marked_obstacle && XPressed () )
	{
	action_remove_obstacle_user ( EditLevel() , level_editor_marked_obstacle );
	level_editor_marked_obstacle = NULL ;
	while ( XPressed() ) SDL_Delay(1);
	}

    //--------------------
    // The HKEY can be used to give a name to the currently marked obstacle
    //
    if ( HPressed() )
	{
	while(HPressed());
	action_change_obstacle_label_user ( EditLevel() , level_editor_marked_obstacle , NULL );
	while ( HPressed() ) SDL_Delay(1);
	}

    //--------------------
    // If the person using the level editor pressed w, the waypoint is
    // toggled on the current square.  That means either removed or added.
    // And in case of removal, also the connections must be removed.
    //
    if ( WPressed( ) )
	{
	if ( ! ShiftPressed() )
	    {
	    action_toggle_waypoint ( EditLevel() , EditX(), EditY() , FALSE );
	    }
	else
	    {
	    action_toggle_waypoint ( EditLevel() , EditX(), EditY() , TRUE );
	    }
	while ( WPressed() ) SDL_Delay(1);
	}

    //--------------------
    // First we find out which map square the player MIGHT wish us to operate on
    // via a POTENTIAL mouse click
    //
    cur_state->TargetSquare = translate_point_to_map_location (
	    (float) GetMousePos_x()  - ( GameConfig . screen_width / 2 ) , 
	    (float) GetMousePos_y()  - ( GameConfig . screen_height / 2 ) ,
	    GameConfig . zoom_is_on );

    //--------------------
    // The 'M' key will activate drag&drop mode to allow for convenient
    // obstacle moving.
    // 
    if ( MPressed () && 
	    MouseLeftClicked() && 
	    level_editor_marked_obstacle != NULL )
	{
	cur_state->mode = DRAG_DROP_MODE ;
	cur_state->d_selected_obstacle = level_editor_marked_obstacle;
	}


    switch ( cur_state->mode )
	{
	case LINE_MODE:
	    handle_line_mode(cur_state);
	    break;
	case RECTANGLE_MODE:
	    if ( ( (int)cur_state->TargetSquare . x >= 0 ) &&
		    ( (int)cur_state->TargetSquare . x <= EditLevel()->xlen-1 ) &&
		    ( (int)cur_state->TargetSquare . y >= 0 ) &&
		    ( (int)cur_state->TargetSquare . y <= EditLevel()->ylen-1 ) )
		handle_rectangle_mode(cur_state);
	    break;
	case DRAG_DROP_MODE:
	    if ( cur_state->d_selected_obstacle->pos.x != cur_state->TargetSquare.x &&
		    cur_state->d_selected_obstacle->pos.y != cur_state->TargetSquare.y )
		{
		cur_state->d_selected_obstacle -> pos . x = cur_state->TargetSquare.x;
		cur_state->d_selected_obstacle -> pos . y = cur_state->TargetSquare.y;
		glue_obstacles_to_floor_tiles_for_level ( EditLevel() -> levelnum );
		}
	    if ( ! MPressed () ) 
		{
		cur_state->mode = NORMAL_MODE;
		}
	    break;
	}

    level_editor_auto_scroll();

    level_editor_handle_left_mouse_button (cur_state );

    //--------------------
    // Maybe a right mouse click has in the map area.  Then it might be best to interpret this
    // simply as bigger move command, which might indeed be much handier than 
    // using only keyboard cursor keys to move around on the map.
    //
    if ( MouseRightPressed() )
	{

	if ( MouseRightClicked() )
	    {
	    cur_state -> c_last_right_click . x = GetMousePos_x();
	    cur_state -> c_last_right_click . y = GetMousePos_y();
	    cur_state -> c_origin . x = cur_state -> c_last_right_click . x;
	    cur_state -> c_origin . y = cur_state -> c_last_right_click . y;
	    }
	else
	    {
	    cur_state -> c_corresponding_position = translate_point_to_map_location (
		    cur_state -> c_last_right_click . x  - ( GameConfig . screen_width / 2 ) , 
		    cur_state -> c_last_right_click . y  - ( GameConfig . screen_height / 2 ) ,
		    GameConfig . zoom_is_on );

	    /* Calculate the new position */
	    Me . pos . x += (cur_state->TargetSquare . x - cur_state -> c_corresponding_position . x) / 30 ;
	    Me . pos . y += (cur_state->TargetSquare . y - cur_state -> c_corresponding_position . y) / 30 ;

	    }

	/* Security */
	if ( Me . pos . x > curShip.AllLevels[Me.pos.z]->xlen )
	    Me . pos . x = curShip.AllLevels[Me.pos.z]->xlen-1 ;
	if ( Me . pos . x < 0 )
	    Me . pos . x = 0;
	if ( Me . pos . y > curShip.AllLevels[Me.pos.z]->ylen )
	    Me . pos . y = curShip.AllLevels[Me.pos.z]->ylen-1 ;
	if ( Me . pos . y < 0 )
	    Me . pos . y = 0;
	}


    if ( EscapePressed() )
	{
	switch ( cur_state -> mode )
	    {
	    case LINE_MODE:
		// End line mode and *do not* place the walls
		end_line_mode(cur_state, FALSE);
		while ( EscapePressed() ) SDL_Delay(1);
		break;
	    case RECTANGLE_MODE:
		// Return to normal mode and *do not* place the walls
		end_rectangle_mode(cur_state, FALSE);
		while ( EscapePressed() ) SDL_Delay(1);
		break;
	    default:
		level_editor_done = DoLevelEditorMainMenu ( EditLevel() );
		break;
	    }
	}
	while( EscapePressed() ) SDL_Delay(1);
	


}

