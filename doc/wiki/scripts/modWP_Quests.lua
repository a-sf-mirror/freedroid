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
--	Lua module to parse FDRPG Character information into wiki page
local modWP_Quests = {}
--	modWPCommon reference
modWP_Quests.modcommon = {}
--	Variable will contained all parsed information after completion of ProcessNPCData()
modWP_Quests.AllQuestsData = {}
--	ReturnOfTux.droid file parsed line-by-line into table
modWP_Quests.QuestsFileData = {}
-- module references
modWP_Quests.modules = {}
-- names of required modules - "modWP_NPC" <-- also required for wiki
modWP_Quests.requiredModules = { moduleNames[6].id, }
--	table of FDRPG filepaths used for parsing ReturnOfTux.Droids information
modWP_Quests.files = {
	quests=""
}
--	"struct" of quest information
modWP_Quests.questItem = {
	name = "",
	desc = "",
	targetlevel = -1,
	targetmarker = -1,
	isDebug = false,
	isTutorial = false,
	code = {},
	questAssign = {},
	questActor = {},
	content = {},	-- nulled after parsing
}
-- markers deliniating code in file data
modWP_Quests.codemarkstart = "Completion LuaCode={"
modWP_Quests.codemarkend = "}"

--	text for Event presentation/parsing - break up file data into sections
--	id) key, srchPatn) text/patter to use to find data	extrctPatn) how to extract found data
modWP_Quests.QuestSection = {
	{ id = "missionStart", srchPatn = "Start of this mission target subsection",                                extrctPatn="[TEXT]" },
	{ id = "missionEnd",   srchPatn = "End of this mission target subsection",                                  extrctPatn="[TEXT]" },
}
--	text for Event presentation/parsing - parsing each event section
--	id) key, srchPatn) text/patter to use to find data	extrctPatn) how to extract found data
modWP_Quests.Triggers = {
	{ id = "name",         srchPatn = "Mission Name=_(%b\"\")",                                                 extrctPatn="[MATCH]" },
	{ id = "desc",         srchPatn = "+++",                                                                    extrctPatn="[EOL]"   },
	{ id = "targetlevel",  srchPatn = "Mission target is to kill all hostile droids on this level%s*:%s*[%d]+", extrctPatn="[%d]+"   },
	{ id = "targetmarker", srchPatn = "Mission target is to kill droids with marker%s*:%s*[%d]+",               extrctPatn="[%d]+"   },
	{ id = "codeStart",    srchPatn = modWP_Quests.codemarkstart,                                               extrctPatn="[TEXT]"  },
	{ id = "codeEnd",      srchPatn = modWP_Quests.codemarkend,                                                 extrctPatn="[TEXT]"  },
}

modWP_Quests.NPCQuestTables = { "quest_given", "quest_update", "quest_end" }

-- special sorting for "all data table"
-- [in]	a|b	elements in all data table
function modWP_Quests.SpecialAllDataSort(a,b)
	if (((a.isDebug) and (not b.isDebug ))
		or ((a.isDebug) and (b.isTutorial ))
		or ((a.isTutorial) and (not b.isTutorial ) and (not a.isDebug) and (not b.isDebug))
	) then
		return false
	elseif (((not a.isDebug) and (b.isDebug ))
		or ((a.isTutorial) and (b.isDebug ))
		or ((not a.isTutorial) and (b.isTutorial ) and (not a.isDebug) and (not b.isDebug))
	) then
		return true
	else
		return (a.name < b.name)
	end
end

-- retrieve the anchortext associated with this id value
function modWP_Quests.GetItemUrlText( idvalue )
	local retText = ""
	if ( not idvalue ) then
		return retText
	end
	local quest = select(1,modWP_Quests.modcommon.Extract.GetTableItem(modWP_Quests.AllQuestsData,"name", idvalue))
	if ( quest ~= nil ) then
		retText = modWP_Quests.modcommon.outputfilenames.quests .. quest.urlAnchor
	end
	return retText
end

--	Parse quests.dat - Find all Start/End Events copying information to ["contents"] table
function modWP_Quests.ParseFile()
	local inQuest = false
	local processed = false
	local section = {}
	-- pass # 1 - parse file
	for key, line in pairs(modWP_Quests.QuestsFileData) do
		for subkey, trigItem in pairs(modWP_Quests.QuestSection) do
			local data = modWP_Quests.modcommon.Extract.SearchText( line, trigItem.srchPatn, trigItem.extrctPatn )
			if ( data ) then
				if ( trigItem.id == "missionEnd" ) then
					inQuest = false
					processed = true
					modWP_Quests.AllQuestsData[#modWP_Quests.AllQuestsData + 1] = section
					break
				elseif ( trigItem.id == "missionStart" ) then
					inQuest = true
					processed = true
					section = modWP_Quests.modcommon.Extract.TblDeepCopy(modWP_Quests.questItem)
					break
				end
			end
		end	--	loop through Quest Section
		if ( not processed and inQuest ) then
			section.content[#section.content + 1] = line
		end
		processed = false
	end	--	loop through quest file data
end

--	Process each found quest event's contents for more details about the event
function modWP_Quests.ParseContents()
	local modNPC = assert(require(modWP_Quests.requiredModules[1]))
	local in_code = false
	local key, section = next(modWP_Quests.AllQuestsData, nil)
	repeat
		for subkey, line in pairs(section.content) do
			local dataItem = {}
			if ( in_code ) then
				dataItem = modWP_Quests.modcommon.Extract.SearchText( line, modWP_Quests.codemarkend, "[TEXT]" )
				if ( dataItem == nil ) then
					section.code[#section.code + 1] = line
				else
					in_code = false
					break
				end
			else
				for sub, trigItem in pairs(modWP_Quests.Triggers) do
					dataItem = modWP_Quests.modcommon.Extract.SearchText( line, trigItem.srchPatn, trigItem.extrctPatn )
					if ( dataItem == nil ) then	goto PARSE_EVENT_NEXT_PATTERN end
					-- search text found
					if (( trigItem.id == "name" )
					or ( trigItem.id == "targetlevel" )
					or ( trigItem.id == "targetmarker" )) then
						section[trigItem.id] = dataItem
						break
					elseif ( trigItem.id == "desc" ) then
						section[trigItem.id] = section[trigItem.id] .. dataItem
						break
					elseif ( trigItem.id == "codeStart" ) then
						local data = modWP_Quests.modcommon.Extract.SearchText( line, modWP_Quests.codemarkend, "[TEXT]" )
						if ( data ~= nil ) then
							in_code = false
							section.code[#section.code + 1] = line:sub((modWP_Quests.codemarkstart:len() + 1), (#line - modWP_Quests.codemarkend:len()))
						else
							in_code = true
						end
						break
					elseif ( trigItem.id == "codeEnd" ) then
						in_code = false
						break
					end
::PARSE_EVENT_NEXT_PATTERN::
				end	--	determine event type
			end	--	copy code contents
		end	--	loop through each line of text in section
		key, section = next(modWP_Quests.AllQuestsData, key)
	until ( key == nil )

	for key, quest in pairs(modWP_Quests.AllQuestsData, nil) do
		local questnamelower = quest.name:lower()
		quest.isDebug = (	( questnamelower:find("debug") ~= nil )
						or ( questnamelower:find("24_") ~= nil ))
		quest.isTutorial = ( questnamelower:find("tutorial")  ~= nil )
		for subkey, npc in pairs(modNPC.AllNPCData) do
			for index, questItem in pairs(modWP_Quests.NPCQuestTables) do
				if (npc[questItem] == nil) then goto NPC_NEXT_QUEST_ITEM end
				local foundquest = select(1,modWP_Quests.modcommon.Extract.GetTableItem(npc[questItem], nil, quest.name))
				if ( foundquest == nil ) then goto NPC_NEXT_QUEST_ITEM
				else
					-- quest.name not found in table npc[questItem]
					if ( questItem == modWP_Quests.NPCQuestTables[1]) then
						modWP_Quests.modcommon.Process.InsertToNoKeyTable( quest.questAssign, npc.name )
					elseif (( questItem == modWP_Quests.NPCQuestTables[2])
					or ( questItem == modWP_Quests.NPCQuestTables[3])) then
						local foundAssign = select(1,modWP_Quests.modcommon.Extract.GetTableItem(quest.questAssign, nil, npc.name))
						if ( foundAssign ~= nil ) then goto NPC_NEXT_QUEST_ITEM end
						modWP_Quests.modcommon.Process.InsertToNoKeyTable( quest.questActor, npc.name )
					end
				end
::NPC_NEXT_QUEST_ITEM::
			end
		end	--	loop through each NPC to see if it acts in quest
		-- and clean up tables
		quest["content"] = nil
		local elements = { "code", "questAssign", "questActor" }
		for key, elem in pairs(elements) do
			if ( #quest[elem] == 0 ) then
				quest[elem] = nil
			end
		end
	end	--	post-process each quest
end

--	Read in FDRPG ReturnOfTux.droid data file
--	All droid information is saved into lua table.
function modWP_Quests.ProcessData()
	modWP_Quests.modcommon = require("modWPCommon")
	local modWIKI = modWP_Quests.modcommon.Wiki
	--	test for presence of source data files
	modWP_Quests.files.quests = tostring(modWP_Quests.modcommon.paths.srcMap .. modWP_Quests.modcommon.datafiles["quests"])
	modWP_Quests.modcommon.Test.Files(modWP_Quests.files)
	-- read file and process into table objects
	modWP_Quests.QuestsFileData = modWP_Quests.modcommon.Process.FileToLines(modWP_Quests.files["quests"])
	modWP_Quests.ParseFile()
	modWP_Quests.ParseContents()
	if (#modWP_Quests.AllQuestsData > 0) then
		table.sort(modWP_Quests.AllQuestsData,modWP_Quests.SpecialAllDataSort)
	end	--	sort AllQuestsData table

	for key, quest in pairs(modWP_Quests.AllQuestsData) do
		quest["urlAnchor"] = modWIKI.HLink .. modWIKI.WikifyLink("q" .. quest.name)
	end --	loop through each npc
end

--	Write FDRPG npc information to file in a wiki format
--	Format is currently pmwiki-specific. Output file is ready
--	to be loaded directly into FDRPG site.
function modWP_Quests.WikiWrite()
	local modWIKI = modWP_Quests.modcommon.Wiki
	local LI = modWIKI.LI
	local SEP = modWIKI.Seperator
	local modNPC = assert(require(modWP_Quests.requiredModules[1]))
	local filename = modWP_Quests.modcommon.outputfilenames.quests
	local filepath = tostring(modWP_Quests.modcommon.paths.destRootFile .. filename)
	local wikitext = {}
	wikitext[#wikitext + 1] = modWIKI.PageSummary("FreedroidRPG NPC\'s")
	--	make menu for npc types
	wikitext[#wikitext + 1] = modWIKI.FrameStartRight("font-size:smaller")
	wikitext[#wikitext + 1] = modWIKI.HeaderLevel(3) .. "Freedroid Quests\'s"
	wikitext[#wikitext + 1] = modWIKI.LinkText(modWIKI.HLink .. "allquests","Quests\'s")
	for key, quest in pairs(modWP_Quests.AllQuestsData) do
		wikitext[#wikitext + 1] = LI .. modWIKI.LinkText(quest.urlAnchor, quest.name)
	end	--	loop to produce menu items
	wikitext[#wikitext + 1] = modWIKI.FrameEnd
	--	end menu

	wikitext = modWIKI.WarnAutoGen( wikitext )
	wikitext = modWIKI.WarnSpoil( wikitext )
	--	page contents start here
	wikitext[#wikitext + 1] = modWIKI.LinkText(modWIKI.HLink .. "allquests")
	wikitext[#wikitext + 1] = modWIKI.HeaderLevel(1) .. "Freedroid Quests"
	--	loop to produce individual entries
	for key, quest in pairs(modWP_Quests.AllQuestsData) do
		--	preprocess quest tables
		if ( quest.questAssign ) then
			local questdata = {}
			for key, questperson in pairs(quest.questAssign) do
				local namelink = modNPC.GetItemUrlText(questperson)
				if ( namelink ) then
					questdata[#questdata + 1] = modWIKI.LinkText(namelink, questperson)
				end
			end
			quest.questAssign = questdata
		end
		if ( quest.questActor ) then
			local questdata = {}
			for key, questperson in pairs(quest.questActor) do
				local namelink = modNPC.GetItemUrlText(questperson)
				if ( namelink ) then
					questdata[#questdata + 1] = modWIKI.LinkText(namelink,questperson)
				end
			end
			quest.questActor = questdata
		end
		--	process presentation
		wikitext[#wikitext + 1] = modWIKI.LinkText(quest.urlAnchor)
		wikitext[#wikitext + 1] = modWIKI.HeaderLevel(2) .. quest.name
		--	quest data wiki presentation
		wikitext[#wikitext + 1] = modWIKI.FrameStartLeft("border=\'0px\' width=70pct")
		wikitext[#wikitext + 1]	= modWIKI.TextEntry( "Description", quest.desc, SEP, nil, true )
		wikitext[#wikitext + 1] = " "
		wikitext[#wikitext + 1]	= modWIKI.TableToWiki( quest.questAssign, "Assigns Quest", SEP )
		wikitext[#wikitext + 1]	= modWIKI.TableToWiki( quest.questActor, "Other Quest Actors", SEP )
		wikitext[#wikitext + 1] = " "
		wikitext[#wikitext + 1] = modWIKI.FrameEnd
		wikitext[#wikitext + 1] = modWIKI.ForceBreak
		wikitext[#wikitext + 1] = modWIKI.LineSep
	end
	local writedata = modWIKI.PageProcess( filename, wikitext )
	modWP_Quests.modcommon.Process.DataToFile(filepath, writedata)
end

--	Print out NPC information based on selected verbosity.
function modWP_Quests.Verbosity()
	if (( not modWP_Quests.modcommon.verbose) and ( not modWP_Quests.modcommon.doubleverbose)) then
		return
	end
	io.stdout:write(modWP_Quests.modcommon.VerboseHeader)
	io.stdout:write("modWP_Quests\n")
	io.stdout:write("number of Quests: " .. #modWP_Quests.AllQuestsData .. "\n")
	io.stdout:write(modWP_Quests.modcommon.VerboseHeader)
	if (modWP_Quests.modcommon.doubleverbose) then
		modWP_Quests.modcommon.Process.TblPrint(modWP_Quests.AllQuestsData, nil, nil, "All Quests Data")
	end
end

return modWP_Quests
