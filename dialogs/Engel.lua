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
--[[WIKI
PERSONALITY = { "Militaristic", "Focused", "Vengeful" },
MARKERS = { NPCID1 = "Geist", },
PURPOSE = "$$NAME$$ improves Tux\'s skills.",
BACKSTORY = "$$NAME$$ and $$NPCID1$$ have become bot hunters after the loss their mother to a bot attack. $$NAME$$ use to
	 construct bots, and now finances himself by recovering bot parts and selling them. He speaks broken English (with some
	 German). His family name is Fleischer (in English - \"butcher\"). $$NAME$$ is mistrustful of the Red Guard.",
RELATIONSHIP = {
	{
		actor = "$$NPCID1$$",
		text = "$$NPCID1$$ and $$NAME$$ are brothers."
	},
}
WIKI]]--

local Tux = FDrpg.get_tux()

return {
	FirstTime = function()
		show("node0", "node6")
	end,

	EveryTime = function()
		if (Tux:get_program_revision("Extract bot parts") > 0) then
			show("node18")
		end
		show("node99")
	end,

	{
		id = "node0",
		text = _"Hi! I'm new here. ",
		code = function()
			--; TRANSLATORS: Fleischer is a name, but also means "butcher"
			npc_says(_"Hello. We are the Fleischer brothers.")
			--; TRANSLATORS: Engel = Angle, Geist = Ghost or Spirit
			npc_says(_"I am Engel. He is Geist, and he does not know the language.")
			set_bot_name("Engel - Hunter")
			set_bot_name("Geist - Hunter", "Geist")
			hide("node0") show("node1")
		end,
	},
	{
		id = "node1",
		text = _"What are you doing here?",
		code = function()
			npc_says(_"Revenge.")
			npc_says(_"The bots have killed our mother. Now we kill them.")
			npc_says(_"No mercy. We hunt them.")
			hide("node1") show("node2", "node11", "node13")
		end,
	},
	{
		id = "node2",
		text = _"Can I help you somehow?",
		code = function()
			--; TRANSLATORS: Nein = No
			npc_says(_"Nein. Let us die as warriors. That is all we ask for.")
			hide("node2") show("node3")
		end,
	},
	{
		id = "node3",
		text = _"If you keep standing here more bots will come and they will kill you. Go somewhere safe, and set up defense there.",
		echo_text = false,
		code = function()
			Tux:says(_"If you keep standing here more bots will come and --")
			--; TRANSLATORS: Sehr gut = Very good or very well.
			npc_says(_"Sehr gut. Let them come. We are ready.")
			npc_says(_"What a beautiful day. Yes.")
			hide("node3") show("node5")
		end,
	},
	{
		id = "node5",
		text = _"You are insane. Get out of here while you still can.",
		code = function()
			npc_says(_"No.")
			npc_says(_"Every day since the mess started I come here to fight the bots.")
			npc_says(_"Today will be no different.")
			npc_says(_"Do not try to stop us. We have our rifles loaded.")
			hide("node5") show("node7")
		end,
	},
	{
		id = "node6",
		text = _"Where can I get some help?",
		code = function()
			npc_says(_"Nowhere. This is the end of the world. There is no help.")
			npc_says(_"There is a town a bit south from here, but if I were you, I would not go there.")
			--; TRANSLATORS: Rot = red
			npc_says(_"The Rot Guard is the ruler there, and cruelty is their solution to everything.")
			npc_says(_"Real freedom is really dead.")
			hide("node6")
		end,
	},
	{
		id = "node7",
		text = _"You guys are totally nuts. Get a life.",
		echo_text = false,
		code = function()
			Tux:says(_"You guys are tota --")
			npc_says(_"WHAT?! What is it that you want to say?")
			Tux:says(_"Erm... I --")
			--; TRANSLATORS: Backpfeife = Slap in the face; Gesicht = face / backpfeigengesicht: a face badly in need of a slap into it
			npc_says(_"WHAT!? SPIT IT OUT, BACKPFEIFENGESICHT!")
			Tux:says(_"Er... You guys are really great and I admire your bravery in the face of certain death.")
			--; TRANSLATORS: donner = thunder; wetter = weather :  Thunderstorm; here 'Donnerwetter' means "wow!".
			npc_says(_"Donnerwetter, you are a funny one.")
			npc_says(_"I hope that one day we shall meet again, maybe in a better time.")
			--; TRANSLATORS: -- gut = good
			npc_says(_"Maybe then you will tell us other gut jokes that you know.")
			npc_says(_"Maybe one day...")
			hide("node7")
		end,
	},
	{
		id = "node11",
		text = _"What did you use to do before you became a hunter?",
		code = function()
			npc_says(_"I was a bot constructor. Built the killers of my mother.")
			npc_says(_"Now I sell the parts that I remove from the bots to buy more ammunition and food.")
			hide("node11") show("node12", "node14")
		end,
	},
	{
		id = "node12",
		text = _"I want to know how to extract modules from the bots.",
		code = function()
			if (Tux:get_program_revision("Extract bot parts") > 4) then
				--; TRANSLATORS: schon = already , alles = everything/all
				npc_says(_"I schon told you alles I know about that.")
				hide("node12", "node15")
			else
				Engel_offered_extraction_skill = true
				npc_says(_"Yes.")
				--; TRANSLATORS: Schaltkreise = circuits
				npc_says(_"I will sell the information for a lot of Schaltkreise.")
				npc_says(_"Interested?")
				show("node15", "node98")
				push_topic("Extract bot parts")
			end
		end,
	},
	{
		id = "node13",
		text = _"Can I join your hunt?",
		code = function()
			--; TRANSLATORS: nein = no
			npc_says(_"Nein. Our problem. Our hunt.")
			hide("node13")
		end,
	},
	{
		id = "node14",
		text = _"What happens to these bot parts?",
		code = function()
			Tux:says(_"Are they used to build other bots?")
			--; TRANSLATORS: bauen = to build, to construct ; in this context more like 'create' ;; nette = nice
			npc_says(_"Most of them are used to bauen little nette gadgets.")
			npc_says(_"You can use them to make weapons better or make you stronger.")
			Tux:says(_"How does this work?")
			npc_says(_"I don't know.")
			npc_says(_"But these gadgets are very nice for making your equipment better.")
			Tux:says(_"So they are kind of add-ons?")
			npc_says(_"Yes.")
			--; TRANSLATORS: Stadtmenschen = townspeople
			npc_says(_"Ask the Stadtmenschen if you want to know more.")

			if (not Tux:has_met("Dixon")) then
			--; TRANSLATORS: Rot = red
				npc_says(_"I think the Rot Guard makes them.")
				--; TRANSLATORS: Gluck (should actually be 'Glueck' but there is no ue character in the font) = luck, fortune
				npc_says(_"If you have Gluck, they won't kill you directly.")
			end

			npc_says(_"But I don't know more about these gadgets.")
			npc_says(_"Asking me more questions about that makes no sense...")
			hide("node14")
		end,
	},
	{
		id = "node15",
		text = _"I'm ready. Please show me how to extract bot parts. (costs 50 circuits, 1 training point)",
		topic = "Extract bot parts",
		code = function()
			next("node17")
		end,
	},
	{
		id = "node17",
		text = "BUG, REPORT ME! Engel node17 -- checks",
		echo_text = false,
		topic = "Extract bot parts",
		code = function()
			local rev = Tux:get_program_revision("Extract bot parts")

			if (rev > 4) then -- Max level
				next("node70")
			elseif (Tux:can_train(50, 1)) then
				Tux:del_gold(50)
				Tux:del_points(1)
				Tux:improve_program("Extract bot parts")
				next("node" .. 20 + 10 * rev) -- Learn next rev; shows one of the nodes 20, 30, 40, 50, 60
			else -- Can't train
				if (Tux:get_gold() >= 50) then -- Training points were the problem
					npc_says(_"Hmmm...")
					npc_says(_"You look green. Toughen up. Get some experience, kill some bots.")
					npc_says(_"Then we can talk.")
				else -- No $$$
					npc_says(_"Hmmm...")
					--; TRANSLATORS: No money, no help.
					npc_says(_"Kein Geld, keine Hilfe.")
					npc_says(_"Come back with some circuits.")
				end

				pop_topic() -- "Extract bot parts"
			end
		end,
	},
	{
		id = "node18",
		text = _"Your lessons are very helpful, but at the same time difficult to understand.",
		code = function()
			npc_says(_"If you want me to repeat something, don't hesitate to ask.")
			local rev = Tux:get_program_revision("Extract bot parts")
			for i = 0, 4 do
				if (rev > i) then show("node" .. 19 + 10*i) else break end -- show some of the nodes 19, 29, 39, 49, 59
			end
			push_topic("Extract bot parts")
		end,
	},
	{
		id = "node19",
		text = _"Could you please repeat the lesson about Entropy Inverters?",
		topic = "Extract bot parts",
		code = function()
			hide("node19") next("node20")
		end,
	},
	{
		id = "node20",
		text = "BUG, REPORT ME! Engel node20 -- 1st LESSON",
		echo_text = false,
		topic = "Extract bot parts",
		code = function()
			npc_says(_"Let us start with Entropy Inverters. This is quite simple.")
			--; TRANSLATORS: sehr = very
			npc_says(_"Just take a hammer and hit the bot sehr hard on the head.")
			npc_says(_"When you have the head open wide, look around for a part which looks like two circles around a cube. That is what you are looking for.")
		end,
	},
	{ -- @TODO:  add some more german stuff to the following sections...
		id = "node29",
		text = _"What was that about Plasma Transistors?",
		topic = "Extract bot parts",
		code = function()
			hide("node29") next("node30")
		end,
	},
	{
		id = "node30",
		text = "BUG, REPORT ME! Engel node30 -- 2nd LESSON",
		echo_text = false,
		topic = "Extract bot parts",
		code = function()
			npc_says(_"How about... Plasma Transistors? Trivial.")
			npc_says(_"Remove the bot's engine and look around somewhere close to the battery.")
			npc_says(_"Plasma Transistors are shaped like the letter Y. Just rip them out, it's safe.")
		end,
	},
	{
		id = "node39",
		text = _"I need a refresh of my knowledge about Superconductors.",
		topic = "Extract bot parts",
		code = function()
			hide("node39") next("node40")
		end,
	},
	{
		id = "node40",
		text = "BUG, REPORT ME! Engel node40 -- 3rd LESSON",
		echo_text = false,
		topic = "Extract bot parts",
		code = function()
			npc_says(_"Hmmm... Ah, gut. Superconductors.")
			npc_says(_"They are relatively easy to get out.")
			npc_says(_"Look around the electric system. Power coils, capacitors, generators and the like. Usually there are plenty of sehr gut parts there.") -- sehr gut = very good
			npc_says(_"Wear rubber gloves. Just in case, you know.")
		end,
	},
	{
		id = "node49",
		text = _"I would like to hear again something about Antimatter-Matter Converters.",
		topic = "Extract bot parts",
		code = function()
			hide("node49") next("node50")
		end,
	},
	{
		id = "node50",
		text = "BUG, REPORT ME! Engel node50 -- 4th LESSON",
		echo_text = false,
		topic = "Extract bot parts",
		code = function()
			npc_says(_"I will tell you everything about Antimatter-Matter Converters.")
			npc_says(_"Yes... Converters are a problem.")
			npc_says(_"They are easy to find, all you have to do is to get to the engine. But you must remember something very important:")
			npc_says(_"Make sure you turn the Converter off before you remove it.")
			npc_says(_"If you do not do that, you will only succeed in converting yourself to anti-matter. And as you know, there is no coming back from there.")
		end,
	},
	{
		id = "node59",
		text = _"My memory of Tachyon Condensators is incomplete.",
		topic = "Extract bot parts",
		code = function()
			hide("node59") next("node60")
		end,
	},
	{
		id = "node60",
		text = "BUG, REPORT ME! Engel node60 -- 5th LESSON",
		echo_text = false,
		topic = "Extract bot parts",
		code = function()
			npc_says(_"I will teach you about Tachyon Condensators.")
			npc_says(_"Tachyons are very interesting particles. Many stories have been told about them.")
			npc_says(_"I have no idea why bots have them inside. They are usually not connected to anything and serve no function.")
			npc_says(_"They are usually quite hot when you get them out, but they cool down quickly, so handling them is not a big deal.")
			npc_says(_"They can be installed just about anywhere, so you have to spend some time digging through the circuits to find one.")
		end,
	},
	{
		id = "node70",
		text = "BUG, REPORT ME! Engel node70 -- NO MORE LESSONS",
		echo_text = false,
		topic = "Extract bot parts",
		code = function()
			-- ; TRANSLATORS: nein = no , ja = yes, in this case it can be understood as "did you understand?"
			npc_says(_"Nein, I told you everything I know about that. No use asking me twice, ja?")
			hide("node12", "node15")
			pop_topic() -- "Extract bot parts"
		end,
	},
	{
		id = "node98",
		text = _"This should be enough for now.",
		topic = "Extract bot parts",
		code = function()
			hide("node19", "node29", "node39", "node49", "node59")
			pop_topic() -- "Extract bot parts"
		end,
	},
	{
		id = "node99",
		text = _"I'll be going then.",
		code = function()
			--; TRANSLATORS: viel Gluck = good luck (should be "viel Glueck, but the ue character is missing in the font)
			npc_says(_"Goodbye and viel Gluck.")
			end_dialog()
		end,
	},
}
