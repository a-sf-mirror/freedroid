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
local modWP_Events = {}
--	modWPCommon reference
modWP_Events.modcommon = {}
--	Variable will contained all parsed information after completion of ProcessNPCData()
modWP_Events.AllEventData = {}
--	ReturnOfTux.droid file parsed line-by-line into table
modWP_Events.EventFileData = {}
--	table of FDRPG filepaths used for parsing ReturnOfTux.Droids information
modWP_Events.files = {
	events=""
}
--	"struct" of event information
--	for teleport event - special variant of a "label" event
--	- events.dat form
--	--	Level x				-> level number where event occurs
--	--	teleport("29from0")	-> numberA		- "29"
--							-> level_action	- "from"
--							-> numberB		- "0"
--	--	use two out of levelnumber and { numberA, numberB }
--	--	and set teleport = {lvlA, lvlB } values
modWP_Events.eventItem = {
	level = -1,
	trigger = "",					-- ["label"|"teleport"|"change"|"obstacle"|"death"]
	name = "",
	label = "",
	silent = false,
	eventlabel = { teleported = false },				-- or nil if not a label event
	eventdeath = { lvl = -1, dialog=""},				-- or nil if not a death event
	eventchange = { levelexit = -1, levelenter = -1 }, -- or nil if not a change event
	teleport = {lvlA = -1, lvlB = -1 },				-- or nil if not a transport event
	code = {},								  -- all lines between <LuaCode></LuaCode>
	content = {},										  -- raw content of event data
}
-- markers deliniating code in file data
modWP_Events.codemarkstart = "<LuaCode>"
modWP_Events.codemarkend = "</LuaCode>"
--	text for Event presentation/parsing - break up file data into sections
--	id) key, srchPatn) text/pattern to use to find data	extrctPatn) how to extract found data
modWP_Events.EventSection = {
	{ id = "triggerStart", srchPatn = "New event trigger",      extrctPatn="[TEXT]" },
	{ id = "triggerEnd",   srchPatn = "End of trigger",         extrctPatn="[TEXT]" },
	{ id = "atlevel",      srchPatn = "##%s+Level%s+%d+%s+##",  extrctPatn="[%d]+" },
	{ id = "npcdeaths",    srchPatn = "##%s+NPC%s+Deaths%s+##", extrctPatn="[TEXT]" },
}
--	text for Event presentation/parsing - parsing each event section
--	id) key, srchPatn) text/pattern to use to find data	extrctPatn) how to extract found data
modWP_Events.Triggers = {
	{ id = "label",      srchPatn = "Trigger at label=(%b\"\")",  extrctPatn="[MATCH]" },
	{ id = "death",      srchPatn = "Trigger on enemy death",     extrctPatn="[TEXT]"  },
	{ id = "change",     srchPatn = "Trigger changing level",     extrctPatn="[TEXT]"  },
	{ id = "obstacle",   srchPatn = "Trigger on obstacle",        extrctPatn="[TEXT]"  },
	{ id = "name",       srchPatn = "Name=_*(%b\"\")",            extrctPatn="[MATCH]" },
	{ id = "silent",     srchPatn = "Silent=[%d]+",               extrctPatn="[%d]+"   },
	{ id = "teleported", srchPatn = "Teleported=[%d]+",           extrctPatn="[%d]+"   },
	{ id = "obslabel",   srchPatn = "Obstacle label=(%b\"\")",    extrctPatn="[MATCH]" },
	{ id = "enemylvl",   srchPatn = "Enemy level=[%d]+",          extrctPatn="[%d]+"   },
	{ id = "enemydgl",   srchPatn = "Enemy dialog name=(%b\"\")", extrctPatn="[MATCH]" },
	{ id = "chgtolvl",   srchPatn = "Entering level=[%d]+",       extrctPatn="[%d]+"   },
	{ id = "chgfrmlvl",  srchPatn = "Exiting level=[%d]+",        extrctPatn="[%d]+"   },
	{ id = "codeStart",  srchPatn = modWP_Events.codemarkstart,   extrctPatn="[TEXT]"  },
	{ id = "codeEnd",    srchPatn = modWP_Events.codemarkend,     extrctPatn="[TEXT]"  },
}
--	text patterns used to locate teleport code lines
modWP_Events.patternTeleport = { srchPatn = "teleport%((%b\"\")%)", extrctPatn = "[MATCH]" }

--	text patterns used to locate special transport events data in events file
modWP_Events.patterns_teleport = {
	{ id = "value",   srchPatn = "(%d+)(%l+)(%d+)%a?", extrctPatn = "[TEXT]" },
	{ id = "special", srchPatn = "(%d+)(%l+)%u%d+%a?", extrctPatn = "[TEXT]" }
}
--	Parse events.dat
--	Find all Start/End Events copying information to ["contents"] table
function modWP_Events.ParseFile()
	local currentlevel = -1
	local inTrigger = false
	local processed = false
	local section = {}
	local key, line = next(modWP_Events.EventFileData, nil)
	repeat
		local subkey, trigItem = next(modWP_Events.EventSection, nil)
		repeat
			local data = modWP_Events.modcommon.Extract.SearchText( line, trigItem.srchPatn, trigItem.extrctPatn )
			if (( data == nil ) or ( not data )) then goto PARSE_FILE_NEXT_SECTION end
			if ( trigItem.id == "atlevel" ) then
				currentlevel = data
				processed = true
			elseif ( trigItem.id == "npcdeaths" ) then
				currentlevel = -1
				processed = true
			elseif ( trigItem.id == "triggerEnd" ) then
				inTrigger = false
				processed = true
				modWP_Events.AllEventData[#modWP_Events.AllEventData + 1] = section
			elseif ( trigItem.id == "triggerStart" ) then
				inTrigger = true
				processed = true
				section = modWP_Events.modcommon.Extract.TblDeepCopy(modWP_Events.eventItem)
				section.level = currentlevel
			end
::PARSE_FILE_NEXT_SECTION::
			subkey, trigItem = next(modWP_Events.EventSection, subkey)
		until ( subkey == nil )
		if ( not processed and inTrigger ) then
			section.content[#section.content + 1] = line
		end
		processed = false
		key, line = next(modWP_Events.EventFileData, key)
	until ( key == nil )
end

-- Parse ["contents"] table
-- Process each found event's contents for more details about the trigger
function modWP_Events.ParseContents()
	local modC = modWP_Events.modcommon
	local in_code = false
	local key, section = next(modWP_Events.AllEventData, nil)
	repeat
		for subkey, line in pairs(section.content) do
			local dataItem = {}
			if ( in_code ) then
				dataItem = modWP_Events.modcommon.Extract.SearchText( line, modWP_Events.codemarkend, "[TEXT]" )
				if ( dataItem == nil ) then
					section.code[#section.code + 1] = line
				else
					in_code = false
					break
				end
			else
				for sub, trigItem in pairs(modWP_Events.Triggers) do
					dataItem = modWP_Events.modcommon.Extract.SearchText( line, trigItem.srchPatn, trigItem.extrctPatn )
					if (( dataItem == nil ) or ( not dataItem )) then goto PROCESS_EVENT_NEXT_TRIGGER end
					if ( trigItem.id == "obslabel" ) then
						section["label"] = dataItem
						break
					elseif ( trigItem.id == "name" ) then
						section[trigItem.id] = dataItem
						break
					elseif ( trigItem.id == "silent" ) then
						section[trigItem.id] = (dataItem ~= 0)
						break
					elseif ( trigItem.id == "teleported" ) then
						section.eventlabel[trigItem.id] = (dataItem ~= 0)
						break
					elseif ( trigItem.id == "enemylvl" ) then
						-- if ## NPC Deaths ## then level = level_enemy
						section.eventdeath["lvl"] = dataItem
						section.level = section.eventdeath["lvl"]
						break
					elseif ( trigItem.id == "enemydgl" ) then
						section.eventdeath["dialog"] = dataItem
						break
					elseif ( trigItem.id == "chgtolvl" ) then
						section.eventchange["levelenter"] = dataItem
						break
					elseif ( trigItem.id == "chgfrmlvl" ) then
						section.eventchange["levelexit"] = dataItem
						break
					elseif ( trigItem.id == "label" ) then
						section["trigger"] = "label"
						section["label"] = dataItem
						section["eventdeath"] = nil
						section["eventchange"] = nil
						break
					elseif ( trigItem.id == "death" ) then
						section["trigger"] = trigItem.id
						section["label"] = "enemy death"
						section["eventlabel"] = nil
						section["eventchange"] = nil
						section["teleport"] = nil
						break
					elseif ( trigItem.id == "change" ) then
						section["trigger"] = trigItem.id
						section["label"] = "change level"
						section["eventlabel"] = nil
						section["eventdeath"] = nil
						section["teleport"] = nil
						break
					elseif ( trigItem.id == "obstacle" ) then
						section["trigger"] = trigItem.id
						section["label"] = "obstacle"
						section["eventlabel"] = nil
						section["eventdeath"] = nil
						section["eventchange"] = nil
						section["teleport"] = nil
						break
					elseif ( trigItem.id == "codeStart" ) then
						local data = modWP_Events.modcommon.Extract.SearchText(	line, modWP_Events.codemarkend, "[TEXT]" )
						if ( data ~= nil ) then
							in_code = false
							section.code[#section.code + 1] = line:sub((modWP_Events.codemarkstart:len() + 1), (#line - modWP_Events.codemarkend:len()))
						else
							in_code = true
						end
						break
					elseif ( trigItem.id == "codeEnd" ) then
						in_code = false
						break
					end
::PROCESS_EVENT_NEXT_TRIGGER::
				end	--	determine trigger type
			end	--	copy code contents
		end	--	loop through each line of text in section
		modWP_Events.AllEventData[key]["content"] = nil
		key, section = next(modWP_Events.AllEventData, key)
	until ( key == nil )

end

--	Process all label events - mark up those that are "teleport" events
--	(i.e. labels that move character between levels)
--	modWP_Events.AllEventData is populated, processed and ready for usage at end of function
function modWP_Events.ProcessLabelEvents()
	local level_a, level_action, level_b = "","",""
	local key, event = next(modWP_Events.AllEventData, nil)
	local srchTeleport = modWP_Events.patternTeleport
	repeat
		local processed = false
		if ( event.trigger ~= "label" ) then goto PROCESS_NEXT_LABEL_EVENT end
		for subkey, codeline in pairs( event.code ) do
			local isteleport = modWP_Events.modcommon.Extract.SearchText( codeline, srchTeleport.srchPatn, srchTeleport.extrctPatn )
			if (( isteleport == nil ) or ( not isteleport )) then goto PROCESS_NEXT_LABEL_CODELINE end
			event.trigger = "teleport"
			-- have teleport code line
			for subsubkey, pattern in pairs( modWP_Events.patterns_teleport ) do
				local value = modWP_Events.modcommon.Extract.SearchText( codeline, pattern.srchPatn, pattern.extrctPatn )
				if (( value == nil ) or ( not value )) then	goto PROCESS_NEXT_LABEL_TELEPORTPATTERN end
				local numberA, numberB = -1, -1
				level_a, level_action, level_b = value:match( pattern.srchPatn )
				if ( pattern.id == "value" ) then
					numberA = assert(tonumber(level_a))
					numberB = assert(tonumber(level_b))
					-- value b is valid - but is it useful?
					if ( level_action:gsub("%a", string.lower) == "from" ) then
						if ( numberA == event.level ) then
							event.teleport.lvlA = event.level
							event.teleport.lvlB = numberB
						elseif ( numberB == event.level ) then
							event.teleport.lvlA = event.level
							event.teleport.lvlB = numberA
						else
							local errstr = "\"modWP_Events.ProcessLabelEvents\""
										.. " - bad case - values are not numbers\n"
							io.stderr:write(errstr)
						end
						processed = true
					end
				elseif ( pattern.id == "special" ) then
					-- value b is invalid
					event.teleport.lvlA = event.level
					event.teleport.lvlB = assert(tonumber(level_a))
					processed = true
				end
::PROCESS_NEXT_LABEL_TELEPORTPATTERN::
			end
::PROCESS_NEXT_LABEL_CODELINE::
		end
		if ( processed == false ) then
			event["teleport"] = nil
		end
::PROCESS_NEXT_LABEL_EVENT::
	key, event = next(modWP_Events.AllEventData, key)
	until ( key == nil )
end

--	Read in FDRPG ReturnOfTux.droid data file
--	All droid information is saved into lua table.
function modWP_Events.ProcessData()
	modWP_Events.modcommon = require("modWPCommon")
	--	test for presence of source data files
	modWP_Events.files.events = tostring(modWP_Events.modcommon.paths.srcData .. modWP_Events.modcommon.datafiles["events"])
	modWP_Events.modcommon.Test.Files(modWP_Events.files)
	-- read file and process into table objects
	modWP_Events.EventFileData = modWP_Events.modcommon.Process.FileToLines( modWP_Events.files["events"] )
	modWP_Events.ParseFile()
	modWP_Events.ParseContents()
	modWP_Events.ProcessLabelEvents()
end

--	Print out NPC information based on selected verbosity.
function modWP_Events.Verbosity()
	if ((modWP_Events.modcommon.verbose) or (modWP_Events.modcommon.doubleverbose)) then
		io.stdout:write(modWP_Events.modcommon.VerboseHeader)
		io.stdout:write("modWP_Events\n")
		io.stdout:write("number of Events: " .. #modWP_Events.AllEventData .. "\n")
		io.stdout:write(modWP_Events.modcommon.VerboseHeader)
		if (modWP_Events.modcommon.doubleverbose) then
			modWP_Events.modcommon.Process.TblPrint( modWP_Events.AllEventData, nil, nil, "All Event Data" )
		end
	end
end

return modWP_Events
