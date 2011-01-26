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

#include "SDL_rotozoom.h"	// that's for rotating the speed-o-meter arrows

#define TEXT_BANNER_DEFAULT_FONT FPS_Display_BFont
#define TEXT_BANNER_HORIZONTAL_MARGIN 4

int best_banner_pos_x, best_banner_pos_y;

/**
 * The HUD contains several status graphs.  These graphs appear as 
 * vertical columns, that are more or less filled, like liquid in a tube.
 * Since these appear multiple times, it appears sensible to make a 
 * function to draw such bars in a convenient way, which is what this
 * function is supposed to do.
 */
static void
blit_vertical_status_bar(float max_value, float current_value, Uint32 filled_color_code,
			 Uint32 empty_color_code, int x, int y, int w, int h)
{
	SDL_Rect running_power_rect;
	SDL_Rect un_running_power_rect;

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

	// Scale the status bar.
	x = x * GameConfig.screen_width / 640.0;
	y = y * GameConfig.screen_height / 480.0;
	w = w * GameConfig.screen_width / 640.0;
	h = h * GameConfig.screen_height / 480.0;

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
	our_SDL_fill_rect_wrapper(Screen, &(running_power_rect), filled_color_code);
	our_SDL_fill_rect_wrapper(Screen, &(un_running_power_rect), empty_color_code);

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
		ErrorMessage(__FUNCTION__, "\
An item description was requested for an item, that does not seem to \n\
exist really (i.e. has a type = (-1) ).", PLEASE_INFORM, IS_FATAL);
		return;
	}

	// Get the pure item name, also with font changes enabled.
	append_item_name(item, str);

	// We don't want any more information for Valuable Circuits
	if (MatchItemWithName(item->type, "Valuable Circuits"))
		return;
	
	autostr_append(str, "\n");

	// Weapon damage
	if (ItemMap[item->type].item_can_be_installed_in_weapon_slot) {
		autostr_append(str, _("Damage: %d"), item->damage);
		if (item->damage_modifier)
			autostr_append(str, _(" to %d"), item->damage_modifier + item->damage);
		autostr_append(str, "\n");
	}
	// Multiplicity
	if (ItemMap[item->type].item_group_together_in_inventory) {
		autostr_append(str, _("Multiplicity: %d\n"), item->multiplicity);
	}
	// Armor bonus
	if (item->armor_class) {
			autostr_append(str, _("Armor: %d\n"), item->armor_class);
	}
	// Durability or indestructible status
	if (item->max_duration != (-1)) {
		autostr_append(str, _("Durability: %d of %d\n"), (int)item->current_duration, (int)item->max_duration);
	} else {
		autostr_append(str, _("Indestructible\n"));
	}
	// Ranged weapon ammunition
	if (ItemMap[item->type].item_gun_ammo_clip_size) {
		autostr_append(str, _("Ammo: %d of %d\n"), item->ammo_clip, ItemMap[item->type].item_gun_ammo_clip_size);
	}
	// Strength, dexterity or cooling requirements
	if ((ItemMap[item->type].item_require_strength != (-1)) || (ItemMap[item->type].item_require_dexterity != (-1))) {
		if (ItemMap[item->type].item_require_strength != (-1)) {
			autostr_append(str, _("Required strength: %d\n"), ItemMap[item->type].item_require_strength);
		}
		if (ItemMap[item->type].item_require_dexterity != (-1)) {
			autostr_append(str, _("Required dexterity: %d\n"), ItemMap[item->type].item_require_dexterity);
		}
	}

	// Usable items should say that they can be used via right-clicking on it
	if (ItemMap[item->type].item_can_be_applied_in_combat) {
		if (MatchItemWithName(item->type, "Diet supplement") || MatchItemWithName(item->type, "Antibiotic")
		    || MatchItemWithName(item->type, "Doc-in-a-can")) {
			autostr_append(str, _("Recover Health"));
		} else if (MatchItemWithName(item->type, "Teleporter homing beacon")) {
			autostr_append(str, _("Teleports you to a safe place or\n back to your previous position"));
		} else if (MatchItemWithName(item->type, "Bottled ice") || MatchItemWithName(item->type, "Industrial coolant")
			   || MatchItemWithName(item->type, "Liquid nitrogen")) {
			autostr_append(str, _("Cooling aid"));
		} else if (MatchItemWithName(item->type, "Barf's Energy Drink")) {
			autostr_append(str, _("Cool down, catch your breath,\n cure minor wounds."));
		} else if (MatchItemWithName(item->type, "Running Power Capsule")) {
			autostr_append(str, _("Recover Running Power"));
		} else if (MatchItemWithName(item->type, "Strength Capsule")) {
			autostr_append(str, _("Temporary Boost to Strength"));
		} else if (MatchItemWithName(item->type, "Dexterity Capsule")) {
			autostr_append(str, _("Temporary Boost to Dexterity"));
		} else if (MatchItemWithName(item->type, "Map Maker")) {
			autostr_append(str, _("To implant the automap device"));
		} else if (MatchItemWithName(item->type, "Strength Pill")) {
			autostr_append(str, _("Permanently gain +1 strength"));
		} else if (MatchItemWithName(item->type, "Dexterity Pill")) {
			autostr_append(str, _("Permanently gain +1 dexterity"));
		} else if (MatchItemWithName(item->type, "Code Pill")) {
			autostr_append(str, _("Permanently gain +1 cooling"));
		} else if (MatchItemWithName(item->type, "Brain Enlargement Pill")) {
			autostr_append(str, _("Gives you fast acting cancer."));
		} else if (strstr(ItemMap[item->type].item_name, "Source Book of")) {
			autostr_append(str, _("Permanently acquire/enhance this program"));
		} else if (strstr(ItemMap[item->type].item_name, "Repair manual")) {
			autostr_append(str, _("Learn about repairing items"));
		} else if (MatchItemWithName(item->type, "Small EMP Shockwave Generator")) {
			autostr_append(str, _("Small Electromagnetic pulse"));
		} else if (MatchItemWithName(item->type, "EMP Shockwave Generator")) {
			autostr_append(str, _("Electromagnetic pulse"));
		} else if (MatchItemWithName(item->type, "VMX Gas Grenade")) {
			autostr_append(str, _("Gas attack"));
		} else if (MatchItemWithName(item->type, "Small Plasma Shockwave Emitter")) {
			autostr_append(str, _("Explosion"));
		} else if (MatchItemWithName(item->type, "Plasma Shockwave Emitter")) {
			autostr_append(str, _("Huge explosion"));
		} else {
			autostr_append(str, _("USE UNDESCRIBED YET (bug)"));
		}

		// Show text only if item is in inventory
		if (item->inventory_position.x != -1) {
			autostr_append(str, "\n");
			autostr_append(str, _("Right click to use\n"));
		}
	}

	// Socket count
	if (item->upgrade_sockets.size) {
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

	text_length = TextWidthFont(BFont_to_use, cur_enemy->short_description_text);

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
	    		description_pos->y) + enemy_images[cur_enemy->type][0][0].offset_y -
	    2.5 * FontHeight(BFont_to_use);

	// Calculates the width of the remaining health bar. Rounds the
	// width up to the nearest integer to ensure that at least one
	// pixel of health is always shown.
	//
	bar_width = (int) ceil((text_length) * (cur_enemy->energy / Druidmap[cur_enemy->type].maxenergy));
	barc_width = (int) floor((text_length) * (1.0 - cur_enemy->energy / Druidmap[cur_enemy->type].maxenergy));
	if (bar_width < 0)
		bar_width = 0;
	if (barc_width < 0)
		barc_width = 0;


	// Draw the energy bar
	rect.x = bar_x;
	rect.y = bar_y;
	rect.w = bar_width;
	if (use_open_gl) {
		gl_draw_rectangle(&rect, r, g, b, BACKGROUND_TEXT_RECT_ALPHA);
	} else {
		sdl_draw_rectangle(&rect, r, g, b, BACKGROUND_TEXT_RECT_ALPHA);
	}

	// Draw the energy bar complement
	rect.x = bar_x + bar_width;
	rect.y = bar_y;
	rect.w = barc_width;
	our_SDL_fill_rect_wrapper(Screen, &rect, SDL_MapRGB(Screen->format, 0x000, 0x000, 0x000));

	// Display droid's short description text
	rect.x = translate_map_point_to_screen_pixel_x(description_pos->x, description_pos->y) - text_length / 2;
	PutStringFont(Screen, BFont_to_use, rect.x, rect.y, cur_enemy->short_description_text);
}

/**
 * This function displays the icon of the current readied skill 
 * The dimensions and location of the picture are
 * specified in defs.h
 */
void ShowCurrentSkill(void)
{
	SDL_Rect Target_Rect;

	Target_Rect.x =
	    UNIVERSAL_COORD_W(CURRENT_SKILL_RECT_X) + (CURRENT_SKILL_RECT_W * GameConfig.screen_width / 640 - CURRENT_SKILL_RECT_W) / 2;
	Target_Rect.y = CURRENT_SKILL_RECT_Y + (CURRENT_SKILL_RECT_H * GameConfig.screen_height / 480 - CURRENT_SKILL_RECT_H) / 2;
	Target_Rect.w = CURRENT_SKILL_RECT_W;
	Target_Rect.h = CURRENT_SKILL_RECT_H;

	spell_skill_spec *spec = &SpellSkillMap[Me.readied_skill];
	load_skill_icon_if_needed(spec);

	display_image_on_screen(&spec->icon_surface, Target_Rect.x, Target_Rect.y);
}

/**
 * This function displays the icon of the current readied weapon, 
 * and the state of the charger
 * The dimensions and location of the picture are
 * specified in defs.h
 */
void ShowCurrentWeapon(void)
{
	char current_ammo[10];
	if (Me.weapon_item.type == -1 || &Me.weapon_item == item_held_in_hand)
		return;

	struct image *img = get_item_inventory_image(Me.weapon_item.type);

	float x = UNIVERSAL_COORD_W(CURRENT_WEAPON_RECT_X + CURRENT_WEAPON_RECT_W / 2) -
	    img->w / 2;
	float y = UNIVERSAL_COORD_H(CURRENT_WEAPON_RECT_Y + CURRENT_WEAPON_RECT_H / 2) -
	    img->h / 2;

	display_image_on_screen(img, x, y);

	if (!ItemMap[Me.weapon_item.type].item_gun_use_ammunition)
		return;

	if (Me.busy_type == WEAPON_RELOAD)
		sprintf(current_ammo, _("reloading"));
	else if (!Me.weapon_item.ammo_clip)
		sprintf(current_ammo, _(" %sEMPTY"), font_switchto_red);
	else
		sprintf(current_ammo, "%2d / %2d", Me.weapon_item.ammo_clip, ItemMap[Me.weapon_item.type].item_gun_ammo_clip_size);

	PutStringFont(Screen, FPS_Display_BFont, x, y + 50, current_ammo);
}

/**
 * The experience needed for the next level and the experience achieved
 * already to gain the next level can be seen from an experience countdown
 * bar on (top of the) screen.  We draw it here.
 */
void blit_experience_countdown_bars(void)
{
	static Uint32 experience_countdown_rect_color = 0;
	static Uint32 un_experience_countdown_rect_color = 0;
	int exp_range = Me.ExpRequired - Me.ExpRequired_previously;
	int exp_achieved = Me.Experience - Me.ExpRequired_previously;

	// At game startup, it might be that an uninitialized Tux (with 0 in the
	// max running power entry) is still in the data structure and when the
	// title displays, this causes division by zero... 
	//
	if (Me.ExpRequired <= 1)
		return;
	if ((Me.Experience > Me.ExpRequired) || (exp_range <= 1) || (exp_achieved < 0)) {
		DebugPrintf(1, "\nblit_experience_countdown_bars(...)\n\
The current experience of the Tux is higher than the next level while trying\n\
to blit the 'experience countdown' bar.  Graphics will be suppressed for now...");
		return;
	}
	// Upon the very first function call, the health and force colors are not yet
	// set.  Therefore we set these colors once and for the rest of the game.
	//
	if (experience_countdown_rect_color == 0) {
		un_experience_countdown_rect_color = SDL_MapRGBA(Screen->format, 50, 50, 50, 80);
		experience_countdown_rect_color = SDL_MapRGBA(Screen->format, 255, 120, 120, 80);
	}

	blit_vertical_status_bar(exp_range, exp_achieved,
				 experience_countdown_rect_color, un_experience_countdown_rect_color,
				 WHOLE_EXPERIENCE_COUNTDOWN_RECT_X,
				 WHOLE_EXPERIENCE_COUNTDOWN_RECT_Y,
				 WHOLE_EXPERIENCE_COUNTDOWN_RECT_W, WHOLE_EXPERIENCE_COUNTDOWN_RECT_H);

};				// void blit_experience_countdown_bars ( void )

static void blit_running_power_bars(void)
{
	static Uint32 running_power_rect_color = 0;
	static Uint32 un_running_power_rect_color = 0;
	static Uint32 rest_running_power_rect_color = 0;
	static Uint32 infinite_running_power_rect_color = 0;

	// At game startup, it might be that an uninitialized Tux (with 0 in the
	// max running power entry) is still in the data structure and when the
	// title displayes, this causes division by zero... 
	//
	if (Me.max_running_power <= 1)
		return;

	// Upon the very first function call, the health and force colors are not yet
	// set.  Therefore we set these colors once and for the rest of the game.
	//
	if (running_power_rect_color == 0) {
		un_running_power_rect_color = SDL_MapRGBA(Screen->format, 20, 20, 20, 80);
		running_power_rect_color = SDL_MapRGBA(Screen->format, 255, 255, 0, 80);
		rest_running_power_rect_color = SDL_MapRGBA(Screen->format, 255, 20, 20, 80);
		infinite_running_power_rect_color = SDL_MapRGBA(Screen->format, 255, 255, 255, 80);
	}

	// Now that all our rects are set up, we can start to display the current
	// running power status on screen...
	//
	SDL_SetClipRect(Screen, NULL);
	if (curShip.AllLevels[Me.pos.z]->infinite_running_on_this_level) {
		blit_vertical_status_bar(2.0, 2.0, infinite_running_power_rect_color,
					 un_running_power_rect_color,
					 WHOLE_RUNNING_POWER_RECT_X,
					 WHOLE_RUNNING_POWER_RECT_Y, WHOLE_RUNNING_POWER_RECT_W, WHOLE_RUNNING_POWER_RECT_H);
	} else {
		if (Me.running_must_rest)
			blit_vertical_status_bar(Me.max_running_power, Me.running_power,
						 rest_running_power_rect_color,
						 un_running_power_rect_color,
						 WHOLE_RUNNING_POWER_RECT_X,
						 WHOLE_RUNNING_POWER_RECT_Y, WHOLE_RUNNING_POWER_RECT_W, WHOLE_RUNNING_POWER_RECT_H);
		else
			blit_vertical_status_bar(Me.max_running_power, Me.running_power,
						 running_power_rect_color,
						 un_running_power_rect_color,
						 WHOLE_RUNNING_POWER_RECT_X,
						 WHOLE_RUNNING_POWER_RECT_Y, WHOLE_RUNNING_POWER_RECT_W, WHOLE_RUNNING_POWER_RECT_H);
	}

};				// void blit_running_power_bars ( void )

/**
 * Basically there are currently two methods of displaying the current
 * energy and mana of the Tux.  One method is to use the energy-o-meter,
 * an analog energy/mana display.  
 * The other method is to use classic energy bars.  This function is here
 * to provide the energy bars if desired.
 */
void blit_energy_and_mana_bars(void)
{

	static Uint32 health_rect_color = 0;
	static Uint32 un_health_rect_color = 0;
	static Uint32 un_force_rect_color = 0;

	// Upon the very first function call, the health and force colors are not yet
	// set.  Therefore we set these colors once and for the rest of the game.
	//
	if (health_rect_color == 0) {
		health_rect_color = SDL_MapRGBA(Screen->format, 255, 0, 0, 0);
		un_health_rect_color = SDL_MapRGBA(Screen->format, 20, 0, 0, 0);
		un_force_rect_color = SDL_MapRGBA(Screen->format, 0, 0, 55, 0);
	}

	blit_vertical_status_bar(Me.maxenergy, Me.energy,
				 health_rect_color, un_health_rect_color,
				 WHOLE_HEALTH_RECT_X, WHOLE_HEALTH_RECT_Y, WHOLE_HEALTH_RECT_W, WHOLE_HEALTH_RECT_H);
/*0 0 255
vert grimpe, bleu baisse, rouge grimpe, vert baisse*/
	int temp_ratio = Me.max_temperature ? (100 * Me.temperature) / Me.max_temperature : 100;
	if (temp_ratio > 100)
		temp_ratio = 100;
	int red = (temp_ratio) > 50 ? ((temp_ratio > 75) ? 255 : 4 * (temp_ratio - 50) * 2.55) : 0;
	int green;
	int blue;
	if (temp_ratio < 25) {
		red = 0;
		green = 2.55 * 4 * temp_ratio;
		blue = 255;
	} else if (temp_ratio < 50) {
		red = 0;
		green = 255;
		blue = 255 - (2.55 * 4 * (temp_ratio - 25));
	} else if (temp_ratio < 75) {
		green = 255;
		blue = 0;
		red = 2.4 * 4 * (temp_ratio - 50);
	} else {
		blue = 0;
		red = 255;
		green = 255 - (1.8 * 4 * (temp_ratio - 75));
	}

	int add = 0;
	if (Me.temperature > Me.max_temperature) {	//make the bar blink
		switch ((int)(Me.current_game_date) % 4) {
		case 0:
		case 2:
			add = (Me.current_game_date - (int)(Me.current_game_date)) * 255;
			red += add;
			blue += add;
			green += add;
			break;
		case 1:
		case 3:
			add = 255 - (Me.current_game_date - (int)(Me.current_game_date)) * 255;
			red += add;
			blue += add;
			green += add;
			break;
		}
	}
	blit_vertical_status_bar(Me.max_temperature, (Me.temperature > Me.max_temperature) ? Me.max_temperature : Me.temperature,
				 SDL_MapRGBA(Screen->format, red > 255 ? 255 : red, green < 255 ? green : 255, blue < 255 ? blue : 255, 0)
				 , un_force_rect_color, WHOLE_FORCE_RECT_X, WHOLE_FORCE_RECT_Y, WHOLE_FORCE_RECT_W, WHOLE_FORCE_RECT_H);

};				// void blit_energy_and_mana_bars ( void )

/**
 * This function displays the status bars for mana and energy in some 
 * corner of the screen.  The dimensions and location of the bar are
 * specified in items.h
 */
void ShowCurrentHealthAndForceLevel(void)
{
	blit_energy_and_mana_bars();

	blit_running_power_bars();

	blit_experience_countdown_bars();

};				// void ShowCurrentHealthAndForceLevel( void )

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
	finepoint MapPositionOfMouse;

	CurPos.x = GetMousePos_x();
	CurPos.y = GetMousePos_y();

	best_banner_pos_x = CurPos.x + 20;
	best_banner_pos_y = CurPos.y;

	autostr_printf(str, "");

	/* If the player has an item in hand, draw the item name into the
	 * description field.  If the requirements for this item are not met, we
	 * show a text. */
	if (item_held_in_hand != NULL) {
		autostr_printf(str, "%s%s", font_switchto_neon, D_(ItemMap[item_held_in_hand->type].item_name));

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

	/* Make the cursor position comparable to the coordinates of UI elements. */
	int x = CurPos.x * 640.0 / GameConfig.screen_width;
	int y = CurPos.y * 480.0 / GameConfig.screen_height;

	if (   x > WHOLE_RUNNING_POWER_RECT_X
	    && x < WHOLE_RUNNING_POWER_RECT_X + WHOLE_RUNNING_POWER_RECT_W
	    && y > WHOLE_RUNNING_POWER_RECT_Y
	    && y < WHOLE_RUNNING_POWER_RECT_Y + WHOLE_RUNNING_POWER_RECT_H)
	{
		autostr_printf(str, "%s\n%s%d/%d", _("RUN"),
			Me.running_power / Me.max_running_power <= 0.1 ? font_switchto_red : "",
			(int)rintf(Me.running_power), (int)rintf(Me.max_running_power));
		best_banner_pos_x = UNIVERSAL_COORD_W(WHOLE_RUNNING_POWER_RECT_X);
		best_banner_pos_y = UNIVERSAL_COORD_H(WHOLE_RUNNING_POWER_RECT_Y)
			- 3 * FontHeight(TEXT_BANNER_DEFAULT_FONT);
		return;
	}

	if (   x > WHOLE_EXPERIENCE_COUNTDOWN_RECT_X
	    && x < WHOLE_EXPERIENCE_COUNTDOWN_RECT_X + WHOLE_EXPERIENCE_COUNTDOWN_RECT_W
	    && y > WHOLE_EXPERIENCE_COUNTDOWN_RECT_Y
	    && y < WHOLE_EXPERIENCE_COUNTDOWN_RECT_Y + WHOLE_EXPERIENCE_COUNTDOWN_RECT_H)
	{
		autostr_printf(str, "%s\n%d/%d", _("XP"), Me.Experience, Me.ExpRequired);
		best_banner_pos_x = UNIVERSAL_COORD_W(WHOLE_RUNNING_POWER_RECT_X + 5);
		best_banner_pos_y = UNIVERSAL_COORD_H(WHOLE_EXPERIENCE_COUNTDOWN_RECT_Y)
			- 3 * FontHeight(TEXT_BANNER_DEFAULT_FONT);
		return;
	}

	if (   x > WHOLE_HEALTH_RECT_X
	    && x < WHOLE_HEALTH_RECT_X + WHOLE_HEALTH_RECT_W
	    && y > WHOLE_HEALTH_RECT_Y
	    && y < WHOLE_HEALTH_RECT_Y + WHOLE_HEALTH_RECT_H)
	{
		autostr_printf(str, "%s\n%s%d/%d", _("Health"),
			Me.energy / Me.maxenergy <= 0.1 ? font_switchto_red : "",
			(int)rintf(Me.energy), (int)rintf(Me.maxenergy));
		best_banner_pos_x = UNIVERSAL_COORD_W(WHOLE_FORCE_RECT_X + WHOLE_FORCE_RECT_W - 5)
			- longest_line_width(str->value) - TEXT_BANNER_HORIZONTAL_MARGIN * 2;
		best_banner_pos_y = UNIVERSAL_COORD_H(WHOLE_HEALTH_RECT_Y)
			- 3 * FontHeight(TEXT_BANNER_DEFAULT_FONT);
		return;
	}

	if (   x > WHOLE_FORCE_RECT_X
	    && x < WHOLE_FORCE_RECT_X + WHOLE_FORCE_RECT_W
	    && y > WHOLE_FORCE_RECT_Y
	    && y < WHOLE_FORCE_RECT_Y + WHOLE_FORCE_RECT_H)
	{
		autostr_printf(str, "%s\n%s%d/%d", _("Temperature"),
			Me.temperature / Me.max_temperature >= 0.9 ? font_switchto_red : "",
			(int)rintf(Me.temperature), (int)rintf(Me.max_temperature));
		best_banner_pos_x = UNIVERSAL_COORD_W(WHOLE_FORCE_RECT_X + WHOLE_FORCE_RECT_W)
			- longest_line_width(str->value) - TEXT_BANNER_HORIZONTAL_MARGIN * 2;
		best_banner_pos_y = UNIVERSAL_COORD_H(WHOLE_FORCE_RECT_Y)
			- 3 * FontHeight(TEXT_BANNER_DEFAULT_FONT);
		return;
	}

	// If the mouse cursor is within the user rectangle, then we check if
	// either the cursor is over an inventory item or over some other droid
	// and in both cases, we give a description of the object in the small
	// black rectangle in the top status banner.
	//

	if (MouseCursorIsInUserRect(CurPos.x, CurPos.y)) {
		level *obj_lvl = NULL;
		
		// DebugPrintf( 2  , "\nCursor is in userfenster... --> see if hovering over an item...");

		MapPositionOfMouse.x = translate_pixel_to_map_location((float)input_axis.x, (float)input_axis.y, TRUE);
		MapPositionOfMouse.y = translate_pixel_to_map_location((float)input_axis.x, (float)input_axis.y, FALSE);

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
				const char *label =  _(obstacle_map[obj_lvl->obstacle_list[index_of_obst_below_mouse_cursor].type].label);
				if (!label) {
					ErrorMessage(__FUNCTION__, "Obstacle type %d is clickable, and as such requires a label to be displayed on mouseover.\n", PLEASE_INFORM, IS_WARNING_ONLY, obj_lvl->obstacle_list[index_of_obst_below_mouse_cursor].type);
					label = "No label for this obstacle";
				}

				autostr_printf(str, "%s", label);
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
	SDL_Rect banner_rect;
	static struct auto_string *txt = NULL;
	if (txt == NULL)
		txt = alloc_autostr(200);

	// Set font first, before making any font specific calculations
	SetCurrentFont(TEXT_BANNER_DEFAULT_FONT);

	// Prepare the string, that is to be displayed inside the text rectangle
	prepare_text_window_content(txt);

	// Do not show anything if the description is too short
	if (strlen(txt->value) <= 1)
		return;

	banner_rect.x = best_banner_pos_x;
	banner_rect.y = best_banner_pos_y;
	banner_rect.h = 100; // some default size

	// Set banner width
	banner_rect.w = longest_line_width(txt->value);
	banner_rect.w += TEXT_BANNER_HORIZONTAL_MARGIN * 2;

	// Set banner height
	int lines_in_text = get_lines_needed(txt->value, banner_rect, TEXT_STRETCH);
	banner_rect.h = lines_in_text * FontHeight(GetCurrentFont());

	// Add extra correction to ensure the banner rectangle stays inside
	// the visible screen.
	if (banner_rect.x < 1)
		banner_rect.x = 1;
	else if (banner_rect.x + banner_rect.w > GameConfig.screen_width - 1)
		banner_rect.x = GameConfig.screen_width - banner_rect.w - 1;
	if (banner_rect.y < 1)
		banner_rect.y = 1;
	else if (banner_rect.y + banner_rect.h > GameConfig.screen_height - 1)
		banner_rect.y = GameConfig.screen_height - banner_rect.h - 1;

	// Draw the rectangle inside which the text will be drawn
	SDL_SetClipRect(Screen, NULL);	// this unsets the clipping rectangle
	if (use_open_gl)
		gl_draw_rectangle(&banner_rect, 0, 0, 0, BACKGROUND_TEXT_RECT_ALPHA);
	else
		sdl_draw_rectangle(&banner_rect, 0, 0, 0, BACKGROUND_TEXT_RECT_ALPHA);

	// Print the text
	int line_spacing = (banner_rect.h - lines_in_text * FontHeight(GetCurrentFont())) / (lines_in_text + 1);
	char *ptr = txt->value;
	int i;
	for (i = 0; i < lines_in_text; i++) {
		char *this_line = ptr;
		char *next_newline = strstr(ptr, "\n");
		if (next_newline) {
			int pos = next_newline - ptr;
			this_line[pos] = '\0';
			ptr += pos + 1;
		}
		PutString(Screen,
			  banner_rect.x + (banner_rect.w - TextWidth(this_line)) / 2,
			  banner_rect.y + line_spacing + i * (line_spacing + FontHeight(GetCurrentFont())), this_line);
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
	autostr_append(message_log.text, "\n* ");

	va_list args;
	va_start(args, fmt);
	autostr_vappend(message_log.text, fmt, args);
	va_end(args);

	message_log.scroll_offset = 0;
}

/**
 * Initialize or reset the message log.
 */
void init_message_log(void)
{
	init_text_widget(&message_log, _("--- Message Log ---"));

	/* Set up the text widget. */
	message_log.rect.x = MESSAGE_TEXT_WIDGET_X;
	message_log.rect.y = MESSAGE_TEXT_WIDGET_Y;
	message_log.rect.w = MESSAGE_TEXT_WIDGET_W;
	message_log.rect.h = MESSAGE_TEXT_WIDGET_H;
	message_log.font = Messagevar_BFont;
}

/**
 * Calculate the current FPS and return it.
 */
static int get_current_fps()
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
	int minutes;
	int seconds;
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

		if (Me.AllMissions[i].MustLiveTime != (-1)) {
			minutes = floor((Me.AllMissions[i].MustLiveTime - Me.MissionTimeElapsed) / 60);
			seconds = rintf(Me.AllMissions[i].MustLiveTime - Me.MissionTimeElapsed) - 60 * minutes;
			if (minutes < 0) {
				minutes = 0;
				seconds = 0;
			}
			autostr_append(txt, _("Time to hold out still: %2d:%2d\n"), minutes, seconds);
		}

		if ((Me.AllMissions[i].must_clear_first_level == Me.pos.z) || (Me.AllMissions[i].must_clear_second_level == Me.pos.z)) {
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
	DisplayText(txt->value, clip.x, clip.y, &clip, 1.0);
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
		RightPutStringFont(Screen, FPS_Display_BFont, 2, level_name_and_time);
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
		ErrorMessage(__FUNCTION__, "\
unhandled skill screen code received.  something is going VERY wrong!", PLEASE_INFORM, IS_FATAL);
		break;
	}

};				// void toggle_game_config_screen_visibility ( int screen_visible )

#undef _hud_c
