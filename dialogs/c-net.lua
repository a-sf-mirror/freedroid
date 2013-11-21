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
		play_sound("effects/Menu_Item_Deselected_Sound_0.ogg")
		-- name = get_player_name() -- We need to generate the option text of option 0 to use this properly
		if (c_net_terminals_disabled) then
			npc_says(_" . ")
			end_dialog()
		else
			npc_says(_"Welcome to the Community Network.", "NO_WAIT")
			cli_says(_"Login :", "NO_WAIT")
			if (knows_c_net_users) then
				show("node1", "node2", "node3")
			end
			show("node0", "node99")
		end
		hide("node10", "node11", "node12", "node13", "node14", "node15", "node16", "node20", "node21", "node23", "node24", "node30", "node31", "node80", "node81", "node82", "node83", "node85", "node86")
	end,

	------------------------------
	-- c-net-nethack_sub
	--
	{
		topic = "c-net-nethack_sub",
		generator = include("c-net-nethack_sub"),
	},
	--
	------------------------------

	{
		id = "node0",
		text = "guest",
		code = function()
			c_net_username = get_player_name()
			c_net_prompt = c_net_username .. "@c-net:~$"
			tux_says("guest", "NO_WAIT")
			if (not c_net_terminal_logged_in) then
				c_net_terminal_logged_in = true
				npc_says(_"First time login detected.")
				npc_says(_"Please enter your name", "NO_WAIT")
				cli_says(_"Name : ", "NO_WAIT")
				tux_says(c_net_username)
				npc_says(_"Please set password for your personalized guest login, %s", c_net_username, "NO_WAIT")
				npc_says(_"Use at least one lower case letter, one upper case letter, one number, and one symbol.", "NO_WAIT")
				cli_says(_"Password : ", "NO_WAIT")
				tux_says("******")
			else
				cli_says(_"Name : ", "NO_WAIT")
				tux_says(c_net_username, "NO_WAIT")
				cli_says(_"Password : ", "NO_WAIT")
				tux_says("******", "NO_WAIT")
			end
			npc_says(_"Last login from /dev/tty3 on unknown", "NO_WAIT")
			cli_says(c_net_prompt, "NO_WAIT")
			show("node10", "node20", "node30", "node80", "node99") hide("node0", "node1", "node2", "node3")
		end,
	},
	{
		id = "node1",
		text = "root",
		code = function()
			tux_says("root", "NO_WAIT")
			cli_says(_"Password : ", "NO_WAIT")
			tux_says("******")
			--if (not knows_root_password) then
			next("node9")
			--else
			-- c_net_username = "root"
			-- c_net_prompt = c_net_username .. "@c-net:~$"
			-- npc_says(_"Last login from /dev/tty3 on unknown" , "NO_WAIT")
			-- cli_says(c_net_prompt, "NO_WAIT")
			-- show("node10", "node20", "node30", "node80", "node99") hide("node0", "node1", "node2", "node3")
			--end
		end,
	},
	{
		id = "node2",
		text = "lily",
		code = function()
			tux_says("lily", "NO_WAIT")
			cli_says(_"Password : ", "NO_WAIT")
			tux_says("******")
			if (not know_lily_password) then
				next("node9")
			else
				c_net_username = "lily"
				c_net_prompt = c_net_username .. "@c-net:~$"
				npc_says(_"Last login from /dev/tty3 on unknown" , "NO_WAIT")
				cli_says(c_net_prompt, "NO_WAIT")
				show("node10", "node20", "node30", "node80", "node99") hide("node0", "node1", "node2", "node3")
			end
		end,
	},
	{
		id = "node3",
		text = "cpain",
		code = function()
			tux_says("cpain", "NO_WAIT")
			cli_says(_"Password: ", "NO_WAIT")
			--if (not knows_sorenson_password) then
			tux_says("******")
			next("node9")
			--else
			-- tux_says("************************************")
			-- c_net_username = "cpain"
			-- c_net_prompt = c_net_username .. "@c-net:~$"
			-- npc_says(_"Last login from /dev/tty3 on unknown" , "NO_WAIT")
			-- cli_says(c_net_prompt, "NO_WAIT")
			-- show("node10", "node20", "node30", "node80", "node99") hide("node0", "node1", "node2", "node3")
			--end
		end,
	},
	{
		id = "node9",
		text = "BUG, REPORT ME! cnet node9",
		code = function()
			npc_says(_"Login incorrect", "NO_WAIT")
			npc_says(_"Connection to c-net terminated.")
			play_sound("effects/Menu_Item_Selected_Sound_1.ogg")
			end_dialog()
		end,
	},
	{
		-- ../ date finger users whoami
		id = "node10",
		text = "cd info_commands/",
		code = function()
			c_net_prompt = c_net_username .. "@c-net:~/info_commands$"
			npc_says(" ", "NO_WAIT")
			cli_says(c_net_prompt, "NO_WAIT")
			show("node11", "node12", "node13", "node14", "node15", "node16") hide("node10", "node20", "node30", "node80", "node99")
		end,
	},
	{
		id = "node11",
		text = "cd ../",
		code = function()
			c_net_prompt = c_net_username .. "@c-net:~$"
			npc_says(" ", "NO_WAIT")
			cli_says(c_net_prompt, "NO_WAIT")
			show("node10", "node20", "node30", "node80", "node99") hide("node11", "node12", "node13", "node14", "node15", "node16")
		end,
	},
	{
		id = "node12",
		text = "date",
		code = function()
			-- npc_says(get_date() ,"NO_WAIT")
			tux_says("date", "NO_WAIT")
			npc_says(_"Date unknown", "NO_WAIT")
			cli_says(c_net_prompt, "NO_WAIT")
		end,
	},
	{
		id = "node13",
		text = "finger",
		code = function()
			tux_says("finger", "NO_WAIT")
			knows_c_net_users = true
			npc_says("Login Tty Name", "NO_WAIT")
			npc_says("bossman tty7 Spencer", "NO_WAIT")
			npc_says("cpain tty5 Sorenson", "NO_WAIT")
			npc_says("guest tty3 " .. get_player_name() .. "", "NO_WAIT")
			npc_says("lily tty2 Lily Stone", "NO_WAIT")
			cli_says(c_net_prompt, "NO_WAIT")
		end,
	},
	{
		id = "node14",
		text = "users",
		code = function()
			tux_says("users", "NO_WAIT")
			npc_says("bossman cpain guest lily", "NO_WAIT")
			knows_c_net_users = true
			cli_says(c_net_prompt, "NO_WAIT")
		end,
	},
	{
		id = "node15",
		text = "whoami",
		code = function()
			tux_says("whoami", "NO_WAIT")
			npc_says(c_net_username, "NO_WAIT")
			cli_says(c_net_prompt, "NO_WAIT")
		end,
	},
	{
		id = "node16",
		text = "uname",
		code = function()
			tux_says("uname", "NO_WAIT")
			npc_says("Nkernel", "NO_WAIT")
			cli_says(c_net_prompt, "NO_WAIT")
		end,
	},
	{
		-- ../ ls readdrive ./statistics.pl
		id = "node20",
		text = "cd file_commands/",
		code = function()
			c_net_prompt = c_net_username .. "@c-net:~/file_commands$"
			npc_says(" ", "NO_WAIT")
			cli_says(c_net_prompt, "NO_WAIT")
			show("node21", "node23", "node24", "node70") hide("node10", "node20", "node30", "node80", "node99")
		end,
	},
	{
		id = "node21",
		text = "cd ../",
		code = function()
			c_net_prompt = c_net_username .. "@c-net:~$"
			npc_says(" ", "NO_WAIT")
			cli_says(c_net_prompt, "NO_WAIT")
			show("node10", "node20", "node30", "node80", "node99") hide("node21", "node23", "node24", "node70")
		end,
	},
	{
		id = "node23",
		text = _"mountdisk.sh",
		code = function()
			tux_says(_"./mountdisk.sh", "NO_WAIT")
			if (has_item_backpack("Kevin's Data Cube")) then
				npc_says(_"Mounting volume \"Kevins_Security_File\"...")
				npc_says(_"Private memory and/or virtual address space exhausted.", "NO_WAIT")
				npc_says(_"Not enough free memory to load data file.", "NO_WAIT")
			elseif (has_quest("Deliverance")) and
			       (not done_quest("Deliverance")) and
			       (has_item_backpack("Data cube")) then
				npc_says(_"List for Spencer:")
				npc_says("Alastra, Maria Grazia", "NO_WAIT")
				npc_says("Arana, Pedro", "NO_WAIT")
				npc_says("Badea, Catalin", "NO_WAIT")
				npc_says("Bourdon, Pierre", "NO_WAIT")
				npc_says("Castellan, Simon", "NO_WAIT")
				npc_says("Cipicchio, Ted", "NO_WAIT")
				npc_says("Danakian, Hike", "NO_WAIT")
				npc_says("Degrande, Samuel", "NO_WAIT")
				npc_says("Gill, Andrew A. ", "NO_WAIT")
				npc_says("Griffiths, Ian", "NO_WAIT")
				npc_says("Hagman, Nick", "NO_WAIT")
				npc_says("Herron, Clint", "NO_WAIT")
				npc_says("Huillet, Arthur", "NO_WAIT")
				npc_says("Huszics, Stefan", "NO_WAIT")
				npc_says("Infrared", "NO_WAIT")
				npc_says("James", "NO_WAIT")
				npc_says("Kangas, Stefan", "NO_WAIT")
				npc_says("Kremer, David", "NO_WAIT")
				npc_says("Kruger, Matthias", "NO_WAIT")
				npc_says("Kucia, Jozef", "NO_WAIT")
				npc_says("Matei, Pavaluca", "NO_WAIT")
				npc_says("McCammon, Miles", "NO_WAIT")
				npc_says("Mendelson, Michael", "NO_WAIT")
				npc_says("Mourujarvi, Esa-Matti", "NO_WAIT")
				npc_says("Mustonen, Ari", "NO_WAIT")
				npc_says("Newton, Simon", "NO_WAIT")
				npc_says("Offermann, Sebastian", "NO_WAIT")
				npc_says("Parramore, Kurtis", "NO_WAIT")
				npc_says("Pepin-Perreault, Nicolas")
				npc_says("Picciani, Arvid", "NO_WAIT")
				npc_says("Pitoiset, Samuel", "NO_WAIT")
				npc_says("Pradet, Quentin", "NO_WAIT")
				npc_says("Prix, Johannes", "NO_WAIT")
				npc_says("Prix, Reinhard", "NO_WAIT")
				npc_says("rudi_s", "NO_WAIT")
				npc_says("Ryushu, Zombie", "NO_WAIT")
				npc_says("Salmela, Bastian", "NO_WAIT")
				npc_says("Starminn", "NO_WAIT")
				npc_says("Solovets, Alexander", "NO_WAIT")
				npc_says("Swietlicki, Karol", "NO_WAIT")
				npc_says("Tetar, Philippe", "NO_WAIT")
				npc_says("Thor", "NO_WAIT")
				npc_says("Voots, Ryan", "NO_WAIT")
				npc_says("Wood, JK", "NO_WAIT")
				npc_says("Winterer, Armin", "NO_WAIT")
				if (not deliverance_datacube_c_net_list) then
					update_quest(_"Deliverance", _"I found a terminal in the town which could read the data cube Francis gave me. It looks like there was a list of names on it, but I have no clue what's the deal with these names.")
					deliverance_datacube_c_net_list = true
				end
			else
				npc_says(_"no disk found", "NO_WAIT")
			end
			cli_says(c_net_prompt, "NO_WAIT")
		end,
	},
	{
		id = "node24",
		text = _"statistics.pl",
		code = function()
			tux_says("./statistics.pl", "NO_WAIT")
			npc_says(_"Corrupted file.", "NO_WAIT")
			-- npc_says(_"Bot #Dead# Tux #Hacked/Failed#Ratio", "NO_WAIT")
			-- npc_says(print_stats(),"NO_WAIT")
			cli_says(c_net_prompt, "NO_WAIT")
		end,
	},
	{
		id = "node30",
		text = "cd documents/",
		code = function()
			c_net_prompt = c_net_username .. "@c-net:~/documents$"
			tux_says("cd documents/", "NO_WAIT")
			if (c_net_username =="root") then
				show("node66")
			end
			--if (c_net_username == "guest") then
			--elseif (c_net_username =="lily") then -- no special files yet
			--elseif (c_net_username =="cpain") then -- no special files yet
			--end
			cli_says(c_net_prompt, "NO_WAIT")
			show("node31") hide("node10", "node20", "node30", "node80", "node99")
		end,
	},
	{
		id = "node31",
		text = "cd ../",
		code = function()
			c_net_prompt = c_net_username .. "@c-net:~$"
			npc_says(" ", "NO_WAIT")
			cli_says(c_net_prompt, "NO_WAIT")
			show("node10", "node20", "node30", "node80", "node99") hide("node31")
		end,
	},
	{
		id = "node66",
		text = _"forkBOMB.sh -arm bomb",
		code = function()
			npc_says(_"bomb armed", "NO_WAIT")
			cli_says(c_net_prompt, "NO_WAIT")
			show("node67", "node68")
		end,
	},
	{
		id = "node67",
		text = _"forkBOMB.sh -disarm bomb",
		code = function()
			npc_says(_"bomb defused", "NO_WAIT")
			cli_says("root@c-net:~$", "NO_WAIT")
			hide("node67", "node68") show("node66")
		end,
	},
	{
		id = "node68",
		text = _"forkBOMB.sh -execute",
		code = function()
			c_net_terminals_disabled = true
			display_big_message(_"Terminals Disabled")
			npc_says(_"Script run. After logout this terminal will be disabled.", "NO_WAIT")
			cli_says(_"root@c-net:~$", "NO_WAIT")
			-- add_xp(30) -- Eventually make this a quest goal.
		end,
	},
	{
		id = "node70",
		text = _"radio.sh",
		code = function()
			npc_says(_"Valid tracks:", "NO_WAIT")
			npc_says("Ambience, Bleostrada, HellFortressEntrance, ImperialArmy, NewTutorialStage, TechBattle, TheBeginning, underground, AmbientBattle, hellforce, HellFortressTwo, menu, Suspicion, temple, town")

			local try_again_radio = true

			while (try_again_radio) do
				local track = user_input_string(_"please enter track")
				if (track == "Ambience" ) or
				   (track == "AmbientBattle" ) or
				   (track == "Bleostrada" ) or
				   (track == "hellforce" ) or
				   (track == "HellFortressEntrance" ) or
				   (track == "HellFortressTwo" ) or
				   (track == "ImperialArmy" ) or
				   (track == "menu" ) or
				   (track == "NewTutorialStage" ) or
				   (track == "Suspicion" ) or
				   (track == "TechBattle" ) or
				   (track == "temple" ) or
				   (track == "TheBeginning" ) or
				   (track == "town" ) or
				   (track == "underground" ) or
				   (running_benchmark()) then
					switch_background_music(track .. ".ogg")
					town_track = track
					try_again_radio = false
					next("node20")
				elseif (track == "exit" ) then
					try_again_radio = false
					next("node20")
				else
					npc_says(_"WARNING, '%s' not a valid track.", track)
					npc_says(_"enter 'exit' to exit.")
					npc_says(_"Please retry.")
					npc_says(_"Valid tracks:", "NO_WAIT")
					npc_says("Ambience, Bleostrada, HellFortressEntrance, ImperialArmy, NewTutorialStage, TechBattle, TheBeginning, underground, AmbientBattle, hellforce, HellFortressTwo, menu, Suspicion, temple, town")
				end
			end

		end,
	},
	{
		id = "node80",
		text = "cd games/",
		code = function()
			npc_says(" ", "NO_WAIT")
			c_net_prompt = c_net_username .. "@c-net:~/games$"
			cli_says(c_net_prompt, "NO_WAIT")
			show("node81", "node82", "node83", "node85", "node86") hide("node10", "node20", "node30", "node80", "node99")
		end,
	},
	{
		id = "node81",
		text = "cd ../",
		code = function()
			c_net_prompt = c_net_username .. "@c-net:~$"
			npc_says(" ", "NO_WAIT")
			cli_says(c_net_prompt, "NO_WAIT")
			show("node10", "node20", "node30", "node80", "node99") hide("node81", "node82", "node83", "node85", "node86")
		end,
	},
	{
		id = "node82",
		text = "nethack",
		code = function()
			tux_says("./nethack", "NO_WAIT")
			push_topic("c-net-nethack_sub")
			-- call c-net-nethack_sub subdialog
			next("c-net-nethack_sub.everytime")
		end,
	},
	{
		-- called after the end of c-net-nethack_sub subdialog
		id = "after-c-net-nethack_sub",
		code = function()
			pop_topic()
			cli_says(c_net_prompt, "NO_WAIT")
		end,
	},
	{
		id = "node83",
		text = _"global_thermonuclear_war",
		code = function()
			tux_says(_"./global_thermonuclear_war", "NO_WAIT")
			npc_says_random(_"Sorry, only winning move is not to play. New game?",
							_"Mankind exterminated. You lost!",
							_"No victory possible. LOSER! Play again?",
							_"Everyone dies, new game?", "NO_WAIT")
			cli_says(c_net_prompt, "NO_WAIT")
		end,
	},
	{
		id = "node85",
		text = _"tetris",
		code = function()
			tux_says(_"./tetris", "NO_WAIT")
			npc_says("Never gonna give you up,")
			npc_says("Never gonna let you down,")
			npc_says("Never gonna run around and desert you.")
			npc_says("Never gonna make you cry,")
			npc_says("Never gonna say goodbye,")
			npc_says("Never gonna tell a lie and hurt you.")
			--left out gettext markers on purpose
			cli_says(c_net_prompt, "NO_WAIT")
			hide("node85")
		end,
	},
	{
		id = "node86",
		text = _"progress_quest",
		code = function()
			tux_says(_"./progress_quest", "NO_WAIT")
			if (not playing_progress_quest) then
				playing_progress_quest = true
				npc_says(_"Roll your Stats.")
				hide("node81", "node82", "node83", "node85", "node86")
				next("node87")
			else
				npc_says(_"You are already playing Progress Quest:")
				npc_says_random(_"You are selling an item!",
								_"You are killing a creature!",
								_"You are gaining a level!",
								_"You are casting a spell!")
				cli_says(c_net_prompt, "NO_WAIT")
			end
		end,
	},
	{
		id = "node87",
		text = _"BUG c-net node 87 -- ROLL STATS",
		code = function()
			local str = math.random(0,6) + math.random(0,6) + math.random(0,6)
			local con = math.random(0,6) + math.random(0,6) + math.random(0,6)
			local dex = math.random(0,6) + math.random(0,6) + math.random(0,6)
			local int = math.random(0,6) + math.random(0,6) + math.random(0,6)
			local wis = math.random(0,6) + math.random(0,6) + math.random(0,6)
			local cha = math.random(0,6) + math.random(0,6) + math.random(0,6)
			npc_says(_"You rolled Stats of STR: [b]%d[/b], CON: [b]%d[/b], DEX: [b]%d[/b], INT: [b]%d[/b], WIS: [b]%d[/b], CHA: [b]%d[/b].", str, con, dex, int, wis, cha, "NO_WAIT")
			show("node88", "node89")
		end,
	},
	{
		id = "node88",
		text = _"Accept Character",
		code = function()
			npc_says(_"Welcome to Progress Quest!")
			cli_says(c_net_prompt, "NO_WAIT")
			show("node81", "node82", "node83", "node85", "node86") hide("node88", "node89")
		end,
	},
	{
		id = "node89",
		text = _"Reroll Character",
		code = function()
			next("node87")
		end,
	},
	{
		id = "node99",
		text = "logout",
		code = function()
			tux_says("logout", "NO_WAIT")
			npc_says(_"Connection to c-net closed.")
			-- set_internet_login_time()
			play_sound("effects/Menu_Item_Selected_Sound_1.ogg")
			end_dialog()
		end,
	},
}
