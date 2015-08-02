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
--	common routines used for parsing FDRPG data files to wiki pages
local modWPCommon = {}
----------------------------------------
-- Wiki Parsing Common Variables
----------------------------------------
--	Enable flag to use the output from wiki parsing in a pmwiki sandbox\n
--	This flag is meant to be used for debuging purposes
modWPCommon.sandbox = false

--	Enable flag to produce a listing of files that needed to be linked\n
--	This flag is meant to be used for debuging purposes\n
--	Enabling this flag causes the file output_images.txt to be retained\n
--	in the wiki parsing documents output directory.
modWPCommon.filecopyoutput = false

--	Enable flag to produce verbose output\n
--	This flag is meant to be used for debuging purposes\n
--	Flag will result in printout of basic module parsing results.
modWPCommon.verbose = false

--	Enable flag to produce extra verbose output\n
--	This flag is meant to be used for debuging purposes\n
--	Flag will result in printout of detailed module table contents as well as
--	verbose output results.
modWPCommon.doubleverbose = false

--	Values are set in wiki_parse.lua after loading this module\n
--	Table becomes available to all modules that load modWPCommon.
modWPCommon.paths = {
	scriptpath = "",
	destRootFile = "",
	destRootImg = "",
	srcroot = "",
	srcMap = "",
	srcDialog = "",
	srcGraphics = ""
}

--	FDRPG data files that are parsed by this script
modWPCommon.datafiles = {
	droid	  = "droid_archetypes.dat",
	events	  = "events.dat",
	items	  = "item_specs.lua",
	levels	  = "levels.dat",
	npc		  = "npc_specs.lua",
	quests    = "quests.dat",
	rotd	  = "ReturnOfTux.droids"
}

--	file names of wiki pages resulting from wiki parsing
modWPCommon.outputfilenames = {
	droids		= "Doc.Droids",
	items		= "Doc.Items",
	levels		= "Doc.Levels",
	npc			= "Doc.NPC",
	quests		= "Doc.Quest",
}
--	Text to delineate sections when printing verbose data
modWPCommon.VerboseHeader = string.rep("-",20) .. "\n"

--	Lua text search delimiters for block comments
modWPCommon.LuaBlkCommentStart = "%-%-%[%["
modWPCommon.LuaBlkCommentEnd = "%]%]%-%-"

--	holding variable for image files that need to be linked.
--	each entry - { srcpath = "", destpath = "", destfile = ""}
modWPCommon.FilesToLink = {}
-- variable naming dialogs to ignore
modWPCommon.IgnoreDialog = { "AfterTakeover", "Singularity-Drone", "TestDroid", "FactionDeadBot" }
-- variable naming ROTD.types to ignore (not necessarily characters)
modWPCommon.IgnoredTypes = { "TRM", "GUN" }
-- variable naming factions to ignore
modWPCommon.IgnoreFaction = { "ms", "test" }

----------------------------------------
-- Wiki Parse Processing functions
----------------------------------------
modWPCommon.Process = {}
--	write to a file in bulk
--	[in] filename	path/filename destination of data
--	==  File path is not multiplatform aware. A "nil" file path redirects output to stdout.
--	[in] writedata	data to be written to output stream
--	[in] appending	appending to existing file
function modWPCommon.Process.DataToFile( filename, writedata, appending )
	local writemode = ""
	if (appending) then
		writemode = "a"
	else
		writemode = "w"
	end
	if (filename ~= nil) then
		local filedata = io.open(filename, writemode)
		if (filedata == nil) then
			io.stderr:write("error - file " .. filename .. ". Unable to open file for writing. Exiting Script\n")
			os.exit(1)
		else
			filedata:write(writedata)
			assert(filedata:close())
		end
	else
		io.stdout:write(writedata)
	end
end

--	parse a text file into a table line-by-line (without carriage returns)
--	each line of data is placed into its own table element.
--	[in] filename	path/filename of file to read
--	[ret]	table with each element being a line of text read from file
function modWPCommon.Process.FileToLines( filename )
	local filedata = io.open(filename,"r")
	local lines = {}
	if (filedata == nil) then
		io.stderr:write("error - file " .. filename .. ". Unable to open file for reading. Exiting Script\n")
		os.exit(1)
	else
		for line in filedata:lines() do
			lines[#lines + 1] = line
		end
	end
	if ((modWPCommon.verbose) or (modWPCommon.doubleverbose)) then
		io.stdout:write(filename .. " linecount: " .. #lines .. "\n")
	end
	return lines
end

--	parse/preprocess a lua file
--	Function reads a lua file in bulk. Preprocessing occurs to remove gettext markers.
--	The entire file is then loaded into memory. Use dofile() on the returned chunk to execute.
--	[in] filename	path/filename of file to read
--	[ret]	lua chunk
function modWPCommon.Process.FileToChunk( filename )
	local filedata = io.open(filename,"r")
	local lines = {}
	local chunk = ""
	if (filedata == nil) then
		io.stderr:write("error - file " .. filename .. ". Unable to open file for reading. Exiting Script\n")
		os.exit(1)
	else
		local pattern_marker_quote = "_\""
		local patter_marker_block = "_%[%["
		for line in filedata:lines() do
			local text =  line:gsub(pattern_marker_quote, "\"")
			text = text:gsub(patter_marker_block,"[[")
			lines[#lines + 1] = text
		end
	end
	if ((modWPCommon.verbose) or (modWPCommon.doubleverbose)) then
		io.stdout:write(filename .. " linecount: " .. #lines .. "\n")
	end
	for key, line in pairs(lines)do
		chunk = chunk .. line .. "\n"
	end
	return chunk
end

--	search for text value in table. If value not found insert to table
--	[in]	tablename	adding data to this table
--	[in]	textToAdd	adding this text to table
function modWPCommon.Process.InsertToNoKeyTable( tablename, textToAdd )
	if (( tablename == nil)
		or ( type(tablename) ~= 'table' )) then
		return
	end
	if (( textToAdd == nil )
		or ( type(textToAdd) ~= 'string' )
		or ( textToAdd:len() <= 0 )) then
		return
	end
	local value = select(1,modWPCommon.Extract.GetTableItem( tablename, nil, textToAdd ))
	if ( value == nil ) then
		table.insert( tablename, textToAdd )
	end
end

--	printout contents of lua table ( recursive function )
--	see http://lua-users.org/wiki/TableSerialization
--	[in] tt	table to parse
--	[in] indent	amount of indentation for this iteration (default 0)
--	[in] done	boolean to indicate completion of processing table
function modWPCommon.Process.TblPrint( tt, indent, done , headlabel)
	done = done or {}
	indent = indent or 0
	if ( headlabel ~= nil ) then
		io.stdout:write(modWPCommon.VerboseHeader .. "\n");
		io.stdout:write(headlabel .. "\n");
		io.stdout:write(modWPCommon.VerboseHeader .. "\n");
	end
	if type(tt) == "table" then
		for key, value in pairs (tt) do
			io.stdout:write(string.rep (" ", indent)) -- indent it
			if type (value) == "table" and not done [value] then
				done [value] = true
				io.stdout:write(string.format("[%s] => table\n", tostring (key)));
				io.stdout:write(string.rep (" ", indent + 2)) -- indent it
				io.stdout:write("(\n");
				modWPCommon.Process.TblPrint (value, indent + 4, done, nil)
				io.stdout:write(string.rep (" ", indent + 2)) -- indent it
				io.stdout:write(")\n");
			else
				io.stdout:write(string.format("[%s] => %s\n",tostring(key), tostring(value)))
			end
		end
		io.stdout:write("\n")
	else
		io.stdout:write(tt .. "\n")
	end
end

--	Actions taken to create links to image files
function modWPCommon.Process.FileLinkAction()
	local verbosity = ""
	if ((modWPCommon.verbose) or (modWPCommon.doubleverbose)) then
		verbosity = " 1"
		io.stdout:write("# of image files to link: " .. #modWPCommon.FilesToLink .. "\n")
	end
	local copytext = modWPCommon.paths.destRootFile .. "output_images.txt"
	local output = ""
	for key,item in pairs(modWPCommon.FilesToLink)do
		local text = item.srcpath .. "\t\t" .. item.destpath .. item.destfile .. "\n"
		output = output .. text
	end
	modWPCommon.Process.DataToFile(copytext, output)
	local cmd = modWPCommon.paths.scriptpath .. "wpImageLink.sh " .. modWPCommon.paths.srcroot .. " "
				.. modWPCommon.paths.destRootFile .. verbosity
	local fnpopen = assert(io.popen(cmd, 'r'))
	local popentext = assert(fnpopen:read('*a'))
	io.stdout:write(popentext .. "\n");
	if (not modWPCommon.filecopyoutput) then
		os.execute( "rm " .. copytext)
	end
end

----------------------------------------
-- Wiki Parse Testing functions(text or presence of data)
----------------------------------------
modWPCommon.Test = {}
--	test for presence of file
--	[in] name	path and name of file to test(not multiplatform aware)
--	[ret]	boolean value if file found (True=yes)
function modWPCommon.Test.FileExists( name )
	local returnval = false
	local f=io.open(name,"r")
	if (f ~= nil) then
		io.close(f)
		returnval = true
	end
	return returnval
end

--	test existence of multiple files
--	[in] filenames	table of path/filenames to be tested
function modWPCommon.Test.Files( filenames )
	for item,filename in pairs(filenames) do
		local filetest = modWPCommon.Test.FileExists(filename)
		if (filetest == false) then
			io.stderr:write("error - file " .. filename .. " not found. Exiting Script\n")
			os.exit(1)
		else
			if ((modWPCommon.verbose) or (modWPCommon.doubleverbose)) then
				io.stdout:write(modWPCommon.VerboseHeader)
				io.stdout:write("found file " .. filename.. "\n")
			end
		end
	end
end

--	test if directory exists
--	[in]	path	string of path to test
--	[ret]	boolean if directory path found
function modWPCommon.Test.DirExists( path )
	local boolreturn = false
	local f  = io.popen("cd " .. path)
	local ff = f:read("*all")
	if (not ff:find("ItemNotFoundException")) then
		boolreturn = true
	end
	return boolreturn
end

----------------------------------------
-- Wiki Parse Extraction functions
----------------------------------------
modWPCommon.Extract = {}
--	extract text from string that may contain gettext markers or quotation marks
--	[in]	text	string of text to be processed
--	==  Text may be of the form _"stuff"
--	==  function will process text and return the text: stuff
--	[ret]	string without gettext or quotation marks
function modWPCommon.Extract.Text( text )
	return modWPCommon.Extract.StripQuotes(modWPCommon.Extract.StripGetText(text))
end

--	remove gettext markers from string
--	NOTE: using with stripQuotes() call stripGetText() first as
--	search is dependent on finding underscore/quotation mark combination
--	[in]	text	string of text to be processed
--	[ret]	string without gettext marks
function modWPCommon.Extract.StripGetText( text )
	return select( 1, text:gsub("(_)(\")", "%2"))
end

--	remove quotation marks from string
--	Function will look for first and last quotation mark.
--	Any other text outside range is discarded.
--	[in]	text	string of text to be processed
--	[ret]	string without quotation marks
function modWPCommon.Extract.StripQuotes( text )
	local temptext = text
	local substart = 1
	local subend = (-1)
	local SearchStart = select( 1, temptext:find("\"", 1))
	if (SearchStart ~= nil) then
		substart = (SearchStart + 1)
	end
	local revtext = temptext:reverse()
	local SearchEnd = select(1, revtext:find("\"", 1))
	if ((SearchEnd ~= nil) and (SearchEnd ~= #temptext)) then
		subend = #temptext - SearchEnd
	end
	return temptext:sub(substart, subend)
end

--	extract arguments in a lua function call
--	Assume lua call of the form func(a, b, c)
--	Function will extract these values into a table.
--
--	NOTE: default is assumed to be text-only arguments.
--	Set nonstrings = true to return non-string arugments also.
--	[in]	input	text to be examined by function
--	[in]	fnName	function call to find
--	[in]	nonstrings	boolean - args may be non-strings
--	[ret]	table of arguments found
function modWPCommon.Extract.FuncArgs( input, fnName, nonstrings )
	if ( input == nil ) or ( fnName == nil ) then
		return nil
	end
	local markerStart,markerEnd = select(1, input:find(fnName))
	if ( markerStart == nil ) then
		return nil
	end
	local altSearch = nonstrings or false
	-- ASSume markerEnd contain usable values
	local temptext = input:sub((markerEnd + 1))
	-- extract text between parentheses
	local textextract = ""
	for a,b,c in temptext:gmatch("(%()(.-)(%))")do
		-- a = "("	b = text c = ")"
		textextract = b
	end
	local foundArgs = {}
	if ( altSearch) then
		-- input is of form ( var, stuff, "text" ) - extract using different pattern
		for w in (textextract .. ","):gmatch("(.-),") do
			if ( type(w) == 'string' ) then
				local textdata = modWPCommon.Extract.StripQuotes(w)
				table.insert(foundArgs, textdata)
			else
				io.stdout:write(w .. "\n")
				table.insert(foundArgs, w)
			end
		end
	else
		-- input is of form ( "stuff", "text" ) - extract these text
		for w in (textextract .. ","):gmatch("\"(.-)\",") do
			if ( w:len() > 0 ) then
				table.insert(foundArgs, w)
			end
		end
	end	-- alternate searching
	return foundArgs
end

--	find and extract text
--	[in]	text	string of text to be examined
--	[in]	searchPattern	search text for this pattern
--	[in]	ExtractPattern	use this pattern to extract from text
--	[ret]	extracted value
function modWPCommon.Extract.SearchText( text, searchPattern, ExtractPattern )
	local retValue = nil
	local asnumber = false
	if (( text == nil ) or ( searchPattern == nil ) or ( ExtractPattern == nil )) then
		return retValue
	end
	if ( ExtractPattern:find("%%d") ~= nil ) then
		asnumber = true
	end
	local patternstart, patternend = text:find(searchPattern)
	if (( patternstart ~= nil ) and ( patternstart >= 1 )) then
		if (( ExtractPattern == "[EOL]" ) or ( ExtractPattern == "[TEXT]" )) then
			if ( ExtractPattern == "[EOL]" ) then
				retValue = text:sub((patternend + 1), -1)
			else
				retValue = text:sub((patternstart), patternend)
			end
		elseif ( ExtractPattern == "[MATCH]" ) then
			local subsearch = text:sub( patternstart, patternend )
			retValue = subsearch:match( searchPattern )
		else
			local subsearch = text:sub( patternstart, patternend )
			retValue = subsearch:sub(subsearch:find(ExtractPattern))
		end
	end
	if ( retValue ~= nil ) then
		if ( asnumber ) then
			retValue = tonumber(retValue)
		else
			retValue = modWPCommon.Extract.Text(retValue)
		end
	end
	return retValue
end

--	read a lua table of strings and return a text string of contents
--	[in] tablearray	table of strings to read and converted to text
--	[in] itemsperline	number of data elements per line of text (default 10)
--	[ret]	data from table as string with embedded carriage returns
function modWPCommon.Extract.OneDTableToString( tablearray, itemsperline, seperator )
	local str = ""
	local limit = itemsperline or 10
	local sep = " "
	if ( seperator ~= nil ) then
		sep = seperator
	end

	local itemcount = 0
	for key,value in pairs(tablearray) do
		if (key > 1) then
			str = str .. sep .. tostring(value)
		else
			str = tostring(value)
		end
		itemcount = itemcount + 1
		if (itemcount > limit) then
			str = str .. "\n"
			itemcount = 1
		end
	end
	return str
end

--	perform a deep copy of a table(table structure and values)
--	use deep copy to copy table AND default values
--	[in] orig	original table to be copied
--	[ret]	duplicate of original table
function modWPCommon.Extract.TblDeepCopy( orig )
	local orig_type = type(orig)
	local copy
	if orig_type == 'table' then
		copy = {}
		for orig_key, orig_value in next, orig, nil do
			copy[modWPCommon.Extract.TblDeepCopy(orig_key)] = modWPCommon.Extract.TblDeepCopy(orig_value)
		end
		setmetatable(copy, modWPCommon.Extract.TblDeepCopy(getmetatable(orig)))
	else -- number, string, boolean, etc
		copy = orig
	end
	return copy
end

--	tokenize a string based on a pattern
--	see: http://lua-users.org/wiki/SplitJoin
--	section on page "Function: Split a string with a pattern, Take Two"
--	[in]	pString	string to be split
--	[in]	pPattern	pattern to be used for splitting
--	[in]	pProcess	boolean - true = process strings with extractText()
--	[ret]	table of values extracted from string
function modWPCommon.Extract.Split(pString, pPattern, pProcess )
	local returnTable = {}
	local fpat = "(.-)" .. pPattern
	local last_end = 1
	local s, e, cap = pString:find(fpat, 1)
	local extracttext = pProcess or false
	while s do
		if s ~= 1 or cap ~= "" then
			if (extracttext) then
				cap = modWPCommon.Extract.Text(cap)
			end
			table.insert(returnTable,cap)
		end
		last_end = e+1
		s, e, cap = pString:find(fpat, last_end)
	end
	if last_end <= #pString then
		cap = pString:sub(last_end)
		if (extracttext) then
			cap = modWPCommon.Extract.Text(cap)
		end
		table.insert(returnTable, cap)
	end
	return returnTable
end

--	retrieve table element for a given id
--	[in] tablename	table used to search for id and retrieve value
--	[in] keyvalue	table key used to compare with idvalue
--	[in] idvalue	use the var to id table item to retrieve
--	[ret]	tablename["keyvalue"] item or nil
--	[ret]	index value in tablename where item is found or nil
function modWPCommon.Extract.GetTableItem(tablename, keyvalue, idvalue )
	local retItem = nil
	local useKeyValue = false
	local foundIndex = -1
	if ( tablename == nil ) then
		return retItem
	end
	if (( idvalue == nil ) or ( type(idvalue) ~= 'string' ) or ( idvalue:len() <= 0 )) then
		return retItem
	end
	if ((keyvalue == nil) or ( type(keyvalue) ~= 'string' ) or ( keyvalue:len() <= 0 )) then
		useKeyValue = false
	else
		useKeyValue = true
	end
	for key, element in pairs(tablename) do
		if ( useKeyValue ) then
			if ( element[keyvalue] == idvalue ) then
				foundIndex = key
				break
			end	--	found id in array
		else
			if ( element == idvalue ) then
				foundIndex = key
				break
			end
		end
	end	--	loop through table
	if ( foundIndex > 0 ) then
		retItem = tablename[foundIndex]
	end
	return retItem, foundIndex
end

----------------------------------------
-- Wiki Variables and functions
----------------------------------------
modWPCommon.Wiki = {}
--	url to FDRPG Droid wiki images
modWPCommon.Wiki.URL_ImgDroid = "Attach:Droids./"
--	url to FDRPG Droid wiki images
modWPCommon.Wiki.URL_ImgItems = "Attach:Items./"
--	url to FDRPG git repo on sourceforge
modWPCommon.Wiki.URL_Git = "https://gitlab.com/freedroid/freedroid-src/tree/master"

-- page setup
----------------------------------------
--	template for wiki page content
--	name, time, and text data is populated in the calling modue\n
modWPCommon.Wiki.Header = {
		version = "version=pmwiki-2.2.30 ordered=1 urlencoded=1",
		charset = "\ncharset=ISO-8859-1",
		author	= "\nauthor=buildbot",
		csum	= "\ncsum=autobuild-generated",
		name	= "\nname=",
		text	= "\ntext=",
		ctime	= "\ntime="
}

--	text used to indicate wiki page summary
function modWPCommon.Wiki.PageSummary( text )
	if ( text == nil ) then
		text = ""
	end
	return "(:Summary: " .. text .. ":)"
end

--	process wiki formated data and create wiki page
--	[in]	filename	filename of wiki page
--	[in]	wikitext	contents of wiki page [each table entry is a line of text]
--	[ret]	string	entire wiki page (header and content)
function modWPCommon.Wiki.PageProcess( filename, wikitext )
	local pagetext = ""
	local linefeed = ""
	if (modWPCommon.sandbox) then
		-- debug/sandbox
		linefeed = "\n"
	else
		-- production
		linefeed = "%0a"
	end
	pagetext = linefeed
	for k,text in ipairs(wikitext) do
		if ( #text > 0 ) then
			if ( modWPCommon.sandbox ) then
				pagetext = pagetext .. text .. linefeed
			else
				pagetext = pagetext .. modWPCommon.Wiki.WikifyText(text) .. linefeed
			end
		end
	end
	-- pmwiki page header data
	local wikihead = modWPCommon.Extract.TblDeepCopy(modWPCommon.Wiki.Header)
	wikihead.name = wikihead.name .. filename
	wikihead.ctime = wikihead.ctime .. tostring(os.time())
	-- write wiki data object to string
	local writedata =	wikihead.version ..
						wikihead.charset ..
						wikihead.author ..
						wikihead.csum ..
						wikihead.name ..
						wikihead.text .. pagetext ..
						wikihead.ctime
	return writedata
end

--	wiki text to display warning to not edit page
--	[in]	wikitext	contents of wiki page [each table entry is a line of text]
--	[ret]	page contents with new text appended
function modWPCommon.Wiki.WarnAutoGen( wikitext )
	local text = {}
	text[#text + 1] = modWPCommon.Wiki.CommentStart .. string.rep("=",40) .. modWPCommon.Wiki.CommentEnd
	text[#text + 1] = modWPCommon.Wiki.CommentStart .. modWPCommon.Wiki.CommentEnd
	text[#text + 1] = modWPCommon.Wiki.CommentStart .. "WARNING" .. modWPCommon.Wiki.CommentEnd
	text[#text + 1] = modWPCommon.Wiki.CommentStart .. "DO NOT EDIT PAGE - Page contents are autogenerated from source files." .. modWPCommon.Wiki.CommentEnd
	text[#text + 1] = modWPCommon.Wiki.CommentStart .. "Edits will not be preserved when page contents are regenerated. " .. modWPCommon.Wiki.CommentEnd
	text[#text + 1] = modWPCommon.Wiki.CommentStart .. modWPCommon.Wiki.CommentEnd
	text[#text + 1] = modWPCommon.Wiki.CommentStart .. string.rep("=",40) .. modWPCommon.Wiki.CommentEnd
	return modWPCommon.Wiki.PageAppend( wikitext, text )
end

--	wiki text to display warning page contains spoilers
--	[in]	wikitext	contents of wiki page [each table entry is a line of text]
--	[ret]	page contents with new text appended
function modWPCommon.Wiki.WarnSpoil( wikitext )
	local text = {}

	text[#text + 1] = modWPCommon.Wiki.LineSep
	local line = modWPCommon.Wiki.TextColour("WARNING" ,modWPCommon.Wiki.ColourWarn)
	line = modWPCommon.Wiki.TextEmbed(line, "boldemphasis")
	text[#text + 1] = modWPCommon.Wiki.HeaderLevel(4) .. line

	line = modWPCommon.Wiki.TextColour("SPOILERS" ,modWPCommon.Wiki.ColourWarn)
	line = line .. " - Page may contain spoilers about the game."
	text[#text + 1] = modWPCommon.Wiki.TextEmbed(line, "textsmall")
	text[#text + 1] = modWPCommon.Wiki.LineSep

	return modWPCommon.Wiki.PageAppend( wikitext, text )
end

-- markers
----------------------------------------
--	wiki header level format
--	[in]	level	header level to display
function modWPCommon.Wiki.HeaderLevel( level )
	local retText = ""
	if (( level >= 1 ) and ( level <= 8 )) then
		retText = string.rep("!",level) .. " "
	end
	return retText
end

--	wiki markup to denote colour used for warning text
modWPCommon.Wiki.ColourWarn	= "red"

--	wiki markup to denote colour used for warning text
modWPCommon.Wiki.ColourCaution	= "color=#ff7f00"

--	wiki markup to denote end of wiki colour markup
modWPCommon.Wiki.ColourEnd		= "%%"

--	text used to have wiki page display a separation line
modWPCommon.Wiki.LineSep = string.rep("-",4)

--	text used to have wiki page insert end of line linebreak
modWPCommon.Wiki.LineBreakEnd = string.rep("\\",2)

--	text used to have wiki page force line break and clear float settings
modWPCommon.Wiki.ForceBreak = "[[<<]]"

--	text to separate label from data
modWPCommon.Wiki.Separator = ": "

--	text used for List Item on wiki pages
modWPCommon.Wiki.LI = "* "

--	text used for wiki hyperlink
modWPCommon.Wiki.HLink = "#"

-- paired elements
----------------------------------------
--	text used to start display of small text
modWPCommon.Wiki.TextSmallStart = "[-"
--	text used to end display of small text
modWPCommon.Wiki.TextSmallEnd = "-]"
--	text used to start display of large text
modWPCommon.Wiki.TextLargeStart = "[+"
--	text used to end display of large text
modWPCommon.Wiki.TextLargeEnd = "+]"
--	surround text to display emphasized
modWPCommon.Wiki.TextEmph = string.rep("\'",3)
--	surround text to display bolded and emphasized
modWPCommon.Wiki.TextBoldEmph = string.rep("\'",5)
--	text used to start display of comment
modWPCommon.Wiki.CommentStart = "(:comment "
--	text used to end display of comment
modWPCommon.Wiki.CommentEnd = " :)"

--	FRAMES
----------------------------------------
--	text to start wiki right frame
function modWPCommon.Wiki.FrameStartRight( extrastyle )
	if ( extrastyle == nil ) then
		extrastyle = ""
	else
		extrastyle = " " .. extrastyle
	end
	return ">>rframe" .. extrastyle .. "<<"
end

--	text to start wiki left frame
function modWPCommon.Wiki.FrameStartLeft( extrastyle )
	if ( extrastyle == nil ) then
		extrastyle = ""
	else
		extrastyle = " " .. extrastyle
	end
	return ">>lframe" .. extrastyle .. "<<"
end

--	wiki text to terminate frame
modWPCommon.Wiki.FrameEnd = ">><<"

-- TABLES ( NOTE: using structured table )
----------------------------------------
--	text to start a structured table
function modWPCommon.Wiki.TableStart( extrastyle )
	if ( extrastyle == nil ) then
		extrastyle = ""
	else
		extrastyle = " " .. extrastyle
	end
	return "(:table" .. extrastyle .. ":)"
end
--	terminate structured table
modWPCommon.Wiki.TableEnd = "(:tableend:)"

--	enter new structured table row
function modWPCommon.Wiki.TableRowStart( extrastyle )
	if ( extrastyle == nil ) then
		extrastyle = ""
	else
		extrastyle = " " .. extrastyle
	end
	return "(:cellnr" .. extrastyle .. ":)"
end

--	enter data continuing on current structured table row
function modWPCommon.Wiki.TableRowAppend( extrastyle )
	if ( extrastyle == nil ) then
		extrastyle = ""
	else
		extrastyle = " " .. extrastyle
	end
	return "(:cell" .. extrastyle .. ":)"
end

--	text placeholder to separate adjacent, inline tables
modWPCommon.Wiki.TableSeparator = "%lfloat%&nbsp;%%"

--	text entry formating
----------------------------------------
--	wiki formating of url data ( incl - anchors )
--	[in]	urltext	destination url as string
--	[in]	displaytext	text to display
--	[ret]	arguments as wiki formatted url string
function modWPCommon.Wiki.LinkText( urltext, displaytext )
	local rettext = ""
	local isLink = ((displaytext ~= nil) and ( type(displaytext) == 'string' ) and ( displaytext:len() > 0 ))
	if ( isLink ) then
		-- hyperlike
		rettext = "[[" .. urltext .. "|" .. displaytext	.. "]]"
	else
		-- anchor
		rettext = "[[" .. urltext .. "]]"
	end
	return rettext
end

--	produce a string of supplied data embedded in wiki formating
--	[in]	displaytext	text to be embedded
--	[in]	marker	displaytext is to embedded with this marker
--	[ret]	string in wiki format
function modWPCommon.Wiki.TextEmbed( displaytext, marker )
	local retText = displaytext
	if (( marker == nil ) or ( type(marker) ~= 'string' ) or ( marker:len() <= 0 )) then
		return retText
	end
	if ( marker == "emphasis" ) then
		retText = modWPCommon.Wiki.TextEmph .. displaytext .. modWPCommon.Wiki.TextEmph
	elseif ( marker == "boldemphasis" ) then
		retText = modWPCommon.Wiki.TextBoldEmph .. displaytext .. modWPCommon.Wiki.TextBoldEmph
	elseif ( marker == "textsmall" ) then
		retText = modWPCommon.Wiki.TextSmallStart .. displaytext .. modWPCommon.Wiki.TextSmallEnd
	elseif ( marker == "textlarge" ) then
		retText = modWPCommon.Wiki.TextLargeStart .. displaytext .. modWPCommon.Wiki.TextLargeEnd
	end
	return retText
end

--	produce a string of supplied data embedded in wiki colour formating
--	NOTE: colour data should not contain wiki colour markup.
--	== Pass colour data only to this function.
--	[in]	displaytext	text to be embedded
--	[in]	colourdata	displaytext is to be displyed with this colour
--	[ret]	string in wiki format
function modWPCommon.Wiki.TextColour( displaytext, colourdata )
	local retText = displaytext
	if (( colourdata == nil ) or ( type(colourdata) ~= 'string' ) or ( colourdata:len() <= 0 )) then
		return retText
	end
	retText = "%" .. colourdata .. "%" .. displaytext .. modWPCommon.Wiki.ColourEnd
	return retText
end

--	bulk appending of text lines (in table) to wiki page data
--	[in]	pageText	current wiki page text
--	[in]	data	table data of new wiki text
--	[ret]	processed wiki page text
function modWPCommon.Wiki.TableGen( tblstyle, tblheader, labelTable, dataTable, labelstyle, datastyle )
	local text = {}
	text[#text + 1] = modWPCommon.Wiki.TableStart(tblstyle)
	if (( tblheader ~= nil ) and ( #tblheader > 0 )) then
		local headtext = modWPCommon.Wiki.TextEmbed(tblheader,"emphasis")
		headtext = modWPCommon.Wiki.TextEmbed(headtext,"textsmall")
		text[#text + 1] = modWPCommon.Wiki.TableRowStart("align=center colspan=2") .. headtext
	end
	for key, label in pairs(labelTable) do
		local datatext = modWPCommon.Wiki.TextEmbed(dataTable[key],"textsmall")
		local labeltext = modWPCommon.Wiki.TextEmbed(label,"emphasis")
		labeltext = modWPCommon.Wiki.TextEmbed(labeltext,"textsmall")
		text[#text + 1] = modWPCommon.Wiki.TableRowStart(labelstyle) .. labeltext
		text[#text + 1] = modWPCommon.Wiki.TableRowAppend(datastyle) .. datatext
	end
	text[#text + 1] = modWPCommon.Wiki.TableEnd
	return text
end

--	replace characters in a line of text destined for use in a pmwiki wiki page
--	[in]	textstring	string to process
--	[ret]	processed text
--	==	'\%' becomes '\%25' (NOTE: MUST DO FIRST - so as not to affect other char replacments)
--	==	'<' becomes '\%3c'\n
--	==	'\\n' becomes '\%0a'
function modWPCommon.Wiki.WikifyText( textstring )
	local temptext = textstring
	temptext = temptext:gsub("%%","%%25")
	temptext = temptext:gsub("<","%%3c")
	temptext = temptext:gsub("\n","%%0a")
	return temptext
end

-- remove spaces and punction from link text
-- also change link text that starts with a number
function modWPCommon.Wiki.WikifyLink( text )
	if ( text == nil ) then
		return ""
	end
	local linktext = text:gsub("( )","")
	linktext = linktext:gsub("(%p)","")
	linktext = linktext:gsub("^([0])(.*)","zero%2")
	linktext = linktext:gsub("^([1])(.*)","one%2")
	linktext = linktext:gsub("^([2])(.*)","two%2")
	linktext = linktext:gsub("^([3])(.*)","three%2")
	linktext = linktext:gsub("^([4])(.*)","four%2")
	linktext = linktext:gsub("^([5])(.*)","five%2")
	linktext = linktext:gsub("^([6])(.*)","six%2")
	linktext = linktext:gsub("^([7])(.*)","seven%2")
	linktext = linktext:gsub("^([8])(.*)","eight%2")
	linktext = linktext:gsub("^([9])(.*)","nine%2")
	return linktext
end

--	bulk appending of text lines (in table) to wiki page data
--	[in]	pageText	current wiki page text
--	[in]	data	table data of new wiki text
--	[ret]	processed wiki page text
function modWPCommon.Wiki.PageAppend( pageText, data )
	if (( data == nil) or ( type(data) ~= 'table' )) then
		return pageText
	else
		for key, text in pairs(data) do
			if ( #text > 1 ) then
				pageText[#pageText + 1] = text
			end	-- line of text has size
		end	-- for each 'new' text line...
	end	-- have data
	return pageText
end

--	manage portrait to use for wiki entry
--	== function ensures path name used is not a duplicate
--	[in]	portraitpathname	path and file name to FDRPG graphics file to be used as a portrait.
--	[in]	exportfolder	path to file to use as wiki resources
--	[in]	exportname	file to use as wiki resources
--	[in]	exportext	extension to use for wiki resources
--	[ret]	FilesToLink index value were export exportpathname was stored
function modWPCommon.Wiki.ManagePortrait( portraitpathname, exportfolder, exportname, exportext )
	local retValue = nil
	local exportfilepath = exportname .. exportext
	local filelink, fileindex = modWPCommon.Extract.GetTableItem( modWPCommon.FilesToLink,"srcpath", portraitpathname )
	if ( filelink == nil ) then
		if (modWPCommon.Test.FileExists(portraitpathname)) then
			local newpath = { srcpath = portraitpathname,
			                  destpath = exportfolder,
			                  destfile = exportfilepath
			                }
			modWPCommon.FilesToLink[#modWPCommon.FilesToLink + 1] = newpath
			retValue = modWPCommon.FilesToLink[#modWPCommon.FilesToLink]
		end
	else
		retValue = filelink
	end
	return retValue
end

--	make a string for image presentation
--	[in]	imageurl	url to image path/name
--	[in]	alttext	alternative text to display
--	[in]	extrastyle	extra styling markup to be used
--	[ret]	wiki text for image presentation
function modWPCommon.Wiki.ImageText( imageurl, alttext, extrastyle )
	return	"%lfloat " .. extrastyle .. "%" .. imageurl	.. "\""	.. alttext .. "\"%%"
end

--	produce wiki format string of information from a lua table
--	Format is currently pmwiki-specific
--	[in]	tablename	table to process
--	[in]	label	label to use for display
--	[in]	seperator	string to display between label and data
--	[in]	colour	colour to use to display text
function modWPCommon.Wiki.TableToWiki( tablename, label, seperator, colour )
	if ( tablename == nil ) or ( label == nil ) then
--		io.stderr:write("TableToWiki: tablename or label is nil\n")
		return nil
	end
	if ( type(tablename) ~= 'table' ) then
		return nil
	end
	if ( #tablename > 0 ) then
		local data = modWPCommon.Wiki.LineBreakEnd .. "\n"
		for key, tableitem in pairs(tablename) do
			data = data .. "&emsp;&emsp;" .. tableitem
			if (key < #tablename) then
				data = data .. modWPCommon.Wiki.LineBreakEnd .. "\n"
			else
				data = data .. "\n"
			end
		end
		data = modWPCommon.Wiki.TextEntry( label, data, seperator, colour, false )
		return data
	end
end

--	consistent formating of wiki text
--	[in]	label	label text to display - emphasised on wiki page
--	[in]	data	data associated with label - normal text
--	[in]	seperator	string to display between label and data
--	[in]	colour	colour to use for text display
--	[in]	addLineBreak	boolean - true - add WikiTextLineBreakEnd (default)
--	[ret]	string of text in wiki format
function modWPCommon.Wiki.TextEntry( label, data, seperator, colour, addLineBreak )
	local returntext = nil
	local sep = seperator or " "
	local useLBE = true
	if ( addLineBreak ~= nil) then
		useLBE = addLineBreak
	end
	if ( data == nil ) then
		-- no data - just a label
		returntext = modWPCommon.Wiki.TextColour( label, colour )
		returntext = modWPCommon.Wiki.TextEmbed( returntext, "emphasis" )
	else
		local labeltext = label .. sep
		labeltext = modWPCommon.Wiki.TextColour( labeltext, colour )
		labeltext = modWPCommon.Wiki.TextEmbed( labeltext, "emphasis" )
		local datatext = modWPCommon.Wiki.TextColour( data, colour )
		returntext = labeltext .. datatext
	end
	if ( useLBE ) then
		returntext = returntext	.. modWPCommon.Wiki.LineBreakEnd
	end
	return returntext
end

return modWPCommon
