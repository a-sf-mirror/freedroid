--[[
  Copyright (c) 2003 Johannes Prix

  This file is part of Freedroid

  Freedroid is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  Freedroid is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with Freedroid; see the file COPYING. If not, write to the 
  Free Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, 
  MA  02111-1307  USA
]]--

-- Feel free to make any modifications you like.  If you set up 
-- something cool, please send your file in to the Freedroid project.

mission_list {

-- Find a cure for Bender, suffering from brain enlargement pills
{
	mission_name = _"Bender's problem",
},

-- Retrieve the toolkit lost by Dixon
{
	mission_name = _"The yellow toolkit",
	completion_code = [=[
		change_obstacle_state("Town-Solar-1", "enabled")
		change_obstacle_state("Town-Solar-2", "enabled")
		change_obstacle_state("Town-Solar-3", "enabled")
		change_obstacle_state("Town-Solar-4", "enabled")
		change_obstacle_state("Town-Solar-5", "enabled")
		change_obstacle_state("Town-Solar-6", "enabled")
		change_obstacle_state("Town-Solar-7", "enabled")
		change_obstacle_state("Town-Solar-8", "enabled")
	]=],
},

-- Find some energy crystals for the cook Michelangelo

{
	mission_name = _"Anything but the army snacks, please!",
},

-- Defeat the bots in the novice arena
{
	mission_name = _"Novice Arena",
},

-- Defeat the bots in the master arena
{
	mission_name = _"Time to say goodnight",
	kill_droids_marked = 1030,
	completion_code = [=[
		change_obstacle_state("MasterArenaExitDoor", "opened")
		display_console_message(_"Master arena cleared. Good job, man.")
		display_big_message(_"Level cleared!")
		update_quest("Time to say goodnight", _"I managed to win the fight in the master arena. Am I really that good, or did I cheat?")
	]=],
},

-- Clean out the first level of the warehouse
{
	mission_name = _"Opening a can of bots...",
	must_clear_level = 1,
	completion_code = [=[
		update_quest("Opening a can of bots...", _"The first level of the warehouse has been cleared of bots; Dixon's men can now safely retrieve the supplies from here.")
	]=],
},

-- Clean out the second level under Kevins station
{
	mission_name = _"And there was light...",
	must_clear_level = 18,
	completion_code = [=[
		update_quest("And there was light...", _"Hope everything is back to normal now. I better go check with Kevin.")
		display_console_message(_"All hostiles on level disabled.")
		display_big_message(_"Level cleared!")
	]=],
},

-- Take Kevins data cube to the cluster maintenance people inside the red guard
-- complex
{
	mission_name = _"A kingdom for a cluster!",
},

-- Reach the disruptor shield generator and take control of it
{
	mission_name = _"Opening access to MS Office",
},

-- ENDGAME MISSION - propagate faulty firmware update.
-- This mission can only be finished through a talk with the firmware update server.
{
	mission_name = _"Propagating a faulty firmware update",
},

-- Bring SADD dilithium crystals 
{
	mission_name = _"SADD's power supply",
},

-- Free Tania & Escort Tania to the town
{
	mission_name = _"Tania's Escape",
	kill_droids_marked = 2035,
	completion_code = [=[
		if (npc_dead("Tania")) then
			display_big_message(_"Tania died!")
			display_console_message(_"Tania died!")
			if (not Tania_surface) then
				update_quest("Tania's Escape", _"Tania died before escaping her underground bunker. I was unable to protect her.")
			elseif (not Tania_stopped_by_Pendragon) then
				update_quest("Tania's Escape", _"Tania died in the desert while trying to make it to the town. I was unable to protect her.")
				change_obstacle_state("DesertGate-Inner", "opened")
			else
				update_quest("Tania's Escape", _"Tania died while trying to make it to the town. I was unable to protect her.")
			end
		else
			display_big_message(_"Tania made it to the town!")
			display_console_message(_"Tania successfully made it to the town!")
			add_xp(3000)
		end
	]=],
},

-- Help Stone pay tax
{
	mission_name = _"Saving the shop",
	completion_code = [=[
		sell_item("Shotgun shells", 1, "Stone")
		sell_item(".22 LR Ammunition", 1, "Stone")
	]=],
},

-- Find Koan
{
	mission_name = _"Doing Duncan a favor",
},

-- Learn to Move
{
	mission_name = _"Tutorial Movement",
},

-- Defeat the shooting range bots in the Tutorial
{
	mission_name = _"Tutorial Melee",
	kill_droids_marked = 2037,
	completion_code = [=[
		display_big_message(_"Melee droids destroyed!")
	]=],
},

-- Defeat the shooting range bots in the Tutorial
{
	mission_name = _"Tutorial Shooting",
	kill_droids_marked = 4037,
	completion_code = [=[
		display_big_message(_"Shooting range cleared!")
	]=],
},

-- Defeat the hacking area bots in the Tutorial
{
	mission_name = _"Tutorial Hacking",
	kill_droids_marked = 3037,
	completion_code = [=[
		display_big_message(_"Hacking area cleared!")
	]=],
},

-- Find Kevin
{
	mission_name = _"A strange guy stealing from town",
},

-- Deliver list from Francis to Spencer
{
	mission_name = _"Deliverance",
},

-- Deliver a nice meal to Will Gapes
{
	mission_name = _"Gapes Gluttony",
},

-- Upgrading Items in the Tutorial
{
	mission_name = _"Tutorial Upgrading Items",
	completion_code = [=[
		display_console_message(_"Tutorial Upgrading Items finished!")
	]=],
},

-- Clean out the old server room
{
	mission_name = _"Droids are my friends",
	must_clear_level = 58,
	completion_code = [=[
		update_quest("Droids are my friends", _"The old server room is now secured.")
	]=],
},

-- Save the town from nuclear disaster
{
	mission_name = _"An Explosive Situation",
	completion_code = [=[
		update_quest("An Explosive Situation", _"Not only is the town safe, but apparently so is Ewald's 296 droid. Oh joy.")
	]=],
},

-- Hack the Hell Fortress Gate Access Server and find the gate.
{
	mission_name = _"Open Sesame",
},

-- Get a red Toolbox back to Jenniffer
{
	mission_name = _"Jennifer's Toolbox",
},

-- This is a test quest
{
	mission_name = _"24_label_test_quest",
},

-- This is a test quest
{
	mission_name = _"24_dude_test_quest",
},

-- Another test quest
{
	mission_name = _"24_guy_death_quest",
	kill_droids_marked = 2025,
	completion_code = [=[
		display_big_message("24 guy death quest solved")
	]=],
},

-- Bring one dilithium crystal to Bob for his 614
{
	mission_name = _"Ticket's price - crystal",
        completion_code = [=[ 
		change_obstacle_state("ToTheWorld2PortalGate", "opened")
		add_xp(1000)
	]=],
},

-- Bring 999 Cerebrum some add-ons for his needs
{
	mission_name = _"Bot in need",
	completion_ccode = [=[
		add_xp(500)
	]=],
},

}

