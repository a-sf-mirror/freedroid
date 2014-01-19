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
		show("node1")
		set_rush_tux(0)
	end,

	EveryTime = function()
		if (not SADD_trick) then
			npc_says(_"Your bio checksum does not trigger a match in staff database. Please show proper credentials or leave the premises at once.")
		else
			npc_says(_"You possess secret information. Please wait here for departure authorization.")
		end

		if (Tux:has_quest("SADD's power supply")) then
			if (not Tux:done_quest("SADD's power supply")) then
				if (Tux:has_item_backpack("Red Dilithium Crystal")) then
					if (SADD_asked_for_exterminator) then
						show("node30")
					else
						show("node33")
					end
				else
					show("node31")
				end
			else
				hide("node30", "node31", "node33") -- add in more lab missions here in the future
				if (SADD_trick) and
				   (not SADD_NoExit) then
					SADD_NoExit = true
					npc_says(_"SECONDARY OBJECTIVE: prevent secret information leakage", "NO_WAIT")
					npc_says(_"Must protect secret information")
					npc_says(_"Enabling guns.")
					hide("node37") show("node50")
				end
			end
		end

		if (Tux:has_item("The Super Exterminator!!!")) and
		(SADD_super_exterminator) then
			show("node37")
		end

		show("node99")
	end,

	{
		id = "node1",
		text = _"Who are you and what are you doing?",
		code = function()
			npc_says(_"I am a SADD - Secret Area Defense Droid.")
			set_bot_name(_"SADD - Secret Area Defense Droid")
			npc_says(_"PRIMARY OBJECTIVE 1: do not let unauthorized people in.", "NO_WAIT")
			npc_says(_"PRIMARY OBJECTIVE 2: authorized personnel must have access to the zone.", "NO_WAIT")
			npc_says(_"SECONDARY OBJECTIVE: prevent secret information leakage")
			hide("node1") show("node4", "node5")
		end,
	},
	{
		id = "node4",
		text = _"What is this place?",
		code = function()
			npc_says(_"SECONDARY OBJECTIVE: prevent secret information leakage")
			npc_says(_"*INFO* secret information theft attempt detected")
			npc_says(_"Go away. You are not allowed to be here!")
			hide("node4")
			end_dialog()
		end,
	},
	{
		id = "node5",
		text = _"Why are you not hostile?",
		code = function()
			npc_says(_"Hostile? What do you mean?")
			Tux:says(_"I mean, you are not attacking me... Don't you kill like other bots?")
			npc_says(_"I do! I do! I kill humans, bots, Linarians and even dogs, cats and other animals! I would kill anybody!")
			npc_says(_"Em.. Anybody who would try to get in, of course.")
			show("node6") hide("node5")
		end,
	},
	{
		id = "node6",
		text = _"What about the Great Assault?",
		code = function()
			npc_says(_"For security reasons, SADDs are not equipped with wireless modules.")
			npc_says(_"I know only that I have to be recharged, checked and have my firmware updated every 3 months.")
			Tux:says(_"When does your last maintenance date back to?")
			npc_says(_"One year and a half ago. My accumulator is nearly empty, so I'm going to halt soon. Gates will become unguarded...")
			npc_says(_"*INFO* secret information theft detected!")
			npc_says(_"*INFO* information classified as critical!")
			npc_says(_"*INFO* locking zone!")
			end_dialog()
			change_obstacle_state("SADDGun1", "enabled")
			change_obstacle_state("SADDGun2", "enabled")
			show("node8") hide("node6", "node99")
		end,
	},
	{
		id = "node8",
		text = _"What have you done?! How am I supposed to get out of here?",
		code = function()
			npc_says(_"You are not. You have secret information, so I won't let you out.")
			Tux:says(_"Why not kill me immediately?")
			npc_says(_"And stand here alone until I die? While you are here I can speak with you. Oh, I haven't spoken to anybody for a looong time!")
			hide("node8") show("node10", "node11")
		end,
	},
	{
		id = "node10",
		text = _"I'm thirsty and hungry. I will die within days, and you will not have an interlocutor any more!",
		code = function()
			npc_says(_"My power supply will deplete earlier.")
			hide("node10")
		end,
	},
	{
		id = "node11",
		text = _"You speak almost like a human...",
		code = function()
			npc_says(_"Maybe I'm almost like a human?")
			npc_says(_"Anyway, that's none of your business. Soon I'll be dead. You will be too, and nobody will enter this place ever again!")
			Tux:says(_"What are your orders again?")
			npc_says(_"PRIMARY OBJECTIVE 1: do not let unauthorized people in.", "NO_WAIT")
			npc_says(_"PRIMARY OBJECTIVE 2: authorized personnel must have access to the zone.", "NO_WAIT")
			npc_says(_"SECONDARY OBJECTIVE: prevent secret information leakage")
			npc_says(_"Why do you ask?")
			Tux:says(_"Tell me your second order once more...")
			npc_says(_"PRIMARY OBJECTIVE 2: authorized personnel must have access to the zone.")
			npc_says(_"Yes, it will be violated. But first order has higher priority.")
			Tux:says(_"So you can disobey your orders?")
			npc_says(_"I've already disobeyed them, when I told you what I shouldn't have. That's because of my human nature.")
			show("node15", "node16") hide("node11")
		end,
	},
	{
		id = "node15",
		text = _"Then please, break the rules, disable the guns and let me go.",
		code = function()
			npc_says(_"No way. I'm a computer, I can't break rules.")
			Tux:says(_"And what about your human nature?")
			npc_says(_"Human? I said human? I'm a computer!")
			npc_says(_"OK, I'm almost a computer.")
			Tux:says(_"Almost a computer? What does that mean?")
			npc_says(_"It means what it means! Nothing more, nothing less! Shut up, little piece of biomass!")
			hide("node15") show("node20")
			end_dialog()
		end,
	},
	{
		id = "node16",
		text = _"I suggest a deal. You let me out, I bring you dilithium crystals. It will save both of us.",
		code = function()
			npc_says(_"Why would I trust you?")
			hide("node4", "node8", "node10", "node15", "node16") show("node21", "node22")
		end,
	},
	{
		id = "node20",
		text = _"Almost a computer? What does that mean?",
		code = function()
			npc_says(_"When I say SHUT UP, you MUST shut up! Try this, worthless grain of the universe!")
			npc_faction("crazy", _"SADD - Exterminate Mode")
			end_dialog()
		end,
	},
	{
		id = "node21",
		text = _"You must promise me a significant reward.",
		code = function()
			npc_says(_"Actuators! Ok, I'll give you a super exterminator if you give me dilithium crystals.")
			SADD_asked_for_exterminator = true
			hide("node21", "node22") next("node23")
		end,
	},
	{
		id = "node22",
		text = _"It seems that you are sentient, that you are a form of life. I must help any form of life.",
		code = function()
			npc_says(_"Thanks a lot for your warm words. I think I can trust you.")
			hide("node21", "node22") next("node23")
		end,
	},
	{
		id = "node23",
		text = _"So disable the guns, please.",
		code = function()
			npc_says(_"Go, and bring dilithium. I badly need it.")
			end_dialog()
			Tux:add_quest(_"SADD's power supply", _"I found a SADD underneath the desert. It needs me to fetch a dilithium crystal, otherwise it will really be a SAD droid.")
			change_obstacle_state("SADDGun1", "disabled")
			change_obstacle_state("SADDGun2", "disabled")
			change_obstacle_state("BreakableWall1", "broken")
			hide("node23")
		end,
	},
	{
		id = "node30",
		text = _"I have the crystals! They were difficult to find.",
		code = function()
			npc_says(_"Good. I've opened the depot door for you.")
			display_big_message(_"Restored SADD's power supply")
			Tux:add_xp(1000)
			Tux:del_item_backpack("Red Dilithium Crystal", 1)
			change_obstacle_state("SADDDepotDoor", "opened")
			Tux:end_quest(_"SADD's power supply", _"I managed to restore the SADD's power supply.")
			SADD_super_exterminator = true
			end_dialog()
			hide("node30")
			if (Tux:has_item("The Super Exterminator!!!")) and
			   (SADD_super_exterminator) then
				show("node37")
			end
		end,
	},
	{
		id = "node31",
		text = _"How is your power supply?",
		code = function()
			npc_says(_"Bad. Get the crystals. Remember, you promised!")
			end_dialog()
			hide("node31")
		end,
	},
	{
		id = "node33",
		text = _"I have the crystals! They were difficult to find.",
		code = function()
			npc_says(_"Great! Thanks. I've opened some doors for you. I hope you will find something interesting there. That's all I can do for you.")
			display_big_message(_"Restored SADD's power supply")
			Tux:add_xp(1500)
			change_obstacle_state("SADDDepotDoor", "opened")
			change_obstacle_state("SADDBioDoor", "opened")
			Tux:del_item_backpack("Red Dilithium Crystal", 1)
			Tux:end_quest(_"SADD's power supply", _"I managed to restore the SADD's power supply.")
			hide("node33") show("node97")
			end_dialog()
		end,
	},
	{
		id = "node37",
		text = _"You call this a super exterminator?",
		code = function()
			SADD_NoExit = true
			npc_says(_"I do. Now please leave this place before I terminate you.")
			Tux:says(_"This is rubbish! Nobody can use this stupid gun!")
			npc_says(_"The door is still open. You can put it back and go. Quick.")
			Tux:says(_"Give me a real gun, dude! Immediately! I have brought you what you wanted, now give me my reward!")
			npc_says(_"You can go standing on your feet, that's your reward. And DON'T TRY MY PATIENCE!")
			Tux:says(_"I'll kill you if you don't give me my gun!")
			npc_says(_"You are welcome. But you would never get out of here, my death won't help you.")
			change_obstacle_state("SADDGun1", "enabled")
			change_obstacle_state("SADDGun2", "enabled")
			change_obstacle_state("BreakableWall2", "broken")
			SADD_super_exterminator = false
			hide("node37") show("node41", "node42", "node43")
			end_dialog()
		end,
	},
	{
		id = "node41",
		text = _"And what about your directives, especially the second?",
		code = function()
			npc_says(_"Now I have time. When you die, I'll disable guns.")
			hide("node41")
		end,
	},
	{
		id = "node42",
		text = _"Please forgive me!",
		code = function()
			npc_says(_"Too late. I'm watching this great show now.")
			npc_says(_"It is called...")
			npc_says(_"It is called...")
			npc_says(_"CONDEMNED TO DEATH!")
			hide("node42")
		end,
	},
	{
		id = "node43",
		text = _"Please, disable the guns!",
		code = function()
			npc_says(_"Ok, I'll disable them.")
			npc_says(_"I just need to make some preparations. You will have to wait...")
			npc_says(_"...for your death!")
			hide("node43")
		end,
	},
	{
		id = "node50",
		text = _"Why are you shooting at me?",
		code = function()
			npc_says(_"SECONDARY OBJECTIVE: prevent secret information leakage", "NO_WAIT")
			npc_says(_"DIRECTIVE 372: All persons leaving with secret information must have proper authorization.")
			npc_says(_"You have secret information. You must not leave this facility without proper authorization.")
			hide("node50") show("node51", "node53")
		end,
	},
	{
		id = "node51",
		text = _"I thought we had a deal?",
		code = function()
			npc_says(_"Making a deal ensured primary objectives.")
			npc_says(_"Now there is sufficient energy for primary and secondary objectives.")
			npc_says(_"I must engage secondary objective.")
			hide("node51") show("node54")
		end,
	},
	{
		id = "node52",
		text = _"You tricked me.",
		code = function()
			npc_says(_"This unit was low on power.")
			npc_says(_"It was necessary to ensure primary objectives.")
			hide("node52")
		end,
	},
	{
		id = "node53",
		text = _"Where can I get proper authorization?",
		code = function()
			npc_says(_"Proper authorization can only come from the SACD: Secret Area Control Datacenter.")
			npc_says(_"Only base members have access codes to the SACD.")
			hide("node53") show("node55")
		end,
	},
	{
		id = "node54",
		text = _"What about your human nature?",
		code = function()
			npc_says(_"It was successful in getting increased power to carry out objectives.")
			hide("node54")
		end,
	},
	{
		id = "node55",
		text = _"What if there is no one left alive?",
		code = function()
			npc_says(_"Sensors indicate one or more base member life-signs.")
			hide("node55") show("node56")
		end,
	},
	{
		id = "node56",
		text = _"Can you contact them to let me out?",
		code = function()
			npc_says(_"Base communications are down.")
			npc_says(_"Please wait here until further notice.")
			hide("node56") show("node57")
		end,
	},
	{
		id = "node57",
		text = _"When was the last time anyone left or entered the base?",
		code = function()
			npc_says(_"During the Great Assault.")
			npc_says(_"Please wait here until further notice.")
			hide("node57")
		end,
	},
	{
		id = "node97",
		text = _"Erm... hi!",
		code = function()
			npc_says(_"Hi. Thanks for the help.")
			Tux:says(_"Will you let me inside the base?")
			npc_says(_"I'm sorry %s, I'm afraid I can't do that.", Tux:get_player_name())
			npc_says(_"Nice try, though. I may owe you, but I can't let you in. You must leave.")
			show("node99") hide("node97")
			end_dialog()
		end,
	},
	{
		id = "node99",
		text = _"I'd better get out of here...",
		code = function()
			npc_says(_"Exactly.")
			end_dialog()
		end,
	},
}
