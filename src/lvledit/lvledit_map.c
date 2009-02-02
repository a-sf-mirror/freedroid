/* 
 *
 *   Copyright (c) 2004-2009 Arthur Huillet
 *   Copyright (c) 2002, 2003 Johannes Prix
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

#define _leveleditor_map_c


#include "system.h"

#include "defs.h"
#include "struct.h"
#include "global.h"
#include "proto.h"

#include "SDL_rotozoom.h"

#include "lvledit/lvledit.h"
#include "lvledit/lvledit_actions.h"
#include "lvledit/lvledit_map.h"

/**
 *
 *
 */      
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

/**
 * If we want to synchronize two levels, we need to remove the old obstacles
 * before we can add new ones.  Else the place might get too crowded with
 * obstacles. :)
 */
void
delete_all_obstacles_in_area (level *TargetLevel , float start_x , float start_y , float area_width , float area_height )
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


/**
 *
 *
 */
void move_obstacles_and_items_south_of ( float from_where , float by_what, level *edit_level )
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
  
}; // void move_obstacles_south_of ( float from_where , float by_what, level *edit_level )


/**
 *
 *
 */
void
move_obstacles_east_of ( float from_where , float by_what, level *edit_level )
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
  
}; // void move_obstacles_and_items_east_of ( float from_where , float by_what, level *edit_level )


/**
 * When new lines are inserted into the map, the map labels south of this
 * line must move too with the rest of the map.  This function sees to it.
 */
static void MoveMapLabelsSouthOf (int FromWhere , int ByWhat, level *EditLevel )
{
  int i;

  for ( i = 0 ; i < MAX_MAP_LABELS_PER_LEVEL ; i ++ )
    {
      if ( EditLevel -> labels [ i ] . pos . x <= ( -1 ) ) continue;
      
      if ( EditLevel -> labels [ i ] . pos . y >= FromWhere )
	EditLevel -> labels [ i ] . pos . y += ByWhat;
    }
  
}; // void MoveMapLabelsSouthOf ( int FromWhere , int ByWhat, level *EditLevel)

/**
 * When new lines are inserted into the map, the waypoints south of this
 * line must move too with the rest of the map.  This function sees to it.
 */
static void MoveWaypointsSouthOf ( int FromWhere , int ByWhat, level *EditLevel )
{
  int i;

  for ( i = 0 ; i < MAXWAYPOINTS ; i ++ )
    {
      if ( EditLevel -> AllWaypoints [ i ] . x == ( 0 ) ) continue;
      
      if ( EditLevel -> AllWaypoints [ i ] . y >= FromWhere )
	EditLevel -> AllWaypoints [ i ] . y += ByWhat;
    }
  
}; // void MoveWaypointsSouthOf ( int FromWhere , int ByWhat, level *EditLevel)

/**
 * When new lines are inserted into the map, the waypoints east of this
 * line must move too with the rest of the map.  This function sees to it.
 */
static void MoveWaypointsEastOf ( int FromWhere , int ByWhat, level *EditLevel )
{
  int i;

  for ( i = 0 ; i < MAXWAYPOINTS ; i ++ )
    {
      if ( EditLevel -> AllWaypoints [ i ] . x == ( 0 ) ) continue;
      
      if ( EditLevel -> AllWaypoints [ i ] . x >= FromWhere )
	EditLevel -> AllWaypoints [ i ] . x += ByWhat;
    }
  
}; // void MoveWaypointsEastOf ( int FromWhere , int ByWhat, level *EditLevel)

void InsertLineVeryNorth (level *EditLevel )
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


void InsertLineVerySouth (level *EditLevel )
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

}; // void InsertLineVerySouth (level *EditLevel )

void InsertColumnVeryEast (level *EditLevel )
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

}; // void InsertColumnVeryEast (level *EditLevel )
      
void InsertColumnVeryWest (level *EditLevel )
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

void InsertColumnEasternInterface(level *EditLevel )
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

void RemoveColumnEasternInterface(level *EditLevel )
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

}; // void RemoveColumnEasternInterface(level *EditLevel );

void InsertColumnWesternInterface(level *EditLevel )
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

}; // void InsertColumnWesternInterface(level *EditLevel )

void RemoveColumnWesternInterface(level *EditLevel )
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

}; // void RemoveColumnWesternInterface(level *EditLevel )

void RemoveColumnVeryWest (level *EditLevel )
{
  int OldEasternInterface;

  //--------------------
  // We shortly change the eastern interface to reuse the code for there
  //
  OldEasternInterface = EditLevel -> jump_threshold_east;

  EditLevel -> jump_threshold_east = EditLevel -> xlen - 1 ;
  RemoveColumnEasternInterface ( EditLevel );

  EditLevel -> jump_threshold_east = OldEasternInterface ;

}; // void RemoveColumnVeryEast (level *EditLevel )

void InsertLineSouthernInterface (level *EditLevel )
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


void RemoveLineSouthernInterface (level *EditLevel )
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

void InsertLineNorthernInterface (level *EditLevel )
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

void RemoveLineNorthernInterface (level *EditLevel )
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

}; // void RemoveLineNorthernInterface (level *EditLevel )


void RemoveLineVeryNorth (level *EditLevel )
{
  int OldSouthernInterface;

  //--------------------
  // We shortly change the southern interface to reuse the code for there
  //
  OldSouthernInterface = EditLevel -> jump_threshold_south;

  EditLevel -> jump_threshold_south = EditLevel -> ylen - 1 ;
  RemoveLineSouthernInterface ( EditLevel );

  EditLevel -> jump_threshold_south = OldSouthernInterface ;

}; // void RemoveLineVeryNorth (level *EditLevel )



/**
 * When new lines are inserted into the map, the map labels east of this
 * line must move too with the rest of the map.  This function sees to it.
 */
void MoveMapLabelsEastOf ( int FromWhere , int ByWhat, level *EditLevel )
{
  int i;

  for ( i = 0 ; i < MAX_MAP_LABELS_PER_LEVEL ; i ++ )
    {
      if ( EditLevel -> labels [ i ] . pos . x <= ( -1 ) ) continue;
      
      if ( EditLevel -> labels [ i ] . pos . x >= FromWhere )
	EditLevel -> labels [ i ] . pos . x += ByWhat;
    }
  
}; // void MoveMapLabelsEastOf ( int FromWhere , int ByWhat, level *EditLevel)

/**
 * When we connect two maps smoothly together, we want an area in both
 * maps, that is really synchronized with the other level we connect to.
 * But this isn't a task that should be done manually.  We rather make
 * a function, that does this synchronisation work, overwriting the 
 * higher level number with the data from the lower level number.
 */
void ExportLevelInterface ( int LevelNum )
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
	
	GetAnimatedMapTiles ();
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
	
	GetAnimatedMapTiles ();
	
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
	
	GetAnimatedMapTiles ();
	
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
	
	GetAnimatedMapTiles ();
    }
    
}; // void SynchronizeLevelInterfaces ( void )
      

