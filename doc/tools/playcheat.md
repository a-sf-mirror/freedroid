Freedroid Play Testing and Cheats	{#playcheat}
=================================
\tableofcontents


Purpose	{#ptPurpose}
=======
In a large multi-level game, playing sequentially through the game itself is very time consuming and tedious.
Development and debugging practices would be degraded with the need to save every game state or playing through to test a minor change.
Freedroid RPG has several means to alter game and player states in order to improve testing/debugging.\n
\n
While described as "cheats", altering the game and player states is not meant to be used during normal game play.
Cheat options will only be effective when no other game dialogs or menus are on screen.
Character information dialogs (i.e. Spells, Character Data, Inventory) on screen does not prevent cheat code functions.\n
\n
Keys to press to active the desired cheat function are denoted between square braces (i.e. <em>[</em> and <em>]</em>).
Codes involving multiple keys require all keys to be pressed.
Sequence and timing of keypresses are not factors, but all keys must be pressed before releasing keys.
Some cheat codes call for a specific "handedness" of keypresses.
In these instances, <em>[LEFT xxxx]</em> and <em>[RIGHT xxxx]</em> keys are not equivalent.\n
\n
Cheat Menu Options {#ptMenuOptions}
======================
The main "cheat" menu is accessed by the keystrokes
<strong>[LEFT CTRL][LEFT ALT][LEFT SHIFT][C]</strong>.\n
The game will be paused and a screen showing current states will be presented.\n
Key presses to alter cheat options will be enabled.\n
\n
<strong>[f]</strong>\n
Give player xray vision (see through walls). (Default:OFF)\n
\n
<strong>[g]</strong>\n
"God Mode" - Player is invincible. (Default:OFF)\n
\n
<strong>[i]</strong>\n
Player is invisible (no-time-limit invisibility). (Default:OFF)\n
\n
<strong>[l]</strong>\n
<em>L</em>ist all NPC's on the current level.\n
\n
<strong>[L]</strong> <em>(note case - same as [SHIFT][l] )</em>\n
List all NPC's alive on the current level.\n
\n
<strong>[k]</strong>\n
List all NPC's killed on the current level.\n
\n
<strong>[d]</strong>\n
Destroy all NPC's (both droids and interactive characters) on the current level.\n
\n
<strong>[h]</strong>\n
Acquire one level of all spells available in game.\n
If spell already acquired, increment skill level of that spell by one.\n
\n
<strong>[c]</strong>\n
Player acquires 1 million circuits.\n
Line also displays current amount of circuits in possession.\n
\n
<strong>[n]</strong>\n
Alter game mechanics to enble hidden droids.\n
Default is to allow droid invisibility (No Hidden Droids: OFF).\n
\n
<strong>[r]</strong>\n
Unlimited running stamina.\n
Normal in-game effects of running for the player are ignored.\n
\n
<strong>[s]</strong>\n
Player is able to run twice as fast. (Default:OFF)\n
\n
<strong>[t]</strong>\n
The penultimate weapon! A Cheat Gun!
This two-handed weapon has excessive druability and does incredible damage with each shot.\n
Cheat Gun enables player to **blast** through any/all droid obstacles with little effort.
<em>(Okay...its fun, too. What's your point?)</em>\n
\n
<strong>[x]</strong>\n
Enable cheat keys (Default: ON).\n
Cheat keys setting will only take affect after the Cheat Menu has been first presented.\n
\n
<strong>[T]</strong> <em>(note case - same as [SHIFT][t] )</em>\n
Player acquires one training point.\n
Line also displays current number training points available to be distributed.\n
\n
<strong>[e]</strong>\n
Control the display of enemy state information (e.g. patrol, attack, etc). (Default: OFF)\n
\n
<strong>[q]</strong>\n
Exit cheat menu and resumes game.\n
\n
\n
In-Game Cheats	{#ptOtherOptions}
==============
Once enabled from the Cheat Menu, the key strokes shown below will be available.\n
\n
<strong>NOTE: </strong>Number keys (such as <em>[1]</em>, etc) is not the same as a Keypad number key.
If a keypad is not available on the users keyboard, these cheats cannot be activated.\n
\n
<strong>[Keypad 1]</strong>\n
Add 1000 experience points to player total.\n
\n
<strong>[Keypad 2]</strong>\n
Double player total experience points.\n
\n
<strong>[Keypad 7]</strong>\n
Increment player <em>Melee</em> skill (hand-to-hand fighting ability).\n
\n
<strong>[Keypad 4]</strong>\n
Decrement player <em>Melee</em> skill.\n
\n
<strong>[Keypad 8]</strong>\n
Increment player <em>Range</em> skill (shooting ability).\n
\n
<strong>[Keypad 5]</strong>\n
Decrement player <em>Range</em> skill.\n
\n
<strong>[Keypad 9]</strong>\n
Increment player <em>Programming</em> skill.\n
\n
<strong>[Keypad 6]</strong>\n
Decrement player <em>Programming</em> skill.\n
\n
<strong>[RIGHT CTRL][r]</strong>\n
Increment player <em>Repair</em> spell ability level.\n
\n
<strong>[RIGHT CTRL][RIGHT SHIFT][r]</strong>\n
Decrement player <em>Repair</em> spell ability level.\n
\n
<strong>[LEFT CTRL][r]</strong>\n
Drop a random item near player's position.\n
\n
<strong>[LEFT CTRL][LEFT SHIFT][r]</strong>\n
Drop a random magical item near player's position.\n
\n
<strong>[LEFT CTRL][LEFT ALT][LEFT SHIFT][r]</strong>\n
Respawn all NPC's (including killed droids) on current level.\n
\n
<strong>[LEFT CTRL][LEFT ALT][LEFT SHIFT][g]</strong>\n
Reload and redisplay current graphics.\n
\n
\n
Takeover Win	{#ptTakeover}
============
When hacking a bot, a forced win can be enacted during the takeover game.
The player must still go through the process of selecting colour (or side).\n
\n
Once the game has started, entering the key combination
<strong>[LEFT CTRL][LEFT ALT][w]</strong>
will cause the takeover game to exit with the player as victor.
The bot attempting to be hacked will now be under control of the player.
Although the player has forced a win, the player statistics reflect a forced win as a normal win.\n
\n
\n
In-Game Level Editing	{#ptLvlEdit}
=====================
There are numerous use cases where there is a need to alter the items in the game "ship" or levels.
The most obvious is those times where testing requires the player character be located on another level.
Rather than manually moving the character, the character can be repositioned using the Level Editor.
This method also allows developers to move the character to the debug screens, which are not available during game play.\n
\n
Level Editor can be accessed in-game, when no other menus or dialogs are active, by using the key combination
<strong>[LEFT CTRL][LEFT ALT][e]</strong>.\n
\n
Usage of the Level Editor is described in other Freedroid RPG support documents.\n
\n
