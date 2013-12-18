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
	end,

	EveryTime = function()
		if (has_quest("Novice Arena")) then
			if (not done_quest("Novice Arena")) then
				npc_says(_"I've already opened the door for you. All you need to do is walk in there and crush all four bots.")
			else
				if (not Butch_enable_first_set) then
					Butch_enable_first_set = true
					show("node7", "node8")
				end
				show("node20")
			end
		end

		if (has_quest("Time to say goodnight")) then
			if (not done_quest("Time to say goodnight")) then
				npc_says(_"I've already opened the door. What are you waiting for? All you need to do is go down to the master arena and fight the bots.")
				npc_says(_"But, I can understand if you're hesitant. The master arena is by no means an easy task. Maybe you should practice some more before you go down.")
			else
				if (not Butch_enable_second_set) then
					Butch_enable_second_set = true
					show("node10")
				end
				show("node21")
			end
		end
		show("node99")
	end,

	{
		id = "node0",
		text = _"Hi! I'm new here.",
		code = function()
			npc_says(_"Yeah, what do you want newbie?")
			hide("node0") show("node1", "node4")
		end,
	},
	{
		id = "node1",
		text = _"Say 'newbie' again and I will rip your spine out.",
		code = function()
			npc_says(_"Fine, newbie. You are greener than green and a total newbie.")
			npc_says(_"You can start ripping out my spine now.")
			npc_says(_"Ha!")
			hide("node1") show("node2")
		end,
	},
	{
		id = "node2",
		text = _"I really mean it. I will kill you for that insult.",
		code = function()
			npc_says(_"Go for it.")
			hide("node2") show("node3")
		end,
	},
	{
		id = "node3",
		text = _"BANZAI! DIE DIE DIE!",
		code = function()
			npc_says(_"Wha --")
			drop_dead()
			set_faction_state("redguard", "hostile")
			hide("node3")
		end,
	},
	{
		id = "node4",
		text = _"And who are you?",
		code = function()
			npc_says(_"Butch... The arena master.")
			set_bot_name(_"Butch - Arena Master")
			hide("node1", "node4") show("node5")
		end,
	},
	{
		id = "node5",
		text = _"Arena? What arena?",
		code = function()
			npc_says(_"The underground arena. With holographic bots to train combat techniques against.")
			npc_says(_"Wanna fight?")
			hide("node5") show("node6", "node12", "node13")
		end,
	},
	{
		id = "node6",
		text = _"Yeah. I want to fight.",
		code = function()
			npc_says(_"Good. I opened up the novice arena door. Just climb down the ladder south of here.")
			npc_says(_"But remember, the escape hatch won't be opened until you kill all the bots.")
			npc_says(_"And be careful, your brain will be fooled by what happens in the holographic world. Any damage you take there, will result in actual physical damage.")
			npc_says(_"So even if the enemies are only holograms, they can still hurt you... and even kill you.")
			add_quest(_"Novice Arena", _"Fighting in the arena is not something I would usually do, but I feel like doing it for a change. Why not?")
			npc_says(_"Oh, and the room is made out of some special material. People reported their teleporter gadgets didn't work in there. Take care!")
			update_quest(_"Novice Arena", _"Butch says that teleporter gadgets may not work in this arena.")
			change_obstacle_state("Arena-AccessTrapdoor", "opened")
			hide("node6", "node12", "node13") show("node9")
		end,
	},
	{
		id = "node7",
		text = _"The bots are dead.",
		code = function()
			npc_says(_"Good for you.")
			npc_says(_"You are a total newbie. But, a talented newbie.")
			npc_says(_"I will put a good word in to Spencer about this.")
			update_quest(_"Novice Arena", _"Now that was easy. *Yawn*. I nearly fell asleep. No problem at all, the bots are dead.")
			hide("node7", "node8") show("node9", "node11")
		end,
	},
	{
		id = "node8",
		text = _"I thought I was going to fight you, coward.",
		code = function()
			npc_says(_"I NEVER said that, newbie.")
			npc_says(_"You wanted a fight, and you got one.")
			npc_says(_"I offered a fight, and you wanted that.")
			npc_says(_"There is nothing more to say, newbie.")
			hide("node7", "node8") show("node9", "node11")
		end,
	},
	{
		id = "node9",
		text = _"I want a bigger challenge.",
		code = function()
			if (not done_quest("Novice Arena")) then
				npc_says(_"I'm sorry, but town regulations forbid access to the master arena unless you've completed the novice arena first.")
				npc_says(_"This regulation is in place for your own safety, so that no newbie accidentally signs up for the master arena.")
				npc_says(_"You might try taking the novice arena first. Then I can grant you access to the master field.")
			else
				npc_says(_"Newbie, I have a second arena.")
				npc_says(_"You will die if you go there.")
				npc_says(_"I pity you, silly bird. Don't go there.")
				npc_says(_"But if you really want to, the door is now open.")
				add_quest(_"Time to say goodnight", _"I told the arena master to let me into his master arena. He agreed. Now all that remains to do is to climb down the ladder to the north arena and wait for death herself to come on her black wings and claim my soul. You know, now that I think about it... Maybe I should stay out of there?")
				npc_says(_"Remember, if your teleporter didn't work in the noob-, errr, novice-arena, it won't work in the master arena either...")
				update_quest(_"Time to say goodnight", _"Again, Butch warned me about possibly broken teleporter gadgets.")
				change_obstacle_state("MasterArenaAccessTrapdoor", "opened")
			end
			hide("node9")
		end,
	},
	{
		id = "node10",
		text = _"The master arena is clear.",
		code = function()
			npc_says(_"What!? Impossible! No one has ever survived the second arena!")
			npc_says(_"One has to be out of his mind to even attempt the second arena.")
			npc_says(_"You are not a newbie... You are COMPLETELY INSANE!")
			npc_says(_"Poor fool... You will surely die young.")
			add_xp(8000)
			update_quest(_"Time to say goodnight", _"I survived the master arena. A miracle. Whew.")
			hide("node10")
		end,
	},
	{
		id = "node11",
		text = _"What is wrong with you? Why do you keep calling me a newbie?",
		code = function()
			npc_says(_"Because you ARE a newbie, newbie!")
			hide("node11")
		end,
	},
	{
		id = "node12",
		text = _"Why the heck are you organizing fights? Enough people have died already!",
		code = function()
			npc_says(_"I have a very good reason not to tell you.")
			npc_says(_"Not like someone as inexperienced as you could understand it anyway.")
			hide("node12")
		end,
	},
	{
		id = "node13",
		text = _"No way! Violence is wrong! Make love not war!",
		code = function()
			npc_says(". . .")
			hide("node6", "node13") show("node14")
		end,
	},
	{
		id = "node14",
		text = _"Roses, not destruction!",
		code = function()
			npc_says(". . .")
			hide("node14") show("node15")
		end,
	},
	{
		id = "node15",
		text = _"End the violence! All blood is red!",
		code = function()
			npc_says(_"Hey, the bots bleed black. With oil.")
			tux_says(_"But... But...")
			npc_says(_"No 'but' about it, newbie.")
			npc_says(_"The bots bleed black.")
			tux_says(_"You... Have ruined my pacifistic slogan... I am angry... So angry that I want to smash things!")
			npc_says(_"Great. Get in the door, newbie.")
			add_quest(_"Novice Arena", _"Fighting in the arena is not something I would usually do, but I feel like doing it for a change. Why not?")
			change_obstacle_state("Arena-AccessTrapdoor", "opened")
			hide("node15")
		end,
	},
	{
		id = "node20",
		text = _"I want to fight in the novice arena!",
		code = function()
			npc_says(_"You've already completed the novice arena. But if you really want to have another go, the door's open.")
			npc_says(_"But you also might want to try the master arena now, maybe? If so, just say so.")
			-- change_obstacle_state("NoviceArenaExitDoor", "closed") -- for this we need to make sure it opens again after killing the bots
		end,
	},
	{
		id = "node21",
		text = _"I want to fight in the master arena!",
		code = function()
			npc_says(_"You've already completed the master arena, but I guess it's no skin off my nose if you want to have another go.")
			npc_says(_"In any case, you've certainly earned yourself a reputation as arena master around here now and we respect that.")
			-- change_obstacle_state("MasterArenaExitDoor", "closed") -- for this we need to make sure it opens again after killing the bots
		end,
	},
	{
		id = "node99",
		text = _"I'll be going then.",
		code = function()
			npc_says_random(_"Sure.",
				_"See you around.",
				_"Remember: pain is temporary but glory is forever.",
				_"Go kill some bots.")
			end_dialog()
		end,
	},
}
