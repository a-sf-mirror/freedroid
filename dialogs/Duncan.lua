---------------------------------------------------------------------
-- This file is part of Freedroid
--
-- Freedroid is free software; you can redistribute it and/or modify
-- it under the terms of the GNU General Public License as published by
-- the Free Software Foundation; either version 2 of the License, or
-- (at your option) any later version.
--
-- Freedroid is distributed in the hope that it will be useful,
-- but WITHOUT ANY WARRANTY; without even the implied warranty of
-- MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
-- GNU General Public License for more details.
--
-- You should have received a copy of the GNU General Public License
-- along with Freedroid; see the file COPYING. If not, write to the
-- Free Software Foundation, Inc., 59 Temple Place, Suite 330, Boston,
-- MA 02111-1307 USA
----------------------------------------------------------------------

return {
	EveryTime = function()
		if (has_met("Koan")) then
			hide("node56")
		end

		if (not has_met("Duncan")) then
			show("node0", "node90", "node99")
		elseif (not guard_follow_tux) and
		       (not Duncan_Koan_quest) then
			next("node50")
		end

		if (not Duncan_Koan_quest_really_done) then
			if (has_item_backpack("Pandora's Cube")) or
			   (npc_dead("Koan")) or
			   (Koan_spared_via_dialog) then
				--Koan died, and you brought the cube back or
				--Koan died, but you didn't bring the cube back or
				--Koan is alive!
				if (not Duncan_Koan_quest_done) then --if you just came back from Koan
					next("node60")
				elseif (not Duncan_not_given_cube) and
				       (npc_dead("Koan")) then
					--if, after talking to Duncan, Koan became dead
					next("node60")
				end
			end
		end

		if (tux_has_joined_guard) then
			hide("node3", "node4")
		end

		if (Duncen_node_62_hide) then
			hide("node62")
		end
	end,

	{
		id = "node0",
		text = _"Hi... Erm... Who are you?",
		code = function()
			npc_says(_"Duncan McNamara, The Red Guard's resident bomb maker, at your service.")
			set_bot_name(_"Duncan - Bombmaker")
			hide("node0", "node90") show("node1")
		end,
	},
	{
		id = "node1",
		text = _"I would like to buy a bomb.",
		code = function()
			npc_says_random(_"Yes. I am sure we can arrange something. Take a look.",
				_"Sure. These are my offers today.")
			trade_with("Duncan")
			if (not guard_follow_tux) then
				hide("node1") show("node2", "node7")
			end
		end,
	},
	{
		id = "node2",
		text = _"I see only grenades. Do you have something bigger?",
		code = function()
			npc_says(_"My deepest apologies, I do not.")
			npc_says(_"The Red Guard prohibits the sales of extremely destructive munitions.")
			npc_says(_"It is not yet the time for the fall of their rule, so I remain loyal.")
			hide("node2") show("node3", "node4", "node5", "node6")
		end,
	},
	{
		id = "node3",
		text = _"The fall of the Red Guard? I am going to report that you are inciting a revolution!",
		code = function()
			npc_says(_"My word against yours. If you wish to try it, please, be my guest.")
			npc_says(_"I am not inciting anything. I am just stating what I see as inevitable in the future.")
			npc_says(_"I have taken part in many conflicts and I have seen how governments collapse.")
			npc_says(_"As soon as the war is over, the Red Guard will disband.")
			npc_says(_"Of course, I am making the assumption of victory, which is highly unlikely with the current state of affairs.")
			hide("node3", "node4")
		end,
	},
	{
		id = "node4",
		text = _"The fall of the Red Guard? Tell me more about it.",
		code = function()
			npc_says(_"It is not the time to speak of such things yet. Come another day, and we shall talk about it in more detail.")
			npc_says(_"The Red Guard can have their great laser beams and plasma cannons... But nothing can save them.")
			hide("node3", "node4")
		end,
	},
	{
		id = "node5",
		text = _"I want to buy some grenades.",
		code = function()
			npc_says_random(_"Certainly.",
				_"Sure.",
				_"With pleasure.")
			trade_with("Duncan")
		end,
	},
	{
		id = "node6",
		text = _"What kind of grenade is the best?",
		code = function()
			npc_says(_"You get what you pay for.")
			hide("node6")
		end,
	},
	{
		id = "node7",
		text = _"How did you get into making explosives?",
		code = function()
			npc_says(_"Twenty years ago there was a conflict.")
			npc_says(_"A grenade I threw destroyed a being which killed many of... my people.")
			npc_says(_"Now I make more to smash the destroyers.")
			hide("node7")
		end,
	},
	{
		id = "node50",
		text = "BUG, REPORT ME! duncan node50 -- starting koan quest",
		code = function()
			npc_says(_"Ah. Welcome again Linarian.")
			npc_says(_"I have a little request for you.")
			npc_says(_"I know someone who needs to be taught a lesson.")
			npc_says(_"Interested?")
			Duncan_Koan_quest = true
			show("node51", "node59")
		end,
	},
	{
		id = "node51",
		text = _"Tell me more.",
		code = function()
			npc_says(_"Ah, excellent.")
			npc_says(_"A... friend of mine, Koan, stole something of great value to me.")
			npc_says(_"He is hiding somewhere in the desert, I don't know exactly where.")
			hide("node51") show("node52")
		end,
	},
	{
		id = "node52",
		text = _"I will get your property back.",
		code = function()
			npc_says(_"Excellent. I am pleased to hear this.")
			npc_says(_"There is something you must know.")
			npc_says(_"There are not too many bots in the desert, but they are invincible. It is best to avoid them.")
			npc_says(_"I will be waiting here for your return.")
			tux_says(_"That is all I need to know. I will find Koan.")
			npc_says(_"Only time will tell.")
			if (cmp_obstacle_state("DesertGate", "closed")) and
			   (not has_met("Tania")) then
				npc_says(_"Here are a couple of circuits to grease open the western gate, if you know what I mean.")
				add_gold(25)
			end
			npc_says(_"Good luck!")
			add_quest(_"Doing Duncan a favor", _"I have to find Koan in the desert west of town and get Duncan something very precious.")
			hide("node52", "node59") show("node55", "node56")
		end,
	},
	{
		id = "node55",
		text = _"What are those bots?",
		code = function()
			npc_says(_"We call them 'Harvesters'.")
			npc_says(_"They were designed to chop down trees in the mountains.")
			npc_says(_"Now they chop down people.")
			npc_says(_"Their only weak point is their security system. But no one has managed to stay alive long enough to hack them.")
			hide("node55")
		end,
	},
	{
		id = "node56",
		text = _"What is it that he took from you?",
		code = function()
			npc_says(_"A cube. Trust me, you can't miss it.")
			hide("node56")
		end,
	},
	{
		id = "node59",
		text = _"No, I don't find bloodshed a pleasurable activity.",
		code = function()
			npc_says(_"That is fine. I will find someone else.")
			end_dialog()
		end,
	},
	{
		id = "node60",
		text = "BUG, REPORT ME! duncan node60 -- MURDERING DONE OR SAVED HIM",
		code = function()
			if (has_item_backpack("Pandora's Cube")) then
				if (Duncan_Koan_quest_done) then
					if (npc_dead("Koan")) then -- we killed koan manually or using the dialog and got
						--the cube when we return first time to Duncan after seeing Koan
						tux_says(_"Hey, I think I finally found your cube.")
						npc_says(_"Oh, great!")
					else -- does this ever show up though?
						tux_says(_"Hey!")
						npc_says(_"Hmm...?")
						tux_says(_"I think I found Koan after all ... finally.")
						npc_says(_"Oh, nice!")
						npc_says(_"Did you also get the cube?")
						Duncan_Koan_quest_really_done = true
					end
				else -- koan is dead
					npc_says(_"So... Any news on the Koan matter?")
				end
				Duncan_Koan_quest_done = true
				show("node62", "node63")
				if (Duncen_node_62_hide) then
					hide("node62")
				end
				Duncen_node_62_hide = true
			elseif (npc_dead("Koan")) then -- we don't have the cube but Koan is dead
				if (Duncan_talked_Koan_dead) then -- let Duncan ask for the cube differently if
					-- tux already returned without the cube while Koan was dead
					npc_says(_"Did you finally find the cube?")
					tux_says(_"Uhmmm...")
					npc_says(_"You better go getting it!")
				else -- we killed koan somehow but don't have the cube
					npc_says(_"You don't look too well, what did happen?")
					tux_says(_"He ... he is dead...")
					npc_says(_"Koan?")
					if (Koan_spared_via_dialog) then -- we told Koan we won't kill him, and lied to Duncan
						--about not finding him. Afterwards we went back to Koan and killed him manually
						npc_says(_"I thought you hadn't found him?")
						tux_says(_"Yes, I didn't find him at first, now I did.")
					else -- we lost the cube and killed Koan via dialog
						tux_says(_"Yes.")
					end
					npc_says(_"And the cube that he was carrying? Where is it?")
					tux_says(_"I must have left it somewhere...")
					npc_says(_"Fetch it and bring it to me.")
					Duncan_talked_Koan_dead = true
					update_quest(_"Doing Duncan a favor", _"Unfortunately, I forgot to bring the cube. Duncan was not amused.")
				end
				end_dialog()
			elseif (Koan_spared_via_dialog) then -- we didn't kill via the dialog
				tux_says(_"I could not find him anywhere in the desert. I don't think he is there anymore.")
				npc_says(_"I see.")
				end_quest(_"Doing Duncan a favor", _"I lied to Duncan about not finding Koan.")
				Duncan_Koan_quest_done = true
				end_dialog()
			end
			show("node64")
		end,
	},
	{
		id = "node62",
		text = _"I think this is your cube.",
		code = function()
			npc_says(_"Yes. I appreciate your help.")
			hide("node62", "node63") next("node69")
		end,
	},
	{
		id = "node63",
		text = _"Now, what is this big cube that you had me carry all the way here?",
		code = function()
			npc_says(_"Just a memento from a friend.")
			npc_says(_"A little more than a portable end of the world, which I am looking forward to disassembling and learning its secrets.")
			npc_says(_"Nothing that important.")
			Duncen_node_62_hide = true
			hide("node62", "node63") show("node67", "node68")
		end,
	},
	{
		id = "node64",
		text = _"A 'few' bots? The place was crawling with Harvesters! I nearly got killed!",
		code = function()
			npc_says(_"There were only one hundred and twenty bots in the entire desert region. This hardly qualifies as many, considering the size of the area.")
			tux_says(_"What? You know exactly how many bots were in the desert? Without being there? I don't like this.")
			npc_says(_"I just know a lot of things. You do not need to worry about it.")
			tux_says(_"I don't like this at all. I'm getting out of here.")
			npc_says(_"As you wish.")
			end_dialog()
			hide("node64")
		end,
	},
	{
		id = "node67",
		text = _"Hey, that is really neat. Here is your cube. Enjoy disassembling it and have fun with it.",
		code = function()
			npc_says(_"I will.")
			hide("node67", "node68") next("node69")
		end,
	},
	{
		id = "node68",
		text = _"WHAT? No way I am giving you a doomsday device! Forget about it.",
		code = function()
			npc_says(_"I understand. So be it.")
			end_quest(_"Doing Duncan a favor", _"No way am I giving Duncan that cube thingie. Who knows what he would do with it.")
			Duncan_not_given_cube = true
			end_dialog()
			hide("node67", "node68")
		end,
	},
	{
		id = "node69",
		text = "BUG, REPORT ME! duncan node69",
		code = function()
			add_xp(3000)
			del_item_backpack("Pandora's Cube", 1)
			sell_item("Plasma Shockwave Emitter")
			end_quest(_"Doing Duncan a favor", _"I gave Duncan the cube thingie. It feels nice to help people.")
			Duncan_Koan_quest_really_done = true
			end_dialog()
		end,
	},
	{
		id = "node90",
		text = _"I feel you are... Different.",
		code = function()
			npc_says(_"What a great way to start a conversation.")
			npc_says(_"Yes, that is true. I am not who I seem to be.")
			npc_says(_"But then again, neither are you, so my thoughts are, we are even.")
			tux_says(_"What do you mean? I am %s and not anyone else.", get_player_name())
			npc_says(_"And I am Duncan McNamara, the maker of grenades and nothing else.")
			set_bot_name(_"Duncan - Bombmaker")
			npc_says(_"I believe the topic is thoroughly exhausted now.")
			hide("node0", "node1", "node2", "node90") show("node5", "node92")
		end,
	},
	{
		id = "node92",
		text = _"Tell me who you are. I need to know.",
		code = function()
			npc_says(_"Twenty years ago, people were kinder to each other.")
			npc_says(_"I suggest you be kind to me as well, and I will be kind to you.")
			npc_says(_"That way I get to keep my secret, and you get to keep yours.")
			hide("node92") show("node93")
		end,
	},
	{
		id = "node93",
		text = _"I don't have any secrets.",
		code = function()
			npc_says(_"And neither do I.")
			hide("node93")
		end,
	},
	{
		id = "node97",
		text = "BUG, REPORT ME! duncan node97",
		code = function()
			npc_says(_"I wish you cold winds.")
			tux_says(_"Huh? How do you know the Linarian farewell? No one around here knows it.")
			npc_says(_"I read many books on Linarians. That is all.")
			npc_says(_"Nothing more and nothing less.")
			show("node98") next("node98")
		end,
	},
	{
		id = "node98",
		text = _"I wish you cold winds.",
		code = function()
			npc_says(_"May the ice bring you wisdom.")
			end_dialog()
		end,
	},
	{
		id = "node99",
		text = _"See you later.",
		code = function()
			if (not Duncan_coldwinds) then
				Duncan_coldwinds = true
				next("node97")
			else
				npc_says(_"See you later.")
				end_dialog()
			end
		end,
	},
}
