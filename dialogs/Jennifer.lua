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

local Tux = FDrpg.get_tux()

return {
	FirstTime = function()
		show("node1")
	end,

	EveryTime = function()
		if (Tux:has_met("Jennifer")) then
			Tux:says(_"Hello")
			npc_says(_"Welcome back.")
		end

		if (Tux:has_item("Toolbox")) then
			show("node10")
		end
		show("node99")
	end,

	{
		id = "node1",
		text = _"Hello!",
		code = function()
			npc_says(_"Hello. I'm Jennifer.")
			Tux:says(_"I'm %s.", Tux:get_player_name())
			Tux:says(_"What are you doing here?")
			npc_says(_"I'm a machinist, working here, repairing bots and stuff like this.")
			hide("node1") show("node2", "node3")
		end,
	},
	{
		id = "node2",
		text = _"You don't look very busy.",
		code = function()
			npc_says(_"Well, yes.")
			npc_says(_"Someone thought he had to tidy up the space...", NO_WAIT)
			npc_says(_"...and now I cannot find my equipment anymore, what an idiot!")
			npc_says(_"I'm especially searching for my toolbox.")
			Tux:says(_"Okay, I will keep my eyes open if I see some toolbox.")
			npc_says(_"Thank you.")
			Tux:add_quest("Jennifer's Toolbox", "Someone rearranged equipment at Jennifers workshop, she does not find her Toolbox anymore. I promised to keep my eyes open for it.")
			hide("node2")
		end,
	},
	{
		id = "node3",
		text = _"You are repairing these bots?!",
		code = function()
			npc_says(_"Uhm yes, that's my job here.")
			npc_says(_"...")
			hide("node3") show("node4")
		end,
	},
	{
		id = "node4",
		text = _"Why are you still alive anyway?",
		code = function()
			Tux:says(_"The bots should have killed you immediately.")
			Tux:says(_"It's hell in here for me and you don't seem to be even armed.")
			npc_says(_"Well...")
			npc_says(_"The workers here get special chips implanted.", NO_WAIT)
			Tux:says(_"Ew.", NO_WAIT)
			npc_says(_"So the bots can identify us as 'friendly' and won't rip us apart.")
			Tux:says(_"Uh.")
			Tux:says(_"Is there any way for me to get such a chip?")
			npc_says(_"I don't think you will be able to get one. You don't look very human-like.")
			npc_says(_"How did you get in here anyway?")
			hide("node4") show("node5", "node6", "node7")
		end,
	},
	{
		id = "node5",
		text = _"That's a long story...",
		code = function()
			npc_says(_"Mh well ok.")
			hide("node5", "node6", "node7")
		end,
	},
	{
		id = "node6",
		text = _"I walked in through the front door.",
		code = function()
			npc_says(_"Heh. I see.")
			hide("node5", "node6", "node7")
		end,
	},
	{
		id = "node7",
		text = _"This is none of your business.",
		code = function()
			npc_says(_"If you say so...")
			hide("node5", "node6", "node7")
		end,
	},
	{
		id = "node10",
		text = _"I think this is your toolbox?",
		code = function()
			npc_says(_"Oh yes, it is!", NO_WAIT)
			npc_says(_"Thank you very much!")
			Tux:end_quest("Jennifer's Toolbox", _"I found the Toolbox and returned it to her") -- we may want to have her repair all of tux' items at some point
			Tux:del_item("Toolbox", 1)
			hide("node10")
		end,
	},
	{
		id = "node99",
		text = _"See you later.",
		code = function()
			npc_says_random(_"See you later.",
				_"Bye.")
			end_dialog()
		end,
	},
}
