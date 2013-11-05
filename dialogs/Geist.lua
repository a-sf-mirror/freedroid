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
		text = _"Hello. Any idea where I can get some help here?",
		code = function()
			npc_says(_"Verschwinde!") -- Vanish!
			hide("node0") show("node1", "node2")
		end,
	},
	{
		id = "node1",
		text = _"Erm... What do you mean?",
		code = function()
			npc_says(_"Ich bin der Geist, der stets verneint.") -- I am the ghost that always negates (?).
			npc_says(_"Und das mit Recht; denn alles, was entsteht,") -- For a reason; because everything that is created,
			npc_says(_"ist wert, dass es zugrunde geht.") -- is worth, to be ruined.
			hide("node1") show("node3")
		end,
	},
	{
		id = "node2",
		text = _"I really do not understand you.",
		code = function()
			npc_says(_"Wer sie nicht kennte, die Elemente,") -- The one who does not know them, the Elements,
			npc_says(_"ihre Kraft und Eigenschaft,") -- their power and their feature/characteristic,
			npc_says(_"waere kein Meister ueber die Geister.") -- would not be a master of the ghosts.
			hide("node2") show("node3")
		end,
	},
	{
		id = "node3",
		text = _"I wish I knew what you are talking about.",
		code = function()
			npc_says(_"Verschwinde!") -- Vanish!
			hide("node3")
			end_dialog()
		end,
	},
	{
		id = "node99",
		text = _"I need to go now.",
		code = function()
			npc_says(_" . . .")
			end_dialog()
		end,
	},
}