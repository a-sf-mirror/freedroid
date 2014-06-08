Dialog Wiki Information Header {#wikiheader}
============================================

This chapter describes an addition to the dialog files to embed character informtion into a dialog file.
This process was originally developed to aid in parsing of source code to populating a wiki page. However, the
wiki header details can also be used to amplify game character details and aid future development.
\n
\n
Located in the dialog file after the License statement and before the dialog Lua code,
a dialog wiki header is in essence Lua code surrounded by special Lua block comment markers.
It is itended to be parseable by wiki parsing code, but not to be processed by the game engine.
\n
\n
The form of a wiki header is as follows:

~~~~~~~~~~~{.lua}
--[[WIKI
PERSONALITY = { "text","text" },
MARKERS = { NPCID1 = "Kevin", QUESTID1 = "Deliverance", NPCID2 = "Tybalt", ITEMID1 = "Mug" },
PURPOSE = "$$NAME$$ lorem ipsum dolor sit amet, consectetur adipisicing elit",
BACKSTORY = "$$NAME$$ $$NPCID1$$ lorem ipsum dolor $$ITEMID1$$",
RELATIONSHIP = {
	{
		actor = "$$NPCID1$$",
		text = "$$NAME$$\'s lorem ipsum $$NPCID1$$ dolor sit"
	},
	{ actor = "$$NPCID2$$", text = "$$NAME$$ lorem ipsum dolor $$NPCID2$$ sit amet" }
}
WIKI]]--
~~~~~~~~~~~
\n
\n
Common Information {#wiki_common}
=================================

____

 - Any quotation marks in text data must be escaped (`\'` or `\"`).
 - Header information is with respect to the subject of the dialog file.
 - `$$NAME$$` is an alias to the dialog file character.
 - content of wiki header __must be__ valid Lua code or wiki parsing will fail.
\n
\n
Wiki Block {#wiki_block}
========================

____

~~~~~~~~~~~{.lua}
--[[WIKI
WIKI]]--
~~~~~~~~~~~
Lua block comments are used to surround the wiki header data so that the FDRPG game engine will not process this code.
\n
\n
`WIKI` is used to indicate where the wiki header begins and ends, distinguishing this section from other Lua block comments
that may be present. All Lines between the markers, exlcusive of the markers proper, are extracted line-by-line for processing.
After the text is extracted, it is placed into a Lua table and loaded into memory. Once in memory, the data in the wiki header
is processed, along with other dialog information, contributing to the NPC wiki page content.
\n
\n
__NOTE:__ wiki block markers must be on their own line with no other text.
\n
\n
Personality {#wiki_personality}
===============================

____

~~~~~~~~~~~{.lua}
PERSONALITY = { "text","text" },
~~~~~~~~~~~
PERSONALITY variable is a table of short, terse and descriptive statements that detail the character traits of the dialog file
subject. These statements serve to enhance an understanding of how the character might responsed to situations in the game.
\n
\n
It is suggested to organize these statements from most to least dominant trait. Statements consisting of sentences or references
to other game elements is discouraged as these can be best detailed in other header elements.
\n
\n
__NOTE:__ Keep It Short!!!
\n
\n
Markers {#wiki_markers}
=======================

____

~~~~~~~~~~~{.lua}
MARKERS = { NPCID1 = "Kevin", QUESTID1 = "Deliverance", NPCID2 = "Tybalt", ITEMID1 = "Mug" },
~~~~~~~~~~~
Markers are _key = value_ pairs that serve to identify game elements in the wiki header. Markers apply to all other wiki header entries
shown below.
\n
\n
Marker - Key {#wiki_marker_key}
-------------------------------

_key_ is used to identify what game element _value_ represents. The text of _key_ will take the form `[element]ID[0-9]` where:
	- `[element]` can be one of:
		+ _ITEM_ \- an in game item.
		+ _NPC_ \- another named character in the game.
		+ _QUEST_ \- a quest that Tux can undertake.
		+ _DROID_ \- a model of droid.

	- `[0-9]` is a numerical identifier to distinguish this marker from others of the same type.

All occurrences of the _key_ text in other header elements are surrounded by double dollar signs - `$$key$$`. When enountered
in wiki parsing, `$$key$$` will be replaced with what value represents (see below). This is intended to simplify future updates
of the header information as only one item needs to be updated in a dialog file. However, this does not guarantee accuracy with
plurals \(\[text\]s\), possessives \(\[text\]'s\) or pronouns ( he | she | it | they).
\n
\n
Marker - Value {#wiki_marker_value}
-----------------------------------

_value_ is a unique identifier of the game element involved. This text will be used by wiki parsing to look up actual presentation text
or links to element's information on other wiki pages. All instances of _key_ will be replaced with an assoicated URL link or anchor
to what value identifies.
\n
\n
If the value text appears in angle brackets ( `<text>` ) on the wiki page it indicates that the identifier _value_ represents could not be
located in the source. Likely causes are that _value_ is mispelled or the identifier has been removed from source.
\n
\n
Purpose {#wiki_purpose}
=======================

_____

~~~~~~~~~~~{.lua}
PURPOSE = "$$NAME$$ lorem ipsum dolor sit amet, consectetur adipisicing elit",
~~~~~~~~~~~
PURPOSE is to contain text that describes why or how this character fits into the overall game mechanics but not the storyline.
\n
\n
Backstory {#wiki_backstory}
===========================

_____

~~~~~~~~~~~{.lua}
BACKSTORY = "$$NAME$$ $$NPCID1$$ lorem ipsum dolor $$ITEMID1$$",
~~~~~~~~~~~
BACKSTORY is narrative that documents the fictional aspects of the character in the game's storyline. Some verbosity can be used.
This text is intended to be fictional in nature and not be related the purpose of the character to the game.
\n
\n
Relationship {#wiki_relationship}
=================================

____

~~~~~~~~~~~{.lua}
RELATIONSHIP = {
		{
			actor = "$$NPCID1$$",
			text = "$$NAME$$ lorem ipsum $$NPCID1$$ dolor sit"
		},
		{ actor = "$$NPCID2$$", text = "$$NAME$$ lorem ipsum dolor $$NPCID2$$ sit amet" }
	},
~~~~~~~~~~~
RELATIONSHIP is a Lua table containing one or more items that each detail the fictionalized dynamics that exist between the character
and other game characters.
\n
\n
On the final wiki page, _actor_ will become a link to another character's wiki page entry. _text_ will describe the relationship between
subject of the dialog and _actor_.
\n
\n
_actor_ should not be interpreted literally, but rather similar in nature to a UML actor.
\n
____
