/* 
 *
 *   Copyright (c) 2002, 2003 Johannes Prix
 *   Copyright (c) 2004-2010 Arthur Huillet
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
 * This file contains all the functions managing the character attributes
 * and the character stats.
 */

#define _character_c

#include "system.h"

#include "defs.h"
#include "struct.h"
#include "global.h"
#include "proto.h"

#define BASE_HPMAX 20
#define HPMAX_GAIN_PER_LEVEL 5
#define HPMAX_GAIN_PER_PHY_POINT 2
#define Maxtemp_Gain_Per_CPU_Point 4

#define RECHARGE_SPEED_PERCENT_PER_DEX_POINT 0
#define TOHIT_PERCENT_PER_DEX_POINT (1.0)

//--------------------
// At first we state some geometry constants for where to insert
// which character stats in the character screen...
//

// common x-offsets for texts
#define LEFT_TXT_X 25
#define RIGHT_TXT_X 195

// x-offset for numbers in the "level / experience / next level" box
#define LEVEL_NR_X 105
// x-offset for numbers in the "health / temperature / stamina" box
#define HEALTH_NR_X 125
// x-offset for numbers in the "armor / attack / damage" box
#define ARMOR_NR_X 260
// x-offset for values in the skills box
#define SKILLS_VALUE_X 198

// y-offsets - left
// STR_X_Y - VIT_Y in defs.h
#define POINTS_Y 245
#define HEALTH_STAT_Y 289
#define TEMP_STAT_Y 308
#define STAMINA_STAT_Y 327

// y-offsets - right
#define TOHIT_Y 138
#define DAMAGE_Y 158
#define DAMRED_Y 178
#define MELEE_SKILL_Y 230
#define RANGED_SKILL_Y 268
#define SPELLCASTING_SKILL_Y 306

/**
 * This function adds any bonuses that might be on the influencers things
 * concerning ONLY PRIMARY STATS, NOT SECONDARY STATS!
 */
static void AddInfluencerItemAttributeBonus(item * BonusItem)
{
	// In case of no item, the thing to do is pretty easy...
	//
	if (BonusItem->type == (-1))
		return;

	// Apply the bonuses to the stats of the character.
	Me.strength += BonusItem->bonus_to_str + BonusItem->bonus_to_all_attributes;
	Me.dexterity += BonusItem->bonus_to_dex + BonusItem->bonus_to_all_attributes;
	Me.cooling += BonusItem->bonus_to_cooling + BonusItem->bonus_to_all_attributes;
	Me.physique += BonusItem->bonus_to_physique + BonusItem->bonus_to_all_attributes;

};				// void AddInfluencerItemAttributeBonus( item* BonusItem )

/**
 * This function adds any bonuses that might be on the influencers things
 * concerning ONLY SECONDARY STATS, NOT PRIMARY STATS!
 */
static void AddInfluencerItemSecondaryBonus(item * BonusItem)
{
	// In case of no item, the thing to do is pretty easy...
	//
	if (BonusItem->type == (-1))
		return;

	// Apply the bonuses to the stats of the character.
	Me.to_hit += BonusItem->bonus_to_attack;
	Me.maxenergy += BonusItem->bonus_to_health_points;
	Me.health_recovery_rate += BonusItem->bonus_to_health_recovery;
	Me.cooling_rate += BonusItem->bonus_to_cooling_rate;

	// Apply bonuses to the special abilities of the character.
	Me.slowing_melee_targets += BonusItem->bonus_to_slow_enemy;
	Me.paralyzing_melee_targets += BonusItem->bonus_to_paralyze_enemy;
	Me.light_bonus_from_tux += BonusItem->bonus_to_light_radius;
	Me.experience_factor += BonusItem->bonus_to_experience_gain / 100.0;
}

/**
 * Maybe the influencer has reached a new experience level?
 * Let's check this...
 */
void check_for_new_experience_level_reached()
{
	int BaseExpRequired = 500;

	if (Me.exp_level >= 24)
		return;

	Me.ExpRequired = BaseExpRequired * (exp((Me.exp_level - 1) * log(2)));

	// For display reasons in the experience graph, we also state the experience 
	// needed for the previous level inside the tux struct.  Therefore all exp/level
	// calculations are found in this function.
	//
	if (Me.exp_level > 1) {
		Me.ExpRequired_previously = BaseExpRequired * (exp((Me.exp_level - 2) * log(2)));
	} else
		Me.ExpRequired_previously = 0;

	if (Me.Experience > Me.ExpRequired) {
		Me.exp_level++;
		Me.points_to_distribute += 5;

		if (Me.exp_level >= 24) {
			SetNewBigScreenMessage(_("Max level reached!"));
			return;
		}

		// When a droid reaches a new experience level, all health and 
		// force are restored to full this one time no longer.  Gothic
		// rulez more than Diablo rulez.
		//
		// Me .energy = Me .maxenergy ;
		// Me .mana   = Me .maxmana   ;

		// Also when a new level is reached, we will display a big message
		// right over the combat window.
		//
		SetNewBigScreenMessage(_("Level Gained!"));
		Takeover_Game_Won_Sound();
	}
};				// void check_for_new_experience_level_reached ( )

/**
 * Update all primary stats.
 */
void update_all_primary_stats()
{
	int i;

	// Now we base PRIMARY stats
	//
	Me.strength = Me.base_strength;
	Me.dexterity = Me.base_dexterity;
	Me.cooling = Me.base_cooling;
	Me.physique = Me.base_physique;

	Me.double_ranged_damage = FALSE;

	// Now we add all bonuses to the influencers PRIMARY stats
	//
	AddInfluencerItemAttributeBonus(&Me.armour_item);
	AddInfluencerItemAttributeBonus(&Me.weapon_item);
	AddInfluencerItemAttributeBonus(&Me.drive_item);
	AddInfluencerItemAttributeBonus(&Me.shield_item);
	AddInfluencerItemAttributeBonus(&Me.special_item);

	// Maybe there is some boost from a potion or magic spell going on right
	// now...
	//
	if (Me.dexterity_bonus_end_date > Me.current_game_date)
		Me.dexterity += Me.current_dexterity_bonus;
	if (Me.power_bonus_end_date > Me.current_game_date)
		Me.strength += Me.current_power_bonus;

	// Unequip items whose requirements aren't met anymore.
	// This needs to be done last so that temporary bonuses, for example from
	// strength and dexterity capsules, allow the player to keep items equipped.
	item *equipment[5] = { &Me.armour_item, &Me.weapon_item, &Me.drive_item,
		&Me.shield_item, &Me.special_item };
	for (i = 0; i < 5; i++) {
		if (equipment[i]->type == -1) {
			continue;
		}
		if (!ItemUsageRequirementsMet(equipment[i], FALSE)) {
			give_item(equipment[i]);
		}
	}

	// Ensure the effective primary stats never drops below 1
	if (Me.strength < 1)
		Me.strength = 1;
	if (Me.dexterity < 1)
		Me.dexterity = 1;
	if (Me.cooling < 1)
		Me.cooling = 1;
	if (Me.physique < 1)
		Me.physique = 1;
}

/**
 * This function computes secondary stats (i.e. chances for success or
 * getting hit and the like) using ONLY THE PRIMARY STATS.  Bonuses from
 * current 'magic' modifiers from equipped items will be applied somewhere
 * else.
 */
void update_secondary_stats_from_primary_stats()
{
	// The chance that this player character will score a hit on an enemy
	//
	Me.to_hit = 60 + (Me.dexterity) * TOHIT_PERCENT_PER_DEX_POINT;

	// How many life points can this character aquire currently
	//
	Me.maxenergy = BASE_HPMAX + (Me.exp_level) * HPMAX_GAIN_PER_LEVEL
                                  + (Me.physique) * HPMAX_GAIN_PER_PHY_POINT;

	// The maximum mana value computed from the primary stats
	//
	Me.max_temperature = (Me.cooling) * Maxtemp_Gain_Per_CPU_Point;

	// How long can this character run until he must take a break and
	// walk a bit
	//
	Me.max_running_power = (Me.strength) + (Me.dexterity) + (Me.physique) + Me.running_power_bonus;

	// base regeneration speed set to 0.2 points per second
	Me.health_recovery_rate = 0.2;
	Me.cooling_rate = 1.0;
};				// void update_secondary_stats_from_primary_stats ( )

/**
 * Now we compute the possible damage the player character can do.
 * The damage value of course depends on the weapon type that the
 * character is using.  And depending on the weapon type (melee or
 * ranged weapon) some additional attributes will also play a role.
 */
void update_damage_tux_can_do()
{
	if (Me.weapon_item.type != (-1)) {
		if (ItemMap[Me.weapon_item.type].item_weapon_is_melee != 0) {
			// Damage modifier in case of MELEE WEAPON is computed:  
			// weapon's modifier * (100+strength)%
			//
			Me.base_damage = Me.weapon_item.damage * (Me.strength + 100.0) / 100.0;

			Me.damage_modifier = Me.weapon_item.damage_modifier * (Me.strength + 100.0) / 100.0;

			// Damage AND damage modifier a modified by additional melee weapon
			// skill:  A multiplier is applied!
			//
			Me.damage_modifier *= MeleeDamageMultiplierTable[Me.melee_weapon_skill];
			Me.base_damage *= MeleeDamageMultiplierTable[Me.melee_weapon_skill];
		} else {
			// Damage modifier in case of RANGED WEAPON is computed:  
			// weapon's modifier * (100+dexterity)%
			//
			Me.base_damage = Me.weapon_item.damage * (Me.dexterity + 100.0) / 100.0;
			Me.damage_modifier = Me.weapon_item.damage_modifier * (Me.dexterity + 100.0) / 100.0;

			// Damage AND damage modifier a modified by additional ranged weapon
			// skill:  A multiplier is applied!
			//
			Me.damage_modifier *= RangedDamageMultiplierTable[Me.ranged_weapon_skill];
			Me.base_damage *= RangedDamageMultiplierTable[Me.ranged_weapon_skill];

			// Maybe there is a plugin for double damage with ranged
			// weapons present?
			//
			if (Me.double_ranged_damage != 0) {
				Me.base_damage *= 2;
				Me.damage_modifier *= 2;
			}
		}
	} else {
		// In case of no weapon equipped at all, we initialize
		// the damage values with some simple numbers.  Currently
		// strength and dexterity play NO ROLE in weaponless combat.
		// Maybe that should be changed for something more suitable
		// at some point...
		//
		Me.base_damage = 0;
		Me.damage_modifier = 1;
	}
};				// void update_damage_tux_can_do ( )

/**
 * This function should re-compute all character stats according to the
 * currently equipped items and currently distributed stats points.
 */
void UpdateAllCharacterStats()
{
	// Maybe the influencer has reached a new experience level?
	// Let's check this...
	// 
	check_for_new_experience_level_reached();

	// The primary status must be computed/updated first, because
	// the secondary status (chances and the like) will depend on
	// them...
	//
	update_all_primary_stats();

	// At this point we know, that the primary stats of the influencer
	// have been fully computed.  So that means, that finally we can compute
	// all base SECONDARY stats, that are dependent upon the influencer primary
	// stats.  Once we are done with that, the modifiers to the secondary
	// stats can be applied as well.
	//
	update_secondary_stats_from_primary_stats();

	// Now we compute the possible damage the player character can do.
	// The damage value of course depends on the weapon type that the
	// character is using.  And depending on the weapon type (melee or
	// ranged weapon) some additional attributes will also play a role.
	//
	update_damage_tux_can_do();

	// Update tux armour class
	//
	update_player_armor_class();

	// So at this point we can finally apply all the modifiers to the influencers
	// SECONDARY stats due to 'magical' items and spells and the like
	//
	Me.slowing_melee_targets = 0;
	Me.paralyzing_melee_targets = 0;
	Me.light_bonus_from_tux = 0;
	Me.experience_factor = 1.0;
	AddInfluencerItemSecondaryBonus(&Me.armour_item);
	AddInfluencerItemSecondaryBonus(&Me.weapon_item);
	AddInfluencerItemSecondaryBonus(&Me.drive_item);
	AddInfluencerItemSecondaryBonus(&Me.shield_item);
	AddInfluencerItemSecondaryBonus(&Me.special_item);

	// Add light bonus if necessary
	if (Me.light_bonus_end_date > Me.current_game_date) {
		// Fade out the bonus in the last second
		float ratio = min(1.0, Me.light_bonus_end_date - Me.current_game_date);
		Me.light_bonus_from_tux += 5 * ratio;
	}

	// Check player's health and temperature
	if (Me.energy > Me.maxenergy)
		Me.energy = Me.maxenergy;
	if (Me.temperature < 0)
		Me.temperature = 0;

}

/**
 * Now we print out the current skill levels in hacking skill, 
 * spellcasting, melee combat, ranged weapon combat and repairing things
 */
void show_character_screen_skills()
{

	// We add some security against skill values out of allowed
	// bounds.
	//
	if ((Me.melee_weapon_skill < 0) || (Me.melee_weapon_skill >= NUMBER_OF_SKILL_LEVELS)) {
		fprintf(stderr, "\nmelee_weapon_skill: %d.", Me.melee_weapon_skill);
		ErrorMessage(__FUNCTION__, "\
Error: melee weapon skill seems out of bounds.", PLEASE_INFORM, IS_FATAL);
	}
	SetCurrentFont(Messagestat_BFont);
	display_text(_("Melee"), RIGHT_TXT_X + CharacterRect.x, MELEE_SKILL_Y + CharacterRect.y, &CharacterRect);
	SetCurrentFont(Messagevar_BFont);
	display_text(_(AllSkillTexts[Me.melee_weapon_skill]),
		    SKILLS_VALUE_X + CharacterRect.x, MELEE_SKILL_Y + 17 + CharacterRect.y, &CharacterRect);

	// We add some security against skill values out of allowed
	// bounds.
	//
	if ((Me.ranged_weapon_skill < 0) || (Me.ranged_weapon_skill >= NUMBER_OF_SKILL_LEVELS)) {
		fprintf(stderr, "\nranged_weapon_skill: %d.", Me.ranged_weapon_skill);
		ErrorMessage(__FUNCTION__, "\
Error: ranged weapon skill seems out of bounds.", PLEASE_INFORM, IS_FATAL);
	}
	SetCurrentFont(Messagestat_BFont);
	display_text(_("Ranged"), RIGHT_TXT_X + CharacterRect.x, RANGED_SKILL_Y + CharacterRect.y, &CharacterRect);
	SetCurrentFont(Messagevar_BFont);
	display_text(_(AllSkillTexts[Me.ranged_weapon_skill]),
		    SKILLS_VALUE_X + CharacterRect.x, RANGED_SKILL_Y + 17 + CharacterRect.y, &CharacterRect);

	// We add some security against skill values out of allowed
	// bounds.
	//
	if ((Me.spellcasting_skill < 0) || (Me.spellcasting_skill >= NUMBER_OF_SKILL_LEVELS)) {
		fprintf(stderr, "\nProgramming_Skill: %d.", Me.spellcasting_skill);
		ErrorMessage(__FUNCTION__, "\
Error: Programming_Skill skill seems out of bounds.", PLEASE_INFORM, IS_FATAL);
	}
	SetCurrentFont(Messagestat_BFont);
	display_text(_("Programming"), RIGHT_TXT_X + CharacterRect.x, SPELLCASTING_SKILL_Y + CharacterRect.y, &CharacterRect);
	SetCurrentFont(Messagevar_BFont);
	display_text(_(AllSkillTexts[Me.spellcasting_skill]),
		    SKILLS_VALUE_X + CharacterRect.x, SPELLCASTING_SKILL_Y + 17 + CharacterRect.y, &CharacterRect);

};				// void show_character_screen_skills ( )

/**
 * This function displays the character screen.
 */
void ShowCharacterScreen()
{
	char CharText[1000];

	DebugPrintf(2, "\n%s(): Function call confirmed.", __FUNCTION__);

	// If the log is not set to visible right now, we do not need to 
	// do anything more, but to restore the usual user rectangle size
	// back to normal and to return...
	//
	if (GameConfig.CharacterScreen_Visible == FALSE)
		return;

	SetCurrentFont(Messagestat_BFont);
	
	// We define the right side of the user screen as the rectangle
	// for our inventory screen.
	//
	CharacterRect.x = GameConfig.screen_width - CHARACTERRECT_W;
	CharacterRect.y = 0;
	CharacterRect.w = CHARACTERRECT_W;
	CharacterRect.h = CHARACTERRECT_H;

	blit_background("character.png");

	SetCurrentFont(Blue_BFont);

	SetCurrentFont(Menu_BFont);
	display_text(Me.character_name, 20 + CharacterRect.x, 12 + CharacterRect.y, &CharacterRect);

	SetCurrentFont(Messagestat_BFont);
	display_text(_("Level"), LEFT_TXT_X + CharacterRect.x, 73 + CharacterRect.y, &CharacterRect);
	SetCurrentFont(Messagevar_BFont);
	sprintf(CharText, "%d", Me.exp_level);
	display_text(CharText, LEVEL_NR_X + CharacterRect.x, 73 + CharacterRect.y, &CharacterRect);

	SetCurrentFont(Messagestat_BFont);
	display_text(_("Experience"), LEFT_TXT_X + CharacterRect.x, 89 + CharacterRect.y, &CharacterRect);
	SetCurrentFont(Messagevar_BFont);
	sprintf(CharText, "%u", Me.Experience);
	display_text(CharText, LEVEL_NR_X + CharacterRect.x, 89 + CharacterRect.y, &CharacterRect);

	SetCurrentFont(Messagestat_BFont);
	display_text(_("Next level"), LEFT_TXT_X + CharacterRect.x, 107 + CharacterRect.y, &CharacterRect);
	SetCurrentFont(Messagevar_BFont);
	sprintf(CharText, "%u", Me.ExpRequired);
	display_text(CharText, LEVEL_NR_X + CharacterRect.x, 107 + CharacterRect.y, &CharacterRect);

	SetCurrentFont(Messagestat_BFont);
	display_text(_("Money"), RIGHT_TXT_X + CharacterRect.x, 71 + CharacterRect.y, &CharacterRect);
	SetCurrentFont(Messagevar_BFont);
	sprintf(CharText, "%6d", Me.Gold);
	display_text(CharText, 240 + CharacterRect.x, 71 + CharacterRect.y, &CharacterRect);

	SetCurrentFont(Messagestat_BFont);
	display_text(_("Strength"), LEFT_TXT_X + CharacterRect.x, STR_Y + CharacterRect.y, &CharacterRect);
	SetCurrentFont(Messagevar_BFont);
	sprintf(CharText, "%d", Me.strength);
	if (Me.strength != Me.base_strength)
		sprintf(CharText + strlen(CharText), " (%+d)", Me.strength - Me.base_strength);
	display_text(CharText, STR_X + CharacterRect.x, STR_Y + CharacterRect.y, &CharacterRect);

	SetCurrentFont(Messagestat_BFont);
	display_text(_("Dexterity"), LEFT_TXT_X + CharacterRect.x, DEX_Y + CharacterRect.y, &CharacterRect);
	SetCurrentFont(Messagevar_BFont);
	sprintf(CharText, "%d", Me.dexterity);
	if (Me.dexterity != Me.base_dexterity)
		sprintf(CharText + strlen(CharText), " (%+d)", Me.dexterity - Me.base_dexterity);
	display_text(CharText, STR_X + CharacterRect.x, DEX_Y + CharacterRect.y, &CharacterRect);

	SetCurrentFont(Messagestat_BFont);
	display_text(_("Physique"), LEFT_TXT_X + CharacterRect.x, VIT_Y + CharacterRect.y, &CharacterRect);
	SetCurrentFont(Messagevar_BFont);
	sprintf(CharText, "%d", Me.physique);
	if (Me.physique != Me.base_physique)
		sprintf(CharText + strlen(CharText), " (%+d)", Me.physique - Me.base_physique);
	display_text(CharText, STR_X + CharacterRect.x, VIT_Y + CharacterRect.y, &CharacterRect);

	SetCurrentFont(Messagestat_BFont);
	display_text(_("Cooling"), LEFT_TXT_X + CharacterRect.x, MAG_Y + CharacterRect.y, &CharacterRect);
	SetCurrentFont(Messagevar_BFont);
	sprintf(CharText, "%d", Me.cooling);
	if (Me.cooling != Me.base_cooling)
		sprintf(CharText + strlen(CharText), " (%+d)", Me.cooling - Me.base_cooling);
	display_text(CharText, STR_X + CharacterRect.x, MAG_Y + CharacterRect.y, &CharacterRect);

	SDL_Rect tmprect = CharacterRect;
	SetCurrentFont(Messagestat_BFont);
	display_text(_("Training points"), LEFT_TXT_X + CharacterRect.x, POINTS_Y + CharacterRect.y, &tmprect);
	SetCurrentFont(Messagevar_BFont);
	sprintf(CharText, "%d", Me.points_to_distribute);
	display_text(CharText, 155 + CharacterRect.x, POINTS_Y + CharacterRect.y, &CharacterRect);

	SetCurrentFont(Messagestat_BFont);
	display_text(_("Health"), LEFT_TXT_X + CharacterRect.x, HEALTH_STAT_Y + CharacterRect.y, &tmprect);
	SetCurrentFont(Messagevar_BFont);
	sprintf(CharText, "%d/%d", (int)Me.energy, (int)Me.maxenergy);
	display_text(CharText, HEALTH_NR_X + CharacterRect.x, HEALTH_STAT_Y + CharacterRect.y, &CharacterRect);

	SetCurrentFont(Messagestat_BFont);
	display_text(_("Temperature"), LEFT_TXT_X + CharacterRect.x, TEMP_STAT_Y + CharacterRect.y, &tmprect);
	SetCurrentFont(Messagevar_BFont);
	sprintf(CharText, "%d/%d", (int)Me.temperature, (int)Me.max_temperature);
	display_text(CharText, HEALTH_NR_X + CharacterRect.x, TEMP_STAT_Y + CharacterRect.y, &CharacterRect);

	SetCurrentFont(Messagestat_BFont);
	display_text(_("Stamina"), LEFT_TXT_X + CharacterRect.x, STAMINA_STAT_Y + CharacterRect.y, &tmprect);
	SetCurrentFont(Messagevar_BFont);
	sprintf(CharText, "%d/%d", (int)Me.running_power, (int)Me.max_running_power);
	display_text(CharText, HEALTH_NR_X + CharacterRect.x, STAMINA_STAT_Y + CharacterRect.y, &CharacterRect);

	SetCurrentFont(Messagestat_BFont);
	display_text(_("Attack"), RIGHT_TXT_X + CharacterRect.x, TOHIT_Y + CharacterRect.y, &CharacterRect);
	SetCurrentFont(Messagevar_BFont);
	sprintf(CharText, "%d%%", (int)Me.to_hit);
	display_text(CharText, ARMOR_NR_X + CharacterRect.x, TOHIT_Y + CharacterRect.y, &CharacterRect);

	SetCurrentFont(Messagestat_BFont);
	display_text(_("Damage"), RIGHT_TXT_X + CharacterRect.x, DAMAGE_Y + CharacterRect.y, &CharacterRect);
	SetCurrentFont(Messagevar_BFont);

	// Display range of damage, or a single value if there is no range
	if (Me.damage_modifier)
		sprintf(CharText, "%d-%d", (int)Me.base_damage, (int)Me.base_damage + (int)Me.damage_modifier);
	else
		sprintf(CharText, "%d", (int)Me.base_damage);

	display_text(CharText, ARMOR_NR_X + CharacterRect.x, DAMAGE_Y + CharacterRect.y, &CharacterRect);

	SetCurrentFont(Messagestat_BFont);
	display_text(_("Armor"), RIGHT_TXT_X + CharacterRect.x, DAMRED_Y + CharacterRect.y, &CharacterRect);
	SetCurrentFont(Messagevar_BFont);
	sprintf(CharText, "%d", (int)Me.armor_class);
	display_text(CharText, ARMOR_NR_X + CharacterRect.x, DAMRED_Y + CharacterRect.y, &CharacterRect);

	// Now we print out the current skill levels in hacking skill, 
	// spellcasting, melee combat, ranged weapon combat and repairing things
	//
	show_character_screen_skills();
	if (Me.points_to_distribute > 0) {
		ShowGenericButtonFromList(MORE_STR_BUTTON);
		ShowGenericButtonFromList(MORE_DEX_BUTTON);
		ShowGenericButtonFromList(MORE_VIT_BUTTON);
		ShowGenericButtonFromList(MORE_MAG_BUTTON);
	}
};				//ShowCharacterScreen ( ) 

/**
 * This function handles input for the character screen.
 */
void HandleCharacterScreen(void)
{

	if (!GameConfig.CharacterScreen_Visible)
		return;

	if (Me.points_to_distribute > 0) {
		if (MouseCursorIsOnButton(MORE_STR_BUTTON, GetMousePos_x(), GetMousePos_y()) && MouseLeftClicked()) {
			Me.base_strength++;
			Me.points_to_distribute--;
		}
		if (MouseCursorIsOnButton(MORE_DEX_BUTTON, GetMousePos_x(), GetMousePos_y()) && MouseLeftClicked()) {
			Me.base_dexterity++;
			Me.points_to_distribute--;
		}
		if (MouseCursorIsOnButton(MORE_MAG_BUTTON, GetMousePos_x(), GetMousePos_y()) && MouseLeftClicked()) {
			Me.base_cooling++;
			Me.points_to_distribute--;
		}
		if (MouseCursorIsOnButton(MORE_VIT_BUTTON, GetMousePos_x(), GetMousePos_y()) && MouseLeftClicked()) {
			Me.base_physique++;
			Me.points_to_distribute--;
		}

	}

};				// HandleCharacterScreen ( void )

#undef _character_c
