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
		if (not DocMoore_met) then
			show("node0")
		end

		if (Tux:has_quest("Bender's problem")) and
		   (DocMoore_met) and
		   (not Tux:done_quest("Bender's problem")) then
			show("node2")
		end

		if (Spencer_Tania_sent_to_DocMoore) and
		(not DocMoore_Tania_OK) then
			DocMoore_Tania_OK = true
			show("node25")
		end

		if (Tux:has_item_backpack("Strength Pill")) then show("node50") end
		if (Tux:has_item_backpack("Dexterity Pill")) then show("node51") end
		if (Tux:has_item_backpack("Code Pill")) then show("node52") end

		if (Tux:has_item_backpack("Diet supplement")) then show("node55") end
		if (Tux:has_item_backpack("Antibiotic")) then show("node56") end
		if (Tux:has_item_backpack("Doc-in-a-can")) then show("node57") end

		if (Tux:get_hp_ratio() < 0.1) then
			npc_says(_"You look gravely injured.")
			npc_says(_"I will help you.")
			Tux:says(_"I...")
			npc_says(_"It's ok, you'll be fine soon.")
			npc_says(_"A little spray here...")
			npc_says(_"An injection there...", "NO_WAIT")
			Tux:says(_"OW!")
			npc_says(_"Now, swallow this pill.")
			Tux:says(_"*glup*")
			npc_says(_"Ok, you are fixed now.")
			Tux:says(_"Oh, thank you!")
			npc_says(_"Take better care of yourself, Linarian.")
			Tux:heal()
		end

		if (Tux:get_hp_ratio() < 0.2) then
			npc_says(_"You don't look too good...")
			npc_says(_"I can help you if you want.")
		elseif (Tux:get_hp_ratio() < 0.4) then
			npc_says(_"You should take better care of yourself out there.")
		end

		if (will_rush_tux()) then
			set_rush_tux(0)
		end

		if (DocMoore_met) and
		   (Tux:has_item("Rubber duck")) and
		   (not DocMoore_not_seen_rubber_duck_lie) then
			npc_says(_"Oh, did you by any chance see a bright yellow item made out of polyvinyl chloride?")
			show("node42", "node43")
		end

		show("node99")
	end,

	{
		id = "node0",
		text = _"Hello!",
		code = function()
			npc_says(_"Hello. I'm Doc Moore. I'm the medic of this town. I don't believe we've met?")
			Tux:says(_"I'm %s.", Tux:get_player_name())
			npc_says(_"Um, what are you? Some kind of overgrown penguin?")
			Tux:says(_"I'm a Linarian.")
			npc_says(_"Oh, I vaguely remember reading something about Linarian biology back in my university days...")
			npc_says(_"Wait... did you come from outside of town?")
			Tux:says(_"Yes, I had to fight my way here through a bunch of bots.")
			npc_says(_"Oh my!")
			npc_says(_"Well, I should be able to heal you if you get hurt.")
			DocMoore_met = true
			if (Tux:has_quest("Bender's problem")) then
				show("node2")
			end
			set_bot_name(_"Doc Moore - Medic")
			hide("node0") show("node1", "node3", "node10")
		end,
	},
	{
		id = "node1",
		text = _"Do you also sell medical equipment?",
		code = function()
			npc_says(_"Yes. Here is what I can offer today.")
			trade_with("DocMoore")
			hide("node1") show("node40")
		end,
	},
	{
		id = "node2",
		text = _"Doc, I took some of those brain enlargement pills...",
		code = function()
			if (DocMoore_healed_tux) then
				npc_says(_"Sorry, but I already gave you some medical help.")
				npc_says(_"To poison yourself again, that's entirely your own business and I'm not responsible.")
				npc_says(_"As far as I am concerned, in this town everyone gets everything equally.")
				show("node20")
			else
				npc_says(_"Oh no, not another one. Those pills are almost pure biological waste.")
				npc_says(_"Taking that stuff almost always equals delayed suicide.")
				npc_says(_"Now, take this antidote. It should remove the dangerous substances within your body.")
				npc_says(_"But remember, I'll only give you this help once, because you didn't know the effects.")
				npc_says(_"Should you take that junk again, I won't feel responsible for what happens to you any more.")
				DocMoore_healed_tux = true
				Tux:update_quest(_"Bender's problem", _"The doctor was easily fooled. I have the pills that Bender needs.")
				Tux:add_item("Brain Enlargement Pills Antidote",1)
			end
			hide("node2")
		end,
	},
	{
		id = "node3",
		text = _"Doctor, how can I keep healthy and alive?",
		code = function()
			npc_says(_"Always remember L-I-F-E")
			npc_says(_"L - Look at your health status regularly.")
			npc_says(_"I - Ingest cold water if you are overheating.")
			npc_says(_"F - Flee if you cannot fight.")
			npc_says(_"E - Evacuate to the town if you cannot flee.")
			hide("node3")
		end,
	},
	{
		id = "node10",
		text = _"Can you fix me up?",
		code = function()
			npc_says(_"Sure, as the only doctor of this slowly growing community, I take responsibility for everyone's health.")
			if (not DocMoore_asked_self_damage) then
				npc_says(_"However, self-inflicted damage might be exempted from this rule in some cases...")
				show("node11")
			end
			if (Tux:get_hp_ratio() == 1) then
				npc_says(_"You seem to be in excellent health, there is nothing I can do for you right now.")
			else
				npc_says_random(_"There, it's done. You're completely fixed. You can go now.",
					_"You need to keep better care of yourself. You're completely fixed. You can go now.")
				Tux:heal()
			end
			hide("node0")
		end,
	},
	{
		id = "node11",
		text = _"What do you mean, self-inflicted damage?",
		code = function()
			if (guard_follow_tux) or
			   (tux_has_joined_guard) then
				npc_says(_"Well, you see that Bender character on my doorstep?")
			else
				npc_says(_"Well, you see that idiotic Bender character on my doorstep?")
			end
			Tux:says(_"What can you tell me about Bender?")
			npc_says(_"Bender asked my advice about some pills he saw advertised in an e-mail. I told him not to buy them.")
			npc_says(_"But guess what? He bought and took the stupid pills anyway, and then he came back to me to fix him.")
			npc_says(_"If he, or anyone, is going to completely disregard my medical advice, and then think they are going to get my medical supplies, then they are wrong.")
			npc_says(_"He won't get anything from me anymore. It would be unfair to the community to waste all the supplies on him, and that's my final word on that.")
			DocMoore_asked_self_damage = true
			hide("node11")
		end,
	},
	{
		id = "node20",
		text = _"Doc... I really want the antidote.",
		code = function()
			npc_says(_"No! I am not giving it to you. Forget about it.")
			hide("node20") show("node21", "node30")
		end,
	},
	{
		id = "node21",
		text = _"My patience is running out. Give me the antidote. Now.",
		code = function()
			npc_says(_"No! Over my dead body.")
			hide("node21") show("node22", "node30")
		end,
	},
	{
		id = "node22",
		text = _"Your wish is my command!",
		code = function()
			npc_says(_"Huh? What?")
			Tux:says(_"Humans... You are so interesting. I always wanted to know exactly how much blood you have.")
			npc_says(_"Don't do this. Don't do this.")
			Tux:says(_"Prepare, Doctor. The experiment begins.")
			npc_says(_"Don't do this! DON'T DO THIS!")
			npc_says(_"AAAAAAAAAAAA!")
			Tux:says(_"Ugh. Human blood is disgusting. At least four liters. I feel sick.")
			npc_says(_" . . .")
			Tux:says(_"I guess this means I get to inherit all your stuff. I hope you don't mind if I take everything?")
			npc_says(_" . . .")
			Tux:says(_"Good. I am very glad not to hear a disapproval.")
			npc_says(_" . . .")
			killed_docmoore = true
			drop_dead()
			Tux:add_item("Diet supplement",15)
			Tux:add_item("Diet supplement",15)
			Tux:add_item("Doc-in-a-can",10)
			Tux:add_item("Antibiotic",10)
			Tux:add_item("Brain Enlargement Pills Antidote",5)
			Tux:add_item("Laser Scalpel",1)
			Tux:add_item("Doc-in-a-can",10)
			Tux:add_item("Antibiotic",10)
			Tux:add_item("Brain Enlargement Pills Antidote",5)
			hide("node22", "node10", "node11", "node50", "node51", "node52", "node55", "node56", "node57")
		end,
	},
	{
		id = "node25",
		text = _"Is Tania OK?",
		code = function()
			npc_says(_"Haven't you heard of doctor-patient confidentiality?")
			Tux:says(_"Ummm...")
			npc_says(_"No worries. She is fine.")
			heal_npc("Tania")
			hide("node25")
		end,
	},
	{
		id = "node30",
		text = _"Forget it. Getting the antidote is not worth the effort.",
		code = function()
			npc_says(_"I am not giving it to you, and that won't change.")
			hide("node21", "node22", "node30")
		end,
	},
	{
		id = "node40",
		text = _"May I buy some medical equipment?",
		code = function()
			npc_says(_"Sure. Here is what I can offer today.")
			trade_with("DocMoore")
		end,
	},
	{
		id = "node42",
		text = _"Hmm. I cannot remember.",
		code = function()
			Tux:says(_"I'm sorry.")
			npc_says(_"No problem. Thanks anyway.")
			DocMoore_not_seen_rubber_duck_lie = true
			hide("node42", "node43")
		end,
	},
	{
		id = "node43",
		text = _"I think I did.",
		echo_text = false,
		code = function()
			Tux:says(_"Hmm... Yellow, bright, PVC...?")
			Tux:says(_"I think I did.")
			Tux:says(_"Do you mean this rubber duck by any chance?")
			npc_says(_"Ooh, you found it.")
			Tux:says(_"Here, take it if it's yours.")
			npc_says(_"Thanks.")
			npc_says(_"Take this healthy drink as reward.")
			Tux:says(_"Looks.. erm... interesting...")
			Tux:says(_"Thank you. I am sure it ... can be quite useful in some situations.")
			Tux:add_item("Doc-in-a-can", 1)
			Tux:del_item("Rubber duck")
			hide("node42", "node43")
		end,
	},
	{
		id = "node50",
		text = _"What can you tell me about Strength Pills?",
		code = function()
			hide("node50") next("node53")
		end,
	},
	{
		id = "node51",
		text = _"What can you tell me about Dexterity Pills?",
		code = function()
			hide("node51") next("node53")
		end,
	},
	{
		id = "node52",
		text = _"What can you tell me about Code Pills?",
		code = function()
			hide("node52") next("node53")
		end,
	},
	{
		id = "node53",
		text = _"BUG REPORT ME! Francis node 53!",
		echo_text = false,
		code = function()
			npc_says(_"Those pills are only one variant of a fantastic scientific breakthrough that happened shortly before the Great Assault.")
			npc_says(_"Three kinds of enhancement pills were developed. One for strength, one for dexterity and one for programming abilities.")
			npc_says(_"These pills work on a nanotechnological basis with small machines connecting to your muscle and nerve tissue.")
			npc_says(_"The machines connect together and form some inorganic artificial tissue that has been optimized for certain qualities.")
			npc_says(_"Since this invention only came about shortly before the Great Assault, these pills are now very rare.")
			npc_says(_"But if you should get them, even better, because the effects are permanent, and as far as we can tell, there aren't any side effects!")
			hide("node53")
		end,
	},
	{
		id = "node54",
		text = _"What can you tell me about Brain Pills?",
		code = function()
			if (DocMoore_healed_tux) then
				npc_says(_"I already warned you about those!")
			end
			npc_says(_"Those pills are almost pure biological waste! They are sold to stupid ignorant gullible people. Never EVER take one.")
			hide("node54")
		end,
	},
	{
		id = "node55",
		text = _"What can you tell me about Diet supplements?",
		code = function()
			npc_says(_"Have you tasted the army snacks that the cook, Michelangelo, has been handing out?")
			if (Michelangelo_been_asked_for_army_snacks) then
				Tux:says(_"Yes, those were horrible. They had a nice color though.")
				npc_says(_"The dye used to make that color is a known carcinogen.")
			else
				Tux:says(_"No, should I?")
				npc_says(_"Not if you can avoid it.")
			end
			npc_says(_"Well, unlike the army snacks, the Diet Supplements actually have a slight nutritional benefit.")
			npc_says(_"After taking one your health should improve slightly.")
			hide("node55")
		end,
	},
	{
		id = "node56",
		text = _"What can you tell me about Antibiotics?",
		code = function()
			npc_says(_"Basically it is bottled up poison made by bacteria.")
			Tux:says(_"And that is good for me?")
			npc_says(_"Yep. It kills the bacteria that want to kill you. It improves your health significantly.")
			hide("node56")
		end,
	},
	{
		id = "node57",
		text = _"What is a Doc-in-a-can?",
		code = function()
			npc_says(_"It is a device that releases millions of short-lived nanobots that swarm all over your body inside and out repairing and fixing almost all but the most serious wounds.")
			Tux:says(_"Wouldn't long-lived nanobots work better?")
			npc_says(_"They found that long-lived nanobots evolve self-replication and act like a cancer. A cancer of gray goo that eats everything.")
			npc_says(_"Several planets were made uninhabitable before they figured that one out.")
			hide("node57")
		end,
	},
	{
		id = "node99",
		text = _"See you later.",
		code = function()
			npc_says_random(_"See you later.",
				_"Keep healthy!")
			end_dialog()
		end,
	},
}
