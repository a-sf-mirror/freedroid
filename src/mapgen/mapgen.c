#include "system.h"

#include "defs.h"
#include "struct.h"
#include "../src/global.h"
#include "proto.h"
#include "savestruct.h"

#include "mapgen/mapgen.h"

#include "lvledit/lvledit.h"

#include "lvledit/lvledit_actions.h"

static struct mapgen_gamelevel map;
static level *target_level;
static int curobstacle;

struct roominfo *rooms;
int total_rooms = 0;
int max_rooms = 0;

static void new_level(int w, int h)
{
	int x, y;
	unsigned char *map_p;
	
	map.w = w;
	map.h = h;
	
	map.m = malloc(map.w * map.h * sizeof(unsigned char));
	map.r = malloc(map.w * map.h * sizeof(int));
	map_p = map.m;
	
	for (y = 0; y < map.h; y++) {
		for (x = 0; x < map.w; x++) {
			*(map_p++) = 17;
			map.r[y*map.w+x] = -1;
		}
	}

	rooms = malloc(100 * sizeof(struct roominfo));
	max_rooms = 100;
	total_rooms = 0;
}

static void free_level()
{
	int i;

	free(map.m);
	free(map.r);

	for (i=0; i < total_rooms; i++) {
		free(rooms[i].neighbors);
	}

	free(rooms);
	total_rooms = 0;
	max_rooms = 0;
	rooms = NULL;
}

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

static void add_label(int labelnum, int posx, int posy, char *name)
{
	target_level->labels[labelnum].pos.x = posx;
	target_level->labels[labelnum].pos.y = posy;
	target_level->labels[labelnum].label_name = name;
}

static void add_teleport(int telnum, int x, int y)
{
	char *warp, *fromwarp;
	char tmp[500];
	
	sprintf(tmp, "Lv%dtoX%d", target_level->levelnum, telnum);
	warp = strdup(tmp);

	sprintf(tmp, "Lv%dfromX%d", target_level->levelnum, telnum);
	fromwarp = strdup(tmp);

	add_obstacle(x, y, 16);
	add_label(telnum*2, x, y, warp); 
	add_label(telnum*2 + 1, x + 1, y, fromwarp);
}

void mapgen_entry_at(struct roominfo *r)
{
	add_teleport(0, r->x + r->w / 2, r->y + r->h / 2, "0");
}

void mapgen_exit_at(struct roominfo *r)
{
	add_teleport(1, r->x + r->w / 2, r->y + r->h / 2, "1");
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

int mapgen_add_room(int x, int y, int w, int h)
{
	int newid = total_rooms;

	if (total_rooms == max_rooms) {
		max_rooms ++;
		rooms = realloc(rooms, max_rooms * sizeof(struct roominfo));
	}

	total_rooms ++;

	rooms[newid].x = x;
	rooms[newid].y = y;
	rooms[newid].w = w;
	rooms[newid].h = h;
	rooms[newid].next_neighbor = 0;
	rooms[newid].max_neighbors = 8;
	rooms[newid].neighbors = malloc(rooms[newid].max_neighbors * sizeof(int));

	return newid;
}

void mapgen_put_tile(int x, int y, unsigned char tile, int room)
{
	map.m[map.w*y+x] = tile;
	map.r[map.w*y+x] = room;
}

unsigned char mapgen_get_tile(int x, int y)
{
	if (x < 0) return 17;
	if (y < 0) return 17;
	if (x >= map.w) return 17;
	if (y >= map.h) return 17;
	
	return map.m[map.w*y+x];
}

int mapgen_get_room(int x, int y)
{
	if (x < 0) return -1;
	if (y < 0) return -1;
	if (x >= map.w) return -1;
	if (y >= map.h) return -1;
	
	return map.r[map.w*y+x];
}

void mapgen_draw_room(int place_x, int place_y, int room_w, int room_h, int room_id)
{
	int x, y, i;
	const unsigned char floortiles[4] = {12, 18, 19, 20};
	
	// Corners
	mapgen_put_tile(place_x, place_y, 11, room_id);
	mapgen_put_tile(place_x + room_w - 1, place_y, 10, room_id);
	mapgen_put_tile(place_x, place_y + room_h - 1, 9, room_id);
	mapgen_put_tile(place_x + room_w - 1, place_y + room_h - 1, 8, room_id);
	
	// Walls
	
	for (i = 0; i < room_w - 2; i++) {
		mapgen_put_tile(place_x + 1 + i, place_y + room_h - 1, 4, room_id);
		mapgen_put_tile(place_x + 1 + i, place_y, 5, room_id);
	}
	for (i = 0; i < room_h - 2; i++) {
		mapgen_put_tile(place_x + room_w - 1, place_y + 1 + i, 6, room_id);
		mapgen_put_tile(place_x, place_y + 1 + i, 7, room_id);
	}
	
	// Floor
	
	for (y = 0; y < room_h - 2; y++) {
		for (x = 0; x < room_w - 2; x++) {
			mapgen_put_tile(place_x + 1 + x, place_y + 1 + y, floortiles[0], room_id);
		}
	}
}

static int SuitableConnection(int t)
{
	switch (t) {
		case 4:
		case 5:
		case 6:
		case 7:
			return 1;
			break;
			
		default:
			break;
	}
	return 0;
}

/** Find the possible connections at each square on the border of the
  given room. 
  Fill out the struct cplist_t array and return the number of possible
  connections.
  */
int find_connection_points(int room_id, struct cplist_t cplist[100])
{
	// Find connection points
	int connect_points = 0;
	int i;

	struct roominfo *r = &rooms[room_id];

	for (i = 0; i < r->w - 2; i++) {
		if (SuitableConnection(mapgen_get_tile(r->x + 1 + i, r->y - 1))) {
			cplist[connect_points].x = r->x + 1 + i;
			cplist[connect_points].y = r->y;
			cplist[connect_points].r = mapgen_get_room(r->x + 1 + i, r->y - 1);
			cplist[connect_points].t = UP;
			connect_points++;
		}
		
		if (SuitableConnection(mapgen_get_tile(r->x + 1 + i, r->y + r->h))) {
			cplist[connect_points].x = r->x + 1 + i;
			cplist[connect_points].y = r->y + r->h - 1;
			cplist[connect_points].r = mapgen_get_room(r->x + 1 + i, r->y + r->h);
			cplist[connect_points].t = DOWN;
			connect_points++;
		}
	}
	for (i = 0; i < r->h - 2; i++) {
		if (SuitableConnection(mapgen_get_tile(r->x - 1, r->y + 1 + i))) {
			cplist[connect_points].x = r->x;
			cplist[connect_points].y = r->y + 1 + i;
			cplist[connect_points].r = mapgen_get_room(r->x - 1, r->y + 1 + i);
			cplist[connect_points].t = LEFT;
			connect_points++;
		}
		
		if (SuitableConnection(mapgen_get_tile(r->x + r->w, r->y + 1 + i))) {
			cplist[connect_points].x = r->x + r->w - 1;
			cplist[connect_points].y = r->y + 1 + i;
			cplist[connect_points].r = mapgen_get_room(r->x + r->w, r->y + 1 + i);
			cplist[connect_points].t = RIGHT;
			connect_points++;
		}
	}
	
	return connect_points;
}

static void recursive_browse(int at, int parent, unsigned char *seen)
{
	int i;

	// If the room has already been seen, return immediately
	if (seen[at])
		return;

	seen[at] = 1;

	for (i = 0; i < rooms[at].next_neighbor; i++) {
		// Don't recurse into our parent
		if (rooms[at].neighbors[i] == parent)
			continue;

		recursive_browse(rooms[at].neighbors[i], at, seen);
	}
}

int mapgen_is_connected(unsigned char *seen)
{
	int i;
	memset(seen, 0, total_rooms);

	recursive_browse(0, 0, seen);

	for (i = 0; i < total_rooms; i++) {
		if (seen[i] == 0) {
			return 0;
		}
	}

	return 1;
}

int mapgen_are_connected(int room1, int room2)
{
	int i;
	struct roominfo *r1 = &rooms[room1];

	for (i = 0; i < r1->next_neighbor; i++) {
		if (r1->neighbors[i] == room2)
			return 1;
	}

	return 0;
}

static void add_neighbor(struct roominfo *r, int neigh)
{
	int newid = r->next_neighbor;

	if (r->next_neighbor >= r->max_neighbors) {
		r->max_neighbors *= 2;
		r->neighbors = realloc(r->neighbors, r->max_neighbors * sizeof(int));
	}

	r->next_neighbor ++;

	r->neighbors[newid] = neigh;	
}

void MakeConnect(int x, int y, enum connection_type type)
{
	int nx, ny;
	int wp_x, wp_y, wp_nx, wp_ny;
	int d1 = 0, d2 = 0;
	int room_1, room_2;

    nx = x;
	ny = y;
	wp_x = x;
	wp_nx = nx;
	wp_y = y;
	wp_ny = ny;
	
	switch (type) {
		case UP:
			ny = y - 1;
			wp_ny = ny - 1;
			wp_y = y + 1;
			d1 = 14;
			d2 = 13;
			break;
		case DOWN:
			ny = y + 1;
			wp_ny = ny + 1;
			wp_y = y - 1;
			d1 = 13;
			d2 = 14;
			break;
		case LEFT:
			nx = x - 1;
			wp_nx = nx - 1;
			wp_x = x + 1;
			d1 = 16;
			d2 = 15;
			break;
		case RIGHT:
			nx = x + 1;
			wp_nx = nx + 1;
			wp_x = x - 1;
			d1 = 15;
			d2 = 16;
			break;
		default:
			ErrorMessage(__FUNCTION__, "Unknown connection type %d\n", PLEASE_INFORM, IS_FATAL, type);
			break;
	}
	
	room_1 = mapgen_get_room(x, y);
	room_2 = mapgen_get_room(nx, ny);
	
	mapgen_put_tile(x, y, d1, room_1);
	mapgen_put_tile(nx, ny, d2, room_2);

	add_neighbor(&rooms[room_1], room_2);	
	add_neighbor(&rooms[room_2], room_1);	

	int useless;
	int wp1 = CreateWaypoint(target_level, wp_x, wp_y, &useless);
	int wp2 = CreateWaypoint(target_level, wp_nx, wp_ny, &useless);

	action_toggle_waypoint_connection(target_level, wp1, wp2, 0);
	action_toggle_waypoint_connection(target_level, wp2, wp1, 0);
}

int generate_dungeon(int w, int h, int nbexits, int difficulty)
{
	int i;
	
	new_level(w, h);

	generate_dungeon_gram(w, h);

	// Select entrance at random.
	int entrance = rand()%total_rooms;
	mapgen_entry_at(&rooms[entrance]);

	// Select random exits
	int exit_points[nbexits];
	for (i=0; i<nbexits; i++) {
		int done;
		do {
			done = 1;
			exit_points[i] = rand() % total_rooms;

			int j = i;

			while (j--) {
				if (exit_points[j] == exit_points[i])
					done = 0;
			}

			if (entrance == exit_points[i])
				done = 0;
		} while (!done);
	
		mapgen_exit_at(&rooms[exit_points[i]]);
	}


	mapgen_convert(w, h, map.m, map.r);
	free_level();
	return 0;
}



