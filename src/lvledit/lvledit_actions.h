/* 
 *
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

/* Undo/redo stack */
void action_freestack(void);

void level_editor_action_undo(void);
void level_editor_action_redo(void);

void action_push(int type, ...);

/* Obstacle manipulation */
obstacle *action_create_obstacle_user(level *, double, double, int);
void action_remove_obstacle_user(level *, obstacle *);

void action_move_obstacle(level *, obstacle *, float, float);

void action_change_obstacle_label_user(level *, obstacle *);

/* Waypoint manipulation */
waypoint *action_create_waypoint(level *, int, int, int);
void action_move_waypoint(level *, waypoint *, int, int);
void action_remove_waypoint(level *, int, int);
void action_toggle_waypoint_randomspawn(level *, int, int);
int action_toggle_waypoint_connection(level *, int, int, int, int);
void level_editor_action_toggle_waypoint_connection_user(level *, int, int);
void lvledit_action_toggle_waypoint(int);

/* Floor tiles manipulation */
void action_set_floor(level *, int, int, int);
void action_set_floor_layer(level *, int, int, int, int);
void action_change_floor_layer(level *, int);

/* Map manipulation */
void level_editor_action_change_map_label_user(level *, float, float);
void action_jump_to_level(int, double, double);
void action_jump_to_level_center(int);
void CreateNewMapLevel(int);
void delete_map_level(int);

/* Chest manipulation */
void level_editor_edit_chest(obstacle *);

// Item manipulation
item *action_create_item(level *, float, float, int);
void action_remove_item(level *, item *);
void action_move_item(level *, item *, float, float);

// Object manipulation
void level_editor_place_aligned_object(int);

#ifndef _leveleditor_actions_c
#define EXTERN extern
#else
#define EXTERN
#endif

EXTERN struct list_head to_undo;
EXTERN struct list_head to_redo;
