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
		show("node0", "node3")
	end,

	EveryTime = function()
		if (Tania_at_Ewalds_Bar) and
		   (not Ewald_got_told_Tanias_story) then
			Ewald_got_told_Tanias_story = true
			npc_says(_"Your lady friend just told me the epic story of how you rescued her.")
			npc_says(_"That was really something.")
			npc_says(_"I'd give you a drink on the house, but unfortunately I'm still out.")
		end

		if (drank_all_Ewald_had) then
		   show("node14")
		end

		if (Ewald_gambling) then
			show("node30")
		end

		show("node99")
	end,

	{
		id = "node0",
		text = _"Hi. You look really happy.",
		code = function()
			npc_says(_"Hey! I'm Ewald. I'm a bartender.")
			npc_says(_"I used to do a lot of drugs, and I destroyed my brain.")
			npc_says(_"I'm not too smart now, but I'm very happy all the time!")
			npc_says(_"Want something to drink?")
			set_bot_name(_"Ewald - Barkeeper")
			hide("node0") show("node1", "node2", "node13")
		end,
	},
	{
		id = "node1",
		text = _"How is your business doing?",
		code = function()
			if (guard_follow_tux) then
				npc_says(_"Can't complain at the moment...")
			else
				npc_says(_"Nah, no darn good at all.")
				npc_says(_"The bots ruined it for me. Now the supplies are very low and I had to make the prices high.")
				npc_says(_"No one wants to pay 499 circs for bottom shelf swill that even Bender would not drink...")
				npc_says(_"The tax is quite brutal as well. And the Red Guard always demands discounts too.")
				npc_says(_"Life is tough. I am still happy though.")
				hide("node1")
			end
		end,
	},
	{
		id = "node2",
		text = _"Are you selling something other than drinks?",
		code = function()
			npc_says(_"Yeah, I have some junk here... Want to see?")
			trade_with("Ewald")
		end,
	},
	{
		id = "node3",
		text = _"I could use a drink. What is there to be had?",
		code = function()
			npc_says(_"Well... We are a bit short on supplies right now... Erm... AH! I know!")
			npc_says(_"I have WATER!")
			if (not Tux:has_item_backpack("Mug")) and
			   (not Tux:has_item_backpack("Cup")) then
				npc_says(_"But you'll need a mug or a cup for me to put it in.")
			else
				hide("node3") show("node4", "node5", "node6")
			end
		end,
	},
	{
		id = "node4",
		text = _"I want double water on the rocks. Stirred, not shaken.",
		code = function()
			npc_says(_"Here you go. That one is on the house. And if the water is not enough, just ask me for more.")
			Tux:heat(-20)
			hide("node4", "node5", "node6") show("node9")
		end,
	},
	{
		id = "node5",
		text = _"Long Island iced water please.",
		code = function()
			npc_says(_"Here you go.")
			Tux:heat(-30)
			hide("node4", "node5", "node6") show("node8")
		end,
	},
	{
		id = "node6",
		text = _"I want something cheap... Give me some fortified water.",
		code = function()
			npc_says(_"Eh... Sure, as you wish. Here you go.")
			Tux:heat(-50)
			hide("node4", "node5", "node6") show("node7")
		end,
	},
	{
		id = "node7",
		text = _"Ugh... Ahh... I like it. Give me some more of that.",
		code = function()
			npc_says(_"Uh... Sure...")
			Tux:heat(-1000)
			hide("node7") show("node10")
		end,
	},
	{
		id = "node8",
		text = _"Ahh... Fresh water! Give me some more of that.",
		code = function()
			npc_says(_"Yeah, I thought you would like it. Here is more.")
			Tux:heat(-1000)
			hide("node8") show("node10")
		end,
	},
	{
		id = "node9",
		text = _"Hmm... Lots of ice. Give me one more double water, but now without any ice.",
		code = function()
			npc_says(_"One double water coming up.")
			Tux:heat(-1000)
			hide("node9") show("node10")
		end,
	},
	{
		id = "node10",
		text = _"MORE! I want more water!",
		code = function()
			npc_says(_"Erm... Ok... Here, have some of my special, water colada.")
			Tux:heat(-1000)
			hide("node10") show("node11")
		end,
	},
	{
		id = "node11",
		text = _"I am really thirsty! I want water.",
		code = function()
			npc_says(_"Well, you drank all my water, so you cannot have any more. There is none left.")
			npc_says(_"I heard Linarians can drink a lot of water, but seven liters...")
			hide("node11") show("node12")
		end,
	},
	{
		id = "node12",
		text = _"MORE! MORE! MORE! I WANT MORE WATER. NOW!",
		code = function()
			npc_says(_"Eh... I think you've had enough... Maybe you should just go for a walk for now?")
			npc_says(_"You have had everything that I had.")
			drank_all_Ewald_had = true
			hide("node12")
		end,
	},
	{
		id = "node13",
		text = _"Any idea where I can find some entertainment here?",
		code = function()
			if (tux_has_joined_guard) then
				npc_says(_"I thought that shooting bots with those big guns was fun...?")
				npc_says(_"*sigh*")
				npc_says(_"I wish I could once use such a gun.")
				npc_says(_"You are a Guard member, right?")
				if (Tux:has_item("Barrett M82 Sniper Rifle") or
					Tux:has_item("Exterminator")) then
					Tux:says(_"Yes, I am a Guard member. No, it is not allowed to give guns to people who are high.")
					npc_says(_"Pleeeeaaaase.")
					Tux:says(_"No exceptions. I apologize. Please think responsibly; there is a good reason for this rule.")
					npc_says(_"Hmm. You're right, I better ask again later.")
					npc_says(_"Well, since you won't let me play with your big guns, we could always try a gambling game or two.")
				else
					Tux:says(_"The fact that I am a Red Guard member does not necessarily mean that I have a big gun with me.")
					npc_says(_"Really?")
					npc_says(_"Stop kidding me. I know you have big a gun somewhere.")
					npc_says(_"Every Guard has one.")
					Tux:says(_"Look at me! Do you see any gun here at my belt or in my hands?")
					npc_says(_"Hmm... You are right. No gun, indeed.")
					npc_says(_"Sorry.")
					npc_says(_"Since there are no big guns to shoot, how about shooting some dice? Would you like to gamble?")
				end
			else
				npc_says(_"Well... Erm... No.")
				npc_says(_"What the bots did not smash, the Red Guard taxed.")
				npc_says(_"Fun is kinda dead right now.")
				npc_says(_"Well, unless you are in the Red Guard itself, of course. They have those big guns that can destroy a dozen bots with one shot...")
				npc_says(_"Now THAT'S fun, if you ask me.")
				npc_says(_"If you are really bored, you can try gambling with me.")
			end
			hide("node13") show("node30")
		end,
	},
	{
		id = "node14",
		text = _"Hey, I could use a refreshing drink!",
		code = function()
			npc_says(_"Here, have a Watery Mary.")
			Tux:heat(-1000)
			hide("node14")
		end,
	},
	{
		id = "node30",
		text = _"Ok, let's gamble a bit.",
		code = function()
			Ewald_gambling = true
			npc_says(_"I play a dice game and a coin-flipping game.")

			-- Initialize some variables here
			next_gamble_node = 0
			bet, next_bet, total_bet = 0, 0, 0
			tux1, tux2, tux3 = 0, 0, 0
			ewd1, ewd2, ewd3, ewd4, ewd5 = 0, 0, 0, 0, 0

			hide("node30") show("node31", "node32", "node33", "node34", "node69")
			push_topic("Gambling")
		end,
	},
	{
		id = "node31",
		text = _"Tell me the rules for your 'dice game'.",
		topic = "Gambling",
		code = function()
			npc_says(_"It is simple. We both get three six sided dice.")
			npc_says(_"You bet. Then you throw one die.")
			npc_says(_"Then you decide on a betting multiplier, up to 3x.")
			npc_says(_"Then you throw two dice and I throw three dice. If sum of your three dice are better than my three dice, you win twice your bet, minus a tax for the Red Guard.")
			npc_says(_"Remember: the odds are slightly against you, so don't gamble anything you can't afford to lose.")
			------------------------------------------------
			-- Assuming optimal playing then
			-- Expected payout: 103,5 % (easy), 98.27% (normal), 95.61% (hard)
		end,
	},
	{
		id = "node32",
		text = _"Tell me the rules for your 'coin-flip game'.",
		topic = "Gambling",
		code = function()
			npc_says(_"It is simple. Five coin flips.")
			npc_says(_"You bet. Then you tell me if I will flip more 'heads' or 'tails'.")
			npc_says(_"I flip one coin.")
			npc_says(_"Then you tell me if there will be an 'odd' or 'even' number of flips for what you chose.")
			if (difficulty() == "easy") then
				npc_says(_"I flip the other four coins. If you were completely right, I give you twice what you bet, if wrong, you get your bet back.")
			elseif (difficulty() == "normal") then
				npc_says(_"I flip the other four coins. If you were completely right, I give you twice what you bet, if completely wrong, you get 3/4 of your bet back.")
			else
				npc_says(_"I flip the other four coins. If you were completely right, I give you twice what you bet, if completely wrong, you get half of your bet back.")
			end
			npc_says(_"But if you are only partially wrong... well, then you get nothing.")
			npc_says(_"Remember: the odds are slightly against you, so don't gamble anything you can't afford to lose.")
			------------------------------------------------
			-- Assuming optimal playing:
			-- Winning both: 11/32 expected payout: 88/128 (easy/normal/hard)
			-- Losing both: 11/32 expected payout: 44/128 (easy), 33/128 (normal), 22/128 (hard)
			-- Intermediate: 10/32 expected payout: 0/128 (easy/normal/hard)
			-- Expected payout: 132/128 = 103.1% (easy), 121/128 = 94.5% (normal), 110/128 = 85.9% (hard)
		end,
	},
	{
		id = "node33",
		text = _"Let's go for a game of dice.",
		topic = "Gambling",
		code = function()
			next_gamble_node = 40
			hide("node31", "node32", "node33", "node34") next("node35")
		end,
	},
	{
		id = "node34",
		text = _"I want to play coin-flipping.",
		topic = "Gambling",
		code = function()
			next_gamble_node = 50
			hide("node31", "node32", "node33", "node34") next("node35")
		end,
	},
	{
		id = "node35",
		text = "BUG, REPORT ME! ewald node35 -- Choose the Bet",
		echo_text = false,
		topic = "Gambling",
		code = function()
			-- Ewald forces you to gamble semi-responsibly ;-)
			max = Tux:get_gold()

			if (max > 40) then
				show("node36")
				for k,v in pairs({[37] = 120, [38] = 400, [39] = 1200}) do
					if (max > v) then
						show("node" .. k)
					end
				end
				npc_says(_"Then, how many are you going to bet?")
			else
				npc_says(_"You should come back when you have more to gamble with.")
				pop_topic()
			end
		end,
	},
	{
		id = "node36",
		text = _"How about a bet of 10 circuits?",
		topic = "Gambling",
		code = function()
			bet, next_bet = 10, 10
			hide("node36", "node37", "node38", "node39", "node69") next("node" .. next_gamble_node)
		end,
	},
	{
		id = "node37",
		text = _"I'm feeling a bet of 30 circuits.",
		topic = "Gambling",
		code = function()
			bet, next_bet = 30, 30
			hide("node36", "node37", "node38", "node39", "node69") next("node" .. next_gamble_node)
		end,
	},
	{
		id = "node38",
		text = _"I will place a bet of 100 circuits.",
		topic = "Gambling",
		code = function()
			bet, next_bet = 100, 100
			hide("node36", "node37", "node38", "node39", "node69") next("node" .. next_gamble_node)
		end,
	},
	{
		id = "node39",
		text = _"I want to bet 300 circuits.",
		topic = "Gambling",
		code = function()
			bet, next_bet = 300, 300
			hide("node36", "node37", "node38", "node39", "node69") next("node" .. next_gamble_node)
		end,
	},
	{
		id = "node40",
		text = "BUG, REPORT ME! ewald node40 -- Dice - Initial roll",
		echo_text = false,
		code = function()
			npc_says(_"Dice it is. Roll your first die.")
			tux1=math.random(1,6)
			Tux:says(_"I rolled a %d.", tux1)
			npc_says(_"What multiplier are you going to bet?")
			show("node41", "node42", "node43")
		end,
	},
	{
		id = "node41",
		text = "1x",
		topic = "Gambling",
		code = function()
			next("node44")
		end,
	},
	{
		id = "node42",
		text = "2x",
		topic = "Gambling",
		code = function()
			bet=bet*2
			next("node44")
		end,
	},
	{
		id = "node43",
		text = "3x",
		topic = "Gambling",
		code = function()
			bet=bet*3
			next("node44")
		end,
	},
	{
		-- finish game here
		id = "node44",
		text = "BUG, REPORT ME! ewald node43 -- Dice - finish game",
		echo_text = false,
		code = function()
			npc_says(_"You've bet %d valuable circuits.", bet)
			ewd1,ewd2,ewd3 = math.random(1,6),math.random(1,6),math.random(1,6)
			npc_says(_"My rolls are %d, %d and %d.", ewd1, ewd2, ewd3)
			tux2,tux3 = math.random(1,6),math.random(1,6)
			Tux:says(_"I rolled a %d and a %d.", tux2, tux3)

			npc_says(_"It looks like I scored %d and you scored %d.", ewd1 + ewd2 + ewd3, tux1 + tux2 + tux3)

			if ((ewd1+ewd2+ewd3) < (tux1+tux2+tux3)) then --Tux wins!
				npc_says_random(_"You won! Good job.",
					_"You won! Man you are lucky!",
					_"You won!")
				local bet_gain = {easy = 0.95, normal = 0.85, hard = 0.8}
				bet = bet * bet_gain[difficulty()]
				Tux:add_gold(bet)
				total_bet = total_bet + bet
				display_console_message(string.format(_"You won %d valuable circuits by gambling dice with Ewald.", math.floor(bet)))
			else -- House Wins!
				npc_says_random(_"You lost. Bad luck.",
					_"You lost. Better luck next time.",
					_"You lost, try again?",
					_"You lost. Maybe you might like coin-flipping better?")
				Tux:del_gold(bet)
				total_bet = total_bet - bet
				display_console_message(string.format(_"You lost %d valuable circuits by gambling dice with Ewald.", bet))
			end
			hide("node41", "node42", "node43") next("node60")
		end,
	},
	{
		id = "node50",
		text = "BUG, REPORT ME! ewald node50 -- Coin-flipping game",
		echo_text = false,
		code = function()
			-- Constant for the coin-flipping game
			coin_str = {_"heads", _"tails"}

			npc_says(_"Coin flipping it is.")
			npc_says(_"So. What do you think? Heads or tails?")
			show("node51", "node52")
		end,
	},
	{
		id = "node51",
		text = _"Heads",
		topic = "Gambling",
		code = function()
			npc_says_random(_"Heads, eh? I always like heads.",
				_"Heads? Sometimes I pick heads... when I don't pick tails.",
				_"When I think too much about it, I end up picking heads.",
				_"Just the other day I found out that 'obverse' was the fancy name for 'heads'. Trippy.",
				_"I think I would have picked tails this time.")
			tux1 = 0
			hide("node51", "node52") next("node53")
		end,
	},
	{
		id = "node52",
		text = _"Tails",
		topic = "Gambling",
		code = function()
			npc_says_random(_"Tails? Fits, since you've got one.",
				_"Tails? I always liked that game... good times.",
				_"Tails. When I don't pick heads, I always pick tails.",
				_"Just the other day I found out that 'reverse' was the fancy name for 'tails'. Trippy.",
				_"I think I would have picked heads this time.")
			tux1 = 1
			hide("node51", "node52") next("node53")
		end,
	},
	{
		id = "node53",
		text = "BUG, REPORT ME! ewald node53 -- Coin-flipping game - first flip",
		echo_text = false,
		topic = "Gambling",
		code = function()
			npc_says(_"Here goes nothing.")
			ewd1=math.random(0,1)
			npc_says(_"Looks like it is '%s'.", coin_str[ewd1 + 1])

			npc_says(_"So do you think there will be an even or odd number of '%s' flips?", coin_str[tux1 + 1])

			show("node54", "node55")
		end,
	},
	{
		id = "node54",
		text = _"Even number",
		topic = "Gambling",
		code = function()
			tux2 = 0 -- Set to 'even'

			local random_says = {_"Even, eh? I always like even. It is so symmetric.",
				_"Even? Sometimes I pick even... when I don't pick odd.",
				_"When I think too much about it, I end up picking even.",
				_"I think I would have picked even this time."}

			if (ewd1 ~= tux1) then
				table.insert(random_says, _"Hoping to guess them all wrong, eh?")
			else
				table.insert(random_says, _"Bucking the statistics. I like that.")
			end

			npc_says_random(unpack(random_says))
			hide("node54", "node55") next("node56")
		end,
	},
	{
		id = "node55",
		text = _"Odd number",
		topic = "Gambling",
		code = function()
			tux2 = 1 -- Set to 'odd'

			local random_says = {_"Odd? Fits, since you're pretty odd yourself.",
				_"Odd? Well that is odd... I was thinking of odd.",
				_"Odd. When I don't pick even, I always pick odd too.",
				_"I think I would have picked even this time."}

			if (ewd1 == tux1) then
				table.insert(random_says, _"Honing up on your math, eh?")
			else
				table.insert(random_says, _"Betting against the game. I like that.")
			end

			npc_says_random(unpack(random_says))
			hide("node54", "node55") next("node56")
		end,
	},
	{
		id = "node56",
		text = "BUG, REPORT ME! ewald node56 -- Coin-flipping game - last four flips",
		echo_text = false,
		topic = "Gambling",
		code = function()
			local function flip_coin()
				result = math.random(0, 1)
				npc_says(_"%s.", coin_str[result + 1]:gsub("%a", string.upper, 1))
				return result
			end

			npc_says(_"The last four coin flips:")
			ewd2, ewd3, ewd4 = flip_coin(), flip_coin(), flip_coin()
			Tux:says(_"Last flip. I hope it is the one I want!")
			ewd5 = flip_coin()

			ewald_sum = ewd1 + ewd2 + ewd3 + ewd4 + ewd5
			npc_says(_"Looks like that is %d tails, and %d heads.", ewald_sum, 5 - ewald_sum)

			local win = 0 -- find if tux won...
			if (ewald_sum > 2) then
				npc_says(_"Tails won...")
				win = 0.5 * tux1
			else
				npc_says(_"Heads won...")
				win = 0.5 * (1 - tux1)
			end

			if (math.fmod(ewald_sum, 2) == tux1) then
				npc_says(_"...and there were an odd number of %s.", coin_str[tux1 + 1])
				win = win + 0.5 * tux2
			else
				npc_says(_"...and there were an even number of %s.", coin_str[tux1 + 1])
				win = win + 0.5 * (1 - tux2)
			end

			if (win == 1) then -- WIN
				npc_says_random(_"Looks like another winning combination! Good job!",
					_"You have the best luck! You won!",
					_"Too many of these and I'll go out of business. You won!",
					_"You will have to tell me how you did it. You Won!",
					_"Let me be even with you: I can't make heads or tails of how you won this odd game. Good Job!")
				display_console_message(string.format(_"You won %d valuable circuits in the coin-flip game with Ewald.", bet))
			elseif (win == 0) then --LOSS
				npc_says_random(_"Looks like you got everything wrong... here is some of your bet back.",
					_"Well, close, but exactly wrong. Here is some of your bet back.",
					_"You need to do exactly opposite of what you did here. Here is part of your bet back.",
					_"Close, but no cigar. Here is some of your money back.",
					_"Next time pick the opposite of what you did this time... or something.",
					_"Close only counts in hand grenades and horseshoes. But some money back.")

				local bet_loss = {easy = 1, normal = 0.75, hard = 0.5} -- easy: 100% back normal: 75% back, hard: 50% back
				bet = bet*bet_loss[difficulty()]

				display_console_message(string.format(_"You lost %d valuable circuits in the coin-flip game with Ewald.", math.floor(bet)))
			elseif (win == 0.5) then -- COMPLETE LOSS (0% back)
				npc_says_random(_"You lost. You were half right, but half wrong. Pick one and stay with it.",
					_"You lost. You were all messed up. Better luck next time.",
					_"You lost. Maybe you should try something else for a while?",
					_"You lost.",
					_"You lost. You might try the dice game next time.")
				bet = -bet

				display_console_message(string.format(_"You lost %d valuable circuits in the coin-flip game with Ewald.", bet))
			else -- ERROR:
				npc_says(_"THIS IS AN ERROR! REPORT IT PLEASE! E-MAIL: freedroid-discussion@lists.sourceforge.net")
			end

			if (not running_benchmark()) then -- dialog validator reports errors here sometimes when "bet" > gold we have
				Tux:add_gold(bet)
			end

			total_bet = total_bet + bet

			next("node60")
		end,
	},
	{
		id = "node60",
		text = _"BUG, REPORT ME! ewald node56 -- Reintialize variable",
		echo_text = false,
		code = function()
			-- ReInitialize variables here
			tux1, tux2, tux3 = 0, 0, 0
			ewd1, ewd2, ewd3, ewd4, ewd5 = 0, 0, 0, 0, 0

			show("node61", "node62", "node63", "node64", "node69")
		end,
	},
	{
		id = "node61",
		text = _"Let us start again with the same bet.",
		topic = "Gambling",
		code = function()
			npc_says(_"Sure.")
			local max = Tux:get_gold()
			if (max > 4 * next_bet) then
				bet = next_bet
				next("node" .. next_gamble_node)
			else
				npc_says(_"You do not have enough bucks to bet the same.")
				next("node35")
			end
			hide("node61", "node62", "node63", "node64", "node69")
		end,
	},
	{
		id = "node62",
		text = _"I prefer to bet another sum.",
		topic = "Gambling",
		code = function()
			hide("node61", "node62", "node63", "node64") next("node35")
		end,
	},
	{
		id = "node63",
		text = _"What other games are you playing?",
		topic = "Gambling",
		code = function()
			npc_says(_"I play just a dice game and a coin-flipping game.")
			hide("node61", "node62", "node63", "node63", "node64") show("node31", "node32", "node33", "node34")
		end,
	},
	{
		id = "node64",
		text = _"What did I win or lose?",
		topic = "Gambling",
		code = function()
			if (total_bet > 0) then
				npc_says(_"You have won %d circuits.", total_bet)
			else
				npc_says(_"You have lost %d circuits.", -total_bet)
			end
		end,
	},
	{
		id = "node69",
		text = _"I'll stop gambling.",
		topic = "Gambling",
		code = function()
			npc_says_random(_"We can play again whenever you want!",
				_"You are right, I'm going to clean you out!",
				_"The house always wins; sooner or later...")
			-- end any partial gambling node
			hide("node31", "node32", "node33", "node34") -- Choose game
			hide("node35", "node36", "node36", "node37", "node38", "node39") -- Choose bet
			hide("node40", "node41", "node43", "node44") -- Dice game
			hide("node50", "node51", "node52", "node53", "node54", "node55", "node56") -- Coin-flipping game
			hide("node60", "node61", "node62", "node63", "node63", "node64") -- After game
			pop_topic()
		end,
	},
	{
		id = "node99",
		text = _"See you later.",
		code = function()
			npc_says_random(_"Goodbye and remember: My bar is always open.",
				_"Come back soon!",
				_"You are welcome to unwind with old Ewald any time. Come back soon.",
				_"Remember, you can always come back here to relax and kick back.")
			end_dialog()
		end,
	},
}
