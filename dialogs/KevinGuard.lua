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
		set_rush_tux(0)
		if (Kevin_entering_from_back) then
			end_dialog()
		elseif (KevinGuard_door_open) then
			tux_says(_"It's me.")
			change_obstacle_state("KevinsDoor", "opened")
			npc_says(_"[b]Identity confirmed. You may pass.[/b]")
			show("node99")
			end_dialog()
		else
			npc_says(_"[b]Intruder, lower your weapons and identify yourself or face immediate termination.[/b]")
			npc_says(_"[b]Any attempts to enter without authorization will be punished.[/b]")
			npc_says(_"[b]I will not let you injure my master.[/b]")
			hide("node99") show("node2", "node8")
		end
	end,

	{
		id = "node1",
		text = _"I must talk to your master.",
		code = function()
			npc_says(_"[b]State your reason.[/b]")
			hide("node1", "node3", "node17") show("node5", "node6", "node7")
		end,
	},
	{
		id = "node2",
		text = _"Don't shoot! I come in peace.",
		code = function()
			npc_says(_"[b]State the purpose of your presence here.[/b]")
			hide("node2", "node8") show("node1", "node3", "node17")
		end,
	},
	{
		id = "node3",
		text = _"I saw this building as I was walking by, and I decided to take a look inside.",
		code = function()
			npc_says(_"[b]Your reason is curiosity. Master Kevin likes curious people.[/b]")
			set_bot_name(_"614 - Kevin's Guard")
			npc_says(_"[b]You may pass.[/b]")
			hide("node1", "node3", "node17") show("node4")
		end,
	},
	{
		id = "node4",
		text = _"Thank you.",
		code = function()
			KevinGuard_door_open = true
			change_obstacle_state("KevinsDoor", "opened")
			hide("node4") show("node99")
		end,
	},
	{
		id = "node5",
		text = _"Umm... His life is in grave danger. ",
		code = function()
			npc_says(_"[b]Error: Reason invalid.[/b]")
			hide("node5") show("node12")
		end,
	},
	{
		id = "node6",
		text = _"The people from the uhh... town want me to have a word with him. Yeah, that's it.",
		code = function()
			npc_says(_"[b]Error: Reason invalid.[/b]")
			hide("node6")
		end,
	},
	{
		id = "node7",
		text = _"I have some very valuable information. Your master might be interested in it.",
		code = function()
			npc_says(_"[b]Master Kevin will be pleased. You may pass.[/b]")
			set_bot_name(_"614 - Kevin's Guard")
			npc_says(_"[b]Door opening sequence initiated.[/b]")
			hide("node5", "node6", "node7", "node12", "node13", "node14", "node18", "node19", "node99") show("node4")
		end,
	},
	{
		id = "node8",
		text = _"Step aside, you stupid tin can!",
		code = function()
			npc_says(_"[b]You shall not pass. Activating disintegrator beam in ten seconds.[/b]")
			npc_says(_"[b]Nine.[/b]")
			npc_says(_"[b]Eight.[/b]")
			hide("node2", "node8") show("node9", "node10")
		end,
	},
	{
		id = "node9",
		text = _"Wait, that's not what I meant!",
		code = function()
			npc_says(_"[b]Seven.[/b]")
			npc_says(_"[b]Six.[/b]")
			hide("node9", "node10") show("node99")
		end,
	},
	{
		id = "node10",
		text = _"I am not afraid of you.",
		code = function()
			npc_says(_"[b]Seven.[/b]")
			npc_says(_"[b]Six.[/b]")
			hide("node9", "node10") show("node11")
		end,
	},
	{
		id = "node11",
		text = _"Wait, that's not what I meant!",
		code = function()
			npc_says(_"[b]Five.[/b]")
			npc_says(_"[b]Four.[/b]")
			hide("node11") show("node99")
		end,
	},
	{
		id = "node12",
		text = _"Enemies are approaching from every side!",
		code = function()
			npc_says(_"[b]Error: Reason invalid.[/b]")
			hide("node12") show("node13")
		end,
	},
	{
		id = "node13",
		text = _"We are surrounded! They will attack any second now!",
		code = function()
			npc_says(_"[b]Error: Reason invalid.[/b]")
			hide("node13") show("node14")
		end,
	},
	{
		id = "node14",
		text = _"Are you listening to me?",
		code = function()
			npc_says(_"[b]Error: Reason invalid.[/b]")
			hide("node14") show("node18")
		end,
	},
	{
		id = "node17",
		text = _"I do not have to tell you anything, bot.",
		code = function()
			npc_says(_"[b]Engaging aggression circuits. You shall be disintegrated in ten seconds.[/b]")
			npc_says(_"[b]Nine.[/b]")
			npc_says(_"[b]Eight.[/b]")
			hide("node1", "node3", "node17") show("node9", "node10")
		end,
	},
	{
		id = "node18",
		text = _"I think I just found an error in your programming.",
		code = function()
			npc_says(_"[b]Error: Reason invalid.[/b]")
			hide("node18") show("node19")
		end,
	},
	{
		id = "node19",
		text = _"Thought so.",
		code = function()
			npc_says(_"[b]Error: Reason invalid.[/b]")
			hide("node19")
		end,
	},
	{
		id = "node99",
		text = _"See you later, tin can.",
		code = function()
			npc_says(_"[b]EOT acknowledged. Connection closed.[/b]")
			npc_says(_"[b]Goodbye.[/b]")
			end_dialog()
		end,
	},
}
