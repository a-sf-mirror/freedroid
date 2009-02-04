/* 
 *
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
 * This file contains all the actions performed by the level editor, ie. the functions that act on the level.
 */

#define _leveleditor_actions_c


#include "system.h"

#include "defs.h"
#include "struct.h"
#include "global.h"
#include "proto.h"

#include "SDL_rotozoom.h"

#include "lvledit/lvledit.h"
#include "lvledit/lvledit_widgets.h"
#include "lvledit/lvledit_widget_typeselect.h"

/* Undo/redo action lists */
LIST_HEAD (to_undo);
LIST_HEAD (to_redo);

static int push_mode = NORMAL; 


/**
 *  @fn void clear_action(action * pos)
 *
 *  @brief clears an action from its list, and pointers held within the action
 */
static void clear_action(action * action)
{
	if (action->type == ACT_SET_OBSTACLE_LABEL)
		free(action->d.change_obstacle_name.new_name);

    else if(action->type == ACT_SET_MAP_LABEL && action->d.change_label_name.new_name != NULL)
        free(action->d.change_label_name.new_name);
    
   list_del(&action->node);//< removes an action from a list
   free(action); //< free's the action
}

/**
 *  @fn void clear_action_list(struct list_head *list)
 *
 *  @brief clears an action list, and all its data
 */
static void clear_action_list(struct list_head *list)
{
    // free actions individually
    action *pos, *next;
    list_for_each_entry_safe(pos, next, list, node)
        clear_action(pos);
}

/**
 *  @fn static void action_freestack(void)
 *
 *  @brief clears to_undo and to_redo when LevelEditor() exits
 */
void action_freestack(void)
{ 
    clear_action_list(&to_redo);
    clear_action_list(&to_undo);
}

void action_push (int type, ...)
{
    va_list val;
    va_start (val, type);
    action *act = malloc (sizeof *act);
    act -> type = type;
    switch (type) {
    case ACT_CREATE_OBSTACLE:
	act -> d . create_obstacle . x = va_arg (val, double);
	act -> d . create_obstacle . y = va_arg (val, double);
	act -> d . create_obstacle . new_obstacle_type = va_arg (val, int);
	break;
    case ACT_REMOVE_OBSTACLE:
	act -> d . delete_obstacle = va_arg (val, obstacle *);
	break;
    case ACT_MOVE_OBSTACLE:
	act->d.move_obstacle.obstacle = va_arg(val, obstacle *);
	act->d.move_obstacle.newx = va_arg(val, double);
	act->d.move_obstacle.newy = va_arg(val, double);
	break;
    case ACT_WAYPOINT_TOGGLE:
    case ACT_WAYPOINT_TOGGLE_CONNECT:
	act -> d . waypoint_toggle . x = va_arg (val, int);
	act -> d . waypoint_toggle . y = va_arg (val, int);
	act -> d . waypoint_toggle . spawn_toggle = va_arg (val, int);
	break;
    case ACT_TILE_FLOOR_SET:
	act -> d . change_floor . x = va_arg (val, int);
	act -> d . change_floor . y = va_arg (val, int);
	act -> d . change_floor . type = va_arg (val, int);
	break;
    case ACT_MULTIPLE_ACTIONS:
	act -> d . number_actions = va_arg (val, int);
	break;
    case ACT_SET_OBSTACLE_LABEL:
	act -> d . change_obstacle_name . obstacle = va_arg (val, obstacle *);
	act -> d . change_obstacle_name . new_name = va_arg (val, char *);
	break;
    case ACT_SET_MAP_LABEL:
	act -> d . change_label_name . id = va_arg (val, int);
	act -> d . change_label_name . new_name = va_arg (val, char *);
	break;
    case ACT_JUMP_TO_LEVEL:
	act -> d . jump_to_level . target_level = va_arg (val, int);
	act -> d . jump_to_level . x = (float) va_arg (val, double);
	act -> d . jump_to_level . y = (float) va_arg (val, double);  
	break;
    }

    if (push_mode == UNDO) {
        // free() and list_del()
        clear_action((action *)to_undo.next);
	list_add (&act->node, &to_redo);
    } else {
        if (push_mode == REDO) clear_action( (action *)to_redo.next );
	    
        list_add (&act->node, &to_undo);
    }
}
	
obstacle * action_create_obstacle (Level EditLevel, double x, double y, int new_obstacle_type)
{
    int i;

    for ( i = 0 ; i < MAX_OBSTACLES_ON_MAP ; i ++ )
    {
	if ( EditLevel -> obstacle_list [ i ] . type == (-1) )
	{
	    EditLevel -> obstacle_list [ i ] . type = new_obstacle_type ;
	    EditLevel -> obstacle_list [ i ] . pos . x = x ;
	    EditLevel -> obstacle_list [ i ] . pos . y = y ;
	    EditLevel -> obstacle_list [ i] . name_index = (-1) ;
	    glue_obstacles_to_floor_tiles_for_level ( EditLevel -> levelnum );
	    DebugPrintf ( 0 , "\nNew obstacle has been added!!!" );
	    fflush(stdout);
	    //--------------------
	    // Now that we have disturbed the order of the obstacles on this level, we need
	    // to re-assemble the lists of pointers to obstacles, like the door list, the
	    // teleporter list and the refreshes list.
	    //
	    GetAnimatedMapTiles();
    
	    return ( & ( EditLevel -> obstacle_list [ i ] ) ) ;
	}
    }
    
    ErrorMessage ( __FUNCTION__  , "\
	    Ran out of obstacle positions in target level!",
			       PLEASE_INFORM , IS_FATAL );
    return ( NULL );
}

void action_move_obstacle(level *EditLevel, obstacle *obs, float newx, float newy)
{
    action_push(ACT_MOVE_OBSTACLE, obs, obs->pos.x, obs->pos.y); 
    obs->pos.x = newx;
    obs->pos.y = newy;
    glue_obstacles_to_floor_tiles_for_level(EditLevel->levelnum);
}

obstacle * action_create_obstacle_user (Level EditLevel, double x, double y, int new_obstacle)
{
    obstacle *o = action_create_obstacle  (EditLevel, x, y, new_obstacle);
    if (o) {
	action_push (ACT_REMOVE_OBSTACLE, o);
    }
    return o;
}
    
void action_remove_obstacle (level *EditLevel, obstacle *our_obstacle)
{
    //--------------------
    // The likely case that no obstacle was currently marked.
    //
    if (our_obstacle == NULL) 
	return;
    
    our_obstacle -> type = ( -1 ) ;
    
    //--------------------
    // Now doing that must have shifted the glue!  That is a problem.  We need to
    // reglue everything to the map...
    //
    glue_obstacles_to_floor_tiles_for_level ( EditLevel -> levelnum );
    
    //--------------------
    // Now that we have disturbed the order of the obstacles on this level, we need
    // to re-assemble the lists of pointers to obstacles, like the door list, the
    // teleporter list and the refreshes list.
    //
    GetAnimatedMapTiles();
}

void action_remove_obstacle_user (Level EditLevel, obstacle *our_obstacle)
{
    action_push (ACT_CREATE_OBSTACLE, our_obstacle -> pos . x,
		 our_obstacle -> pos . y, 
		 our_obstacle -> type);
    action_remove_obstacle (EditLevel, our_obstacle);
}

static void action_change_obstacle_label (level *EditLevel, obstacle *obstacle, char *name)
{
    int check_double;
    char *old_name = NULL;
    int index = -1;
    int i;

    //--------------------
    // If the obstacle already has a name, we can use that index for the 
    // new name now.
    //
    if ( obstacle -> name_index >= 0 )
	index = obstacle -> name_index ;
    else
    {
	//--------------------
	// Else we must find a free index in the list of obstacle names for this level
	//
	for ( i = 0 ; i < MAX_OBSTACLE_NAMES_PER_LEVEL ; i ++ )
	{
	    if ( EditLevel -> obstacle_name_list [ i ] == NULL )
	    {
		index = i ;
		break;
	    }
	}
	if ( index < 0 ) return;
    }

    
    old_name = EditLevel -> obstacle_name_list [ index ];
    if (!name || strlen (name) == 0) {
	obstacle -> name_index = -1;
	EditLevel -> obstacle_name_list [ index ] = NULL;
    } else {
	EditLevel -> obstacle_name_list [ index ] = name;
	obstacle -> name_index = index ;
    }
    action_push (ACT_SET_OBSTACLE_LABEL, obstacle, old_name);
    
    if ( obstacle->name_index == -1 )
	    return;

    //--------------------
    // But even if we fill in something new, we should first
    // check against double entries of the same label.  Let's
    // do it...
    //
    for ( check_double = 0 ; check_double < MAX_OBSTACLE_NAMES_PER_LEVEL ; check_double++ )
	{
	//--------------------
	// We must not use null pointers for string comparison...
	//
	if ( EditLevel -> obstacle_name_list [ check_double ] == NULL ) continue ;

	//--------------------
	// We must not overwrite ourself with us in foolish ways :)
	//
	if ( check_double == index ) continue ;

	//--------------------
	// But in case of real double-entries, we'll handle them right.
	//
	if ( ! strcmp ( EditLevel -> obstacle_name_list [ index ] , 
		    EditLevel -> obstacle_name_list [ check_double ] ) )
	    {
	    ErrorMessage ( __FUNCTION__  , "\
		    The label %s did already exist on this map!  Deleting old entry in favour of the new one!",
		    NO_NEED_TO_INFORM , IS_WARNING_ONLY, EditLevel -> obstacle_name_list [ index ] );
	    EditLevel -> obstacle_name_list [ index ] = NULL ;
	    obstacle -> name_index = check_double ;
	    break;
	    }
	}
}

void action_change_obstacle_label_user (level *EditLevel, obstacle *our_obstacle, char *predefined_name)
{
    int cur_idx;
    char *name;

    if (!our_obstacle)
	return;

    cur_idx = our_obstacle->name_index;

    //--------------------
    // Maybe we must query the user for the desired new name.
    // On the other hand, it might be that a name has been
    // supplied as an argument.  That depends on whether the
    // argument string is NULL or not.
    //
    if ( predefined_name == NULL )
    {
	name = 
	    GetEditableStringInPopupWindow ( 1000 , _("\nPlease enter name for this obstacle: \n\n") ,
					     cur_idx != -1 ? EditLevel -> obstacle_name_list [ cur_idx ] : "");
    }
    else
    {
	name = strdup(predefined_name);
    }

    action_change_obstacle_label ( EditLevel, our_obstacle, name);
}   

void action_change_obstacle_description (level *EditLevel , obstacle* our_obstacle , char* predefined_description )
{
    int i;
    int free_index=(-1);
    
    //--------------------
    // If the obstacle already has a name, we can use that index for the 
    // new name now.
    //
    if ( our_obstacle -> description_index >= 0 )
	free_index = our_obstacle -> description_index ;
    else
    {
	//--------------------
	// Else we must find a free index in the list of obstacle names for this level
	//
	for ( i = 0 ; i < MAX_OBSTACLE_NAMES_PER_LEVEL ; i ++ )
	{
	    if ( EditLevel -> obstacle_description_list [ i ] == NULL )
	    {
		free_index = i ;
		break;
	    }
	}
	if ( free_index < 0 ) 
	{
	    ErrorMessage ( __FUNCTION__  , "\
		    Ran out of obstacle description positions on this map level!",
				       PLEASE_INFORM , IS_WARNING_ONLY );
	    return;
	}
    }
    
    //--------------------
    // Maybe we must query the user for the desired new name.
    // On the other hand, it might be that a name has been
    // supplied as an argument.  That depends on whether the
    // argument string is NULL or not.
    //
    if ( EditLevel -> obstacle_description_list [ free_index ] == NULL )
	EditLevel -> obstacle_description_list [ free_index ] = "" ;
    if ( predefined_description == NULL )
    {
	EditLevel -> obstacle_description_list [ free_index ] = 
	    GetEditableStringInPopupWindow ( 1000 , _("\n\
Please enter new description text for this obstacle: \n\n") ,
					     EditLevel -> obstacle_description_list [ free_index ] );
    }
    else
    {
	EditLevel -> obstacle_description_list [ free_index ] = MyMalloc ( 10000 );
	strncpy ( EditLevel -> obstacle_description_list [ free_index ] , predefined_description , 9900 ) ;
    }
    
    //--------------------
    // We must select the right index as the name of this obstacle.
    //
    our_obstacle -> description_index = free_index ;
    
    //--------------------
    // But if the given name was empty, then we remove everything again
    // and RETURN
    //
    if ( strlen ( EditLevel -> obstacle_description_list [ free_index ] ) == 0 )
    {
    	free (EditLevel -> obstacle_description_list [ free_index ]);
	EditLevel -> obstacle_description_list [ free_index ] = NULL ;
	our_obstacle -> description_index = (-1);
    }
    
};



void action_toggle_waypoint (level *EditLevel , int BlockX , int BlockY , int toggle_random_spawn )
{
    int i;
    // find out if there is a waypoint on the current square
    for ( i = 0 ; i < EditLevel->num_waypoints ; i++ )
    {
	if ( ( EditLevel->AllWaypoints[i].x == BlockX ) &&
	     ( EditLevel->AllWaypoints[i].y == BlockY ) ) break;
    }
    
    //--------------------
    // If its waypoint already, this waypoint must either be deleted
    // or the random spawn bit reset...
    //
    if ( i < EditLevel -> num_waypoints )
    {
	if ( toggle_random_spawn )
	{
	    if ( EditLevel -> AllWaypoints [ i ] . suppress_random_spawn )
		EditLevel -> AllWaypoints [ i ] . suppress_random_spawn = 0 ;
	    else
		EditLevel -> AllWaypoints [ i ] . suppress_random_spawn = 1 ;
	}
	else
	    DeleteWaypoint ( EditLevel , i );
    }
    else // if its not a waypoint already, it must be made into one
    {
	if ( ! toggle_random_spawn )
	    CreateWaypoint ( EditLevel , BlockX , BlockY );
    }
    action_push (ACT_WAYPOINT_TOGGLE, BlockX, BlockY, toggle_random_spawn);
}


int action_toggle_waypoint_connection (level *EditLevel, int id_origin, int id_target)
{
    int i = 0;
    waypoint *SrcWp = &(EditLevel->AllWaypoints[id_origin]);
    for (i = 0; i < SrcWp -> num_connections; i++) {
	// Already a waypoint, remove it
	if (SrcWp -> connections [ i ] == id_target) {
	    memmove (SrcWp->connections + i, SrcWp->connections + i + 1,
		     (SrcWp->num_connections - (i + 1)) * sizeof (SrcWp->connections[0]));
	    SrcWp -> num_connections -- ;
	    return -1;
	}
    }
    SrcWp -> connections [ SrcWp -> num_connections ] = id_target;
    SrcWp -> num_connections ++;
    SrcWp = NULL;
    action_push (ACT_WAYPOINT_TOGGLE_CONNECT, id_origin, id_target, -1);
    return 1;
}

void level_editor_action_toggle_waypoint_connection_user (level *EditLevel)
{
    int i;


    int xpos = EditX();
    int ypos = EditY();

    // Determine which waypoint is currently targeted
    for (i=0 ; i < EditLevel->num_waypoints ; i++)
    {
	if ( ( EditLevel->AllWaypoints[i].x == xpos ) &&
	     ( EditLevel->AllWaypoints[i].y == ypos ) ) break;
    }
    
    if ( i == EditLevel->num_waypoints )
    {
	sprintf( VanishingMessage , _("Sorry, don't know which waypoint you mean."));
	VanishingMessageEndDate = SDL_GetTicks() + 7000;
    }
    else
    {
	sprintf( VanishingMessage , _("You specified waypoint nr. %d.") , i );
	VanishingMessageEndDate = SDL_GetTicks() + 7000;
	if ( OriginWaypoint== (-1) )
	{
	    OriginWaypoint = i;
	    if (EditLevel->AllWaypoints[OriginWaypoint].num_connections < MAX_WP_CONNECTIONS)
	    {
		strcat ( VanishingMessage , _("\nIt has been marked as the origin of the next connection." ));
		DebugPrintf (1, "\nWaypoint nr. %d. selected as origin\n", i);
	    }
	    else
	    {
		sprintf (VanishingMessage, _("\nsORRY, MAXIMAL NUMBER OF WAYPOINT-CONNECTIONS (%d) REACHED!\n"), MAX_WP_CONNECTIONS);
		DebugPrintf (0, "Operation not possible\n");
		OriginWaypoint = (-1);
	    }
	}
	else
	{
	    if ( OriginWaypoint == i )
	    {
		strcat ( VanishingMessage , _("\n\nOrigin==Target --> Connection Operation cancelled."));
		OriginWaypoint = (-1);
	    }
	    else
	    {
		sprintf( VanishingMessage , _("\n\nOrigin: %d Target: %d. Operation makes sense."), OriginWaypoint , i );
		if (action_toggle_waypoint_connection ( EditLevel, OriginWaypoint, i ) < 0) {
		    strcat ( VanishingMessage , _("\nOperation done, connection removed." ));
		} else {
		    strcat ( VanishingMessage , _("\nOperation done, connection added." ));
		}
		OriginWaypoint = (-1);
	    }
	}
    }
    
    return;
    
}

void action_set_floor (Level EditLevel, int x, int y, int type)
{
    int old = EditLevel -> map [ y ] [ x ] . floor_value;
    EditLevel -> map [ y ] [ x ] . floor_value = type;
    action_push (ACT_TILE_FLOOR_SET, x, y, old);
}

static int tile_is_free (level *EditLevel, int y_old, int x_old, int y_new, int x_new) 
{
	int i;
	float x, y;
	int wall_id = -1;
	int obstacle_id;
	for ( i = 0; i < MAX_OBSTACLES_GLUED_TO_ONE_MAP_TILE; ++i) {
		obstacle_id = EditLevel -> map [ y_new ] [ x_new ] . obstacles_glued_to_here [ i ] ;
		if (obstacle_id != -1 && 
				(EditLevel -> obstacle_list [ obstacle_id ] . type >= ISO_V_WALL ||
				 EditLevel -> obstacle_list [ obstacle_id ] . type <= ISO_OUTER_WALL_E3) ) {
			wall_id = obstacle_id;
			y = EditLevel -> obstacle_list [ obstacle_id ] . pos . y;
			x = EditLevel -> obstacle_list [ obstacle_id ] . pos . x;
			break;
		}
	}
	return TRUE;
}

static void action_fill_user_recursive (level *EditLevel, int x, int y, int type, int *changed)
{
    int source_type = EditLevel->map [ y ] [ x ] . floor_value;
    /* security */
    if ( ( x < 0 ) || ( y < 0 ) || ( x >= EditLevel->xlen ) || ( y >= EditLevel->ylen ) ) 
	return;

#define at(x,y) (EditLevel -> map [ y ] [ x ] . floor_value)    
    if ( at (x , y) == type )
	return;
    action_set_floor ( EditLevel, x, y, type);
    (*changed) ++;
    if ( x > 0 && at (x-1, y) == source_type && tile_is_free(EditLevel, y, x, y, x-1) )
	action_fill_user_recursive (EditLevel, x-1, y, type, changed);
    if ( x < EditLevel->xlen-1 && at (x+1, y) == source_type && tile_is_free(EditLevel, y, x, y, x+1) )
	action_fill_user_recursive (EditLevel, x+1, y, type, changed);
    if ( y > 0 && at (x, y-1) == source_type && tile_is_free(EditLevel, y, x, y-1, x) )
	action_fill_user_recursive (EditLevel, x, y-1, type, changed);
    if ( y < EditLevel->ylen-1 && at (x-1, y+1) == source_type && tile_is_free(EditLevel, y+1, x, y, x) )
	action_fill_user_recursive (EditLevel, x, y+1, type, changed);
}

void action_fill_user (level *EditLevel, int BlockX, int BlockY, int SpecialMapValue)
{
    int number_changed = 0;
    action_fill_user_recursive ( EditLevel, BlockX, BlockY, SpecialMapValue, &number_changed);
    action_push ( ACT_MULTIPLE_ACTIONS, number_changed);
}

static void action_change_map_label (level *EditLevel, int i, char *name)
{
    if (EditLevel -> labels [ i ] . pos . x != -1 ) {
	int check_double;
	if (name) {
	    for ( check_double = 0 ; check_double < MAX_MAP_LABELS_PER_LEVEL ; check_double++ )
		{
		    if ( ! strcmp ( name , EditLevel -> labels [ check_double ] . label_name ) )
			{
			    ErrorMessage ( __FUNCTION__  , "\
				    The label just entered did already exist on this map!  Deleting old entry in favour of the new one!",
					   PLEASE_INFORM , IS_WARNING_ONLY );
			    i = check_double ;
			    break;
			}
		}
	}
	action_push (ACT_SET_MAP_LABEL, i, EditLevel -> labels [ i ] . label_name);
    } else {
	action_push (ACT_SET_MAP_LABEL, i, NULL);
    }	     
    if (name && strlen (name)) {
	EditLevel -> labels [ i ] . label_name = name;
	EditLevel -> labels [ i ] . pos . x = rintf( Me.pos.x - 0.5 );
	EditLevel -> labels [ i ] . pos . y = rintf( Me.pos.y - 0.5 );
    } else {
	EditLevel -> labels [ i ] . label_name = ("NoLabelHere") ;
	EditLevel -> labels [ i ] . pos . x = (-1) ;
	EditLevel -> labels [ i ] . pos . y = (-1) ;
    }
}

void level_editor_action_change_map_label_user (level *EditLevel)
{
    char* NewCommentOnThisSquare;
    int i;

    SetCurrentFont( FPS_Display_BFont );

    //--------------------
    // Now we see if a map label entry is existing already for this spot
    //
    for ( i = 0 ; i < MAX_MAP_LABELS_PER_LEVEL ; i ++ )
    {
	if ( ( fabsf ( EditLevel -> labels [ i ] . pos . x + 0.5 - Me.pos.x ) < 0.5 ) &&
	     ( fabsf ( EditLevel -> labels [ i ] . pos . y + 0.5 - Me.pos.y ) < 0.5 ) ) 
	{
	    break;
	}
    }
    if ( i >= MAX_MAP_LABELS_PER_LEVEL ) 
    {
	NewCommentOnThisSquare = 
	    GetEditableStringInPopupWindow ( 1000 , _("\nNo existing map label entry for this position found...\n Please enter new label for this map position: \n\n") ,
					     "");
	
	i=0;
	for ( i = 0 ; i < MAX_MAP_LABELS_PER_LEVEL ; i ++ )
	{
	    if ( EditLevel -> labels [ i ] . pos . x == (-1) )
		break;
	}
	if ( i >= MAX_MAP_LABELS_PER_LEVEL )
	{
	    DisplayText ( _("\nNo more free map label entry found... using first on instead ...\n") , -1 , -1 , &User_Rect , 1.0 );
	    i = 0;
	}
	else
	{
	    DisplayText ( _("\nUsing new map label list entry...\n") , -1 , -1 , &User_Rect , 1.0 );
	}
	// Terminate( ERR );
    }
    else
    {
	NewCommentOnThisSquare = 
	    GetEditableStringInPopupWindow ( 1000 , _("\nOverwriting existing map label list entry...\n Please enter new label for this map position: \n\n") ,
					     EditLevel -> labels [ i ] . label_name );
    }
    action_change_map_label ( EditLevel, i, NewCommentOnThisSquare);
}

/**
 *  @fn void jump_to_level( int target_map, float x, float y)
 *
 *  @brief jumps to a target level, saving this level on the undo/redo stack
 */
void action_jump_to_level( int target_level, double x, double y)
{
    action_push(ACT_JUMP_TO_LEVEL,EditLevel()->levelnum, Me.pos.x, Me.pos.y);//< sets undo or redo stack, depending on push_mode state
    Teleport(target_level, (float)x, (float)y, FALSE);
}

static void action_do (level *level, action *a )
{
    switch (a->type) {
    case ACT_CREATE_OBSTACLE:
	action_create_obstacle_user ( level, a->d.create_obstacle.x, 
				 a->d.create_obstacle.y, a->d.create_obstacle.new_obstacle_type );
	break;
    case ACT_REMOVE_OBSTACLE:
	action_remove_obstacle_user ( level, a->d.delete_obstacle);
	break;
    case ACT_MOVE_OBSTACLE:
	action_move_obstacle(level, a->d.move_obstacle.obstacle, a->d.move_obstacle.newx, a->d.move_obstacle.newy);
	break;
    case ACT_WAYPOINT_TOGGLE:
	action_toggle_waypoint ( level, a->d.waypoint_toggle.x, a->d.waypoint_toggle.y,
				 a->d.waypoint_toggle.spawn_toggle );
	break;
    case ACT_WAYPOINT_TOGGLE_CONNECT:
	action_toggle_waypoint_connection ( level, a->d.waypoint_toggle.x, a->d.waypoint_toggle.y);
	break;
    case ACT_TILE_FLOOR_SET:
	action_set_floor ( level, a->d.change_floor.x, a->d.change_floor.y, a->d.change_floor.type);
	break; 
    case ACT_MULTIPLE_ACTIONS:
	break;
    case ACT_SET_OBSTACLE_LABEL:
	action_change_obstacle_label (level, a->d.change_obstacle_name.obstacle, 
				      a->d.change_obstacle_name.new_name);
	break;
    case ACT_SET_MAP_LABEL:
	action_change_map_label (level, a->d.change_label_name.id, a->d.change_label_name.new_name);
	break;
    case ACT_JUMP_TO_LEVEL:
	action_jump_to_level (a->d.jump_to_level.target_level,a->d.jump_to_level.x,a->d.jump_to_level.y);
	break;

    }
}

void level_editor_action_undo ()
{
    if (!list_empty(&to_undo)) {
	action *a = (action *)to_undo.next;
	push_mode = UNDO;
	if (a->type == ACT_MULTIPLE_ACTIONS) {
	    int i;
            
		list_del(to_undo.next);
            
	    for(i = 0; i < a->d.number_actions; i++) {
		level_editor_action_undo();
	    }
	    list_add(&a->node, &to_redo);
	} else {
	    action_do(EditLevel(), a);
	}
	push_mode = NORMAL;
    }
}

void level_editor_action_redo ()
{
     if (!list_empty(&to_redo)) {
	action *a = (action *)to_redo.next;
	push_mode = REDO;
	if (a->type == ACT_MULTIPLE_ACTIONS) {
	    int i;
            
		list_del(to_redo.next);
            
	    for (i = 0; i < a->d.number_actions; i++) {
		level_editor_action_redo ();
	    }
	    list_add(&a->node, &to_undo);
	} else {
	    action_do(EditLevel(), a);
	}
	push_mode = NORMAL;
    }
}
    
/**
 *
 *
 */
void level_editor_place_aligned_obstacle ( int positionid )
{
    struct quickbar_entry *entry = NULL;
    struct leveleditor_typeselect *ts = get_current_object_type();
    int obstacle_id = -1;
    int placement_is_possible = TRUE;
    int obstacle_created = FALSE;
    float position_offset_x[9] = { 0, 0.5, 1.0, 0, 0.5, 1.0, 0, 0.5, 1.0 };
    float position_offset_y[9] = { 1.0, 1.0, 1.0, 0.5, 0.5, 0.5, 0, 0, 0 };

    positionid--;

    if (ts->type != OBJECT_OBSTACLE && ts->type != OBJECT_ANY )
	return;
	//ErrorMessage(__FUNCTION__, "Cannot \"place aligned obstacle\" for selected object type %d\n", PLEASE_INFORM, IS_FATAL, ts->type);

    /* Try to get a quickbar entry */
    if (ts->type == OBJECT_ANY) {
	GiveMouseAlertWindow("Implement quickbar");
	/*entry = quickbar_getentry ( selected_tile_nb );
	  if (entry)
	  {
	  obstacle_type = entry -> obstacle_type;
	  id = entry -> id;
	  }
	  else
	  {
	  placement_is_possible = FALSE;
	  }*/
    } else {
	obstacle_id = ts->indices[ts->selected_tile_nb];
    }

    if (placement_is_possible)  {
	action_create_obstacle_user (EditLevel(), ((int)Me.pos.x) + position_offset_x[positionid], ((int)Me.pos.y) + position_offset_y[positionid], obstacle_id);
	obstacle_created = TRUE;
	/*	if (object_list !=  && obstacle_created )
		quickbar_use ( obstacle_type, id );*/
    }
}


#undef _leveleditor_action_c

