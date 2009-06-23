#include "system.h"

#include "defs.h"
#include "struct.h"
#include "../src/global.h"
#include "proto.h"
#include "savestruct.h"

#include "mapgen/mapgen.h"
#include "lvledit/lvledit.h"

#include "lvledit/lvledit_actions.h"

static level *target_level;

void set_dungeon_output(level *output)
{
	target_level = output;
}

void mapgen_convert(int w, int h, unsigned char *tiles, int *rooms)
{
	int y, x;
	
	for (y = 0; y < h; y++) {
		for (x = 0; x < w; x++) {
			switch (tiles[y*w+x]) {
				case 4:
					action_create_obstacle(target_level, x + 0.5, y + 1, 2);
					action_set_floor(target_level, x, y, 0);
					break;
				case 5:
					action_create_obstacle(target_level, x + 0.5, y, 2);
					action_set_floor(target_level, x, y, 0);
					break;
				case 6:
					action_create_obstacle(target_level, x + 1.0, y + 0.5, 1);
					action_set_floor(target_level, x, y, 0);
					break;
				case 7:
					action_create_obstacle(target_level, x, y + 0.5, 1);
					action_set_floor(target_level, x, y, 0);
					break;
				case 11:
					action_create_obstacle(target_level, x + 0.5, y, 2);
					action_create_obstacle(target_level, x, y + 0.5, 1);
					action_set_floor(target_level, x, y, 0);
					break;
				case 10:
					action_create_obstacle(target_level, x + 0.5, y, 2);
					action_create_obstacle(target_level, x + 1, y + 0.5, 1);
					action_set_floor(target_level, x, y, 0);
					break;
				case 9:
					action_create_obstacle(target_level, x, y + 0.5, 1);
					action_create_obstacle(target_level, x + 0.5, y + 1, 2);
					action_set_floor(target_level, x, y, 0);
					break;
				case 8:
					action_create_obstacle(target_level, x + 1, y + 0.5, 1);
					action_set_floor(target_level, x, y, 0);
					action_create_obstacle(target_level, x + 0.5, y + 1, 2);
					break;

				case 12:
				case 0:
				case 1:
				case 2:
				case 3:
					action_set_floor(target_level, x, y, 0);
					break;

				case 17:
					action_set_floor(target_level, x, y, 31);
					break;

				case 16:
					action_set_floor(target_level, x, y, 56);
				    break;	

				case 15:
					action_create_obstacle(target_level, x + 1, y + 0.5, 11);
					action_set_floor(target_level, x, y, 58);
				    break;	
				
				case 14:
					action_create_obstacle(target_level, x + 0.5, y, 6);
					action_set_floor(target_level, x, y, 57);
				    break;	

				case 13:
					action_set_floor(target_level, x, y, 59);
				    break;	

				default:
					action_set_floor(target_level, x, y, tiles[y*w+x]);
			}
		}
	}

}

void mapgen_entry_at(struct roominfo *r)
{
	action_create_obstacle(target_level, r->x + r->w / 2, r->y + r->h / 2, 16);
}

void mapgen_exit_at(struct roominfo *r)
{
	action_create_obstacle(target_level, r->x + r->w / 2, r->y + r->h / 2, 16);
}

void mapgen_gift(struct roominfo *r)
{
	int obstacle_id = 50 + rand() % 4;
	int pos = rand() % 4;
	
	struct { int x; int y; } positions[4] = {
			{ r->x + 1, r->y + r->h / 2 },
			{ r->x + r->w - 1, r->y + r->h / 2},
			{ r->x + r->w / 2, r->y + 1 },
			{ r->x + r->w / 2, r->y + r->h - 1}
	};


	action_create_obstacle(target_level, positions[pos].x, positions[pos].y, obstacle_id);
}
