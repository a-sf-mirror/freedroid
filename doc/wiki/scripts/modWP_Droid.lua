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
--	lua module for parsing FDRPG droid file data
local modWP_Droid = {}
--	modWPCommon reference
modWP_Droid.modcommon = {}
--	Variable will contained all parsed droid information after completion of ProcessDroidData()
modWP_Droid.AllDroidData = {}
--	"struct" of droid infromation for each droid
--	use deep copy to pass default values to new copy
modWP_Droid.droidDataItem = {
--	Each "droid" section starts with "** Start of new Robot: **"
--	variable                    delimiter  src matching variable
	name = "",              --    : www    Droidname
	desc = "",              --    :_"*"    Default description
	is_human = false,       --    : d      Is this 'droid' a human (1=true,else false)
	graphics_prefix = "",   --    ="*"     Filename prefix for graphics
	portrait_prefix = "",   --    ="*"     Droid uses portrait rotation series with prefix
	speed_max = 0,          --    : dd     Maximum speed of this droid
	energy_max = 0,         --    : dd     Maximum energy of this droid
	droid_class = 0,        --    : dd     Class of this droid
	drop_item_class = 0,    --    =dd      Drops item class
	heal_rate = 0,          --    : dd     Rate of healing,
	destroy_xp = 0,         --    : ddd    Experience_Reward gained for destroying one of this type
	aggr_dist = 0,          --    : dd     Aggression distance of this droid
	time_eye_Tux = 0.0,     --    : f.f    Time spent eyeing Tux
	weapon = "",            --    ="*"     Weapon item
	pct_EntropyInvrtr = 0,  --    =dd      Percent to drop Entropy Inverter
	pct_PlasmaTrans = 0,    --    =dd      Percent to drop Plasma Transistor
	pct_SprCondRelay = 0,   --    =dd      Percent to drop Superconducting Relay Unit
	pct_AMConverter = 0,    --    =dd      Percent to drop Antimatter-Matter Converter
	pct_TachyonCond = 0,    --    =dd      Percent to drop Tachyon Condensator
	sound_num_greet = 0,    --    =dd      Greeting Sound number
	sound_death = "",       --    ="*"     Death sound file name (*.ogg)
	sound_attack = "",      --    ="*"     Attack animation sound file name (*.ogg)
	pct_hit = 0,            --    =dd      Chance of this robot scoring a hit
	time_hit_recover = 0.0, --    +f.ff    Time to recover after getting hit
	droid_notes = "",       --    =_"*"    Notes concerning this droid
	urlAnchor = "",         --             text to use for wiki page anchor
	image = { ext = "",     --    image extension
	          src = "",     --    source file path for image (system dependent)
	          dest = "",    --    destination folder for image link
	          name = "",    --    text to use for link to image
	}
}
-- names of required modules - "modWP_Items"
modWP_Droid.wikirequiredModules = { moduleNames[3].id }

-- special sorting for "all data table"
-- [in]	a|b	elements in all data table
function modWP_Droid.SpecialAllDataSort(a,b)
	return a.name < b.name
end
--	Droid_Archetypes.dat file parsed line-by-line into table
modWP_Droid.DroidFileData = {}
--	table of FDRPG filepaths used for parsing droid information
modWP_Droid.files = {
	droid = ""
}
--	text items for Droid parsing and presentation
--	id) key, label) text to display loop) can use in loop when search for text
--	srchPatn) text/patter to use to find data
--	extrctPatn) how to extract found data
modWP_Droid.textDroid = {
	{ id = "spec_item_start",   label = "end_of_level",                loop = true,  srchPatn = "Start of new Robot:",                                  extrctPatn = "[TEXT]" },
	{ id = "name",              label = "Name",                        loop = true,  srchPatn = "Droidname:%s*([%w]+)",                                 extrctPatn = "[MATCH]" },
	{ id = "desc",              label = "Description",                 loop = true,  srchPatn = "Default description:",                                 extrctPatn = "[EOL]" },
	{ id = "is_human",          label = "Is Human",                    loop = true,  srchPatn = "Is this \'droid\' a human%s*:%s*[%d]+",                extrctPatn = "[%d]+" },
	{ id = "graphics_prefix",   label = "Graphics Prefix",             loop = true,  srchPatn = "Filename prefix for graphics=",                        extrctPatn = "[EOL]" },
	{ id = "portrait_prefix",   label = "Portrait_Prefix",             loop = true,  srchPatn = "Droid uses portrait rotation series with prefix=",     extrctPatn = "[EOL]" },
	{ id = "speed_max",         label = "Max Speed",                   loop = true,  srchPatn = "Maximum speed of this droid:%s*[%d]+",                 extrctPatn = "%d+" },
	{ id = "energy_max",        label = "Max Energy",                  loop = true,  srchPatn = "Maximum energy of this droid:%s*[%d]+",                extrctPatn = "%d+" },
	{ id = "droid_class",       label = "Class",                       loop = true,  srchPatn = "Class of this droid:%s*[%d]+",                         extrctPatn = "%d+" },
	{ id = "drop_item_class",   label = "Drop Item Class",             loop = true,  srchPatn = "Drops item class=%s*[%d]+",                            extrctPatn = "%d+" },
	{ id = "heal_rate",         label = "Heal Rate",                   loop = true,  srchPatn = "Rate of healing:%s*[%d]+[.%d+]*",                      extrctPatn = "[%d]+[.%d+]*" },
	{ id = "destroy_xp",        label = "Destroy XP Gain",             loop = true,  srchPatn = "Experience_Reward gained for destroying one of this type:%s*[%d]+", extrctPatn = "[%d]+" },
	{ id = "aggr_dist",         label = "Aggression Distance",         loop = true,  srchPatn = "Aggression distance of this droid=%s*[%d]+",           extrctPatn = "[%d]+" },
	{ id = "time_eye_Tux",      label = "Time Spent Eyeing Tux",       loop = true,  srchPatn = "Time spent eyeing Tux=[%d]+[.%d+]*",                   extrctPatn = "[%d]+[.%d+]*" },
	{ id = "weapon",            label = "Weapon",                      loop = true,  srchPatn = "Weapon item=",                                         extrctPatn = "[EOL]" },
	{ id = "droppct",           label = "Drop Percentages",            loop = false, srchPatn = "" },
	{ id = "pct_EntropyInvrtr", label = "Entropy Inverter",            loop = true,  srchPatn = "Percent to drop Entropy Inverter=%s*[%d]+",            extrctPatn = "[%d]+" },
	{ id = "pct_PlasmaTrans",   label = "Plasma Transistor",           loop = true,  srchPatn = "Percent to drop Plasma Transistor=%s*[%d]+",           extrctPatn = "[%d]+" },
	{ id = "pct_SprCondRelay",  label = "Superconducting Relay Unit",  loop = true,  srchPatn = "Percent to drop Superconducting Relay Unit=%s*[%d]+",  extrctPatn = "[%d]+" },
	{ id = "pct_AMConverter",   label = "Antimatter-Matter Converter", loop = true,  srchPatn = "Percent to drop Antimatter-Matter Converter=%s*[%d]+", extrctPatn = "[%d]+" },
	{ id = "pct_TachyonCond",   label = "Tachyon Condensator",         loop = true,  srchPatn = "Percent to drop Tachyon Condensator=%s*[%d]+",         extrctPatn = "[%d]+" },
	{ id = "sound_num_greet",   label = "Greeting Sound Number",       loop = true,  srchPatn = "Greeting Sound number=%s*[%d]+",                       extrctPatn = "[%d]+" },
	{ id = "sound_death",       label = "Death Sound File Name",       loop = true,  srchPatn = "Death sound file name=",                               extrctPatn = "[EOL]" },
	{ id = "sound_attack",      label = "Attack Sound File Name",      loop = true,  srchPatn = "Attack animation sound file name=",                    extrctPatn = "[EOL]" },
	{ id = "pct_hit",           label = "Hit Percentage",              loop = true,  srchPatn = "Chance of this robot scoring a hit=%s*[%d]+",          extrctPatn = "[%d]+" },
	{ id = "time_hit_recover",  label = "Hit Recovery Time",           loop = true,  srchPatn = "Time to recover after getting hit=[%d]+[.%d+]*",       extrctPatn = "[%d]+[.%d+]*" },
	{ id = "droid_notes",       label = "Notes concerning this droid", loop = true,  srchPatn = "Notes concerning this droid=",                         extrctPatn = "[EOL]" },
	{ id = "spec_file_end",     label = "end_of_level",                loop = true,  srchPatn = "End of Robot Data Section",                            extrctPatn = "[TEXT]" },
}

-- retrieve the anchortext associated with this id value
function modWP_Droid.GetItemUrlText( idvalue )
	local retText = ""
	if ( not idvalue ) then
		return retText
	end
	local droid = select(1,modWP_Droid.modcommon.Extract.GetTableItem(modWP_Droid.AllDroidData,"name", idvalue))
	if ( droid ~= nil ) then
		retText = modWP_Droid.modcommon.outputfilenames.droids .. droid.urlAnchor
	end
	return retText
end

--	Look up droidDataItem variable value and its presentation label
--	Assumes DroidFileData is populated and processed
--	[in]	droiditem	droid object under examination (type droidDataItem)
--	[in]	idvalue	search id - same as id value in textDroid
--	[ret]	pair of strings representing label|data from droid item
function modWP_Droid.GetDroidStringsPair( droiditem, idvalue )
	local retLabel, retData = "", ""
	if (( idvalue == nil ) or ( type(idvalue) ~= 'string' ) or ( idvalue:len() <= 0 )) then
		return retLabel, retData
	end
	local labelitem = select(1,modWP_Droid.modcommon.Extract.GetTableItem( modWP_Droid.textDroid, "id", idvalue))
	if ( labelitem ~= nil ) then
		retLabel = labelitem.label
		if	( idvalue == "droppct") then
			retData = ""
		else
			retData = tostring(droiditem[idvalue])
		end
	end
	return retLabel, retData
end

--	Process file data for all droid information
--	Populates DroidFileData variable
function modWP_Droid.ParseDroidSpec()
	local tempdroidspecitem = {}
	local firstpass = true
	for key,line in pairs(modWP_Droid.DroidFileData)do
		for subkey, textitem in pairs(modWP_Droid.textDroid)do
			if ( not textitem.loop ) then goto PARSE_DROID_NEXT_SRCH_PATTERN end
			local value = modWP_Droid.modcommon.Extract.SearchText( line, textitem.srchPatn, textitem.extrctPatn )
			if ( not value ) then goto PARSE_DROID_NEXT_SRCH_PATTERN end
			if	( textitem.id == "spec_item_start" ) then
				--	process begin|end of droid data section
				if (firstpass) then
					firstpass = false
				else
					table.insert(modWP_Droid.AllDroidData, tempdroidspecitem)
				end
				tempdroidspecitem = modWP_Droid.modcommon.Extract.TblDeepCopy(modWP_Droid.droidDataItem)
			elseif ( textitem.id == "spec_file_end" ) then
				--	reached the end of droid specs data in file
				table.insert(modWP_Droid.AllDroidData, tempdroidspecitem)
			elseif ( textitem.id == "is_human" ) then
				tempdroidspecitem[textitem.id] = ( value ~= 0 )
			else
				tempdroidspecitem[textitem.id] = value
			end	--	data processed
			break	--	next line of data
::PARSE_DROID_NEXT_SRCH_PATTERN::
		end	--	pattern/value found
	end	--	loop through text file data
	if (#modWP_Droid.AllDroidData > 0) then
		table.sort( modWP_Droid.AllDroidData, modWP_Droid.SpecialAllDataSort )
	end	--	sort AllDroidData table
end

--	Read in FDRPG Droid_Archetypes data file and process for each droid type.
--	All droid information is saved into lua table.
function modWP_Droid.ProcessData()
	modWP_Droid.modcommon = require("modWPCommon")
	modWP_Droid.files.droid = tostring(modWP_Droid.modcommon.paths.srcMap .. modWP_Droid.modcommon.datafiles["droid"])
	modWP_Droid.modcommon.Test.Files(modWP_Droid.files)
	--	process data
	modWP_Droid.DroidFileData = modWP_Droid.modcommon.Process.FileToLines(modWP_Droid.files.droid)
	modWP_Droid.ParseDroidSpec()
	for key, droid in pairs(modWP_Droid.AllDroidData) do
		droid.urlAnchor = modWP_Droid.modcommon.Wiki.HLink .. "droid" .. droid.name
		local srcpathshort = "droids/" .. droid.graphics_prefix
		droid.image.ext = ".png"
		droid.image.src = modWP_Droid.modcommon.paths.srcGraphics .. srcpathshort .. "/portrait" .. droid.image.ext
		droid.image.dest = modWP_Droid.modcommon.paths.destRootImg .. "Droids/"
		droid.image.name = srcpathshort:gsub("%/","_")
	end
end

--	Write FDRPG droid information to file in a wiki format.
function modWP_Droid.WikiWrite()
	local modItems = assert(require(modWP_Droid.wikirequiredModules[1]))
	local modWIKI = modWP_Droid.modcommon.Wiki
	local LI = modWIKI.LI
	local SEP = modWIKI.Seperator
	local filename = modWP_Droid.modcommon.outputfilenames.droids
	local filepath = tostring(modWP_Droid.modcommon.paths.destRootFile .. filename)
	local wikitext = {}
	wikitext[#wikitext + 1] = modWIKI.PageSummary("FreedroidRPG Droid Types")
	--	make menu for droid types
	wikitext[#wikitext + 1] = modWIKI.FrameStartRight("font-size:smaller")
	wikitext[#wikitext + 1] = modWIKI.HeaderLevel(3) .. "Freedroid Droid Types"
	wikitext[#wikitext + 1] = modWIKI.LinkText(modWIKI.HLink .. "alldroids","Droids")
	for key, droiditem in pairs(modWP_Droid.AllDroidData) do
		if (not droiditem.is_human) then
			local linktext = modWIKI.HLink .. modWIKI.WikifyLink("droid" .. droiditem.name)
			wikitext[#wikitext + 1] = LI .. modWIKI.LinkText(linktext, droiditem.name)
		end
	end	--	loop to produce menu items
	wikitext[#wikitext + 1] = modWIKI.FrameEnd
	--	end menu

	wikitext = modWIKI.WarnAutoGen( wikitext )
	wikitext = modWIKI.WarnSpoil( wikitext )
	--	page contents start here
	wikitext[#wikitext + 1] = modWIKI.LinkText(modWIKI.HLink .. "alldroids")
	wikitext[#wikitext + 1] = modWIKI.HeaderLevel(1) .. "Droids"
	wikitext[#wikitext + 1] = modWIKI.LineSep
	--	loop to produce droid items
	for key, droiditem in pairs(modWP_Droid.AllDroidData) do
		if ( droiditem.is_human ) then
			goto WIKI_PROCESS_NEXT_DROID end
		local portraitname = ""
		--	add paths to list of images to copy
		local portaititem = modWIKI.ManagePortrait( droiditem.image.src, droiditem.image.dest, droiditem.image.name, droiditem.image.ext)
		if ( portaititem ~= nil ) then
			portraitname = portaititem.destfile
		end
		--	process presentation
		wikitext[#wikitext + 1] = modWIKI.LinkText(droiditem.urlAnchor)
		wikitext[#wikitext + 1] = modWIKI.HeaderLevel(2) .. droiditem.name
		--	image wiki presentation
		wikitext[#wikitext + 1] = modWIKI.ImageText( modWIKI.URL_ImgDroid .. portraitname, droiditem.graphics_prefix, "margin-right=\'1.0em\'")
		--	display droid description
		local data = select(2,modWP_Droid.GetDroidStringsPair(droiditem,"desc"))
		data = modWIKI.TextEmbed(data,"emphasis")
		data = modWIKI.TextEmbed(data,"textlarge")
		wikitext[#wikitext + 1] =	data .. modWIKI.LineBreakEnd
		--	display droid notes
		wikitext[#wikitext + 1] = select(2,modWP_Droid.GetDroidStringsPair(droiditem,"droid_notes")) .. "\n\n"
		--	display basic droid information
		local DataSequence = { "weapon", "sound_num_greet", "sound_death", "sound_attack" }
		for key, idval in pairs(DataSequence) do
			local appending = ""
			if (key == #DataSequence) then
				appending = string.rep("\n",2)
			else
				appending = modWIKI.LineBreakEnd
			end
			droidlabel, droiddata = modWP_Droid.GetDroidStringsPair(droiditem, idval)
			droidlabel = modWIKI.TextEmbed(droidlabel,"emphasis")
			if ( idval == "weapon" ) then
				local weaponlink = modItems.GetItemUrlText(droiddata)
				if ( weaponlink ) then
					droiddata = modWIKI.LinkText(weaponlink, droiddata)
				end
			end
			wikitext[#wikitext + 1] =	droidlabel .. SEP .. droiddata .. appending
		end
		--	display droid table data
		DataSequence = { "droid_class", "speed_max", "energy_max", "drop_item_class", "heal_rate",
		                 "destroy_xp", "aggr_dist", "time_eye_Tux", "pct_hit", "time_hit_recover" }
		local tblstyle = "border=0 width=20% align=left cellspacing=0 cellpadding=4"
		local droidlabel = {}
		local droiddata = {}
		local textdata = {}
		for key, part in pairs(DataSequence) do
			droidlabel[#droidlabel + 1],droiddata[#droiddata + 1] = modWP_Droid.GetDroidStringsPair(droiditem, part)
		end
		textdata = modWIKI.TableGen( tblstyle, nil, droidlabel, droiddata, "width=\'80%\'", "align=center" )
		wikitext = modWIKI.PageAppend( wikitext, textdata )
		--	table seperator
		wikitext[#wikitext + 1] = modWIKI.TableSeperator
		wikitext[#wikitext + 1] = modWIKI.TableSeperator
		--	display drop percentages
		DataSequence = { "pct_EntropyInvrtr", "pct_PlasmaTrans", "pct_SprCondRelay", "pct_AMConverter", "pct_TachyonCond" }
		local tblheader = select(1, modWP_Droid.GetDroidStringsPair( droiditem, "droppct" ))
		droidlabel = {}
		droiddata = {}
		for key, part in pairs(DataSequence) do
			droidlabel[#droidlabel + 1],droiddata[#droiddata + 1] = modWP_Droid.GetDroidStringsPair(droiditem, part)
		end
		textdata = modWIKI.TableGen( tblstyle, tblheader, droidlabel, droiddata, "width=\'86%\'", "align=center" )
		wikitext = modWIKI.PageAppend( wikitext, textdata )
		wikitext[#wikitext + 1] = modWIKI.ForceBreak
		wikitext[#wikitext + 1] = " "
		wikitext[#wikitext + 1] = modWIKI.LineSep
::WIKI_PROCESS_NEXT_DROID::
	end
	--	write wiki data object to string
	local writedata = modWIKI.PageProcess( filename, wikitext )
	--	write string to file
	modWP_Droid.modcommon.Process.DataToFile(filepath, writedata)
end

--	Print out droid information based on selected verbosity.
function modWP_Droid.Verbosity()
	if (( not modWP_Droid.modcommon.verbose) and ( not modWP_Droid.modcommon.doubleverbose)) then
		return
	end
	io.stdout:write(modWP_Droid.modcommon.VerboseHeader)
	io.stdout:write("modWP_Droid\n")
	io.stdout:write("number of droid spec: " .. #modWP_Droid.AllDroidData .. "\n")
	if (modWP_Droid.modcommon.doubleverbose) then
		modWP_Droid.modcommon.Process.TblPrint(modWP_Droid.AllDroidData, nil, nil, "All Droid Data")
	end
end

return modWP_Droid
