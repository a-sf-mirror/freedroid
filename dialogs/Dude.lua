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

local function dude_dialog_name(filename)
	return string.sub(filename, 1, -(string.len(".lua") + 1))
end

local function dude_create_node(dialogname)
	return {
		id = dialogname,
		text = dialogname,
		enabled = true,
		code = function()
			Tux:says(_"Starting " .. dialogname)
			start_chat(dialogname)
		end,
	}
end

local function dude_choose_action(this_dialog)
	local actionname = user_input_string("Please enter the dialog name of the desired NPC (Chandra, Bender...) " ..
	                                     "or other action to jump to (redguard, craft, upgrade, shop or exit). " ..
	                                     "If the input is invalid, the game will crash! To abort, enter " ..
	                                     "'continue' or press enter or escape.")
	if (not (actionname == "continue" or actionname == "")) then
		local actionnode = this_dialog:find_node(actionname)
		if (actionnode and actionnode.code) then
			actionnode:code(this_dialog)
		end
	end
end

return {
	FirstTime = function()
		level24obstacles()
		get_town_score()
		Dude_exit_node_count = 0
	end,

	EveryTime = function(this_node, this_dialog)
		npc_says(_"Hello.")
		if (tux_has_joined_guard) then
			npc_says(_"Red Guard membership: [b]true[/b]")
		else
			npc_says(_"Red Guard membership: [b]false[/b]")
		end
		npc_says(_"Here you'll be able to access all dialogs that are available ingame.", "NO_WAIT")
		npc_says(_"Take care, this may be a little buggy.", "NO_WAIT")
		npc_says(_"Don't do this if you are just normally playing.", "NO_WAIT")
		npc_says(_"These dialogs can currently be accessed:", "NO_WAIT")
		local node_list = ""
		for idx,node in ipairs(this_dialog.nodes) do
			if (node.id ~= "node0") then
				node_list = node_list .. node.id .. ", "
			end
		end
		npc_says(string.sub(node_list, 1, -2))

		dude_choose_action(this_dialog)
	end,

	{
		id = "node0",
		enabled = true,
		text = _"Show input field again.",
		code = function(this_node, this_dialog)
			dude_choose_action(this_dialog)
			hide("node0")
			show("node0")  -- done on purpose
		end,
	},
	
	{
		id = "redguard",
		enabled = true,
		text = _"Become Red Guard",
		code = function()
			tux_has_joined_guard = true
			npc_says(_"You are now a member of the Red Guard.")
		end,
	},

	{
		id = "craft",
		enabled = true,
		text = _"Craft addons",
		code = function()
			craft_addons()
		end,
	},

	{
		id = "upgrade",
		enabled = true,
		text = _"Upgrade items",
		code = function()
			upgrade_items()
		end,
	},

	{
		id = "shop",
		enabled = true,
		text = _"Shop",
		code = function()
			trade_with("Dude")
		end,
	},
	
	{
		generator = function()
			local nodes = {}
			local exclude = {
				-- "subdialogs" can not be run solely
				"614_sub.lua", "c-net-nethack_sub.lua",
				-- dialog of terminals can not be run, currently, because they have no associated 'bot'
				"Cryo-Terminal.lua", "c-net.lua", "DSB-MachineDeckControl.lua",
				"DSB-PowerControlGate1.lua", "HF-FirmwareUpdateServer.lua",
				"MO-HFGateAccessServer.lua", "MS-Factory-Addon-Terminal.lua",
				"Maintenance-Terminal.lua", "MiniFactory-Terminal.lua",
				"Terminal.lua", "TutorialTerminal.lua", "Vending-Machine.lua",
				"DSB-PowerControl.lua" }

			local dircontent = FDutils.system.scandir(FDdialog.dialogs_dir, ".*%.lua", exclude)
			if (dircontent) then
				for none,filename in ipairs(dircontent) do
					nodes[#nodes + 1] = dude_create_node(dude_dialog_name(filename))
				end
			end

			return nodes
		end
	},

	{
		id = "exit",
		enabled = true,
		text = function()
			return string.format( _"Exit this dialog for the %sth time", Dude_exit_node_count + 1)
		end,
		code = function()
			npc_says(_"Closing...")
			Dude_exit_node_count = Dude_exit_node_count + 1 -- do the computation of the var now
			end_dialog()
		end,
	},
}
