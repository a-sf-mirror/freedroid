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
static struct auto_string *message_log;

/**
 * The hud contains several status graphs.  These graphs appear as 
 * vertical columns, that are more or less filled, like liquid in a tube.
 * Since these appear multiple times, it appears sensible to make a 
 * function to draw such bars in a convenient way, which is what this
 * function is supposed to do.
 */
void
blit_vertical_status_bar(float max_value, float current_value, Uint32 filled_color_code,
			 Uint32 empty_color_code, int x, int y, int w, int h, int scale_to_screen_resolution)
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

	// If we are to scale the status bar, we do so :)
	//
	if (scale_to_screen_resolution) {
		un_running_power_rect.x = un_running_power_rect.x * GameConfig.screen_width / 640.0;
		un_running_power_rect.y = un_running_power_rect.y * GameConfig.screen_height / 480.0;
		un_running_power_rect.w = un_running_power_rect.w * GameConfig.screen_width / 640.0;
		un_running_power_rect.h = un_running_power_rect.h * GameConfig.screen_height / 480.0;
		running_power_rect.x = running_power_rect.x * GameConfig.screen_width / 640.0;
		running_power_rect.y = running_power_rect.y * GameConfig.screen_height / 480.0;
		running_power_rect.w = running_power_rect.w * GameConfig.screen_width / 640.0;
		running_power_rect.h = running_power_rect.h * GameConfig.screen_height / 480.0;
	}
	// Now wthat all our rects are set up, we can start to display the current
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
void give_item_description(char *target, item *item, int for_shop)
{
	strcpy(target, "");

	if (item == NULL)
		return;

	if (item->type == (-1)) {
		ErrorMessage(__FUNCTION__, "\
An item description was requested for an item, that does not seem to \n\
exist really (i.e. has a type = (-1) ).", PLEASE_INFORM, IS_FATAL);
		return;
	}

	// Get the pure item name, also with font changes enabled.
	write_full_item_name_into_string(item, target);

	// We don't want any more information for Valuable Circuits
	if (MatchItemWithName(item->type, "Valuable Circuits"))
		return;
	
	struct auto_string *desc = alloc_autostr(100);

	if (for_shop)
		autostr_append(desc, "\n             ");

	// Weapon damage
	if (ItemMap[item->type].item_can_be_installed_in_weapon_slot) {
		if (!for_shop) {
			autostr_append(desc, "\n");
			autostr_append(desc, _("Damage: %d to %d"), item->damage, item->damage_modifier + item->damage);
		} else
			autostr_append(desc, _("Dam: %d-%d "), item->damage, item->damage_modifier + item->damage);
	}
	// Multiplicity
	if (ItemMap[item->type].item_group_together_in_inventory) {
		autostr_append(desc, "\n");
		autostr_append(desc, _("Multiplicity: %d"), item->multiplicity);
	}
	// Armor bonus
	if (item->damred_bonus) {
		autostr_append(desc, "\n");
		if (ItemMap[item->type].item_can_be_installed_in_shield_slot)
			autostr_append(desc, _("Block: %d%%"), item->damred_bonus);
		else
			autostr_append(desc, _("Armor: %d%%"), item->damred_bonus);
	}
	// Durability or indestructible status
	if (item->max_duration != (-1)) {
		if (!for_shop) {
			autostr_append(desc, "\n");
			autostr_append(desc, _("Durability: %d of %d"), (int)item->current_duration, (int)item->max_duration);
		} else
			autostr_append(desc, _("Dur: %d/%d\n"), (int)item->current_duration, (int)item->max_duration);
	} else {
		autostr_append(desc, "\n");
		autostr_append(desc, _("Indestructible"));
	}
	// Ranged weapon amunition
	if (ItemMap[item->type].item_gun_ammo_clip_size) {
		autostr_append(desc, "\n");
		autostr_append(desc, _("Ammo: %d of %d"), item->ammo_clip, ItemMap[item->type].item_gun_ammo_clip_size);
	}
	// Strength, dexterity or magic requirements
	if ((ItemMap[item->type].item_require_strength != (-1)) || (ItemMap[item->type].item_require_dexterity != (-1))) {
		if (!for_shop)
			autostr_append(desc, "\n"); // separate requirements from the rest of description
		if (for_shop)
			autostr_append(desc, _("Required:"));
		if (ItemMap[item->type].item_require_strength != (-1)) {
			if (!for_shop) {
				autostr_append(desc, "\n");
				autostr_append(desc, _("Required strength: %d"), ItemMap[item->type].item_require_strength);
			} else
				autostr_append(desc, _("   Str: %d"), ItemMap[item->type].item_require_strength);
		}
		if (ItemMap[item->type].item_require_dexterity != (-1)) {
			if (!for_shop) {
				autostr_append(desc, "\n");
				autostr_append(desc, _("Required dexterity: %d"), ItemMap[item->type].item_require_dexterity);
			} else
				autostr_append(desc, _("   Dex: %d"), ItemMap[item->type].item_require_dexterity);
		}
	}/* else if (ItemMap[item->type].item_can_be_applied_in_combat) {
		// Maybe it's an applicable item, that still has some stat
		// requirements.  Typically spellbooks fall into that category.
		if (strstr(ItemMap[item->type].item_name, "Source Book of")) {
			autostr_append( desc , "Program execution status: %s\n " ,  
				_(AllSkillTexts [ required_spellcasting_skill_for_item ( item -> type ) ]));
			autostr_append( desc , "Required for next upgrade: %d\n " ,  
				required_magic_stat_for_next_level_and_item ( item -> type ) );
		}
	}*/
	// Usable items should say that it can be used via right-clicking on it
	if ((ItemMap[item->type].item_can_be_applied_in_combat) && (!for_shop)) {
		autostr_append(desc, "\n");
		if (MatchItemWithName(item->type, "Diet supplement") || MatchItemWithName(item->type, "Antibiotic")
		    || MatchItemWithName(item->type, "Doc-in-a-can")) {
			autostr_append(desc, _("Recover Health"));
		} else if (MatchItemWithName(item->type, "Identification Script")) {
			autostr_append(desc, _("Analyze one item"));
		} else if (MatchItemWithName(item->type, "Teleporter homing beacon")) {
			autostr_append(desc, _("Teleports you to a safe place or\n back to your previous position"));
		} else if (MatchItemWithName(item->type, "Bottled ice") || MatchItemWithName(item->type, "Industrial coolant")
			   || MatchItemWithName(item->type, "Liquid nitrogen")) {
			autostr_append(desc, _("Cooling aid"));
		} else if (MatchItemWithName(item->type, "Barf's Energy Drink")) {
			autostr_append(desc, _("Recover Health, Force\nand Running Power"));
		} else if (MatchItemWithName(item->type, "Running Power Capsule")) {
			autostr_append(desc, _("Recover Running Power"));
		} else if (MatchItemWithName(item->type, "Strength Capsule")) {
			autostr_append(desc, _("Temporary Boost to Strength"));
		} else if (MatchItemWithName(item->type, "Dexterity Capsule")) {
			autostr_append(desc, _("Temporary Boost to Dexterity"));
		} else if (MatchItemWithName(item->type, "Map Maker")) {
			autostr_append(desc, _("To implant the automap device"));
		} else if (MatchItemWithName(item->type, "Strength Pill")) {
			autostr_append(desc, _("Permanently gain +1 strength"));
		} else if (MatchItemWithName(item->type, "Dexterity Pill")) {
			autostr_append(desc, _("Permanently gain +1 dexterity"));
		} else if (MatchItemWithName(item->type, "Code Pill")) {
			autostr_append(desc, _("Permanently gain +1 cooling"));
		} else if (strstr(ItemMap[item->type].item_name, "Source Book of")) {
			autostr_append(desc, _("Permanently acquire/enhance this program"));
		} else if (MatchItemWithName(item->type, "EMP Shockwave Generator")) {
			autostr_append(desc, _("Electromagnetic pulse"));
		} else if (MatchItemWithName(item->type, "VMX Gas Grenade")) {
			autostr_append(desc, _("Gas attack"));
		} else if (MatchItemWithName(item->type, "Plasma Shockwave Emitter")) {
			autostr_append(desc, _("Huge explosion"));
		} else {
			autostr_append(desc, _("USE UNDESCRIBED YET (bug)"));
		}
		autostr_append(desc, "\n");
		autostr_append(desc, _("Right click to use"));
	}
	// Prefix and/or suffix bonuses
	if ((item->suffix_code != (-1)) || (item->prefix_code != (-1))) {
		if (item->is_identified == FALSE) {
			autostr_append(desc, "\n");
			autostr_append(desc, font_switchto_red);
			autostr_append(desc, _("UNIDENTIFIED"));
		} else {
			autostr_append(desc, "\n");

			// separate the next bonus with a comma or a newline?
			int need_separation = 0;

			char separator[3];
			if (for_shop)
				strcpy(separator, ", ");
			else
				strcpy(separator, "\n");

			if (for_shop)
				autostr_append(desc, "             ");
			if (item->bonus_to_str) {
				if (item->bonus_to_str > 0)
					autostr_append(desc, "+");
				else
					autostr_append(desc, "-");
				autostr_append(desc, _("%d to strength"), item->bonus_to_str);
				need_separation = TRUE;
			}
			if (item->bonus_to_dex) {
				if (need_separation)
					autostr_append(desc, separator);
				need_separation = TRUE;
				if (item->bonus_to_dex > 0)
					autostr_append(desc, "+");
				autostr_append(desc, _("%d to dexterity"), item->bonus_to_dex);
			}
			if (item->bonus_to_mag) {
				if (need_separation)
					autostr_append(desc, separator);
				need_separation = TRUE;
				if (item->bonus_to_mag > 0)
					autostr_append(desc, "+");
				autostr_append(desc, _("%d to CPU"), item->bonus_to_mag);
			}
			if (item->bonus_to_vit) {
				if (need_separation)
					autostr_append(desc, separator);
				need_separation = TRUE;
				if (item->bonus_to_vit > 0)
					autostr_append(desc, "+");
				autostr_append(desc, _("%d to life"), item->bonus_to_vit);
			}
			if (item->bonus_to_life) {
				if (need_separation)
					autostr_append(desc, separator);
				need_separation = TRUE;
				if (item->bonus_to_life > 0)
					autostr_append(desc, "+");
				autostr_append(desc, _("%d health points"), item->bonus_to_life);
			}
			if (item->bonus_to_health_recovery) {
				if (need_separation)
					autostr_append(desc, separator);
				need_separation = TRUE;
				if (item->bonus_to_health_recovery > 0)
					autostr_append(desc, "+");
				autostr_append(desc, _("%0.1f health points per second"), item->bonus_to_health_recovery);

			}
			if (item->bonus_to_cooling_rate) {
				if (need_separation)
					autostr_append(desc, separator);
				need_separation = TRUE;
				if (item->bonus_to_cooling_rate > 0)
					autostr_append(desc, _("%0.1f cooling per second"), item->bonus_to_cooling_rate);
				else if (item->bonus_to_cooling_rate < 0)
					autostr_append(desc, _("%0.1f heating per second"), -item->bonus_to_cooling_rate);
			}
			if (item->bonus_to_force) {
				if (need_separation)
					autostr_append(desc, separator);
				need_separation = TRUE;
				if (item->bonus_to_force > 0)
					autostr_append(desc, "+");
				autostr_append(desc, _("%d Force"), item->bonus_to_force);
			}
			if (item->bonus_to_tohit) {
				if (need_separation)
					autostr_append(desc, separator);
				need_separation = TRUE;
				if (item->bonus_to_tohit > 0)
					autostr_append(desc, "+");
				autostr_append(desc, _("%d%% to hit"), item->bonus_to_tohit);
			}
			if (item->bonus_to_all_attributes) {
				if (need_separation)
					autostr_append(desc, separator);
				need_separation = TRUE;
				if (item->bonus_to_all_attributes > 0)
					autostr_append(desc, "+");
				autostr_append(desc, _("%d to all attributes"), item->bonus_to_all_attributes);
			}
			// Percentage bonus to ac or damage
			if (item->bonus_to_damred_or_damage) {
				if (ItemMap[item->type].base_damred_bonus) {
					// if ( for_shop ) strcat( target , "             " );
					if (need_separation)
						autostr_append(desc, separator);
					need_separation = TRUE;
					if (item->bonus_to_damred_or_damage > 0)
						autostr_append(desc, "+");
					autostr_append(desc, _("%d%% to armor"), item->bonus_to_damred_or_damage);
				}
				if (ItemMap[item->type].base_item_gun_damage) {
					// if ( for_shop ) strcat( target , "             " );
					if (need_separation)
						autostr_append(desc, separator);
					need_separation = TRUE;
					if (item->bonus_to_damred_or_damage > 0)
						autostr_append(desc, "+");
					autostr_append(desc, _("%d%% to damage"), item->bonus_to_damred_or_damage);
				}
			}
			if (item->bonus_to_resist_fire) {
				if (need_separation)
					autostr_append(desc, separator);
				need_separation = TRUE;
				if (item->bonus_to_all_attributes > 0)
					autostr_append(desc, "+");
				autostr_append(desc, _("+%d to resist fire"), item->bonus_to_resist_fire);
			}
			if (item->bonus_to_resist_electricity) {
				if (need_separation)
					autostr_append(desc, separator);
				need_separation = TRUE;
				if (item->bonus_to_all_attributes > 0)
					autostr_append(desc, "+");
				autostr_append(desc, _("+%d to resist electricity"), item->bonus_to_resist_electricity);
			}
			// Maybe this item will give some bonus to the light radius?
			// (That is a very special case, because light bonuses are 
			// currently attached to the suffix/prefix, not to the item 
			// itself, so they also have no randomness...)
			if (item->prefix_code != (-1)) {
				if (PrefixList[item->prefix_code].light_bonus_value) {
					if (need_separation)
						autostr_append(desc, separator);
					need_separation = TRUE;
					autostr_append(desc, "+");
					autostr_append(desc, _("%d to light radius"), PrefixList[item->prefix_code].light_bonus_value);
				}
			}
		}
	}
	strcat(target, desc->value);
	free_autostr(desc);
	return;
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
	    		description_pos->y) + enemy_iso_images[cur_enemy->type][0][0].offset_y -
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
		GL_HighlightRectangle(Screen, &rect, r, g, b, BACKGROUND_TEXT_RECT_ALPHA);
	} else {
		our_SDL_fill_rect_wrapper(Screen, &rect, SDL_MapRGB(Screen->format, r, g, b));
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
	if ((GameConfig.SkillScreen_Visible || GameConfig.CharacterScreen_Visible) && GameConfig.screen_width == 640)
		return;

	Target_Rect.x =
	    UNIVERSAL_COORD_W(CURRENT_SKILL_RECT_X) + (CURRENT_SKILL_RECT_W * GameConfig.screen_width / 640 - CURRENT_SKILL_RECT_W) / 2;
	Target_Rect.y = CURRENT_SKILL_RECT_Y + (CURRENT_SKILL_RECT_H * GameConfig.screen_height / 480 - CURRENT_SKILL_RECT_H) / 2;
	Target_Rect.w = CURRENT_SKILL_RECT_W;
	Target_Rect.h = CURRENT_SKILL_RECT_H;

	LoadOneSkillSurfaceIfNotYetLoaded(Me.readied_skill);

	if (use_open_gl) {
		draw_gl_textured_quad_at_screen_position(&SpellSkillMap[Me.readied_skill].icon_surface, Target_Rect.x, Target_Rect.y);
	} else
		our_SDL_blit_surface_wrapper(SpellSkillMap[Me.readied_skill].icon_surface.surface, NULL, Screen, &Target_Rect);
};				// void ShowCurrentSkill ( void )

/**
 * This function displays the icon of the current readied weapon, 
 * and the state of the charger
 * The dimensions and location of the picture are
 * specified in defs.h
 */
void ShowCurrentWeapon(void)
{
	SDL_Rect Target_Rect;
	char current_ammo[10];
	if ((GameConfig.Inventory_Visible || GameConfig.skill_explanation_screen_visible) && GameConfig.screen_width == 640)
		return;
	if (Me.weapon_item.type == -1)
		return;

	Target_Rect.x =
	    UNIVERSAL_COORD_W(CURRENT_WEAPON_RECT_X) + UNIVERSAL_COORD_W(CURRENT_WEAPON_RECT_W) / 2 -
	    ItemMap[Me.weapon_item.type].inv_image.Surface->w / 2;
	Target_Rect.y =
	    UNIVERSAL_COORD_H(CURRENT_WEAPON_RECT_Y) + UNIVERSAL_COORD_H(CURRENT_WEAPON_RECT_H) / 2 -
	    ItemMap[Me.weapon_item.type].inv_image.Surface->h / 2;
	Target_Rect.w = CURRENT_WEAPON_RECT_W;
	Target_Rect.h = CURRENT_WEAPON_RECT_H;
	our_SDL_blit_surface_wrapper(ItemMap[Me.weapon_item.type].inv_image.Surface, NULL, Screen, &Target_Rect);

	if (!ItemMap[Me.weapon_item.type].item_gun_use_ammunition)
		return;

	if (Me.busy_type == WEAPON_RELOAD)
		sprintf(current_ammo, _("reloading"));
	else if (!Me.weapon_item.ammo_clip)
		sprintf(current_ammo, _(" %sEMPTY"), font_switchto_red);
	else
		sprintf(current_ammo, "%2d / %2d", Me.weapon_item.ammo_clip, ItemMap[Me.weapon_item.type].item_gun_ammo_clip_size);
	PutStringFont(Screen, FPS_Display_BFont, Target_Rect.x, Target_Rect.y + 50, current_ammo);

};				// void ShowCurrentWeapon ( void )

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

	if ((GameConfig.Inventory_Visible || GameConfig.skill_explanation_screen_visible) && GameConfig.screen_width == 640) {
		return;
	}
	// At game startup, it might be that an uninitialized Tux (with 0 in the
	// max running power entry) is still in the data structure and when the
	// title displayes, this causes division by zero... 
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
				 WHOLE_EXPERIENCE_COUNTDOWN_RECT_W, WHOLE_EXPERIENCE_COUNTDOWN_RECT_H, TRUE);

};				// void blit_experience_countdown_bars ( void )

/**
 * The tux has a limited running ability.
 *
 */
void blit_running_power_bars(void)
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

	if ((GameConfig.Inventory_Visible || GameConfig.skill_explanation_screen_visible) && GameConfig.screen_width == 640) {
		return;
	}
	// Now wthat all our rects are set up, we can start to display the current
	// running power status on screen...
	//
	SDL_SetClipRect(Screen, NULL);
	if (curShip.AllLevels[Me.pos.z]->infinite_running_on_this_level) {
		blit_vertical_status_bar(2.0, 2.0, infinite_running_power_rect_color,
					 un_running_power_rect_color,
					 WHOLE_RUNNING_POWER_RECT_X,
					 WHOLE_RUNNING_POWER_RECT_Y, WHOLE_RUNNING_POWER_RECT_W, WHOLE_RUNNING_POWER_RECT_H, TRUE);
	} else {
		if (Me.running_must_rest)
			blit_vertical_status_bar(Me.max_running_power, Me.running_power,
						 rest_running_power_rect_color,
						 un_running_power_rect_color,
						 WHOLE_RUNNING_POWER_RECT_X,
						 WHOLE_RUNNING_POWER_RECT_Y, WHOLE_RUNNING_POWER_RECT_W, WHOLE_RUNNING_POWER_RECT_H, TRUE);
		else
			blit_vertical_status_bar(Me.max_running_power, Me.running_power,
						 running_power_rect_color,
						 un_running_power_rect_color,
						 WHOLE_RUNNING_POWER_RECT_X,
						 WHOLE_RUNNING_POWER_RECT_Y, WHOLE_RUNNING_POWER_RECT_W, WHOLE_RUNNING_POWER_RECT_H, TRUE);
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
				 WHOLE_HEALTH_RECT_X, WHOLE_HEALTH_RECT_Y, WHOLE_HEALTH_RECT_W, WHOLE_HEALTH_RECT_H, TRUE);
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
				 , un_force_rect_color, WHOLE_FORCE_RECT_X, WHOLE_FORCE_RECT_Y, WHOLE_FORCE_RECT_W, WHOLE_FORCE_RECT_H,
				 TRUE);

};				// void blit_energy_and_mana_bars ( void )

/**
 * This function displays the status bars for mana and energy in some 
 * corner of the screen.  The dimensions and location of the bar are
 * specified in items.h
 */
void ShowCurrentHealthAndForceLevel(void)
{
	if (GameConfig.screen_width != 640 || ((!GameConfig.CharacterScreen_Visible) && (!GameConfig.SkillScreen_Visible))) {
		blit_energy_and_mana_bars();
	}

	blit_running_power_bars();

	blit_experience_countdown_bars();

};				// void ShowCurrentHealthAndForceLevel( void )

/**
 * This function sets up the text, that is to appear in a bigger text
 * rectangle, possibly next to the mouse cursor, e.g. when the mouse is
 * hovering over an item or barrel or crate or teleporter.
 */
void prepare_text_window_content(char *ItemDescText)
{
	point CurPos;
	point inv_square;
	int InvIndex;
	int index_of_obst_below_mouse_cursor = (-1);
	int index_of_floor_item_below_mouse_cursor = (-1);
	finepoint MapPositionOfMouse;

	CurPos.x = GetMousePos_x();
	CurPos.y = GetMousePos_y();

	best_banner_pos_x = CurPos.x;
	best_banner_pos_y = CurPos.y;

	/* If the player has an item in hand, draw the item name into the
	 * description field.  If the requirements for this item are not met, we
	 * show a text. */
	if (GetHeldItemPointer() != NULL) {
		best_banner_pos_x = CurPos.x + 20;

		strncpy(ItemDescText, font_switchto_neon, 1);
		strcat(ItemDescText, D_(ItemMap[GetHeldItemCode()].item_name));

		if (!ItemUsageRequirementsMet(GetHeldItemPointer(), FALSE)) {
			strcat(ItemDescText, "\n");
			strncat(ItemDescText, font_switchto_red, 1);
			strcat(ItemDescText, _("REQUIREMENTS NOT MET"));
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
				give_item_description(ItemDescText, &(Me.Inventory[InvIndex]), FALSE);
				best_banner_pos_x =
				    (Me.Inventory[InvIndex].inventory_position.x +
				     ItemMap[Me.Inventory[InvIndex].type].inv_image.inv_size.x) * 30 + 16;
				best_banner_pos_y = 300;
			}
		} else if (MouseCursorIsOnButton(WEAPON_RECT_BUTTON, CurPos.x, CurPos.y)) {
			if (Me.weapon_item.type > 0) {
				give_item_description(ItemDescText, &(Me.weapon_item), FALSE);
				best_banner_pos_x = WEAPON_RECT_X + 30 + WEAPON_RECT_WIDTH;
				best_banner_pos_y = WEAPON_RECT_Y - 30;
			}
		} else if (MouseCursorIsOnButton(DRIVE_RECT_BUTTON, CurPos.x, CurPos.y)) {
			if (Me.drive_item.type > 0) {
				give_item_description(ItemDescText, &(Me.drive_item), FALSE);
				best_banner_pos_x = DRIVE_RECT_X + 30 + DRIVE_RECT_WIDTH;
				best_banner_pos_y = DRIVE_RECT_Y - 30;
			}
		} else if (MouseCursorIsOnButton(SHIELD_RECT_BUTTON, CurPos.x, CurPos.y)) {
			if (Me.shield_item.type > 0) {
				give_item_description(ItemDescText, &(Me.shield_item), FALSE);
				best_banner_pos_x = SHIELD_RECT_X + 30 + SHIELD_RECT_WIDTH;
				best_banner_pos_y = SHIELD_RECT_Y - 30;
			} else if (Me.weapon_item.type > 0) {
				if (ItemMap[Me.weapon_item.type].item_gun_requires_both_hands) {
					give_item_description(ItemDescText, &(Me.weapon_item), FALSE);
					best_banner_pos_x = SHIELD_RECT_X + 30 + SHIELD_RECT_WIDTH;
					best_banner_pos_y = SHIELD_RECT_Y - 30;
				}
			}
		} else if (MouseCursorIsOnButton(ARMOUR_RECT_BUTTON, CurPos.x, CurPos.y)) {
			if (Me.armour_item.type > 0) {
				give_item_description(ItemDescText, &(Me.armour_item), FALSE);
				best_banner_pos_x = ARMOUR_RECT_X + 30 + ARMOUR_RECT_WIDTH;
				best_banner_pos_y = ARMOUR_RECT_Y - 30;
			}
		} else if (MouseCursorIsOnButton(HELMET_RECT_BUTTON, CurPos.x, CurPos.y)) {
			if (Me.special_item.type > 0) {
				give_item_description(ItemDescText, &(Me.special_item), FALSE);
				best_banner_pos_x = HELMET_RECT_X + 30 + HELMET_RECT_WIDTH;
				best_banner_pos_y = HELMET_RECT_Y - 30;
			}
		}
	}			// if nothing is 'held in hand' && inventory-screen visible

	/* Make the cursor position comparable to the coordinates of UI elements. */
	int x = CurPos.x * 640.0 / GameConfig.screen_width;
	int y = CurPos.y * 480.0 / GameConfig.screen_height;

	if (   x > WHOLE_RUNNING_POWER_RECT_X
	    && x < WHOLE_RUNNING_POWER_RECT_X + WHOLE_RUNNING_POWER_RECT_W
	    && y > WHOLE_RUNNING_POWER_RECT_Y
	    && y < WHOLE_RUNNING_POWER_RECT_Y + WHOLE_RUNNING_POWER_RECT_H)
	{
		sprintf(ItemDescText, _("Run\n%s%d/%d"),
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
		sprintf(ItemDescText, _("XP\n%d/%d"), Me.Experience, Me.ExpRequired);
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
		sprintf(ItemDescText, _("Health\n%s%d/%d"),
			Me.energy / Me.maxenergy <= 0.1 ? font_switchto_red : "",
			(int)rintf(Me.energy), (int)rintf(Me.maxenergy));
		best_banner_pos_x = UNIVERSAL_COORD_W(WHOLE_FORCE_RECT_X + WHOLE_FORCE_RECT_W - 5)
			- longest_line_width(ItemDescText) - TEXT_BANNER_HORIZONTAL_MARGIN * 2;
		best_banner_pos_y = UNIVERSAL_COORD_H(WHOLE_HEALTH_RECT_Y)
			- 3 * FontHeight(TEXT_BANNER_DEFAULT_FONT);
		return;
	}

	if (   x > WHOLE_FORCE_RECT_X
	    && x < WHOLE_FORCE_RECT_X + WHOLE_FORCE_RECT_W
	    && y > WHOLE_FORCE_RECT_Y
	    && y < WHOLE_FORCE_RECT_Y + WHOLE_FORCE_RECT_H)
	{
		sprintf(ItemDescText, _("Temperature\n%s%d/%d"),
			Me.temperature / Me.max_temperature >= 0.9 ? font_switchto_red : "",
			(int)rintf(Me.temperature), (int)rintf(Me.max_temperature));
		best_banner_pos_x = UNIVERSAL_COORD_W(WHOLE_FORCE_RECT_X + WHOLE_FORCE_RECT_W)
			- longest_line_width(ItemDescText) - TEXT_BANNER_HORIZONTAL_MARGIN * 2;
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
				give_item_description(ItemDescText, &(obj_lvl->ItemList[index_of_floor_item_below_mouse_cursor]), FALSE);
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
				strcpy(ItemDescText, _(obstacle_map[obj_lvl->obstacle_list[index_of_obst_below_mouse_cursor].type].label));
				best_banner_pos_x = translate_map_point_to_screen_pixel_x(obst_vpos.x, obst_vpos.y) + 70;
				best_banner_pos_y = translate_map_point_to_screen_pixel_y(obst_vpos.x, obst_vpos.y) - 20;
			}
		}

		// Maybe there is a teleporter event connected to the square where the mouse
		// cursor is currently hovering.  In this case we should create a message about
		// where the teleporter connection would bring the Tux...
		//
		if (teleporter_square_below_mouse_cursor(ItemDescText)) {
			// Do nothing here, 'cause the function above has filled in the proper
			// text already...
			//
		}
		// Maybe there is a living droid below the current mouse cursor, and it is visible to the player.
		// In this case, we'll give the decription of the corresponding bot.
		// Nota : the call to GetLivingDroidBelowMouseCursor() does set the virt_pos attribute
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
	char banner_text[5000] = "";

	// Set font first, before making any font specific calculations
	SetCurrentFont(TEXT_BANNER_DEFAULT_FONT);

	// Prepare the string, that is to be displayed inside the text rectangle
	prepare_text_window_content(banner_text);

	// Do not show anything if the description is too short
	if (strlen(banner_text) <= 1)
		return;

	banner_rect.x = best_banner_pos_x;
	banner_rect.y = best_banner_pos_y;
	banner_rect.h = 100; // some default size

	// Set banner width
	banner_rect.w = longest_line_width(banner_text);
	banner_rect.w += TEXT_BANNER_HORIZONTAL_MARGIN * 2;

	// Set banner height
	int lines_in_text = 1 + CountStringOccurences(banner_text, "\n");
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
		GL_HighlightRectangle(Screen, &banner_rect, 0, 0, 0, BACKGROUND_TEXT_RECT_ALPHA);
	else
		our_SDL_fill_rect_wrapper(Screen, &banner_rect, 0x00);

	// Print the text
	int line_spacing = (banner_rect.h - lines_in_text * FontHeight(GetCurrentFont())) / (lines_in_text + 1);
	char *ptr = banner_text;
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
	autostr_append(message_log, "\n* ");

	va_list args;
	va_start(args, fmt);
	autostr_vappend(message_log, fmt, args);
	va_end(args);

	message_log_scroll_override_from_user = 0;
}

/**
 * Clear message log, initializing message_log as needed.
 */
void reset_message_log(void)
{
	if (message_log == NULL)
		message_log = alloc_autostr(10000);
	message_log->length = 0;
	autostr_printf(message_log, _("--- Message Log ---"));
}

/**
 * We display a window with the current text messages.
 */
void display_current_game_message_window(void)
{
	SDL_Rect Subtitle_Window;
	int lines_needed;
	int log_offset;
	float text_stretch = 1.00;
	// float extra_stretch_calibrator = ((float)1.0/(float)1.07) ;  // 1.04
	float extra_stretch_calibrator = 1.00;

#define AVERAGE_LINES_IN_MESSAGE_WINDOW 3*GameConfig . screen_height/480
	SetCurrentFont(Messagevar_BFont);

	Subtitle_Window.x = UNIVERSAL_COORD_W(SUBTITLEW_RECT_X);
	Subtitle_Window.y = UNIVERSAL_COORD_H(SUBTITLEW_RECT_Y);
	Subtitle_Window.w = UNIVERSAL_COORD_W(SUBTITLEW_RECT_W);
	Subtitle_Window.h = UNIVERSAL_COORD_H(SUBTITLEW_RECT_H);

	// First we need to know where to begin with our little display.
	//
	lines_needed = GetNumberOfTextLinesNeeded(message_log->value, Subtitle_Window, text_stretch);
	DebugPrintf(1, "\nLines needed: %d. ", lines_needed);

	if (lines_needed <= AVERAGE_LINES_IN_MESSAGE_WINDOW) {
		// When there isn't anything to scroll yet, we keep the default
		// position and also the users clicks on up/down button will be
		// reset immediately
		//
		log_offset = 0;
		message_log_scroll_override_from_user = 0;
	} else
		log_offset = (FontHeight(GetCurrentFont()) * text_stretch)
		    * (lines_needed - AVERAGE_LINES_IN_MESSAGE_WINDOW +
		       message_log_scroll_override_from_user) * extra_stretch_calibrator;

	/*
	 * If the log offset is negative, that means the user was at the
	 * beginning of the text and tried to scroll up.  Let's not allow that.
	 */
	if (log_offset < 0) {
		message_log_scroll_override_from_user++;
		log_offset = 0;
	}

	blit_special_background(HUD_BACKGROUND_CODE);

	// Now we can display the text and update the screen...
	//
	SDL_SetClipRect(Screen, NULL);
	SetCurrentFont(Messagevar_BFont);
	DisplayText(message_log->value, Subtitle_Window.x, Subtitle_Window.y - log_offset, &Subtitle_Window, text_stretch);
}

/**
 * This function updates the various displays that are usually blitted
 * right into the combat window, like energy and status meter and that...
 */
void DisplayBanner(void)
{
	char level_name_and_time[1000];
	char temp_text[1000];

	SDL_SetClipRect(Screen, NULL);

	ShowCurrentHealthAndForceLevel();

	ShowCurrentSkill();
	ShowCurrentWeapon();
	show_current_text_banner();

	// We display the name of the current level and the current time inside
	// the game.
	//
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
};				// void DisplayBanner( void ) 

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
