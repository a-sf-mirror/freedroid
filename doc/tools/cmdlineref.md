Freedroid Command Line Arguments	{#cmdlineref}
================================
\tableofcontents

Game Play and Informational Options	{#normalstart}
===================================
<em>see the function [ParseCommandLine](../init_8c.html) for details</em>\n
\n
Command line options and switches shown below will result in either\n
	1) Game information returned on the command line then exit, or\n
	2) Freedroid RPG to start-up with a defined state.\n
\n
Unless stated otherwise, game setting changes are saved to user preferences and are persistent between game restarts.\n
If switches are stated as being mutally exclusive, and more than one is used, only the last instanced is honoured at game start.\n
\n

	freedroidRPG    [-h | --help]
	                [-v | --version]
	                [-e | --editor]
	                [-s | --sound]        [-q | --nosound]
	                [-o | --open_gl]      [-n | --no_open_gl]
	                [-f | --fullscreen]   [-w | --window]
	                [-t isocode | --system_lang=isocode]
	                [-l character-name | --load=character-name]
	                [-r Y | --resolution=Y]
	                [-r HxV | --resolution=HxV]
	                [-d X | --debug=X]

\n
<strong>[-h | --help]</strong>\n
Show help message for freedroidRPG game and exit to command prompt.\n
\n
<strong>[-v | --version]</strong>\n
Print Freedroid RPG version, brief GNU GPL statement and exit to command prompt.\n
\n
<strong>[-e | --editor]</strong>\n
Starts Freedroid RPG in the level editor at the last selected level.\n
If level has not been set previously, default level selected will be the Cryo Stasis Lab (level 12).\n
\n
<strong>[-s | --sound]</strong>\n
<strong>[-q | --nosound]</strong>\n
Mutally exclusive switches to start Freedroid RPG with sound setting either enabled or disabled.\n
\n
<strong>[-o | --open_gl]</strong>\n
<strong>[-n | --no_open_gl]</strong>\n
Mutally exclusive option to start Freedroid RPG with Open GL graphic output setting either enabled or disabled.\n
\n
<strong>[-f | --fullscreen]</strong>\n
<strong>[-w | --window]</strong>\n
Mutally exclusive option to start Freedroid RPG either in a window or fullscreen display.\n
\n
<strong>[-t *isocode* | --system_lang=<em>isocode</em>]</strong>\n
Freedroid RPG will start and attempt to use the language identified by *isocode* during game play.\n
See __setlocale__ man page for details. The language specified by the user must be available on the system.\n
If Freedroid RPG cannot find the requested language, it will revert to the system default for game play.\n
\n
<strong>[-l *character-name* | --load=<em>character-name</em>]</strong>\n
Freedroid RPG game will start normally and attempt to load a previously saved game for *character-name*\n
If the *character-name* saved game cannot be loaded, Freedroid RPG will display a warning and revert to the main menu.\n
\n
<strong>[-r *Y* | --resolution=<em>Y</em>]</strong>\n
<strong>[-r *HxV* | --resolution=<em>HxV</em>]</strong>\n
Start Freedroid RPG using the selected video resolution.\n
*Y* is an integer value starting ranging from 0(indicating 640x480 display) to as many supported resolutions.
Using *99* as a value for *Y* will cause Freedroid RPG to print all available supported resolutions and exit to command line.
On first usage, the game will use default resolution 0.\n
\n
A specific resolution value can be used and must be of the form WxH (eg. 1400x900).
The choosen custom resolution value will not appear in the options menu resolution screen.\n
\n
Resolution value can be used for both full screen and windowed display.
Some graphics (e.g. main screen) may appear stretched due to images being developed based on an assumed screen dimensions.
If Freedroid RPG is started in windowed mode with a specific resolution, the window will be sized to the dimensions given.\n
\n
Resolution can be changed from the game options menu, but a restart is required for the settings to take effect.\n
 Display resolution is stored in the settings file and recalled on each game start.\n
\n
<strong>[-d *X* | --debug=<em>X</em>]</strong>\n
Option will set verbosity of debug ouput the game will produce in the console during execution.
*X* is an integer value ranging from 1(default value) to 5.
Debug verbosity is not persistent between restarts as it is not stored in game settings file.\n
\n

Benchmark and Self-Testing Options	{#testingstart}
==================================
<em>see the function [benchmark](../benchmark_8c.html) for details</em>\n
\n
The benchmark switch (-b) is utilized for game development and testing.\n
Each option for benchmark performs a testing function of game mechanics on the local system and returns a result to the console.\n
\n
For all tests listed below, Freedroid RPG will start, initialize, execute the designated test(s) and return to the command prompt.
Indication will be given of testing success/fail and elapsed testing time.\n
\n

	freedroidRPG    [-b | --benchmark {text | dialog | loadship | loadgame | savegame | dynarray | mapgen | leveltest} ]

\n
<strong>freedroidRPG -b text</strong>\n
Code will be executed to test whether a string can be rendered using a designated game font.\n
\n
<strong>freedroidRPG -b dialog</strong>\n
All lua-based dialog files are parsed and validated.\n
Files are each parsed for "node" values to ensure dialog continuity and no broken code is present.
During parsing of each dialog file, the name of the file as well as each node name found are printed to the console.
On completion of file parsing a success/fail status is returned. Parsing continues with the next dialog file.
See [Dialog Designer Manual](../manual/dialog.html) for details on dialog file structure.\n
\n
<strong>freedroidRPG -b loadship</strong>\n
Instructs game logic to load the "levels.dat" file. Benchmarking involves 10 file load cycles\n
Loading will execute all the logic needed to insure file is parsable and no errors in game logic occurred.\n
\n
<strong>freedroidRPG -b loadgame</strong>\n
Saved game loading performance test.\n
\n
<strong>freedroidRPG -b savegame</strong>\n
Save game writing performance test.\n
\n
<strong>freedroidRPG -b dynarray</strong>\n
Execute game code to allocate memory, create dynamic arrays, deallocate and release memory.\n
\n
<strong>freedroidRPG -b mapgen</strong>\n
Benchmark testing for creating a temporary *ship* with random levels.\n
\n
<strong>freedroidRPG -b leveltest</strong>\n
Instructs game logic to load the "levels.dat" file. Only one file load cycle is executed\n
Loading will execute all the logic needed to insure file is parsable and no errors in game logic occurred.\n
\n
\n
