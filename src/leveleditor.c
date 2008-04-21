/* 
 *
 *   Copyright (c) 2002, 2003 Johannes Prix
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

/* ----------------------------------------------------------------------
 * This file contains all functions for the heart of the level editor.
 * ---------------------------------------------------------------------- */

#define _leveleditor_c

#include "system.h"

#include "defs.h"
#include "struct.h"
#include "global.h"
#include "proto.h"

#include "SDL_rotozoom.h"

#include "leveleditor.h"

int OriginWaypoint = (-1);

char VanishingMessage[10000]="";
float VanishingMessageDisplayTime = 0;
SDL_Rect EditorBannerRect = { 0 , 0 , 640 , 90 } ;
int FirstBlock = 0 ;
int Highlight = 3 ;
int number_of_walls [ NUMBER_OF_LEVEL_EDITOR_GROUPS ] ;
int level_editor_mouse_move_mode = FALSE ;
int level_editor_done = FALSE;

int BlockX ;
int BlockY ;
Level EditLevel;
int main_menu_requested;

LIST_HEAD (quickbar_entries);

LIST_HEAD (to_undo);
LIST_HEAD (to_redo);
int push_mode = NORMAL; 
    
void
action_freestack ( void )
{
    struct list_head *pos, *n;
    list_for_each_safe (pos, n, &to_undo) {
	action *a = (action *)pos;
	if (a->type == ACT_SET_OBSTACLE_LABEL)
	    free ( a->d.change_obstacle_name.new_name );

	else if (a->type == ACT_SET_MAP_LABEL)
	    free ( a->d.change_label_name.new_name );

	free ( a );
    }

    INIT_LIST_HEAD (&to_undo);

    list_for_each_safe (pos, n, &to_redo) {
	action *a = (action *)pos;
	if (a->type == ACT_SET_OBSTACLE_LABEL)
	    free ( a->d.change_obstacle_name.new_name );

	else if (a->type == ACT_SET_MAP_LABEL)
	    free ( a->d.change_label_name.new_name );
	free ( a );
    }

    INIT_LIST_HEAD (&to_redo);
}

void
action_do ( Level level, action *a )
{
    switch (a->type) {
    case ACT_CREATE_OBSTACLE:
	action_create_obstacle_user ( level, a->d.create_obstacle.x, 
				 a->d.create_obstacle.y, a->d.create_obstacle.new_obstacle_type );
	break;
    case ACT_REMOVE_OBSTACLE:
	action_remove_obstacle_user ( level, a->d.delete_obstacle);
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
    case ACT_MULTIPLE_FLOOR_SETS:
	break;
    case ACT_SET_OBSTACLE_LABEL:
	action_change_obstacle_label (level, a->d.change_obstacle_name.obstacle, 
				      a->d.change_obstacle_name.new_name);
	break;
    case ACT_SET_MAP_LABEL:
	action_change_map_label (level, a->d.change_label_name.id, a->d.change_label_name.new_name);
	break;

    }
}

void 
action_undo ( Level level ) 
{
    if (!list_empty(&to_undo)) {
	action *a = (action *)to_undo.next;
	push_mode = UNDO;
	if (a->type == ACT_MULTIPLE_FLOOR_SETS) {
	    int i;
	    list_del (to_undo.next);
	    for (i = 0; i < a->d.number_fill_set; i++) {
		action_undo (level);
	    }
	    list_add (&a->node, &to_redo);
	} else {
	    action_do (level, a);
	}
	push_mode = NORMAL;
    }
}

void 
action_redo ( Level level ) 
{
     if (!list_empty(&to_redo)) {
	action *a = (action *)to_redo.next;
	push_mode = REDO;
	if (a->type == ACT_MULTIPLE_FLOOR_SETS) {
	    int i;
	    list_del (to_redo.next);
	    for (i = 0; i < a->d.number_fill_set; i++) {
		action_redo (level);
	    }
	    list_add (&a->node, &to_undo);
	} else {
	    action_do (level, a);
	}
	push_mode = NORMAL;
    }
}
    
	
void
action_push (int type, ...)
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
    case ACT_MULTIPLE_FLOOR_SETS:
	act -> d . number_fill_set = va_arg (val, int);
	break;
    case ACT_SET_OBSTACLE_LABEL:
	act -> d . change_obstacle_name . obstacle = va_arg (val, obstacle *);
	act -> d . change_obstacle_name . new_name = va_arg (val, char *);
	break;
    case ACT_SET_MAP_LABEL:
	act -> d . change_label_name . id = va_arg (val, int);
	act -> d . change_label_name . new_name = va_arg (val, char *);
	break;
    }

    if (push_mode == UNDO) {
	list_del (to_undo.next);
	list_add (&act->node, &to_redo);
    } else {
	if (push_mode == REDO) list_del(to_redo.next);
	list_add (&act->node, &to_undo);
    }
}
	
obstacle *
action_create_obstacle (Level EditLevel, double x, double y, int new_obstacle_type)
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
	    return ( & ( EditLevel -> obstacle_list [ i ] ) ) ;
	}
    }
    
    ErrorMessage ( __FUNCTION__  , "\
Ran out of obstacle positions in target level!",
			       PLEASE_INFORM , IS_FATAL );
    return ( NULL );
}

obstacle *
action_create_obstacle_user (Level EditLevel, double x, double y, int new_obstacle)
{
    obstacle *o = action_create_obstacle  (EditLevel, x, y, new_obstacle);
    if (o) {
	action_push (ACT_REMOVE_OBSTACLE, o);
    }
    return o;
}
void
action_remove_obstacle_user (Level EditLevel, obstacle *our_obstacle)
{
    action_push (ACT_CREATE_OBSTACLE, our_obstacle -> pos . x,
		 our_obstacle -> pos . y, 
		 our_obstacle -> type);
    action_remove_obstacle (EditLevel, our_obstacle);
}
void
action_remove_obstacle ( Level EditLevel, obstacle *our_obstacle)
{
    int i;
    int obstacle_index = (-1) ;
    
    //--------------------
    // The likely case that no obstacle was currently marked.
    //
    if ( our_obstacle == NULL ) return;
    
    //--------------------
    // We need to find out the index of the obstacle in question,
    // so that we can find out and eliminate any glue for this 
    // obstacle.
    //
    for ( i = 0 ; i < MAX_OBSTACLES_ON_MAP ; i ++ )
    {
	if ( our_obstacle == & ( EditLevel -> obstacle_list [ i ] ) )
	{
	    obstacle_index = i ;
	    break;
	}
    }
    
    //--------------------
    // Maybe there is a severe bug somewhere in FreedroidRPG.  We catch
    // this case as well...
    //
    if ( obstacle_index == (-1) )
    {
	ErrorMessage ( __FUNCTION__  , "\
Unable to find the obstacle in question within the obstacle list!",
				   PLEASE_INFORM , IS_FATAL );
    }
    
    //--------------------
    // And of course we must not forget to delete the obstalce itself
    // as well, not only the glue...
    //
    our_obstacle -> type = ( -1 ) ;
    
    //--------------------
    // Maybe filling out the gap isn't so desireable after all.  Might that cause
    // problems with keeping track of the indices when obstacles are named?  Should
    // we do away with this?  But then we also need to go over -1 entries in the
    // loops coursing throught he whole list in other places...  So it will stay for
    // now I guess...
    //
    memmove ( & ( EditLevel -> obstacle_list [ obstacle_index ] ) , 
	      & ( EditLevel -> obstacle_list [ obstacle_index + 1 ] ) ,
	      ( MAX_OBSTACLES_ON_MAP - obstacle_index - 2 ) * sizeof ( obstacle ) );
    
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
    GetAllAnimatedMapTiles( EditLevel );
}


void 
action_toggle_waypoint ( Level EditLevel , int BlockX , int BlockY , int toggle_random_spawn )
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


int 
action_toggle_waypoint_connection ( Level EditLevel, int id_origin, int id_target)
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
void
action_toggle_waypoint_connection_user ( Level EditLevel , int BlockX , int BlockY )
{
    int i;
 
    // Determine which waypoint is currently targeted
    for (i=0 ; i < EditLevel->num_waypoints ; i++)
    {
	if ( ( EditLevel->AllWaypoints[i].x == BlockX ) &&
	     ( EditLevel->AllWaypoints[i].y == BlockY ) ) break;
    }
    
    if ( i == EditLevel->num_waypoints )
    {
	sprintf( VanishingMessage , "Sorry, don't know which waypoint you mean." );
	VanishingMessageDisplayTime = 0;
    }
    else
    {
	sprintf( VanishingMessage , "You specified waypoint nr. %d." , i );
	VanishingMessageDisplayTime = 0;
	if ( OriginWaypoint== (-1) )
	{
	    OriginWaypoint = i;
	    if (EditLevel->AllWaypoints[OriginWaypoint].num_connections < MAX_WP_CONNECTIONS)
	    {
		strcat ( VanishingMessage , "\nIt has been marked as the origin of the next connection." );
		DebugPrintf (1, "\nWaypoint nr. %d. selected as origin\n", i);
	    }
	    else
	    {
		strcat ( VanishingMessage , "\nSORRY. NO MORE CONNECTIONS AVAILABLE FROM THERE." );
		strcat ( VanishingMessage, 
			 va("\nSorry, maximal number of waypoint-connections (%d) reached!\n", MAX_WP_CONNECTIONS));
		DebugPrintf (0, "Operation not possible\n");
		OriginWaypoint = (-1);
	    }
	}
	else
	{
	    if ( OriginWaypoint == i )
	    {
		strcat ( VanishingMessage , "\n\nOrigin==Target --> Connection Operation cancelled.");
		OriginWaypoint = (-1);
	    }
	    else
	    {
		sprintf( VanishingMessage , "\n\nOrigin: %d Target: %d. Operation makes sense.", OriginWaypoint , i );
		if (action_toggle_waypoint_connection ( EditLevel, OriginWaypoint, i ) < 0) {
		    strcat ( VanishingMessage , "\nOperation done, connection removed." );
		} else {
		    strcat ( VanishingMessage , "\nOperation done, connection added." );
		}
		OriginWaypoint = (-1);
	    }
	}
    }
    
    return;
    
}


void
action_set_floor (Level EditLevel, int x, int y, int type)
{
    int old = EditLevel -> map [ y ] [ x ] . floor_value;
    EditLevel -> map [ y ] [ x ] . floor_value = type;
    action_push (ACT_TILE_FLOOR_SET, x, y, old);
}

void
action_fill_user_recursive ( Level EditLevel, int x, int y, int type, int *changed)
{
    int source_type = EditLevel->map [ y ] [ x ] . floor_value;
    /* security */
    if ( ( y < 0 ) || ( y < 0 ) || ( x >= EditLevel->xlen ) || ( y >= EditLevel->ylen ) )
	return;
#define at(x,y) (EditLevel -> map [ y ] [ x ] . floor_value)    
    if ( at (x , y) == type )
	return;
    action_set_floor ( EditLevel, x, y, type);
    (*changed) ++;
    if ( x > 0 && at (x-1, y) == source_type)
	action_fill_user_recursive (EditLevel, x-1, y, type, changed);
    if ( x < EditLevel->xlen-1 && at (x+1, y) == source_type)
	action_fill_user_recursive (EditLevel, x+1, y, type, changed);
    if ( y > 0 && at (x, y-1) == source_type)
	action_fill_user_recursive (EditLevel, x, y-1, type, changed);
    if ( y < EditLevel->ylen-1 && at (x-1, y+1) == source_type)
	action_fill_user_recursive (EditLevel, x, y+1, type, changed);
}
void
action_fill_user ( Level EditLevel, int BlockX, int BlockY, int SpecialMapValue)
{
    int number_changed = 0;
    action_fill_user_recursive ( EditLevel, BlockX, BlockY, SpecialMapValue, &number_changed);
    action_push ( ACT_MULTIPLE_FLOOR_SETS, number_changed);
}

void
action_change_obstacle_label ( Level EditLevel, obstacle *obstacle, char *name)
{
    int index = -1;
    char *old_name;
    if (obstacle -> name_index >= 0) {
	index = obstacle -> name_index;
    } else {
	int i;
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
    }
    action_push (ACT_SET_OBSTACLE_LABEL, obstacle, old_name);
    obstacle -> name_index = index ;
}
    
void
action_change_obstacle_label_user ( Level EditLevel, obstacle *our_obstacle, char *predefined_name)
{
    int i;
    int free_index=(-1);
    int check_double;
    char *name;
    //--------------------
    // If the obstacle already has a name, we can use that index for the 
    // new name now.
    //
    if ( our_obstacle -> name_index >= 0 )
	free_index = our_obstacle -> name_index ;
    else
    {
	//--------------------
	// Else we must find a free index in the list of obstacle names for this level
	//
	for ( i = 0 ; i < MAX_OBSTACLE_NAMES_PER_LEVEL ; i ++ )
	{
	    if ( EditLevel -> obstacle_name_list [ i ] == NULL )
	    {
		free_index = i ;
		break;
	    }
	}
	if ( free_index < 0 ) return;
    }
    
    //--------------------
    // Maybe we must query the user for the desired new name.
    // On the other hand, it might be that a name has been
    // supplied as an argument.  That depends on whether the
    // argument string is NULL or not.
    //
    if ( EditLevel -> obstacle_name_list [ free_index ] == NULL )
	EditLevel -> obstacle_name_list [ free_index ] = "" ;
    if ( predefined_name == NULL )
    {
	name = 
	    GetEditableStringInPopupWindow ( 1000 , "\nPlease enter name for this obstacle: \n\n" ,
					     EditLevel -> obstacle_name_list [ free_index ] );
    }
    else
    {
	name = EditLevel -> obstacle_name_list [ free_index ];
    }

    // if name is a valid label, otherwise delete me
    if(strlen(name) != 0)
    {// valid label
        action_change_obstacle_label ( EditLevel, our_obstacle, name );
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
	            if ( check_double == free_index ) continue ;
	
	            //--------------------
	            // But in case of real double-entries, we'll handle them right.
	            //
	            if ( ! strcmp ( EditLevel -> obstacle_name_list [ free_index ] , 
			            EditLevel -> obstacle_name_list [ check_double ] ) )
	            {
	                ErrorMessage ( __FUNCTION__  , "\
The label just entered did already exist on this map!  Deleting old entry in favour of the new one!",
				                   NO_NEED_TO_INFORM , IS_WARNING_ONLY );
	                EditLevel -> obstacle_name_list [ free_index ] = NULL ;
	                our_obstacle -> name_index = check_double ;
	                break;
	            }
	        }
    }
    else
    {// not a label, throw away
       EditLevel -> obstacle_name_list [ free_index ] = NULL;
       our_obstacle -> name_index = -1;
    }


}   

void
action_change_map_label ( Level EditLevel, int i, char *name)
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
	EditLevel -> labels [ i ] . label_name = "NoLabelHere" ;
	EditLevel -> labels [ i ] . pos . x = (-1) ;
	EditLevel -> labels [ i ] . pos . y = (-1) ;
    }
}
void
action_change_map_label_user (Level EditLevel)
{
    char* NewCommentOnThisSquare;
    int i;
    
    while (PPressed());
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
	    GetEditableStringInPopupWindow ( 1000 , "\nNo existing map label entry for this position found...\n Please enter new label for this map position: \n\n" ,
					     "" );
	
	i=0;
	for ( i = 0 ; i < MAX_MAP_LABELS_PER_LEVEL ; i ++ )
	{
	    if ( EditLevel -> labels [ i ] . pos . x == (-1) )
		break;
	}
	if ( i >= MAX_MAP_LABELS_PER_LEVEL )
	{
	    DisplayText ( "\nNo more free map label entry found... using first on instead ...\n" , -1 , -1 , &User_Rect , 1.0 );
	    i = 0;
	}
	else
	{
	    DisplayText ( "\nUsing new map label list entry...\n" , -1 , -1 , &User_Rect , 1.0 );
	}
	// Terminate( ERR );
    }
    else
    {
	NewCommentOnThisSquare = 
	    GetEditableStringInPopupWindow ( 1000 , "\nOverwriting existing map label list entry...\n Please enter new label for this map position: \n\n" ,
					     EditLevel -> labels [ i ] . label_name );
    }
    action_change_map_label ( EditLevel, i, NewCommentOnThisSquare);
}

/* ------------------
 * Quickbar functions
 * ------------------
 */
struct quickbar_entry *
quickbar_getentry ( int id )
{
    int i = 0;
    struct list_head *node;
    list_for_each (node, &quickbar_entries) {
	if (id == i) {
	    struct quickbar_entry *entry = list_entry (node, struct quickbar_entry, node);
	    return entry;
	}
	i ++;
    }
    return NULL;
}

iso_image *
quickbar_getimage ( int selected_index , int *placing_floor ) 
{
    struct quickbar_entry *entry = quickbar_getentry ( selected_index );
    if (!entry) 
	return NULL;
    if (entry->obstacle_type == LEVEL_EDITOR_SELECTION_FLOOR) {
	*placing_floor = TRUE;
	return &floor_iso_images  [ entry->id ];
    } else {
	return &obstacle_map [wall_indices [ entry -> obstacle_type ] [ entry->id ] ] . image;
    }
}

/* Inserts an item in a sorted list */
void
quickbar_additem (struct quickbar_entry *entry)
{
    struct quickbar_entry *tmp1, *tmp2;
    struct quickbar_entry *smallest, *biggest;
    struct list_head *node;
    /* The smallest element (if the list is non-empty) is the last element */
    smallest = list_entry(quickbar_entries.prev, struct quickbar_entry, node);
    /* Biggest one */ 
    biggest = list_entry(quickbar_entries.next, struct quickbar_entry, node);

    /* If the list is empty or if the entry we want to insert is smaller than 
     * the smallest element, just insert the entry */
    if ((list_empty(&quickbar_entries)) ||
	    (entry->used < smallest->used)) {
	list_add_tail(&entry->node, &quickbar_entries);
    /* If it's bigger than the biggest one, let it be the first */
    } else if (entry->used > biggest->used) {
	list_add(&entry->node, &quickbar_entries);
    } else {
	/* We know the element is between two entries, so let's find the place */
	list_for_each (node, &quickbar_entries) {
	    tmp1 = list_entry (node, struct quickbar_entry, node);
	    tmp2 = list_entry (node->next, struct quickbar_entry, node);
	    if (tmp1->used >= entry->used && entry->used >= tmp2->used) {
		list_add (&entry->node, &tmp1->node);
		break;
	    }
	}
    }

    int i = 0;
    list_for_each (node, &quickbar_entries) i++;
    number_of_walls [ LEVEL_EDITOR_SELECTION_QUICK ] = i;
}

void
quickbar_use (int obstacle, int id)
{
    struct list_head *node;
    struct quickbar_entry *entry = NULL;;
    list_for_each (node, &quickbar_entries) {
	entry = list_entry (node, struct quickbar_entry, node);
	if (entry->id == id && entry->obstacle_type == obstacle)  {
	    break;
	}
    }
    if (entry && node != &quickbar_entries) {
	entry->used ++;
	list_del (&entry->node);
	quickbar_additem (entry);
    } else {
	entry = MyMalloc (sizeof *entry);
	entry->obstacle_type = obstacle;
	entry->id = id;
	entry->used = 1;
	quickbar_additem (entry);
    }
}

void
quickbar_click ( Level level, int id, moderately_finepoint TargetSquare, whole_line *walls)
{
    struct quickbar_entry *entry = quickbar_getentry ( id );
    if ( entry ) {
	switch ( entry->obstacle_type )
	{
	    case LEVEL_EDITOR_SELECTION_FLOOR:
		action_set_floor (level, TargetSquare.x, TargetSquare.y, entry->id);
		break;
	    case LEVEL_EDITOR_SELECTION_WALLS:
		walls->editor_mode = entry->obstacle_type;
		walls->id = entry->id;
		start_line_mode(walls, TargetSquare, TRUE);
		handle_line_mode(walls, TargetSquare);
		break;
	    default:
	    action_create_obstacle_user (level, 
		    TargetSquare . x, TargetSquare . y, 
		    wall_indices [ entry -> obstacle_type ] [ entry -> id ]);
	}
	entry->used ++;
    }
}    

/* ----------------------------------------------------------------------
 * Is this tile a 'full' grass tile, i.e. a grass tile with ABSOLUTELY
 * NO SAND on it?
 * ---------------------------------------------------------------------- */
int
is_full_grass_tile ( map_tile* this_tile )
{

    switch ( this_tile -> floor_value )
    {
	case ISO_FLOOR_SAND_WITH_GRASS_1:
	case ISO_FLOOR_SAND_WITH_GRASS_2:
	case ISO_FLOOR_SAND_WITH_GRASS_3:
	case ISO_FLOOR_SAND_WITH_GRASS_4:
	    return ( TRUE );
	    break;

	default:
	    return ( FALSE );
	    break;
    }

}; // int is_full_grass_tile ( map_tile* this_tile )

/* ----------------------------------------------------------------------
 * Is this tile 'some' grass tile, i.e. a grass tile with JUST ANY BIT
 * OF GRASS ON IT?
 * ---------------------------------------------------------------------- */
int
is_some_grass_tile ( map_tile* this_tile )
{

    if ( this_tile -> floor_value < ISO_FLOOR_SAND_WITH_GRASS_1 )
	return ( FALSE );
    if ( this_tile -> floor_value > ISO_FLOOR_SAND_WITH_GRASS_29 )
	return ( FALSE );


    switch ( this_tile -> floor_value )
    {
	case ISO_WATER:
	case ISO_COMPLETELY_DARK:
	case ISO_RED_WAREHOUSE_FLOOR:
	    return ( FALSE );
	    break;
	default:
	    break;
    }

    return ( TRUE );

}; // int is_full_grass_tile ( map_tile* this_tile )

/* ----------------------------------------------------------------------
 *
 *
 * ---------------------------------------------------------------------- */
void
fix_corners_in_this_grass_tile ( level* EditLevel , int x , int y ) 
{
    map_tile* this_tile = & ( EditLevel -> map [ y ] [ x ] ) ;
    int north_grass = 0 ;
    int south_grass = 0 ;
    int east_grass = 0 ;
    int west_grass = 0 ;

    if ( is_full_grass_tile ( & ( EditLevel -> map [ y - 1 ] [ x     ] ) ) )
	north_grass = TRUE ;
    if ( is_full_grass_tile ( & ( EditLevel -> map [ y + 1 ] [ x     ] ) ) )
	south_grass = TRUE ;
    if ( is_full_grass_tile ( & ( EditLevel -> map [ y     ] [ x + 1 ] ) ) )
	east_grass = TRUE ;
    if ( is_full_grass_tile ( & ( EditLevel -> map [ y     ] [ x - 1 ] ) ) )
	west_grass = TRUE ;

    //--------------------
    // Upper left corner:
    //
    if ( north_grass && west_grass 
	 && ( EditLevel -> map [ y     ] [ x + 1 ] . floor_value == ISO_FLOOR_SAND )
	 && ( EditLevel -> map [ y + 1 ] [ x     ] . floor_value == ISO_FLOOR_SAND ) )
    {
	this_tile -> floor_value = ISO_FLOOR_SAND_WITH_GRASS_15 ;
	DebugPrintf ( -4 , "\nFixed a north-west corner in the grass tiles." );
    }

    //--------------------
    // Upper right corner
    //
    if ( north_grass && east_grass 
	 && ( EditLevel -> map [ y     ] [ x - 1 ] . floor_value == ISO_FLOOR_SAND )
	 && ( EditLevel -> map [ y + 1 ] [ x     ] . floor_value == ISO_FLOOR_SAND ) )
    {
	this_tile -> floor_value = ISO_FLOOR_SAND_WITH_GRASS_16 ;
	DebugPrintf ( -4 , "\nFixed a north-east corner in the grass tiles." );
    }
    
    //--------------------
    // Lower left corner:
    //
    if ( south_grass && west_grass 
	 && ( EditLevel -> map [ y     ] [ x + 1 ] . floor_value == ISO_FLOOR_SAND )
	 && ( EditLevel -> map [ y - 1 ] [ x     ] . floor_value == ISO_FLOOR_SAND ) )
    {
	this_tile -> floor_value = ISO_FLOOR_SAND_WITH_GRASS_14 ;
	DebugPrintf ( -4 , "\nFixed a south-west corner in the grass tiles." );
    }

    //--------------------
    // Lower right corner
    //
    if ( south_grass && east_grass 
	 && ( EditLevel -> map [ y     ] [ x - 1 ] . floor_value == ISO_FLOOR_SAND )
	 && ( EditLevel -> map [ y - 1 ] [ x     ] . floor_value == ISO_FLOOR_SAND ) )
    {
	this_tile -> floor_value = ISO_FLOOR_SAND_WITH_GRASS_17 ;
	DebugPrintf ( -4 , "\nFixed a south-east corner in the grass tiles." );
    }
    
}; // void fix_corners_in_this_grass_tile ( EditLevel , x , y ) 

/* ----------------------------------------------------------------------
 * Now we fix those grass tiles, that have only very little contact to
 * pure sand tiles, i.e. only 1/8 of the area is grass.
 * ---------------------------------------------------------------------- */
void
fix_anticorners_in_this_grass_tile ( level* EditLevel , int x , int y ) 
{
    map_tile* this_tile = & ( EditLevel -> map [ y ] [ x ] ) ;
    int north_grass = 0 ;
    int south_grass = 0 ;
    int east_grass = 0 ;
    int west_grass = 0 ;
    int x_offset, y_offset;

    if ( is_some_grass_tile ( & ( EditLevel -> map [ y - 1 ] [ x     ] ) ) ){
	north_grass = TRUE ;
        y_offset = -1;
    }
    if ( is_some_grass_tile ( & ( EditLevel -> map [ y + 1 ] [ x     ] ) ) ){
	south_grass = TRUE ;
        y_offset = 1;
    }
    if ( is_some_grass_tile ( & ( EditLevel -> map [ y     ] [ x + 1 ] ) ) ){
	east_grass = TRUE ;
        x_offset = 1;
    }
    if ( is_some_grass_tile ( & ( EditLevel -> map [ y     ] [ x - 1 ] ) ) ){
	west_grass = TRUE ;
        x_offset = -1;
    }

    //--------------------
    // Upper left corner:
    //
    if ( north_grass && west_grass 
	 && ( EditLevel -> map [ y - 1 ] [ x - 1 ] . floor_value == ISO_FLOOR_SAND ) )
    {
	this_tile -> floor_value = ISO_FLOOR_SAND_WITH_GRASS_21 ;
	DebugPrintf ( -4 , "\nFixed a north-west anticorner in the grass tiles." );
    }

    //--------------------
    // Upper right corner
    //
    if ( north_grass && east_grass 
	 && ( EditLevel -> map [ y - 1 ] [ x + 1 ] . floor_value == ISO_FLOOR_SAND ) )
    {
	this_tile -> floor_value = ISO_FLOOR_SAND_WITH_GRASS_20 ;
	DebugPrintf ( -4 , "\nFixed a north-east anticorner in the grass tiles." );
    }
    
    //--------------------
    // Lower left corner:
    //
    if ( south_grass && west_grass 
	 && ( EditLevel -> map [ y + 1 ] [ x - 1 ] . floor_value == ISO_FLOOR_SAND ) )
    {
	this_tile -> floor_value = ISO_FLOOR_SAND_WITH_GRASS_18 ;
	DebugPrintf ( -4 , "\nFixed a south-west anticorner in the grass tiles." );
    }

    //--------------------
    // Lower right corner
    //
    if ( south_grass && east_grass 
	 && ( EditLevel -> map [ y + 1 ] [ x + 1 ] . floor_value == ISO_FLOOR_SAND ) )
    {
	this_tile -> floor_value = ISO_FLOOR_SAND_WITH_GRASS_19 ;
	DebugPrintf ( -4 , "\nFixed a south-east anticorner in the grass tiles." );
    }
    
}; // void fix_anticorners_in_this_grass_tile ( EditLevel , x , y ) 

/* ----------------------------------------------------------------------
 * Now we fix those grass tiles, that have only very little contact to
 * pure sand tiles, i.e. only 1/8 of the area is grass.
 * ---------------------------------------------------------------------- */
void
fix_halfpieces_in_this_grass_tile ( level* EditLevel , int x , int y ) 
{
    map_tile* this_tile = & ( EditLevel -> map [ y ] [ x ] ) ;
    int north_grass = 0 ;
    int south_grass = 0 ;
    int east_grass = 0 ;
    int west_grass = 0 ;

    if ( is_some_grass_tile ( & ( EditLevel -> map [ y - 1 ] [ x     ] ) ) )
	north_grass = TRUE ;
    if ( is_some_grass_tile ( & ( EditLevel -> map [ y + 1 ] [ x     ] ) ) )
	south_grass = TRUE ;
    if ( is_some_grass_tile ( & ( EditLevel -> map [ y     ] [ x + 1 ] ) ) )
	east_grass = TRUE ;
    if ( is_some_grass_tile ( & ( EditLevel -> map [ y     ] [ x - 1 ] ) ) )
	west_grass = TRUE ;

    //--------------------
    // Fix sand on the west:
    //
    if ( east_grass && !west_grass )
    {
	if ( MyRandom ( 100 ) < 50 )
	    this_tile -> floor_value = ISO_FLOOR_SAND_WITH_GRASS_6 ;
	else
	    this_tile -> floor_value = ISO_FLOOR_SAND_WITH_GRASS_7 ;
	DebugPrintf ( -4 , "\nFixed east-grass tiles." );
    }

    //--------------------
    // Fix sand on the east:
    //
    if ( !east_grass && west_grass )
    {
	if ( MyRandom ( 100 ) < 50 )
	    this_tile -> floor_value = ISO_FLOOR_SAND_WITH_GRASS_9 ;
	else
	    this_tile -> floor_value = ISO_FLOOR_SAND_WITH_GRASS_8 ;
	DebugPrintf ( -4 , "\nFixed a west-grass tiles." );
    }
    
    //--------------------
    // Fix sand on the north:
    //
    if ( south_grass && !north_grass )
    {
	if ( MyRandom ( 100 ) < 50 )
	    this_tile -> floor_value = ISO_FLOOR_SAND_WITH_GRASS_13 ;
	else
	    this_tile -> floor_value = ISO_FLOOR_SAND_WITH_GRASS_12 ;
	DebugPrintf ( -4 , "\nFixed a south-west anticorner in the grass tiles." );
    }

    //--------------------
    // Fix sand on the south:
    //
    if ( !south_grass && north_grass )
    {
	if ( MyRandom ( 100 ) < 50 )
	    this_tile -> floor_value = ISO_FLOOR_SAND_WITH_GRASS_11 ;
	else
	    this_tile -> floor_value = ISO_FLOOR_SAND_WITH_GRASS_10 ;
	DebugPrintf ( -4 , "\nFixed a south-east anticorner in the grass tiles." );
    }
    
}; // void fix_halfpieces_in_this_grass_tile ( EditLevel , x , y ) 

/* ----------------------------------------------------------------------
 *
 *
 * ---------------------------------------------------------------------- */
void
fix_isolated_grass_tile ( level* EditLevel , int x , int y  )
{
    map_tile* this_tile = & ( EditLevel -> map [ y ] [ x ] ) ;
    int north_grass = 0 ;
    int south_grass = 0 ;
    int east_grass = 0 ;
    int west_grass = 0 ;
    int our_rand ;

    if ( is_some_grass_tile ( & ( EditLevel -> map [ y - 1 ] [ x     ] ) ) )
	north_grass = TRUE ;
    if ( is_some_grass_tile ( & ( EditLevel -> map [ y + 1 ] [ x     ] ) ) )
	south_grass = TRUE ;
    if ( is_some_grass_tile ( & ( EditLevel -> map [ y     ] [ x + 1 ] ) ) )
	east_grass = TRUE ;
    if ( is_some_grass_tile ( & ( EditLevel -> map [ y     ] [ x - 1 ] ) ) )
	west_grass = TRUE ;

    if ( !north_grass && !south_grass && !east_grass && !west_grass )
    {
	DebugPrintf ( -4 , "\nFixed an isolated grass tile." );
	our_rand = MyRandom ( 100 );
	if ( our_rand < 33 )
	    this_tile -> floor_value = ISO_FLOOR_SAND_WITH_GRASS_22 ;
	else if ( our_rand < 66 )
	    this_tile -> floor_value = ISO_FLOOR_SAND_WITH_GRASS_23 ;
	else
	    this_tile -> floor_value = ISO_FLOOR_SAND_WITH_GRASS_24 ;
    }

}; // void fix_isolated_grass_tile ( EditLevel , x , y ) 

/* ----------------------------------------------------------------------
 * When planting grass tiles, taking care of the smooth borders for these
 * grass tiles can be a tedious job.  A short function might help to 
 * beautify the grass.  If focuses on replacing 'full' grass tiles with 
 * proper full and part-full grass tiles.
 * ---------------------------------------------------------------------- */
void
beautify_grass_tiles_on_level ( level* EditLevel )
{
    int x ;
    int y ;
    int our_rand;
    map_tile* this_tile;
    
    DebugPrintf ( -4 , "\nbeautify_grass_tiles_on_level (...): process started..." );

    //--------------------
    // First we fix the pure corner pieces, cutting away quit some grass
    // 
    for ( x = 1 ; x < EditLevel -> xlen - 1 ; x ++ )
    {
	for ( y = 1 ; y < EditLevel -> ylen - 1 ; y ++ )
	{
	    this_tile = & ( EditLevel -> map [ y ] [ x ] ) ;

	    if ( is_full_grass_tile ( this_tile ) )
	    {
		fix_corners_in_this_grass_tile ( EditLevel , x , y ) ;
		DebugPrintf ( 1 , "\nbeautify_grass_tiles_on_level (...): found a grass tile." );
	    }
	}
    }

    //--------------------
    // Now we fix the anticorner pieces, cutting away much less grass
    // 
    for ( x = 1 ; x < EditLevel -> xlen - 1 ; x ++ )
    {
	for ( y = 1 ; y < EditLevel -> ylen - 1 ; y ++ )
	{
	    this_tile = & ( EditLevel -> map [ y ] [ x ] ) ;

	    if ( is_full_grass_tile ( this_tile ) )
	    {
		fix_anticorners_in_this_grass_tile ( EditLevel , x , y ) ;
		DebugPrintf ( 1 , "\nbeautify_grass_tiles_on_level (...): found a grass tile." );
	    }
	}
    }

    //--------------------
    // Now we fix the halftile pieces, cutting away much less grass
    // 
    for ( x = 1 ; x < EditLevel -> xlen - 1 ; x ++ )
    {
	for ( y = 1 ; y < EditLevel -> ylen - 1 ; y ++ )
	{
	    this_tile = & ( EditLevel -> map [ y ] [ x ] ) ;

	    if ( is_full_grass_tile ( this_tile ) )
	    {
		fix_halfpieces_in_this_grass_tile ( EditLevel , x , y ) ;
		DebugPrintf ( 1 , "\nbeautify_grass_tiles_on_level (...): found a grass tile." );
	    }
	}
    }

    //--------------------
    // Finally we randomize the full grass tiles
    // 
    for ( x = 1 ; x < EditLevel -> xlen - 1 ; x ++ )
    {
	for ( y = 1 ; y < EditLevel -> ylen - 1 ; y ++ )
	{
	    this_tile = & ( EditLevel -> map [ y ] [ x ] ) ;

	    if ( is_full_grass_tile ( this_tile ) )
	    {
		our_rand = MyRandom ( 106 ) ;
		if ( our_rand < 25 )
		    this_tile -> floor_value = ISO_FLOOR_SAND_WITH_GRASS_1 ;
		else if ( our_rand < 50 )
		    this_tile -> floor_value = ISO_FLOOR_SAND_WITH_GRASS_2 ;
		else if ( our_rand < 75 )
		    this_tile -> floor_value = ISO_FLOOR_SAND_WITH_GRASS_3 ;
		else if ( our_rand < 100 )
		    this_tile -> floor_value = ISO_FLOOR_SAND_WITH_GRASS_4 ;
		else if ( our_rand < 102 )
		    this_tile -> floor_value = ISO_FLOOR_SAND_WITH_GRASS_25 ;
		else if ( our_rand < 104 )
		    this_tile -> floor_value = ISO_FLOOR_SAND_WITH_GRASS_26 ;
		else 
		    this_tile -> floor_value = ISO_FLOOR_SAND_WITH_GRASS_27 ;
	    }
	}
    }

    //--------------------
    // Finally we randomize the full grass tiles
    // 
    for ( x = 1 ; x < EditLevel -> xlen - 1 ; x ++ )
    {
	for ( y = 1 ; y < EditLevel -> ylen - 1 ; y ++ )
	{
	    this_tile = & ( EditLevel -> map [ y ] [ x ] ) ;

	    if ( is_full_grass_tile ( this_tile ) )
	    {
		fix_isolated_grass_tile ( EditLevel , x , y ) ;
		DebugPrintf ( 1 , "\nbeautify_grass_tiles_on_level (...): found a grass tile." );
	    }
	}
    }

}; // void beautify_grass_tiles_on_level ( void )

/* ----------------------------------------------------------------------
 *
 *
 * ---------------------------------------------------------------------- */
void
close_all_chests_on_level ( int l_num ) 
{
  int i;

  for ( i = 0 ; i < MAX_OBSTACLES_ON_MAP ; i ++ )
    {
      switch ( curShip . AllLevels [ l_num ] -> obstacle_list [ i ] . type )
	{
	case ISO_H_CHEST_OPEN:
	  curShip . AllLevels [ l_num ] -> obstacle_list [ i ] . type = ISO_H_CHEST_CLOSED ;
	  break;
	case ISO_V_CHEST_OPEN:
	  curShip . AllLevels [ l_num ] -> obstacle_list [ i ] . type = ISO_V_CHEST_CLOSED ;
	  break;
	default:
	  break;
	}
    }

}; // void close_all_chests_on_level ( int l_num ) 

/* ----------------------------------------------------------------------
 *
 *
 * ---------------------------------------------------------------------- */
void
create_new_obstacle_on_level ( Level EditLevel , int our_obstacle_type , float pos_x , float pos_y )
{
    int i;
    int free_index = ( -1 ) ;
    //--------------------
    // The special 'obstacle_type' (-1) can be given, which means that this
    // function will have to find out the proper type all by itself...
    //
    if ( our_obstacle_type == (-1) )
	{
	ErrorMessage ( __FUNCTION__, "Attempted to create an obstacle of type -1. This is not supported any longer, obstacle type has to be specified explicitely both for create_new_obstacle_on_level and action_create_obstacle*.", PLEASE_INFORM, IS_FATAL);
	}
    
  //--------------------
  // First we find a free obstacle index to insert our new obstacle
  //
  for ( i = 0 ; i < MAX_OBSTACLES_ON_MAP ; i ++ )
    {
      if ( EditLevel -> obstacle_list [ i ] . type == (-1) )
	{
	  free_index = i ;
	  break;
	}
    }
  if ( free_index < 0 )
    {
      ErrorMessage ( __FUNCTION__  , "\
Ran out of obstacles!   Too bad!  Raise max obstacles constant!" , 
				 PLEASE_INFORM , IS_FATAL );
    }

  //--------------------
  // Now that we have an obstacle at our disposal, we can start to fill in
  // the right obstacle properties.
  //
  EditLevel -> obstacle_list [ free_index ] . type = our_obstacle_type; 
  EditLevel -> obstacle_list [ free_index ] . pos . x = pos_x ;
  EditLevel -> obstacle_list [ free_index ] . pos . y = pos_y ;
  EditLevel -> obstacle_list [ free_index ] . name_index = (-1) ;

  //--------------------
  // Now that the new obstacle has been created, it must still be glued to the
  // floor on some tile, so that the engine will recognize the need to blit it.
  //
  glue_obstacles_to_floor_tiles_for_level ( EditLevel -> levelnum );

}; // void create_new_obstacle_on_level ( Level EditLevel , int our_obstacle_type , float pos_x , float pos_y )




	
/* ----------------------------------------------------------------------
 * When new lines are inserted into the map, the map labels south of this
 * line must move too with the rest of the map.  This function sees to it.
 * ---------------------------------------------------------------------- */
void
MoveMapLabelsSouthOf ( int FromWhere , int ByWhat , Level EditLevel )
{
  int i;

  for ( i = 0 ; i < MAX_MAP_LABELS_PER_LEVEL ; i ++ )
    {
      if ( EditLevel -> labels [ i ] . pos . x <= ( -1 ) ) continue;
      
      if ( EditLevel -> labels [ i ] . pos . y >= FromWhere )
	EditLevel -> labels [ i ] . pos . y += ByWhat;
    }
  
}; // void MoveMapLabelsSouthOf ( int FromWhere , int ByWhat , Level EditLevel)

/* ----------------------------------------------------------------------
 *
 *
 * ---------------------------------------------------------------------- */
void
move_obstacles_east_of ( float from_where , float by_what , Level edit_level )
{
  int i;

  for ( i = 0 ; i < MAX_OBSTACLES_ON_MAP ; i ++ )
    {
      //--------------------
      // Maybe the obstacle entry isn't used at all.  That's the simplest
      // case...: do nothing.
      //
      if ( edit_level -> obstacle_list [ i ] . type <= ( -1 ) ) continue;
      if ( edit_level -> obstacle_list [ i ] . pos . x <= ( -1 ) ) continue;
      
      //--------------------
      // Maybe the obstacle is right on the spot where it must be deleted
      // because the floor under it will move out.
      //
      if ( ( edit_level -> obstacle_list [ i ] . pos . x >= from_where ) &&
	   ( edit_level -> obstacle_list [ i ] . pos . x <= from_where - by_what ) )
	{
	  action_remove_obstacle ( edit_level , & ( edit_level -> obstacle_list [ i ] ) ) ;
	  i -- ; 
	  DebugPrintf ( 0 , "\nRemoved another obstacle in resizing operation." );
	  continue;
	}

      //--------------------
      // Now at this point we can be sure that the obstacle just needs to be 
      // moved a bit.  That shouldn't be too hard to do...
      //
      if ( edit_level -> obstacle_list [ i ] . pos . x > from_where )
	edit_level -> obstacle_list [ i ] . pos . x += by_what ;
    }

  for ( i = 0 ; i < MAX_ITEMS_PER_LEVEL ; i ++ )
    {
      //--------------------
      // Maybe the item entry isn't used at all.  That's the simplest
      // case...: do nothing.
      //
      if ( edit_level -> ItemList [ i ] . type <= ( -1 ) ) continue;
      if ( edit_level -> ItemList [ i ] . pos . x <= ( -1 ) ) continue;
      
      //--------------------
      // Maybe the item is right on the spot where it must be deleted
      // because the floor under it will move out.
      //
      if ( ( edit_level -> ItemList [ i ] . pos . x >= from_where ) &&
	   ( edit_level -> ItemList [ i ] . pos . x <= from_where - by_what ) )
	{
	  DeleteItem ( & ( edit_level -> ItemList [ i ] ) ) ;
	  DebugPrintf ( 0 , "\nRemoved another item in resizing operation." );
	  continue;
	}

      //--------------------
      // Now at this point we can be sure that the obstacle just needs to be 
      // moved a bit.  That shouldn't be too hard to do...
      //
      if ( edit_level -> ItemList [ i ] . pos . x > from_where )
	edit_level -> ItemList [ i ] . pos . x += by_what ;
    }

  for ( i = 0 ; i < MAX_CHEST_ITEMS_PER_LEVEL ; i ++ )
    {
      //--------------------
      // Maybe the item entry isn't used at all.  That's the simplest
      // case...: do nothing.
      //
      if ( edit_level -> ChestItemList [ i ] . type <= ( -1 ) ) continue;
      if ( edit_level -> ChestItemList [ i ] . pos . x <= ( -1 ) ) continue;
      
      //--------------------
      // Maybe the item is right on the spot where it must be deleted
      // because the floor under it will move out.
      //
      if ( ( edit_level -> ChestItemList [ i ] . pos . x >= from_where ) &&
	   ( edit_level -> ChestItemList [ i ] . pos . x <= from_where - by_what ) )
	{
	  DeleteItem ( & ( edit_level -> ChestItemList [ i ] ) ) ;
	  DebugPrintf ( 0 , "\nRemoved another item in resizing operation." );
	  continue;
	}

      //--------------------
      // Now at this point we can be sure that the obstacle just needs to be 
      // moved a bit.  That shouldn't be too hard to do...
      //
      if ( edit_level -> ChestItemList [ i ] . pos . x > from_where )
	edit_level -> ChestItemList [ i ] . pos . x += by_what ;
    }

  glue_obstacles_to_floor_tiles_for_level ( edit_level -> levelnum ) ;
  
}; // void move_obstacles_and_items_east_of ( float from_where , float by_what , Level edit_level )

/* ----------------------------------------------------------------------
 *
 *
 * ---------------------------------------------------------------------- */
void
move_obstacles_and_items_south_of ( float from_where , float by_what , Level edit_level )
{
  int i;

  for ( i = 0 ; i < MAX_OBSTACLES_ON_MAP ; i ++ )
    {
      //--------------------
      // Maybe the obstacle entry isn't used at all.  That's the simplest
      // case...: do nothing.
      //
      if ( edit_level -> obstacle_list [ i ] . type <= ( -1 ) ) continue;
      // if ( edit_level -> obstacle_list [ i ] . pos . y <= ( -1 ) ) continue;
      
      //--------------------
      // Maybe the obstacle is right on the spot where it must be deleted
      // because the floor under it will move out.
      //
      if ( ( edit_level -> obstacle_list [ i ] . pos . y >= from_where ) &&
	   ( edit_level -> obstacle_list [ i ] . pos . y <= from_where - by_what ) )
	{
	  action_remove_obstacle ( edit_level , & ( edit_level -> obstacle_list [ i ] ) ) ;
	  i -- ;
	  DebugPrintf ( 0 , "\nRemoved another obstacle in resizing operation." );
	  continue;
	}

      //--------------------
      // Now at this point we can be sure that the obstacle just needs to be 
      // moved a bit.  That shouldn't be too hard to do...
      //
      if ( edit_level -> obstacle_list [ i ] . pos . y > from_where )
	edit_level -> obstacle_list [ i ] . pos . y += by_what ;
    }
  

  for ( i = 0 ; i < MAX_ITEMS_PER_LEVEL ; i ++ )
    {
      //--------------------
      // Maybe the item entry isn't used at all.  That's the simplest
      // case...: do nothing.
      //
      if ( edit_level -> ItemList [ i ] . type <= ( -1 ) ) continue;
      if ( edit_level -> ItemList [ i ] . pos . y <= ( -1 ) ) continue;
      
      //--------------------
      // Maybe the item is right on the spot where it must be deleted
      // because the floor under it will move out.
      //
      if ( ( edit_level -> ItemList [ i ] . pos . y >= from_where ) &&
	   ( edit_level -> ItemList [ i ] . pos . y <= from_where - by_what ) )
	{
	  DeleteItem ( & ( edit_level -> ItemList [ i ] ) ) ;
	  DebugPrintf ( 0 , "\nRemoved another item in resizing operation." );
	  continue;
	}

      //--------------------
      // Now at this point we can be sure that the obstacle just needs to be 
      // moved a bit.  That shouldn't be too hard to do...
      //
      if ( edit_level -> ItemList [ i ] . pos . y > from_where )
	edit_level -> ItemList [ i ] . pos . y += by_what ;
    }

  for ( i = 0 ; i < MAX_CHEST_ITEMS_PER_LEVEL ; i ++ )
    {
      //--------------------
      // Maybe the item entry isn't used at all.  That's the simplest
      // case...: do nothing.
      //
      if ( edit_level -> ChestItemList [ i ] . type <= ( -1 ) ) continue;
      if ( edit_level -> ChestItemList [ i ] . pos . y <= ( -1 ) ) continue;
      
      //--------------------
      // Maybe the item is right on the spot where it must be deleted
      // because the floor under it will move out.
      //
      if ( ( edit_level -> ChestItemList [ i ] . pos . y >= from_where ) &&
	   ( edit_level -> ChestItemList [ i ] . pos . y <= from_where - by_what ) )
	{
	  DeleteItem ( & ( edit_level -> ChestItemList [ i ] ) ) ;
	  DebugPrintf ( 0 , "\nRemoved another item in resizing operation." );
	  continue;
	}

      //--------------------
      // Now at this point we can be sure that the obstacle just needs to be 
      // moved a bit.  That shouldn't be too hard to do...
      //
      if ( edit_level -> ChestItemList [ i ] . pos . y > from_where )
	edit_level -> ChestItemList [ i ] . pos . y += by_what ;
    }

  glue_obstacles_to_floor_tiles_for_level ( edit_level -> levelnum ) ;
  
}; // void move_obstacles_south_of ( float from_where , float by_what , Level edit_level )

/* ----------------------------------------------------------------------
 * When new lines are inserted into the map, the map labels east of this
 * line must move too with the rest of the map.  This function sees to it.
 * ---------------------------------------------------------------------- */
void
MoveMapLabelsEastOf ( int FromWhere , int ByWhat , Level EditLevel )
{
  int i;

  for ( i = 0 ; i < MAX_MAP_LABELS_PER_LEVEL ; i ++ )
    {
      if ( EditLevel -> labels [ i ] . pos . x <= ( -1 ) ) continue;
      
      if ( EditLevel -> labels [ i ] . pos . x >= FromWhere )
	EditLevel -> labels [ i ] . pos . x += ByWhat;
    }
  
}; // void MoveMapLabelsEastOf ( int FromWhere , int ByWhat , Level EditLevel)

/* ----------------------------------------------------------------------
 * When new lines are inserted into the map, the waypoints south of this
 * line must move too with the rest of the map.  This function sees to it.
 * ---------------------------------------------------------------------- */
void
MoveWaypointsSouthOf ( int FromWhere , int ByWhat , Level EditLevel )
{
  int i;

  for ( i = 0 ; i < MAXWAYPOINTS ; i ++ )
    {
      if ( EditLevel -> AllWaypoints [ i ] . x == ( 0 ) ) continue;
      
      if ( EditLevel -> AllWaypoints [ i ] . y >= FromWhere )
	EditLevel -> AllWaypoints [ i ] . y += ByWhat;
    }
  
}; // void MoveWaypointsSouthOf ( int FromWhere , int ByWhat , Level EditLevel)

/* ----------------------------------------------------------------------
 * This function should associate the current mouse position with an
 * index in the level editor item drop screen.
 * (-1) is returned when cursor is not on any item in the item drop grid.
 * ---------------------------------------------------------------------- */
int
level_editor_item_drop_index ( int row_len , int line_len )
{
    if ( ( GetMousePos_x ( )  > 55 ) && ( GetMousePos_x ( )  < 55 + 64 * line_len * GameConfig . screen_width / 640 ) &&
	 ( GetMousePos_y ( )  > 32 ) && ( GetMousePos_y ( )  < 32 + 66 * row_len * GameConfig . screen_height / 480 ) )
	{
	    return (   ( GetMousePos_x()  - 55 ) / ( 64 * GameConfig . screen_width / 640 ) + 
		     ( ( GetMousePos_y()  - 32 ) / ( 66 * GameConfig . screen_height / 480 ) ) * line_len ) ;
	}

    //--------------------
    // If no level editor item grid index was found under the current
    // mouse cursor position, we just return (-1) to indicate that.
    //
    return ( -1 ) ;
    
}; // int level_editor_item_drop_index ( void )

/* ----------------------------------------------------------------------
 * This function drops an item onto the floor.  It works with a selection
 * of item images and clicking with the mouse on an item image or on one
 * of the buttons presented to the person editing the level.
 * ---------------------------------------------------------------------- */
void
ItemDropFromLevelEditor( void )
{
    int SelectionDone = FALSE;
    int NewItemCode = ( -1 );
    int i;
    int j;
    item temp_item;
    int row_len = 5 ;
    int line_len = 8 ; 
    int our_multiplicity = 1 ;
    int item_group = 0 ; 
    static int previous_mouse_position_index = (-1) ;
    static int previous_suffix_selected = (-1) ;
    static int previous_prefix_selected = (-1) ;
    
    while ( GPressed() ) SDL_Delay(1);
    while ( SpacePressed()  || MouseLeftPressed() ) SDL_Delay(1);
    
    while ( !SelectionDone )
    {
	track_last_frame_input_status();
	
	our_SDL_fill_rect_wrapper ( Screen , NULL , 0 );
	
	for ( j = 0 ; j < row_len ; j ++ )
	{
	    for ( i = 0 ; i < line_len ; i ++ ) 
	    {
		temp_item . type = i + j * line_len + item_group * line_len * row_len ;
		if ( temp_item.type >= Number_Of_Item_Types )  temp_item.type = 1 ;
		ShowRescaledItem ( i , 32 + (64*GameConfig.screen_height/480+2) * j, & ( temp_item ) );
	    }
	}
	
	ShowGenericButtonFromList ( LEVEL_EDITOR_NEXT_ITEM_GROUP_BUTTON );
	ShowGenericButtonFromList ( LEVEL_EDITOR_PREV_ITEM_GROUP_BUTTON );
	ShowGenericButtonFromList ( LEVEL_EDITOR_NEXT_PREFIX_BUTTON );
	ShowGenericButtonFromList ( LEVEL_EDITOR_PREV_PREFIX_BUTTON );
	ShowGenericButtonFromList ( LEVEL_EDITOR_NEXT_SUFFIX_BUTTON );
	ShowGenericButtonFromList ( LEVEL_EDITOR_PREV_SUFFIX_BUTTON );
	ShowGenericButtonFromList ( LEVEL_EDITOR_CANCEL_ITEM_DROP_BUTTON );

	if ( MouseCursorIsOnButton ( LEVEL_EDITOR_CANCEL_ITEM_DROP_BUTTON ,
                                             GetMousePos_x()  , GetMousePos_y()  ) )	    
	    PutStringFont ( Screen , FPS_Display_BFont , 20 , 440 * GameConfig . screen_height / 480 , _("Cancel item drop")) ;
	if ( level_editor_item_drop_index ( row_len , line_len ) != (-1) )
	{
	    previous_mouse_position_index = level_editor_item_drop_index ( row_len , line_len ) + 
		item_group * line_len * row_len ;
	    if ( previous_mouse_position_index >= Number_Of_Item_Types ) 
	    {
		previous_mouse_position_index = Number_Of_Item_Types - 1 ;
	    }
	    PutStringFont ( Screen , FPS_Display_BFont , 20 , 440 * GameConfig . screen_height / 480 , D_(ItemMap [ previous_mouse_position_index ] . item_name )) ;
	}

	if ( previous_prefix_selected != (-1) )
	{
	    PutStringFont ( Screen , FPS_Display_BFont , 300 * GameConfig . screen_width / 640 , 370 * GameConfig . screen_height / 480, 
			    PrefixList [ previous_prefix_selected ] . bonus_name ) ;
	}
	else
	{
	    PutStringFont ( Screen , FPS_Display_BFont , 300 * GameConfig . screen_width / 640 , 370 * GameConfig . screen_height / 480, 
			    "NO PREFIX" ) ;
	}

	if ( previous_suffix_selected != (-1) )
	{
	    PutStringFont ( Screen , FPS_Display_BFont , 300 * GameConfig . screen_width / 640 , 410 * GameConfig . screen_height / 480 , 
			    SuffixList [ previous_suffix_selected ] . bonus_name ) ;
	}
	else
	{
	    PutStringFont ( Screen , FPS_Display_BFont , 300 * GameConfig . screen_width / 640 , 410 * GameConfig . screen_height / 480 , 
			    "NO SUFFIX" ) ;
	}
	
	our_SDL_flip_wrapper( Screen );
	
	if ( EscapePressed() )
	{ //Pressing escape cancels the dropping
	    while ( EscapePressed() );
	    return ;
	}

	if ( SpacePressed() || MouseLeftClicked())
	{
	    if ( MouseCursorIsOnButton ( 
		     LEVEL_EDITOR_NEXT_ITEM_GROUP_BUTTON ,
		     GetMousePos_x()  , 
		     GetMousePos_y()  ) )
	    {
		item_group ++ ;
	    }
	    else if ( MouseCursorIsOnButton ( 
			  LEVEL_EDITOR_PREV_ITEM_GROUP_BUTTON ,
			  GetMousePos_x()  , 
			  GetMousePos_y()  ) )
	    {
		if ( item_group > 0 ) item_group -- ;
	    }

	    if ( MouseCursorIsOnButton ( 
		     LEVEL_EDITOR_NEXT_PREFIX_BUTTON ,
		     GetMousePos_x()  , 
		     GetMousePos_y()  ) )
	    {
		if ( PrefixList[ previous_prefix_selected + 1 ] . bonus_name != NULL )
		    previous_prefix_selected ++ ;
	    }
	    else if ( MouseCursorIsOnButton ( 
			  LEVEL_EDITOR_PREV_PREFIX_BUTTON ,
			  GetMousePos_x()  , 
			  GetMousePos_y()  ) )
	    {
		if ( previous_prefix_selected > (-1) )
		    previous_prefix_selected -- ;
	    }

	    if ( MouseCursorIsOnButton ( 
		     LEVEL_EDITOR_NEXT_SUFFIX_BUTTON ,
		     GetMousePos_x()  , 
		     GetMousePos_y()  ) )
	    {
		if ( SuffixList [ previous_suffix_selected + 1 ] . bonus_name != NULL )
		    previous_suffix_selected ++ ;
	    }
	    else if ( MouseCursorIsOnButton ( 
			  LEVEL_EDITOR_PREV_SUFFIX_BUTTON ,
			  GetMousePos_x()  , 
			  GetMousePos_y()  ) )
	    {
		if ( previous_suffix_selected > (-1) )
		    previous_suffix_selected -- ;
	    }
	    else if ( MouseCursorIsOnButton ( LEVEL_EDITOR_CANCEL_ITEM_DROP_BUTTON ,
					 GetMousePos_x()  , GetMousePos_y()  ) )
	    {
		return ;
	    }
	    else if ( level_editor_item_drop_index ( row_len , line_len ) != (-1) )
	    {
		SelectionDone = TRUE ;
		NewItemCode = level_editor_item_drop_index ( row_len , line_len ) + item_group * line_len * row_len ;
		if ( NewItemCode < 0 ) NewItemCode = 0 ; // just if the mouse has moved away in that little time...
	    }
	}
    }
    
    if ( NewItemCode >= Number_Of_Item_Types ) 
    {
	NewItemCode=0;
    }
    
    
    if ( ItemMap [ NewItemCode ] . item_group_together_in_inventory )
    {
	our_multiplicity = do_graphical_number_selection_in_range ( 1 , (!MatchItemWithName(NewItemCode, "Cyberbucks")) ? 100 : 1000, 1 );
	if ( our_multiplicity == 0 ) our_multiplicity = 1;
    }
    DropItemAt( NewItemCode , Me . pos . z , rintf( Me.pos.x ) , rintf( Me.pos.y ) , 
		previous_prefix_selected , previous_suffix_selected , our_multiplicity );
    
    while ( SpacePressed() || MouseLeftPressed() ) SDL_Delay(1);
    
}; // void ItemDropFromLevelEditor( void )

/* ----------------------------------------------------------------------
 * This function shall determine, whether a given left mouse click was in 
 * given rect or not.
 * ---------------------------------------------------------------------- */
int
ClickWasInRect ( SDL_Rect TargetRect )
{
    if ( GetMousePos_x()  > TargetRect.x + TargetRect.w ) return FALSE;
    if ( GetMousePos_x()  < TargetRect.x ) return FALSE;
    if ( GetMousePos_y()  > TargetRect.y + TargetRect.h ) return FALSE;
    if ( GetMousePos_y()  < TargetRect.y ) return FALSE;
    
    return ( TRUE );
}; // int ClickWasInRect ( SDL_Rect TargetRect )

/* ----------------------------------------------------------------------
 *
 *
 * ---------------------------------------------------------------------- */
int 
ClickWasInEditorBannerRect( void )
{
    return ( ClickWasInRect ( EditorBannerRect ) );
}; // int ClickWasInEditorBannerRect( void )

/* ----------------------------------------------------------------------
 *
 *
 * ---------------------------------------------------------------------- */
void
update_number_of_walls ( void )
{
    int inside_index ;
    int group_index ;
    
    for ( group_index = 0 ; group_index < NUMBER_OF_LEVEL_EDITOR_GROUPS ; group_index ++ )
    {
	for ( inside_index = 0 ; inside_index < NUMBER_OF_OBSTACLE_TYPES ; inside_index ++ )
	{
	    switch ( group_index )
	    {
		case LEVEL_EDITOR_SELECTION_FLOOR:
		    number_of_walls [ group_index ] = ALL_ISOMETRIC_FLOOR_TILES ;
		    if ( inside_index < ALL_ISOMETRIC_FLOOR_TILES )
			wall_indices [ group_index ] [ inside_index ] = inside_index ;
		    else
			wall_indices [ group_index ] [ inside_index ] = (-1);
		    break;
		case LEVEL_EDITOR_SELECTION_WALLS:
		case LEVEL_EDITOR_SELECTION_MACHINERY:
		case LEVEL_EDITOR_SELECTION_FURNITURE:
		case LEVEL_EDITOR_SELECTION_CONTAINERS:
		case LEVEL_EDITOR_SELECTION_PLANTS:
		    if ( wall_indices [ group_index ] [ inside_index ] == (-1) )
		    {
			number_of_walls [ group_index ] = inside_index ;
			inside_index = NUMBER_OF_OBSTACLE_TYPES ; // --> we MUST leave the loop here!
			break;
		    }
		    break;
		case LEVEL_EDITOR_SELECTION_ALL:
		    //--------------------
		    // In this case we have to fill the array with data, cause it's
		    // not hard-coded for this group...
		    //
		    number_of_walls [ group_index ] = NUMBER_OF_OBSTACLE_TYPES ;
		    if ( inside_index < NUMBER_OF_OBSTACLE_TYPES )
			wall_indices [ group_index ] [ inside_index ] = inside_index ;
		    else
			wall_indices [ group_index ] [ inside_index ] = (-1);
		    break;
		case LEVEL_EDITOR_SELECTION_QUICK:
		    break;
		default:
		    ErrorMessage ( __FUNCTION__  , "\
Unhandled level editor edit mode received.",
					       PLEASE_INFORM , IS_FATAL );
		    break;
	    }
	}
    }
    
}; // void update_number_of_walls ( void )

/* ----------------------------------------------------------------------
 *
 *
 * ---------------------------------------------------------------------- */
void
HandleBannerMouseClick( void )
{
    SDL_Rect TargetRect;
    int i;
    
    if ( MouseCursorIsOnButton ( LEFT_LEVEL_EDITOR_BUTTON , GetMousePos_x()  , GetMousePos_y()  ))
    {
	FirstBlock-= 8;
	DebugPrintf ( 1 , "\nBlocks should be scrolling now, if appropriate..." );
    }
    else if ( MouseCursorIsOnButton ( RIGHT_LEVEL_EDITOR_BUTTON , GetMousePos_x()  , GetMousePos_y()  ))
    {
	FirstBlock+=8 ;
	DebugPrintf ( 1 , "\nBlocks should be scrolling now, if appropriate..." );
    }
    else if( MouseCursorIsOnButton(  LEVEL_EDITOR_FLOOR_TAB, GetMousePos_x()  , GetMousePos_y()  ))
    {
	GameConfig . level_editor_edit_mode = LEVEL_EDITOR_SELECTION_FLOOR ;
    }
    else if( MouseCursorIsOnButton(  LEVEL_EDITOR_WALLS_TAB, GetMousePos_x()  , GetMousePos_y()  ))
    {
	GameConfig . level_editor_edit_mode = LEVEL_EDITOR_SELECTION_FLOOR + 1;
    }
    else if( MouseCursorIsOnButton(  LEVEL_EDITOR_MACHINERY_TAB, GetMousePos_x()  , GetMousePos_y()  ))
    {
	GameConfig . level_editor_edit_mode = LEVEL_EDITOR_SELECTION_FLOOR + 2 ;
    }
    else if( MouseCursorIsOnButton(  LEVEL_EDITOR_FURNITURE_TAB, GetMousePos_x()  , GetMousePos_y()  ))
    {
	GameConfig . level_editor_edit_mode = LEVEL_EDITOR_SELECTION_FLOOR + 3;
    }
    else if( MouseCursorIsOnButton(  LEVEL_EDITOR_CONTAINERS_TAB, GetMousePos_x()  , GetMousePos_y()  ))
    {
	GameConfig . level_editor_edit_mode = LEVEL_EDITOR_SELECTION_FLOOR + 4;
    }
    else if( MouseCursorIsOnButton(  LEVEL_EDITOR_PLANTS_TAB, GetMousePos_x()  , GetMousePos_y()  ))
    {
	GameConfig . level_editor_edit_mode = LEVEL_EDITOR_SELECTION_FLOOR + 5;
    }
    else if( MouseCursorIsOnButton(  LEVEL_EDITOR_ALL_TAB, GetMousePos_x()  , GetMousePos_y()  ))
    {
	GameConfig . level_editor_edit_mode = LEVEL_EDITOR_SELECTION_FLOOR + 6;
    }
    else if( MouseCursorIsOnButton(  LEVEL_EDITOR_QUICK_TAB, GetMousePos_x()  , GetMousePos_y()  ))
    {
	GameConfig . level_editor_edit_mode = LEVEL_EDITOR_SELECTION_FLOOR + 7;
    }
    else
    {
	// could be a click on a block
        unsigned int num_blocks = GameConfig.screen_width / INITIAL_BLOCK_WIDTH - 1;
	for ( i = 0 ; i < num_blocks ; i ++ ) 
        {
	    TargetRect.x = INITIAL_BLOCK_WIDTH/2 + INITIAL_BLOCK_WIDTH * i; 
	    TargetRect.y = INITIAL_BLOCK_HEIGHT/3;
	    TargetRect.w = INITIAL_BLOCK_WIDTH;
	    TargetRect.h = INITIAL_BLOCK_HEIGHT;
	    if ( ClickWasInRect ( TargetRect ) )
		Highlight = FirstBlock + i;
        }
    }
    
    // check limits
    if ( FirstBlock + 9 >= number_of_walls [ GameConfig . level_editor_edit_mode ] )
	FirstBlock = number_of_walls [ GameConfig . level_editor_edit_mode ] - 9 ;
    
    if ( FirstBlock < 0 )
	FirstBlock = 0;
    
    //--------------------
    // Now some extra security against selecting indices that would point to
    // undefined objects (floor tiles or obstacles) later
    // The following should never occur now - SN
    //
    if ( Highlight >= number_of_walls [ GameConfig . level_editor_edit_mode ] )
	Highlight = number_of_walls [ GameConfig . level_editor_edit_mode ] -1 ;
    
}; // void HandleBannerMouseClick( void )

/* ----------------------------------------------------------------------
 * On the very top of the screen during level editor work, there is a 
 * scrollbar with the various tiles that may be placed on the floor of the
 * map.  This scrollbar is drawn here.
 * ---------------------------------------------------------------------- */

void build_level_editor_banner(unsigned int selected_tab){
    char *tab_text[9]={
        _("FLOOR") ,     
        _("WALLS") ,     
        _("MACHINERY") , 
        _("FURNITURE") , 
        _("CONTAINERS"), 
        _("NATURE")    , 
        _("ALL") ,       
        _("QUICK"),
        NULL
    };
    unsigned int tab_num=8;

    SDL_Rect tr,hr;
    
    // here I modify default values 
    AllMousePressButtons[ RIGHT_LEVEL_EDITOR_BUTTON ] . button_rect . x = 
        GameConfig.screen_width - 16;
    EditorBannerRect.w = GameConfig.screen_width;

    // object selector background
    tr . x = 0 ;
    tr . w = GameConfig.screen_width;
    tr . y = 14 ;
    tr . h = 77;
    our_SDL_fill_rect_wrapper(Screen, &tr, 0x556889);

    ShowGenericButtonFromList (  LEFT_LEVEL_EDITOR_BUTTON );
    ShowGenericButtonFromList ( RIGHT_LEVEL_EDITOR_BUTTON );

    // object selector tabset background
    tr.x = 0; tr . y = 0; tr . h = 14; tr . w = 640;
    our_SDL_fill_rect_wrapper(Screen, &tr, 0x656565);

    // tabset
    BFont_Info * PreviousFont;
    PreviousFont = GetCurrentFont();
    SetCurrentFont( Message_BFont );
    unsigned int tab_y=1, tab_width = 80, ii;
    tr . w = 2;
    tr . h = 14;
    hr . y =0 ; hr.w = tab_width-2; hr.h = 14;

    for(ii=0;ii<tab_num;ii++){
        unsigned int tab_x = ii * tab_width;
        hr.x=tab_x;
        if(ii==selected_tab) // the currently selected tab has different color
           our_SDL_fill_rect_wrapper(Screen,&hr,0x556889);
        DisplayText( tab_text[ii] , tab_x+2 , tab_y , &hr , TEXT_STRETCH );
        tr.x = tab_x + tab_width - 2;
        // tab separators
        our_SDL_fill_rect_wrapper(Screen,&tr,0x88000000);
    }
    SetCurrentFont( PreviousFont );
}

void
ShowLevelEditorTopMenu( int Highlight )
{
    int i;
    SDL_Rect TargetRectangle;
    int selected_index = FirstBlock;
    int placing_floor;
    SDL_Surface *tmp = NULL;
    float zoom_factor;

    build_level_editor_banner(GameConfig.level_editor_edit_mode);

    //--------------------
    // Time to fill something into the top selection banner, so that the
    // user can really has something to select from there.  But this must be
    // done differently, depending on whether we show the menu for the floor
    // edit mode or for the obstacle edit mode.
    //
    unsigned int num_blocks = GameConfig.screen_width / INITIAL_BLOCK_WIDTH - 1;
    for ( i = 0 ; i < num_blocks ; i ++ )
    {
	if ( selected_index >= number_of_walls [ GameConfig . level_editor_edit_mode ] ) 
	    break;

	TargetRectangle.x = INITIAL_BLOCK_WIDTH/2 + INITIAL_BLOCK_WIDTH * i ;
	TargetRectangle.y = INITIAL_BLOCK_HEIGHT/3 ;
	TargetRectangle.w = INITIAL_BLOCK_WIDTH ;
	TargetRectangle.h = INITIAL_BLOCK_HEIGHT ;
	    
        iso_image * img=NULL;
        int y_off;
        y_off=TargetRectangle.y;
	placing_floor = FALSE;
	switch ( GameConfig . level_editor_edit_mode )
	{
	    case LEVEL_EDITOR_SELECTION_FLOOR:
		img = & (floor_iso_images [ selected_index ]);
		placing_floor = TRUE;
		break;
	    case LEVEL_EDITOR_SELECTION_WALLS:
	    case LEVEL_EDITOR_SELECTION_MACHINERY:
	    case LEVEL_EDITOR_SELECTION_FURNITURE:
	    case LEVEL_EDITOR_SELECTION_CONTAINERS:
	    case LEVEL_EDITOR_SELECTION_PLANTS:
	    case LEVEL_EDITOR_SELECTION_ALL:
                img = &(obstacle_map [wall_indices [ GameConfig . level_editor_edit_mode ] [ selected_index ] ] . image);
                break;
	    case LEVEL_EDITOR_SELECTION_QUICK:
		img = quickbar_getimage ( selected_index, &placing_floor );
		break;
	    default:
		ErrorMessage ( __FUNCTION__  , 
					   "Unhandled level editor edit mode received.",
					   PLEASE_INFORM , IS_FATAL );
		break;
        }
	if (!img) break; 
	// We find the proper zoom_factor, so that the obstacle/tile in question will
		// fit into one tile in the level editor top status selection row.
		//
		if ( use_open_gl )
		{
		    zoom_factor = min ( 
		( (float)INITIAL_BLOCK_WIDTH / (float)img -> original_image_width ) ,
		( (float)INITIAL_BLOCK_HEIGHT / (float)img -> original_image_height ) );
		}
		else
		{
		    zoom_factor = min ( 
		( (float)INITIAL_BLOCK_WIDTH / (float)img -> surface->w ) ,
		( (float)INITIAL_BLOCK_HEIGHT / (float)img -> surface->h ) );
		}

        if( placing_floor )
                y_off = TargetRectangle . y + 0.75 * TargetRectangle.h - 
                           zoom_factor * (float)(img -> original_image_height);

		if ( use_open_gl )
		{
		if ( placing_floor )
		    draw_gl_scaled_quad_from_atlas_at_screen_position ( img, &floor_atlas [ selected_index ], TargetRectangle.x, y_off, zoom_factor);
		else
		    draw_gl_scaled_textured_quad_at_screen_position ( img , TargetRectangle . x , 
			    y_off, zoom_factor) ;
		//additionally in the ALL tab, display object number
		if ( GameConfig . level_editor_edit_mode == LEVEL_EDITOR_SELECTION_ALL)
			{
			char obsnum[5];
			BFont_Info * PreviousFont = GetCurrentFont ();
		        SetCurrentFont (Message_BFont);
			sprintf(obsnum, "%d", wall_indices [ GameConfig . level_editor_edit_mode ] [ selected_index ] );
			DisplayText(obsnum, TargetRectangle . x, y_off, NULL, 1);
			SetCurrentFont (PreviousFont);
			}
		}
		else
		{
		    //--------------------
	    // We create a scaled version of the obstacle/floorpiece in question
		    //
	    tmp = zoomSurface ( img -> surface , zoom_factor, zoom_factor, FALSE );
		    
		    //--------------------
		    // Now we can show and free the scaled verion of the floor tile again.
		    //
		    our_SDL_blit_surface_wrapper( tmp , NULL , Screen, &TargetRectangle);
		    SDL_FreeSurface ( tmp );
		}

	//--------------------
	// Maybe we've just displayed the obstacle/floorpiece that is currently
	// selected.  In this case we should also draw the marker right on it.
	//
	if ( selected_index == Highlight ) 
	    HighlightRectangle ( Screen , TargetRectangle );
	  
	//--------------------
	// We can proceed here, since 'out of bounds' checks are done
	// above anyway.
	//
	selected_index ++ ;
    }
    
}; // void ShowLevelEditorTopMenu( void )

/* ----------------------------------------------------------------------
 * When new lines are inserted into the map, the waypoints east of this
 * line must move too with the rest of the map.  This function sees to it.
 * ---------------------------------------------------------------------- */
void
MoveWaypointsEastOf ( int FromWhere , int ByWhat , Level EditLevel )
{
  int i;

  for ( i = 0 ; i < MAXWAYPOINTS ; i ++ )
    {
      if ( EditLevel -> AllWaypoints [ i ] . x == ( 0 ) ) continue;
      
      if ( EditLevel -> AllWaypoints [ i ] . x >= FromWhere )
	EditLevel -> AllWaypoints [ i ] . x += ByWhat;
    }
  
}; // void MoveWaypointsEastOf ( int FromWhere , int ByWhat , Level EditLevel)

/* ----------------------------------------------------------------------
 * Self-explanatory.
 * ---------------------------------------------------------------------- */
void
InsertLineVerySouth ( Level EditLevel )
{
  int i;
  int j;

  //--------------------
  // The enlargement of levels in y direction is limited by a constant
  // defined in defs.h.  This is carefully checked or no operation at
  // all will be performed.
  //
  if ( (EditLevel->ylen)+1 < MAX_MAP_LINES )
    {
      EditLevel->ylen++;
      // In case of enlargement, we need to do more:
      EditLevel->map[ EditLevel->ylen-1 ] = MyMalloc( ( EditLevel->xlen + 1 ) * sizeof ( map_tile ) ) ;
      // We don't want to fill the new area with junk, do we? So we make it floor tiles

      //--------------------
      // Now we insert the new line.  But we can't just initialize it with memset like
      // earlier, but instead this has to be done with more care, using the map_tile
      // structures.
      //
      //memset( EditLevel->map[ EditLevel->ylen-1 ] , FLOOR , EditLevel->xlen );
      for ( i = 0 ; i < EditLevel->xlen ; i ++ )
	{
	  EditLevel->map[ EditLevel->ylen-1 ] [ i ] . floor_value = ISO_FLOOR_SAND ;
	  for ( j = 0 ; j < MAX_OBSTACLES_GLUED_TO_ONE_MAP_TILE ; j ++ )
	    {
	      EditLevel->map[ EditLevel->ylen-1 ] [ i ] . obstacles_glued_to_here [ j ] = (-1) ;
	    }
	}
    }

}; // void InsertLineVerySouth ( Level EditLevel )

/* ----------------------------------------------------------------------
 * Self-explanatory.
 * ---------------------------------------------------------------------- */
void
InsertColumnVeryEast ( Level EditLevel )
{
  int i;
  map_tile* OldMapPointer;

  EditLevel->xlen++;
  // In case of enlargement, we need to do more:
  for ( i = 0 ; i < EditLevel->ylen ; i++ )
    {
      OldMapPointer=EditLevel->map[i];
      EditLevel->map[i] = MyMalloc ( sizeof ( map_tile ) * ( EditLevel->xlen +1) ) ;
      memcpy( EditLevel -> map [ i ] , OldMapPointer , ( EditLevel -> xlen - 1 ) * sizeof ( map_tile ) );
      // We don't want to fill the new area with junk, do we? So we make it floor tiles
      EditLevel->map[ i ] [ EditLevel->xlen-1 ] . floor_value = FLOOR;  
    }

}; // void InsertColumnVeryEast ( Level EditLevel )
      
/* ----------------------------------------------------------------------
 * Self-explanatory.
 * ---------------------------------------------------------------------- */
void
InsertColumnEasternInterface( Level EditLevel )
{
  int i;

  //--------------------
  // First a sanity check:  If there's no eastern threshold, this
  // must be a mistake and will just be ignored...
  //
  if ( EditLevel -> jump_threshold_east <= 0 ) return;

  //--------------------
  // We use availabel methods to add a column, even if in the wrong
  // place for now.
  //
  InsertColumnVeryEast ( EditLevel );

  //--------------------
  // Now the new memory and everything is done.  All we
  // need to do is move the information to the east
  //
  for ( i = 0 ; i < EditLevel->ylen ; i ++ )
    {
      //--------------------
      // REMEMBER:  WE MUST NO USE MEMCPY HERE, CAUSE THE AREAS IN QUESTION
      // MIGHT (EVEN WILL) OVERLAP!!  THAT MUST NOT BE THE CASE WITH MEMCPY!!
      //
      memmove ( & ( EditLevel->map [ i ] [ EditLevel->xlen - EditLevel->jump_threshold_east - 1 ] ) ,
		& ( EditLevel->map [ i ] [ EditLevel->xlen - EditLevel->jump_threshold_east - 2 ] ) ,
		EditLevel->jump_threshold_east * sizeof ( map_tile ) );
      EditLevel->map [ i ] [ EditLevel->xlen - EditLevel->jump_threshold_east - 1 ] . floor_value = FLOOR ;
    }

  MoveWaypointsEastOf ( EditLevel->xlen - EditLevel->jump_threshold_east - 1 , +1 , EditLevel ) ;
  MoveMapLabelsEastOf ( EditLevel->xlen - EditLevel->jump_threshold_east - 1 , +1 , EditLevel ) ;
  move_obstacles_east_of ( EditLevel->xlen - EditLevel->jump_threshold_east - 1.0 , +1 , EditLevel ) ;

}; // void InsertColumnEasternInterface( EditLevel );

/* ----------------------------------------------------------------------
 * Self-explanatory.
 * ---------------------------------------------------------------------- */
void
RemoveColumnEasternInterface( Level EditLevel )
{
  int i;

  //--------------------
  // First a sanity check:  If there's no eastern threshold, this
  // must be a mistake and will just be ignored...
  //
  if ( EditLevel -> jump_threshold_east <= 0 ) return;

  //--------------------
  // First we move the obstacles, cause they will be glued and moved and doing that should
  // be done before the floor to glue them to vanishes in the very east.
  //
  // But of course we should glue once more later...
  //
  move_obstacles_east_of ( EditLevel->xlen - EditLevel->jump_threshold_east + 1.0 , -1 , EditLevel ) ;

  //--------------------
  // Now the new memory and everything is done.  All we
  // need to do is move the information to the east
  //
  for ( i = 0 ; i < EditLevel->ylen ; i ++ )
    {
      //--------------------
      // REMEMBER:  WE MUST NO USE MEMCPY HERE, CAUSE THE AREAS IN QUESTION
      // MIGHT (EVEN WILL) OVERLAP!!  THAT MUST NOT BE THE CASE WITH MEMCPY!!
      //
      memmove ( & ( EditLevel->map [ i ] [ EditLevel->xlen - EditLevel->jump_threshold_east - 1 ] ) ,
		& ( EditLevel->map [ i ] [ EditLevel->xlen - EditLevel->jump_threshold_east - 0 ] ) ,
		EditLevel->jump_threshold_east * sizeof ( map_tile ) );
      // EditLevel->map [ i ] [ EditLevel->xlen - EditLevel->jump_threshold_east - 1 ] = FLOOR ;
    }

  EditLevel -> xlen --;

  MoveWaypointsEastOf ( EditLevel->xlen - EditLevel->jump_threshold_east + 1 , -1 , EditLevel ) ;
  MoveMapLabelsEastOf ( EditLevel->xlen - EditLevel->jump_threshold_east + 1 , -1 , EditLevel ) ;

  glue_obstacles_to_floor_tiles_for_level ( EditLevel -> levelnum );

}; // void RemoveColumnEasternInterface( Level EditLevel );

/* ----------------------------------------------------------------------
 * Self-explanatory.
 * ---------------------------------------------------------------------- */
void
InsertColumnWesternInterface( Level EditLevel )
{
  int BackupOfEasternInterface;

  //--------------------
  // First a sanity check:  If there's no western threshold, this
  // must be a mistake and will just be ignored...
  //
  if ( EditLevel -> jump_threshold_west <= 0 ) return;

  //--------------------
  // Again we exploit existing code, namely the insertion procedure
  // for the eastern interface.  We shortly change the interface, use
  // that code from the eastern interface and restore the eastern interface.
  //
  BackupOfEasternInterface = EditLevel->jump_threshold_east;
  EditLevel->jump_threshold_east = EditLevel->xlen - EditLevel->jump_threshold_west ;
  InsertColumnEasternInterface ( EditLevel );
  EditLevel->jump_threshold_east = BackupOfEasternInterface ;

}; // void InsertColumnWesternInterface( Level EditLevel )

/* ----------------------------------------------------------------------
 * Self-explanatory.
 * ---------------------------------------------------------------------- */
void
RemoveColumnWesternInterface( Level EditLevel )
{
  int BackupOfEasternInterface;

  //--------------------
  // First a sanity check:  If there's no western threshold, this
  // must be a mistake and will just be ignored...
  //
  if ( EditLevel -> jump_threshold_west <= 0 ) return;

  //--------------------
  // Again we exploit existing code, namely the insertion procedure
  // for the eastern interface.  We shortly change the interface, use
  // that code from the eastern interface and restore the eastern interface.
  //
  BackupOfEasternInterface = EditLevel->jump_threshold_east;
  EditLevel->jump_threshold_east = EditLevel->xlen - EditLevel->jump_threshold_west - 1;
  RemoveColumnEasternInterface ( EditLevel );
  EditLevel->jump_threshold_east = BackupOfEasternInterface ;

}; // void RemoveColumnWesternInterface( Level EditLevel )

/* ----------------------------------------------------------------------
 * Self-Explanatory.
 * ---------------------------------------------------------------------- */
void
InsertColumnVeryWest ( Level EditLevel )
{
  int OldEasternInterface;

  //--------------------
  // We shortly change the eastern interface to reuse the code for there
  //
  OldEasternInterface = EditLevel -> jump_threshold_south;

  EditLevel -> jump_threshold_east = EditLevel -> xlen - 0 ;
  InsertColumnEasternInterface ( EditLevel );

  EditLevel -> jump_threshold_east = OldEasternInterface ;

}; // void InsertColumnVeryWest ( EditLevel )

/* ----------------------------------------------------------------------
 * Self-Explanatory.
 * ---------------------------------------------------------------------- */
void
RemoveColumnVeryWest ( Level EditLevel )
{
  int OldEasternInterface;

  //--------------------
  // We shortly change the eastern interface to reuse the code for there
  //
  OldEasternInterface = EditLevel -> jump_threshold_east;

  EditLevel -> jump_threshold_east = EditLevel -> xlen - 1 ;
  RemoveColumnEasternInterface ( EditLevel );

  EditLevel -> jump_threshold_east = OldEasternInterface ;

}; // void RemoveColumnVeryEast ( Level EditLevel )

/* ----------------------------------------------------------------------
 * Self-Explanatory.
 * ---------------------------------------------------------------------- */
void
InsertLineSouthernInterface ( Level EditLevel )
{
  map_tile* temp;
  int i;

  //--------------------
  // First a sanity check for existing interface
  //
  if ( EditLevel -> jump_threshold_south <= 0 ) return;

  //--------------------
  // We build upon the existing code again.
  //
  InsertLineVerySouth( EditLevel );
  
  //--------------------
  // Now we do some swapping of lines
  //
  temp = EditLevel -> map [ EditLevel -> ylen - 1 ] ;

  for ( i = 0 ; i < EditLevel -> jump_threshold_south ; i ++ )
    {
      EditLevel -> map [ EditLevel -> ylen - i - 1 ] = 
	EditLevel -> map [ EditLevel -> ylen - i - 2 ] ;
    }
  EditLevel -> map [ EditLevel -> ylen - 1 - EditLevel -> jump_threshold_south ] = temp ;

  //--------------------
  // Now we have the waypoints moved as well
  //
  MoveWaypointsSouthOf ( EditLevel -> ylen - 1 - EditLevel -> jump_threshold_south , +1 , EditLevel ) ;
  MoveMapLabelsSouthOf ( EditLevel -> ylen - 1 - EditLevel -> jump_threshold_south , +1 , EditLevel ) ;
  move_obstacles_and_items_south_of ( EditLevel -> ylen - 1 - EditLevel -> jump_threshold_south , +1 , EditLevel ) ;

  glue_obstacles_to_floor_tiles_for_level ( EditLevel -> levelnum );

}; // void InsertLineSouthernInterface ( EditLevel )

/* ----------------------------------------------------------------------
 * Self-Explanatory.
 * ---------------------------------------------------------------------- */
void
RemoveLineSouthernInterface ( Level EditLevel )
{
  int i;

  //--------------------
  // First a sanity check for existing interface
  //
  if ( EditLevel -> jump_threshold_south <= 0 ) return;

  //--------------------
  // First we move the obstacles, cause they will be glued and moved and doing that should
  // be done before the floor to glue them to vanishes in the very south.
  //
  // But of course we should glue once more later...
  //
  move_obstacles_and_items_south_of ( EditLevel -> ylen - 0 - EditLevel -> jump_threshold_south , -1 , EditLevel ) ;

  //--------------------
  // Now we do some swapping of lines
  //
  for ( i = EditLevel -> ylen - 1 - EditLevel -> jump_threshold_south ; 
	i < EditLevel -> ylen - 1 ; i ++ )
    {
      EditLevel -> map [ i ] = EditLevel -> map [ i + 1 ] ;
    }
  EditLevel -> ylen -- ;

  //--------------------
  // Now we have the waypoints moved as well
  //
  MoveWaypointsSouthOf ( EditLevel -> ylen - 0 - EditLevel -> jump_threshold_south , -1 , EditLevel ) ;
  MoveMapLabelsSouthOf ( EditLevel -> ylen - 0 - EditLevel -> jump_threshold_south , -1 , EditLevel ) ;

  glue_obstacles_to_floor_tiles_for_level ( EditLevel -> levelnum );

}; // void RemoveLineSouthernInterface ( EditLevel )

/* ----------------------------------------------------------------------
 * Self-Explanatory.
 * ---------------------------------------------------------------------- */
void
InsertLineNorthernInterface ( Level EditLevel )
{
  int OldSouthernInterface;

  //--------------------
  // First a sanity check for existing interface
  //
  if ( EditLevel -> jump_threshold_north <= 0 ) return;

  //--------------------
  // We shortly change the southern interface to reuse the code for there
  //
  OldSouthernInterface = EditLevel -> jump_threshold_south;

  EditLevel -> jump_threshold_south = EditLevel -> ylen - EditLevel -> jump_threshold_north - 0 ;
  InsertLineSouthernInterface ( EditLevel );

  EditLevel -> jump_threshold_south = OldSouthernInterface ;

}; // void InsertLineNorthernInterface ( EditLevel )

/* ----------------------------------------------------------------------
 * Self-Explanatory.
 * ---------------------------------------------------------------------- */
void
RemoveLineNorthernInterface ( Level EditLevel )
{
  int OldSouthernInterface;

  //--------------------
  // First a sanity check for existing interface
  //
  if ( EditLevel -> jump_threshold_north <= 0 ) return;

  //--------------------
  // We shortly change the southern interface to reuse the code for there
  //
  OldSouthernInterface = EditLevel -> jump_threshold_south;

  EditLevel -> jump_threshold_south = EditLevel -> ylen - EditLevel -> jump_threshold_north - 1 ;
  RemoveLineSouthernInterface ( EditLevel );

  EditLevel -> jump_threshold_south = OldSouthernInterface ;

}; // void RemoveLineNorthernInterface ( Level EditLevel )

/* ----------------------------------------------------------------------
 * Self-Explanatory.
 * ---------------------------------------------------------------------- */
void
InsertLineVeryNorth ( Level EditLevel )
{
  int OldSouthernInterface;

  //--------------------
  // We shortly change the southern interface to reuse the code for there
  //
  OldSouthernInterface = EditLevel -> jump_threshold_south;

  EditLevel -> jump_threshold_south = EditLevel -> ylen - 0 ;
  InsertLineSouthernInterface ( EditLevel );

  EditLevel -> jump_threshold_south = OldSouthernInterface ;

}; // void InsertLineVeryNorth ( EditLevel )

/* ----------------------------------------------------------------------
 * Self-Explanatory.
 * ---------------------------------------------------------------------- */
void
RemoveLineVeryNorth ( Level EditLevel )
{
  int OldSouthernInterface;

  //--------------------
  // We shortly change the southern interface to reuse the code for there
  //
  OldSouthernInterface = EditLevel -> jump_threshold_south;

  EditLevel -> jump_threshold_south = EditLevel -> ylen - 1 ;
  RemoveLineSouthernInterface ( EditLevel );

  EditLevel -> jump_threshold_south = OldSouthernInterface ;

}; // void RemoveLineVeryNorth ( Level EditLevel )

/* ----------------------------------------------------------------------
 * This a a menu interface to allow to edit the level dimensions in a
 * convenient way, i.e. so that little stupid copying work or things like
 * that have to be done and more time can be spent creating game material.
 * ---------------------------------------------------------------------- */
void
EditLevelDimensions ( void )
{
  char* MenuTexts[ 20 ];
  char Options [ 20 ] [1000];
  int MenuPosition = 1 ;
  int proceed_now = FALSE ;
  Level EditLevel;

  EditLevel = curShip.AllLevels [ Me . pos . z ] ;

  while ( !proceed_now )
    {

      InitiateMenu( -1 );
      
      MenuTexts[ 0 ] = "Insert/Remove column to the very west" ;
      MenuTexts[ 1 ] = "Insert/Remove column just east of western Interface" ;
      MenuTexts[ 2 ] = "Insert/Remove column just west of eastern Interface" ;
      MenuTexts[ 3 ] = "Insert/Remove column to the very east" ;

      MenuTexts[ 4 ] = "Insert/Remove line to the very north" ;
      MenuTexts[ 5 ] = "Insert/Remove line just south of northern Interface" ;
      MenuTexts[ 6 ] = "Insert/Remove line just north of southern Interface" ;
      MenuTexts[ 7 ] = "Insert/Remove line to the very south" ;
      
      sprintf( Options [ 0 ] , "Current level size in X: %d." , EditLevel->xlen );
      MenuTexts[ 8 ] = Options [ 0 ];
      sprintf( Options [ 1 ] , "Current level size in Y: %d." , EditLevel->ylen  );
      MenuTexts[ 9 ] = Options [ 1 ] ;

      MenuTexts[ 10 ] = "Back To Level Editor Main Menu" ;
      MenuTexts[ 11 ] = "" ;
      
      MenuPosition = DoMenuSelection( "" , MenuTexts , -1 , -1 , FPS_Display_BFont );
      
      while (EnterPressed() || SpacePressed() || MouseLeftPressed() ) SDL_Delay(1);
      
      switch (MenuPosition) 
	{
	case INSERTREMOVE_COLUMN_VERY_EAST:
	  if ( RightPressed() )
	    {
	      InsertColumnVeryEast( EditLevel );
	      while (RightPressed());
	    }
	  if ( LeftPressed() )
	    {
	      EditLevel->xlen--; // making it smaller is always easy:  just modify the value for size
	      // allocation of new memory or things like that are not nescessary
	      while (LeftPressed());
	    }
	  break;

	case INSERTREMOVE_COLUMN_EASTERN_INTERFACE:
	  if ( RightPressed() )
	    {
	      InsertColumnEasternInterface( EditLevel );
	      while (RightPressed());
	    }
	  if ( LeftPressed() )
	    {
	      RemoveColumnEasternInterface( EditLevel );
	      while (LeftPressed());
	    }
	  break;

	case INSERTREMOVE_COLUMN_WESTERN_INTERFACE:
	  if ( RightPressed() )
	    {
	      InsertColumnWesternInterface( EditLevel );
	      while (RightPressed());
	    }
	  if ( LeftPressed() )
	    {
	      RemoveColumnWesternInterface( EditLevel );
	      while (LeftPressed());
	    }
	  break;
	  
	case INSERTREMOVE_COLUMN_VERY_WEST:
	  if ( RightPressed() )
	    {
	      InsertColumnVeryWest ( EditLevel );
	      while (RightPressed());
	    }
	  if ( LeftPressed() )
	    {
	      RemoveColumnVeryWest ( EditLevel );
	      while (LeftPressed());
	    }
	  break;

	case INSERTREMOVE_LINE_VERY_SOUTH:
	  if ( RightPressed() )
	    {
	      InsertLineVerySouth ( EditLevel );
	      while (RightPressed());
	    }
	  
	  if ( LeftPressed() )
	    {
	      EditLevel->ylen--; // making it smaller is always easy:  just modify the value for size
	      // allocation of new memory or things like that are not nescessary.
	      while (LeftPressed());
	    }
	  break;

	case INSERTREMOVE_LINE_SOUTHERN_INTERFACE:
	  if ( RightPressed() )
	    {
	      InsertLineSouthernInterface ( EditLevel );
	      while (RightPressed());
	    }
	  if ( LeftPressed() )
	    {
	      RemoveLineSouthernInterface ( EditLevel );
	      while (LeftPressed());
	    }
	  break;

	case INSERTREMOVE_LINE_NORTHERN_INTERFACE:
	  if ( RightPressed() )
	    {
	      InsertLineNorthernInterface ( EditLevel );
	      while (RightPressed());
	    }
	  if ( LeftPressed() )
	    {
	      RemoveLineNorthernInterface ( EditLevel );
	      while (LeftPressed());
	    }
	  break;

	case INSERTREMOVE_LINE_VERY_NORTH:
	  if ( RightPressed() )
	    {
	      InsertLineVeryNorth ( EditLevel );
	      while (RightPressed());
	    }
	  if ( LeftPressed() )
	    {
	      RemoveLineVeryNorth ( EditLevel );
	      while (LeftPressed());
	    }
	  break;

	case (-1):
	case BACK_TO_LE_MAIN_MENU:
	  while (EnterPressed() || SpacePressed() || EscapePressed() || MouseLeftPressed() ) SDL_Delay(1);
	  GetAllAnimatedMapTiles ( EditLevel );
	  proceed_now=!proceed_now;
	  break;

	default: 
	  break;

	}

    } // while (!proceed_now)
    
}; // void EditLevelDimensions ( void )
  
/* ----------------------------------------------------------------------
 *
 *
 * ---------------------------------------------------------------------- */
int
DoLevelEditorMainMenu ( Level EditLevel )
{
    char* MenuTexts[ 100 ];
    char Options [ 20 ] [1000];
    int proceed_now = FALSE ;
    int MenuPosition=1;
    int Done=FALSE;
    int i;

    enum
	{ 
	    SAVE_LEVEL_POSITION=1, 
	    CHANGE_LEVEL_POSITION, 
	    CHANGE_LIGHT_RADIUS_BONUS, 
	    CHANGE_MINIMAL_LIGHT_ON_LEVEL, 
	    CHANGE_INFINITE_RUNNING, 
	    SET_LEVEL_NAME , 
	    SET_BACKGROUND_SONG_NAME , 
	    SET_LEVEL_COMMENT, 
	    ADD_NEW_LEVEL , 
	    SET_LEVEL_INTERFACE_POSITION , 
	    EDIT_LEVEL_DIMENSIONS,
	    QUIT_LEVEL_EDITOR_POSITION 
	};
    
    while (!proceed_now)
    {
	
	EditLevel = curShip.AllLevels [ Me . pos . z ] ;
	
	InitiateMenu( -1 );
	
	i = 0 ;
	MenuTexts[ i ] = "SAVE EVERYTHING" ; i++;
	sprintf( Options [ 0 ] , "Current: %d.  Level Up/Down" , EditLevel->levelnum );
	MenuTexts[ i ] = Options [ 0 ]; i++;
	sprintf( Options [ 1 ] , "Light radius bonus: %d" , EditLevel -> light_radius_bonus );
	MenuTexts[ i ] = Options [ 1 ]; i++;
	sprintf( Options [ 6 ] , "Minimal light value: %d" , EditLevel -> minimum_light_value );
	MenuTexts[ i ] = Options [ 6 ]; i++;
	sprintf( Options [ 2 ] , "Infinite running on this level: " );
	if ( EditLevel -> infinite_running_on_this_level ) strcat ( Options [ 2 ] , "YES" ) ;
	else ( strcat ( Options [ 2 ] , "NO" ) ) ;
	MenuTexts[ i ] = Options [ 2 ]; i++;
	sprintf( Options [ 3 ] , "Level name: %s" , EditLevel->Levelname );
	MenuTexts[ i ] = Options [ 3 ] ; i++;
	sprintf( Options [ 4 ] , "Background music file name: %s" , EditLevel->Background_Song_Name );
	MenuTexts[ i ] = Options [ 4 ] ; i++;
	sprintf( Options [ 5 ] , "Set Level Comment: %s" , EditLevel->Level_Enter_Comment );
	MenuTexts[ i ] = Options [ 5 ] ; i++;
	MenuTexts[ i ] = "Add completely new level" ; i++;
	MenuTexts[ i ] = "Set Level Interfaces" ; i++;
	MenuTexts[ i ] = "Edit Level Dimensions" ; i++;
	MenuTexts[ i ] = "Quit Level Editor" ; i++;
	MenuTexts[ i ] = "" ; i++;
	
	MenuPosition = DoMenuSelection( "" , MenuTexts , -1 , -1 , FPS_Display_BFont );
	
	
	while ( EnterPressed ( ) || SpacePressed ( ) || MouseLeftPressed() ) SDL_Delay(1);
	
	switch ( MenuPosition ) 
	{
	    
	    case (-1):
		while ( EscapePressed() );
		proceed_now=!proceed_now;
		// if ( CurrentCombatScaleFactor != 1 ) SetCombatScaleTo( 1 );
		break;
	    case SAVE_LEVEL_POSITION:
		while (EnterPressed() || SpacePressed() || MouseLeftPressed()) SDL_Delay(1);
		close_all_chests_on_level ( Me . pos . z ) ;
		char fp[2048];
		find_file("Asteroid.maps", MAP_DIR, fp, 0);
		SaveShip(fp);
		CenteredPutString ( Screen ,  11*FontHeight(Menu_BFont),    "Your ship was saved...");
		our_SDL_flip_wrapper ( Screen );
		while (EnterPressed() || SpacePressed() || MouseLeftPressed()) SDL_Delay(1);
		// proceed_now=!proceed_now;
		break;
	    case CHANGE_LEVEL_POSITION: 
		// if ( EditLevel->levelnum ) Teleport ( EditLevel->levelnum-1 , Me.pos.x , Me.pos.y );
		while (EnterPressed() || SpacePressed() || MouseLeftPressed()) SDL_Delay(1);
		break;
	    case CHANGE_LIGHT_RADIUS_BONUS: 
		while (EnterPressed() || SpacePressed() || MouseLeftPressed()) SDL_Delay(1);
		break;
	    case CHANGE_MINIMAL_LIGHT_ON_LEVEL:
		while (EnterPressed() || SpacePressed() || MouseLeftPressed()) SDL_Delay(1);
		break;
	    case SET_LEVEL_NAME:
		while (EnterPressed() || SpacePressed() || MouseLeftPressed()) SDL_Delay(1);
		EditLevel->Levelname = 
		    GetEditableStringInPopupWindow ( 1000 , "\n Please enter new level name: \n\n" ,
						     EditLevel->Levelname );
		proceed_now=!proceed_now;
		break;
	    case SET_BACKGROUND_SONG_NAME:
		while (EnterPressed() || SpacePressed() || MouseLeftPressed()) SDL_Delay(1);
		EditLevel->Background_Song_Name = 
		    GetEditableStringInPopupWindow ( 1000 , "\n Please enter new music file name: \n\n" ,
						     EditLevel->Background_Song_Name );
		proceed_now=!proceed_now;
		break;
	    case SET_LEVEL_COMMENT:
		while (EnterPressed() || SpacePressed() || MouseLeftPressed()) SDL_Delay(1);
		EditLevel->Level_Enter_Comment = 
		    GetEditableStringInPopupWindow ( 1000 , "\n Please enter new level comment: \n\n" ,
						     EditLevel->Level_Enter_Comment );
		proceed_now=!proceed_now;
		break;
	    case ADD_NEW_LEVEL:
		while (EnterPressed() || SpacePressed() || MouseLeftPressed()) SDL_Delay(1);
		if ( curShip . num_levels < MAX_LEVELS )
		{
		    CreateNewMapLevel ( ) ;
		    Me . pos . z = curShip.num_levels - 1;
		    Me . pos . x = 3;
		    Me . pos . y = 3;
		}
		proceed_now=!proceed_now;
		break;
	    case SET_LEVEL_INTERFACE_POSITION:
		while (EnterPressed() || SpacePressed() || MouseLeftPressed()) SDL_Delay(1);
		// proceed_now=!proceed_now;
		SetLevelInterfaces ( );
		break;
	    case EDIT_LEVEL_DIMENSIONS:
		while (EnterPressed() || SpacePressed() || MouseLeftPressed()) SDL_Delay(1);
		// proceed_now=!proceed_now;
		EditLevelDimensions ( );
		break;
	    case QUIT_LEVEL_EDITOR_POSITION:
		while (EnterPressed() || SpacePressed() || MouseLeftPressed()) SDL_Delay(1);
		proceed_now=!proceed_now;
		Done=TRUE;
		break;
	    case CHANGE_INFINITE_RUNNING:
		while (EnterPressed() || SpacePressed() || MouseLeftPressed()) SDL_Delay(1);
		EditLevel -> infinite_running_on_this_level =
		    ! EditLevel -> infinite_running_on_this_level ;
		break;
	    default: 
		break;
		
	} // switch
	
	// If the user of the level editor pressed left or right, that should have
	// an effect IF he/she is a the change level menu point
	
	if ( LeftPressed ( ) || RightPressed ( ) ) 
	{
	    switch (MenuPosition)
	    {
		
		case CHANGE_LEVEL_POSITION:
		    if ( LeftPressed() )
		    {
			if ( EditLevel->levelnum > 0 )
			    Teleport ( EditLevel->levelnum -1 , 3 , 3 , FALSE );
			while (LeftPressed());
		    }
		    if ( RightPressed() )
		    {
			if ( EditLevel->levelnum < curShip.num_levels -1 )
			    Teleport ( EditLevel->levelnum +1 , 3 , 3 , FALSE );
			while (RightPressed());
		    }
		    break;
		    
		case CHANGE_LIGHT_RADIUS_BONUS:
		    if ( RightPressed() )
		    {
			EditLevel -> light_radius_bonus ++;
			while (RightPressed());
		    }
		    if ( LeftPressed() )
		    {
			EditLevel -> light_radius_bonus --;
			while (LeftPressed());
		    }
		    Teleport ( EditLevel -> levelnum , 
			       Me . pos . x , Me . pos . y , FALSE ); 
		    break;
		    
		case CHANGE_MINIMAL_LIGHT_ON_LEVEL:
		    if ( RightPressed() )
		    {
			EditLevel -> minimum_light_value ++;
			while (RightPressed());
		    }
		    if ( LeftPressed() )
		    {
			EditLevel -> minimum_light_value --;
			while (LeftPressed());
		    }
		    Teleport ( EditLevel -> levelnum , 
			       Me . pos . x , Me . pos . y , FALSE ); 
		    break;
	    }
	} // if LeftPressed || RightPressed
    }
    return ( Done );
}; // void DoLevelEditorMainMenu ( Level EditLevel );

/* ----------------------------------------------------------------------
 * There is a 'help' screen for the level editor too.  This help screen
 * is presented as a scrolling text, giving a short introduction and also
 * explaining the keymap to the level editor.  The info for this scrolling
 * text is all in a title file in the maps dir, much like the initial
 * scrolling text at any new game startup.
 * ---------------------------------------------------------------------- */
void 
ShowLevelEditorKeymap ( void )
{
    PlayATitleFile ( "level_editor_help.title" );
}; // void ShowLevelEditorKeymap ( void )

/* ----------------------------------------------------------------------
 * The levels in Freedroid may be connected into one big map by simply
 * 'gluing' then together, i.e. we define some interface areas to the
 * sides of a map and when the Tux passes these areas, he'll be silently
 * put into another map without much fuss.  This operation is performed
 * silently and the two maps must be synchronized in this interface area
 * so the map change doesn't become apparend to the player.  Part of this
 * synchronisation, namely copying the map tiles to the other map, is 
 * done automatically, but some inconsistencies like non-matching map
 * sizes or non-symmetric jump directions (i.e. not back and forth but
 * back and forth-to-somewhere else) are not resolved automatically.
 * Instead, a report on inconsistencies will be created and the person
 * editing the map can then resolve the inconsistencies manually in one
 * fashion or the other.
 * ---------------------------------------------------------------------- */
void
ReportInconsistenciesForLevel ( int LevelNum )
{
    int TargetLevel;
    SDL_Rect ReportRect;
    
    ReportRect.x = 20;
    ReportRect.y = 20;
    ReportRect.w = 600;
    ReportRect.h = 440;
    
    AssembleCombatPicture ( ONLY_SHOW_MAP_AND_TEXT | SHOW_GRID | SKIP_LIGHT_RADIUS );
    
    DisplayText ( "\nThe list of inconsistencies of the jump interfaces for this level:\n\n" ,
		  ReportRect.x, ReportRect.y + FontHeight ( GetCurrentFont () ) , &ReportRect , 1.0 );
    
    //--------------------
    // First we test for inconsistencies of back-forth ways, i.e. if the transit
    // in one direction will lead back in the right direction when returning.
    //
    if ( curShip.AllLevels [ LevelNum ] -> jump_target_north != (-1) ) 
    {
	TargetLevel = curShip.AllLevels [ LevelNum ] -> jump_target_north ;
	if ( curShip.AllLevels [ TargetLevel ] -> jump_target_south != LevelNum )
	{
	    DisplayText ( "BACK-FORTH-MISMATCH: North doesn't lead back here (yet)!\n" ,
			  -1 , -1 , &ReportRect , 1.0 );
	}
    }
    if ( curShip.AllLevels [ LevelNum ] -> jump_target_south != (-1) ) 
    {
	TargetLevel = curShip.AllLevels [ LevelNum ] -> jump_target_south ;
	if ( curShip.AllLevels [ TargetLevel ] -> jump_target_north != LevelNum )
	{
	    DisplayText ( "BACK-FORTH-MISMATCH: South doesn't lead back here (yet)!\n" ,
			  -1 , -1 , &ReportRect , 1.0 );
	}
    }
    if ( curShip.AllLevels [ LevelNum ] -> jump_target_east != (-1) ) 
    {
	TargetLevel = curShip.AllLevels [ LevelNum ] -> jump_target_east ;
	if ( curShip.AllLevels [ TargetLevel ] -> jump_target_west != LevelNum )
	{
	    DisplayText ( "BACK-FORTH-MISMATCH: East doesn't lead back here (yet)!\n" ,
			  -1 , -1 , &ReportRect , 1.0 );
	}
    }
    if ( curShip.AllLevels [ LevelNum ] -> jump_target_west != (-1) ) 
    {
	TargetLevel = curShip.AllLevels [ LevelNum ] -> jump_target_west ;
	if ( curShip.AllLevels [ TargetLevel ] -> jump_target_east != LevelNum )
	{
	    DisplayText ( "BACK-FORTH-MISMATCH: West doesn't lead back here (yet)!\n" ,
			  -1 , -1 , &ReportRect , 1.0 );
	}
    }
    DisplayText ( "\nNO OTHER BACK-FORTH-MISMATCH ERRORS other than those listed above\n\n" ,
		  -1 , -1 , &ReportRect , 1.0 );
    
    //--------------------
    // Now we test for inconsistencies of interface sizes, i.e. if the interface source level
    // has an interface as large as the target interface level.
    //
    if ( curShip.AllLevels [ LevelNum ] -> jump_target_north != (-1) ) 
    {
	TargetLevel = curShip.AllLevels [ LevelNum ] -> jump_target_north ;
	if ( curShip.AllLevels [ TargetLevel ] -> jump_threshold_south != 
	     curShip.AllLevels [ LevelNum ] -> jump_threshold_north )
	{
	    DisplayText ( "INTERFACE SIZE MISMATCH: North doesn't lead so same-sized interface level!!!\n" ,
			  -1 , -1 , &ReportRect , 1.0 );
	}
    }
    if ( curShip.AllLevels [ LevelNum ] -> jump_target_south != (-1) ) 
    {
	TargetLevel = curShip.AllLevels [ LevelNum ] -> jump_target_south ;
	if ( curShip.AllLevels [ TargetLevel ] -> jump_threshold_north != 
	     curShip.AllLevels [ LevelNum ] -> jump_threshold_south )
	{
	    DisplayText ( "INTERFACE SIZE MISMATCH: South doesn't lead so same-sized interface level!!!\n" ,
			  -1 , -1 , &ReportRect , 1.0 );
	}
    }
    if ( curShip.AllLevels [ LevelNum ] -> jump_target_east != (-1) ) 
    {
	TargetLevel = curShip.AllLevels [ LevelNum ] -> jump_target_east ;
	if ( curShip.AllLevels [ TargetLevel ] -> jump_threshold_west != 
	     curShip.AllLevels [ LevelNum ] -> jump_threshold_east )
	{
	    DisplayText ( "INTERFACE SIZE MISMATCH: East doesn't lead so same-sized interface level!!!\n" ,
			  -1 , -1 , &ReportRect , 1.0 );
	}
    }
    if ( curShip.AllLevels [ LevelNum ] -> jump_target_west != (-1) ) 
    {
	TargetLevel = curShip.AllLevels [ LevelNum ] -> jump_target_west ;
	if ( curShip.AllLevels [ TargetLevel ] -> jump_threshold_east != 
	     curShip.AllLevels [ LevelNum ] -> jump_threshold_west )
	{
	    DisplayText ( "INTERFACE SIZE MISMATCH: West doesn't lead so same-sized interface level!!!\n" ,
			  -1 , -1 , &ReportRect , 1.0 );
	}
    }
    
    //--------------------
    // Now we test for inconsistencies of level sizes, i.e. if the interface source level
    // has the same relevant dimension like the target interface level.
    //
    if ( curShip.AllLevels [ LevelNum ] -> jump_target_north != (-1) ) 
    {
	TargetLevel = curShip.AllLevels [ LevelNum ] -> jump_target_north ;
	if ( curShip.AllLevels [ TargetLevel ] -> xlen != curShip.AllLevels [ LevelNum ] -> xlen )
	{
	    DisplayText ( "LEVEL DIMENSION MISMATCH: North doesn't lead so same-sized level (non-fatal, but no good comes from this)!\n" ,
			  -1 , -1 , &ReportRect , 1.0 );
	}
    }
    if ( curShip.AllLevels [ LevelNum ] -> jump_target_south != (-1) ) 
    {
	TargetLevel = curShip.AllLevels [ LevelNum ] -> jump_target_south ;
	if ( curShip.AllLevels [ TargetLevel ] -> xlen != curShip.AllLevels [ LevelNum ] -> xlen )
	{
	    DisplayText ( "LEVEL DIMENSION MISMATCH: South doesn't lead so same-sized level (non-fatal, but no good comes from this)!\n" ,
			  -1 , -1 , &ReportRect , 1.0 );
	}
    }
    if ( curShip.AllLevels [ LevelNum ] -> jump_target_east != (-1) ) 
    {
	TargetLevel = curShip.AllLevels [ LevelNum ] -> jump_target_east ;
	if ( curShip.AllLevels [ TargetLevel ] -> ylen != curShip.AllLevels [ LevelNum ] -> ylen )
	{
	    DisplayText ( "LEVEL DIMENSION MISMATCH: East doesn't lead so same-sized level (non-fatal, but no good comes from this)!\n" ,
			  -1 , -1 , &ReportRect , 1.0 );
	}
    }
    if ( curShip.AllLevels [ LevelNum ] -> jump_target_west != (-1) ) 
    {
	TargetLevel = curShip.AllLevels [ LevelNum ] -> jump_target_west ;
	if ( curShip.AllLevels [ TargetLevel ] -> ylen != curShip.AllLevels [ LevelNum ] -> ylen )
	{
	    DisplayText ( "LEVEL DIMENSION MISMATCH: West doesn't lead so same-sized level (non-fatal, but no good comes from this)!\n" ,
			  -1 , -1 , &ReportRect , 1.0 );
	}
    }
    
    //--------------------
    // This was it.  We can say so and return.
    //
    DisplayText ( "\n\n--- End of List --- Press Space to return to menu ---\n" ,
		  -1 , -1 , &ReportRect , 1.0 );
    
    our_SDL_flip_wrapper ( Screen );
    
}; // void ReportInconsistenciesForLevel ( int LevelNum )

/* ----------------------------------------------------------------------
 * If we want to synchronize two levels, we need to remove the old obstacles
 * before we can add new ones.  Else the place might get too crowded with
 * obstacles. :)
 * ---------------------------------------------------------------------- */
void
delete_all_obstacles_in_area ( Level TargetLevel , float start_x , float start_y , float area_width , float area_height )
{
    int i;
    
    for ( i = 0 ; i < MAX_OBSTACLES_ON_MAP ; i ++ )
    {
	if ( TargetLevel -> obstacle_list [ i ] . type <= (-1) ) continue;
	if ( TargetLevel -> obstacle_list [ i ] . pos . x < start_x ) continue;
	if ( TargetLevel -> obstacle_list [ i ] . pos . y < start_y ) continue;
	if ( TargetLevel -> obstacle_list [ i ] . pos . x > start_x + area_width ) continue;
	if ( TargetLevel -> obstacle_list [ i ] . pos . y > start_y + area_height ) continue;
	action_remove_obstacle ( TargetLevel , & ( TargetLevel -> obstacle_list [ i ] ) );
	i--; // this is so that this obstacle will be processed AGAIN, since deleting might
	// have moved a different obstacle to this list position.
    }
}; // void delete_all_obstacles_in_area ( curShip . AllLevels [ TargetLevel ] , 0 , TargetLevel->ylen-AreaHeight , AreaWidth , AreaHeight )

/* ----------------------------------------------------------------------
 * After exporting a level, there might be some old corpses of 
 * descriptions that were deleted when the target level was partly cleared
 * out and overwritten with the new obstacles that brought their own new
 * obstacle descriptions.
 *
 * In this function, we try to clean out those old corpses to avoid 
 * cluttering in the map file.
 * ---------------------------------------------------------------------- */
void
eliminate_dead_obstacle_descriptions ( Level target_level )
{
    int i;
    int is_in_use;
    int desc_index;

    //--------------------
    // We proceed through the list of known descriptions.  Some of them
    // might not be in use, but still hold a non-null content string.
    // Such instances will be eliminated.
    //
    for ( desc_index = 0 ; desc_index < MAX_OBSTACLE_DESCRIPTIONS_PER_LEVEL ; desc_index ++ )
    {
	//--------------------
	// Maybe the description in question is an empty index anyway.
	// Then of course there is no need to eliminate anything and
	// we can proceed right away.
	//
	if ( target_level -> obstacle_description_list [ desc_index ] == NULL ) continue;

	//--------------------
	// So now we've encountered some string.  Let's see if it's really
	// in use.  For that, we need to proceed through all the obstacles
	// of this level and see if one of them has a description index 
	// pointing to this description string.
	//
	is_in_use = FALSE ;
	for ( i = 0 ; i < MAX_OBSTACLES_ON_MAP ; i ++ )
	{
	    if ( target_level -> obstacle_list [ i ] . description_index == desc_index )
	    {
		DebugPrintf ( 1 , "\nvoid eliminate_dead_obstacle_descriptions(...):  This descriptions seems to be in use still." );
		is_in_use = TRUE ;
		break;
	    }
	}
	
	if ( is_in_use ) continue;
	target_level -> obstacle_description_list [ desc_index ] = NULL ;
	DebugPrintf ( 1 , "\nNOTE: void eliminate_dead_obstacle_descriptions(...):  dead description found.  Eliminated." );
    }
	
}; // void eliminate_dead_obstacle_descriptions ( Level target_level )

/* ----------------------------------------------------------------------
 * This function should allow for conveninet duplication of obstacles from
 * one map to the other.  It assumes, that the target area has been cleaned
 * out of obstacles already.
 * ---------------------------------------------------------------------- */
void
duplicate_all_obstacles_in_area ( Level source_level ,
				  float source_start_x , float source_start_y , 
				  float source_area_width , float source_area_height ,
				  Level target_level ,
				  float target_start_x , float target_start_y )
{
    int i;
    obstacle* new_obstacle;

    for ( i = 0 ; i < MAX_OBSTACLES_ON_MAP ; i ++ )
    {
	if ( source_level -> obstacle_list [ i ] . type <= (-1) ) continue;
	if ( source_level -> obstacle_list [ i ] . pos . x < source_start_x ) continue;
	if ( source_level -> obstacle_list [ i ] . pos . y < source_start_y ) continue;
	if ( source_level -> obstacle_list [ i ] . pos . x > source_start_x + source_area_width ) continue;
	if ( source_level -> obstacle_list [ i ] . pos . y > source_start_y + source_area_height ) continue;
	
	new_obstacle = 
	    action_create_obstacle ( target_level , 
			   target_start_x  + source_level -> obstacle_list [ i ] . pos . x - source_start_x ,
			   target_start_y  + source_level -> obstacle_list [ i ] . pos . y - source_start_y ,
			   source_level -> obstacle_list [ i ] . type );
	
	//--------------------
	// Maybe the source obstacle had a label attached to it?  Then
	// We should also duplicate the obstacle label.  Otherwise it
	// might get overwritten when exporting in the other direction.
	//
	if ( source_level -> obstacle_list [ i ] . name_index != (-1) )
	{
	    action_change_obstacle_label_user ( 
		target_level , new_obstacle , 
		source_level -> obstacle_name_list [ source_level -> obstacle_list [ i ] . name_index ] );
	    DebugPrintf ( 1 , "\nNOTE: void duplicate_all_obstacles_in_area(...):  obstacle name was exported:  %s." ,
			  source_level -> obstacle_name_list [ source_level -> obstacle_list [ i ] . name_index ] );
	}

	//--------------------
	// Maybe the source obstacle had a description attached to it?  Then
	// We should also duplicate the obstacle description.  Otherwise it
	// might get overwritten when exporting in the other direction.
	//
	if ( source_level -> obstacle_list [ i ] . description_index != (-1) )
	{
	    give_new_description_to_obstacle ( 
		target_level , new_obstacle , 
		source_level -> obstacle_description_list [ source_level -> obstacle_list [ i ] . description_index ] );
	    DebugPrintf ( -1 , "\nNOTE:  obstacle description was exported:  %s." ,
			  source_level -> obstacle_description_list [ source_level -> obstacle_list [ i ] . description_index ] );
	}
	
	//action_remove_obstacle ( source_level , & ( source_level -> obstacle_list [ i ] ) );
	// i--; // this is so that this obstacle will be processed AGAIN, since deleting might
	// // have moved a different obstacle to this list position.
    }

    eliminate_dead_obstacle_descriptions ( target_level );
    
}; // void duplicate_all_obstacles_in_area ( ... )

/* ----------------------------------------------------------------------
 *
 *
 * ---------------------------------------------------------------------- */      
void 
floor_copy ( map_tile* target_pointer , map_tile* source_pointer , int amount )
{
    int i;
    
    for ( i = 0 ; i < amount ; i ++ )
    {
	target_pointer -> floor_value = source_pointer -> floor_value ;
	target_pointer ++ ;
	source_pointer ++ ;
    }
}; // void floor_copy ( map_tile* target_pointer , map_tile* source_pointer , int amount )

/* ----------------------------------------------------------------------
 * When we connect two maps smoothly together, we want an area in both
 * maps, that is really synchronized with the other level we connect to.
 * But this isn't a task that should be done manually.  We rather make
 * a function, that does this synchronisation work, overwriting the 
 * higher level number with the data from the lower level number.
 * ---------------------------------------------------------------------- */
void
ExportLevelInterface ( int LevelNum )
{
    int AreaWidth;
    int AreaHeight;
    int TargetLevel;
    int y;
    int TargetStartLine;
    
    //--------------------
    // First we see if we need to copy the northern interface region
    // into another map.
    //
    TargetLevel = curShip.AllLevels [ LevelNum ] -> jump_target_north ;
    if ( TargetLevel != (-1) ) 
    {
	//--------------------
	// First we find out the dimensions of the area we want to copy
	//
	if ( curShip . AllLevels [ LevelNum ] -> xlen < curShip . AllLevels [ TargetLevel ] -> xlen )
	    AreaWidth = curShip . AllLevels [ LevelNum ] -> xlen;
	else 
	    AreaWidth = curShip . AllLevels [ TargetLevel ] -> xlen ;
	
	AreaHeight = curShip . AllLevels [ LevelNum ] -> jump_threshold_north;
	
	if ( AreaHeight <= 0 ) return;
	
	TargetStartLine = ( curShip . AllLevels [ TargetLevel ] -> ylen ) - 1 ;
	
	//--------------------
	// Now we can start to make the copy...
	//
	for ( y = 0 ; y < AreaHeight ; y ++ )
	{
	    floor_copy ( curShip . AllLevels [ TargetLevel ] -> map[ TargetStartLine - y ] ,
			 curShip . AllLevels [ LevelNum ] -> map[ AreaHeight-1 - y ] ,
			 AreaWidth ) ;
	    // memset ( curShip . AllLevels [ TargetLevel ] -> map[ TargetStartLine - y ] , 0 , AreaWidth ); 
	    // DebugPrintf ( 0 , "\nAreaWidth: %d." , AreaWidth * sizeof ( );
	}
	
	delete_all_obstacles_in_area ( curShip . AllLevels [ TargetLevel ] , 0 , curShip . AllLevels [ TargetLevel ] ->ylen-AreaHeight , AreaWidth , AreaHeight );
	
	
	duplicate_all_obstacles_in_area ( curShip . AllLevels [ LevelNum ] , 0 , 0 , 
					  AreaWidth , AreaHeight ,
					  curShip . AllLevels [ TargetLevel ] , 0 , curShip . AllLevels [ TargetLevel ] -> ylen-AreaHeight );
	
	GetAllAnimatedMapTiles ( curShip . AllLevels [ TargetLevel ] );
    }

    //--------------------
    // Now we see if we need to copy the southern interface region
    // into another map.
    //
    TargetLevel = curShip.AllLevels [ LevelNum ] -> jump_target_south ;
    if ( TargetLevel != (-1) ) 
    {
	//--------------------
	// First we find out the dimensions of the area we want to copy
	//
	if ( curShip . AllLevels [ LevelNum ] -> xlen < curShip . AllLevels [ TargetLevel ] -> xlen )
	    AreaWidth = curShip . AllLevels [ LevelNum ] -> xlen;
	else 
	    AreaWidth = curShip . AllLevels [ TargetLevel ] -> xlen ;
	
	AreaHeight = curShip . AllLevels [ LevelNum ] -> jump_threshold_south;
	
	if ( AreaHeight <= 0 ) return;
	
	TargetStartLine = ( curShip . AllLevels [ LevelNum ] -> ylen ) - 1 ;
	
	//--------------------
	// Now we can start to make the copy...
	//
	for ( y = 0 ; y < AreaHeight ; y ++ )
	{
	    floor_copy ( curShip . AllLevels [ TargetLevel ] -> map[ AreaHeight-1 - y ] ,
			 curShip . AllLevels [ LevelNum ] -> map[ TargetStartLine - y ] ,
			 AreaWidth ) ;
	}
	
	delete_all_obstacles_in_area ( curShip . AllLevels [ TargetLevel ] , 0 , 0 , AreaWidth , AreaHeight );
	
	duplicate_all_obstacles_in_area ( curShip . AllLevels [ LevelNum ] , 0 , curShip . AllLevels [ LevelNum ] -> ylen - AreaHeight ,
					  AreaWidth , AreaHeight ,
					  curShip . AllLevels [ TargetLevel ] , 0 , 0 );
	
	GetAllAnimatedMapTiles ( curShip . AllLevels [ TargetLevel ] );
	
    }
    
    //--------------------
    // Now we see if we need to copy the eastern interface region
    // into another map.
    //
    TargetLevel = curShip.AllLevels [ LevelNum ] -> jump_target_east ;
    if ( TargetLevel != (-1) ) 
    {
	//--------------------
	// First we find out the dimensions of the area we want to copy
	//
	if ( curShip . AllLevels [ LevelNum ] -> ylen < curShip . AllLevels [ TargetLevel ] -> ylen )
	    AreaHeight = curShip . AllLevels [ LevelNum ] -> ylen;
	else 
	    AreaHeight = curShip . AllLevels [ TargetLevel ] -> ylen ;
	
	AreaWidth = curShip . AllLevels [ LevelNum ] -> jump_threshold_east;
	
	if ( AreaWidth <= 0 ) return;
	
	TargetStartLine = ( curShip . AllLevels [ TargetLevel ] -> ylen ) - 1 ;
	
	//--------------------
	// Now we can start to make the copy...
	//
	for ( y = 0 ; y < AreaHeight ; y ++ )
	{
	    floor_copy ( curShip . AllLevels [ TargetLevel ] -> map[ y ] ,
			 (curShip . AllLevels [ LevelNum ] -> map[ y ]) + 
			 curShip . AllLevels [ LevelNum ] -> xlen - 0 - AreaWidth ,
			 AreaWidth ) ;
	}
	
	delete_all_obstacles_in_area ( curShip . AllLevels [ TargetLevel ] , 0 , 0 , AreaWidth , AreaHeight );
	
	duplicate_all_obstacles_in_area ( curShip . AllLevels [ LevelNum ] , curShip . AllLevels [ LevelNum ] -> xlen - AreaWidth , 0 , 
					  AreaWidth , AreaHeight ,
					  curShip . AllLevels [ TargetLevel ] , 0 , 0 );
	
	GetAllAnimatedMapTiles ( curShip . AllLevels [ TargetLevel ] );
	
    }
    
    //--------------------
    // Now we see if we need to copy the western interface region
    // into another map.
    //
    TargetLevel = curShip.AllLevels [ LevelNum ] -> jump_target_west ;
    if ( TargetLevel != (-1) ) 
    {
	//--------------------
	// First we find out the dimensions of the area we want to copy
	//
	if ( curShip . AllLevels [ LevelNum ] -> ylen < curShip . AllLevels [ TargetLevel ] -> ylen )
	    AreaHeight = curShip . AllLevels [ LevelNum ] -> ylen;
	else 
	    AreaHeight = curShip . AllLevels [ TargetLevel ] -> ylen ;
	
	AreaWidth = curShip . AllLevels [ LevelNum ] -> jump_threshold_west;
	
	if ( AreaWidth <= 0 ) return;
	
	TargetStartLine = ( curShip . AllLevels [ TargetLevel ] -> ylen ) - 1 ;
	
	//--------------------
	// Now we can start to make the copy...
	//
	for ( y = 0 ; y < AreaHeight ; y ++ )
	{
	    floor_copy ( ( curShip . AllLevels [ TargetLevel ] -> map[ y ] ) + 
			 curShip . AllLevels [ TargetLevel ] -> xlen - 0 - AreaWidth,
			 ( curShip . AllLevels [ LevelNum ] -> map[ y ] ) + 0 , 
			 AreaWidth ) ;
	}
	
	delete_all_obstacles_in_area ( curShip . AllLevels [ TargetLevel ] , curShip . AllLevels [ TargetLevel ] -> xlen - AreaWidth , 0 , AreaWidth , AreaHeight );
	
	duplicate_all_obstacles_in_area ( curShip . AllLevels [ LevelNum ] , 0 , 0 , AreaWidth , AreaHeight ,
					  curShip . AllLevels [ TargetLevel ] , curShip . AllLevels [ TargetLevel ] -> xlen - AreaWidth , 0 );
	
	GetAllAnimatedMapTiles ( curShip . AllLevels [ TargetLevel ] );
    }
    
}; // void SynchronizeLevelInterfaces ( void )
      
/* ----------------------------------------------------------------------
 *
 *
 * ---------------------------------------------------------------------- */
void
SetLevelInterfaces ( void )
{
    char *MenuTexts[ 100 ];
    int proceed_now = FALSE;
    int MenuPosition = 1 ;
    char Options [ 20 ] [ 500 ] ;
    Level EditLevel;
    
    EditLevel = curShip.AllLevels [ Me . pos . z ] ;
    
    while (!proceed_now)
    {
	EditLevel = curShip.AllLevels [ Me . pos . z ] ;
	InitiateMenu( -1 );
	sprintf( Options [ 0 ] , "Jump threshold north: %d.  Up/Down" , EditLevel->jump_threshold_north );
	MenuTexts [ 0 ] = Options [ 0 ] ;
	sprintf( Options [ 1 ] , "Jump threshold south: %d.  Up/Down" , EditLevel->jump_threshold_south );
	MenuTexts [ 1 ] = Options [ 1 ] ;
	sprintf( Options [ 2 ] , "Jump threshold east: %d.  Up/Down" , EditLevel->jump_threshold_east );
	MenuTexts [ 2 ] = Options [ 2 ] ;
	sprintf( Options [ 3 ] , "Jump threshold west: %d.  Up/Down" , EditLevel->jump_threshold_west );
	MenuTexts [ 3 ] = Options [ 3 ] ;
	sprintf( Options [ 4 ] , "Jump target north: %d.  Up/Down" , EditLevel->jump_target_north );
	MenuTexts [ 4 ] = Options [ 4 ] ;
	sprintf( Options [ 5 ] , "Jump target south: %d.  Up/Down" , EditLevel->jump_target_south );
	MenuTexts [ 5 ] = Options [ 5 ] ;
	sprintf( Options [ 6 ] , "Jump target east: %d.  Up/Down" , EditLevel->jump_target_east );
	MenuTexts [ 6 ] = Options [ 6 ] ;
	sprintf( Options [ 7 ] , "Jump target west: %d.  Up/Down" , EditLevel->jump_target_west );
	MenuTexts [ 7 ] = Options [ 7 ] ;
	MenuTexts [ 8 ] = "Export this level to other target levels" ;
	MenuTexts [ 9 ] = "Report interface inconsistencies";
	MenuTexts [ 10 ] = "Quit Threshold Editor" ;
	MenuTexts [ 11 ] = "" ;
	
	MenuPosition = DoMenuSelection( "" , MenuTexts , -1 , -1 , FPS_Display_BFont );
	
	while (EnterPressed() || MouseLeftPressed() ) SDL_Delay(1);
	
	switch (MenuPosition) 
	{
	    case (-1):
		while ( EscapePressed() ) SDL_Delay(1);
		proceed_now=!proceed_now;
		break;
	    case EXPORT_THIS_LEVEL:
		while (EnterPressed() || MouseLeftPressed() ) SDL_Delay(1);
		ExportLevelInterface ( Me . pos . z );
		// proceed_now=!proceed_now;
		break;
	    case REPORT_INTERFACE_INCONSISTENCIES:
		while (EnterPressed() || SpacePressed() || MouseLeftPressed() ) SDL_Delay(1) ;
		ReportInconsistenciesForLevel ( Me . pos . z );
		while ( !EnterPressed() && !SpacePressed() && !MouseLeftPressed()) SDL_Delay(1);
		while (EnterPressed() || SpacePressed() || MouseLeftPressed()) SDL_Delay(1) ;
		break;
		
	    case QUIT_THRESHOLD_EDITOR_POSITION:
		while (EnterPressed() || SpacePressed() || MouseLeftPressed()) SDL_Delay(1) ;
		proceed_now=!proceed_now;
		break;
	    default: 
		break;
	} // switch
	
	  // If the user of the level editor pressed left or right, that should have
	  // an effect IF he/she is a the change level menu point
	
	if (LeftPressed() || RightPressed() ) 
	{
	    switch (MenuPosition)
	    {
		case JUMP_THRESHOLD_NORTH:
		    if ( LeftPressed() )
		    {
			if ( EditLevel->jump_threshold_north >= 0 ) EditLevel->jump_threshold_north -- ;
			while (LeftPressed());
		    }
		    if ( RightPressed() )
		    {
			EditLevel->jump_threshold_north ++ ;
			while (RightPressed());
		    }
		    break;
		    
		case JUMP_THRESHOLD_SOUTH:
		    if ( LeftPressed() )
		    {
			if ( EditLevel->jump_threshold_south >= 0 ) EditLevel->jump_threshold_south -- ;
			while (LeftPressed());
		    }
		    if ( RightPressed() )
		    {
			EditLevel->jump_threshold_south ++ ;
			while (RightPressed());
		    }
		    break;
		    
		case JUMP_THRESHOLD_EAST:
		    if ( LeftPressed() )
		    {
			if ( EditLevel->jump_threshold_east >= 0 ) EditLevel->jump_threshold_east -- ;
			while (LeftPressed());
		    }
		    if ( RightPressed() )
		    {
			EditLevel->jump_threshold_east ++ ;
			while (RightPressed());
		    }
		    break;
		    
		case JUMP_THRESHOLD_WEST:
		    if ( LeftPressed() )
		    {
			if ( EditLevel->jump_threshold_west >= 0 ) EditLevel->jump_threshold_west -- ;
			while (LeftPressed());
		    }
		    if ( RightPressed() )
		    {
			EditLevel->jump_threshold_west ++ ;
			while (RightPressed());
		    }
		    break;
		    
		case JUMP_TARGET_NORTH:
		    if ( LeftPressed() )
		    {
			if ( EditLevel->jump_target_north >= 0 ) EditLevel->jump_target_north -- ;
			while (LeftPressed());
		    }
		    if ( RightPressed() )
		    {
			EditLevel->jump_target_north ++ ;
			while (RightPressed());
		    }
		    break;
		    
		case JUMP_TARGET_SOUTH:
		    if ( LeftPressed() )
		    {
			if ( EditLevel->jump_target_south >= 0 ) EditLevel->jump_target_south -- ;
			while (LeftPressed());
		    }
		    if ( RightPressed() )
		    {
			EditLevel->jump_target_south ++ ;
			while (RightPressed());
		    }
		    break;
		    
		case JUMP_TARGET_EAST:
		    if ( LeftPressed() )
		    {
			if ( EditLevel->jump_target_east >= 0 ) EditLevel->jump_target_east -- ;
			while (LeftPressed());
		    }
		    if ( RightPressed() )
		    {
			EditLevel->jump_target_east ++ ;
			while (RightPressed());
		    }
		    break;
		    
		case JUMP_TARGET_WEST:
		    if ( LeftPressed() )
		    {
			if ( EditLevel->jump_target_west >= 0 ) EditLevel->jump_target_west -- ;
			while (LeftPressed());
		    }
		    if ( RightPressed() )
		    {
			EditLevel->jump_target_west ++ ;
			while (RightPressed());
		    }
		    break;
		    
		    
	    }
	} // if LeftPressed || RightPressed
	
    }
    
}; // void SetLevelInterfaces ( void )

/* ----------------------------------------------------------------------
 * This function should create a completely new level into the existing
 * ship structure that we already have.  The new level will be rather
 * small and simple.
 * ---------------------------------------------------------------------- */
void
CreateNewMapLevel( void )
{
    Level NewLevel;
    int i, k, l ;
    
    //--------------------
    // Get the memory for one level 
    //
    NewLevel = (Level) MyMalloc ( sizeof ( level ) );
    
    DebugPrintf (0, "\n-----------------------------------------------------------------");
    DebugPrintf (0, "\nStarting to create and add a completely new level to the ship.");
    
    //--------------------
    // Now we proceed in the order of the struct 'level' in the
    // struct.h file so that we can easily verify if we've handled
    // all the data structure or left something out which could
    // be terrible!
    //
    NewLevel -> levelnum = curShip.num_levels ;
    NewLevel -> xlen = 90 ;
    NewLevel -> ylen = 90 ;
    NewLevel -> light_radius_bonus = 1 ;
    NewLevel -> minimum_light_value = 13 ;
    NewLevel -> Levelname = "New Level just created..." ;
    NewLevel -> Background_Song_Name = "TheBeginning.ogg" ;
    NewLevel -> Level_Enter_Comment = "This is a new level..." ;
    //--------------------
    // Now we initialize the statement array with 'empty' values
    //
    for ( i = 0 ; i < MAX_STATEMENTS_PER_LEVEL ; i ++ )
    {
	NewLevel -> StatementList [ i ] . x = ( -1 ) ;
	NewLevel -> StatementList [ i ] . y = ( -1 ) ;
	NewLevel -> StatementList [ i ] . Statement_Text = "No Statement loaded." ;
    }
    //--------------------
    // Now we initialize the obstacle name list with 'empty' values
    //
    for ( i = 0 ; i < MAX_OBSTACLE_NAMES_PER_LEVEL ; i ++ )
    {
	NewLevel -> obstacle_name_list [ i ] = NULL ;
    }
    //--------------------
    // Now we initialize the obstacle description list with 'empty' values
    //
    for ( i = 0 ; i < MAX_OBSTACLE_DESCRIPTIONS_PER_LEVEL ; i ++ )
    {
	NewLevel -> obstacle_description_list [ i ] = NULL ;
    }
    //--------------------
    // First we initialize the floor with 'empty' values
    //
    for ( i = 0 ; i < NewLevel -> ylen ; i ++ )
    {
	NewLevel -> map [ i ] = MyMalloc ( NewLevel -> xlen * sizeof ( map_tile ) ) ;
	for ( k = 0 ; k < NewLevel -> xlen ; k ++ )
	{
	    NewLevel -> map [ i ] [ k ] . floor_value = ISO_FLOOR_SAND ;
	    for ( l = 0 ; l < MAX_OBSTACLES_GLUED_TO_ONE_MAP_TILE ; l ++ )
	    {
		NewLevel -> map [ i ] [ k ] . obstacles_glued_to_here [ l ] = (-1) ;
	    }
	}
    }
    //--------------------
    // Now we initialize the level jump interface variables with 'empty' values
    //
    NewLevel->jump_target_north = (-1) ;
    NewLevel->jump_target_south = (-1) ;
    NewLevel->jump_target_east = (-1) ;
    NewLevel->jump_target_west = (-1) ;
    NewLevel->jump_threshold_north = (-1) ;
    NewLevel->jump_threshold_south = (-1) ;
    NewLevel->jump_threshold_east = (-1) ;
    NewLevel->jump_threshold_west = (-1) ;
    //--------------------
    // Now we initialize the map obstacles with 'empty' information
    //
    for ( i = 0 ; i < MAX_OBSTACLES_ON_MAP ; i ++ )
    {
	NewLevel -> obstacle_list [ i ] . type = ( -1 ) ;
	NewLevel -> obstacle_list [ i ] . pos . x = ( -1 ) ;
	NewLevel -> obstacle_list [ i ] . pos . y = ( -1 ) ;
	NewLevel -> obstacle_list [ i ] . name_index = ( -1 ) ;
    }
    for ( i = 0 ; i < MAX_OBSTACLES_ON_MAP ; i ++ )
    {
	NewLevel -> obstacle_list [ i ] . type = ( -1 ) ;
	NewLevel -> obstacle_list [ i ] . pos . x = ( -1 ) ;
	NewLevel -> obstacle_list [ i ] . pos . y = ( -1 ) ;
	NewLevel -> obstacle_list [ i ] . name_index = ( -1 ) ;
    }
    //--------------------
    // This should initialize the lists with the refreshed and other
    // animated map tiles...
    //
    GetAllAnimatedMapTiles ( NewLevel ) ;
    //--------------------
    // Now we initialize the map labels array with 'empty' information
    //
    for ( i = 0 ; i < MAX_MAP_LABELS_PER_LEVEL ; i ++ )
    {
	NewLevel -> labels [ i ] . pos . x = ( -1 ) ;
	NewLevel -> labels [ i ] . pos . y = ( -1 ) ;
	NewLevel -> labels [ i ] . label_name = "no_label_defined" ;
    }
    //--------------------
    // Now we add empty waypoint information...
    //
    NewLevel -> num_waypoints = 0 ;
    for ( i = 0 ; i < MAXWAYPOINTS ; i++ )
    {
	NewLevel -> AllWaypoints [ i ] . x = 0 ;
	NewLevel -> AllWaypoints [ i ] . y = 0 ;
	
	for ( k = 0 ; k < MAX_WP_CONNECTIONS ; k++ )
	{
	    NewLevel -> AllWaypoints [ i ] . connections [ k ] = -1 ;
	}
    }
    //--------------------
    // First we initialize the items arrays with 'empty' information
    //
    for ( i = 0 ; i < MAX_ITEMS_PER_LEVEL ; i ++ )
    {
	NewLevel -> ItemList [ i ] . pos.x = ( -1 ) ;
	NewLevel -> ItemList [ i ] . pos.y = ( -1 ) ;
	NewLevel -> ItemList [ i ] . type = ( -1 ) ;
	NewLevel -> ItemList [ i ] . currently_held_in_hand = FALSE;
	
    }
    //--------------------
    // Now we initialize the chest items arrays with 'empty' information
    //
    for ( i = 0 ; i < MAX_CHEST_ITEMS_PER_LEVEL ; i ++ )
    {
	NewLevel -> ChestItemList [ i ] . pos . x = ( -1 ) ;
	NewLevel -> ChestItemList [ i ] . pos . y = ( -1 ) ;
	NewLevel -> ChestItemList [ i ] . type = ( -1 ) ;
	NewLevel -> ChestItemList [ i ] . currently_held_in_hand = FALSE ;
    }
    
    curShip . AllLevels [ curShip.num_levels ] = NewLevel ;
    curShip . num_levels ++ ;
    
    glue_obstacles_to_floor_tiles_for_level ( NewLevel -> levelnum );
    
}; // void CreateNewMapLevel( void )

/* ----------------------------------------------------------------------
 * Now we print out the map label information about this map location.
 * ---------------------------------------------------------------------- */
void
PrintMapLabelInformationOfThisSquare ( Level EditLevel )
{
    int MapLabelIndex;
    char PanelText[5000]="";
    
    for ( MapLabelIndex = 0 ; MapLabelIndex < MAX_MAP_LABELS_PER_LEVEL ; MapLabelIndex ++ )
    {
	if ( ( fabsf ( Me . pos . x - ( EditLevel -> labels [ MapLabelIndex ] . pos . x + 0.5 ) ) <= 0.5 ) && 
	     ( fabsf ( Me . pos . y - ( EditLevel -> labels [ MapLabelIndex ] . pos . y + 0.5 ) ) <= 0.5 ) )
	    break;
    }
    
    if ( MapLabelIndex >= MAX_MAP_LABELS_PER_LEVEL ) return;
    
    sprintf( PanelText , _("\n Map label information: \n label_name=\"%s\".") , 
	     EditLevel -> labels [ MapLabelIndex ] . label_name );
    
    DisplayText ( PanelText , User_Rect.x , GameConfig.screen_height - 5 * FontHeight( GetCurrentFont() ) , 
            NULL/*&User_Rect*/ , 1.0 );
    
}; // void PrintMapLabelInformationOfThisSquare ( Level EditLevel )

/* ----------------------------------------------------------------------
 * This function is used by the Level Editor integrated into 
 * freedroid.  It highlights the map position that is currently 
 * edited or would be edited, if the user pressed something.  I.e. 
 * it provides a "cursor" for the Level Editor.
 * ---------------------------------------------------------------------- */
void 
Highlight_Current_Block (int mask)
{
    Level EditLevel;
    static iso_image level_editor_cursor = { NULL, 0, 0 };
    char fpath[2048];
    
  EditLevel = curShip.AllLevels[Me.pos.z];
#define HIGHLIGHTCOLOR 255
    
    //--------------------
    // Maybe, if the level editor floor cursor has not yet been loaded,
    // we need to load it.
    //
  if (level_editor_cursor.surface == NULL && ! level_editor_cursor . texture_has_been_created )
    {
      find_file ("level_editor_floor_cursor.png", GRAPHICS_DIR, fpath, 0);
      get_iso_image_from_file_and_path (fpath, &(level_editor_cursor), TRUE);
      if (level_editor_cursor.surface == NULL)
	{
          ErrorMessage (__FUNCTION__, "\
Unable to load the level editor floor cursor.", PLEASE_INFORM, IS_FATAL);
	}
      if (use_open_gl)
        make_texture_out_of_surface (&level_editor_cursor);
    }
    
  if (mask & ZOOM_OUT)
    {
      if (use_open_gl)
        draw_gl_textured_quad_at_map_position (&level_editor_cursor,
                                                     Me.pos.x, Me.pos.y,
                                                     1.0, 1.0, 1.0, 0,
                                                     FALSE, ONE_OVER_LEVEL_EDITOR_ZOOM_OUT_FACT);
      else
        blit_zoomed_iso_image_to_map_position (&level_editor_cursor,
                                               Me.pos.x, Me.pos.y);
    }
  else
    {
      if (use_open_gl)
        draw_gl_textured_quad_at_map_position (&level_editor_cursor,
                                              Me.pos.x, Me.pos.y, 1.0,
                                              1.0, 1.0, 0, FALSE, 1.0);
      else
        blit_iso_image_to_map_position (&level_editor_cursor, Me.pos.x,
                                        Me.pos.y);
    }
    
  PrintMapLabelInformationOfThisSquare (EditLevel);

} // void Highlight_Current_Block(void)

/* ----------------------------------------------------------------------
 * This function is used to draw a line between given map tiles.  It is
 * mainly used for the map editor to highlight connections and the 
 * current map tile target.
 * ---------------------------------------------------------------------- */
void 
draw_connection_between_tiles ( float x1 , float y1 , float x2 , float y2 , int mask )
{
    float steps;
    float dist;
    int i;
    static iso_image level_editor_dot_cursor = UNLOADED_ISO_IMAGE ;
    char fpath[2048];
    
    //--------------------
    // Maybe, if the level editor dot cursor has not yet been loaded,
    // we need to load it.
    //
    if ( ( level_editor_dot_cursor . surface == NULL ) && ( ! level_editor_dot_cursor . texture_has_been_created ) )
    {
	find_file ( "level_editor_waypoint_dot.png" , GRAPHICS_DIR, fpath, 0 );
	get_iso_image_from_file_and_path ( fpath , & ( level_editor_dot_cursor ) , TRUE ) ;
	if ( level_editor_dot_cursor . surface == NULL )
	{
	    ErrorMessage ( __FUNCTION__  , "\
Unable to load the level editor waypoint dot cursor.",
				       PLEASE_INFORM, IS_FATAL );
	}
	
	if ( use_open_gl )
	    make_texture_out_of_surface ( & level_editor_dot_cursor ) ;
    }
    
    //--------------------
    // So now that the dot cursor has been loaded, we can start to
    // actually draw the dots.
    //
    
    //--------------------
    // We measure the distance that we have to go and then we draw some
    // dots at some convex combinations of our two vectors.  Very fine.
    //
    dist = sqrt ( ( x1 - x2 ) * ( x1 - x2 ) + ( y1 - y2 ) * ( y1 - y2 ) );
    
    steps = dist * 4;  // let's say 4 dots per square to mark the line, ok?
    
    if (!steps) // Oh, noh, the points are at the same place
	return ;
    for ( i = 0 ; i < steps+1 ; i ++ )
    {
        float x,y;
        x=( ((float)i) / steps ) * x1 + x2 * ( steps - i )/steps;
        y=( ((float)i) / steps ) * y1 + y2 * ( steps - i )/steps;
	if ( mask & ZOOM_OUT )
	{
	    if ( use_open_gl )
		draw_gl_textured_quad_at_map_position ( &level_editor_dot_cursor , x , y ,
							      1.0 , 1.0 , 1.0 , 0.25 , FALSE, ONE_OVER_LEVEL_EDITOR_ZOOM_OUT_FACT);
	    else
		blit_iso_image_to_map_position ( &level_editor_dot_cursor , x , y);
	}
	else
	{
	    if ( use_open_gl )
		draw_gl_textured_quad_at_map_position ( &level_editor_dot_cursor , x , y ,
						       1.0 , 1.0 , 1.0 , TRUE , FALSE, 1.0);
	    else
		blit_iso_image_to_map_position ( &level_editor_dot_cursor , x , y);
	}
    }
    
}; // void draw_connection_between_tiles ( .... )

/* ----------------------------------------------------------------------
 * This function is used by the Level Editor integrated into 
 * freedroid.  It marks all waypoints with a cross.
 * ---------------------------------------------------------------------- */
void 
ShowWaypoints( int PrintConnectionList , int mask )
{
    int wp;
    int i;
    int BlockX, BlockY;
    char ConnectionText[5000];
    char TextAddition[1000];
    Level EditLevel;
    static iso_image level_editor_waypoint_cursor [ 2 ] = { UNLOADED_ISO_IMAGE , UNLOADED_ISO_IMAGE } ;
    char fpath[2048];
    waypoint *this_wp;
    
    EditLevel = curShip.AllLevels [ Me . pos . z ] ;
    
#define ACTIVE_WP_COLOR 0x0FFFFFFFF
    
    //--------------------
    // Maybe, if the level editor floor cursor has not yet been loaded,
    // we need to load it.
    //
    for ( i = 0 ; i < 2 ; i ++ )
    {
	if ( ( level_editor_waypoint_cursor [ i ] . surface == NULL ) && ( ! level_editor_waypoint_cursor [ i ] . texture_has_been_created ) )
	{
	    if ( i == 0 )
		find_file ( "level_editor_waypoint_cursor.png" , GRAPHICS_DIR, fpath, 0 );
	    else
		find_file ( "level_editor_norand_waypoint_cursor.png" , GRAPHICS_DIR, fpath, 0 );
	    get_iso_image_from_file_and_path ( fpath , & ( level_editor_waypoint_cursor [ i ] ) , TRUE ) ;
	    
	    if ( level_editor_waypoint_cursor [ i ] . surface == NULL )
	    {
		ErrorMessage ( __FUNCTION__  , "\
Unable to load the level editor waypoint cursor.",
					   PLEASE_INFORM, IS_FATAL );
	    }
	    
	    if ( use_open_gl )
		make_texture_out_of_surface ( & ( level_editor_waypoint_cursor [ i ] ) );
	}
    }
    
    BlockX = rintf ( Me . pos . x - 0.5 );
    BlockY = rintf ( Me . pos . y - 0.5 );
    
    for ( wp = 0 ; wp < EditLevel -> num_waypoints ; wp++ )
    {
	this_wp = &(EditLevel->AllWaypoints[wp]);
	if ( this_wp->x == 0 && this_wp->y == 0) continue;
	
	if ( mask && ZOOM_OUT )
	{
	    if ( use_open_gl )
	    {
		draw_gl_textured_quad_at_map_position ( 
                        &level_editor_waypoint_cursor [ this_wp -> suppress_random_spawn ] , 
							      this_wp->x + 0.5 , this_wp->y + 0.5 , 1.0 , 1.0 , 1.0 , 0.25, FALSE, ONE_OVER_LEVEL_EDITOR_ZOOM_OUT_FACT ) ;
	    }
	    else
	    {
		blit_zoomed_iso_image_to_map_position ( 
                        & ( level_editor_waypoint_cursor [ this_wp -> suppress_random_spawn ]  ) , 
							this_wp->x + 0.5 , this_wp->y + 0.5 ) ;
	    }
	}
	else
	{
	    if ( use_open_gl )
		draw_gl_textured_quad_at_map_position ( &level_editor_waypoint_cursor [ this_wp -> suppress_random_spawn ]  , 
						       this_wp->x + 0.5 , this_wp->y + 0.5 , 1.0 , 1.0 , 1.0 , 0 , FALSE, 1.0) ;
	    else
		blit_iso_image_to_map_position ( &level_editor_waypoint_cursor [ this_wp -> suppress_random_spawn ] , 
						 this_wp->x + 0.5 , this_wp->y + 0.5 ) ;
	}
	
	//--------------------
	// Draw the connections to other waypoints, BUT ONLY FOR THE WAYPOINT CURRENTLY TARGETED
	//
	if ( PrintConnectionList )
	{
	    strcpy( ConnectionText , "List of connection for this wp:\n" );
	}
	
	for ( i=0; i < this_wp->num_connections; i++ )
	{
	    if ( this_wp->connections[i] != (-1) )
	    {
		if ( ( BlockX == this_wp->x ) && ( BlockY == this_wp->y ) )
		{
		    // color = ACTIVE_WP_COLOR ;
		    // else color = HIGHLIGHTCOLOR ; 
		    // printf(" Found a connection!! ");
		    // printf_SDL ( Screen  , 100 , 100 , "Waypoint connection to: " );
		    
		    
		    //--------------------
		    // If this is desired, we also print a list of connections from
		    // this waypoint to other waypoints in text form...
		    //
		    if ( PrintConnectionList )
		    {
			SDL_UnlockSurface( Screen );
			sprintf ( TextAddition , "To: X=%d Y=%d    " , 
				  EditLevel->AllWaypoints[this_wp->connections[i]].x , 
				  EditLevel->AllWaypoints[this_wp->connections[i]].y);
			strcat ( ConnectionText , TextAddition );
			DisplayText ( ConnectionText , User_Rect.x , User_Rect.y , &User_Rect , 1.0 );
			SDL_LockSurface( Screen );
		    }
		    
		    draw_connection_between_tiles ( this_wp->x + 0.5, this_wp->y + 0.5 , 
						    EditLevel->AllWaypoints[this_wp->connections[i]].x + 0.5 , 
						    EditLevel->AllWaypoints[this_wp->connections[i]].y + 0.5 , mask );
		    
		}
	    }
	}
    }

    //--------------------
    // Now we do something extra:  If there is a connection attempt currently
    // going on, then we also draw a connection from the origin point to the
    // current cursor (i.e. 'me') position.
    //
    if ( OriginWaypoint != (-1) )
    {
	this_wp = & ( EditLevel -> AllWaypoints [ OriginWaypoint ] ) ;
	draw_connection_between_tiles ( this_wp->x + 0.5, this_wp->y + 0.5 , 
					Me . pos . x , 
					Me . pos . y , mask );
    }

}; // void ShowWaypoints( int PrintConnectionList );

/* ----------------------------------------------------------------------
 * This function is used by the Level Editor integrated into 
 * freedroid.  It marks all places that have a label attached to them.
 * ---------------------------------------------------------------------- */
void 
ShowMapLabels( int mask )
{
    int LabelNr;
    Level EditLevel;
    static iso_image map_label_indicator = UNLOADED_ISO_IMAGE ;
    static int first_function_call = TRUE ;
    char fpath[2048];
    EditLevel = curShip.AllLevels [ Me . pos . z ] ;
    
    //--------------------
    // On the first function call to this function, we must load the map label indicator
    // iso image from the disk to memory and keep it there as static.  That should be
    // it for here.
    //
    if ( first_function_call )
    {
	first_function_call = FALSE;
	find_file ( "level_editor_map_label_indicator.png" , GRAPHICS_DIR, fpath, 0);
	get_iso_image_from_file_and_path ( fpath , & ( map_label_indicator ) , TRUE );
	
	if ( use_open_gl ) 
	    make_texture_out_of_surface ( & map_label_indicator ) ;
    }
    
    //--------------------
    // Now we can draw a fine indicator at all the position nescessary...
    //
    for ( LabelNr = 0 ; LabelNr < MAX_MAP_LABELS_PER_LEVEL ; LabelNr ++ )
    {
	if ( EditLevel->labels[LabelNr].pos.x == (-1) ) continue;
	
	if ( ! ( mask && ZOOM_OUT ) )
	{
	    if ( use_open_gl )
		draw_gl_textured_quad_at_map_position ( &map_label_indicator , EditLevel -> labels [ LabelNr ] . pos . x + 0.5 , 
						       EditLevel -> labels [ LabelNr ] . pos . y + 0.5 , 1.0, 1.0 , 1.0 , FALSE , FALSE, 1.);
	    else
		blit_iso_image_to_map_position ( &map_label_indicator , EditLevel -> labels [ LabelNr ] . pos . x + 0.5 , 
						 EditLevel -> labels [ LabelNr ] . pos . y + 0.5 );
	}
	else
	{
	    if ( use_open_gl )
		draw_gl_textured_quad_at_map_position ( &map_label_indicator , EditLevel -> labels [ LabelNr ] . pos . x + 0.5 , 
							      EditLevel -> labels [ LabelNr ] . pos . y + 0.5 , 1.0 , 1.0 , 1.0 , 0.25, FALSE, ONE_OVER_LEVEL_EDITOR_ZOOM_OUT_FACT );
	    else
		blit_zoomed_iso_image_to_map_position ( & ( map_label_indicator ) , EditLevel -> labels [ LabelNr ] . pos . x + 0.5 , 
							EditLevel -> labels [ LabelNr ] . pos . y + 0.5 );
	}
    }
    
}; // void ShowMapLabels( void );

/* ----------------------------------------------------------------------
 *
 *
 * ---------------------------------------------------------------------- */
void
HandleMapTileEditingKeys ( Level EditLevel , int BlockX , int BlockY )
{
    if ( GameConfig . level_editor_edit_mode == LEVEL_EDITOR_SELECTION_FLOOR )
	return;

    if ( KP1Hit() ) 
    {
	action_create_obstacle_user ( EditLevel, ((int)Me.pos.x) , ((int)Me.pos.y) + 1.0, wall_indices [ GameConfig . level_editor_edit_mode ] [ Highlight ] );
    }
    if (KP2Hit()) 
    {
    	action_create_obstacle_user ( EditLevel,  ((int)Me.pos.x)+ 0.5 , ((int)Me.pos.y) + 1.0,  wall_indices [ GameConfig . level_editor_edit_mode ] [ Highlight ]);
    }
    if (KP3Hit()) 
    {
    	action_create_obstacle_user ( EditLevel,  ((int)Me.pos.x)+1.0 , ((int)Me.pos.y)+1.0,  wall_indices [ GameConfig . level_editor_edit_mode ] [ Highlight ]);
    }
    if (KP4Hit()) 
    {
    	action_create_obstacle_user ( EditLevel,  ((int)Me.pos.x), ((int)Me.pos.y) + 0.5,  wall_indices [ GameConfig . level_editor_edit_mode ] [ Highlight ]);
    }
    if (KP5Hit()) 
    {
    	action_create_obstacle_user ( EditLevel,  ((int)Me.pos.x) + 0.5 , ((int)Me.pos.y)+0.5,  wall_indices [ GameConfig . level_editor_edit_mode ] [ Highlight ]);
    }
    if (KP6Hit()) 
    {
    	action_create_obstacle_user ( EditLevel,  ((int)Me.pos.x)+ 1.0 , ((int)Me.pos.y) + 0.5,  wall_indices [ GameConfig . level_editor_edit_mode ] [ Highlight ]);
    }
    if (KP7Hit()) 
    {
    	action_create_obstacle_user ( EditLevel,  ((int)Me.pos.x), ((int)Me.pos.y),  wall_indices [ GameConfig . level_editor_edit_mode ] [ Highlight ]);
    }
    if ( KP8Hit() ) 
    {
    	action_create_obstacle_user ( EditLevel,  ((int)Me.pos.x)+ 0.5 , ((int)Me.pos.y),  wall_indices [ GameConfig . level_editor_edit_mode ] [ Highlight ]);
    }
    if (KP9Hit()) 
    {
    	action_create_obstacle_user ( EditLevel,  ((int)Me.pos.x) + 1.0 , ((int)Me.pos.y),  wall_indices [ GameConfig . level_editor_edit_mode ] [ Highlight ]);
    }
}; // void HandleMapTileEditingKeys ( Level EditLevel , int BlockX , int BlockY )

/* ----------------------------------------------------------------------
 *
 *
 * ---------------------------------------------------------------------- */
void 
HandleLevelEditorCursorKeys ( void )
{
    Level EditLevel;
    
    EditLevel = curShip.AllLevels [ Me . pos . z ] ;
    
    if (LeftPressed()) 
    {
	if ( rintf(Me.pos.x) > 0 ) Me.pos.x-=1;
	while (LeftPressed());
    }
    if (RightPressed()) 
    {
	if ( rintf(Me.pos.x) < EditLevel->xlen-1 ) Me.pos.x+=1;
	while (RightPressed());
    }
    if (UpPressed()) 
    {
	if ( rintf(Me.pos.y) > 0 ) Me.pos.y-=1;
	while (UpPressed());
    }
    if (DownPressed()) 
    {
	if ( rintf(Me.pos.y) < EditLevel->ylen-1 ) Me.pos.y+=1;
	while (DownPressed());
    }
}; // void HandleLevelEditorCursorKeys ( void )


/* ----------------------------------------------------------------------
 * When the mouse has rested idle on some mouse button in the level 
 * editor (and also tooltips are enabled) then some small window (a 
 * tooltip) will appear and describe the purpose of the button under the
 * mouse cursor.  Bringing up this tooltip window is the purpose of this
 * function.
 * ---------------------------------------------------------------------- */
void
show_button_tooltip ( char* tooltip_text )
{
    SDL_Rect TargetRect;
    
    TargetRect . w = 400 ; 
    TargetRect . h = 220 ; 
    TargetRect . x = ( 640 - TargetRect . w ) / 2 ; 
    TargetRect . y = 2 * ( 480 - TargetRect . h ) / 3 ; 
    our_SDL_fill_rect_wrapper ( Screen , &TargetRect , 
				SDL_MapRGB ( Screen->format, 0 , 0 , 0 ) ) ;
    
#define IN_WINDOW_TEXT_OFFSET 15
    TargetRect . w -= IN_WINDOW_TEXT_OFFSET;
    TargetRect . h -= IN_WINDOW_TEXT_OFFSET;
    TargetRect . x += IN_WINDOW_TEXT_OFFSET;
    TargetRect . y += IN_WINDOW_TEXT_OFFSET;
    
    SetCurrentFont ( FPS_Display_BFont );
    
    DisplayText ( tooltip_text, TargetRect . x, TargetRect . y , &TargetRect , 1.0 )  ;
    
}; // void show_button_tooltip ( char* tooltip_text )

/* ----------------------------------------------------------------------
 *
 *
 * ---------------------------------------------------------------------- */
void
show_level_editor_tooltips ( void )
{
    static float time_spent_on_some_button = 0 ;
    static float previous_function_call_time = 0 ;
    
    time_spent_on_some_button += SDL_GetTicks() - previous_function_call_time ; 
    
    previous_function_call_time = SDL_GetTicks();
    
#define TICKS_UNTIL_TOOLTIP 1200
    
    if ( MouseCursorIsOnButton ( GO_LEVEL_NORTH_BUTTON , GetMousePos_x()  , GetMousePos_y()  ) )
    {
	if ( time_spent_on_some_button > TICKS_UNTIL_TOOLTIP )
	    show_button_tooltip ( _("Go level north\n\nUse this button to move one level north, i.e. to the level that is glued to the northern side of this level." ));
    }
    else if ( MouseCursorIsOnButton ( GO_LEVEL_SOUTH_BUTTON , GetMousePos_x()  , GetMousePos_y()  ) )
    {
	if ( time_spent_on_some_button > TICKS_UNTIL_TOOLTIP )
	    show_button_tooltip ( _("Go level south\n\nUse this button to move one level south, i.e. to the level that is glued to the southern side of this level." ));
    }
    else if ( MouseCursorIsOnButton ( GO_LEVEL_EAST_BUTTON , GetMousePos_x()  , GetMousePos_y()  ) )
    {
	if ( time_spent_on_some_button > TICKS_UNTIL_TOOLTIP )
	    show_button_tooltip ( _("Go level east\n\nUse this button to move one level east, i.e. to the level that is glued to the eastern side of this level." ));
    }
    else if ( MouseCursorIsOnButton ( GO_LEVEL_WEST_BUTTON , GetMousePos_x()  , GetMousePos_y()  ) )
    {
	if ( time_spent_on_some_button > TICKS_UNTIL_TOOLTIP )
	    show_button_tooltip ( _("Go level west\n\nUse this button to move one level west, i.e. to the level that is glued to the western side of this level." ));
    }
    else if ( MouseCursorIsOnButton ( EXPORT_THIS_LEVEL_BUTTON , GetMousePos_x()  , GetMousePos_y()  ) )
    {
	if ( time_spent_on_some_button > TICKS_UNTIL_TOOLTIP )
	    show_button_tooltip ( _("Export this level\n\nIn FreedroidRPG maps can be glued together to form one big map.  But that requires that the maps are identical where they overlap.  This button will copy the borders of this level to the borders of the neighbouring levels, so that the maps are in sync again." ));
    }
    else if ( MouseCursorIsOnButton ( LEVEL_EDITOR_SAVE_SHIP_BUTTON , GetMousePos_x()  , GetMousePos_y()  ) )
    {
	if ( time_spent_on_some_button > TICKS_UNTIL_TOOLTIP )
	    show_button_tooltip ( _("Save Map\n\nThis button will save your current map over the file '../map/Asteroid.maps' from your current working directory." ));
    }    
    else if ( MouseCursorIsOnButton ( LEVEL_EDITOR_TOGGLE_WAYPOINT_BUTTON , GetMousePos_x()  , GetMousePos_y()  ) )
    {
	if ( time_spent_on_some_button > TICKS_UNTIL_TOOLTIP )
	    show_button_tooltip ( _("Toggle waypoints\n\nUse this button to toggle waypoints on the current cursor location.  Waypoints are marked with a white (or multicolored) 4-direction arrow.\n\nYou can also use the W key for this." ));
    }
    else if ( MouseCursorIsOnButton ( LEVEL_EDITOR_TOGGLE_CONNECTION_BLUE_BUTTON , GetMousePos_x()  , GetMousePos_y()  ) )
    {
	if ( time_spent_on_some_button > TICKS_UNTIL_TOOLTIP )
	{
	    if ( OriginWaypoint == (-1) )
		show_button_tooltip ( _("Make waypoint connection\n\nUse this button to start a connection between waypoints, so that droids can move along this connection.\n\nYou can also use the C key for this." ));
	    else
		show_button_tooltip ( _("Complete waypoint connection\n\nYou have currently a connection attempt going on already.  Go to the desired destination waypoint and hit this button again to establish the connection.\n\nYou can also use the C key for this." ));
	}
    }
    else if ( MouseCursorIsOnButton ( LEVEL_EDITOR_DELETE_OBSTACLE_BUTTON , GetMousePos_x()  , GetMousePos_y()  ) )
    {
	if ( time_spent_on_some_button > TICKS_UNTIL_TOOLTIP )
	    show_button_tooltip ( _("Delete selected obstacle\n\nUse this button to delete the currently marked obstacle.\n\nYou can also use the X key for this." ));
    }
    else if ( MouseCursorIsOnButton ( LEVEL_EDITOR_NEXT_OBSTACLE_BUTTON , GetMousePos_x()  , GetMousePos_y()  ) )
    {
	if ( time_spent_on_some_button > TICKS_UNTIL_TOOLTIP )
	    show_button_tooltip ( _("Next obstacle on currently selected tile\n\nUse this button to cycle the currently marked obstacle on this tile.\n\nYou can also use the N key for this." ));
    }
    else if ( MouseCursorIsOnButton ( LEVEL_EDITOR_BEAUTIFY_GRASS_BUTTON , GetMousePos_x()  , GetMousePos_y()  ) )
    {
	if ( time_spent_on_some_button > TICKS_UNTIL_TOOLTIP )
	    show_button_tooltip ( _("Beautify grass button\n\nUse this button to 'beautify' rough edges of the grass-sand tiles on this entire level automatically.  The function will attempt to create 'round' borders and corners.\n\nYou can also use Ctrl-B for this." ));
    }
    else if ( MouseCursorIsOnButton ( LEVEL_EDITOR_ZOOM_OUT_BUTTON , GetMousePos_x()  , GetMousePos_y()  ) ||
              MouseCursorIsOnButton ( LEVEL_EDITOR_ZOOM_IN_BUTTON , GetMousePos_x()  , GetMousePos_y()  ) )
    {
	if ( time_spent_on_some_button > TICKS_UNTIL_TOOLTIP )
	    show_button_tooltip ( _("Zoom in/out\n\nUse this button to zoom INTO or OUT of the level.\n\nYou can also use the hotkey 'O' for this." ));
    }
    else if ( MouseCursorIsOnButton ( LEVEL_EDITOR_RECURSIVE_FILL_BUTTON , GetMousePos_x()  , GetMousePos_y()  ) )
    {
	if ( time_spent_on_some_button > TICKS_UNTIL_TOOLTIP )
	    show_button_tooltip ( _("Fill\n\nUse this button to fill a concurrent area of the map with the currently selected map tile.  Filling will proceed from the cursor in all directions until a change of map tile is encountered." ));
    }
    else if ( MouseCursorIsOnButton ( LEVEL_EDITOR_NEW_OBSTACLE_LABEL_BUTTON , GetMousePos_x()  , GetMousePos_y()  ) )
    {
	if ( time_spent_on_some_button > TICKS_UNTIL_TOOLTIP )
	    show_button_tooltip ( _("New obstacle label\n\nUse this button to attach a label to the currently marked obstacle.  These obstacle labels can be used to define obstacles to be modified by events.\n Note that you can also use the hotkey 'h' for this." ));
    }
    else if ( MouseCursorIsOnButton ( LEVEL_EDITOR_NEW_OBSTACLE_DESCRIPTION_BUTTON , GetMousePos_x()  , GetMousePos_y()  ) )
    {
    if ( time_spent_on_some_button > TICKS_UNTIL_TOOLTIP )
	    show_button_tooltip ( _("New obstacle description\n\nedits the description seen when the item is selected" ));
    }
    else if ( MouseCursorIsOnButton ( LEVEL_EDITOR_NEW_MAP_LABEL_BUTTON , GetMousePos_x()  , GetMousePos_y()  ) )
    {
	if ( time_spent_on_some_button > TICKS_UNTIL_TOOLTIP )
	    show_button_tooltip ( _("New map label\n\nUse this button to attach a new map label to the current cursor position.  These map labels can be used to define starting points for bots and characters or also to define locations for events and triggers." ));
    }
    else if ( MouseCursorIsOnButton ( LEVEL_EDITOR_NEW_ITEM_BUTTON , GetMousePos_x()  , GetMousePos_y()  ) )
    {
	if ( time_spent_on_some_button > TICKS_UNTIL_TOOLTIP )
	    show_button_tooltip ( _("Add item\n\nUse this button to drop a new item to the floor.  You can also use the hotkey 'G' for this." ));
    }
    else if ( MouseCursorIsOnButton ( LEVEL_EDITOR_ESC_BUTTON , GetMousePos_x()  , GetMousePos_y()  ) )
    {
	if ( time_spent_on_some_button > TICKS_UNTIL_TOOLTIP )
	    show_button_tooltip ( _("Menu\n\nUse this button to enter the main menu of the level editor.\n Note, that you can also use the Escape key to enter the level editor main menu." ));
    }
    else if ( MouseCursorIsOnButton ( LEVEL_EDITOR_LEVEL_RESIZE_BUTTON , GetMousePos_x()  , GetMousePos_y()  ) )
    {
	if ( time_spent_on_some_button > TICKS_UNTIL_TOOLTIP )
	    show_button_tooltip ( _("Level resize\n\nUse this button to enter the level resize menu.  Levels can be resized in various ways so as not to destroy your current map too much and so as to insert the new space where you would best like it to be." ));
    }
    else if ( MouseCursorIsOnButton ( LEVEL_EDITOR_KEYMAP_BUTTON , GetMousePos_x()  , GetMousePos_y()  ) )
    {
	if ( time_spent_on_some_button > TICKS_UNTIL_TOOLTIP )
	    show_button_tooltip ( _("Level editor guide\n\nUse this button to enter the level editor keymap display." ));
    }
    else if ( MouseCursorIsOnButton ( LEVEL_EDITOR_QUIT_BUTTON , GetMousePos_x()  , GetMousePos_y()  ) )
    {
	if ( time_spent_on_some_button > TICKS_UNTIL_TOOLTIP )
	    show_button_tooltip ( _("Back to game\n\nQuits the level editor and takes you into a FredroidRPG game." ));
    }
    else if ( MouseCursorIsOnButton ( LEVEL_EDITOR_UNDERGROUND_LIGHT_ON_BUTTON , GetMousePos_x()  , GetMousePos_y()  ) )
    {
	if ( time_spent_on_some_button > TICKS_UNTIL_TOOLTIP )
	    show_button_tooltip ( _("Underground lighting\n\nUse this button to toggle underground lighting, i.e. shadows coming from the light that the tux emanates.\n\nThere isn't any key to toggle this on the keyboard." ));
    }
    else if ( MouseCursorIsOnButton ( LEVEL_EDITOR_TOGGLE_ENEMIES_BUTTON , GetMousePos_x()  , GetMousePos_y()  ) ||
	      MouseCursorIsOnButton ( LEVEL_EDITOR_TOGGLE_ENEMIES_BUTTON_OFF , GetMousePos_x()  , GetMousePos_y()  ) )
    {
	if ( time_spent_on_some_button > TICKS_UNTIL_TOOLTIP )
	    show_button_tooltip ( _("Toggle display enemies\n\nUse this button to toggle between enemies displayed in level editor or enemies hidden in level editor." ));
    }
    else if ( MouseCursorIsOnButton ( LEVEL_EDITOR_TOGGLE_OBSTACLES_BUTTON , GetMousePos_x()  , GetMousePos_y()  ) ||
	      MouseCursorIsOnButton ( LEVEL_EDITOR_TOGGLE_OBSTACLES_BUTTON_OFF , GetMousePos_x()  , GetMousePos_y()  ) )
    {
	if ( time_spent_on_some_button > TICKS_UNTIL_TOOLTIP )
	    show_button_tooltip ( _("Toggle display obstacles\n\nUse this button to toggle between obstacles displayed in level editor or obstacles hidden in level editor." ));
    }
    else if ( MouseCursorIsOnButton ( LEVEL_EDITOR_UNDO_BUTTON , GetMousePos_x()  , GetMousePos_y()  ) ||
	      MouseCursorIsOnButton ( LEVEL_EDITOR_UNDO_BUTTON_PUSHED , GetMousePos_x()  , GetMousePos_y()  ) )
    {
	if ( time_spent_on_some_button > TICKS_UNTIL_TOOLTIP && to_undo.next != &to_undo)
	    show_button_tooltip ( _("Undo\n\nUse this button to undo your last actions." ));
    }
    else if ( MouseCursorIsOnButton ( LEVEL_EDITOR_REDO_BUTTON , GetMousePos_x()  , GetMousePos_y()  ) ||
	      MouseCursorIsOnButton ( LEVEL_EDITOR_REDO_BUTTON_PUSHED , GetMousePos_x()  , GetMousePos_y()  ) )
    {
	if ( time_spent_on_some_button > TICKS_UNTIL_TOOLTIP && to_redo.next != &to_redo)
	    show_button_tooltip ( _("Redo\n\nUse this button to redo an action." ));
    }
    else if ( MouseCursorIsOnButton ( LEVEL_EDITOR_TOGGLE_TUX_BUTTON , GetMousePos_x()  , GetMousePos_y()  ) ||
	      MouseCursorIsOnButton ( LEVEL_EDITOR_TOGGLE_TUX_BUTTON_OFF , GetMousePos_x()  , GetMousePos_y()  ) )
    {
	if ( time_spent_on_some_button > TICKS_UNTIL_TOOLTIP )
	    show_button_tooltip ( _("Toggle display Tux\n\nUse this button to toggle between Tux displayed in level editor or Tux hidden in level editor." ));
    }
    else if ( MouseCursorIsOnButton ( LEVEL_EDITOR_TOGGLE_TOOLTIPS_BUTTON , GetMousePos_x()  , GetMousePos_y()  ) ||
	      MouseCursorIsOnButton ( LEVEL_EDITOR_TOGGLE_TOOLTIPS_BUTTON_OFF , GetMousePos_x()  , GetMousePos_y()  ) )
    {
	if ( time_spent_on_some_button > TICKS_UNTIL_TOOLTIP )
	    show_button_tooltip ( _("Toggle display tooltips\n\nUse this button to toggle these annoying help windows on and off." ));
    }
    else if ( MouseCursorIsOnButton ( LEVEL_EDITOR_TOGGLE_COLLISION_RECTS_BUTTON , GetMousePos_x()  , GetMousePos_y()  ) ||
	      MouseCursorIsOnButton ( LEVEL_EDITOR_TOGGLE_COLLISION_RECTS_BUTTON_OFF , GetMousePos_x()  , GetMousePos_y()  ) )
    {
	if ( time_spent_on_some_button > TICKS_UNTIL_TOOLTIP )
	    show_button_tooltip ( _("Toggle display colision rectangles\n\nUse this button to toggle the visible collision rectangles on and off." ));
    }   
    else if ( MouseCursorIsOnButton (  LEVEL_EDITOR_TOGGLE_GRID_BUTTON , GetMousePos_x()  , GetMousePos_y())    ||
            MouseCursorIsOnButton (  LEVEL_EDITOR_TOGGLE_GRID_BUTTON_FULL , GetMousePos_x()  , GetMousePos_y()) ||
            MouseCursorIsOnButton (  LEVEL_EDITOR_TOGGLE_GRID_BUTTON_OFF , GetMousePos_x()  , GetMousePos_y())  )
    {
        if ( time_spent_on_some_button > TICKS_UNTIL_TOOLTIP )
            show_button_tooltip ( _("Change grid mode ( placement / full / off )" ));
    }
    else
    {
	time_spent_on_some_button = 0 ;
    }
    
}; // void show_level_editor_tooltips ( void )

/* ----------------------------------------------------------------------
 *
 *
 * ---------------------------------------------------------------------- */
int
marked_obstacle_is_glued_to_here ( Level EditLevel , float x , float y )
{
    int j;
    int current_mark_index = (-1);
    
    if ( level_editor_marked_obstacle == NULL ) return ( FALSE );
    
    for ( j = 0 ; j < MAX_OBSTACLES_GLUED_TO_ONE_MAP_TILE ; j ++ )
    {
	if ( level_editor_marked_obstacle == & ( EditLevel -> obstacle_list [ EditLevel -> map [ (int)y ] [ (int)x ] . obstacles_glued_to_here [ j ] ] ) )
	current_mark_index = j ;
    }
    
    if ( current_mark_index != (-1) ) return ( TRUE );
    return ( FALSE );
    
}; // int marked_obstacle_is_glued_to_here ( Me . pos . x , Me . pos . y )


/* ----------------------------------------------------------------------
 * This function should assign a new individual obstacle description to a 
 * single instance of a given obstacle type on a given level.  
 *
 * New indices must be found and the user must be queried for his input 
 * about the desired description text for the obstacle.
 *
 * ---------------------------------------------------------------------- */
void
give_new_description_to_obstacle ( Level EditLevel , obstacle* our_obstacle , char* predefined_description )
{
    int i;
    int free_index=(-1);
    
    //--------------------
    // If the obstacle already has a name, we can use that index for the 
    // new name now.
    //
    if ( our_obstacle -> name_index >= 0 )
	free_index = our_obstacle -> name_index ;
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
	    GetEditableStringInPopupWindow ( 1000 , "\n\
Please enter new description text for this obstacle: \n\n" ,
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
    
}; // void give_new_description_to_obstacle ( EditLevel , level_editor_marked_obstacle )


/* ----------------------------------------------------------------------
 * In an effort to reduce the massive size of the level editor main
 * function, we take the left mouse button handling out into a separate
 * function now.
 * ---------------------------------------------------------------------- */
int
level_editor_handle_left_mouse_button ( int proceed_now )
{
    static int LeftMousePressedPreviousFrame = FALSE;
    int new_x, new_y;
    
    if ( MouseLeftPressed() && !LeftMousePressedPreviousFrame )
    {
	if ( ClickWasInEditorBannerRect() )
	    HandleBannerMouseClick();
	else if ( MouseCursorIsOnButton ( GO_LEVEL_NORTH_BUTTON , GetMousePos_x()  , GetMousePos_y()  ) )
	{
	    if ( Me . pos . x < curShip . AllLevels [ EditLevel -> jump_target_north ] -> xlen -1 )
		new_x = Me . pos . x ; 
	    else 
		new_x = 3;
	    new_y = curShip . AllLevels [ EditLevel -> jump_target_north ] -> xlen - 4 ;
	    if ( EditLevel -> jump_target_north >= 0 ) 
		Teleport ( EditLevel -> jump_target_north , new_x , new_y , FALSE );
	}
	else if ( MouseCursorIsOnButton ( GO_LEVEL_SOUTH_BUTTON , GetMousePos_x()  , GetMousePos_y()  ) )
	{
	    if ( Me . pos . x < curShip . AllLevels [ EditLevel -> jump_target_south ] -> xlen -1 )
		new_x = Me . pos . x ; 
	    else 
		new_x = 3;
	    new_y = 4;
	    if ( EditLevel -> jump_target_south >= 0 ) 
		Teleport ( EditLevel -> jump_target_south , new_x , new_y , FALSE );
	}
	else if ( MouseCursorIsOnButton ( GO_LEVEL_EAST_BUTTON , GetMousePos_x()  , GetMousePos_y()  ) )
	{
	    new_x = 3;
	    if ( Me . pos . y < curShip . AllLevels [ EditLevel -> jump_target_east ] -> ylen -1 )
		new_y = Me . pos . y ; 
	    else 
		new_y = 4;
	    if ( EditLevel -> jump_target_east >= 0 ) 
		Teleport ( EditLevel -> jump_target_east , new_x , new_y , FALSE );
	}
	else if ( MouseCursorIsOnButton ( GO_LEVEL_WEST_BUTTON , GetMousePos_x()  , GetMousePos_y()  ) )
	{
	    new_x = curShip . AllLevels [ EditLevel -> jump_target_west ] -> xlen -4 ;
	    if ( Me . pos . y < curShip . AllLevels [ EditLevel -> jump_target_west ] -> ylen -1 )
		new_y = Me . pos . y ; 
	    else 
		new_y = 4;
	    if ( EditLevel -> jump_target_west >= 0 ) 
		Teleport ( EditLevel -> jump_target_west , new_x , new_y , FALSE );
	}
	else if ( MouseCursorIsOnButton ( EXPORT_THIS_LEVEL_BUTTON , GetMousePos_x()  , GetMousePos_y()  ) )
	{
	    ExportLevelInterface ( Me . pos . z );
	}
	else if ( MouseCursorIsOnButton ( LEVEL_EDITOR_UNDERGROUND_LIGHT_ON_BUTTON , GetMousePos_x()  , GetMousePos_y()  ) )
	{
	    EditLevel -> use_underground_lighting = ! EditLevel -> use_underground_lighting ;
	    while ( MouseLeftPressed() );
	}
	else if ( MouseCursorIsOnButton ( LEVEL_EDITOR_UNDO_BUTTON , GetMousePos_x()  , GetMousePos_y()  ) )
	{
	    action_undo ( EditLevel );
	    while ( MouseLeftPressed() );
	}
	else if ( MouseCursorIsOnButton ( LEVEL_EDITOR_REDO_BUTTON , GetMousePos_x()  , GetMousePos_y()  ) )
	{
	    action_redo ( EditLevel );
	    while ( MouseLeftPressed() );
	}
	else if ( MouseCursorIsOnButton ( LEVEL_EDITOR_SAVE_SHIP_BUTTON , GetMousePos_x()  , GetMousePos_y()  ) )
	{
	    close_all_chests_on_level ( Me . pos . z ) ;
	    char fp[2048];
	    find_file("Asteroid.maps", MAP_DIR, fp, 0);
	    SaveShip(fp);
	    
	    // CenteredPutString ( Screen ,  11*FontHeight(Menu_BFont),    "Your ship was saved...");
	    // our_SDL_flip_wrapper ( Screen );
	    
	    GiveMouseAlertWindow ( "\nM E S S A G E\n\nYour ship was saved to file 'Asteroids.map' in the map directory.\n\nIf you have set up something cool and you wish to contribute it to FreedroidRPG, please contact the FreedroidRPG dev team." ) ;
	    
	}
	else if ( GameConfig . zoom_is_on && MouseCursorIsOnButton ( LEVEL_EDITOR_ZOOM_IN_BUTTON , GetMousePos_x()  , GetMousePos_y()  ) )
	{
	    GameConfig . zoom_is_on = !GameConfig . zoom_is_on ;
	    while ( MouseLeftPressed() );
	}
	else if ( !GameConfig . zoom_is_on && MouseCursorIsOnButton ( LEVEL_EDITOR_ZOOM_OUT_BUTTON , GetMousePos_x()  , GetMousePos_y()  ) )
	{
	    GameConfig . zoom_is_on = !GameConfig . zoom_is_on ;
	    while ( MouseLeftPressed() );
	}
	else if ( MouseCursorIsOnButton ( LEVEL_EDITOR_TOGGLE_WAYPOINT_BUTTON , GetMousePos_x()  , GetMousePos_y()  ) )
	{
	    action_toggle_waypoint ( EditLevel , BlockX, BlockY , FALSE );
	    while ( MouseLeftPressed() || SpacePressed() );
	}
	else if ( MouseCursorIsOnButton ( LEVEL_EDITOR_TOGGLE_CONNECTION_BLUE_BUTTON , GetMousePos_x()  , GetMousePos_y()  ) )
	{
	    action_toggle_waypoint_connection_user ( EditLevel, BlockX, BlockY );
	    while ( MouseLeftPressed() );
	}
	else if ( MouseCursorIsOnButton ( LEVEL_EDITOR_BEAUTIFY_GRASS_BUTTON , GetMousePos_x()  , GetMousePos_y()  ) )
	{
	    beautify_grass_tiles_on_level ( EditLevel );
	}
	else if ( MouseCursorIsOnButton ( LEVEL_EDITOR_DELETE_OBSTACLE_BUTTON , GetMousePos_x()  , GetMousePos_y()  ) )
	{
	    if ( level_editor_marked_obstacle != NULL )
	    {
		action_remove_obstacle_user ( EditLevel , level_editor_marked_obstacle );
		level_editor_marked_obstacle = NULL ;
		while ( SpacePressed() || MouseLeftPressed() ) SDL_Delay(1); 
	    }
	}
	else if ( MouseCursorIsOnButton ( LEVEL_EDITOR_NEXT_OBSTACLE_BUTTON , GetMousePos_x()  , GetMousePos_y()  ) )
	{
	    cycle_marked_obstacle( EditLevel );
	}
	else if ( MouseCursorIsOnButton ( LEVEL_EDITOR_RECURSIVE_FILL_BUTTON , GetMousePos_x()  , GetMousePos_y()  ) )
	{
	    action_fill_user ( EditLevel , BlockX , BlockY , Highlight );
	}
	else if ( MouseCursorIsOnButton ( LEVEL_EDITOR_NEW_OBSTACLE_LABEL_BUTTON , GetMousePos_x()  , GetMousePos_y()  ) )
	{
	    if ( level_editor_marked_obstacle != NULL )
	    {
		action_change_obstacle_label_user ( EditLevel , level_editor_marked_obstacle , NULL );
		while ( SpacePressed() || MouseLeftPressed()) SDL_Delay(1);
	    }
	}
	else if ( MouseCursorIsOnButton ( LEVEL_EDITOR_NEW_OBSTACLE_DESCRIPTION_BUTTON , GetMousePos_x()  , GetMousePos_y()  ) )
	{
	    if ( level_editor_marked_obstacle != NULL )
	    {
		give_new_description_to_obstacle ( EditLevel , level_editor_marked_obstacle , NULL );
		while ( SpacePressed() || MouseLeftPressed()) SDL_Delay(1);
	    }
	}
	else if ( MouseCursorIsOnButton ( LEVEL_EDITOR_NEW_MAP_LABEL_BUTTON , GetMousePos_x()  , GetMousePos_y()  ) )
	{
	    action_change_map_label_user ( EditLevel );
	}
	else if ( MouseCursorIsOnButton ( LEVEL_EDITOR_NEW_ITEM_BUTTON , GetMousePos_x()  , GetMousePos_y()  ) )
	{
	    ItemDropFromLevelEditor(  );
	}
	else if ( MouseCursorIsOnButton ( LEVEL_EDITOR_ESC_BUTTON , GetMousePos_x()  , GetMousePos_y()  ) )
	{
	    main_menu_requested = TRUE ;
	    while ( SpacePressed() || MouseLeftPressed()) SDL_Delay(1);
	}
	else if ( MouseCursorIsOnButton ( LEVEL_EDITOR_LEVEL_RESIZE_BUTTON , GetMousePos_x()  , GetMousePos_y()  ) )
	{
	    EditLevelDimensions (  );
	}
	else if ( MouseCursorIsOnButton ( LEVEL_EDITOR_KEYMAP_BUTTON , GetMousePos_x()  , GetMousePos_y()  ) )
	{
	    ShowLevelEditorKeymap (  );
	}
	else if ( MouseCursorIsOnButton ( LEVEL_EDITOR_TOGGLE_TUX_BUTTON , GetMousePos_x()  , GetMousePos_y()  ) ||
		  MouseCursorIsOnButton ( LEVEL_EDITOR_TOGGLE_TUX_BUTTON_OFF , GetMousePos_x()  , GetMousePos_y()  ) )
	{
	    GameConfig . omit_tux_in_level_editor = ! GameConfig . omit_tux_in_level_editor ;
	}
	else if ( MouseCursorIsOnButton ( LEVEL_EDITOR_TOGGLE_ENEMIES_BUTTON , GetMousePos_x()  , GetMousePos_y()  ) ||
		  MouseCursorIsOnButton ( LEVEL_EDITOR_TOGGLE_ENEMIES_BUTTON_OFF , GetMousePos_x()  , GetMousePos_y()  ))
	{
	    GameConfig . omit_enemies_in_level_editor = ! GameConfig . omit_enemies_in_level_editor ;
	}
	else if ( MouseCursorIsOnButton ( LEVEL_EDITOR_TOGGLE_OBSTACLES_BUTTON , GetMousePos_x()  , GetMousePos_y()  ) ||
		  MouseCursorIsOnButton ( LEVEL_EDITOR_TOGGLE_OBSTACLES_BUTTON_OFF , GetMousePos_x()  , GetMousePos_y()  ) )
	{
	    GameConfig . omit_obstacles_in_level_editor = ! GameConfig . omit_obstacles_in_level_editor ;
	}
	else if ( MouseCursorIsOnButton ( LEVEL_EDITOR_TOGGLE_TOOLTIPS_BUTTON , GetMousePos_x()  , GetMousePos_y()  ) ||
		  MouseCursorIsOnButton ( LEVEL_EDITOR_TOGGLE_TOOLTIPS_BUTTON_OFF , GetMousePos_x()  , GetMousePos_y()  ) )
	{
	    GameConfig . show_tooltips = ! GameConfig . show_tooltips ;
	}
	else if ( MouseCursorIsOnButton ( LEVEL_EDITOR_TOGGLE_COLLISION_RECTS_BUTTON , GetMousePos_x()  , GetMousePos_y()  ) ||
		  MouseCursorIsOnButton ( LEVEL_EDITOR_TOGGLE_COLLISION_RECTS_BUTTON_OFF , GetMousePos_x()  , GetMousePos_y()  ) )
	{
	    draw_collision_rectangles = ! draw_collision_rectangles;
	}
	else if ( MouseCursorIsOnButton ( LEVEL_EDITOR_TOGGLE_GRID_BUTTON , GetMousePos_x()  , GetMousePos_y()  ) ||
		  MouseCursorIsOnButton ( LEVEL_EDITOR_TOGGLE_GRID_BUTTON_FULL , GetMousePos_x()  , GetMousePos_y()  ) ||
		  MouseCursorIsOnButton ( LEVEL_EDITOR_TOGGLE_GRID_BUTTON_OFF , GetMousePos_x()  , GetMousePos_y()  ) )
	{
	    draw_grid = (draw_grid+1) % 3;
	}
	else if ( MouseCursorIsOnButton ( LEVEL_EDITOR_QUIT_BUTTON , GetMousePos_x()  , GetMousePos_y()  ) )
	{
	    proceed_now=!proceed_now;
	    level_editor_done = TRUE;
	    Me . mouse_move_target . x = Me . pos . x ;
	    Me . mouse_move_target . y = Me . pos . y ;
	    Me . mouse_move_target . z = Me . pos . z ;
	    enemy_set_reference(&Me . current_enemy_target_n, &Me . current_enemy_target_addr, NULL);
	}
	else
	{
	    //--------------------
	    // Maybe a left mouse click has in the map area.  Then it might be best to interpret this
	    // simply as bigger move command, which might indeed be much handier than 
	    // using only keyboard cursor keys to move around on the map.
	    //
	    if ( GameConfig . zoom_is_on )
		Me . pos . x = 
		    translate_pixel_to_zoomed_map_location ( (float) GetMousePos_x()  - ( GameConfig . screen_width / 2 ) , 
							     (float) GetMousePos_y()  - ( GameConfig . screen_height / 2 ), TRUE ); 
	    else
		Me . pos . x = 
		    translate_pixel_to_map_location ( (float) GetMousePos_x()  - ( GameConfig . screen_width / 2 ) , 
						      (float) GetMousePos_y()  - ( GameConfig . screen_height / 2 ), TRUE ); 
	    
	    if ( Me . pos . x >= curShip.AllLevels[Me.pos.z]->xlen-1 )
		Me . pos . x = curShip.AllLevels[Me.pos.z]->xlen-1 ;
	    if ( Me . pos . x <= 0 ) Me . pos . x = 0;
	    
	    if ( GameConfig . zoom_is_on )
		Me . pos . y = 
		    translate_pixel_to_zoomed_map_location ( (float) GetMousePos_x()  - ( GameConfig . screen_width / 2 ), 
							     (float) GetMousePos_y()  - ( GameConfig . screen_height / 2 ), FALSE );
	    else
		Me . pos . y = 
		    translate_pixel_to_map_location ( (float) GetMousePos_x()  - ( GameConfig . screen_width / 2 ), 
						      (float) GetMousePos_y()  - ( GameConfig . screen_height / 2 ), FALSE );
	    if ( Me . pos . y >= curShip.AllLevels[Me.pos.z]->ylen-1 )
		Me . pos . y = curShip.AllLevels[Me.pos.z]->ylen-1 ;
	    if ( Me . pos . y <= 0 ) Me . pos . y = 0;
	}
    }

    LeftMousePressedPreviousFrame = MouseLeftPressed(); 

    return ( proceed_now );

}; // void level_editor_handle_left_mouse_button ( void )


/*---------------------------------------------------------------------
 * Begins a new line of walls
 * -------------------------------------------------------------------- */
void start_line_mode(whole_line *walls, moderately_finepoint TargetSquare, 
	int already_defined)
{
    // Initialize a line
    INIT_LIST_HEAD(&(walls->elements.list));
    walls->activated = TRUE;
    walls->direction = UNDEFINED;
    /* If the tile is not already defined (ie. if the function is not 
     * called from the quickbar_click function) */
    if (! already_defined)
    {
	walls->editor_mode = GameConfig . level_editor_edit_mode;
	walls->id = Highlight;
    }
    walls->elements.position.x = (int)TargetSquare.x + ((wall_orientation(wall_indices [ walls->editor_mode ] [ walls->id ]) == HORIZONTAL) ? 0.5 : 0);
    walls->elements.position.y = (int)TargetSquare.y + ((wall_orientation(wall_indices [ walls->editor_mode ] [ walls->id ]) == HORIZONTAL) ? 0 : 0.5);
    walls->elements.address = action_create_obstacle_user ( EditLevel , 
	    walls->elements.position.x , walls->elements.position.y , 
	    wall_indices [ walls->editor_mode ] [ walls->id ] );
}

/*----------------------------------------------------------------------
 * This function handles the line mode; adds a wall, or starts line mode
 * --------------------------------------------------------------------- */
void handle_line_mode(whole_line *walls, moderately_finepoint TargetSquare)
{ 
    if(! walls->activated) {
	start_line_mode(walls, TargetSquare, FALSE);	
    } else {
	line_element *wall;
	int actual_direction;
	float distance;
	moderately_finepoint pos_last;
	moderately_finepoint offset;
	int direction_is_possible;

	wall = list_entry((walls->elements).list.prev, line_element, list);
	pos_last = wall->position;
	distance = calc_euklid_distance(TargetSquare.x, TargetSquare.y, pos_last.x, pos_last.y);

	// Let's calculate the difference of position since last time
	offset.x = pos_last.x - TargetSquare.x;
	offset.y = pos_last.y - TargetSquare.y;

	// Then we want to find out in which direction the mouse has moved
	// since the last time, and compute the distance relatively to the axis
	if (fabsf(offset.y) > fabsf(offset.x))
	{
	    if (offset.y > 0)
	    {
		actual_direction = NORTH;
	    }
	    else
	    {
		actual_direction = SOUTH;
	    }
	    distance = fabsf(TargetSquare.y - pos_last.y);
	}
	else
	{
	    if (offset.x > 0) {
		actual_direction = WEST;
	    }
	    else
	    {
		actual_direction = EAST;
	    }
	    distance = fabsf(TargetSquare.x - pos_last.x);
	}

	// Are we going in a direction possible with that wall?
	switch (wall_orientation(wall_indices [ walls->editor_mode ] [ walls->id ])) {
	    case HORIZONTAL:
		direction_is_possible = (actual_direction == WEST) || (actual_direction == EAST);
		break;
	    case VERTICAL:
		direction_is_possible = (actual_direction == NORTH) || (actual_direction == SOUTH);
		break;
	    default:
		// We don't want weird lines
		direction_is_possible = FALSE;
	}

	// If the mouse is far away enoug
	if ((distance > 1) && (direction_is_possible) &&
		((walls->direction == actual_direction) || 
		 (walls->direction == UNDEFINED))) 
	{
	    wall = malloc(sizeof(line_element));

	    // Then we calculate the position of the next wall
	    wall->position = pos_last;
	    switch(actual_direction) {
		case NORTH:
		    wall->position.y--;
		    break;
		case SOUTH:
		    wall->position.y++;
		    break;
		case EAST:
		    wall->position.x++;
		    break;
		case WEST:
		    wall->position.x--;
		    break;
		default:
		    break;
	    }
	    // And add the wall, to the linked list and to the map
	    list_add_tail(&(wall->list), &(walls->elements.list));
	    wall->address = action_create_obstacle_user ( EditLevel , wall->position.x , wall->position.y , wall_indices [ walls->editor_mode ] [ walls->id ] );

	    // If the direction is unknown (ie. we only have one wall), 
	    // let's define it
	    if (walls->direction == UNDEFINED)
	    {
		walls->direction = actual_direction;
	    }
	}
	if ((walls->direction == (- actual_direction)) && (!list_empty(&(walls->elements.list))))
	{
	    // Looks like the user wants to go back, so let's remove the line wall
	    wall = list_entry(walls->elements.list.prev, line_element, list);
	    action_remove_obstacle_user(EditLevel, wall->address);
	    list_del(walls->elements.list.prev);
	    free(wall);
	    if(list_empty(&(walls->elements.list)))
	    {
		walls->direction = UNDEFINED;
	    }
	}
    }
}; // void handle_line_mode(line_element *wall_line, moderately_finepoint TargetSquare, int *direction)

void end_line_mode(whole_line *walls, int place_line)
{
    line_element *tmp;
    int list_length = 1;  

    // Remove the linked list
    while(!(list_empty(&(walls->elements.list))))
    {
	tmp = list_entry(walls->elements.list.prev, line_element, list);
	free(tmp);
	if(!place_line)
	    action_remove_obstacle(EditLevel, walls->elements.address);
	list_del(walls->elements.list.prev);
	list_length++;
    }
    if(!place_line)
	action_remove_obstacle(EditLevel, walls->elements.address);

    list_del(&(walls->elements.list));

    if (place_line)
	action_push(ACT_MULTIPLE_FLOOR_SETS, list_length);

}; // void end_line_mode(line_element *wall_line, int place_line)

/* ----------------------------------------------------------------------
 * This function automatically scrolls the leveleditor window when the
 * mouse reaches an edge 
 * ---------------------------------------------------------------------- */
void level_editor_auto_scroll()
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

/* ----------------------------------------------------------------------
 * In an effort to reduce the massive size of the level editor main
 * function, we take the mouse wheel handling out into a separate
 * function now.
 * ---------------------------------------------------------------------- */
void
level_editor_handle_mouse_wheel ( void )
{
    if ( MouseWheelDownPressed() )
    {
	if ( Highlight < number_of_walls [ GameConfig . level_editor_edit_mode ] -1 )
	    Highlight++;
	
	// check if we have to scroll the list
	if( Highlight < FirstBlock )
	    // block is to the left
	    FirstBlock = Highlight ;
	else if (Highlight > FirstBlock +8)
	    // block is to the right
	    FirstBlock = Highlight - 8;
    } 
    if ( MouseWheelUpPressed() && Highlight != 0)
    {
	Highlight--;
	
	// check if we have to scroll the list
	if(Highlight < FirstBlock )
	    // block is to the left
	    FirstBlock = Highlight ;
	else if (Highlight > FirstBlock +8)
	    // block is to the right
	    FirstBlock = Highlight - 8;
    } 
}; // void level_editor_handle_mouse_wheel ( void )
    
/* ----------------------------------------------------------------------
 * In an effort to reduce the massive size of the level editor main
 * function, we take the mouse button blitting out into a separate
 * function now.
 * ---------------------------------------------------------------------- */
void
level_editor_blit_mouse_buttons ( Level EditLevel )
{
    int a = MouseLeftPressed();

#define A(val) do { if ( !a || ! MouseCursorIsOnButton(val, GetMousePos_x() , GetMousePos_y() ) ) \
    		 ShowGenericButtonFromList ( val );\
    		else ShowGenericButtonFromList ( val + 1);\
		} while(0)

    if ( EditLevel -> jump_target_north >= 0 )
	A ( GO_LEVEL_NORTH_BUTTON );
    if ( EditLevel -> jump_target_south >= 0 )
	A ( GO_LEVEL_SOUTH_BUTTON );
    if ( EditLevel -> jump_target_east >= 0 )
	A ( GO_LEVEL_EAST_BUTTON );
    if ( EditLevel -> jump_target_west >= 0 )
	A ( GO_LEVEL_WEST_BUTTON );
    A ( EXPORT_THIS_LEVEL_BUTTON );
    
    A ( LEVEL_EDITOR_SAVE_SHIP_BUTTON );
    
    if ( GameConfig . zoom_is_on )
	A ( LEVEL_EDITOR_ZOOM_IN_BUTTON );
    else
	A ( LEVEL_EDITOR_ZOOM_OUT_BUTTON );
    A ( LEVEL_EDITOR_TOGGLE_WAYPOINT_BUTTON );
    if ( OriginWaypoint == (-1) )
	A ( LEVEL_EDITOR_TOGGLE_CONNECTION_BLUE_BUTTON );
    else
	A ( LEVEL_EDITOR_TOGGLE_CONNECTION_RED_BUTTON );
    A ( LEVEL_EDITOR_BEAUTIFY_GRASS_BUTTON );

    A ( LEVEL_EDITOR_DELETE_OBSTACLE_BUTTON );
    A ( LEVEL_EDITOR_NEXT_OBSTACLE_BUTTON );

    A ( LEVEL_EDITOR_RECURSIVE_FILL_BUTTON );
    A ( LEVEL_EDITOR_NEW_OBSTACLE_LABEL_BUTTON );
    A ( LEVEL_EDITOR_NEW_OBSTACLE_DESCRIPTION_BUTTON );
    A ( LEVEL_EDITOR_NEW_MAP_LABEL_BUTTON );
    A ( LEVEL_EDITOR_NEW_ITEM_BUTTON );
    A ( LEVEL_EDITOR_ESC_BUTTON );
    A ( LEVEL_EDITOR_LEVEL_RESIZE_BUTTON );
    A ( LEVEL_EDITOR_KEYMAP_BUTTON );
    A ( LEVEL_EDITOR_QUIT_BUTTON );
    
    if (to_undo.next != &to_undo) {
	A ( LEVEL_EDITOR_UNDO_BUTTON);
    }

    if (to_redo.next != &to_redo) {
	A ( LEVEL_EDITOR_REDO_BUTTON);
    }
    if ( EditLevel -> use_underground_lighting )
	ShowGenericButtonFromList ( LEVEL_EDITOR_UNDERGROUND_LIGHT_ON_BUTTON );
    else
	ShowGenericButtonFromList ( LEVEL_EDITOR_UNDERGROUND_LIGHT_OFF_BUTTON );

    if ( GameConfig . omit_tux_in_level_editor ) 
	A ( LEVEL_EDITOR_TOGGLE_TUX_BUTTON_OFF );
    else
	A ( LEVEL_EDITOR_TOGGLE_TUX_BUTTON );
    
    if ( GameConfig . omit_enemies_in_level_editor ) 
	A ( LEVEL_EDITOR_TOGGLE_ENEMIES_BUTTON_OFF );
    else 
	A ( LEVEL_EDITOR_TOGGLE_ENEMIES_BUTTON );
    
    if ( GameConfig . omit_obstacles_in_level_editor ) 
	A ( LEVEL_EDITOR_TOGGLE_OBSTACLES_BUTTON_OFF );
    else
	A ( LEVEL_EDITOR_TOGGLE_OBSTACLES_BUTTON );
    
    if ( GameConfig . show_tooltips ) 
    {
	A ( LEVEL_EDITOR_TOGGLE_TOOLTIPS_BUTTON );
	show_level_editor_tooltips (  );
    }
    else
	A ( LEVEL_EDITOR_TOGGLE_TOOLTIPS_BUTTON_OFF );

    if ( draw_collision_rectangles ) 
	A ( LEVEL_EDITOR_TOGGLE_COLLISION_RECTS_BUTTON );
    else
	A ( LEVEL_EDITOR_TOGGLE_COLLISION_RECTS_BUTTON_OFF );

    switch(draw_grid){
        case 1:
        A( LEVEL_EDITOR_TOGGLE_GRID_BUTTON );
          break;
        case 2:
          A( LEVEL_EDITOR_TOGGLE_GRID_BUTTON_FULL );
          break;
        case 0:
        A( LEVEL_EDITOR_TOGGLE_GRID_BUTTON_OFF );
          break;
    }

#undef A
}; // void level_editor_blit_mouse_buttons ( Level EditLevel )

/* ----------------------------------------------------------------------
 *
 *
 * ---------------------------------------------------------------------- */
void
cycle_marked_obstacle( Level EditLevel )
{
    int current_mark_index ;
    int j;

    if ( level_editor_marked_obstacle != NULL )
    {
	//--------------------
	// See if this floor tile has some other obstacles glued to it as well
	//
	if ( EditLevel -> map [ BlockY ] [ BlockX ] . obstacles_glued_to_here [ 1 ] != (-1) )
	{
	    //--------------------
	    // Find out which one of these is currently marked
	    //
	    current_mark_index = (-1);
	    for ( j = 0 ; j < MAX_OBSTACLES_GLUED_TO_ONE_MAP_TILE ; j ++ )
	    {
		if ( level_editor_marked_obstacle == & ( EditLevel -> obstacle_list [ EditLevel -> map [ BlockY ] [ BlockX ] . obstacles_glued_to_here [ j ] ] ) )
		    current_mark_index = j ;
	    }
	    
	    if ( current_mark_index != (-1) ) 
	    {
		if ( EditLevel -> map [ BlockY ] [ BlockX ] . obstacles_glued_to_here [ current_mark_index + 1 ] != (-1) )
		    level_editor_marked_obstacle = & ( EditLevel -> obstacle_list [ EditLevel -> map [ BlockY ] [ BlockX ] . obstacles_glued_to_here [ current_mark_index + 1 ] ] ) ;
		else
		    level_editor_marked_obstacle = & ( EditLevel -> obstacle_list [ EditLevel -> map [ BlockY ] [ BlockX ] . obstacles_glued_to_here [ 0 ] ] ) ;
	    }
	    
	}
    }

    while ( NPressed() || SpacePressed() || MouseLeftPressed() ) SDL_Delay(1);

}; // void cycle_marked_obstacle( Level EditLevel )

/* ----------------------------------------------------------------------
 * This function provides the Level Editor integrated into 
 * freedroid.  Actually this function is a submenu of the big
 * Escape Menu.  In here you can edit the level and, upon pressing
 * escape, you can enter a new submenu where you can save the level,
 * change level name and quit from level editing.
 * ---------------------------------------------------------------------- */
void 
LevelEditor(void)
{
    int proceed_now = FALSE;
    int i ;
    char linebuf[10000];
    long OldTicks;
    char* NewCommentOnThisSquare;
    int LeftMousePressedPreviousFrame = FALSE;
    int RightMousePressedPreviousFrame = FALSE;
    moderately_finepoint TargetSquare;
    whole_line *walls = MyMalloc(sizeof(whole_line));
    walls->activated = FALSE;

    BlockX = rintf( Me . pos . x + 0.5 );
    BlockY = rintf( Me . pos . y + 0.5 );
    level_editor_done = FALSE;
    
    //--------------------
    // We initialize some arrays with info for proper handling
    // of the level editor selection bar later...
    //
    update_number_of_walls ( );

    //--------------------
    // We set the Tux position to something 'round'.
    //
    Me . pos . x = rintf ( Me . pos . x ) + 0.5 ;
    Me . pos . y = rintf ( Me . pos . y ) + 0.5 ;
    
    //--------------------
    // We disable all the 'screens' so that we have full view on the
    // map for the purpose of level editing.
    //
    GameConfig.Inventory_Visible = FALSE;
    GameConfig.CharacterScreen_Visible = FALSE;
    GameConfig.SkillScreen_Visible = FALSE;
    RespectVisibilityOnMap = FALSE ;
    
    //--------------------
    // We init the 'vanishing message' structs, so that there is always
    // something to display, and we set the time to 'out of date' already.
    //
    EditLevel = curShip.AllLevels [ Me . pos . z ] ;
    strcpy ( VanishingMessage , "" );
    VanishingMessageDisplayTime = 0 ;
    
    //--------------------
    // For drawing new waypoints, we init this.
    //
    OriginWaypoint = (-1);
    
    //--------------------
    // Maybe the cursor has moved into the top bar with the selection tab?
    // In that case we might want to change the appearance of the mouse 
    // cursor a bit, like to arrow shape or something, for conveninet selection...
    //
  //  set_mouse_cursor_to_shape ( MOUSE_CURSOR_ARROW_SHAPE ) ;

    while ( !level_editor_done )
    {
	proceed_now=FALSE;
	OldTicks = SDL_GetTicks ( ) ;
	main_menu_requested = FALSE ;
	while ( ( !level_editor_done ) && ( ! main_menu_requested ) )
	{
            track_last_frame_input_status();
	    //--------------------
	    // Even the level editor might be fast or slow or too slow, so we'd like to 
	    // know speed in here too, so that we can identify possible unnescessary lags
	    // and then maybe do something about them...
	    //
	    ComputeFPSForThisFrame();
	    if ( SkipAFewFrames ) SkipAFewFrames--;
	    StartTakingTimeForFPSCalculation(); 
	    
	    //--------------------
	    // Also in the Level-Editor, there is no need to go at full framerate...
	    // We can do with less, cause there are no objects supposed to be 
	    // moving fluently anyway.  Therefore we introduce some rest for the CPU.
	    //
	    if ( ! GameConfig . hog_CPU ) SDL_Delay (1);
	    
	    BlockX = rintf ( Me . pos . x - 0.5 );
	    BlockY = rintf ( Me . pos . y - 0.5 );
	    if ( BlockX < 0 ) 
	    {
		BlockX = 0 ;
		Me . pos . x = 0.51 ;
	    }
	    if ( BlockY < 0 ) 
	    {
		BlockY = 0 ;
		Me . pos . y = 0.51 ;
	    }

	    EditLevel = curShip.AllLevels [ Me . pos . z ] ;	  
	    GetAllAnimatedMapTiles ( EditLevel );
	    
	    //--------------------
	    // If the cursor is close to the currently marked obstacle, we leave everything as it
	    // is.  (There might be some human choice made here already.)
	    // Otherwise we just select the next best obstacle as the new marked obstacle.
	    //
	    if ( level_editor_marked_obstacle != NULL ) 
	    {
		// if ( ( fabsf ( level_editor_marked_obstacle -> pos . x - Me . pos . x ) >= 0.98 ) ||
		// ( fabsf ( level_editor_marked_obstacle -> pos . y - Me . pos . y ) >= 0.98 ) )
		//level_editor_marked_obstacle = NULL ;
		if ( ! marked_obstacle_is_glued_to_here ( EditLevel , Me . pos . x , Me . pos . y ) )
		    level_editor_marked_obstacle = NULL ;
	    }
	    else
	    {
		if ( EditLevel -> map [ BlockY ] [ BlockX ] . obstacles_glued_to_here [ 0 ] != (-1) )
		{
		    level_editor_marked_obstacle = & ( EditLevel -> obstacle_list [ EditLevel -> map [ BlockY ] [ BlockX ] . obstacles_glued_to_here [ 0 ] ] ) ;
		    DebugPrintf ( 0 , "\nObstacle marked now!" );
		}
		else
		{
		    level_editor_marked_obstacle = NULL ;
		    DebugPrintf ( 0 , "\nNo obstacle marked now!" );
		}
	    }
	    
	    VanishingMessageDisplayTime += ( SDL_GetTicks ( ) - OldTicks ) / 1000.0 ;
	    OldTicks = SDL_GetTicks ( ) ;
	    
	    AssembleCombatPicture ( ONLY_SHOW_MAP_AND_TEXT | SHOW_GRID | SHOW_ITEMS | GameConfig.omit_tux_in_level_editor * OMIT_TUX | GameConfig.omit_obstacles_in_level_editor * OMIT_OBSTACLES | GameConfig.omit_enemies_in_level_editor * OMIT_ENEMIES | SHOW_OBSTACLE_NAMES | ZOOM_OUT * GameConfig . zoom_is_on | OMIT_BLASTS | SKIP_LIGHT_RADIUS );
	    
	    Highlight_Current_Block(ZOOM_OUT * GameConfig . zoom_is_on );
	    
	    ShowWaypoints( FALSE , ZOOM_OUT * GameConfig . zoom_is_on );
	    ShowMapLabels( ZOOM_OUT * GameConfig . zoom_is_on );
	    
	    SetCurrentFont ( FPS_Display_BFont ) ;
	    
	    //--------------------
	    // Now we print out the current status directly onto the window:
	    //
	    if ( OriginWaypoint == ( -1 ) )
	    {
		sprintf ( linebuf , _(" Source waypoint selected : NONE" ));
	    }
	    else
	    {
		sprintf ( linebuf , _(" Source waypoint selected : X=%d Y=%d. ") , 
			  EditLevel -> AllWaypoints [ OriginWaypoint ] . x , 
			  EditLevel -> AllWaypoints [ OriginWaypoint ] . y );
	    }
	    LeftPutString ( Screen , GameConfig.screen_height - 2*FontHeight( GetCurrentFont() ), linebuf );
	    
	    //--------------------
	    // Now we print out the latest connection operation success or failure...
	    //
	    if ( VanishingMessageDisplayTime < 7 )
	    {
		DisplayText ( VanishingMessage ,  1 , GameConfig.screen_height - 8 * FontHeight ( GetCurrentFont () ) ,
                        NULL , 1.0 );
	    }
	    
	    ShowLevelEditorTopMenu( Highlight );

	    level_editor_blit_mouse_buttons ( EditLevel );

	    //--------------------
	    // Now that everything is blitted and printed, we may update the screen again...
	    //
	    our_SDL_flip_wrapper( Screen );
	    
	    //--------------------
	    // If the user of the Level editor pressed some cursor keys, move the
	    // highlited filed (that is Me.pos) accordingly. This is done here:
	    //
	    HandleLevelEditorCursorKeys();
	    
	    /* Undo / Redo */
	    if ( UPressed () ) {
		action_undo ( EditLevel );
		while ( UPressed ( ) );
	    }
	    if ( RPressed () ) {
		action_redo ( EditLevel );
		while ( RPressed ( ) );
	    }
	    //--------------------
	    // With the 'S' key, you can attach a statement for the influencer to 
	    // say to a given location, i.e. the location the map editor cursor
	    // currently is on.
	    //
	    if ( SPressed () )
	    {
		while (SPressed());
		SetCurrentFont( FPS_Display_BFont );
		NewCommentOnThisSquare = 
		    GetEditableStringInPopupWindow ( 1000 , "\n Please enter new statement for this tile: \n\n" ,
						     "" );
		for ( i = 0 ; i < MAX_STATEMENTS_PER_LEVEL ; i ++ )
		{
		    if ( EditLevel->StatementList[ i ].x == (-1) ) break;
		}
		if ( i == MAX_STATEMENTS_PER_LEVEL ) 
		{
		    DisplayText ( "\nNo more free comment position.  Using first. " , -1 , -1 , &User_Rect , 1.0 );
		    i=0;
		    our_SDL_flip_wrapper ( Screen );
		    getchar_raw();
		    // Terminate( ERR );
		}
		
		EditLevel->StatementList[ i ].Statement_Text = NewCommentOnThisSquare;
		EditLevel->StatementList[ i ].x = rintf( Me.pos.x );
		EditLevel->StatementList[ i ].y = rintf( Me.pos.y );
	    }
	    
	    //--------------------
	    // With the 'L' key, you can edit the current map label.
	    // The label will be assumed to be directly under the cursor.
	    //
	    if ( LPressed () ) action_change_map_label_user (EditLevel);
	    
	    //--------------------
	    // The 'M' key will activate mouse-move-mode to allow for convenient
	    // mouse-based re-placement of the currently marked obstacle, OR,
	    // if the mouse-move-mode was already activated, it will drop the
	    // marked obstacle to it's new place.
	    //
	    if ( MPressed () ) 
	    {
		if ( ! level_editor_mouse_move_mode )
		{
		    if ( level_editor_marked_obstacle != NULL ) 
			level_editor_mouse_move_mode = TRUE ;
		}
		else
		{
		    if ( level_editor_marked_obstacle != NULL ) 
		    {
			level_editor_marked_obstacle -> pos . x = 
			    translate_pixel_to_map_location ( (float) input_axis.x , 
							      (float) input_axis.y , TRUE ) ;
			level_editor_marked_obstacle -> pos . y = 
			    translate_pixel_to_map_location ( (float) input_axis.x , 
							      (float) input_axis.y , FALSE ) ;
			glue_obstacles_to_floor_tiles_for_level ( EditLevel -> levelnum );
			level_editor_marked_obstacle = NULL ;
		    }
		    level_editor_mouse_move_mode = FALSE ;
		}
		while ( MPressed() );
	    }
	    else
	    {
		if ( level_editor_mouse_move_mode && ( level_editor_marked_obstacle == NULL ) )
		    level_editor_mouse_move_mode = FALSE ;
	    }
	    
	    //--------------------
	    // From the level editor, it should also be possible to drop new goods
	    // at some location via the 'G' key. (G like in Goods.)
	    //
	    if ( GPressed () )
	    {
		ItemDropFromLevelEditor(  );
	    }
	    
	    //--------------------
	    // From the level editor, it should also be possible to drop new goods
	    // at some location via the 'G' key. (G like in Goods.)
	    //
	    if ( BPressed () )
	    {
		if ( CtrlWasPressed() )
		    beautify_grass_tiles_on_level ( EditLevel );
		while ( BPressed() ) SDL_Delay ( 1 ) ;
	    }
	    
	    //--------------------
	    // The tab key should toggle the automap.  Inside the level editor,
	    // if would also be good, if the automap could immediately reveal
	    // all the info on the current map.
	    //
	    if ( TabPressed () )
	    {
		GameConfig . Automap_Visible = ! GameConfig . Automap_Visible ;
		full_update_of_automap_texture ( );
		while ( TabPressed() ) SDL_Delay(1);
	    }
	    
	    //--------------------
	    // The FKEY can be used to toggle between 'floor' and 'obstacle' edit modes
	    //
	    if ( FPressed () )
	    {
		GameConfig . level_editor_edit_mode ++ ;
		if ( GameConfig . level_editor_edit_mode >= NUMBER_OF_LEVEL_EDITOR_GROUPS )
		    GameConfig . level_editor_edit_mode = LEVEL_EDITOR_SELECTION_FLOOR ;
		while ( FPressed ( ) ) SDL_Delay(1);
		Highlight = 0 ;
		FirstBlock = 0 ;
	    }
	    
	    //--------------------
	    // If the person using the level editor decides he/she wants a different
	    // scale for the editing process, he/she may say so by using the O/I keys.
	    //
	    if ( OPressed () ) 
	    {
		GameConfig . zoom_is_on = !GameConfig . zoom_is_on ;
		while ( OPressed() ) SDL_Delay(1);
	    }
	    
	    if ( level_editor_marked_obstacle && XPressed () )
	    {
		action_remove_obstacle_user ( EditLevel , level_editor_marked_obstacle );
		level_editor_marked_obstacle = NULL ;
		while ( XPressed() ) SDL_Delay(1);
	    }
	    
	    //--------------------
	    // The HKEY can be used to give a name to the currently marked obstacle
	    //
	    if ( HPressed() )
	    {
		if ( level_editor_marked_obstacle != NULL )
		{
		    action_change_obstacle_label ( EditLevel , level_editor_marked_obstacle , NULL );
		    while ( HPressed() ) SDL_Delay(1);
		}
	    }
	    
	    if ( NPressed() )
	    {
		cycle_marked_obstacle( EditLevel );		
		while ( NPressed() ) SDL_Delay(1);
	    }
	    
	    //--------------------
	    // If the person using the level editor pressed w, the waypoint is
	    // toggled on the current square.  That means either removed or added.
	    // And in case of removal, also the connections must be removed.
	    //
	    if ( WPressed( ) )
	    {
		action_toggle_waypoint ( EditLevel , BlockX, BlockY , FALSE );
		while ( WPressed() ) SDL_Delay(1);
	    }
	    
	    //--------------------
	    // If the person using the level editor pressed r, the waypoint on 
	    // the current square is toggled concerning random spawning of bots.
	    //
	    if ( RPressed( ) )
	    {
		action_toggle_waypoint ( EditLevel , BlockX, BlockY , TRUE );
		while ( RPressed() ) SDL_Delay(1);
	    }
	    
	    //--------------------
	    // If the person using the level editor presses C that indicated he/she wants
	    // a connection between waypoints.  If this is the first selected waypoint, its
	    // an origin and the second "C"-pressed waypoint will be used a target.
	    // If origin and destination are the same, the operation is cancelled.
	    //
	    if (CPressed())
	    {
		action_toggle_waypoint_connection_user ( EditLevel, BlockX, BlockY );
		while (CPressed()) SDL_Delay(1);
		fflush(stdout);
	    }
	    
	    //----------------------------------------------------------------------
	    // If the person using the level editor pressed some editing keys, insert the
	    // corresponding map tile.  This is done in the following:
	    //
	    HandleMapTileEditingKeys ( EditLevel , BlockX , BlockY );
	    
	    //--------------------
	    // First we find out which map square the player MIGHT wish us to operate on
	    // via a POTENTIAL mouse click
	    //
	    if ( GameConfig . zoom_is_on )
	    {
		TargetSquare . x = translate_pixel_to_zoomed_map_location ( (float) GetMousePos_x()  - ( GameConfig . screen_width / 2 ) , 
									    (float) GetMousePos_y()  - ( GameConfig . screen_height / 2 ), TRUE );
		TargetSquare . y = translate_pixel_to_zoomed_map_location ( (float) GetMousePos_x()  - ( GameConfig . screen_width / 2 ), 
									    (float) GetMousePos_y()  - ( GameConfig . screen_height / 2 ), FALSE );
	    }
	    else
	    {
		TargetSquare . x = translate_pixel_to_map_location ( (float) GetMousePos_x()  - ( GameConfig . screen_width / 2 ) , 
								     (float) GetMousePos_y()  - ( GameConfig . screen_height / 2 ), TRUE );
		TargetSquare . y = translate_pixel_to_map_location ( (float) GetMousePos_x()  - ( GameConfig . screen_width / 2 ), 
								     (float) GetMousePos_y()  - ( GameConfig . screen_height / 2 ), FALSE );
	    }

	    if(walls->activated) 
		handle_line_mode(walls, TargetSquare);
	    

	    level_editor_handle_mouse_wheel();

	    level_editor_auto_scroll();

	    proceed_now = level_editor_handle_left_mouse_button ( proceed_now );
	    
	    //--------------------
	    // With the right mouse button, it should be possible to actually 'draw'
	    // something into the level.  This seems to work so far.  Caution is nescessary
	    // to prevent segfault due to writing outside the level, but that's easily
	    // accomplished.
	    //

	    if ( MouseRightPressed() && !RightMousePressedPreviousFrame )
	    {
		if ( ( (int)TargetSquare . x >= 0 ) &&
		     ( (int)TargetSquare . x <= EditLevel->xlen-1 ) &&
		     ( (int)TargetSquare . y >= 0 ) &&
		     ( (int)TargetSquare . y <= EditLevel->ylen-1 ) )
		{
		    if ( GameConfig . level_editor_edit_mode == LEVEL_EDITOR_SELECTION_FLOOR )
		    {
			action_set_floor (EditLevel, TargetSquare . x, TargetSquare . y, Highlight );
			quickbar_use ( GameConfig . level_editor_edit_mode, Highlight );
		    }
		    else if ( GameConfig . level_editor_edit_mode == LEVEL_EDITOR_SELECTION_QUICK)
		    {
			quickbar_click ( EditLevel , Highlight , TargetSquare, walls); 
		    }
		    /* If the tile can be part of a line */
		    else if ( ( GameConfig . level_editor_edit_mode == LEVEL_EDITOR_SELECTION_WALLS) &&
			      (wall_orientation(wall_indices[GameConfig.level_editor_edit_mode][Highlight]) != UNDEFINED) )
		    {
			/* Let's start the line (FALSE because the function will
			 * find the tile by itself) */
			start_line_mode(walls, TargetSquare, FALSE);
			quickbar_use ( walls->editor_mode, walls->id );
		    }
		    else
		    {
			/* Completely disallow unaligned placement of walls, with tile granularity, using right click */
			moderately_finepoint pos;
			pos . x = TargetSquare . x;
			pos . y = TargetSquare . y;
			if (GameConfig . level_editor_edit_mode == LEVEL_EDITOR_SELECTION_WALLS)
			{
			    pos . x = (int)pos.x;
			    pos . y = (int)pos.y;
			}
			action_create_obstacle_user ( EditLevel , pos . x , pos . y , wall_indices [ GameConfig . level_editor_edit_mode ] [ Highlight ] );
			quickbar_use ( GameConfig . level_editor_edit_mode, Highlight );
		    }
		}
	    }
	    
	    if ( ! MouseRightPressed() && RightMousePressedPreviousFrame && walls->activated )
	    { /* Mouse right released ? terminate line of wall */
		walls->activated = FALSE;
		// End line mode and place the walls
		end_line_mode(walls, TRUE);
	    }
		    
	    if ( QPressed ( ) &&  CtrlWasPressed() )
	    {
		Terminate(0);
	    }
	    
	    if ( EscapePressed() )
	    {
		if ( level_editor_mouse_move_mode )
		{
		    level_editor_mouse_move_mode = FALSE ;
		    while ( EscapePressed() ) ;
		}
		else if ( walls->activated)
		{
		    // Return to normal mode
		    walls->activated = FALSE;
		    // End line mode and *do not* place the walls
		    end_line_mode(walls, FALSE);
		    while ( EscapePressed() ) SDL_Delay(1);
		} 
		else
		{
		    main_menu_requested = TRUE ;
		}
	    }
	    
	    LeftMousePressedPreviousFrame = MouseLeftPressed(); 
	    RightMousePressedPreviousFrame = MouseRightPressed() ;
	    
	}
	while( EscapePressed() ) SDL_Delay(1);
	
	//--------------------
	// After Level editing is done and escape has been pressed, 
	// display the Menu with level save options and all that.
	//
	if ( !level_editor_done ) level_editor_done = DoLevelEditorMainMenu ( EditLevel );
	
    } // while (!level_editor_done)
    
    free(walls);
    RespectVisibilityOnMap = TRUE ;
    level_editor_marked_obstacle = NULL ; 
    
    Activate_Conservative_Frame_Computation();
    action_freestack ( ) ;

}; // void LevelEditor ( void )

#undef _leveleditor_c
