---------------------------------------------------------------------
-- This file is part of Freedroid
--
-- Freedroid is free software; you can redistribute it and/or modify
-- it under the terms of the GNU General Public License as published by
-- the Free Software Foundation; either version 2 of the License, or
-- (at your option) any later version.
--
-- Freedroid is distributed in the hope that it will be useful,
-- but WITHOUT ANY WARRANTY; without even the implied warranty of
-- MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
-- GNU General Public License for more details.
--
-- You should have received a copy of the GNU General Public License
-- along with Freedroid; see the file COPYING. If not, write to the
-- Free Software Foundation, Inc., 59 Temple Place, Suite 330, Boston,
-- MA 02111-1307 USA
----------------------------------------------------------------------
--[[WIKI
PERSONALITY = { "Militaristic", "Condescending" },
MARKERS = { NPCID1 = "Spencer" },
PURPOSE = "When Tux fist visits the town, $$NAME$$ will escort Tux to visit $$NPCID1$$. After that, $$NAME$$ will patrol the town."
WIKI]]--

local Npc = FDrpg.get_npc()
local Tux = FDrpg.get_tux()

return {
	EveryTime = function()
		if (guard_follow_tux) then
			Npc:says(_"I'm to keep an eye on you until you talk to Spencer.")
			Npc:set_name("Red Guard Escort")
		elseif (tux_has_joined_guard) then
			Npc:says(_"I'll let you be now; no need to suspect one of our own!")
		else
			Npc:says(_"Spencer seems to think that you are harmless. Go about your business.")
			if (Town_NorthGateGuard_tux_nickname_loon) then
				Npc:says(_"Babysitting a loon... Now I can say I've done it all!")
			end
		end
		show_if((not Tux:has_met("Spencer")), "node20")
		show("node99")
	end,

	{
		id = "node20",
		text = _"Where can I find Spencer?",
		code = function()
			if (not knows_spencer_office) then
				Npc:says(_"He's usually in his office in the citadel.")
				Npc:says(_"Head down the main corridor until you pass the citadel gates.")
				Npc:says(_"His office has purple walls, you won't miss it.")
				Tux:says(_"Okay.")
				knows_spencer_office = true
			else
				Npc:says_random(_"Come on, you know that already.",
								_"I'm pretty sure you know that...",
								_"We told you! Stop stalling!")
			end
			hide("node20")
		end,
	},
	{
		id = "node99",
		text = _"I'll be going then.",
		code = function()
			if (guard_follow_tux) then
				Npc:says_random(_"Not without me, you won't.",
								_"I'll be watching you, then.",
								_"Hey, wait up!")
			else
				Npc:says(_"Mhmmm.")
			end
			if (Town_NorthGateGuard_tux_nickname_loon) then
				--; TRANSLATORS: %s = Tux:get_player_name()
				Npc:says(_"%s... the Loon", Tux:get_player_name())
			end
			end_dialog()
		end,
	},
}
