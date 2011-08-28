/* 
 *
 *   Copyright (c) 2002, 2003 Johannes Prix
 *   Copyright (c) 2004-2010 Arthur Huillet
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

#ifndef _leveleditor_h_
#define _leveleditor_h_

#undef EXTERN
#ifndef _leveleditor_c
#define EXTERN extern
#else
#define EXTERN
#endif

typedef struct line_element {
	moderately_finepoint position;
	obstacle *address;

	list_head_t list;
} line_element, *Line_element;

enum ActionType {
	ACT_CREATE_OBSTACLE,
	ACT_REMOVE_OBSTACLE,
	ACT_MOVE_OBSTACLE,
	ACT_CREATE_ITEM,
	ACT_REMOVE_ITEM,
	ACT_MOVE_ITEM,
	ACT_CREATE_WAYPOINT,
	ACT_REMOVE_WAYPOINT,
	ACT_MOVE_WAYPOINT,
	ACT_TOGGLE_WAYPOINT_RSPAWN,
	ACT_TOGGLE_WAYPOINT_CONNECTION,
	ACT_TILE_FLOOR_SET,
	ACT_SET_OBSTACLE_LABEL,
	ACT_SET_MAP_LABEL,
	ACT_JUMP_TO_LEVEL,
	ACT_MULTIPLE_ACTIONS,
	ACT_CHANGE_FLOOR_LAYER,
	ACT_CREATE_MAP_LABEL,
	ACT_REMOVE_MAP_LABEL,
};

typedef struct {
	struct list_head node;
	enum ActionType type;

	union {
		struct {
			double x, y;
			int new_obstacle_type;
		} create_obstacle;

		obstacle *delete_obstacle;

		struct {
			obstacle *obstacle;
			float newx;
			float newy;
		} move_obstacle;

		struct {
			float x, y;
			int type;		
		} create_item;

		item *delete_item;

		struct {
			item *item;
			float newx;
			float newy;
		} move_item;

		struct {
			int x, y;
			int suppress_random_spawn;
		} create_waypoint;

		struct {
			int x, y;
		} delete_waypoint;

		struct {
			waypoint *w;
			int newx, newy;
		} move_waypoint;

		struct {
			int x, y;
		} toggle_waypoint_rspawn;

		struct {
			int x, y;
		} toggle_waypoint_connection;

		struct {
			int x, y;
			int layer;
			int type;
		} change_floor;

		int number_actions;

		struct {
			obstacle *obstacle;
			char *new_name;
		} change_obstacle_name;	/* give_new_name_to_obstacle */

		struct {
			int id;
			char *new_name;
			int x, y;
		} change_label_name;	/* EditMapLabelData */

		struct {
			int target_level;
			double x, y;
		} jump_to_level;

		int target_layer;

		struct {
			int x;
			int y;
			char *label_name;
		} create_map_label;

		struct {
			int x;
			int y;
		} delete_map_label;
	} d;
} action;

enum {
	REDO = -1,		/* pop in to_redo and push in to_undo */
	NORMAL = 0,		/* push only in to_undo */
	UNDO = 1		/* pop in to_undo and push in to_redo  */
};

EXTERN void LevelEditor(void);
EXTERN item *ItemDropFromLevelEditor(void);
EXTERN void TestMap(void);

enum lvledit_object_type {
	OBJECT_OBSTACLE,
	OBJECT_FLOOR,
	OBJECT_ITEM,
	OBJECT_ENEMY,
	OBJECT_WAYPOINT,
	OBJECT_MAP_LABEL,
	OBJECT_NONE,
};

EXTERN int EditX(void);
EXTERN int EditY(void);
EXTERN level *EditLevel(void);

EXTERN char VanishingMessage[10000];
EXTERN float VanishingMessageEndDate;

EXTERN int OriginWaypoint;

EXTERN int level_editor_done;

EXTERN int current_floor_layer;

EXTERN void get_random_droids_from_user(void);

EXTERN void place_special_force(gps, int);

#endif
