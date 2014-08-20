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

static struct widget_lvledit_categoryselect *previous_category = NULL;
static int num_blocks_per_line = 0;

static int display_info_idx = 0;

static int toolbar_mousepress(struct widget *vt, SDL_Event *event, int select)
{
	struct widget_lvledit_categoryselect *cs = get_current_object_type();
	int i;
	int x;
	int idx = -1;

	for (i = 0; i < num_blocks_per_line; i++) {
		x = INITIAL_BLOCK_WIDTH / 2 + INITIAL_BLOCK_WIDTH * i;

		if (event->button.x > x && event->button.x < x + INITIAL_BLOCK_WIDTH)
			idx = cs->toolbar_first_block + i;
	}

	for (i = 0; cs->indices[i] != -1; i++) ;
	if (idx >= i - 1)
		idx = i - 1;

	if (select && idx != -1)
		cs->selected_tile_nb = idx;

	return idx;
}

static int toolbar_handle_event(struct widget *w, SDL_Event *event)
{
	switch (event->type) {
		case SDL_MOUSEBUTTONDOWN:
			switch (event->button.button) {
				case MOUSE_BUTTON_3:
					display_info_idx = toolbar_mousepress(w, event, FALSE);
					break;

				case MOUSE_BUTTON_1:
					toolbar_mousepress(w, event, TRUE);
					break;	

				case SDL_BUTTON_WHEELUP:
					widget_lvledit_toolbar_left();
					break;	

				case SDL_BUTTON_WHEELDOWN:
					widget_lvledit_toolbar_right();
					break;	

				default:
					return 0;
			}
			return 1;

		case SDL_MOUSEMOTION:
			return 1;

		case SDL_MOUSEBUTTONUP:
			if (event->button.button == 3)
				display_info_idx = -1;
			return 1;

		case SDL_USEREVENT:
			if (event->user.code == EVENT_MOUSE_LEAVE)
				display_info_idx = -1;
			break;

		default:
			break;
	}
	return 0;
}

static void print_obstacle_info(char *str, int obs_idx)
{
	int flags = get_obstacle_spec(obs_idx)->flags;

	// ;TRANSLATORS: "Obs." is for "obstacle" please keep it abbreviated in your language respectively
	sprintf(str, _("Obs. number %d, %s\n"), obs_idx, ((char **)get_obstacle_spec(obs_idx)->filenames.arr)[0]);
	// these refer to the internal flags, so let's not translate them for now...
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

static void print_enemy_info(char *str, int en_idx)
{
	sprintf(str, 	_("%s\n\
			class: %d, xp_reward: %hd\n\
			heal: %.2f /s, max_energy: %.2f, max_speed: %.2f\n\
			aggression_dist: %.2f, eyeing_tux_for: %.2f s"),
				Droidmap[en_idx].droidname,
				Droidmap[en_idx].class, Droidmap[en_idx].experience_reward,
				Droidmap[en_idx].healing_friendly, Droidmap[en_idx].maxenergy, Droidmap[en_idx].maxspeed,
				Droidmap[en_idx].aggression_distance, Droidmap[en_idx].time_spent_eyeing_tux);
}

static void print_item_info(char *str, int item_idx)
{
	struct itemspec *item = &ItemMap[item_idx];
	if (item->weapon_ammo_type) { // Weapons with ammunition: mostly guns, or melee weapons needing a power pack
		sprintf(str, 	_("%s (%s)\n\
			Damage: %d-%d, Attack Time: %.2fs, Reload Time: %.2fs, Ammo: (%d/%s)\n\
			STR: >%d\n%s"),
				item_specs_get_name(item_idx), item->id,
				item->weapon_base_damage, item->weapon_base_damage + item->weapon_damage_modifier,
				item->weapon_attack_time, item->weapon_reloading_time,
				item->weapon_ammo_clip_size, item->weapon_ammo_type,
				item->item_require_strength,
				item->weapon_needs_two_hands ? _("Requires two hands\n") : "" );

	} else if (item->slot == WEAPON_SLOT) { // Other weapons
		sprintf(str, 	_("%s (%s)\n\
			Damage: %d-%d, Attack Time: %.2fs,\n\
			STR: >%d, DEX: >%d, COOL: >%d\n%s"),
				item_specs_get_name(item_idx), item->id,
				item->weapon_base_damage, item->weapon_base_damage + item->weapon_damage_modifier,
				item->weapon_attack_time,
				item->item_require_strength, item->item_require_dexterity, item->item_require_cooling,
				item->weapon_needs_two_hands ? _("Requires two hands\n") : "" );

	} else if (item->slot & (SHIELD_SLOT | HELM_SLOT | ARMOR_SLOT | BOOT_SLOT)) {
		sprintf(str, 	_("%s (%s)\n\
			Armor: %d-%d, Durability:  %d-%d,\n\
			STR: >%d, DEX: >%d, COOL: >%d\n"),
				item_specs_get_name(item_idx), item->id,
				item->base_armor_class, item->base_armor_class + item->armor_class_modifier,
				item->base_item_durability, item->base_item_durability + item->item_durability_modifier,
				item->item_require_strength, item->item_require_dexterity, item->item_require_cooling);

	} else {
		sprintf(str, "%s (%s)\n%s",
			item_specs_get_name(item_idx), item->id, item->item_description);
	}
}

static void leveleditor_print_object_info(enum lvledit_object_type type, int *array, int idx, char *str)
{
	switch (type) {
	case OBJECT_FLOOR:
			if (array[idx] >= MAX_UNDERLAY_FLOOR_TILES) {
				int index = array[idx] - MAX_UNDERLAY_FLOOR_TILES;
				struct floor_tile_spec *floor_tile = dynarray_member(&overlay_floor_tiles, index, sizeof(struct floor_tile_spec));
				sprintf(str, _("Overlay floor tile number %d, filename %s\n"), index, ((char **)floor_tile->filenames.arr)[0]);
			} else {
				struct floor_tile_spec *floor_tile = dynarray_member(&underlay_floor_tiles, array[idx], sizeof(struct floor_tile_spec));
				sprintf(str, _("Underlay floor tile number %d, filename %s\n"), array[idx], ((char **)floor_tile->filenames.arr)[0]);
			}
			break;
	case OBJECT_OBSTACLE:
			print_obstacle_info(str, array[idx]);
			break;
	case OBJECT_WAYPOINT:
			sprintf(str, _("Waypoint %s connection, %sused for random spawn\n"), _("two way"), array[idx] ? _("not ") : "");
			break;
	case OBJECT_ITEM:
			print_item_info(str, array[idx]);
			break;
	case OBJECT_ENEMY:
			print_enemy_info(str, array[idx]);
			break;
	case OBJECT_MAP_LABEL:
			sprintf(str, _("There currently is only one type of map label"));
			break;
	default:
			*str = 0;
	}
}

static struct image *leveleditor_get_object_image(enum lvledit_object_type type, int *array, int idx)
{
	extern struct image level_editor_waypoint_cursor[];
	switch (type) {
	case OBJECT_FLOOR:
			return get_floor_tile_image(array[idx]);
	case OBJECT_OBSTACLE:
			return get_obstacle_image(array[idx], 0);
	case OBJECT_WAYPOINT:
			return &(level_editor_waypoint_cursor[idx]);
	case OBJECT_ITEM:
			return get_item_shop_image(array[idx]);
	case OBJECT_MAP_LABEL:
			return get_map_label_image();
	case OBJECT_ENEMY:
			return get_droid_portrait_image(array[idx]);
	default:
			error_message(__FUNCTION__, "Abstract object type %d for leveleditor not supported.", PLEASE_INFORM | IS_FATAL, type);
			break;
	}

	return NULL;
}

static void toolbar_display(struct widget *vt)
{
	(void)vt;
	struct widget_lvledit_categoryselect *cs = get_current_object_type();

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

		if (!image_loaded(img)) {
			cindex++;
			continue;
		}

		zoom_factor = min(((float)INITIAL_BLOCK_WIDTH / (float)img->w),
				((float)INITIAL_BLOCK_HEIGHT / (float)img->h));

		display_image_on_screen(img, TargetRectangle.x - img->offset_x * zoom_factor, TargetRectangle.y - img->offset_y * zoom_factor, IMAGE_SCALE_TRANSFO(zoom_factor));

		if (cindex == cs->selected_tile_nb) {
			HighlightRectangle(Screen, TargetRectangle);
			if (display_info_idx != -1) {
				// Display information about the currently selected object
				leveleditor_print_object_info(cs->type, cs->indices, display_info_idx, VanishingMessage);
				VanishingMessageEndDate = SDL_GetTicks() + 100;
			}
		}

		cindex++;
	}
}

struct widget *widget_lvledit_toolbar_create()
{
	struct widget *a = MyMalloc(sizeof(struct widget));
	widget_init(a);
	a->type = WIDGET_TOOLBAR;
	widget_set_rect(a, 0, 0, GameConfig.screen_width, 73);
	a->display = toolbar_display;
	a->handle_event = toolbar_handle_event;

	struct widget_lvledit_toolbar *t = MyMalloc(sizeof(struct widget_lvledit_toolbar));
	a->ext = t;

	return a;
}

void widget_lvledit_toolbar_left()
{
	struct widget_lvledit_categoryselect *cs = get_current_object_type();

	cs->selected_tile_nb = cs->selected_tile_nb <= 1 ? 0 : cs->selected_tile_nb - 1;

	if (cs->selected_tile_nb < cs->toolbar_first_block)
		cs->toolbar_first_block--;
}

void widget_lvledit_toolbar_right()
{
	struct widget_lvledit_categoryselect *cs = get_current_object_type();
	int i;

	for (i = 0; cs->indices[i] != -1; i++) ;

	cs->selected_tile_nb = cs->selected_tile_nb >= i - 1 ? i - 1 : cs->selected_tile_nb + 1;

	if (cs->selected_tile_nb >= cs->toolbar_first_block + num_blocks_per_line) {
		cs->toolbar_first_block++;
	}
}

void widget_lvledit_toolbar_scroll_left()
{
	struct widget_lvledit_categoryselect *cs = get_current_object_type();

	if (cs->toolbar_first_block < 8)
		cs->toolbar_first_block = 0;
	else
		cs->toolbar_first_block -= 8;

	if (cs->toolbar_first_block + num_blocks_per_line <= cs->selected_tile_nb) {
		cs->selected_tile_nb = cs->toolbar_first_block + num_blocks_per_line - 1;
	}
}

void widget_lvledit_toolbar_scroll_right()
{
	struct widget_lvledit_categoryselect *cs = get_current_object_type();
	int i;

	for (i = 0; cs->indices[i] != -1; i++) ;

	cs->toolbar_first_block += 8;
	if (cs->toolbar_first_block >= i - 1)
		cs->toolbar_first_block -= 8;

	if (cs->selected_tile_nb < cs->toolbar_first_block)
		cs->selected_tile_nb = cs->toolbar_first_block;
}
