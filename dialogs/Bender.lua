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
		Tux:says(_"Hello! I'm new here.")
		npc_says(_"Hey, man! I'm Bender, the dead man of this town. And you?")
		set_bot_name("Bender")
		Tux:says(_"I'm %s. I'm fine, thank you.", Tux:get_player_name(), "NO_WAIT")
		show("node0")
	end,

	EveryTime = function()
		if (Tux:has_met("Bender")) then -- to avoid displaying 2 "hello"s etc when we talk to Bender for the first time
			Tux:says(_"Hello!")
			npc_says(_"Hey, man!")
			Tux:says(_"How are you doing?")
			if (Tux:has_quest("Bender's problem")) then
				if (not Tux:done_quest("Bender's problem")) then
					npc_says(_"Too bad. I have never felt so sick.", "NO_WAIT")
					npc_says(_"Let me die, man!")

					if (Tux:has_item_backpack("Brain Enlargement Pills Antidote")) then
						hide("node11", "node12") show("node9")
					else
						hide("node9") show("node12")
					end

					hide("node1", "node2", "node3", "node4", "node5", "node6", "node7", "node8", "node10", "node50")
				elseif (not tux_has_joined_guard) then
					npc_says(_"Much better. I'm cured. You're a good guy, man!", "NO_WAIT")
					npc_says(_"What was your name again?")
					Tux:says(Tux:get_player_name())
					--; TRANSLATORS: %s = Tux:get_player_name()
					npc_says(_"Thanks %s, I owe you one.", Tux:get_player_name(), "NO_WAIT")
					npc_says(_"And I'll sure vote for you, if you want into the Red Guard!")
				elseif (not Bender_congrats) then
					npc_says(_"Much better. I'm cured. You're a good guy, man!", "NO_WAIT")
					npc_says(_"Congratulations on getting into the Red Guard!", "NO_WAIT")
					npc_says(_"I voted for you and that was what got you in!", "NO_WAIT")
					npc_says(_"I said we'd be buddies, didn't I? Want to stand guard at the gate with me? It gets boring with just that 614 to talk to.")
					npc_says(_"I could tell you all the secrets of the Red Guard.")
					Bender_congrats = true
					show("node30")
				else
					npc_says(_"Much better. I'm cured. You're a good guy, man!", "NO_WAIT")
				end
			else
				npc_says(_"Too bad. I have never felt so sick.", "NO_WAIT")
				npc_says(_"Let me die, man!")

				if (refused_to_help_bender) then
					show("node8")
				end
			end
		end

		if (Bender_gave_elbow_grease) then
			hide("node40")
		elseif (Bender_elbow_grease or Tux:has_item_backpack("Manual of the Automated Factory")) then
			show("node40")
		end

		if (Bender_at_Spencer) then
			Bender_at_Spencer = false
			set_bot_destination("BenderStartGameSquare")
		end
		show("node99")
	end,

	{
		id = "node0",
		text = _"What's wrong? You seem to be in pretty good shape for a dead man.",
		code = function()
			npc_says(_"Yeah. I'm the strongest man in all the world. That's 'cause I took ALL of the strength pills.")
			npc_says(_"And yet, I might be dead soon. The doc warned me, but I didn't listen. GRAH!")
			set_bot_name(_"Bender - The strongest one")
			show("node1", "node2")
			push_topic("Sick Bender")
		end,
	},
	{
		id = "node1",
		text = _"You took too many strength pills and now you're sick?",
		topic = "Sick Bender",
		code = function()
			npc_says(_"Nah. The strength pills were fine. They made me strong. The doc said so.")
			npc_says(_"It was the brain enlargement pills. They did me no good, man.")
			hide("node1", "node2") show("node3", "node10")
		end,
	},
	{
		id = "node2",
		text = _"What made you so sick?",
		topic = "Sick Bender",
		code = function()
			npc_says(_"It was the brain enlargement pills. They did me no good, man.")
			hide("node1", "node2") show("node3", "node10")
		end,
	},
	{
		id = "node3",
		text = _"Brain enlargement pills? Sounds ridiculous!",
		topic = "Sick Bender",
		code = function()
			npc_says(_"You know man, I got those offers everybody gets.")
			npc_says(_"They offer some brain enlargement pills to enhance brain performance.")
			npc_says(_"'Enlarg3 your brain! Bu.y pi11s! Che4p!!11!!!'")
			npc_says(_"And everybody in the town said I was dumb, which by the way IS NOT TRUE.")
			npc_says(_"So I thought these pills might make them think differently about me.")
			hide("node3") show("node4", "node5")
		end,
	},
	{
		id = "node4",
		text = _"But if you aren't dumb, why the brain enlargement pills?",
		topic = "Sick Bender",
		code = function()
			npc_says(_"Because they said I was dumb. So I had to do something about it, eh?")
			hide("node4")
		end,
	},
	{
		id = "node5",
		text = _"Isn't there some doctor in town who could cure you?",
		topic = "Sick Bender",
		code = function()
			npc_says(_"Yeah, the doc could help. But he won't. I've threatened him, but he won't.")
			npc_says(_"He's so angry cause I didn't listen. He warned me about the brain enlargement pill offers.")
			npc_says(_"They can cause some awful forms of cancer. But I didn't listen. And now he refuses to help me, man.")
			hide("node5") show("node6", "node7")
		end,
	},
	{
		id = "node6",
		text = _"Maybe I can help you somehow?",
		topic = "Sick Bender",
		code = function()
			npc_says(_"Man, if you could do that, I'd give you everything. I still got some of those strength pills left.")
			if (not tux_has_joined_guard) then
				npc_says(_"Also I could vote for you if you seek to join the guard of the town.")
			end
			npc_says(_"Just get me a cure, and I'll be forever grateful!")
			Tux:add_quest("Bender's problem", _"I met a Red Guardsman named Bender. He poisoned himself with some brain enlargement pills, which turned out to be highly carcinogenic. The town doctor will not give him the antidote, so it is up to me to save him.")
			refused_to_help_Bender = false
			hide("node0", "node6", "node7", "node8") show("node11")
			pop_topic()
		end,
	},
	{
		id = "node7",
		text = _"I'm sorry, but it looks like there's nothing I could do for you.",
		topic = "Sick Bender",
		code = function()
			npc_says(_"Of course not. You're not a doctor. Only the doctor has the right medicine.")
			npc_says(_"But this rat would rather let me die!")
			npc_says(_"I'll rip his guts out as soon as I get my hands on him, man!")
			refused_to_help_Bender = true
			hide("node0")
			pop_topic()
		end,
	},
	{
		id = "node8",
		text = _"Have you found a way to cure yourself?",
		code = function()
			npc_says(_"No, the doc still refuses. I've tried to fetch the medicine by force with little luck.")
			npc_says(_"No matter how hard I shake him, the rat has never accepted. He's so angry 'cause I didn't listen.")
			npc_says(_"Nobody else has the right medicine. I need a way to get it from him!")
			hide("node8")
			push_topic("Sick Bender")
		end,
	},
	{
		id = "node9",
		text = _"I've got your medicine. With best wishes from the doctor.",
		code = function()
			npc_says(_"Wow, you really did it! I guess you beat the hell out of him.")
			npc_says(_"He wouldn't help me even after I pounded him a bit. A big bit.")
			npc_says(_"Man, you are the greatest hero!")
			if (not tux_has_joined_guard) then
				npc_says(_"Here, take this as a reward - and you can be sure that I'll vote for you if you seek membership in the Red Guard!")
			else
				npc_says(_"Here, take this as a reward.")
			end
			npc_says(_"Thanks, man!")
			change_obstacle_state("TownDocBackdoor", "opened") -- Allows Bender to do something useful instead of just keeping watch outside the Docs after healed.
			Tux:add_xp(150)
			Tux:del_item_backpack("Brain Enlargement Pills Antidote", 1)
			Tux:add_item("Strength Pill", 2)
			Tux:end_quest("Bender's problem", _"Bender is fine now. He gave me some muscle enlargement pills in return for my help. Hmm...")
			set_bot_destination("NewSpencer")
			Bender_at_Spencer = true --have Bender go "talk" to Spencer
			hide("node9", "node10", "node50") show("node20")
		end,
	},
	{
		id = "node10",
		text = _"Maybe taking the brain enlargement pills really was stupid.",
		topic = "Sick Bender",
		code = function()
			npc_says(_"WHAT? Did you just say I'm stupid?")
			npc_says(_"Don't say it, man.")
			npc_says(_"I can kill you so much, that no one will recognize you!")
			hide("node10") show("node50")
		end,
	},
	{
		id = "node11",
		text = _"Where can I find this doctor?",
		code = function()
			npc_says(_"He's right inside this building. I'm waiting here for him, man.")
			npc_says(_"When he comes out, maybe I can pound him some more. Maybe then he'll help me at last!")
			npc_says(_"Yes, he'll get a good beating. That will make him reconsider! That rat!")
			hide("node11")
		end,
	},
	{
		id = "node12",
		text = _"About that medicine that you need...",
		code = function()
			npc_says(_"Did you get it?")
			npc_says(_"What? You didn't get it?")
			npc_says(_"Man, what are you doing here? Get the medicine! I need it now!")
			hide("node12")
		end,
	},
	{
		id = "node20",
		text = _"Did you take all the Brain Enlargement Pills?",
		code = function()
			npc_says(_"No. I have one left.")
			npc_says(_"Here, take it. I don't want it.")
			Tux:add_item("Brain Enlargement Pill", 1)
			hide("node20")
		end,
	},
	{
		id = "node30",
		text = _"What secrets? Like a secret handshake?",
		code = function()
			npc_says(_"Nah. But that is a totally awesome idea.")
			npc_says(_"Let's work it out -- just, uh, be sure not to let the Doc in on it.")
			hide("node30") show("node31")
		end,
	},
	{
		id = "node31",
		text = _"Do you actually know any *real* secrets?",
		code = function()
			npc_says(_"Yeah, sure.")
			npc_says(_"I know the shop keeper Lily's password.")
			npc_says(_"I sometimes login to her account and mess with it for the LULZ.")
			npc_says(_"It's the end of the world as we know it, and I... am so incredibly bored.")
			Tux:says(_"Erm... excuse me?")
			npc_says(_"Just a song my mother used to sing to me.")
			hide("node31") show("node32", "node33")
		end,
	},
	{
		id = "node32",
		text = _"Hey, could you tell me Lily's password?",
		code = function()
			npc_says(_"Sure, but we will have to shake on it.")
			Tux:says(_"OK...")
			npc_says(_"It is an asterisk repeated four times.")
			know_lily_password = true
			Tux:says(_"Wow.")
			npc_says(_"I am the most awesome hacker ever.")
			Tux:says(_"Sure...")
			hide("node32")
		end,
	},
	{
		id = "node33",
		text = _"What are you doing now?",
		code = function()
			npc_says(_"I'm killing time. Guard work is very boring.")
		end,
	},
	{
		id = "node40",
		text = _"Can you spare some elbow grease?",
		code = function()
			npc_says(_"Yes, you are talking to the right guy, man.")
			if (not Tux:done_quest("Bender's problem")) then
				npc_says(_"But now, I'm no good. I can't make you any elbow grease, sorry man.")
				npc_says(_"Maybe if I was feeling better...")
			else
				npc_says(_"Elbow grease just takes a bit of hard work. I'm not a fan of hard work, so I keep a can with me at all times.")
				npc_says(_"But since you helped me out with the Doctor, I'll give it to you.")
				Bender_gave_elbow_grease = true
				Tux:add_item("Elbow Grease Can", 1)
				Tux:says(_"Thanks man!")
			end
			hide("node40")
		end,
	},
	{
		id = "node50",
		text = _"Yeah, it was a totally idiotic idea to take those pills.",
		topic = "Sick Bender",
		code = function()
			npc_says(_"What? I'm not an idiot! Understood?")
			npc_says(_"No one says I'm stupid! I'm NOT STUPID!")
			npc_says(_"I'LL BEAT YOU TO SPLINTERS!")
			npc_says(_"I WILL KILL YOU SO HARD, YOU WILL DIE TO DEATH YOU FAT DUCK!")
			npc_faction("crazy", _"Bender - Homicidal")
			end_dialog()
			hide("node50")
		end,
	},
	{
		id = "node99",
		text = _"See you later.",
		code = function()
			npc_says_random(_"Later, man!",
				_"Stay cool dude.")
			end_dialog()
		end,
	},
}
