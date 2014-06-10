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
--	lua module for parsing FDRPG character information
local modWP_NPC = {}
--	modWPCommon reference
modWP_NPC.modcommon = {}
-- table of modules need by modWP_NPC
modWP_NPC.modules = {}
-- names of required modules for data processing - modWP_ROTD, modWP_Levelsm modWP_Droid, modWP_Dialogs
modWP_NPC.requiredModules = { moduleNames[1].id, moduleNames[4].id, moduleNames[5].id, moduleNames[8].id }
-- names of required modules for wiki presentation - modWP_Items, modWP_Levels, modWP_Droids, modWP_Quests
modWP_NPC.wikirequiredModules = { moduleNames[3].id, moduleNames[4].id, moduleNames[5].id, moduleNames[7].id }
--	Variable will contained all parsed npc information after completion of ProcessData()
modWP_NPC.AllNPCData = {}
--	npc_specs.lua file parsed line-by-line into table
modWP_NPC.NPCFileData = {}
--table of FDRPG filepaths used for parsing NPC information
modWP_NPC.files = {
	npcspecs = "",
}
--	"struct" of npc infromation for each character
--	use deep copy to pass default values to new copy
modWP_NPC.npcDataItem = {
	name = "",
	dialog = "",
	ROTDType = "",
	is_human = false,
	level = -1,
	faction = "",
	graphics_prefix = "",
	urlAnchor = "",			--	text to use for wiki page anchor
	image = {	ext = "",	--	image extension
				src = "",	--	source file path for image (system dependent)
				dest = "",	--	destination folder for image link
				name = "",	--	text to use for link to image
	},
	wikihead = {},
	alias = {},
	quest_given = {},
	quest_update = {},
	quest_end = {},
	skills = {},
	programs = {}
}

modWP_NPC.MarkerSearchText = "%$%$"

-- table listing of order of npcDataItems tables elements and label for wiki presentation
-- format { element key, presentation label }
modWP_NPC.itemTablesWikiPresentation = {
	{ "wikihead","PERSONALITY", "Personality Traits" },
	{ "alias", "Other Names" },
	{ "wikihead","PURPOSE", "Purpose" },
	{ "wikihead","BACKSTORY", "Backstory" },
	{ "wikihead","RELATIONSHIP", "Relationships" },
	{ "quest_given", "Quests Given" },
	{ "quest_update", "Updates Quests" },
	{ "quest_end", "Completes Quests" },
	{ "skills", "Teaches Skill" },
	{ "programs", "Enhances Programs" }
}

-- special sorting for "all data table"
-- [in]	a|b	elements in all data table
function modWP_NPC.SpecialAllDataSort(a,b)
	local modLevel = modWP_NPC.modules[modWP_NPC.requiredModules[2]]
	local Cryo_A = (a.level == 12)
	local Cryo_B = (b.level == 12)
	local Debug_A = modLevel.LevelIsDebug(a.level)
	local Debug_B = modLevel.LevelIsDebug(b.level)
	local Tutorial_A = modLevel.LevelIsTutorial(a.level)
	local Tutorial_B = modLevel.LevelIsTutorial(b.level)
	local Human_A = ((a.is_human) and ( a.ROTDType ~= "INV" ))
	local Human_B = ((b.is_human) and ( b.ROTDType ~= "INV" ))
	if (( not Cryo_A ) and ( Cryo_B )) then
		return false
	elseif (( Cryo_A ) and ( not Cryo_B )) then
		return true
	elseif (( Debug_A ) and ( not Debug_B )) then
		return false
	elseif (( not Debug_A ) and ( Debug_B )) then
		return true
	elseif (( Tutorial_A ) and ( not Tutorial_B ) and ( not Debug_B )) then
		return false
	elseif (( not Tutorial_A ) and ( not Debug_A ) and ( Tutorial_B )) then
		return true
	elseif (( Tutorial_A ) and ( Debug_B )) then
		return true
	elseif (( Debug_A ) and ( Tutorial_B )) then
		return false
	elseif ( a.level == b.level ) then
		if (( not Human_A ) and ( Human_B )) then
			return false
		elseif (( Human_A ) and (not Human_B )) then
			return true
		else
			--a and b are human || a and b are not human
			return (a.name < b.name)
		end
	else
		return ( a.level < b.level )
	end
end

--	Function has to be in "global" space to parse npc_specs.lua and is called "automagically"
--	when dofile(npc_specs.lua) is called. Variable AllNPCData populated by this function.
--	[in] filedata	file information to be parsed.
function npc_list( filedata )
	if (type(filedata) == "table") then
		for key,value in pairs(filedata) do
			local tempNPCitem = modWP_NPC.modcommon.Extract.TblDeepCopy(modWP_NPC.npcDataItem)
			tempNPCitem.name = value
			modWP_NPC.AllNPCData[#modWP_NPC.AllNPCData + 1] = tempNPCitem
		end
	end
end

--	Function has to be in "global" space to parse npc_specs.lua and is called "automagically"
--	when dofile(npc_specs.lua) is called. Variable AllNPCData populated by this function.
--	[in] filedata	file information to be parsed.
function npc_shop( filedata )
	if (type(filedata) == "table") then
		local npc = select(1,modWP_NPC.modcommon.Extract.GetTableItem(modWP_NPC.AllNPCData,"name", filedata.name))
		if ( npc ~= nil ) then
			npc.ShopItems = filedata.items
		else
			local tempNPCitem = modWP_NPC.modcommon.Extract.TblDeepCopy(modWP_NPC.npcDataItem)
			tempNPCitem.name = filedata.name
			tempNPCitem.ShopItems = filedata.items
			modWP_NPC.AllNPCData[#modWP_NPC.AllNPCData + 1] = tempNPCitem
		end
	end
end

-- retrieve the anchortext from the NPC with associated id value
function modWP_NPC.GetItemUrlText( idvalue )
	local retText = ""
	if ( not idvalue ) then
		return retText
	end
	local npc = select(1,modWP_NPC.modcommon.Extract.GetTableItem(modWP_NPC.AllNPCData,"name", idvalue))
	if ( npc ~= nil ) then
		retText = modWP_NPC.modcommon.outputfilenames.npc .. npc.urlAnchor
	end
	return retText
end

--	process each NPC data and save paths for linked images
--	this is apart of data processing - needed for wiki presentation on other pages
function modWP_NPC.ProcessPathAndLinks()
	if ( modWP_NPC.AllNPCData == nil ) then return end
	local modWIKI = modWP_NPC.modcommon.Wiki
	for key, npc in pairs(modWP_NPC.AllNPCData)do
		--	set npc wiki url anchor value and image information
		npc.urlAnchor = modWIKI.HLink .. modWIKI.WikifyLink("npc" .. npc.name)
		local srcpathshort = "droids/" .. npc.graphics_prefix
		npc.image.ext = ".png"
		npc.image.src = modWP_NPC.modcommon.paths.srcGraphics .. srcpathshort .. "/portrait" .. npc.image.ext
		npc.image.dest = modWP_NPC.modcommon.paths.destRootImg .. "Droids/"
		npc.image.name = srcpathshort:gsub("%/","_")
	end	--	loop through each npc data entry
end

--	Process extracted information for npcs.
function modWP_NPC.ProcessData()
	modWP_NPC.modcommon = require("modWPCommon")
	modWP_NPC.files.npcspecs = tostring(modWP_NPC.modcommon.paths.srcMap .. modWP_NPC.modcommon.datafiles["npc"])
	modWP_NPC.modcommon.Test.Files(modWP_NPC.files)
	for key, value in pairs(modWP_NPC.requiredModules) do
		modWP_NPC.modules[value] = assert(require(value))
	end
	--	process data - parse lua file first to remove gettext markers, load it, execute into global space
	--	execution calls the global npc_item/npc_shop functions
	local luatext = modWP_NPC.modcommon.Process.FileToChunk(modWP_NPC.files.npcspecs)
	local stuff = load(luatext)
	stuff()
	--	populate AKA - aliases - reload and process npc data file as text
	modWP_NPC.ProcessNPCAliases()

	local removedlg = {}
	for key, npc in pairs(modWP_NPC.AllNPCData) do
		local ignoreDlg = select(1,modWP_NPC.modcommon.Extract.GetTableItem(modWP_NPC.modcommon.IgnoreDialog, nil, npc.name ))
		if ( ignoreDlg ~= nil ) then
			removedlg[#removedlg + 1] = key
			goto NEXT_NPC_IGNORE_DLG_CHECK
		end	-- this an item to be ignored - modWPCommon.IgnoreDialog value
::NEXT_NPC_IGNORE_DLG_CHECK::
	end
	if (#removedlg > 0) then
		for data = #removedlg, 1, -1 do
			if ( modWP_NPC.modcommon.doubleverbose ) then
				local npcname = modWP_NPC.AllNPCData[removedlg[data]].name
				modWP_NPC.modcommon.Process.TblPrint({ name = npcname },nil,nil,"Removed NPC Item(IgnoreDialog)")
			end
			table.remove(modWP_NPC.AllNPCData, removedlg[data] )
		end
	end

	modROTD = modWP_NPC.modules[modWP_NPC.requiredModules[1]]
	modDroid = modWP_NPC.modules[modWP_NPC.requiredModules[3]]
	for key, ROTDItem in pairs(modROTD.AllROTDData) do
		local curDroid = select(1,modWP_NPC.modcommon.Extract.GetTableItem(modDroid.AllDroidData, "name", ROTDItem.itemtype ))
		if ((curDroid == nil )
		or ((select(1,modWP_NPC.modcommon.Extract.GetTableItem(modWP_NPC.modcommon.IgnoreFaction, nil, ROTDItem.faction))) ~= nil )
		or ((select(1,modWP_NPC.modcommon.Extract.GetTableItem(modWP_NPC.modcommon.IgnoredTypes, nil, ROTDItem.itemtype))) ~= nil )
		or ((select(1,modWP_NPC.modcommon.Extract.GetTableItem(modWP_NPC.modcommon.IgnoreDialog, nil, ROTDItem.dialog))) ~= nil )) then
			goto PROCESS_NEXTROTD_ENTRY
		end
		local curNPC = select(1,modWP_NPC.modcommon.Extract.GetTableItem(modWP_NPC.AllNPCData, "name", ROTDItem.dialog ))
		if (curNPC == nil) then
			goto PROCESS_NEXTROTD_ENTRY end
		curNPC.dialog			= ROTDItem.dialog
		curNPC.ROTDType 		= ROTDItem.itemtype
		curNPC.level			= ROTDItem.itemlevel
		curNPC.faction			= ROTDItem.faction
		curNPC.faction = curNPC.faction:gsub("^%l", string.upper)
		curNPC.is_human			= curDroid.is_human
		curNPC.graphics_prefix	= curDroid.graphics_prefix
		if ((curNPC.name:gmatch(ROTDItem.label)) and ( curNPC.name:len() ~= ROTDItem.label:len() )) then
			modWP_NPC.modcommon.Process.InsertToNoKeyTable( curNPC.alias, ROTDItem.label )
		end	--	process NPC alias in ROTD file
::PROCESS_NEXTROTD_ENTRY::
	end	--	loop through parsed ROTD data

	local removeItems = {}
	if (#modWP_NPC.AllNPCData > 0) then
		for key, npc in pairs(modWP_NPC.AllNPCData) do
			if ((npc.dialog:len() < 1)
				or (npc.graphics_prefix:len() < 1)
				or (npc.ROTDType:len() < 1)
				or (npc.faction:len() < 1)
				or (npc.level == -1)
				) then
				removeItems[#removeItems + 1] = key
				goto PROCESS_NEXT_NPC_TESTREMOVED
			end -- if item has no faction/dialog/ROTDType - remove
::PROCESS_NEXT_NPC_TESTREMOVED::
		end
	end
	if (#removeItems > 0) then
		for data = #removeItems, 1, -1 do
			if ( modWP_NPC.modcommon.doubleverbose ) then
				local npcname = modWP_NPC.AllNPCData[removeItems[data]].name
				modWP_NPC.modcommon.Process.TblPrint({ name = npcname },nil,nil,"Removed NPC Item (empty Data)")
			end
			table.remove(modWP_NPC.AllNPCData, removeItems[data] )
		end
	end
	modWP_NPC.ProcessNPCDialog()
	if (#modWP_NPC.AllNPCData > 0) then
		table.sort( modWP_NPC.AllNPCData, modWP_NPC.SpecialAllDataSort )
	end	--	sort the npclist array - see modWP_NPC.SpecialAllDataSort for how
	for key, npc in pairs(modWP_NPC.AllNPCData) do
		for subkey, element in pairs(modWP_NPC.itemTablesWikiPresentation) do
			if ( element[1] ~= "wikihead" ) then
				if ( #npc[element[1]] > 0 ) then
					--	sort table data in npc item
					table.sort(npc[element[1]])
				else
					--	null unused tables in npc item
					npc[element[1]] = nil
				end
			end
		end
	end
	modWP_NPC.ProcessPathAndLinks()
end

--	Locate and read in dialog file for each character.
--	Find functions 'set_bot_name', 'Tux:add_quest("*")', 'Tux:done_quest("*")'
--	,'Tux:improve_skill' and 'Tux:improve_program'.
--	Save data to NPC entry in AllNPCData.
function modWP_NPC.ProcessNPCDialog()
	local modDlg = modWP_NPC.modules[modWP_NPC.requiredModules[4]]
	for key, npc in pairs(modWP_NPC.AllNPCData) do
		-- process each NPC's dialog file
		if (not modDlg.ProcessData(npc.dialog)) then
			io.stderr:write("error - dialog file: " .. npc.dialog .. " not processed.\n")
			goto DIALOG_NEXT_NPC_ENTRY
		end
		-- retrieve data from the dialog object into NPC's object
		local dlgdata = select(1,modWP_NPC.modcommon.Extract.GetTableItem(modDlg.AllDialogData, "dialog", npc.dialog ))
		if ( dlgdata == nil ) then goto DIALOG_NEXT_NPC_ENTRY end
		for subkey, dlgtbl in pairs (dlgdata.data) do
			if ( subkey == "alias" ) then
				for subkey, aliasvalue in pairs(dlgtbl) do
					if ( type(aliasvalue) == 'string' ) then
						modWP_NPC.modcommon.Process.InsertToNoKeyTable( npc.alias, aliasvalue )
					elseif ( type(aliasvalue) == 'table' ) then
						otherNPC = select(1,modWP_NPC.modcommon.Extract.GetTableItem(modWP_NPC.AllNPCData, "name", aliasvalue.npc ))
						if ( otherNPC ~= nil ) then
							modWP_NPC.modcommon.Process.InsertToNoKeyTable( otherNPC.alias, aliasvalue.name )
						end
					end
				end
			elseif ( subkey == "wikihead" ) then
				npc.wikihead = modWP_NPC.modcommon.Extract.TblDeepCopy(dlgtbl)
				if ( npc.wikihead.MARKERS == nil ) then
					npc.wikihead.MARKERS = {}
				end
				npc.wikihead.MARKERS.NAME = npc.name
			else
				for index, content in pairs(dlgtbl) do
					content = content:gsub("^%l", string.upper)
					modWP_NPC.modcommon.Process.InsertToNoKeyTable( npc[subkey], content )
				end
			end
		end	-- loop through each table element in retrieved dialog data
::DIALOG_NEXT_NPC_ENTRY::
	end	--	loop through each saved NPC
end

--	Read in FDRPG npc_specs.lua data file manually and parse file for
--	character aliases stored as lua comments. Save found aliases to AllNPCData.
function modWP_NPC.ProcessNPCAliases()
	modWP_NPC.NPCFileData = modWP_NPC.modcommon.Process.FileToLines(modWP_NPC.files.npcspecs)
	local inNPCList=false
	for key, line in pairs (modWP_NPC.NPCFileData) do
		if ( line:find("npc_list{", 1, true) ) then
			inNPCList = true
		end
		if ( inNPCList and ( line:find("}", 1, true )) ) then
			inNPCList = false
			break
		end
		if (not inNPCList ) then goto ALIAS_NEXT_LINE end
		--	does line start with comment? (one or more spaces followed by "--")
		--	pattern would appear at position 1 in string
		local commentstart, commentend = line:find("%s*%-%-", 1)
		if ( commentstart == 1 ) then goto ALIAS_NEXT_LINE end
		--	non-comment string - process
		--	use lazy repetitions pattern to find "smallest" text between quotes
		local charnamestart, charnameend = line:find("\".-\"", 1)
		-- did not find quoted text
		if (not charnamestart) then	goto ALIAS_NEXT_LINE end
		local charname = line:sub((charnamestart + 1), (charnameend - 1))
		--	find if line contains aliases ( text between "--[[" and "]]--" )
		local pattern = modWP_NPC.modcommon.LuaBlkCommentStart .. "(.+)" .. modWP_NPC.modcommon.LuaBlkCommentEnd
		local aliastext = modWP_NPC.modcommon.Extract.SearchText( line, pattern, "[MATCH]" )
		--	alias not found
		if ( aliastext == nil ) then goto ALIAS_NEXT_LINE end
		--	stuff in between are aliases (may be comma seperated)
		local npc,npcindex = modWP_NPC.modcommon.Extract.GetTableItem(modWP_NPC.AllNPCData,"name", charname)
		--	character name not found in AllNPCData
		if (npc == nil) then goto ALIAS_NEXT_LINE end
		local aliases = modWP_NPC.modcommon.Extract.Split(aliastext, ",", true)
		for key, alias in pairs(aliases) do
			local ingnoreAlias = select(1,modWP_NPC.modcommon.Extract.GetTableItem(modWP_NPC.modcommon.IgnoreDialog, nil, alias ))
			local ignoreType = select(1,modWP_NPC.modcommon.Extract.GetTableItem(modWP_NPC.modcommon.IgnoredTypes, nil, alias ))
			local ignText = ""
			if ( ingnoreAlias ~= nil ) then
				ignText = ingnoreAlias
			elseif ( ignoreType ~= nil ) then
				ignText = ignoreType
			end
			if (( ingnoreAlias ~= nil )
			or ( ignoreType ~= nil )) then
				if ( modWP_NPC.modcommon.doubleverbose ) then
					modWP_NPC.modcommon.Process.TblPrint({ name = npc.name },nil,nil,"Removed NPC Item(" .. ignText .. ")")
				end
				table.remove(modWP_NPC.AllNPCData,npcindex)
				goto ALIAS_NEXT_LINE
			end	-- this an item to be ignored - modWPCommon.Ignored* values
			if ((npc.name:gmatch(alias)) and ( npc.name:len() == alias:len()) ) then
			else
				modWP_NPC.modcommon.Process.InsertToNoKeyTable( npc.alias, alias )
			end	--	process alias
		end	--	loop through aliases
::ALIAS_NEXT_LINE::
	end	--	processing file data
end

--	cycle through all NPCs and process the text replacements that need to occur for wiki presentation
--	NOTE: This has to occur near the start of WikiWrite due to dependencies on processed data
function modWP_NPC.ProcessHeaderMarkers()
	if ( modWP_NPC.AllNPCData == nil ) then return end
	for key, npc in pairs(modWP_NPC.AllNPCData)do
		if ( npc.wikihead == nil ) then goto MARKER_NEXT_NPC end
		-- error checking wiki header
		if ((( npc.wikihead.PURPOSE ~= nil) and ( type(npc.wikihead.PURPOSE) == 'table' ))
		or (( npc.wikihead.BACKSTORY ~= nil) and ( type(npc.wikihead.BACKSTORY) == 'table' ))) then
			io.stderr:write("error processing wiki header in " .. modWP_NPC.modcommon.paths.srcDialog .. npc.dialog .. ".lua\n")
			os.exit(1)
		end
		-- have a wiki header
		if ( npc.wikihead.MARKERS == nil ) then goto MARKER_NEXT_NPC end
		-- npc wiki header has text replacement markers
		for subkey, marker in pairs(npc.wikihead.MARKERS) do
			local replacetext = ""
			local anchor = ""
			local replace = ""
			if ( subkey:match("NAME") ) then
				replace = "NAME"
				replacetext = npc.name
			elseif ( subkey:match("NPCID") ) then
				replace = "NPC"
				otherNPC = select(1,modWP_NPC.modcommon.Extract.GetTableItem(modWP_NPC.AllNPCData, "name", marker ))
				if ( otherNPC ~= nil ) then
					replacetext = modWP_NPC.modcommon.Wiki.LinkText( otherNPC.urlAnchor, marker )
				end
			elseif ( subkey:match("DROID") ) then
				replace = "DROID"
				local modDroid = modWP_NPC.modules[modWP_NPC.wikirequiredModules[3]]
				anchor = modDroid.GetItemUrlText(marker)
				if ( #anchor > 0 ) then
					replacetext = modWP_NPC.modcommon.Wiki.LinkText( anchor, marker )
				end
			elseif ( subkey:match("ITEM") ) then
				replace = "ITEM"
				local modItem = modWP_NPC.modules[modWP_NPC.wikirequiredModules[1]]
				anchor = modItem.GetItemUrlText(marker)
				if ( #anchor > 0 ) then
					replacetext = modWP_NPC.modcommon.Wiki.LinkText( anchor, marker )
				end
			elseif ( subkey:match("QUEST") ) then
				replace = "QUEST"
				local modQuests = modWP_NPC.modules[modWP_NPC.wikirequiredModules[4]]
				anchor = modQuests.GetItemUrlText(marker)
				if ( #anchor > 0 ) then
					replacetext = modWP_NPC.modcommon.Wiki.LinkText( anchor, marker )
				end
			end
			-- if marker ( which is an ID value ) is not found - flag it in the wiki text
			if (not (#replacetext > 0)) then
				replacetext = "<" .. marker .. ">"
			end
			-- replace all occurrances of "subkey" with replace text
			local searchText = modWP_NPC.MarkerSearchText .. subkey .. modWP_NPC.MarkerSearchText
			if ( npc.wikihead.PURPOSE ~= nil ) then
				npc.wikihead.PURPOSE = npc.wikihead.PURPOSE:gsub(searchText, replacetext)
			end
			if ( npc.wikihead.BACKSTORY ~= nil ) then
				npc.wikihead.BACKSTORY = npc.wikihead.BACKSTORY:gsub(searchText, replacetext)
			end
			if (( npc.wikihead.RELATIONSHIP ~= nil) and ( #npc.wikihead.RELATIONSHIP > 0 )) then
				for index, relations in pairs(npc.wikihead.RELATIONSHIP) do
					if (replace:match("NPC")) then
						relations.actor = relations.actor:gsub(searchText, replacetext)
						relations.text = relations.text:gsub(searchText, marker)
					else
						relations.text = relations.text:gsub(searchText, replacetext)
					end
				end
			end
::MARKER_NEXT_HEADELEMENT::
		end
::MARKER_NEXT_NPC::
	end	--	loop through each npc data entry
end

--	loop through all NPC data and process the links to quest information
--	NOTE: This has to occur near the start of WikiWrite due to dependencies on processed data
function modWP_NPC.WikiPreProcessQuests()
	if ( modWP_NPC.AllNPCData == nil ) then return end
	local modWIKI = modWP_NPC.modcommon.Wiki
	local modQuests = modWP_NPC.modules[modWP_NPC.wikirequiredModules[4]]
	for key, npc in pairs(modWP_NPC.AllNPCData)do
		--	preprocess quest tables
		if ( npc.quest_given ) then
			local questdata = {}
			for key, questname in pairs(npc.quest_given) do
				local namelink = modQuests.GetItemUrlText(questname)
				if ( namelink ) then
					questdata[#questdata + 1] = modWIKI.LinkText(namelink, questname)
				end
			end
			npc.quest_given = questdata
		end
		if ( npc.quest_update ) then
			local questdata = {}
			for key, questname in pairs(npc.quest_update) do
				local namelink = modQuests.GetItemUrlText(questname)
				if ( namelink ) then
					questdata[#questdata + 1] = modWIKI.LinkText(namelink, questname)
				end
			end
			npc.quest_update = questdata
		end
		if ( npc.quest_end ) then
			local questdata = {}
			for key, questname in pairs(npc.quest_end) do
				local namelink = modQuests.GetItemUrlText(questname)
				if ( namelink ) then
					questdata[#questdata + 1] = modWIKI.LinkText(namelink, questname)
				end
			end
			npc.quest_end = questdata
		end
	end
end

--	Write FDRPG npc information to file in a wiki format
--	Format is currently pmwiki-specific. Output file is ready
--	to be loaded directly into FDRPG site.
function modWP_NPC.WikiWrite()
	-- NOTE: Load Modules BEFORE ATTEMPTING TO PROCESS MARKERS
		for key, value in pairs(modWP_NPC.wikirequiredModules) do
		modWP_NPC.modules[value] = assert(require(value))
	end
	-- some preprocessing is required before wiki presentation
	modWP_NPC.ProcessHeaderMarkers()
	modWP_NPC.WikiPreProcessQuests()
	-- now make the page
	local modWIKI = modWP_NPC.modcommon.Wiki
	local LI = modWIKI.LI
	local SEP = modWIKI.Seperator
	local filename = modWP_NPC.modcommon.outputfilenames.npc
	local filepath = tostring(modWP_NPC.modcommon.paths.destRootFile .. filename)
	local wikitext = {}

	local modItems = modWP_NPC.modules[modWP_NPC.wikirequiredModules[1]]
	local modLevel = modWP_NPC.modules[modWP_NPC.wikirequiredModules[2]]
	local modDroid = modWP_NPC.modules[modWP_NPC.wikirequiredModules[3]]
	local modQuests = modWP_NPC.modules[modWP_NPC.wikirequiredModules[4]]

	wikitext[#wikitext + 1] = modWIKI.PageSummary("FreedroidRPG NPC\'s")
	--	make menu for npc types
	wikitext[#wikitext + 1] = modWIKI.FrameStartRight("font-size:smaller")
	wikitext[#wikitext + 1] = modWIKI.HeaderLevel(3) .. "Freedroid NPC\'s"
	wikitext[#wikitext + 1] = modWIKI.LinkText(modWIKI.HLink .. "allNPC","NPC\'s")
	for key, npc in pairs(modWP_NPC.AllNPCData) do
		wikitext[#wikitext + 1] = LI .. modWIKI.LinkText(npc.urlAnchor, npc.name)
	end	--	loop to produce menu items
	wikitext[#wikitext + 1] = modWIKI.FrameEnd
	--	end menu

	wikitext = modWIKI.WarnAutoGen( wikitext )
	wikitext = modWIKI.WarnSpoil( wikitext )
	--	page contents start here
	wikitext[#wikitext + 1] = modWIKI.LinkText(modWIKI.HLink .. "allNPC")
	wikitext[#wikitext + 1] = modWIKI.HeaderLevel(1) .. "NPC\'s"
	--	loop to produce individual npc entries
	for key, npc in pairs(modWP_NPC.AllNPCData) do
		--	image portrait
		local portraitname = ""
		local portaititem = modWIKI.ManagePortrait(	npc.image.src, npc.image.dest, npc.image.name, npc.image.ext)
		if ( portaititem ~= nil ) then
			portraitname = portaititem.destfile
		end
		--	process presentation
		local dialogname = npc.dialog .. ".lua"
		local sfdialog = modWIKI.URL_SF .. "dialogs/" .. dialogname
		local dialoglink = modWIKI.LinkText( sfdialog, dialogname )
		wikitext[#wikitext + 1] = modWIKI.LinkText(npc.urlAnchor)
		wikitext[#wikitext + 1] = modWIKI.HeaderLevel(2) .. npc.name
		wikitext[#wikitext + 1] = modWIKI.ImageText( modWIKI.URL_ImgDroid .. portraitname, npc.graphics_prefix, "margin-right=\'1.0em\'")
		wikitext[#wikitext + 1] = modWIKI.FrameStartLeft("border=\'0px\' width=65pct")
		wikitext[#wikitext + 1] = modLevel.WikiEntryLevelText( npc.level, "Location", true ) .. modWIKI.ForceBreak
		--	preprocess link to droid type if not human
		local modeltext = npc.ROTDType
		if ( not npc.is_human ) then
			local namelink = modDroid.GetItemUrlText(modeltext)
			if ( namelink ) then
				modeltext = modWIKI.LinkText(namelink, modeltext)
			end
		end
		wikitext[#wikitext + 1]	= modWIKI.TextEntry( "Model Type", modeltext, SEP )
		wikitext[#wikitext + 1]	= modWIKI.TextEntry( "Dialog File", dialoglink, SEP )
		wikitext[#wikitext + 1]	= modWIKI.TextEntry( "Faction", npc.faction, SEP, nil, false )
		wikitext[#wikitext + 1] = " "	--	<--keep to force line break between faction and next element
		for key, item in pairs(modWP_NPC.itemTablesWikiPresentation) do
			if ( item[1] == "wikihead" ) then
				local wikiheaddata = npc[item[1]]
				if (( item[2] == "PERSONALITY" ) and (wikiheaddata[item[2]] ~= nil )) then
					local traittext = modWP_NPC.modcommon.Extract.OneDTableToString(wikiheaddata[item[2]],nil,", ")
					wikitext[#wikitext + 1]	= modWIKI.TableToWiki( {traittext}, item[3], SEP )
				elseif ((( item[2] == "PURPOSE" ) or ( item[2] == "BACKSTORY" ))
				and ( wikiheaddata[item[2]] ~= nil )) then
					wikitext[#wikitext + 1]	= modWIKI.TableToWiki( {wikiheaddata[item[2]]}, item[3], SEP )
				elseif (( item[2] == "RELATIONSHIP" ) and ( wikiheaddata[item[2]] ~= nil )) then
					local wikirelations = wikiheaddata[item[2]]
					wikitext[#wikitext + 1]	= modWIKI.TextEntry( item[3], nil, SEP )
					for subkey, relations in pairs(wikirelations) do
						wikitext[#wikitext + 1]	= modWIKI.TableToWiki( {relations.text}, relations.actor, SEP )
					end
				else
					wikitext[#wikitext + 1]	= modWIKI.TableToWiki( wikiheaddata[item[2]], item[3], SEP )
				end
			else
				wikitext[#wikitext + 1]	= modWIKI.TableToWiki( npc[item[1]], item[2], SEP )
			end
		end
		wikitext[#wikitext + 1] = modWIKI.FrameEnd
		wikitext[#wikitext + 1] = modWIKI.ForceBreak
		wikitext[#wikitext + 1] = modWIKI.LineSep
	end
	--	write wiki data object to string
	local writedata = modWIKI.PageProcess( filename, wikitext )
	--	write string to file
	modWP_NPC.modcommon.Process.DataToFile(filepath, writedata)
end

--	Print out NPC information based on selected verbosity.
function modWP_NPC.Verbosity()
	if (( not modWP_NPC.modcommon.verbose) and ( not modWP_NPC.modcommon.doubleverbose)) then
		return
	end
	io.stdout:write(modWP_NPC.modcommon.VerboseHeader)
	io.stdout:write("modWP_NPC\n")
	io.stdout:write("number of npcs: " .. #modWP_NPC.AllNPCData .. "\n")
	io.stdout:write(modWP_NPC.modcommon.VerboseHeader)
	if (modWP_NPC.modcommon.doubleverbose) then
		modWP_NPC.modcommon.Process.TblPrint(modWP_NPC.AllNPCData, nil, nil, "All NPC Data")
	end
end

return modWP_NPC
