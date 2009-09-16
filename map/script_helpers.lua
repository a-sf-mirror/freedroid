-- aliases for dialogs
next = set_next_node
show = enable_node
hide = disable_node

-- gettext function for dialogs
function _(a)
    return a
end

-- compute the "town score" to determine whether the player can become
-- a red guard
function get_town_score()
    town_score = 0
    if done_quest("Bender's problem") then
	town_score = town_score + 10
    end

    if done_quest("The yellow toolkit") then
	town_score = town_score + 15
    end

    if done_quest("Anything but the army snacks, please!") then
	town_score = town_score + 10
    end

    if done_quest("Novice Arena") then
	town_score = town_score + 10
    end

    if done_quest("Time to say goodnight") then
	town_score = town_score + 20
    end

    if done_quest("Opening a can of bots...") then
	town_score = town_score + 15
    end

    return town_score
end

-- this table describes obstacle states
obstacle_states = {
	[6] = {["opened"] = 6, ["closed"] = 26,},
	[11] = {["opened"] = 11, ["closed"] = 27,},
	[26] = {["closed"] = 26, ["opened"] = 6,},
	[27] = {["closed"] = 27, ["opened"] = 11,},
	[32] = {["enabled"] = 32, ["disabled"] = 324,},
	[34] = {["enabled"] = 34, ["disabled"] = 326,},
	[181] = {["opened"] = 181, ["closed"] = 191,},
	[186] = {["opened"] = 186, ["closed"] = 192,},
	[191] = {["closed"] = 191, ["opened"] = 181,},
	[192] = {["closed"] = 192, ["opened"] = 186,},
	[235] = {["broken"] = 237,},
	[236] = {["broken"] = 237,},
        [237] = {["broken"] = 237,},
	[322] = {["broken"] = 237,},
	[324] = {["disabled"] = 324, ["enabled"] = 32,},
	[326] = {["disabled"] = 326, ["enabled"] = 34,},
	[351] = {["closed"] = 351, ["opened"] = 353,},
	[352] = {["closed"] = 352, ["opened"] = 358,},
	[353] = {["opened"] = 353, ["closed"] = 351,},
	[358] = {["opened"] = 358, ["closed"] = 352,},
};

function get_obstacle_state_id(id, state)
	a = rawget(obstacle_states, id);
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
	id = get_obstacle_type(label);

	change_obstacle_type(label, get_obstacle_state_id(id, state));
end

