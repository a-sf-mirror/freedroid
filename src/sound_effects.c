/*
 *
 *   Copyright (c) 2006 Arvid Picciani
 *   Copyright (c) 2004-2007 Arthur Huillet 
 *   Copyright (c) 1994, 2002, 2003 Johannes Prix
 *   Copyright (c) 1994, 2002 Reinhard Prix
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

#include "struct.h"
#include "global.h"
#include "proto.h"

void play_open_chest_sound(void)
{
	play_sound("effects/open_chest_sound.ogg");
}

void Play_Spell_ForceToEnergy_Sound()
{
	play_sound_cached("effects/Spell_ForceToEnergy_Sound_0.ogg");
}

/**
 * This function plays a voice sample, stating that not enough power
 * (strength) is available to use a certain item.
 * The sample must of course only be played, if it hasn't been played just
 * milliseconds before, so a check is made to see that the file is played
 * with at least a certain interval in between to the last occasion of the
 * file being played.
 */
void No_Ammo_Sound(void)
{
	static Uint32 PreviousSound = (-1);
	Uint32 now;

	now = SDL_GetTicks();
	if (SDL_GetTicks() - PreviousSound >= 0.25 * 1000) {
		play_sound_cached("effects/No_Ammo_Sound_0.ogg");
		PreviousSound = now;
	}
}

/**
 * This function plays a voice sample, stating that not enough power
 * (strength) is available to use a certain item.
 * The sample must of course only be played, if it hasn't been played just
 * milliseconds before, so a check is made to see that the file is played
 * with at least a certain interval in between to the last occasion of the
 * file being played.
 */
void Not_Enough_Power_Sound(void)
{
	static Uint32 PreviousSound = (-1);
	Uint32 now;

	now = SDL_GetTicks();
	if (SDL_GetTicks() - PreviousSound >= 1.15 * 1000) {
		play_sound_cached("effects/tux_ingame_comments/Not_Enough_Power_Sound_0.ogg");
		PreviousSound = now;
	}
}

/**
 * This function plays a voice sample, stating that not enough power
 * distribution (dexterity) is available to use a certain item.
 * The sample must of course only be played, if it hasn't been played just
 * milliseconds before, so a check is made to see that the file is played
 * with at least a certain interval in between to the last occasion of the
 * file being played.
 */
void Not_Enough_Dist_Sound(void)
{
	static Uint32 PreviousSound = (-1);
	Uint32 now;

	now = SDL_GetTicks();
	if (SDL_GetTicks() - PreviousSound >= 1.15 * 1000) {
		play_sound_cached("effects/tux_ingame_comments/Not_Enough_Dist_Sound_0.ogg");
		PreviousSound = now;
	}
}

/**
 * Whenever the Tux meets someone in the game for the very first time, 
 * this enemy or friend will issue the first-time greeting.
 * Depending on whether that is typically a hostile unit or not, we play
 * the corresponding sound from preloaded memory or, if there is no combat
 * to expect, by loading it freshly from disk and then forgetting about it
 * again.
 */
void play_greeting_sound(enemy *ThisRobot)
{
	int SoundCode = Droidmap[ThisRobot->type].greeting_sound_type;
	const char *sounds[] = {
		"effects/bot_sounds/First_Contact_Sound_0.ogg",
		"effects/bot_sounds/First_Contact_Sound_1.ogg",
		"effects/bot_sounds/First_Contact_Sound_2.ogg",
		"effects/bot_sounds/First_Contact_Sound_3.ogg",
		"effects/bot_sounds/First_Contact_Sound_4.ogg",
		"effects/bot_sounds/First_Contact_Sound_5.ogg",
		"effects/bot_sounds/First_Contact_Sound_6.ogg",
		"effects/bot_sounds/First_Contact_Sound_7.ogg",
		"effects/bot_sounds/First_Contact_Sound_8.ogg",
		"effects/bot_sounds/First_Contact_Sound_9.ogg",
		"effects/bot_sounds/First_Contact_Sound_10.ogg",
		"effects/bot_sounds/First_Contact_Sound_11.ogg",
		"effects/bot_sounds/First_Contact_Sound_12.ogg",
		"effects/bot_sounds/First_Contact_Sound_13.ogg",
		"effects/bot_sounds/First_Contact_Sound_14.ogg",
		"effects/bot_sounds/First_Contact_Sound_15.ogg",
		"effects/bot_sounds/First_Contact_Sound_16.ogg",
		"effects/bot_sounds/First_Contact_Sound_17.ogg",
		"effects/bot_sounds/First_Contact_Sound_18.ogg",
	};

	// Ensure that SoundCode will index a sound in the sounds[] c string array.
	if ((SoundCode < 0) || (SoundCode >= (sizeof(sounds) / sizeof(sounds[0])))) {
		error_message(__FUNCTION__, "Unknown Greeting sound!!!", PLEASE_INFORM);
		return;
	}

	play_sound_at_position(sounds[SoundCode], &Me.pos, &ThisRobot->pos);
}

/**
 * Whenever a bot dies, that should create a dying sound.  But so far,
 * this will be done only for fully animated bots, since the other bots
 * just explode and that has a sound of it's own.
 */
void play_death_sound_for_bot(enemy * ThisRobot)
{
	char filename[5000];

	// If the keyword 'none' for the death sound file name is encountered,
	// nothing will be done...
	//.
	if (!strcmp(Droidmap[ThisRobot->type].droid_death_sound_file_name, "none"))
		return;

	// Now we play the given death sound, looking for the file in the
	// appropriate sound folder.
	//
	strcpy(filename, "effects/bot_sounds/");
	strcat(filename, Droidmap[ThisRobot->type].droid_death_sound_file_name);
	play_sound_at_position(filename, &Me.pos, &ThisRobot->pos);
}

/**
 * Whenever a bot starts to attack the Tux, he'll issue the attack cry.
 * This is done here, and no respect to loading time issues for now...
 */
void play_enter_attack_run_state_sound(enemy *ThisRobot)
{
	int SoundCode;
	const char *sounds[] = {
		"effects/bot_sounds/Start_Attack_Sound_0.ogg",
		"effects/bot_sounds/Start_Attack_Sound_1.ogg",
		"effects/bot_sounds/Start_Attack_Sound_2.ogg",
		"effects/bot_sounds/Start_Attack_Sound_9.ogg",
		"effects/bot_sounds/Start_Attack_Sound_10.ogg",
		"effects/bot_sounds/Start_Attack_Sound_11.ogg",
		"effects/bot_sounds/Start_Attack_Sound_12.ogg",
		"effects/bot_sounds/Start_Attack_Sound_13.ogg",
		"effects/bot_sounds/Start_Attack_Sound_14.ogg",
		"effects/bot_sounds/Start_Attack_Sound_15.ogg",
		"effects/bot_sounds/Start_Attack_Sound_16.ogg",
		"effects/bot_sounds/Start_Attack_Sound_17.ogg",
		"effects/bot_sounds/Start_Attack_Sound_18.ogg",
	};

	if (MyRandom(5)) {
		// get sound code
		SoundCode = Droidmap[ThisRobot->type].greeting_sound_type;

		// sound codes are [0, 2]U[9, 18]
		// this makes it so the index will be [0, number_of_sounds - 1]
		if (SoundCode > 2)
			SoundCode -= 6;

		if (SoundCode < 0) {
			return;
		}

		// if index is higher than sound count, display debug message then return
		if (SoundCode > (sizeof(sounds) / sizeof(sounds[0]) - 1)) {
			DebugPrintf(0, "\nUnknown Start Attack sound!!! NOT TERMINATING CAUSE OF THIS...");
			return;
		}

		play_sound_at_position(sounds[SoundCode], &Me.pos, &ThisRobot->pos);

	} else {		//either we output a standard sound, either we output a special voice sample such as "drill eyes"
		char sample_path[1024] = "effects/bot_sounds/voice_samples/";
		sprintf(sample_path + strlen(sample_path), "%d.ogg", MyRandom(46) + 1);
		//printf("Playing %s\n", sample_path);
		play_sound_at_position(sample_path, &Me.pos, &ThisRobot->pos);
	}
}

/**
 * Whenever an item is placed or taken, we'll issue a sound attached to
 * that item. 
 */
void play_item_sound(int item_type, struct gps *item_pos)
{
	char final_file_name[5000];

	// First some sanity check...
	//
	if (item_type < 0) {
		error_message(__FUNCTION__, "\
		                           negative item type received (%d)!", PLEASE_INFORM | IS_FATAL, item_type);
	}
	// Now we can proceed and just play the sound associated with that item...
	//
	strcpy(final_file_name, "effects/item_sounds/");
	strcat(final_file_name, ItemMap[item_type].item_drop_sound_file_name);
	play_sound_at_position(final_file_name, &Me.pos, item_pos);

}

/**
 * This function generates a voice output stating that the influencer 
 * can't carry any more right now.  Also this function will see to it,
 * that the sentence is not repeated until 4 seconds after the previous
 * cant-carry-sentence have passed.
 */
void CantCarrySound(void)
{
	static Uint32 CurrentTicks = 0;

	if ((SDL_GetTicks() - CurrentTicks) > 2 * 1000) {
		CurrentTicks = SDL_GetTicks();
		switch (MyRandom(2)) {
		case 0:
			play_sound_cached("effects/tux_ingame_comments/ICantCarryAnyMore_Sound_0.ogg");
			break;
		case 1:
			play_sound_cached("effects/tux_ingame_comments/ICantCarryAnyMore_Sound_1.ogg");
			break;
		case 2:
			play_sound_cached("effects/tux_ingame_comments/ICantCarryAnyMore_Sound_2.ogg");
			break;
		default:
			break;
		}
	}
}


/**
 * When the Tux uses the 'teleport home' spell, this sound will be played.
 */
void teleport_arrival_sound(void)
{
	// play_sound_cached("effects/LeaveElevator_Sound_0.ogg");
	play_sound_cached("effects/new_teleporter_sound.ogg");
}

/**
 * Whenever the Tux gets hit, we *might* play a sound sample for the hit,
 * depending on random results.  Since we don't want to hear the same 
 * sound sample all the time, there is a selection of possible files 
 * one of which will be selected at random.
 */
void tux_scream_sound(void)
{
	switch (MyRandom(4)) {
	case 0:
		play_sound_cached("effects/Influencer_Scream_Sound_0.ogg");
		break;
	case 1:
		play_sound_cached("effects/Influencer_Scream_Sound_1.ogg");
		break;
	case 2:
		play_sound_cached("effects/Influencer_Scream_Sound_2.ogg");
		break;
	case 3:
		play_sound_cached("effects/Influencer_Scream_Sound_3.ogg");
		break;
	case 4:
		play_sound_cached("effects/Influencer_Scream_Sound_4.ogg");
		break;
	default:
		break;
	}
}

/**
 * We add a matching group of sounds here for the menu movements.  It's
 * a 'ping-ping' sound, well, not super, but where do we get a better one?
 */
void MenuItemSelectedSound(void)
{
	play_sound_cached("effects/Menu_Item_Selected_Sound_1.ogg");
}

void MenuItemDeselectedSound(void)
{
	play_sound_cached("effects/Menu_Item_Deselected_Sound_0.ogg");
}

void MoveMenuPositionSound(void)
{
	play_sound_cached("effects/Move_Menu_Position_Sound_0.ogg");
}

void ThouArtDefeatedSound(void)
{
	if (!sound_on)
		return;
	play_sound("effects/ThouArtDefeated_Sound_0.ogg");
}

/**
 * When the Tux makes a weapon swing, this will either cause a swinging
 * sound and then a 'hit' sound or it will just be a swinging sound.  The
 * following functions do this, also creating some variation in the choice
 * of sample used.
 */
void play_melee_weapon_hit_something_sound(void)
{
	switch (MyRandom(3)) {
	case 0:
		play_sound_cached("effects/swing_then_hit_1.ogg");
		break;
	case 1:
		play_sound_cached("effects/swing_then_hit_2.ogg");
		break;
	case 2:
		play_sound_cached("effects/swing_then_hit_3.ogg");
		break;
	case 3:
		play_sound_cached("effects/swing_then_hit_4.ogg");
		break;
	default:
		break;
	}
}

void play_melee_weapon_missed_sound(struct gps *attacker_pos)
{
	const char *sounds[] = {
		"effects/swing_then_nohit_1.ogg",
		"effects/swing_then_nohit_2.ogg",
		"effects/swing_then_nohit_3.ogg",
		"effects/swing_then_nohit_4.ogg",
	};
	int SoundCode = MyRandom(sizeof(sounds) / sizeof(sounds[0]) - 1);
	
	play_sound_at_position(sounds[SoundCode], &Me.pos, attacker_pos);
}

/**
 * This function should generate the sound that belongs to a certain
 * (ranged) weapon.  
 *
 * This does not include the Tux swinging/swinging_and_hit
 * sounds, when Tux is using melee weapons, but it does include ranged
 * weapons and the non-animated bot weapons too.
 */
void fire_bullet_sound(int BulletType, struct gps *shooter_pos)
{
	if (!sound_on)
		return;

	struct bulletspec *bullet_spec = dynarray_member(&bullet_specs, BulletType, sizeof(struct bulletspec));

	if (!bullet_spec->sound) {
		play_melee_weapon_missed_sound(shooter_pos);
		return;
	}

	char sound_file[100] = "effects/bullets/";
	strcat(sound_file, bullet_spec->sound);

	play_sound_at_position(sound_file, &Me.pos, shooter_pos);
}

/**
 * For the takeover game, there are 4 main sounds.  We handle them from
 * the cache, even if that might also be possible as 'once_needed' type
 * sound samples...
 */
void Takeover_Set_Capsule_Sound(void)
{
	play_sound_cached("effects/TakeoverSetCapsule_Sound_0.ogg");
}

void Takeover_Game_Won_Sound(void)
{
	play_sound_cached("effects/Takeover_Game_Won_Sound_0.ogg");
}

void Takeover_Game_Deadlock_Sound(void)
{
	play_sound_cached("effects/Takeover_Game_Deadlock_Sound_0.ogg");
}

void Takeover_Game_Lost_Sound(void)
{
	play_sound_cached("effects/Takeover_Game_Lost_Sound_0.ogg");
}

void play_blast_sound(char *blast_sound, struct gps *blast_pos)
{
	char fpath[2048];

	if (!blast_sound)
		return;

	strcpy(fpath, "effects/");
	strcat(fpath, blast_sound);
	play_sound_at_position(fpath, &Me.pos, blast_pos);
}

/**
 * This function plays a sound of a bullet being reflected. It is only
 * used, when a bullets is compensated by the tux armor.
 */
void BulletReflectedSound(void)
{
	play_sound_cached("effects/Bullet_Reflected_Sound_0.ogg");
}
