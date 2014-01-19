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

local Tux = FDrpg.get_tux()

return {
	FirstTime = function()
		show("node0")
	end,

	EveryTime = function()
		branch_to_pendragon = false
		show("node90")
		if (Koan_murdered) then
			npc_says(_"You just killed him!")
			npc_says(_"How could you?")
			npc_says(_"You murderer!")
			npc_faction("crazy", _"Tania - Avenging Koan")
			end_dialog()
		end

		if (Tux:has_quest("Tania's Escape")) then
			hide("node1", "node2", "node3", "node4", "node5", "node6", "node7", "node10", "node11", "node12", "node13", "node14", "node17", "node18", "node19", "node90")
			if (not Tania_follow_tux) then
				npc_says_random(_"I long to see the surface.",
								_"You are the most interesting hallucination yet.")
				show("node90")
				if (not SACD_gunsoff) then --Guns are still on
					hide("node27")
					show("node28")
				elseif (not Tania_guns_off) then --Guns are off, but you haven't told Tania yet
					hide("node28")
					show("node27")
				else --Tania can escape, but you haven't decided to let her follow you yet
					hide("node27", "node28")
					show("node40", "node41")
				end
			elseif (not Tania_surface) then --Tania is following you, but still Underground
				npc_says(_"I can't wait to leave this place.")
				show("node91")
			elseif (not Tania_stopped_by_Pendragon) then --in the Western Desert
				if (not Tania_discussed_surface) then
					Tania_discussed_surface = true
					npc_says(_"It is so very bright and hot out here.") --thirsty dialog
					show("node45", "node46", "node47")
				elseif (npc_damage_amount() > 10) then --skip to injury dialog
					next("node55")
				else
					npc_says_random(_"Are we there yet?",
									_"How much longer?")
				end
				show("node92")
			elseif (not Spencer_Tania_sent_to_DocMoore) and
			       (not Tania_set_free) then --at the Town Entrance, waiting for Spencer's OK
				hide("node45", "node46", "node47", "node49", "node50", "node51", "node52", "node53", "node56", "node57", "node58", "node59", "node92")
				if (Tania_just_stopped_by_Pendragon) then
					Tania_just_stopped_by_Pendragon = false
					Tux:update_quest(_"Tania's Escape", _"Pendragon just stopped Tania and I at the town gate. Apparently he won't let her in, unless Spencer gives the go-ahead.")
					npc_says(_"It is OK. I'll wait here at the gate.")
					end_dialog()
				end
				show("node93")
			elseif (not Tux:done_quest("Tania's Escape")) then --Town Entrance then (if Doc is alive) send her DocMoore's office, else set free (send to Bar)
				npc_says(_"What is the news?")
				Tux:says(_"I talked to Spencer, and he said you were welcome to enter the town.")
				npc_says(_"That is great news!")
				if (Spencer_Tania_sent_to_DocMoore) then
					Tux:says(_"He said you must first get checked out by Doc Moore though.")
					set_bot_destination("DocPatient-Enter")
				else --DocMoore is Dead!
					Tux:says(_"He said you are free to go where you like.")
					set_bot_destination("BarPatron-Enter")
					Tania_at_Ewalds_Bar = true
				end
				Tux:end_quest(_"Tania's Escape", _"I successfully brought Tania safely to the town. I hope she likes it here.")
				if (difficulty("hard")) and
				   (not Tania_mapper_given == true) then
					npc_says("I'm so glad that I am finally here, take this.")
					Tux:add_item("Source Book of Network Mapper")
					Tania_mapper_given = true
				end
				branch_to_pendragon = true
			else --"Tania's Escape" was a success!
				if (not Tania_DocMoore_cleared) and
				   (not Tania_set_free) then --send to Bar
					Tania_DocMoore_cleared = true
					heal_npc()
					npc_says(_"I have good news: the doctor says I'm healthy!")
					npc_says(_"Where should I go?")
					if (Tux:done_quest("Anything but the army snacks, please!")) then
						show("node70")
					else
						show("node71")
					end
				end
				show("node94")
			end
		end

		if (Tania_heal_node8) then
			show("node8")
		end

		if (branch_to_pendragon) then
			start_chat("Pendragon")
		end
	end,

	{
		id = "node0",
		text = _"Um, hi.",
		code = function()
			npc_says(_"Oh! It happened! I have been waiting for you for 3226 hours!")
			hide("node0") show("node1", "node3", "node14", "node19")
		end,
	},
	{
		id = "node1",
		text = _"You knew I would come?",
		code = function()
			npc_says(_"Of course, my little imaginary friend!")
			hide("node1") show("node2", "node5")
		end,
	},
	{
		id = "node2",
		text = _"Imaginary?! I am as real as you are.",
		code = function()
			npc_says(_"This place is locked. Hermetically sealed and guarded by bots. So you can't possibly be anything more than the product of my imagination.")
			Tux:says(_"But who are you?")
			npc_says(_"A figment of my imagination should know my name: Tania.")
			set_bot_name(_"Tania - lonely scientist")
			hide("node2") show("node12")
		end,
	},
	{
		id = "node3",
		text = _"Where am I?",
		code = function()
			npc_says(_"In a prison, a luxurious prison. I am feeling quite lonely, as you can see. Big room, plenty of food, a waterfall behind the window...")
			hide("node3") show("node4", "node6")
		end,
	},
	{
		id = "node4",
		text = _"All I see through the window are ray emitters. And we are below ground level..",
		code = function()
			npc_says(_"Yes, I know. But I so badly want to see sun, rivers, trees... there definitely is waterfall behind them!")
			hide("node4")
		end,
	},
	{
		id = "node5",
		text = _"It is dark here.",
		code = function()
			npc_says(_"The main power supply is down, so only the emergency lights work. Peter tried to switch it back on, but he didn't succeed.")
			hide("node5")
		end,
	},
	{
		id = "node6",
		text = _"This place doesn't look like a prison.",
		code = function()
			npc_says(_"In the past it was a secret lab. You heard about those strength pills? They were invented here. Other pills too, but I was working on the strength ones.")
			hide("node6") show("node7", "node8")
		end,
	},
	{
		id = "node7",
		text = _"Can you give me some pills then?",
		code = function()
			npc_says(_"No, Peter ate them all. That killed him.")
			npc_says(_"*cries*")
			Tux:says(_"They say that strength pills are absolutely safe!")
			npc_says(_"They are like money. Having money is good, but if you have too much, you can decide that you can do anything.")
			npc_says(_"Peter became not very strong, but very-very-very strong. He thought he could make an exit with his bare hands.")
			npc_says(_"*cries*")
			npc_says(_"If you go further down the corridor, you will find debris and a big stone. That is his grave.")
			hide("node7") show("node10", "node11")
		end,
	},
	{
		id = "node8",
		text = _"So you are a biologist? Could you heal me?",
		code = function()
			npc_says(_"That would be something different from what I've done in the last months at least. Some entertainment.")
			npc_says_random(_"Let me take a look at that... it's nothing some nanobots couldn't take care of.... You will be all fixed up in a minute.",
				_"You are now completely healed. You should take better care of yourself.")
			Tux:heal()
			Tania_heal_node8 = true
			hide("node8")
		end,
	},
	{
		id = "node10",
		text = _"You loved him?",
		code = function()
			npc_says(_"I still do.")
			hide("node10")
		end,
	},
	{
		id = "node11",
		text = _"But why not go out through the door?",
		code = function()
			npc_says(_"It is locked and very reliable. Even a tank would not be able to smash it! And if a door was broken, SADDs would attack, that's their program.")
			hide("node11") show("node18")
		end,
	},
	{
		id = "node12",
		text = _"Autoguns have made a hole in the wall. I came through this hole.",
		code = function()
			npc_says(_"Do you mean I can go out to the surface?!")
			npc_says(_"Oh god! I - I told Peter that he chose the wrong place, but he was so stubborn!")
			hide("node12") show("node13")
		end,
	},
	{
		id = "node13",
		text = _"Yes, you can get to the surface. But the guns are still on.",
		code = function()
			npc_says(_"All hope is lost! I'm a scientist, not a warrior.")
			hide("node13") show("node15")
		end,
	},
	{
		id = "node14",
		text = _"It is strange to see a girl in such a beautiful dress here.",
		code = function()
			npc_says(_"I'm going to spend all my life here! So forgive me some little indulgences.")
			hide("node14")
		end,
	},
	{
		id = "node15",
		text = _"Maybe is it possible to disable the guns?",
		code = function()
			npc_says(_"Theoretically yes, but in practice I'm not sure you would be able to do that.")
			hide("node15") show("node16")
		end,
	},
	{
		id = "node16",
		text = _"How can I disable the guns?",
		code = function()
			npc_says(_"Somewhere in a distant part of the lab should be an SACD - Secret Area Control Datacenter. It controls all the defense systems in the base. If you manage to get to it, you would be able to control the base. However, it's very hard to find and get to the SACD.")
			hide("node16") show("node17")
		end,
	},
	{
		id = "node17",
		text = _"I will disable the guns for you.",
		code = function()
			npc_says(_"Thanks... please be careful. You will not be able to access the control center directly, it is behind a triple hermetic door. Try using the service tunnels.")
			Tux:add_quest(_"Tania's Escape", _"I have met a girl locked in a secret area. If I manage to disable the autoguns, she will be able to go to the surface and look at the sun again.")
			npc_says(_"It's dangerous to go alone! Take this!")
			if (difficulty("easy")) and
			   (not Tania_mapper_given == true) then
				Tux:add_item("Source Book of Network Mapper")
				Tania_mapper_given = true
			end
			Tux:add_item("EMP Shockwave Generator", 5)
			hide("node17")
		end,
	},
	{
		id = "node18",
		text = _"Why not just open them? There should be a way of unlocking the base.",
		code = function()
			npc_says(_"To open the door you need to know the password. Only the commander of the area and his deputy knew it. The commander was killed by bots as soon as the assault started, and the deputy was in town, I don't know what happened to him.")
			hide("node18")
		end,
	},
	{
		id = "node19",
		text = _"Carpets, sofa, bookshelves. Where did you get all this?",
		code = function()
			npc_says(_"Peter did all he could to make this room comfortable. The sofa and armchair are from the commanders cabinet, the books are from the lounge...")
			hide("node19")
		end,
	},
	{
		id = "node27",
		text = _"The sentry guns are off. You can go now!",
		code = function()
			npc_says(_"Thanks, thanks a lot! Now I will be able to see the sun, trees, rivers... I missed those so much!")
			npc_says(_"I hope these books will help you.")
			display_big_message(_"Tania is now free!")
			Tux:add_xp(1500)
			if (difficulty("normal")) and
			   (not Tania_mapper_given == true) then
				Tux:add_item("Source Book of Network Mapper")
				Tania_mapper_given = true
			end
			Tux:add_item("Source Book of Check system integrity",1)
			Tux:add_item("Source Book of Sanctuary",1)
			Tux:update_quest(_"Tania's Escape", _"Tania is free now, I got some books as reward.")
			Tux:says(_"You could always come back with me to the town. There are people there.")
			npc_says(_"I'd love to, but you'll have to escort me.")
			Tania_guns_off = true
			hide("node27") show("node40", "node41")
		end,
	},
	{
		id = "node28",
		text = _"Where can I find that SACD?",
		code = function()
			npc_says(_"To the right from the entrance of the area, there is a hall. In the south part of this hall you will find a triple hermetic door. The control center is behind that door. But it is locked, so you will have to find another way there. The cable collectors may help you.")
			hide("node28")
		end,
	},
	{
		id = "node40",
		text = _"I'm not ready to escort you to the town.",
		code = function()
			next("node99")
		end,
	},
	{
		id = "node41",
		text = _"I'm ready to escort you to the town.",
		code = function()
			Tania_follow_tux = true
			Tux:update_quest(_"Tania's Escape", _"I have agreed to escort Tania to the town. Once I'm there I'll introduce her to Spencer.")
			set_bot_state("follow_tux")
			hide("node8", "node40", "node41") next("node91")
		end,
	},
	{
		id = "node45",
		text = _"It isn't all like this, the town is very nice.",
		code = function()
			hide("node45", "node46", "node47") next("node48")
		end,
	},
	{
		id = "node46",
		text = _"I got lost in this desert once.",
		code = function()
			hide("node45", "node46", "node47") next("node48")
		end,
	},
	{
		id = "node47",
		text = _"It must be hard adjusting to the bright sunlight.",
		code = function()
			npc_says(_"I was underground for so long.")
			npc_says(_"It is all so bright on my eyes.")
			hide("node45", "node46", "node47") next("node48")
		end,
	},
	{
		id = "node48",
		text = "BUG, REPORT ME! Tania node48 -- TANIA'S THIRST FOR WATER",
		echo_text = false,
		code = function()
			number_of_liquid_items = 0
			if (Tux:has_item_backpack("Bottled ice")) then
				number_of_liquid_items = number_of_liquid_items + 1
				show("node49")
			end
			if (Tux:has_item_backpack("Industrial coolant")) then
				number_of_liquid_items = number_of_liquid_items + 1
				show("node50")
			end
			if (Tux:has_item_backpack("Liquid nitrogen")) then
				number_of_liquid_items = number_of_liquid_items + 1
				show("node51")
			end
			if (Tux:has_item_backpack("Barf's Energy Drink")) then
				number_of_liquid_items = number_of_liquid_items + 1
				show("node52")
			end
			if (number_of_liquid_items > 0) then
				npc_says(_"I hope we get there soon. I'm very thirsty.")
			else
				next("node54")
			end
			show("node53")
		end,
	},
	{
		id = "node49",
		text = _"Would you like some bottled ice?",
		code = function()
			npc_says(_"Thank you very much.")
			npc_says(_"I feel very refreshed!")
			heal_npc()
			Tux:del_item_backpack("Bottled ice")
			Tux:update_quest(_"Tania's Escape", _"Tania wasn't prepared for the desert heat. I gave her some bottled ice, and she looked much more healthy.")
			hide("node49", "node50", "node51", "node52", "node53")
		end,
	},
	{
		id = "node50",
		text = _"I have some industrial coolant you could have.",
		code = function()
			npc_says(_"I can't drink this.")
			if Tux:has_met("Ewald") then
				Tux:says(_"I've seen the town bartender put it in drinks.")
				npc_says(_"I guess I'll give it a try then.")
				Tux:del_item_backpack("Industrial coolant")
				heal_npc()
				npc_says(_"I feel very cold, but better.") --TODO: freeze her here
				Tux:update_quest(_"Tania's Escape", _"Tania wasn't prepared for the desert heat. I gave her some Industrial coolant. At first she was hesitant, but she tried it.")
				hide("node49", "node50", "node52", "node53")
			else
				next("node54")
			end
			hide("node51")
		end,
	},
	{
		id = "node51",
		text = _"Can I offer you some liquid nitrogen?",
		code = function()
			npc_says(_"I can't drink this.")
			next("node54") hide("node51")
		end,
	},
	{
		id = "node52",
		text = _"You could have a bottle of Barf's Energy Drink if you are thirsty?",
		code = function()
			npc_says(_"Thank you very much.")
			npc_says(_"I feel very energetic!")
			heal_npc()
			Tux:del_item_backpack("Barf's Energy Drink")
			Tux:update_quest(_"Tania's Escape", _"Tania wasn't prepared for the desert heat. I gave her a bottle of Barf's Energy Drink. After downing it in a couple seconds, she looked much more energetic!")
			hide("node49", "node50", "node51", "node52", "node53")
		end,
	},
	{
		--THIS WILL BE A LIE
		id = "node53",
		text = _"Sorry, I have nothing to offer you.",
		code = function()
			npc_says(_"I feel very ill.")
			Tux:update_quest(_"Tania's Escape", _"Tania wasn't prepared for the desert heat, but I decided not to share any of my liquids with her.")
			drop_dead()
			hide("node49", "node50", "node51", "node52", "node53")
		end,
	},
	{
		id = "node54",
		text = "BUG, REPORT ME! Tania node54 -- NO LIQUID ITEMS",
		echo_text = false,
		code = function()
			number_of_liquid_items = number_of_liquid_items - 1
			if (number_of_liquid_items < 1) then
				if (npc_damage_amount() > 10) then
					next("node55")
				else
					npc_says(_"I feel a little faint, but I think I will survive.")
				end
				hide("node53")
			end
		end,
	},
	{
		id = "node55",
		text = "BUG, REPORT ME! Tania node55 -- HANDLE INJURIES",
		echo_text = false,
		code = function()
			injured_level = 0
			if (npc_damage_amount() > 10) then
				npc_says(_"I am injured!")
				injured_level = 1
			elseif (npc_damage_amount() > 40) then
				npc_says(_"I am badly injured!")
				injured_level = 2
			elseif (npc_damage_amount() > 60) then
				npc_says(_"I am seriously injured!")
				injured_level = 3
			end

			if (Tux:has_item_backpack("Doc-in-a-can")) then
				show("node59")
			end
			if (Tux:has_item_backpack("Antibiotic") and
			   (injured_level < 3)) then
				show("node58")
			end
			if (Tux:has_item_backpack("Diet supplement") and
			   (injured_level < 2)) then
				show("node57")
			end
			show("node56")
		end,
	},
	{
		id = "node56",
		text = _"There is nothing I can do about your injuries right now.",
		code = function()
			npc_says(_"I hope we get to the town soon.")
			hide("node56", "node57", "node58", "node59")
		end,
	},
	{
		id = "node57",
		text = _"Here, take this Diet supplement.",
		code = function()
			npc_says(_"I feel better now.")
			Tux:del_item_backpack("Diet supplement")
			heal_npc()
			hide("node56", "node57", "node58", "node59")
		end,
	},
	{
		id = "node58",
		text = _"I'm proscribing you some antibiotics.",
		code = function()
			npc_says(_"I feel much better.")
			Tux:del_item_backpack("Antibiotic")
			heal_npc()
			hide("node56", "node57", "node58", "node59")
		end,
	},
	{
		id = "node59",
		text = _"I have a Doc-in-a-can. It should heal you right up.",
		code = function()
			npc_says(_"I feel fit as new!")
			Tux:del_item_backpack("Doc-in-a-can")
			heal_npc()
			hide("node56", "node57", "node58", "node59")
		end,
	},
	{
		id = "node70",
		text = _"I think you would enjoy some of Michelangelo's cooking at the restaurant.",
		code = function()
			set_bot_destination("BarPatron-Enter")
			Tania_at_Ewalds_Bar = true
			npc_says(_"It has been so long since I've had a nice meal.")
			hide("node70")
		end,
	},
	{
		id = "node71",
		text = _"You might try the bar. But stay away from the food. It is horrible.",
		code = function()
			set_bot_destination("BarPatron-Enter")
			Tania_at_Ewalds_Bar = true
			npc_says(_"I really miss good food, especially lemon meringue pie.")
			npc_says(_"Oh well.")
			hide("node71")
		end,
	},
	{
		--Pre-"Tania's Escape" Quest
		id = "node90",
		text = _"I think I have to go.",
		echo_text = false,
		code = function()
			hide("node90") next("node99")
		end,
	},
	{
		--"Tania's Escape" Quest (Underground)
		id = "node91",
		text = _"Follow me to the Surface!",
		echo_text = false,
		code = function()
			hide("node91") next("node99")
		end,
	},
	{
		--"Tania's Escape" Quest (Western Desert)
		id = "node92",
		text = _"Follow me to the Town!",
		echo_text = false,
		code = function()
			hide("node92") next("node99")
		end,
	},
	{
		--"Tania's Escape" Quest (Western Town Gate)
		id = "node93",
		text = _"Wait here.",
		echo_text = false,
		code = function()
			hide("node93") next("node99")
		end,
	},
	{
		--"Tania's Escape" Quest (Western Town Gate to Doctor's Office)
		id = "node94",
		text = _"See you later.",
		echo_text = false,
		code = function()
			hide("node94") next("node99")
		end,
	},
	{
		id = "node99",
		text = "BUG, REPORT ME! Tania node99 -- EXIT DIALOG",
		echo_text = false,
		code = function()
			if (Tux:has_quest("Tania's Escape")) then
				hide("node1", "node2", "node3", "node4", "node5", "node6", "node7", "node10", "node11", "node12", "node13", "node14", "node18", "node19")
				if (not SACD_gunsoff) then --still underground, guns on
					Tux:says(_"I think I have to go.")
				elseif (not Tania_surface) then --still underground
					Tux:says(_"Follow me to the Surface!")
				elseif (not Tania_stopped_by_Pendragon) then --"Tania's Escape" Quest (Western Desert)
					Tux:says(_"Follow me to the Town!")
					npc_says(_"Lead on, my little penguin.")
				elseif (not Spencer_Tania_sent_to_DocMoore) then --"Tania's Escape" Quest (Western Town Gate)
					Tux:says(_"Wait here.")
					npc_says(_"OK. But please, come back soon.")
				elseif (Spencer_Tania_sent_to_DocMoore) then --"Tania's Escape" Quest (Tania free in the town somewhere)
					Tux:says(_"See you later.")
					npc_says_random(_"Please be safe!",
						_"Thanks again.",
						_"Please come back again, my little penguin.")
				end
			else
				npc_says_random(_"That's OK. But please, please, come back. I'm so lonely.",
					_"Please come back again and get me out of here.")
			end
			end_dialog()
		end,
	},
}
