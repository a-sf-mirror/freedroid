/* 
 *
 *   Copyright (c) 1994, 2002, 2003, 2004 Johannes Prix
 *   Copyright (c) 1994, 2002 Reinhard Prix
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
 * This file contains all functions to update and draw the top status 
 * displays with status etc...
 */

#define _hud_c

#include "system.h"

#include "defs.h"
#include "struct.h"
#include "proto.h"
#include "global.h"
#include "widgets/widgets.h"

#define TEXT_BANNER_DEFAULT_FONT FPS_Display_BFont

int best_banner_pos_x, best_banner_pos_y;

/**
 * The HUD contains several status graphs.  These graphs appear as 
 * vertical columns, that are more or less filled, like liquid in a tube.
 * Since these appear multiple times, it appears sensible to make a 
 * function to draw such bars in a convenient way, which is what this
 * function is supposed to do.
 */
void blit_vertical_status_bar(float max_value, float current_value, Uint32 filled_color_code,
			 Uint32 empty_color_code, int x, int y, int w, int h)
{
	SDL_Rect running_power_rect;
	SDL_Rect un_running_power_rect;
	uint8_t r, g, b, a;

	if (!max_value) {
		max_value = 1;
		current_value = 1;
	}
	// Now we might get the case of current value exceeding the max value by far.  This 
	// does happen when we set ridiculously high energy values for invincible Tux in the
	// course of testing and convenient debugging.  To prevent arithmetic exceptions, we
	// set precautions to allow for a maximum of 3 times the full scale to represent 
	// extremely high (energy) values.
	//
	if (current_value > 3 * max_value)
		current_value = 3 * max_value;

	running_power_rect.x = x;
	running_power_rect.y = y + ((h * (max_value - current_value)) / max_value);
	running_power_rect.w = w;
	running_power_rect.h = (h * current_value) / max_value;
	if (current_value < 0)
		running_power_rect.h = 0;

	un_running_power_rect.x = running_power_rect.x;
	un_running_power_rect.y = y;
	un_running_power_rect.w = w;
	un_running_power_rect.h = h - ((h * current_value) / max_value);
	if (current_value < 0)
		un_running_power_rect.h = h;
	if (current_value > max_value)
		un_running_power_rect.h = 0;

	// Now that all our rects are set up, we can start to display the current
	// running power status on screen...
	//
	SDL_SetClipRect(Screen, NULL);

	SDL_GetRGBA(filled_color_code, Screen->format, &r, &g, &b, &a);
	draw_rectangle(&running_power_rect, r, g, b, a);

	SDL_GetRGBA(empty_color_code, Screen->format, &r, &g, &b, &a);
	draw_rectangle(&un_running_power_rect, r, g, b, a);
};				// void blit_vertical_status_bar ( ... )

/**
 * This function writes the description of an item into the item description
 * string.
 *
 *  Note: We do not want a trailing newline, since that will make text areas
 *  larger than necessary.
 */
void append_item_description(struct auto_string *str, item *item)
{
	if (item == NULL)
		return;

	if (item->type == (-1)) {
		error_message(__FUNCTION__, "\
An item description was requested for an item, that does not seem to \n\
exist really (i.e. has a type = (-1) ).", PLEASE_INFORM | IS_FATAL);
		return;
	}

	// Get the pure item name, also with font changes enabled.
	append_item_name(item, str);

	// We don't want any more information for Valuable Circuits
	if (item_spec_eq_id(item->type, "Valuable Circuits"))
		return;
	
	autostr_append(str, "\n");

	// Weapon damage
	if (ItemMap[item->type].slot == WEAPON_SLOT) {
		if (item->damage_modifier) {
			// TRANSLATORS: On item description: range
			autostr_append(str, _("Damage: %d to %d\n"), item->damage, item->damage_modifier + item->damage);
		} else {
			// TRANSLATORS: On item description
			autostr_append(str, _("Damage: %d\n"), item->damage);
		}
	}
	// Multiplicity
	if (ItemMap[item->type].item_group_together_in_inventory) {
		// TRANSLATORS: On item description
		autostr_append(str, _("Multiplicity: %d\n"), item->multiplicity);
	}
	// Armor bonus
	if (item->armor_class) {
		// TRANSLATORS: On item description: Armor bonus
		autostr_append(str, _("Armor: %d\n"), item->armor_class);
	}
	// Durability or indestructible status
	if (item->max_durability != (-1)) {
		// TRANSLATORS: On item description: 'current' of 'maximum'
		autostr_append(str, _("Durability: %d of %d\n"), (int)item->current_durability, (int)item->max_durability);
	} else if (ItemMap[item->type].base_item_durability != (-1)) {
		// TRANSLATORS: On item description : durability
		autostr_append(str, _("Indestructible\n"));
	}
	// Ranged weapon ammunition
	if (ItemMap[item->type].item_gun_ammo_clip_size) {
		// TRANSLATORS: On item description: 'current' of 'maximum' amount of ammo
		autostr_append(str, _("Ammo: %d of %d\n"), item->ammo_clip, ItemMap[item->type].item_gun_ammo_clip_size);
	}
	// Strength, dexterity or cooling requirements
	if ((ItemMap[item->type].item_require_strength != (-1)) || (ItemMap[item->type].item_require_dexterity != (-1))) {
		if (ItemMap[item->type].item_require_strength != (-1)) {
			// TRANSLATORS: On item description
			autostr_append(str, _("Required strength: %d\n"), ItemMap[item->type].item_require_strength);
		}
		if (ItemMap[item->type].item_require_dexterity != (-1)) {
			// TRANSLATORS: On item description
			autostr_append(str, _("Required dexterity: %d\n"), ItemMap[item->type].item_require_dexterity);
		}
	}

	// Usable items should say that they can be used via right-clicking on it
	if (ItemMap[item->type].right_use.tooltip) {
		if (game_status == INSIDE_GAME) {
			autostr_append(str, "%s\n", D_(ItemMap[item->type].right_use.tooltip));

			// Show text only if item is in inventory
			if (item->inventory_position.x != -1) {
				autostr_append(str, "\n");
				autostr_append(str, _("Right click to use\n"));
			}
		} else {
			autostr_append(str, _("Item use: %s\n"), D_(ItemMap[item->type].right_use.tooltip));
		}
	}

	// Socket count
	if (item->upgrade_sockets.size) {
		// TRANSLATORS: On item description: 'used sockets' / 'number of sockets'
		autostr_append(str, _("Sockets: used %d/%d\n"), count_used_sockets(item), 
							     item->upgrade_sockets.size);
	}

	// Item bonuses
	get_item_bonus_string(item, "\n", str);

	// Add-on specific information
	struct addon_spec *addon = get_addon_spec(item->type);
	if (addon) {
		print_addon_description(addon, str);
	}
}

/**
 * This function writes the description of a droid above its head,
 * and shows the remaining energy.
 */
static void show_droid_description(enemy *cur_enemy, gps *description_pos)
{
	int text_length;
	int bar_width; //size of the energy bar
	int barc_width; //size of the enery bar complement (black part)
	int bar_y;
	int bar_x;
	SDL_Rect rect;
	BFont_Info *BFont_to_use = Blue_BFont;
	Uint8 r, g, b;

	text_length = text_width(BFont_to_use, D_(cur_enemy->short_description_text));

	rect.h = FontHeight(BFont_to_use);

	// Hostile droids' bars are shown in red, friendly in green.
	if (!is_friendly(cur_enemy->faction, FACTION_SELF)) {
		r = 0x99;
		g = 0x00;
		b = 0x00;
	} else {
		r = 0x00;
		g = 0x55;
		b = 0x00;
	}

	// Position of the bar
	bar_x = translate_map_point_to_screen_pixel_x(description_pos->x, description_pos->y) - text_length / 2;
	bar_y =
	    translate_map_point_to_screen_pixel_y(description_pos->x,
	    		description_pos->y) + enemy_images[Droidmap[cur_enemy->type].individual_shape_nr][0][0].offset_y -
	    2.5 * FontHeight(BFont_to_use);

	// Calculates the width of the remaining health bar. Rounds the
	// width up to the nearest integer to ensure that at least one
	// pixel of health is always shown.
	//
	bar_width = (int) ceil((text_length) * (cur_enemy->energy / Droidmap[cur_enemy->type].maxenergy));
	barc_width = (int) floor((text_length) * (1.0 - cur_enemy->energy / Droidmap[cur_enemy->type].maxenergy));
	if (bar_width < 0)
		bar_width = 0;
	if (barc_width < 0)
		barc_width = 0;


	// Draw the energy bar
	rect.x = bar_x;
	rect.y = bar_y;
	rect.w = bar_width;
	draw_rectangle(&rect, r, g, b, BACKGROUND_TEXT_RECT_ALPHA);

	// Draw the energy bar complement
	rect.x = bar_x + bar_width;
	rect.y = bar_y;
	rect.w = barc_width;
	draw_rectangle(&rect, 0, 0, 0, 255);

	// Display droid's short description text
	rect.x = translate_map_point_to_screen_pixel_x(description_pos->x, description_pos->y) - text_length / 2;
	put_string(BFont_to_use, rect.x, rect.y, D_(cur_enemy->short_description_text));
}

/**
 * This function sets up the text, that is to appear in a bigger text
 * rectangle, possibly next to the mouse cursor, e.g. when the mouse is
 * hovering over an item or barrel or crate or teleporter.
 */
static void prepare_text_window_content(struct auto_string *str)
{
	point CurPos;
	point inv_square;
	int InvIndex;
	int index_of_obst_below_mouse_cursor = (-1);
	int index_of_floor_item_below_mouse_cursor = (-1);

	CurPos.x = GetMousePos_x();
	CurPos.y = GetMousePos_y();

	best_banner_pos_x = CurPos.x + 20;
	best_banner_pos_y = CurPos.y;

	autostr_printf(str, "");

	/* If the player has an item in hand, draw the item name into the
	 * description field.  If the requirements for this item are not met, we
	 * show a text. */
	if (item_held_in_hand != NULL) {
		autostr_printf(str, "%s%s", font_switchto_neon, D_(item_specs_get_name(item_held_in_hand->type)));

		if (!ItemUsageRequirementsMet(item_held_in_hand, FALSE)) {
			autostr_append(str, "\n%s%s", font_switchto_red, _("REQUIREMENTS NOT MET"));
		}
		return;
	}
	// in the other case however, that no item is currently held in hand, we need to
	// work a little more:  we need to find out if the cursor is currently over some
	// inventory or other item and in case that's true, we need to give the 
	// description of this item.
	//
	else if (GameConfig.Inventory_Visible) {
		// Perhaps the cursor is over some item of the inventory?
		// let's check this case first.
		if (MouseCursorIsInInventoryGrid(CurPos.x, CurPos.y)) {
			inv_square.x = GetInventorySquare_x(CurPos.x);
			inv_square.y = GetInventorySquare_y(CurPos.y);
			InvIndex = GetInventoryItemAt(inv_square.x, inv_square.y);
			if (InvIndex != (-1)) {
				append_item_description(str, &(Me.Inventory[InvIndex]));
				best_banner_pos_x =
				    (Me.Inventory[InvIndex].inventory_position.x +
				     ItemMap[Me.Inventory[InvIndex].type].inv_size.x) * 30 + 16;
				best_banner_pos_y = 300;
			}
		} else if (MouseCursorIsOnButton(WEAPON_RECT_BUTTON, CurPos.x, CurPos.y)) {
			if (Me.weapon_item.type > 0) {
				append_item_description(str, &(Me.weapon_item));
				best_banner_pos_x = WEAPON_RECT_X + 30 + WEAPON_RECT_WIDTH;
				best_banner_pos_y = WEAPON_RECT_Y - 30;
			}
		} else if (MouseCursorIsOnButton(DRIVE_RECT_BUTTON, CurPos.x, CurPos.y)) {
			if (Me.drive_item.type > 0) {
				append_item_description(str, &(Me.drive_item));
				best_banner_pos_x = DRIVE_RECT_X + 30 + DRIVE_RECT_WIDTH;
				best_banner_pos_y = DRIVE_RECT_Y - 30;
			}
		} else if (MouseCursorIsOnButton(SHIELD_RECT_BUTTON, CurPos.x, CurPos.y)) {
			if (Me.shield_item.type > 0) {
				append_item_description(str, &(Me.shield_item));
				best_banner_pos_x = SHIELD_RECT_X + 30 + SHIELD_RECT_WIDTH;
				best_banner_pos_y = SHIELD_RECT_Y - 30;
			} else if (Me.weapon_item.type > 0) {
				if (ItemMap[Me.weapon_item.type].item_gun_requires_both_hands) {
					append_item_description(str, &(Me.weapon_item));
					best_banner_pos_x = SHIELD_RECT_X + 30 + SHIELD_RECT_WIDTH;
					best_banner_pos_y = SHIELD_RECT_Y - 30;
				}
			}
		} else if (MouseCursorIsOnButton(ARMOUR_RECT_BUTTON, CurPos.x, CurPos.y)) {
			if (Me.armour_item.type > 0) {
				append_item_description(str, &(Me.armour_item));
				best_banner_pos_x = ARMOUR_RECT_X + 30 + ARMOUR_RECT_WIDTH;
				best_banner_pos_y = ARMOUR_RECT_Y - 30;
			}
		} else if (MouseCursorIsOnButton(HELMET_RECT_BUTTON, CurPos.x, CurPos.y)) {
			if (Me.special_item.type > 0) {
				append_item_description(str, &(Me.special_item));
				best_banner_pos_x = HELMET_RECT_X + 30 + HELMET_RECT_WIDTH;
				best_banner_pos_y = HELMET_RECT_Y - 30;
			}
		}
	}			// if nothing is 'held in hand' && inventory-screen visible

	// Check if the crafting UI is open and the cursor is inside it.
	// No banner should be shown if that's the case.
	if (cursor_is_on_addon_crafting_ui(&CurPos)) {
		return;
	}

	// Check if the item upgrade UI is open and the cursor is inside it.
	// We show a tooltip for the item upgrade UI if that's the case.
	if (append_item_upgrade_ui_tooltip(&CurPos, str)) {
		return;
	}

	// If the mouse cursor is within the user rectangle, then we check if
	// either the cursor is over an inventory item or over some other droid
	// and in both cases, we give a description of the object in the small
	// black rectangle in the top status banner.
	//

	if (MouseCursorIsInUserRect(CurPos.x, CurPos.y)) {
		level *obj_lvl = NULL;
		
		index_of_floor_item_below_mouse_cursor = get_floor_item_index_under_mouse_cursor(&obj_lvl);
		
		if (index_of_floor_item_below_mouse_cursor != (-1) && obj_lvl != NULL) {
			gps item_vpos;
			update_virtual_position(&item_vpos, &(obj_lvl->ItemList[index_of_floor_item_below_mouse_cursor].pos), Me.pos.z);
			if (item_vpos.x != -1) {
				append_item_description(str, &(obj_lvl->ItemList[index_of_floor_item_below_mouse_cursor]));
				best_banner_pos_x =	translate_map_point_to_screen_pixel_x(item_vpos.x, item_vpos.y) + 80;
				best_banner_pos_y =	translate_map_point_to_screen_pixel_y(item_vpos.x, item_vpos.y) - 30;
			}
		}

		// Display Clickable Obstacle label  in the top status banner.
		//
		obj_lvl = NULL;
		index_of_obst_below_mouse_cursor = clickable_obstacle_below_mouse_cursor(&obj_lvl);
		if (index_of_obst_below_mouse_cursor != (-1)) {
			gps obst_vpos;
			update_virtual_position(&obst_vpos, &(obj_lvl->obstacle_list[index_of_obst_below_mouse_cursor].pos), Me.pos.z);
			if (obst_vpos.x != -1) {
				char *label =  D_(get_obstacle_spec(obj_lvl->obstacle_list[index_of_obst_below_mouse_cursor].type)->label);
				if (!label) {
					error_message(__FUNCTION__, "Obstacle type %d is clickable, and as such requires a label to be displayed on mouseover.", PLEASE_INFORM, obj_lvl->obstacle_list[index_of_obst_below_mouse_cursor].type);
					label = _("No label for this obstacle");
				}

				autostr_printf(str, label);
				best_banner_pos_x = translate_map_point_to_screen_pixel_x(obst_vpos.x, obst_vpos.y) + 50;
				best_banner_pos_y = translate_map_point_to_screen_pixel_y(obst_vpos.x, obst_vpos.y) - 20;
			}
		}

		// Maybe there is a teleporter event connected to the square where the mouse
		// cursor is currently hovering.  In this case we should create a message about
		// where the teleporter connection would bring the Tux...
		//
		if (teleporter_square_below_mouse_cursor()) {
			autostr_append(str, "%s", teleporter_square_below_mouse_cursor());
		}
		// Maybe there is a living droid below the current mouse cursor, and it is visible to the player.
		// In this case, we'll give the description of the corresponding bot.
		// Note : the call to GetLivingDroidBelowMouseCursor() does set the virt_pos attribute
		// of the found droid to be the bot's position relatively to Tux current level
		//
		enemy *droid_below_mouse_cursor = GetLivingDroidBelowMouseCursor();
		if (droid_below_mouse_cursor != NULL
		    && DirectLineColldet(Me.pos.x, Me.pos.y, droid_below_mouse_cursor->virt_pos.x, droid_below_mouse_cursor->virt_pos.y, Me.pos.z,
					 &VisiblePassFilter)) {
			show_droid_description(droid_below_mouse_cursor, &droid_below_mouse_cursor->virt_pos);
			return;
		}
	}
}

/**
 * At various points in the game, especially when the mouse in over an
 * interesting object inside the game, a text banner will appear, e.g.
 * to describe the item in question.
 */
void show_current_text_banner(void)
{
	static struct auto_string *txt = NULL;
	if (txt == NULL)
		txt = alloc_autostr(200);

	// Prepare the string, that is to be displayed inside the text rectangle
	prepare_text_window_content(txt);

	SDL_Rect rect;
	rect.x = best_banner_pos_x;
	rect.y = best_banner_pos_y;
	rect.w = 0;
	rect.h = 0;

	display_tooltip(txt->value, 1, rect);
}

/**
 * This function displays a tooltip in the specified rectangle. 
 * The rectangle size can be specified or left to 0, meaning the rectangle will be
 * expanded to fit the text. 
 * @param text Text to be displayed.
 * @param centered Flag marking whether the text will be centered or not.
 * @param rect Rectangle in which the tooltip will be displayed.
 */
void display_tooltip(const char *text, int centered, SDL_Rect rect)
{
	// Displaying the text aligned to center modifies the string
	// so we make a temporary copy to keep the original text untouched.
	char buffer[strlen(text) + 1];
	strcpy(buffer, text);

	// Set font before making any font specific calculations.
	SetCurrentFont(TEXT_BANNER_DEFAULT_FONT);

	// If the width is not specified, expand the rectangle to fit
	// the longest line width.
	if (!rect.w)
		rect.w = longest_line_width(buffer) + 2 * TEXT_BANNER_HORIZONTAL_MARGIN;

	// Compute the required height.
	int lines_in_text = get_lines_needed(buffer, rect, LINE_HEIGHT_FACTOR);
	rect.h = lines_in_text * FontHeight(GetCurrentFont());
	
	// Add extra correction to ensure the banner rectangle stays inside
	// the visible screen.
	if (rect.x < 1)
		rect.x = 1;
	else if (rect.x + rect.w > GameConfig.screen_width - 1)
		rect.x = GameConfig.screen_width - rect.w - 1;
	if (rect.y < 1)
		rect.y = 1;
	else if (rect.y + rect.h > GameConfig.screen_height - 1)
		rect.y = GameConfig.screen_height - rect.h - 1;
	
	// Compute actual coordinates for the text.
	SDL_Rect text_rect = rect;
	text_rect.x += TEXT_BANNER_HORIZONTAL_MARGIN;
	text_rect.w -= 2 * TEXT_BANNER_HORIZONTAL_MARGIN;

	// Draw the background rectangle
	SDL_SetClipRect(Screen, NULL);	// this unsets the clipping rectangle
	draw_rectangle(&rect, 0, 0, 0, BACKGROUND_TEXT_RECT_ALPHA);
	
	// Print the text.
	if (!centered) {
		display_text_using_line_height(buffer, text_rect.x, text_rect.y, &text_rect, 1.0);
		return;
	}

	// Print the text centered.
	int line_spacing = (text_rect.h - lines_in_text * FontHeight(GetCurrentFont())) / (lines_in_text + 1);
	char *ptr = buffer;
	int i;
	for (i = 0; i < lines_in_text; i++) {
		char *this_line = ptr;
		char *next_newline = strstr(ptr, "\n");
		if (next_newline) {
			int pos = next_newline - ptr;
			this_line[pos] = '\0';
			ptr += pos + 1;
		}
		int offset = (text_rect.w - text_width(GetCurrentFont(), this_line)) / 2;
		put_string(GetCurrentFont(),
			      text_rect.x + offset,
			      text_rect.y + line_spacing + i * (line_spacing + FontHeight(GetCurrentFont())), this_line);
	}
}

/** 
 * This function derives the 'minutes' component of the time already 
 * elapsed in this game.
 */
int get_minutes_of_game_duration(float current_game_date)
{
	return (((int)(10 * current_game_date / (60))) % 60);
};				// void get_minutes_of_game_duration ( float current_game_date )

/** 
 * This function derives the 'hours' component of the time already 
 * elapsed in this game.
 */
int get_hours_of_game_duration(float current_game_date)
{
	return (((int)(10 * current_game_date / (60 * 60))) % 24);
};				// void get_hours_of_game_duration ( float current_game_date )

/** 
 * This function derives the 'days' component of the time already 
 * elapsed in this game.
 */
int get_days_of_game_duration(float current_game_date)
{
	return (((int)(1 + 10 * current_game_date / (60 * 60 * 24))));
};				// void get_days_of_game_duration ( float current_game_date )

/**
 * Add a new message to the game log.
 */
void append_new_game_message(const char *fmt, ...)
{
	autostr_append(message_log->text, "\n* ");

	va_list args;
	va_start(args, fmt);
	autostr_vappend(message_log->text, fmt, args);
	va_end(args);

	message_log->scroll_offset = 0;
}

/**
 * Initialize or reset the message log.
 */
void init_message_log(void)
{
	if (!message_log)
		message_log = widget_text_create();
}

/**
 * Calculate the current FPS and return it.
 */
int get_current_fps(void)
{
	static float time_since_last_fps_update = 10;
	static int frames_counted = 0;
	static int current_fps = 0;

	time_since_last_fps_update += Frame_Time();
	frames_counted++;
	if (frames_counted > 50) {
		current_fps = frames_counted / time_since_last_fps_update;
		time_since_last_fps_update = 0;
		frames_counted = 0;
	}
	return current_fps;
}


/**
 * Show the texts that are usually shown in the top left corner e.g. the FPS.
 */
static void show_top_left_text(void)
{
	SDL_Rect clip;
	int i;
	int remaining_bots;
	static struct auto_string *txt;
	if (txt == NULL)
		txt = alloc_autostr(200);
	autostr_printf(txt, "");

	// Show FPS
	if (GameConfig.Draw_Framerate)
		autostr_append(txt, _("FPS: %d\n"), get_current_fps());

	// Show quest information for current level
	for (i = 0; i < MAX_MISSIONS_IN_GAME; i++) {
		if (!Me.AllMissions[i].MissionWasAssigned)
			continue;

		if (Me.AllMissions[i].must_clear_level == Me.pos.z) {
			remaining_bots = 0;

			enemy *erot, *nerot;
			BROWSE_ALIVE_BOTS_SAFE(erot, nerot) {
				if ((erot->pos.z == Me.pos.z) && (!is_friendly(erot->faction, FACTION_SELF)))
					remaining_bots++;

			}
			autostr_append(txt, _("Bots remaining on level: %d\n"), remaining_bots);
		}
	}

	clip.x = User_Rect.x + 1;
	clip.y = User_Rect.y + 1;
	clip.w = GameConfig.screen_width;
	clip.h = GameConfig.screen_height;

	SetCurrentFont(FPS_Display_BFont);
	display_text_using_line_height(txt->value, clip.x, clip.y, &clip, 1.0);
}

/**
 * Show the texts that is written in the top right corner, like the game time
 * and current position.
 */
static void show_top_right_text(void)
{
	char level_name_and_time[1000];
	char temp_text[1000];

	// We display the name of the current level and the current time inside
	// the game.
	if (!(GameConfig.CharacterScreen_Visible || GameConfig.SkillScreen_Visible)) {
		if (GameConfig.Draw_Position) {
			sprintf(level_name_and_time, "%s (%03.1f:%03.1f:%d)  ",
				D_(curShip.AllLevels[Me.pos.z]->Levelname), Me.pos.x, Me.pos.y, Me.pos.z);
			// TRANSLATORS: In-game date: Day <day number> <hour>:<minute>
			sprintf(temp_text, _("Day %d  %02d:%02d"),
				get_days_of_game_duration(Me.current_game_date),
				get_hours_of_game_duration(Me.current_game_date), get_minutes_of_game_duration(Me.current_game_date));
			strcat(level_name_and_time, temp_text);
			strcat(level_name_and_time, " ");
		} else {
			sprintf(level_name_and_time, "%s  ", D_(curShip.AllLevels[Me.pos.z]->Levelname));
			sprintf(temp_text, _("Day %d  %02d:%02d"),
				get_days_of_game_duration(Me.current_game_date),
				get_hours_of_game_duration(Me.current_game_date), get_minutes_of_game_duration(Me.current_game_date));
			strcat(level_name_and_time, temp_text);
			strcat(level_name_and_time, " ");
		}
		put_string_right(FPS_Display_BFont, 0.3 * FontHeight(GetCurrentFont()), level_name_and_time);
	}
}

/**
 * Show all texts and banners that should be blitted right inside the combat
 * window.
 */
void show_texts_and_banner(void) {
	SDL_SetClipRect(Screen, NULL);
	show_current_text_banner();
	show_top_left_text();
	show_top_right_text();
	display_effect_countdowns();
	DisplayBigScreenMessage();
}

/**
 * This function should toggle the visibility of the inventory/character
 * and skill screen.  Of course, when one of them is turned on, the other
 * ones should be turned off again.  At least that was a popular request
 * from various sources in the past, so we heed it now.
 */
void toggle_game_config_screen_visibility(int screen_visible)
{

	switch (screen_visible) {
	case GAME_CONFIG_SCREEN_VISIBLE_INVENTORY:
		GameConfig.Inventory_Visible = !GameConfig.Inventory_Visible;
		GameConfig.skill_explanation_screen_visible = FALSE;
		break;
	case GAME_CONFIG_SCREEN_VISIBLE_SKILLS:
		GameConfig.SkillScreen_Visible = !GameConfig.SkillScreen_Visible;
		if (!GameConfig.SkillScreen_Visible)
			GameConfig.skill_explanation_screen_visible = 0;
		GameConfig.CharacterScreen_Visible = FALSE;
		break;
	case GAME_CONFIG_SCREEN_VISIBLE_CHARACTER:
		GameConfig.CharacterScreen_Visible = !GameConfig.CharacterScreen_Visible;
		GameConfig.SkillScreen_Visible = FALSE;
		break;
	case GAME_CONFIG_SCREEN_VISIBLE_SKILL_EXPLANATION:
		GameConfig.skill_explanation_screen_visible = !GameConfig.skill_explanation_screen_visible;
		GameConfig.Inventory_Visible = FALSE;
		break;
	default:
		error_message(__FUNCTION__, "\
unhandled skill screen code received.  something is going VERY wrong!", PLEASE_INFORM | IS_FATAL);
		break;
	}

};				// void toggle_game_config_screen_visibility ( int screen_visible )

#undef _hud_c
