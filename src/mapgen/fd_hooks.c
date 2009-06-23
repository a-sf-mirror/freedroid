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
static int curobstacle;

void set_dungeon_output(level *output)
{
	target_level = output;
	curobstacle = 0;
}

static void add_obstacle(double x, double y, int type)
{
	if (curobstacle >= MAX_OBSTACLES_ON_MAP) {
		ErrorMessage(__FUNCTION__, "Too many obstacles on random dungeon level %d\n", PLEASE_INFORM, IS_FATAL, target_level->levelnum);
	}

	target_level->obstacle_list[curobstacle].type = type;
	target_level->obstacle_list[curobstacle].pos.x = x;
	target_level->obstacle_list[curobstacle].pos.y = y;
	target_level->obstacle_list[curobstacle].pos.z = target_level->levelnum;
	target_level->obstacle_list[curobstacle++].name_index = -1;
}

static void set_floor(int x, int y, int type)
{
    target_level->map[y][x].floor_value = type;
}

void mapgen_convert(int w, int h, unsigned char *tiles, int *rooms)
{
	int y, x;
	
	for (y = 0; y < h; y++) {
		for (x = 0; x < w; x++) {
			switch (tiles[y*w+x]) {
				case 4:
					add_obstacle(x + 0.5, y + 1, 2);
					set_floor(x, y, 0);
					break;
				case 5:
					add_obstacle(x + 0.5, y, 2);
					set_floor(x, y, 0);
					break;
				case 6:
					add_obstacle(x + 1.0, y + 0.5, 1);
					set_floor(x, y, 0);
					break;
				case 7:
					add_obstacle(x, y + 0.5, 1);
					set_floor(x, y, 0);
					break;
				case 11:
					add_obstacle(x + 0.5, y, 2);
					add_obstacle(x, y + 0.5, 1);
					set_floor(x, y, 0);
					break;
				case 10:
					add_obstacle(x + 0.5, y, 2);
					add_obstacle(x + 1, y + 0.5, 1);
					set_floor(x, y, 0);
					break;
				case 9:
					add_obstacle(x, y + 0.5, 1);
					add_obstacle(x + 0.5, y + 1, 2);
					set_floor(x, y, 0);
					break;
				case 8:
					add_obstacle(x + 1, y + 0.5, 1);
					set_floor(x, y, 0);
					add_obstacle(x + 0.5, y + 1, 2);
					break;

				case 12:
				case 0:
				case 1:
				case 2:
				case 3:
					set_floor(x, y, 0);
					break;

				case 17:
					set_floor(x, y, 31);
					break;

				case 16:
					set_floor(x, y, 56);
				    break;	

				case 15:
					add_obstacle(x + 1, y + 0.5, 11);
					set_floor(x, y, 58);
				    break;	
				
				case 14:
					add_obstacle(x + 0.5, y, 6);
					set_floor(x, y, 57);
				    break;	

				case 13:
					set_floor(x, y, 59);
				    break;	

				default:
					set_floor(x, y, tiles[y*w+x]);
			}
		}
	}

}

void mapgen_entry_at(struct roominfo *r)
{
	add_obstacle( r->x + r->w / 2, r->y + r->h / 2, 16);
}

void mapgen_exit_at(struct roominfo *r)
{
	add_obstacle( r->x + r->w / 2, r->y + r->h / 2, 16);
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


	add_obstacle( positions[pos].x, positions[pos].y, obstacle_id);
}
