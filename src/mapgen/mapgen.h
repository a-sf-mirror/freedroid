#ifndef MAPGEN_H
#define MAPGEN_H

/* Bind tile numbers to named constants, considering
 * that Y axis is vector (N, S) and X is (W, E)
 */
// straight walls
#define TILE_WALL_S		4
#define TILE_WALL_N		5
#define TILE_WALL_E		6
#define TILE_WALL_W		7
// corner walls
#define TILE_WALL_SE	8
#define TILE_WALL_SW	9
#define TILE_WALL_NE	10
#define TILE_WALL_NW	11
// ramps and doors
#define TILE_RAMP_H		13	
#define TILE_DOOR_H		14
#define TILE_DOOR_V		15
#define TILE_RAMP_V		16
// other
#define TILE_EMPTY		17
#define TILE_FLOOR		12

struct roominfo {
	int x, y;
	int w, h;

	int *neighbors;
	int next_neighbor;
	int max_neighbors;
};

struct mapgen_gamelevel {
	int w, h;
	unsigned char *m;
	int *r;
};

enum connection_type {
	UP = 0,
	DOWN = 1,
	LEFT = 2,
	RIGHT = 3
};

struct cplist_t {
	int x, y, r;
	enum connection_type t;
};

extern struct roominfo *rooms;
extern int total_rooms;

// Interface to the game
void (*dungeonmap_convert) (int, int, unsigned char *, int *);
void (*dungeonmap_entry_at) (struct roominfo *);
void (*dungeonmap_exit_at) (struct roominfo *);
void (*dungeonmap_place_enemies) (struct roominfo *);
void (*dungeonmap_gift) (struct roominfo *);

int generate_dungeon_gram(int, int);

int mapgen_add_room(int, int, int, int);
void mapgen_put_tile(int, int, unsigned char, int);
unsigned char mapgen_get_tile(int x, int y);
int mapgen_get_room(int x, int y);
void mapgen_draw_room(int place_x, int place_y, int room_w, int room_h, int room_id);
int mapgen_are_connected(int, int);
int mapgen_is_connected(unsigned char *);

int find_connection_points(int room_id, struct cplist_t cplist[100]);

void MakeConnect(int x, int y, enum connection_type type);

#endif
