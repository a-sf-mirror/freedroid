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
--	lua module for parsing FDRPG item specs file data
local modWP_Items = {}
--	modWPCommon reference
modWP_Items.modcommon = {}
--------------------
--	variables populated by item_spec need to be declared above functions
--------------------
--	container for all item_list elements parsed
modWP_Items.itemlist = {}

-- special sorting for "all data table"
-- [in]	a|b	elements in all data table
function modWP_Items.SpecialAllDataSort(a,b)
	return a.name < b.name
end

--------------------
--	table of filepaths to FDRPG files used for parsing item information
modWP_Items.files = {
	items = ""
}

--	Function has to be in "global" space and is called "automagically" when
--	dofile(item_specs.lua) is called. Function used for parsing item_list
--	data items. Variable itemlist populated by this function.
--	[in] b	item_list from file.
function item_list( b )
	if (type(b) == "table") then
		modWP_Items.itemlist = b
	end
end

--	table of item slots
--	1) local key, 2) Display Text 3) id value in item_specs.lua
modWP_Items.slots = {
	{ "weapons", "Weapons", "weapon" },
	{ "sheilds", "Shields", "shield" },
	{ "armour", "Armour", "armour" },
	{ "drive", "Drive", "drive" },
	{ "special", "Special", "special" },
	{ "others", "Others", "none" }
}

--	text for Item presentation
--	parentID array of sequential Ids of lua tables to id.
--	== ex. weapon.bullet.type -> parent {weapon, bullet} id= type
--	label - presentation Text
-- note: inventory.size = concat string inventory.x + inventory.y
modWP_Items.textItem = {
	{ id = "levelnumber",     parentId = {},                       label = "ID",                       },
	{ id = "name",            parentId = {},                       label = "Name",                     },
	{ id = "slot",            parentId = {},                       label = "Slot",                     },
	{ id = "description",     parentId = {},                       label = "Description",              },
	{ id = "rotation_series", parentId = {},                       label = "Rotation Series",          },
	{ id = "durability",      parentId = {},                       label = "Durability",               },
	{ id = "base_price",      parentId = {},                       label = "Base Price",               },
	{ id = "use_help",        parentId = {},                       label = "Use Help",                 },
	{ id = "tux_part",        parentId = {},                       label = "Tux Part",                 },
	{ id = "armor_class",     parentId = {},                       label = "Armour Class",             },
	{ id = "size",            parentId = {"inventory"},            label = "Inventory.Size(x,y)",      },
	{ id = "stackable",       parentId = {"inventory"},            label = "Inventory.Is_Stackable",   },
	{ id = "image",           parentId = {"inventory"},            label = "Inventory Image",          },
	{ id = "class",           parentId = {"drop"},                 label = "Drop Class",               },
	{ id = "number",          parentId = {"drop"},                 label = "Drop Number",              },
	{ id = "sound",           parentId = {"drop"},                 label = "Drop Sound",               },
	{ id = "damage",          parentId = {"weapon"},               label = "Damage",                   },
	{ id = "melee",           parentId = {"weapon"},               label = "Weapon.Melee",             },
	{ id = "motion_class",    parentId = {"weapon"},               label = "Weapon.Motion_Class",      },
	{ id = "attack_time",     parentId = {"weapon"},               label = "Weapon.Attack_Time",       },
	{ id = "reloading_time",  parentId = {"weapon"},               label = "Reloading Time",           },
	{ id = "reloading_sound", parentId = {"weapon"},               label = "Reloading Sound",          },
	{ id = "type",            parentId = {"weapon", "bullet"},     label = "Weapon.Bullet.Type",       },
	{ id = "speed",           parentId = {"weapon", "bullet"},     label = "Weapon.Bullet.Speed",      },
	{ id = "lifetime",        parentId = {"weapon", "bullet"},     label = "Weapon.Bullet.Lifetime",   },
	{ id = "id",              parentId = {"weapon", "ammunition"}, label = "Ammunition",               },
	{ id = "clip",            parentId = {"weapon", "ammunition"}, label = "Ammunition Clip Size",     },
	{ id = "strength",        parentId = {"requirements"},         label = "Requirements - Strength",  },
	{ id = "dexterity",       parentId = {"requirements"},         label = "Requirements - Dexterity", },
	{ id = "tooltip",         parentId = {"right_use"},            label = "Use - Tip",                },
	{ id = "add_skill",       parentId = {"right_use"},            label = "Use - Skill",              },
}

-- retrieve the anchortext associated with this id value
function modWP_Items.GetItemUrlText( idvalue )
	local retText = ""
	if ( not idvalue ) then
		return retText
	end
	local item = select(1,modWP_Items.modcommon.Extract.GetTableItem(modWP_Items.itemlist, "id", idvalue))
	if ( item ~= nil ) then
		retText = modWP_Items.modcommon.outputfilenames.items .. item.urlAnchor
	end
	return retText
end

--	retrieve modWP_Levels.textLevel item for a given id
--	[in]	idvalue		id if value to retrieve from textItem
--	[in]	idparent	id of parent value
--	[ret]	itemtext object
function modWP_Items.GetItemText( idvalue, idparent )
	local retObj = {}
	local foundIndex = -1
	local patternRetrieve = true
	if (( idvalue == nil ) or ( type(idvalue) ~= 'string' ) or ( idvalue:len() <= 0 )) then
		return retObj
	end		-- valid id text?
	for key, textItem in pairs(modWP_Items.textItem) do
		if ( textItem.id ~= idvalue ) then
			goto GETITEMTEXT_NEXT_ITEM end	-- ids do not match
		if (( idparent ~= nil ) and ( idparent:len() > 0 )) then
			local item = select(1,modWP_Items.modcommon.Extract.GetTableItem(modWP_Items.textItem[key].parentId, nil, idparent))
			if ( item ~= nil) then
				foundIndex = key
				break
			end -- found matching parent
		else
			foundIndex = key
			break
		end	-- search parent
::GETITEMTEXT_NEXT_ITEM::
	end	-- loop through text array
	if ( foundIndex >= 1 ) then
		retObj = modWP_Items.modcommon.Extract.TblDeepCopy(modWP_Items.textItem[foundIndex])
	end
	return retObj
end

--	Look up itemData variable value and its presentation label
--	Assumes modWP_Items.itemlist is populated and processed
--	[in]	itemData	item entry object under examination
--	[in]	idvalue		label/data pair to be retrieved
--	[in]	idparent	id of parent value
--	[ret]	label,data	values
function modWP_Items.GetItemStringsPair( itemData, idvalue, idparent )
	local retLabel, retData = nil, nil
	if (( idvalue == nil ) or ( type(idvalue) ~= 'string' ) or ( idvalue:len() <= 0 )) then
		return retLabel, retData
	end
	local itemtextobj = modWP_Items.GetItemText( idvalue, idparent )
	if ( itemtextobj ~= nil ) then
		retLabel = itemtextobj.label
		if	(( idvalue == "size") and ( idparent == "inventory" )) then
			retData = tostring(itemData.inventory.x) .. " , " .. tostring(itemData.inventory.y)
		else
			-- recursive object build based on parentId table
			if (( idparent ~= nil ) and ( idparent:len() > 0 )) then
				local itemObj = itemData
				for keyValue = 1, #itemtextobj.parentId do
					if ( itemObj == nil) then
						break
					end
					itemObj = itemObj[itemtextobj.parentId[keyValue]]
				end
				retData = itemObj[idvalue]
			else
				retData = itemData[idvalue]
			end	-- parent supplied to function
		end	-- action based on idvalue
	end	-- found textItemObj
	if ( retData == nil ) then
		retData = "No Data"
	end
	return retLabel, retData
end

--	Read in FDRPG item_specs.lua data file and process information
--	for each item found. All item information is saved into modWP_Items.itemlist
function modWP_Items.ProcessData()
	modWP_Items.modcommon = require("modWPCommon")
	modWP_Items.files.items = tostring(modWP_Items.modcommon.paths.srcMap .. modWP_Items.modcommon.datafiles["items"])
	modWP_Items.modcommon.Test.Files(modWP_Items.files)
	-- process data - parse lua file first to remove gettext markers, load it, execute into global space
	-- execution calls the global list_item function populating modWP_Items.itemlist
	local luatext = modWP_Items.modcommon.Process.FileToChunk(modWP_Items.files.items)
	local stuff = load(luatext)
	stuff()
	-- remove "buggy item" from list
	for key, item in pairs(modWP_Items.itemlist) do
		local buggytext = "The first item is VERY buggy ingame, so don't use it."
		if ( select( 1, item.id:find(buggytext, 1, true)) == 1) then
			table.remove(modWP_Items.itemlist, key)	-- found "buggy item"
			break
		end
	end	-- loop through item list
	-- save anchor url and image reference information
	for key, item in pairs(modWP_Items.itemlist) do
		item["urlAnchor"] = modWP_Items.modcommon.Wiki.HLink .. modWP_Items.modcommon.Wiki.WikifyLink( item.id )
		local srcpath = ""
		local srcpathshort = ""
		item.image = {}
		if (item.rotation_series) then
			item.image["ext"] = ".jpg"
			srcpath = item.rotation_series .. "/portrait_0001" .. item.image.ext
			srcpathshort = item.rotation_series
		else
			item.image["ext"] = ".png"
			srcpath = "inventory_image_bug" .. item.image.ext
			srcpathshort = "inventory_image_bug" .. item.image.ext
		end
		item.image["src"] = modWP_Items.modcommon.paths.srcGraphics .. "items/" .. srcpath
		item.image["dest"] = modWP_Items.modcommon.paths.destRootImg .. "Items/"
		item.image["name"] = srcpathshort:gsub("%/","_")
	end	-- loop through item list
	if (#modWP_Items.itemlist > 0) then
		table.sort( modWP_Items.itemlist, modWP_Items.SpecialAllDataSort )
	end
end

--	Write FDRPG droid information to file in a wiki format
function modWP_Items.WikiWrite()
	local modWIKI = modWP_Items.modcommon.Wiki
	local LI = modWIKI.LI
	local filename = modWP_Items.modcommon.outputfilenames.items
	local filepath = tostring(modWP_Items.modcommon.paths.destRootFile .. filename)
	local wikitext = {}
	wikitext[#wikitext + 1] = modWIKI.PageSummary("FreedroidRPG Items")
	wikitext = modWIKI.WarnAutoGen( wikitext )
	wikitext[#wikitext + 1] = modWIKI.FrameStartRight("font-size:smaller")
	wikitext[#wikitext + 1] = modWIKI.HeaderLevel(3) .. "Freedroid Items Types"
	wikitext[#wikitext + 1] = modWIKI.LinkText(modWIKI.HLink .. "allItems","Items")
	for key, slotvalue in pairs(modWP_Items.slots) do
		wikitext[#wikitext + 1] = LI .. modWIKI.LinkText(modWIKI.HLink .. slotvalue[1], slotvalue[2])
	end	--	make menu for item types (slots)
	wikitext[#wikitext + 1] = modWIKI.FrameEnd
	--	end menu
	--	auto-generated text warning - spoilers
	wikitext = modWIKI.WarnSpoil( wikitext )
	--	page contents start here
	wikitext[#wikitext + 1] = modWIKI.HeaderLevel(1) .. "Items"
	wikitext[#wikitext + 1] = modWIKI.LinkText(modWIKI.HLink .. "allItems")
	wikitext[#wikitext + 1] = modWIKI.LineSep
	-- loop to produce items wiki presentation
	for key, slotvalue in pairs(modWP_Items.slots)do
		wikitext[#wikitext + 1] = modWIKI.LinkText(modWIKI.HLink .. slotvalue[1])
		wikitext[#wikitext + 1] = modWIKI.HeaderLevel(2) .. slotvalue[2]
		for subkey, item in pairs(modWP_Items.itemlist) do
			-- test item slot against selected modWP_Items.slots
			if ((item.slot == slotvalue[3])
			or	((item.slot == nil) and (slotvalue[3] == "none"))) then
				local portraitname = ""
				local textitem = {}
				local is_weapon = ( item["weapon"] ~= nil )
				-- add paths to list of images to copy
				local portaititem = modWIKI.ManagePortrait( item.image.src, item.image.dest, item.image.name, item.image.ext)
				if ( portaititem ~= nil ) then
					portraitname = portaititem.destfile
				end
				-- process presentation
				wikitext[#wikitext + 1] = modWIKI.LinkText(item["urlAnchor"])
				wikitext[#wikitext + 1] = modWIKI.HeaderLevel(3) .. item.name
				wikitext[#wikitext + 1] = modWIKI.ImageText(modWIKI.URL_ImgItems .. portraitname,item.id, "margin-right=\'1.0em\'")
				-- item data wiki presentation
				wikitext[#wikitext + 1] = modWIKI.FrameStartLeft("border=\'0px\' width=70pct")
				wikitext[#wikitext + 1] = select(2,modWP_Items.GetItemStringsPair( item, "description" ))
				wikitext[#wikitext + 1] = " "
				-- create item details
				textitem = modWP_Items.WikiPrintParentChildData(subkey, {"requirements"}, { "strength", "dexterity" }, modWIKI.ColourCaution )
				wikitext = modWIKI.PageAppend(wikitext,textitem)
				if ( is_weapon ) then
					local weapontext = ""
					local weaponcolour = nil
					if (item.weapon.two_hand) then
						weapontext = "Two Handed"
						weaponcolour = modWIKI.ColourCaution
					end
					if (item.weapon.melee) then
						weapontext = weapontext .. " Melee"
					end
					if (#weapontext > 1) then
						weapontext = weapontext .. " Weapon"
						wikitext[#wikitext + 1] = modWIKI.TextEntry( weapontext, nil, nil, weaponcolour )
					end
				end
				textitem = modWP_Items.WikiPrintParentChildData( subkey, { "base_price", "durability", "armor_class" } )
				wikitext = modWIKI.PageAppend(wikitext,textitem)

				if (item.drop.sound ~= nil) then
					local sfsndfile = modWIKI.URL_SF .. "sound/effects/item_sounds/" .. item.drop.sound
					local sndfilelink = modWIKI.LinkText( sfsndfile, item.drop.sound )
					item.drop.sound = sndfilelink
					textitem = modWP_Items.WikiPrintParentChildData( subkey, { "drop" }, { "sound"} )
					wikitext = modWIKI.PageAppend(wikitext,textitem)
				end

				if ( is_weapon ) then
					textitem = modWP_Items.WikiPrintParentChildData( subkey, { "weapon" }, { "damage", "reloading_time"} )
					wikitext = modWIKI.PageAppend(wikitext,textitem)

					if (item.weapon.reloading_sound ~= nil) then
						local ret = modWP_Items.modcommon.Extract.Split(item.weapon.reloading_sound, "/", false )
						local size = table.maxn(ret)
						local sfsndfile = modWIKI.URL_SF .. "effects/item_sounds/" .. item.weapon.reloading_sound
						local sndfilelink = modWIKI.LinkText( sfsndfile, ret[size] )
						item.weapon.reloading_sound = sndfilelink
						textitem = modWP_Items.WikiPrintParentChildData( subkey, { "weapon" }, { "reloading_sound"} )
						wikitext = modWIKI.PageAppend(wikitext,textitem)
					end

					textitem = modWP_Items.WikiPrintParentChildData( subkey, {"weapon", "ammunition"}, { "id" }, nil, true )
					wikitext = modWIKI.PageAppend(wikitext,textitem)

					textitem = modWP_Items.WikiPrintParentChildData( subkey, {"weapon", "ammunition"}, { "clip" } )
					wikitext = modWIKI.PageAppend(wikitext,textitem)
				end	-- have weapon

				textitem = modWP_Items.WikiPrintParentChildData( subkey, {"right_use"}, { "tooltip", "add_skill" } )
				wikitext = modWIKI.PageAppend(wikitext,textitem)
				wikitext[#wikitext + 1] = " "
				wikitext[#wikitext + 1] = modWIKI.FrameEnd
				wikitext[#wikitext + 1] = modWIKI.ForceBreak
			end	-- test item slot value
		end	-- loop through item list
		wikitext[#wikitext + 1] = modWIKI.LineSep
	end	-- loop through slots
	-- write wiki data object to string
	local writedata = modWIKI.PageProcess( filename, wikitext )
	-- write string to file
	modWP_Items.modcommon.Process.DataToFile(filepath, writedata)
end

--	wiki format parent/child item data
function modWP_Items.WikiPrintParentChildData( itemIndex, parentID, childrenID, textColour, makelink )
	local modWIKI = modWP_Items.modcommon.Wiki
	local SEP = modWIKI.Separator
	local text = {}
	if ((( parentID == nil ) or ( itemIndex == nil ))
		or ( type(parentID) ~= 'table' )) then
		return text
	end
	if ( makelink == nil ) then
		makelink = false
	end
	local item = modWP_Items.itemlist[itemIndex]
	if ( childrenID == nil ) then
		for key, child in pairs(parentID) do
			if ( item[child] ~= nil ) then
				local itemlabel ,itemdata = modWP_Items.GetItemStringsPair( item, child )
				text[#text + 1] = modWIKI.TextEntry( itemlabel, itemdata, SEP, textColour)
			end
		end
	else
		local itemObj = item
		for key, parent in pairs(parentID) do
			if ( itemObj[parent] ~= nil) then
				itemObj = itemObj[parent]
			else
				return text
			end
		end
		for key, child in pairs(childrenID) do
			if ( itemObj[child] ~= nil ) then
				local itemlabel,itemdata = modWP_Items.GetItemStringsPair( item, child, parentID[#parentID] )
				if ( makelink ) then
					local linktext = modWIKI.HLink .. modWIKI.WikifyLink(itemdata)
					itemdata = modWIKI.LinkText(linktext, itemdata)
				end
				text[#text + 1] = modWIKI.TextEntry( itemlabel, itemdata, SEP, textColour )
			end
		end
	end
	return text
end

-- Print out Items information based on selected verbosity.
function modWP_Items.Verbosity()
	if (( not modWP_Items.modcommon.verbose) and ( not modWP_Items.modcommon.doubleverbose)) then
		return
	end
	io.stdout:write(modWP_Items.modcommon.VerboseHeader)
	io.stdout:write("modWP_Items\n")
	io.stdout:write("number of item spec: " .. #modWP_Items.itemlist .. "\n")
	io.stdout:write(modWP_Items.modcommon.VerboseHeader)
	if (modWP_Items.modcommon.doubleverbose) then
		modWP_Items.modcommon.Process.TblPrint(modWP_Items.itemlist, nil, nil, "All Item Data")
	end
end

return modWP_Items
