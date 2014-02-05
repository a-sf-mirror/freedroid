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
		desertgate_tax = 0
		show("node0")
	end,

	EveryTime = function()
		branch_to_tania = false
		if (Tania_stopped_by_Pendragon) and
		   (not Pendragon_OK_w_Tania) and --Tania Escape Quest stuff
		   (Spencer_Tania_sent_to_DocMoore) or
		   (Tania_set_free) then
			if (not Tux:done_quest("Tania's Escape")) then
				npc_says(_"Tania has been waiting to talk to you.")
			else
				npc_says(_"Spencer says it is OK?")
				npc_says(_"Well that is good enough for me.")
				npc_says(_"She may enter.")
				Pendragon_OK_w_Tania = true
				end_dialog()
			end
		elseif (Tania_stopped_by_Pendragon) then
			if (not Tania_met_Pendragon) then
				npc_says(_"Halt! Who goes there?")
				Tux:says(_"Someone I found in the desert, Tania.")
			end
			--; TRANSLATORS: %s = Tux:get_player_name()
			npc_says(_"%s, you may enter.", Tux:get_player_name())
			npc_says(_"However, you will have to talk to Spencer before you bring your friend, Tania, into the town.")
			if (not Tania_met_Pendragon) then
				change_obstacle_state("DesertGate-Inner", "opened")
				teleport_npc("W-enter-2", "Tania") --Ensure that Tania is on Level 0!
				set_bot_state("patrol", "Tania")
				Tania_met_Pendragon = true
				Tania_stopped_by_Pendragon = true
				branch_to_tania = true
			end
		end

		show("node99")

		if (not guard_follow_tux) then
			show("node40")
		end

		if (desertgate_tax ~= 0) then
			if (cmp_obstacle_state("DesertGate", "closed")) then
				npc_says(_"Got the money?")
				show_if((Tux:get_gold() >= desertgate_tax), "node45")
			end
			hide("node40")
		end

		if (cmp_obstacle_state("DesertGate", "opened")) or
		(Tux:has_met("Tania")) then
			hide("node40", "node45")
		end

		if (branch_to_tania) then
			start_chat("Tania")
		end
	end,

	{
		id = "node0",
		text = _"Hi! I'm new here.",
		code = function()
			npc_says(_"You don't say. I'm not living here either. I'm just on vacation.")
			npc_says(_"Normally I'm a fighter. But now I try to relax, chill, take it easy and have a lot of fun.")
			npc_says(_"And in case you couldn't tell, that was sarcasm.")
			set_bot_name(_"Fighter")
			hide("node0") show("node1", "node2")
		end,
	},
	{
		id = "node1",
		text = _"Could you teach me how to fight?",
		code = function()
			if (Pendragon_beaten_up) then
				npc_says(_"NO! NO! Er... I mean, not right now. I am busy. Come again later.")
			else
				npc_says(_"Sure. I can help you.", "NO_WAIT")
				npc_says(_"For a price. Hundred circuits should be enough.")
				hide("node1", "node99") show("node11", "node19")
			end
		end,
	},
	{
		id = "node2",
		text = _"Do you have a name?",
		code = function()
			npc_says(_"I go by Pendragon, because I can pull a knife from thin air.")
			npc_says(_"And you?")
			--; TRANSLATORS: %s = Tux:get_player_name()
			Tux:says(_"%s, because it is my name?", Tux:get_player_name())
			set_bot_name(_"Pendragon - Fighter")
			hide("node2")
		end,
	},
	{
		id = "node11",
		text = _"Teach me the basic stuff.",
		code = function()
			npc_says(_"Are you sure?", "NO_WAIT")
			npc_says(_"I only accept students that already have some experience in fighting bots.")
			hide("node11", "node19") show("node12", "node29")
		end,
	},
	{
		id = "node12",
		text = _"Yes (costs 100 valuable circuits, 5 training points)",
		code = function()
			if (Tux:get_skill("melee") < 1) then
				if (Tux:train_skill(100, 5, "melee")) then
					Tux:del_health(25)
					npc_says(_"Let us begin then.")
					npc_says(_"Come closer...")
					npc_says(_"HA!")
					Tux:says(_"Ouch! What the hell are you doing?")
					npc_says(_"Lesson number one. Never trust the opponent.")
					Tux:says(_"Just let me get up an -- ouch! Stop it.")
					npc_says(_"Lesson number two. The bots have no mercy.")
					npc_says(_"They won't let you get up if you fall down. They will kill you.")
					npc_says(_"Now, lesson number -- ugh! That hurt!")
					Tux:says(_"Lesson number three. Never underestimate the enemy.")
					npc_says(_"Ugh. Yes. You are correct.")
					npc_says(_"I need to... Lie down for a while... My head hurts...")
					npc_says(_"Lesson number four. Never hit your sparring partner with full force.")
					npc_says(_"I think you broke my ribs...")
					npc_says(_"This is enough training for today.")
					Pendragon_beaten_up = true
				else
					if (Tux:get_gold() >= 100) then
						npc_says(_"You don't have enough experience. I can't teach you anything more right now.")
						npc_says(_"First collect more experience. Then we can go on.")
					else
						npc_says(_"You don't have enough valuable circuits on you! Come back when you have the money.")
					end
				end
			else
				npc_says(_"Hey, wake up! I taught you that stuff already. Go practice on the bots, if you really want more training.")
			end
			hide("node12")
		end,
	},
	{
		id = "node19",
		text = _"Teach me how to smash concrete blocks with my bare hands.",
		code = function()
			npc_says(_"Oh, Linarian, you amuse me so.", "NO_WAIT")
			npc_says(_"You are quite an ambitious fellow.")
			npc_says(_"First learn the basics. Spend a year working on that, and then I'll be able to teach you some advanced tricks.")
			hide("node19") show("node20")
		end,
	},
	{
		id = "node20",
		text = _"Teach me Ninjitsu.",
		code = function()
			npc_says(_"Go away.")
			hide("node20") show("node21")
		end,
	},
	{
		id = "node21",
		text = _"I want to know the Mega Uber Crazy Double-Fang Wolf-Sunlight I-Like-Tea Death Touch of Major Destruction Flesh-Bursting Attack!",
		code = function()
			npc_says(_"Wow.", "NO_WAIT")
			npc_says(_"You have a very rich imagination. There is no such thing. Get a life.")
			hide("node21") show("node22")
		end,
	},
	{
		id = "node22",
		text = _"I want to know the forbidden skill of furuike ya kawazu tobikomu mizu no oto.",
		code = function()
			npc_says(_"This is starting to make me angry. Shut up and go away.")
			hide("node22") show("node23")
		end,
	},
	{
		id = "node23",
		text = _"Teach me how to flip out and kill people!",
		code = function()
			npc_says(_"*sigh*")
			npc_says(_"I will teach you to keep your mouth shut when told to. Observe very carefully.")
			npc_says(_"Make sure you pay attention.")
			npc_says(_"TAKE THIS!")
			Tux:says(_"OW!")
			if (not Tux:del_health(40)) then
				if (not Tux:del_health(20)) then
					Tux:del_health(5)
				end
			end
			npc_says(_"Now go away.")
			hide("node23")
			end_dialog()
		end,
	},
	{
		id = "node29",
		text = _"Enough for now.",
		code = function()
			npc_says(_"Fine.")
			hide("node11", "node12", "node19", "node20", "node21", "node22", "node23", "node29") show("node1", "node99")
		end,
	},
	{
		id = "node40",
		text = _"I want to pass the gate.",
		code = function()
			npc_says(_"Fine.")
			Tux:says(_"Can you open it please?")
			npc_says(_"Yes, I can.")
			Tux:says(_"Thanks.")
			Tux:says(_"Uhm...")
			Tux:says(_"Will you do it?")
			npc_says(_"Yes. But you have to pay a one-time tax for it first.")
			if (not Town_NorthGateGuard_tux_nickname_loon) then
				npc_says(_"Pay 30 circuits and you may pass.")
				desertgate_tax = 30
			else
				npc_says(_"You can enter the desert, but first you'll have to pay 40 circuits.")
				desertgate_tax = 40
			end
			if (Tux:get_gold() >= desertgate_tax) then
				show("node45")
			end
			hide("node40")
		end,
	},
	{
		id = "node45",
		text = _"Yes, I've got the money.",
		code = function()
			if (Tux:get_gold() >= desertgate_tax) then
				if (desertgate_tax == 30) then
					npc_says(_"Good.")
					Tux:says(_"Here, take it.")
					npc_says(_"You may pass.")
					if (Tux:has_quest("Doing Duncan a favor")) then
						Tux:update_quest("Doing Duncan a favor", _"Pendragon opened the gate for me, but I had to pay a little tax.")
					end
				elseif (desertgate_tax == 40) then
					npc_says(_"At last.")
					Tux:says(_"Here, let me pass now.")
					npc_says(_"You may pass, Loon.")
					npc_says(_"But don't expect any help in there.")
					if (Tux:has_quest("Doing Duncan a favor")) then
						Tux:update_quest("Doing Duncan a favor", _"Pendragon opened the gate for me, but I had to pay a tax.")
					end
				end
				Tux:del_gold(desertgate_tax)
				desertgate_tax = 0
				change_obstacle_state("DesertGate", "opened")
			else
				npc_says(_"Don't try to trick me, duck.") --just in case...
				npc_says(_"Come back if you have the bucks!")
			end
			hide("node45")
			end_dialog()
		end,
	},
	{
		id = "node99",
		text = _"See you later.",
		code = function()
			npc_says_random(_"Have courage.",
							_"Be strong.")
			end_dialog()
		end,
	},
}
