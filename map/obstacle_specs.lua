function borders(left, right, upper, lower)
	if upper == nil then
		return { -left / 2, left / 2, -right / 2, right / 2 }
	end
	return { left, right, upper, lower }
end

obstacle {
	image_filenames = "iso_tree_0000.png",
	borders = borders(0.60, 0.60),
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_walls_0001.png",
	borders = borders(0.40, 1.10),
	flags = { IS_VERTICAL, BLOCKS_VISION },
}

obstacle {
	image_filenames = "iso_walls_0002.png",
	borders = borders(1.10, 0.40),
	flags = { IS_HORIZONTAL, BLOCKS_VISION },
}

obstacle {
	image_filenames = "iso_walls_0003.png",
	borders = borders(0.40, 1.10),
	flags = { IS_VERTICAL, BLOCKS_VISION },
}

obstacle {
	image_filenames = "iso_walls_0004.png",
	borders = borders(1.10, 0.40),
	flags = { IS_HORIZONTAL, BLOCKS_VISION },
}

obstacle {
	image_filenames = "iso_tree_0001.png",
	borders = borders(1.30, 1.30),
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_doors_0001.png",
	borders = borders(1.00, 0.40),
	flags = { IS_HORIZONTAL, IS_WALKABLE },
	transparency = NO_TRANSPARENCY,
	animation = "door",
	groups = "blue doors"
}

obstacle {
	image_filenames = "iso_doors_0002.png",
	borders = borders(1.00, 0.40),
	flags = { IS_WALKABLE },
	transparency = NO_TRANSPARENCY,
	animation = "door",
}

obstacle {
	image_filenames = "iso_doors_0003.png",
	borders = borders(1.00, 0.40),
	flags = { IS_WALKABLE },
	transparency = NO_TRANSPARENCY,
	animation = "door",
}

obstacle {
	image_filenames = "iso_doors_0004.png",
	borders = borders(1.00, 0.40),
	flags = { IS_WALKABLE },
	transparency = NO_TRANSPARENCY,
	animation = "door",
}

obstacle {
	image_filenames = "iso_doors_0005.png",
	flags = { BLOCKS_VISION },
	transparency = NO_TRANSPARENCY,
	animation = "door",
}

obstacle {
	image_filenames = "iso_doors_0006.png",
	borders = borders(0.40, 1.00),
	flags = { IS_VERTICAL, IS_WALKABLE },
	transparency = NO_TRANSPARENCY,
	animation = "door",
	groups = "blue doors"
}

obstacle {
	image_filenames = "iso_doors_0007.png",
	borders = borders(0.40, 1.00),
	flags = { IS_WALKABLE },
	transparency = NO_TRANSPARENCY,
	animation = "door",
}

obstacle {
	image_filenames = "iso_doors_0008.png",
	borders = borders(0.40, 1.00),
	flags = { IS_WALKABLE },
	transparency = NO_TRANSPARENCY,
	animation = "door",
}

obstacle {
	image_filenames = "iso_doors_0009.png",
	borders = borders(0.40, 1.00),
	flags = { IS_WALKABLE },
	transparency = NO_TRANSPARENCY,
	animation = "door",
}

obstacle {
	image_filenames = "iso_doors_0010.png",
	flags = { BLOCKS_VISION },
	transparency = NO_TRANSPARENCY,
	animation = "door",
}

obstacle {
	image_filenames = { "iso_obstacle_0018.png", "iso_obstacle_0019.png", "iso_obstacle_0020.png", "iso_obstacle_0016.png", "iso_obstacle_0017.png" },
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
	image_filenames = "DUMMY OBSTACLE"
}

obstacle {
	image_filenames = { "iso_machinery_0009.png", "iso_machinery_0010.png", "iso_machinery_0011.png", "iso_machinery_0012.png", "iso_machinery_0013.png" },
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
	image_filenames = "iso_doors_0011.png",
	borders = borders(1.10, 0.40),
	flags = { IS_HORIZONTAL },
	transparency = NO_TRANSPARENCY,
	groups = "red door"
}

obstacle {
	image_filenames = "iso_doors_0012.png",
	borders = borders(0.40, 1.10),
	flags = { IS_VERTICAL },
	transparency = NO_TRANSPARENCY,
	groups = "red door"
}

obstacle {
	image_filenames = "iso_container_0001.png",
	label = "Chest",
	borders = borders(0.80, 0.60),
	flags = { GROUND_LEVEL, IS_CLICKABLE },
	transparency = NO_TRANSPARENCY,
	action = "chest",
}

obstacle {
	image_filenames = "iso_container_0002.png",
	label = "Chest",
	borders = borders(0.60, 0.80),
	flags = { GROUND_LEVEL, IS_CLICKABLE },
	transparency = NO_TRANSPARENCY,
	action = "chest",
}

obstacle {
	image_filenames = "iso_container_0003.png",
	borders = borders(0.80, 0.60),
	flags = { GROUND_LEVEL },
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_container_0004.png",
	borders = borders(0.60, 0.80),
	flags = { GROUND_LEVEL },
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_autogun_act_0001.png",
	borders = borders(0.70, 0.70),
	transparency = NO_TRANSPARENCY,
	animation = "autogun",
}

obstacle {
	image_filenames = "iso_autogun_act_0002.png",
	borders = borders(0.70, 0.70),
	transparency = NO_TRANSPARENCY,
	animation = "autogun",
}

obstacle {
	image_filenames = "iso_autogun_act_0003.png",
	borders = borders(0.70, 0.70),
	transparency = NO_TRANSPARENCY,
	animation = "autogun",
}

obstacle {
	image_filenames = "iso_autogun_act_0004.png",
	borders = borders(0.70, 0.70),
	transparency = NO_TRANSPARENCY,
	animation = "autogun",
}

obstacle {
	image_filenames = "iso_cave_wall_0001.png",
	borders = borders(1.50, 1.00),
	flags = { IS_HORIZONTAL, BLOCKS_VISION },
	transparency = NO_TRANSPARENCY,
	groups = "cave wall"
}

obstacle {
	image_filenames = "iso_cave_wall_0002.png",
	borders = borders(1.00, 1.50),
	flags = { IS_VERTICAL, BLOCKS_VISION },
	transparency = NO_TRANSPARENCY,
	groups = "cave wall"
}

obstacle {
	image_filenames = "iso_cave_wall_0003.png",
	borders = borders(1.00, 1.00),
	flags = { BLOCKS_VISION, CORNER_NE },
	transparency = NO_TRANSPARENCY,
	groups = "cave wall"
}

obstacle {
	image_filenames = "iso_cave_wall_0004.png",
	borders = borders(1.00, 1.00),
	flags = { BLOCKS_VISION, CORNER_SE },
	transparency = NO_TRANSPARENCY,
	groups = "cave wall"
}

obstacle {
	image_filenames = "iso_cave_wall_0005.png",
	borders = borders(1.00, 1.00),
	flags = { BLOCKS_VISION, CORNER_NW },
	transparency = NO_TRANSPARENCY,
	groups = "cave wall"
}

obstacle {
	image_filenames = "iso_cave_wall_0006.png",
	borders = borders(1.00, 1.00),
	flags = { BLOCKS_VISION, CORNER_SW },
	transparency = NO_TRANSPARENCY,
	groups = "cave wall"
}

obstacle {
	image_filenames = "iso_obstacle_0042.png",
	borders = borders(0.50, 0.50),
	flags = { IS_SMASHABLE, GROUND_LEVEL },
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_obstacle_0043.png",
	label = "Terminal",
	borders = borders(0.80, 0.80),
	flags = { IS_CLICKABLE },
	transparency = NO_TRANSPARENCY,
	action = "terminal",
}

obstacle {
	image_filenames = "iso_obstacle_0044.png",
	label = "Terminal",
	borders = borders(0.80, 0.80),
	flags = { IS_CLICKABLE },
	transparency = NO_TRANSPARENCY,
	action = "terminal",
}

obstacle {
	image_filenames = "iso_obstacle_0045.png",
	label = "Terminal",
	borders = borders(0.80, 0.80),
	flags = { IS_CLICKABLE },
	transparency = NO_TRANSPARENCY,
	action = "terminal",
}

obstacle {
	image_filenames = "iso_obstacle_0046.png",
	label = "Terminal",
	borders = borders(0.80, 0.80),
	flags = { IS_CLICKABLE },
	transparency = NO_TRANSPARENCY,
	action = "terminal",
}

obstacle {
	image_filenames = "iso_obstacle_0047.png",
	borders = borders(-0.50, 0.25, -0.50, 0.25),
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_obstacle_0048.png",
	borders = borders(-0.50, 0.25, -0.50, 0.25),
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_machinery_0001.png",
	borders = borders(1.00, 1.00),
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_barrel_1.png",
	label = "Barrel",
	borders = borders(0.70, 0.70),
	flags = { BLOCKS_VISION, IS_SMASHABLE, DROPS_RANDOM_TREASURE, IS_CLICKABLE },
	transparency = NO_TRANSPARENCY,
	action = "barrel",
}

obstacle {
	image_filenames = "iso_barrel_2.png",
	label = "Barrel",
	borders = borders(0.70, 0.70),
	flags = { BLOCKS_VISION, IS_SMASHABLE, DROPS_RANDOM_TREASURE, IS_CLICKABLE },
	transparency = NO_TRANSPARENCY,
	action = "barrel",
}

obstacle {
	image_filenames = "iso_barrel_3.png",
	label = "Crate",
	borders = borders(0.80, 0.95),
	flags = { BLOCKS_VISION, IS_SMASHABLE, DROPS_RANDOM_TREASURE, IS_CLICKABLE },
	transparency = NO_TRANSPARENCY,
	action = "barrel",
}

obstacle {
	image_filenames = "iso_barrel_4.png",
	label = "Crate",
	borders = borders(0.80, 0.75),
	flags = { BLOCKS_VISION, IS_SMASHABLE, DROPS_RANDOM_TREASURE, IS_CLICKABLE },
	transparency = NO_TRANSPARENCY,
	action = "barrel",
}

obstacle {
	image_filenames = "iso_obstacle_0054.png",
	borders = borders(0.50, 0.50),
	flags = { GROUND_LEVEL },
	emitted_light_strength = 24,
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_machinery_0005.png",
	borders = borders(-0.60, 0.55, -0.60, 0.50),
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_machinery_0006.png",
	borders = borders(-0.60, 0.55, -0.60, 0.55),
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_machinery_0007.png",
	borders = borders(-0.60, 0.50, -0.60, 0.55),
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_machinery_0008.png",
	borders = borders(-0.60, 0.50, -0.60, 0.50),
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_obstacle_0059.png",
	borders = borders(1.10, 2.20),
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_obstacle_0060.png",
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
	image_filenames = "iso_obstacle_0063.png",
	borders = borders(0.80, 2.20),
	flags = { GROUND_LEVEL },
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_obstacle_0064.png",
	borders = borders(2.20, 0.80),
	flags = { GROUND_LEVEL },
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_obstacle_0065.png",
	borders = borders(0.80, 2.20),
	flags = { GROUND_LEVEL },
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_obstacle_0066.png",
	borders = borders(2.20, 0.80),
	flags = { GROUND_LEVEL },
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_bathroom_furniture_0008.png",
	borders = borders(0.40, 0.40),
	flags = { IS_SMASHABLE, GROUND_LEVEL },
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_bathroom_furniture_0009.png",
	borders = borders(0.40, 0.40),
	flags = { IS_SMASHABLE, GROUND_LEVEL },
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_obstacle_0069.png",
	borders = borders(0.40, 0.40),
	flags = { IS_SMASHABLE, GROUND_LEVEL },
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_obstacle_0070.png",
	borders = borders(0.40, 0.40),
	flags = { IS_SMASHABLE, GROUND_LEVEL },
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_bathroom_furniture_0004.png",
	borders = borders(0.68, 0.50),
	flags = { IS_SMASHABLE, GROUND_LEVEL },
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_bathroom_furniture_0005.png",
	borders = borders(0.50, 0.68),
	flags = { IS_SMASHABLE, GROUND_LEVEL },
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_bathroom_furniture_0006.png",
	borders = borders(0.68, 0.50),
	flags = { IS_SMASHABLE, GROUND_LEVEL },
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_bathroom_furniture_0007.png",
	borders = borders(0.50, 0.68),
	flags = { IS_SMASHABLE, GROUND_LEVEL },
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_chairs_0009.png",
	borders = borders(0.40, 0.40),
	flags = { IS_SMASHABLE },
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_chairs_0010.png",
	borders = borders(0.40, 0.40),
	flags = { IS_SMASHABLE },
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_chairs_0011.png",
	borders = borders(0.40, 0.40),
	flags = { IS_SMASHABLE },
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_chairs_0012.png",
	borders = borders(0.40, 0.40),
	flags = { IS_SMASHABLE },
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_tables_0001.png",
	borders = borders(0.40, 1.00),
	flags = { IS_SMASHABLE, GROUND_LEVEL },
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_tables_0002.png",
	borders = borders(1.00, 0.40),
	flags = { IS_SMASHABLE, GROUND_LEVEL },
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_tables_0003.png",
	borders = borders(0.40, 1.00),
	flags = { IS_SMASHABLE, GROUND_LEVEL },
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_tables_0004.png",
	borders = borders(1.00, 0.40),
	flags = { IS_SMASHABLE, GROUND_LEVEL },
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_obstacle_0083.png",
	borders = borders(0.40, 0.40),
	flags = { IS_SMASHABLE, GROUND_LEVEL },
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_obstacle_0084.png",
	borders = borders(0.40, 0.40),
	flags = { IS_SMASHABLE, GROUND_LEVEL },
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_obstacle_0085.png",
	borders = borders(0.40, 0.40),
	flags = { IS_SMASHABLE, GROUND_LEVEL },
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_obstacle_0086.png",
	borders = borders(0.40, 0.40),
	flags = { IS_SMASHABLE, GROUND_LEVEL },
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_obstacle_0087.png",
	borders = borders(1.10, 0.70),
	flags = { IS_SMASHABLE, GROUND_LEVEL },
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_obstacle_0088.png",
	borders = borders(0.70, 1.10),
	flags = { IS_SMASHABLE, GROUND_LEVEL },
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_obstacle_0089.png",
	borders = borders(1.10, 0.70),
	flags = { IS_SMASHABLE, GROUND_LEVEL },
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_obstacle_0090.png",
	borders = borders(0.70, 1.10),
	flags = { IS_SMASHABLE, GROUND_LEVEL },
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_obstacle_0091.png",
	borders = borders(0.60, 2.20),
	flags = { BLOCKS_VISION },
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_obstacle_0092.png",
	borders = borders(2.20, 0.60),
	flags = { BLOCKS_VISION },
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_obstacle_0093.png",
	borders = borders(0.60, 2.20),
	flags = { BLOCKS_VISION },
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_obstacle_0094.png",
	borders = borders(2.20, 0.60),
	flags = { BLOCKS_VISION },
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_obstacle_0095.png",
	borders = borders(1.10, 0.60),
	flags = { BLOCKS_VISION },
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_obstacle_0096.png",
	borders = borders(0.60, 1.10),
	flags = { BLOCKS_VISION },
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_obstacle_0097.png",
	borders = borders(0.60, 1.10),
	flags = { BLOCKS_VISION },
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_obstacle_0098.png",
	borders = borders(1.10, 0.60),
	flags = { BLOCKS_VISION },
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_obstacle_0099.png",
	borders = borders(0.70, 1.30),
	flags = { IS_SMASHABLE, GROUND_LEVEL },
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_obstacle_0100.png",
	borders = borders(1.30, 0.70),
	flags = { IS_SMASHABLE, GROUND_LEVEL },
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_obstacle_0101.png",
	borders = borders(1.30, 0.70),
	flags = { IS_SMASHABLE, GROUND_LEVEL },
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_obstacle_0102.png",
	borders = borders(0.70, 1.30),
	flags = { IS_SMASHABLE, GROUND_LEVEL },
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_bathroom_furniture_0000.png",
	borders = borders(1.50, 1.00),
	flags = { IS_SMASHABLE, GROUND_LEVEL },
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_bathroom_furniture_0001.png",
	borders = borders(1.00, 1.50),
	flags = { IS_SMASHABLE, GROUND_LEVEL },
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_obstacle_0105.png",
	borders = borders(0.40, 0.50),
	flags = { IS_SMASHABLE, GROUND_LEVEL },
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_obstacle_0106.png",
	borders = borders(0.50, 0.40),
	flags = { IS_SMASHABLE, GROUND_LEVEL },
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_obstacle_0107.png",
	flags = { IS_VERTICAL, BLOCKS_VISION },
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_obstacle_0108.png",
	flags = { IS_HORIZONTAL, BLOCKS_VISION },
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_obstacle_0111.png",
	borders = borders(0.50, 1.00),
	flags = { GROUND_LEVEL },
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_obstacle_0112.png",
	borders = borders(1.00, 0.50),
	flags = { GROUND_LEVEL },
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_obstacle_0109.png",
	borders = borders(0.50, 1.00),
	flags = { GROUND_LEVEL },
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_obstacle_0110.png",
	borders = borders(1.00, 0.50),
	flags = { GROUND_LEVEL },
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_obstacle_0113.png",
	borders = borders(0.60, 0.60),
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_obstacle_0114.png",
	borders = borders(0.60, 0.60),
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_obstacle_0115.png",
	borders = borders(0.60, 0.80),
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_thick_wall_0001.png",
	borders = borders(1.10, 0.40),
	flags = { IS_HORIZONTAL, BLOCKS_VISION },
	groups = "thick wall"
}

obstacle {
	image_filenames = "iso_thick_wall_0002.png",
	borders = borders(0.40, 1.10),
	flags = { IS_VERTICAL, BLOCKS_VISION },
	groups = "thick wall"
}

obstacle {
	image_filenames = "iso_thick_wall_0003.png",
	borders = borders(-0.55, 0.20, -0.20, 0.55),
	flags = { BLOCKS_VISION, CORNER_NE },
	groups = "thick wall"
}

obstacle {
	image_filenames = "iso_thick_wall_0004.png",
	borders = borders(-0.55, 0.20, -0.55, 0.20),
	flags = { BLOCKS_VISION, CORNER_SE },
	groups = "thick wall"
}

obstacle {
	image_filenames = "iso_thick_wall_0005.png",
	borders = borders(-0.20, 0.55, -0.20, 0.55),
	flags = { BLOCKS_VISION, CORNER_NW },
	groups = "thick wall"
}

obstacle {
	image_filenames = "iso_thick_wall_0006.png",
	borders = borders(-0.20, 0.55, -0.55, 0.20),
	flags = { BLOCKS_VISION, CORNER_SW },
	groups = "thick wall"
}

obstacle {
	image_filenames = "iso_thick_wall_0007.png",
	borders = borders(-0.55, 0.55, -0.55, 0.20),
	flags = { BLOCKS_VISION },
}

obstacle {
	image_filenames = "iso_thick_wall_0008.png",
	borders = borders(-0.20, 0.55, -0.55, 0.55),
	flags = { BLOCKS_VISION },
}

obstacle {
	image_filenames = "iso_thick_wall_0009.png",
	borders = borders(-0.55, 0.55, -0.20, 0.55),
	flags = { BLOCKS_VISION },
}

obstacle {
	image_filenames = "iso_thick_wall_0010.png",
	borders = borders(-0.55, 0.20, -0.55, 0.55),
	flags = { BLOCKS_VISION },
}

obstacle {
	image_filenames = "iso_cave_wall_0007.png",
	borders = borders(1.00, 1.00),
	flags = { BLOCKS_VISION },
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_cave_wall_0008.png",
	borders = borders(1.00, 1.00),
	flags = { BLOCKS_VISION },
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_cave_wall_0009.png",
	borders = borders(1.00, 1.00),
	flags = { BLOCKS_VISION },
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_cave_wall_0010.png",
	borders = borders(1.00, 1.00),
	flags = { BLOCKS_VISION },
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_walls_0005.png",
	borders = borders(0.40, 1.10),
	flags = { IS_VERTICAL },
}

obstacle {
	image_filenames = "iso_walls_0006.png",
	borders = borders(1.10, 0.40),
	flags = { IS_HORIZONTAL },
}

obstacle {
	image_filenames = "iso_walls_0007.png",
	borders = borders(0.40, 1.10),
	flags = { IS_VERTICAL, BLOCKS_VISION },
}

obstacle {
	image_filenames = "iso_walls_0008.png",
	borders = borders(1.10, 0.40),
	flags = { IS_HORIZONTAL, BLOCKS_VISION },
}

obstacle {
	image_filenames = "iso_brick_wall_0002.png",
	borders = borders(1.20, 0.80),
	flags = { IS_HORIZONTAL, BLOCKS_VISION },
	after_smashing = 235,
	groups = "brick wall"
}

obstacle {
	image_filenames = "iso_brick_wall_0001.png",
	borders = borders(0.80, 1.20),
	flags = { IS_VERTICAL, BLOCKS_VISION },
	after_smashing = 236,
	groups = "brick wall"
}

obstacle {
	image_filenames = "iso_brick_wall_0003.png",
	borders = borders(1.00, 1.00),
	flags = { BLOCKS_VISION },
}

obstacle {
	image_filenames = "iso_brick_wall_0004.png",
	borders = borders(-0.60, 0.30, -0.60, 0.60),
	flags = { BLOCKS_VISION, CORNER_NE },
	groups = { "brick wall", "brick wall 2" }
}

obstacle {
	image_filenames = "iso_brick_wall_0005.png",
	borders = borders(-0.60, 0.65, -0.60, 0.30),
	flags = { BLOCKS_VISION, CORNER_SW },
	groups = { "brick wall", "brick wall 2" }
}

obstacle {
	image_filenames = "iso_brick_wall_0006.png",
	borders = borders(-0.30, 0.60, -0.30, 0.60),
	flags = { BLOCKS_VISION, CORNER_NW },
	groups = { "brick wall", "brick wall 2" }
}

obstacle {
	image_filenames = "iso_brick_wall_0007.png",
	borders = borders(-0.60, 0.30, -0.60, 0.30),
	flags = { BLOCKS_VISION, CORNER_SE },
	groups = { "brick wall", "brick wall 2" }
}

obstacle {
	image_filenames = "iso_blood_0001.png",
	flags = { IS_VOLATILE, BLOCKS_VISION, NEEDS_PRE_PUT },
	transparency = NO_TRANSPARENCY,
	groups = "blood"
}

obstacle {
	image_filenames = "iso_blood_0002.png",
	flags = { IS_VOLATILE, BLOCKS_VISION, NEEDS_PRE_PUT },
	transparency = NO_TRANSPARENCY,
	groups = "blood"
}

obstacle {
	image_filenames = "iso_blood_0003.png",
	flags = { IS_VOLATILE, BLOCKS_VISION, NEEDS_PRE_PUT },
	transparency = NO_TRANSPARENCY,
	groups = "blood"
}

obstacle {
	image_filenames = "iso_blood_0004.png",
	flags = { IS_VOLATILE, BLOCKS_VISION, NEEDS_PRE_PUT },
	transparency = NO_TRANSPARENCY,
	groups = "blood"
}

obstacle {
	image_filenames = "iso_blood_0005.png",
	flags = { IS_VOLATILE, BLOCKS_VISION, NEEDS_PRE_PUT },
	transparency = NO_TRANSPARENCY,
	groups = "blood"
}

obstacle {
	image_filenames = "iso_blood_0006.png",
	flags = { IS_VOLATILE, BLOCKS_VISION, NEEDS_PRE_PUT },
	transparency = NO_TRANSPARENCY,
	groups = "blood"
}

obstacle {
	image_filenames = "iso_blood_0007.png",
	flags = { IS_VOLATILE, BLOCKS_VISION, NEEDS_PRE_PUT },
	transparency = NO_TRANSPARENCY,
	groups = "blood"
}

obstacle {
	image_filenames = "iso_blood_0008.png",
	flags = { IS_VOLATILE, BLOCKS_VISION, NEEDS_PRE_PUT },
	transparency = NO_TRANSPARENCY,
	groups = "blood"
}

obstacle {
	image_filenames = "iso_exits_0001.png",
	flags = { BLOCKS_VISION },
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_exits_0002.png",
	flags = { BLOCKS_VISION },
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_rocks_n_plants_0000.png",
	borders = borders(0.40, 0.40),
	flags = { GROUND_LEVEL },
	emitted_light_strength = 10,
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_rocks_n_plants_0001.png",
	borders = borders(1.50, 1.50),
	flags = { GROUND_LEVEL },
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_rocks_n_plants_0002.png",
	flags = { BLOCKS_VISION, NEEDS_PRE_PUT },
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_rocks_n_plants_0003.png",
	borders = borders(1.00, 1.00),
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_walls_0016.png",
	borders = borders(0.40, 1.10),
	flags = { IS_VERTICAL, BLOCKS_VISION },
}

obstacle {
	image_filenames = "iso_walls_0017.png",
	borders = borders(1.10, 0.40),
	flags = { IS_HORIZONTAL, BLOCKS_VISION },
}

obstacle {
	image_filenames = "iso_walls_0018.png",
	borders = borders(0.40, 1.10),
	flags = { IS_VERTICAL, BLOCKS_VISION },
}

obstacle {
	image_filenames = "iso_walls_0019.png",
	borders = borders(1.10, 0.40),
	flags = { IS_HORIZONTAL, BLOCKS_VISION },
}

obstacle {
	image_filenames = "iso_shop_furniture_0001.png",
	borders = borders(3.50, 1.50),
	flags = { GROUND_LEVEL },
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_shop_furniture_0002.png",
	borders = borders(1.50, 3.50),
	flags = { GROUND_LEVEL },
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_shop_furniture_0003.png",
	borders = borders(2.20, 0.60),
	flags = { BLOCKS_VISION },
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_shop_furniture_0004.png",
	borders = borders(0.60, 2.20),
	flags = { BLOCKS_VISION },
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_shop_furniture_0005.png",
	borders = borders(2.20, 0.60),
	flags = { BLOCKS_VISION },
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_shop_furniture_0006.png",
	borders = borders(0.60, 2.20),
	flags = { BLOCKS_VISION },
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_outer_walls_0002.png",
	borders = borders(-0.55, 0.55, -0.05, 0.60),
	flags = { IS_HORIZONTAL, BLOCKS_VISION },
	groups = "outer wall 1"
}

obstacle {
	image_filenames = "iso_outer_walls_0006.png",
	borders = borders(-0.55, 0.55, -0.05, 0.60),
	flags = { IS_HORIZONTAL, BLOCKS_VISION },
	groups = "outer wall 2"
}

obstacle {
	image_filenames = "iso_outer_walls_0010.png",
	borders = borders(-0.55, 0.55, -0.05, 0.60),
	flags = { IS_HORIZONTAL, BLOCKS_VISION },
	groups = "outer wall 3"
}

obstacle {
	image_filenames = "iso_outer_walls_0004.png",
	borders = borders(-0.55, 0.55, -0.05, 0.60),
	flags = { IS_HORIZONTAL, BLOCKS_VISION },
	groups = "outer wall 4"
}

obstacle {
	image_filenames = "iso_outer_walls_0008.png",
	borders = borders(-0.55, 0.55, -0.05, 0.60),
	flags = { IS_HORIZONTAL, BLOCKS_VISION },
	groups = "outer wall 5"
}

obstacle {
	image_filenames = "iso_outer_walls_0012.png",
	borders = borders(-0.55, 0.55, -0.05, 0.60),
	flags = { IS_HORIZONTAL, BLOCKS_VISION },
	groups = "outer wall 6"
}

obstacle {
	image_filenames = "iso_outer_walls_0003.png",
	borders = borders(-0.05, 0.60, -0.55, 0.55),
	flags = { IS_VERTICAL, BLOCKS_VISION },
	groups = "outer wall 4"
}

obstacle {
	image_filenames = "iso_outer_walls_0007.png",
	borders = borders(-0.05, 0.60, -0.55, 0.55),
	flags = { IS_VERTICAL, BLOCKS_VISION },
	groups = "outer wall 5"
}

obstacle {
	image_filenames = "iso_outer_walls_0011.png",
	borders = borders(-0.05, 0.60, -0.55, 0.55),
	flags = { IS_VERTICAL, BLOCKS_VISION },
	groups = "outer wall 6"
}

obstacle {
	image_filenames = "iso_outer_walls_0001.png",
	borders = borders(-0.05, 0.60, -0.55, 0.55),
	flags = { IS_VERTICAL, BLOCKS_VISION },
	groups = "outer wall 1"
}

obstacle {
	image_filenames = "iso_outer_walls_0005.png",
	borders = borders(-0.05, 0.60, -0.55, 0.55),
	flags = { IS_VERTICAL, BLOCKS_VISION },
	groups = "outer wall 2"
}

obstacle {
	image_filenames = "iso_outer_walls_0009.png",
	borders = borders(-0.05, 0.60, -0.55, 0.55),
	flags = { IS_VERTICAL, BLOCKS_VISION },
	groups = "outer wall 3"
}

obstacle {
	image_filenames = "iso_outer_walls_0013.png",
	borders = borders(1.10, 1.10),
	flags = { BLOCKS_VISION, CORNER_NW },
	groups = { "outer wall 1", "outer wall 2", "outer wall 3", "outer wall 4", "outer wall 5", "outer wall 6" }
}

obstacle {
	image_filenames = "iso_outer_walls_0014.png",
	borders = borders(1.10, 1.10),
	flags = { BLOCKS_VISION, CORNER_SW },
	groups = { "outer wall 1", "outer wall 2", "outer wall 3", "outer wall 4", "outer wall 5", "outer wall 6" }
}

obstacle {
	image_filenames = "iso_outer_walls_0015.png",
	borders = borders(1.10, 1.10),
	flags = { BLOCKS_VISION, CORNER_SE },
	groups = { "outer wall 1", "outer wall 2", "outer wall 3", "outer wall 4", "outer wall 5", "outer wall 6" }
}

obstacle {
	image_filenames = "iso_outer_walls_0016.png",
	borders = borders(1.10, 1.10),
	flags = { BLOCKS_VISION, CORNER_NE },
	groups = { "outer wall 1", "outer wall 2", "outer wall 3", "outer wall 4", "outer wall 5", "outer wall 6" }
}

obstacle {
	image_filenames = "iso_doors_0018.png",
	borders = borders(-0.05, 0.60, -1.55, 0.55),
	flags = { IS_WALKABLE },
	animation = "door",
}

obstacle {
	image_filenames = "iso_doors_0019.png",
	borders = borders(1.20, 1.20),
	flags = { IS_WALKABLE },
	animation = "door",
}

obstacle {
	image_filenames = "iso_doors_0020.png",
	borders = borders(1.20, 1.20),
	flags = { IS_WALKABLE },
	animation = "door",
}

obstacle {
	image_filenames = "iso_doors_0021.png",
	borders = borders(1.20, 1.20),
	flags = { IS_WALKABLE },
	animation = "door",
}

obstacle {
	image_filenames = "iso_doors_0022.png",
	flags = { BLOCKS_VISION },
	animation = "door",
}

obstacle {
	image_filenames = "iso_doors_0013.png",
	borders = borders(-1.55, 0.55, -0.05, 0.60),
	flags = { IS_WALKABLE },
	animation = "door",
}

obstacle {
	image_filenames = "iso_doors_0014.png",
	borders = borders(1.20, 1.20),
	flags = { IS_WALKABLE },
	animation = "door",
}

obstacle {
	image_filenames = "iso_doors_0015.png",
	borders = borders(1.20, 1.20),
	flags = { IS_WALKABLE },
	animation = "door",
}

obstacle {
	image_filenames = "iso_doors_0016.png",
	borders = borders(1.20, 1.20),
	flags = { IS_WALKABLE },
	animation = "door",
}

obstacle {
	image_filenames = "iso_doors_0017.png",
	flags = { BLOCKS_VISION },
	animation = "door",
}

obstacle {
	image_filenames = "iso_doors_0024.png",
	borders = borders(-0.05, 0.60, -1.55, 0.55),
}

obstacle {
	image_filenames = "iso_doors_0023.png",
	borders = borders(-1.55, 0.55, -0.05, 0.60),
}

obstacle {
	image_filenames = "iso_machinery_0002.png",
	borders = borders(1.00, 1.00),
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_machinery_0003.png",
	borders = borders(1.00, 1.00),
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_machinery_0004.png",
	borders = borders(1.00, 1.00),
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_chairs_0004.png",
	borders = borders(0.80, 0.80),
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_chairs_0001.png",
	borders = borders(0.80, 0.80),
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_chairs_0002.png",
	borders = borders(0.80, 0.80),
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_chairs_0003.png",
	borders = borders(0.80, 0.80),
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_chairs_0008.png",
	borders = borders(1.60, 0.80),
	flags = { GROUND_LEVEL },
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_chairs_0005.png",
	borders = borders(0.80, 1.60),
	flags = { GROUND_LEVEL },
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_chairs_0006.png",
	borders = borders(1.60, 0.80),
	flags = { GROUND_LEVEL },
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_chairs_0007.png",
	borders = borders(0.80, 1.60),
	flags = { GROUND_LEVEL },
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_body_0001.png",
	flags = { BLOCKS_VISION, NEEDS_PRE_PUT },
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_body_0002.png",
	flags = { BLOCKS_VISION, NEEDS_PRE_PUT },
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_body_0003.png",
	flags = { BLOCKS_VISION, NEEDS_PRE_PUT },
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_body_0004.png",
	flags = { BLOCKS_VISION, NEEDS_PRE_PUT },
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_conference_furniture_0001.png",
	borders = borders(2.00, 2.00),
	flags = { GROUND_LEVEL },
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_conference_furniture_0000.png",
	borders = borders(2.00, 2.00),
	flags = { GROUND_LEVEL },
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_conference_furniture_0003.png",
	borders = borders(2.00, 2.00),
	flags = { GROUND_LEVEL },
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_conference_furniture_0002.png",
	borders = borders(2.00, 2.00),
	flags = { GROUND_LEVEL },
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_fence_0001.png",
	borders = borders(0.80, 2.30),
}

obstacle {
	image_filenames = "iso_fence_0002.png",
	borders = borders(2.30, 0.80),
}

obstacle {
	image_filenames = "iso_beds_0000.png",
	borders = borders(1.20, 2.00),
	flags = { GROUND_LEVEL },
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_beds_0001.png",
	borders = borders(2.00, 1.20),
	flags = { GROUND_LEVEL },
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_beds_0002.png",
	borders = borders(1.20, 2.00),
	flags = { GROUND_LEVEL },
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_beds_0003.png",
	borders = borders(2.00, 1.20),
	flags = { GROUND_LEVEL },
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_beds_0004.png",
	borders = borders(1.20, 2.00),
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_beds_0005.png",
	borders = borders(2.00, 1.20),
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_beds_0006.png",
	borders = borders(1.20, 2.00),
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_beds_0007.png",
	borders = borders(2.00, 1.20),
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_conference_furniture_0004.png",
	borders = borders(0.50, 0.50),
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_conference_furniture_0006.png",
	borders = borders(0.50, 0.50),
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_obstacle_0056.png",
	borders = borders(0.50, 0.50),
	flags = { GROUND_LEVEL },
	emitted_light_strength = 24,
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_obstacle_0055.png",
	borders = borders(0.50, 0.50),
	flags = { GROUND_LEVEL },
	emitted_light_strength = 24,
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_obstacle_0057.png",
	borders = borders(0.50, 0.50),
	flags = { GROUND_LEVEL },
	emitted_light_strength = 24,
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_rocks_n_plants_0004.png",
	borders = borders(1.00, 1.00),
	flags = { GROUND_LEVEL },
	emitted_light_strength = 7,
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_rocks_n_plants_0005.png",
	borders = borders(1.00, 1.00),
	flags = { GROUND_LEVEL },
	emitted_light_strength = 9,
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_rocks_n_plants_0006.png",
	borders = borders(0.90, 0.90),
	flags = { GROUND_LEVEL },
	emitted_light_strength = 8,
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_rocks_n_plants_0007.png",
	borders = borders(0.90, 0.90),
	flags = { GROUND_LEVEL },
	emitted_light_strength = 11,
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_brick_wall_0008.png",
	borders = borders(1.20, 1.20),
	flags = { BLOCKS_VISION },
}

obstacle {
	image_filenames = "iso_brick_wall_0009.png",
	borders = borders(1.20, 1.20),
	flags = { BLOCKS_VISION },
}

obstacle {
	image_filenames = "iso_brick_wall_0010.png",
	borders = borders(1.20, 1.20),
	flags = { BLOCKS_VISION },
}

obstacle {
	image_filenames = "iso_brick_wall_0011.png",
	borders = borders(1.20, 1.20),
	flags = { BLOCKS_VISION },
}

obstacle {
	image_filenames = "iso_brick_wall_0012.png",
	label = "",
	borders = borders(0.50, 1.20),
	flags = { IS_VERTICAL, BLOCKS_VISION, IS_SMASHABLE, IS_CLICKABLE },
	after_smashing = 237,
	action = "barrel",
}

obstacle {
	image_filenames = "iso_brick_wall_0013.png",
	label = "",
	borders = borders(1.20, 0.50),
	flags = { IS_HORIZONTAL, BLOCKS_VISION, IS_SMASHABLE, IS_CLICKABLE },
	after_smashing = 238,
	action = "barrel",
}

obstacle {
	image_filenames = "iso_brick_wall_0014.png",
	flags = { IS_VERTICAL, BLOCKS_VISION },
}

obstacle {
	image_filenames = "iso_brick_wall_0015.png",
	flags = { IS_HORIZONTAL, BLOCKS_VISION },
}

obstacle {
	image_filenames = "iso_conference_furniture_0011.png",
	borders = borders(2.20, 1.00),
	flags = { BLOCKS_VISION },
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_conference_furniture_0008.png",
	borders = borders(1.00, 2.20),
	flags = { BLOCKS_VISION },
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_conference_furniture_0009.png",
	borders = borders(2.00, 1.00),
	flags = { BLOCKS_VISION },
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_conference_furniture_0010.png",
	borders = borders(1.00, 2.20),
	flags = { BLOCKS_VISION },
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_conference_furniture_0007.png",
	borders = borders(0.50, 0.50),
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_conference_furniture_0005.png",
	borders = borders(0.50, 0.50),
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_signs_0000.png",
	label = "Sign",
	borders = borders(0.50, 0.60),
	emitted_light_strength = 5,
	flags = { GROUND_LEVEL, IS_CLICKABLE },
	transparency = NO_TRANSPARENCY,
	action = "sign",
}

obstacle {
	image_filenames = "iso_signs_0001.png",
	label = "Sign",
	borders = borders(0.60, 0.50),
	emitted_light_strength = 5,
	flags = { GROUND_LEVEL, IS_CLICKABLE },
	transparency = NO_TRANSPARENCY,
	action = "sign",
}

obstacle {
	image_filenames = "iso_signs_0002.png",
	label = "Sign",
	borders = borders(0.50, 0.60),
	emitted_light_strength = 5,
	flags = { GROUND_LEVEL, IS_CLICKABLE },
	transparency = NO_TRANSPARENCY,
	action = "sign",
}

obstacle {
	image_filenames = "iso_walls_0010.png",
	borders = borders(0.40, 1.10),
	flags = { IS_VERTICAL, BLOCKS_VISION },
}

obstacle {
	image_filenames = "iso_walls_0011.png",
	borders = borders(1.10, 0.40),
	flags = { IS_HORIZONTAL, BLOCKS_VISION },
}

obstacle {
	image_filenames = "iso_walls_0012.png",
	borders = borders(0.40, 1.10),
	flags = { IS_VERTICAL, BLOCKS_VISION },
}

obstacle {
	image_filenames = "iso_walls_0013.png",
	borders = borders(1.10, 0.40),
	flags = { IS_HORIZONTAL, BLOCKS_VISION },
}

obstacle {
	image_filenames = "iso_walls_0014.png",
	borders = borders(0.40, 1.10),
	flags = { IS_VERTICAL, BLOCKS_VISION },
}

obstacle {
	image_filenames = "iso_walls_0015.png",
	borders = borders(1.10, 0.40),
	flags = { IS_HORIZONTAL, BLOCKS_VISION },
}

obstacle {
	image_filenames = "iso_counter_0001.png",
	borders = borders(0.80, 1.05),
	flags = { GROUND_LEVEL },
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_counter_0002.png",
	borders = borders(1.05, 0.80),
	flags = { GROUND_LEVEL },
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_counter_0003.png",
	borders = borders(0.80, 1.05),
	flags = { GROUND_LEVEL },
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_counter_0004.png",
	borders = borders(1.05, 0.80),
	flags = { GROUND_LEVEL },
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_counter_0005.png",
	borders = borders(1.10, 1.10),
	flags = { GROUND_LEVEL },
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_counter_0006.png",
	borders = borders(1.10, 1.10),
	flags = { GROUND_LEVEL },
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_counter_0007.png",
	borders = borders(1.10, 1.10),
	flags = { GROUND_LEVEL },
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_counter_0008.png",
	borders = borders(1.10, 1.10),
	flags = { GROUND_LEVEL },
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_counter_0009.png",
	borders = borders(1.10, 1.10),
	flags = { GROUND_LEVEL },
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_counter_0010.png",
	borders = borders(1.10, 1.10),
	flags = { GROUND_LEVEL },
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_counter_0011.png",
	borders = borders(1.10, 1.10),
	flags = { GROUND_LEVEL },
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_counter_0012.png",
	borders = borders(1.10, 1.10),
	flags = { GROUND_LEVEL },
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_library_furniture_0001.png",
	borders = borders(3.50, 1.50),
	flags = { GROUND_LEVEL },
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_library_furniture_0002.png",
	borders = borders(1.50, 3.50),
	flags = { GROUND_LEVEL },
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_bathroom_furniture_0002.png",
	borders = borders(1.50, 1.00),
	flags = { IS_SMASHABLE, GROUND_LEVEL },
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_bathroom_furniture_0003.png",
	borders = borders(1.00, 1.50),
	flags = { IS_SMASHABLE, GROUND_LEVEL },
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_tables_0005.png",
	borders = borders(0.80, 0.80),
	flags = { GROUND_LEVEL },
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_exits_0003.png",
	flags = { BLOCKS_VISION },
	emitted_light_strength = 29,
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_exits_0004.png",
	flags = { BLOCKS_VISION },
	emitted_light_strength = 29,
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_outer_walls_0017.png",
	borders = borders(1.00, 1.00),
	flags = { BLOCKS_VISION },
}

obstacle {
	image_filenames = "iso_outer_walls_0018.png",
	borders = borders(1.00, 1.00),
	flags = { BLOCKS_VISION },
}

obstacle {
	image_filenames = "iso_outer_walls_0019.png",
	borders = borders(1.00, 1.00),
	flags = { BLOCKS_VISION },
}

obstacle {
	image_filenames = "iso_outer_walls_0020.png",
	borders = borders(1.00, 1.00),
	flags = { BLOCKS_VISION },
}

obstacle {
	image_filenames = "iso_tables_0006.png",
	borders = borders(0.85, 1.50),
	flags = { GROUND_LEVEL },
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_tables_0007.png",
	borders = borders(1.50, 0.85),
	flags = { GROUND_LEVEL },
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_tables_0008.png",
	borders = borders(1.00, 1.20),
	flags = { GROUND_LEVEL },
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_tables_0009.png",
	borders = borders(1.20, 1.00),
	flags = { GROUND_LEVEL },
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_walls_0020.png",
	label = "",
	borders = borders(0.40, 1.10),
	flags = { IS_VERTICAL, IS_SMASHABLE, IS_CLICKABLE },
	after_smashing = 348,
	transparency = NO_TRANSPARENCY,
	action = "barrel",
}

obstacle {
	image_filenames = "iso_walls_0021.png",
	label = "",
	borders = borders(1.10, 0.40),
	flags = { IS_HORIZONTAL, IS_SMASHABLE, IS_CLICKABLE },
	after_smashing = 446,
	transparency = NO_TRANSPARENCY,
	action = "barrel",
}

obstacle {
	image_filenames = "iso_walls_0022.png",
	borders = borders(0.40, 1.10),
	flags = { IS_VERTICAL },
}

obstacle {
	image_filenames = "iso_walls_0023.png",
	borders = borders(1.10, 0.40),
	flags = { IS_HORIZONTAL },
}

obstacle {
	image_filenames = "iso_walls_0024.png",
	borders = borders(0.40, 1.10),
	flags = { IS_VERTICAL },
}

obstacle {
	image_filenames = "iso_walls_0025.png",
	borders = borders(1.10, 0.40),
	flags = { IS_HORIZONTAL },
}

obstacle {
	image_filenames = "iso_walls_0026.png",
	borders = borders(0.40, 1.10),
	flags = { IS_VERTICAL },
}

obstacle {
	image_filenames = "iso_walls_0027.png",
	borders = borders(1.10, 0.40),
	flags = { IS_HORIZONTAL },
}

obstacle {
	image_filenames = "iso_walls_0028.png",
	borders = borders(0.40, 1.10),
	flags = { IS_VERTICAL },
}

obstacle {
	image_filenames = "iso_walls_0029.png",
	borders = borders(1.10, 0.40),
	flags = { IS_HORIZONTAL },
}

obstacle {
	image_filenames = "iso_restaurant_furniture_0001.png",
	borders = borders(0.60, 0.60),
	flags = { BLOCKS_VISION },
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_restaurant_furniture_0002.png",
	borders = borders(0.60, 0.60),
	flags = { BLOCKS_VISION },
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_restaurant_furniture_0003.png",
	borders = borders(0.60, 0.60),
	flags = { BLOCKS_VISION },
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_restaurant_furniture_0004.png",
	borders = borders(0.60, 0.60),
	flags = { BLOCKS_VISION },
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_restaurant_furniture_0005.png",
	borders = borders(0.60, 0.60),
	flags = { BLOCKS_VISION },
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_restaurant_furniture_0006.png",
	borders = borders(0.60, 0.60),
	flags = { BLOCKS_VISION },
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_restaurant_furniture_0007.png",
	borders = borders(0.60, 0.60),
	flags = { BLOCKS_VISION },
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_restaurant_furniture_0008.png",
	borders = borders(0.60, 0.60),
	flags = { BLOCKS_VISION },
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_restaurant_furniture_0009.png",
	borders = borders(0.60, 0.60),
	flags = { BLOCKS_VISION },
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_restaurant_furniture_0010.png",
	borders = borders(0.60, 0.60),
	flags = { BLOCKS_VISION },
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_chairs_0013.png",
	borders = borders(0.60, 1.20),
	flags = { GROUND_LEVEL },
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_chairs_0014.png",
	borders = borders(1.20, 0.60),
	flags = { GROUND_LEVEL },
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_chairs_0015.png",
	borders = borders(0.60, 1.20),
	flags = { GROUND_LEVEL },
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_chairs_0016.png",
	borders = borders(1.20, 0.60),
	flags = { GROUND_LEVEL },
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_chairs_0017.png",
	borders = borders(0.60, 0.60),
	flags = { GROUND_LEVEL },
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_chairs_0018.png",
	borders = borders(0.60, 0.60),
	flags = { GROUND_LEVEL },
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_chairs_0019.png",
	borders = borders(0.60, 0.60),
	flags = { GROUND_LEVEL },
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_chairs_0020.png",
	borders = borders(0.60, 0.60),
	flags = { GROUND_LEVEL },
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_chairs_0021.png",
	borders = borders(0.60, 0.60),
	flags = { GROUND_LEVEL },
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_chairs_0022.png",
	borders = borders(0.60, 0.60),
	flags = { GROUND_LEVEL },
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_chairs_0023.png",
	borders = borders(0.60, 0.60),
	flags = { GROUND_LEVEL },
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_chairs_0024.png",
	borders = borders(0.60, 0.60),
	flags = { GROUND_LEVEL },
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_oil_stains_0001.png",
	flags = { IS_VOLATILE, BLOCKS_VISION, NEEDS_PRE_PUT },
	transparency = NO_TRANSPARENCY,
	groups = "oil stains"
}

obstacle {
	image_filenames = "iso_oil_stains_0002.png",
	flags = { IS_VOLATILE, BLOCKS_VISION, NEEDS_PRE_PUT },
	transparency = NO_TRANSPARENCY,
	groups = "oil stains"
}

obstacle {
	image_filenames = "iso_oil_stains_0003.png",
	flags = { IS_VOLATILE, BLOCKS_VISION, NEEDS_PRE_PUT },
	transparency = NO_TRANSPARENCY,
	groups = "oil stains"
}

obstacle {
	image_filenames = "iso_oil_stains_0004.png",
	flags = { IS_VOLATILE, BLOCKS_VISION, NEEDS_PRE_PUT },
	transparency = NO_TRANSPARENCY,
	groups = "oil stains"
}

obstacle {
	image_filenames = "iso_oil_stains_0005.png",
	flags = { IS_VOLATILE, BLOCKS_VISION, NEEDS_PRE_PUT },
	transparency = NO_TRANSPARENCY,
	groups = "oil stains"
}

obstacle {
	image_filenames = "iso_oil_stains_0006.png",
	flags = { IS_VOLATILE, BLOCKS_VISION, NEEDS_PRE_PUT },
	transparency = NO_TRANSPARENCY,
	groups = "oil stains"
}

obstacle {
	image_filenames = "iso_oil_stains_0007.png",
	flags = { IS_VOLATILE, BLOCKS_VISION, NEEDS_PRE_PUT },
	transparency = NO_TRANSPARENCY,
	groups = "oil stains"
}

obstacle {
	image_filenames = "iso_oil_stains_0008.png",
	flags = { IS_VOLATILE, BLOCKS_VISION, NEEDS_PRE_PUT },
	transparency = NO_TRANSPARENCY,
	groups = "oil stains"
}

obstacle {
	image_filenames = "iso_transp_for_water.png",
	borders = borders(1.00, 1.00),
	flags = { GROUND_LEVEL },
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_brick_wall_0017.png",
	borders = borders(1.20, 0.80),
	flags = { IS_HORIZONTAL, BLOCKS_VISION },
	after_smashing = 235,
	groups = "brick wall 2"
}

obstacle {
	image_filenames = "iso_brick_wall_0016.png",
	borders = borders(0.80, 1.20),
	flags = { IS_VERTICAL, BLOCKS_VISION },
	after_smashing = 236,
	groups = "brick wall 2"
}

obstacle {
	image_filenames = "iso_autogun_0001.png",
	borders = borders(0.70, 0.70),
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_autogun_0002.png",
	borders = borders(0.70, 0.70),
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_autogun_0003.png",
	borders = borders(0.70, 0.70),
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_autogun_0004.png",
	borders = borders(0.70, 0.70),
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_brick_wall_cables_0001.png",
	borders = borders(1.20, 0.80),
	flags = { IS_HORIZONTAL, BLOCKS_VISION },
	groups = "brick wall cables"
}

obstacle {
	image_filenames = "iso_brick_wall_cables_0002.png",
	borders = borders(0.80, 1.20),
	flags = { IS_VERTICAL, BLOCKS_VISION },
	groups = "brick wall cables"
}

obstacle {
	image_filenames = "iso_brick_wall_cables_0004.png",
	borders = borders(-0.60, 0.30, -0.60, 0.60),
	flags = { BLOCKS_VISION, CORNER_NE },
	groups = "brick wall cables"
}

obstacle {
	image_filenames = "iso_brick_wall_cables_0005.png",
	borders = borders(-0.60, 0.65, -0.60, 0.30),
	flags = { BLOCKS_VISION, CORNER_SW },
	groups = "brick wall cables"
}

obstacle {
	image_filenames = "iso_brick_wall_cables_0006.png",
	borders = borders(-0.30, 0.60, -0.30, 0.60),
	flags = { BLOCKS_VISION, CORNER_NW },
	groups = "brick wall cables"
}

obstacle {
	image_filenames = "iso_brick_wall_cables_0007.png",
	borders = borders(-0.60, 0.30, -0.60, 0.30),
	flags = { BLOCKS_VISION, CORNER_SE },
	groups = "brick wall cables"
}

obstacle {
	image_filenames = "iso_restaurant_desk_0001.png",
	borders = borders(1.50, 5.00),
	flags = { GROUND_LEVEL },
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_restaurant_desk_0002.png",
	borders = borders(5.00, 1.50),
	flags = { GROUND_LEVEL },
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_restaurant_desk_0003.png",
	borders = borders(0.65, 5.50),
	flags = { BLOCKS_VISION },
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_restaurant_desk_0004.png",
	borders = borders(5.50, 0.65),
	flags = { BLOCKS_VISION },
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_crystal_fields_0001.png",
	borders = borders(0.50, 0.50),
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_crystal_fields_0002.png",
	borders = borders(1.15, 1.15),
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_crystal_fields_0003.png",
	borders = borders(0.95, 0.95),
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_crystal_fields_0004.png",
	borders = borders(1.25, 1.05),
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_crystal_fields_0005.png",
	borders = borders(1.20, 1.05),
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_crystal_fields_0006.png",
	borders = borders(1.10, 1.10),
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_wall_corners_0001.png",
	borders = borders(1.10, 1.00),
	flags = { BLOCKS_VISION },
}

obstacle {
	image_filenames = "iso_wall_corners_0002.png",
	borders = borders(1.10, 1.00),
	flags = { BLOCKS_VISION },
}

obstacle {
	image_filenames = "iso_wall_corners_0003.png",
	borders = borders(1.10, 1.00),
	flags = { BLOCKS_VISION },
}

obstacle {
	image_filenames = "iso_wall_corners_0004.png",
	borders = borders(1.10, 1.00),
	flags = { BLOCKS_VISION },
}

obstacle {
	image_filenames = "iso_walls_0030.png",
	flags = { IS_WALKABLE },
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_doors_0022_blocked.png",
	flags = { BLOCKS_VISION },
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_doors_0017_blocked.png",
	flags = { BLOCKS_VISION },
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_doubledoors_0011.png",
	borders = borders(-0.55, 1.55, -0.80, 0.20),
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_doubledoors_0012.png",
	borders = borders(-0.80, 0.20, -0.55, 1.55),
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_doubledoors_0001.png",
	borders = borders(-0.55, 1.55, -0.20, 0.20),
	flags = { IS_WALKABLE },
	transparency = NO_TRANSPARENCY,
	animation = "door",
}

obstacle {
	image_filenames = "iso_doubledoors_0002.png",
	borders = borders(-0.55, 1.55, -0.20, 0.20),
	flags = { IS_WALKABLE },
	transparency = NO_TRANSPARENCY,
	animation = "door",
}

obstacle {
	image_filenames = "iso_doubledoors_0003.png",
	borders = borders(-0.55, 1.55, -0.20, 0.20),
	flags = { IS_WALKABLE },
	transparency = NO_TRANSPARENCY,
	animation = "door",
}

obstacle {
	image_filenames = "iso_doubledoors_0004.png",
	borders = borders(-0.55, 1.55, -0.20, 0.20),
	flags = { IS_WALKABLE },
	transparency = NO_TRANSPARENCY,
	animation = "door",
}

obstacle {
	image_filenames = "iso_doubledoors_0005.png",
	flags = { BLOCKS_VISION },
	transparency = NO_TRANSPARENCY,
	animation = "door",
}

obstacle {
	image_filenames = "iso_doubledoors_0006.png",
	borders = borders(-0.20, 0.20, -0.55, 1.55),
	flags = { IS_WALKABLE },
	transparency = NO_TRANSPARENCY,
	animation = "door",
}

obstacle {
	image_filenames = "iso_doubledoors_0007.png",
	borders = borders(-0.20, 0.20, -0.55, 1.55),
	flags = { IS_WALKABLE },
	transparency = NO_TRANSPARENCY,
	animation = "door",
}

obstacle {
	image_filenames = "iso_doubledoors_0008.png",
	borders = borders(-0.20, 0.20, -0.55, 1.55),
	flags = { IS_WALKABLE },
	transparency = NO_TRANSPARENCY,
	animation = "door",
}

obstacle {
	image_filenames = "iso_doubledoors_0009.png",
	borders = borders(-0.20, 0.20, -0.55, 1.55),
	flags = { IS_WALKABLE },
	transparency = NO_TRANSPARENCY,
	animation = "door",
}

obstacle {
	image_filenames = "iso_doubledoors_0010.png",
	flags = { BLOCKS_VISION },
	transparency = NO_TRANSPARENCY,
	animation = "door",
}

obstacle {
	image_filenames = "iso_basin_0001.png",
	borders = borders(1.05, 0.95),
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_basin_0002.png",
	borders = borders(0.95, 1.05),
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_basin_0003.png",
	borders = borders(1.05, 0.95),
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_basin_0004.png",
	borders = borders(0.95, 1.05),
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_deskchair_0001.png",
	borders = borders(0.90, 0.90),
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_deskchair_0002.png",
	borders = borders(0.90, 0.90),
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_deskchair_0003.png",
	borders = borders(0.90, 0.90),
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_exits_0005.png",
	flags = { BLOCKS_VISION },
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_exits_0006.png",
	flags = { BLOCKS_VISION },
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_container_0006.png",
	label = "Chest",
	borders = borders(0.60, 0.80),
	flags = { GROUND_LEVEL, IS_CLICKABLE },
	transparency = NO_TRANSPARENCY,
	action = "chest",
}

obstacle {
	image_filenames = "iso_container_0005.png",
	label = "Chest",
	borders = borders(0.80, 0.60),
	flags = { GROUND_LEVEL, IS_CLICKABLE },
	transparency = NO_TRANSPARENCY,
	action = "chest",
}

obstacle {
	image_filenames = "iso_container_0008.png",
	borders = borders(0.60, 0.80),
	flags = { GROUND_LEVEL },
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_container_0007.png",
	borders = borders(0.80, 0.60),
	flags = { GROUND_LEVEL },
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_container_0009.png",
	label = "Chest",
	borders = borders(0.80, 0.60),
	flags = { GROUND_LEVEL, IS_CLICKABLE },
	transparency = NO_TRANSPARENCY,
	action = "chest",
}

obstacle {
	image_filenames = "iso_container_0010.png",
	label = "Chest",
	borders = borders(0.60, 0.80),
	flags = { GROUND_LEVEL, IS_CLICKABLE },
	transparency = NO_TRANSPARENCY,
	action = "chest",
}

obstacle {
	image_filenames = "iso_container_0011.png",
	borders = borders(0.80, 0.60),
	flags = { GROUND_LEVEL },
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_container_0012.png",
	borders = borders(0.60, 0.80),
	flags = { GROUND_LEVEL },
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_security_gate_0001.png",
	borders = borders(0.95, 1.05),
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_security_gate_0002.png",
	borders = borders(0.95, 1.05),
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_security_gate_0003.png",
	borders = borders(0.95, 1.05),
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_security_gate_0004.png",
	borders = borders(0.95, 1.05),
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_security_gate_0005.png",
	borders = borders(0.95, 1.05),
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_security_gate_0006.png",
	borders = borders(0.95, 1.05),
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_security_gate_0007.png",
	borders = borders(0.95, 1.05),
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_security_gate_0008.png",
	borders = borders(0.95, 1.05),
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_solar_panel_0001.png",
	label = "Solar Panel",
	borders = borders(0.95, 1.05),
	flags = { IS_SMASHABLE, IS_CLICKABLE },
	after_smashing = 407,
	transparency = NO_TRANSPARENCY,
	action = "barrel",
}

obstacle {
	image_filenames = "iso_botline_0000.png",
	borders = borders(3.00, 2.00),
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_botline_0001.png",
	borders = borders(2.00, 3.00),
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_botline_0002.png",
	borders = borders(2.46, 1.94),
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_botline_0003.png",
	borders = borders(1.94, 2.46),
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_botline_0004.png",
	borders = borders(2.46, 1.94),
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_botline_0005.png",
	borders = borders(1.94, 2.46),
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_botline_0006_N.png",
	borders = borders(1.00, 1.00),
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_botline_0006_W.png",
	borders = borders(1.00, 1.00),
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_botline_0006_S.png",
	borders = borders(1.00, 1.00),
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_botline_0006_E.png",
	borders = borders(1.00, 1.00),
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_botline_0007_N.png",
	borders = borders(1.00, 1.00),
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_botline_0007_W.png",
	borders = borders(1.00, 1.00),
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_freighter_railway_0000.png",
	borders = borders(3.00, 3.00),
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_freighter_railway_0001.png",
	borders = borders(3.00, 3.00),
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_freighter_railway_0002.png",
	borders = borders(3.00, 3.00),
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_freighter_railway_0003.png",
	borders = borders(3.00, 3.00),
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_freighter_railway_0004.png",
	borders = borders(3.00, 3.00),
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_freighter_railway_0005.png",
	borders = borders(3.00, 3.00),
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_solar_panel_0000.png",
	borders = borders(0.95, 1.05),
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_barrel_5.png",
	label = "Crate",
	borders = borders(0.80, 0.95),
	flags = { BLOCKS_VISION, IS_SMASHABLE, DROPS_RANDOM_TREASURE, IS_CLICKABLE },
	transparency = NO_TRANSPARENCY,
	action = "barrel",
}

obstacle {
	image_filenames = "iso_reactor_1_0000.png",
	borders = borders(4.50, 4.00),
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_reactor_1_0001.png",
	borders = borders(4.00, 4.50),
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_reactor_1_0002.png",
	borders = borders(4.50, 4.00),
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_reactor_1_0003.png",
	borders = borders(4.00, 4.50),
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_wall_terminal_0000.png",
	label = "Terminal",
	borders = borders(0.60, 0.40),
	flags = { IS_CLICKABLE },
	transparency = NO_TRANSPARENCY,
	action = "terminal",
}

obstacle {
	image_filenames = "iso_wall_terminal_0002.png",
	label = "Terminal",
	borders = borders(0.40, 0.60),
	flags = { IS_CLICKABLE },
	transparency = NO_TRANSPARENCY,
	action = "terminal",
}

obstacle {
	image_filenames = "iso_wall_terminal_0004.png",
	label = "Terminal",
	borders = borders(0.60, 0.40),
	flags = { IS_CLICKABLE },
	transparency = NO_TRANSPARENCY,
	action = "terminal",
}

obstacle {
	image_filenames = "iso_wall_terminal_0006.png",
	label = "Terminal",
	borders = borders(0.40, 0.60),
	flags = { IS_CLICKABLE },
	transparency = NO_TRANSPARENCY,
	action = "terminal",
}

obstacle {
	image_filenames = "iso_turbines_small_0000.png",
	borders = borders(1.10, 1.80),
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_turbines_small_0001.png",
	borders = borders(1.80, 1.05),
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_turbines_small_0002.png",
	borders = borders(1.10, 1.80),
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_turbines_small_0003.png",
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
	image_filenames = "iso_default_dead_body_00_0001.png",
	flags = { BLOCKS_VISION },
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_default_dead_body_02_0001.png",
	flags = { BLOCKS_VISION },
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_default_dead_body_04_0001.png",
	flags = { BLOCKS_VISION },
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_default_dead_body_06_0001.png",
	flags = { BLOCKS_VISION },
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_default_dead_body_08_0001.png",
	flags = { BLOCKS_VISION },
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_default_dead_body_10_0001.png",
	flags = { BLOCKS_VISION },
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_default_dead_body_12_0001.png",
	flags = { BLOCKS_VISION },
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_default_dead_body_14_0001.png",
	flags = { BLOCKS_VISION },
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_default_dead_human_00_0001.png",
	flags = { BLOCKS_VISION },
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_ladder_0001.png",
	flags = { BLOCKS_VISION },
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_ladder_0002.png",
	flags = { BLOCKS_VISION },
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_wrecked_car_0001.png",
	borders = borders(1.40, 2.80),
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_wrecked_car_0002.png",
	borders = borders(2.80, 1.40),
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_wrecked_car_0003.png",
	borders = borders(1.40, 2.80),
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_wrecked_car_0004.png",
	borders = borders(2.80, 1.40),
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_obstacle_0068.png",
	borders = borders(0.40, 0.40),
	flags = { IS_SMASHABLE, GROUND_LEVEL },
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_obstacle_0067.png",
	borders = borders(0.40, 0.40),
	flags = { IS_SMASHABLE, GROUND_LEVEL },
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_botline_0007_S.png",
	borders = borders(1.00, 1.00),
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_botline_0007_E.png",
	borders = borders(1.00, 1.00),
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_botline_0008_N.png",
	borders = borders(1.00, 1.00),
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_botline_0008_W.png",
	borders = borders(1.00, 1.00),
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_botline_0008_S.png",
	borders = borders(1.00, 1.00),
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_botline_0008_E.png",
	borders = borders(1.00, 1.00),
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_crushed_476_0001.png",
	borders = borders(2.10, 2.60),
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_walls_0031.png",
	flags = { IS_WALKABLE },
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = { "iso_signs_0000_dark_00.png", "iso_signs_0000_dark_01.png" },
	label = "Sign",
	borders = borders(0.50, 0.60),
	emitted_light_strength = { 0, 5 },
	flags = { GROUND_LEVEL, IS_CLICKABLE },
	transparency = NO_TRANSPARENCY,
	action = "sign",
	animation_fps = 0.44
}

obstacle {
	image_filenames = { "iso_signs_0001_dark_00.png", "iso_signs_0001_dark_01.png" },
	label = "Sign",
	borders = borders(0.60, 0.50),
	emitted_light_strength = { 0, 5 },
	flags = { GROUND_LEVEL, IS_CLICKABLE },
	transparency = NO_TRANSPARENCY,
	action = "sign",
	animation_fps = 0.4
}
obstacle {
	image_filenames = { "iso_signs_0002_dark_00.png", "iso_signs_0002_dark_01.png" },
	label = "Sign",
	borders = borders(0.50, 0.60),
	emitted_light_strength = { 0, 5 },
	flags = { GROUND_LEVEL, IS_CLICKABLE },
	transparency = NO_TRANSPARENCY,
	action = "sign",
	animation_fps = 0.6
}
	
obstacle {
	image_filenames = { "iso_barrel_radioactive_0000.png" },
	borders = borders(0.70, 0.70),
	emitted_light_strength = { 1, 2, 3, 2, 1 },
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_vending_machine_0000.png",
	label = "Vending Machine",
	borders = borders(1.10, 1.55),
	emitted_light_strength = { 3, 5, 7, 6, 4, 3, 3, 0, 0, 0, 3, 0 },
	flags = { BLOCKS_VISION, IS_CLICKABLE },
	transparency = NO_TRANSPARENCY,
	action = "terminal",
}

obstacle {
	image_filenames = "iso_vending_machine_0001.png",
	label = "Vending Machine",
	borders = borders(1.55, 1.10),
	emitted_light_strength = { 3, 5, 7, 6, 4, 3, 3, 0, 0, 0, 3, 0  },
	flags = { BLOCKS_VISION, IS_CLICKABLE },
	transparency = NO_TRANSPARENCY,
	action = "terminal",
}

obstacle {
	image_filenames = "iso_vending_machine_0002.png",
	label = "Vending Machine",
	borders = borders(1.10, 1.55),
	emitted_light_strength = { 3, 5, 7, 6, 4, 3, 3, 0, 0, 0, 3, 0 },
	flags = { BLOCKS_VISION, IS_CLICKABLE },
	transparency = NO_TRANSPARENCY,
	action = "terminal",
}

obstacle {
	image_filenames = "iso_vending_machine_0003.png",
	label = "Vending Machine",
	borders = borders(1.55, 1.10),
	emitted_light_strength = { 3, 5, 7, 6, 4, 3, 3, 0, 0, 0, 3, 0 },
	flags = { BLOCKS_VISION, IS_CLICKABLE },
	transparency = NO_TRANSPARENCY,
	action = "terminal",
}

obstacle {
	image_filenames = "iso_vending_machine_0004.png",
	label = "Vending Machine",
	borders = borders(1.00, 1.55),
	emitted_light_strength = { 3, 5, 7, 6, 4, 3, 3, 0, 0, 0, 3, 0 },
	flags = { BLOCKS_VISION, IS_CLICKABLE },
	transparency = NO_TRANSPARENCY,
	action = "terminal",
}

obstacle {
	image_filenames = "iso_vending_machine_0005.png",
	label = "Vending Machine",
	borders = borders(1.55, 1.00),
	emitted_light_strength = { 3, 5, 7, 6, 4, 3, 3, 0, 0, 0, 3, 0 },
	flags = { BLOCKS_VISION, IS_CLICKABLE },
	transparency = NO_TRANSPARENCY,
	action = "terminal",
}

obstacle {
	image_filenames = "iso_vending_machine_0006.png",
	label = "Vending Machine",
	borders = borders(1.00, 1.55),
	emitted_light_strength = { 3, 5, 7, 6, 4, 3, 3, 0, 0, 0, 3, 0 },
	flags = { BLOCKS_VISION, IS_CLICKABLE },
	transparency = NO_TRANSPARENCY,
	action = "terminal",
}

obstacle {
	image_filenames = "iso_vending_machine_0007.png",
	label = "Vending Machine",
	borders = borders(1.55, 1.00),
	emitted_light_strength = { 3, 5, 7, 6, 4, 3, 3, 0, 0, 0, 3, 0 },
	flags = { BLOCKS_VISION, IS_CLICKABLE },
	transparency = NO_TRANSPARENCY,
	action = "terminal",
}

obstacle {
	image_filenames = "iso_vending_machine_0008.png",
	label = "Vending Machine",
	borders = borders(1.10, 1.55),
	emitted_light_strength = { 3, 5, 7, 6, 4, 3, 3, 0, 0, 0, 3, 0 },
	flags = { BLOCKS_VISION, IS_CLICKABLE },
	transparency = NO_TRANSPARENCY,
	action = "terminal",
}

obstacle {
	image_filenames = "iso_vending_machine_0009.png",
	label = "Vending Machine",
	borders = borders(1.55, 1.10),
	emitted_light_strength = { 3, 5, 7, 6, 4, 3, 3, 0, 0, 0, 3, 0 },
	flags = { BLOCKS_VISION, IS_CLICKABLE },
	transparency = NO_TRANSPARENCY,
	action = "terminal",
}

obstacle {
	image_filenames = "iso_vending_machine_0010.png",
	label = "Vending Machine",
	borders = borders(1.10, 1.55),
	emitted_light_strength = { 3, 5, 7, 6, 4, 3, 3, 0, 0, 0, 3, 0 },
	flags = { BLOCKS_VISION, IS_CLICKABLE },
	transparency = NO_TRANSPARENCY,
	action = "terminal",
}

obstacle {
	image_filenames = "iso_vending_machine_0011.png",
	label = "Vending Machine",
	borders = borders(1.55, 1.10),
	emitted_light_strength = { 3, 5, 7, 6, 4, 3, 3, 0, 0, 0, 3, 0 },
	flags = { BLOCKS_VISION, IS_CLICKABLE },
	transparency = NO_TRANSPARENCY,
	action = "terminal",
}

obstacle {
	image_filenames = "iso_transformer_0000.png",
	borders = borders(0.95, 0.95),
	flags = { BLOCKS_VISION },
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_transformer_0001.png",
	borders = borders(0.95, 0.95),
	flags = { BLOCKS_VISION },
	transparency = NO_TRANSPARENCY,
}

obstacle {
	image_filenames = "iso_transformer_0002.png",
	borders = borders(0.95, 0.95),
	flags = { BLOCKS_VISION },
	transparency = NO_TRANSPARENCY,
}
