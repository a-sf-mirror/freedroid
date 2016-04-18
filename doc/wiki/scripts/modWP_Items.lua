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
	{ "shields", "Shields", "shield" },
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
	local modWIKI = modWP_Items.modcommon.Wiki
	if ( not idvalue ) then
		return retText
	end
	local item = select(1,modWP_Items.modcommon.Extract.GetTableItem(modWP_Items.itemlist, "id", idvalue))
	if ( item ~= nil ) then
		retText = modWP_Items.modcommon.outputfilenames.items .. modWIKI.HLink .. modWIKI.WikifyLink(item.name)
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
	modWP_Items.files.items = tostring(modWP_Items.modcommon.paths.srcData .. modWP_Items.modcommon.datafiles["items"])
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
	local filename = modWP_Items.modcommon.outputfilenames.items .. ".html.md.eco"
	local filepath = tostring(modWP_Items.modcommon.paths.destRootFile .. filename)
	local wikitext = {}

	wikitext[#wikitext + 1] = "---"
	wikitext[#wikitext + 1] = "layout: 'page'"
	wikitext[#wikitext + 1] = "title: 'Items Guide'"
	wikitext[#wikitext + 1] = "comment: 'Characteristics of the items (weapons, ammunition, shield, armor) available in game.'"
	wikitext[#wikitext + 1] = ""
	wikitext[#wikitext + 1] = "categories:"

	-- loop to produce items wiki presentation
	for key, slotvalue in pairs(modWP_Items.slots) do
		modWIKI.StartMapping()
		wikitext[#wikitext + 1] = modWIKI.AddAttr('id', slotvalue[1])
		wikitext[#wikitext + 1] = modWIKI.AddAttr('name', slotvalue[2])
		wikitext[#wikitext + 1] = modWIKI.AddAttr('items', nil)
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
				modWIKI.StartMapping()
				wikitext[#wikitext + 1] = modWIKI.AddAttr('id', modWIKI.WikifyLink(item.name))
				wikitext[#wikitext + 1] = modWIKI.AddAttr('name', item.name)
				wikitext[#wikitext + 1] = modWIKI.AddAttr('image', portraitname)
				-- item data wiki presentation
				wikitext[#wikitext + 1] = modWIKI.AddAttr('description', item.description) --select(2,modWP_Items.GetItemStringsPair( item, "description" )))
				-- create item details
				if item.requirements or (is_weapon and item.weapon.two_hand) then
					wikitext[#wikitext + 1] = modWIKI.AddAttr('requirements', nil)
					modWIKI.StartSequence()
					if item.requirements then
						if item.requirements.strength then
							wikitext[#wikitext + 1] = modWIKI.AddAttr('strength', item.requirements.strength)
						end
						if item.requirements.dexterity then
							wikitext[#wikitext + 1] = modWIKI.AddAttr('dexterity', item.requirements.dexterity)
						end
					end
					if ( is_weapon ) then
						local weapontext = ""
						if (item.weapon.two_hand) then
							weapontext = "Two Handed"
							weaponcolour = modWIKI.ColourCaution
							if (item.weapon.melee) then
								weapontext = weapontext .. " Melee"
							end
							weapontext = weapontext .. " Weapon"
							wikitext[#wikitext + 1] = modWIKI.AddAttr('two_hands', weapontext)
						end
					end
					modWIKI.EndSequence()
				end
				if item.base_price then
					wikitext[#wikitext + 1] = modWIKI.AddAttr('base_price', item.base_price)
				end
				if item.durability then
					wikitext[#wikitext + 1] = modWIKI.AddAttr('durability', item.durability)
				end
				if item.armor_class then
					wikitext[#wikitext + 1] = modWIKI.AddAttr('armor_class', item.armor_class)
				end

				if item.drop then
					wikitext[#wikitext + 1] = modWIKI.AddAttr('drop', nil)
					modWIKI.StartSequence()
					if item.drop.class then
						wikitext[#wikitext + 1] = modWIKI.AddAttr('class', item.drop.class)
					end
					if item.drop.number then
						wikitext[#wikitext + 1] = modWIKI.AddAttr('number', item.drop.number)
					end
					if item.drop.sound then
						local sfsndfile = modWIKI.URL_Git .. "sound/effects/item_sounds/" .. item.drop.sound
						local sndfilelink = modWIKI.LinkText( sfsndfile, item.drop.sound )
						item.drop.sound = sndfilelink
						wikitext[#wikitext + 1] = modWIKI.AddAttr('sound', item.drop.sound)
					end
					modWIKI.EndSequence()
				end

				if ( is_weapon ) then
					wikitext[#wikitext + 1] = modWIKI.AddAttr('weapon', nil)
					modWIKI.StartSequence()
					if item.weapon.damage then
						wikitext[#wikitext + 1] = modWIKI.AddAttr('damage', item.weapon.damage)
					end
					if item.weapon.reloading_time then
						wikitext[#wikitext + 1] = modWIKI.AddAttr('reloading_time', item.weapon.reloading_time)
					end
					if item.weapon.reloading_sound then
						local ret = modWP_Items.modcommon.Extract.Split(item.weapon.reloading_sound, "/", false )
						local size = table.maxn(ret)
						local sfsndfile = modWIKI.URL_Git .. "sound/" .. item.weapon.reloading_sound
						local sndfilelink = modWIKI.LinkText( sfsndfile, ret[size])
						item.weapon.reloading_sound = sndfilelink
						wikitext[#wikitext + 1] = modWIKI.AddAttr('reloading_sound', item.weapon.reloading_sound)
					end
					if item.weapon.ammunition then
						wikitext[#wikitext + 1] = modWIKI.AddAttr('ammunition', nil)
						modWIKI.StartSequence()
						if item.weapon.ammunition.id then
							local amm_link = modWIKI.LinkText(modWP_Items.GetItemUrlText(item.weapon.ammunition.id), item.weapon.ammunition.id)
							wikitext[#wikitext + 1] = modWIKI.AddAttr('id', amm_link)
						end
						if item.weapon.ammunition.clip then
							wikitext[#wikitext + 1] = modWIKI.AddAttr('clip', item.weapon.ammunition.clip)
						end
						modWIKI.EndSequence()
					end

					modWIKI.EndSequence()
				end	-- have weapon

				if item.right_use then
					wikitext[#wikitext + 1] = modWIKI.AddAttr('right_use', nil)
					modWIKI.StartSequence()
					if item.right_use.tooltip then
						wikitext[#wikitext + 1] = modWIKI.AddAttr('tooltip', item.right_use.tooltip)
					end
					if item.right_use.add_skill then
						wikitext[#wikitext + 1] = modWIKI.AddAttr('add_skill', item.right_use.add_skill)
					end
					modWIKI.EndSequence()
				end

				modWIKI.EndMapping()
			end	-- test item slot value

		end	-- loop through item list
		modWIKI.EndMapping()
	end	-- loop through slots

	wikitext[#wikitext + 1] = "---"
	wikitext[#wikitext + 1] = ""
	wikitext[#wikitext + 1] = [[
# Items Guide

<div class="row">
 <div class="toc col-md-2 pull-right">
  <span><b>Categories</b></span>
  <ul>
  <% for cat in @document.categories: %>
   <li><a href="#<%- cat.id %>"><%- cat.name %></a></li>
  <% end %>
  </ul>
 </div>

 <div class="col-md-10">
 <% for cat in @document.categories: %>
  <h1 id="<%- cat.id %>"><%- cat.name %></h1>
  <% for item in cat.items: %>
   <div class="row">
    <h2 id="<%- item.id %>"><%- item.name %></h2>
    <div class="obj-portrait col-md-2 text-center">
     <img src="/images/items/<%- item.image %>">
    </div>
    <div class="obj-sheet col-md-8">
     <p><%- item.description %></p>
     <% if item.requirements: %>
      <% if item.requirements.strength: %><span class="require"><b>Requirements - Strength:</b> <%- item.requirements.strength %></span><br/><% end %>
      <% if item.requirements.dexterity: %><span class="require"><b>Requirements - Dexterity:</b> <%- item.requirements.dexterity %></span><br/><% end %>
      <% if item.requirements.two_hands: %><span class="require"><b><%- item.requirements.two_hands %></b></span><br/><% end %>
     <% end %>
     <% if item.base_price: %><b>Base Price:</b> <%- item.base_price %><br/><% end %>
     <% if item.durability: %><b>Durability:</b> <%- item.durability %><br/><% end %>
     <% if item.armor_class: %><b>Armor Class:</b> <%- item.armor_class %><br/><% end %>
     <% if item.drop: %>
      <% if item.drop.sound: %><b>Drop Sound:</b> <%- item.drop.sound %><br/><% end %>
     <% end %>
     <% if item.weapon: %>
      <% if item.weapon.damage: %><b>Damage:</b> <%- item.weapon.damage %><br/><% end %>
      <% if item.weapon.reloading_time: %><b>Reloading Time:</b> <%- item.weapon.reloading_time %><br/><% end %>
      <% if item.weapon.reloading_sound: %><b>Reloading Sound:</b> <%- item.weapon.reloading_sound %><br/><% end %>
      <% if item.weapon.ammunition: %>
       <% if item.weapon.ammunition.id: %><b>Ammunition:</b> <%- item.weapon.ammunition.id %><br/><% end %>
       <% if item.weapon.ammunition.clip: %><b>Ammunition Clip Size:</b> <%- item.weapon.ammunition.clip %><br/><% end %>
      <% end %>
     <% end %>
     <% if item.right_use: %>
      <% if item.right_use.tooltip: %><b>Use - Tip:</b> <%- item.right_use.tooltip %><br/><% end %>
      <% if item.right_use.add_skill: %><b>Use - Skill:</b> <%- item.right_use.add_skill %><br/><% end %>
     <% end %>
    </div>
  </div>
  <% end %>
 <% end %>
 </div>
</div>
]]
	modWP_Items.modcommon.Process.DataToFile(filepath, table.concat(wikitext, "\n"))
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
