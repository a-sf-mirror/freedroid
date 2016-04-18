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
--	file:	wiki_parse.lua
--	Parsing FDRPG data files to wiki pages


--	extract arguments from command line
--	src: http://lua-users.org/wiki/AlternativeGetOpt
--	[ret]	table of key,value pairs of arguments to script
function getopt()
	local tab = {}
	for k, v in ipairs(arg) do
		if (string.sub( v, 1, 1 ) == "-") then
			local x = string.find( v, "=", 1, true )
			if x then tab[ string.sub( v, 2, x-1 ) ] = string.sub( v, x+1 )
			else      tab[ string.sub( v, 2 ) ] = true
			end
		end
	end
	return tab
end

--	print script usage
function usage ()
	io.stderr:write("\n")
	io.stderr:write("Usage: (from FDRPG root)\n\n")
	io.stderr:write("lua " .. arg[0] .. " [options] \n")
	io.stderr:write("options:\n")
	io.stderr:write("\t -s\t enable sandbox option - disables some wiki conversions\n")
	io.stderr:write("\t -l\t enable saving image linking list file\n")
	io.stderr:write("\t -v\t enable verbose output to stdout\n")
	io.stderr:write("\t -vv\t enable VERY verbose output to stdout (recommend pipe output to file)\n")
	io.stderr:write("\n")
	os.exit(0)
end

---------- path handling
--	path assumptions
--	==>	FDRPG_Root/wiki/lua			<== location of code
--	==> FDRPG_Root/wiki/			<== resultant output of wiki parsing
--	==>	FDRPG_Root/dialogs/*.lua	<== npc dialogs
--	==> FDRPG_Root/map/*			<== game data files
local destRoot = "./doc/wiki/"
local srcRoot = "./"
local OrigPath = package.path
package.path = package.path .. ";" .. destRoot .. "/scripts/?.lua"

--	names of lua modules used for wiki parsing put into the global space ON PURPOSE
--	NOTE: if you change the order - update all instances of moduleNames[x]
moduleNames = {
	{ id = "modWP_ROTD",	data = true, wiki = false },
	{ id = "modWP_Events",	data = true, wiki = false },
	{ id = "modWP_Items",	data = true, wiki = true },
	{ id = "modWP_Levels",	data = true, wiki = true },	-- dependent on above
	{ id = "modWP_Droid",	data = true, wiki = true },	-- dependent on above
	{ id = "modWP_NPC",		data = true, wiki = true },	-- dependent on above
	{ id = "modWP_Quests",	data = true, wiki = true },	-- dependent on above
	{ id = "modWP_Dialog",	data = true, wiki = false },-- special handling
}

--	setup common paths needed for all modules
local modcommon = require("modWPCommon")
modcommon.paths.scriptpath = destRoot .. "scripts/"
modcommon.paths.destRootFile = destRoot .. "wiki.d/"
modcommon.paths.destRootImg = destRoot .. "uploads/"
modcommon.paths.srcroot = srcRoot
modcommon.paths.srcData = srcRoot .. "data/"
modcommon.paths.srcDialog = srcRoot .. "data/storyline/act1/dialogs/"
modcommon.paths.srcGraphics = srcRoot .. "data/graphics/"
--	test existence of directories
if ( ( not modcommon.Test.DirExists(modcommon.paths.srcGraphics))
	or ( not modcommon.Test.DirExists(modcommon.paths.srcDialog))
	or ( not modcommon.Test.DirExists(modcommon.paths.srcMap))
	) then
	io.stderr:write("Unable to locate needed FDRPG folders.\n Exiting Script\n")
	os.exit(1)
end

--	create destination directories if needed
if (not modcommon.Test.FileExists(modcommon.paths.destRootImg .. "/*")) then
	os.execute("mkdir -p " .. modcommon.paths.destRootImg)
end
if (not modcommon.Test.FileExists(modcommon.paths.destRootFile .. "/*")) then
	os.execute("mkdir -p " .. modcommon.paths.destRootFile)
end

--	get command line options
local opts = getopt()
for k, value in pairs(opts) do
	--	todo parse for help/usage
	if (k == "s") then
		--	is output to be used in pmwiki sandbox?
		modcommon.sandbox = value
	elseif (k == "l") then
		--	need output list of files to be copied?
		modcommon.filecopyoutput = value
	elseif (k == "v") then
		--	verbose printing of parsing information
		modcommon.verbose = value
	elseif (k == "vv") then
		--	detailed verbose output
		modcommon.doubleverbose = value
	else
		usage()
	end
end

--	load needed modules and process data
local modules = {}
for key, modItem in pairs(moduleNames)do
	local wpmod = require(modItem.id)
	wpmod.ProcessData()
	if ( modItem.wiki == true ) then
		modules[#modules + 1] = wpmod
	else
		wpmod.Verbosity()
	end
end
--	first ^^ process all data
--		wiki depends on processed data
--	second __ do the wiki pages
for key, modItem in pairs(modules)do
	modItem.WikiWrite()
	modItem.Verbosity()
end
---------- display/manage image link information
io.stdout:write("\n")
modcommon.Process.FileLinkAction()
package.path = OrigPath
---------- end processing
