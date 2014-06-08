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
local modWP_ROTD = {}
--	modWPCommon reference
modWP_ROTD.modcommon = {}
--	Variable will contained all parsed information after completion of ProcessNPCData()
modWP_ROTD.AllROTDData = {}
--	ReturnOfTux.droid file parsed line-by-line into table
modWP_ROTD.ROTDFileData = {}
--	table of FDRPG filepaths used for parsing ReturnOfTux.Droids information
modWP_ROTD.files = {
	rotd=""
}

--	"struct" of ReturnOfTux.droid infromation for each entry found
--	use deep copy to pass default values to new copy
modWP_ROTD.rotdDataItem = {
	itemlevel = -1,
	itemtype = "",
	posX = -1,
	posY = -1,
	faction = "",
	dialog = "",
	label = "",
	marker = -1,
	rushTux = -1,
	fixed = -1,
	dropitemID = "",
	MaxDistToHome = 0
}

--	text for ROTD presentation
--	id) key, label) text to display
--	srchPatn) text/patter to use to find data
--	extrctPatn) how to extract found data
modWP_ROTD.textROTD = {
	{ id = "itemlevel",     label = "Level",                srchPatn = "Level=[%d]+",             extrctPatn="[%d]+"   },
	{ id = "itemtype",      label = "Type",                 srchPatn = "T=([%w]+):",              extrctPatn="[MATCH]" },
	{ id = "posX",          label = "Pos X",                srchPatn = "PosX=[%d]+",              extrctPatn="[%d]+"   },
	{ id = "posY",          label = "Pos Y",                srchPatn = "PosY=[%d]+",              extrctPatn="[%d]+"   },
	{ id = "faction",       label = "Faction",              srchPatn = "Faction=_*(%b\"\")",      extrctPatn="[MATCH]" },
	{ id = "dialog",        label = "Dialog",               srchPatn = "UseDialog=_*(%b\"\")",    extrctPatn="[MATCH]" },
	{ id = "label",         label = "Label",                srchPatn = "ShortLabel=_*(%b\"\")",   extrctPatn="[MATCH]" },
	{ id = "marker",        label = "Marker",               srchPatn = "Marker=[%d]+",            extrctPatn="[%d]+"   },
	{ id = "rushTux",       label = "RushTux",              srchPatn = "RushTux=[%d]+",           extrctPatn="[%d]+"   },
	{ id = "fixed",         label = "Fixed",                srchPatn = "Fixed=[%d]+",             extrctPatn="[%d]+"   },
	{ id = "dropitemID",    label = "Drop Item ID",         srchPatn = "DropItemId=(%b\"\")",     extrctPatn="[MATCH]" },
	{ id = "MaxDistToHome", label = "Max Distance To Home", srchPatn = "MaxDistanceToHome=[%d]+", extrctPatn="[%d]+"   },
}
-- action to parse ROTD file line containing character information
-- populates modWP_ROTD.AllROTDData
function modWP_ROTD.ParseROTD()
	local p_extractStart = 0
	local p_extractEnd = 0
	local temprotddataitem = {}
	local currentitemlevel = -1
	local markers = { "itemlevel", "itemtype" }
	for key,line in pairs(modWP_ROTD.ROTDFileData) do
		for subkey, marker in pairs(markers) do
			local markerItem = select(1,modWP_ROTD.modcommon.Extract.GetTableItem( modWP_ROTD.textROTD, "id", marker ))
			local value = modWP_ROTD.modcommon.Extract.SearchText( line, markerItem.srchPatn, markerItem.extrctPatn )
			if ( value == nil ) then goto PARSE_ROTD_NEXT_MARKER end	--	did not extract marker value
			if	( markerItem.id == "itemlevel" ) then
				currentitemlevel = value
			elseif(markerItem.id == "itemtype") then
				-- this is an ROTD data line
				temprotddataitem = modWP_ROTD.modcommon.Extract.TblDeepCopy(modWP_ROTD.rotdDataItem)
				temprotddataitem.itemlevel = currentitemlevel
				temprotddataitem.itemtype = value
				for subkey, textitem in pairs(modWP_ROTD.textROTD) do
					if (( textitem.id ~= "itemlevel") and ( textitem.id ~= "itemtype")) then
						local subvalue = modWP_ROTD.modcommon.Extract.SearchText( line, textitem.srchPatn, textitem.extrctPatn )
						if ( subvalue ) then
							temprotddataitem[textitem.id] = subvalue
						end	-- extracted a value
					end	-- looping -- key not level or type
				end	-- looping through search markers (textROTD)
				-- done parsing this line - add to AllROTDData
				table.insert(modWP_ROTD.AllROTDData, temprotddataitem)
			end	-- parsing ROTD data line
::PARSE_ROTD_NEXT_MARKER::
		end	-- testing line for level or NPC type
	end	-- looping through ROTD file
end

--	Read in FDRPG ReturnOfTux.droid data file
--	All droid information is saved into lua table.
function modWP_ROTD.ProcessData()
	modWP_ROTD.modcommon = require("modWPCommon")
	-- test for presence of source data files
	modWP_ROTD.files.rotd = tostring(modWP_ROTD.modcommon.paths.srcMap .. modWP_ROTD.modcommon.datafiles["rotd"])
	modWP_ROTD.modcommon.Test.Files(modWP_ROTD.files)
	-- read ROTD and process into table objects
	modWP_ROTD.ROTDFileData = modWP_ROTD.modcommon.Process.FileToLines(modWP_ROTD.files["rotd"])
	modWP_ROTD.ParseROTD()
end

--	Print out NPC information based on selected verbosity.
function modWP_ROTD.Verbosity()
	if ((modWP_ROTD.modcommon.verbose) or (modWP_ROTD.modcommon.doubleverbose)) then
		io.stdout:write(modWP_ROTD.modcommon.VerboseHeader)
		io.stdout:write("modWP_ROTD\n")
		io.stdout:write("number of ROTD: " .. #modWP_ROTD.AllROTDData .. "\n")
		io.stdout:write(modWP_ROTD.modcommon.VerboseHeader)
		if (modWP_ROTD.modcommon.doubleverbose) then
			modWP_ROTD.modcommon.Process.TblPrint(modWP_ROTD.AllROTDData, nil, nil, "All ROTD Data")
		end
	end
end

return modWP_ROTD
