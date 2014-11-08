#include "system.h"

#include "defs.h"
#include "struct.h"
#include "../src/global.h"
#include "proto.h"

#include "mapgen/mapgen.h"
#include "mapgen/themes.h"

#include "lvledit/lvledit.h"

#include "lvledit/lvledit_actions.h"

#define		SET_PILLAR_PROB		70

static struct mapgen_gamelevel map;
static level *target_level;

struct roominfo *rooms;
int total_rooms = 0;
int max_rooms = 0;

const struct { int enter, exit; } teleport_pairs[] = {
	{ ISO_TELEPORTER_1, ISO_TELEPORTER_1},	// enter: cloud, exit: cloud
	{ ISO_TELEPORTER_1, ISO_EXIT_5 },		// enter: cloud, exit: ladder to upstairs
	{ ISO_EXIT_3, ISO_TELEPORTER_1 },		// enter: ladder to downstairs, exit: cloud
	{ ISO_EXIT_3, ISO_EXIT_5}				// enter: ladder to downstairs, exit: ladder to upstairs
};

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
}

void mapgen_add_obstacle(double x, double y, int type)
{
	add_obstacle(target_level, x, y, type);
}

void mapgen_set_floor(int x, int y, int type)
{
	target_level->map[y][x].floor_values[0] = type;
}

static void split_wall(int w, int h, unsigned char *tiles) 
{
	int y, x; 
	int room;
#define SET(X,Y,TILE) mapgen_put_tile(X, Y, TILE, room)

	// Reduce the size of the rooms, not lying on the map's boundary.
	for (x = 0; x < total_rooms; x++) {
		if (rooms[x].x + rooms[x].w < w - 1 )
			rooms[x].w--;
		if (rooms[x].y + rooms[x].h < h - 1)
			rooms[x].h--;
	}

	for (y = 1; y < h - 1; y++) 
		for (x = 1; x < w - 1; x++) {
			room = mapgen_get_room(x, y);
			if (tiles[y * w + x] == TILE_WALL) {
				SET(x - 1, y    , TILE_WALL);
				SET(x    , y - 1, TILE_WALL);
				SET(x - 1, y - 1, TILE_WALL);
			}
		}
}

// Randomly reduce size of rooms. We go alongside the
// room border and check whether we can shift the current wall
// tile closer to the center of rooms. Therefore the room size
// becomes smaller and free space between rooms increases.
static void reduce_room_space() {
	int x, y;
	int i, j;
	int point_x[4];
	int point_y[4];
	int count[4];
	const int point_dx[4] = { 1,  0, -1,  0};
	const int point_dy[4] = { 0,  1,  0, -1};
	const int check_dir[4] = { 3, 1, 2, 0 };
	for (i = 0; i < total_rooms; i++) {
		if (rooms[i].w < 4 || rooms[i].h < 4)
			continue;

		// 4 start points, one per room corner
		point_x[0] = rooms[i].x;
		point_y[0] = rooms[i].y;
		count[0] = rooms[i].w;

		point_x[1] = rooms[i].x + rooms[i].w - 1;
		point_y[1] = rooms[i].y; 
		count[1] = rooms[i].h;

		point_x[2] = rooms[i].x + rooms[i].w - 1;
		point_y[2] = rooms[i].y + rooms[i].h - 1;
		count[2] = rooms[i].w;

		point_x[3] = rooms[i].x;
		point_y[3] = rooms[i].y + rooms[i].h - 1;
		count[3] = rooms[i].h;

		for (j = 0; j < 4; j++) {
			if (MyRandom(100) > 40)
				continue;

			x = point_x[j];
			y = point_y[j];
			if (mapgen_get_tile(x + point_dx[check_dir[j]], y + point_dy[check_dir[j]]) != TILE_WALL)
				continue;

			switch(j) {
				case 0:
					rooms[i].y++;
					rooms[i].h--;
					break;
				case 1:
					rooms[i].w--;
					break;
				case 2:
					rooms[i].h--;
					break;
				case 3:
					rooms[i].x++;
					rooms[i].w--;
					break;
			}
			while (count[j]--) {
				mapgen_put_tile(x, y, TILE_WALL, -1);
				x +=  point_dx[j];
				y +=  point_dy[j];
			}
			// If the given room took part in fusion there may be additional tile ahead
			// that should be removed
			if (mapgen_get_tile(x, y) == TILE_PARTITION)
				mapgen_put_tile(x, y, TILE_WALL, -1);
		}
	}
}

static void place_internal_door(int x, int y, enum connection_type dir)
{
	const int check_horz[] = { 0, -1 };
	const int check_vert[] = { -1, 0 };
	const float dx_horz[] = { 0.5, 0.5 };
	const float dy_horz[] = {   0,   1 };
	const float dx_vert[] = {   0,   1 };
	const float dy_vert[] = { 0.5, 0.5 };
	const int *check;
	const float *dx, *dy;
	int room, door;

	if (dir == UP || dir == DOWN) {
		check = check_horz;
		dx = dx_horz;
		dy = dy_horz;
		door = ISO_H_DOOR_000_OPEN;
	} else {
		check = check_vert;
		dx = dx_vert;
		dy = dy_vert;
		door = ISO_V_DOOR_000_OPEN;
	}
	room = mapgen_get_room(x, y);
	mapgen_put_tile(x, y, TILE_FLOOR, room);
	if (mapgen_get_room(x + check[0], y + check[1]) != room)
		mapgen_add_obstacle(x + dx[0], y + dy[0], door);
	else
		mapgen_add_obstacle(x + dx[1], y + dy[1], door);
}

static void place_doors()
{
	int i, j, room;
	int x, y;
	int w;
	int x1, x2, y1, y2;
	enum connection_type dir;
	int id, id1, id2;
	int place_door;

	for (i = 0; i < total_rooms; i++) {
		for (j = 0; j < rooms[i].num_doors; j++) {
			x = rooms[i].doors[j].x;
			y = rooms[i].doors[j].y;
			room = rooms[i].doors[j].room;
			w = 2;
			id1 = -1;
			id2 = -1;
			place_door = 1;
			// Place door only if both rooms have its side length greater then 2
			// If one of the rooms is a corridor then whole doorway should belong to it
			if (rooms[i].w < 3 || rooms[i].h < 3) {
				id1 = i;
				id2 = i;
				place_door = 0;
			}
			if (rooms[room].w < 3 || rooms[room].h < 3) {
				id1 = room;
				id2 = room;
				place_door = 0;
			}

			if (rooms[i].x < x - 1 && x - 1 <= (rooms[i].x + rooms[i].w - 1)) {
				// Place horizontal wall

				// Set single horizontal door if the wall is internal
				if (mapgen_get_tile(x, y) == TILE_PARTITION) {
					place_internal_door(x, y, UP);
					continue;
				}

				// Shift to left by 1 due to split done
				x1 = x - w;
				x2 = x;
				if (y < rooms[i].y) {
					y1 = rooms[room].y + rooms[room].h;
					y2 = rooms[i].y;
					dir = UP;
					if (id1 == -1) {
						id1 = room;
						id2 = i;
					}
				} else {
					y1 = rooms[i].y + rooms[i].h;
					y2 = rooms[room].y;
					dir = DOWN;
					if (id1 == -1) {
						id1 = i;
						id2 = room;
					}
				}
				if (place_door)
					mapgen_add_obstacle(x1 + 0.5, (y1 + y2) / 2.0, ISO_DH_DOOR_000_OPEN);

				if (MyRandom(100) < SET_PILLAR_PROB &&  y2 - y1 > 2) {
					if (rooms[i].x <= x - w - 1 && rooms[i].x + rooms[i].w > x &&
						rooms[room].x <= x - w - 1 && rooms[room].x + rooms[room].w > x) {
						mapgen_put_tile(x1-1, y1, TILE_FLOOR, id1);
						mapgen_add_obstacle(x1 - 0.5, y1 + 0.5, ISO_PILLAR_SHORT);

						mapgen_put_tile(x1 + w, y1, TILE_FLOOR, id1);
						mapgen_add_obstacle(x1 + w + 0.5, y1 + 0.5, ISO_PILLAR_SHORT);

						mapgen_put_tile(x1-1, y2 - 1, TILE_FLOOR, id2);
						mapgen_add_obstacle(x1 - 0.5, y2 - 0.5, ISO_PILLAR_SHORT);

						mapgen_put_tile(x1 + w, y2 - 1, TILE_FLOOR, id2);
						mapgen_add_obstacle(x1 + w + 0.5, y2 - 0.5, ISO_PILLAR_SHORT);
					}
				} 
			} else {
				// Place vertical wall

				// Set signle vertical door if the wall is internal
				if (mapgen_get_tile(x, y) == TILE_PARTITION) {
					place_internal_door(x, y, LEFT);
					continue;
				}
				// Shift to top by 1 due to split done
				y1 = y - w;
				y2 = y;
				if (x < rooms[i].x) {
					x1 = rooms[room].x + rooms[room].w;
					x2 = rooms[i].x;
					dir = LEFT;
					if (id1 == -1) {
						id1 = room;
						id2 = i;
					}
				} else {
					x1 = rooms[i].x + rooms[i].w;
					x2 = rooms[room].x;
					dir = RIGHT;
					if (id1 == -1) {
						id1 = i;
						id2 = room;
					}
				}
				if (place_door)
					mapgen_add_obstacle((x1 + x2) / 2.0, y1 + 0.5, ISO_DV_DOOR_000_OPEN);

				if (MyRandom(100) < SET_PILLAR_PROB &&  x2 - x1 > 2) {
					if (rooms[i].y <= y - w - 1 && rooms[i].y + rooms[i].h > y &&
						rooms[room].y <= y - w - 1 && rooms[room].y + rooms[room].h > y) {
						mapgen_put_tile(x1, y1 - 1, TILE_FLOOR, id1);
						mapgen_add_obstacle(x1 + 0.5, y1 - 0.5, ISO_PILLAR_SHORT);

						mapgen_put_tile(x1, y1 + w, TILE_FLOOR, id2);
						mapgen_add_obstacle(x1 + 0.5, y1 + w + 0.5, ISO_PILLAR_SHORT);

						mapgen_put_tile(x2 - 1, y1 - 1, TILE_FLOOR, id1);
						mapgen_add_obstacle(x2 - 0.5, y1 - 0.5, ISO_PILLAR_SHORT);

						mapgen_put_tile(x2 - 1, y1 + w, TILE_FLOOR, id2);
						mapgen_add_obstacle(x2 - 0.5, y1 + w + 0.5, ISO_PILLAR_SHORT);
					}
				}
			}

			for (y = y1; y < y2; y++) {
				for (x = x1; x < x2; x++) {
					// New tiles are divided between rooms equally
					if (dir == LEFT || dir == RIGHT)
						id = x < (x1 + x2) / 2 ? id1 : id2;
					else
						id = y < (y1 + y2) / 2 ? id1 : id2;
					mapgen_put_tile(x, y, TILE_FLOOR, id);
				}
			}
		}
	}
}

// Turn the given room into corridor and return whether
// the transformation was successful
static int make_corridor(int room)
{
#define	MAX_DOORS_IN_CORRIDOR	3
	struct doorinfo doors[3];
	int num_doors = 0;
	int i, j;
	int x1 = rooms[room].x;
	int y1 = rooms[room].y;
	int x2 = x1 + rooms[room].w - 1;
	int y2 = y1 + rooms[room].h - 1;
	int xmin = x2 + 1;
	int ymin = y2 + 1;
	int xmax = x1;
	int ymax = y1;

	if (rooms[room].num_doors > MAX_DOORS_IN_CORRIDOR)
		return 0;


	for (i = 0; i < rooms[room].num_doors; i++) {
		// We don't connect internal door to the corridor so exit
		if (rooms[room].doors[i].internal)
			return 0;
		doors[num_doors++] = rooms[room].doors[i];
	}
	// Find the doors of other rooms leading to the given room
	for (i = 0; i < total_rooms; i++) {
		for (j = 0; j < rooms[i].num_doors; j++) {
			if (rooms[i].doors[j].room == room) {
				if (rooms[i].doors[j].internal)
					return 0;
				if (num_doors == MAX_DOORS_IN_CORRIDOR)
					return 0;
				doors[num_doors++] = rooms[i].doors[j];
			}
		}
	}

	if (num_doors == 1)
		return 0;

	// Calculate new room width and height
	for (i = 0; i < num_doors; i++) {
		if (x1 <= doors[i].x - 1 && doors[i].x - 1 < x2) {
			xmin = min(doors[i].x, xmin);
			xmax = max(doors[i].x + 2, xmax);
		} else {
			ymin = min(doors[i].y, ymin);
			ymax = max(doors[i].y + 2, ymax);
		}
	}
	// If there is not enoguh information to calculate room dimensions
	// we will suggest its position in the center
	if (xmin > xmax) {
		xmin = (x1 + x2) / 2 - 1;
		xmax = xmin + 1;
	}
	if (ymin > ymax) {
		ymin = (y1 + y2) / 2 - 1;
		ymax = ymin + 1;
	}
	// Remove old tiles and redraw room
	rooms[room].x = xmin - 2;
	rooms[room].y = ymin - 2;
	rooms[room].w = max(xmax - xmin, 2);
	rooms[room].h = max(ymax - ymin, 2);
	for (i = y1; i <= y2; i++) {
		for (j = x1; j <= x2; j++)
			mapgen_put_tile(j, i, TILE_WALL, -1);
	}
	mapgen_draw_room(room);

	return 1;
}

static int cmp_room_surface(const void *room1, const void *room2)
{
	int r1 = *(int *)room1;
	int r2 = *(int *)room2;

	int s1 = rooms[r1].w * rooms[r1].h;
	int s2 = rooms[r2].w * rooms[r2].h;

	if (s1 == s2) { 
		return 0;
	} else if (s1 < s2) {
		return 1;
	} else 
		return -1;
}

// Convert tile matrix to a set of obstacles and decorate rooms according to their themes,
// using mid_room as the center of dungeon
void mapgen_convert(struct dungeon_info *di, int w, int h, unsigned char *tiles)
{
	int i;
	int idx[di->num_rooms];
	int tries = 30;
	int n = 7;

	reduce_room_space();
	split_wall(w, h, tiles);

	// Sort rooms by their surface
	for (i = 0; i < di->num_rooms; i++)
		idx[i] = i;
	qsort(idx, di->num_rooms, sizeof(int), cmp_room_surface);

	i = 0;
	while(tries && n && (i < di->num_rooms)) {
		if (idx[i] != di->enter && idx[i] != di->exit) {
			if (make_corridor(idx[i]))
				n--;
			else
				tries--;
		}
		i++;
	}

	place_doors();

	qsort(idx, di->num_rooms, sizeof(int), cmp_room_surface);

	mapgen_place_obstacles(di, w, h, tiles, idx);
}

static void add_teleport(int telnum, int x, int y, int tpair)
{
	const int helpers[2][4] = {
		{ ISO_DROID_NEST_GREEN, ISO_DROID_NEST_GREEN, ISO_DROID_NEST_GREEN, ISO_DROID_NEST_GREEN },
		{ ISO_ENHANCER_RU, ISO_ENHANCER_LU, ISO_ENHANCER_RD, ISO_ENHANCER_LD }
	};

	char *warp, *fromwarp;
	char tmp[500];
	int obs_type, helper;

	sprintf(tmp, "%dtoX%d", target_level->levelnum, telnum);
	warp = strdup(tmp);

	sprintf(tmp, "%dfromX%d", target_level->levelnum, telnum);
	fromwarp = strdup(tmp);

	add_map_label(target_level, x, y, warp);
	add_map_label(target_level, x + 1, y, fromwarp);

	// A label placed at (x, y) is actually positioned at (x+0.5, y+0.5).
	// That 0.5 translation is added to the other obstacles around the labels
	// to have a coherent position.

	obs_type = telnum ? teleport_pairs[tpair].exit : teleport_pairs[tpair].enter;
	mapgen_add_obstacle(x + 0.5, y + 0.5, obs_type);

	// Decorate room with teleport if the obstacle is the cloud
	if (obs_type == ISO_TELEPORTER_1) {
		helper = MyRandom(1);
		mapgen_add_obstacle(x + 1 + 0.5, y - 1 + 0.5, helpers[helper][0]);
		mapgen_add_obstacle(x - 1 + 0.5, y - 1 + 0.5, helpers[helper][1]);
		mapgen_add_obstacle(x + 1 + 0.5, y + 1 + 0.5, helpers[helper][2]);
		mapgen_add_obstacle(x - 1 + 0.5, y + 1 + 0.5, helpers[helper][3]);
	}
}

void mapgen_entry_at(struct roominfo *r, int tpair)
{
	add_teleport(0, r->x + r->w / 2, r->y + r->h / 2, tpair);
}

void mapgen_exit_at(struct roominfo *r, int tpair)
{
	add_teleport(1, r->x + r->w / 2, r->y + r->h / 2, tpair);
}

void mapgen_gift(struct roominfo *r)
{
	const int gifts[] = {
		ISO_E_CHEST2_CLOSED,
		ISO_W_CHEST2_CLOSED,
		ISO_S_CHEST2_CLOSED,
		ISO_N_CHEST2_CLOSED
	};
	const int dx[] = { -2, 1, 0, 0 };
	const int dy[] = { 0, 0, -2, 1 };

	int pos = MyRandom(3);

	struct {
		float x;
		float y;
	} positions[4] = {
		{
		r->x, r->y + r->h / 2}, {
		r->x + r->w - 1, r->y + r->h / 2}, {
		r->x + r->w / 2, r->y}, {
		r->x + r->w / 2, r->y + r->h - 1}
	};

	if (mapgen_get_tile(positions[pos].x + dx[pos], positions[pos].y + dy[pos]) == TILE_WALL) {
		obstacle_spec *spec = get_obstacle_spec(gifts[pos]);
		positions[pos].x += spec->right_border;
		positions[pos].y += spec->lower_border;
		mapgen_add_obstacle(positions[pos].x, positions[pos].y, gifts[pos]);
	}
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
	rooms[newid].x = x;
	rooms[newid].y = y;
	rooms[newid].w = w;
	rooms[newid].h = h;
	rooms[newid].num_neighbors = 0;
	rooms[newid].max_neighbors = 8;
	rooms[newid].neighbors = malloc(rooms[newid].max_neighbors * sizeof(int));
	rooms[newid].num_doors = 0;

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

void mapgen_add_door(int x, int y, int from, int to)
{
	int num = rooms[from].num_doors;
	if (num == MAX_DOORS) {
			error_message(__FUNCTION__, "Maximal number of doors for a room exceeded", PLEASE_INFORM | IS_FATAL);
			return;
	}
	rooms[from].doors[num].x = x;
	rooms[from].doors[num].y = y;
	rooms[from].doors[num].room = to;
	rooms[from].doors[num].internal = 0;
	rooms[from].num_doors++;
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
	const int shift = 2;
	int wp_x, wp_y, wp_nx, wp_ny;
	int room_1, room_2;

	wp_x = wp_nx = x;
	wp_y = wp_ny = y;

	switch (type) {
		case UP:
			wp_ny = y - 1;
			wp_y = y + 1; 
			break;
		case DOWN:
			wp_ny = y + 1;
			wp_y = y - 1;
			break;
		case LEFT:
			wp_nx = x - 1;
			wp_x = x + 1;
			break;
		case RIGHT:
			wp_nx = x + 1;
			wp_x = x - 1;
			break;
		default:
			error_message(__FUNCTION__, "Unknown connection type %d", PLEASE_INFORM | IS_FATAL, type);
			break;

	} 

	room_1 = mapgen_get_room(wp_nx, wp_ny);
	room_2 = mapgen_get_room(wp_x, wp_y);
	mapgen_add_door(x, y, room_2, room_1);
	add_neighbor(&rooms[room_1], room_2);
	add_neighbor(&rooms[room_2], room_1);

	// Make waypoint correction according to pending
	if (type == UP || type == DOWN) {
		wp_x -= shift;
		wp_nx -= shift;
	} else if (type == LEFT || type == RIGHT) {
		wp_y -= shift;
		wp_ny -= shift;
	}
	int wp1 = add_waypoint(target_level, wp_x, wp_y, 0);
	int wp2 = add_waypoint(target_level, wp_nx, wp_ny, 0);

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
		int max_wps = find_waypoints(rooms[rn].x - 1, rooms[rn].y - 1, rooms[rn].x + rooms[rn].w + 1, rooms[rn].y + rooms[rn].h + 1, wps, 25);
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

		int retries = 15;

		while ((nb--) > 0) {
			int newx = rooms[rn].x;
			int newy = rooms[rn].y;
			newx += MyRandom(rooms[rn].w - 1);
			newy += MyRandom(rooms[rn].h - 1);

			colldet_filter my_filter = { WalkablePassFilterCallback, NULL, 0.8, NULL };
			if (!SinglePointColldet(newx + 0.5, newy + 0.5, target_level->levelnum, &my_filter)) {
				// If the randomly chosen position is not passable, retry... a certain number of times before giving up.
				if (retries-- > 0) {
					nb++;
				}
				continue;
			}

			add_waypoint(target_level, newx, newy, 0);
		}
	}
}

// The function computes eccentricity for each room and picks the one
// with the minimal eccentricity as the center. Also it fills 'distance'
// array with the distances from the 'entrance' in terms of rooms.
static int get_middle_room(int entrance, int *distance)
{
	int i, j, k;
	int m;
	int dist[total_rooms][total_rooms];
	int eccentricity[total_rooms];

	// Initialize distance between pairs of rooms with fake infinity
	for (i = 0; i < total_rooms; i++) {
		for (j = 0; j < total_rooms; j++)
			dist[i][j] = 99999;
		// Distance from a room to itself is 0
		dist[i][i] = 0;
		eccentricity[i] = 0;
	}
	for (i = 0; i < total_rooms; i++) {
		// Distance from a room to its neighbors is 1
		for (j = 0; j < rooms[i].num_neighbors; j++)
			dist[i][rooms[i].neighbors[j]] = 1;
	}
	// Calculate distance for each pair of rooms
	for (k = 0; k < total_rooms; k++) {
		for (i = 0; i < total_rooms; i++) {
			for (j = 0; j < total_rooms; j++)
				dist[i][j] = min(dist[i][j], dist[i][k] + dist[k][j]);
		}
	}
	// Eccentricity of a room is a maximum of the shortest distances
	// to all other rooms.
	for (i = 0; i < total_rooms; i++) {
		for (j = 0; j < total_rooms; j++)
			eccentricity[i] = max(eccentricity[i], dist[i][j]);
	}
	// Thus the central room is one with the minimal
	// eccentricity
	m = 0;
	for (i = 0; i < total_rooms; i++) {
		if (eccentricity[i] < eccentricity[m])
			m = i;
	}
	// Fill array of distances for a room with the entrance
	for (i = 0; i < total_rooms; i++)
		distance[i] = dist[entrance][i];

	return m;
}

int generate_dungeon(int w, int h, int nbconnec, int tpair)
{
	int i, j;
	int max, max_idx = 0;
	struct dungeon_info di;

	new_level(w, h);

	generate_dungeon_gram(w, h);

	// Select entrance at random.
	int dist[total_rooms];
	int vis[total_rooms];
	int entrance = rand() % total_rooms;
	int mid_room = get_middle_room(entrance, dist);

	mapgen_entry_at(&rooms[entrance], tpair);

	memset(vis, 0, sizeof(int) * total_rooms);
	// Choose N farthest rooms and place exits there
	for (i = 0; i < nbconnec - 1; i++) {
		max = dist[0];
		max_idx = 0;
		for (j = 1; j < total_rooms; j++) {
			if (dist[j] > max && !vis[j]) {
				max = dist[j];
				max_idx = j;
			}
		}
		mapgen_exit_at(&rooms[max_idx], tpair);
		vis[max_idx] = 1;
	}

	di.enter = entrance;
	di.exit = max_idx;
	di.middle_room = mid_room;
	di.num_rooms = total_rooms;
	di.distance = dist;
	mapgen_convert(&di, w, h, map.m);

	// Place random waypoints
	place_waypoints();

	// Connect waypoints
	connect_waypoints();

	free_level();
	return 0;
}

const char * mapgen_teleport_pair_str(int idx)
{
	const char *teleport_pair_str[] = {
		"In - cloud; Out - cloud",
		"In - cloud; Out - ladder up",
		"In - ladder down; Out - cloud",
		"In - ladder down; Out - ladder up"
	};

	return teleport_pair_str[idx];
}

unsigned int mapgen_cycle_teleport_pair(unsigned int value)
{
	unsigned int sz = sizeof(teleport_pairs) / sizeof(teleport_pairs[0]);

	value++;
	if (value >= sz)
		value = 0;

	return value;
}
