#include "system.h"

#include "defs.h"
#include "struct.h"
#include "../src/global.h"
#include "proto.h"
#include "savestruct.h"

#include "mapgen/mapgen.h"
#include "mapgen/themes.h"

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
			*(map_p++) = TILE_EMPTY;
			map.r[y * map.w + x] = -1;
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

	for (i = 0; i < total_rooms; i++) {
		free(rooms[i].neighbors);
	}

	free(rooms);
	total_rooms = 0;
	max_rooms = 0;
	rooms = NULL;
}

void set_dungeon_output(level * output)
{
	target_level = output;
	curobstacle = 0;
}

void mapgen_add_obstacle(double x, double y, int type)
{
	if (curobstacle >= MAX_OBSTACLES_ON_MAP) {
		ErrorMessage(__FUNCTION__, "Too many obstacles on random dungeon level %d\n", PLEASE_INFORM, IS_FATAL,
			     target_level->levelnum);
	}

	target_level->obstacle_list[curobstacle].type = type;
	target_level->obstacle_list[curobstacle].pos.x = x;
	target_level->obstacle_list[curobstacle].pos.y = y;
	target_level->obstacle_list[curobstacle].pos.z = target_level->levelnum;
	curobstacle++;
}

void mapgen_set_floor(int x, int y, int type)
{
	target_level->map[y][x].floor_value = type;
}

static void split_wall(int w, int h, unsigned char *tiles) 
{
	int y, x; 
	int tile, room;
#define SET(X,Y,TILE) mapgen_put_tile(X, Y, TILE, room)

	for (y = 1; y < h - 1; y++) 
		for (x = 1; x < w - 1; x++) {
			tile = tiles[y * w + x];
			room = mapgen_get_room(x, y);
			switch(tile) {
				case TILE_WALL_L:
					SET(x - 1, y, TILE_WALL);
					SET(x    , y, TILE_WALL);
					break;
				case TILE_WALL_T:
					SET(x, y - 1, TILE_WALL);
					SET(x, y    , TILE_WALL);
					break;
				case TILE_WALL:
					SET(x - 1, y    , TILE_WALL);
					SET(x    , y - 1, TILE_WALL);
					SET(x - 1, y - 1, TILE_WALL);
					break;
				case TILE_DOOR_H:
					SET(x - 1, y    , TILE_FLOOR);
					SET(x    , y - 1, TILE_FLOOR);
					SET(x - 1, y - 1, TILE_FLOOR);
					SET(x + 1, y - 1, TILE_FLOOR);
					SET(x + 1, y    , TILE_FLOOR); 
					SET(x + 2, y    , TILE_WALL_T);
					break;
				case TILE_DOOR_V:
					SET(x    , y - 1, TILE_FLOOR);
					SET(x - 1, y    , TILE_FLOOR);
					SET(x    , y + 1, TILE_FLOOR);
					SET(x - 1, y + 1, TILE_FLOOR);
					SET(x - 1, y - 1, TILE_FLOOR);
					SET(x    , y + 2, TILE_WALL_L);
					break;
				case TILE_DOOR_H2:
					SET(x - 1, y    , TILE_DOOR_H2);
					SET(x    , y - 1, TILE_FLOOR);
					SET(x - 1, y - 1, TILE_FLOOR);
					SET(x    , y    , TILE_FLOOR);
					SET(x + 1, y    , TILE_WALL_T);
					break;
				case TILE_DOOR_V2:
					SET(x    , y - 1, TILE_DOOR_V2);
					SET(x    , y + 1, TILE_FLOOR);
					SET(x - 1, y - 1, TILE_FLOOR);
					SET(x    , y    , TILE_FLOOR);
					SET(x    , y + 1, TILE_WALL_L);
					break;
			}
		}
}

// Randomly reduce size of rooms. We go alongside the
// room border and check whether we can shift the current wall
// tile closer to the center of rooms. Therefore the room size
// becomes smaller and free space between rooms increases.
static void reduce_room_space() {
	int x, y;
	int i, j, k, tile;
	int flag, bevel;
	int point_x[4];
	int point_y[4];
	const int point_dx[4] = { 1,  0, -1,  0};
	const int point_dy[4] = { 0,  1,  0, -1};
	for (i = 0; i < total_rooms; i++) {
		if (rooms[i].w < 4 || rooms[i].h < 4)
			continue;

		// 4 start points, one per room corner
		point_x[0] = rooms[i].x;
		point_y[0] = rooms[i].y - 1;

		point_x[1] = rooms[i].x - 1;
		point_y[1] = rooms[i].y; 

		point_x[2] = rooms[i].x + rooms[i].w;
		point_y[2] = rooms[i].y + rooms[i].h - 1;

		point_x[3] = rooms[i].x + rooms[i].w - 1;
		point_y[3] = rooms[i].y + rooms[i].h;

		for (j = 0; j < 4; j++) {
			x = point_x[j];
			y = point_y[j];
			// Inrease(probably by 1) amount of cells that should not be changed near a door
			bevel = MyRandom(2);
			if (MyRandom(100) < 25) {
				// Go alonside room border and fill neighbour cell with a wall tile
				while (labs(x - point_x[j]) < rooms[i].w && labs(y - point_y[j]) < rooms[i].h) {
					if (mapgen_get_tile(x, y) == TILE_WALL) {
						flag = 1;
						for (k = -2 - bevel; k <= 1 + bevel; k++) {
							tile = mapgen_get_tile(x + k*point_dx[j], y + k*point_dy[j]);
							// Shouldn't place wall if there is a door somewhere near
							if (tile == TILE_DOOR_H || tile == TILE_DOOR_V || tile == TILE_DOOR_H2 || tile == TILE_DOOR_V2) {
								flag = 0;
								break;
							}
						}
						if (flag) 
							mapgen_put_tile(x + point_dy[j], y + point_dx[j], TILE_WALL, -1);
					}
					x += point_dx[j];
					y += point_dy[j];
				}
			}
		}
	}
}

void mapgen_convert(int w, int h, unsigned char *tiles)
{
	reduce_room_space();
	split_wall(w, h, tiles);
	mapgen_place_obstacles(w, h, tiles);
}

static void add_teleport(int telnum, int x, int y)
{
	char *warp, *fromwarp;
	char tmp[500];

	sprintf(tmp, "%dtoX%d", target_level->levelnum, telnum);
	warp = strdup(tmp);

	sprintf(tmp, "%dfromX%d", target_level->levelnum, telnum);
	fromwarp = strdup(tmp);

	mapgen_add_obstacle(x, y, ISO_TELEPORTER_1);
	add_map_label(target_level, x, y, warp);
	add_map_label(target_level, x + 1, y, fromwarp);
}

void mapgen_entry_at(struct roominfo *r)
{
	add_teleport(0, r->x + r->w / 2, r->y + r->h / 2);
}

void mapgen_exit_at(struct roominfo *r)
{
	add_teleport(1, r->x + r->w / 2, r->y + r->h / 2);
}

void mapgen_gift(struct roominfo *r)
{
	int obstacle_id = ISO_BARREL_1 + rand() % 4;
	int pos = rand() % 4;

	struct {
		int x;
		int y;
	} positions[4] = {
		{
		r->x + 1, r->y + r->h / 2}, {
		r->x + r->w - 1, r->y + r->h / 2}, {
		r->x + r->w / 2, r->y + 1}, {
		r->x + r->w / 2, r->y + r->h - 1}
	};

	mapgen_add_obstacle(positions[pos].x, positions[pos].y, obstacle_id);
}

int mapgen_add_room(int x, int y, int w, int h)
{
	int newid = total_rooms;

	if (total_rooms == max_rooms) {
		max_rooms++;
		rooms = realloc(rooms, max_rooms * sizeof(struct roominfo));
	}

	total_rooms++;

	// don't forget to reserve space for bounding walls
	rooms[newid].x = x + 1;
	rooms[newid].y = y + 1;
	rooms[newid].w = w - 2;
	rooms[newid].h = h - 2;
	rooms[newid].num_neighbors = 0;
	rooms[newid].max_neighbors = 8;
	rooms[newid].neighbors = malloc(rooms[newid].max_neighbors * sizeof(int));

	return newid;
}

void mapgen_put_tile(int x, int y, unsigned char tile, int room)
{
	map.m[map.w * y + x] = tile;
	map.r[map.w * y + x] = room;
}

unsigned char mapgen_get_tile(int x, int y)
{
	if (x < 0)
		return TILE_EMPTY;
	if (y < 0)
		return TILE_EMPTY;
	if (x >= map.w)
		return TILE_EMPTY;
	if (y >= map.h)
		return TILE_EMPTY;

	return map.m[map.w * y + x];
}

int mapgen_get_room(int x, int y)
{
	if (x < 0)
		return -1;
	if (y < 0)
		return -1;
	if (x >= map.w)
		return -1;
	if (y >= map.h)
		return -1;

	return map.r[map.w * y + x];
}

void mapgen_draw_room(int room_id)
{
	int place_x = rooms[room_id].x - 1;
  	int	place_y = rooms[room_id].y - 1;
	int room_w = rooms[room_id].w + 1;
	int room_h = rooms[room_id].h + 1;
	int x, y, i;

	// Corners
	mapgen_put_tile(place_x, place_y, TILE_WALL, -1);
	mapgen_put_tile(place_x + room_w, place_y, TILE_WALL, -1);
	mapgen_put_tile(place_x, place_y + room_h, TILE_WALL, -1);
	mapgen_put_tile(place_x + room_w, place_y + room_h, TILE_WALL, -1);

	// Walls 
	for (i = 1; i < room_w; i++) {
		mapgen_put_tile(place_x + i, place_y + room_h, TILE_WALL, -1);
		mapgen_put_tile(place_x + i, place_y, TILE_WALL, -1);
	}
	for (i = 1; i < room_h; i++) {
		mapgen_put_tile(place_x + room_w, place_y + i, TILE_WALL, -1);
		mapgen_put_tile(place_x, place_y + i, TILE_WALL, -1);
	}

	// Floor 
	for (y = 1; y < room_h; y++)
		for (x = 1; x < room_w; x++)
			mapgen_put_tile(place_x + x, place_y + y, TILE_FLOOR, room_id);
}

// Check if the given cell is suitable for connections. Condition of success is that 
// the current cell as well as 'offset' adjacent cells are free.
static int SuitableConnection(int x, int y, enum connection_type t, int offset)
{
	int i;
	const int dx[] = { -1, 1,  0, 0};
	const int dy[] = {  0, 0, -1, 1};
	if (mapgen_get_room(x, y) == -1)
		return 0;
	for (i = -offset; i <= offset; i++) {
		if (mapgen_get_tile(x + i * dx[t], y + i * dy[t]) != TILE_FLOOR)
			return 0;
	}
	return 1;
}

/** Find the possible connections at each square on the border of the
  given room. 
  Fill out the struct cplist_t array and return the number of possible
  connections.
  */
int find_connection_points(int room_id, struct cplist_t cplist[100], int offset)
{
	// Find connection points
	int connect_points = 0;
	int i;

	struct roominfo *r = &rooms[room_id];

	for (i = offset; i < r->w - offset; i++) {
		if (SuitableConnection(r->x + i, r->y - 2, UP, offset)) {
			cplist[connect_points].x = r->x + i;
			cplist[connect_points].y = r->y - 1;
			cplist[connect_points].r = mapgen_get_room(r->x + i, r->y - 2);
			cplist[connect_points].t = UP;
			connect_points++;
		}

		if (SuitableConnection(r->x + i, r->y + r->h + 1, DOWN, offset)) {
			cplist[connect_points].x = r->x + i;
			cplist[connect_points].y = r->y + r->h;
			cplist[connect_points].r = mapgen_get_room(r->x + i, r->y + r->h + 1);
			cplist[connect_points].t = DOWN;
			connect_points++;
		}
	}
	for (i = offset; i < r->h - offset; i++) {
		if (SuitableConnection(r->x - 2, r->y + i, LEFT, offset)) {
			cplist[connect_points].x = r->x - 1;
			cplist[connect_points].y = r->y + i;
			cplist[connect_points].r = mapgen_get_room(r->x - 2, r->y + i);
			cplist[connect_points].t = LEFT;
			connect_points++;
		}

		if (SuitableConnection(r->x + r->w + 1, r->y + i, RIGHT, offset)) {
			cplist[connect_points].x = r->x + r->w;
			cplist[connect_points].y = r->y + i;
			cplist[connect_points].r = mapgen_get_room(r->x + r->w + 1, r->y + i);
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

	for (i = 0; i < rooms[at].num_neighbors; i++) {
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

	for (i = 0; i < r1->num_neighbors; i++) {
		if (r1->neighbors[i] == room2)
			return 1;
	}

	return 0;
}

static void add_neighbor(struct roominfo *r, int neigh)
{
	int newid = r->num_neighbors;

	if (r->num_neighbors >= r->max_neighbors) {
		r->max_neighbors *= 2;
		r->neighbors = realloc(r->neighbors, r->max_neighbors * sizeof(int));
	}

	r->num_neighbors++;

	r->neighbors[newid] = neigh;
}

void MakeConnect(int x, int y, enum connection_type type)
{
	int wp_x, wp_y, wp_nx, wp_ny;
	int room_1, room_2;
	int tile = 0;

	wp_x = wp_nx = x;
	wp_y = wp_ny = y;

	switch (type) {
		case UP:
			wp_ny = y - 1;
			wp_y = y + 1;
			tile = (rand() % 2) ? TILE_DOOR_H : TILE_DOOR_H2;
			break;
		case DOWN:
			wp_ny = y + 1;
			wp_y = y - 1;
			tile = (rand() % 2) ? TILE_DOOR_H : TILE_DOOR_H2;
			break;
		case LEFT:
			wp_nx = x - 1;
			wp_x = x + 1;
			tile = (rand() % 2) ? TILE_DOOR_V : TILE_DOOR_V2;
			break;
		case RIGHT:
			wp_nx = x + 1;
			wp_x = x - 1;
			tile = (rand() % 2) ? TILE_DOOR_V : TILE_DOOR_V2;
			break;
		default:
			ErrorMessage(__FUNCTION__, "Unknown connection type %d\n", PLEASE_INFORM, IS_FATAL, type);
			break;

	} 

	room_1 = mapgen_get_room(wp_nx, wp_ny);
	room_2 = mapgen_get_room(wp_x, wp_y);
	mapgen_put_tile(x, y, tile, room_2);
	add_neighbor(&rooms[room_1], room_2);
	add_neighbor(&rooms[room_2], room_1);

	int useless;
	int wp1 = CreateWaypoint(target_level, wp_x, wp_y, &useless);
	int wp2 = CreateWaypoint(target_level, wp_nx, wp_ny, &useless);

	action_toggle_waypoint_connection(target_level, wp1, wp2, 0, 0);
	action_toggle_waypoint_connection(target_level, wp2, wp1, 0, 0);
}

static int find_waypoints(int x1, int y1, int x2, int y2, int *wps, int max)
{
	waypoint *wpts = target_level->waypoints.arr;
	int total_wps = 0;
	int i;

	for (i = 0; i < target_level->waypoints.size; i++) {
		if (wpts[i].x >= x1 && wpts[i].x < x2 && wpts[i].y >= y1 && wpts[i].y < y2) {
			wps[total_wps] = i;
			total_wps++;

			if (total_wps == max)
				break;
		}
	}

	return total_wps;
}

static void connect_waypoints()
{
	int rn;

	for (rn = 0; rn < total_rooms; rn++) {
		int wps[25];
		int max_wps = find_waypoints(rooms[rn].x, rooms[rn].y, rooms[rn].x + rooms[rn].w, rooms[rn].y + rooms[rn].h, wps, 25);
		int nbconn = max_wps;

		if (max_wps == 1 || max_wps == 0)
			continue;

		while (nbconn--) {
			int wp1 = nbconn;
			int wp2 = rand() % max_wps;

			while (wp2 == wp1)
				wp2 = rand() % max_wps;

			if (wp1 != wp2) {
				action_toggle_waypoint_connection(target_level, wps[wp1], wps[wp2], 0, 0);
				action_toggle_waypoint_connection(target_level, wps[wp2], wps[wp1], 0, 0);
			}
		}
	}
}

static void place_waypoints()
{
	int rn;
	int nb;

	for (rn = 0; rn < total_rooms; rn++) {
		int func = sqrt(rooms[rn].w * rooms[rn].h);

		nb = -1 + func / 3;

		while ((nb--) > 0) {
			int newx = rooms[rn].x + 1;
			int newy = rooms[rn].y + 1;
			newx += (rand() % (rooms[rn].w - 2));
			newy += (rand() % (rooms[rn].h - 2));

			int useless;

			CreateWaypoint(target_level, newx, newy, &useless);
		}
	}
}

int generate_dungeon(int w, int h, int nbconnec)
{
	int i;

	new_level(w, h);

	generate_dungeon_gram(w, h);

	// Select entrance at random.
	int entrance = rand() % total_rooms;
	mapgen_entry_at(&rooms[entrance]);

	// Select random exits
	int exit_points[nbconnec - 1];
	for (i = 0; i < nbconnec - 1; i++) {
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

	mapgen_convert(w, h, map.m);

	// Place random waypoints
	place_waypoints();

	// Connect waypoints
	connect_waypoints();

	free_level();
	return 0;
}
