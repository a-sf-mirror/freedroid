--[[

	Copyright (c) 2014 Scott Furry

	This file is part of Freedroid

	Freedroid is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.

	Freedroid is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with Freedroid; see the file COPYING. If not, write to the
	Free Software Foundation, Inc., 59 Temple Place, Suite 330, Boston,
	MA  02111-1307  USA

]]--
--	lua module for parsing FDRPG character dialog
local modWP_Dialog = {}
--	modWPCommon reference
modWP_Dialog.modcommon = {}
--	Variable will contained all parsed npc dialog information after completion of ProcessData()
--	storage: each entry is a table w/ format: {dialog = dialogname, data = modWP_Dialog.dataItem}
modWP_Dialog.AllDialogData = {}
--	dialog file parsed line-by-line into table
--	storage: each entry is a table w/ format: {dialog = dialogname, data = filedata}
modWP_Dialog.DialogFileData = {}
--	"struct" of dialog infromation for each character
--	use deep copy to pass default values to new copy
modWP_Dialog.dataItem = {
	alias = {},
	quest_given = {},
	quest_update = {},
	quest_end = {},
	skills = {},
	programs = {},
	wikihead = {
		PERSONALITY = {},
		MARKERS = {},
		PURPOSE = {},
		BACKSTORY = {},
		RELATIONSHIP = {
			{ actor = "", text = ""	},
		},
	}
}
--	text patterns used to locate wiki header data in dialog file
modWP_Dialog.wikiheadmarker = "WIKI"
-- text used to name "code chunk" extracted from dialog file (wiki header)
-- NOTE: text must be the same as function below
modWP_Dialog.codemarker = "wikiheader"
--	text patterns used to locate data in dialog file
modWP_Dialog.patterns_dialog = {
	botname = "Npc:set_name",
	quest_given = "Tux:add_quest",
	quest_update = "Tux:update_quest",
	quest_end = "Tux:end_quest",
	improveprog = "Tux:improve_program",
	improveSkill = "Tux:improve_skill",
	trainskill = "Tux:train_skill"
}
--	variable holder id of dialog file being updated.
--	referenced in fn wkihead to direct storage of wikihead data
modWP_Dialog.lastdialog = ""

--	Function has to be in "global" space to parse wiki header data and is called "automagically"
--	[in] data	wiki header information to be parsed.
function wikiheader( data )
	if ( type(data) ~= 'table' ) then
		io.stderr:write("error - dialog: " .. modWP_Dialog.lastdialog .. ". Can not execute wiki header - Not a table.\n")
		os.exit(1)
	end
	local dlgIndex = select(2,modWP_Dialog.modcommon.Extract.GetTableItem( modWP_Dialog.AllDialogData, "dialog" , modWP_Dialog.lastdialog))
	if (( dlgIndex ~= nil ) and ( dlgIndex > 0 )) then
		modWP_Dialog.AllDialogData[dlgIndex].data.wikihead = modWP_Dialog.modcommon.Extract.TblDeepCopy(data)
	end
end

--	Locate and read in dialog file for each character.
--	Find functions named in patterns_dialog and save data
function modWP_Dialog.ParseDialog( npcDlgName )
	if (( npcDlgName == nil )
	or ( type(npcDlgName) ~= 'string' )
	or ( npcDlgName:len() < 1 )) then
		return false
	end
	-- get reference to dialog data to be processed
	local dialogItem = select(1,modWP_Dialog.modcommon.Extract.GetTableItem(modWP_Dialog.DialogFileData, "dialog", npcDlgName ))
	if ( dialogItem == nil ) then
		return false
	end
	local inWikiHeader = false
	local doneWikiheader = false
	local wikiheadtext = ""
	local wikiheadstart = modWP_Dialog.modcommon.LuaBlkCommentStart .. modWP_Dialog.wikiheadmarker
	local wikiheadend = modWP_Dialog.wikiheadmarker .. modWP_Dialog.modcommon.LuaBlkCommentEnd
	local dlgData = modWP_Dialog.modcommon.Extract.TblDeepCopy(modWP_Dialog.dataItem)
	local subkey, fnCall = nil,nil	-- resolves goto DIALOG_NEXT_LINE statement error
	local key, line = next(dialogItem.data, nil)
	repeat
		if ( inWikiHeader ) then
			local textWikiHeaderEnd = modWP_Dialog.modcommon.Extract.SearchText( line, wikiheadend, "[TEXT]" )
			if ( textWikiHeaderEnd ~= nil ) then
				doneWikiheader = true
				inWikiHeader = false
			else
				wikiheadtext = wikiheadtext .. line:gsub(("\t"),"")
				goto DIALOG_NEXT_LINE
			end
		else
			if (not doneWikiheader ) then
				local textWikiHeaderStart = modWP_Dialog.modcommon.Extract.SearchText( line, wikiheadstart, "[TEXT]" )
				if ( textWikiHeaderStart ~= nil ) then
					inWikiHeader = true
					goto DIALOG_NEXT_LINE
				end
			end
		end
		-- wiki header has been processed process rest of the dialog
		subkey, fnCall = next(modWP_Dialog.patterns_dialog, nil)
		repeat
			local retrievedArgs = {}
			local altExtract = ( subkey == "trainskill" )
			retrievedArgs = modWP_Dialog.modcommon.Extract.FuncArgs(line, fnCall, altExtract)
			if ( retrievedArgs == nil ) then
				goto DIALOG_NEXT_TESTPATTERN end
			if subkey == "botname" then
				if (#retrievedArgs == 1) then
					-- alias for this character
					for subsubkey, aliasvalue in pairs(retrievedArgs) do
						if ( npcDlgName:gmatch(aliasvalue) and ( npcDlgName:len() == aliasvalue:len() )) then
						else
							modWP_Dialog.modcommon.Process.InsertToNoKeyTable( dlgData.alias, aliasvalue )
						end
					end
				else
					--	alias for another character
					--	bot name,bot alias = retrievedArgs[2], retrievedArgs[1]
					dlgData.alias[#dlgData.alias + 1] = { npc = retrievedArgs[2], name = retrievedArgs[1] }
				end	--	botname & argument size
			elseif (( subkey == "quest_given" )
				or ( subkey == "quest_update" )
				or ( subkey == "quest_end" )) then
				modWP_Dialog.modcommon.Process.InsertToNoKeyTable( dlgData[subkey], retrievedArgs[1] )
			elseif ( subkey == "improveSkill" ) then
				modWP_Dialog.modcommon.Process.InsertToNoKeyTable( dlgData.skills, retrievedArgs[1] )
			elseif ( subkey == "improveprog" ) then
				modWP_Dialog.modcommon.Process.InsertToNoKeyTable( dlgData.programs, retrievedArgs[1] )
			elseif ( subkey == "trainskill" ) then
				--	trainskill is a fn w/ 3 arguments - cost, training points, skill name
				modWP_Dialog.modcommon.Process.InsertToNoKeyTable( dlgData.skills, retrievedArgs[3] )
			end	--	switch dialog pattern / function call
::DIALOG_NEXT_TESTPATTERN::
			subkey, fnCall = next(modWP_Dialog.patterns_dialog, subkey)
		until ( subkey == nil )	--	loop through dialog file patterns
::DIALOG_NEXT_LINE::
		key, line = next(dialogItem.data, key)
	until ( key == nil )	--	loop through dialog file
	local newdata = {dialog = npcDlgName, data = dlgData}
	modWP_Dialog.AllDialogData[#modWP_Dialog.AllDialogData + 1] = newdata
	-- process wiki header information
	if ( #wikiheadtext > 0 ) then
		wikiheadtext = modWP_Dialog.codemarker .. "{" .. wikiheadtext .. "}"
		local headtext = load(wikiheadtext)
		if ( headtext ~= nil ) then
			modWP_Dialog.lastdialog = npcDlgName
			headtext()
			modWP_Dialog.lastdialog = ""
		else
			io.stderr:write("error - dialog: " .. npcDlgName .. ". Can not execute wiki header - Nil Header.\n")
			os.exit(1)
		end
	else
		io.stderr:write("error - dialog: " .. npcDlgName .. ". Can not process wiki header - no data retrieved.\n")
		os.exit(1)
	end
	return true
end

--	this is a special version of ProcessData - it takes an argument!!
--	[in]	dialogname	name of dialog file to parse - assumes file in FDRPG dialogs folder
function modWP_Dialog.ProcessData( dialogname )
	if (( dialogname == nil )
	or ( type(dialogname) ~= 'string' )
	or ( dialogname:len() < 1 )) then
		return false
	end
	modWP_Dialog.modcommon = require("modWPCommon")
	--	test for presence of source data file
	local filename = tostring(modWP_Dialog.modcommon.paths.srcDialog .. dialogname .. ".lua")
	local files = { filename }
	modWP_Dialog.modcommon.Test.Files(files)
	if ( modWP_Dialog.modcommon.Test.FileExists(filename)) then
		--	process data
		local filedata = modWP_Dialog.modcommon.Process.FileToLines(filename)
		table.insert(modWP_Dialog.DialogFileData, { dialog = dialogname, data = filedata })
		return modWP_Dialog.ParseDialog(dialogname)
	else
		return false
	end
end

--	Print out NPC information based on selected verbosity.
function modWP_Dialog.Verbosity()
	if (( not modWP_Dialog.modcommon.verbose) and ( not modWP_Dialog.modcommon.doubleverbose)) then
		return
	end
	io.stdout:write(modWP_Dialog.modcommon.VerboseHeader)
	io.stdout:write("modWP_Dialog\n")
	io.stdout:write("number of npc dialogs: " .. #modWP_Dialog.AllDialogData .. "\n")
	io.stdout:write(modWP_Dialog.modcommon.VerboseHeader)
	if (modWP_Dialog.modcommon.doubleverbose) then
		modWP_Dialog.modcommon.Process.TblPrint(modWP_Dialog.AllDialogData, nil, nil, "All Dialog Data")
	end
end

return modWP_Dialog
