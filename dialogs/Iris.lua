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
PERSONALITY = { "Independent", "Free Spirit", "Reclusive" },
MARKERS = { ITEMID1 = "Desk Lamp" },
BACKSTORY = "$$NAME$$ is a bot hunter living in the town. $$NAME$$ complains about the amount of light in her quarters
	 and will trade with Tux for a $$ITEMID1$$."
WIKI]]--

local Npc = FDrpg.get_npc()
local Tux = FDrpg.get_tux()

return {
	FirstTime = function()
		show("node0")
	end,

	EveryTime = function()
		if (Iris_wants_lamp) then
			if (Tux:has_item_backpack("Desk Lamp")) then
				if (Iris_deal) then
					Npc:says(_"Hey, remember our deal?")
					Npc:says(_"I'll trade you a book, for that lamp and 100 credits.")
					show("node7")
				else
					show("node5")
				end
			else
				Npc:says_random(_"Sure is dark around here.",
								_"I wish there was some decent lighting.")
			end
		end

		if (Iris_false_happy) and
		   (not guard_follow_tux) then
			Iris_false_happy = false
			show("node4")
		end
		show("node99")
	end,

	{
		id = "node0",
		text = _"Hello.",
		code = function()
			Npc:says_random(_"Hello",
							_"Hi there!")
			hide("node0") show("node1")
		end,
	},
	{
		id = "node1",
		text = _"Who are you?",
		code = function()
			Npc:says(_"I'm Iris")
			Npc:set_name("Iris")
			Tux:says(_"Hello Iris.")
			hide("node1") show("node2")
		end,
	},
	{
		id = "node2",
		text = _"What are you doing here?",
		code = function()
			Npc:says(_"I'm here just for vacations.")
			Tux:says(_"And what do you usually do?")
			Npc:says(_"Usually I hunt bots.") --she may be a spy of the rebel faction that is not yet implemented
			Npc:set_name("Iris - bot hunter")
			hide("node2") show("node3")
		end,
	},
	{
		id = "node3",
		text = _"Where do you come from?",
		code = function()
			Npc:says(_"Nowhere and everywhere.")
			Npc:says(_"As I already said, I walk around and kill bots.")
			hide("node3") show("node4")
		end,
	},
	{
		id = "node4",
		text = _"Do you like it here?",
		code = function()
			if (guard_follow_tux) then
				Iris_false_happy = true
				Npc:says(_"Yes, sure.")
				Npc:says(_"The Red Guard is very polite and keeps us safe from bots.")
				Npc:says(_"And the overall design of the town is quite clever.")
				Npc:says(_"Nevertheless this room is quite dark.")
				Npc:says(_"It could use some light.")
			else
				Npc:says(_"Not really.")
				Npc:says(_"The town is quite ugly and dark.")
				Npc:says(_"And there are very strange people here.")
				Tux:says(_"I know exactly what you are talking about...")
				Npc:says(_"I think this room could use some decoration.")
				Npc:says(_"It looks so cold and dark...")
			end
			if (not Iris_traded_lamp) then
				Iris_wants_lamp = true
				if (Tux:has_item_backpack("Desk Lamp")) then
					show("node5")
				else
					Npc:says(_"Perhaps what I need is a lamp.")
				end
			end
			hide("node4")
		end,
	},
	{
		id = "node5",
		text = _"Look, I have this Lamp.",
		code = function()
			Npc:says(_"Wow, nice.")
			hide("node5") show("node6")
		end,
	},
	{
		id = "node6",
		text = _"You can have it.",
		code = function()
			Npc:says(_"Oh great, thanks!")
			Npc:says(_"Hmm, what can I give you as thank-you...?")
			Npc:says(_"Oh, I have a nice book here.")
			Npc:says(_"But it's not worth the old dusty lamp.")
			Npc:says(_"I give it to you for the lamp and 100 circuits.")
			if (Tux:get_gold() > 99) then
				show("node7")
			else
				Tux:says(_"I don't have that much right now.")
				Tux:says(_"Let me get back to you about that.")
				end_dialog()
			end
			hide("node6")
			Iris_deal = true
		end,
	},
	{
		id = "node7",
		text = _"Sure, sounds like a fair trade.",
		code = function()
			if (Tux:del_gold(100)) then
				Tux:says(_"Take these 100 valuable circuits.")
				Npc:says(_"Ok, here it is. Take this book.")
				Npc:says(_"It's quite old and some pages are missing, but the main message is still clear.")
				Tux:says(_"Oh, thanks!")
				Tux:add_item("Source Book of Invisibility")
				Iris_wants_lamp = false
				Tux:del_item_backpack("Desk Lamp", 1)
				Iris_traded_lamp = true
			else
				Npc:says(_"Don't try to trick me, fat bird.")
				Npc:says(_"I know you can't afford it.")
				Npc:says(_"Eff off until you have the right sum!")
			end
			hide("node7")
		end,
	},
	{
		id = "node99",
		text = _"See you later.",
		code = function()
			if (Iris_wants_lamp) then
				Npc:says(_"Bye.")
			else
				Npc:says(_"In your dreams, Penguin!")
			end
			end_dialog()
		end,
	},
}
