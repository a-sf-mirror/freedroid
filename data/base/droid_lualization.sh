#!/bin/bash

AWKSCRIPT=$(mktemp --suffix=.awk)

cat >$AWKSCRIPT <<EOT
BEGIN { 
	FS=": *|= *";
	in_spec = 0;
	# Default values
	def_name = "";
	def_desc = "";
	def_notes = "";
	def_is_human = 1;
	def_class = 1;
	def_ab_speed_max = 2;
	def_ab_energy_max = 10;
	def_ab_healing_rate = 2;
	def_ab_hit_draw = 50;
	def_ab_aggression_distance = 10;
	def_ab_time_eyeing = 1.0;
	def_ab_recover_time = 1.0;
	def_ab_xp_reward = 40;
	def_eq_weapon = "NPC Hand to hand weapon";
	def_eq_sensor = "spectral";
	def_drop_class = -1;
	def_drop_plasma = 0;
	def_drop_superconductors = 0;
	def_drop_antimatter = 0;
	def_drop_entropy = 0;
	def_drop_tachyon = 0;
	def_gfx_prefix = "";
	def_gfx_muzzle = 30;
	def_gfx_rotations = 0;
	def_sound_greeting = -1;
	def_sound_attack = -1;
	def_sound_death = "none";
	print "droid_list {";
}

END { print "}"; }

/^** Start of new Robot: **/ && in_spec == 0 {
		in_spec = 1;
		# init data to default
		name = def_name;
		desc = def_desc;
		notes = def_notes;
		is_human = def_is_human;
		class = def_class;
		ab_speed_max = def_ab_speed_max;
		ab_energy_max = def_ab_energy_max;
		ab_healing_rate = def_ab_healing_rate;
		ab_hit_draw = def_ab_hit_draw;
		ab_aggression_distance = def_ab_aggression_distance;
		ab_time_eyeing = def_ab_time_eyeing;
		ab_recover_time = def_ab_recover_time;
		ab_xp_reward = def_ab_xp_reward;
		eq_weapon = def_eq_weapon;
		eq_sensor = def_eq_sensor;
		drop_class = def_drop_class;
		drop_plasma = def_drop_plasma;
		drop_superconductors = def_drop_superconductors;
		drop_antimatter = def_drop_antimatter;
		drop_entropy = def_drop_entropy;
		drop_tachyon = def_drop_tachyon;
		gfx_prefix = def_gfx_prefix;
		gfx_muzzle = def_gfx_muzzle;
		gfx_rotations = def_gfx_rotations;
		sound_greeting = def_sound_greeting;
		sound_attack = def_sound_attack;
		sound_death = def_sound_death;
		next;
	}

(/^** Start of new Robot: **/ || /^*** End of Robot Data Section: ***/) && in_spec == 1 {
		# print previous droid specs
		print "{";
			print "\tname = \""name"\",";
			if (desc != def_desc) print "\tdesc = "desc",";
			if (notes != def_notes) print "\tnotes = "notes",";
			if (is_human != def_is_human) print "\tis_human = "is_human",";
			if (class != def_class) print "\tclass = "class",";
			print "\tabilities = {";
				if (ab_speed_max != def_ab_speed_max) print "\t\tspeed_max = "ab_speed_max",";
				if (ab_energy_max != def_ab_energy_max) print "\t\tenergy_max = "ab_energy_max",";
				if (ab_healing_rate != def_ab_healing_rate) print "\t\thealing_rate = "ab_healing_rate",";
				if (ab_hit_draw != def_ab_hit_draw) print "\t\thit_draw = "ab_hit_draw",";
				if (ab_aggression_distance != def_ab_aggression_distance) print "\t\taggression_distance = "ab_aggression_distance",";
				if (ab_time_eyeing != def_ab_time_eyeing) print "\t\ttime_eyeing = "ab_time_eyeing",";
				if (ab_recover_time != def_ab_recover_time) print "\t\trecover_time = "ab_recover_time",";
				if (ab_xp_reward != def_ab_xp_reward) print "\t\txp_reward = "ab_xp_reward",";
			print "\t},";
			print "\tequip = {";
				if (eq_weapon != def_eq_weapon) print "\t\tweapon = "eq_weapon",";
				if (eq_sensor != def_eq_sensor) print "\t\tsensor = "eq_sensor",";
			print "\t},";
			print "\tdrop_draw = {";
				if (drop_class != def_drop_class) print "\t\tclass = "drop_class",";
				if (drop_plasma != def_drop_plasma) print "\t\tplasma_transistors = "drop_plasma",";
				if (drop_superconductors != def_drop_superconductors) print "\t\tsuperconductors = "drop_superconductors",";
				if (drop_antimatter != def_drop_antimatter) print "\t\tantimatter_converters = "drop_antimatter",";
				if (drop_entropy != def_drop_entropy) print "\t\tentropy_inverters = "drop_entropy",";
				if (drop_tachyon != def_drop_tachyon) print "\t\ttachyon_condensators = "drop_tachyon",";
			print "\t},";
			print "\tgfx = {";
				if (gfx_prefix != def_gfx_prefix) print "\t\tprefix = "gfx_prefix",";
				if (gfx_muzzle != def_gfx_muzzle) print "\t\tgun_muzzle_height = "gfx_muzzle",";
				print "\t\tanimation = {";
					if (gfx_rotations != def_gfx_rotations) print "\t\t\tportrait_rotations = "gfx_rotations",";
				print "\t\t},";
			print "\t},";
			print "\tsound = {";
				if (sound_greeting != def_sound_greeting) print "\t\tgreeting = sfx_sounds.g"sound_greeting",";
				sound_attack = sound_greeting;
				if (sound_attack != def_sound_attack) {
				    if (sound_attack <= 2 || sound_attack >= 6) print "\t\tattack = sfx_sounds.a"sound_attack",";
				}
				if (sound_death != def_sound_death) {
					if (sound_death == "\"death_sound_123.ogg\"") print "\t\tdeath = sfx_sounds.d123,";
					if (sound_death == "\"death_sound_247.ogg\"") print "\t\tdeath = sfx_sounds.d247,";
					if (sound_death == "\"death_sound_302.ogg\"") print "\t\tdeath = sfx_sounds.d302,";
				}
			print "\t},";
		print "},";
		next;
	}

/^Droidname/ { name = \$2; next; }
/^Default description/ {
		gsub("Default description: *", "");
		desc = \$0;
		next;
	}
/^Notes concerning this droid/ {
		gsub("Notes concerning this droid= *", "");
		gsub(" *$", "");
		notes = \$0;
		next;
	}
/^Is this 'droid' a human/ { is_human = \$2; next; }
/^Class of this droid/ { class = \$2; next; }
/^Maximum speed of this droid/ { ab_speed_max = \$2; next; }
/^Maximum energy of this droid/ { ab_energy_max = \$2; next; }
/^Rate of healing/ { ab_healing_rate = \$2; next; }
/^Chance of this robot scoring a hit/ { ab_hit_draw = \$2; next; }
/^Aggression distance of this droid/ { ab_aggression_distance = \$2; next; }
/^Time spent eyeing Tux/ { ab_time_eyeing = \$2; next; }
/^Time to recover after getting hit/ { ab_recover_time = \$2; next; }
/^Experience_Reward gained for destroying one of this type/ { ab_xp_reward = \$2; next; }
/^Weapon item/ { eq_weapon = \$2; next; }
/^Sensor ID/ { eq_sensor = \$2; next; }
/^Drops item class/ { drop_class = \$2; next; }
/^Percent to drop Plasma Transistor/ { drop_plasma = \$2; next; }
/^Percent to drop Superconducting Relay Unit/ { drop_superconductors = \$2; next; }
/^Percent to drop Antimatter-Matter Converter/ { drop_antimatter = \$2; next; }
/^Percent to drop Entropy Inverter/ { drop_entropy = \$2; next; }
/^Percent to drop Tachyon Condensator/ { drop_tachyon = \$2; next; }
/^Filename prefix for graphics/ { gfx_prefix = \$2; next; }
/^Gun muzzle offset/ { gfx_muzzle = \$2; next; }
/^Droid uses portrait rotation series with prefix/ { gfx_rotations = 1; next; }
/^Greeting Sound number/ { sound_greeting = \$2; next; }
/^Death sound file name/ { sound_death = \$2; next; }
/^Attack animation sound file name/ { sound_attack = \$2; next; }
EOT

rm -f droid_specs.lua
cat >droid_specs.lua <<EOT
------------------------------------------------------------------------------
--
--  Copyright (c) 2002, 2003 Johannes Prix
--  Copyright (c) 2002 Reinhard Prix
--  Copyright (c) 2004-2007 Arthur Huillet
--  Copyright (c) 2007-2010 Stefan Huszics
--
--  This file is part of Freedroid
--
--  Freedroid is free software; you can redistribute it and/or modify
--  it under the terms of the GNU General Public License as published by
--  the Free Software Foundation; either version 2 of the License, or
--  (at your option) any later version.
--
--  Freedroid is distributed in the hope that it will be useful,
--  but WITHOUT ANY WARRANTY; without even the implied warranty of
--  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
--  GNU General Public License for more details.
--
--  You should have received a copy of the GNU General Public License
--  along with Freedroid; see the file COPYING. If not, write to the 
--  Free Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, 
--  MA  02111-1307  USA
--
-- 
-- Feel free to make any modifications you like.  If you set up 
-- something cool, please send your file in to the FreedroidRPG project.
--
------------------------------------------------------------------------------

------------------------------------------------------------------------------
-- This file defines the behavior and rendering of the droids.
------------------------------------------------------------------------------

---------
-- Human/NPCs 8
-- Civilians 2
-- Spectral 10 (Can't see invisible tux. Sensor ID: spectral. Most common and default.)
-- IR 11 (Sees only heatsources: Can see invisible Tux. Sensor ID: infrared. Use on advanced droids.)
-- X-Ray 12 (Sees through the walls. Sensor ID: xray.)
-- Radar 16 (Ultrasonic + IR. Use only on the most advanced droids. Caution to don't affect the game balance.)-
-- Subsonic 6 (soundwaves/vibrations, see through walls, invisible if not moving) // TODO: To be implemented.
---------

sfx_sounds = {
	g0  = "effects/bot_sounds/First_Contact_Sound_0.ogg",
	g1  = "effects/bot_sounds/First_Contact_Sound_1.ogg",
	g2  = "effects/bot_sounds/First_Contact_Sound_2.ogg",
	g3  = "effects/bot_sounds/First_Contact_Sound_3.ogg",
	g4  = "effects/bot_sounds/First_Contact_Sound_4.ogg",
	g5  = "effects/bot_sounds/First_Contact_Sound_5.ogg",
	g6  = "effects/bot_sounds/First_Contact_Sound_6.ogg",
	g7  = "effects/bot_sounds/First_Contact_Sound_7.ogg",
	g8  = "effects/bot_sounds/First_Contact_Sound_8.ogg",
	g9  = "effects/bot_sounds/First_Contact_Sound_9.ogg",
	g10 = "effects/bot_sounds/First_Contact_Sound_10.ogg",
	g11 = "effects/bot_sounds/First_Contact_Sound_11.ogg",
	g12 = "effects/bot_sounds/First_Contact_Sound_12.ogg",
	g13 = "effects/bot_sounds/First_Contact_Sound_13.ogg",
	g14 = "effects/bot_sounds/First_Contact_Sound_14.ogg",
	g15 = "effects/bot_sounds/First_Contact_Sound_15.ogg",
	g16 = "effects/bot_sounds/First_Contact_Sound_16.ogg",
	g17 = "effects/bot_sounds/First_Contact_Sound_17.ogg",
	g18 = "effects/bot_sounds/First_Contact_Sound_18.ogg",

	a0  = "effects/bot_sounds/Start_Attack_Sound_0.ogg",
	a1  = "effects/bot_sounds/Start_Attack_Sound_1.ogg",
	a2  = "effects/bot_sounds/Start_Attack_Sound_2.ogg",
	a6  = "effects/bot_sounds/Start_Attack_Sound_0.ogg",
	a7  = "effects/bot_sounds/Start_Attack_Sound_1.ogg",
	a8  = "effects/bot_sounds/Start_Attack_Sound_2.ogg",
	a9  = "effects/bot_sounds/Start_Attack_Sound_9.ogg",
	a10 = "effects/bot_sounds/Start_Attack_Sound_10.ogg",
	a11 = "effects/bot_sounds/Start_Attack_Sound_11.ogg",
	a12 = "effects/bot_sounds/Start_Attack_Sound_12.ogg",
	a13 = "effects/bot_sounds/Start_Attack_Sound_13.ogg",
	a14 = "effects/bot_sounds/Start_Attack_Sound_14.ogg",
	a15 = "effects/bot_sounds/Start_Attack_Sound_15.ogg",
	a16 = "effects/bot_sounds/Start_Attack_Sound_16.ogg",
	a17 = "effects/bot_sounds/Start_Attack_Sound_17.ogg",
	a18 = "effects/bot_sounds/Start_Attack_Sound_18.ogg",

	d123 = "effects/bot_sounds/death_sound_123.ogg",
	d247 = "effects/bot_sounds/death_sound_247.ogg",
	d302 = "effects/bot_sounds/death_sound_302.ogg",
}

EOT

awk -f $AWKSCRIPT droid_archetypes.dat >> droid_specs.lua

rm -f $AWKSCRIPT
