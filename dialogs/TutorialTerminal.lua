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
		play_sound("effects/Menu_Item_Deselected_Sound_0.ogg")
		TutorialTerminal_year = os.date("%Y") + 45 -- current year + 45
		TutorialTerminal_date_1 = os.date("%a %b %d %H:%M:%S") -- emulate os.date() but without the year
		TutorialTerminal_prompt = "guest@example.com: ~ #"

		cli_says("Login : ", "NO_WAIT")
		tux_says(_"guest", "NO_WAIT")
		cli_says(_"Entering as Guest", "NO_WAIT")
		npc_says("", "NO_WAIT")
		if (TutorialTerminal_date == nil) then
			npc_says(_"First login from /dev/ttySO on %s %d", TutorialTerminal_date_1, TutorialTerminal_year, "NO_WAIT")
		else
			npc_says(_"Last login from /dev/ttyS0 on %s %d", TutorialTerminal_date, TutorialTerminal_year, "NO_WAIT")
		end
		TutorialTerminal_date = TutorialTerminal_date_1

		if (cmp_obstacle_state("TutorialDoor", "closed")) then
			npc_says(_"Gate status: CLOSED", "NO_WAIT")
			show("node0")
		elseif (cmp_obstacle_state("TutorialDoor", "opened")) then
			npc_says(_"Gate status: OPEN", "NO_WAIT")
			show("node10")
		else
			npc_says(_"GAME BUG. PLEASE REPORT, TUTORIAL-TERMINAL EveryTime LuaCode")
		end
		cli_says(TutorialTerminal_prompt, "NO_WAIT")
		show("node99")
	end,

	{
		id = "node0",
		text = _"open gate",
		echo_text = false,
		code = function()
			tux_says(_"open gate", "NO_WAIT")
			npc_says(_"Access granted. Opening gate ...")
			npc_says(_"Gate status: OPEN")
			change_obstacle_state("TutorialDoor", "opened")
			cli_says(TutorialTerminal_prompt, "NO_WAIT")
			hide("node0") show("node10")
		end,
	},
	{
		id = "node10",
		text = _"close gate",
		echo_text = false,
		code = function()
			tux_says(_"close gate", "NO_WAIT")
			npc_says(_"Access granted. Closing gate ...")
			npc_says(_"Gate status: CLOSED")
			change_obstacle_state("TutorialDoor", "closed")
			cli_says(TutorialTerminal_prompt, "NO_WAIT")
			hide("node10") show("node0")
		end,
	},
	{
		id = "node99",
		text = _"logout",
		echo_text = false,
		code = function()
			tux_says(_"logout", "NO_WAIT")
			npc_says(_"Exiting...")
			play_sound("effects/Menu_Item_Selected_Sound_1.ogg")
			if (cmp_obstacle_state("TutorialDoor", "opened")) and
			   (TutorialTom_start_chat == nil) then
					TutorialTom_start_chat = true
					start_chat("TutorialTom")
			end
			end_dialog()
		end,
	},
}
