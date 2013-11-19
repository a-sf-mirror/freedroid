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
				npc_says("You have won the arena.")
				next("choose")
			elseif (arena_left) then
				npc_says("You have left the arena.")
				next("choose")
			else
				npc_says("The arena is already started.")
			end
		else
			next("choose")
		end

		show("end")
	end,
	{
		id = "choose",
		text = _"BUG, REPORT ME! Francis choose",
		code = function()
			arena_current_level = nil
			arena_won = false
			arena_left = nil

			npc_says("Choose a level.")
			show("arena1", "arena2", "arena3")
		end,
	},
	{
		id = "arena1",
		text = _"Arena Level Novice",
		code = function()
			npc_says("Start Novice Arena")
			start_arena(1)
		end,
	},
	{
		id = "arena2",
		text = _"Arena Level Elite",
		code = function()
			npc_says("Start Elite Arena")
			start_arena(2)
		end,
	},
	{
		id = "arena3",
		text = _"Arena Level Champion",
		code = function()
			npc_says("Start Champion Arena")
			start_arena(3)
		end,
	},
	{
		id = "end",
		text = _"Exit",
		code = function()
			npc_says("Terminate")
			end_dialog()
		end,
	},
}
