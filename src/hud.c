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

//--------------------
// Here we define the dimensions of the banner rectangle.  Note,
// that some items have a lot of modifiers and magic boni, therefore
// they need a lot of lines, so 120 really seems reasonable.  If you
// want less, take care that the uppermost line is not omitted, a 
// bug that only occurs when very little space is given for those
// rectangles...
//
#define UPPER_BANNER_TEXT_RECT_X (GameConfig . screen_width/4)
#define UPPER_BANNER_TEXT_RECT_Y 1
#define UPPER_BANNER_TEXT_RECT_W (GameConfig . screen_width/3)
#define UPPER_BANNER_TEXT_RECT_H 130
#define LOWER_BANNER_TEXT_RECT_X UPPER_BANNER_TEXT_RECT_X
#define LOWER_BANNER_TEXT_RECT_Y ( GameConfig . screen_height - UPPER_BANNER_TEXT_RECT_Y - UPPER_BANNER_TEXT_RECT_H )
#define LOWER_BANNER_TEXT_RECT_W UPPER_BANNER_TEXT_RECT_W
#define LOWER_BANNER_TEXT_RECT_H UPPER_BANNER_TEXT_RECT_H
#define BANNER_TEXT_REC_BACKGROUNDCOLOR 0x00

int best_banner_pos_x, best_banner_pos_y;
char *game_message_protocol = NULL;

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
	//--------------------
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

	//--------------------
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
	//--------------------
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
 */
void GiveItemDescription(char *ItemDescText, item * CurItem, int ForShop)
{
	char linebuf[5000];
	char AppendToLine = 0;	// if we should write the next bonus with a comma as separator or with a newline

	//--------------------
	// We initialize the description text, so that at least something,
	// i.e. something terminated with a null charcter is in there.
	//
	strcpy(ItemDescText, "");

	//--------------------
	// In case of a NULL given as item pointer, we can return immediately.
	//
	if (CurItem == NULL)
		return;
	if (CurItem->type == (-1)) {
		ErrorMessage(__FUNCTION__, "\
An item description was requested for an item, that does not seem to \n\
exist really (i.e. has a type = (-1) ).", PLEASE_INFORM, IS_FATAL);
		return;
	}
	//--------------------
	// we get the pure item name, also with font changes enabled.
	//
	write_full_item_name_into_string(CurItem, linebuf);
	strcat(ItemDescText, linebuf);

	if (ForShop) {
		strcat(ItemDescText, "\n             ");
	} else {
		strcat(ItemDescText, "\n");
	}

	// --------------------
	// If it's a weapon, then we give out the damage value of that weapon as well
	//
	if (ItemMap[CurItem->type].item_can_be_installed_in_weapon_slot) {
		if (!ForShop)
			sprintf(linebuf, _("Damage: %d to %d \n"), CurItem->damage, CurItem->damage_modifier + CurItem->damage);
		else
			sprintf(linebuf, _("Dam: %d-%d "), CurItem->damage, CurItem->damage_modifier + CurItem->damage);
		strcat(ItemDescText, linebuf);
	}
	//--------------------
	// If this item has a multiplicity, we print it out
	//
	if (ItemMap[CurItem->type].item_group_together_in_inventory) {
		if (!MatchItemWithName(CurItem->type, "Valuable Circuits")) {
			sprintf(linebuf, _("Multiplicity: %d \n"), CurItem->multiplicity);
			strcat(ItemDescText, linebuf);
		}
	}
	// --------------------
	// If this item gives some armour bonus, we say so
	//
	if (CurItem->damred_bonus) {
		if (ItemMap[CurItem->type].item_can_be_installed_in_shield_slot) {
			sprintf(linebuf, _("Block: %d%%"), CurItem->damred_bonus);
		} else {
			sprintf(linebuf, _("Armor: %d%%"), CurItem->damred_bonus);
		}
		strcat(ItemDescText, linebuf);
		if (!ForShop)
			strcat(ItemDescText, " \n ");
	}
	// --------------------
	// If this is a destructible item, we finally give it's current condition
	// and if it can be equipped, but not destroyed, we will also say so
	//
	if (CurItem->max_duration != (-1)) {
		if (!ForShop)
			sprintf(linebuf, _(" Durability: %d of %d\n"), (int)CurItem->current_duration, (int)CurItem->max_duration);
		else
			sprintf(linebuf, _(" Dur: %d/%d\n"), (int)CurItem->current_duration, (int)CurItem->max_duration);
		strcat(ItemDescText, linebuf);
	} else {
		strcat(ItemDescText, _(" Indestructible\n"));
	};

	if (ItemMap[CurItem->type].item_gun_ammo_clip_size) {
		sprintf(linebuf, _("Ammo : %d of %d\n"), CurItem->ammo_clip, ItemMap[CurItem->type].item_gun_ammo_clip_size);
		strcat(ItemDescText, linebuf);
	}
	// --------------------
	// If this item has some strength or dex or magic requirements, we say so
	//
	if ((ItemMap[CurItem->type].item_require_strength != (-1)) || (ItemMap[CurItem->type].item_require_dexterity != (-1))) {
		if (!ForShop)
			strcat(ItemDescText, "\n");
		if (ForShop)
			strcat(ItemDescText, _(" Required:"));
		if (ItemMap[CurItem->type].item_require_strength != (-1)) {
			if (ForShop)
				sprintf(linebuf, _("   Str: %d"), ItemMap[CurItem->type].item_require_strength);
			else
				sprintf(linebuf, _(" Required strength: %d "), ItemMap[CurItem->type].item_require_strength);
			strcat(ItemDescText, linebuf);
			if (!ForShop)
				strcat(ItemDescText, "\n");
		}
		if (ItemMap[CurItem->type].item_require_dexterity != (-1)) {
			if (ForShop)
				sprintf(linebuf, _("   Dex: %d"), ItemMap[CurItem->type].item_require_dexterity);
			else
				sprintf(linebuf, _(" Required dexterity: %d "), ItemMap[CurItem->type].item_require_dexterity);
			strcat(ItemDescText, linebuf);
			if (!ForShop)
				strcat(ItemDescText, "\n");
		}
	} else if (ItemMap[CurItem->type].item_can_be_applied_in_combat) {
		//--------------------
		// Maybe it's an applicable item, that still has some stat
		// requirements.  Typically spellbooks fall into that category.
		//
		if (strstr(ItemMap[CurItem->type].item_name, "Source Book of")) {
/*		sprintf( linebuf , "Program execution status: %s\n " ,  
			 _(AllSkillTexts [ required_spellcasting_skill_for_item ( CurItem -> type ) ]));
		strcat( ItemDescText , linebuf );*/
/*		sprintf( linebuf , "Required for next upgrade: %d\n " ,  
			 required_magic_stat_for_next_level_and_item ( CurItem -> type ) );
		strcat( ItemDescText , linebuf );*/
		}
	} else if (ForShop) {
		strcat(ItemDescText, _(" , No required attributes "));
	}

	// --------------------
	// If it's a usable item, then we say, that it can be used via right-clicking on it
	//
	if ((ItemMap[CurItem->type].item_can_be_applied_in_combat) && (!ForShop)) {
		if (MatchItemWithName(CurItem->type, "Diet supplement") || MatchItemWithName(CurItem->type, "Antibiotic")
		    || MatchItemWithName(CurItem->type, "Doc-in-a-can")) {
			sprintf(linebuf, _("Recover Health\n"));
			strcat(ItemDescText, linebuf);
		} else if (MatchItemWithName(CurItem->type, "Identification Script")) {
			sprintf(linebuf, _("Analyze one item\n"));
			strcat(ItemDescText, linebuf);
		} else if (MatchItemWithName(CurItem->type, "Teleporter homing beacon")) {
			sprintf(linebuf, _("Teleports you to a safe place or\n back to your previous position\n"));
			strcat(ItemDescText, linebuf);
		} else if (MatchItemWithName(CurItem->type, "Bottled ice") || MatchItemWithName(CurItem->type, "Industrial coolant")
			   || MatchItemWithName(CurItem->type, "Liquid nitrogen")) {
			sprintf(linebuf, _("Cooling aid\n"));
			strcat(ItemDescText, linebuf);
		} else if (MatchItemWithName(CurItem->type, "Barf's Energy Drink")) {
			sprintf(linebuf, _("Recover Health, Force\nand Running Power\n"));
			strcat(ItemDescText, linebuf);
		} else if (MatchItemWithName(CurItem->type, "Running Power Capsule")) {
			sprintf(linebuf, _("Recover Running Power\n"));
			strcat(ItemDescText, linebuf);
		} else if (MatchItemWithName(CurItem->type, "Strength Capsule")) {
			sprintf(linebuf, _("Temporary Boost to Strength\n"));
			strcat(ItemDescText, linebuf);
		} else if (MatchItemWithName(CurItem->type, "Dexterity Capsule")) {
			sprintf(linebuf, _("Temporary Boost to Dexterity\n"));
			strcat(ItemDescText, linebuf);
		} else if (MatchItemWithName(CurItem->type, "Map Maker")) {
			sprintf(linebuf, _("To implant the automap device\n"));
			strcat(ItemDescText, linebuf);
		} else if (MatchItemWithName(CurItem->type, "Strength Pill")) {
			sprintf(linebuf, _("Permanently gain +1 strength\n"));
			strcat(ItemDescText, linebuf);
		} else if (MatchItemWithName(CurItem->type, "Dexterity Pill")) {
			sprintf(linebuf, _("Permanently gain +1 dexterity\n"));
			strcat(ItemDescText, linebuf);
		} else if (MatchItemWithName(CurItem->type, "Code Pill")) {
			sprintf(linebuf, _("Permanently gain +1 cooling\n"));
			strcat(ItemDescText, linebuf);
		} else if (strstr(ItemMap[CurItem->type].item_name, "Source Book of")) {
			sprintf(linebuf, _("Permanently acquire/enhance this program\n"));
			strcat(ItemDescText, linebuf);
		} else if (MatchItemWithName(CurItem->type, "EMP Shockwave Generator")) {
			sprintf(linebuf, _("Electromagnetic pulse\n"));
			strcat(ItemDescText, linebuf);
		} else if (MatchItemWithName(CurItem->type, "VMX Gas Grenade")) {

			sprintf(linebuf, _("Gas attack\n"));
			strcat(ItemDescText, linebuf);
		} else if (MatchItemWithName(CurItem->type, "Plasma Shockwave Emitter")) {
			sprintf(linebuf, _("Huge explosion\n"));
			strcat(ItemDescText, linebuf);
		}

		else {
			sprintf(linebuf, _("USE UNDESCRIBED YET (bug)\n"));
			strcat(ItemDescText, linebuf);

		}
		sprintf(linebuf, _("Right click to use"));
		strcat(ItemDescText, linebuf);
	}
	//--------------------
	// If the item has some suffixes, we describe these as well, but ONLY IF
	// THE ITEM HAS BEEN IDENTIFIED YET of course.
	//
	if ((CurItem->suffix_code != (-1)) || (CurItem->prefix_code != (-1))) {
		if (CurItem->is_identified == TRUE) {
			strcat(ItemDescText, "\n");
			//strcat ( ItemDescText , font_switchto_red );
			AppendToLine = 0;

			if (ForShop)
				strcat(ItemDescText, "             ");

			if (CurItem->bonus_to_str) {
				if (CurItem->bonus_to_str > 0)
					strcat(ItemDescText, "+");
				else
					strcat(ItemDescText, "-");
				sprintf(linebuf, _("%d to strength"), CurItem->bonus_to_str);
				AppendToLine = TRUE;
				strcat(ItemDescText, linebuf);
			}

			if (CurItem->bonus_to_dex) {
				if (AppendToLine) {
					if (ForShop)
						strcat(ItemDescText, ", ");
					else
						strcat(ItemDescText, "\n");
				};
				AppendToLine = TRUE;
				if (CurItem->bonus_to_dex > 0)
					strcat(ItemDescText, "+");
				sprintf(linebuf, _("%d to dexterity"), CurItem->bonus_to_dex);
				strcat(ItemDescText, linebuf);
			}

			if (CurItem->bonus_to_mag) {
				if (AppendToLine) {
					if (ForShop)
						strcat(ItemDescText, ", ");
					else
						strcat(ItemDescText, "\n");
				};
				AppendToLine = TRUE;
				if (CurItem->bonus_to_mag > 0)
					strcat(ItemDescText, "+");
				sprintf(linebuf, _("%d to CPU"), CurItem->bonus_to_mag);
				strcat(ItemDescText, linebuf);
			}

			if (CurItem->bonus_to_vit) {
				if (AppendToLine) {
					if (ForShop)
						strcat(ItemDescText, ", ");
					else
						strcat(ItemDescText, "\n");
				};
				if (CurItem->bonus_to_vit > 0)
					strcat(ItemDescText, "+");
				AppendToLine = TRUE;
				sprintf(linebuf, _("%d to life"), CurItem->bonus_to_vit);
				strcat(ItemDescText, linebuf);
			}

			if (CurItem->bonus_to_life) {
				if (AppendToLine) {
					if (ForShop)
						strcat(ItemDescText, ", ");
					else
						strcat(ItemDescText, "\n");
				};
				AppendToLine = TRUE;
				if (CurItem->bonus_to_life > 0)
					strcat(ItemDescText, "+");
				sprintf(linebuf, _("%d health points"), CurItem->bonus_to_life);
				strcat(ItemDescText, linebuf);
			}

			if (CurItem->bonus_to_health_recovery) {
				if (AppendToLine) {
					if (ForShop)
						strcat(ItemDescText, ", ");
					else
						strcat(ItemDescText, "\n");
				};
				AppendToLine = TRUE;
				if (CurItem->bonus_to_health_recovery > 0)
					strcat(ItemDescText, "+");
				sprintf(linebuf, _("%0.1f health points per second"), CurItem->bonus_to_health_recovery);
				strcat(ItemDescText, linebuf);
			}

			if (CurItem->bonus_to_cooling_rate) {
				if (AppendToLine) {
					if (ForShop)
						strcat(ItemDescText, ", ");
					else
						strcat(ItemDescText, "\n");
				};
				AppendToLine = TRUE;
				if (CurItem->bonus_to_cooling_rate > 0)
					sprintf(linebuf, _("%0.1f cooling per second"), CurItem->bonus_to_cooling_rate);
				else if (CurItem->bonus_to_cooling_rate < 0)
					sprintf(linebuf, _("%0.1f heating per second"), -CurItem->bonus_to_cooling_rate);
				strcat(ItemDescText, linebuf);
			}

			if (CurItem->bonus_to_force) {
				if (AppendToLine) {
					if (ForShop)
						strcat(ItemDescText, ", ");
					else
						strcat(ItemDescText, "\n");
				};
				AppendToLine = TRUE;
				if (CurItem->bonus_to_force > 0)
					strcat(ItemDescText, "+");
				sprintf(linebuf, _("%d Force"), CurItem->bonus_to_force);
				strcat(ItemDescText, linebuf);
			}

			if (CurItem->bonus_to_tohit) {
				if (AppendToLine) {
					if (ForShop)
						strcat(ItemDescText, ", ");
					else
						strcat(ItemDescText, "\n");
				};
				if (CurItem->bonus_to_tohit > 0)
					strcat(ItemDescText, "+");
				AppendToLine = TRUE;
				sprintf(linebuf, _("%d%% to hit"), CurItem->bonus_to_tohit);
				strcat(ItemDescText, linebuf);
			}

			if (CurItem->bonus_to_all_attributes) {
				if (AppendToLine) {
					if (ForShop)
						strcat(ItemDescText, ", ");
					else
						strcat(ItemDescText, "\n");
				};
				if (CurItem->bonus_to_all_attributes > 0)
					strcat(ItemDescText, "+");
				AppendToLine = TRUE;
				sprintf(linebuf, _("%d to all attributes"), CurItem->bonus_to_all_attributes);
				strcat(ItemDescText, linebuf);
			}
			// Now we display the percentage bonus to ac or damage
			if (CurItem->bonus_to_damred_or_damage) {
				if (ItemMap[CurItem->type].base_damred_bonus) {
					// if ( ForShop ) strcat( ItemDescText , "             " );
					if (AppendToLine) {
						if (ForShop)
							strcat(ItemDescText, ", ");
						else
							strcat(ItemDescText, "\n");
					}
					AppendToLine = TRUE;
					if (CurItem->bonus_to_damred_or_damage > 0)
						strcat(ItemDescText, "+");
					sprintf(linebuf, _("%d%% to armor"), CurItem->bonus_to_damred_or_damage);
					strcat(ItemDescText, linebuf);
				}
				if (ItemMap[CurItem->type].base_item_gun_damage) {
					// if ( ForShop ) strcat( ItemDescText , "             " );
					if (AppendToLine) {
						if (ForShop)
							strcat(ItemDescText, ", ");
						else
							strcat(ItemDescText, "\n");
					}
					AppendToLine = TRUE;
					if (CurItem->bonus_to_damred_or_damage > 0)
						strcat(ItemDescText, "+");
					sprintf(linebuf, _("%d%% to damage"), CurItem->bonus_to_damred_or_damage);
					strcat(ItemDescText, linebuf);
				}
			}

			if (CurItem->bonus_to_resist_fire) {
				if (AppendToLine) {
					if (ForShop)
						strcat(ItemDescText, ", ");
					else
						strcat(ItemDescText, "\n");
				};
				if (CurItem->bonus_to_all_attributes > 0)
					strcat(ItemDescText, "+");
				AppendToLine = TRUE;
				sprintf(linebuf, _("+%d to resist fire"), CurItem->bonus_to_resist_fire);
				strcat(ItemDescText, linebuf);
			}

			if (CurItem->bonus_to_resist_electricity) {
				if (AppendToLine) {
					if (ForShop)
						strcat(ItemDescText, ", ");
					else
						strcat(ItemDescText, "\n");
				};
				if (CurItem->bonus_to_all_attributes > 0)
					strcat(ItemDescText, "+");
				AppendToLine = TRUE;
				sprintf(linebuf, _("+%d to resist electricity"), CurItem->bonus_to_resist_electricity);
				strcat(ItemDescText, linebuf);
			}

			//--------------------
			// Maybe this item will give some bonus to the light radius?
			// (That is a very special case, because light bonuses are 
			// currently attached to the suffix/prefix, not to the item 
			// itself, so they also have no randomness...)
			//
			if (CurItem->prefix_code != (-1)) {
				if (PrefixList[CurItem->prefix_code].light_bonus_value) {
					if (AppendToLine) {
						if (ForShop)
							strcat(ItemDescText, ", ");
						else
							strcat(ItemDescText, "\n");
					};
					strcat(ItemDescText, "+");
					AppendToLine = TRUE;
					sprintf(linebuf, _("%d to light radius"), PrefixList[CurItem->prefix_code].light_bonus_value);
					strcat(ItemDescText, linebuf);
				}
			}

		} else {
			strcat(ItemDescText, "\n");
			strcat(ItemDescText, font_switchto_red);
			strcat(ItemDescText, _(" UNIDENTIFIED "));
		}
	}
};				// void GiveItemDescription ( char* ItemDescText , item* CurItem , int ForShop )

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
	if (!cur_enemy->is_friendly) {
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

	//--------------------
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
	//--------------------
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
	//--------------------
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

	//--------------------
	// At game startup, it might be that an uninitialized Tux (with 0 in the
	// max running power entry) is still in the data structure and when the
	// title displayes, this causes division by zero... 
	//
	if (Me.max_running_power <= 1)
		return;

	//--------------------
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
	//--------------------
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

	//--------------------
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
	int index_of_floor_item_below_mouse_cursor = (-1);
	int index_of_chest_below_mouse_cursor = (-1);
	int index_of_barrel_below_mouse_cursor = (-1);
	finepoint MapPositionOfMouse;
	int obs_type;

#define REQUIREMENTS_NOT_MET_TEXT _("REQUIREMENTS NOT MET")

	CurPos.x = GetMousePos_x();
	CurPos.y = GetMousePos_y();

	best_banner_pos_x = CurPos.x;
	best_banner_pos_y = CurPos.y;

	//--------------------
	// In case some item is held in hand by the player, the situation is simple:
	// we merely need to draw this items description into the description field and
	// that's it OR WE MUST SAY THAT THE requirements for this item are not met
	//
	if (GetHeldItemPointer() != NULL) {
		if (ItemUsageRequirementsMet(GetHeldItemPointer(), FALSE))
			strcpy(ItemDescText, D_(ItemMap[GetHeldItemCode()].item_name));
		else {
			strcpy(ItemDescText, REQUIREMENTS_NOT_MET_TEXT);
		}
	}
	//--------------------
	// in the other case however, that no item is currently held in hand, we need to
	// work a little more:  we need to find out if the cursor is currently over some
	// inventory or other item and in case that's true, we need to give the 
	// description of this item.
	//
	else if (GameConfig.Inventory_Visible) {
		//--------------------
		// Perhaps the cursor is over some item of the inventory?
		// let's check this case first.
		//
		if (MouseCursorIsInInventoryGrid(CurPos.x, CurPos.y)) {
			inv_square.x = GetInventorySquare_x(CurPos.x);
			inv_square.y = GetInventorySquare_y(CurPos.y);
			// DebugPrintf( 0 , "\nInv target x: %d." , inv_square.x );
			// DebugPrintf( 0 , "\nInv target y: %d." , inv_square.y );
			InvIndex = GetInventoryItemAt(inv_square.x, inv_square.y);
			// DebugPrintf( 0 , "\nInv Index targeted: %d." , InvIndex );
			if (InvIndex != (-1)) {
				GiveItemDescription(ItemDescText, &(Me.Inventory[InvIndex]), FALSE);
				best_banner_pos_x =
				    (Me.Inventory[InvIndex].inventory_position.x +
				     ItemMap[Me.Inventory[InvIndex].type].inv_image.inv_size.x) * 30 + 16;
				best_banner_pos_y = 300;
			}
		} else if (MouseCursorIsOnButton(WEAPON_RECT_BUTTON, CurPos.x, CurPos.y)) {
			if (Me.weapon_item.type > 0) {
				GiveItemDescription(ItemDescText, &(Me.weapon_item), FALSE);
				best_banner_pos_x = WEAPON_RECT_X + 30 + WEAPON_RECT_WIDTH;
				best_banner_pos_y = WEAPON_RECT_Y - 30;
			}
		} else if (MouseCursorIsOnButton(DRIVE_RECT_BUTTON, CurPos.x, CurPos.y)) {
			if (Me.drive_item.type > 0) {
				GiveItemDescription(ItemDescText, &(Me.drive_item), FALSE);
				best_banner_pos_x = DRIVE_RECT_X + 30 + DRIVE_RECT_WIDTH;
				best_banner_pos_y = DRIVE_RECT_Y - 30;
			}
		} else if (MouseCursorIsOnButton(SHIELD_RECT_BUTTON, CurPos.x, CurPos.y)) {
			if (Me.shield_item.type > 0) {
				GiveItemDescription(ItemDescText, &(Me.shield_item), FALSE);
				best_banner_pos_x = SHIELD_RECT_X + 30 + SHIELD_RECT_WIDTH;
				best_banner_pos_y = SHIELD_RECT_Y - 30;
			} else if (Me.weapon_item.type > 0) {
				if (ItemMap[Me.weapon_item.type].item_gun_requires_both_hands) {
					GiveItemDescription(ItemDescText, &(Me.weapon_item), FALSE);
					best_banner_pos_x = SHIELD_RECT_X + 30 + SHIELD_RECT_WIDTH;
					best_banner_pos_y = SHIELD_RECT_Y - 30;
				}
			}
		} else if (MouseCursorIsOnButton(ARMOUR_RECT_BUTTON, CurPos.x, CurPos.y)) {
			if (Me.armour_item.type > 0) {
				GiveItemDescription(ItemDescText, &(Me.armour_item), FALSE);
				best_banner_pos_x = ARMOUR_RECT_X + 30 + ARMOUR_RECT_WIDTH;
				best_banner_pos_y = ARMOUR_RECT_Y - 30;
			}
		} else if (MouseCursorIsOnButton(HELMET_RECT_BUTTON, CurPos.x, CurPos.y)) {
			if (Me.special_item.type > 0) {
				GiveItemDescription(ItemDescText, &(Me.special_item), FALSE);
				best_banner_pos_x = HELMET_RECT_X + 30 + HELMET_RECT_WIDTH;
				best_banner_pos_y = HELMET_RECT_Y - 30;
			}
		}

	}			// if nothing is 'held in hand' && inventory-screen visible

	if ((CurPos.x > WHOLE_HEALTH_RECT_X * GameConfig.screen_width / 640) &&
	    (CurPos.x * 640 / GameConfig.screen_width < WHOLE_HEALTH_RECT_X + WHOLE_HEALTH_RECT_W) &&
	    (CurPos.y > WHOLE_HEALTH_RECT_Y * GameConfig.screen_height / 480) &&
	    (CurPos.y * 480 / GameConfig.screen_height < WHOLE_HEALTH_RECT_Y + WHOLE_HEALTH_RECT_H)) {
		best_banner_pos_x = (WHOLE_HEALTH_RECT_X) * GameConfig.screen_width / 640;
		best_banner_pos_y = (WHOLE_HEALTH_RECT_Y) * GameConfig.screen_height / 480 - 5 * FontHeight(FPS_Display_BFont);
		sprintf(ItemDescText, _("Health\n%s%d/%d"), Me.energy / Me.maxenergy <= 0.1 ? font_switchto_red : "", (int)rintf(Me.energy),
			(int)rintf(Me.maxenergy));
	}

	if ((CurPos.x > WHOLE_FORCE_RECT_X * GameConfig.screen_width / 640) &&
	    (CurPos.x * 640 / GameConfig.screen_width < WHOLE_FORCE_RECT_X + WHOLE_FORCE_RECT_W) &&
	    (CurPos.y > WHOLE_FORCE_RECT_Y * GameConfig.screen_height / 480) &&
	    (CurPos.y * 480 / GameConfig.screen_height < WHOLE_FORCE_RECT_Y + WHOLE_FORCE_RECT_H)) {
		best_banner_pos_x = (WHOLE_FORCE_RECT_X) * GameConfig.screen_width / 640;
		best_banner_pos_y = (WHOLE_FORCE_RECT_Y) * GameConfig.screen_height / 480 - 5 * FontHeight(FPS_Display_BFont);
		sprintf(ItemDescText, _("Temperature \n%s%d/%d"), Me.temperature / Me.max_temperature >= 0.9 ? font_switchto_red : "",
			(int)rintf(Me.temperature), (int)rintf(Me.max_temperature));
	}

	if ((CurPos.x > WHOLE_RUNNING_POWER_RECT_X * GameConfig.screen_width / 640) &&
	    (CurPos.x * 640 / GameConfig.screen_width < WHOLE_RUNNING_POWER_RECT_X + WHOLE_RUNNING_POWER_RECT_W) &&
	    (CurPos.y > WHOLE_RUNNING_POWER_RECT_Y * GameConfig.screen_height / 480) &&
	    (CurPos.y * 480 / GameConfig.screen_height < WHOLE_RUNNING_POWER_RECT_Y + WHOLE_RUNNING_POWER_RECT_H)) {
		best_banner_pos_x = (WHOLE_RUNNING_POWER_RECT_X) * GameConfig.screen_width / 640;
		best_banner_pos_y = (WHOLE_RUNNING_POWER_RECT_Y) * GameConfig.screen_height / 480 - 5 * FontHeight(FPS_Display_BFont);
		sprintf(ItemDescText, _("Run\n%s%d/%d"), Me.running_power / Me.max_running_power <= 0.1 ? font_switchto_red : "",
			(int)rintf(Me.running_power), (int)rintf(Me.max_running_power));
	}

	if ((CurPos.x > WHOLE_EXPERIENCE_COUNTDOWN_RECT_X * GameConfig.screen_width / 640) &&
	    (CurPos.x * 640 / GameConfig.screen_width < WHOLE_EXPERIENCE_COUNTDOWN_RECT_X + WHOLE_EXPERIENCE_COUNTDOWN_RECT_W) &&
	    (CurPos.y > WHOLE_EXPERIENCE_COUNTDOWN_RECT_Y * GameConfig.screen_height / 480) &&
	    (CurPos.y * 480 / GameConfig.screen_height < WHOLE_EXPERIENCE_COUNTDOWN_RECT_Y + WHOLE_EXPERIENCE_COUNTDOWN_RECT_H)) {
		best_banner_pos_x = (WHOLE_EXPERIENCE_COUNTDOWN_RECT_X) * GameConfig.screen_width / 640;
		best_banner_pos_y = (WHOLE_EXPERIENCE_COUNTDOWN_RECT_Y) * GameConfig.screen_height / 480 - 5 * FontHeight(FPS_Display_BFont);
		sprintf(ItemDescText, _("XP\n%d/%d"), Me.Experience, Me.ExpRequired);
	}

	//--------------------
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
				GiveItemDescription(ItemDescText, &(obj_lvl->ItemList[index_of_floor_item_below_mouse_cursor]), FALSE);
				best_banner_pos_x =	translate_map_point_to_screen_pixel_x(item_vpos.x, item_vpos.y) + 80;
				best_banner_pos_y =	translate_map_point_to_screen_pixel_y(item_vpos.x, item_vpos.y) - 30;
			}
		}
		//--------------------
		// Maybe the cursor in the user rect is hovering right over a closed chest.
		// In this case we say so in the top status banner.
		//
		obj_lvl = NULL;
		index_of_chest_below_mouse_cursor = closed_chest_below_mouse_cursor(&obj_lvl);
		if (index_of_chest_below_mouse_cursor != (-1)) {
			gps chest_vpos;
			update_virtual_position(&chest_vpos, &(obj_lvl->obstacle_list[index_of_chest_below_mouse_cursor].pos), Me.pos.z);
			if (chest_vpos.x != -1) {
				strcpy(ItemDescText, _("   Chest   "));
				best_banner_pos_x = translate_map_point_to_screen_pixel_x(chest_vpos.x, chest_vpos.y) + 70;
				best_banner_pos_y = translate_map_point_to_screen_pixel_y(chest_vpos.x, chest_vpos.y) - 20;
			}
		}
		//--------------------
		// Maybe the cursor in the user rect is hovering right over a closed chest.
		// In this case we say so in the top status banner.
		//
		index_of_barrel_below_mouse_cursor = smashable_barrel_below_mouse_cursor(&obj_lvl);
		if (index_of_barrel_below_mouse_cursor != (-1)) {
			gps barrel_vpos;
			update_virtual_position(&barrel_vpos, &(obj_lvl->obstacle_list[index_of_barrel_below_mouse_cursor].pos), Me.pos.z);
			//--------------------
			// We do some case separation for the type of barrel/crate
			// in question.
			//
			obs_type = obj_lvl->obstacle_list[index_of_barrel_below_mouse_cursor].type;
			switch (obs_type) {
			case ISO_BARREL_1:
			case ISO_BARREL_2:
				strcpy(ItemDescText, _("   Barrel   "));
				break;
			case ISO_BARREL_3:
			case ISO_BARREL_4:
				strcpy(ItemDescText, _("   Crate   "));
				break;
			default:
				fprintf(stderr, "\nobs_type: %d.", obs_type);
				ErrorMessage(__FUNCTION__, "\
A barrel was detected, but the barrel type was not valid.", PLEASE_INFORM, IS_FATAL);
				break;
			}
			best_banner_pos_x = translate_map_point_to_screen_pixel_x(barrel_vpos.x, barrel_vpos.y) + 70;
			best_banner_pos_y = translate_map_point_to_screen_pixel_y(barrel_vpos.x, barrel_vpos.y) - 20;
		}
		//--------------------
		// Maybe there is a teleporter event connected to the square where the mouse
		// cursor is currently hovering.  In this case we should create a message about
		// where the teleporter connection would bring the Tux...
		//
		if (teleporter_square_below_mouse_cursor(ItemDescText)) {
			//--------------------
			// Do nothing here, 'cause the function above has filled in the proper
			// text already...
			//
		}
		//--------------------
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

};				// void prepare_text_window_content ( ItemDescText ) 

/**
 * At various points in the game, especially when the mouse in over an
 * interesting object inside the game, a popup window will appear, e.g.
 * to describe the object in question.
 * This function is responsible for bringing up these text windows.
 */
void ShowCurrentTextWindow(void)
{
	SDL_Rect Banner_Text_Rect;
	char ItemDescText[5000] = " ";
	char TextLine[10][1000];
	int i;
	int NumberOfLinesInText = 1;
	char *LongTextPointer;
	int InterLineDistance;
	int StringLength;
	int lines_needed;

	//--------------------
	// We prepare the string, that is to be displayed inside the text 
	// rectangle...
	//
	prepare_text_window_content(ItemDescText);

	Banner_Text_Rect.x = best_banner_pos_x;
	Banner_Text_Rect.y = best_banner_pos_y;
	
	Banner_Text_Rect.h = LOWER_BANNER_TEXT_RECT_H;

        // Banner width required for a given text.
        Banner_Text_Rect.w = LongestTextLine(ItemDescText);
        

	//--------------------
	// We count the text lines needed for the banner rectangle, just
	// to make sure we don't wast too much space here.
	//
	lines_needed = GetNumberOfTextLinesNeeded(ItemDescText, Banner_Text_Rect, 1.0);
	if (lines_needed <= 20) {
		Banner_Text_Rect.h = (lines_needed + 2) * FontHeight(FPS_Display_BFont);
	}
	//--------------------
	// Now we add some extra correction, so that the banner rectangle can not
	// reach outside of the visible screen...
	//
	if (Banner_Text_Rect.x + Banner_Text_Rect.w > GameConfig.screen_width) {
		Banner_Text_Rect.x = GameConfig.screen_width - Banner_Text_Rect.w;
	}
	if (Banner_Text_Rect.y + Banner_Text_Rect.h > GameConfig.screen_height) {
		Banner_Text_Rect.y = GameConfig.screen_height - Banner_Text_Rect.h;
	}
	//--------------------
	// Now we can start to draw the rectangle
	//
	SDL_SetClipRect(Screen, NULL);	// this unsets the clipping rectangle
	if (strlen(ItemDescText) > 1) {
		if (use_open_gl)
			GL_HighlightRectangle(Screen, &Banner_Text_Rect, 0, 0, 0, BACKGROUND_TEXT_RECT_ALPHA);
		else
			our_SDL_fill_rect_wrapper(Screen, &Banner_Text_Rect, BANNER_TEXT_REC_BACKGROUNDCOLOR);
	} else {
		return;
	}

	if (strcmp(ItemDescText, REQUIREMENTS_NOT_MET_TEXT) == 0) {
		SetCurrentFont(Red_BFont);
	} else {
		SetCurrentFont(FPS_Display_BFont);
	}

	//--------------------
	// Now we count how many lines are to be printed
	//
	NumberOfLinesInText = 1 + CountStringOccurences(ItemDescText, "\n");

	//--------------------
	// Now we separate the lines and fill them into the line-array
	//
	InterLineDistance = (Banner_Text_Rect.h - NumberOfLinesInText * FontHeight(GetCurrentFont())) / (NumberOfLinesInText + 1);

	LongTextPointer = ItemDescText;
	for (i = 0; i < NumberOfLinesInText - 1; i++) {
		StringLength = strstr(LongTextPointer, "\n") - LongTextPointer;

		strncpy(TextLine[i], LongTextPointer, StringLength);
		TextLine[i][StringLength] = 0;

		LongTextPointer += StringLength + 1;
		PutString(Screen,
			  Banner_Text_Rect.x + (Banner_Text_Rect.w - TextWidth(TextLine[i])) / 2,
			  Banner_Text_Rect.y + InterLineDistance + i * (InterLineDistance + FontHeight(GetCurrentFont())), TextLine[i]);
	}
	PutString(Screen,
		  Banner_Text_Rect.x + (Banner_Text_Rect.w - TextWidth(LongTextPointer)) / 2,
		  Banner_Text_Rect.y + InterLineDistance + i * (InterLineDistance + FontHeight(GetCurrentFont())), LongTextPointer);

};				// void ShowCurrentTextWindow ( void )

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
 *
 *
 */

void append_new_game_message(const char *message, ...)
{
	va_list args;
	va_start(args, message);
	static char *msg_ptr = NULL;

	if (game_message_protocol == NULL) {
		game_message_protocol = MyMalloc(500000);	// enough for any protocol
		msg_ptr = game_message_protocol;
		msg_ptr += sprintf(msg_ptr, _("--- Message Protocol ---"));
	}

	msg_ptr += sprintf(msg_ptr, "\n* ");
	msg_ptr += vsprintf(msg_ptr, message, args);

	va_end(args);

	game_message_protocol_scroll_override_from_user = 0;

};				// void append_new_game_message ( const char* message, ... )

/**
 * We display a window with the current text messages.
 */
void display_current_game_message_window(void)
{
	SDL_Rect Subtitle_Window;
	int lines_needed;
	int protocol_offset;
	float our_stretch_factor = 1.00;	// TEXT_STRETCH
	// float extra_stretch_calibrator = ((float)1.0/(float)1.07) ;  // 1.04
	float extra_stretch_calibrator = 1.00;

#define AVERAGE_LINES_IN_MESSAGE_WINDOW 3*GameConfig . screen_height/480

	SetCurrentFont(Messagevar_BFont);
	if (game_message_protocol == NULL) {
		game_message_protocol = MyMalloc(500000);	// enough for any protocol
		sprintf(game_message_protocol,
			_
			("This is the protocol.\nOh yes, it is indeed!\nIt has multiple lines too!\nIsn't that great?  But how long may any one line be?  Is there a limit for that?  Anyway, it looks good."));
	}

	Subtitle_Window.x = UNIVERSAL_COORD_W(SUBTITLEW_RECT_X);
	Subtitle_Window.y = UNIVERSAL_COORD_H(SUBTITLEW_RECT_Y);
	Subtitle_Window.w = UNIVERSAL_COORD_W(SUBTITLEW_RECT_W);
	Subtitle_Window.h = UNIVERSAL_COORD_H(SUBTITLEW_RECT_H);

	//--------------------
	// First we need to know where to begin with our little display.
	//
	lines_needed = GetNumberOfTextLinesNeeded(game_message_protocol, Subtitle_Window, our_stretch_factor);
	DebugPrintf(1, "\nLines needed: %d. ", lines_needed);

	if (lines_needed <= AVERAGE_LINES_IN_MESSAGE_WINDOW) {
		//--------------------
		// When there isn't anything to scroll yet, we keep the default
		// position and also the users clicks on up/down button will be
		// reset immediately
		//
		protocol_offset = 0;
		game_message_protocol_scroll_override_from_user = 0;
	} else
		protocol_offset = (FontHeight(GetCurrentFont()) * our_stretch_factor)
		    * (lines_needed - AVERAGE_LINES_IN_MESSAGE_WINDOW +
		       game_message_protocol_scroll_override_from_user) * extra_stretch_calibrator;

	//--------------------
	// Now if the protocol offset is really negative, we don't really want
	// that and force the user offset back to something sane again.
	//
	if (protocol_offset < 0) {
		game_message_protocol_scroll_override_from_user++;
		protocol_offset = 0;
	}

	blit_special_background(HUD_BACKGROUND_CODE);

	//--------------------
	// Now we can display the text and update the screen...
	//
	SDL_SetClipRect(Screen, NULL);
	SetCurrentFont(Messagevar_BFont);
	DisplayText(game_message_protocol, Subtitle_Window.x, Subtitle_Window.y - protocol_offset, &Subtitle_Window, our_stretch_factor);

};				// void display_current_game_message_window ( int background_picture_code , int with_update )

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
	ShowCurrentTextWindow();

	//--------------------
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
