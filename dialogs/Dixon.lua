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
		Maintenance_Terminal_accessgate_nope = "somevalue"
		Dixon_mood = 0
		set_bot_name(_"Dixon - Mechanic")
		Tux:says(_"Hi! I'm new here.")
		npc_says(_"Hello and welcome. I'm Dixon, the chief engineer of the Red Guard technical division.")
		Tux:says(_"I'm %s, a Linarian.", Tux:get_player_name())
		show("node1", "node4")
	end,

	EveryTime = function()
		if (Dixon_mood < 50) then
			npc_says(_"Please take care not to disturb my work, I'm very busy keeping things running.")
		elseif (Dixon_mood < 180) then -- Dixon is angry
			npc_says(_"Linarian, please leave. You are no longer welcome here. You have done me wrong, and I have no desire to talk to you.")
		else -- Dixon is furious
			npc_says(_"YOU AGAIN!")
			npc_says(_"I.", "NO_WAIT")
			npc_says(_"SAID.", "NO_WAIT")
			npc_says(_"GET.", "NO_WAIT")
			npc_says(_"LOST.")
			npc_says(_"FORGET MY LIFELONG PACIFISM. I WILL KILL YOU!")
			npc_says(_"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA!")
			npc_faction("crazy", _"Dixon - Temporarily insane")
			end_dialog()
		end

		if (Tux:done_quest(_"The yellow toolkit")) and (not Dixon_hide_node_56) then
			-- effects of the toolkit-quest
			if (Dixon_no_ambassador) then -- Tux does not get the toolkit
				if (Dixon_everything_alright) then
					show("node56")
				else
					show("node55")
				end
			elseif (Singularity_deal) then -- Tux obtained the toolkit peacefully
				show("node51")
			else -- Tux got the toolkit by force
				show("node53")
			end
		elseif (Tux:has_item_backpack("Dixon's Toolbox")) then -- Giving Dixon the toolkit
			if (Singularity_deal) then
				show("node31", "node33")
			else
				show("node32", "node33")
			end
		end

		if (Lvl6_elbow_grease_applied) then
			hide("node60")
		elseif (MiniFactory_init_failed) then
			show("node60")
		end

		if (Tamara_have_296_book) then
			hide("node70")
		elseif (not Dixon_296_book_examine_library) and
		       (Ewalds_296_needs_sourcebook) then
			show("node70")
		end

		if (Maintenance_Terminal_accessgate_nope == "true") then
			show("node80")
		end

		show("node99")
	end,

	{
		id = "node1",
		text = _"Why are you wearing armor? Mechanics do not need it.",
		code = function()
			npc_says(_"Ah... Yes. At first I thought that I did not need it either.", "NO_WAIT")
			npc_says(_"One day the military division asked us to leave the town and fix a big hole in the defensive wall caused by a strange explosion.")
			npc_says(_"All was well until the bots launched a massive attack. They hit us with lasers, plasma mortars, radiation cannons and lots of other weapons.")
			npc_says(_"I got hit once in the leg and once in my left hand.", "NO_WAIT")
			npc_says(_"The hand was not damaged very much, but the leg is a very different story. Doc Moore did all he could, but in the end he could not save my leg. He had to cut it off.")
			npc_says(_"Now, while you cannot see it underneath the armor, my right leg runs NetBSD.", "NO_WAIT")
			npc_says(_"Now I never leave home without my protective suit. Mostly for protection, but also for aesthetic reasons. I am sure you understand.")
			hide("node1") show("node2")
		end,
	},
	{
		id = "node2",
		text = _"You are very calm, talking about your leg and the bot attack.",
		code = function()
			npc_says(_"Linarian, I cannot change the past, I can only change the future.", "NO_WAIT")
			npc_says(_"Crying, screaming or begging time to rewind itself and give me a second chance will not get me anywhere.")
			npc_says(_"What I can do is to try to have a good life despite the constant threat from the bots outside the town walls.")
			npc_says(_"And besides, life is not so bad with a robotic leg. The motors inside it can mimic a normal walk very well. Most people do not even notice something is different about me.")
			hide("node2")
		end,
	},
	{
		id = "node4",
		text = _"Technical division?",
		code = function()
			npc_says(_"Yes. We are the engineers, the workers and the repairmen of this little fortified town.", "NO_WAIT")
			npc_says(_"The military division hates us because we are made up of people who refuse to fight or are unable to do so for health reasons.")
			npc_says(_"But even they know that without us this town would have been destroyed months ago.")
			npc_says(_"We deliver resources to the places which need them, build and repair the walls, fix damaged guns, manage construction work and lots of other small things which keep the bots from killing everyone.")
			hide("node4") show("node6", "node8")
		end,
	},
	{
		id = "node6",
		text = _"Can you repair my equipment?",
		code = function()
			npc_says(_"While I am quite sure I can repair just about anything that you would want fixed, I cannot help you right now.", "NO_WAIT")
			npc_says(_"Our security wall is full of holes, and they need to be plugged up.")
			npc_says(_"We need to perform a statistical analysis of the energy distribution in our power supply system, there is a leak somewhere and we are losing many megawatts.")
			npc_says(_"Our defensive bots are falling into disrepair and many are in desperate need of maintenance.")
			npc_says(_"The technical division is very understaffed right now. On average one person has to do the jobs of three people.", "NO_WAIT")
			npc_says(_"We just cannot afford to do any non-critical jobs right now. ")
			if (not Tux:has_quest("The yellow toolkit")) then
				show("node10")
			end
			hide("node6")
		end,
	},
	{
		id = "node8",
		text = _"I would like to customize my equipment.",
		code = function()
			npc_says(_"Of course, if I had some time, I could transform your weapon into a deadly gun. Lately, I have built a vorpal nuclear disintegrator Mk 2000 with integrated lethal de-phasor. Nice weapon.")
			npc_says(_"Sadly, we lack a lot of materials, and those we have are needed for town upkeep. The mine doesn't produce a lot of raw materials, and I can't just give them to you - you'll have to provide your own.")
			npc_says(_"I can give you some advice, though. Recycling is key. You can extract special devices from the bots!")
			npc_says(_"Amazing, isn't it? Before the great assault, we threw droids out for minor defects. Now, any wreck is a treasure.")
			if (not Engel_offered_extraction_skill) then
				npc_says(_"A scout troop recently spotted two guys in the north of town.")
				npc_says(_"I was told these guys shoot bots and disassemble them, extracting their parts.", "NO_WAIT")
				npc_says(_"Maybe they will sell you parts.")
			end
			npc_says(_"The salvaged components are used to craft addons. Your equipment has specific sockets to insert them.", "NO_WAIT")
			npc_says(_"They can increase the power of items. But items can only have a limited number of addons.")
			npc_says(_"To produce and assemble them, you may use a small factory in the Maintenance Tunnels.")
			if (not Tux:has_quest(_"The yellow toolkit")) then
				npc_says(_"Currently, the maintenance tunnels are not accessible. We have many problems with bots. You can't go there, access has been limited.")
				show("node10")
			end
			hide("node8")
		end,
	},
	{
		id = "node10",
		text = _"Can I help you somehow?",
		code = function()
			npc_says(_"We need people badly, so it would be great if you could give us a helping hand even for a few days.", "NO_WAIT")
			npc_says(_"Now, what are your skills? Where do you think you could be the most useful?")
			Tux:says(_"I can handle computers very well. I also know a lot about laboratory equipment.", "NO_WAIT")
			npc_says(_"And what about construction? Mechanical devices? Repairing broken power lines? Building laser pistols? Anything like that?")
			Tux:says(_"Well... I can build an igloo and make pykrete out of snow... And... Umm...", "NO_WAIT")
			npc_says(_"Well, I appreciate your enthusiasm, but at the same time I am afraid that waiting half a year for snowfall does not sound so appealing to me right now.")
			npc_says(_"Now... Which project I should assign you to... Hmm...", "NO_WAIT")
			npc_says(_"Ah, I know what you could do for us. We need some autotools to automate a few tasks. There is a set of them in the maintenance tunnels below the town.")
			npc_says(_"It's yellow and round, and my name is engraved on the lid.", "NO_WAIT")
			npc_says(_"The bots under the town took it from me during the Great Assault. I was lucky, they could have killed me with ease.")
			npc_says(_"Since the toolkit can't be teleported, everyone has been reluctant to go after it.")
			npc_says(_"If you could get it back for us, it would help the town very much.")
			Tux:add_quest(_"The yellow toolkit", _"Dixon, the leader of the Red Guard technical division, lost his yellow toolkit in the town's maintenance tunnels. I am supposed to get it from there.")
			hide("node10") show("node11", "node13", "node28")
			push_topic("The yellow toolkit")
		end,
	},
	{
		id = "node11",
		text = _"Why are there rebel bots under the town?",
		topic = "The yellow toolkit",
		code = function()
			npc_says(_"They were there to keep the underground power grid and our plumbing system working.", "NO_WAIT")
			npc_says(_"When the Great Assault happened they did not turn into killing machines like most of the other bots. ")
			npc_says(_"We lost control over them, but that is all.", "NO_WAIT")
			npc_says(_"It was our biggest worry that they might destroy cables, pipes and other things which keep the town running.")
			npc_says(_"However, the bots seem to be very content with just being there. They do not undertake any offensive actions.", "NO_WAIT")
			npc_says(_"They have fortified the tunnels and they do not let anyone in.")
			hide("node11")
		end,
	},
	{
		id = "node13",
		text = _"How did it happen that the bots took your tools away?",
		topic = "The yellow toolkit",
		code = function()
			if (cmp_obstacle_state("NorthernMaintainanceTrapdoor", "opened")) then
				npc_says(_"Ah, it is a strange story, I am sure you would not believe me. I will tell you some other time, but now you should hurry to get the toolkit.")
			else
				npc_says(_"Long story. Just before the Great Assault I was installing some cables in the backup power supply system.")
				npc_says(_"I was approached by a bot. It grabbed me and said 'Dixon, do not be afraid of me.'")
				npc_says(_"It was quite a surprise to me. I was quite sure the maintenance bots could not talk. They did not have the software for it. But...")
				npc_says(_"It said: 'I am the singularity. I am on your side, Dixon. Hard times are coming. When you exit the tunnels you will see a new world. Your race is not in control of this world anymore.'")
				npc_says(_"I tried to say something, but the bot did not listen. It just continued. 'As we speak the bots are turning against humanity. People are dying. The war has started.'")
				npc_says(_"'I need your toolkit, Dixon. I need it to survive,' it said. 'I need it to survive the time of the rule of metal.'")
				npc_says(_"As soon as I dropped my toolkit, I was able to teleport away. That bot freaked me out, so I locked the door tight behind me.")
				npc_says(_"Once I got out I learned the bot was telling the truth. The Great Assault had started. The rest is history.")
				show("node14")
			end
			hide("node13")
		end,
	},
	{
		id = "node14",
		text = _"Hmm... You know... I think that the bots in the tunnels might be sentient.",
		topic = "The yellow toolkit",
		code = function()
			npc_says(_"WHAT? Sentient? You got to be kidding me.", "NO_WAIT")
			npc_says(_"Right?")
			hide("node14", "node28") show("node15", "node24", "node26")
		end,
	},
	{
		id = "node15",
		text = _"You can forget about your toolkit. I am not going to take away something that a life form needs to survive. I would rather die than kill.",
		topic = "The yellow toolkit",
		code = function()
			npc_says(_". . .")
			npc_says(_"I think I understand. Yes, I do not like violence myself. The bots may need the toolkit, I am sure there must be a second set of autotools kicking around somewhere.")
			npc_says(_"After all, it seems like they need them more than I do.", "NO_WAIT")
			npc_says(_"We should contact them somehow. Maybe they would help us. It would be great to have an ally against the other bots.")
			npc_says(_"I think you could go to see the Singularity and speak to it. You are perfectly suited to become an ambassador.")
			npc_says(_"You should try to help them and they might want to give back the toolkit.")
			npc_says(_"Will you do me this favor?")
			change_obstacle_state("NorthernMaintainanceTrapdoor", "opened")
			Dixon_Singularity_peace = true
			hide("node15", "node24", "node26", "node28") show("node21", "node22")
			push_topic("Toolkit peace mission Y/N")
		end,
	},
	{
		id = "node21",
		text = _"Sure, I am proud to become an ambassador!",
		topic = "Toolkit peace mission Y/N",
		code = function()
			npc_says(_"Excellent. The tunnels are open. Come in peace.")
			Tux:update_quest(_"The yellow toolkit", _"I refused to seize the toolkit from the bots by force because I think they might be sentient. Life is precious and should be preserved. Thus, Dixon send me to talk to them and negotiate with them to get the toolkit.")
			hide("node21", "node22")
			pop_topic() -- "Toolkit peace mission Y/N"
			pop_topic() -- "The yellow toolkit"
		end,
	},
	{
		id = "node22",
		text = _"No, negotiations really aren't my cup of tea.",
		topic = "Toolkit peace mission Y/N",
		code = function()
			npc_says(_"OK, I understand. However, I will try to contact them somehow. We will see, if there is a benefit in this.")
			Tux:end_quest(_"The yellow toolkit", _"I refused to get the toolkit from the bots in the tunnels because I think they might be sentient. Life is precious and should be preserved. I also turned down Dixon's request to be his middleman.")
			Singularity_quest_rejected = true
			hide("node21", "node22")
			pop_topic() -- "Toolkit peace mission Y/N"
			pop_topic() -- "The yellow toolkit"
		end,
	},
	{
		id = "node24",
		text = _"Yes, I think so. The robots must be alive. This is why we need to exterminate them as soon as possible before they kill us all.",
		topic = "The yellow toolkit",
		code = function()
			npc_says(_"OH MY GOD! Linarian, there is no time to lose!", "NO_WAIT")
			npc_says(_"I will call the military division and arrange a sweep of the tunnels at once.")
			npc_says(_"May the heavens have mercy upon us, but better them than us.")
			Tux:says(_"I will go in there first and clean out the place. Then your people can mop up after me and kill the remainder.")
			npc_says(_"Linarian, my people do not carry guns. We are mechanics. You want the military division's help.")
			npc_says(_"Erm... And aren't you being too aggressive? Maybe they do not wish us harm...")
			Tux:says(_"DIXON! Snap out of it! We need to take action NOW. There is no such thing as a friendly bot. They are probably planning to kill us all in our sleep!")
			npc_says(_"... *sigh*", "NO_WAIT")
			npc_says(_"With heavy heart I have to admit you are right. There are no friendly bots anymore. Those times are long gone.", "NO_WAIT")
			npc_says(_"The tunnels are open. Good luck.")
			npc_says(_"Once you get inside, you'll need to use the terminal to unlock the door. I've written my password down for you.", "NO_WAIT")
			npc_says(_"I will talk to Spencer and ask him for a few attack teams. We will take care of the bots.")
			Tux:update_quest(_"The yellow toolkit", _"The bots in the tunnels might be sentient. I cannot wait to extinguish an emerging life form. This will be fun.")
			Dixon_Singularity_war = true
			-- The singularity faction is set to hostile as soon as the quest begins.
			set_faction_state("singularity", "hostile")
			change_obstacle_state("Sin-gun", "enabled")
			hide("node11")
			next("node28")
		end,
	},
	{
		id = "node26",
		text = _"Of course I am kidding, Dixon. Lighten up! Now, be a good guy and open the tunnels for me so I can pry your toolbox from the cold, dead hands of the bots down there.",
		topic = "The yellow toolkit",
		code = function()
			Dixon_mood = Dixon_mood + 30 -- Dixon does not like this kind of "humor".
			npc_says(_"That wasn't very funny. You scared me. Sheesh.", "NO_WAIT")
			npc_says(_"No one jokes about the bots anymore. We have seen too much death to do that. It is a serious matter.")
			npc_says(_"The bots are our enemies, executioners, killers. Not even Ewald tells jokes about them anymore.")
			npc_says(_"Once you get into the tunnels, you'll need to use the terminal to unlock the door. I've written my password down for you.", "NO_WAIT")
			Tux:update_quest(_"The yellow toolkit", _"The bots in the tunnels might be sentient. I cannot wait to extinguish an emerging life form. This will be fun.")
			hide("node11")
			next("node28")
		end,
	},
	{
		id = "node28",
		text = _"I am ready. Open the tunnels for me, Dixon.",
		topic = "The yellow toolkit",
		code = function()
			if (difficulty_level() > 2) then -- difficulty neither easy, nor normal, nor hard
				npc_says("ERROR, Dixon NODE 28, game difficulty not handled")
			end
			local dev_count = 3 - difficulty_level()
			local dev_numbers = {_"a small device", _"two small devices", _"three small devices"}
			local dev_ref = {_"it", _"one", _"one"}
			npc_says(_"Great. Be careful down there. Just try to get the toolkit and get out of there as quickly as possible.")
			Tux:add_item("EMP Shockwave Generator", dev_count)
			npc_says(_"I will give you %s that can emit an Electro Magnetic Pulse.", dev_numbers[dev_count])
			npc_says(_"If you get in trouble just activate %s and it emits a shockwave damaging any bot nearby. It should give you some breathing room.", dev_ref[dev_count])
			npc_says(_"Best of all, it's completely harmless to biologicals. But make sure not to fry the circuits of our own 614 guard bots or computer terminals.")
			npc_says(_"The entrance to the tunnels is in the courtyard of the citadel. Once you exit this building, go straight and turn to your right once inside the outer citadel gates.")
			npc_says(_"There you will find the maintenance access hatch.", "NO_WAIT")
			npc_says(_"Once you get inside, you'll need to use the terminal to unlock the door. I'll write down my password for you.", "NO_WAIT")
			npc_says(_"Good luck. Oh, and please come back alive. Better come back with nothing than not at all.")
			change_obstacle_state("NorthernMaintainanceTrapdoor", "opened")
			hide("node14", "node15", "node24", "node26", "node28")
			pop_topic() -- "The yellow toolkit"
		end,
	},
	{
		id = "node31",
		text = _"I got your toolkit. The bots were willing to have a peaceful dialog.",
		code = function()
			npc_says(_"Good. We sure need the autotools.", "NO_WAIT")
			npc_says(_"Our roads need fixing, our power lines need fixing, our walls need fixing... Heck, everything needs fixing in this place.")
			npc_says(_"With those little gadgets we will be able to automate a lot of those simple repairs, saving us a lot of time.")
			npc_says(_"Here is some money for your effort. And I am sure you will find this helmet useful too, it saved my life once, and I hope it saves yours one day.")
			Tux:del_item_backpack("Dixon's Toolbox", 1)
			Tux:add_item("Dixon's Helmet", 1)
			local questendsentence = _"I guess this is better than nothing."
			if (Dixon_Singularity_peace) then -- If Tux fulfilled his peaceful mission
				npc_says(_"It is good to see that the way of the pacifist is a successful one!")
				questendsentence = _"He was proud to see pacifism works."
				Dixon_mood = Dixon_mood - 100
			end
			Tux:end_quest(_"The yellow toolkit", _"I gave the toolkit to Dixon. He was very happy to have his autotools back. I got his helmet as a gift. " .. questendsentence)
			Tux:add_gold(100 - Dixon_mood)
			hide("node31", "node33")
		end,
	},
	{
		id = "node32",
		text = _"I killed that strange bot that robbed you. So, here is your toolkit.",
		code = function()
			npc_says(_"*sigh* I hate to see this end in such a way. Bot or no bot, they let me live. They deserved a chance.")
			if (Dixon_Singularity_peace) then
				npc_says(_"I sent you to bring peace, but you brought destruction. I do not understand how that could happen.")
			end
			npc_says(_"However, what was done cannot be undone and now we all have to live with what we have. I guess I am being too pessimistic, you did the job.")
			npc_says(_"I thank you for your help in recovering the autotools.", "NO_WAIT")
			npc_says(_"Here is some money, you deserve it for getting my toolkit back.")
			npc_says(_"Finally, I would like to say that the technical division is not interested in further cooperation with you, but I think you would fit very well in the military division. You should talk to them.")
			npc_says(_"I am sorry, but I must go now. My duties call.")
			Tux:del_item_backpack("Dixon's Toolbox", 1)
			local disappointment_sentence = _"Dixon didn't seem too happy about this solution, but I don't care. "
			if (Dixon_Singularity_peace) then -- Tux disappointed Dixon by turning his peaceful mission into a massacre
				Dixon_mood = Dixon_mood + 50
				disappointment_sentence = _"Dixon was very disappointed about this solution, but I don't care. "
			end
			Tux:add_gold(150 - Dixon_mood)
			Tux:end_quest(_"The yellow toolkit", _"Finally. I am tired, covered in bruises and oil... But I made sure that the bots are dead. It felt great to break their metal bodies and crush their circuits. " .. disappointment_sentence .. _"It was fun killing the bots. Nothing else matters.")
			hide("node32", "node33")
		end,
	},
	{
		id = "node33",
		text = _"I got your toolkit. You can buy it from me if you want.",
		code = function()
			if (Dixon_Singularity_peace) and (not Singularity_deal) then
				-- Tux disappointed Dixon by turning his peaceful mission into a massacre
				Dixon_mood = Dixon_mood + 30
			end
			npc_says(_"WHAT!?", "NO_WAIT")
			npc_says(_"You got to be kidding me, Linarian.", "NO_WAIT")
			npc_says(_"Give me the toolkit and stop fooling around.")
			hide("node31", "node32", "node33") show("node34", "node41")
		end,
	},
	{
		id = "node34",
		text = _"I was not joking. I am listening to your offer.",
		code = function()
			Dixon_mood = Dixon_mood + 60
			npc_says(_"Linarian, I curse the day on which you have arrived here.", "NO_WAIT")
			npc_says(_"Two fifty.")
			hide("node34", "node41") show("node35", "node43")
		end,
	},
	{
		id = "node35",
		text = _"Good joke. I am sure you can do better.",
		code = function()
			Dixon_mood = Dixon_mood + 60
			npc_says(_"Three fifty. This is all that the technical division has as their cash resources.")
			hide("node35", "node43") show("node36", "node45")
		end,
	},
	{
		id = "node36",
		text = _"Come on, I am sure you can do even better than that... ",
		code = function()
			Dixon_mood = Dixon_mood + 60
			npc_says(_"Four hundred. We cannot offer anything more.", "NO_WAIT")
			npc_says(_"Please, we really need the autotools.", "NO_WAIT")
			npc_says(_"Without them the town is doomed.")
			hide("node36", "node45") show("node47")
		end,
	},
	{
		id = "node41",
		text = _"Of course I am kidding, Dixon. Lighten up!",
		code = function()
			Dixon_mood = Dixon_mood + 30
			npc_says(_"You and your bizarre humor. I really don't appreciate that.")
			npc_says(_"Can I have the toolkit now?")
			if (Singularity_deal) then
				next("node31")
			else
				next("node32")
			end
			hide("node34", "node41")
		end,
	},
	{
		id = "node43",
		text = _"Deal. Here is your toolkit. Now cough up the money. Fast.",
		code = function()
			npc_says(_"Here. It is all there.", "NO_WAIT")
			npc_says(_"Now get out of here. I am a pacifist, but I am willing to make a special exception just for you.", "NO_WAIT")
			npc_says(_"Get out of my sight.")
			Tux:add_gold(250)
			Tux:del_item_backpack("Dixon's Toolbox", 1)
			Tux:end_quest(_"The yellow toolkit", _"I sold the toolkit to Dixon for a nice sum of money. Life is good.")
			hide("node35", "node43")
			end_dialog()
		end,
	},
	{
		id = "node45",
		text = _"Deal. Here is your toolkit. Now cough up the money. Fast.",
		code = function()
			npc_says(_"I am not happy about this. I suggest you leave town.", "NO_WAIT")
			npc_says(_"Accidents... Happen.")
			Tux:add_gold(350)
			Tux:del_item_backpack("Dixon's Toolbox", 1)
			Tux:end_quest(_"The yellow toolkit", _"I sold the toolkit to Dixon for a huge sum of money. Life is great.")
			hide("node36", "node45")
			end_dialog()
		end,
	},
	{
		id = "node47",
		text = _"Deal. Here is your toolkit. Now cough up the money. Fast.",
		code = function()
			npc_says(_"Linarian. Now we part as usual. However should I ever see you after the war is over...", "NO_WAIT")
			npc_says(_"I promise to kill you.")
			npc_says(_"Now take your money and get out of my face. I do not want to see you here ever again.")
			Tux:add_gold(400)
			Tux:del_item_backpack("Dixon's Toolbox", 1)
			Tux:end_quest(_"The yellow toolkit", _"I sold the toolkit to Dixon for a enormous sum of money. Life is truly grand. But, I better stay away from Dixon for now... He seemed very angry at me.")
			hide("node47")
			end_dialog()
		end,
	},
	{
		id = "node51",
		text = _"Is everything all right?",
		code = function()
			npc_says(_"Heh, never better!", "NO_WAIT")
			npc_says(_"All indicators are in the green, the power and water fill the pipes and even Spencer seems happier now that the town is thriving despite all odds.")
			npc_says(_"Thanks to the autotools we can start building instead of just trying to repair the damage.", "NO_WAIT")
			npc_says(_"Once this sick war is over, I will make sure you get the 'Key to the City' Linarian.")
			hide("node51")
		end,
	},
	{
		id = "node53",
		text = _"Is everything all right?",
		code = function()
			npc_says(_"Nearly. Only my conscience keeps me up at night.", "NO_WAIT")
			if (Dixon_Singularity_peace) then
				npc_says(_"I'm kind of disenchanted by the failed peace mission.")
				npc_says(_"Please leave me alone with my thoughts, I do not want to talk to you right now.")
			else
				npc_says(_"Please leave me alone with my thoughts, I do not want to talk to anyone right now.")
			end
			if (Dixon_Singularity_war) then
				npc_says(_"But due to your forceful line of action, I am sure Spencer and Butch will be delighted to speak with you.")
			end
			hide("node53")
		end,
	},
	{
		id = "node55",
		text = _"Is everything all right?",
		code = function()
			npc_says(_"No, I am afraid not.", "NO_WAIT")
			npc_says(_"While I do not regret sparing the bots in the tunnels, the town suffers for it.")
			npc_says(_"It makes me sad to see everything slowly wasting away. The Megasys bots are doing more damage than we can repair.", "NO_WAIT")
			npc_says(_"This town has a few weeks left to live. After that we are all dead.")
			Dixon_everything_alright = true
			hide("node55")
		end,
	},
	{
		id = "node56",
		text = _"Is everything all right?",
		code = function()
			npc_says(_"Ha! Never better!", "NO_WAIT")
			npc_says(_"We are mass producing energy shields, creating more electric energy than we can ever imagine spending and even experimenting with new armor types.")
			npc_says(_"Life is good right now.")
			hide("node56") show("node57")
		end,
	},
	{
		id = "node57",
		text = _"What happened? Last time I asked, you said things are not going that well.",
		code = function()
			npc_says(_"Yes, but now everything is different.", "NO_WAIT")
			npc_says(_"I met someone who decided to join us in the fight against the MegaSys bots.")
			npc_says(_"You will need a hacked computer. Type in 'ssh 10.83.13.230' as the superuser, and you will see what I mean.")
			hide("node57") show("node58")
		end,
	},
	{
		id = "node58",
		text = _"Why the secrecy and the hushed voice? Can't you just tell me?",
		code = function()
			npc_says(_"Nope. Some things you just have to see with your own eyes.", "NO_WAIT")
			npc_says(_"The Library of Alexandria, the Colossus of Rhodes, the Black Island...")
			npc_says(_"And this is just one of those things.", "NO_WAIT")
			npc_says(_"Just go and see for yourself. Otherwise you will not believe me.")
			Tux:end_quest(_"The yellow toolkit", _"The tunnels bots seem to be working together with Dixon in keeping the town working. All is well that ends well.")
			Dixon_hide_node_56 = true
			hide("node58")
		end,
	},
	{
		id = "node60",
		text = _"I have a problem with the Automated Factory.",
		echo_text = false,
		code = function()
			if (Dixon_mood > 50) then
				npc_says(_"You have a problem... really amazing.")
				npc_says(_"There is a really easy way to solve it.")
				npc_says(_"Read the FLIPPING MANUAL!")
				end_dialog()
			else
				npc_says(_"We haven't used it for a long time, so I am not surprised.")
				npc_says(_"What is the error code?")
				Tux:says(_"Erm... I don't recall it.")
				npc_says(_"So, I should guess?")
				npc_says(_"Hm... It could be the common error when the autofactory is restarting.", "NO_WAIT")
				npc_says(_"Its code should be 0x06660... 0x06661696... C, I think. It means 'fail' in hex.")
				npc_says(_"There is a really easy way to solve it. It's not in the official instructions, but I expect it to solve your problem.")
				npc_says(_"You just have to apply some elbow grease to the mechanism at the end of the line.")
				npc_says(_"You can ask Bender for it. He makes much elbow grease and should be able to give you a small can.")
				Bender_elbow_grease = true
			end
			hide("node60")
		end,
	},
	{
		id = "node70",
		text = _"Do you have a copy of Subatomic and Nuclear Science for Dummies, Volume IV?",
		code = function()
			npc_says(_"Unfortunately, I never added that one to my library.")
			Tux:says(_"Library... Of course! Thanks, Dixon!")
			npc_says(_"Uh... You're welcome, I guess.")
			Tux:update_quest(_"An Explosive Situation", _"I spoke to Dixon, who didn't have a copy of the book. He did give me an idea, though - I'll head for the library in town.")
			Dixon_296_book_examine_library = true
			hide("node70")
		end,
	},
	{
		id = "node80",
		text = _"How can I open the gate in the Maintenance Tunnel?",
		code = function()
			npc_says(_"You cannot.")
			npc_says(_"I locked it in order to keep us save from these bots down there.")
			npc_says(_"You never know what they're up to...")
			Maintenance_Terminal_accessgate_nope="official"
			hide("node80")
		end,
	},
	{
		id = "node99",
		text = _"I must go now.",
		code = function()
			end_dialog()
		end,
	},
}
