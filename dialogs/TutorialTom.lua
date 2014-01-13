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

-- helper functions for the quickmenu thingy

local function tutorial_chests_and_armor_and_shops()
		tut_tux_items_entered = true
		tut_tux_chest_entered = true
end

local function tutorial_melee_combat()
	tutorial_chests_and_armor_and_shops()

	tut_tux_melee_entered = true
	add_item("Normal Jacket")
	add_item("Improvised Buckler")
	add_item("Shoes")
	add_item("Worker Helmet")
end

local function tutorial_abilities()
	tutorial_melee_combat()

	tut_tux_terminal_entered = true
end

local function tutorial_upgrade_items_and_terminal()
	tutorial_abilities()
end

local function tutorial_hacking()
	tutorial_upgrade_items_and_terminal()

	tut_tux_takeover_entered = true
end

local function tutorial_ranged_combat()
	tutorial_hacking()

	tut_tux_glass_entered = true
end

-- actual dialog

return {
	FirstTime = function()
		-- initialize
		TutorialTom_doors = {"TutorialEntry", "TutorialZigzag1", "TutorialZigzag2", "TutorialZigzag3", "TutorialZigzag4"}
		TutorialTom_doors2 = {TutorialTom_doors[1], TutorialTom_doors[2], TutorialTom_doors[3], TutorialTom_doors[4], TutorialTom_doors[5], "TutorialEquipOut", "TutorialMeleeOut", "TutorialDoor", "TutorialGlasswallDoor", "TutorialStorage", "TutorialTakeover", "TutorialExit1"}
		items_up = {"Entropy Inverter", "Tachyon Condensator", "Antimatter-Matter Converter", "Superconducting Relay Unit", "Plasma Transistor"}
		ranged_combat = {"Normal Jacket", "Big kitchen knife", "Shoes", "Standard Shield", "Worker Helmet"}
		-- to avoid excessive map validator waypoint errors, map starts with all relevant doors open, so we need to close them here
		for var in ipairs(TutorialTom_doors2) do
			change_obstacle_state(TutorialTom_doors2[var], "closed")
		end
		-- we also need remove all tux skills, except use weapon
		downgrade_program("Hacking")
		downgrade_program("Repair equipment")
		downgrade_program("Emergency shutdown")
		-- done initializing
		set_bot_name(_"Tutorial Tom")
		display_console_message(_"Met [b]Tutorial Tom[/b]!")
	end,

	EveryTime = function()
		if (TutorialTom_start_chat) then --initiated by TutorialTerminal.dialog
			npc_says(_"Well done.")
			npc_says(_"Now follow me to the next section. You will learn how to hack bots.")
			TutorialTom_start_chat = false -- don't show this again
			teleport_npc("TutorialTomPostTerminalTeleportTarget")
			set_bot_destination("TutorialTom-Takeover")
			change_obstacle_state("TutorialTakeover", "opened") --door
			end_dialog()
		end

		if (partner_started()) and
		   (TutorialTom_glasswall_smashed) and
		   (cmp_obstacle_state("TutorialWall", "broken")) and
		   (not TutorialTom_glasswall_done) then
			-- wall section, shown if tux destroyed the wall by hand
			npc_says(_"Great, you figured out how deal with this obstacle.")
			npc_says(_"Let's proceed.")
			TutorialTom_glasswall_done = true
			if (not TutorialTom_Tux_PastWall) then
				TutorialTom_Tux_PastWall = true
				set_bot_destination("TutorialTom-Ranged", "TutorialTom")
			end
			end_dialog()
		end

		if (not cmp_obstacle_state("TutorialWall", "broken")) then
			--to avoid regressions with code above
			if (partner_started()) then
				npc_says(_"Hi Linarian, I've been expecting you. My name is Tutorial Tom.")
				npc_says(_"Long stasis sleep unfortunately has the side effect of temporary memory loss, sometimes even for simple ordinary every day things.")
				npc_says(_"So let's start with the basics, shall we?")
				show("node1", "node7", "node80")
				set_rush_tux(0)
			elseif (has_quest("Tutorial Movement")) and
				   (not TutorialTom_talk_to_tom_EveryTime) then
				-- only once
				TutorialTom_talk_to_tom_EveryTime = true
				npc_says(_"Glad to see you figured out the quest log!")
			elseif (has_quest("Tutorial Movement")) and
			       (TutorialTom_move_black) and
			       (not TutorialTom_sprinting) then
				TutorialTom_sprinting = true
				npc_says(_"Speaking of moment, you can also [b]sprint[/b] by holding the [b]control key[/b].")
				update_quest(_"Tutorial Movement", _"Apparently I can sprint if I hold down the control key as I move. This might help if I get in a tight spot.")
				npc_says(_"This distance is determined by your yellow stamina bar; the more you have, the farther you can run without becoming winded.")
			end
		end

		if (tut_tux_items_entered) and
		   (not TutorialTom_Tux_found_all_items) then
			show("node5")
		end

		if (not tut_tux_melee_entered ) then

			if (tut_tux_chest_entered) and
			   (not tut_item_chest_opened) then
				show("node10")
			elseif (tut_item_chest_opened) then
				hide("node10")
				show("node11")
			end

			if (has_item("Normal Jacket")) or
			   (has_item("Improvised Buckler")) or
			   (has_item("Shoes")) or
			   (has_item("Worker Helmet")) then
				hide("node11")
				npc_says(_"Excellent, I see you already took on the armor that was in the box.")
				if (not armor_node_one) then
					show("node12")
				end
				if (not armor_node_two) then
					show("node13")
				end
			end
		end

		if (tut_tux_melee_entered) and
		   (not tut_tux_glass_entered) then
			-- tux is still in bot melee area
			show("node24")
		end

		if (tut_tux_terminal_entered) then
			hide("node24")
		end

		if (cmp_obstacle_state("TutorialDoor", "opened")) and
		   (tut_tux_takeover_entered) and
		   (not TutorialTom_node50_done) then
			show("node50")
		end

		if (tut_tux_glass_entered) then
			show("node70")
		end

		if (TutorialTom_Tux_PastWall) then
			hide("node70") show("node30")
		end

		if (done_quest("Tutorial Hacking")) and
		   (not tut_tux_glass_entered) and
		   (not TutorialTom_hide_71) then
			show("node71")
		else
			hide("node71")
		end

		if (tut_tux_glass_entered) then
			hide("node55", "node57")
		end

		if (tut_tux_ranged_entered) and
		   (not tux_node38_done) then
			show("node38")
		end

		if (has_quest("Tutorial Shooting")) then
			hide("node30") --avoid assigning the quest twice etc..
		end

		show("node99")
	end,

	{
		id = "node1",
		text = _"Yes, it would be most kind of you. I don't remember anything.",
		code = function()
			npc_says(_"You Linarians keep a quest [b]Log[/b] where you write down key bits of information.")
			npc_says(_"When you aren't speaking with someone, you can open it up or close it by pressing the [b]q key[/b].")
			npc_says(_"I'll assign you a new quest, so you can try it out. Once you are done, [b]left click[/b] on me.")
			if (not has_quest("Tutorial Movement")) then
				add_quest(_"Tutorial Movement", _"To talk to someone, left click on them. You can talk to any friendly bot or person. Friendly people and bots have a green bar above them. Press the 'q' key to open or close the quest log. Left click back on Tutorial Tom to learn more!")
			end
			display_big_message(_"Press 'q' for Quests!")
			hide("node1", "node7", "node80") show("node2", "node3", "node4", "node8")
			if (tut_tux_items_entered) and
			   (not TutorialTom_Tux_found_all_items) then
				show("node5")
			end
			end_dialog()
		end,
	},
	{
		id = "node2",
		text = _"What can you tell me about finding my way in the world?",
		code = function()
			npc_says(_"You Linarians see the world similar to the Heads Up Displays, or [b]HUD[/b], used in computer games.")
			npc_says(_"Linarians, like all birds, have a remarkable internal sense of direction. Apparently your HUD has a [b]compass[/b], represented by four red arrows in the upper right corner with North marked on it.")
			npc_says(_"You can temporarily disable this compass by pressing the 'tab' key.")
			update_quest(_"Tutorial Movement", _"I learned the four red arrows in the upper-right of my HUD is a compass with north marked on it. I can toggle it using the 'tab' key.")
			npc_says(_"Rumor is there was a device Linarians used to remember every place they had been, called an [b]automap[/b].")
			npc_says(_"Although, you don't currently have such a device, you might find someone with nanotechnology skills to make one.")
			tux_says(_"I'll keep that in mind.")
			hide("node2")
		end,
	},
	{
		id = "node3",
		text = _"My limbs don't seem to want to work properly. Can you help me with that?",
		code = function()
			npc_says(_"Ah yes, movement. It's second nature to us humans, but our research with you in the past showed that your brain seems to work more like a computer.")
			npc_says(_"To move somewhere, you first need to determine where you want to go by [b]left clicking[/b] where you want to go on your HUD.")
			npc_says(_"If you can go there, you will do so.")
			npc_says(_"Now try to move to the red dot to the north. Then to the black grating to the east, and then talk to me again.")
			update_quest(_"Tutorial Movement", _"I'm supposed to left click anywhere to move to a location. If I can figure out a way, I will move there automatically. Tom wants me to first try to move to the red dot to the North.")
			TutorialTom_learn_to_walk = true
			hide("node3")
			end_dialog()
		end,
	},
	{
		id = "node4",
		text = _"What else makes me different as a Linarian?",
		code = function()
			npc_says(_"As a Linarian, you have some special abilities that no human does: Linarians can translate programming source code into something like magic.")
			npc_says(_"However, using these [b]Programs[/b], adversely affect your body temperature, which you must regulate.")
			npc_says(_"But you normally have a special program called [b]Emergency Shutdown[/b], which freezes your motions for several seconds but significantly cools your body.")
			npc_says(_"This, and the ability to hack bots will soon return once your mind recovers from the stasis.")
			npc_says(_"Once it does, we'll talk more about programs. You can toggle the [b]Skills/Program menu[/b] by pressing the [b]s key[/b].")
			update_quest(_"Tutorial Movement", _"View the Skills/Program menu by pressing the 's' key.")
			hide("node4")
		end,
	},
	{
		id = "node5",
		text = _"I'm sensing some sort of strange presence in the vicinity.",
		code = function()
			npc_says(_"Ah, yes.", "NO_WAIT")
			npc_says(_"Linarians seem to [b]detect items[/b] - even through walls - in an augmented reality.")
			npc_says(_"The [b]z key[/b] will toggles this augmentation, while the [b]x key[/b] momentarily flashes it.")
			npc_says(_"Moving your pointer over an item will tell you more about that item.")
			npc_says(_"On our way to the next stopping point, we'll pass some items on the floor.")
			npc_says(_"Feel free to stop and examine them to get practice.")
			npc_says(_"[b]Left clicking[/b] on an item will pick it up and put it in your inventory, toggled by the [b]i key[/b], where you can examine it further.")
			if (not TutorialTom_TutMovement_ToggleDetectItems) and
			    has_quest("Tutorial Movement") then
				update_quest(_"Tutorial Movement", _"Toggle detect items by pressing the 'z' key, and press 'x' to flash the ability. To pick up an item, left click on it. To view an item you've picked up, open the inventory by pressing 'i'.")
				TutorialTom_TutMovement_ToggleDetectItems = true
			end
			if (has_item("Mug")) and
			   (has_item("Anti-grav Pod for Droids")) and
			   (has_item("Plasma Transistor")) then
				-- these are the items in the room item
				npc_says(_"Oh great, I see you found all the items hidden in the room. Good job!")
				TutorialTom_Tux_found_all_items = true
			end
			hide("node5")
		end,
	},
	{
		id = "node7",
		text = _"No, thanks, I still remember how to walk, run, navigate, and pick things up.",
		code = function()
			hide("node1", "node7", "node80") next("node9")
		end,
	},
	{
		id = "node8",
		text = _"Thank you, that was very informative. I think I got how to move around. Can we proceed now?",
		code = function()
			end_quest(_"Tutorial Movement")
			npc_says(_"I just closed your first quest. You can still see what you learned by clicking on the 'done quests' tab in the quest screen.", "NO_WAIT")
			hide("node8") next("node9")
		end,
	},
	{
		id = "node9",
		text = _"I'm eager to learn more.",
		code = function()
			npc_says(_"Ok, follow me into the next area.")
			for var in ipairs(TutorialTom_doors) do
				change_obstacle_state(TutorialTom_doors[var], "opened")
			end
			hide("node1", "node2", "node3", "node4", "node5")
			end_dialog()
		end,
	},
	{
		id = "node10",
		text = _"What is in that chest?",
		code = function()
			npc_says(_"Well, you can interact with several types of objects by simply [b]left clicking[/b] on them.")
			npc_says(_"Left click on the chest to open it up and see what is inside.")
			if (not has_quest("Tutorial Melee")) then
				add_quest(_"Tutorial Melee", _"I'm supposed to left click on chests to open them up.")
			end
			hide("node10") show("node19")
			end_dialog()
		end,
	},
	{
		id = "node11",
		text = _"Some items came out.",
		code = function()
			npc_says(_"Well, you've discovered a great secret: chests sometimes contain items.")
			npc_says(_"We humans have a fascination with hiding our treasures. What comprises a treasure means different things to different people.")
			npc_says(_"For some, it's valuable metals in robotic circuitry. That's the current monetary standard on our world.")
			npc_says(_"For others, it may be weapons, or even just items that may be used as weapons in a pinch.")
			npc_says(_"Still others, odd as it may sound, keep dishes hidden away.")
			npc_says(_"Perhaps the best thing you can find, though, is armor. At least, given the current situation with the bots, that is.")
			hide("node11") show("node12", "node13")
		end,
	},
	{
		-- armor_node_one
		id = "node12",
		text = _"Armor? I should know more about this.",
		code = function()
			npc_says(_"In the years since you entered stasis, mankind has done much research in the field of robotics.")
			npc_says(_"One branch in particular that has served us well is the realm of nanotechnology.")
			npc_says(_"Because they run a very simple operating system, nanobots were not affected by whatever caused the Great Assault.")
			npc_says(_"To this day, our armor and clothing are made out of nanobots. This enables many different sizes of people to wear the same clothing.")
			npc_says(_"The nanobots reshape the garment for a comfortable fit, so even you should be able to wear them without problem.")
			hide("node12") show("node14")
		end,
	},
	{
		-- armor_node_two
		id = "node13",
		text = _"How does wearing clothes and holding shields help me?",
		code = function()
			npc_says(_"Well, it can help you avoid indecent exposure charges.")
			tux_says("...")
			npc_says(_"But all jokes aside, the bots will not mess around.")
			npc_says(_"Even the best of fighters sometimes get hit by enemy blows. If I were you, I would seriously consider wearing some sort of armor before mixing it up with those bots.")
			npc_says(_"They will help to mitigate some of the damage you take from melee or ranged combat.")
			npc_says(_"Keep in mind, though, that there is a chance that your armor will take damage whenever you get hit. Keep an eye on it, or you'll lose it.")
			hide("node13") show("node15")
		end,
	},
	{
		id = "node14",
		text = _"That sounds useful. How do I use armor?",
		code = function()
			npc_says(_"Well, let me start by saying that there are four classes of armor: headgear, shoes, body armor, and shields.")
			npc_says(_"When you pick up a piece of armor of a particular class, and you don't already have one equipped, you will automatically equip it if you can.")
			npc_says(_"Picking up armor and items can be accomplished by [b]left clicking[/b] on the item. If your inventory has space, you will pick the item up.")
			npc_says(_"If you currently have one piece of armor equipped that you would like to swap out for another, simply drag the new armor piece to the current armor piece and left click.")
			npc_says(_"They will swap places, and you can put the other piece of armor back into your inventory.")
			npc_says(_"Alternatively, you can drag it into your field of view and drop it on the ground.")
			hide("node14") show("node16")
		end,
	},
	{
		id = "node15",
		text = _"I wouldn't want to lose any armor. What can I do about that?",
		code = function()
			npc_says(_"Well, there are several ways to keep that from happening.")
			npc_says(_"Equipped items in poor condition will show up yellow and should turn red if in critical condition.")
			npc_says(_"If you don't unequip an item in poor or critical condition it may be ruined!")
			npc_says(_"There are three things you can do when an item reaches critical condition:")
			npc_says(_"First, you can sell or discard the item.")
			npc_says(_"Second, you can use your [b]repair equipment[/b] program on the item, which will repair the item at the cost of some of its durability.")
			npc_says(_"Or third, you can have the item repaired at a shop, which maintains the item's durability at a price.")
			hide("node15") show("node17", "node61")
		end,
	},
	{
		id = "node16",
		text = _"You mentioned discarding an item. Why would I want to do that?",
		code = function()
			npc_says(_"Well, I can think of a few reasons.")
			npc_says(_"You might not have enough room in your inventory for an item you need more, or can sell for more.")
			npc_says(_"If an item is close to being destroyed, you might also consider dropping it.")
			npc_says(_"And finally some items give negative status effects, and aren't worth selling.")
			npc_says(_"Armor or weapons may affect you in different ways.")
			npc_says(_"Some will help you out, for example by increasing your cooling rate or increasing your dexterity. This is the work of specialized nanobots, but the reverse can happen too.")
			hide("node16")
			armor_node_two = true
			if (armor_node_one) then
				show("node19")
			end
		end,
	},
	{
		id = "node17",
		text = _"Repair equipment program? How do I use that?",
		code = function()
			npc_says(_"Perhaps I should start by explaining programs in general.")
			npc_says(_"To use a program, select it through your HUD (remember the [b]s key[/b] toggles your [b]Skills/Program menu[/b]).")
			npc_says(_"After you've selected a program, run the program by right clicking. Depending on the program you can target an enemy, an item, or just the world.")
			npc_says(_"You can also assign quick-select keys to programs. Simply hover over the program in the Skills/Program menu and press one of the keys [b]F5[/b] to [b]F12[/b]. Then, whenever you press this button, you will select the corresponding program.")
			npc_says(_"If you're curious what a program does, you can find out by clicking the question mark in the Skills/Program menu.")
			npc_says(_"Now, to answer your question, you use your repair equipment program by selecting it in the Skills/Program menu, then right clicking on the item you wish to repair.")
			improve_program("Repair equipment")
			TutorialTom_has_repair = true
			if (not has_quest("Tutorial Melee")) then -- this might be tricky
				add_quest(_"Tutorial Melee", _"Run a program by right clicking after it has been selected from the Skills/Program menu. My shoes are damaged, so I might try the repair equipment program on them to slightly repair them.")
			else
				update_quest(_"Tutorial Melee", _"Run a program by right clicking after it has been selected from the Skills/Program menu. My shoes are damaged, so I might try the repair equipment program on them to slightly repair them.")
			end
			hide("node17")
		end,
	},
	{
		id = "node19",
		text = _"I think I'm ready for the next section of the tutorial.",
		code = function()
			npc_says(_"Ok, I'll open the next door then. But prepare yourself, soon you will learn how to fight against bots in hand-to-hand combat.")
			if (not TutorialTom_has_repair) then
				improve_program("Repair equipment")
			end
			change_obstacle_state("TutorialEquipOut", "opened")
			end_dialog()
			hide("node10", "node11", "node12", "node13", "node14", "node15", "node16", "node17", "node61", "node62", "node63", "node64", "node65", "node19") show("node20")
		end,
	},
	{
		id = "node20",
		text = _"Can you tell me how to fight the bots?",
		code = function()
			npc_says(_"I will certainly try my best.")
			npc_says(_"It's odd to think that a flightless bird would be such a scrapper, but you Linarians have been surprising us since the beginning.")
			npc_says(_"We prefer to fight the blamed droids at a distance. They're less likely to rend us limb from limb that way.")
			npc_says(_"Unfortunately, guns and other ranged weaponry are hard to come by these days.")
			npc_says(_"That goes double for ammunition. Because these are bots, you really can't effectively keep them at bay by just waving around an empty gun.")
			npc_says(_"With that in mind, your most reliable weapons are your fists. They never run out of ammo or wear down, but of course they don't do much damage against a metal body, either.")
			hide("node20") show("node21", "node23")
		end,
	},
	{
		id = "node21",
		text = _"So I'm supposed to fight bots with my fists?",
		code = function()
			npc_says(_"There may be times when you have no choice.")
			npc_says(_"Melee combat, armed or unarmed, can be initiated by [b]left clicking[/b] on an enemy.")
			npc_says(_"You will then walk over to the enemy and start hitting them.")
			npc_says(_"Now, I understand you may have some concerns over your safety during this...")
			tux_says(_"CONCERNS? Erm... Yes, this doesn't sound like a particularly healthy endeavor.")
			npc_says(_"Fred, if you're afraid, you'll have to overlook it. Besides, you knew the job was dangerous when you took it.")
			tux_says(_"I beg your pardon? Who is Fred?")
			npc_says(_"Never mind, just an old song.")
			npc_says(_"Fighting the bots hand-to-hand has risks, there is no question. However, there are things you can do to even the odds.")
			npc_says(_"We already discussed armor. Another useful type of item is medical supplies.")
			npc_says(_"There are a few different types of restorative items. The most common are health drinks.")
			hide("node21") show("node22")
		end,
	},
	{
		id = "node22",
		text = _"Tell me about health drinks.",
		code = function()
			npc_says(_"Man has practiced medicine in one form or another for hundreds of years.")
			npc_says(_"Using some of the same nanotechnology that makes up your armor, we've perfected drinks that instantaneously restore your health.")
			npc_says(_"There are many kinds, but all accomplish the same thing, to varying degrees.")
			npc_says(_"Certain items, like health drinks, can be used directly without clicking on them.")
			npc_says(_"At the bottom of your HUD, you'll notice your item access belt. Small items can go here and be used by pressing the numbers [b]0[/b] through [b]9[/b].")
			npc_says(_"Unlike armor or weapons, multiple of these items will group together in your inventory slots.")
			npc_says(_"If you have Diet Supplements equipped in item slot 1, you can press 1 to use one.")
			npc_says(_"This will restore your health, and will be invaluable in the heat of battle.")
			update_quest(_"Tutorial Melee", _"Small one-time-use items, like diet supplements, can be placed in inventory spots labels 1 to 0, and can be used by pressing the corresponding key.")
			hide("node22")
		end,
	},
	{
		id = "node23",
		text = _"I'm ready to fight my first bot.",
		code = function()
			npc_says(_"We keep two bots captive for melee practice.")
			npc_says(_"I've unlocked the door leading to the first bot. Take care of it, then come back and talk to me.")
			npc_says(_"I'm also giving you some health drinks. Feel free to use them, though I can heal you if you need it.")
			npc_says(_"Your health will also regenerate over time. I wish it were so easy for us humans.")
			npc_says(_"The first bot is carrying an [b]Entropy Inverter[/b]. Bring it to me to prove you've beaten it.")
			npc_says(_"Come back and talk to me when you have the Entropy Inverter.")
			change_obstacle_state("TutorialMelee1", "opened")
			add_item("Doc-in-a-can", 3)
			update_quest(_"Tutorial Melee", _"I'm going to fight my first bot! To start melee combat left click on the bot. Tom wants the Entropy Inverter from the bot after I've defeated it, so I should pick that up.")
			hide("node23") show("node24", "node25")
			end_dialog()
		end,
	},
	{
		id = "node24",
		text = _"Can you heal me?",
		code = function()
			if (tux_hp_ratio() == 1) then
				npc_says(_"You are fine, there is nothing that I can do.")
				hide("node24")
			else
				npc_says(_"You should be more careful, Linarian. But yes, I can heal you.")
				npc_says(_"There, all better.")
				heal_tux()
			end
		end,
	},
	{
		id = "node25",
		text = _"I've beaten the first bot.",
		code = function()
			if (has_item_backpack("Entropy Inverter")) then
				npc_says(_"So you have. Well done.")
				npc_says(_"I'll exchange this wrench for it. I think you'll find it handy.")
				npc_says(_"Now would be an appropriate time to mention bot parts, I guess.")
				npc_says(_"Certain parts in the bots are extra valuable. You can sell them at stores for cash.")
				npc_says(_"If you can learn to extract these parts, it can help improve your lot in the world.")
				npc_says(_"Hopefully, you can find someone to teach you.")
				del_item_backpack("Entropy Inverter", 1)
				add_item("Big wrench", 1)
				update_quest(_"Tutorial Melee", _"I brought the entropy inverter to Tom and he gave me my first weapon: a Big wrench. I should equip it.")
				hide("node25") show("node26")
			else
				npc_says(_"Trying to cheat your way through the tutorial doesn't bode well for your chances in the real world.")
				npc_says(_"I'll need proof that you've beaten the first bot.")
				npc_says(_"Don't come back without the [b]Entropy Inverter[/b] it was carrying.")
				end_dialog()
			end
		end,
	},
	{
		id = "node26",
		text = _"You mentioned another bot?",
		code = function()
			npc_says(_"That's correct. The second bot is a lot more dangerous, and you're not likely to make much headway against it with your fists.")
			npc_says(_"Lucky for you, there's another dimension to melee combat.")
			npc_says(_"Fighting bots with your fists is fine, but there are a number of items you can use for melee.")
			npc_says(_"There are several traditional weapons in the world such as swords or light sabers that can be used against the bots.")
			npc_says(_"However, more common items you encounter everyday can often be used to bash or pry at droid armor.")
			npc_says(_"For example, that wrench I just gave you will increase the damage you do, and should let you best the next bot.")
			hide("node26") show("node27")
		end,
	},
	{
		id = "node27",
		text = _"I'm ready to take the next bot on.",
		code = function()
			npc_says(_"I certainly hope so. I will heal you before you go to the next room.")
			npc_says(_"I'm also giving you another healing drink, just in case.")
			npc_says(_"A 247 is nothing to trifle with, so I'm giving you a shield in the hopes that it'll help keep you alive.")
			npc_says(_"Finally, I'm giving you couple of somewhat experimental pills that will temporarily increase your strength and dexterity, respectively.")
			npc_says(_"Use them in the same way you would use a health drink.")
			npc_says(_"If I were you, I would think about using your repair equipment program on your armor and that wrench before you go into the room.")
			npc_says(_"You'll also need to equip them. I won't do that for you.")
			npc_says(_"Like the last bot, this one will be carrying an item. Bring me the [b]Tachyon Condensator[/b] it is carrying to continue.")
			npc_says(_"I've unlocked the door. Good luck, and talk to me when you have the Tachyon Condensator.")
			change_obstacle_state("TutorialMelee2", "opened")
			heal_tux()
			add_item("Doc-in-a-can", 1)
			add_item("Strength Capsule", 1)
			add_item("Dexterity Capsule", 1)
			add_item("Standard Shield", 1)
			hide("node27") show("node28")
			end_dialog()
		end,
	},
	{
		id = "node28",
		text = _"I've beaten the second bot.",
		code = function()
			if (has_item_backpack("Tachyon Condensator")) then
				npc_says(_"Very good! You've completed your melee training.")
				npc_says(_"These skills will serve you well in the future.")
				npc_says(_"Talk to me when you're ready to move on to the next part of the tutorial.")
				del_item_backpack("Tachyon Condensator", 1)
				hide("node28") show("node29")
			else
				npc_says(_"You didn't bring back the Tachyon Condensator. You'll need to bring it back before we can continue.")
				npc_says(_"I'll go ahead and heal you. If your weapon or armor are in bad shape, perhaps you should repair them before going back in there.")
				npc_says(_"Don't come back without the [b]Tachyon Condensator[/b].")
				end_dialog()
			end
			heal_tux()
		end,
	},
	{
		id = "node29",
		text = _"I'm ready to move on.",
		code = function()
			if (not done_quest("Tutorial Melee")) then
				end_quest(_"Tutorial Melee", _"I decided to move on and go to next unit of the tutorial.")
			end
			--[[ if (TutorialTom_has_gun) then
			npc_says(_"I'll bet you're itching to try that pistol out on some bots.")
			npc_says(_"Our next destination is the ranged combat training area, so you're in luck.")
			end ]]--
			npc_says(_"I'll unlock the door to the south. Follow the corridor east.")
			change_obstacle_state("TutorialMeleeOut", "opened")
			set_bot_destination("TutorialTom-Terminal")
			hide("node20", "node21", "node22", "node23", "node24", "node25", "node26", "node27", "node28", "node29")
			show("node40")
			end_dialog()
		end,
	},
	{
		id = "node30",
		text = _"Ranged weaponry seems safer than getting up close and personal.",
		code = function()
			npc_says(_"Well, Linarian, most of the time it is.")
			npc_says(_"There are some bots with ranged weaponry, but we don't have any of those in captivity.")
			add_quest(_"Tutorial Shooting", _"I am learning about how to use ranged weapons.")
			hide("node30") show("node31", "node38")
		end,
	},
	{
		id = "node31",
		text = _"So what can you tell me about guns?",
		code = function()
			npc_says(_"The basic idea behind ranged weaponry is to hurt or kill an enemy without having to come within swinging range.")
			npc_says(_"There are three basic ways to do this: traditional firearms, lasers, and plasma weapons.")
			npc_says(_"Traditional firearms are things that go bang. A small explosion accelerates the ammunition towards the target at a high velocity, causing injury.")
			npc_says(_"Lasers use an intensified beam of light to burn through the intended victim. These operate on replaceable charge packs.")
			npc_says(_"Plasma weapons use superheated ionized gas to burn the target. The gas comes in loadable canisters.")
			npc_says(_"Most other ranged weapons are variations of these three. We have a few in storage for training purposes.")
			hide("node31") show("node32")
		end,
	},
	{
		id = "node32",
		text = _"So how do I use guns?",
		code = function()
			npc_says(_"Well, firing is pretty simple. First, equip a ranged weapon that you have ammunition for.")
			npc_says(_"Next, simply [b]left click[/b] on your target, and you'll begin firing.")
			npc_says(_"Alternatively, you can fire a single shot using the [b]Fire Weapon[/b] program, and [b]right clicking[/b] on the enemy.", "NO_WAIT")
			npc_says(_"That is the best way if you are trying to conserve precious ammo with high rate of fire guns.")
			npc_says(_"There are a couple of new tricks with ranged weapons. If you want to [b]move while firing[/b]: hold [b]shift[/b] and [b]left click[/b] your desired position.")
			npc_says(_"If you'd rather stay in place: Hold [b]A[/b], then [b]left click[/b] your target.")
			npc_says(_"When your weapon runs out of ammo during combat, it will try to automatically reload if you have extra ammunition for it.")
			npc_says(_"Sometimes, you will need or want to manually reload. You can do this by pressing the [b]r key[/b].")
			npc_says(_"If you are reloading, or you are out of ammo, the small message screen at the bottom of your HUD will tell you what type of ammo to get.")
			if (not TutorialTom_guns_how_to) then
				TutorialTom_guns_how_to = true
				update_quest(_"Tutorial Shooting", _"If I have equipped a ranged weapon and I left click on an enemy, I will begin firing upon it until my gun is empty, and automatically reload. But if I want to fire a single shot, I need to also equip the Fire Weapon program, and then right click. To move while firing, I have to shift + left click to where I want to go. Also, if I want to manually reload I have to press the 'r' key. The small message screen at the bottom of my HUD will tell me if I'm reloading or out of ammunition.")
			end
			if (not TutorialTom_opened_door) then
				show("node33")
			end
			hide("node32")
		end,
	},
	{
		id = "node33",
		text = _"I think I understand how to use guns now.",
		code = function()
			npc_says(_"In that case, I'll let you in to the armory. There are some crates and chests in there with weapons and ammunition.")
			npc_says(_"Be careful though, smashing open crates and barrels could hurt you.")
			if (TutorialTom_has_gun) then
				npc_says(_"You already got the .22 pistol I gave you earlier? It's not much as guns go, but it can do the trick.")
			else
				npc_says(_"Here's my personal .22 Automatic pistol. It's not much as guns go, but it can do the trick.")
				add_item(".22 Automatic", 1)
				add_item(".22 LR Ammunition", 10)
			end
			npc_says(_"Here is some more ammunition for it.")
			npc_says(_"You'll no doubt get more mileage out of the plasma pistol, though. Or any other gun, for that matter.")
			npc_says(_"Practice switching weapons and reloading.")
			npc_says(_"On the other side of that fence are some bots. Practice killing them off with various ranged weapons.")
			npc_says(_"Once you've gotten rid of them all, we can move on.")
			update_quest(_"Tutorial Shooting", _"Tutorial Tom wants me to shoot those bots.")
			-- TODO probably explain leveling as well as balancing changes to XP always risk causing leveling here. Or somehow make sure Tux can't level
			TutorialTom_opened_door = true
			change_obstacle_state("TutorialStorage", "opened")
			change_obstacle_state("TutorialShootCage", "opened")
			add_item(".22 LR Ammunition", 50)
			hide("node33") show("node34", "node35")
			end_dialog()
		end,
	},
	{
		id = "node34",
		text = _"I ran out of ammo. Can I have some more?",
		code = function()
			if (cmp_obstacle_state("37-ammo-chest", "unlocked")) then -- check if tux has opened the ammo chest already : yes
				if (has_item_backpack("Laser power pack")) or (has_item_backpack("Plasma energy container")) or (has_item_backpack(".22 LR Ammunition")) then
					-- check if tux still has ammo left
					npc_says(_"Looks like you still have some ammo left in your inventory...")
					npc_says(_"Why don't you use this first?")
					npc_says(_"Try to switch to another gun maybe...")
				else -- chest opened but no ammo left in inventory...
					npc_says(_"Ok, I will give you some more ammo. You seem to be in need, indeed...")
					add_item("Laser power pack", 10)
					add_item("Plasma energy container", 10)
					add_item(".22 LR Ammunition", 10)
				end
			else
				npc_says(_"Check the storage room to the west for more guns and ammo.")
			end
		end,
	},
	{
		id = "node35",
		text = _"I killed all the bots.",
		code = function()
			if (done_quest("Tutorial Shooting")) then
				npc_says(_"Excellent job.")
				npc_says(_"Keep shooting like that, and the bots will never stand a chance, as long as you still have ammo.")
				update_quest(_"Tutorial Shooting", _"That was like shooting fish in a barrel. I could use a fish about now.")
				hide("node35") show("node36", "node37")
				end_dialog()
			else
				npc_says(_"I don't mind if you don't want to do this, but please don't try to trick me.")
				npc_says(_"I don't have verified kills on all those bots.")
			end
		end,
	},
	{
		id = "node36",
		text = _"I would like a few more bots to shoot at.",
		code = function()
			npc_says(_"Ok, I'll create some more droids.")
			i = 1
			while (i < 8) do
				create_droid("shootingrange"..i, 123)
				i = i + 2
			end
			hide("node36")
			end_dialog()
		end,
	},
	{
		id = "node37",
		text = _"So, what's next?",
		code = function()
			npc_says(_"Now, there's one last thing you should know.")
			npc_says(_"You Linarians can apparently withdraw from our world temporarily.")
			npc_says(_"Time stops, and everything freezes in place.")
			npc_says(_"This can be accomplished by using the [b]Esc key[/b].")
			if (not TutorialTom_hacking_bots) then
				npc_says(_"Your final task is to leave us using this menu.")
				npc_says(_"I'm afraid there's nothing more I can teach you. I wish you good luck, Linarian.")
				npc_says(_"You'll need it if our world is to survive.")
			end
			change_obstacle_state("TutorialExit1", "opened")
			hide("node30", "node31", "node32", "node33", "node34", "node35", "node36", "node38")
		end,
	},
	{
		id = "node38",
		text = _"Why can't I hack the bots here, or use programs on them?",
		code = function()
			npc_says(_"Well, that would be cheating, don't you think so?")
			npc_says(_"Shooting moving targets needs a bit of practice, yes...")
			npc_says(_"...but in the end you will profit a lot from it.")
			tux_says(_"Ok, I will try my best.")
			tux_node38_done = true
			hide("node38")
		end,
	},
	{
		id = "node40",
		text = _"Why are we stopping?",
		code = function()
			npc_says(_"You've done well so far, Linarian. Let's take a little break.")
			npc_says(_"Once you get out into the real world, you'll find that your current abilities aren't always enough to survive.")
			npc_says(_"As you achieve combat experience, you'll be able to apply it to better yourself.")
			npc_says(_"As with most things, you view this as a panel in your HUD. Press the [b]c key[/b] to open the [b]Character panel[/b].")
			npc_says(_"Once you've gained enough experience, you'll level up. This will automatically improve some of your statistics.")
			npc_says(_"You'll also be given 5 points to spend however you choose in three different areas. Characteristics, Skills and Programs.")
			npc_says(_"[b]Strength[/b] governs the amount of damage you do with each hit in melee combat.")
			npc_says(_"[b]Dexterity[/b] controls your hit rate, or how likely you are to hit the enemy. It also determines how likely you are to dodge a hit from them.")
			npc_says(_"[b]Cooling[/b] is your processors cooling. or heat resilience, and determines how much you can process before your system starts overheating.")
			npc_says(_"Finally, among your main characteristics, [b]Physique[/b] determines how much health you have.")
			npc_says(_"There are general [b]Melee[/b] and [b]Ranged[/b] fighting skills, both of which increase the damage you inflict and how fast you attack as you improve them.")
			npc_says(_"You also have the [b]Programming[/b] skill, which essentially is about programming in an energy-efficient manner so programs cause less heat.")
			npc_says(_"Apart from this, a few programs require a teacher and training points to improve.")
			npc_says(_"[b]Extract Bot Parts[/b] is a special passive skill that will allow you to salvage parts from bots you 'retire', which you can trade for money in the town.")
			npc_says(_"A good technician is able to make use of those parts.")
			npc_says(_"He can create, let's say, an add-on and, after doing some modifications to the item, attach this add-on.")
			npc_says(_"This way an upgraded item is created.")
			add_quest(_"Tutorial Upgrading Items", _"There is a way to upgrade items if I meet a good technician. It sounds useful, maybe I should learn more about this.")
			npc_says(_"The [b]Hacking[/b] program we will speak a bit more of in a little while.")
			hide("node40") show("node41")
		end,
	},
	{
		id = "node41",
		text = _"I'll have to keep that in mind.",
		code = function()
			npc_says(_"Well, how about you try it out?")
			npc_says(_"I'll give you some points to spend, and you can distribute them as you like.")
			npc_says(_"Since you'll be hacking up ahead, you should probably use a few of them on [b]Cooling[/b].")
			npc_says(_"But, again, that's your choice.")
			add_xp(2000)
			hide("node41") show("node58")
			end_dialog()
		end,
	},
	{
		id = "node42",
		text = _"Alright, I'm ready to go hack some bots.",
		code = function()
			if (not done_quest("Tutorial Upgrading Items")) then
				end_quest(_"Tutorial Upgrading Items",_"I decided that I don't need to know how to upgrade items.")
			end
			npc_says(_"First, you'll need to unlock the door.")
			npc_says(_"See that [b]Terminal[/b] to the left?")
			npc_says(_"You will need to login, by left clicking on it, and then select unlock door.")
			npc_says(_"Terminals act very similar to hacked bots.")
			add_quest(_"Tutorial Hacking", _"I need to login to the Terminal by left clicking on it. Then I'll unlock the door, so I can go hack some bots!")
			hide("node42", "node58", "node59", "node60")
			end_dialog()
		end,
	},
	{
		id = "node50",
		text = _"So how does this work?",
		code = function()
			npc_says(_"We talked about programs before. Now, we're going to focus on hacking programs in particular.")
			npc_says(_"As I mentioned before, your brain works in a way remarkably similar to a central processing unit.")
			npc_says(_"In many ways, you interact with the world around you as a virtual environment.")
			npc_says(_"You seem to have access to an API that isn't exposed to humans. You can interact with the environment in ways we only dream of.")
			npc_says(_"Because of this, you can run various programs on the bots.")
			npc_says(_"You can attempt to take them over, or monkey with their programming, or even cause them to damage themselves.")
			npc_says(_"Likewise, there are certain programs that affect you or items around you.")
			if (not has_quest("Tutorial Hacking")) then -- quest already added in node 42
				add_quest(_"Tutorial Hacking", _"I need to login to the Terminal by left clicking on it. Then I'll unlock the door, so I can go hack some bots!")
			end
			TutorialTom_node50_done = true
			hide("node50") show("node51")
		end,
	},
	{
		id = "node51",
		text = _"Tell me about taking bots over.",
		code = function()
			npc_says(_"I'm afraid that you will have to experience it for yourself to really understand.")
		--	npc_says(_"However, you did leave yourself instructions before you went into stasis sleep.")
			npc_says(_"Taking over a bot comes in the form of a game or competition placing you against the bot.")
			npc_says(_"You initiate it by selecting the [b]Hacking[/b] program, then [b]right-clicking[/b] on a hostile bot.")
			npc_says(_"The field of play is split into two sides, one yellow, one purple, representing circuit boards.")
			npc_says(_"In the center of the field is a column of boxes known as logic cores. These cores have wires attached to them from both circuit boards.")
			npc_says(_"Your objective is to activate as many cores in your chosen color as possible at the end of each round.")
			npc_says(_"This is accomplished by selecting a wire with the up or down buttons, then pressing spacebar to activate that core.")
			npc_says(_"You will be given about 10 seconds initially to select which color you want with spacebar.")
			npc_says(_"At that point, you have 10 more seconds in which to activate more cores than the bot.")
			npc_says(_"Each activation of one core will consume one charge and you have a limited number of charges, 3 by default, so choose your targets carefully.")
			npc_says(_"I can tell you about some more advanced techniques, if you'd like.")
			if (not TutorialTom_updated_hacking_quest) then
				update_quest(_"Tutorial Hacking", _"Apparently in order to hack, all I have to do is select the Hacking program, right click a bot, and play a little game. I've got this!")
				TutorialTom_updated_hacking_quest = true
			end
			show("node52", "node53", "node54") hide("node51")
		end,
	},
	{
		id = "node52",
		text = _"What was that about activating more cores than the bot?",
		code = function()
			npc_says(_"Ah, yes. While you are busy trying to take the bot over, it will be busy trying to stop you.")
			npc_says(_"The truth is, if the bot programming was perfect, it would be almost impossible to defeat.")
			npc_says(_"Fortunately for you, software bugs do indeed exist, and therefore some cores will be on your side from the beginning.")
			npc_says(_"Likewise, the bots only have a limited number of charges. Occasionally, they will have more than you, but that is a risk you have to take.")
			npc_says(_"Because you are initiating the attack, you do get to choose your side, which can let you handicap the bot greatly if used wisely.")
			hide("node52")
		end,
	},
	{
		id = "node53",
		text = _"Tell me about more advanced techniques.",
		code = function()
			npc_says(_"Each charge will activate and remain active for a certain amount of time. This will let you take over a core that a bot just took over.")
			npc_says(_"Sometimes, the bots will already have charges on the wires. When you activate one of these, it will increase the time that wire is activated for.")
			npc_says(_"The bot can use this trick too, so it may be wise to give up a heavily guarded wire for an easier target.")
			npc_says(_"The wiring inside the bots sometimes contain splitters. These can be used in two ways.")
			npc_says(_"A 2-to-1 splitter is undesirable. This requires two charges to activate one core, halving your efficiency.")
			npc_says(_"A 1-to-2 splitter, however, will activate two nodes for the price of one charge.")
			npc_says(_"Sometimes these splitters will be stacked. Tracing the route ahead of time will be crucial to your tactical success.")
			npc_says(_"Ideally, you want to stick your opponent with 2-to-1 splitters, while you have 1-to-2 splitters at your disposal.")
			npc_says(_"Finally, remember that patience is often a virtue. Taking over all cores early on only to lose them later can be a crushing blow.")
			hide("node53")
		end,
	},
	{
		id = "node54",
		text = _"I'm ready to try hacking bots.",
		code = function()
			npc_says(_"I certainly hope so.")
			npc_says(_"I should, however, take the time to tell you a few more things.")
			npc_says(_"Hacking, like most other programs, incurs a cost to you.")
			npc_says(_"Processing programs seems to heat up your brain.")
			npc_says(_"You seem to be able to handle quite a bit of heat, but you have your limits.")
			npc_says(_"If you do not use all charges when attempting to take over a bot, you will spare some heat.")
			npc_says(_"Not activating all charges or beating the bot with a comfortable winning margin is less exhaustive for you.")
			npc_says(_"If you nevertheless accumulate too much heat, you will automatically shut down in order to cool off.")
			npc_says(_"Over time, the heat will seep away from your body naturally. However, there are ways to speed up the process.")
			npc_says(_"If you can find them, cooling drinks will quickly relieve your thermal discomfort, in much the same way healing drinks help your health.")
			npc_says(_"Remember you also have a special program called [b]Emergency Shutdown[/b] that will lower your heat level drastically. Unfortunately using Emergency Shutdown will shut you down and render you paralyzed for several seconds. It makes you a sitting duck... er, penguin.")
			tux_says(_"I take it that was a joke also?")
			npc_says(_"Quite.", "NO_WAIT")
			npc_says(_"In addition to hacking, you have the ability to execute arbitrary code on bots.")
			npc_says(_"These scripts and programs can be learned through reading source code books. To read a book, right-click on it in inventory.")
			npc_says(_"You should have recovered your hacking ability by now, and I'll give you a couple of source code books as well.")
			show("node55", "node57") hide("node54")
		end,
	},
	{
		id = "node55",
		text = _"Thanks. I'm ready now.",
		code = function()
			npc_says(_"We have several bots behind that fence for you to practice on.")
			npc_says(_"Take them over, then talk to me when they're all dead.")
			npc_says(_"If you need more help, don't hesitate to ask.")
			npc_says(_"Oh, and Linarian... If you fail when hacking, you'll receive an electrical shock.")
			npc_says(_"It shouldn't kill you, but you need to be aware of it nonetheless.")
			if (not TutorialTom_hacking_bots) then
				improve_program("Hacking")
				improve_program("Emergency shutdown")
				add_item("Liquid nitrogen", 1)
				add_item("Source Book of Calculate Pi", 1)
				add_item("Source Book of Malformed packet", 1)
				TutorialTom_hacking_bots = true
				update_quest(_"Tutorial Hacking", _"Tutorial Tom wants me to hack those bots. The only thing I need to worry about is overheating. But if I get too hot I can always try the Emergency shutdown program.")
				change_obstacle_state("TutorialTakeoverCage", "opened")
			end
			hide("node55")
			end_dialog()
			-- TODO Also explain improving program revisions
		end,
	},
	{
		id = "node57",
		text = _"Tell me again about take over, I'm not sure I got it all.",
		code = function()
			npc_says(_"Tell me exactly what part you want me to explain again.")
			npc_says(_"But things will be much clearer once you try for yourself than when I try to explain.")
			npc_says(_"Use your experience as notes, and all should be clear.")
			show("node51", "node52", "node53") hide("node57")
		end,
	},
	{
		id = "node58",
		text = _"Can you tell me more about upgrading items?",
		code = function()
			npc_says(_"You can upgrade most weapons and clothing. First, you'll need some materials to craft add-ons.")
			npc_says(_"Parts needed for this process can be extracted from bots. You'll have to learn how to do it.")
			npc_says(_"When you have enough materials, you can craft an add-on. Make sure the add-on is suitable for your equipment.")
			npc_says(_"After this, you have to make a socket in your equipment so you can plug in the add-on.")
			npc_says(_"That's the theory. Simple, isn't it?")
			npc_says(_"I'll give you some materials now and let you craft an add-on.")
			npc_says(_"Pay attention to the type of socket the add-on needs.")
			update_quest(_"Tutorial Upgrading Items", _"Before crafting an add-on I have to get some materials. The best way is to extract them from the bots, using Extract Bot Parts skill. Each add-on requires a socket in the item. When the add-on is ready, I have to create a socket in the item and plug the add-on. Only technicians can craft add-ons or plug them.")
			-- TODO decide on what type of upgrade should player make?
			--[[ for var in ipairs(items_up) do
			add_item(items_up[var], 20)
			end ]]-- FOR SOME REASON this was not working... add items one by one

			add_item("Entropy Inverter", 20)
			add_item("Tachyon Condensator", 20)
			add_item("Antimatter-Matter Converter", 20)
			add_item("Superconducting Relay Unit", 20)
			add_item("Plasma Transistor", 20)

			craft_addons()
			npc_says(_"Now you'll need to pay some more money in order to make a socket and plug in the add-on. Here, take some.")
			add_gold(1200)
			npc_says(_"Choose an item and add an appropriate socket by clicking '+' on the right of the item. Then attach the add-on to this item and confirm your choice.")
			upgrade_items()
			end_quest(_"Tutorial Upgrading Items",_"Maybe I need some time to practice this, but I got the basics.")
			hide("node58") show("node59", "node60", "node42")
		end,
	},
	{
		id = "node59",
		text = _"I wasted my materials on the add-on I didn't need. Can you give more?",
		code = function()
			npc_says(_"Yes, but only this time. I'll give you some money too. Just be more careful in the future and make wise choices.")
			npc_says(_"Reality is not as generous as I am.")
			--[[ for var in ipairs(items_up) do
			add_item(items_up[var], 20)
			end ]]--FOR SOME REASON this was not working... add items one by one

			add_item("Entropy Inverter", 20)
			add_item("Tachyon Condensator", 20)
			add_item("Antimatter-Matter Converter", 20)
			add_item("Superconducting Relay Unit", 20)
			add_item("Plasma Transistor", 20)

			add_gold(1200)
			tux_says(_"Thank you.")
			npc_says(_"Think carefully before you make your next add-on.")
			hide("node59")
		end,
	},
	{
		id = "node60",
		text = _"Upgrading items is still too complex for me. Can you explain it again?",
		code = function()
			npc_says(_"First step is finding materials. They may be extracted from bots, but you'll have to learn how to do it.")
			npc_says(_"Those materials can be used in crafting add-ons. Make sure the add-on will fit your equipment. Also, remember the type of the socket your add-on needs.")
			npc_says(_"I gave you materials. Let's craft an add-on now.")
			craft_addons()
			npc_says(_"All you have to do now is to choose a suitable item and add a socket by clicking ''+'' on the right of the chosen item.")
			npc_says(_"Remember you pay for both adding a socket and plugging an add-on.")
			npc_says(_"Let's try it, shall we?")
			upgrade_items()
			npc_says(_"I hope it's clear now.")
		end,
	},
	{
		id = "node61",
		text = _"Shops?",
		code = function()
			npc_says(_"Some people are willing to trade items and circuits with you.")
			--npc_says(_"Although, if you have too many circuits, they will charge you a bit more.") http://rb.freedroid.org/r/1327/
			npc_says(_"Most are willing to buy whatever you have, at a discounted price.")
			npc_says(_"There are also some automated vending machines, that dispense small items.") -- but these cannot buy from you.")
			hide("node61") show("node62")
		end,
	},
	{
		id = "node62",
		text = _"I would like to practice selling an item.",
		code = function()
			npc_says(_"I'm going to give you a Small Axe, which you can't use yet, so you can practice selling it to me.")
			npc_says(_"You will see the Small Axe appear in the lower shop bar. You can sell most of these items.")
			npc_says(_"Items that you have equipped have a hand icon on them.")
			npc_says(_"However, if you sell the items from the chest (which should have been equipped automatically) I won't be giving you replacements.")
			npc_says(_"Select the Small Axe and click the sell button.")
			add_item("Small Tutorial Axe")
			i = 1
			while (i < 100) do -- this is a ugly hack to get rid of the "Item received: Small Axe" centerprint
				display_big_message("")
				i = i + 1
			end
			hide("node62") next("node63")
		end,
	},
	{
		id = "node63",
		text = _"Let me try that again.",
		echo_text = false,
		code = function()
			trade_with("TutorialTom")
			if (has_item_backpack("Small Tutorial Axe")) then
				show("node63")
			else
				hide("node63")
				show("node64")
			end
		end,
	},
	{
		id = "node64",
		text = _"How about buying an item?",
		code = function()
			npc_says(_"Well it is much the same procedure, except the items for sale are on the top row.")
			npc_says(_"With the circuits from selling the Small Axe to me, you can afford to buy a unit of Bottled ice.")
			npc_says(_"Since Bottled ice combine together, a slider will pop-up asking how many you'd like to buy.")
			npc_says(_"You have enough circuits for one unit.")
			sell_item("Bottled Tutorial ice")
			hide("node64") next("node65")
		end,
	},
	{
		--hidden
		id = "node65",
		text = _"Let me try that again.",
		echo_text = false,
		code = function()
			if (get_gold() < 15) then
				add_gold(15 - get_gold())
			end
			trade_with("TutorialTom")
			if (count_item_backpack("Bottled Tutorial ice") < 1) then
				show("node65")
			else
				hide("node65")
				next("node66")
			end
		end,
	},
	{
		--hidden
		id = "node66",
		text = "BUG, REPORT ME! tutorial node66 -- LAST BITS ABOUT SHOPS",
		echo_text = false,
		code = function()
			npc_says(_"You will occasionally run into items that can't be sold.")
			npc_says(_"This generally means that the shopkeeper can't give you a reasonable deal on it.")
			npc_says(_"Someone else probably wants the item.")
			npc_says(_"You might check your questbook to see if it is mentioned there.")
			if (has_item_backpack("Bottled Tutorial ice")) then --before player notices, swap the "tutorial" item to normal one :P
				local Tutorial_ice_amount = count_item("Bottled Tutorial ice")
				del_item_backpack("Bottled Tutorial ice", Tutorial_ice_amount)
				add_item("Bottled ice", Tutorial_ice_amount)
				display_big_message("") -- so that we don't have the Item Recieved: spam on the screen which might be confusing
				display_big_message("")
				display_big_message("")
				display_big_message("")
				display_big_message("")
				display_big_message("")
				display_big_message("")
				display_big_message("")
				display_big_message("")
				display_big_message("")
				display_big_message("")
			else
				npc_says("ERROR OCCURED, TutorialTom node 66, player doesn't have botteled tutorial ice, WTF happened??")
			end
			armor_node_one = true
			if (armor_node_two) then
				show("node19")
			end
		end,
	},
	{
		id = "node70",
		text = _"There is a wall in the path! How will we get through?",
		code = function()
			if (cmp_obstacle_state("TutorialWall", "broken")) then
				npc_says(_"Uhm, looks like the wall is already broken, I'll repair it.")
				change_obstacle_state("TutorialWall", "intact")
			else
				npc_says(_"Notice that part of it is made of a weak material, glass.")
				npc_says(_"Obstacles you can interact with change color when your cursor is hovering over them.")
				npc_says(_"[b]Left click[/b] on the glass wall to break it.")
				npc_says(_"After you break through the glass wall, walk to the other side so I know you are ready to move on.")
			end
			end_dialog()
		end,
	},
	{
		id = "node71",
		text = _"I defeated all the bots in the cage.",
		code = function()
			npc_says(_"Well done!")
			npc_says(_"Then let us move towards the next stage of the Tutorial")
			change_obstacle_state("TutorialGlasswallDoor", "opened")
			npc_says(_"As a reward, I'm giving you a .22 Automatic pistol.")
			if (has_item("Source Book of Calculate Pi")) and
			   (has_item("Source Book of Malformed packet")) then
				--plural
				npc_says(_"But I have to take this sourcebooks from you to preven you from cheating in the next section of the tutorial.")
				npc_says(_"Sorry about that...")
				tux_says(_"Ok...")
			elseif (has_item("Source Book of Calculate Pi")) or
			       (has_item("Source Book of Malformed packet")) then
				--singular
				npc_says(_"But I have to take this sourcebook from you to preven you from cheating in the next section of the tutorial.")
				npc_says(_"Sorry about that...")
				tux_says(_"Ok...")
			end
			if (has_item("Source Book of Calculate Pi")) then -- ugly, but it shall do
				del_item("Source Book of Calculate Pi")
			end
			if (has_item("Source Book of Malformed packet")) then
				del_item("Source Book of Malformed packet")
			end
			add_item(".22 Automatic", 1)
			add_item(".22 LR Ammunition", 10)
			TutorialTom_has_gun = true
			TutorialTom_hide_71 = true
			hide("node42", "node50", "node51", "node52", "node53", "node57", "node71")
			end_dialog()
		end,
	},

--    							Quick Menu (to jump to sections)

	{
		id = "node80",
		text = _"I'd like to skip to a later section of the Tutorial.",
		code = function()
			npc_says(_"OK, what would you like to learn about?")
			show("node81", "node82", "node83", "node84", "node85", "node86", "node87")
			hide("node1", "node7", "node80")
		end,
	},

	{
		id = "node81",
		text = _"Chests and Armor",
		code = function()
			npc_says(_"OK, I'll take us to the Chest.")
			teleport_npc("TutorialTom-Chest")
			teleport("TutorialTux-Chest")
			tutorial_chests_and_armor_and_shops()
			next("node10")
			hide("node81", "node82", "node83", "node84", "node85", "node86", "node87")
		end,
	},

	{
		id = "node82",
		text = _"Shops",
		code = function()
			npc_says(_"Remember to pick up the items in the chest.")
			npc_says(_"I'll tell you about the shops.")
			teleport_npc("TutorialTom-Chest")
			teleport("TutorialTux-Chest")
			tutorial_chests_and_armor_and_shops()
			next("node61")
			show("node19")
			hide("node81", "node82", "node83", "node84", "node85", "node86", "node87")
		end,
	},

	{
		id = "node83",
		text = _"Melee Combat",
		code = function()
			npc_says(_"OK, I'll take us to the melee arena and give you some armor.")
			teleport_npc("TutorialTom-Melee")
			teleport("TutorialTux-Melee")
			add_quest(_"Tutorial Melee", _"I skipped ahead to the Melee practice. Tom gave me some armor to use.")
			tutorial_melee_combat()
			next("node20")
			hide("node81", "node82", "node83", "node84", "node85", "node86", "node87")
		end,
	},

	{
		id = "node85",
		text = _"Abilities",
		code = function()
			npc_says(_"OK, I'll take us there and give you some armor and a weapon.")
			teleport_npc("TutorialTom-Terminal")
			teleport("TutorialTux-Terminal")
			tutorial_abilities()
			next("node40")
			hide("node81", "node82", "node83", "node84", "node85", "node86", "node87")
		end,
	},

	{
		id = "node86",
		text = _"Upgrading Items & Terminals",
		code = function()
			npc_says(_"OK, I'll take us there and give you some armor and a weapon to work with.")
			teleport_npc("TutorialTom-Terminal")
			teleport("TutorialTux-Terminal")
			tutorial_upgrade_items_and_terminal()
			if (not has_quest("Tutorial Upgrading Items")) then
				add_quest("Tutorial Upgrading Items", "")
			end
			next("node58")
			show("node42")
			hide("node81", "node82", "node83", "node84", "node85", "node86", "node87")
		end,
	},

	{
		id = "node87",
		text = _"Hacking Bots",
		code = function()
			npc_says(_"OK, I'll take us to the hacking area. Also I'll give you some XP so you can improve your cooling.")
			add_xp(2000)
			teleport_npc("TutorialTom-Takeover")
			teleport("TutorialTom-Takeover")
			tutorial_hacking()
			next("node50")
			hide("node81", "node82", "node83", "node84", "node85", "node86", "node87")
		end,
	},

	{
		id = "node84",
		text = _"Ranged Combat",
		code = function()
			npc_says(_"OK, I'll take us to the shooting range and give you some armor and a weapon.")
			teleport_npc("TutorialTom-Ranged")
			teleport("TutorialTux-Ranged")
			change_obstacle_state("TutorialShootCage", "opened")
			tutorial_ranged_combat()
			next("node30")
			hide("node81", "node82", "node83", "node84", "node85", "node86", "node87")
		end,
	},

	{
		id = "node99",
		text = _"Let me take a short break and practice on my own for a while.",
		code = function()
			npc_says(_"Ok, I'll be around if you need any more questions answered. Don't hesitate to ask.")
			end_dialog()
		end,
	},
}
