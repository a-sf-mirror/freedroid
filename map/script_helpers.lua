-- aliases for dialogs
next = set_next_node
show = enable_node
hide = disable_node

-- gettext function for dialogs
function _(a)
    return a
end

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
