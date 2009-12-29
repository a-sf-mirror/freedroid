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
LIST_HEAD(to_undo);
LIST_HEAD(to_redo);

static int push_mode = NORMAL;

/**
 *  @fn void clear_action(action * pos)
 *
 *  @brief clears an action from its list, and pointers held within the action
 */
static void clear_action(action * action)
{
	if (action->type == ACT_SET_OBSTACLE_LABEL && action->d.change_obstacle_name.new_name != NULL)
		free(action->d.change_obstacle_name.new_name);
	else if (action->type == ACT_SET_MAP_LABEL && action->d.change_label_name.new_name != NULL)
		free(action->d.change_label_name.new_name);

	list_del(&action->node);	//< removes an action from a list
	free(action);		//< free's the action
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

static action *action_create(int type, va_list args)
{
	action *act = malloc(sizeof(action));
	act->type = type;
	switch (type) {
		case ACT_CREATE_OBSTACLE:
			act->d.create_obstacle.x = va_arg(args, double);
			act->d.create_obstacle.y = va_arg(args, double);
			act->d.create_obstacle.new_obstacle_type = va_arg(args, int);
			break;
		case ACT_REMOVE_OBSTACLE:
			act->d.delete_obstacle = va_arg(args, obstacle *);
			break;
		case ACT_MOVE_OBSTACLE:
			act->d.move_obstacle.obstacle = va_arg(args, obstacle *);
			act->d.move_obstacle.newx = va_arg(args, double);
			act->d.move_obstacle.newy = va_arg(args, double);
			break;
		case ACT_WAYPOINT_TOGGLE:
		case ACT_WAYPOINT_TOGGLE_CONNECT:
			act->d.waypoint_toggle.x = va_arg(args, int);
			act->d.waypoint_toggle.y = va_arg(args, int);
			act->d.waypoint_toggle.spawn_toggle = va_arg(args, int);
			break;
		case ACT_TILE_FLOOR_SET:
			act->d.change_floor.x = va_arg(args, int);
			act->d.change_floor.y = va_arg(args, int);
			act->d.change_floor.type = va_arg(args, int);
			break;
		case ACT_MULTIPLE_ACTIONS:
			act->d.number_actions = va_arg(args, int);
			break;
		case ACT_SET_OBSTACLE_LABEL:
			act->d.change_obstacle_name.obstacle = va_arg(args, obstacle *);
			act->d.change_obstacle_name.new_name = va_arg(args, char *);
			break;
		case ACT_SET_MAP_LABEL:
			act->d.change_label_name.id = va_arg(args, int);
			act->d.change_label_name.new_name = va_arg(args, char *);
			break;
		case ACT_JUMP_TO_LEVEL:
			act->d.jump_to_level.target_level = va_arg(args, int);
			act->d.jump_to_level.x = (float)va_arg(args, double);
			act->d.jump_to_level.y = (float)va_arg(args, double);
			break;
		default:
			ErrorMessage(__FUNCTION__, "Unknown action type %d\n", PLEASE_INFORM, IS_FATAL, type);
	}

	return act;
}

void action_push(int type, ...)
{
	va_list args;
	va_start(args, type);
	
	action *act = action_create(type, args);

	switch (push_mode) {
		case UNDO:
			list_add(&act->node, &to_redo);
			break;
		case REDO:
		case NORMAL:
			list_add(&act->node, &to_undo);
			break;
	}

	va_end(args);
}

obstacle *action_create_obstacle(level * EditLevel, double x, double y, int new_obstacle_type)
{
	int i;

	for (i = 0; i < MAX_OBSTACLES_ON_MAP; i++) {
		if (EditLevel->obstacle_list[i].type == (-1)) {
			EditLevel->obstacle_list[i].type = new_obstacle_type;
			EditLevel->obstacle_list[i].pos.x = x;
			EditLevel->obstacle_list[i].pos.y = y;
			EditLevel->obstacle_list[i].pos.z = EditLevel->levelnum;
			EditLevel->obstacle_list[i].name_index = (-1);
			glue_obstacles_to_floor_tiles_for_level(EditLevel->levelnum);
			DebugPrintf(0, "\nNew obstacle has been added!!!");
			fflush(stdout);
			//--------------------
			// Now that we have disturbed the order of the obstacles on this level, we need
			// to re-assemble the lists of pointers to obstacles, like the door list, the
			// teleporter list and the refreshes list.
			//
			dirty_animated_obstacle_lists(EditLevel->levelnum);
			
			return (&(EditLevel->obstacle_list[i]));
		}
	}

	ErrorMessage(__FUNCTION__, "\
	    Ran out of obstacle positions (%d) in level %d!", PLEASE_INFORM, IS_FATAL, MAX_OBSTACLES_ON_MAP, EditLevel->levelnum);
	return (NULL);
}

void action_move_obstacle(level * EditLevel, obstacle * obs, float newx, float newy)
{
	action_push(ACT_MOVE_OBSTACLE, obs, obs->pos.x, obs->pos.y);
	obs->pos.x = newx;
	obs->pos.y = newy;
	glue_obstacles_to_floor_tiles_for_level(EditLevel->levelnum);
}

obstacle *action_create_obstacle_user(Level EditLevel, double x, double y, int new_obstacle)
{
	obstacle *o = action_create_obstacle(EditLevel, x, y, new_obstacle);
	if (o) {
		action_push(ACT_REMOVE_OBSTACLE, o);
	}
	return o;
}

/**
 * Change an obstacle label, possibly removing it.
 * @return the number of actions that were pushed on the stack. Removing a non existing label 
 * will not create a undo action.
 */
static int action_change_obstacle_label(level *EditLevel, obstacle *obstacle, char *name, int undoable)
{
	int check_double;
	char *old_name = NULL;
	int index = -1;
	int i;
	int nbact = 0;

	if (name)
		name = strdup(name);

	//--------------------
	// If the obstacle already has a name, we can use that index for the 
	// new name now.
	//
	if (obstacle->name_index >= 0)
		index = obstacle->name_index;
	else {
		//--------------------
		// Else we must find a free index in the list of obstacle names for this level
		//
		for (i = 0; i < MAX_OBSTACLE_NAMES_PER_LEVEL; i++) {
			if (EditLevel->obstacle_name_list[i] == NULL) {
				index = i;
				break;
			}
		}
		if (index < 0)
			goto ret;
	}

	old_name = EditLevel->obstacle_name_list[index];
	if (!name || strlen(name) == 0) {
		obstacle->name_index = -1;
		EditLevel->obstacle_name_list[index] = NULL;
	} else {
		EditLevel->obstacle_name_list[index] = name;
		obstacle->name_index = index;
	}

	if (undoable && old_name != name) {
		action_push(ACT_SET_OBSTACLE_LABEL, obstacle, old_name);
		nbact ++;
	} else if (old_name) {
		free(old_name);
	}

	if (obstacle->name_index == -1)
		goto ret;

	//--------------------
	// But even if we fill in something new, we should first
	// check against double entries of the same label.  Let's
	// do it...
	//
	for (check_double = 0; check_double < MAX_OBSTACLE_NAMES_PER_LEVEL; check_double++) {
		//--------------------
		// We must not use null pointers for string comparison...
		//
		if (EditLevel->obstacle_name_list[check_double] == NULL)
			continue;

		//--------------------
		// We must not overwrite ourself with us in foolish ways :)
		//
		if (check_double == index)
			continue;

		//--------------------
		// But in case of real double-entries, we'll handle them right.
		//
		if (!strcmp(EditLevel->obstacle_name_list[index], EditLevel->obstacle_name_list[check_double])) {
			ErrorMessage(__FUNCTION__, "\
		    The label %s did already exist on this map!  Deleting old entry in favour of the new one!", NO_NEED_TO_INFORM, IS_WARNING_ONLY, EditLevel->obstacle_name_list[index]);
			EditLevel->obstacle_name_list[index] = NULL;
			obstacle->name_index = check_double;
			break;
		}
	}

ret: 
	return nbact;
}

void action_change_obstacle_label_user(level *EditLevel, obstacle *our_obstacle, char *predefined_name)
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
	if (predefined_name == NULL) {
		name =
		    GetEditableStringInPopupWindow(1000, _("\nPlease enter name for this obstacle: \n\n"),
						   cur_idx != -1 ? EditLevel->obstacle_name_list[cur_idx] : "");
	} else {
		name = strdup(predefined_name);
	}

	action_change_obstacle_label(EditLevel, our_obstacle, name, 1);

	free(name);
}

/**
 * Remove an obstacle from the map.
 * @param undoable indicates whether the action should be made undoable by pushing 
 * undo actions on the stack.
 */
void action_remove_obstacle(level *EditLevel, obstacle *our_obstacle)
{
	//--------------------
	// The likely case that no obstacle was currently marked.
	//
	if (our_obstacle == NULL)
		return;

	our_obstacle->type = (-1);

	// Remove the obstacle label if we had one
	// with the current design this won't be undoable
	action_change_obstacle_label(EditLevel, our_obstacle, NULL, 0);

	//--------------------
	// Now doing that must have shifted the glue!  That is a problem.  We need to
	// reglue everything to the map...
	//
	glue_obstacles_to_floor_tiles_for_level(EditLevel->levelnum);

	//--------------------
	// Now that we have disturbed the order of the obstacles on this level, we need
	// to re-assemble the lists of pointers to obstacles, like the door list, the
	// teleporter list and the refreshes list.
	//
	dirty_animated_obstacle_lists(EditLevel->levelnum);
}

void action_remove_obstacle_user(Level EditLevel, obstacle * our_obstacle)
{
	/* Save obstacle information for the undo action */
	double posx, posy;
	int type;

	type = our_obstacle->type;
	posx = our_obstacle->pos.x;
	posy = our_obstacle->pos.y;

	// make an undoable removal
	action_remove_obstacle(EditLevel, our_obstacle);
	action_push(ACT_CREATE_OBSTACLE, posx, posy, type);
}

void action_toggle_waypoint(level * EditLevel, int BlockX, int BlockY, int toggle_random_spawn)
{
	int wpnum;
	int isnew = 0;

	wpnum = CreateWaypoint(EditLevel, BlockX, BlockY, &isnew);

	//--------------------
	// If its waypoint already, this waypoint must either be deleted
	// or the random spawn bit reset...
	//
	if (!isnew) {
		if (toggle_random_spawn) {
			if (EditLevel->AllWaypoints[wpnum].suppress_random_spawn)
				EditLevel->AllWaypoints[wpnum].suppress_random_spawn = 0;
			else
				EditLevel->AllWaypoints[wpnum].suppress_random_spawn = 1;
		} else
			DeleteWaypoint(EditLevel, wpnum);
	}

	action_push(ACT_WAYPOINT_TOGGLE, BlockX, BlockY, toggle_random_spawn);
}

int action_toggle_waypoint_connection(level * EditLevel, int id_origin, int id_target, int removeifpresent, int undoable)
{
	int i = 0;
	waypoint *SrcWp = &(EditLevel->AllWaypoints[id_origin]);
	for (i = 0; i < SrcWp->num_connections; i++) {
		if (SrcWp->connections[i] == id_target) {
			// Already a waypoint, remove it
			if (removeifpresent) {
				memmove(SrcWp->connections + i, SrcWp->connections + i + 1,
					(SrcWp->num_connections - (i + 1)) * sizeof(SrcWp->connections[0]));
				SrcWp->num_connections--;
			}
			if (undoable)
				action_push(ACT_WAYPOINT_TOGGLE_CONNECT, id_origin, id_target, -1);
			return -1;
		}
	}
	SrcWp->connections[SrcWp->num_connections] = id_target;
	SrcWp->num_connections++;
	SrcWp = NULL;

	if (undoable)
		action_push(ACT_WAYPOINT_TOGGLE_CONNECT, id_origin, id_target, -1);
	return 1;
}

void level_editor_action_toggle_waypoint_connection_user(level * EditLevel, int xpos, int ypos)
{
	int i;

	// Determine which waypoint is currently targeted
	for (i = 0; i < EditLevel->num_waypoints; i++) {
		if ((EditLevel->AllWaypoints[i].x == xpos) && (EditLevel->AllWaypoints[i].y == ypos))
			break;
	}

	if (i == EditLevel->num_waypoints) {
		sprintf(VanishingMessage, _("Sorry, don't know which waypoint you mean."));
		VanishingMessageEndDate = SDL_GetTicks() + 7000;
	} else {
		sprintf(VanishingMessage, _("You specified waypoint nr. %d."), i);
		VanishingMessageEndDate = SDL_GetTicks() + 7000;
		if (OriginWaypoint == (-1)) {
			OriginWaypoint = i;
			if (EditLevel->AllWaypoints[OriginWaypoint].num_connections < MAX_WP_CONNECTIONS) {
				strcat(VanishingMessage, _("\nIt has been marked as the origin of the next connection."));
				DebugPrintf(1, "\nWaypoint nr. %d. selected as origin\n", i);
			} else {
				sprintf(VanishingMessage, _("\nsORRY, MAXIMAL NUMBER OF WAYPOINT-CONNECTIONS (%d) REACHED!\n"),
					MAX_WP_CONNECTIONS);
				DebugPrintf(0, "Operation not possible\n");
				OriginWaypoint = (-1);
			}
		} else {
			if (OriginWaypoint == i) {
				strcat(VanishingMessage, _("\n\nOrigin==Target --> Connection Operation cancelled."));
				OriginWaypoint = (-1);
			} else {
				sprintf(VanishingMessage, _("\n\nOrigin: %d Target: %d. Operation makes sense."), OriginWaypoint, i);
				if (action_toggle_waypoint_connection(EditLevel, OriginWaypoint, i, 1, 1) < 0) {
					strcat(VanishingMessage, _("\nOperation done, connection removed."));
				} else {
					strcat(VanishingMessage, _("\nOperation done, connection added."));
				}
				OriginWaypoint = (-1);
			}
		}
	}

	return;

}

void action_set_floor(Level EditLevel, int x, int y, int type)
{
	int old = EditLevel->map[y][x].floor_value;
	EditLevel->map[y][x].floor_value = type;
	action_push(ACT_TILE_FLOOR_SET, x, y, old);
}

static int tile_is_free(level * EditLevel, int y_old, int x_old, int y_new, int x_new)
{
	int i;
	float x, y;
	int wall_id = -1;
	int obstacle_id;
	for (i = 0; i < MAX_OBSTACLES_GLUED_TO_ONE_MAP_TILE; ++i) {
		obstacle_id = EditLevel->map[y_new][x_new].obstacles_glued_to_here[i];
		if (obstacle_id != -1 &&
		    (EditLevel->obstacle_list[obstacle_id].type >= ISO_V_WALL ||
		     EditLevel->obstacle_list[obstacle_id].type <= ISO_OUTER_WALL_E3)) {
			wall_id = obstacle_id;
			y = EditLevel->obstacle_list[obstacle_id].pos.y;
			x = EditLevel->obstacle_list[obstacle_id].pos.x;
			break;
		}
	}
	return TRUE;
}

static void action_fill_user_recursive(level * EditLevel, int x, int y, int type, int *changed)
{
	int source_type = EditLevel->map[y][x].floor_value;
	/* security */
	if (!pos_inside_level(x, y, EditLevel))
		return;

#define at(x,y) (EditLevel -> map [ y ] [ x ] . floor_value)
	if (at(x, y) == type)
		return;
	action_set_floor(EditLevel, x, y, type);
	(*changed)++;
	if (x > 0 && at(x - 1, y) == source_type && tile_is_free(EditLevel, y, x, y, x - 1))
		action_fill_user_recursive(EditLevel, x - 1, y, type, changed);
	if (x < EditLevel->xlen - 1 && at(x + 1, y) == source_type && tile_is_free(EditLevel, y, x, y, x + 1))
		action_fill_user_recursive(EditLevel, x + 1, y, type, changed);
	if (y > 0 && at(x, y - 1) == source_type && tile_is_free(EditLevel, y, x, y - 1, x))
		action_fill_user_recursive(EditLevel, x, y - 1, type, changed);
	if (y < EditLevel->ylen - 1 && at(x - 1, y + 1) == source_type && tile_is_free(EditLevel, y + 1, x, y, x))
		action_fill_user_recursive(EditLevel, x, y + 1, type, changed);
}

void action_fill_user(level * EditLevel, int BlockX, int BlockY, int SpecialMapValue)
{
	int number_changed = 0;
	action_fill_user_recursive(EditLevel, BlockX, BlockY, SpecialMapValue, &number_changed);
	action_push(ACT_MULTIPLE_ACTIONS, number_changed);
}

static void action_change_map_label(level * EditLevel, int i, char *name)
{
	if (EditLevel->labels[i].pos.x != -1) {
		int check_double;
		if (name) {
			for (check_double = 0; check_double < MAX_MAP_LABELS_PER_LEVEL; check_double++) {
				if (!strcmp(name, EditLevel->labels[check_double].label_name)) {
					ErrorMessage(__FUNCTION__, "\
				    The label just entered did already exist on this map!  Deleting old entry in favour of the new one!", PLEASE_INFORM, IS_WARNING_ONLY);
					i = check_double;
					break;
				}
			}
		}
		action_push(ACT_SET_MAP_LABEL, i, EditLevel->labels[i].label_name);
	} else {
		action_push(ACT_SET_MAP_LABEL, i, NULL);
	}
	if (name && strlen(name)) {
		EditLevel->labels[i].label_name = name;
		EditLevel->labels[i].pos.x = rintf(Me.pos.x - 0.5);
		EditLevel->labels[i].pos.y = rintf(Me.pos.y - 0.5);
	} else {
		EditLevel->labels[i].label_name = ("NoLabelHere");
		EditLevel->labels[i].pos.x = (-1);
		EditLevel->labels[i].pos.y = (-1);
	}
}

void level_editor_action_change_map_label_user(level * EditLevel)
{
	char *NewCommentOnThisSquare;
	int i;

	SetCurrentFont(FPS_Display_BFont);

	//--------------------
	// Now we see if a map label entry is existing already for this spot
	//
	for (i = 0; i < MAX_MAP_LABELS_PER_LEVEL; i++) {
		if ((fabsf(EditLevel->labels[i].pos.x + 0.5 - Me.pos.x) < 0.5) &&
		    (fabsf(EditLevel->labels[i].pos.y + 0.5 - Me.pos.y) < 0.5)) {
			break;
		}
	}
	if (i >= MAX_MAP_LABELS_PER_LEVEL) {
		NewCommentOnThisSquare =
		    GetEditableStringInPopupWindow(1000,
						   _
						   ("\nNo existing map label entry for this position found...\n Please enter new label for this map position: \n\n"),
						   "");

		i = 0;
		for (i = 0; i < MAX_MAP_LABELS_PER_LEVEL; i++) {
			if (EditLevel->labels[i].pos.x == (-1))
				break;
		}
		if (i >= MAX_MAP_LABELS_PER_LEVEL) {
			DisplayText(_("\nNo more free map label entry found... using first on instead ...\n"), -1, -1, &User_Rect, 1.0);
			i = 0;
		} else {
			DisplayText(_("\nUsing new map label list entry...\n"), -1, -1, &User_Rect, 1.0);
		}
		// Terminate( ERR );
	} else {
		NewCommentOnThisSquare =
		    GetEditableStringInPopupWindow(1000,
						   _
						   ("\nOverwriting existing map label list entry...\n Please enter new label for this map position: \n\n"),
						   EditLevel->labels[i].label_name);
	}
	action_change_map_label(EditLevel, i, NewCommentOnThisSquare);
}

/**
 *  @fn void jump_to_level( int target_map, float x, float y)
 *
 *  @brief jumps to a target level, saving this level on the undo/redo stack
 */
void action_jump_to_level(int target_level, double x, double y)
{
	action_push(ACT_JUMP_TO_LEVEL, EditLevel()->levelnum, Me.pos.x, Me.pos.y);	//< sets undo or redo stack, depending on push_mode state
	reset_visible_levels();
	Teleport(target_level, (float)x, (float)y, FALSE);
}

static void action_do(level * level, action * a)
{
	switch (a->type) {
	case ACT_CREATE_OBSTACLE:
		action_create_obstacle_user(level, a->d.create_obstacle.x, a->d.create_obstacle.y, a->d.create_obstacle.new_obstacle_type);
		break;
	case ACT_REMOVE_OBSTACLE:
		action_remove_obstacle_user(level, a->d.delete_obstacle);
		break;
	case ACT_MOVE_OBSTACLE:
		action_move_obstacle(level, a->d.move_obstacle.obstacle, a->d.move_obstacle.newx, a->d.move_obstacle.newy);
		break;
	case ACT_WAYPOINT_TOGGLE:
		action_toggle_waypoint(level, a->d.waypoint_toggle.x, a->d.waypoint_toggle.y, a->d.waypoint_toggle.spawn_toggle);
		break;
	case ACT_WAYPOINT_TOGGLE_CONNECT:
		action_toggle_waypoint_connection(level, a->d.waypoint_toggle.x, a->d.waypoint_toggle.y, 1, 1);
		break;
	case ACT_TILE_FLOOR_SET:
		action_set_floor(level, a->d.change_floor.x, a->d.change_floor.y, a->d.change_floor.type);
		break;
	case ACT_MULTIPLE_ACTIONS:
		ErrorMessage(__FUNCTION__, "Passed a multiple actions meta-action as parameter. A real action is needed.\n", PLEASE_INFORM, IS_WARNING_ONLY);
		break;
	case ACT_SET_OBSTACLE_LABEL:
		action_change_obstacle_label(level, a->d.change_obstacle_name.obstacle, a->d.change_obstacle_name.new_name, 1);
		break;
	case ACT_SET_MAP_LABEL:
		action_change_map_label(level, a->d.change_label_name.id, a->d.change_label_name.new_name);
		break;
	case ACT_JUMP_TO_LEVEL:
		action_jump_to_level(a->d.jump_to_level.target_level, a->d.jump_to_level.x, a->d.jump_to_level.y);
		break;

	}
}

static void __level_editor_do_action_from_stack(struct list_head *stack)
{
	action *a;

	if (list_empty(stack))
		return;

	a = list_entry(stack->next, action, node);

	if (a->type == ACT_MULTIPLE_ACTIONS) {
		int i, max;

		max = a->d.number_actions;
		clear_action(a);

		for (i = 0; i < max; i++) {
			__level_editor_do_action_from_stack(stack);
		}
	} else {
		action_do(EditLevel(), a);
		clear_action(a);
	}
}

void level_editor_action_undo()
{
	push_mode = UNDO;
	__level_editor_do_action_from_stack(&to_undo);
	push_mode = NORMAL;
}

void level_editor_action_redo()
{
	push_mode = REDO;
	__level_editor_do_action_from_stack(&to_redo);
	push_mode = NORMAL;
}

/**
 *
 *
 */
void level_editor_place_aligned_obstacle(int positionid)
{
	struct leveleditor_typeselect *ts = get_current_object_type();
	int obstacle_id = -1;
	int obstacle_created = FALSE;
	float position_offset_x[9] = { 0, 0.5, 1.0, 0, 0.5, 1.0, 0, 0.5, 1.0 };
	float position_offset_y[9] = { 1.0, 1.0, 1.0, 0.5, 0.5, 0.5, 0, 0, 0 };

	positionid--;

	if (ts->type != OBJECT_OBSTACLE && ts->type != OBJECT_FLOOR)
		return;

	switch (ts->type) {
	case OBJECT_ANY:
		GiveMouseAlertWindow("Quickbar support was removed.");
		break;

	case OBJECT_OBSTACLE:
		obstacle_id = ts->indices[ts->selected_tile_nb];
		action_create_obstacle_user(EditLevel(), ((int)Me.pos.x) + position_offset_x[positionid],
					    ((int)Me.pos.y) + position_offset_y[positionid], obstacle_id);
		obstacle_created = TRUE;
		break;

	case OBJECT_FLOOR:
		action_set_floor(EditLevel(), (int)Me.pos.x, (int)Me.pos.y, ts->indices[ts->selected_tile_nb]);
		break;

	default:
		return;
	}
}

/**
 * This function should create a completely new level into the existing
 * ship structure that we already have.  The new level will be rather
 * small and simple.
 */
void CreateNewMapLevel(int level_num)
{
	level *NewLevel;
	int i, k, l;

	//--------------------
	// Get the memory for one level 
	//
	NewLevel = (Level) MyMalloc(sizeof(level));

	DebugPrintf(0, "\n-----------------------------------------------------------------");
	DebugPrintf(0, "\nStarting to create and add a completely new level to the ship.");

	//--------------------
	// Now we proceed in the order of the struct 'level' in the
	// struct.h file so that we can easily verify if we've handled
	// all the data structure or left something out which could
	// be terrible!
	//
	NewLevel->levelnum = level_num;
	NewLevel->xlen = 90;
	NewLevel->ylen = 90;
	NewLevel->light_bonus = 19;
	NewLevel->minimum_light_value = 19;
	NewLevel->Levelname = strdup("New level just created");
	NewLevel->Background_Song_Name = strdup("TheBeginning.ogg");
	//--------------------
	// Now we initialize the obstacle name list with 'empty' values
	//
	for (i = 0; i < MAX_OBSTACLE_NAMES_PER_LEVEL; i++) {
		NewLevel->obstacle_name_list[i] = NULL;
	}
	
	//--------------------
	// First we initialize the floor with 'empty' values
	//
	for (i = 0; i < NewLevel->ylen; i++) {
		NewLevel->map[i] = MyMalloc(NewLevel->xlen * sizeof(map_tile));
		for (k = 0; k < NewLevel->xlen; k++) {
			NewLevel->map[i][k].floor_value = ISO_FLOOR_SAND;
			for (l = 0; l < MAX_OBSTACLES_GLUED_TO_ONE_MAP_TILE; l++) {
				NewLevel->map[i][k].obstacles_glued_to_here[l] = (-1);
			}
		}
	}
	//--------------------
	// Now we initialize the level jump interface variables with 'empty' values
	//
	NewLevel->jump_target_north = (-1);
	NewLevel->jump_target_south = (-1);
	NewLevel->jump_target_east = (-1);
	NewLevel->jump_target_west = (-1);

	//--------------------
	// Now we initialize the map obstacles with 'empty' information
	//
	for (i = 0; i < MAX_OBSTACLES_ON_MAP; i++) {
		NewLevel->obstacle_list[i].type = (-1);
		NewLevel->obstacle_list[i].pos.x = (-1);
		NewLevel->obstacle_list[i].pos.y = (-1);
		NewLevel->obstacle_list[i].pos.z = level_num;
		NewLevel->obstacle_list[i].name_index = (-1);
	}
	for (i = 0; i < MAX_OBSTACLES_ON_MAP; i++) {
		NewLevel->obstacle_list[i].type = (-1);
		NewLevel->obstacle_list[i].pos.x = (-1);
		NewLevel->obstacle_list[i].pos.y = (-1);
		NewLevel->obstacle_list[i].name_index = (-1);
	}

	//--------------------
	// Now we initialize the map labels array with 'empty' information
	//
	for (i = 0; i < MAX_MAP_LABELS_PER_LEVEL; i++) {
		NewLevel->labels[i].pos.x = (-1);
		NewLevel->labels[i].pos.y = (-1);
		NewLevel->labels[i].label_name = "no_label_defined";
	}
	//--------------------
	// Now we add empty waypoint information...
	//
	NewLevel->num_waypoints = 0;
	for (i = 0; i < MAXWAYPOINTS; i++) {
		NewLevel->AllWaypoints[i].x = 0;
		NewLevel->AllWaypoints[i].y = 0;

		for (k = 0; k < MAX_WP_CONNECTIONS; k++) {
			NewLevel->AllWaypoints[i].connections[k] = -1;
		}
	}
	//--------------------
	// First we initialize the items arrays with 'empty' information
	//
	for (i = 0; i < MAX_ITEMS_PER_LEVEL; i++) {
		NewLevel->ItemList[i].pos.x = (-1);
		NewLevel->ItemList[i].pos.y = (-1);
		NewLevel->ItemList[i].pos.z = (-1);
		NewLevel->ItemList[i].type = (-1);
		NewLevel->ItemList[i].currently_held_in_hand = FALSE;

	}
	//--------------------
	// Now we initialize the chest items arrays with 'empty' information
	//
	for (i = 0; i < MAX_CHEST_ITEMS_PER_LEVEL; i++) {
		NewLevel->ChestItemList[i].pos.x = (-1);
		NewLevel->ChestItemList[i].pos.y = (-1);
		NewLevel->ChestItemList[i].pos.z = (-1);
		NewLevel->ChestItemList[i].type = (-1);
		NewLevel->ChestItemList[i].currently_held_in_hand = FALSE;
	}

	curShip.AllLevels[level_num] = NewLevel;

	glue_obstacles_to_floor_tiles_for_level(level_num);

};				// void CreateNewMapLevel( int )

void delete_map_level(int lnum)
{
	int i;
	level *l = curShip.AllLevels[lnum];

	// Delete floor tiles
	for (i = 0; i < l->ylen; i++)
		free(l->map[i]);

	// Remove references to this level from others
	for (i = 0; i < MAX_LEVELS; i++) {
		if (curShip.AllLevels[i] == NULL)
			continue;

		if (curShip.AllLevels[i]->jump_target_north == lnum)
			curShip.AllLevels[i]->jump_target_north = -1;
		if (curShip.AllLevels[i]->jump_target_south == lnum)
			curShip.AllLevels[i]->jump_target_south = -1;
		if (curShip.AllLevels[i]->jump_target_west == lnum)
			curShip.AllLevels[i]->jump_target_west = -1;
		if (curShip.AllLevels[i]->jump_target_east == lnum)
			curShip.AllLevels[i]->jump_target_east = -1;
	}

	// Free memory
	free(curShip.AllLevels[lnum]);

	curShip.AllLevels[lnum] = NULL;

	if (lnum == curShip.num_levels)
		curShip.num_levels--;

}

void level_editor_edit_chest(obstacle * o)
{
	item *chest_items[MAX_CHEST_ITEMS_PER_LEVEL];
	item *user_items[MAX_CHEST_ITEMS_PER_LEVEL];
	int chest_nb_items;
	int done = 0;
	shop_decision shop_order;
	item *tmp;
	int idx;

	item dummy_addtochest = {.type = 1,.suffix_code = -1,.prefix_code = -1,.is_identified = 1 };
	FillInItemProperties(&dummy_addtochest, 2, 1);

	user_items[0] = &dummy_addtochest;
	user_items[1] = NULL;

	// Safety check
	switch (o->type) {
	case ISO_H_CHEST_CLOSED:
	case ISO_H_CHEST_OPEN:
	case ISO_V_CHEST_CLOSED:
	case ISO_V_CHEST_OPEN:
	case ISO_E_CHEST2_CLOSED:
	case ISO_S_CHEST2_CLOSED:
	case ISO_N_CHEST2_CLOSED:
	case ISO_W_CHEST2_CLOSED:
		break;
	default:
		ErrorMessage(__FUNCTION__, "Tried to edit the contents of a chest, but the obstacle is not a chest.\n", PLEASE_INFORM,
			     IS_FATAL);
	}

	while (!done) {

		// Build the list of items in the chest
		chest_nb_items = AssemblePointerListForChestShow(&chest_items[0], o->pos);

		// Display the shop interface
		done = GreatShopInterface(chest_nb_items, chest_items, 1, user_items, &shop_order);

		// BUY removes an item from the chest
		// SELL spaws the drop item interface
		switch (shop_order.shop_command) {
		case BUY_1_ITEM:
			DeleteItem(chest_items[shop_order.item_selected]);
			break;
		case SELL_1_ITEM:
			tmp = ItemDropFromLevelEditor();
			if (tmp) {
				tmp->pos.x = o->pos.x;
				tmp->pos.y = o->pos.y;
				tmp->pos.z = o->pos.z;

				for (idx = 0; idx < MAX_CHEST_ITEMS_PER_LEVEL; idx++) {
					if (EditLevel()->ChestItemList[idx].type == -1)
						break;
				}

				if (idx == MAX_CHEST_ITEMS_PER_LEVEL) {
					ErrorMessage(__FUNCTION__, "Chests on current level are full.", PLEASE_INFORM, IS_WARNING_ONLY);
					idx = 0;
				}
				MoveItem(tmp, &EditLevel()->ChestItemList[idx]);
			}
			break;
		default:
			;
		}
	}
}

#undef _leveleditor_action_c
