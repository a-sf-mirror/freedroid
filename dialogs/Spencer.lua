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
	FirstTime = function()
		show("node0")
		guard_follow_tux = false
		set_bot_state("patrol", "Town-TuxGuard")
		change_obstacle_state("Dixon-autogun", "enabled")
	end,

	EveryTime = function()
		if (MO_HFGateAccessServer_Spencer) then
			npc_says(_"Red Guard HQ, Spencer speaking.")
			npc_says(_"Are you receiving me?")
			tux_says(_"Yes.")
			tux_says(_"Spencer, we have a problem.")
			npc_says(_"Yes, Richard detected abnormalities with the server. Can you report anything regarding this?")
			tux_says(_"Indeed, the server says it is a gate server, not a firmware server.")
			npc_says(_"Bah!")
			npc_says(_"Must prevent access to the real update server. Can you open the gate?")
			npc_says(_"Inside, follow the mrkers o e groun")
			npc_says(_"Ri ard sa... e c nn cti n *bzzzzzzzzz* bad.")
			npc_says(_"Goo*sizzle*ck.")
			npc_says(_"O*crack*r")
			update_quest(_"Propagating a faulty firmware update", _"Spencer contacted me and said I was supposed to find the real firmware update server. I hope I survive this...")
			add_quest(_"Open Sesame", "It turns out what we thought was the firmware update server was just a gate access server. Spencer speculates the real firmware server is behind this gate. There should be something on the ground I am supposed to follow.")
			Spencer_can_die = true
			end_dialog()
		elseif (tux_has_joined_guard) then
			npc_says(_"Greetings fellow Red Guard member.")
			hide("node2", "node12")
		end

		if (Tania_met_Pendragon) and
		   (not Spencer_Tania_sent_to_DocMoore) and
		   (not Tania_set_free) then
			show("node50")
		end

		if (not has_quest("Opening access to MS Office")) and
		   (done_quest("A kingdom for a cluster!")) then
			show("node37")
		end

		if (not has_quest("Propagating a faulty firmware update")) and
		   (done_quest("Opening access to MS Office")) then
			show("node44")
		end

		if (data_cube_lost) and
		   (not done_quest("Deliverance")) then
			show("node29")
		end

		show("node99")
	end,

	{
		id = "node0",
		text = _"Hi! I'm new here.",
		code = function()
			npc_says(_"I'm Spencer. I'm the leader of the Red Guard. Is there anything I can help you with?")
			knows_spencer_office = true
			if (has_quest("Deliverance")) and
			   (not done_quest("Deliverance")) then
				show("node20")
			end
			hide("node0") show("node1", "node7")
		end,
	},
	{
		id = "node1",
		text = _"I want to join the Red Guard.",
		code = function()
			npc_says(_"Hmm... Really? Well, you cannot join just like that, you know...", "NO_WAIT")
			npc_says(_"You must prove that you would make a good new member.")
			hide("node1") show("node2")
		end,
	},
	{
		id = "node2",
		text = _"I really want to become a member.",
		code = function()
			npc_says(_"This is not so easy. First you must establish a reputation around here. Ask around, talk to people and build your reputation.")
			npc_says(_"Once you're a known character around here, we might let you join the ranks of the Guard.")
			hide("node2") show("node3", "node12")
		end,
	},
	{
		id = "node3",
		text = _"How about some circuits instead?",
		code = function()
			npc_says(_"For 15 million circuits, no less, hehe.")
			if (get_gold() >= 15000000) then -- player cheated obviously :-)
				show("node9")
			end
			hide("node3", "node7", "node12") show("node4")
		end,
	},
	{
		id = "node4",
		text = _"But I don't have that many circuits!",
		code = function()
			npc_says(_"That much I can tell.")
			npc_says(_"Look, I'm just fooling around. We don't let just anyone join the Red Guard.", "NO_WAIT")
			npc_says(_"If you seriously want to join, you have to prove yourself first. You might want to ask around town for things you can do.")
			hide("node4", "node9") show("node7", "node12")
		end,
	},
	{
		id = "node6",
		text = _"Maybe I could help somehow?",
		code = function()
			npc_says(_"That would be most kind of you, but I doubt that you will be able to clear out the warehouse for us.")
			npc_says(_"But since you are said to be powerful and a former hero, I'll once more put trust into a stranger.")
			npc_says(_"I've unlocked the access-way to the warehouse. It's to the north of this town, somewhat hidden in the woods northeast.")
			npc_says(_"The stuff we need is on the first floor. Don't go any deeper, there are only bots in there.", "NO_WAIT")
			npc_says(_"I wish you the best of luck.")
			add_quest(_"Opening a can of bots...", _"I am supposed to clean out the first level of some warehouse. Sounds easy. It lies nearby, somewhat hidden in the woods north-east of town.")
			change_obstacle_state("TrapdoorToWarehouse", "opened")
			hide("node6")
		end,
	},
	{
		id = "node7",
		text = _"How is it going?",
		code = function()
			if (has_quest("Opening a can of bots...")) then
				if (done_quest("Opening a can of bots...")) then
					if (Spencer_reward_for_warehouse_given) then
						npc_says(_"Thanks to you, we've been able to transport all the goods we need right now. You've really helped us out there.")
						npc_says(_"Rest assured that we will never forget your brave activity for our community.")
					else
						npc_says(_"Man, you really did it! I can hardly believe it, but all the bots are gone!", "NO_WAIT")
						npc_says(_"Take these 500 circuits as a reward. And be assured that you've earned my deepest respect, Linarian.")
						npc_says(_"Our people are transporting the goods as we speak. It can't be too long until new bots from ships in the orbit of the planet will beam down to replace the dead bots.")
						add_gold(500)
						Spencer_reward_for_warehouse_given = true
						update_quest(_"Opening a can of bots...", _"Ouch. It wasn't. At least I am alive, and the warehouse is clear. *Whew*.")
					end
				else
					npc_says(_"Not too good. Without the supplies from the warehouse we are doomed. So my problems are still the same.")
					npc_says(_"Maybe later, when you grow more experienced you might be able to help us after all.")
				end
			else
				npc_says(_"These are bad times. Interplanetary travel is made impossible by bot ships, so we need to stick to our local resources.")
				npc_says(_"We've got a list of stuff we need from the automated underground storage north of town. But the bots there are numerous.")
				npc_says(_"And currently I can't spare a single man from the town's defenses. It's quite a difficult situation.")
				show("node6")
			end
		end,
	},
	{
		id = "node9",
		text = _"I actually do have the 15 million bucks, here take it!",
		code = function()
			npc_says(_"Wow, err, I mean, thanks.")
			npc_says(_"You are a Red Guard now.")
			npc_says(_"Oh, wait, you cheated, didn't you?")
			npc_says(_"There is now way we can have lame cheaters in the the Red Guard, forget about it!")
			npc_says(_"However, if you did in fact NOT cheat, please tell the developers how you got so much money so they can fix it. :)")
			npc_says(_"Contact information can be found at http://www.freedroid.org/Contact")
			hide("node9")
			del_gold(1000000)
		end,
	},
	{
		id = "node12",
		text = _"Have I done enough quests to become a member now?",
		code = function()
			if (not done_quest("The yellow toolkit")) then
				npc_says(_"I think our teleporter service man, Dixon, has some problem. You might want to talk to him.")
			elseif (not Dixon_mood) or
			       (Dixon_mood < 50) then
				npc_says(_"Dixon told me about the matter with his toolkit. He seemed pretty impressed by you.")
			elseif (Dixon_mood < 120) then
				npc_says(_"Dixon has his toolkit back.", "NO_WAIT")
				npc_says(_"He was in bad mood and told me he had to pay you for getting his own property back.")
				npc_says(_"I think it is the best for him if we leave him alone the next days.", "NO_WAIT")
				npc_says(_"I'm sure he has to do a lot now.")
				-- Joining the guard, tux has to pay 500 circuits.
			elseif (Dixon_mood < 180) then
				npc_says(_"Dixon has his toolkit back.", "NO_WAIT")
				npc_says(_"He seemed quite aggressive and stressed.")
				npc_says(_"Poor guy, if we weren't in such a bad situation, he could have some work-free days, but bot attacks continue all the time.", "NO_WAIT")
				npc_says(_"You better don't bother him the next days.")
				-- Tux will have to pay 700 circuits to join the red guard
			else
				npc_says(_"Dixon has his toolkit back.", "NO_WAIT")
				npc_says(_"He did rail against you though. He said he had to give you 400 circuits to get his toolkit back, his own property.")
				npc_says(_"That was nearly all money he had. He said you were too greedy to become a good member and he doesn't want to see you anymore.")
				npc_says(_"I think everybody has to get a chance. But look ahead! When you are negatively conspicuous again, we will ban you from our town!")
				-- Tux needs to pay 800 circuits to join the guard.
			end

			if (done_quest("Anything but the army snacks, please!")) then
				npc_says(_"When I was eating, Michelangelo told me about his renewed oven energy supply. He seemed very pleased, and so was I.")
			else
				npc_says(_"I think you should visit the town's cook sometime. He's usually in the restaurant kitchen.")
			end

			if (done_quest("Novice Arena")) then
				npc_says(_"From Butch I hear you've become a novice arena master. Congratulations.")
			else
				npc_says(_"You might want to score some arena victories. That could also help your reputation a lot.")
			end

			if (done_quest("Bender's problem")) then
				npc_says(_"Helping Bender along was also a smart move. But you should be very careful with that one. He can get mad rather easily. A bit of a security threat, but we can't be too picky.")
			else
				npc_says(_"As far as I know, Bender is still very sick.")
			end

			if (done_quest("Opening a can of bots...")) then
				npc_says(_"But most importantly, I was very impressed with you when you cleared out the warehouse. That was a huge deed I will never forget.")
			else
				npc_says(_"Personally I'm also worrying about how we will manage to get some necessary supplies from our warehouse. It's filled with bots and we just don't have the manpower to spare to clean them out.")
				if (not has_quest("Opening a can of bots...")) then
					show("node6")
				end
			end

			if (get_town_score() > 49) then
				-- ENOUGH POINTS TO JOIN RG
				npc_says(_"OK. Your list of achievements is long enough. You can join us. So I hereby declare you a member of the Red Guard.")
				npc_says(_"But now that you are in the Guard, know this: There is only one rule for us guards: We stick together. We survive together or we die together. But we do it together.")
				npc_says(_"And now you might want to inspect the guard house. Tell Tybalt to open the door for you. Lukas at the arms counter will give you your armor.")
				npc_says(_"I hope you will prove yourself a worthy member of the Red Guard.")
				display_big_message(_"Joined Town Guard!!")
				tux_has_joined_guard = true
				change_obstacle_state("Main Gate Guardhouse", "opened")
				sell_item("Shotgun shells", 1, "Stone")
				sell_item(".22 LR Ammunition", 1, "Stone")
				hide("node3", "node12")
			else
				if (get_town_score() > 29) then
					npc_says(_"All in all, not quite so bad. But you still need to do the one thing or the other. Then we can talk about it.")
				else
					npc_says(_"I can't accept you into the guard like this. Get going. There are still many things to do for you.")
				end
			end
		end,
	},
	{
		id = "node20",
		text = _"Francis wanted me to give you a data cube.",
		code = function()
			npc_says(_"Ah, excellent, the list I asked for.")
			npc_says(_"It figures he would ask someone to deliver it for him.")
			tux_says(_"Why's that?")
			npc_says(_"We had a little disagreement, Francis and I. He refused to accept our rule and do the task we gave him.")
			npc_says(_"I had to persuade him myself.")
			npc_says(_"Well, give me the data cube.")
			if (has_item_backpack("Data cube")) then
				show("node21", "node22", "node25", "node26")
			else
				show("node23", "node25", "node26")
			end
			hide("node20")
			push_topic("Deliver the cube")
		end,
	},
	{
		id = "node21",
		text = _"(Give the data cube to Spencer)",
		topic = "Deliver the cube",
		code = function()
			tux_says(_"Here, take it.")
			del_item_backpack("Data cube", 1)
			npc_says(_"Thank you for the good work you have done. I think you deserve a small reward.")
			add_xp(100)
			add_gold(100)
			end_quest(_"Deliverance", _"I gave Spencer the data cube. He gave me a small reward.")
			data_cube_lost = false
			hide("node20", "node21", "node22", "node23", "node24", "node25", "node26")
			pop_topic("Deliver the cube")
		end,
	},
	{
		id = "node22",
		text = _"(Lie about the oversight of the data cube)",
		topic = "Deliver the cube",
		code = function()
			next("node23")
		end,
	},
	{
		id = "node23",
		text = _"(Apologize for the oversight of the data cube)",
		topic = "Deliver the cube",
		code = function()
			if (not data_cube_lost) then
				tux_says(_"Oh, erm... Hehe, I think I forgot it somewhere.")
			else
				tux_says(_"Hm, I still don't have the data cube.")
			end
			npc_says(_"... What?")
			npc_says(_"Then you better go and look for it. Don't waste my time, Linarian.")
			data_cube_lost = true
			hide("node21", "node22", "node23")
			pop_topic("Deliver the cube")
		end,
	},
	{
		id = "node24",
		text = _"(Lie about the loss of the data cube)",
		topic = "Deliver the cube",
		code = function()
			tux_says(_"I think I lost the data cube.")
			npc_says(_"Come on, you've got to be kidding! ...")
			npc_says(_"So I will call one minion for this job. You are very useless, unable to bring a small thing.")
			npc_says(_"Get out of my sight!")
			end_quest(_"Deliverance", _"I lied about the data cube and Spencer thinks now I lost the cube. I won a little time for people in cryonic stasis. But, I couldn't stop Spencer's project.")
			add_xp(250)
			hide("node21", "node22", "node23")
			end_dialog()
		end,
	},
	{
		id = "node25",
		text = _"How did you persuade Francis? Did you beat him up?",
		topic = "Deliver the cube",
		code = function()
			npc_says(_"No, nothing so violent. Let's just say I know more about Francis than he would like to remember.")
			npc_says(_"Francis is someone who is very understandable. I just had to find the right words.")
			npc_says(_"Anyway, we have nearly been close friends since.")
			hide("node25")
		end,
	},
	{
		id = "node26",
		text = _"Why didn't Francis want to do this task you gave him? What was it?",
		topic = "Deliver the cube",
		code = function()
			npc_says(_"We had an unfortunate misunderstanding, so we had a very hard time talking together.")
			npc_says(_"I wanted him to go through the people in cryonic freezing in the facility, and make a list of disposable ones and people unlikely to survive. This cube contains that list.")
			tux_says(_"Disposable people? Unlikely to survive? Can't they just stay in cryonics indefinitely?")
			npc_says(_"No. They take up a lot of space, and keeping them alive takes a lot of power, which is running out. Most of the people there are sick or dying anyway, which is why they're frozen in the first place. We can't afford to waste any resources.")
			npc_says(_"We even had to confiscate the town cook's macrowave oven battery, which means we can't eat warm food anymore. We needed it to keep the town's defenses up.")
			update_quest(_"Deliverance", _"I learn incredible information. Apparently the data cube stored a list of people in freezing in the cryonic facility. Spencer wants to dispose of some of them because keeping them alive uses up the town's power...")
			hide("node26") show("node24")
		end,
	},
	{
		id = "node29",
		text = _"I would like to talk about the Francis' cube.",
		code = function()
			npc_says(_"Well, I'm listening to you. But you must be quickly, I've no time to loose.")
			npc_says(_"If you found the data cube, just give it.")
			if (has_item_backpack("Data cube")) then
				show("node21", "node22")
			else
				show("node23")
			end
			hide("node29")
			push_topic("Deliver the cube")
		end,
	},
	{
		id = "node37",
		text = _"I've heard Richard obtained new information on the town.",
		code = function()
			npc_says(_"Yes, that's right, and in fact it might be crucial. As you may have heard, the MS office is defended by a disruptor shield. They open it only to let out new armies of bots.")
			npc_says(_"The data on the cube he obtained indicates the existence of a secret experimental facility in this region. Our findings suggest that they were testing some new form of disruptor shield for MS, so the shield can be controlled from that facility.")
			npc_says(_"If the information is true, then you can take over the control droid and disable the shield permanently so we can get in.")
			npc_says(_"We know that MS had a firmware update system, which could be used to propagate a malicious update to disable all bots. It is very alluring, but to perform this trick you need to hack the control droid, which is in the heart of the HF. To enter HF you would have to disable the disruptor shield.")
			npc_says(_"This seems like a gift sent from the heavens. Cleaning the Hell Fortress is not going to be easy, though..")
			hide("node37") show("node38")
		end,
	},
	{
		id = "node38",
		text = _"I'd like to participate in this operation.",
		code = function()
			npc_says(_"That's most kind of you to volunteer. So far, I've sent two scouts into the area. They have found the facility entrance and unlocked the gate.")
			npc_says(_"That is a good sign, because it shows that the key combinations from the data cube were correct.")
			npc_says(_"However, we lost contact with them shortly after they went inside. They also reported heavy bot resistance.")
			npc_says(_"I'd be glad if you could take a look. But use the utmost care. We can't afford to lose another guard.")
			hide("node38") show("node39", "node40")
		end,
	},
	{
		id = "node39",
		text = _"OK. I'll be careful. But I'll do it.",
		code = function()
			npc_says(_"Good. The base entrance is somewhat hidden in the caves to the northeast. Best to use the north gate out of town, then head east, and turn north again along the shore.")
			npc_says(_"I wish you the best of luck for this operation. It might be that our survival depends on it. Don't wait for assistance.")
			npc_says(_"Try to get control over the disruptor shield if you can. The control droid should be somewhere on the lowest level of the installation. Simply destroying the droid might suffice in disabling the shield.")
			add_quest(_"Opening access to MS Office", _"Spencer has revealed the information from the data cube evaluation to me. It seems there is an old military research facility north of the town. By taking over the control droid, I should be able to control the disruptor shield at the facility. I could activate it permanently, thereby locking off the Hell Fortress, and the bots it's producing, forever. Alternatively, I could disable it, and fight my way through the Hell Fortress droids until I reach the main control droid and update it, thereby disabling all bots in the entire area around town in one fell swoop.")
			change_obstacle_state("DisruptorShieldBaseGate", "opened")
			hide("node39", "node40")
		end,
	},
	{
		id = "node40",
		text = _"I don't feel like doing it now. I'd rather prepare some more.",
		code = function()
			npc_says(_"Good. You should be well prepared if you intend to go.")
			npc_says(_"Also there is no need to hurry with this. After all, the installation is not running away, so it's best to take a cautious approach.")
			hide("node40")
		end,
	},
	{
		id = "node44",
		text = _"It's done. Your soldiers were killed, but I managed to reach a computer terminal that controls the shield. Access to the bot factory is now open, after I changed the password on the terminal so as to prevent the bots from enabling the shield again.",
		code = function()
			npc_says(_"Good. We cannot help you much in this final mission, but I can tell you what our recon teams gathered behind the factory doors. You will enter a zone that used to be a MS office.")
			npc_says(_"They carried out some development there, and had part of their patching division and update management department. The actual factory is located behind the office.")
			npc_says(_"With a bit of luck, you might not need to access it. We know they have their update server in the office.")
			npc_says(_"If you can find it and get it to propagate a faulty update, this could suffice to stopping bots dead in their tracks.")
			npc_says(_"Look for the entrance of the office in the crystal fields. I will send a message to the guards so they let you pass. Then you will be on your own.", "NO_WAIT")
			npc_says(_"However I'll ask Richard to see if we can contact you as soon as you find the server so we know if you're alive and there is still hope, or if things are going to go back to the way they were before you were taken out of stasis sleep...")
			npc_says(_"Good luck.")
			add_quest(_"Propagating a faulty firmware update", _"I can now enter the fortress and find the upgrade server terminal. The fortress gates are in the Crystal Fields. Spencer told the guards to open the doors for me. He said he'd probably contact me when I found the server.")
			hide("node44") show("node45")
		end,
	},
	{
		id = "node45",
		text = _"I will need some time to get myself ready before I clean up Hell Fortress.",
		code = function()
			npc_says(_"You better be ready.")
		end,
	},
	{
		id = "node50",
		text = _"I found someone out in the desert.",
		code = function()
			npc_says(_"Great, another mouth to feed.")
			npc_says(_"What is this person's name?")
			hide("node50") show("node51")
		end,
	},
	{
		id = "node51",
		text = _"Tania",
		code = function()
			if (tux_has_joined_guard) then
				npc_says(_"Well, since you are a guard member, I'll let you vouch for this Tania person.")
			else
				npc_says(_"Well, we have enough food for now. I'll let this Tania person in.")
			end

			if (npc_dead("DocMoore")) then
				npc_says(_"I'd say you should take her straight away to Doc Moore, but he was found dead earlier.")
				npc_says(_"You wouldn't happen to know anything about that, would you?")
				if (killed_docmoore) then
					show("node53")
				end
				show("node52", "node54")
			else
				npc_says(_"You must take her straight away to Doc Moore. We can't have a disease breaking out.")
				Spencer_Tania_sent_to_DocMoore = true
				update_quest(_"Tania's Escape", _"Spencer said it was okay for Tania to enter the town, as long as she goes to see Doc Moore first thing. Now all I have to do is tell her and Pendragon.")
			end
			hide("node51")
		end,
	},
	{
		id = "node52",
		text = _"No, of course not.",
		code = function()
			npc_says(_"Good.")
			npc_says(_"I didn't think it was you, but you never know.")
			npc_says(_"About your friend, she can come in provided that she pulls her weight around here.")
			Tania_set_free = true
			if (killed_docmoore) then
				update_quest(_"Tania's Escape", _"When I asked about Tania entering the town, Spencer confronted me about Doc Moore's death. I denied everything and he bought it! He says it is OK for Tania to enter the town: I should tell her and Pendragon.")
			else
				update_quest(_"Tania's Escape", _"When I asked about Tania entering the town, Spencer said Doc Moore was found dead! Oh, and Tania can enter the town. I should go tell her that.")
			end
			hide("node52", "node53", "node54")
		end,
	},
	{
		id = "node53",
		text = _"He and I had a disagreement, which we settled.",
		code = function()
			npc_says(_"I am the law here. If you have a problem, you come to me.")
			if (tux_has_joined_guard) then
				npc_says(_"I'm going to strip you of your membership in the Red Guard.")
				tux_has_joined_guard = false
				change_obstacle_state("Main Gate Guardhouse", "closed")
				npc_says(_"Your friend can come in, but we will be watching the two of you closely.")
				Tania_set_free = true
				update_quest(_"Tania's Escape", _"When I asked about Tania entering the town, Spencer confronted me about Doc Moore's death. I told him the truth, and he kicked me out of the Red Guard. But he let Tania in, Now all I have to do is tell her and Pendragon.")
			else
				npc_says(_"As the law, I pronounce you GUILTY of MURDER.")
				npc_says(_"The punishment is death.")
				update_quest(_"Tania's Escape", _"When I asked about Tania entering the town, Spencer confronted me about Doc Moore's death. He found me guilty of murder, and sentenced me to death.")
				set_faction_state("redguard", "hostile")
				end_dialog()
			end
			hide("node52", "node53", "node54")
		end,
	},
	{
		id = "node54",
		text = _"I killed him just to see what it was like. It was awesome.",
		code = function()
			npc_says(_"You are a sociopath, and a danger to us all!")
			npc_says(_"We must stop you before you kill again.")
			update_quest(_"Tania's Escape", _"When I asked about Tania entering the town, Spencer confronted me about Doc Moore's death. He found me too dangerous to live.")
			set_faction_state("redguard", "hostile")
			hide("node52", "node53", "node54")
			end_dialog()
		end,
	},
	{
		id = "node99",
		text = _"I'll be going then.",
		code = function()
			npc_says(_"See you later.")
			end_dialog()
		end,
	},
}
