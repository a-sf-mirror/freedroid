Freedroid Dialog Node Mapper	{#fddnm}
============================
\tableofcontents

With repeated editing or refactoring of FDRPG dialog files, circumstances arise where sections
of the dialog are no longer reachable during game play. Editing of dialog files to correct
misconnected or unconnected dialog code can be a difficult proposition.\n
\n
By parsing a dialog file and transforming the data into graphic output, dialog developers and
maintainers now have a tool to "visually" ensure circumstances above do not occur. Freedroid
RPG Dialog Node Mapper( FDDNM ) tool parses dialog files and can producing graphic output in
several formats, all to indicate inter-connectivity and viability of dialog file nodes.\n
\n
Freedroid Dialog Node Mapper is a standalone program that is **not** compiled along with Freedroid RPG.
Users/Developers must compile this tool seperately.\n
\n
**Note to Freedroid RPG developers/maintainers**\n
Changes to dialog structure or variables will require that source for this tool be reviewed and if necessary updated.\n


Requirements	{#requirements}
============

- compiler with C++ 11 capability (gcc >= 4.9.x series or clang >= 3.4.x series w/ c++ option enabled)
- boost C++ libraries (boost >= 1.5.5 - need filesystem, algorithms and program_options)
- graphviz version >= 2.30.1 ( at or after git commit dated 07Mar2013 id:a97e1306c43b2ead2f45c6bb93a9fc62f98fad1a)


Building	{#building}
========

For BSD and MacOS X users, you must use gmake to build the FDDNM tool.
Any reference below to **make** should be replaced with **gmake** command.
The **gmake** tool is available for your platform from ports|packages repository.
All other platforms can use **make**.\n
\n
Execute the sequence below to configure and build the FDDNM tool.\n

	cd [FDRPG root]/tools/fddnm
	./autogen_fddnm.sh
	./configure
	make

The tool is not meant to be installed on the users system, but remain resident in the FDRPG root directory.\n
\n
A gitignore file has been established that will exclude the tool, object files and any program output (dot, png, svg, et al)\n
from being accidentally included into the FDRPG git tree.\n


Command Line	{#cmdline}
============

After successful compile and link, the executable *fddnm* can be used to process FDRPG dialog files.\n
\n
Usage:


	fddnm	[-h|--help]

Show help message for fddnm\n
\n

	fddnm	[-q]  [-P | --verboseprint]
			[-D | --verbosedot] [-G | --group]
			[-H | --extrainfo] [-I | --individual]
			[[-s | --dirsearch=]arg] [[-o | --diroutput=]arg]
			[[-F | --fileprefix=]arg] [[-L | --layout=]arg]
			[[-T | --format=]arg] [[-d | --dialog=]arg]

\n
	[-q]\n
	Quiet - no output to stdout\n
\n
	[-P | \-\-verboseprint]\n
	Print parsed dialog data to text file. Text file placed in output directory.\n
	Note - output file will be named "[PREFIX][Dialog Name]_VPARSE.txt"\n
\n
	[-D | \-\-verbosedot]\n
	Print analyzed dialog data used to create graphic to text file. Text file placed in output directory.\n
	Note - output file will be named "[PREFIX][Dialog Name]_VDOT.txt"\n
\n
	[-G | --group]
	Grouping is used as a layout hint to graphviz.\n
	Enables nodes connected to "end_dialog" to be grouped closer to "end_dialog" node.\n
	(E.g. Dixon or Tania graphs produce better output with this setting enabled).\n
\n
	[-H | --extrainfo]
	enable HTML-like dot output - includeds presentation of extra node information.\n
	(See below for an example of the output produced.)\n
\n
	[-I | \-\-individual]\n
	Program is to parse and process all dialog files individually.\n
	Any references to \"include\" another dialog file will not be processed. (e.g. 614-cryo.lua)\n
\n
	[-s | \-\-dirsearch=] *arg* \n
	Directory *arg* will be searched for dialog files (DEFAULT: [FDRPG root]/dialogs)\n
\n
	[-o | \-\-diroutput=] *arg* \n
	Directory *arg* will be used to store program output (graphic or text) (DEFAULT: current directory)\n
\n
	[-F | \-\-fileprefix=] *arg* \n
	*arg* will be the validated text used as a prefix for output file names.\n
	Note - see the second entry "portable_name" in the [Boost Filesystem Portability Guide](http://www.boost.org/doc/libs/1_55_0/libs/filesystem/doc/portability_guide.htm#recommendations) for limitations\n
\n
	[-L | \-\-layout=] *arg* \n
	*arg* is the direction of graph layout. One of [ TB (DEFAULT) | LR | RL | BT ]\n
	Note: see [Graphviz rankdir attribute](http://www.graphviz.org/content/attrs#drankdir) for details\n
\n
	[-T | \-\-format=] *arg* \n
	*arg* is the graphic format of output. One of [ none | dot | jpg | png | svg (DEFAULT) ]\n
\n
	[ | -d | \-\-dialog=] *arg* \n
	*arg* is the file name of dialog to be parsed without its *lua* file extension.\n
	No supplied dialog names implies all dialogs in search directory are to be parsed.\n
	More than one dialog name can be used without the switch as the use of the switch is optional.
\n

Examples	{#examples}
========

Usage Examples:\n
(all examples assume current directory is [FDRPG root]/tools/fddnm)\n
\n
- Parse all found dialogs and produce output in the form of png graphic files\n


	./fddnm -Tpng

- Parse dialog file *Tania.lua* and convert to DOT Language file\n
Output file *Tania.dot* would be produced and stored in the output directory.\n


	./fddnm -Tdot Tania

- Convert Doc Moore dialog to svg using Left to Right layout.\n
Output to user's home directory and set prefix to "TESTING_".\n


	./fddnm -Tsvg -FTESTING_ -o/home/user -LLR DocMoore

- A special condition can exist were no graphic output is produced but
the text-only parse data can be made available for examination.\n
Command below would create text file dumps of all parsed/analyzed data
and store these files in the default output directory.\n

	./fddnm -Tnone -P -D

- Parse all dialog files and produce PNG images with "extra detail".\n
(See below for an example of output with this output enabled.)\n

	./fddnm -Tpng -H


\n
Ouput Interpretaton		{#output}
===================

Current implementation uses the following colour scheme:\n
![Example - Dialog Conversion Output (no detail)](@ref example.png)

	blue		parent node calls "show" child node
	green		parent node calls "next" on child node
	orange		parent node calls "show_if" child node
	red(dashed)	parent node calls "hide" child node
	purple		parent node is calling end_dialog

Dialog nodes that are apart of a dialog topic are grouped inside a black line box.\n
\n
Black lines are drawn from the dialog start to any "FirstTime" and "EveryTime" dialog nodes.\n
\n
During parsing printout to console or review of verbose output, the user may notice the following text:\n

	Tania	Detected 48 nodes	Parsed 49 nodes

This output indicates that the dialog contains within code a call to *end_dialog*. To allow lines (or edges)
to be drawn from the calling node, an "artificial" end node is appended to the dialog node data. As a result of
the addition of an end_dialog node, the Parsed node count will be set to (Detected Node Count + 1).\n
\n

![Example Dialog Conversion Output (with detail)](@ref example_extra.png)
Using the command line switch *[-H | --extrainfo]* will result in diagrams similar to above.
The only change is the inclusion of text ( *text="..."* for each node in the dialog file).
Currently, the line wrap value is set for 30 characters.

![Example Error Indication](@ref example_error.png)
Diagram above shows *node99* as having no *show*, *next* or *show_if* connections leading to this node.
The node is not a child of any other nodes. Although the node has a *hide* command (red arrow), this is
in fact a dangling node.\n
\n
There are two possible interpretations: a) FDDNM has a bug (not unlikely considering the tool is
parsing fluid lua language without interpretation) or b) the node in question is unreferenced in code.
Which situation is occurring can be verified by doing a search of the file similar to...

	grep -n "nodeXX" dialogfile.lua

or similar code tools. If after the search no references can be found to *show*, *next* or *show_if* of
"nodeXX" then the node is unreferenced (or not called) in the dialog. A developer or dialog writer should
view this kind of output as an error in the dialog file. This is a type of error FDDNM tool is trying to highlight.

Known Issues			{#known}
============

Current implementation has issues with some node parsing that involve programmatically
determining the node value to be used. The dialogs involved include Tamara, Engel (bot parts topic) and
Ewald (gamble topic). This problem will be corrected in a future release.\n
\n
There are known issues when running address sanitizer or valgrind against the fddnm sources.
These issues stem from code within graphviz and do not appear to be related to fddnm. This is
an ongoing issue that is being monitored.\n
\n


Troubleshoot Build 	{#troubleshoot}
==================

If you receive an error during config that the boost libraries could not be found, do the following:\n
- Verify boost is installed on your system by locating libboost_system.so and boost/system header.\n
- If not found, install boost to your system (check distribution) and re-attempt configure.\n
\n
- If boost is present on the system and error is persistent, manually add these paths to the configure command as follows:\n


	./configure CPPFLAGS="-I/folder/path_boost_folder" LDFLAGS="-L/folder/path_libboost_system"


If you receive an error during configure that the graphviz libraries could not be found, attempt the following:\n
- Verify graphviz is installed by typing 'dot -V' ( should report "dot - graphviz version 2.x.y")\n
If this step fails - install the graphviz library and retry configure\n
\n
- If graphviz and pkg-config are installed, use the following:\n
Locate libgvc.pc file (or similar pkg-config for graphviz - check distribution)\n
Use the pkg-config output to populate configure variables as follows: (note quotation/backtick marks below)\n


	./configure CPPFLAGS="`pkg-config libgvc --cflags`" LDFLAGS="`pkg-config libgvc --libs-only-L`"



- If graphviz is installed but pkg-config is not present on your platform, manually locate gvc.h and libgvc.so and use the following:\n


	./configure CPPFLAGS="-I/folder/path_gvc_h" LDFLAGS="-L/folder/path_gvc_so/"

\n
During compile you receive the error...

	fddnm.cpp: In member function ‘void fddnm::graphivOutput(const string&, const string&)’:
	fddnm.cpp:653:38: error: invalid conversion from ‘const char*’ to ‘char*’ [-fpermissive]
	   Agraph_t* G = agmemread(userDotData);
										  ^
	In file included from /usr/include/graphviz/types.h:717:0,
					 from /usr/include/graphviz/gvc.h:20,
					 from fddnm.cpp:39:
	/usr/include/graphviz/graph.h:165:22: note: initializing argument 1 of ‘Agraph_t* agmemread(char*)’
		 extern Agraph_t *agmemread(char *);
						  ^

This indicates that the graphviz library requirement has not been met. See requirements above and follow
the instructions for your distribution to update graphviz.\n
\n

[MacOS X] If during linking you receive the error:\n

	Undefined symbols for architecture x86_64:
	boost::...

there is a difference in compiler used to build boost library and FDDNM. ( http://stackoverflow.com/a/20015083 )\n
This can be verified by executing the following command:\n

	otool -L /opt/local/lib/libboost_system-mt.dylib

The command should report back libboost_system library linkage to either libc++ (clang) or libstdc++ (gcc).\n
The solution is to either use the same compiler for FDDNM or rebuild boost libraries with prefered compiler.\n
To build FDDNM with same compiler used to build boost library, use the following during configure:\n

	./configure ... CXX="[ g++ | clang++ ]"

\n
Should these steps not work for you, please contact freedroidRPG on IRC for help.\n
\n
