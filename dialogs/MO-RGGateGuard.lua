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
		show("node99")
	end,

	{
		id = "node0",
		text = _"Hi! I'm new here.",
		code = function()
			npc_says(_"Great. Now back to where you came from. This is no place for you.")
			hide("node0") show("node1")
		end,
	},
	{
		id = "node1",
		text = _"What's the matter with this gate?",
		code = function()
			npc_says(_"This is a gate to an automated bot factory we call the Hell Fortress.")
			npc_says(_"You must be crazy if you want to get in there. That place is full of bots.")
			npc_says(_"Get out of here. They might attack at any moment!")
			hide("node1") show("node2")
		end,
	},
	{
		id = "node2",
		text = _"What if I just walk in there? Will you try to stop me?",
		code = function()
			npc_says(_"Of course not. Murdering you would be a complete waste of time.")
			npc_says(_"The bots and disruptor shield will take care of that for me.")
			npc_says(_"Just remember to give me your valuables before you commit suicide.")
			npc_says(_"You won't need any money once you're dead, and I could use it, eh?")
			hide("node2") show("node3")
		end,
	},
	{
		id = "node3",
		text = _"Thanks for the advice.",
		code = function()
			npc_says(_"You're dead if you don't take it.")
			hide("node3")
		end,
	},
	{
		id = "node99",
		text = _"I'll be going then.",
		code = function()
			npc_says_random(_"Finally.",
				_"Don't come back.",
				_"It is dangerous here.",
				_"Leave and stay away.")
			show("node1")
			end_dialog()
		end,
	},
}
