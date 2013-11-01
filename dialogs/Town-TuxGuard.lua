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

return {
	EveryTime = function()
		if (guard_follow_tux) then
			npc_says(_"I'm to keep an eye on you until you talk to Spencer.")
			set_bot_name(_"Red Guard Escort")
		elseif (tux_has_joined_guard) then
			npc_says(_"I'll let you be now; no need to suspect one of our own!")
		else
			npc_says(_"Spencer seems to think that you are harmless. Go about your business.")
			if (Town_NorthGateGuard_tux_nickname_loon) then
				npc_says(_"Babysitting a loon... Now I can say I've done it all!")
			else end
		end
		show_node_if((not has_met("Spencer")), "node20")
		show("node99")
	end,

	{
		id = "node20",
		text = _"Where can I find Spencer?",
		code = function()
			if (not knows_spencer_office) then
				npc_says(_"He's usually in his office in the citadel.")
				npc_says(_"Head down the main corridor until you pass the citadel gates.")
				npc_says(_"His office has purple walls, you won't miss it.")
				tux_says(_"Okay.")
				knows_spencer_office = true
			else
				npc_says_random(_"Come on, you know that already.",
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
				npc_says_random(_"Not without me, you won't.",
					_"I'll be watching you, then.",
					_"Hey, wait up!")
			else
				npc_says(_"Mhmmm.")
			end
			if (Town_NorthGateGuard_tux_nickname_loon) then
				npc_says(_"%s... the Loon", get_player_name())
			end
			end_dialog()
		end,
	},
}
