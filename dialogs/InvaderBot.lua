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
	EveryTime = function()
		npc_says(_"Target Acquired...","NO_WAIT")
		npc_says(_"Scanning...")
		npc_says(_"Non-Human Lifeform Identified","NO_WAIT")
		npc_says(_"Species Identified: Linarian")
		npc_says(_"Current Status: Unknown")
		show("node0", "node1", "node2")
	end,

	{
		id = "node0",
		text = _"Hello there.",
		code = function()
			next("node99")
		end,
	},
	{
		id = "node1",
		text = _"What's up?",
		code = function()
			next("node99")
		end,
	},
	{
		id = "node2",
		text = _"Die!",
		code = function()
			next("node99")
		end,
	},
	{
		id = "node99",
		text = "BUG, REPORT ME! invaderbot node99 -- END NODE",
		echo_text = false,
		code = function()
			play_sound("effects/bot_sounds/First_Contact_Sound_3.ogg")
			npc_says(_"Uploading Status...")
			set_npc_faction("ms")
			npc_says(_"Linarian is hostile. Destroy!")
			end_dialog()
		end,
	},
}
