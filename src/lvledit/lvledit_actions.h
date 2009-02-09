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

/* Undo/redo stack */
void action_freestack(void);

void level_editor_action_undo(void);
void level_editor_action_redo(void);

void action_push(int type, ...);

/* Obstacle manipulation */
obstacle * action_create_obstacle (level *, double, double, int);
obstacle * action_create_obstacle_user(level *, double, double, int);
void level_editor_place_aligned_obstacle (int);
void action_remove_obstacle_user (level *, obstacle *);
void action_remove_obstacle(level *, obstacle *);

void action_move_obstacle(level *, obstacle *, float, float);

void action_change_obstacle_label_user (level *, obstacle *, char *);
void action_change_obstacle_description (level *, obstacle *, char *);

/* Waypoint manipulation */
void action_toggle_waypoint (level *, int, int, int);
int action_toggle_waypoint_connection (level *, int , int, int);
void level_editor_action_toggle_waypoint_connection_user(level *, int, int);

/* Floor tiles manipulation */
void action_set_floor (level *, int, int, int);
void action_fill_user (level *, int , int , int );

/* Map manipulation */
void level_editor_action_change_map_label_user (level *);
void action_jump_to_level(int, double, double);

#ifndef _leveleditor_actions_c
#define EXTERN extern
#else
#define EXTERN
#endif

EXTERN struct list_head to_undo;
EXTERN struct list_head to_redo;
