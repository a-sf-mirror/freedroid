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
---------------------------------------------------------- HAS_MET
		if (not has_met("Guy")) then
			npc_says("HAS MET test 1 succeeded", "NO_WAIT")
		else
			guy_fail("HAS MET")
		end

		if (not has_quest("24_guy_death_quest")) then
			add_quest("24_guy_death_quest", "Quest to check if droid markers work.")
		end

		npc_says("This text is only shown as you speak to this character the first time.")
	end,

	EveryTime = function()
		tux_says("tux_says()", "NO_WAIT")
		npc_says("npc_says()", "NO_WAIT")
		cli_says("cli_says()", "NO_WAIT")
		npc_says("", "NO_WAIT") -- extra linebreak for cli_says()

		GuyArray = {"one", "two", "three"}
		for var in ipairs(GuyArray) do
			npc_says(GuyArray[var], "NO_WAIT")
		end

		next("node0")
	end,

	{
		id = "node0",
		text = _"RUNNING TEST NODE",
		code = function()
			show("node0")
			hide("node0")
---------------------------------------------------------- ITEM
			add_item("Laser Scalpel")
			add_item(".22 Automatic", 2)
			if (has_item(".22 Automatic")) then
				npc_says("ADD ITEM test 1 succeeded", "NO_WAIT")
			else
				guy_fail("ADD ITEM 1")
			end

			if (has_item_backpack(".22 Automatic")) then
				npc_says("ADD ITEM test 2 succeeded", "NO_WAIT")
			else
				guy_fail("ADD ITEM 2")
			end

			if (has_item_equipped("Laser Scalpel")) then
				npc_says("ADD ITEM test 3 succeeded", "NO_WAIT")
			else
				guy_fail("ADD ITEM 3")
			end

			-- ru654
			--[[ NOTE we need del_item_equipped() or unequipp_item()
			del_item("Laser Scalpel")
			if (not has_item("Laser Scalpel")) then
				npc_says("DEL ITEM 1 test succeeded", "NO_WAIT")
			else
				guy_fail("DEL ITEM 1")
			end
			]]--

			del_item_backpack(".22 Automatic", 2)
			if (not has_item_backpack(".22 Automatic")) then
				npc_says("DEL ITEM test 2 succeeded", "NO_WAIT")
			else
				guy_fail("DEL ITEM 2")
			end
---------------------------------------------------------- FACTION
			npc_faction("self", "Guy - self")
			npc_faction("ms", "Guy - ms")
			npc_faction("redguard", "Guy - redguard" )
			npc_faction("resistance", "Guy - resistance")
			npc_faction("civilian", "Guy - civilian")
			npc_faction("crazy", "Guy - crazy")
			npc_faction("singularity", "Guy - singularity")
			npc_faction("neutral", "Guy - neutral")
---------------------------------------------------------- HEALTH
			heal_tux()
			hurt_tux(1)
			hurt_tux(-1)
			if (get_tux_hp() == 60) then
				npc_says("HEALTH test 1 succeded", "NO_WAIT")
			else
				guy_fail("HEALTH 1")
			end

			if (get_tux_max_hp() == 60) then
				npc_says("HEALTH test 2 succeded", "NO_WAIT")
			else
				guy_fail("HEALTH 2")
			end
---------------------------------------------------------- COOL
			heat_tux(1)
			heat_tux(-1)
			if (get_tux_cool() == 100) then
				npc_says("COOL test 1 succeeded", "NO_WAIT")
			else
				guy_fail("COOL 1")
			end
---------------------------------------------------------- TELEPORT
			teleport("24-tux1")
			teleport("24-tux2")
			teleport_npc("24-guy1")
			teleport_npc("24-guy2")
			teleport_npc("24-dude1", "Dude")
			teleport_npc("24-dude2", "Dude")
---------------------------------------------------------- SKILLS
			--npc_says(get_skill("programming"))
			if (not has_met("Guy")) then
				improve_skill("programming")
				if (get_skill("programming") == 1) then
					npc_says("SKILL test 1 succeeded", "NO_WAIT")
				else
					guy_fail("SKILL 1")
				end
			else
				tux_says("Skipping SKILL test 1 due to missing possibility to downgrade skills!")
			end
---------------------------------------------------------- PROGRAMMS
			improve_program("Ricer CFLAGS")
			downgrade_program("Ricer CFLAGS")
			if (get_program_revision("Ricer CFLAGS") == 0) then
				npc_says("PROGRAMM test 1 succeeded", "NO_WAIT")
			else
				guy_fail("PROGRAMM 1")
			end
---------------------------------------------------------- QUESTS
			if (not has_met("Guy")) then
				if (not has_quest("24_dude_test_quest")) then
					npc_says("QUEST test 1 succeeded", "NO_WAIT")
				else
					guy_fail("QUEST 1")
				end
				add_quest("24_dude_test_quest", "Add 24 dude quest.")
			else
				tux_says("Skipping QUEST test 1 due to missing possibility to remove quests!")
			end

			if (has_quest("24_dude_test_quest")) then
				npc_says("QUEST test 2 succeeded", "NO_WAIT")
			else
				guy_fail("QUEST 2")
			end
			update_quest("24_dude_test_quest", "Update 24 dude quest.")

			if (not has_met("Guy")) then
				if (not done_quest("24_dude_test_quest")) then
					npc_says("QUEST test 3 succeeded", "NO_WAIT")
				else
					guy_fail("QUEST 3")
				end

				end_quest("24_dude_test_quest", "Complete 24 dude quest.")
				if (done_quest("24_dude_test_quest")) then
					npc_says("QUEST test 4 succeeded", "NO_WAIT")
				else
					guy_fail("QUEST 4")
				end
			else
				tux_says("Skipping QUEST test 3 due to missing possibility to remove quests!")
				tux_says("Skipping QUEST test 4 due to missing possibility to remove quests!")
			end

			if (has_met("Guy")) then -- need to have met guy to let the DeadGuy die...
				if (done_quest("24_guy_death_quest")) then -- check droid markers
					npc_says("QUEST test 5 succeeded","NO_WAIT")
				else
					guy_fail("QUEST 5")
				end
			else
				tux_says("Skipping QUEST test 5, we need to have met Guy...")
			end

---------------------------------------------------------- OBSTACLES

			change_obstacle_message("24_guy_sign", "Guy signmessage B")
			display_big_message("Sign message changed from")
			display_big_message("Guy signmessage A' to 'Guy signmessage B'")

			if (cmp_obstacle_state("24_guy_door", "opened")) then
				npc_says("OBSTACLE test 1 succeeded", "NO_WAIT")
			else
				guy_fail("OBSTACLE 1")
			end
			change_obstacle_state("24_guy_door", "closed")
			if (cmp_obstacle_state("24_guy_door", "closed")) then
				npc_says("OBSTACLE test 2 succeeded", "NO_WAIT")
			else
				guy_fail("OBSTACLE 2")
			end
			change_obstacle_state("24_guy_door", "opened") -- 6 = door
			change_obstacle_type("24_guy_door", "1")
			if (get_obstacle_type("24_guy_door") == 1) then
				npc_says("OBSTACLE test 3 succeeded", "NO_WAIT")
			else
				guy_fail("OBSTACLE 3")
			end
			change_obstacle_type("24_guy_door", "6") -- set it back to door

---------------------------------------------------------- NPC DEATH TEST
			if (not has_met("Guy")) then
				if (not npc_dead("DeadGuy")) then
					npc_says("NPC DEATH test 1 succeeded", "NO_WAIT")
				else
					guy_fail("NPC DEATH 1")
				end

				drop_dead("DeadGuy") -- kill Dude
			end

			if not (running_benchmark()) then -- remember: our dialog validator is quite dump :(
				if (npc_dead("DeadGuy")) then
					npc_says("NPC DEATH test 2 succeeded", "NO_WAIT")
				else
					guy_fail("NPC DEATH 2")
				end
			end

			-- one day we might be able to revive DeadGuy

			--[[ if (not npc_dead("Dude")) then
			npc_says("NPC DEATH test 3 succeeded", "NO_WAIT")
		else
			guy_fail("NPC DEATH 3")
			end ]]--

---------------------------------------------------------- EVENTS

			if not (running_benchmark()) then
				if (l24_event_test == "works" ) then
					npc_says("EVENT test 1 (map label) succeeded", "NO_WAIT")
				else
					guy_fail("EVENT test 1 (map label)")
				end
			end

			if not (running_benchmark()) then
				if (DeadGuy_death_trigger == "works" ) then
					npc_says("EVENT test 2 (death event) succeeded", "NO_WAIT")
				else
					guy_fail("EVENT test (death event) 2")
				end
			end

---------------------------------------------------------- Gold
			if (get_gold() == 0) then
				npc_says("GOLD test 1 succeded", "NO_WAIT")
			else
				guy_fail("GOLD 1")
			end

			add_gold(100)
			if (get_gold() == 100) then
				npc_says("GOLD test 2 succeded", "NO_WAIT")
			else
				guy_fail("GOLD 2")
			end

			if (not del_gold(1000)) then
				npc_says("GOLD test 3 succeded", "NO_WAIT")
			else
				guy_fail("GOLD 3")
			end

			if (get_gold() == 100) then -- check if gold changed, just to be sure...
				npc_says("GOLD test 4 succeded", "NO_WAIT")
			else
				guy_fail("GOLD 4")
			end

			if (del_gold(100)) then
				npc_says("GOLD test 5 succeded", "NO_WAIT")
			else
				guy_fail("GOLD 5")
			end

			add_gold(100)
			add_gold(-100)
			if (get_gold() == 0) then
				npc_says("GOLD test 6 succeded")
			else
				guy_fail("GOLD 6")
			end


---------------------------------------------------------- RUSH TUX
			if (not has_met("Guy")) then
				if not (will_rush_tux()) then
					npc_says("RUSH TUX test 1 succeded", "NO_WAIT")
				else
					guy_fail("RUSH TUX 1")
				end
			else
				tux_says("Skipping RUSH TUX test 1 because it would fail since we directly rush tux on second time we call the dialog.")
			end

			set_rush_tux(1)

			if (will_rush_tux()) then
				npc_says("RUSH TUX test 2 succeded", "NO_WAIT")
			else
				guy_fail("RUSH TUX 2")
			end
---------------------------------------------------------- OTHER
			-- print some useless colorful foo

			Guy_colors={"black", "red", "green", "yellow", "blue", "purple", "cyan", "white"}
			if (running_benchmark()) then -- for nodes we use printf which has no linebreak at the end.
				print("")				  -- "black" would get appended directly to a node but this avoids it.
			end

			for k, color in ipairs(Guy_colors) do
				print(FDutils.text.highlight(color, color))
			end

			set_death_item("Pandora's Cube")
			npc_says("")
			display_big_message("Big Message")
			display_console_message("Console message, [b]blue[/b], not blue.")
			end_dialog()
		end,
	},
	{
		id = "node99",
		text = _"logout",
		code = function()
			end_dialog()
		end,
	},
}
