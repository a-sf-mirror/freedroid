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
		if (not MO_HFGateAccessServer_skip_captcha) then
			-- Are we human? CAPTCHA!!!
			number_one=math.random(2,7)
			number_two=math.random(1,number_one-1)
			captcha = number_one - number_two
			if (captcha == 1) then
				captcha = _"one"
			elseif (captcha == 2) then
				captcha = _"two"
			elseif (captcha == 3) then
				captcha = _"three"
			elseif (captcha == 4) then
				captcha = _"four"
			elseif (captcha == 5) then
				captcha = _"five"
			elseif (captcha == 6) then
				captcha = _"six"
			end
			response = user_input_string(string.format(_"CAPTCHA: Please write the lowercase word that answers the following: %d - %d = ?", number_one, number_two))
		else
			MO_HFGateAccessServer_skip_captcha = false
		end
		if (captcha ~= response) then
			npc_says(_"Non-human detected. Administering paralyzing shock.")
			npc_says(_"NOTE: If you are a human, try again, and make sure you enter a word and not digits.")
			freeze_tux_npc(7)
			Tux:hurt(20)
			Tux:heat(20)
			play_sound("effects/Menu_Item_Selected_Sound_1.ogg")
			end_dialog()
		else
			npc_says(_"Welcome to MS gate access server for region #54648.")
			if (not MO_HFGateAccessServer_Spencer_chat) then
				Tux:says(_"WHAT?!")
				Tux:update_quest("Propagating a faulty firmware update", _"The firmware server seems to actually be an access server to a gate. What am I supposed to do now?")
				MO_HFGateAccessServer_Spencer = true
				MO_HFGateAccessServer_Spencer_chat = true
				MO_HFGateAccessServer_skip_captcha = true
				start_chat("Spencer")
				play_sound("effects/Menu_Item_Selected_Sound_1.ogg")
				end_dialog()
			end
			if (MO_HFGateAccessServer_Spencer) then
				MO_HFGateAccessServer_Spencer = false
				play_sound("effects/Menu_Item_Selected_Sound_1.ogg")
				end_dialog()
			else
				npc_says(_"Please select action")
			end
			show("node1", "node99")
		end
	end,

	{
		id = "node1",
		text = _"status",
		echo_text = false,
		code = function()
			Tux:says(_"status", "NO_WAIT")
			if (cmp_obstacle_state("HF-Gate-outer", "opened")) then
				npc_says(_"Gate 1 status: OPENED", "NO_WAIT")
			else
				npc_says(_"Gate 1 status: CLOSED", "NO_WAIT")
				if (not MO_HFGateAccessServer_hacked) then
					show("node2")
				end
			end

			if (cmp_obstacle_state("HF-Gate-inner", "opened")) then
				npc_says(_"Gate 2 status: OPENED", "NO_WAIT")
			else
				npc_says(_"Gate 2 status: CLOSED", "NO_WAIT")
				if (not MO_HFGateAccessServer_hacked) then
					show("node2")
				end
			end

			if (cmp_obstacle_state("HF-Gate-inner", "opened")) and
			(cmp_obstacle_state("HF-Gate-outer", "opened")) then
			end
		end,
	},
	{
		id = "node2",
		text = _"open gate",
		echo_text = false,
		code = function()
			Tux:says(_"open gate", "NO_WAIT")
			npc_says(_"Permission denied")
			Tux:hurt(5)
			Tux:heat(10)
			Tux:update_quest("Open Sesame", "The server is secured, looks like I have to hack it.")
			hide("node2") show("node3")
			play_sound("effects/Menu_Item_Selected_Sound_1.ogg")
			end_dialog()
		end,
	},
	{
		id = "node3",
		text = _"(Try hacking the server)",
		code = function()
			if (takeover(get_program("Hacking")+3)) then
				Tux:says(_"sudo !!")
				npc_says(_"sudo open gates", "NO_WAIT")
				npc_says(_"Which gates do you want to open?")
				Tux:update_quest("Open Sesame", "Whew, I finally managed to hack the gate access server. I can open the gates now.")
				MO_HFGateAccessServer_hacked = true
				hide("node3") show("node4")
			else
				npc_says(_"Permission denied.")
				Tux:heat(15)
				Tux:hurt(10)
				play_sound("effects/Menu_Item_Selected_Sound_1.ogg")
				end_dialog()
			end
		end,
	},
	{
		id = "node4",
		text = _"man open gates",
		echo_text = false,
		code = function()
			Tux:says(_"man open gates", "NO_WAIT")
			npc_says(_"NAME", "NO_WAIT")
			npc_says(_" open gates -- opens gates via console", "NO_WAIT")
			npc_says(" ")
			npc_says(_"SYNOPSIS", "NO_WAIT")
			npc_says(_" open gates --inner --outer", "NO_WAIT")
			npc_says(_" ")
			npc_says(_"DESCRIPTION", "NO_WAIT")
			npc_says(_" Opens gates via console. Awesome, isn't it?", "NO_WAIT")
			npc_says(" ")
			npc_says(_"OPTIONS", "NO_WAIT")
			npc_says(_" --inner", "NO_WAIT")
			npc_says(_" Opens the inner gate.", "NO_WAIT")
			npc_says(" ", "NO_WAIT")
			npc_says(_" --outer", "NO_WAIT")
			npc_says(_" Opens the outer gate.", "NO_WAIT")
			npc_says(" ")
			npc_says(_"SEE ALSO", "NO_WAIT")
			npc_says(_" These are not the gates you are looking for.", "NO_WAIT")
			npc_says(" ")
			npc_says(_"AUTHOR", "NO_WAIT")
			npc_says(_" The Hell Fortress GateAccessServer Manual Writer", "NO_WAIT")
			npc_says(" ")
			npc_says(_" February 22, 1992")
			hide("node4") show("node5", "node6", "node7")
		end,
	},
	{
		id = "node5",
		text = _"open gates --inner",
		echo_text = false,
		code = function()
			Tux:says(_"open gates --inner", "NO_WAIT")
			npc_says(_"inner gate opened", "NO_WAIT")
			npc_says(_"[b]WARNING[/b]:", "NO_WAIT")
			npc_says(_"Anomalies detected!")
			change_obstacle_state("HF-Gate-inner", "opened")
			if (cmp_obstacle_state("HF-Gate-outer", "opened")) then
				Tux:update_quest("Open Sesame", "I think I managed to open the gates to the Hell Fortress. But where can I find them?")
				hide("node7")
			end
			hide("node5")
		end,
	},
	{
		id = "node6",
		text = _"open gates --outer",
		echo_text = false,
		code = function()
			Tux:says(_"open gates --outer", "NO_WAIT")
			npc_says(_"outer gate opened", "NO_WAIT")
			npc_says(_"[b]WARNING[/b]:", "NO_WAIT")
			npc_says(_"Anomalies detected!")
			change_obstacle_state("HF-Gate-outer", "opened")
			if (cmp_obstacle_state("HF-Gate-inner", "opened")) then
				Tux:update_quest("Open Sesame", "I think I managed to open the gates to Hell Fortress. But where can I find them?")
				hide("node7")
			end
			hide("node6")
		end,
	},
	{
		id = "node7",
		text = _"open gates --inner --outer",
		echo_text = false,
		code = function()
			Tux:says(_"open gates --inner --outer", "NO_WAIT")
			npc_says(_"inner gate opened", "NO_WAIT")
			npc_says(_"outer gate opened", "NO_WAIT")
			npc_says(_"[b]WARNING[/b]:", "NO_WAIT")
			npc_says(_"Anomalies detected!")
			change_obstacle_state("HF-Gate-inner", "opened")
			change_obstacle_state("HF-Gate-outer", "opened")
			Tux:update_quest("Open Sesame", "I think I managed to open the gates to Hell Fortress. But where can I find them?")
			hide("node5", "node6", "node7")
		end,
	},
	{
		id = "node99",
		text = _"logout",
		echo_text = false,
		code = function()
			Tux:says(_"logout")
			npc_says(_"exiting...")
			play_sound("effects/Menu_Item_Selected_Sound_1.ogg")
			end_dialog()
		end,
	},
}
