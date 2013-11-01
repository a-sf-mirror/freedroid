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
		if (Tania_surface) and
		(not done_quest("Tania's Escape")) and
		(not Tania_stopped_by_Pendragon) then --Tania is following you
			tania_is_here = true
			heal_npc("Tania")
		else
			tania_is_here = false
		end

		if (not Koan_met) then
			Koan_met = true
			npc_says(_"So you have come. I knew Duncan would not let me live for too long.")
			npc_says(_"Once I am gone this oasis of peace in the world will dry up and die. A shame.")
			npc_says(_"I hoped to turn the whole desert into a sanctuary. And now my dream will never come true.")
			npc_says(_"Do what you must.")
			if has_quest("Doing Duncan a favor") then
				show("node2")
			else
				if (not has_met("Duncan")) then
					show("node4")
				else
					show("node5")
				end
			end
			show("node3")
		elseif (Koan_spared_via_dialog) then
			npc_says_random(_"It's a pleasure to meet you again.",
				_"I am happy to see you've returned.")
			if (tania_is_here) then
				npc_says(_"I see you have brought a friend along with you.")
			end
		elseif has_quest("Doing Duncan a favor") then
			npc_says(_"I see the troubles of the world in your eyes.")
			show("node10", "node11")
			hide("node20", "node21", "node99")
			--elseif (met_koan_before_duncan) then
			-- npc_says_random(_"Come sit down my friend.",
			-- _"Please take some time and relax here in the cool.")
			-- if (tania_is_here) then
			-- npc_says(_"I see you've brought a friend along with you.")
			-- end
		end
	end,

	{
		id = "node2",
		text = _"With pleasure.",
		code = function()
			drop_dead()
			update_quest(_"Doing Duncan a favor", _"I took revenge for Duncan by killing Koan.")
			respawn_level(38) --this is the desert above the bunker
			if (tania_is_here) then
				Koan_murdered = true
				start_chat("Tania")
			end
		end,
	},
	{
		id = "node3",
		text = _"No. I have not come here to kill you.",
		code = function()
			npc_says(_"Thank you.")
			npc_says(_"Feel free to stay here. There is no danger.")
			npc_says(_"The water is safe to drink and the fruit from the tree is quite delicious. I have a few extra provisions here with me. Please take some.")
			add_item("Strength Pill", 1)
			add_item("Doc-in-a-can", 2)
			add_item("Source Book of Repair equipment", 1)
			add_item("Strength Capsule", 1)
			if has_quest("Doing Duncan a favor") then
				next("node12")
			end
			hide("node2", "node3") show("node5", "node20", "node99")
		end,
	},
	{
		--has not met Duncan yet
		id = "node4",
		text = _"Who are you? Who is Duncan?",
		code = function()
			set_bot_name(_"Koan")
			npc_says(_"I am Koan.")
			npc_says(_"Duncan is one obsessed with destroying things, while I create.")
			npc_says(_"Therefore he desires my destruction.")
			hide("node3", "node4", "node5") show("node20", "node99")
		end,
	},
	{
		id = "node5",
		text = _"Who are you?",
		code = function()
			set_bot_name(_"Koan")
			npc_says(_"I am Koan.")
			npc_says(_"Beyond that I am a creator.")
			npc_says(_"I give life to things, even if they eventually will be destroyed.")
			hide("node3", "node5") show("node20", "node99")
		end,
	},
	{
		id = "node10",
		text = _"You shall die.",
		code = function()
			drop_dead()
			update_quest(_"Doing Duncan a favor", _"I took revenge for Duncan by killing Koan.")
			respawn_level(38) --this is the desert above the bunker
			if (tania_is_here) then
				Koan_murdered = true
				start_chat("Tania")
			end
		end,
	},
	{
		id = "node11",
		text = _"Duncan sent me to kill you, but I came to make certain you were safe.",
		code = function()
			npc_says(_"Thanks for checking up on me.")
			hide("node10", "node11") show("node99") next("node12")
		end,
	},
	{
		id = "node12",
		text = _"BUG, REPORT ME! Koan node12 -- UPDATE QUEST",
		code = function()
			Koan_spared_via_dialog = true
			update_quest(_"Doing Duncan a favor", _"I met and talked to Koan, but didn't kill him.")
			show("node13")
		end,
	},
	{
		id = "node13",
		text = _"Why does Duncan want you dead?",
		code = function()
			npc_says(_"Very simple. He likes destruction, and I am creation.")
			npc_says(_"There is nothing more to it.")
			hide("node13")
		end,
	},
	{
		id = "node20",
		text = _"What is this place? Why is there grass and water under the desert?",
		code = function()
			npc_says(_"Ah, yes. That is my fault, I am afraid.")
			npc_says(_"Wherever I go, life follows me.")
			npc_says(_"I thought I could hide from Duncan in this old bunker.")
			npc_says(_"When I woke up the next day, things were as you see them now.")
			hide("node20") show("node21")
		end,
	},
	{
		id = "node21",
		text = _"This was a bunker?",
		code = function()
			npc_says(_"Yes. The place has changed a bit, as you can see.")
			npc_says(_"I like the way it is right now. The water cools everything down and the plants give the air a very fresh smell.")
			npc_says(_"As soon as I leave, everything will die. I have seen that happen before.")
			npc_says(_"Because this place is just like a small bit of paradise, I decided to stay here.")
			npc_says(_"I do not want this wonderful bunker to become devoid of life again.")
			hide("node21")
		end,
	},
	{
		id = "node99",
		text = _"I will go now.",
		code = function()
			npc_says_random(_"This place will always wait for you.",
				_"This is a sanctuary from the desert above.")
			heal_tux()
			hide("node2", "node3", "node4", "node5")
			end_dialog()
		end,
	},
}
