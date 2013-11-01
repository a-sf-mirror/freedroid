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
	FirstTime = function()
		show("node0")
	end,

	EveryTime = function()
		if (has_met("Peter")) then
			tux_says_random(_"Hello.",
				_"Hi there.", "NO_WAIT")
			npc_says_random(_"Well, hello again.",
				_"Hello hello.",
				_"Welcome back.")
		end
		show("node99")
	end,

	{
		id = "node0",
		text = _"Why are you here?",
		code = function()
			npc_says(_"I'm a political prisoner.")
			npc_says(_"Those red fascists put me here.")
			npc_says(_"And I guess you are a fascist too, as only Red Guard members are allowed in here.")
			hide("node0") show("node1")
		end,
	},
	{
		id = "node1",
		text = _"What is a fascist?",
		code = function()
			tux_says(_"What is ...", "NO_WAIT")
			npc_says(_"LALALALALA - I'm not hearing you. I have my fingers in my ears and just go - LALALALALA")
		end,
	},
	{
		id = "node99",
		text = _"Talk to you later.",
		code = function()
			npc_says(_"Not in your life, Fascist!")
			end_dialog()
		end,
	},
}
