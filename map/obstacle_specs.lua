function borders(left, right, upper, lower)
	if upper == nil then
		return { -left / 2, left / 2, -right / 2, right / 2 }
	end
	return { left, right, upper, lower }
end

obstacle {
	image_filenames = "iso_tree_stump.png",
	borders = borders(0.60, 0.60),
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_wall_grey_ns.png",
	borders = borders(0.40, 1.10),
	flags = { IS_VERTICAL, BLOCKS_VISION },
}

obstacle {
	image_filenames = "iso_wall_grey_ew.png",
	borders = borders(1.10, 0.40),
	flags = { IS_HORIZONTAL, BLOCKS_VISION },
}

obstacle {
	image_filenames = "iso_wall_grey_handle_ns.png",
	borders = borders(0.40, 1.10),
	flags = { IS_VERTICAL, BLOCKS_VISION },
}

obstacle {
	image_filenames = "iso_wall_grey_handle_ew.png",
	borders = borders(1.10, 0.40),
	flags = { IS_HORIZONTAL, BLOCKS_VISION },
}

obstacle {
	image_filenames = "iso_tree_big.png",
	borders = borders(1.30, 1.30),
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_door_unlocked_closed_we_1.png",
	borders = borders(1.00, 0.40),
	flags = { IS_HORIZONTAL, IS_WALKABLE },
	transparency = NO_TRANSPARENCY,
	animation = "door",
	groups = "blue doors"
}

obstacle {
	image_filenames = "iso_door_unlocked_opened_we_2.png",
	borders = borders(1.00, 0.40),
	flags = { IS_WALKABLE },
	transparency = NO_TRANSPARENCY,
	animation = "door",
}

obstacle {
	image_filenames = "iso_door_unlocked_opened_we_3.png",
	borders = borders(1.00, 0.40),
	flags = { IS_WALKABLE },
	transparency = NO_TRANSPARENCY,
	animation = "door",
}

obstacle {
	image_filenames = "iso_door_unlocked_opened_we_4.png",
	borders = borders(1.00, 0.40),
	flags = { IS_WALKABLE },
	transparency = NO_TRANSPARENCY,
	animation = "door",
}

obstacle {
	image_filenames = "iso_door_unlocked_opened_we_5.png",
	flags = { BLOCKS_VISION },
	transparency = NO_TRANSPARENCY,
	animation = "door",
}

obstacle {
	image_filenames = "iso_door_unlocked_closed_ns_1.png",
	borders = borders(0.40, 1.00),
	flags = { IS_VERTICAL, IS_WALKABLE },
	transparency = NO_TRANSPARENCY,
	animation = "door",
	groups = "blue doors"
}

obstacle {
	image_filenames = "iso_door_unlocked_opened_ns_2.png",
	borders = borders(0.40, 1.00),
	flags = { IS_WALKABLE },
	transparency = NO_TRANSPARENCY,
	animation = "door",
}

obstacle {
	image_filenames = "iso_door_unlocked_opened_ns_3.png",
	borders = borders(0.40, 1.00),
	flags = { IS_WALKABLE },
	transparency = NO_TRANSPARENCY,
	animation = "door",
}

obstacle {
	image_filenames = "iso_door_unlocked_opened_ns_4.png",
	borders = borders(0.40, 1.00),
	flags = { IS_WALKABLE },
	transparency = NO_TRANSPARENCY,
	animation = "door",
}

obstacle {
	image_filenames = "iso_door_unlocked_opened_ns_5.png",
	flags = { BLOCKS_VISION },
	transparency = NO_TRANSPARENCY,
	animation = "door",
}

obstacle {
	image_filenames = { "iso_purplecloud_3.png", "iso_purplecloud_4.png", "iso_purplecloud_5.png", "iso_purplecloud_1.png", "iso_purplecloud_2.png" },
	emitted_light_strength = { 20, 19, 18, 19, 20 },
	transparency = NO_TRANSPARENCY,
	animation_fps = 10
}

obstacle {
	image_filenames = { "iso_teleport_0000.png", "iso_teleport_0001.png", "iso_teleport_0002.png", "iso_teleport_0003.png", "iso_teleport_0004.png" },
	emitted_light_strength = { 20, 19, 18, 19, 20 },
	transparency = NO_TRANSPARENCY,
	animation_fps = 10
}

obstacle {
	image_filenames = "DUMMY OBSTACLE"
}

obstacle {
	image_filenames = "DUMMY OBSTACLE"
}

obstacle {
	image_filenames = "DUMMY OBSTACLE"
}

obstacle {
	image_filenames = { "iso_droidnest_1.png", "iso_droidnest_2.png", "iso_droidnest_3.png", "iso_droidnest_4.png", "iso_droidnest_5.png" },
	flags = { NEEDS_PRE_PUT },
	emitted_light_strength = 10,
	transparency = NO_TRANSPARENCY,
	animation_fps = 3
}

obstacle {
	image_filenames = "DUMMY OBSTACLE"
}

obstacle {
	image_filenames = "DUMMY OBSTACLE"
}

obstacle {
	image_filenames = "DUMMY OBSTACLE"
}

obstacle {
	image_filenames = "DUMMY OBSTACLE"
}

obstacle {
	image_filenames = "iso_door_locked_we.png",
	borders = borders(1.10, 0.40),
	flags = { IS_HORIZONTAL },
	transparency = NO_TRANSPARENCY,
	groups = "red door"
}

obstacle {
	image_filenames = "iso_door_locked_ns.png",
	borders = borders(0.40, 1.10),
	flags = { IS_VERTICAL },
	transparency = NO_TRANSPARENCY,
	groups = "red door"
}

obstacle {
	image_filenames = "iso_chest_grey_closed_n.png",
	label = "Chest",
	borders = borders(0.80, 0.60),
	flags = { GROUND_LEVEL, IS_CLICKABLE },
	transparency = NO_TRANSPARENCY,
	action = "chest",
}

obstacle {
	image_filenames = "iso_chest_grey_closed_w.png",
	label = "Chest",
	borders = borders(0.60, 0.80),
	flags = { GROUND_LEVEL, IS_CLICKABLE },
	transparency = NO_TRANSPARENCY,
	action = "chest",
}

obstacle {
	image_filenames = "iso_chest_grey_opened_n.png",
	borders = borders(0.80, 0.60),
	flags = { GROUND_LEVEL },
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_chest_grey_opened_w.png",
	borders = borders(0.60, 0.80),
	flags = { GROUND_LEVEL },
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_autogun_on_w.png",
	borders = borders(0.70, 0.70),
	transparency = NO_TRANSPARENCY,
	animation = "autogun",
}

obstacle {
	image_filenames = "iso_autogun_on_n.png",
	borders = borders(0.70, 0.70),
	transparency = NO_TRANSPARENCY,
	animation = "autogun",
}

obstacle {
	image_filenames = "iso_autogun_on_e.png",
	borders = borders(0.70, 0.70),
	transparency = NO_TRANSPARENCY,
	animation = "autogun",
}

obstacle {
	image_filenames = "iso_autogun_on_s.png",
	borders = borders(0.70, 0.70),
	transparency = NO_TRANSPARENCY,
	animation = "autogun",
}

obstacle {
	image_filenames = "iso_wall_cave_we.png",
	borders = borders(1.50, 1.00),
	flags = { IS_HORIZONTAL, BLOCKS_VISION },
	transparency = NO_TRANSPARENCY,
	groups = "cave wall"
}

obstacle {
	image_filenames = "iso_wall_cave_ns.png",
	borders = borders(1.00, 1.50),
	flags = { IS_VERTICAL, BLOCKS_VISION },
	transparency = NO_TRANSPARENCY,
	groups = "cave wall"
}

obstacle {
	image_filenames = "iso_wall_cave_curve_ws.png",
	borders = borders(1.00, 1.00),
	flags = { BLOCKS_VISION, CORNER_NE },
	transparency = NO_TRANSPARENCY,
	groups = "cave wall"
}

obstacle {
	image_filenames = "iso_wall_cave_curve_nw.png",
	borders = borders(1.00, 1.00),
	flags = { BLOCKS_VISION, CORNER_SE },
	transparency = NO_TRANSPARENCY,
	groups = "cave wall"
}

obstacle {
	image_filenames = "iso_wall_cave_curve_es.png",
	borders = borders(1.00, 1.00),
	flags = { BLOCKS_VISION, CORNER_NW },
	transparency = NO_TRANSPARENCY,
	groups = "cave wall"
}

obstacle {
	image_filenames = "iso_wall_cave_curve_ne.png",
	borders = borders(1.00, 1.00),
	flags = { BLOCKS_VISION, CORNER_SW },
	transparency = NO_TRANSPARENCY,
	groups = "cave wall"
}

obstacle {
	image_filenames = "iso_pot.png",
	borders = borders(0.50, 0.50),
	flags = { IS_SMASHABLE, GROUND_LEVEL },
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_terminal_s.png",
	label = "Terminal",
	borders = borders(0.80, 0.80),
	flags = { IS_CLICKABLE },
	transparency = NO_TRANSPARENCY,
	action = "terminal",
}

obstacle {
	image_filenames = "iso_terminal_e.png",
	label = "Terminal",
	borders = borders(0.80, 0.80),
	flags = { IS_CLICKABLE },
	transparency = NO_TRANSPARENCY,
	action = "terminal",
}

obstacle {
	image_filenames = "iso_terminal_n.png",
	label = "Terminal",
	borders = borders(0.80, 0.80),
	flags = { IS_CLICKABLE },
	transparency = NO_TRANSPARENCY,
	action = "terminal",
}

obstacle {
	image_filenames = "iso_terminal_w.png",
	label = "Terminal",
	borders = borders(0.80, 0.80),
	flags = { IS_CLICKABLE },
	transparency = NO_TRANSPARENCY,
	action = "terminal",
}

obstacle {
	image_filenames = "iso_pillar_high.png",
	borders = borders(-0.50, 0.25, -0.50, 0.25),
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_pillar_short.png",
	borders = borders(-0.50, 0.25, -0.50, 0.25),
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_computerpillar_e.png",
	borders = borders(1.00, 1.00),
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_barrel.png",
	label = "Barrel",
	borders = borders(0.70, 0.70),
	flags = { BLOCKS_VISION, IS_SMASHABLE, DROPS_RANDOM_TREASURE, IS_CLICKABLE },
	transparency = NO_TRANSPARENCY,
	action = "barrel",
}

obstacle {
	image_filenames = "iso_barrel_rusty.png",
	label = "Barrel",
	borders = borders(0.70, 0.70),
	flags = { BLOCKS_VISION, IS_SMASHABLE, DROPS_RANDOM_TREASURE, IS_CLICKABLE },
	transparency = NO_TRANSPARENCY,
	action = "barrel",
}

obstacle {
	image_filenames = "iso_crate_ns.png",
	label = "Crate",
	borders = borders(0.80, 0.95),
	flags = { BLOCKS_VISION, IS_SMASHABLE, DROPS_RANDOM_TREASURE, IS_CLICKABLE },
	transparency = NO_TRANSPARENCY,
	action = "barrel",
}

obstacle {
	image_filenames = "iso_crate_we.png",
	label = "Crate",
	borders = borders(0.80, 0.75),
	flags = { BLOCKS_VISION, IS_SMASHABLE, DROPS_RANDOM_TREASURE, IS_CLICKABLE },
	transparency = NO_TRANSPARENCY,
	action = "barrel",
}

obstacle {
	image_filenames = "iso_lamp_s.png",
	borders = borders(0.50, 0.50),
	flags = { GROUND_LEVEL },
	emitted_light_strength = 24,
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_raylamp_right.png",
	borders = borders(-0.60, 0.55, -0.60, 0.50),
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_raylamp_down.png",
	borders = borders(-0.60, 0.55, -0.60, 0.55),
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_raylamp_left.png",
	borders = borders(-0.60, 0.50, -0.60, 0.55),
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_raylamp_up.png",
	borders = borders(-0.60, 0.50, -0.60, 0.50),
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_fence_white_ns.png",
	borders = borders(1.10, 2.20),
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_fence_white_we.png",
	borders = borders(2.20, 1.10),
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "DUMMY OBSTACLE"
}

obstacle {
	image_filenames = "DUMMY OBSTACLE"
}

obstacle {
	image_filenames = "iso_fence_wire_red_ns.png",
	borders = borders(0.80, 2.20),
	flags = { GROUND_LEVEL },
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_fence_wire_red_we.png",
	borders = borders(2.20, 0.80),
	flags = { GROUND_LEVEL },
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_fence_wire_green_ns.png",
	borders = borders(0.80, 2.20),
	flags = { GROUND_LEVEL },
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_fence_wire_green_we.png",
	borders = borders(2.20, 0.80),
	flags = { GROUND_LEVEL },
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_urinal_w.png",
	borders = borders(0.40, 0.40),
	flags = { IS_SMASHABLE, GROUND_LEVEL },
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_urinal_s.png",
	borders = borders(0.40, 0.40),
	flags = { IS_SMASHABLE, GROUND_LEVEL },
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_toilet_white_s.png",
	borders = borders(0.40, 0.40),
	flags = { IS_SMASHABLE, GROUND_LEVEL },
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_toilet_white_e.png",
	borders = borders(0.40, 0.40),
	flags = { IS_SMASHABLE, GROUND_LEVEL },
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_toilet_beige_w.png",
	borders = borders(0.68, 0.50),
	flags = { IS_SMASHABLE, GROUND_LEVEL },
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_toilet_beige_n.png",
	borders = borders(0.50, 0.68),
	flags = { IS_SMASHABLE, GROUND_LEVEL },
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_toilet_beige_e.png",
	borders = borders(0.68, 0.50),
	flags = { IS_SMASHABLE, GROUND_LEVEL },
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_toilet_beige_s.png",
	borders = borders(0.50, 0.68),
	flags = { IS_SMASHABLE, GROUND_LEVEL },
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_chair_brown_w.png",
	borders = borders(0.40, 0.40),
	flags = { IS_SMASHABLE },
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_chair_brown_n.png",
	borders = borders(0.40, 0.40),
	flags = { IS_SMASHABLE },
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_chair_brown_e.png",
	borders = borders(0.40, 0.40),
	flags = { IS_SMASHABLE },
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_chair_brown_s.png",
	borders = borders(0.40, 0.40),
	flags = { IS_SMASHABLE },
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_workdesk_w.png",
	borders = borders(0.40, 1.00),
	flags = { IS_SMASHABLE, GROUND_LEVEL },
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_workdesk_n.png",
	borders = borders(1.00, 0.40),
	flags = { IS_SMASHABLE, GROUND_LEVEL },
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_workdesk_e.png",
	borders = borders(0.40, 1.00),
	flags = { IS_SMASHABLE, GROUND_LEVEL },
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_workdesk_s.png",
	borders = borders(1.00, 0.40),
	flags = { IS_SMASHABLE, GROUND_LEVEL },
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_chair_white_w.png",
	borders = borders(0.40, 0.40),
	flags = { IS_SMASHABLE, GROUND_LEVEL },
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_chair_white_n.png",
	borders = borders(0.40, 0.40),
	flags = { IS_SMASHABLE, GROUND_LEVEL },
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_chair_white_s.png",
	borders = borders(0.40, 0.40),
	flags = { IS_SMASHABLE, GROUND_LEVEL },
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_chair_white_e.png",
	borders = borders(0.40, 0.40),
	flags = { IS_SMASHABLE, GROUND_LEVEL },
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_bed_white_w.png",
	borders = borders(1.10, 0.70),
	flags = { IS_SMASHABLE, GROUND_LEVEL },
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_bed_white_n.png",
	borders = borders(0.70, 1.10),
	flags = { IS_SMASHABLE, GROUND_LEVEL },
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_bed_white_e.png",
	borders = borders(1.10, 0.70),
	flags = { IS_SMASHABLE, GROUND_LEVEL },
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_bed_white_s.png",
	borders = borders(0.70, 1.10),
	flags = { IS_SMASHABLE, GROUND_LEVEL },
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_bookshelf_long_w.png",
	borders = borders(0.60, 2.20),
	flags = { BLOCKS_VISION },
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_bookshelf_long_s.png",
	borders = borders(2.20, 0.60),
	flags = { BLOCKS_VISION },
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_bookshelf_long_e.png",
	borders = borders(0.60, 2.20),
	flags = { BLOCKS_VISION },
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_bookshelf_long_n.png",
	borders = borders(2.20, 0.60),
	flags = { BLOCKS_VISION },
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_bookshelf_s.png",
	borders = borders(1.10, 0.60),
	flags = { BLOCKS_VISION },
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_bookshelf_e.png",
	borders = borders(0.60, 1.10),
	flags = { BLOCKS_VISION },
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_bookshelf_w.png",
	borders = borders(0.60, 1.10),
	flags = { BLOCKS_VISION },
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_bookshelf_n.png",
	borders = borders(1.10, 0.60),
	flags = { BLOCKS_VISION },
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_bench_white_w.png",
	borders = borders(0.70, 1.30),
	flags = { IS_SMASHABLE, GROUND_LEVEL },
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_bench_white_s.png",
	borders = borders(1.30, 0.70),
	flags = { IS_SMASHABLE, GROUND_LEVEL },
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_bench_white_n.png",
	borders = borders(1.30, 0.70),
	flags = { IS_SMASHABLE, GROUND_LEVEL },
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_bench_white_e.png",
	borders = borders(0.70, 1.30),
	flags = { IS_SMASHABLE, GROUND_LEVEL },
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_bathtub_w.png",
	borders = borders(1.50, 1.00),
	flags = { IS_SMASHABLE, GROUND_LEVEL },
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_bathtub_n.png",
	borders = borders(1.00, 1.50),
	flags = { IS_SMASHABLE, GROUND_LEVEL },
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_tub_ns.png",
	borders = borders(0.40, 0.50),
	flags = { IS_SMASHABLE, GROUND_LEVEL },
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_tub_we.png",
	borders = borders(0.50, 0.40),
	flags = { IS_SMASHABLE, GROUND_LEVEL },
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_curtain_ns.png",
	flags = { IS_VERTICAL, BLOCKS_VISION },
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_curtain_we.png",
	flags = { IS_HORIZONTAL, BLOCKS_VISION },
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_sofa_white_w.png",
	borders = borders(0.50, 1.00),
	flags = { GROUND_LEVEL },
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_sofa_white_s.png",
	borders = borders(1.00, 0.50),
	flags = { GROUND_LEVEL },
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_sofa_white_e.png",
	borders = borders(0.50, 1.00),
	flags = { GROUND_LEVEL },
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_sofa_white_n.png",
	borders = borders(1.00, 0.50),
	flags = { GROUND_LEVEL },
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_tree_1.png",
	borders = borders(0.60, 0.60),
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_tree_2.png",
	borders = borders(0.60, 0.60),
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_tree_3.png",
	borders = borders(0.60, 0.80),
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_wall_purple_we.png",
	borders = borders(1.10, 0.40),
	flags = { IS_HORIZONTAL, BLOCKS_VISION },
	groups = "thick wall"
}

obstacle {
	image_filenames = "iso_wall_purple_ns.png",
	borders = borders(0.40, 1.10),
	flags = { IS_VERTICAL, BLOCKS_VISION },
	groups = "thick wall"
}

obstacle {
	image_filenames = "iso_wall_purple_curve_ws.png",
	borders = borders(-0.55, 0.20, -0.20, 0.55),
	flags = { BLOCKS_VISION, CORNER_NE },
	groups = "thick wall"
}

obstacle {
	image_filenames = "iso_wall_purple_nw.png",
	borders = borders(-0.55, 0.20, -0.55, 0.20),
	flags = { BLOCKS_VISION, CORNER_SE },
	groups = "thick wall"
}

obstacle {
	image_filenames = "iso_wall_purple_es.png",
	borders = borders(-0.20, 0.55, -0.20, 0.55),
	flags = { BLOCKS_VISION, CORNER_NW },
	groups = "thick wall"
}

obstacle {
	image_filenames = "iso_wall_purple_ne.png",
	borders = borders(-0.20, 0.55, -0.55, 0.20),
	flags = { BLOCKS_VISION, CORNER_SW },
	groups = "thick wall"
}

obstacle {
	image_filenames = "iso_wall_purple_T_nwe.png",
	borders = borders(-0.55, 0.55, -0.55, 0.20),
	flags = { BLOCKS_VISION },
}

obstacle {
	image_filenames = "iso_wall_purple_T_nws.png",
	borders = borders(-0.20, 0.55, -0.55, 0.55),
	flags = { BLOCKS_VISION },
}

obstacle {
	image_filenames = "iso_wall_purple_T_wes.png",
	borders = borders(-0.55, 0.55, -0.20, 0.55),
	flags = { BLOCKS_VISION },
}

obstacle {
	image_filenames = "iso_wall_purple_T_ess.png",
	borders = borders(-0.55, 0.20, -0.55, 0.55),
	flags = { BLOCKS_VISION },
}

obstacle {
	image_filenames = "iso_wall_cave_end_w.png",
	borders = borders(1.00, 1.00),
	flags = { BLOCKS_VISION },
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_wall_cave_end_n.png",
	borders = borders(1.00, 1.00),
	flags = { BLOCKS_VISION },
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_wall_cave_end_e.png",
	borders = borders(1.00, 1.00),
	flags = { BLOCKS_VISION },
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_wall_cave_end_s.png",
	borders = borders(1.00, 1.00),
	flags = { BLOCKS_VISION },
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_wall_grey_window_ns.png",
	borders = borders(0.40, 1.10),
	flags = { IS_VERTICAL },
}

obstacle {
	image_filenames = "iso_wall_grey_window_ew.png",
	borders = borders(1.10, 0.40),
	flags = { IS_HORIZONTAL },
}

obstacle {
	image_filenames = "iso_wall_grey_striation_ns.png",
	borders = borders(0.40, 1.10),
	flags = { IS_VERTICAL, BLOCKS_VISION },
}

obstacle {
	image_filenames = "iso_wall_grey_striation_ew.png",
	borders = borders(1.10, 0.40),
	flags = { IS_HORIZONTAL, BLOCKS_VISION },
}

obstacle {
	image_filenames = "iso_wall_brick_we.png",
	borders = borders(1.20, 0.80),
	flags = { IS_HORIZONTAL, BLOCKS_VISION },
	after_smashing = 235,
	groups = "brick wall"
}

obstacle {
	image_filenames = "iso_wall_brick_ns.png",
	borders = borders(0.80, 1.20),
	flags = { IS_VERTICAL, BLOCKS_VISION },
	after_smashing = 236,
	groups = "brick wall"
}

obstacle {
	image_filenames = "iso_wall_brick_end_w.png",
	borders = borders(1.00, 1.00),
	flags = { BLOCKS_VISION },
}

obstacle {
	image_filenames = "iso_wall_brick_edge_ws.png",
	borders = borders(-0.60, 0.30, -0.60, 0.60),
	flags = { BLOCKS_VISION, CORNER_NE },
	groups = { "brick wall", "brick wall 2" }
}

obstacle {
	image_filenames = "iso_wall_brick_edge_ne.png",
	borders = borders(-0.60, 0.65, -0.60, 0.30),
	flags = { BLOCKS_VISION, CORNER_SW },
	groups = { "brick wall", "brick wall 2" }
}

obstacle {
	image_filenames = "iso_wall_brick_edge_es.png",
	borders = borders(-0.30, 0.60, -0.30, 0.60),
	flags = { BLOCKS_VISION, CORNER_NW },
	groups = { "brick wall", "brick wall 2" }
}

obstacle {
	image_filenames = "iso_wall_brick_edge_nw.png",
	borders = borders(-0.60, 0.30, -0.60, 0.30),
	flags = { BLOCKS_VISION, CORNER_SE },
	groups = { "brick wall", "brick wall 2" }
}

obstacle {
	image_filenames = "iso_blood_1.png",
	flags = { IS_VOLATILE, BLOCKS_VISION, NEEDS_PRE_PUT },
	transparency = NO_TRANSPARENCY,
	groups = "blood"
}

obstacle {
	image_filenames = "iso_blood_3_1.png",
	flags = { IS_VOLATILE, BLOCKS_VISION, NEEDS_PRE_PUT },
	transparency = NO_TRANSPARENCY,
	groups = "blood"
}

obstacle {
	image_filenames = "iso_blood_3_2.png",
	flags = { IS_VOLATILE, BLOCKS_VISION, NEEDS_PRE_PUT },
	transparency = NO_TRANSPARENCY,
	groups = "blood"
}

obstacle {
	image_filenames = "iso_blood_3_3.png",
	flags = { IS_VOLATILE, BLOCKS_VISION, NEEDS_PRE_PUT },
	transparency = NO_TRANSPARENCY,
	groups = "blood"
}

obstacle {
	image_filenames = "iso_blood_8.png",
	flags = { IS_VOLATILE, BLOCKS_VISION, NEEDS_PRE_PUT },
	transparency = NO_TRANSPARENCY,
	groups = "blood"
}

obstacle {
	image_filenames = "iso_blood_4.png",
	flags = { IS_VOLATILE, BLOCKS_VISION, NEEDS_PRE_PUT },
	transparency = NO_TRANSPARENCY,
	groups = "blood"
}

obstacle {
	image_filenames = "iso_blood_5.png",
	flags = { IS_VOLATILE, BLOCKS_VISION, NEEDS_PRE_PUT },
	transparency = NO_TRANSPARENCY,
	groups = "blood"
}

obstacle {
	image_filenames = "iso_blood_10.png",
	flags = { IS_VOLATILE, BLOCKS_VISION, NEEDS_PRE_PUT },
	transparency = NO_TRANSPARENCY,
	groups = "blood"
}

obstacle {
	image_filenames = "iso_trapdoor_n.png",
	flags = { BLOCKS_VISION },
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_trapdoor_w.png",
	flags = { BLOCKS_VISION },
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_shroom_white_1.png",
	borders = borders(0.40, 0.40),
	flags = { GROUND_LEVEL },
	emitted_light_strength = 10,
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_rock_big.png",
	borders = borders(1.50, 1.50),
	flags = { GROUND_LEVEL },
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_rock_small.png",
	flags = { BLOCKS_VISION, NEEDS_PRE_PUT },
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_rock_pillar.png",
	borders = borders(1.00, 1.00),
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_wall_red_ns.png",
	borders = borders(0.40, 1.10),
	flags = { IS_VERTICAL, BLOCKS_VISION },
}

obstacle {
	image_filenames = "iso_wall_red_we.png",
	borders = borders(1.10, 0.40),
	flags = { IS_HORIZONTAL, BLOCKS_VISION },
}

obstacle {
	image_filenames = "iso_wall_turqois_ns.png",
	borders = borders(0.40, 1.10),
	flags = { IS_VERTICAL, BLOCKS_VISION },
}

obstacle {
	image_filenames = "iso_wall_turqois_we.png",
	borders = borders(1.10, 0.40),
	flags = { IS_HORIZONTAL, BLOCKS_VISION },
}

obstacle {
	image_filenames = "iso_shop_counter_s.png",
	borders = borders(3.50, 1.50),
	flags = { GROUND_LEVEL },
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_shop_counter_w.png",
	borders = borders(1.50, 3.50),
	flags = { GROUND_LEVEL },
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_shelf_s.png",
	borders = borders(2.20, 0.60),
	flags = { BLOCKS_VISION },
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_shelf_e.png",
	borders = borders(0.60, 2.20),
	flags = { BLOCKS_VISION },
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_shelf_n.png",
	borders = borders(2.20, 0.60),
	flags = { BLOCKS_VISION },
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_shelf_w.png",
	borders = borders(0.60, 2.20),
	flags = { BLOCKS_VISION },
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_wall_yellow_ellipsis_we.png",
	borders = borders(-0.55, 0.55, -0.05, 0.60),
	flags = { IS_HORIZONTAL, BLOCKS_VISION },
	groups = "outer wall 1"
}

obstacle {
	image_filenames = "iso_wall_yellow_ellipsis_dots_pipes_we.png",
	borders = borders(-0.55, 0.55, -0.05, 0.60),
	flags = { IS_HORIZONTAL, BLOCKS_VISION },
	groups = "outer wall 2"
}

obstacle {
	image_filenames = "iso_wall_yellow_ellipsis_dots_we.png",
	borders = borders(-0.55, 0.55, -0.05, 0.60),
	flags = { IS_HORIZONTAL, BLOCKS_VISION },
	groups = "outer wall 3"
}

obstacle {
	image_filenames = "iso_walls_yellow_we.png",
	borders = borders(-0.55, 0.55, -0.05, 0.60),
	flags = { IS_HORIZONTAL, BLOCKS_VISION },
	groups = "outer wall 4"
}

obstacle {
	image_filenames = "iso_wall_yellow_dots_pipes_we.png",
	borders = borders(-0.55, 0.55, -0.05, 0.60),
	flags = { IS_HORIZONTAL, BLOCKS_VISION },
	groups = "outer wall 5"
}

obstacle {
	image_filenames = "iso_wall_yellow_dots_we.png",
	borders = borders(-0.55, 0.55, -0.05, 0.60),
	flags = { IS_HORIZONTAL, BLOCKS_VISION },
	groups = "outer wall 6"
}

obstacle {
	image_filenames = "iso_walls_yellow_ns.png",
	borders = borders(-0.05, 0.60, -0.55, 0.55),
	flags = { IS_VERTICAL, BLOCKS_VISION },
	groups = "outer wall 4"
}

obstacle {
	image_filenames = "iso_wall_yellow_dots_pipes_ns.png",
	borders = borders(-0.05, 0.60, -0.55, 0.55),
	flags = { IS_VERTICAL, BLOCKS_VISION },
	groups = "outer wall 5"
}

obstacle {
	image_filenames = "iso_wall_yellow_dots_ns.png",
	borders = borders(-0.05, 0.60, -0.55, 0.55),
	flags = { IS_VERTICAL, BLOCKS_VISION },
	groups = "outer wall 6"
}

obstacle {
	image_filenames = "iso_wall_yellow_ellipsis_ns.png",
	borders = borders(-0.05, 0.60, -0.55, 0.55),
	flags = { IS_VERTICAL, BLOCKS_VISION },
	groups = "outer wall 1"
}

obstacle {
	image_filenames = "iso_wall_yellow_ellipsis_dots_pipes_ns.png",
	borders = borders(-0.05, 0.60, -0.55, 0.55),
	flags = { IS_VERTICAL, BLOCKS_VISION },
	groups = "outer wall 2"
}

obstacle {
	image_filenames = "iso_wall_yellow_ellipsis_dots_ns.png",
	borders = borders(-0.05, 0.60, -0.55, 0.55),
	flags = { IS_VERTICAL, BLOCKS_VISION },
	groups = "outer wall 3"
}

obstacle {
	image_filenames = "iso_wall_yellow_curve_long_es.png",
	borders = borders(1.10, 1.10),
	flags = { BLOCKS_VISION, CORNER_NW },
	groups = { "outer wall 1", "outer wall 2", "outer wall 3", "outer wall 4", "outer wall 5", "outer wall 6" }
}

obstacle {
	image_filenames = "iso_wall_yellow_curve_long_ne.png",
	borders = borders(1.10, 1.10),
	flags = { BLOCKS_VISION, CORNER_SW },
	groups = { "outer wall 1", "outer wall 2", "outer wall 3", "outer wall 4", "outer wall 5", "outer wall 6" }
}

obstacle {
	image_filenames = "iso_wall_yellow_curve_long_nw.png",
	borders = borders(1.10, 1.10),
	flags = { BLOCKS_VISION, CORNER_SE },
	groups = { "outer wall 1", "outer wall 2", "outer wall 3", "outer wall 4", "outer wall 5", "outer wall 6" }
}

obstacle {
	image_filenames = "iso_wall_yellow_curve_long_ws.png",
	borders = borders(1.10, 1.10),
	flags = { BLOCKS_VISION, CORNER_NE },
	groups = { "outer wall 1", "outer wall 2", "outer wall 3", "outer wall 4", "outer wall 5", "outer wall 6" }
}

obstacle {
	image_filenames = "iso_gate_unlocked_closed_ns_1.png",
	borders = borders(-0.05, 0.60, -1.55, 0.55),
	flags = { IS_WALKABLE },
	animation = "door",
}

obstacle {
	image_filenames = "iso_gate_unlocked_opened_ns_2.png",
	borders = borders(1.20, 1.20),
	flags = { IS_WALKABLE },
	animation = "door",
}

obstacle {
	image_filenames = "iso_gate_unlocked_opened_ns_3.png",
	borders = borders(1.20, 1.20),
	flags = { IS_WALKABLE },
	animation = "door",
}

obstacle {
	image_filenames = "iso_gate_unlocked_opened_ns_4.png",
	borders = borders(1.20, 1.20),
	flags = { IS_WALKABLE },
	animation = "door",
}

obstacle {
	image_filenames = "iso_gate_unlocked_opened_ns_5.png",
	flags = { BLOCKS_VISION },
	animation = "door",
}

obstacle {
	image_filenames = "iso_gate_unlocked_closed_we_1.png",
	borders = borders(-1.55, 0.55, -0.05, 0.60),
	flags = { IS_WALKABLE },
	animation = "door",
}

obstacle {
	image_filenames = "iso_gate_unlocked_opened_we_2.png",
	borders = borders(1.20, 1.20),
	flags = { IS_WALKABLE },
	animation = "door",
}

obstacle {
	image_filenames = "iso_gate_unlocked_opened_we_3.png",
	borders = borders(1.20, 1.20),
	flags = { IS_WALKABLE },
	animation = "door",
}

obstacle {
	image_filenames = "iso_gate_unlocked_opened_we_4.png",
	borders = borders(1.20, 1.20),
	flags = { IS_WALKABLE },
	animation = "door",
}

obstacle {
	image_filenames = "iso_gate_unlocked_opened_we_5.png",
	flags = { BLOCKS_VISION },
	animation = "door",
}

obstacle {
	image_filenames = "iso_gate_locked_ns.png",
	borders = borders(-0.05, 0.60, -1.55, 0.55),
}

obstacle {
	image_filenames = "iso_gate_locked_we.png",
	borders = borders(-1.55, 0.55, -0.05, 0.60),
}

obstacle {
	image_filenames = "iso_computerpillar_n.png",
	borders = borders(1.00, 1.00),
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_computerpillar_w.png",
	borders = borders(1.00, 1.00),
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_computerpillar_s.png",
	borders = borders(1.00, 1.00),
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_chairs_ball_s.png",
	borders = borders(0.80, 0.80),
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_chairs_ball_w.png",
	borders = borders(0.80, 0.80),
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_chairs_ball_n.png",
	borders = borders(0.80, 0.80),
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_chairs_ball_e.png",
	borders = borders(0.80, 0.80),
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_sofa_red_s.png",
	borders = borders(1.60, 0.80),
	flags = { GROUND_LEVEL },
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_sofa_red_w.png",
	borders = borders(0.80, 1.60),
	flags = { GROUND_LEVEL },
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_sofa_red_n.png",
	borders = borders(1.60, 0.80),
	flags = { GROUND_LEVEL },
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_sofa_red_e.png",
	borders = borders(0.80, 1.60),
	flags = { GROUND_LEVEL },
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_body_redguard_1.png",
	flags = { BLOCKS_VISION, NEEDS_PRE_PUT },
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_body_redguard_2.png",
	flags = { BLOCKS_VISION, NEEDS_PRE_PUT },
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_body_redguard_4.png",
	flags = { BLOCKS_VISION, NEEDS_PRE_PUT },
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_body_redguard_3.png",
	flags = { BLOCKS_VISION, NEEDS_PRE_PUT },
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_conference_table_nw.png",
	borders = borders(2.00, 2.00),
	flags = { GROUND_LEVEL },
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_conference_table_ne.png",
	borders = borders(2.00, 2.00),
	flags = { GROUND_LEVEL },
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_conference_table_es.png",
	borders = borders(2.00, 2.00),
	flags = { GROUND_LEVEL },
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_conference_table_ws.png",
	borders = borders(2.00, 2.00),
	flags = { GROUND_LEVEL },
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_wall_redbrownspiked_ns.png",
	borders = borders(0.80, 2.30),
}

obstacle {
	image_filenames = "iso_wall_redbrownspiked_we.png",
	borders = borders(2.30, 0.80),
}

obstacle {
	image_filenames = "iso_sleepingcapsule_n.png",
	borders = borders(1.20, 2.00),
	flags = { GROUND_LEVEL },
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_sleepingcapsule_w.png",
	borders = borders(2.00, 1.20),
	flags = { GROUND_LEVEL },
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_sleepingcapsule_s.png",
	borders = borders(1.20, 2.00),
	flags = { GROUND_LEVEL },
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_sleepingcapsule_e.png",
	borders = borders(2.00, 1.20),
	flags = { GROUND_LEVEL },
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_sleepingcapsule_double_n.png",
	borders = borders(1.20, 2.00),
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_sleepingcapsule_double_e.png",
	borders = borders(2.00, 1.20),
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_sleepingcapsule_double_s.png",
	borders = borders(1.20, 2.00),
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_sleepingcapsule_double_w.png",
	borders = borders(2.00, 1.20),
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_cinematograph_e.png",
	borders = borders(0.50, 0.50),
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_cinematograph_w.png",
	borders = borders(0.50, 0.50),
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_lamp_n.png",
	borders = borders(0.50, 0.50),
	flags = { GROUND_LEVEL },
	emitted_light_strength = 24,
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_lamp_e.png",
	borders = borders(0.50, 0.50),
	flags = { GROUND_LEVEL },
	emitted_light_strength = 24,
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_lamp_w.png",
	borders = borders(0.50, 0.50),
	flags = { GROUND_LEVEL },
	emitted_light_strength = 24,
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_shroom_blue_1.png",
	borders = borders(1.00, 1.00),
	flags = { GROUND_LEVEL },
	emitted_light_strength = 7,
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_shroom_blue_2.png",
	borders = borders(1.00, 1.00),
	flags = { GROUND_LEVEL },
	emitted_light_strength = 9,
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_shroom_blue_3.png",
	borders = borders(0.90, 0.90),
	flags = { GROUND_LEVEL },
	emitted_light_strength = 8,
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_shroom_white_2.png",
	borders = borders(0.90, 0.90),
	flags = { GROUND_LEVEL },
	emitted_light_strength = 11,
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_wall_brick_T_nwe.png",
	borders = borders(1.20, 1.20),
	flags = { BLOCKS_VISION },
}

obstacle {
	image_filenames = "iso_wall_brick_T_nes.png",
	borders = borders(1.20, 1.20),
	flags = { BLOCKS_VISION },
}

obstacle {
	image_filenames = "iso_wall_brick_T_wes.png",
	borders = borders(1.20, 1.20),
	flags = { BLOCKS_VISION },
}

obstacle {
	image_filenames = "iso_wall_brick_T_nws.png",
	borders = borders(1.20, 1.20),
	flags = { BLOCKS_VISION },
}

obstacle {
	image_filenames = "iso_wall_brick_cracked_ns.png",
	label = "",
	borders = borders(0.50, 1.20),
	flags = { IS_VERTICAL, BLOCKS_VISION, IS_SMASHABLE, IS_CLICKABLE },
	after_smashing = 237,
	action = "barrel",
}

obstacle {
	image_filenames = "iso_wall_brick_cracked_we.png",
	label = "",
	borders = borders(1.20, 0.50),
	flags = { IS_HORIZONTAL, BLOCKS_VISION, IS_SMASHABLE, IS_CLICKABLE },
	after_smashing = 238,
	action = "barrel",
}

obstacle {
	image_filenames = "iso_wall_brick_smashed_ns.png",
	flags = { IS_VERTICAL, BLOCKS_VISION },
}

obstacle {
	image_filenames = "iso_wall_brick_smashed_we.png",
	flags = { IS_HORIZONTAL, BLOCKS_VISION },
}

obstacle {
	image_filenames = "iso_projectionscreen_s.png",
	borders = borders(2.20, 1.00),
	flags = { BLOCKS_VISION },
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_projectionscreen_w.png",
	borders = borders(1.00, 2.20),
	flags = { BLOCKS_VISION },
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_projectionscreen_n.png",
	borders = borders(2.00, 1.00),
	flags = { BLOCKS_VISION },
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_projectionscreen_e.png",
	borders = borders(1.00, 2.20),
	flags = { BLOCKS_VISION },
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_cinematograph_n.png",
	borders = borders(0.50, 0.50),
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_cinematograph_s.png",
	borders = borders(0.50, 0.50),
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_sign_questionmark.png",
	label = "Sign",
	borders = borders(0.50, 0.60),
	emitted_light_strength = 5,
	flags = { GROUND_LEVEL, IS_CLICKABLE },
	transparency = NO_TRANSPARENCY,
	action = "sign",
}

obstacle {
	image_filenames = "iso_sign_exclamationmark.png",
	label = "Sign",
	borders = borders(0.60, 0.50),
	emitted_light_strength = 5,
	flags = { GROUND_LEVEL, IS_CLICKABLE },
	transparency = NO_TRANSPARENCY,
	action = "sign",
}

obstacle {
	image_filenames = "iso_sign_lessthenmark.png",
	label = "Sign",
	borders = borders(0.50, 0.60),
	emitted_light_strength = 5,
	flags = { GROUND_LEVEL, IS_CLICKABLE },
	transparency = NO_TRANSPARENCY,
	action = "sign",
}

obstacle {
	image_filenames = "iso_wall_green_wallpaper_ns.png",
	borders = borders(0.40, 1.10),
	flags = { IS_VERTICAL, BLOCKS_VISION },
}

obstacle {
	image_filenames = "iso_wall_green_wallpaper_we.png",
	borders = borders(1.10, 0.40),
	flags = { IS_HORIZONTAL, BLOCKS_VISION },
}

obstacle {
	image_filenames = "iso_wall_green_brown_manyspots_ns.png",
	borders = borders(0.40, 1.10),
	flags = { IS_VERTICAL, BLOCKS_VISION },
}

obstacle {
	image_filenames = "iso_wall_green_brown_manyspots_we.png",
	borders = borders(1.10, 0.40),
	flags = { IS_HORIZONTAL, BLOCKS_VISION },
}

obstacle {
	image_filenames = "iso_wall_green_brown_fewspots_ns.png",
	borders = borders(0.40, 1.10),
	flags = { IS_VERTICAL, BLOCKS_VISION },
}

obstacle {
	image_filenames = "iso_wall_green_brown_fewspots_ew.png",
	borders = borders(1.10, 0.40),
	flags = { IS_HORIZONTAL, BLOCKS_VISION },
}

obstacle {
	image_filenames = "iso_counter_small_w.png",
	borders = borders(0.80, 1.05),
	flags = { GROUND_LEVEL },
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_counter_small_n.png",
	borders = borders(1.05, 0.80),
	flags = { GROUND_LEVEL },
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_counter_small_e.png",
	borders = borders(0.80, 1.05),
	flags = { GROUND_LEVEL },
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_counter_small_s.png",
	borders = borders(1.05, 0.80),
	flags = { GROUND_LEVEL },
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_counter_small_curve_nw.png",
	borders = borders(1.10, 1.10),
	flags = { GROUND_LEVEL },
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_counter_small_curve_ne.png",
	borders = borders(1.10, 1.10),
	flags = { GROUND_LEVEL },
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_counter_small_curve_es.png",
	borders = borders(1.10, 1.10),
	flags = { GROUND_LEVEL },
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_counter_small_curve_ws.png",
	borders = borders(1.10, 1.10),
	flags = { GROUND_LEVEL },
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_counter_small_edge_ws.png",
	borders = borders(1.10, 1.10),
	flags = { GROUND_LEVEL },
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_counter_small_edge_nw.png",
	borders = borders(1.10, 1.10),
	flags = { GROUND_LEVEL },
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_counter_small_edge_ne.png",
	borders = borders(1.10, 1.10),
	flags = { GROUND_LEVEL },
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_counter_small_edge_es.png",
	borders = borders(1.10, 1.10),
	flags = { GROUND_LEVEL },
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_library_counter_we.png",
	borders = borders(3.50, 1.50),
	flags = { GROUND_LEVEL },
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_library_counter_ns.png",
	borders = borders(1.50, 3.50),
	flags = { GROUND_LEVEL },
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_bathtub_e.png",
	borders = borders(1.50, 1.00),
	flags = { IS_SMASHABLE, GROUND_LEVEL },
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_bathtub_s.png",
	borders = borders(1.00, 1.50),
	flags = { IS_SMASHABLE, GROUND_LEVEL },
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_table_round_yellow.png",
	borders = borders(0.80, 0.80),
	flags = { GROUND_LEVEL },
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_ladderring_n.png",
	flags = { BLOCKS_VISION },
	emitted_light_strength = 29,
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_ladderring_w.png",
	flags = { BLOCKS_VISION },
	emitted_light_strength = 29,
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_wall_yellow_curve_short_es.png",
	borders = borders(1.00, 1.00),
	flags = { BLOCKS_VISION },
}

obstacle {
	image_filenames = "iso_wall_yellow_curve_short_ne.png",
	borders = borders(1.00, 1.00),
	flags = { BLOCKS_VISION },
}

obstacle {
	image_filenames = "iso_wall_yellow_curve_short_nw.png",
	borders = borders(1.00, 1.00),
	flags = { BLOCKS_VISION },
}

obstacle {
	image_filenames = "iso_wall_yellow_curve_short_ws.png",
	borders = borders(1.00, 1.00),
	flags = { BLOCKS_VISION },
}

obstacle {
	image_filenames = "iso_table_elliptic_yellow_ns.png",
	borders = borders(0.85, 1.50),
	flags = { GROUND_LEVEL },
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_table_elliptic_yellow_ew.png",
	borders = borders(1.50, 0.85),
	flags = { GROUND_LEVEL },
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_table_glass_ns.png",
	borders = borders(1.00, 1.20),
	flags = { GROUND_LEVEL },
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_table_glass_we.png",
	borders = borders(1.20, 1.00),
	flags = { GROUND_LEVEL },
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_wall_glass_ns.png",
	label = "",
	borders = borders(0.40, 1.10),
	flags = { IS_VERTICAL, IS_SMASHABLE, IS_CLICKABLE },
	after_smashing = 348,
	transparency = NO_TRANSPARENCY,
	action = "barrel",
	smashed_sound = "Glass_Break.ogg",
}

obstacle {
	image_filenames = "iso_wall_glass_we.png",
	label = "",
	borders = borders(1.10, 0.40),
	flags = { IS_HORIZONTAL, IS_SMASHABLE, IS_CLICKABLE },
	after_smashing = 446,
	transparency = NO_TRANSPARENCY,
	action = "barrel",
	smashed_sound = "Glass_Break.ogg",
}

obstacle {
	image_filenames = "iso_wall_turquois_window_ns.png",
	borders = borders(0.40, 1.10),
	flags = { IS_VERTICAL },
}

obstacle {
	image_filenames = "iso_wall_turquois_window_we.png",
	borders = borders(1.10, 0.40),
	flags = { IS_HORIZONTAL },
}

obstacle {
	image_filenames = "iso_wall_red_window_ns.png",
	borders = borders(0.40, 1.10),
	flags = { IS_VERTICAL },
}

obstacle {
	image_filenames = "iso_wall_red_window_we.png",
	borders = borders(1.10, 0.40),
	flags = { IS_HORIZONTAL },
}

obstacle {
	image_filenames = "iso_wall_green_wallpaper_window_ns.png",
	borders = borders(0.40, 1.10),
	flags = { IS_VERTICAL },
}

obstacle {
	image_filenames = "iso_wall_green_wallpaper_window_we.png",
	borders = borders(1.10, 0.40),
	flags = { IS_HORIZONTAL },
}

obstacle {
	image_filenames = "iso_wall_green_brown_manyspots_window_ns.png",
	borders = borders(0.40, 1.10),
	flags = { IS_VERTICAL },
}

obstacle {
	image_filenames = "iso_wall_green_brown_manyspots_window_we.png",
	borders = borders(1.10, 0.40),
	flags = { IS_HORIZONTAL },
}

obstacle {
	image_filenames = "iso_barshelf_middle_we.png",
	borders = borders(0.60, 0.60),
	flags = { BLOCKS_VISION },
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_barshelf_middle_ns.png",
	borders = borders(0.60, 0.60),
	flags = { BLOCKS_VISION },
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_barshelf_left_ns.png",
	borders = borders(0.60, 0.60),
	flags = { BLOCKS_VISION },
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_barshelf_left_we.png",
	borders = borders(0.60, 0.60),
	flags = { BLOCKS_VISION },
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_barshelf_right_we.png",
	borders = borders(0.60, 0.60),
	flags = { BLOCKS_VISION },
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_barshelf_left_ew.png",
	borders = borders(0.60, 0.60),
	flags = { BLOCKS_VISION },
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_barshelf_rightouter_ew.png",
	borders = borders(0.60, 0.60),
	flags = { BLOCKS_VISION },
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_barshelf_rightouter_we.png",
	borders = borders(0.60, 0.60),
	flags = { BLOCKS_VISION },
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_barshelf_rightouter_ns.png",
	borders = borders(0.60, 0.60),
	flags = { BLOCKS_VISION },
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_barshelf_leftouter_we.png",
	borders = borders(0.60, 0.60),
	flags = { BLOCKS_VISION },
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_bench_red_w.png",
	borders = borders(0.60, 1.20),
	flags = { GROUND_LEVEL },
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_bench_red_n.png",
	borders = borders(1.20, 0.60),
	flags = { GROUND_LEVEL },
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_bench_red_e.png",
	borders = borders(0.60, 1.20),
	flags = { GROUND_LEVEL },
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_bench_red_s.png",
	borders = borders(1.20, 0.60),
	flags = { GROUND_LEVEL },
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_stool_brown_w.png",
	borders = borders(0.60, 0.60),
	flags = { GROUND_LEVEL },
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_stool_brown_n.png",
	borders = borders(0.60, 0.60),
	flags = { GROUND_LEVEL },
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_stool_brown_e.png",
	borders = borders(0.60, 0.60),
	flags = { GROUND_LEVEL },
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_stool_brown_s.png",
	borders = borders(0.60, 0.60),
	flags = { GROUND_LEVEL },
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_stool_plant_brown_w.png",
	borders = borders(0.60, 0.60),
	flags = { GROUND_LEVEL },
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_stool_plant_brown_n.png",
	borders = borders(0.60, 0.60),
	flags = { GROUND_LEVEL },
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_stool_plant_brown_e.png",
	borders = borders(0.60, 0.60),
	flags = { GROUND_LEVEL },
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_stool_plant_brown_s.png",
	borders = borders(0.60, 0.60),
	flags = { GROUND_LEVEL },
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_oil_1.png",
	flags = { IS_VOLATILE, BLOCKS_VISION, NEEDS_PRE_PUT },
	transparency = NO_TRANSPARENCY,
	groups = "oil stains"
}

obstacle {
	image_filenames = "iso_oil_5_1.png",
	flags = { IS_VOLATILE, BLOCKS_VISION, NEEDS_PRE_PUT },
	transparency = NO_TRANSPARENCY,
	groups = "oil stains"
}

obstacle {
	image_filenames = "iso_oil_4_1.png",
	flags = { IS_VOLATILE, BLOCKS_VISION, NEEDS_PRE_PUT },
	transparency = NO_TRANSPARENCY,
	groups = "oil stains"
}

obstacle {
	image_filenames = "iso_oil_4_2.png",
	flags = { IS_VOLATILE, BLOCKS_VISION, NEEDS_PRE_PUT },
	transparency = NO_TRANSPARENCY,
	groups = "oil stains"
}

obstacle {
	image_filenames = "iso_oil_10.png",
	flags = { IS_VOLATILE, BLOCKS_VISION, NEEDS_PRE_PUT },
	transparency = NO_TRANSPARENCY,
	groups = "oil stains"
}

obstacle {
	image_filenames = "iso_oil_7.png",
	flags = { IS_VOLATILE, BLOCKS_VISION, NEEDS_PRE_PUT },
	transparency = NO_TRANSPARENCY,
	groups = "oil stains"
}

obstacle {
	image_filenames = "iso_oil_5_2.png",
	flags = { IS_VOLATILE, BLOCKS_VISION, NEEDS_PRE_PUT },
	transparency = NO_TRANSPARENCY,
	groups = "oil stains"
}

obstacle {
	image_filenames = "iso_oil_11.png",
	flags = { IS_VOLATILE, BLOCKS_VISION, NEEDS_PRE_PUT },
	transparency = NO_TRANSPARENCY,
	groups = "oil stains"
}

obstacle {
	image_filenames = "iso_pathblocker_1x1.png",
	borders = borders(1.00, 1.00),
	flags = { GROUND_LEVEL },
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_wall_brick_longend_we.png",
	borders = borders(1.20, 0.80),
	flags = { IS_HORIZONTAL, BLOCKS_VISION },
	after_smashing = 235,
	groups = "brick wall 2"
}

obstacle {
	image_filenames = "iso_wall_brick_longend_ns.png",
	borders = borders(0.80, 1.20),
	flags = { IS_VERTICAL, BLOCKS_VISION },
	after_smashing = 236,
	groups = "brick wall 2"
}

obstacle {
	image_filenames = "iso_autogun_w.png",
	borders = borders(0.70, 0.70),
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_autogun_n.png",
	borders = borders(0.70, 0.70),
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_autogun_e.png",
	borders = borders(0.70, 0.70),
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_autogun_s.png",
	borders = borders(0.70, 0.70),
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_wall_brick_cable_we.png",
	borders = borders(1.20, 0.80),
	flags = { IS_HORIZONTAL, BLOCKS_VISION },
	groups = "brick wall cables"
}

obstacle {
	image_filenames = "iso_wall_brick_cable_ns.png",
	borders = borders(0.80, 1.20),
	flags = { IS_VERTICAL, BLOCKS_VISION },
	groups = "brick wall cables"
}

obstacle {
	image_filenames = "iso_wall_brick_cable_edge_ws.png",
	borders = borders(-0.60, 0.30, -0.60, 0.60),
	flags = { BLOCKS_VISION, CORNER_NE },
	groups = "brick wall cables"
}

obstacle {
	image_filenames = "iso_wall_brick_cable_edge_ne.png",
	borders = borders(-0.60, 0.65, -0.60, 0.30),
	flags = { BLOCKS_VISION, CORNER_SW },
	groups = "brick wall cables"
}

obstacle {
	image_filenames = "iso_wall_brick_cable_edge_es.png",
	borders = borders(-0.30, 0.60, -0.30, 0.60),
	flags = { BLOCKS_VISION, CORNER_NW },
	groups = "brick wall cables"
}

obstacle {
	image_filenames = "iso_wall_brick_cable_edge_nw.png",
	borders = borders(-0.60, 0.30, -0.60, 0.30),
	flags = { BLOCKS_VISION, CORNER_SE },
	groups = "brick wall cables"
}

obstacle {
	image_filenames = "iso_restaurant_counter_w.png",
	borders = borders(1.50, 5.00),
	flags = { GROUND_LEVEL },
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_restaurant_counter_n.png",
	borders = borders(5.00, 1.50),
	flags = { GROUND_LEVEL },
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_bar_counter_w.png",
	borders = borders(0.65, 5.50),
	flags = { BLOCKS_VISION },
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_bar_counter_s.png",
	borders = borders(5.50, 0.65),
	flags = { BLOCKS_VISION },
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_crystal_pillar_1.png",
	borders = borders(0.50, 0.50),
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_crystal_pillar_2.png",
	borders = borders(1.15, 1.15),
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_crystal_stump_1.png",
	borders = borders(0.95, 0.95),
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_crystal_stump_2.png",
	borders = borders(1.25, 1.05),
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_crystal_pillar_3.png",
	borders = borders(1.20, 1.05),
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_crystal_stump_3.png",
	borders = borders(1.10, 1.10),
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_wall_corners_es.png",
	borders = borders(1.10, 1.00),
	flags = { BLOCKS_VISION },
}

obstacle {
	image_filenames = "iso_wall_corners_ws.png",
	borders = borders(1.10, 1.00),
	flags = { BLOCKS_VISION },
}

obstacle {
	image_filenames = "iso_wall_corners_nw.png",
	borders = borders(1.10, 1.00),
	flags = { BLOCKS_VISION },
}

obstacle {
	image_filenames = "iso_wall_corners_ne.png",
	borders = borders(1.10, 1.00),
	flags = { BLOCKS_VISION },
}

obstacle {
	image_filenames = "iso_wall_glass_broken_ns.png",
	flags = { IS_WALKABLE },
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_gate_unlocked_opened_ns_5_blocked.png",
	flags = { BLOCKS_VISION },
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_gate_unlocked_opened_we_5_blocked.png",
	flags = { BLOCKS_VISION },
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_doubledoor_locked_we.png",
	borders = borders(-0.55, 1.55, -0.80, 0.20),
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_doubledoor_locked_ns.png",
	borders = borders(-0.80, 0.20, -0.55, 1.55),
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_doubledoor_unlocked_opened_we_1.png",
	borders = borders(-0.55, 1.55, -0.20, 0.20),
	flags = { IS_WALKABLE },
	transparency = NO_TRANSPARENCY,
	animation = "door",
}

obstacle {
	image_filenames = "iso_doubledoor_unlocked_opened_we_2.png",
	borders = borders(-0.55, 1.55, -0.20, 0.20),
	flags = { IS_WALKABLE },
	transparency = NO_TRANSPARENCY,
	animation = "door",
}

obstacle {
	image_filenames = "iso_doubledoor_unlocked_opened_we_3.png",
	borders = borders(-0.55, 1.55, -0.20, 0.20),
	flags = { IS_WALKABLE },
	transparency = NO_TRANSPARENCY,
	animation = "door",
}

obstacle {
	image_filenames = "iso_doubledoor_unlocked_opened_we_4.png",
	borders = borders(-0.55, 1.55, -0.20, 0.20),
	flags = { IS_WALKABLE },
	transparency = NO_TRANSPARENCY,
	animation = "door",
}

obstacle {
	image_filenames = "iso_doubledoor_unlocked_opened_we_5.png",
	flags = { BLOCKS_VISION },
	transparency = NO_TRANSPARENCY,
	animation = "door",
}

obstacle {
	image_filenames = "iso_doubledoor_unlocked_opened_ns_1.png",
	borders = borders(-0.20, 0.20, -0.55, 1.55),
	flags = { IS_WALKABLE },
	transparency = NO_TRANSPARENCY,
	animation = "door",
}

obstacle {
	image_filenames = "iso_doubledoor_unlocked_opened_ns_2.png",
	borders = borders(-0.20, 0.20, -0.55, 1.55),
	flags = { IS_WALKABLE },
	transparency = NO_TRANSPARENCY,
	animation = "door",
}

obstacle {
	image_filenames = "iso_doubledoor_unlocked_opened_ns_3.png",
	borders = borders(-0.20, 0.20, -0.55, 1.55),
	flags = { IS_WALKABLE },
	transparency = NO_TRANSPARENCY,
	animation = "door",
}

obstacle {
	image_filenames = "iso_doubledoor_unlocked_opened_ns_4.png",
	borders = borders(-0.20, 0.20, -0.55, 1.55),
	flags = { IS_WALKABLE },
	transparency = NO_TRANSPARENCY,
	animation = "door",
}

obstacle {
	image_filenames = "iso_doubledoor_unlocked_opened_ns_5.png",
	flags = { BLOCKS_VISION },
	transparency = NO_TRANSPARENCY,
	animation = "door",
}

obstacle {
	image_filenames = "iso_basin_n.png",
	borders = borders(1.05, 0.95),
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_basin_e.png",
	borders = borders(0.95, 1.05),
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_basin_s.png",
	borders = borders(1.05, 0.95),
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_basin_w.png",
	borders = borders(0.95, 1.05),
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_deskchair_w.png",
	borders = borders(0.90, 0.90),
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_deskchair_n.png",
	borders = borders(0.90, 0.90),
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_deskchair_e.png",
	borders = borders(0.90, 0.90),
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_ladder_w.png",
	flags = { BLOCKS_VISION },
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_ladder_n.png",
	flags = { BLOCKS_VISION },
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_chest_greyrusty_closed_w.png",
	label = "Chest",
	borders = borders(0.60, 0.80),
	flags = { GROUND_LEVEL, IS_CLICKABLE },
	transparency = NO_TRANSPARENCY,
	action = "chest",
}

obstacle {
	image_filenames = "iso_chest_greyrusty_closed_n.png",
	label = "Chest",
	borders = borders(0.80, 0.60),
	flags = { GROUND_LEVEL, IS_CLICKABLE },
	transparency = NO_TRANSPARENCY,
	action = "chest",
}

obstacle {
	image_filenames = "iso_chest_greyrusty_opened_w.png",
	borders = borders(0.60, 0.80),
	flags = { GROUND_LEVEL },
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_chest_greyrusty_opened_n.png",
	borders = borders(0.80, 0.60),
	flags = { GROUND_LEVEL },
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_chest_greyrusty_closed_s.png",
	label = "Chest",
	borders = borders(0.80, 0.60),
	flags = { GROUND_LEVEL, IS_CLICKABLE },
	transparency = NO_TRANSPARENCY,
	action = "chest",
}

obstacle {
	image_filenames = "iso_chest_greyrusty_closed_e.png",
	label = "Chest",
	borders = borders(0.60, 0.80),
	flags = { GROUND_LEVEL, IS_CLICKABLE },
	transparency = NO_TRANSPARENCY,
	action = "chest",
}

obstacle {
	image_filenames = "iso_chest_greyrusty_opened_s.png",
	borders = borders(0.80, 0.60),
	flags = { GROUND_LEVEL },
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_chest_greyrusty_opened_e.png",
	borders = borders(0.60, 0.80),
	flags = { GROUND_LEVEL },
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_security_gate_opened_w.png",
	borders = borders(0.95, 1.05),
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_security_gate_opened_n.png",
	borders = borders(0.95, 1.05),
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_security_gate_closed_w.png",
	borders = borders(0.95, 1.05),
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_security_gate_closed_n.png",
	borders = borders(0.95, 1.05),
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_security_gate_opened_e.png",
	borders = borders(0.95, 1.05),
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_security_gate_opened_s.png",
	borders = borders(0.95, 1.05),
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_security_gate_closed_e.png",
	borders = borders(0.95, 1.05),
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_security_gate_closed_s.png",
	borders = borders(0.95, 1.05),
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_solarpanel.png",
	label = "Solar Panel",
	borders = borders(0.95, 1.05),
	flags = { IS_SMASHABLE, IS_CLICKABLE },
	after_smashing = 407,
	transparency = NO_TRANSPARENCY,
	action = "barrel",
}

obstacle {
	image_filenames = "iso_conveyor_ns.png",
	borders = borders(3.00, 2.00),
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_conveyor_we.png",
	borders = borders(2.00, 3.00),
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_ramp_w.png",
	borders = borders(2.46, 1.94),
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_ramp_s.png",
	borders = borders(1.94, 2.46),
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_ramp_e.png",
	borders = borders(2.46, 1.94),
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_ramp_n.png",
	borders = borders(1.94, 2.46),
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_tesla_n.png",
	borders = borders(1.00, 1.00),
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_tesla_w.png",
	borders = borders(1.00, 1.00),
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_tesla_s.png",
	borders = borders(1.00, 1.00),
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_tesla_e.png",
	borders = borders(1.00, 1.00),
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_roboarm_1_n.png",
	borders = borders(1.00, 1.00),
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_roboarm_1_w.png",
	borders = borders(1.00, 1.00),
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_freighter_railway_ns.png",
	borders = borders(3.00, 3.00),
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_freighter_railway_we.png",
	borders = borders(3.00, 3.00),
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_freighter_railway_end_s.png",
	borders = borders(3.00, 3.00),
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_freighter_railway_end_e.png",
	borders = borders(3.00, 3.00),
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_freighter_railway_end_n.png",
	borders = borders(3.00, 3.00),
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_freighter_railway_end_w.png",
	borders = borders(3.00, 3.00),
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_solarpanel_pillar.png",
	borders = borders(0.95, 1.05),
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_crate_ns_megasys.png",
	label = "Crate",
	borders = borders(0.80, 0.95),
	flags = { BLOCKS_VISION, IS_SMASHABLE, DROPS_RANDOM_TREASURE, IS_CLICKABLE },
	transparency = NO_TRANSPARENCY,
	action = "barrel",
}

obstacle {
	image_filenames = "iso_reactor_w.png",
	borders = borders(4.50, 4.00),
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_reactor_s.png",
	borders = borders(4.00, 4.50),
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_reactor_e.png",
	borders = borders(4.50, 4.00),
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_reactor_n.png",
	borders = borders(4.00, 4.50),
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_wallterminal_n.png",
	label = "Terminal",
	borders = borders(0.60, 0.40),
	flags = { IS_CLICKABLE },
	transparency = NO_TRANSPARENCY,
	action = "terminal",
}

obstacle {
	image_filenames = "iso_wallterminal_w.png",
	label = "Terminal",
	borders = borders(0.40, 0.60),
	flags = { IS_CLICKABLE },
	transparency = NO_TRANSPARENCY,
	action = "terminal",
}

obstacle {
	image_filenames = "iso_wallterminal_s.png",
	label = "Terminal",
	borders = borders(0.60, 0.40),
	flags = { IS_CLICKABLE },
	transparency = NO_TRANSPARENCY,
	action = "terminal",
}

obstacle {
	image_filenames = "iso_wallterminal_e.png",
	label = "Terminal",
	borders = borders(0.40, 0.60),
	flags = { IS_CLICKABLE },
	transparency = NO_TRANSPARENCY,
	action = "terminal",
}

obstacle {
	image_filenames = "iso_turbines_n.png",
	borders = borders(1.10, 1.80),
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_turbines_w.png",
	borders = borders(1.80, 1.05),
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_turbines_s.png",
	borders = borders(1.10, 1.80),
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_turbines_e.png",
	borders = borders(1.80, 1.05),
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_weapon_crate.png",
	label = "Weapon Crate",
	borders = borders(1.30, 1.30),
	flags = { BLOCKS_VISION, IS_SMASHABLE, DROPS_RANDOM_TREASURE, IS_CLICKABLE },
	transparency = NO_TRANSPARENCY,
	action = "barrel",
}

obstacle {
	image_filenames = "iso_electronicscrap_1.png",
	flags = { BLOCKS_VISION },
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_electronicscrap_2.png",
	flags = { BLOCKS_VISION },
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_electronicscrap_3.png",
	flags = { BLOCKS_VISION },
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_electronicscrap_4.png",
	flags = { BLOCKS_VISION },
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_electronicscrap_5.png",
	flags = { BLOCKS_VISION },
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_electronicscrap_6.png",
	flags = { BLOCKS_VISION },
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_electronicscrap_7.png",
	flags = { BLOCKS_VISION },
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_electronicscrap_8.png",
	flags = { BLOCKS_VISION },
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_body_human.png",
	flags = { BLOCKS_VISION },
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_ladder_short_n.png",
	flags = { BLOCKS_VISION },
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_ladder_short_w.png",
	flags = { BLOCKS_VISION },
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_wrecked_car_w.png",
	borders = borders(1.40, 2.80),
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_wrecked_car_s.png",
	borders = borders(2.80, 1.40),
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_wrecked_car_e.png",
	borders = borders(1.40, 2.80),
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_wrecked_car_n.png",
	borders = borders(2.80, 1.40),
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_toilet_white_n.png",
	borders = borders(0.40, 0.40),
	flags = { IS_SMASHABLE, GROUND_LEVEL },
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_toilet_white_w.png",
	borders = borders(0.40, 0.40),
	flags = { IS_SMASHABLE, GROUND_LEVEL },
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_roboarm_1_s.png",
	borders = borders(1.00, 1.00),
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_roboarm_1_e.png",
	borders = borders(1.00, 1.00),
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_roboarm_2_n.png",
	borders = borders(1.00, 1.00),
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_roboarm_2_w.png",
	borders = borders(1.00, 1.00),
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_roboarm_2_s.png",
	borders = borders(1.00, 1.00),
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_roboarm_2_e.png",
	borders = borders(1.00, 1.00),
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_crushed_476.png",
	borders = borders(2.10, 2.60),
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_wall_glass_broken_we.png",
	flags = { IS_WALKABLE },
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = { "iso_sign_questionmark_anim_dark.png", "iso_sign_questionmark_anim_bright.png" },
	label = "Sign",
	borders = borders(0.50, 0.60),
	emitted_light_strength = { 0, 5 },
	flags = { GROUND_LEVEL, IS_CLICKABLE },
	transparency = NO_TRANSPARENCY,
	action = "sign",
	animation_fps = 0.44
}

obstacle {
	image_filenames = { "iso_sign_exclamationmark_anim_dark.png", "iso_sign_exclamationmark_anim_bright.png" },
	label = "Sign",
	borders = borders(0.60, 0.50),
	emitted_light_strength = { 0, 5 },
	flags = { GROUND_LEVEL, IS_CLICKABLE },
	transparency = NO_TRANSPARENCY,
	action = "sign",
	animation_fps = 0.4
}
obstacle {
	image_filenames = { "iso_sign_lessthenmark_anim_dark.png", "iso_sign_lessthenmark_anim_bright.png" },
	label = "Sign",
	borders = borders(0.50, 0.60),
	emitted_light_strength = { 0, 5 },
	flags = { GROUND_LEVEL, IS_CLICKABLE },
	transparency = NO_TRANSPARENCY,
	action = "sign",
	animation_fps = 0.6
}
	
obstacle {
	image_filenames = { "iso_barrel_radioactive.png" },
	borders = borders(0.70, 0.70),
	emitted_light_strength = { 1, 2, 3, 2, 1 },
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_vendingmachine_blue_w.png",
	label = "Vending Machine",
	borders = borders(1.10, 1.55),
	emitted_light_strength = { 3, 5, 7, 6, 4, 3, 3, 0, 0, 0, 3, 0 },
	flags = { BLOCKS_VISION, IS_CLICKABLE },
	transparency = NO_TRANSPARENCY,
	action = "terminal",
}

obstacle {
	image_filenames = "iso_vendingmachine_blue_s.png",
	label = "Vending Machine",
	borders = borders(1.55, 1.10),
	emitted_light_strength = { 3, 5, 7, 6, 4, 3, 3, 0, 0, 0, 3, 0  },
	flags = { BLOCKS_VISION, IS_CLICKABLE },
	transparency = NO_TRANSPARENCY,
	action = "terminal",
}

obstacle {
	image_filenames = "iso_vendingmachine_blue_e.png",
	label = "Vending Machine",
	borders = borders(1.10, 1.55),
	emitted_light_strength = { 3, 5, 7, 6, 4, 3, 3, 0, 0, 0, 3, 0 },
	flags = { BLOCKS_VISION, IS_CLICKABLE },
	transparency = NO_TRANSPARENCY,
	action = "terminal",
}

obstacle {
	image_filenames = "iso_vendingmachine_blue_n.png",
	label = "Vending Machine",
	borders = borders(1.55, 1.10),
	emitted_light_strength = { 3, 5, 7, 6, 4, 3, 3, 0, 0, 0, 3, 0 },
	flags = { BLOCKS_VISION, IS_CLICKABLE },
	transparency = NO_TRANSPARENCY,
	action = "terminal",
}

obstacle {
	image_filenames = "iso_vendingmachine_white_w.png",
	label = "Vending Machine",
	borders = borders(1.00, 1.55),
	emitted_light_strength = { 3, 5, 7, 6, 4, 3, 3, 0, 0, 0, 3, 0 },
	flags = { BLOCKS_VISION, IS_CLICKABLE },
	transparency = NO_TRANSPARENCY,
	action = "terminal",
}

obstacle {
	image_filenames = "iso_vendingmachine_white_s.png",
	label = "Vending Machine",
	borders = borders(1.55, 1.00),
	emitted_light_strength = { 3, 5, 7, 6, 4, 3, 3, 0, 0, 0, 3, 0 },
	flags = { BLOCKS_VISION, IS_CLICKABLE },
	transparency = NO_TRANSPARENCY,
	action = "terminal",
}

obstacle {
	image_filenames = "iso_vendingmachine_white_e.png",
	label = "Vending Machine",
	borders = borders(1.00, 1.55),
	emitted_light_strength = { 3, 5, 7, 6, 4, 3, 3, 0, 0, 0, 3, 0 },
	flags = { BLOCKS_VISION, IS_CLICKABLE },
	transparency = NO_TRANSPARENCY,
	action = "terminal",
}

obstacle {
	image_filenames = "iso_vendingmachine_white_n.png",
	label = "Vending Machine",
	borders = borders(1.55, 1.00),
	emitted_light_strength = { 3, 5, 7, 6, 4, 3, 3, 0, 0, 0, 3, 0 },
	flags = { BLOCKS_VISION, IS_CLICKABLE },
	transparency = NO_TRANSPARENCY,
	action = "terminal",
}

obstacle {
	image_filenames = "iso_vendingmachine_red_w.png",
	label = "Vending Machine",
	borders = borders(1.10, 1.55),
	emitted_light_strength = { 3, 5, 7, 6, 4, 3, 3, 0, 0, 0, 3, 0 },
	flags = { BLOCKS_VISION, IS_CLICKABLE },
	transparency = NO_TRANSPARENCY,
	action = "terminal",
}

obstacle {
	image_filenames = "iso_vendingmachine_red_s.png",
	label = "Vending Machine",
	borders = borders(1.55, 1.10),
	emitted_light_strength = { 3, 5, 7, 6, 4, 3, 3, 0, 0, 0, 3, 0 },
	flags = { BLOCKS_VISION, IS_CLICKABLE },
	transparency = NO_TRANSPARENCY,
	action = "terminal",
}

obstacle {
	image_filenames = "iso_vendingmachine_red_e.png",
	label = "Vending Machine",
	borders = borders(1.10, 1.55),
	emitted_light_strength = { 3, 5, 7, 6, 4, 3, 3, 0, 0, 0, 3, 0 },
	flags = { BLOCKS_VISION, IS_CLICKABLE },
	transparency = NO_TRANSPARENCY,
	action = "terminal",
}

obstacle {
	image_filenames = "iso_vendingmachine_red_n.png",
	label = "Vending Machine",
	borders = borders(1.55, 1.10),
	emitted_light_strength = { 3, 5, 7, 6, 4, 3, 3, 0, 0, 0, 3, 0 },
	flags = { BLOCKS_VISION, IS_CLICKABLE },
	transparency = NO_TRANSPARENCY,
	action = "terminal",
}

obstacle {
	image_filenames = "iso_transformer.png",
	borders = borders(0.95, 0.95),
	flags = { BLOCKS_VISION },
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_transformer_rusty.png",
	borders = borders(0.95, 0.95),
	flags = { BLOCKS_VISION },
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_transformer_sparkles.png",
	borders = borders(0.95, 0.95),
	flags = { BLOCKS_VISION },
	transparency = NO_TRANSPARENCY,
}
