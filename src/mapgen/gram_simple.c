#include "system.h"

#include "defs.h"
#include "struct.h"
#include "../src/global.h"
#include "proto.h"
#include "savestruct.h"

#include "mapgen/mapgen.h"

/* Minimum surface of a room */
static const int Smin = 40;

/* Dimensions of the dungeon */
static int dim_x_init;
static int dim_y_init;

/* Worst y/x and x/y ratios accepted. */
#define WORST_ROOM_RATIO 2.0

static enum cut_axis {
	CUT_HORIZONTALLY = 0,
	CUT_VERTICALLY   = 1,
	DO_NOT_CUT       = 2
};

/** Check if the two proposed room sizes satisfy the minimal surface criteria or not.
 */
static int check_surf(int w_creator, int h_creator, int w_newroom, int h_newroom)
{
	if ((w_creator * h_creator > Smin) && (h_newroom * w_newroom > Smin))
		return 1;

	return 0;
}

/**
 * Try to cut a room of dimensions dim_x and dim_y, horizontally or vertically
 * depending on the value of vert, with a ratio r.
 *
 * This function picks a ratio for the cut and sets r accordingly.
 */
static int trycut(int dim_x, int dim_y, int *r, enum cut_axis vert)
{
	/*
	 * Example computation for a vertical cut : 
	 *
	 * x1 + x2 = dim_x
	 * x1 = r * x
	 * x2 = (1-r) * x
	 * log(x1/y) <= b, b constant
	 * log(x1/y) >= -b
	 *
	 * =>  y/(a * x) <= r <= a * y/x 
	 * and 1-(ya)/x <= r <= 1 - y/(ax) 
	 *
	 * with a = exp(b)
	 */
	float rmin = 1, rmin2 = 1, rmax = 0, rmax2 = 0;

	switch (vert) {
	case CUT_HORIZONTALLY:
		rmax = WORST_ROOM_RATIO * (float)dim_x / dim_y;
		rmin2 = (1.0 / WORST_ROOM_RATIO) * dim_x / dim_y;
		break;
	case CUT_VERTICALLY:
		rmax = WORST_ROOM_RATIO * (float)dim_y / dim_x;
		rmin2 = (1.0 / WORST_ROOM_RATIO) * dim_y / dim_x;
		break;
	default:
		;
	}

	rmin = 1 - rmax;
	rmax2 = 1 - rmin2;

	rmax = rmax > rmax2 ? rmax2 : rmax;
	rmin = rmin < rmin2 ? rmin2 : rmin;

	if (rmax < rmin) {
		// No cut possible this way
		return 0;
	}
	// Pick a cut ratio
	*r = 100.0 * (rmin + (rmax - rmin) * ((float)rand() / (float)RAND_MAX));

	return 1;
}

/**
 * Make a decision about a cut in a room: whether to cut at all,
 *  what way to cut, and at what position
 */
static enum cut_axis cut(int dim_x, int dim_y, int *r)
{
	enum cut_axis ret = DO_NOT_CUT;
	// linear probability of cut
	float chancetocut = (1 / (dim_x_init * dim_y_init)) * dim_x * dim_y - (Smin / (dim_x_init * dim_y_init - Smin));
	float p = rand() % 10000 + 1;

	// Test whether to cut at all
	if (p / 10000 < chancetocut) {
		return DO_NOT_CUT;
	}
	// Pick a random direction to cut along
	ret = rand() % 2;

	// Try to cut along this direction
	if (!trycut(dim_x, dim_y, r, ret)) {
		// If we cannot, try the other direction
		if (!trycut(dim_x, dim_y, r, !ret))
			return DO_NOT_CUT;
		else
			ret = !ret;
	}
	// Rooms need to be 4 tiles wide at least
	switch (ret) {
	case CUT_HORIZONTALLY:
		if ((*r / 100.0 * dim_y) < 4 || ((100.0 - *r) / 100.0 * dim_y) < 4)
			return DO_NOT_CUT;
		break;
	case CUT_VERTICALLY:
		if ((*r / 100.0 * dim_x) < 4 || ((100.0 - *r) / 100.0 * dim_x) < 4)
			return DO_NOT_CUT;
		break;
	default:
		;
	}

	return ret;
}

static void deriv_P(int id)
{
	int p;
	int prop;
	int x = rooms[id].x;
	int y = rooms[id].y;
	int creator = id;
	int dim_x = rooms[creator].w;
	int dim_y = rooms[creator].h;
	int newroom;
	p = cut(dim_x, dim_y, &prop);

	int w_creator = dim_x;
	int h_creator = dim_y;
	int w_newroom = dim_x;
	int h_newroom = dim_y;
	int x_newroom = x;
	int y_newroom = y;

	/*
	   Two rules : 
	   P -> P h P | P -> P v P

	   h horizontal cut
	   v vertical cut
	 */
	switch (p) {
	case CUT_HORIZONTALLY:
		h_creator = rooms[creator].h * prop / 100.0;
		h_newroom = dim_y - h_creator;
		y_newroom = y + h_creator;
		break;
	case CUT_VERTICALLY:
		w_creator = rooms[creator].w * prop / 100.0;
		w_newroom = dim_x - w_creator;
		x_newroom = x + w_creator;
		break;
	default:
		return;
	}

	if (!check_surf(w_creator, h_creator, w_newroom, h_newroom)) {
		return;
	}

	newroom = mapgen_add_room(x_newroom, y_newroom, w_newroom, h_newroom);

	rooms[creator].w = w_creator;
	rooms[creator].h = h_creator;

	mapgen_draw_room(rooms[newroom].x, rooms[newroom].y, rooms[newroom].w, rooms[newroom].h, newroom);
	mapgen_draw_room(rooms[creator].x, rooms[creator].y, rooms[creator].w, rooms[creator].h, creator);
	deriv_P(id);
	deriv_P(newroom);
}

static int change_cote(int depart, int ajout)
{

	if (depart > 7) {
		return ajout;
	}
	switch (depart) {
	case 4:
	case 5:{
			switch (ajout) {
			case 6:
			case 7:
				return TILE_FLOOR;
				break;
			default:
				return ajout;
				break;
			}
		}
		break;
	case 6:
	case 7:{
			switch (ajout) {
			case 4:
			case 5:
				return TILE_FLOOR;
				break;
			default:
				return ajout;
				break;
			}
		}
		break;
	default:{
			return ajout;
		}
		break;
	}
}

/**
 * Is the given position a room corner?
 */
static int is_room_corner(int x, int y)
{
	int id = mapgen_get_room(x, y);
	if ((x == rooms[id].x && y == rooms[id].y) ||
	    (x == rooms[id].x && y == rooms[id].y + rooms[id].h - 1) ||
	    (x == rooms[id].x + rooms[id].w - 1 && y == rooms[id].y) ||
	    (x == rooms[id].x + rooms[id].w - 1 && y == rooms[id].y + rooms[id].h - 1)) {
		return 1;
	}
	return 0;
}

static void adj(struct cplist_t cplist, int x, int y, int *nx, int *ny)
{
	switch (cplist.t) {
	case UP:
		*nx = x;
		*ny = y - 1;
		break;
	case DOWN:
		*nx = x;
		*ny = y + 1;
		break;
	case LEFT:
		*nx = x - 1;
		*ny = y;
		break;
	case RIGHT:
		*nx = x + 1;
		*ny = y;
		break;
	default:
		*nx = 0;
		*ny = 0;
		break;
	}
}

/**
 * Join two rooms by breaking the wall between them
 */
void fusion(int id, int cible)
{
	struct cplist_t cplist[100];
	int correct_directory[100];

	memset(correct_directory, -1, 100 * sizeof(int));
	memset(cplist, -1, 100 * sizeof(struct cplist_t));

	int nb_max;
	nb_max = find_connection_points(id, cplist);
	int k = 0;
	int l = 0;		//index du tableau correct_directory
	int nx, ny;
	int x, y;
	while (k < nb_max) {
		x = cplist[k].x;
		y = cplist[k].y;
		nx = 0;
		ny = 0;
		adj(cplist[k], x, y, &nx, &ny);
		if (cplist[k].r == cible && !is_room_corner(x, y) && !is_room_corner(nx, ny)) {
			correct_directory[l] = k;
			l++;
		}
		k++;
	}

	for (k = 0; k < l; k++) {
		x = cplist[correct_directory[k]].x;
		y = cplist[correct_directory[k]].y;
		nx = 0;
		ny = 0;
		adj(cplist[correct_directory[k]], x, y, &nx, &ny);
		mapgen_put_tile(x, y, TILE_FLOOR, id);
		mapgen_put_tile(nx, ny, TILE_FLOOR, cible);

		if (k == 0 && cplist[correct_directory[k]].t == UP) {
			if (rooms[id].x > rooms[cible].x) {
				mapgen_put_tile(x - 1, y, change_cote(mapgen_get_tile(x - 1, y), TILE_WALL_W), id);
				mapgen_put_tile(nx - 1, ny, change_cote(mapgen_get_tile(nx - 1, ny), TILE_FLOOR), cible);
			} else if (rooms[id].x == rooms[cible].x) {
				mapgen_put_tile(x - 1, y, change_cote(mapgen_get_tile(x - 1, y), TILE_WALL_W), id);
				mapgen_put_tile(nx - 1, ny, change_cote(mapgen_get_tile(nx - 1, ny), TILE_WALL_W), cible);
			} else {
				mapgen_put_tile(x - 1, y, change_cote(mapgen_get_tile(x - 1, y), TILE_FLOOR), id);
				mapgen_put_tile(nx - 1, ny, change_cote(mapgen_get_tile(nx - 1, ny), TILE_WALL_W), cible);
			}
		}

		if (k == l - 1 && cplist[correct_directory[k]].t == UP) {
			if (rooms[id].x + rooms[id].w > rooms[cible].x + rooms[cible].w) {
				mapgen_put_tile(x + 1, y, change_cote(mapgen_get_tile(x + 1, y), TILE_FLOOR), id);
				mapgen_put_tile(nx + 1, ny, change_cote(mapgen_get_tile(nx + 1, ny), TILE_WALL_E), cible);
			} else if (rooms[id].x + rooms[id].w == rooms[cible].x + rooms[cible].w) {
				mapgen_put_tile(x + 1, y, change_cote(mapgen_get_tile(x + 1, y), TILE_WALL_E), id);
				mapgen_put_tile(nx + 1, ny, change_cote(mapgen_get_tile(nx + 1, ny), TILE_WALL_E), cible);

			} else {
				mapgen_put_tile(x + 1, y, change_cote(mapgen_get_tile(x + 1, y), TILE_WALL_E), id);
				mapgen_put_tile(nx + 1, ny, change_cote(mapgen_get_tile(nx + 1, ny), TILE_FLOOR), cible);
			}
		}

		if (k == 0 && cplist[correct_directory[k]].t == DOWN) {
			if (rooms[id].x < rooms[cible].x) {
				mapgen_put_tile(x - 1, y, change_cote(mapgen_get_tile(x - 1, y), TILE_FLOOR), id);
				mapgen_put_tile(nx - 1, ny, change_cote(mapgen_get_tile(nx - 1, ny), TILE_WALL_W), cible);
			} else if (rooms[id].x == rooms[cible].x) {
				mapgen_put_tile(x - 1, y, change_cote(mapgen_get_tile(x - 1, y), TILE_WALL_W), id);
				mapgen_put_tile(nx - 1, ny, change_cote(mapgen_get_tile(nx - 1, ny), TILE_WALL_W), cible);
			} else {
				mapgen_put_tile(x - 1, y, change_cote(mapgen_get_tile(x - 1, y), TILE_WALL_W), id);
				mapgen_put_tile(nx - 1, ny, change_cote(mapgen_get_tile(nx - 1, ny), TILE_FLOOR), cible);
			}
		}

		if (k == l - 1 && cplist[correct_directory[k]].t == DOWN) {
			if (rooms[id].x + rooms[id].w < rooms[cible].x + rooms[cible].w) {
				mapgen_put_tile(x + 1, y, change_cote(mapgen_get_tile(x + 1, y), TILE_WALL_E), id);
				mapgen_put_tile(nx + 1, ny, change_cote(mapgen_get_tile(nx + 1, ny), TILE_FLOOR), cible);
			} else if (rooms[id].x + rooms[id].w == rooms[cible].x + rooms[cible].w) {
				mapgen_put_tile(x + 1, y, change_cote(mapgen_get_tile(x + 1, y), TILE_WALL_E), id);
				mapgen_put_tile(nx + 1, ny, change_cote(mapgen_get_tile(nx + 1, ny), TILE_WALL_E), cible);
			} else {
				mapgen_put_tile(x + 1, y, change_cote(mapgen_get_tile(x + 1, y), TILE_FLOOR), id);
				mapgen_put_tile(nx + 1, ny, change_cote(mapgen_get_tile(nx + 1, ny), TILE_WALL_E), cible);
			}
		}

		if (k == 0 && cplist[correct_directory[k]].t == LEFT) {
			if (rooms[id].y > rooms[cible].y) {
				mapgen_put_tile(x, y - 1, change_cote(mapgen_get_tile(x, y - 1), TILE_WALL_N), id);
				mapgen_put_tile(nx, ny - 1, change_cote(mapgen_get_tile(nx, ny - 1), TILE_FLOOR), cible);
			} else if (rooms[id].y == rooms[cible].y) {
				mapgen_put_tile(x, y - 1, change_cote(mapgen_get_tile(x, y - 1), TILE_WALL_N), id);
				mapgen_put_tile(nx, ny - 1, change_cote(mapgen_get_tile(nx, ny - 1), TILE_WALL_N), cible);
			} else {
				mapgen_put_tile(x, y - 1, change_cote(mapgen_get_tile(x, y - 1), TILE_FLOOR), id);
				mapgen_put_tile(nx, ny - 1, change_cote(mapgen_get_tile(nx, ny - 1), TILE_WALL_N), cible);
			}
		}

		if (k == l - 1 && cplist[correct_directory[k]].t == LEFT) {
			if (rooms[id].y + rooms[id].h < rooms[cible].y + rooms[cible].h) {
				mapgen_put_tile(x, y + 1, change_cote(mapgen_get_tile(x, y + 1), TILE_WALL_S), id);
				mapgen_put_tile(nx, ny + 1, change_cote(mapgen_get_tile(nx, ny + 1), TILE_FLOOR), cible);
			} else if (rooms[id].y + rooms[id].h == rooms[cible].y + rooms[cible].h) {
				mapgen_put_tile(x, y + 1, change_cote(mapgen_get_tile(x, y + 1), TILE_WALL_S), id);
				mapgen_put_tile(nx, ny + 1, change_cote(mapgen_get_tile(nx, ny + 1), TILE_WALL_S), cible);
			} else {
				mapgen_put_tile(x, y + 1, change_cote(mapgen_get_tile(x, y + 1), TILE_FLOOR), id);
				mapgen_put_tile(nx, ny + 1, change_cote(mapgen_get_tile(nx, ny + 1), TILE_WALL_S), cible);
			}
		}

		if (k == 0 && cplist[correct_directory[k]].t == RIGHT) {
			if (rooms[id].y < rooms[cible].y) {
				mapgen_put_tile(x, y - 1, change_cote(mapgen_get_tile(x, y - 1), TILE_FLOOR), id);
				mapgen_put_tile(nx, ny - 1, change_cote(mapgen_get_tile(nx, ny - 1), TILE_WALL_N), cible);
			} else if (rooms[id].y == rooms[cible].y) {
				mapgen_put_tile(x, y - 1, change_cote(mapgen_get_tile(x, y - 1), TILE_WALL_N), id);
				mapgen_put_tile(nx, ny - 1, change_cote(mapgen_get_tile(nx, ny - 1), TILE_WALL_N), cible);
			} else {
				mapgen_put_tile(x, y - 1, change_cote(mapgen_get_tile(x, y - 1), TILE_WALL_N), id);
				mapgen_put_tile(nx, ny - 1, change_cote(mapgen_get_tile(nx, ny - 1), TILE_FLOOR), cible);
			}
		}

		if (k == l - 1 && cplist[correct_directory[k]].t == RIGHT) {
			if (rooms[id].y + rooms[id].h > rooms[cible].y + rooms[cible].h) {
				mapgen_put_tile(x, y + 1, change_cote(mapgen_get_tile(x, y + 1), TILE_FLOOR), id);
				mapgen_put_tile(nx, ny + 1, change_cote(mapgen_get_tile(nx, ny + 1), TILE_WALL_S), cible);
			} else if ((rooms[id].y + rooms[id].h) == (rooms[cible].y + rooms[cible].h)) {
				mapgen_put_tile(x, y + 1, change_cote(mapgen_get_tile(x, y + 1), TILE_WALL_S), id);
				mapgen_put_tile(nx, ny + 1, change_cote(mapgen_get_tile(nx, ny + 1), TILE_WALL_S), cible);
			} else {
				mapgen_put_tile(x, y + 1, change_cote(mapgen_get_tile(x, y + 1), TILE_WALL_S), id);
				mapgen_put_tile(nx, ny + 1, change_cote(mapgen_get_tile(nx, ny + 1), TILE_FLOOR), cible);
			}
		}
	}
}

static void add_rel(int x, int y, enum connection_type type, int cible)
{
	int id = mapgen_get_room(x, y);
	MakeConnect(x, y, type);
	if ((((rooms[id].x != rooms[cible].x) || (rooms[id].w != rooms[cible].w)) && (type == UP || type == DOWN))
	    || (((rooms[id].y != rooms[cible].y) || (rooms[id].h != rooms[cible].h)) && (type == RIGHT || type == LEFT))) {
		if (!(rand() % 4))
			fusion(id, cible);
	}
}

static void bulldozer(unsigned char *seen, int r)
{
	struct cplist_t cplist[300];
	int max_connections = find_connection_points(r, cplist);

	// Mark the room as seen by a bulldozer
	seen[r] = 1;

	// Pick a random connection to do
	int i = rand() % max_connections;
	int x2 = cplist[i].x;
	int y2 = cplist[i].y;
	adj(cplist[i], x2, y2, &x2, &y2);

	// if the rooms are already connected we do not create a new connection and stop the bulldozer
	if (mapgen_are_connected(r, cplist[i].r))
		return;

	// else we create a connection
	add_rel(cplist[i].x, cplist[i].y, cplist[i].t, cplist[i].r);

	// and move the bulldozer to the next room
	bulldozer(seen, mapgen_get_room(x2, y2));
}

static void launch_buldo()
{
	int r, i, recalculate_components;
	unsigned char seen[total_rooms];
	unsigned char connected_to_room_0[total_rooms];
	memset(seen, 0, total_rooms);

	// Start bulldozers so that every room has been seen
	for (r = 0; r < total_rooms; r++) {
		if (!(seen[r])) {
			bulldozer(seen, r);
		}
	}

	// Check whether the graph is connected.
	// If it is not, ensure connectivity by connecting each vertex from 
	// a component that is not 0's to 0's whenever possible, until we have
	// connectivity.
	while (!mapgen_is_connected(connected_to_room_0)) {
		recalculate_components = 0;
		// Find the first room that is not connected to room 0
		for (i = 1; i < total_rooms && !recalculate_components; i++) {
			if (!connected_to_room_0[i]) {
				// See if we can connect it to a room that belongs to 0's connected component
				int n;
				struct cplist_t neigh[100];
				int nbconn, prevneigh = -1;
				nbconn = find_connection_points(i, neigh);
				for (n = 0; n < nbconn; n++) {
					if (neigh[n].r == prevneigh)
						continue;
					else {
						int next = 0;
						while (next < nbconn && neigh[n + next].r == neigh[n].r)
							next++;

						int pick = n + rand() % next;

						n = pick;
					}

					prevneigh = neigh[n].r;

					if (connected_to_room_0[neigh[n].r]) {
						add_rel(neigh[n].x, neigh[n].y, neigh[n].t, neigh[n].r);

						if (!(rand() % 3)) {
							recalculate_components = 1;
						}

						break;
					}
				}
			}
		}
	}
}

int generate_dungeon_gram(int dim_x, int dim_y)
{
	dim_x_init = dim_x;
	dim_y_init = dim_y;
	total_rooms = 0;

	// Create first room
	mapgen_add_room(0, 0, dim_x_init, dim_y_init);
	mapgen_draw_room(0, 0, dim_x_init, dim_y_init, total_rooms);

	// Recursively cut
	deriv_P(0);

	// Make connections between rooms
	launch_buldo();
	return 0;
}
