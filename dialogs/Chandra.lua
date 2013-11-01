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
		npc_says(_"So the rumors are true - a real live Linarian walks among us!")
		Chandra_bot_information_nodes = 0
		show("node0")
	end,

	EveryTime = function()
		if (not has_met("Chandra")) then
			npc_says(_"Good to see you again. How can I help you?")
		end

		if (Chandra_node10_show) and
		(not tux_has_joined_guard) then
			show("node10")
		end

		if (Chandra_node16_show) then
			show("node16")
		end

		if (Chandra_bot_information_nodes > 1) then
			show("node17")
		end

		show("node99")
	end,

	{
		id = "node0",
		text = _"Hi! I'm new here.",
		code = function()
			npc_says(_"Welcome. I am Chandra. Some would call me the local sage.")
			if (linarian_chandra) and
			(not Chandra_first_contact) then
				show("node15")
			end
			hide("node0") show("node1", "node2", "node9")
		end,
	},
	{
		id = "node1",
		text = _"What can you tell me about this place?",
		code = function()
			npc_says(_"This small town used to be a somewhat successful mining community, exporting rare earth materials and other resources to the rest of the solar system.")
			npc_says(_"The mines have depleted recently, however. The town was under a threat of bankruptcy. Living here became harder and many left to seek new opportunities elsewhere.")
			npc_says(_"But now after the Great Assault, the rest of the planet gets bombarded by automated bot ships every now and then, so being here isn't so bad at all.")
			npc_says(_"Even with the Red Guard in charge.")
			hide("node1") show("node5", "node10", "node14")
		end,
	},
	{
		id = "node2",
		text = _"Where can I get better equipment?",
		code = function()
			npc_says(_"The shop is to the north, near the gate. Ms. Stone always has a good range of equipment there, with fair prices too.")
			npc_says(_"Not that you have much choice.")
			npc_says(_"Of course, the Red Guard always has the biggest guns and the best armor, but they do not sell them to non-members.")
			hide("node2")
		end,
	},
	{
		id = "node3",
		text = _"So what's so strange about the attack?",
		code = function()
			npc_says(_"According to my calculations based on the Fermi estimate, there are about seven million bots active in the region, everything from cleaner bots and servant droids to much more dangerous ones.")
			npc_says(_"I think you will agree with me that the following statement is true: If bots, then death to everything alive.")
			npc_says(_"Now, I make the assumption that you and I are alive.")
			npc_says(_"But, if we are not dead, and if bots are killing everything alive...")
			npc_says(_"... THEN WE HAVE A CONTRADICTION!")
			tux_says(_"Calm down. There must be some logic behind this.")
			npc_says(_"There can be none!")
			npc_says(_"They could just enter the town and massacre everyone...")
			npc_says(_"But they do not even seem to try!")
			npc_says(_"This makes no sense at all!")
			hide("node3")
		end,
	},
	{
		id = "node5",
		text = _"What do you know about the hostile bots?",
		code = function()
			npc_says(_"The phenomenon of hostile bots has occupied my mind for quite some time. There are some strange irrationalities in this behavior, that I cannot explain.")
			npc_says(_"This encompasses not only the suddenness of the attack, but also the methods and the scale of it all.")
			hide("node5") show("node3", "node6", "node7")
		end,
	},
	{
		id = "node6",
		text = _"What types of bots are out there?",
		code = function()
			npc_says(_"There are many. Each kind of bot has a number consisting of three digits. The first one is the most significant, the class of a bot.")
			npc_says(_"I know a bit about three classes.")
			npc_says(_"The 100s are cleaning bots of old. Therefore, they are quite common. Fortunately for us they aren't as good at killing things as they are at cleaning up the mess afterwards.")
			npc_says(_"The 200s are servant bots, designed to come quickly when summoned and be able to lift heavy weights. That makes them more dangerous under most circumstances, so beware of being attacked by more than one at the same time.")
			npc_says(_"Right now, an encounter with a 300 type bot equals death, so try to avoid it. They were messenger bots before the Great Assault, which means they have superb visual sensors, and some are very fast.")
			npc_says(_"There are many more classes out there. The military used bots, of course, and they were used on ships, but I know nothing about those types.")
			Chandra_bot_information_nodes = 4
			hide("node6") show("node11", "node12", "node13", "node18")
			push_topic("Bot Information")
		end,
	},
	{
		id = "node7",
		text = _"I want to leave this place.",
		code = function()
			npc_says(_"We all do, Linarian, we all do.")
			npc_says(_"The act in question is made impossible by the quasi-infinite bots outside of the town walls.")
			npc_says(_"You cannot live for long out there.")
			hide("node7") show("node8")
		end,
	},
	{
		id = "node8",
		text = _"No, I meant I would like to leave this world.",
		code = function()
			npc_says(_"Simple. Walk outside the town walls, and the bots will send you to the other world within a few minutes.")
			npc_says(_"Unless you mean leave the planet, in which case that is impossible.")
			npc_says(_"Most likely all of our cosmodromes have been destroyed in the bot attacks.")
			hide("node8")
		end,
	},
	{
		id = "node9",
		text = _"I'm having difficulty overcoming the bots. What can I do?",
		code = function()
			npc_says(_"There are many things you can do. The first is to fight more bots.")
			tux_says(_"Er... Wouldn't that cause me to be beaten up more, and therefore be counter-productive?")
			npc_says(_"Your logic is infallible, Linarian, but you are forgetting one thing: the more you fight, the more experienced you become, and the better you become at surviving.")
			npc_says(_"Also, if you have the money and can find a supplier, it is wise to invest in better equipment - more protective clothing and better weaponry.")
			npc_says(_"My advice is to invest in a ranged weapon. While it can create a huge hole in your wallet, it also creates several in the bots, and is better than a huge hole in you.")
			npc_says(_"Currently there is a ban on selling such things. The Red Guard wants to be the only ones with good weapons.")
			npc_says(_"Should you fail to find something decent to shoot or swing with, there are always other options.")
			npc_says(_"Ewald, the barkeeper, usually sells an assortment of junk, but even he sometimes has something worthwhile on display.")
			npc_says(_"There was a person selling grenades, too. Eh... I forgot his name, my memory is not serving me very well today. He's a very mysterious fellow. Perhaps he can help you.")
			npc_says(_"Other than that, you are on your own.")
			hide("node9")
		end,
	},
	{
		id = "node10",
		text = _"Do you think I could join the guardians of this town?",
		code = function()
			if (guard_follow_tux) then
				npc_says(_"The Red Guard...? Only if you're as strong and courageous as they are! There is no one better than the Red Guard! Nothing but top quality men there!")
			else
				npc_says(_"The Red Guard? Unlikely. You'd have to establish a reputation beforehand, and demonstrate that you can make things happen.")
				npc_says(_"Also, the Red Guard is not a very popular organization, so if you do join them then you can expect to be hated by nearly everyone here.")
				npc_says(_"As the ancients said, 'Boni pastoris est tondere pecus, non deglubere'. A good tax must be a reasonable tax.")
			end
			hide("node10")
			Chandra_node10_show = true
		end,
	},
	{
		id = "node11",
		text = _"What bots are in the 100 class?",
		topic = "Bot Information",
		code = function()
			npc_says(_"I know of two types, the 123 and the 139. The former is a simple cleaning bot, and is fairly weak. The latter is a mobile trash compactor. Don't get your flippers get caught in one of those.")
			tux_says(_"I've seen some of those on my way here. They were... Shooting at me.")
			npc_says(_"Many strange things have happened since the Great Assault.")
			npc_says(_"Anyway, because the class 100 bots are neither fast nor dangerous, the Red Guard often uses them for target practice.")
			npc_says(_"If I were you, I would not overestimate them.")
			npc_says(_"Nec Hercules contra plures.")
			Chandra_bot_information_nodes = Chandra_bot_information_nodes - 1
			hide("node11")
		end,
	},
	{
		id = "node12",
		text = _"What bots are in the 200 class?",
		topic = "Bot Information",
		code = function()
			npc_says(_"In fitting with the lazy nature of humans, there are several servant bots of the 200 class.")
			npc_says(_"The 247 is often called the 'Banshee'. Being a simple servant robot, it isn't very well equipped to kill, although its arms are quite strong.")
			npc_says(_"Also, as I said, the 200s were built to report quickly. The 247 is no exception, and moves faster than some of us can run. As a killer bot, it has a fast rate of attack. It is not something to be trifled with.")
			npc_says(_"The 249 is a cheaper version of the Banshee. It uses a tripedal drive instead of anti-gravity propulsion, and is therefore slower. However it is much more dangerous: its machine gun has a very high rate of fire.")
			tux_says(_"Machine gun?! But... But it's a servant bot!")
			npc_says(_"I know, %s. I know.", get_player_name())
			npc_says(_"Let's see... Oh, there is also the 296, which was used for serving drinks. Ewald once had one of those in his bar.")
			tux_says(_"Once? What happened to it?")
			npc_says(_"No one knows. One day it simply wasn't there. Ewald probably remembers more than I do, though.")
			Chandra_bot_information_nodes = Chandra_bot_information_nodes - 1
			hide("node12")
		end,
	},
	{
		id = "node13",
		text = _"What other bots are out there?",
		topic = "Bot Information",
		code = function()
			npc_says(_"Though I do not know the details, I assure you there are many. Robots and automated machinery served almost every purpose you can imagine, and probably some that you can't.")
			npc_says(_"They were used by the military, aboard ships, as security droids and more. Right now, however, most of the people who knew them up close are pushing up daisies.")
			Chandra_bot_information_nodes = Chandra_bot_information_nodes - 1
			hide("node13")
		end,
	},
	{
		id = "node14",
		text = _"Where in the void is this world?",
		code = function()
			npc_says(_"I guess you mean in the universe?")
			tux_says(_"Yes, I think that is your name for it.")
			npc_says(_"Well, we are in a barred spiral galaxy, twenty-six thousand light years away from the galactic core.")
			npc_says(_"The galactic radius is 43,000 light years and the circumference is estimated at 270,000 light years.")
			npc_says(_"This is a planetary system of ten planets, and we are presently located on the third one from our star, Sol.")
			npc_says(_"I am not an astronomer, so I could be wrong. I am too tired to keep track of what is a planet and what is just an oversized rock.")
			tux_says(_"This could be just about anywhere. Your description is too ambiguous.")
			npc_says(_"Sorry, star sciences are my weak point.")
			tux_says(_"...Right.")
			hide("node14")
		end,
	},
	{
		id = "node15",
		text = _"Chandra? Francis in the cryonic facility mentioned your name.",
		code = function()
			npc_says(_"Oh did he? What exactly did he say about me?")
			tux_says(_"He said you might know more about who I am, about Linarians.")
			npc_says(_"Hmmm... I'm not so sure about 'who', but he is quite right about my knowledge of Linarians... quite right indeed. What would you like to know, my friend?")
			if (Chandra_revenge) then
				end_dialog()
			end
			Chandra_node16_show = true
			hide("node15") show("node20", "node25", "node26", "node29")
			push_topic("Linarians")
		end,
	},
	{
		id = "node16",
		text = _"I wanted to ask you some questions about Linarians.",
		code = function()
			npc_says(_"Yes?")
			if (Chandra_revenge) then
				end_dialog()
			end
			hide("node16") show("node20")
			push_topic("Linarians")
		end,
	},
	{
		id = "node17",
		text = _"I wanted to ask you some questions about droids.",
		code = function()
			npc_says(_"Yes?")
			hide("node17")
			push_topic("Bot Information")
		end,
	},
	{
		id = "node18",
		text = _"That's all I wanted to know about droids.",
		topic = "Bot Information",
		code = function()
			npc_says(_"Did you have any other questions?")
			pop_topic()
		end,
	},
	{
		id = "node20",
		text = _"What is a Linarian?",
		topic = "Linarians",
		code = function()
			if (not Chandra_first_contact) then
				npc_says(_"Ah yes, I heard that your memory had been affected by stasis. I will share with you what I know.")
				npc_says(_"Linarians came to this planet about a century ago. While their specialties can vary, they are most widely known for their nearly-magical skills with computers.")

				if (not guard_follow_tux) then
					npc_says(_"[b](Chandra quickly glances around. After verifying that no one else seems to be paying attention to your conversation, he takes a quick breath and continues quietly.)[/b]")
					npc_says(_"Since you are asking this question and you have even come into our town, I am assuming that your memory has been completely damaged.")
					npc_says(_"What I am about to tell you is not easy to say, nor will it be easy to hear. For that I apologize, but you must know the truth.")
					npc_says(_"[b](He pauses for a breath.)[/b]")
					npc_says(_"When your people first arrived, there was a miscommunication that resulted in the leaders of Earth acting aggressively. Unfortunately, most of your people did not survive...")
					npc_says(_"For that my friend, I am deeply sorry.")
					npc_says(_"[b](He hangs his head slightly and you feel he is sincere in his remorse.)[/b]")
					show("node21", "node22")
					push_topic("Linarians Attacked")
				else
					npc_says(_"[b](Chandra moves his head ever so slightly as he quickly glances around. After noticing the nearby presence of the Red Guard member, his posture and tone changes. Perhaps you should speak with him again once you lose your escort...)[/b]")
					npc_says(_"What else would you like to know?")
				end
			else
				npc_says(_"Linarians came to this planet about a century ago. While their specialties can vary, they are most widely known for their nearly-magical skills with computers.")
				npc_says(_"I'd rather not repeat what I said earlier. It pains me to think of it...")
			end
			hide("node20")
		end,
	},
	{
		id = "node21",
		text = _"(Calmly accept this information)",
		topic = "Linarians Attacked",
		code = function()
			tux_says(_"What happened to the other Linarians that survived first contact?")
			npc_says(_"Sadly, I do not know. All that remains are legends. For a time, Linarians wandered the planet performing grand deeds and earning the respect of most of humanity.")
			npc_says(_"But as time passed, so did the memories of the great deeds. It is unlikely that you will find many who know more about your people's noble past.")
			npc_says(_"Perhaps there are still other Linarians alive here on our planet. Or maybe some went back to your own planet.")
			Chandra_first_contact = true
			hide("node20")
			pop_topic()
		end,
	},
	{
		id = "node22",
		text = _"(Respond angrily)",
		topic = "Linarians Attacked",
		code = function()
			tux_says(_"Your people slaughtered us?!")
			npc_says(_"Yes, this is the truth and it is something most humans regret. It is this regret that has helped erase the Linarians from our history.")
			npc_says(_"Humanity does not have a pretty history. We have done many horrible things to each other as well. There are many things we regret so deeply that we wish we could forget.")
			npc_says(_"Over time, most people have willingly forgotten our actions and focused on the good deeds the Linarians performed despite our senseless behavior.")
			npc_says(_"Even though we tried to celebrate the grand deeds and heroics of the surviving Linarians, the memories faded. The memories turned into legends and myths as the Linarians slowly withdrew from the world.")
			npc_says(_"Perhaps there are still other Linarians alive here on our planet. Or maybe some went back to your own planet.")
			npc_says(_"Spencer and the Red Guard would not want you to know these things, as you might not be willing to help our failing community.")
			npc_says(_"However, to not share this information with you is another transgression that I cannot be a part of. Please %s, we need your help!", get_player_name())
			hide("node21", "node22") show("node23", "node24")
		end,
	},
	{
		id = "node23",
		text = _"Chandra, it saddens me to hear this. I will keep what you say in mind, but there is too much suffering for me to ignore.",
		topic = "Linarians Attacked",
		code = function()
			npc_says(_"Oh! This is great news! I was fearful that the truth would not settle well.")
			npc_says(_"I should have never doubted the nobility and legendary honor of Linarians.")
			Chandra_first_contact = true
			hide("node20")
			pop_topic()
		end,
	},
	{
		id = "node24",
		text = _"(Attack)",
		topic = "Linarians Attacked",
		code = function()
			tux_says(_"Stupid humans! How dare you?! Your crimes are unforgivable! You all deserve to be slaughtered by these robots!")
			npc_says(_"But...")
			tux_says(_"And I am more than happy to help them!")
			set_npc_faction("crazy")
			Chandra_revenge = true
			end_dialog()
		end,
	},
	{
		id = "node25",
		text = _"How come no one seems freaked out by a big fracking penguin?",
		topic = "Linarians",
		code = function()
			npc_says(_"[b]<chuckles>[/b] This is not the first time the people of Earth have seen one of your kind.")
			npc_says(_"Of course, its been almost one hundred years since Linarians first came to this planet. As you might imagine most of the people who were alive then are long gone.")
			npc_says(_"But the townspeople were made aware of your existence upon your discovery. Everyone was quickly informed of the legends performed by the Linarians of the past.")
			npc_says(_"While you will find some people helpful because of this, others may be a bit more hostile. The hostility is a matter of xenophobia and pride in one's own species.")
			if (not guard_follow_tux) then
				npc_says(_"Don't tell anyone else, but I'm surprised the Red Guard ordered you to be thawed. They are human supremacists to the extreme. For them to accept your help does not make much sense. They must be playing at something. Beware the Red Guard, Linarian.")
			end
			hide("node25")
		end,
	},
	{
		id = "node26",
		text = _"Why did the Linarians come to this world?",
		topic = "Linarians",
		code = function()
			npc_says(_"The planet you mean? No one knows that. None of you ever said anything about it.")
			npc_says(_"Sorry, I cannot help you with that.")
			hide("node26")
		end,
	},
	{
		id = "node29",
		text = _"That's all I wanted to ask about that.",
		topic = "Linarians",
		code = function()
			npc_says(_"Did you have any other questions?")
			pop_topic()
		end,
	},
	{
		id = "node99",
		text = _"Thank you for your wise words.",
		code = function()
			npc_says_random(_"Feel free to come here any time you want.",
				_"You flatter me.")
			end_dialog()
		end,
	},
}
