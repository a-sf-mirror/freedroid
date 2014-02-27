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
		Jasmine_count = 0
		show("node0")
	end,

	EveryTime = function()
		show("node99")
	end,

	{
		id = "node0",
		text = _"Hi! I'm new here.",
		code = function()
			npc_says(_"Yes. Hello. What do you want?")
			hide("node0") show("node1", "node8")
		end,
	},
	{
		id = "node1",
		text = _"Who are you?",
		code = function()
			npc_says(_"People call me Jasmine...")
			npc_says(_"That is not my real name, but it will do for now.")
			set_bot_name("Jasmine")
			hide("node1") show("node2", "node3", "node4", "node5")
		end,
	},
	{
		id = "node2",
		text = _"What are you doing in Kevin's place?",
		code = function()
			npc_says(_"I live with him, stupid.")
			npc_says(_"I like him. He... He is very much like me.")
			set_bot_name("Jasmine - Girlfriend of a true Hacker")
			hide("node2")
		end,
	},
	{
		id = "node3",
		text = _"How are you?",
		code = function()
			npc_says(_"Annoyed. Now leave me alone.")
			hide("node3")
		end,
	},
	{
		id = "node4",
		text = _"Why does such a nice lady act so harshly? Can't we all just get along?",
		code = function()
			npc_says(_"No.")
			hide("node4")
		end,
	},
	{
		id = "node5",
		text = _"What are you up to?",
		code = function()
			npc_says(_"Nothing. Quit asking questions.")
			npc_says(_"Don't bore me.")
			hide("node5")
		end,
	},
	{
		id = "node8",
		text = _"You are the most beautiful human I have ever seen.",
		code = function()
			npc_says(_"And you are at least a quarter wrong.")
			Tux:says(_"You're so cute, baby seals and polar bears sent each other pictures of you.")
			npc_says(_"Now, please get out of my sight.")
			hide("node8") show("node9")
		end,
	},
	{
		id = "node9",
		text = _"Will you marry me?",
		code = function()
			npc_says(_"Forget it.")
			Tux:says(_"Roses are red,")
			Tux:says(_"Violets are blue,")
			Tux:says(_"All of my base,")
			Tux:says(_"Are belong to you.")
			npc_says(_"...")
			hide("node9")
		end,
	},
	{
		id = "node99",
		text = _"See you later.",
		code = function()
			if (Jasmine_count < 2) then
				Jasmine_count = Jasmine_count + 1
				npc_says(_"Yeah. Whatever.")
				end_dialog()
			else
				npc_says(_"Yeah. Wha -- SIGSEGV.")
				npc_says(_"Segmentation fault.")
				Tux:says(_"Huh?")
				npc_says(_"Traceback printed to stdout.")
				npc_says(_"Kernel panic in module: Jasmine")
				npc_says(_"Buffer overrun in anger.c:1532")
				npc_says(_"Core dumped. System halted.")
				Jasmine_count = 0
				drop_dead()
				Jasmine_sigsegv = true
			end
		end,
	},
}
