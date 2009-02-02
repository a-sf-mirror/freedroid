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
#include "leveleditor_menu.h"
#include "leveleditor_widgets.h"

static void HandleLevelEditorCursorKeys ( )
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
    if (!MPressed())
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
	    w->mouseenter(event, w);
	if (previously_active_widget)
	    previously_active_widget->mouseleave(event, previously_active_widget);
	previously_active_widget = w;
    }

    if (w && w->mousemove)
	w->mousemove(event, w);
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
			w->mouserelease(event, w);
			break;
		    case 3:
			w->mouserightrelease(event, w);
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
			w->mousepress(event, w);
			break;
		    case 3:
			w->mouserightpress(event, w);
			break;
		    case 4:
			w->mousewheelup(event, w);
			break;
		    case 5:
			w->mousewheeldown(event, w);
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

void leveleditor_input_keybevent(SDL_Event *event)
{
    struct leveleditor_widget *w, *n;
       
    w = get_active_widget(GetMousePos_x(), GetMousePos_y());

    // When we get a keyboard event that wasn't handled by the "general" keybinding system,
    // it means we have something widget- and state- dependant.
    // We will forward the event to the currently active widget, and if it did not handle it,
    // forward it to each widget in the list in order.

    if (w && w->keybevent && !w->keybevent(event, w))
	return;

    list_for_each_entry_safe(w, n, &leveleditor_widget_list, node) {
	if (w && w->keybevent && !w->keybevent(event, w))
	    return;
    }       

}

void leveleditor_process_input()
{
    int i;
    leveleditor_update_button_states();
    save_mouse_state();
    input_handle();
    
    HandleLevelEditorCursorKeys();
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
    // The 'M' key will activate drag&drop mode to allow for convenient
    // obstacle moving.
    // 
/*    if ( MPressed () && 
	    MouseLeftClicked() && 
	    level_editor_marked_obstacle != NULL )
	{
	cur_state->mode = DRAG_DROP_MODE ;
	cur_state->d_selected_obstacle = level_editor_marked_obstacle;
	}
*/

 /*   switch ( cur_state->mode )
	{
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
*/
    level_editor_auto_scroll();

    if ( EscapePressed() )
	{
	level_editor_done = DoLevelEditorMainMenu ( EditLevel() );
	}
    while( EscapePressed() ) SDL_Delay(1);
	


}

