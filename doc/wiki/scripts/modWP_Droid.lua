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
	},
	sensor = ""				--	="*"		one of {spectral (default), infrared, xray, radar, subsonic}
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
--	id) key, label) text to display testInLoop) when searching, test for this id in loop?
--	srchPatn) text/patter to use to find data
--	extrctPatn) how to extract found data
modWP_Droid.textDroid = {
	{ id = "spec_item_start",   label = "end_of_level",                testInLoop = true,  srchPatn = "Start of new Robot:",                                  extrctPatn = "[TEXT]" },
	{ id = "name",              label = "Name",                        testInLoop = true,  srchPatn = "Droidname:%s*([%w]+)",                                 extrctPatn = "[MATCH]" },
	{ id = "desc",              label = "Description",                 testInLoop = true,  srchPatn = "Default description:",                                 extrctPatn = "[EOL]" },
	{ id = "is_human",          label = "Is Human",                    testInLoop = true,  srchPatn = "Is this \'droid\' a human%s*:%s*[%d]+",                extrctPatn = "[%d]+" },
	{ id = "graphics_prefix",   label = "Graphics Prefix",             testInLoop = true,  srchPatn = "Filename prefix for graphics=",                        extrctPatn = "[EOL]" },
	{ id = "portrait_prefix",   label = "Portrait_Prefix",             testInLoop = true,  srchPatn = "Droid uses portrait rotation series with prefix=",     extrctPatn = "[EOL]" },
	{ id = "speed_max",         label = "Max Speed",                   testInLoop = true,  srchPatn = "Maximum speed of this droid:%s*[%d]+",                 extrctPatn = "%d+" },
	{ id = "energy_max",        label = "Max Energy",                  testInLoop = true,  srchPatn = "Maximum energy of this droid:%s*[%d]+",                extrctPatn = "%d+" },
	{ id = "droid_class",       label = "Class",                       testInLoop = true,  srchPatn = "Class of this droid:%s*[%d]+",                         extrctPatn = "%d+" },
	{ id = "drop_item_class",   label = "Drop Item Class",             testInLoop = true,  srchPatn = "Drops item class=%s*[%d]+",                            extrctPatn = "%d+" },
	{ id = "heal_rate",         label = "Heal Rate",                   testInLoop = true,  srchPatn = "Rate of healing:%s*[%d]+[.%d+]*",                      extrctPatn = "[%d]+[.%d+]*" },
	{ id = "destroy_xp",        label = "Destroy XP Gain",             testInLoop = true,  srchPatn = "Experience_Reward gained for destroying one of this type:%s*[%d]+", extrctPatn = "[%d]+" },
	{ id = "aggr_dist",         label = "Aggression Distance",         testInLoop = true,  srchPatn = "Aggression distance of this droid=%s*[%d]+",           extrctPatn = "[%d]+" },
	{ id = "time_eye_Tux",      label = "Time Spent Eyeing Tux",       testInLoop = true,  srchPatn = "Time spent eyeing Tux=[%d]+[.%d+]*",                   extrctPatn = "[%d]+[.%d+]*" },
	{ id = "weapon",            label = "Weapon",                      testInLoop = true,  srchPatn = "Weapon item=",                                         extrctPatn = "[EOL]" },
	{ id = "droppct",           label = "Drop Percentages",            testInLoop = false, srchPatn = "" },
	{ id = "pct_EntropyInvrtr", label = "Entropy Inverter",            testInLoop = true,  srchPatn = "Percent to drop Entropy Inverter=%s*[%d]+",            extrctPatn = "[%d]+" },
	{ id = "pct_PlasmaTrans",   label = "Plasma Transistor",           testInLoop = true,  srchPatn = "Percent to drop Plasma Transistor=%s*[%d]+",           extrctPatn = "[%d]+" },
	{ id = "pct_SprCondRelay",  label = "Superconducting Relay Unit",  testInLoop = true,  srchPatn = "Percent to drop Superconducting Relay Unit=%s*[%d]+",  extrctPatn = "[%d]+" },
	{ id = "pct_AMConverter",   label = "Antimatter-Matter Converter", testInLoop = true,  srchPatn = "Percent to drop Antimatter-Matter Converter=%s*[%d]+", extrctPatn = "[%d]+" },
	{ id = "pct_TachyonCond",   label = "Tachyon Condensator",         testInLoop = true,  srchPatn = "Percent to drop Tachyon Condensator=%s*[%d]+",         extrctPatn = "[%d]+" },
	{ id = "sound_num_greet",   label = "Greeting Sound Number",       testInLoop = true,  srchPatn = "Greeting Sound number=%s*[%d]+",                       extrctPatn = "[%d]+" },
	{ id = "sound_death",       label = "Death Sound File Name",       testInLoop = true,  srchPatn = "Death sound file name=",                               extrctPatn = "[EOL]" },
	{ id = "sound_attack",      label = "Attack Sound File Name",      testInLoop = true,  srchPatn = "Attack animation sound file name=",                    extrctPatn = "[EOL]" },
	{ id = "pct_hit",           label = "Hit Percentage",              testInLoop = true,  srchPatn = "Chance of this robot scoring a hit=%s*[%d]+",          extrctPatn = "[%d]+" },
	{ id = "time_hit_recover",  label = "Hit Recovery Time",           testInLoop = true,  srchPatn = "Time to recover after getting hit=[%d]+[.%d+]*",       extrctPatn = "[%d]+[.%d+]*" },
	{ id = "droid_notes",       label = "Notes concerning this droid", testInLoop = true,  srchPatn = "Notes concerning this droid=",                         extrctPatn = "[EOL]" },
	{ id = "sensor",			label = "Sensor",					   testInLoop = true,  srchPatn = "Sensor ID=",                         					extrctPatn = "[EOL]" },
	{ id = "spec_file_end",     label = "end_of_level",                testInLoop = true,  srchPatn = "End of Robot Data Section",                            extrctPatn = "[TEXT]" },
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
	--	firstpass - are we passing through the file for the first time?
	--	prevents "saving" droid data when first start-of-section is read
	local firstpass = true
	--	foundsensor - was a sensor value found when reading droid data?
	local foundsensor = false
	-- loop through droid data file line-by-line
	for key,line in pairs(modWP_Droid.DroidFileData)do
		-- for each line - compare to each item in textDroid array
		for subkey, textitem in pairs(modWP_Droid.textDroid)do

			-- should we test for this item? N0 - start next iteration
			if ( not textitem.testInLoop ) then goto PARSE_DROID_NEXT_SRCH_PATTERN end
			-- for test item - extract value based on search pattern
			local value = modWP_Droid.modcommon.Extract.SearchText( line, textitem.srchPatn, textitem.extrctPatn )
			-- value not found - start next iteration
			if ( not value ) then goto PARSE_DROID_NEXT_SRCH_PATTERN end
			-- value found - what was found?
			if	( textitem.id == "spec_item_start" ) then
				--	found start-of-droid data section
				if (firstpass) then
					-- found first start-of-droid data section in file
					-- no previous data to save
					firstpass = false
				else
					--	save previous and setup for next pass
					if ( foundsensor == false) then
						-- no sensor data found - set to default
						tempdroidspecitem.sensor = "spectral"
					end
					-- reset sensor flag for next droid data section
					foundsensor = false
					-- save found droid data to array
					table.insert(modWP_Droid.AllDroidData, tempdroidspecitem)
				end
				-- reset temp droid data holder for next droid data section
				tempdroidspecitem = modWP_Droid.modcommon.Extract.TblDeepCopy(modWP_Droid.droidDataItem)

			elseif ( textitem.id == "spec_file_end" ) then
				--	reached the end of droid specs data in file
				--	save previous before exit
				if ( foundsensor == false) then
					-- no sensor data found - set to default
					tempdroidspecitem.sensor = "spectral"
				end
				-- save found droid data to array
				table.insert(modWP_Droid.AllDroidData, tempdroidspecitem)
				-- reset sensor flag for next iteration
				foundsensor = false
				-- will exit nested loops after this point

			elseif ( textitem.id == "is_human" ) then
				-- element data forced to boolean - saved in droid data file as integer
				tempdroidspecitem[textitem.id] = ( value ~= 0 )
			else
				-- have extracted value to be saved to current tempdroidspecitem
				if (textitem.id == "sensor" ) then
					-- sensor data found - set flag to prevent using default
					foundsensor = true
				end
				tempdroidspecitem[textitem.id] = value
			end		--	process extracted value read from file
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

--	Write FDRPG droid information to file in the format needed by the website.
function modWP_Droid.WikiWrite()
	local modItems = assert(require(modWP_Droid.wikirequiredModules[1]))
	local modWIKI = modWP_Droid.modcommon.Wiki
	local filename = modWP_Droid.modcommon.outputfilenames.droids .. ".html.md.eco"
	local filepath = tostring(modWP_Droid.modcommon.paths.destRootFile .. filename)
	local wikitext = {}

	wikitext[#wikitext + 1] = "---"
	wikitext[#wikitext + 1] = "layout: 'page'"
	wikitext[#wikitext + 1] = "title: 'Droid Guide'"
	wikitext[#wikitext + 1] = "comment: 'Characteristics of the droids that can be met in the game.'"
	wikitext[#wikitext + 1] = ""
	wikitext[#wikitext + 1] = "droids:"

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
		--	process data
		modWIKI.StartMapping()
		wikitext[#wikitext + 1] = modWIKI.AddAttr('id', modWIKI.WikifyLink("droid" .. droiditem.name))
		wikitext[#wikitext + 1] = modWIKI.AddAttr('name', droiditem.name)
		wikitext[#wikitext + 1] = modWIKI.AddAttr('time_hit_recover', droiditem.time_hit_recover)
		wikitext[#wikitext + 1] = modWIKI.AddAttr('pct_TachyonCond', droiditem.pct_TachyonCond)
		wikitext[#wikitext + 1] = modWIKI.AddAttr('droid_class', droiditem.droid_class)
		wikitext[#wikitext + 1] = modWIKI.AddAttr('graphics_prefix', droiditem.graphics_prefix)
		wikitext[#wikitext + 1] = modWIKI.AddAttr('weapon', droiditem.weapon)
		wikitext[#wikitext + 1] = modWIKI.AddAttr('speed_max', droiditem.speed_max)
		wikitext[#wikitext + 1] = modWIKI.AddAttr('time_eye_Tux', droiditem.time_eye_Tux)
		wikitext[#wikitext + 1] = modWIKI.AddAttr('urlAnchor', droiditem.urlAnchor)
		wikitext[#wikitext + 1] = modWIKI.AddAttr('sound_num_greet', droiditem.sound_num_greet)
		wikitext[#wikitext + 1] = modWIKI.AddAttr('aggr_dist', droiditem.aggr_dist)
		wikitext[#wikitext + 1] = modWIKI.AddAttr('sensor', droiditem.sensor)
		wikitext[#wikitext + 1] = modWIKI.AddAttr('pct_SprCondRelay', droiditem.pct_SprCondRelay)
		wikitext[#wikitext + 1] = modWIKI.AddAttr('energy_max', droiditem.energy_max)
		wikitext[#wikitext + 1] = modWIKI.AddAttr('destroy_xp', droiditem.destroy_xp)
		wikitext[#wikitext + 1] = modWIKI.AddAttr('desc', droiditem.desc)
		wikitext[#wikitext + 1] = modWIKI.AddAttr('drop_item_class', droiditem.drop_item_class)
		wikitext[#wikitext + 1] = modWIKI.AddAttr('droid_notes', droiditem.droid_notes)
		wikitext[#wikitext + 1] = modWIKI.AddAttr('image', droiditem.image.name .. droiditem.image.ext)
		wikitext[#wikitext + 1] = modWIKI.AddAttr('is_human', droiditem.is_human)
		wikitext[#wikitext + 1] = modWIKI.AddAttr('heal_rate', droiditem.heal_rate)
		wikitext[#wikitext + 1] = modWIKI.AddAttr('pct_EntropyInvrtr', droiditem.pct_EntropyInvrtr)
		wikitext[#wikitext + 1] = modWIKI.AddAttr('pct_PlasmaTrans', droiditem.pct_PlasmaTrans)
		wikitext[#wikitext + 1] = modWIKI.AddAttr('portrait_prefix', droiditem.portrait_prefix)
		wikitext[#wikitext + 1] = modWIKI.AddAttr('sound_death', droiditem.sound_death)
		wikitext[#wikitext + 1] = modWIKI.AddAttr('pct_AMConverter', droiditem.pct_AMConverter)
		wikitext[#wikitext + 1] = modWIKI.AddAttr('sound_attack', droiditem.sound_attack)
		wikitext[#wikitext + 1] = modWIKI.AddAttr('pct_hit', droiditem.pct_hit)
		modWIKI.EndMapping()
::WIKI_PROCESS_NEXT_DROID::
	end
	wikitext[#wikitext + 1] = "---"
	wikitext[#wikitext + 1] = ""
	wikitext[#wikitext + 1] = [[
# Droids Guide

<div class="row">
 <div class="toc col-md-2 pull-right">
  <span><b>Droid Types</b></span>
  <ul>
  <% for droid in @document.droids: %>
   <li><a href="#<%- droid.id %>"><%- droid.name %></a></li>
  <% end %>
  </ul>
 </div>

 <div class="col-md-10">
 <% for droid in @document.droids: %>
  <div class="row">
   <h1 id="<%- droid.id %>"><%- droid.name %></h1>
   <div class="obj-portrait col-md-2 text-center">
    <img src="/images/droids/<%- droid.image %>">
   </div>
   <div class="col-md-8">
    <h3><%- droid.desc %></h3>
    <p><%- droid.droid_notes %></p>
    <p><strong>Weapon</strong>: <%- droid.weapon %><br/>
       <strong>Sensor</strong>: <%- droid.sensor %><br/>
       <strong>Greeting Sound Number</strong>: <%- droid.sound_num_greet %><br/>
       <strong>Death Sound File Name</strong>: <%- droid.sound_death %><br/>
       <strong>Attack Sound File Name</strong>: <%- droid.sound_attack %></p>
    <div class="row">
     <div class="col-md-6 bordered-table">
      <table width="100%"><tbody>
       <tr>
        <td align="left" width="80%" valign="top">Class</td>
        <td align="center" valign="top"><%- droid.droid_class %></td>
       </tr>
       <tr>
        <td align="left" width="80%" valign="top">Max Speed</td>
        <td align="center" valign="top"><%- droid.speed_max %></td>
       </tr>
       <tr>
        <td align="left" width="80%" valign="top">Max Energy</td>
        <td align="center" valign="top"><%- droid.energy_max %></td>
       </tr>
       <tr>
        <td align="left" width="80%" valign="top">Drop Item Class</td>
        <td align="center" valign="top"><%- droid.drop_item_class %></td>
       </tr>
       <tr>
        <td align="left" width="80%" valign="top">Heal Rate</td>
        <td align="center" valign="top"><%- droid.heal_rate %></td>
       </tr>
       <tr>
        <td align="left" width="80%" valign="top">Destroy XP Gain</td>
        <td align="center" valign="top"><%- droid.destroy_xp %></td>
       </tr>
       <tr>
        <td align="left" width="80%" valign="top">Aggression Distance</td>
        <td align="center" valign="top"><%- droid.aggr_dist %></td>
       </tr>
       <tr>
        <td align="left" width="80%" valign="top">Time Spent Eyeing Tux</td>
        <td align="center" valign="top"><%- droid.time_eye_Tux %></td>
       </tr>
       <tr>
        <td align="left" width="80%" valign="top">Hit Percentage</td>
        <td align="center" valign="top"><%- droid.pct_hit %></td>
       </tr>
       <tr>
        <td align="left" width="80%" valign="top">Hit Recovery Time</td>
        <td align="center" valign="top"><%- droid.time_hit_recover %></td>
       </tr>
      </tbody></table>
     </div>
     <div class="col-md-6 bordered-table">
      <table width="100%"><tbody>
       <tr>
        <td colspan=2 align="center">Drop Percentages</td>
       </tr>
       <tr>
        <td align="left" width="80%" valign="top">Entropy Inverter</td>
        <td align="center" valign="top"><%- droid.pct_EntropyInvrtr %></td>
       </tr>
       <tr>
        <td align="left" width="80%" valign="top">Plasma Transistor</td>
        <td align="center" valign="top"><%- droid.pct_PlasmaTrans %></td>
       </tr>
       <tr>
        <td align="left" width="80%" valign="top">Superconducting Relay Unit</td>
        <td align="center" valign="top"><%- droid.pct_SprCondRelay %></td>
       </tr>
       <tr>
        <td align="left" width="80%" valign="top">Antimatter-Matter Converter</td>
        <td align="center" valign="top"><%- droid.pct_AMConverter %></td>
       </tr>
       <tr>
        <td align="left" width="80%" valign="top">Tachyon Condensator</td>
        <td align="center" valign="top"><%- droid.pct_TachyonCond %></td>
       </tr>
      </tbody></table>
     </div>
    </div>
   </div>
  </div>
  <% end %>
 </div>
</div>
]]
	--	write string to file
	modWP_Droid.modcommon.Process.DataToFile(filepath, table.concat(wikitext, "\n"))
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
