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
		tux_says(_"Hello!")
		npc_says(_"Bonjour!")
		tux_says(_"I'm %s, and who are you?", get_player_name())
		npc_says(_"My name is Arthur.")
		set_bot_name(_"Arthur")
		show("node1")
	end,

	EveryTime = function()
		if (Arthur_node_1 == "node2") then
			show("node2")
		elseif (Arthur_node_1 == "node3") then
			show("node3")
		end

		show("node99")
	end,

	{
		id = "node1",
		text = _"What are you doing here?",
		code = function()
			npc_says(_"I'm programming.")
			tux_says(_"Oh, interesting.")
			tux_says(_"And, mmh, what are working on?")
			npc_says(_"My current project is a role playing game.")
			npc_says(_"The main character will be a penguin which, well, has to save the world basically.")
			tux_says(_"Oh, interesting! Tell me when you have it ready, I'd love to take a look at it.")
			npc_says(_"It's going to be ready when it's going to be ready.")
			tux_says(_"Sure, take your time.")
			set_bot_name(_"Arthur - Game developer")
			hide("node1")
			Arthur_node_1 = "node2"
		end,
	},
	{
		id = "node2",
		text = _"Hi Arthur, how is your game going?",
		code = function()
			tux_says(_"Do you mind sharing any more details?")
			npc_says(_"Hmm, there are still some parts to be finished.")
			npc_says(_"Do you think there should be some kind of zombie apocalypse which the player will have to deal with?")
			tux_says(_"Of course! Sounds cool!")
			Arthur_node_1 = "node3"
			hide("node2")
		end,
	},
	{
		id = "node3",
		text = _"Hey, is the your game finished already?",
		code = function()
			npc_says(_"I'm doing the final polishing right now.")
			tux_says(_"Ah ok.")
			hide("node3")
		end,
	},
	{
		id = "node99",
		text = _"I have to go now.",
		code = function()
			npc_says_random(_"See you later.",
				_"Au revoir.")
			end_dialog()
		end,
	},
}
