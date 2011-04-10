/* 
 *
 *   Copyright (c) 2009 Arthur Huillet
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

#include "system.h"

#include "defs.h"
#include "struct.h"
#include "global.h"
#include "proto.h"

#include "lvledit/lvledit.h"
#include "lvledit/lvledit_actions.h"
#include "lvledit/lvledit_widgets.h"
#include "lvledit/lvledit_tools.h"

static struct leveleditor_categoryselect *previous_category = NULL;
static int num_blocks_per_line = 0;

static int display_info = 0;

void leveleditor_toolbar_mouseenter(SDL_Event * event, struct leveleditor_widget *vt)
{
	(void)vt;
}

void leveleditor_toolbar_mouseleave(SDL_Event * event, struct leveleditor_widget *vt)
{
	(void)vt;
	display_info = 0;
}

void leveleditor_toolbar_mouserelease(SDL_Event * event, struct leveleditor_widget *vt)
{
	(void)vt;
}

void leveleditor_toolbar_mousepress(SDL_Event * event, struct leveleditor_widget *vt)
{
	(void)vt;

	struct leveleditor_categoryselect *cs = get_current_object_type();
	int i;
	int x;

	for (i = 0; i < num_blocks_per_line; i++) {
		x = INITIAL_BLOCK_WIDTH / 2 + INITIAL_BLOCK_WIDTH * i;

		if (event->button.x > x && event->button.x < x + INITIAL_BLOCK_WIDTH)
			cs->selected_tile_nb = cs->toolbar_first_block + i;
	}

	for (i = 0; cs->indices[i] != -1; i++) ;
	if (cs->selected_tile_nb >= i - 1)
		cs->selected_tile_nb = i - 1;

}

void leveleditor_toolbar_mouserightrelease(SDL_Event * event, struct leveleditor_widget *vt)
{
	(void)vt;
	display_info = 0;
}

void leveleditor_toolbar_mouserightpress(SDL_Event * event, struct leveleditor_widget *vt)
{
	(void)vt;
	display_info = 1;
	leveleditor_toolbar_mousepress(event, vt);
}

void leveleditor_toolbar_mousewheelup(SDL_Event * event, struct leveleditor_widget *vt)
{
	(void)vt;
	leveleditor_toolbar_left();
}

void leveleditor_toolbar_mousewheeldown(SDL_Event * event, struct leveleditor_widget *vt)
{
	(void)vt;
	leveleditor_toolbar_right();
}

static void print_obstacle_info(char *str, int obs_idx)
{
	int flags = obstacle_map[obs_idx].flags;

	sprintf(str, "Obs. number %d, %s\n", obs_idx, obstacle_map[obs_idx].filename);

	if (flags & IS_HORIZONTAL)
		strcat(str, "- IS_HORIZONTAL\n");
	if (flags & IS_VERTICAL)
		strcat(str, "- IS_VERTICAL\n");
	if (flags & BLOCKS_VISION_TOO)
		strcat(str, "- BLOCKS_VISION_TOO\n");
	if (flags & IS_SMASHABLE)
		strcat(str, "- IS_SMASHABLE\n");
	if (flags & IS_WALKABLE)
		strcat(str, "- IS_WALKABLE\n");
	if (flags & IS_CLICKABLE)
		strcat(str, "- IS_CLICKABLE\n");
}

static void leveleditor_print_object_info(enum leveleditor_object_type type, int *array, int idx, char *str)
{
	switch (type) {
	case OBJECT_FLOOR:
			sprintf(str, "Floor tile number %d\n", array[idx]);
			break;
	case OBJECT_OBSTACLE:
			print_obstacle_info(str, array[idx]);
			break;
	case OBJECT_WAYPOINT:
			sprintf(str, "Waypt %s connection, %s for random spawn\n", "two way", array[idx] ? "NOK" : "OK");
			break;
	case OBJECT_ITEM:
			sprintf(str, "Item name: %s", ItemMap[array[idx]].item_name);
			break;
	default:
			*str = 0;
	}
}

static struct image *leveleditor_get_object_image(enum leveleditor_object_type type, int *array, int idx)
{
	extern struct image level_editor_waypoint_cursor[];
	switch (type) {
	case OBJECT_FLOOR:
			return &(floor_images[array[idx]]);
	case OBJECT_OBSTACLE:
			return get_obstacle_image(array[idx]);
	case OBJECT_WAYPOINT:
			return &(level_editor_waypoint_cursor[idx]);
	case OBJECT_ITEM:
			return get_item_shop_image(array[idx]);
	default:
			ErrorMessage(__FUNCTION__, "Abstract object type %d for leveleditor not supported.\n", PLEASE_INFORM, IS_FATAL, type);
			break;
	}

	return NULL;
}

void leveleditor_toolbar_display(struct leveleditor_widget *vt)
{
	(void)vt;
	struct leveleditor_categoryselect *cs = get_current_object_type();

	if (cs != previous_category) {
		previous_category = cs;
	}

	int number_of_tiles = 0;
	int i;
	int cindex = cs->toolbar_first_block;
	float zoom_factor;
	SDL_Rect TargetRectangle;

	// toolbar background
	draw_rectangle(&vt->rect, 85, 100, 100, 150);

	// now the tiles to be selected   

	// compute the number of tiles in this list
	for (i = 0; cs->indices[i] != -1; i++) ;
	number_of_tiles = i;

	if (num_blocks_per_line == 0) {
		num_blocks_per_line = GameConfig.screen_width / INITIAL_BLOCK_WIDTH - 1;
	}

	for (i = 0; i < num_blocks_per_line; i++) {

		if (cindex >= number_of_tiles)
			break;

		TargetRectangle.x = INITIAL_BLOCK_WIDTH / 2 + INITIAL_BLOCK_WIDTH * i;
		TargetRectangle.y = vt->rect.y + 2;
		TargetRectangle.w = INITIAL_BLOCK_WIDTH;
		TargetRectangle.h = INITIAL_BLOCK_HEIGHT;

		struct image *img = leveleditor_get_object_image(cs->type, cs->indices, cindex);
		if (!img)
			break;

		zoom_factor = min(((float)INITIAL_BLOCK_WIDTH / (float)img->w),
				((float)INITIAL_BLOCK_HEIGHT / (float)img->h));

		display_image_on_screen(img, TargetRectangle.x - img->offset_x * zoom_factor, TargetRectangle.y - img->offset_y * zoom_factor, IMAGE_SCALE_TRANSFO(zoom_factor));

		if (cindex == cs->selected_tile_nb) {
			HighlightRectangle(Screen, TargetRectangle);
			if (display_info) {
				// Display information about the currently selected object
				leveleditor_print_object_info(cs->type, cs->indices, cindex, VanishingMessage);
				VanishingMessageEndDate = SDL_GetTicks() + 100;
			}
		}

		cindex++;
	}
}

void leveleditor_toolbar_left()
{
	struct leveleditor_categoryselect *cs = get_current_object_type();

	cs->selected_tile_nb = cs->selected_tile_nb <= 1 ? 0 : cs->selected_tile_nb - 1;

	if (cs->selected_tile_nb < cs->toolbar_first_block)
		cs->toolbar_first_block--;
}

void leveleditor_toolbar_right()
{
	struct leveleditor_categoryselect *cs = get_current_object_type();
	int i;

	for (i = 0; cs->indices[i] != -1; i++) ;

	cs->selected_tile_nb = cs->selected_tile_nb >= i - 1 ? i - 1 : cs->selected_tile_nb + 1;

	if (cs->selected_tile_nb >= cs->toolbar_first_block + num_blocks_per_line) {
		cs->toolbar_first_block++;
	}
}

void leveleditor_toolbar_scroll_left()
{
	struct leveleditor_categoryselect *cs = get_current_object_type();

	if (cs->toolbar_first_block < 8)
		cs->toolbar_first_block = 0;
	else
		cs->toolbar_first_block -= 8;

	if (cs->toolbar_first_block + num_blocks_per_line <= cs->selected_tile_nb) {
		cs->selected_tile_nb = cs->toolbar_first_block + num_blocks_per_line - 1;
	}
}

void leveleditor_toolbar_scroll_right()
{
	struct leveleditor_categoryselect *cs = get_current_object_type();
	int i;

	for (i = 0; cs->indices[i] != -1; i++) ;

	cs->toolbar_first_block += 8;
	if (cs->toolbar_first_block >= i - 1)
		cs->toolbar_first_block -= 8;

	if (cs->selected_tile_nb < cs->toolbar_first_block)
		cs->selected_tile_nb = cs->toolbar_first_block;
}
