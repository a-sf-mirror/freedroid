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
PERSONALITY = { "Robotic" },
PURPOSE = "$$NAME$$ controls arena functions in the game.",
BACKSTORY = "$$NAME$$ is the Arena Master for the Red Guard.",
WIKI]]--

local Npc = FDrpg.get_npc()
local Tux = FDrpg.get_tux()

local function start_arena(level) 
	arena_current_level = level
	arena_next_wave = 1

	change_obstacle_state("Arena-ChangingRoomDoor", "opened")
	change_obstacle_state("Arena-RingEntrance", "opened")

	hide("arena1", "arena2", "arena3")
end

return {
	EveryTime = function()
		if (arena_current_level ~= nil) then
			if (arena_won) then
				Npc:says(_"You have won the arena.")
				if (not done_quest("Novice Arena")) then
					--change_obstacle_state("NoviceArenaExitDoor", "opened")
					display_console_message(_"Novice arena cleared!")
					display_big_message(_"Level cleared!")
					Tux:end_quest("Novice Arena", _"I won the fight in the novice arena.")
				end
				next("choose")
			elseif (arena_left) then
				Npc:says(_"You have left the arena.")
				next("choose")
			else
				Npc:says(_"The arena is already started.")
			end
		else
			next("choose")
		end

		show("end")
	end,
	{
		id = "choose",
		text = _"BUG, REPORT ME! Mike node 'choose'",
		echo_text = false,
		code = function()
			arena_current_level = nil
			arena_won = false
			arena_left = nil

			Npc:says(_"Choose a level.")
			show("arena1", "arena2", "arena3")
		end,
	},
	{
		id = "arena1",
		text = _"Arena Level Novice",
		code = function()
			Npc:says(_"Start Novice Arena")
			start_arena(1)
		end,
	},
	{
		id = "arena2",
		text = _"Arena Level Elite",
		code = function()
			Npc:says(_"Start Elite Arena")
			start_arena(2)
		end,
	},
	{
		id = "arena3",
		text = _"Arena Level Champion",
		code = function()
			Npc:says(_"Start Champion Arena")
			start_arena(3)
		end,
	},
	{
		id = "end",
		text = _"Exit",
		echo_text = false,
		code = function()
			Npc:says(_"Terminate")
			end_dialog()
		end,
	},
}
