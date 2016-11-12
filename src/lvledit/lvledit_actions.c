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

/**
 * This file contains all the actions performed by the level editor, ie. the functions that act on the level.
 */

#define _leveleditor_actions_c 1

#include "system.h"

#include "defs.h"
#include "struct.h"
#include "global.h"
#include "proto.h"


#include "lvledit/lvledit.h"
#include "lvledit/lvledit_widgets.h"
#include "lvledit/lvledit_tool_select.h"
#include "lvledit/lvledit_tool_place.h"

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
	else if (action->type == ACT_CREATE_MAP_LABEL && action->d.create_map_label.label_name != NULL)
		free(action->d.create_map_label.label_name);
	else if (action->type == ACT_CREATE_ENEMY && action->d.create_enemy != NULL)
		enemy_free(action->d.create_enemy);

	list_del(&action->node);	//< removes an action from a list
	free(action);		//< frees the action
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
		case ACT_CREATE_ITEM:
			act->d.create_item.x = (float)va_arg(args, double);
			act->d.create_item.y = (float)va_arg(args, double);
			act->d.create_item.type = va_arg(args, int);
			break;
		case ACT_REMOVE_ITEM:
			act->d.delete_item = va_arg(args, item *);
			break;
		case ACT_MOVE_ITEM:
			act->d.move_item.item = va_arg(args, item *);
			act->d.move_item.newx = (float)va_arg(args, double);
			act->d.move_item.newy = (float)va_arg(args, double);
			break;
		case ACT_CREATE_WAYPOINT:
			act->d.create_waypoint.x = va_arg(args, int);
			act->d.create_waypoint.y = va_arg(args, int);
			act->d.create_waypoint.suppress_random_spawn = va_arg(args, int);
			break;
		case ACT_REMOVE_WAYPOINT:
			act->d.delete_waypoint.x = va_arg(args, int);
			act->d.delete_waypoint.y = va_arg(args, int);
			break;
		case ACT_MOVE_WAYPOINT:
			act->d.move_waypoint.w = va_arg(args, waypoint *);
			act->d.move_waypoint.newx = va_arg(args, int);
			act->d.move_waypoint.newy = va_arg(args, int);
			break;
		case ACT_TOGGLE_WAYPOINT_RSPAWN:
			act->d.toggle_waypoint_rspawn.x = va_arg(args, int);
			act->d.toggle_waypoint_rspawn.y = va_arg(args, int);
			break;
		case ACT_TOGGLE_WAYPOINT_CONNECTION:
			act->d.toggle_waypoint_connection.x = va_arg(args, int);
			act->d.toggle_waypoint_connection.y = va_arg(args, int);
			break;
		case ACT_TILE_FLOOR_SET:
			act->d.change_floor.x = va_arg(args, int);
			act->d.change_floor.y = va_arg(args, int);
			act->d.change_floor.layer = va_arg(args, int);
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
			act->d.change_label_name.x = va_arg(args, int);
			act->d.change_label_name.y = va_arg(args, int);
			break;
		case ACT_JUMP_TO_LEVEL:
			act->d.jump_to_level.target_level = va_arg(args, int);
			act->d.jump_to_level.x = (float)va_arg(args, double);
			act->d.jump_to_level.y = (float)va_arg(args, double);
			break;
		case ACT_CHANGE_FLOOR_LAYER:
			act->d.target_layer = va_arg(args, int);
			break;
		case ACT_CREATE_MAP_LABEL:
			act->d.create_map_label.x = va_arg(args, int);
			act->d.create_map_label.y = va_arg(args, int);
			act->d.create_map_label.label_name = va_arg(args, char *);
			break;
		case ACT_REMOVE_MAP_LABEL:
			act->d.delete_map_label.x = va_arg(args, int);
			act->d.delete_map_label.y = va_arg(args, int);
			break;
		case ACT_CREATE_ENEMY:
			act->d.create_enemy = va_arg(args, enemy *);
			break;
		case ACT_REMOVE_ENEMY:
			act->d.delete_enemy = va_arg(args, enemy *);
			break;
		default:
			error_message(__FUNCTION__, "Unknown action type %d", PLEASE_INFORM | IS_FATAL, type);
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
			list_add(&act->node, &to_undo);
			break;
		case NORMAL:
			list_add(&act->node, &to_undo);
			clear_action_list(&to_redo);
			break;
		default:
			// Should not happen, so this is a protection against a bug in the
			// calling stack, to avoid a memory leak.
			free(act);
			break;
	}

	va_end(args);
}

void action_move_obstacle(level * EditLevel, obstacle * obs, float newx, float newy)
{
	action_push(ACT_MOVE_OBSTACLE, obs, obs->pos.x, obs->pos.y);

	move_obstacle(obs, newx, newy);
}

obstacle *action_create_obstacle_user(Level EditLevel, double x, double y, int new_obstacle)
{
	obstacle *o = add_obstacle(EditLevel, x, y, new_obstacle);
	if (o) {
		action_push(ACT_REMOVE_OBSTACLE, o);
	}
	return o;
}

/**
 * Change an obstacle label, possibly removing it.
 * @return the number of actions that were pushed on the stack.
 */
static int action_change_obstacle_label(struct level *edit_level, struct obstacle *our_obstacle, char *name, int undoable)
{
	// If the obstacle already has a label, remove it
	char *old_label = get_obstacle_extension(edit_level, our_obstacle, OBSTACLE_EXTENSION_LABEL);
	if (old_label) {
		old_label = strdup(old_label);
		del_obstacle_extension(edit_level, our_obstacle, OBSTACLE_EXTENSION_LABEL);
	}

	// Create the undo action if appropriate
	if (undoable) {
		action_push(ACT_SET_OBSTACLE_LABEL, our_obstacle, old_label);
	} else {
		if (old_label) {
			free(old_label);
			old_label = NULL;
		}
	}

	// If the new label is empty, we are done
	if (!name || !strlen(name))
		return 0;

	// Assign the new label
	add_obstacle_extension(edit_level, our_obstacle, OBSTACLE_EXTENSION_LABEL, strdup(name));

	return undoable;
}

void action_change_obstacle_label_user(level *EditLevel, obstacle *our_obstacle)
{
	char *name = NULL;

	if (!our_obstacle)
		return;

	char *old_label = get_obstacle_extension(EditLevel, our_obstacle, OBSTACLE_EXTENSION_LABEL);
	name = get_editable_string_in_popup_window(1000, _("\nPlease enter obstacle label: \n\n"), old_label ? old_label : "");
	
	if (name) {
		action_change_obstacle_label(EditLevel, our_obstacle, name, 1);
		free(name);
	}
}

void action_remove_obstacle_user(Level EditLevel, obstacle * our_obstacle)
{
	/* Save obstacle information for the undo action */
	double posx, posy;
	int type;

	type = our_obstacle->type;
	posx = our_obstacle->pos.x;
	posy = our_obstacle->pos.y;

	// The obstacle can be in the selection list
	remove_element_from_selection(our_obstacle);

	// make an undoable removal
	del_obstacle(our_obstacle);
	action_push(ACT_CREATE_OBSTACLE, posx, posy, type);
}

/**
 * Create an item on the map, add this action in the stack undo/redo
 * \param EditLevel Pointer towards the editing level where create the item
 * \param x The x position of the item
 * \param y The y position of the item
 * \param type The type of the item
 * \return The new item created
 */
item *action_create_item(level *EditLevel, float x, float y, int type)
{
	int multiplicity = 1;

	if (ItemMap[type].item_group_together_in_inventory) {
		// Display a popup window with a number selector in order to choose the
		// multiplicity of the item
		multiplicity =
		    do_graphical_number_selection_in_range(1, (!item_spec_eq_id(type, "Valuable Circuits")) ? 100 : 1000, 1, 0);

		if (multiplicity == 0) {
			// Do not drop an item on the floor when the user cancelled the action.
			return NULL;
		}
	}

	// Create an item on the map
	item *it = DropItemAt(type, EditLevel->levelnum, x, y, multiplicity);
	if (it) {
		action_push(ACT_REMOVE_ITEM, it);
	}
	return it;
}

/**
 * Remove an item on the map and add this action in the stack undo/redo
 * \param EditLevel Pointer towards the editing level where delete the item
 * \param it The item which we want deleted
 */
void action_remove_item(level *EditLevel, item *it)
{
	float x, y;
	int type;

	// Save item information for the undo action
	type = it->type;
	x = it->pos.x;
	y = it->pos.y;

	// The item can be in the selection list
	remove_element_from_selection(it);

	// Remove the item on the map
	DeleteItem(it);

	// Make an undoable removal
	action_push(ACT_CREATE_ITEM, x, y, type);
}

/**
 * Move an item on the map and add this action in stack undo/redo
 * \param EditLevel Pointer towards the editing level where move the item
 * \param it The item which we want moved
 * \param x The new x position of the item
 * \param y The new y position of the item
 */
void action_move_item(level *EditLevel, item *it, float x, float y)
{
	float oldx, oldy;

	// Save item position for the undo action
	oldx = it->pos.x;
	oldy = it->pos.y;

	// Define the new position of the item
	it->pos.x = x;
	it->pos.y = y;

	action_push(ACT_MOVE_ITEM, it, oldx, oldy);
}

/**
 * Create a waypoint on the map, add this action in the undo/redo stack
 * \param EditLevel Pointer towards the currently edited level where create the waypoint
 * \param x The x position of the waypoint
 * \param y The y position of the waypoint
 * \param random_spawn TRUE if the waypoint can be used to place a random bot
 * \return The new waypoint created
 */
waypoint *action_create_waypoint(level *EditLevel, int x, int y, int random_spawn)
{
	waypoint *wpts = EditLevel->waypoints.arr;
	int wpnum;

	wpnum = get_waypoint(EditLevel, x, y);
	if (wpnum < 0) {
		// When the waypoint doesn't exist on the map, create it
		wpnum = add_waypoint(EditLevel, x, y, random_spawn);

		// The waypoints array may have been moved by the add_waypoint call
		wpts = EditLevel->waypoints.arr;

		// Make an undoable action
		action_push(ACT_REMOVE_WAYPOINT, wpts[wpnum].x, wpts[wpnum].y);
	}

	return &wpts[wpnum];
}

/**
 * Remove a waypoint on the map and add this action in the undo/redo stack
 * \param EditLevel Pointer towards the currently edited level where delete the waypoint
 * \param x The x position of the waypoint
 * \param y The y position of the waypoint
 */
void action_remove_waypoint(level *EditLevel, int x, int y)
{
	waypoint *wpts = EditLevel->waypoints.arr;

	int wpnum = get_waypoint(EditLevel, x, y);
	if (wpnum < 0) {
		// When the waypoint doesn't exist on the map, we are done
		return;
	}

	// Save waypoint information for the undo action
	int old_random_spawn = wpts[wpnum].suppress_random_spawn;

	// The waypoint can be in the selection list
	remove_element_from_selection(&wpts[wpnum]);

	// Remove the waypoint on the map
	del_waypoint(EditLevel, x, y);

	// If a route was being traced from this waypoint, remove it
	leveleditor_place_reset_waypoint_route(wpnum);

	// Make an undoable action
	action_push(ACT_CREATE_WAYPOINT, x, y, old_random_spawn);
}

void action_move_waypoint(level *lvl, waypoint *w, int x, int y)
{
	int oldx, oldy;

	oldx = w->x;
	oldy = w->y;

	move_waypoint(lvl, w, x, y);

	action_push(ACT_MOVE_WAYPOINT, w, oldx, oldy);
}

/**
 * Set/unset the flag for random bots and add this action in the undo/redo stack
 * \param EditLevel Pointer towards the currently edited level where toggle the waypoint
 * \param x The x position of the waypoint
 * \param y The y position of the waypoint
 */
void action_toggle_waypoint_randomspawn(level *EditLevel, int x, int y)
{
	waypoint *wpts = EditLevel->waypoints.arr;

	int wpnum = get_waypoint(EditLevel, x, y);
	if (wpnum < 0) {
		return;
	}

	// Toggle the flag for random bots
	wpts[wpnum].suppress_random_spawn = !wpts[wpnum].suppress_random_spawn;

	// Make an undoable action
	action_push(ACT_TOGGLE_WAYPOINT_RSPAWN, x, y);
}

int action_toggle_waypoint_connection(level *EditLevel, int id_origin, int id_target, int removeifpresent, int undoable)
{
	waypoint *wpts = EditLevel->waypoints.arr;
	int *connections;
	int i;

	// Get the connections of the waypoint;
	connections = wpts[id_origin].connections.arr;

	for (i = 0; i < wpts[id_origin].connections.size; i++) {
		if (connections[i] == id_target) {
			if (removeifpresent) {
				// Delete the connection of the waypoint
				dynarray_del(&wpts[id_origin].connections, i, sizeof(int));
			}
			if (undoable)
				action_push(ACT_TOGGLE_WAYPOINT_CONNECTION, id_origin, id_target);
			return -1;
		}
	}

	// Add the target connection of the waypoint
	dynarray_add(&wpts[id_origin].connections, &id_target, sizeof(int));

	if (undoable)
		action_push(ACT_TOGGLE_WAYPOINT_CONNECTION, id_origin, id_target);

	return 1;
}

void level_editor_action_toggle_waypoint_connection_user(level * EditLevel, int xpos, int ypos)
{
	int wpnum;

	wpnum = get_waypoint(EditLevel, xpos, ypos);
	if (wpnum < 0) {
		// No waypoint is currently targeted.
		return;
	}

	if (OriginWaypoint == -1) {
		// Set the origin waypoint for the next connection.
		OriginWaypoint = wpnum;
		return;
	}

	if (OriginWaypoint != wpnum) {
		// Toggle a connection between the origin waypoint and the currently targeted.
		action_toggle_waypoint_connection(EditLevel, OriginWaypoint, wpnum, 1, 1);
	}

	OriginWaypoint = -1;
}

void lvledit_action_toggle_waypoint(int randomspawn)
{
	int wpnum = get_waypoint(EditLevel(), EditX(), EditY());
	if (wpnum < 0) {
		// If the waypoint doesn't exist at the map position, create it
		action_create_waypoint(EditLevel(), EditX(), EditY(), randomspawn);
	} else {
		// An existing waypoint will be removed or have its
		// randomspawn flag toggled
		waypoint *wpts = EditLevel()->waypoints.arr;
		if (randomspawn) {
			action_toggle_waypoint_randomspawn(EditLevel(), wpts[wpnum].x, wpts[wpnum].y);
		} else {
			action_remove_waypoint(EditLevel(), wpts[wpnum].x, wpts[wpnum].y);
		}
	}
}

void action_set_floor_layer(Level EditLevel, int x, int y, int layer, int type)
{
	if (layer >= EditLevel->floor_layers)
		EditLevel->floor_layers++;

	int old = EditLevel->map[y][x].floor_values[layer];
	EditLevel->map[y][x].floor_values[layer] = type;
	action_push(ACT_TILE_FLOOR_SET, x, y, layer, old);
}

void action_set_floor(Level EditLevel, int x, int y, int type)
{
	// Underlay floor tiles are placed in layer #0.
	// Overlay floor tiles are placed in layer #1.
	int layer = type < MAX_UNDERLAY_FLOOR_TILES ? 0 : 1;
	action_set_floor_layer(EditLevel, x, y, layer, type);
}

void action_change_floor_layer(level *lvl, int layer)
{
	int old_floor_layer = current_floor_layer;
	current_floor_layer = layer;
	action_push(ACT_CHANGE_FLOOR_LAYER, old_floor_layer);
}

static void action_change_map_label(level *EditLevel, int i, char *name, int x, int y)
{

	char *old_label = NULL;

	// If the map label exist, remove it
	if (i < EditLevel->map_labels.size) {
		// Get the map label
		struct map_label *map_label =  &ACCESS_MAP_LABEL(EditLevel->map_labels, i);

		// Get the old label for undoable actions
		old_label = strdup(map_label->label_name);

		// Delete the map label
		del_map_label(EditLevel, map_label->label_name);
	}

	action_push(ACT_SET_MAP_LABEL, i, old_label, x, y);

	// If the new label is empty, we are done
	if (!name || !strlen(name))
		return;

	// Create a new map label at the position of cursor
	add_map_label(EditLevel, x, y, strdup(name));
}

void level_editor_action_change_map_label_user(level *EditLevel, float x, float y)
{
	struct map_label *map_label = NULL;
	char *name;
	char *old_name = NULL;
	char suggested_label[200];
	int i;

	suggested_label[0] = '\0';

	// We check if a map label already exists for this spot
	for (i = 0; i < EditLevel->map_labels.size; i++) {
		map_label = &ACCESS_MAP_LABEL(EditLevel->map_labels, i);

		if ((fabs(map_label->pos.x + 0.5 - x) < 0.5) &&
			 (fabs(map_label->pos.y + 0.5 - y) < 0.5)) {

			// Use the old label as a suggestion
			old_name = map_label->label_name;
			strncpy(suggested_label, old_name, sizeof(suggested_label) - 1);
			suggested_label[sizeof(suggested_label) - 1] = '\0';
			break;
		}
	}

	// Check if the name entered already exists

	while (1) {
		// Show popup window to enter a new map label
		name = get_editable_string_in_popup_window(sizeof(suggested_label) - 1, _("\nPlease enter map label: \n\n"), suggested_label);
		if (!name || (old_name && !strcmp(name, old_name))) {
			// Do not change label
			free(name);
			return;
		}

		map_label = map_label_exists(name);
		if (!map_label) {
			// When the new name of the map label does not exist, we are done
			break;
		}

		// When the new name already exists, we must not create another map
		// label with the same name, but we want to display an alert window,
		// and then go back to input box with the name still present
		alert_window("%s", _("The new name of the map label already exists, please choose another name."));

		// Copy the name in order to have it in the input box
		strncpy(suggested_label, name, sizeof(suggested_label) - 1);
		suggested_label[sizeof(suggested_label) - 1] = '\0';
		free(name);

		// Restore the menu background in order to correctly draw the next popup window
		RestoreMenuBackground(1);
	}

	// Change a map label when the name enter by the user is valid
	action_change_map_label(EditLevel, i, name, rintf(x - 0.5), rintf(y - 0.5));
	free(name);
}

/**
 * @brief Create a map label on the map and push this action on the undo/redo stack.
 *
 * @param lvl Pointer towards the level where the map label is created.
 * @param x The x position of the map label.
 * @param y The y position of the map label.
 * @param label_name The name of the map label.
 */
void action_create_map_label(level *lvl, int x, int y, char *label_name)
{
	add_map_label(lvl, x, y, strdup(label_name));

	action_push(ACT_REMOVE_MAP_LABEL, x, y);
}

/**
 * @brief Remove a map label on the map and push this action on the undo/redo stack.
 *
 * @param lvl Pointer towards the level where the map label is removed.
 * @param x The x position of the map label.
 * @param y The y position of the map label.
 */
void action_remove_map_label(level *lvl, int x, int y)
{
	struct map_label *m = get_map_label_from_coords(lvl, x, y);
	if (m) {
		char *old_label_name = strdup(m->label_name);

		remove_element_from_selection(m);

		del_map_label(lvl, m->label_name);

		action_push(ACT_CREATE_MAP_LABEL, x, y, old_label_name);
	}
}

void action_create_enemy(level *lvl, enemy *en)
{
	enemy_insert_into_lists(en, TRUE);
	action_push(ACT_REMOVE_ENEMY, en);
}

void action_remove_enemy(level *lvl, enemy *en)
{
	enemy *current_enemy, *nerot;

	remove_element_from_selection(en);

	BROWSE_ALIVE_BOTS_SAFE(current_enemy, nerot) {
		if (current_enemy == en)
			list_del(&current_enemy->global_list);
	}

	BROWSE_DEAD_BOTS_SAFE(current_enemy, nerot) {
		if (current_enemy == en)
			list_del(&current_enemy->global_list);
	}

	BROWSE_LEVEL_BOTS_SAFE(current_enemy, nerot, lvl->levelnum) {
		if (current_enemy == en)
			list_del(&current_enemy->level_list);
	}

	action_push(ACT_CREATE_ENEMY, en);
}

/**
 *  @fn void jump_to_level( int target_map, float x, float y)
 *
 *  @brief jumps to a target level, saving this level on the undo/redo stack
 */
void action_jump_to_level(int target_level, double x, double y)
{
	// When the user wants to change the current edited level, reset tools
	lvledit_reset_tools();

	action_push(ACT_JUMP_TO_LEVEL, EditLevel()->levelnum, Me.pos.x, Me.pos.y);	//< sets undo or redo stack, depending on push_mode state
	reset_visible_levels();
	Teleport(target_level, (float)x, (float)y, FALSE, TRUE);
}

/**
 * Jump to the center of a level.
 *
 * @param level_num The level id.
 */
void action_jump_to_level_center(int level_num)
{
	// Calculate the center of the level.
	float x = curShip.AllLevels[level_num]->xlen / 2;
	float y = curShip.AllLevels[level_num]->ylen / 2;

	action_jump_to_level(level_num, x, y);
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
	case ACT_CREATE_ITEM:
		action_create_item(level, a->d.create_item.x, a->d.create_item.y, a->d.create_item.type);
		break;
	case ACT_REMOVE_ITEM:
		action_remove_item(level, a->d.delete_item);
		break;
	case ACT_MOVE_ITEM:
		action_move_item(level, a->d.move_item.item, a->d.move_item.newx, a->d.move_item.newy);
		break;
	case ACT_CREATE_WAYPOINT:
		action_create_waypoint(level, a->d.create_waypoint.x, a->d.create_waypoint.y, a->d.create_waypoint.suppress_random_spawn);
		break;
	case ACT_REMOVE_WAYPOINT:
		action_remove_waypoint(level, a->d.delete_waypoint.x, a->d.delete_waypoint.y);
		break;
	case ACT_MOVE_WAYPOINT:
		action_move_waypoint(level, a->d.move_waypoint.w, a->d.move_waypoint.newx, a->d.move_waypoint.newy);
		break;
	case ACT_TOGGLE_WAYPOINT_RSPAWN:
		action_toggle_waypoint_randomspawn(level, a->d.toggle_waypoint_rspawn.x, a->d.toggle_waypoint_rspawn.y);
		break;
	case ACT_TOGGLE_WAYPOINT_CONNECTION:
		action_toggle_waypoint_connection(level, a->d.toggle_waypoint_connection.x, a->d.toggle_waypoint_connection.y, 1, 1);
		break;
	case ACT_TILE_FLOOR_SET:
		action_set_floor_layer(level, a->d.change_floor.x, a->d.change_floor.y, a->d.change_floor.layer, a->d.change_floor.type);
		break;
	case ACT_MULTIPLE_ACTIONS:
		error_message(__FUNCTION__, "Passed a multiple actions meta-action as parameter. A real action is needed.", PLEASE_INFORM);
		break;
	case ACT_SET_OBSTACLE_LABEL:
		action_change_obstacle_label(level, a->d.change_obstacle_name.obstacle, a->d.change_obstacle_name.new_name, 1);
		break;
	case ACT_SET_MAP_LABEL:
		action_change_map_label(level, a->d.change_label_name.id, a->d.change_label_name.new_name, a->d.change_label_name.x, a->d.change_label_name.y);
		break;
	case ACT_JUMP_TO_LEVEL:
		action_jump_to_level(a->d.jump_to_level.target_level, a->d.jump_to_level.x, a->d.jump_to_level.y);
		break;
	case ACT_CHANGE_FLOOR_LAYER:
		action_change_floor_layer(level, a->d.target_layer);
		break;
	case ACT_CREATE_MAP_LABEL:
		action_create_map_label(level, a->d.create_map_label.x, a->d.create_map_label.y, a->d.create_map_label.label_name);
		break;
	case ACT_REMOVE_MAP_LABEL:
		action_remove_map_label(level, a->d.delete_map_label.x, a->d.delete_map_label.y);
		break;
	case ACT_CREATE_ENEMY:
		action_create_enemy(level, a->d.create_enemy);
		a->d.create_enemy = NULL;
		break;
	case ACT_REMOVE_ENEMY:
		action_remove_enemy(level, a->d.delete_enemy);
		break;
	}
}

static void __level_editor_do_action_from_stack(struct list_head *stack)
{
	action *a;

	if (list_empty(stack))
		return;

	// Get the top action from the undo/redo stack
	a = list_entry(stack->next, action, node);

	if (a->type == ACT_MULTIPLE_ACTIONS) {
		// When the action is multiple, we must execute each action,
		// and push all actions in the stack in order to undo
		int i, max;

		max = a->d.number_actions;
		clear_action(a);

		// Execute all actions
		for (i = 0; i < max; i++) {
			__level_editor_do_action_from_stack(stack);
		}

		// Push all actions in the stack in order to undo
		action_push(ACT_MULTIPLE_ACTIONS, max);
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
 * Place an aligned object on the map with the keypad
 * \param positionid The position of the object on the purple grid
 */
void level_editor_place_aligned_object(int positionid)
{
	float position_offset_x[9] = { 0, 0.5, 1.0, 0, 0.5, 1.0, 0, 0.5, 1.0 };
	float position_offset_y[9] = { 1.0, 1.0, 1.0, 0.5, 0.5, 0.5, 0, 0, 0 };
	struct widget_lvledit_categoryselect *cs = get_current_object_type();
	int type = cs->indices[cs->selected_tile_nb];
	pointf pos;

	positionid--;

	// Calculate the position of the object
	pos.x = (int)Me.pos.x + position_offset_x[positionid];
	pos.y = (int)Me.pos.y + position_offset_y[positionid];

	if (!pos_inside_level(pos.x, pos.y, EditLevel()))
	{
		// Do not place an aligned object outside the current level.
		return;
	}

	switch (cs->type) {
	case OBJECT_OBSTACLE:
			action_create_obstacle_user(EditLevel(), pos.x, pos.y, type);
			break;
	case OBJECT_FLOOR:
			action_set_floor(EditLevel(), (int)Me.pos.x, (int)Me.pos.y, type);
			break;
	case OBJECT_ITEM:
			action_create_item(EditLevel(), pos.x, pos.y, type);
			break;
	default:
			break;
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
	int i, k;

	// Get the memory for one level 
	//
	NewLevel = (Level) MyMalloc(sizeof(level));

	DebugPrintf(0, "\n-----------------------------------------------------------------");
	DebugPrintf(0, "\nStarting to create and add a completely new level to the ship.");

	// Now we proceed in the order of the struct 'level' in the
	// struct.h file so that we can easily verify if we've handled
	// all the data structure or left something out which could
	// be terrible!
	//
	NewLevel->levelnum = level_num;
	NewLevel->xlen = 90;
	NewLevel->ylen = 90;
	NewLevel->floor_layers = 1;
	NewLevel->light_bonus = 19;
	NewLevel->minimum_light_value = 19;
	NewLevel->infinite_running_on_this_level = FALSE;
	NewLevel->random_dungeon = 0;
	NewLevel->dungeon_generated = FALSE;
	NewLevel->Levelname = strdup("New level just created");
	NewLevel->Background_Song_Name = strdup("TheBeginning.ogg");

	// First we initialize the floor with 'empty' values
	//
	for (i = 0; i < NewLevel->ylen; i++) {
		NewLevel->map[i] = MyMalloc(NewLevel->xlen * sizeof(map_tile));
		for (k = 0; k < NewLevel->xlen; k++) {
			init_map_tile(&NewLevel->map[i][k]);
		}
	}
	// Now we initialize the level jump interface variables with 'empty' values
	//
	NewLevel->jump_target_north = (-1);
	NewLevel->jump_target_south = (-1);
	NewLevel->jump_target_east = (-1);
	NewLevel->jump_target_west = (-1);

	// Now we initialize the map obstacles with 'empty' information
	//
	for (i = 0; i < MAX_OBSTACLES_ON_MAP; i++) {
		NewLevel->obstacle_list[i].type = (-1);
		NewLevel->obstacle_list[i].pos.x = (-1);
		NewLevel->obstacle_list[i].pos.y = (-1);
		NewLevel->obstacle_list[i].pos.z = level_num;
	}

	// First we initialize the items arrays with 'empty' information
	//
	for (i = 0; i < MAX_ITEMS_PER_LEVEL; i++) {
		NewLevel->ItemList[i].pos.x = (-1);
		NewLevel->ItemList[i].pos.y = (-1);
		NewLevel->ItemList[i].pos.z = (-1);
		NewLevel->ItemList[i].type = (-1);

	}

	// Initialize obstacle extensions
	dynarray_init(&NewLevel->obstacle_extensions, 10, sizeof(struct obstacle_extension));

	// Initialize map labels
	dynarray_init(&NewLevel->map_labels, 10, sizeof(struct map_label));

	// Initialize waypoints
	dynarray_init(&NewLevel->waypoints, 10, sizeof(struct waypoint));
	
	curShip.AllLevels[level_num] = NewLevel;
}

void delete_map_level(int lnum)
{
	// Remove references to this level from others
	level *l = NULL;

	BROWSE_LEVELS(l) {
		if (l->jump_target_north == lnum)
			l->jump_target_north = -1;
		if (l->jump_target_south == lnum)
			l->jump_target_south = -1;
		if (l->jump_target_west == lnum)
			l->jump_target_west = -1;
		if (l->jump_target_east == lnum)
			l->jump_target_east = -1;
	}

	// Free memory
	free_ship_level(curShip.AllLevels[lnum]);
	curShip.AllLevels[lnum] = NULL;

	if (lnum == curShip.num_levels - 1)
		curShip.num_levels--;

	// Removing a level cannot be undo, so we need to clear everything that could have
	// a reference to an object of the deleted level.
	action_freestack();
	clear_selection(-1);
	clear_clipboard(-1);
}

static int get_chest_contents(level *l, obstacle *o, item *items[MAX_ITEMS_IN_INVENTORY])
{
	struct dynarray *itemlist = get_obstacle_extension(l, o, OBSTACLE_EXTENSION_CHEST_ITEMS);

	memset(items, 0, MAX_ITEMS_IN_INVENTORY*sizeof(item *));

	if (!itemlist) {
		return 0;
	}

	int i;
	int curitem = 0;
	for (i = 0; i < itemlist->size && i < MAX_ITEMS_IN_INVENTORY; i++) {
		items[curitem++] = &((item *)itemlist->arr)[i];
	}

	return curitem;
}

void level_editor_edit_chest(struct obstacle *o)
{
	struct item *chest_items[MAX_ITEMS_IN_INVENTORY];
	struct item *user_items[2];
	struct shop_decision shop_order;

	item dummy_addtochest;
	init_item(&dummy_addtochest);
	dummy_addtochest.type = 1;
	FillInItemProperties(&dummy_addtochest, 2, 1);

	user_items[0] = &dummy_addtochest;
	user_items[1] = NULL;

	struct dynarray *itemlist = get_obstacle_extension(CURLEVEL(), o, OBSTACLE_EXTENSION_CHEST_ITEMS);

	// Safety check
	struct obstacle_spec *obs_spec = get_obstacle_spec(o->type);
	if (!obs_spec->action || strncmp(obs_spec->action, "chest", 5)) {
		error_message(__FUNCTION__, "Tried to edit the contents of a chest, but the obstacle is not a chest.",
				     PLEASE_INFORM | IS_FATAL);
	}

	int done = 0;
	while (!done) {

		// Build the list of items in the chest
		int chest_nb_items = get_chest_contents(CURLEVEL(), o, chest_items);

		// Display the shop interface
		done = GreatShopInterface(chest_nb_items, chest_items, 1, user_items, &shop_order);

		// BUY removes an item from the chest
		// SELL spawns the drop item interface
		switch (shop_order.shop_command) {
		case BUY_1_ITEM:
			DeleteItem(chest_items[shop_order.item_selected]);
			dynarray_del(itemlist, shop_order.item_selected, sizeof(item));
			break;
		case SELL_1_ITEM:
			{
				struct item *tmp = ItemDropFromLevelEditor();
				if (tmp) {

					if (!itemlist) {
						itemlist = dynarray_alloc(10, sizeof(item));
						add_obstacle_extension(CURLEVEL(), o, OBSTACLE_EXTENSION_CHEST_ITEMS, itemlist);
					}

					dynarray_add(itemlist, tmp, sizeof(item));

					// delete the ground copy
					DeleteItem(tmp);
				}
			}
			break;
		default:
			;
		}
	}
}

#undef _leveleditor_actions_c
