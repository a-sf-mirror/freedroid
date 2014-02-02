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
		play_sound("effects/Menu_Item_Deselected_Sound_0.ogg")
		terminal = "dixon@autofactory: ~ # "

		cli_says(_"Login : ", "NO_WAIT")
		-- ; TRANSLATORS: dixon = name of the npc, uncapitalized
		Tux:says(_"dixon", "NO_WAIT")
		cli_says(_"Password : ", "NO_WAIT")
		Tux:says("*******", "NO_WAIT")
		npc_says(_"Hello, Dave.")
		npc_says(_"Last login from /dev/ttyS0 on Sun, 3 dec 2056.", "NO_WAIT")
		cli_says(terminal, "NO_WAIT")
		show("node0", "node99")
	end,

	{
		id = "node0",
		text = _"autofactory start",
		code = function()
			npc_says(_"I'm sorry. I'm afraid I can't let you do that.")
			cli_says(terminal, "NO_WAIT")
			hide("node0") show("node1")
		end,
	},
	{
		id = "node1",
		text = _"sudo autofactory start",
		code = function()
			npc_says(_"Commencing Initialization Process")
			npc_says(_"Activating Production Line...", "NO_WAIT")
			npc_says(_"SUCCESS")
			npc_says(_"Configuring Fabrication Data...", "NO_WAIT")
			npc_says(_"SUCCESS")
			npc_says(_"Testing Assembly Hardware...", "NO_WAIT")
			if (not Lvl6_elbow_grease_applied) then
				npc_says(_"DECLINE")
				npc_says(_"Error code: 0x6465636c6365")
				npc_says(_"Please read the manual and follow the instructions.")
				cli_says(_"WARNING: ", "NO_WAIT")
				npc_says(_"The manual is outdated. Please refer to the fixed directive 33 in the reference book.")
				cli_says(_"WARNING: ", "NO_WAIT")
				npc_says(_"The reference book is outdated. Please refer to the ticket 2012 in the technician database.")
				cli_says(_"WARNING: ", "NO_WAIT")
				npc_says(_"The ticket 2012 is outdated. Please refer to the manual.")
				npc_says(_"Canceling Initialization Process...")
				npc_says(_"Initialization has been canceled to prevent any damage.")
				MiniFactory_init_failed = true
			else
				npc_says(_"SUCCESS")
				npc_says(_"Final Initialization Instructions...")
				npc_says(_"The Automated Factory is booted up and ready to work.", "NO_WAIT")
				if (Maintenace_Terminal_want_sandwich) then
					cli_says(_"WARNING:", "NO_WAIT")
					npc_says(_"The Automated Factory is unable to make a sandwich.")
				end
				Minifactory_online = true
				MiniFactory_init_failed = false
				hide("node1") show("node10", "node20")
			end
			cli_says(terminal, "NO_WAIT")
		end,
	},
	{
		id = "node10",
		text = _"craft --addon",
		code = function()
			craft_addons()
			cli_says(terminal, "NO_WAIT")
		end,
	},
	{
		id = "node20",
		text = _"assemble --item --addon",
		code = function()
			upgrade_items()
			cli_says(terminal, "NO_WAIT")
		end,
	},
	{
		id = "node99",
		text = _"logout",
		code = function()
			npc_says(_"Exiting", "NO_WAIT")
			npc_says(_"Goodbye Dave.")
			hide("node10", "node20")
			play_sound("effects/Menu_Item_Selected_Sound_1.ogg")
			end_dialog()
		end,
	},
}
