-- set the random seed
math.randomseed(os.time())

-- aliases for dialogs
next = set_next_node
show = enable_node
hide = disable_node
get_program = get_program_revision

-- gettext function for dialogs
function _(a)
	return a
end

-- compute the "town score" to determine whether the player can become
-- a red guard
function get_town_score()
	local town_score = 0

	if (has_quest("Deliverance")) then
		if (data_cube_lost) then
			town_score = town_score - 5
		elseif (done_quest("Deliverance")) then
			town_score = town_score + 5
		end
	end

	if (done_quest("Bender's problem")) then
		town_score = town_score + 10
	end

	if (done_quest("The yellow toolkit")) then
		town_score = town_score + 15
	end

	if (done_quest("Anything but the army snacks, please!")) then
		town_score = town_score + 10
	end

	if (done_quest("Novice Arena")) then
		town_score = town_score + 10
	end

	if (done_quest("Time to say goodnight")) then
		town_score = town_score + 20
	end

	if (done_quest("Opening a can of bots...")) then
		town_score = town_score + 15
	end

	if (KevinMurderCongratulation) then
		town_score = town_score + 20
	end

    return town_score
end

-- this table describes obstacle states
obstacle_states = {
	  [3] = {["opened"] = -1, ["closed"] = 3,},
	  [4] = {["opened"] = -1, ["closed"] = 4,},
	  [6] = {["opened"] =  6, ["closed"] = 26,},
	  [7] = {["opened"] =  6, ["closed"] = 26,},
	  [8] = {["opened"] =  6, ["closed"] = 26,},
	  [9] = {["opened"] =  6, ["closed"] = 26,},
	 [10] = {["opened"] =  6, ["closed"] = 26,},

	 [11] = {["opened"] = 11, ["closed"] = 27,},
	 [12] = {["opened"] = 11, ["closed"] = 27,},
	 [13] = {["opened"] = 11, ["closed"] = 27,},
	 [14] = {["opened"] = 11, ["closed"] = 27,},
	 [15] = {["opened"] = 11, ["closed"] = 27,},

	 [26] = {["closed"] = 26, ["opened"] = 6,},
	 [27] = {["closed"] = 27, ["opened"] = 11,},

	 [32] = {["enabled"] = 32, ["disabled"] = 324,},
	 [33] = {["enabled"] = 33, ["disabled"] = 325,},
	 [34] = {["enabled"] = 34, ["disabled"] = 326,},
	 [35] = {["enabled"] = 35, ["disabled"] = 327,},

	[181] = {["opened"] = 181, ["closed"] = 191,},
	[182] = {["opened"] = 181, ["closed"] = 191,},
	[183] = {["opened"] = 181, ["closed"] = 191,},
	[184] = {["opened"] = 181, ["closed"] = 191,},
	[185] = {["opened"] = 181, ["closed"] = 191,},

	[186] = {["opened"] = 186, ["closed"] = 192,},
	[187] = {["opened"] = 186, ["closed"] = 192,},
	[188] = {["opened"] = 186, ["closed"] = 192,},
	[189] = {["opened"] = 186, ["closed"] = 192,},
	[190] = {["opened"] = 186, ["closed"] = 192,},

	[191] = {["closed"] = 191, ["opened"] = 181,},
	[192] = {["closed"] = 192, ["opened"] = 186,},

	[235] = {["broken"] = 237,},
	[236] = {["broken"] = 237,},
	[237] = {["broken"] = 237,},
	[323] = {["broken"] = 237,},

	[324] = {["disabled"] = 324, ["enabled"] = 32,},
	[325] = {["disabled"] = 325, ["enabled"] = 33,},
	[326] = {["disabled"] = 326, ["enabled"] = 34,},
	[327] = {["disabled"] = 327, ["enabled"] = 35,},

	[351] = {["closed"] = 351, ["opened"] = 353,},
	[352] = {["closed"] = 352, ["opened"] = 358,},

	[353] = {["opened"] = 353, ["closed"] = 351,},
	[354] = {["opened"] = 353, ["closed"] = 351,},
	[355] = {["opened"] = 353, ["closed"] = 351,},
	[356] = {["opened"] = 353, ["closed"] = 351,},
	[357] = {["opened"] = 353, ["closed"] = 351,},

	[358] = {["opened"] = 358, ["closed"] = 352,},
	[359] = {["opened"] = 358, ["closed"] = 352,},
	[360] = {["opened"] = 358, ["closed"] = 352,},
	[361] = {["opened"] = 358, ["closed"] = 352,},
	[362] = {["opened"] = 358, ["closed"] = 352,},

	[388] = {["enabled"] = 388, ["disabled"] = 407,},
	[407] = {["disabled"] = 407, ["enabled"] = 388,},

	-- glass walls
	[281] = {["intact"] = 281, ["broken"] = 348,},
	[282] = {["intact"] = 282, ["broken"] = 446,},

	[348] = {["broken"] = 348, ["intact"] = 281,},
	[446] = {["broken"] = 446, ["intact"] = 282,},

	-- chests
	[28] = {["locked"] = 28, ["unlocked"] = 30,},
	[29] = {["locked"] = 29, ["unlocked"] = 31,},

	[30] = {["unlocked"] = 30, ["locked"] = 28,},
	[31] = {["unlocked"] = 31, ["locked"] = 29,},

	[372] = {["locked"] = 372, ["unlocked"] = 374,},
	[373] = {["locked"] = 373, ["unlocked"] = 375,},

	[374] = {["unlocked"] = 374, ["locked"] = 372,},
	[375] = {["unlocked"] = 375, ["locked"] = 373,},


	[376] = {["locked"] = 376, ["unlocked"] = 378,},
	[377] = {["locked"] = 377, ["unlocked"] = 379,},

	[378] = {["unlocked"] = 378, ["locked"] = 376,},
	[379] = {["unlocked"] = 379, ["locked"] = 377,},

	-- lootable shelves
	[466] = {["unlooted"] = 466, ["looted"] = 467,},
	[467] = {["looted"] = 467, ["unlooted"] = 466,},

	[468] = {["unlooted"] = 468, ["looted"] = 469,},
	[469] = {["looted"] = 469, ["unlooted"] = 468,},

	[470] = {["unlooted"] = 470, ["looted"] = 471,},
	[471] = {["looted"] = 471, ["unlooted"] = 470,},

	[472] = {["unlooted"] = 472, ["looted"] = 473,},
	[473] = {["looted"] = 473, ["unlooted"] = 472,},

	[474] = {["unlooted"] = 474, ["looted"] = 475,},
	[475] = {["looted"] = 475, ["unlooted"] = 474,},

	[476] = {["unlooted"] = 476, ["looted"] = 477,},
	[477] = {["looted"] = 477, ["unlooted"] = 476,},

	[478] = {["unlooted"] = 478, ["looted"] = 479,},
	[479] = {["looted"] = 479, ["unlooted"] = 478,},

	[480] = {["unlooted"] = 480, ["looted"] = 481,},
	[481] = {["looted"] = 481, ["unlooted"] = 480,},

	-- trapdoors
	[150] = {["opened"] = 150, ["closed"] = 482,},
	[482] = {["closed"] = 482, ["opened"] = 150,},

	[149] = {["opened"] = 149, ["closed"] = 483,},
	[483] = {["closed"] = 483, ["opened"] = 149,},

	[24] = {["opened"] = 24, ["closed"] = 61,},
	[61] = {["closed"] = 61, ["opened"] = 24,},

	[23] = {["opened"] = 23, ["closed"] = 62,},
	[62] = {["closed"] = 62, ["opened"] = 23,},
};

function get_obstacle_state_id(id, state)
	local a = rawget(obstacle_states, id);
	if (a == nil) then
		error("Obstacle number " .. id .. " does not have any states defined.", 2);
	end

	a = rawget(a, state)
	if (a == nil) then
		error("State \"" .. state .. "\" is not defined for obstacle type " .. id .. ".", 2);
	end

	return a
end

function change_obstacle_state(label, state)
	local id = get_obstacle_type(label);

	change_obstacle_type(label, get_obstacle_state_id(id, state));
end

function cmp_obstacle_state(label, state)
	local id = get_obstacle_type(label);

	return (id == get_obstacle_state_id(id, state));
end

-- Quest functions
function add_quest(quest, text)
	if (not running_benchmark()) then
		if done_quest(quest) or
		   has_quest(quest) then
			print("\n\tSEVERE ERROR")
			print("\tTried to assign already assigned quest!")
			print("\tWe will continue execution, quest is:")
			print(quest)
		end
	end
	assign_quest(quest, text)
	play_sound("effects/Mission_Status_Change_Sound_0.ogg")
	if (run_from_dialog()) then
		cli_says("   ".._"New Quest assigned: " .. quest,"NO_WAIT")
		npc_says("")
	else
		display_big_message("   ".._"New Quest assigned: " .. quest)
	end
end

function update_quest(quest, text)
	if (has_quest(quest)) then
		add_diary_entry(quest, text)
		play_sound("effects/Mission_Status_Change_Sound_0.ogg")
		if (run_from_dialog()) then
			cli_says("   ".._"Quest log updated: " .. quest, "NO_WAIT")
			npc_says("")
		else
			display_big_message("   ".._"Quest log updated: " .. quest)
		end
	end
end

function end_quest(quest, text)
	if (done_quest(quest)) then
		print("\n\tERROR")
		print("\tTried to end already done quest!")
		print("\tWe will continue execution, quest is:")
		print(quest)
	elseif (not has_quest(quest)) then
		print("\n\tSEVERE ERROR")
		print("\tTried to end never assigned quest!")
		print("\tWe will continue execution, quest is:")
		print(quest)
	end

	complete_quest(quest, text)
	play_sound("effects/Mission_Status_Change_Sound_0.ogg")
	if (run_from_dialog()) then
		cli_says("   ".._"Quest completed: " .. quest,"NO_WAIT")
		npc_says("")
	else
		display_big_message("   ".._"Quest completed: " .. quest)
	end
end

-- Set faction and name
function npc_faction(faction, name)
	set_npc_faction(faction)
	if (name == nil) then
	else
		set_bot_name(name)
	end
end

function chat_says_format(text, ...)
	local arg = {...}
	local no_wait = "WAIT"
	if (arg[#arg] == "NO_WAIT") then
		no_wait = "NO_WAIT"
		table.remove(arg)
	end
	text = string.format(text, unpack(arg))
	return text, no_wait
end

function tux_says(text, ...)
	local text, no_wait = chat_says_format('\1- ' .. text .. '\n', ...)
	chat_says(text, no_wait)
end

function npc_says(text, ...)
	local text, no_wait = chat_says_format('\2' .. text .. '\n', ...)
	chat_says(apply_bbcode(text,"\3","\2"), no_wait)
end

function cli_says(text, ...)
	local text, no_wait = chat_says_format('\3' .. text, ...)
	chat_says(text, no_wait)
end

function display_console_message(text)
	event_display_console_message(apply_bbcode(text,"\4","\5"))
end

-- Magic font-numbers table:
-- number  font face         default
-- '\1'    medium orange    tux_says()
-- '\2'    medium yellow    npc_says()
-- '\3'    medium blue      cli_cays()
-- '\4'    small blue
-- '\5'    small yellow   display_console_message()
function apply_bbcode(text,magic_num_b,magic_num_nrm)
	local text = string.gsub(text,  '%[b%]', magic_num_b)
	text = string.gsub(text, '%[/b%]', magic_num_nrm)
	return text
end

function npc_says_random(...)
	arg = {...}
	if (arg[#arg] == "NO_WAIT") then
		npc_says(arg[math.random(#arg-1)],"NO_WAIT")
	else
		npc_says(arg[math.random(#arg)])
	end
end

function tux_says_random(...)
	arg = {...}
	if (arg[#arg] == "NO_WAIT") then
		tux_says(arg[math.random(#arg-1)],"NO_WAIT")
	else
		tux_says(arg[math.random(#arg)])
	end
end

function get_random(...)
	arg = {...}
	return arg[math.random(#arg)]
end

function del_gold(gold_amount)
	if (gold_amount <= get_gold()) then
		add_gold(-gold_amount)
		return true
	else
		return false
	end
end

function count_item(item_name) 
	local number = count_item_backpack(item_name) 
	if has_item_equipped(item_name) then
		return number + 1
	else
		return number
	end
end

function has_item_backpack(item_name)
	return (count_item_backpack(item_name) > 0)
end

function del_item(item_name) 
	if (count_item_backpack(item_name) > 0) then
		del_item_backpack(item_name)
		return true
	else
		return false
	end
end

function del_points(num_points)
	if (get_training_points() >= num_points) then
		del_training_points(num_points)
		return true
	else
		return false
	end
end

function can_tux_train(gold_amount, num_points)
	if (get_gold() < gold_amount) then
		return false
	end
	
	if (get_training_points() < num_points) then
		return false
	end

	return true
end

function train_skill(gold_amount, num_points, skill)
	if (get_gold() < gold_amount) then
		return false
	end
	
	if (get_training_points() < num_points) then
		return false
	end

	add_gold(-gold_amount)
	del_training_points(num_points)
	improve_skill(skill)
	return true
end

function train_program(gold_amount, num_points, program)
	if (get_gold() < gold_amount) then
		return false
	end

	if (get_training_points() < num_points) then
		return false
	end

	add_gold(-gold_amount)
	del_training_points(num_points)
	improve_program(program)
	return true
end

function tux_hp_ratio()
	return get_tux_hp()/get_tux_max_hp()
end

function npc_damage_ratio()
	return npc_damage_amount()/npc_max_health()
end

function del_health(num_points)
	if (num_points < get_tux_hp()) then
		hurt_tux(num_points)
		return true
	else
		return false
	end
end

function drain_bot()
--[[ npc_damage_amount - npc_max_health   will return the HPs of the droid
	 for example:  max_HP = 20, DMG_amount = 10 ; 10-20 = -10
					 hurt_tux(-10) will heal tux by 10 HP
	 to have the difficulty lvl influence the HP tux gets we divide the
	 heal-HP by the difficulty_lvl+1 (to prevent division by zero)
	 -10/2 (difficulty_lvl = 3/hard) = -5.  Tux will be healed by 5 HP ]]--
	hurt_tux((npc_damage_amount() - npc_max_health())/(difficulty_level()+1))
	drop_dead()
end

function difficulty()
	local levels = {"easy", "normal", "hard"}
	return levels[difficulty_level()+1]
end


function show_node_if(case, ...)
	local arg = {...}
	if (case) then
		show(unpack(arg))
	else
		hide(unpack(arg))
	end
end

function get_random_bot_59()
	local bot_59 = get_random(123, 139, 247, 249, 296, 296, 302, 302, 329, 329, 420,
			  420, 476, 476, 493, 493, 516, 516, 516, 571, 571, 571, 598,
			  598, 598, 614, 614, 614, 615, 615, 615, 629, 629, 629, 711,
			  711, 711, 742, 742, 742, 751, 751, 751, 821, 821, 821, 834,
			  834, 834, 883, 883, 883, 999, 999, 999, 543,
			  543, 543, 603) -- "GUN")
	return bot_59
end

function scandir(subdir, filter, exclude)
	local filtered = {}
	local exclude_dict = {}
	local files = dir(subdir)
	filter = filter or ".*"
	exclude = exclude or {}

	if (files) then
		-- transform the exclude list into a 'dictionary'
		for _,v in ipairs(exclude) do
			exclude_dict[v] = true
		end

		-- for each file in the directory, check if it matches the regexp
		-- filter and if it is not in the exclude list
		for _,file in ipairs(files) do
			if ((file:match(filter) == file) and not exclude_dict[file]) then
				filtered[#filtered + 1] = file
			end
		end

		-- alphabetic sort
		table.sort(filtered)
	end

	return filtered
end

function has_item(...)
	for _,item in ipairs({...}) do
		if (not (count_item(item) > 0)) then
			return false
		end
	end
	return true
end


--[[
>>>>>>>>>>>>>>>>>>                                  <<<<<<<<<<<<<<<<<<<<
                  24 / Debug Level  below this line:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
]]--

function closelvl24doors()
	change_obstacle_state("24door1", "closed")
	change_obstacle_state("24door2", "closed")
	change_obstacle_state("24door3", "closed")
	change_obstacle_state("24door4", "closed")
end

function openlvl24doors()
	change_obstacle_state("24door1", "opened")
	change_obstacle_state("24door2", "opened")
	change_obstacle_state("24door3", "opened")
	change_obstacle_state("24door4", "opened")
end

function level24obstacles()
	local randobstacletype
	function level24newid()
		randobstacletype = math.random(0,465) -- 466 obstacles
	end
	function level24idcheck()
		if (randobstacletype == 22) or
		   (randobstacletype == 23) or
		   (randobstacletype == 24) or
		   (randobstacletype == 25) or
		   (randobstacletype == 61) or
		   (randobstacletype == 62) then
		   -- cat ./map/obstacle_specs.lua | grep "image_filenames" | nl -v 0| grep "DUMMY"
			display_big_message(randobstacletype.._" getting new id")
			level24newid()
			level24idcheck()
		end
	end
	level24newid()
	level24idcheck()
	display_big_message(randobstacletype)
	change_obstacle_type("24randobst", randobstacletype)
end

function guy_fail(test, ...)
	print[[ERROR! The following test failed:]]
	print(test)
	npc_says("%s failed!" ,test , "NO_WAIT")
	end_dialog()
	exit_game()
end

