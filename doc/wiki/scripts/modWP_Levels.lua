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
--	lua module for parsing FDRPG data files for ship/level data
local modWP_Levels = {}
--	modWPCommon reference
modWP_Levels.modcommon = {}
--	Variable will contain all parsed level information after completion of ProcessLevelData()
modWP_Levels.AllLevelData = {}
--	levels.dat file parsed line-by-line into table
modWP_Levels.LevelFileData = {}
--	Storage of 2D grid map of levels at ground
modWP_Levels.GridData = {}
--	table of level numbers at ground stored by level number
modWP_Levels.LevelsGround = {}
--	table of level numbers found not at ground stored by level number
modWP_Levels.LevelsNonGround = {}
--	table of level numbers used as tutorial levels stored by level number
modWP_Levels.LevelsTutorial = {}
--	table of level numbers used for debugging/development stored by level number
modWP_Levels.LevelsDebug = {}
--	names in table format for looping
modWP_Levels.TypeNames = {
	{ name = "LevelsGround", display = "Ground Levels"},
	{ name = "LevelsNonGround", display = "Non-Ground Levels"},
	{ name = "LevelsTutorial", display = "Tutorial Levels"},
	{ name = "LevelsDebug", display = "Debug Levels"},
}
--	table of level numbers found to be unconnected/unused in FDRPG stored by level number
modWP_Levels.LevelsUnreferenced = {}
--	table of filepaths to FDRPG files used for parsing level information
modWP_Levels.files = { levels="" }
-- names of required modules - "modWP_Events"
modWP_Levels.requiredModules = { moduleNames[2].id }
--	Default grid center - value will change after trimming
modWP_Levels.grid_center = {
	x = 11,
	y = 11
}
--	Default grid size - value will change after trimming
modWP_Levels.grid_max = {
	x = 21,
	y = 21
}

--	"struct" of level information for each level
modWP_Levels.leveldata = {
	levelnumber	= -1,
	levelname="",
	bgSong = "",
	is_groundlvl = false,
	is_tutorial	= false,
	is_debug	= false,
	is_random	= false,
	levelnumber_north = -1,
	levelnumber_east = -1,
	levelnumber_south = -1,
	levelnumber_west = -1,
	levelnumber_above = -1,
	levelnumber_below = {-1},
	xlen = 0,
	ylen = 0
}

--	text items for Level parsing and presentation
--	id) key, label) text to display loop) can use in loop when search for text
--	srchPatn) text/patter to use to find data
--	extrctPatn) how to extract found data
modWP_Levels.textLevel = {
	{ id = "levelnumber",       label = "Level Number",           loop = true,  srchPatn = "Levelnumber:%s*[%d]+",        extrctPatn = "[%d]+",  },
	{ id = "levelname",         label = "Level Name",             loop = true,  srchPatn = "Name of this level=",         extrctPatn = "[EOL]",  },
	{ id = "levelsize",         label = "Level Size",             loop = false, srchPatn = "" },
	{ id = "xlen",              label = "X: ",                    loop = true,  srchPatn = "xlen of this level:%s*[%d]+", extrctPatn = "[%d]+",  },
	{ id = "ylen",              label = "Y: ",                    loop = true,  srchPatn = "ylen of this level:%s*[%d]+", extrctPatn = "[%d]+",  },
	{ id = "bgSong",            label = "Background Song",        loop = true,  srchPatn = "BgSong=",                     extrctPatn = "[EOL]",  },
	{ id = "is_groundlvl",      label = "Level at Ground",        loop = false, srchPatn = "" },
	{ id = "is_tutorial",       label = "Tutorial Level",         loop = false, srchPatn = "tutorial",                    extrctPatn = "[TEXT]", },
	{ id = "is_debug",          label = "Debug Level",            loop = false, srchPatn = "debug level",                 extrctPatn = "[TEXT]", },
	{ id = "debugcomment",      label = "",                       loop = false, srchPatn = "%s+%-%-%s+",                  extrctPatn = "[TEXT]", },
	{ id = "is_unref",          label = "Unreferenced Level",     loop = false, srchPatn = "" },
	{ id = "is_random",         label = "Random Generated Level", loop = true,  srchPatn = "random dungeon:%s*[%d]+",     extrctPatn = "[%d]+",  },
	{ id = "levelnumber_north", label = "Level To North",         loop = true,  srchPatn = "jump target north:%s*[%d]+",  extrctPatn = "[%d]+",  },
	{ id = "levelnumber_east",  label = "Level To East",          loop = true,  srchPatn = "jump target east:%s*[%d]+",   extrctPatn = "[%d]+",  },
	{ id = "levelnumber_south", label = "Level To South",         loop = true,  srchPatn = "jump target south:%s*[%d]+",  extrctPatn = "[%d]+",  },
	{ id = "levelnumber_west",  label = "Level To West",          loop = true,  srchPatn = "jump target west:%s*[%d]+",   extrctPatn = "[%d]+",  },
	{ id = "levelnumber_above", label = "Level Above",            loop = false, srchPatn = "" },
	{ id = "levelnumber_below", label = "Levels Below",           loop = false, srchPatn = "" },
	{ id = "end_of_level",      label = "end_of_level",           loop = true,  srchPatn = "end_of_level",                extrctPatn = "[TEXT]", },
}

--	test if level is a debug level
--	[in]	levelnumber	level to be tested
--	[ret]	boolean - Y level is debug
function modWP_Levels.LevelIsDebug( levelnumber )
	local retval = false
	local levelindex = modWP_Levels.GetIndexByValue( modWP_Levels.AllLevelData, levelnumber, "levelnumber" )
	if (( levelindex ~= nil ) and ( levelindex > 0 )) then
		retval = modWP_Levels.AllLevelData[levelindex].is_debug
	end
	return retval
end

--	test if level is a tutorial level
--	[in]	levelnumber	level to be tested
--	[ret]	boolean - Y level is tutorial
function modWP_Levels.LevelIsTutorial( levelnumber )
	local retval = false
	local levelindex = modWP_Levels.GetIndexByValue( modWP_Levels.AllLevelData, levelnumber, "levelnumber" )
	if (( levelindex ~= nil ) and ( levelindex > 0 )) then
		retval = modWP_Levels.AllLevelData[levelindex].is_tutorial
	end
	return retval
end

--	Look up leveldata variable value and its presentation label
--	Assumes AllLevelData is populated and processed
--	[in]	levelitemdata	object under examination (type leveldata)
--	[in]	idvalue	search id - same as id value in textLevel
--	[ret]	pair of strings representing label|data from level item
function modWP_Levels.GetLevelItemStringsPair( levelitemdata, idvalue )
	local retLabel, retData = "", ""
	if (( idvalue == nil ) or ( type(idvalue) ~= 'string' ) or ( idvalue:len() <= 0 )) then
		return retLabel, retData
	end
	local labelitem = select(1,modWP_Levels.modcommon.Extract.GetTableItem( modWP_Levels.textLevel, "id", idvalue))
	if ( labelitem ~= nil ) then
		retLabel = labelitem.label
		if	( idvalue == "is_unref") then
			retData = ""
		elseif ( idvalue == "levelnumber_below" ) then
			local str = ""
			for key,value in pairs(levelitemdata[idvalue]) do
				if (key > 1) then
					str = str .. " " .. tostring(value)
				else
					str = tostring(value)
				end
			end
			retData = str
		elseif ( idvalue == "levelsize" ) then
			retData = string.format("%02d X %02d ", levelitemdata.xlen, levelitemdata.ylen )
		else
			retData = tostring(levelitemdata[idvalue])
		end
	end
	return retLabel, retData
end

--	Read in FDRPG levels and events data file.
--	Process extracted information for different level
--	information: ground, underground, debug and tutorial.
--	All level information is saved into lua tables.
--	A map of the levels at ground (GridData) is also produced.
function modWP_Levels.ProcessData()
	modWP_Levels.modcommon = require("modWPCommon")
	--	test for presence of source data files
	modWP_Levels.files.levels = tostring(modWP_Levels.modcommon.paths.srcMap .. modWP_Levels.modcommon.datafiles["levels"])
	modWP_Levels.modcommon.Test.Files(modWP_Levels.files)
	--	read levels.dat and process into table objects
	modWP_Levels.LevelFileData = modWP_Levels.modcommon.Process.FileToLines(modWP_Levels.files["levels"])
	modWP_Levels.ParseLevel()
	--	generate grid from parsed Level Data
	modWP_Levels.GridProcess()
	--	update level data to reflect all found ground levels
	--	if its referenced in the "map" grid - that level is at the same level as town or "ground level"
	--	populate modWP_Levels.LevelsGround -> making 1D table of levels at ground from grid
	for y = 1, modWP_Levels.grid_max.y do
		for x = 1, modWP_Levels.grid_max.x do
			local value = tonumber(modWP_Levels.GridSquareRead(x,y))
			if (value >= 0) then
				table.insert(modWP_Levels.LevelsGround, value)
			end
		end
	end
	if (#modWP_Levels.LevelsGround > 0) then
		table.sort(modWP_Levels.LevelsGround)
	end
	local templeveldata = {}
	for key,value in pairs(modWP_Levels.LevelsGround) do
		local index = modWP_Levels.GetIndexByValue( modWP_Levels.AllLevelData, value, "levelnumber" )
		templeveldata = modWP_Levels.AllLevelData[index]
		templeveldata.is_groundlvl = true
	end
	--	flag the rest - except debug/tutorial - as NON-ground levels (i.e. subterrainian)
	for key, LevelItem in pairs(modWP_Levels.AllLevelData) do
		if (not(LevelItem.is_groundlvl) and not(LevelItem.is_debug) and not(LevelItem.is_tutorial)) then
			table.insert(modWP_Levels.LevelsNonGround, LevelItem.levelnumber)
		end
	end
	modWP_Levels.events = require(modWP_Levels.requiredModules[1])
	assert( modWP_Levels.events ~= nil )
	--	find all transport events - used to map the vertical connections between levels
	for key, event in pairs(modWP_Levels.events.AllEventData) do
		if (( event.trigger == "teleport" ) and ( event.teleport ~= nil )) then
			local indexP = modWP_Levels.GetIndexByValue( modWP_Levels.AllLevelData, event.teleport.lvlA, "levelnumber" )
			local indexQ = modWP_Levels.GetIndexByValue( modWP_Levels.AllLevelData, event.teleport.lvlB, "levelnumber" )
			modWP_Levels.ProcessParentChild( indexP, indexQ )
		end
	end
	--	found all ground/non-ground/debug/tutorial levels
	--	anything left over is "unreferenced" - flag and update tables with this data
	for key,LevelItem in pairs(modWP_Levels.AllLevelData) do
		if (not(modWP_Levels.hasreferenceIndex(key))) then
			modWP_Levels.LevelsUnreferenced[#modWP_Levels.LevelsUnreferenced + 1] = LevelItem.levelnumber
			--	remove reference of this level in other tables
			for subkey, tbl in pairs(modWP_Levels.TypeNames) do
				local levelindex = modWP_Levels.GetIndexByValue(modWP_Levels[tbl.name],LevelItem.levelnumber)
				if (levelindex > 0) then
					table.remove(modWP_Levels[tbl.name],levelindex)
				end
			end	--	loop through other tables
		end	--	Level as referenece
		LevelItem["urlAnchor"] = modWP_Levels.modcommon.Wiki.HLink .. modWP_Levels.modcommon.Wiki.WikifyLink( "level" .. LevelItem.levelnumber )
	end	--	loop through AllLevelData
	modWP_Levels.GridTrim()
end

-- retrieve the anchortext associated with this id value
function modWP_Levels.GetItemUrlText( idvalue )
	local retText = ""
	if ( not idvalue ) then
		return retText
	end
	local index = 	modWP_Levels.GetIndexByValue( modWP_Levels.AllLevelData, idvalue, "levelnumber" )
	if ( index ) then
		local item = modWP_Levels.AllLevelData[index]
		retText = modWP_Levels.modcommon.outputfilenames.levels .. item.urlAnchor
	end
	return retText
end

--	Process Level Above/Below for each level in AllLevelData table
--	Action to update AllLevelData - level above/below information.
--	Assumes AllLevelData is populated and processed.
--	[in]	indexP	index value of level in AllLevelData to be processed
--	[in]	indexQ	index value of level in AllLevelData to be processed
function modWP_Levels.ProcessParentChild( indexP, indexQ )
	local parentlvl = -1
	local childlvl = -1
	local parentdata = {}
	local childdata = {}
	local parentIsP = false
	--	determine which of these two numbers is the "parent" level (above)
	local isgroundP = (modWP_Levels.AllLevelData[indexP].is_groundlvl)
	local isgroundQ = (modWP_Levels.AllLevelData[indexQ].is_groundlvl)
	if ((isgroundP) and (not(isgroundQ))) then
		parentIsP = true
	elseif ((not(isgroundP)) and (isgroundQ)) then
		parentIsP = false
	else
		--	which level is closer to ground level?
		--	and would indicate it has been processed/touched
		if (       (modWP_Levels.AllLevelData[indexP].levelnumber_above >= 0)
			and not(modWP_Levels.AllLevelData[indexQ].levelnumber_above >= 0)) then
			parentIsP = true
		elseif (not(modWP_Levels.AllLevelData[indexP].levelnumber_above >= 0)
				and (modWP_Levels.AllLevelData[indexQ].levelnumber_above >= 0)) then
			parentIsP = false
		else
			--	unable to determine parent level
			parentIsP = nil
		end
	end
	if (parentIsP ~= nil) then
		if (parentIsP) then
			--	process modWP_Levels.AllLevelData[indexP] as parent
			--	process modWP_Levels.AllLevelData[indexQ] as child
			parentdata = modWP_Levels.AllLevelData[indexP]
			childdata = modWP_Levels.AllLevelData[indexQ]
		else
			--	process modWP_Levels.AllLevelData[indexP] as child
			--	process modWP_Levels.AllLevelData[indexQ] as parent
			parentdata = modWP_Levels.AllLevelData[indexQ]
			childdata = modWP_Levels.AllLevelData[indexP]
		end
		parentlvl = parentdata.levelnumber
		childlvl = childdata.levelnumber
		--process child/parent values
		local foundlvl = false
		if (not modWP_Levels.hasChildren(parentdata.levelnumber)) then
			parentdata.levelnumber_below = {}
			parentdata.levelnumber_below[#parentdata.levelnumber_below + 1] = childlvl
			foundlvl = true
		else
			for key,lvldata in pairs(parentdata.levelnumber_below) do
				if (lvldata == childlvl) then
					--	found the child level number in the list
					foundlvl = true
					break
				end
			end
		end
		if (not(foundlvl)) then
			parentdata.levelnumber_below[#parentdata.levelnumber_below + 1] = childlvl
		end
		childdata.levelnumber_above = parentlvl
		if (parentIsP) then
			modWP_Levels.AllLevelData[indexP] = parentdata
			modWP_Levels.AllLevelData[indexQ] = childdata
		else
			modWP_Levels.AllLevelData[indexP] = childdata
			modWP_Levels.AllLevelData[indexQ] = parentdata
		end
	end	--	test flag parentIsP
end

--	Find the location of a level number in a table
--	[in]	tabletosearch	table to search for levelnumbertosearch
--	[in]	levelnumbertosearch	level number of level to find
--	[ret]	table index or -1 if not found
function modWP_Levels.GetIndexByValue( tabletosearch, levelnumbertosearch, keyvalue )
	local index = -1
	for key,value in pairs(tabletosearch) do
		local match = false
		if ( keyvalue ~= nil ) then
			match = (value[keyvalue] == levelnumbertosearch)
		else
			match = (value == levelnumbertosearch)
		end
		if (match) then
			index = key
			break
		end
	end
	return index
end

--	Determine if level under examination has levels below ( using level number )
--	[in]	levelnumbertosearch level number of level under examination
--	[ret]	boolean (True = level has levels below)
function modWP_Levels.hasChildren( levelnumbertosearch )
	local index = modWP_Levels.GetIndexByValue( modWP_Levels.AllLevelData, levelnumbertosearch, "levelnumber" )
	return modWP_Levels.hasChildrenIndex(index)
end

--	Determine if level under examination has levels below ( using table index )
--	[in]	location index in AllLevelData table of level under examination
--	[ret]	boolean (True = level has levels below)
function modWP_Levels.hasChildrenIndex( index )
	local boolreturn = false
	if (modWP_Levels.AllLevelData[index].levelnumber ~= -1) then
		--	not a valid level item
		for key,value in pairs(modWP_Levels.AllLevelData[index].levelnumber_below) do
			if (value >= 0) then
				boolreturn = true
				break
			end
		end
	end
	return boolreturn
end
--	Determine if level is connected to any another levels ( using level number )
--	check the processed level data for connections N/S/E/W, above and below.
--	[in]	levelnumbertosearch	level number to be examined
--	[ret]	boolean - does level have any connections? (true = yes)
function modWP_Levels.hasreference( levelnumbertosearch )
	local index = modWP_Levels.GetIndexByValue( modWP_Levels.AllLevelData, levelnumbertosearch, "levelnumber" )
	return modWP_Levels.hasreferenceIndex(index)
end
--	Determine if level is connected to another level ( using table index )
--	check the processed level data for connections N/S/E/W, above and below
--	[in]	index	table index in AllLevelData of level to be examined
--	[ret]	boolean - does level have any connections? (true = yes)
function modWP_Levels.hasreferenceIndex( index )
	local hasref = false
	if (index >= 1) then
		hasref	=	((modWP_Levels.AllLevelData[index].levelnumber_north >= 0)
					or	(modWP_Levels.AllLevelData[index].levelnumber_east >= 0)
					or	(modWP_Levels.AllLevelData[index].levelnumber_south >= 0)
					or	(modWP_Levels.AllLevelData[index].levelnumber_west >= 0)
					or	(modWP_Levels.AllLevelData[index].levelnumber_above >= 0)
					or	(modWP_Levels.AllLevelData[index].is_tutorial)
					or	(modWP_Levels.AllLevelData[index].is_debug)
					or	modWP_Levels.hasChildrenIndex(index)
					)
	end
	return hasref
end

--	Process file data for all level information
--	Populates AllLevelData variable
function modWP_Levels.ParseLevel()
	local templeveldataitem = modWP_Levels.modcommon.Extract.TblDeepCopy(modWP_Levels.leveldata)
	for key,line in pairs(modWP_Levels.LevelFileData) do
		for key, textitem in pairs(modWP_Levels.textLevel) do
			if ( not textitem.loop ) then goto PARSE_LEVEL_NEXT_PATTERN end
			--	pattern can be used in a loop
			local value = modWP_Levels.modcommon.Extract.SearchText( line, textitem.srchPatn, textitem.extrctPatn )
			if ( not value ) then goto PARSE_LEVEL_NEXT_PATTERN end
			--	data returned from search/extract
			if ( textitem.id == "end_of_level" ) then
				if ( value == textitem.id ) then
					modWP_Levels.AllLevelData[#modWP_Levels.AllLevelData + 1] = templeveldataitem
					templeveldataitem = nil
					templeveldataitem = modWP_Levels.modcommon.Extract.TblDeepCopy(modWP_Levels.leveldata)
				end
			elseif ( textitem.id == "levelname" ) then
				templeveldataitem[textitem.id] = value
				value = string.lower(templeveldataitem.levelname)
				local subitem = select(1,modWP_Levels.modcommon.Extract.GetTableItem( modWP_Levels.textLevel, "id", "is_tutorial" ))
				local subvalue = modWP_Levels.modcommon.Extract.SearchText( value, subitem.srchPatn, subitem.extrctPatn )
				if ( subvalue ) then
					templeveldataitem.is_tutorial = true
					table.insert(modWP_Levels.LevelsTutorial, templeveldataitem.levelnumber)
				end
				local subitem = select(1,modWP_Levels.modcommon.Extract.GetTableItem( modWP_Levels.textLevel, "id", "is_debug" ))
				local subvalue = modWP_Levels.modcommon.Extract.SearchText( value, subitem.srchPatn, subitem.extrctPatn )
				if ( subvalue ) then
					templeveldataitem.is_debug = true
					table.insert(modWP_Levels.LevelsDebug, templeveldataitem.levelnumber)
					--	remove everything after " -- " -> debug commentary
					textpattern = select(1,modWP_Levels.modcommon.Extract.GetTableItem( modWP_Levels.textLevel, "id", "debugcomment"))
					local debug_start = select(1,templeveldataitem.levelname:find(textpattern.srchPatn))
					if (debug_start) then
						templeveldataitem.levelname = templeveldataitem.levelname:sub(1,(debug_start - 1))
					end
				end
			elseif ( textitem.id == "is_random" ) then
				templeveldataitem.is_random = (value ~= 0)
			else
				templeveldataitem[textitem.id] = value
			end --	process found data based on id
			break	--	goto next line of text
::PARSE_LEVEL_NEXT_PATTERN::
		end	--	loop through textLevel table looking for patterns
	end	--	loop through level file data
end

--	Grid Function - Process level data parsed from file into a 2D grid of levels at ground level
--	GridProcess takes the populated AllLevelData variable and attempts to build a 2D grid \"map\" of levels at ground.
--
--	Process starts with an arbitrary grid containing values all set to -1. The grid is sized to the values in grid_max.
--
--	Processing starts with the Level 0 (or \"Town" Level) located at variable grid_center. The level numbers in the
--	surrounding squares (N, E, S and W) are populated (GridSquarePopulateCross()). The \"focus\" (or CurGridLoc) is
--	changed and the process is repeated until the entire grid is populated.
--
--	Changing \"focus\" is done in a spiral movement outwards from Level 0 moving in the sequence of directions
--	S -> E -> N -> W - uses fn GridSquareChangeFocus(). Spiral movement is achieved by changing the value of curDirWriteLimit.
--	When curDirWriteLimit is equal to MaxWriteLimit(how far to write for a given direction), the direction of movement for
--	focus is changed (S -> E -> N -> W).
--
--	For example, after writing the town square, focus is moved south one, east one.
--	curDirWriteLimit is found to be equal to MaxWriteLimit. MaxWriteLimit is incremented. curDirWriteLimit is reset to 1.
--	Focus is moved north then west. curDirWriteLimit increments to two - equal to MaxWriteLimit. MaxWriteLimit is incremented.
--	curDirWriteLimit is reset to 1. Focus is moved south then east. The sequence is repeated until the grid is populated.
--
--	If grid square under focus does not have a level number( value is -1), focus is changed and processing continues with
--	the next square in sequence.
--
--	The function GridSquarePopulate() prevents bad data being stored by limiting stored values to be > 1.
--
--	Note: grid definition -> Y is row data and X is column data.
--	If accessing the grid directly, do so by using GridData[y][x], or use the function GridSquareRead().
function modWP_Levels.GridProcess()
	--	populate the grid with default value
	for y = 1, modWP_Levels.grid_max.y do
		modWP_Levels.GridData[y] = {}
		for x = 1, modWP_Levels.grid_max.x do
			modWP_Levels.GridSquarePopulate( x, y, -1 )
		end
	end
	--	fill in center square - town
	local CurGridLoc = { x = modWP_Levels.grid_center.x, y = modWP_Levels.grid_center.y }
	local CurLvlIndex = modWP_Levels.GetIndexByValue( modWP_Levels.AllLevelData, 0 , "levelnumber" )
	modWP_Levels.GridSquarePopulate(CurGridLoc.x, CurGridLoc.y, modWP_Levels.AllLevelData[CurLvlIndex].levelnumber)
	modWP_Levels.GridSquarePopulateCross(CurGridLoc.x, CurGridLoc.y, modWP_Levels.AllLevelData[CurLvlIndex])
	--	now fill in map levels for the rest of grid
	local MaxWriteLimit = 2 * (modWP_Levels.grid_center.x - 2) + 1
	local curDirWriteLimit = 1
	local curdir = 1	--	start filling in other grid squares going south
	CurGridLoc.x, CurGridLoc.y = modWP_Levels.GridSquareChangeFocus(CurGridLoc.x , CurGridLoc.y, curdir)
	local curlvl = modWP_Levels.GridSquareRead(CurGridLoc.x, CurGridLoc.y)
	--	we spiral outwards from center(town - level 0) going S(1)/E(2)/N(3)/W(4)
	local countOfWriteCurDir = 1
	local index = -1
	while (curDirWriteLimit < MaxWriteLimit) do
		if (curlvl >= 0) then
			index = modWP_Levels.GetIndexByValue( modWP_Levels.AllLevelData, curlvl , "levelnumber" )
			modWP_Levels.GridSquarePopulateCross(CurGridLoc.x, CurGridLoc.y, modWP_Levels.AllLevelData[index])
		end
		countOfWriteCurDir = countOfWriteCurDir + 1
		if (countOfWriteCurDir > curDirWriteLimit) then
			curdir = curdir + 1
			if (curdir > 4) then
				curdir = 1
			end
			if ((curdir == 1) or (curdir == 3)) then
				curDirWriteLimit = curDirWriteLimit + 1
			end
			countOfWriteCurDir = 1
		end
		if (curlvl < 0) then
			CurGridLoc.x, CurGridLoc.y = modWP_Levels.GridSquareChangeFocus(CurGridLoc.x , CurGridLoc.y, curdir)
			curlvl = modWP_Levels.GridSquareRead(CurGridLoc.x, CurGridLoc.y)
		else
			curlvl = modWP_Levels.GridGetNextSquare(modWP_Levels.AllLevelData[index], curdir)
			CurGridLoc.x, CurGridLoc.y = modWP_Levels.GridSquareChangeFocus(CurGridLoc.x , CurGridLoc.y, curdir)
		end
	end
end

--	Grid Function - Read Level Number at a grid location
--	This is a convenience function to hide the details of grid construction.
--	See description of GridProcess()
--	[in]	localx	X coordinate of grid square data to read
--	[in]	localy	Y coordinate  of grid square data to read
--	[ret]	Level number found stored at grid square(x,y)
function modWP_Levels.GridSquareRead( localx, localy )
	return modWP_Levels.GridData[localy][localx]
end
--	Grid Function - Write Level Number to a grid location
--	This is a convenience function to hide the details of grid construction.
--	See description of GridProcess()
--	[in]	localx	X coordinate of grid square data to write
--	[in]	localy	Y coordinate  of grid square data to write
--	[in]	value	Level number to be written at grid square(x,y)
function modWP_Levels.GridSquarePopulate( localx, localy, value )
	local gridvalue = modWP_Levels.GridData[localy][localx]
	if (gridvalue ~= value) then
		modWP_Levels.GridData[localy][localx] = value
	end
end

--	Grid Function - Populate grid data in a N/S/E/W direction from current grid square in focus
--	[in]	cross_center_x	X coordinate of current grid square being processed
--	[in]	cross_center_y	Y coordinate of current grid square being processed
--	[in]	lvldata	Level data associated with the grid square being processed
function modWP_Levels.GridSquarePopulateCross( cross_center_x, cross_center_y, lvldata )
	modWP_Levels.GridSquarePopulate(cross_center_x,			(cross_center_y + 1),	lvldata.levelnumber_south)
	modWP_Levels.GridSquarePopulate((cross_center_x + 1),	cross_center_y,			lvldata.levelnumber_east)
	modWP_Levels.GridSquarePopulate(cross_center_x,			(cross_center_y - 1),	lvldata.levelnumber_north)
	modWP_Levels.GridSquarePopulate((cross_center_x - 1),	cross_center_y,			lvldata.levelnumber_west)
end

--	Grid Function - Change the grid focus for next grid square to be processed
--	[in]	grid_locx	X coordinate of current grid square
--	[in]	grid_locy	Y coordinate of current grid square
--	[in]	curdir	direction of processing
--	== direction can be one of four values: 1 = south, 2 = east, 3 = north, 4 = west.
--	[ret]	table of x,y coordinates of grid square to be processed next
function modWP_Levels.GridSquareChangeFocus( grid_locx, grid_locy, curdir )
	if (curdir == 1) then
		--	south
		grid_locx = grid_locx
		grid_locy = grid_locy + 1
	elseif (curdir == 2) then
		--	east
		grid_locx = grid_locx + 1
		grid_locy = grid_locy
	elseif (curdir == 3) then
		--	north
		grid_locx = grid_locx
		grid_locy = grid_locy - 1
	elseif (curdir == 4) then
		--	west
		grid_locx = grid_locx - 1
		grid_locy = grid_locy
	else	--	shouldn't receive direction outsite of 1 <= curdir <= 4
		grid_locx = -1
		grid_locy = -1
	end
	return grid_locx, grid_locy
end

--	Grid Function - Determine the next level for grid processing
--	[in]	lvldata	current level being processed
--	[in]	curdir	direction of processing
--	== direction can be one of four values: 1 = south, 2 = east, 3 = north, 4 = west.
--	[ret]	level number that is to be processed next OR -1 if not determined
function modWP_Levels.GridGetNextSquare( lvldata, curdir )
	local nextlvl = -1
	if (curdir == 1) then
		nextlvl = lvldata.levelnumber_south
	elseif (curdir == 2) then
		nextlvl = lvldata.levelnumber_east
	elseif (curdir == 3) then
		nextlvl = lvldata.levelnumber_north
	elseif (curdir == 4) then
		nextlvl = lvldata.levelnumber_west
--	else	--	shouldn't receive direction outsite of 1 <= curdir <= 4
	end
	return nextlvl
end

--	Grid Function - Examine global GridData (ground levels) and trim excess rows/columns
--	see GridDataIsTrimmable() for definition of 'Trimmable'
function modWP_Levels.GridTrim()
	local trim_columns = {}
	local trim_rows = {}
	local size = {x = modWP_Levels.grid_max.x, y = modWP_Levels.grid_max.y}
	local center = {x = modWP_Levels.grid_center.x, y = modWP_Levels.grid_center.y}
	--	find all columns that can be trimmed
	for y = size.y, 1, -1 do
		if (modWP_Levels.GridDataIsTrimmable(y, false)) then
			table.insert(trim_rows, y)
		end
	end
	table.sort(trim_rows)
	--	find all rows that can be trimmed
	for x = size.x, 1, -1 do
		if (modWP_Levels.GridDataIsTrimmable(x, true)) then
			table.insert(trim_columns, x)
		end
	end
	table.sort(trim_columns)
	--	trim rows
	for value = #trim_rows, 1, -1 do
		local rowToRemove = trim_rows[value]
		table.remove(modWP_Levels.GridData, rowToRemove)
		if (rowToRemove < center.y) then
			center.y = center.y - 1
		end
		size.y = size.y - 1
	end
	--	trim columns
	for value = #trim_columns, 1, -1 do
		local columnToRemove = trim_columns[value]
		for n = 1, size.y do
			--	for each row - remove the column
			table.remove(modWP_Levels.GridData[n],columnToRemove )
		end
		if (columnToRemove < center.x) then
			center.x = center.x - 1
		end
		size.x = size.x - 1
	end
	modWP_Levels.grid_max = { x = size.x, y = size.y }
	modWP_Levels.grid_center = { x = center.x, y = center.y }
end

--	Grid Function - Determine if row/column in global GridData can be trimmed
--	Definition of a "Trimmable" column/row
--	element[j][i] = -1 for all j[1 to #col] or [1 to #row]i
--	[in]	index	row/column index number in grid to be examined
--	[in]	isrow	bool:	true - index is a row value
--							false - index is a column value
--	[ret]	bool - Is row/column "Trimmable" (True = Yes)
function modWP_Levels.GridDataIsTrimmable( index, isrow )
	local cantrim = false
	--	check incoming values are useful
	if ((index == nil) or (isrow == nil)) then
		return false
	end
	local valuefirst = -1
	if (isrow) then
		--	checking against row data
		if ((index <= modWP_Levels.grid_max.y) and
			(index >= 1)) then
			for dirY = 1, modWP_Levels.grid_max.y do
				valuefirst = modWP_Levels.GridSquareRead(index, dirY)
				if (valuefirst >= 0) then
					cantrim = false
					break
				else
					cantrim = true
				end	--	check grid data at {[index | secondIndex], dirY}
			end	--	for loop on grid Y dir
		end	--	check index value will not cause read beyond grid bounds
	else
		--	checking against column data
		if ((index <= modWP_Levels.grid_max.x) and
			(index >= 1)) then
			for dirX = 1, modWP_Levels.grid_max.x do
				valuefirst = modWP_Levels.GridSquareRead(dirX, index)
				if (valuefirst >= 0) then
					cantrim = false
					break
				else
					cantrim = true
				end --	check grid data at {dirX, [index | secondIndex]}
			end	--	for loop on grid X dir
		end	--	check index value will not cause read beyond grid bounds
	end	--	is index a row or column value
	return cantrim
end

--	Build up wiki string of underground FDRPG levels
--	This function is used recursively!
--	[in]	levelnumbertosearch	examine all children of this level
--	[in]	indent	amount of push in text from left margin
--	[ret]	text string in wiki format of underground levels for this levelnumber
function modWP_Levels.TraverseLevels( levelnumbertosearch, indent )
	local modWIKI = modWP_Levels.modcommon.Wiki
	indent = indent or 0
	local str = ""
	if (levelnumbertosearch >= 0) then
		local leveltext = modWP_Levels.WikiEntryLevelAnchorText( levelnumbertosearch )
		local levelindex = modWP_Levels.GetIndexByValue( modWP_Levels.AllLevelData, levelnumbertosearch , "levelnumber" )
		local strindent = string.rep("&emsp;", indent)
		str = strindent .. leveltext .. modWIKI.LineBreakEnd .. "\n"
		if (modWP_Levels.hasChildrenIndex(levelindex)) then
			for key, value in pairs(modWP_Levels.AllLevelData[levelindex].levelnumber_below) do
				str =  str .. modWP_Levels.TraverseLevels(value,(indent + 2))
			end
		else
			str = str .. "\n"
		end
	end
	return str
end

--	Write FDRPG level information to file in a wiki format
function modWP_Levels.WikiWrite()
	local modWIKI = modWP_Levels.modcommon.Wiki
	local LI = modWIKI.LI
	local SEP = modWIKI.Seperator
	local colourCryo = "color=#5f5fff"
	local colourTown = "green"
	local filename = modWP_Levels.modcommon.outputfilenames.levels
	local filepath = tostring(modWP_Levels.modcommon.paths.destRootFile .. filename)

	local wikitbl = {
		{ name = "LevelsDebug", link = "lvlsmapdebug", head = "Debug Levels", verbage = "Following level(s) are used for game testing/debug only:"},
		{ name = "LevelsTutorial", link = "lvlsmaptutorial", head = "Tutorial Levels", verbage = "Following level(s) are used as tutorials for helping users learn how to play:"},
		{ name = "LevelsUnreferenced", link = "lvlsmapunref", head = "Unreferenced Levels", verbage = "Following level(s) were found to be unconnected in the level map:"}
	}

	local wikitext = {}
	wikitext[#wikitext + 1] = modWIKI.PageSummary("FreedroidRPG Level Maps")
	wikitext = modWIKI.WarnAutoGen( wikitext )	
	--	make menu for levels
	wikitext[#wikitext + 1] = modWIKI.FrameStartRight("font-size:smaller")
	wikitext[#wikitext + 1] = modWIKI.HeaderLevel(3) .. "Freedroid RPG Levels"
	wikitext[#wikitext + 1] = modWIKI.LinkText(modWIKI.HLink .. "mapship", "Ship Map")
	wikitext[#wikitext + 1] = LI .. modWIKI.LinkText(modWIKI.HLink .. "lvlsmapground", "Map of Ground Levels")
	wikitext[#wikitext + 1] = LI .. modWIKI.LinkText(modWIKI.HLink .. "lvlsmapunder", "Map of Underground Levels")
	for key, tbl in pairs(wikitbl) do
		wikitext[#wikitext + 1] = LI .. modWIKI.LinkText(modWIKI.HLink .. tbl.link, tbl.head)
	end
	wikitext[#wikitext + 1] = LI .. modWIKI.LinkText(modWIKI.HLink .. "lvlslistnumber", "List of All FDRPG Levels")
	wikitext[#wikitext + 1] = modWIKI.FrameEnd

	wikitext = modWIKI.WarnSpoil( wikitext )
	--	page contents start here
	wikitext[#wikitext + 1] = modWIKI.LinkText(modWIKI.HLink .. "mapship")
	wikitext[#wikitext + 1] = modWIKI.HeaderLevel(1) .. "Freedroid RPG Levels"
	wikitext[#wikitext + 1] = modWIKI.LinkText(modWIKI.HLink .. "lvlsmapground")
	wikitext[#wikitext + 1] = modWIKI.HeaderLevel(2) .. "Map of Ground Levels"
	--	convert 'trimmed' grid to table
	--	structured table
	wikitext[#wikitext + 1] = modWIKI.TableStart("border=1 cellpadding=1 cellspacing=0 align=center width=70%")
	local cellpct = math.floor(100 / modWP_Levels.grid_max.x)
	for y = 1, modWP_Levels.grid_max.y do
		local firstcell = true
		for x = 1, modWP_Levels.grid_max.x do
			local levelnum = modWP_Levels.GridSquareRead(x,y)
			local levelindex = modWP_Levels.GetIndexByValue( modWP_Levels.AllLevelData, levelnum , "levelnumber" )
			local levelname = ""
			local cellmarker = ""
			local cellstyle = "width=" .. cellpct .. "% align=center"
			if (firstcell) then
				cellmarker = modWIKI.TableRowStart(cellstyle)
				firstcell = false
			else
				cellmarker = modWIKI.TableRowAppend(cellstyle)
			end
			if (levelnum == -1) then
				wikitext[#wikitext + 1] = cellmarker .. string.rep("&nbsp;",6)
			else
				local levelcolour = nil
				if (levelnum == 0) then
					--	town
					levelcolour = colourTown
				elseif (levelnum == 12) then
					--	cryo facility
					levelcolour = colourCryo
				end
				levelname = modWP_Levels.AllLevelData[levelindex].levelname
				local leveltext = modWP_Levels.LevelShortLinkText(levelnum)
								.. modWIKI.LineBreakEnd	.. "\n"
								.. modWIKI.TextColour( levelname, levelcolour )
				wikitext[#wikitext + 1] =	cellmarker .. leveltext
			end
		end
	end
	wikitext[#wikitext + 1] = modWIKI.TableEnd
	--	map of underground levels
	wikitext[#wikitext + 1] = modWIKI.LinkText(modWIKI.HLink .. "lvlsmapunder")
	wikitext[#wikitext + 1] = modWIKI.HeaderLevel(2) .. "Map of Underground Levels"
	--	for each level in ground levels - find if level has children
	for key, levelnumber in pairs(modWP_Levels.LevelsGround)do
		if (modWP_Levels.hasChildren(levelnumber)) then
			wikitext[#wikitext + 1] = modWP_Levels.TraverseLevels(levelnumber)
		end
	end

	--	debug/tutorial/unreferenced levels
	for key, tbl in pairs(wikitbl) do
		wikitext[#wikitext + 1] = modWIKI.LinkText(modWIKI.HLink .. tbl.link)
		wikitext[#wikitext + 1] = modWIKI.HeaderLevel(2) .. tbl.head
		wikitext[#wikitext + 1] = tbl.vergabe
		for key, levelnumber in pairs(modWP_Levels[tbl.name]) do
			wikitext[#wikitext + 1] = LI .. modWP_Levels.WikiEntryLevelAnchorText(levelnumber)
		end
	end
	--	list of all levels in FDRPG (by level number)
	wikitext[#wikitext + 1] = modWIKI.LinkText(modWIKI.HLink .. "lvlslistnumber")
	wikitext[#wikitext + 1] = modWIKI.HeaderLevel(2) .. "List of All FDRPG Levels"
	for key, levelitem in pairs(modWP_Levels.AllLevelData)do
		--	processing for surrounding levels
		local index_north = modWP_Levels.GetIndexByValue( modWP_Levels.AllLevelData, levelitem.levelnumber_north, "levelnumber" )
		local index_east = modWP_Levels.GetIndexByValue( modWP_Levels.AllLevelData, levelitem.levelnumber_east, "levelnumber" )
		local index_south = modWP_Levels.GetIndexByValue( modWP_Levels.AllLevelData, levelitem.levelnumber_south, "levelnumber" )
		local index_west = modWP_Levels.GetIndexByValue( modWP_Levels.AllLevelData, levelitem.levelnumber_west, "levelnumber" )
		local level_northeast = -1
		local level_northwest = -1
		local level_southeast = -1
		local level_southwest = -1
		if (index_north > 0) then
			level_northeast = modWP_Levels.AllLevelData[index_north].levelnumber_east
			level_northwest = modWP_Levels.AllLevelData[index_north].levelnumber_west
		else
			if (index_east > 0) then
				level_northeast = modWP_Levels.AllLevelData[index_east].levelnumber_north
			end
			if (index_west > 0) then
				level_northwest = modWP_Levels.AllLevelData[index_west].levelnumber_north
			end
		end
		if (index_south > 0) then
			level_southeast = modWP_Levels.AllLevelData[index_south].levelnumber_east
			level_southwest = modWP_Levels.AllLevelData[index_south].levelnumber_west
		else
			if (index_east > 0) then
				level_southeast = modWP_Levels.AllLevelData[index_east].levelnumber_south
			end
			if (index_west > 0) then
				level_southwest = modWP_Levels.AllLevelData[index_west].levelnumber_south
			end
		end
		local levelcolour = nil
		if (levelitem.levelnumber == 0) then
			--	town
			levelcolour = colourTown
		elseif (levelitem.levelnumber == 12) then
			--	cryo facility
			levelcolour = colourCryo
		end
		wikitext[#wikitext + 1] = modWIKI.LinkText(levelitem.urlAnchor)
		local writedata = modWP_Levels.WikiEntryLevelText(levelitem.levelnumber)
		wikitext[#wikitext + 1] = modWIKI.HeaderLevel(5) .. modWIKI.TextColour(writedata, levelcolour)
		--	display surrounding levels
		wikitext[#wikitext + 1] = modWIKI.TableStart("border=1 cellpadding=0 cellspacing=0 align=\"left\" width=15%")
		wikitext[#wikitext + 1] = modWIKI.TableRowStart("align=center") .. modWP_Levels.LevelShortLinkText( level_northwest )
		wikitext[#wikitext + 1] = modWIKI.TableRowAppend("align=center") .. modWP_Levels.LevelShortLinkText( levelitem.levelnumber_north )
		wikitext[#wikitext + 1] = modWIKI.TableRowAppend("align=center") .. modWP_Levels.LevelShortLinkText( level_northeast )
		wikitext[#wikitext + 1] = modWIKI.TableRowStart("align=center") .. modWP_Levels.LevelShortLinkText( levelitem.levelnumber_west )
		local levelnumberdata = string.format("%02d", tostring(levelitem.levelnumber))
		wikitext[#wikitext + 1] = modWIKI.TableRowAppend("align=center") .. modWIKI.TextEmbed(levelnumberdata,"boldemphasis")
		wikitext[#wikitext + 1] = modWIKI.TableRowAppend("align=center") .. modWP_Levels.LevelShortLinkText( levelitem.levelnumber_east )
		wikitext[#wikitext + 1] = modWIKI.TableRowStart("align=center") .. modWP_Levels.LevelShortLinkText( level_southwest )
		wikitext[#wikitext + 1] = modWIKI.TableRowAppend("align=center") .. modWP_Levels.LevelShortLinkText( levelitem.levelnumber_south )
		wikitext[#wikitext + 1] = modWIKI.TableRowAppend("align=center") .. modWP_Levels.LevelShortLinkText( level_southeast )
		wikitext[#wikitext + 1] = modWIKI.TableEnd
		--	display details about current level
		wikitext[#wikitext + 1] = modWIKI.FrameStartLeft("border=\'0px\' margin-top=\'0px\' margin-left=\'1.0em\'")
		local levellabel, leveldata = "", ""
		if ( not modWP_Levels.hasreference(levelitem.levelnumber) ) then
			levellabel = select( 1, modWP_Levels.GetLevelItemStringsPair( levelitem, "is_unref" ))
			wikitext[#wikitext + 1]	= modWIKI.TextEntry(levellabel, nil, nil, modWIKI.ColourWarn)
		end --	level is unreferenced
		if (levelitem.is_random) then
			levellabel = select( 1, modWP_Levels.GetLevelItemStringsPair( levelitem, "is_random" ))
			wikitext[#wikitext + 1]	= modWIKI.TextEntry(levellabel, nil, nil, modWIKI.ColourCaution)
		end --	level is random
		if (levelitem.is_tutorial) then
			levellabel = select( 1, modWP_Levels.GetLevelItemStringsPair( levelitem, "is_tutorial" ))
			wikitext[#wikitext + 1]	= modWIKI.TextEntry(levellabel, nil, nil, modWIKI.ColourCaution)
		end --	level is tutorial
		if (levelitem.is_debug) then
			levellabel = select( 1, modWP_Levels.GetLevelItemStringsPair( levelitem, "is_debug" ))
			wikitext[#wikitext + 1]	= modWIKI.TextEntry(levellabel, nil, nil, modWIKI.ColourCaution)
		end --	level is debug
		--	level size
		local levellabel = select(1, modWP_Levels.GetLevelItemStringsPair( levelitem, "levelsize" ))
		local sizelabelx,sizedatax = modWP_Levels.GetLevelItemStringsPair( levelitem, "xlen" )
		local sizelabely,sizedatay = modWP_Levels.GetLevelItemStringsPair( levelitem, "ylen" )
		local sizetext = sizelabelx .. "&nbsp;".. sizedatax	.. "&emsp;"	.. sizelabely .. "&nbsp;" .. sizedatay
		wikitext[#wikitext + 1] = modWIKI.TextEntry( levellabel, sizetext, SEP )
		--	level song
		levellabel, leveldata = modWP_Levels.GetLevelItemStringsPair( levelitem, "bgSong" )
		wikitext[#wikitext + 1] = modWIKI.TextEntry( levellabel, leveldata, SEP , nil, false)
		wikitext[#wikitext + 1] = " "	--	<--keep to force line break between bgsound and next element
		if ( levelitem.levelnumber_above >= 0 ) then
			local levelsabove = {}
			levellabel, leveldata = modWP_Levels.GetLevelItemStringsPair( levelitem, "levelnumber_above" )
			leveldata = tonumber(leveldata)
			levelsabove[#levelsabove + 1] =  modWP_Levels.WikiEntryLevelAnchorText( leveldata )
			wikitext[#wikitext + 1] = modWIKI.TableToWiki( levelsabove, levellabel, SEP )
		end --	level has levels above
		if ( modWP_Levels.hasChildren(levelitem.levelnumber )) then
			--	make a table of levels below
			local levelsbelow = {}
			levellabel = select(1, modWP_Levels.GetLevelItemStringsPair( levelitem, "levelnumber_below" ))
			for key, level in pairs(levelitem.levelnumber_below) do
				levelsbelow[#levelsbelow + 1] =  modWP_Levels.WikiEntryLevelAnchorText( level )
			end
			wikitext[#wikitext + 1] = modWIKI.TableToWiki( levelsbelow, levellabel, SEP )
		end --	level has levels below
		wikitext[#wikitext + 1] = modWIKI.FrameEnd
		wikitext[#wikitext + 1] = modWIKI.ForceBreak
	end
	--	write wiki data object to string
	local writedata = modWIKI.PageProcess( filename, wikitext )
	--	write string to file
	modWP_Levels.modcommon.Process.DataToFile(filepath, writedata)
end

--	convert level number and name to text format
--	[in]	levelnumber	level to be converted
--	[in]	label		text to be used for wiki presentation
--	[in]	withURL		false - return just formated anchor
--						true - return full url#anchor to level information
--	[ret]	string	"XX - level name"
function modWP_Levels.WikiEntryLevelText( levelnumber, label, withURL )
	local asLink = false
	if (( label == nil) or ( not label )) then
		label = ""
	else
		asLink = true
	end
	local useURL = false
	if (( withURL == nil ) or ( not withURL )) then
		useURL = false
	else
		useURL = true
	end
	
	levelnumber = assert(tonumber(levelnumber), "WikiEntryLevelText: unable to convert " .. levelnumber)
	local levelindex = modWP_Levels.GetIndexByValue( modWP_Levels.AllLevelData, levelnumber, "levelnumber" )
	local levelname = modWP_Levels.AllLevelData[levelindex].levelname
	local strText = string.format("%02d", tostring(levelnumber)) .. "&nbsp;-&nbsp;"	.. levelname
	if ( not asLink ) then
		return strText
	else
		strText = "Level " .. strText
		local urltext = ""
		if ( not useURL ) then
			urltext = modWP_Levels.AllLevelData[levelindex].urlAnchor
		else
			urltext = modWP_Levels.modcommon.outputfilenames.levels .. modWP_Levels.AllLevelData[levelindex].urlAnchor
		end
		local leveltext = modWP_Levels.modcommon.Wiki.LinkText(	urltext, strText )
		return modWP_Levels.modcommon.Wiki.TextEntry( label, leveltext,	modWP_Levels.modcommon.Wiki.Seperator, nil, false )
	end
end

function modWP_Levels.WikiEntryLevelAnchorText( levelnumber )
	levelnumber = assert(tonumber(levelnumber), "WikiEntryLevelAnchorText: unable to convert " .. tostring(levelnumber))
	local levelindex = modWP_Levels.GetIndexByValue( modWP_Levels.AllLevelData, levelnumber, "levelnumber" )
	local levelname = modWP_Levels.AllLevelData[levelindex].levelname
	local anchortext = modWP_Levels.AllLevelData[levelindex].urlAnchor
	local levelnum = string.format("%02d", tostring(levelnumber))
	return modWP_Levels.modcommon.Wiki.LinkText( anchortext, levelnum ) .. "&nbsp;-&nbsp;" .. levelname
end

function modWP_Levels.LevelShortLinkText( levelnumber )
	if (( levelnumber == nil ) or ( levelnumber < 0 )) then
		return tostring(levelnumber)
	end
	local index = modWP_Levels.GetIndexByValue( modWP_Levels.AllLevelData, levelnumber, "levelnumber" )
	if ( index ) then
		local levelnumformat = string.format("%02d", tostring(levelnumber))
		local leveltext = modWP_Levels.AllLevelData[index].urlAnchor
		return modWP_Levels.modcommon.Wiki.LinkText( leveltext, levelnumformat )
	else
		return tostring(levelnumber)
	end
end

--	Print out level information based on selected verbosity.
function modWP_Levels.Verbosity()
	if (( not modWP_Levels.modcommon.verbose) and ( not modWP_Levels.modcommon.doubleverbose)) then
		return
	end
	local copyType = modWP_Levels.TypeNames
	copyType[#copyType + 1] = { name = "LevelsUnreferenced", display = "Unreferenced Levels" }
	io.stdout:write(modWP_Levels.modcommon.VerboseHeader)
	io.stdout:write("All Level Data\n")
	io.stdout:write(modWP_Levels.modcommon.VerboseHeader)
	local sum = 0
	for key, tbl in pairs(copyType) do
		sum = sum + #modWP_Levels[tbl.name]
	end
	local printoutput	= "=== Level Check ===\n"
	for key, tbl in pairs(copyType) do
		printoutput = printoutput .. "size of " .. tbl.display .. ": " .. #modWP_Levels[tbl.name] .. "\n"
	end
	printoutput = printoutput .. "size of AllLevelData = sum of above: " .. tostring((#modWP_Levels.AllLevelData == sum )) .. "\n"
	io.stdout:write(printoutput)

	for key, tbl in pairs(copyType) do
		local str = modWP_Levels.modcommon.Extract.OneDTableToString(modWP_Levels[tbl.name])
		io.stdout:write("\n=== Listing " .. tbl.display .. ":\n " .. str .. "\n")
	end

	io.stdout:write("\n=== Printing Ground Level Map (Grid):\n")
	local writedata = ""
	for y = 1, modWP_Levels.grid_max.y do
		local row_data = ""
		for x = 1, modWP_Levels.grid_max.x do
			row_data = row_data .. string.format("%02d ", tostring(modWP_Levels.GridSquareRead(x,y)))
		end
		writedata = writedata .. row_data .. "\n"
	end
	io.stdout:write(writedata)
	io.stdout:write(modWP_Levels.modcommon.VerboseHeader)
	if (modWP_Levels.modcommon.doubleverbose) then
		modWP_Levels.modcommon.Process.TblPrint( modWP_Levels.AllLevelData, nil, nil, "AllLevelData: " .. #modWP_Levels.AllLevelData )
		for key, tbl in pairs(copyType) do
			modWP_Levels.modcommon.Process.TblPrint( modWP_Levels[tbl.name], nil, nil, tbl.name .. ": " .. #modWP_Levels[tbl.name])
		end
	end
	io.stdout:write(modWP_Levels.modcommon.VerboseHeader)
end

return modWP_Levels
