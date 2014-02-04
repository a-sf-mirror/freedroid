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
		show("node0")
		JohnPuzzlefalse = 0
		JohnPuzzlefalseA = 0
		JohnPuzzlefalseB = 0
		JohnPuzzlefalseC = 0
	end,

	EveryTime = function()
		John_puzzle_score = JohnPuzzlefalseA + JohnPuzzlefalseB + JohnPuzzlefalseC

		if (John_dislike) then
			Tux:says(_"I am s-")
			npc_says(_"Go away please.")
			end_dialog()
		end

		if (John_puzzle_started) then

			-- if Tux ran into false teleporter of the storage room
			if (John_lvl28to28_teleported) and
			   (not John_lvl28to28_hide) then
				Tux:says(_"What was that?!")
				npc_says(_"See..?")
				npc_says(_"It's not as easy as you thought.")
				npc_says(_"Take a closer look at the storage room again...")
				Tux:says(_"Okay.")
				John_lvl28to28_hide = true
			end

			-- Puzzle is neither solved nor locked:
			if (not John_puzzle_locked) or
			   (not John_puzzle_solved) then

				-- Tux walked through the false teleporter one time
				if (not John_problems_1) then
					if (JohnPuzzlefalseA == 1) or
					   (JohnPuzzlefalseB == 1) or
					   (JohnPuzzlefalseC == 1) then
						npc_says(_"Looks like you had some problems?")
						Tux:says(_"Yes...")
						npc_says(_"Ok, I'll give you another chance.")
						npc_says(_"But it will be your last chance, so take care...")
						Tux:says(_"Thanks.")
						John_problems_1 = true
					end
				end

				--Tux walked through the same teleporter twice (wtf?!)

				if (JohnPuzzlefalseA == 2) or
				   (JohnPuzzlefalseB == 2) or
				   (JohnPuzzlefalseC == 2) then
					npc_says(_"Dude!")
					npc_says(_"You made the same mistake twice.")
					npc_says(_"You walked through the same teleporter twice!")
					npc_says(_"What's wrong with your mind?")
					Tux:says(_"I-")
					npc_says(_"How is that ever going to work?")
					npc_says(_"I locked the puzzle.")
					npc_says(_"It doesn't seem like you'd ever be able to solve it.")
					John_puzzle_locked = true
				end

				if (not John_puzzle_locked) then
					-- Tux walked through a false teleporter second time
					if (John_puzzle_score >= 2) then
						npc_says(_"Sorry, seems like that puzzle was too hard for you.")
						Tux:says(_"But I-")
						npc_says(_"It is locked now.")
						Tux:says(_"I was very close to the solution.")
						npc_says(_"Well, why did you still fail if you were so close?")
						Tux:says(_"Hmpf..")
						John_puzzle_locked = true
					end
				end

			end -- of 'if (not John_puzzle_locked) or
			-- (not John_puzzle_solved) then'

			-- if Tux solved the puzzle, went into the correct teleporter
			if (not John_puzzle_locked) and
			   (John_puzzle_solved) then
				if (John_lvl28to28_teleported) or
				   (John_puzzle_score > 0 ) then
					npc_says(_"Congratulations, you solved my little puzzle.")
					npc_says(_"I give you this as a small reward.")
					Tux:add_item("Dexterity Pill", 1)
				else
					-- Tux solved the puzzle without retrying!
					npc_says(_"Wow... you solved it, and on your first time, too.")
					npc_says(_"That's pretty good.")
					npc_says(_"Here, take this as reward.")
					Tux:says(_"Oh, thank you.")
					Tux:add_item("Dexterity Pill", 1)
					Tux:add_item("Strength Pill", 1)
				end
				John_puzzle_locked = true
				set_bot_destination("JohnFreeLabel")
			end

		end -- of 'if (John_puzzle_started) then'

		set_rush_tux(0, "John")

		show("node99")
	end,

	{
		id = "node0",
		text = _"Hello.",
		code = function()
			npc_says(_"Hello.")
			npc_says(_"Errr... How did you get in here?!")
			Tux:says(_"I just walked through the door.")
			npc_says(_"And your name?")
			--; TRANSLATORS: %s = Tux:get_player_name()
			Tux:says(_"I'm %s.", Tux:get_player_name())
			npc_says(_"And you were not slaughtered by the bots outside?")
			Tux:says(_"Apparently, I am still alive.")
			Tux:says(_"Does that answer your question?")
			npc_says(_"It does... smart aleck.")
			hide("node0") show("node1")
		end,
	},
	{
		id = "node1",
		text = _"What do you do here?",
		code = function()
			npc_says(_"I'm living here.")
			npc_says(_"What did you expect?")
			Tux:says(_"Okay, that question was a little stupid.")
			hide("node1") show("node2")
		end,
	},
	{
		id = "node2",
		text = _"Are there any other people around here?",
		code = function()
			npc_says(_"Not that I know.")
			npc_says(_"Only you and me.")
			Tux:says(_"Okay, that question was also a little stupid.")
			hide("node2") show("node3")
		end,
	},
	{
		id = "node3",
		text = _"Isn't it a little boring in here, isolated from the outer world?",
		code = function()
			npc_says(_"I'm fine.")
			npc_says(_"When I'm bored, I try to make up some puzzles...")
			hide("node3") show("node4", "node5")
		end,
	},
	{
		id = "node4",
		text = _"'Puzzles'?",
		code = function()
			npc_says(_"Yeah, puzzles.")
			npc_says(_"Do you want to try one?")
			hide("node4", "node5") show("node10", "node11")
		end,
	},
	{
		id = "node5",
		text = _"Sounds interesting.",
		code = function()
			Tux:says(_"Can you tell me more about it?")
			npc_says(_"No.")
			npc_says(_"But you can experience it for yourself.")
			npc_says(_"Interested?")
			hide("node4", "node5") show("node10", "node11")
		end,
	},
	{
		id = "node10",
		text = _"Yeah, sure!",
		code = function()
			npc_says(_"Ok, so the entrance is in my storage room.")
			npc_says(_"Shouldn't be too difficult to find.")
			Tux:says(_"Ok, let me take a look.")
			npc_says(_"Wait, wait. I'll give you a hint:")
			npc_says(_"'Looking from above hostiles may appear as salvation.'")
			Tux:says(_"Uh, ok, thanks.")
			John_puzzle_started = true
			hide("node10", "node11")
			end_dialog()
		end,
	},
	{
		id = "node11",
		text = _"Hmm... I don't trust you.",
		code = function()
			Tux:says(_"I think, it's a trap!")
			npc_says(_"Well, you can leave at any time.")
			npc_says(_"I wish you would!")
			John_dislike = true
			hide("node10", "node11")
			end_dialog()
		end,
	},
	{
		id = "node99",
		text = _"See you.",
		code = function()
			if (John_dislike) then
				npc_says(_"I don't think so...")
			else
				npc_says(_"See you later.")
			end
			end_dialog()
		end,
	},
}
