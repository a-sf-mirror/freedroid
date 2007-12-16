/* 
 *
 *   Copyright (c) 2002, 2003 Johannes Prix
 *   Copyright (c) 2004-2007 Arthur Huillet
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

/* ----------------------------------------------------------------------
 * This file contains all the functions managing the character attributes
 * and the character stats.
 * ---------------------------------------------------------------------- */

#define _character_c

#include "system.h"

#include "defs.h"
#include "struct.h"
#include "global.h"
#include "proto.h"

#define Energy_Gain_Per_Vit_Point 2;
#define Maxtemp_Gain_Per_CPU_Point 4;
#define AC_Gain_Per_Dex_Point 0.5;

#define RECHARGE_SPEED_PERCENT_PER_DEX_POINT 0
#define TOHIT_PERCENT_PER_DEX_POINT (0.5)

//--------------------
// At first we state some geometry constants for where to insert
// which character stats in the character screen...
//

#define EXPERIENCE_Y 56
#define NEXT_LEVEL_Y 82

#define GOLD_Y 133

#define DAMAGE_X 255
#define DAMAGE_Y 193

#define TOHIT_X 255
#define TOHIT_Y 167

#define AC_X 255
#define AC_Y 138


#define MELEE_SKILL_X 247
#define MELEE_SKILL_Y 228
#define RANGED_SKILL_X MELEE_SKILL_X
#define RANGED_SKILL_Y 252
#define SPELLCASTING_SKILL_X MELEE_SKILL_X
#define SPELLCASTING_SKILL_Y 276
#define HACKING_SKILL_X MELEE_SKILL_X
#define HACKING_SKILL_Y 301

/* ----------------------------------------------------------------------
 * This function displays all the buttons that open up the character
 * screen and the invenotry screen
 * ---------------------------------------------------------------------- */
void
DisplayButtons( void )
{

    //--------------------
    // When the Tux has some extra skill points, that can be distributed
    // to some character stats or saved for later training with some trainer
    // character in the city, we mark the character screen toggle button as
    // red to indicate the available points.
    //
    if ( Me.points_to_distribute > 0 )
    {
	// blit_special_background ( MOUSE_BUTTON_PLUS_BACKGROUND_PICTURE_CODE );
	ShowGenericButtonFromList ( CHA_SCREEN_TOGGLE_BUTTON_RED );
    }
    
    if ( MouseCursorIsOnButton( INV_SCREEN_TOGGLE_BUTTON , GetMousePos_x()  , GetMousePos_y()  ) )
    {
	ShowGenericButtonFromList ( INV_SCREEN_TOGGLE_BUTTON_YELLOW );
	if ( MouseLeftClicked())
	{
	    toggle_game_config_screen_visibility ( GAME_CONFIG_SCREEN_VISIBLE_INVENTORY );
	    DebugPrintf ( 2 , "\nClick inside inventory button registered..." );
	}
    }
    else if ( MouseCursorIsOnButton( CHA_SCREEN_TOGGLE_BUTTON , GetMousePos_x()  , GetMousePos_y()  ) )
    {
	ShowGenericButtonFromList ( CHA_SCREEN_TOGGLE_BUTTON_YELLOW );
	if ( MouseLeftClicked() )
	{
	    toggle_game_config_screen_visibility ( GAME_CONFIG_SCREEN_VISIBLE_CHARACTER );
	    DebugPrintf ( 2 , "\nClick inside character button registered..." );
	}
    }
    else if ( MouseCursorIsOnButton( SKI_SCREEN_TOGGLE_BUTTON , GetMousePos_x()  , GetMousePos_y()  ) )
    {
	ShowGenericButtonFromList ( SKI_SCREEN_TOGGLE_BUTTON_YELLOW );
	if ( MouseLeftClicked() )
	{
	    toggle_game_config_screen_visibility ( GAME_CONFIG_SCREEN_VISIBLE_SKILLS );
	    DebugPrintf ( 2 , "\nClick inside skills button registered..." );
	}
    }
    else if ( MouseCursorIsOnButton( LOG_SCREEN_TOGGLE_BUTTON , GetMousePos_x()  , GetMousePos_y()  ) )
    {
	ShowGenericButtonFromList ( LOG_SCREEN_TOGGLE_BUTTON_YELLOW );
	if ( MouseLeftClicked() )
	{
	    DebugPrintf ( 2 , "\nClick inside questlog button registered..." );
	    quest_browser_interface ( );
	}
    }
    else if ( MouseCursorIsOnButton( WEAPON_MODE_BUTTON , GetMousePos_x()  , GetMousePos_y()  ) )
    {
	if ( MouseLeftClicked() )
	{
	TuxReloadWeapon ( );
	}
    }
    else if ( MouseCursorIsOnButton( SKI_ICON_BUTTON , GetMousePos_x()  , GetMousePos_y()  ) )
    {
	if ( MouseLeftClicked() )
	{
        toggle_game_config_screen_visibility ( GAME_CONFIG_SCREEN_VISIBLE_SKILLS );
	}
    }
    
}; // void DisplayButtons( void )


/* ----------------------------------------------------------------------
 * This function adds any bonuses that might be on the influencers things
 * concerning ONLY PRIMARY STATS, NOT SECONDARY STATS!
 * ---------------------------------------------------------------------- */
void
AddInfluencerItemAttributeBonus( item* BonusItem )
{
  //--------------------
  // In case of no item, the thing to do is pretty easy...
  //
  if ( BonusItem->type == ( -1 ) ) return;

  //--------------------
  // In case of a suffix modifier, we need to apply the suffix...
  //
  if ( ( ( BonusItem->suffix_code != ( -1 ) ) || ( BonusItem->prefix_code != ( -1 ) ) ) &&
       BonusItem -> is_identified )
    {
      Me.Strength  += BonusItem->bonus_to_str + BonusItem->bonus_to_all_attributes ;
      Me.Dexterity += BonusItem->bonus_to_dex + BonusItem->bonus_to_all_attributes ;
      Me.Magic     += BonusItem->bonus_to_mag + BonusItem->bonus_to_all_attributes ;
      Me.Vitality  += BonusItem->bonus_to_vit + BonusItem->bonus_to_all_attributes ;
    }

}; // void AddInfluencerItemAttributeBonus( item* BonusItem )

/* ----------------------------------------------------------------------
 * This function adds any bonuses that might be on the influencers things
 * concerning ONLY SECONDARY STATS, NOT PRIMARY STATS!
 * ---------------------------------------------------------------------- */
void
AddInfluencerItemSecondaryBonus( item* BonusItem )
{
    //--------------------
    // In case of no item, the thing to do is pretty easy...
    //
    if ( BonusItem->type == ( -1 ) ) return;
    
    //--------------------
    // In case of present suffix or prefix modifiers, we need to apply the suffix...
    //
    if ( ( ( BonusItem->suffix_code != ( -1 ) ) || ( BonusItem->prefix_code != ( -1 ) ) ) &&
	 BonusItem->is_identified )
    {
	
	//--------------------
	// Some modifiers might not be random at all but fixed to the 
	// item prefix or item suffix.  In that case, we must get the
	// modifier strength from the suffix/prefix spec itself...
	//
	if ( BonusItem -> suffix_code != ( -1 ) )
	    Me . light_bonus_from_tux += SuffixList [ BonusItem -> suffix_code ] . light_bonus_value ;
	if ( BonusItem -> prefix_code != ( -1 ) )
	    Me . light_bonus_from_tux += PrefixList [ BonusItem -> prefix_code ] . light_bonus_value ;

	//--------------------
	// Now we can apply the modifiers, that have been generated from
	// the suffix spec and then (with some randomness) written into the
	// item itself.  In that case we won't need the suffix- or 
	// prefix-lists here...
	//
	Me . to_hit    += BonusItem -> bonus_to_tohit ;
	Me . max_temperature   += BonusItem -> bonus_to_force ;
	Me . maxenergy += BonusItem -> bonus_to_life ; 
	Me . health_recovery_rate += BonusItem -> bonus_to_health_recovery ; 
	Me . cooling_rate += BonusItem -> bonus_to_cooling_rate ; 
	
	Me . resist_disruptor   += BonusItem -> bonus_to_resist_disruptor ;
	Me . resist_fire        += BonusItem -> bonus_to_resist_fire ;
	Me . resist_electricity += BonusItem -> bonus_to_resist_electricity ;

	
	// if ( ItemMap [ BonusItem->type ] . can_be_installed_in_weapon_slot )
	// Me.freezing_enemys_property += BonusItem->freezing_time_in_seconds;
    }

}; // void AddInfluencerItemSecondaryBonus( item* BonusItem )

/* ----------------------------------------------------------------------
 * Maybe the influencer has reached a new experience level?
 * Let's check this...
 * ---------------------------------------------------------------------- */
void
check_for_new_experience_level_reached ()
{
    int BaseExpRequired = 400;

    if ( Me . exp_level >= 24 )
	{
	SetNewBigScreenMessage(_("Max level reached!"));
	return;
	}

    Me . ExpRequired = 
	BaseExpRequired * ( exp ( ( Me . exp_level - 1 ) * log ( 2 ) ) ) ;
    
    //--------------------
    // For display reasons in the experience graph, we also state the experience 
    // needed for the previous level inside the tux struct.  Therefore all exp/level
    // calculations are found in this function.
    //
    if ( Me . exp_level > 1 )
    {
	Me . ExpRequired_previously = 
	    BaseExpRequired * ( exp ( ( Me .exp_level - 2 ) * log ( 2 ) ) ) ;
    }
    else
	Me . ExpRequired_previously = 0 ;

    if ( Me . Experience > Me . ExpRequired ) 
    {
	Me . exp_level ++ ;
	Me . points_to_distribute += 5;

	//--------------------
	// Like in the Gothic 1 game, maximum life force will now automatically
	// be increased upon reaching a new character level.
	//
	Me . base_vitality += 3;

	//--------------------
	// When a droid reaches a new experience level, all health and 
	// force are restored to full this one time no longer.  Gothic
	// rulez more than Diablo rulez.
	//
	// Me .energy = Me .maxenergy ;
	// Me .mana   = Me .maxmana   ;

	//--------------------
	// Also when a new level is reached, we will display a big message
	// right over the combat window.
	//
	SetNewBigScreenMessage(_("Level Gained!"));
	Takeover_Game_Won_Sound();
    }
}; // void check_for_new_experience_level_reached ( )

/* ----------------------------------------------------------------------
 *
 *
 * ---------------------------------------------------------------------- */
void
update_all_primary_stats ()
{
    int i;

    //--------------------
    // Now we base PRIMARY stats
    //
    Me . Strength = Me . base_strength;
    Me . Dexterity = Me . base_dexterity;
    Me . Magic = Me . base_magic;
    Me . Vitality = Me . base_vitality;
    
    Me . freezing_melee_targets = 0;
    Me . double_ranged_damage = FALSE;

    //--------------------
    // Now we re-initialize the SKILL LEVELS
    //
    for ( i = 0; i < number_of_skills; i ++)
	 Me . SkillLevel[i] =  Me . base_skill_level[i];

    //--------------------
    // Now we add all bonuses to the influencers PRIMARY stats
    //
    AddInfluencerItemAttributeBonus( & Me . armour_item );
    AddInfluencerItemAttributeBonus( & Me . weapon_item );
    AddInfluencerItemAttributeBonus( & Me . drive_item );
    AddInfluencerItemAttributeBonus( & Me . shield_item );
    AddInfluencerItemAttributeBonus( & Me . special_item );

    item * itrot [5] = {  & Me . armour_item,  & Me . weapon_item ,  & Me . drive_item ,  
    		& Me . shield_item , & Me . special_item };
    i = 0;
    while ( i < 5 ) 
	{
	if ( itrot [ i ] -> type == -1 ) 
		{
		i ++;
		continue;
		}
	if ( ! ItemUsageRequirementsMet ( itrot [ i ] , FALSE ) )
		{ //we have to move away the item
		if ( ! AddFloorItemDirectlyToInventory ( itrot [ i ] ) ) itrot [ i ] -> type = -1;
		else 
			{ //inventory is full... ouch
			DropItemToTheFloor(itrot[i], Me.pos.x, Me.pos.y, Me.pos.z);
			itrot [ i ] -> type = -1;
			}
		}
	i ++;
	}



    //--------------------
    // Maybe there is some boost from a potion or magic spell going on right
    // now...
    //
    if ( Me . dexterity_bonus_end_date > Me . current_game_date )
	Me . Dexterity += Me . current_dexterity_bonus ;
    if ( Me . power_bonus_end_date > Me . current_game_date )
	Me . Strength += Me . current_power_bonus ;

}; // void update_all_primary_stats ( )

/* ----------------------------------------------------------------------
 * This function computes secondary stats (i.e. chances for success or
 * getting hit and the like) using ONLY THE PRIMARY STATS.  Bonuses from
 * current 'magic' modifiers from equipped items will be applied somewhere
 * else.
 * ---------------------------------------------------------------------- */
void
update_secondary_stats_from_primary_stats ()
{
    //--------------------
    // The chance that this player character will score a hit on an enemy
    //
    Me . to_hit = 
	60 + ( Me . Dexterity - 15 ) * TOHIT_PERCENT_PER_DEX_POINT;

    //--------------------
    // How many life points can this character aquire currently
    //
    Me . maxenergy = 
	( Me . Vitality ) * Energy_Gain_Per_Vit_Point;
    
    //--------------------
    // The maximum mana value computed from the primary stats
    //
    Me . max_temperature = 
	( Me . Magic )    * Maxtemp_Gain_Per_CPU_Point;

    //--------------------
    // How long can this character run until he must take a break and
    // walk a bit
    //
    Me . max_running_power = 
	( Me . Strength ) + 
	( Me . Dexterity ) + 
	( Me . Vitality ) +
	Me . running_power_bonus ;


    //--------------------
    // base regeneration speed set to 0.2 points per second
    Me . health_recovery_rate = 0.2;
    Me . cooling_rate = 1.0;
}; // void update_secondary_stats_from_primary_stats ( )

/* ----------------------------------------------------------------------
 * Now we compute the possible damage the player character can do.
 * The damage value of course depends on the weapon type that the
 * character is using.  And depending on the weapon type (melee or
 * ranged weapon) some additional attributes will also play a role.
 * ---------------------------------------------------------------------- */
void
update_damage_tux_can_do ()
{
    if ( Me . weapon_item . type != (-1) )
    {
	if ( ItemMap[ Me . weapon_item . type ] . item_gun_angle_change != 0 )
	{
	    //--------------------
	    // Damage modifier in case of MELEE WEAPON is computed:  
	    // weapon's modifier * (100+Strength)%
	    //
	    Me . base_damage = Me . weapon_item.damage * 
		( Me . Strength + 100.0) / 100.0 ;
	    
	    Me . damage_modifier = Me . weapon_item . damage_modifier * 
		( Me . Strength + 100.0) / 100.0 ;
	    
	    //--------------------
	    // Damage AND damage modifier a modified by additional melee weapon
	    // skill:  A multiplier is applied!
	    //
	    Me . damage_modifier *= MeleeDamageMultiplierTable [ Me . melee_weapon_skill ] ;
	    Me . base_damage     *= MeleeDamageMultiplierTable [ Me . melee_weapon_skill ] ;
	}
	else
	{
	    //--------------------
	    // Damage modifier in case of RANGED WEAPON is computed:  
	    // weapon's modifier * (100+Dexterity)%
	    //
	    Me . base_damage = Me . weapon_item . damage * 
		( Me . Dexterity + 100.0 ) / 100.0 ;
	    Me . damage_modifier = Me . weapon_item . damage_modifier * 
		( Me . Dexterity + 100.0 ) / 100.0 ;
	    
	    //--------------------
	    // Damage AND damage modifier a modified by additional ranged weapon
	    // skill:  A multiplier is applied!
	    //
	    Me . damage_modifier *= RangedDamageMultiplierTable [ Me . ranged_weapon_skill ] ;
	    Me . base_damage     *= RangedDamageMultiplierTable [ Me . ranged_weapon_skill ] ;
	    
	    //--------------------
	    // Maybe there is a plugin for double damage with ranged
	    // weapons present?
	    //
	    if ( Me . double_ranged_damage != 0 )
	    {
		Me . base_damage *= 2;
		Me . damage_modifier *= 2;
	    }
	}
    }
    else
    {
	//--------------------
	// In case of no weapon equipped at all, we initialize
	// the damage values with some simple numbers.  Currently
	// strength and dexterity play NO ROLE in weaponless combat.
	// Maybe that should be changed for something more suitable
	// at some point...
	//
	Me . base_damage = 0;
	Me . damage_modifier = 1;
    }
}; // void update_damage_tux_can_do ( )

/* ----------------------------------------------------------------------
 *
 *
 * ---------------------------------------------------------------------- */
void
update_tux_armour_class ()
{
    //--------------------
    // We initialize the armour class value from the primary stat, 
    // using the dexterity value (and the 'character class')
    //
    Me . AC = 
	( Me . Dexterity - 15 ) * AC_Gain_Per_Dex_Point ;

    //--------------------
    // Now we apply the armour bonuses from the currently equipped
    // items to the total defence value
    //
    if ( Me . armour_item . type != (-1) )
    {
	Me . AC += Me . armour_item . ac_bonus;
    }
    if ( Me . shield_item.type != (-1) )
    {
	Me . AC += Me . shield_item . ac_bonus;
    }
    if ( Me . special_item.type != (-1) )
    {
	Me . AC += Me . special_item . ac_bonus;
    }
    if ( Me . drive_item.type != (-1) )
    {
        Me . AC += Me . drive_item . ac_bonus;
    }

}; // void update_tux_armour_class ( )


/* ----------------------------------------------------------------------
 * This function should re-compute all character stats according to the
 * currently equipped items and currenly distributed stats points.
 * ---------------------------------------------------------------------- */
void 
UpdateAllCharacterStats ()
{
    //--------------------
    // Maybe the influencer has reached a new experience level?
    // Let's check this...
    // 
    check_for_new_experience_level_reached ();

    //--------------------
    // The primary status must be computed/updated first, because
    // the secondary status (chances and the like) will depend on
    // them...
    //
    update_all_primary_stats ();

    //--------------------
    // At this point we know, that the primary stats of the influencer
    // have been fully computed.  So that means, that finally we can compute
    // all base SECONDARY stats, that are dependent upon the influencer primary
    // stats.  Once we are done with that, the modifiers to the secondary
    // stats can be applied as well.
    //
    update_secondary_stats_from_primary_stats ();

    //--------------------
    // Now we compute the possible damage the player character can do.
    // The damage value of course depends on the weapon type that the
    // character is using.  And depending on the weapon type (melee or
    // ranged weapon) some additional attributes will also play a role.
    //
    update_damage_tux_can_do ();

    //--------------------
    // Update tux armour class
    //
    update_tux_armour_class ();

    //--------------------
    // So at this point we can finally apply all the modifiers to the influencers
    // SECONDARY stats due to 'magical' items and spells and the like
    //
    Me . light_bonus_from_tux = 0 ;
    Me . resist_disruptor = 0 ;
    AddInfluencerItemSecondaryBonus( & Me . armour_item );
    AddInfluencerItemSecondaryBonus( & Me . weapon_item );
    AddInfluencerItemSecondaryBonus( & Me . drive_item );
    AddInfluencerItemSecondaryBonus( & Me . shield_item );
    AddInfluencerItemSecondaryBonus( & Me . special_item );

    //--------------------
    // There also should be an upper limit to disruptor protection,
    // so that negative values can be avoided and also such that
    // disruptor bots don't become completely useless...
    //
    if ( Me . resist_disruptor > 85 ) Me . resist_disruptor = 85 ;
	
	
    //--------------------
    // Check player's health and temperature
    if ( Me . energy > Me . maxenergy ) Me . energy = Me . maxenergy;
    if ( Me . temperature < 0 ) Me . temperature = 0;


    //--------------------
    // Now that the defence stat is computed, we can compute the chance, that
    // a randomly chosen lv. 1 bot will hit the Tux in any given strike...
    //
    Me . lv_1_bot_will_hit_percentage =
	( int ) ( exp ( - 0.018 * ( (float) Me . AC ) ) * 100.0 );

}; // void UpdateAllCharacterStats ( void )

/* ----------------------------------------------------------------------
 * Now we print out the current skill levels in hacking skill, 
 * spellcasting, melee combat, ranged weapon combat and repairing things
 * ---------------------------------------------------------------------- */
void
show_character_screen_skills ( )
{
    
    //--------------------
    // We add some security against skill values out of allowed
    // bounds.
    //
    if ( ( Me . melee_weapon_skill  < 0 ) ||
	 ( Me . melee_weapon_skill >= NUMBER_OF_SKILL_LEVELS ) )
    {
	fprintf ( stderr , "\nmelee_weapon_skill: %d." , Me . melee_weapon_skill );
	ErrorMessage ( __FUNCTION__ , "\
Error: melee weapon skill seems out of bounds.",
				   PLEASE_INFORM, IS_FATAL );
    }
    DisplayText( _(AllSkillTexts [ Me . melee_weapon_skill ]), 
		 MELEE_SKILL_X + CharacterRect.x , MELEE_SKILL_Y + CharacterRect.y , &CharacterRect , TEXT_STRETCH );

    //--------------------
    // We add some security against skill values out of allowed
    // bounds.
    //
    if ( ( Me . ranged_weapon_skill < 0 ) ||
	 ( Me . ranged_weapon_skill >= NUMBER_OF_SKILL_LEVELS ) )
    {
	fprintf ( stderr , "\nranged_weapon_skill: %d." , Me . ranged_weapon_skill );
	ErrorMessage ( __FUNCTION__ , "\
Error: ranged weapon skill seems out of bounds.",
				   PLEASE_INFORM, IS_FATAL );
    }
    DisplayText( _(AllSkillTexts [ Me . ranged_weapon_skill ]), 
		 RANGED_SKILL_X + CharacterRect.x , RANGED_SKILL_Y + CharacterRect.y , &CharacterRect , TEXT_STRETCH );

    //--------------------
    // We add some security against skill values out of allowed
    // bounds.
    //
    if ( ( Me . spellcasting_skill < 0 ) ||
	 ( Me . spellcasting_skill >= NUMBER_OF_SKILL_LEVELS ) )
    {
	fprintf ( stderr , "\nProgramming_Skill: %d." , Me . spellcasting_skill );
	ErrorMessage ( __FUNCTION__ , "\
Error: Programming_Skill skill seems out of bounds.",
				   PLEASE_INFORM, IS_FATAL );
    }
    DisplayText( _(AllSkillTexts [ Me . spellcasting_skill ]), 
		 SPELLCASTING_SKILL_X + CharacterRect.x , SPELLCASTING_SKILL_Y + CharacterRect.y , &CharacterRect , TEXT_STRETCH );

    //--------------------
    // We add some security against skill values out of allowed
    // bounds.
    //
    if ( ( Me . hacking_skill < 0 ) ||
	 ( Me . hacking_skill >= NUMBER_OF_SKILL_LEVELS ) )
    {
	fprintf ( stderr , "\nhacking_skill: %d." , Me . hacking_skill );
	ErrorMessage ( __FUNCTION__ , "\
Error: hacking skill seems out of bounds.",
				   PLEASE_INFORM, IS_FATAL );
    }
    DisplayText( _(AllSkillTexts [ Me . hacking_skill ]), 
		 HACKING_SKILL_X + CharacterRect.x , HACKING_SKILL_Y + CharacterRect.y , &CharacterRect , TEXT_STRETCH );

    /*
      if ( Me . repair_skill ) 
      DisplayText( "Yes" , CharacterRect.x + 80 , CharacterRect.y + 444 , &CharacterRect );
      else
      DisplayText( "No" , CharacterRect.x + 80 , CharacterRect.y + 444 , &CharacterRect );
    */
}; // void show_character_screen_skills ( )

/* ----------------------------------------------------------------------
 * This function displays the character screen.
 * ---------------------------------------------------------------------- */
void 
ShowCharacterScreen ( )
{
    char CharText[1000];
    point CurPos;
    
    DebugPrintf ( 2 , "\n%s(): Function call confirmed." , __FUNCTION__ );
    
    //--------------------
    // If the log is not set to visible right now, we do not need to 
    // do anything more, but to restore the usual user rectangle size
    // back to normal and to return...
    //
    if ( GameConfig.CharacterScreen_Visible == FALSE ) return;

    SetCurrentFont ( Message_BFont );
    
    // --------------------
    // We will need the current mouse position on several spots...
    //
    CurPos.x = GetMousePos_x() ;
    CurPos.y = GetMousePos_y() ;
    
    //--------------------
    // We define the right side of the user screen as the rectangle
    // for our inventory screen.
    //
    CharacterRect . x = GameConfig . screen_width - CHARACTERRECT_W;
    CharacterRect . y = 0; 
    CharacterRect . w = CHARACTERRECT_W;
    CharacterRect . h = CHARACTERRECT_H;
    
    blit_special_background ( CHARACTER_SCREEN_BACKGROUND_CODE );
    
    SetCurrentFont ( Menu_BFont );
    DisplayText( Me.character_name , 20 + CharacterRect.x , 12 + CharacterRect.y , &CharacterRect , TEXT_STRETCH );
    SetCurrentFont ( Message_BFont );
    
    sprintf( CharText , "%d", Me.exp_level );
    DisplayText( CharText , 110 + CharacterRect.x , 73 + CharacterRect.y , &CharacterRect , TEXT_STRETCH );
    
    sprintf( CharText , "%lu", Me.Experience ); 
    DisplayText( CharText , 110 + CharacterRect.x ,  89 + CharacterRect.y , &CharacterRect , TEXT_STRETCH );
    
    sprintf( CharText , "%lu", Me.ExpRequired );
    DisplayText( CharText , 110 + CharacterRect.x ,  107 + CharacterRect.y , &CharacterRect , TEXT_STRETCH ) ;
    
    sprintf( CharText , "%6ld", Me.Gold ); 
    DisplayText( CharText , 240 + CharacterRect.x ,  71 + CharacterRect.y , &CharacterRect , TEXT_STRETCH );
    
    SetCurrentFont( Message_BFont) ;
    sprintf( CharText , "%d", Me.Strength);
    if ( Me.Strength != Me.base_strength ) 
	sprintf ( CharText + strlen(CharText), " (%+d)", Me.Strength - Me.base_strength); 
    DisplayText( CharText , STR_X + CharacterRect.x , STR_Y + CharacterRect.y , &CharacterRect , TEXT_STRETCH );
    
    SetCurrentFont( Message_BFont) ;
    sprintf( CharText , "%d", Me.Magic );
    if ( Me.Magic != Me.base_magic ) 
	sprintf ( CharText + strlen(CharText), " (%+d)", Me.Magic - Me.base_magic); 
    DisplayText( CharText , STR_X + CharacterRect.x , MAG_Y + CharacterRect.y , &CharacterRect , TEXT_STRETCH );
    
    SetCurrentFont( Message_BFont) ;
    sprintf( CharText , "%d", Me.Dexterity );
    if ( Me.Dexterity != Me.base_dexterity ) 
	sprintf ( CharText + strlen(CharText), " (%+d)", Me.Dexterity - Me.base_dexterity); 
    DisplayText( CharText , STR_X + CharacterRect.x , DEX_Y + CharacterRect.y , &CharacterRect , TEXT_STRETCH );
    
    SetCurrentFont( Message_BFont) ;
    sprintf( CharText , "%d", Me.Vitality );
    if ( Me.Vitality != Me.base_vitality ) 
	sprintf ( CharText + strlen(CharText), " (%+d)", Me.Vitality - Me.base_vitality); 
    DisplayText( CharText , STR_X + CharacterRect.x , VIT_Y + CharacterRect.y , &CharacterRect , TEXT_STRETCH );
    
    SetCurrentFont( Message_BFont) ;
    sprintf( CharText , "%d", Me.points_to_distribute );
    DisplayText( CharText , STR_X + CharacterRect.x , POINTS_Y + CharacterRect.y , &CharacterRect , TEXT_STRETCH );
    
    sprintf( CharText , "%d/%d", (int) Me.energy, (int) Me.maxenergy );
    DisplayText( CharText , 105 + CharacterRect.x , 289 + CharacterRect.y , &CharacterRect , TEXT_STRETCH );
    
    sprintf( CharText , "%d/%d", (int) Me . temperature, (int) Me.max_temperature );
    DisplayText( CharText , 105 + CharacterRect.x , 308+ CharacterRect.y , &CharacterRect , TEXT_STRETCH );

    sprintf( CharText , "%d/%d", (int) Me . running_power, (int) Me . max_running_power );
    DisplayText( CharText , 105 + CharacterRect.x , 327 + CharacterRect.y , &CharacterRect , TEXT_STRETCH );
    
    sprintf( CharText , "%d", (int) Me.AC );
    DisplayText( CharText , AC_X + CharacterRect.x , AC_Y + CharacterRect.y , &CharacterRect , TEXT_STRETCH );

    sprintf( CharText , "%d%%", (int) Me.to_hit );
    DisplayText( CharText , TOHIT_X + CharacterRect.x , TOHIT_Y + CharacterRect.y , &CharacterRect , TEXT_STRETCH );

    sprintf( CharText , "%d-%d", (int) Me.base_damage , (int) Me.base_damage + (int) Me.damage_modifier );
    DisplayText( CharText , DAMAGE_X + CharacterRect.x , DAMAGE_Y + CharacterRect.y , &CharacterRect , TEXT_STRETCH );
    
    
    
    //--------------------
    // Now we print out the current skill levels in hacking skill, 
    // spellcasting, melee combat, ranged weapon combat and repairing things
    //
    show_character_screen_skills ( );
    if ( Me.points_to_distribute > 0 )
    {
	ShowGenericButtonFromList ( MORE_STR_BUTTON );
	ShowGenericButtonFromList ( MORE_DEX_BUTTON );
	ShowGenericButtonFromList ( MORE_VIT_BUTTON );
	ShowGenericButtonFromList ( MORE_MAG_BUTTON );
    }
}; //ShowCharacterScreen ( ) 

/* ----------------------------------------------------------------------
 * This function handles input for the character screen.
 * ---------------------------------------------------------------------- */
void
HandleCharacterScreen ( void )
{

    if ( ! GameConfig . CharacterScreen_Visible ) return;
    
    if ( Me.points_to_distribute > 0 )
    {
	if ( MouseCursorIsOnButton( MORE_STR_BUTTON , GetMousePos_x()  , GetMousePos_y()  ) && MouseLeftClicked() )
	{
	    Me.base_strength++;
	    Me.points_to_distribute--;
	}
	if ( MouseCursorIsOnButton( MORE_DEX_BUTTON , GetMousePos_x()  , GetMousePos_y()  ) && MouseLeftClicked() )
	{
	    Me.base_dexterity++;
	    Me.points_to_distribute--;
	}
	if ( MouseCursorIsOnButton( MORE_MAG_BUTTON , GetMousePos_x()  , GetMousePos_y()  ) && MouseLeftClicked() )
	{
	    Me.base_magic++;
	    Me.points_to_distribute--;
	}
	if ( MouseCursorIsOnButton( MORE_VIT_BUTTON , GetMousePos_x()  , GetMousePos_y()  ) && MouseLeftClicked() )
	{
	    Me.base_vitality++;
	    Me.points_to_distribute--;
	}
	
    }

    
}; // HandleCharacterScreen ( void )

#undef _character_c
