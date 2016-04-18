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
PERSONALITY = { "Militaristic", "Helpful" },
PURPOSE = "$$NAME$$ will sell Tux more capable weapons and armor after Tux has joined the Red Guard.",
BACKSTORY = "$$NAME$$ is the Red Guard\'s Quartermaster/Storesman",
WIKI]]--

local Npc = FDrpg.get_npc()
local Tux = FDrpg.get_tux()

return {
	FirstTime = function()
		show("node0")
	end,

	EveryTime = function()
		show("node99")
	end,

	{
		id = "node0",
		text = _"Hi! I'm new here.",
		code = function()
			Npc:says(_"A newcomer! Great! We can always use more people. Welcome to the Red Guard! I'm Lukas. I'm in charge of our little armory here.")
			hide("node0") show("node1", "node2")
		end,
	},
	{
		id = "node1",
		text = _"Spencer said you'd be able to provide me with better armor and equipment.",
		code = function()
			Npc:says(_"Of course. As a member of our Guard, you're entitled to wear one of our suits of armor.")
			Npc:says(_"There are two kinds or armor, a heavy Red Guard robe and a light version of the same thing, though we also trade guns and some ammo.")
			Npc:says_random(_"Now, what will it be?",
							_"So, what do you want to buy?")
			trade_with("Lukas")
			show("node3")
		end,
	},
	{
		id = "node2",
		text = _"What do you do all day here at the armory?",
		code = function()
			Npc:says(_"Oh, there's always something to do. Don't worry. I'm pretty occupied keeping the place in order.")
			hide("node2")
		end,
	},
	{
		id = "node3",
		text = _"I'd like to buy an exterminator.",
		code = function()
			Npc:says(_"Haha, good one.")
			Npc:says(_"Sorry, they are not for sale.")
			Tux:says(_"I'll get one for free?")
			Npc:says(_"You are the funniest Linarian I've ever seen.")
			Npc:says(_"If the only one.")
			Npc:says(_"Now go, kill some bots.")
			hide("node3")
		end,
	},
	{
		id = "node99",
		text = _"See you later!",
		code = function()
			end_dialog()
		end,
	},
}
