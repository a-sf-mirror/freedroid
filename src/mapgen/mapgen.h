#ifndef MAPGEN_H
#define MAPGEN_H

#define 	MAX_DOORS	4

enum TILES {
	TILE_WALL,
	TILE_EMPTY,
	TILE_FLOOR,
	// Internal wall
	TILE_PARTITION
};

struct doorinfo {
	int x, y;
	int room;
	int internal;
};

struct roominfo {
	int x, y;
	int w, h;
	int theme;
	// Period is required to generate various periodical things,
	// like windows in the walls
	int period;

	int *neighbors;
	int num_neighbors;
	int max_neighbors;

	struct doorinfo doors[MAX_DOORS];
	int num_doors;
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

struct dungeon_info {
	int enter;
	int exit;
	int middle_room;

	int num_rooms;
	int *distance;
};

extern struct roominfo *rooms;
extern int total_rooms;

// Interface to the game
void (*dungeonmap_convert) (int, int, unsigned char *);
void (*dungeonmap_place_enemies) (struct roominfo *);
void (*dungeonmap_gift) (struct roominfo *);

int generate_dungeon_gram(int, int);

int mapgen_add_room(int, int, int, int);
void mapgen_put_tile(int, int, unsigned char, int);
unsigned char mapgen_get_tile(int x, int y);
int mapgen_get_room(int x, int y);
void mapgen_draw_room(int room_id);
int mapgen_are_connected(int, int);
int mapgen_is_connected(unsigned char *);
void mapgen_add_obstacle(double x, double y, int type);
void mapgen_set_floor(int x, int y, int type);
void mapgen_gift(struct roominfo *r);
void mapgen_add_door(int, int, int, int);
unsigned int mapgen_cycle_teleport_pair(unsigned int);
const char * mapgen_teleport_pair_str(int);

int find_connection_points(int room_id, struct cplist_t cplist[100], int offset);

void MakeConnect(int x, int y, enum connection_type type);


#endif
