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
PERSONALITY = { "Militaristic", "Abrasive", "Domineering", "Condescending" },
PURPOSE = "$$NAME$$ enables Tux's access to the training arenas."
WIKI]]--

local Npc = FDrpg.get_npc()
local Tux = FDrpg.get_tux()

return {
	FirstTime = function()
		show("node0")
	end,

	EveryTime = function()
		if (Tux:has_quest("Novice Arena")) then
			if (not Tux:done_quest("Novice Arena")) then
				Npc:says(_"I've already opened the door for you. All you need to do is walk in there and talk to Mike.")
			else
				if (not Butch_enable_first_set) then
					Butch_enable_first_set = true
					show("node7", "node8")
				end
				show("node20")
			end
		end

		if (Tux:has_quest("Time to say goodnight")) then
			if (not Tux:done_quest("Time to say goodnight")) then
				Npc:says(_"I've already opened the door. What are you waiting for? All you need to do is go down to the master arena and fight the bots.")
				Npc:says(_"But, I can understand if you're hesitant. The master arena is by no means an easy task. Maybe you should practice some more before you go down.")
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
			Npc:says(_"Yeah, what do you want newbie?")
			hide("node0") show("node1", "node4")
		end,
	},
	{
		id = "node1",
		text = _"Say 'newbie' again and I will rip your spine out.",
		code = function()
			Npc:says(_"Fine, newbie. You are greener than green and a total newbie.")
			Npc:says(_"You can start ripping out my spine now.")
			Npc:says(_"Ha!")
			hide("node1") show("node2")
		end,
	},
	{
		id = "node2",
		text = _"I really mean it. I will kill you for that insult.",
		code = function()
			Npc:says(_"Go for it.")
			hide("node2") show("node3")
		end,
	},
	{
		id = "node3",
		text = _"BANZAI! DIE DIE DIE!",
		code = function()
			Npc:says(_"Wha --")
			Npc:drop_dead()
			set_faction_state("redguard", "hostile")
			hide("node3")
		end,
	},
	{
		id = "node4",
		text = _"And who are you?",
		code = function()
			Npc:says(_"Butch... The arena master.")
			Npc:set_name("Butch - Arena Master")
			hide("node1", "node4") show("node5")
		end,
	},
	{
		id = "node5",
		text = _"Arena? What arena?",
		code = function()
			Npc:says(_"The underground arena. With holographic bots to train combat techniques against.")
			Npc:says(_"Wanna fight?")
			hide("node5") show("node6", "node12", "node13")
		end,
	},
	{
		id = "node6",
		text = _"Yeah. I want to fight.",
		code = function()
			Npc:says(_"Good. I opened up the arena door. Just climb down the ladder south of here.")
			Npc:says(_"You'll need search for Mike, the arena manager, there.")
			Npc:says(_"And be careful, your brain will be fooled by what happens in the holographic world. Any damage you take there, will result in actual physical damage.")
			Npc:says(_"So even if the enemies are only holograms, they can still hurt you... and even kill you.")
			Tux:add_quest("Novice Arena", _"Fighting in the arena is not something I would usually do, but I feel like doing it for a change. Why not?")
			Npc:says(_"Oh, and the room is made out of some special material. People reported their teleporter gadgets didn't work in there. Take care!")
			Tux:update_quest("Novice Arena", _"Butch says that teleporter gadgets may not work in this arena.")
			change_obstacle_state("Arena-AccessTrapdoor", "opened")
			hide("node6", "node12", "node13") show("node9")
		end,
	},
	{
		id = "node7",
		text = _"The bots are dead.",
		code = function()
			Npc:says(_"Good for you.")
			Npc:says(_"You are a total newbie. But, a talented newbie.")
			Npc:says(_"I will put a good word in to Spencer about this.")
			Tux:update_quest("Novice Arena", _"Now that was easy. *Yawn*. I nearly fell asleep. No problem at all, the bots are dead.")
			hide("node7", "node8") show("node9", "node11")
		end,
	},
	{
		id = "node8",
		text = _"I thought I was going to fight you, coward.",
		code = function()
			Npc:says(_"I NEVER said that, newbie.")
			Npc:says(_"You wanted a fight, and you got one.")
			Npc:says(_"I offered a fight, and you wanted that.")
			Npc:says(_"There is nothing more to say, newbie.")
			hide("node7", "node8") show("node9", "node11")
		end,
	},
	{
		id = "node9",
		text = _"I want a bigger challenge.",
		code = function()
			if (not Tux:done_quest("Novice Arena")) then
				Npc:says(_"I'm sorry, but town regulations forbid access to the master arena unless you've completed the novice arena first.")
				Npc:says(_"This regulation is in place for your own safety, so that no newbie accidentally signs up for the master arena.")
				Npc:says(_"You might try taking the novice arena first. Then I can grant you access to the master field.")
			else
				Npc:says(_"Newbie, I have a second arena.")
				Npc:says(_"You will die if you go there.")
				Npc:says(_"I pity you, silly bird. Don't go there.")
				Npc:says(_"But if you really want to, the door is now open.")
				Tux:add_quest("Time to say goodnight", _"I told the arena master to let me into his master arena. He agreed. Now all that remains to do is to climb down the ladder to the north arena and wait for death herself to come on her black wings and claim my soul. You know, now that I think about it... Maybe I should stay out of there?")
				Npc:says(_"Remember, if your teleporter didn't work in the noob-, errr, novice-arena, it won't work in the master arena either...")
				Tux:update_quest("Time to say goodnight", _"Again, Butch warned me about possibly broken teleporter gadgets.")
				change_obstacle_state("MasterArenaAccessTrapdoor", "opened")
			end
			hide("node9")
		end,
	},
	{
		id = "node10",
		text = _"The master arena is clear.",
		code = function()
			Npc:says(_"What!? Impossible! No one has ever survived the second arena!")
			Npc:says(_"One has to be out of his mind to even attempt the second arena.")
			Npc:says(_"You are not a newbie... You are COMPLETELY INSANE!")
			Npc:says(_"Poor fool... You will surely die young.")
			Tux:add_xp(8000)
			Tux:update_quest("Time to say goodnight", _"I survived the master arena. A miracle. Whew.")
			hide("node10")
		end,
	},
	{
		id = "node11",
		text = _"What is wrong with you? Why do you keep calling me a newbie?",
		code = function()
			Npc:says(_"Because you ARE a newbie, newbie!")
			hide("node11")
		end,
	},
	{
		id = "node12",
		text = _"Why the heck are you organizing fights? Enough people have died already!",
		code = function()
			Npc:says(_"I have a very good reason not to tell you.")
			Npc:says(_"Not like someone as inexperienced as you could understand it anyway.")
			hide("node12")
		end,
	},
	{
		id = "node13",
		text = _"No way! Violence is wrong! Make love not war!",
		code = function()
			Npc:says(_". . .")
			hide("node6", "node13") show("node14")
		end,
	},
	{
		id = "node14",
		text = _"Roses, not destruction!",
		code = function()
			Npc:says(_". . .")
			hide("node14") show("node15")
		end,
	},
	{
		id = "node15",
		text = _"End the violence! All blood is red!",
		code = function()
			Npc:says(_"Hey, the bots bleed black. With oil.")
			Tux:says(_"But... But...")
			Npc:says(_"No 'but' about it, newbie.")
			Npc:says(_"The bots bleed black.")
			Tux:says(_"You... Have ruined my pacifistic slogan... I am angry... So angry that I want to smash things!")
			Npc:says(_"Great. Get in the door, newbie.")
			Tux:add_quest("Novice Arena", _"Fighting in the arena is not something I would usually do, but I feel like doing it for a change. Why not?")
			change_obstacle_state("Arena-AccessTrapdoor", "opened")
			hide("node15")
		end,
	},
	{
		id = "node20",
		text = _"I want to fight in the novice arena!",
		code = function()
			Npc:says(_"You've already completed the novice arena. But if you really want to have another go, the door's open.")
			Npc:says(_"But you also might want to try the master arena now, maybe? If so, just say so.")
			-- change_obstacle_state("NoviceArenaExitDoor", "closed") -- for this we need to make sure it opens again after killing the bots
		end,
	},
	{
		id = "node21",
		text = _"I want to fight in the master arena!",
		code = function()
			Npc:says(_"You've already completed the master arena, but I guess it's no skin off my nose if you want to have another go.")
			Npc:says(_"In any case, you've certainly earned yourself a reputation as arena master around here now and we respect that.")
			-- change_obstacle_state("MasterArenaExitDoor", "closed") -- for this we need to make sure it opens again after killing the bots
		end,
	},
	{
		id = "node99",
		text = _"I'll be going then.",
		code = function()
			Npc:says_random(_"Sure.",
							_"See you around.",
							_"Remember: pain is temporary but glory is forever.",
							_"Go kill some bots.")
			end_dialog()
		end,
	},
}
