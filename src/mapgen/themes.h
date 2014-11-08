/* 
 *
 *   Copyright (c) 2010, Alexander Solovets
 *
 *
 *  This file is part of Freedroid
 *
 *  Freedroid is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  Freedroid is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with Freedroid; see the file COPYING. If not, write to the 
 *  Free Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, 
 *  MA  02111-1307  USA
 *
 */

#ifndef THEMES_H
#define THEMES_H

#define		WALL_N	1
#define		WALL_W	(1 << 1)
#define		WALL_S	(1 << 2)
#define		WALL_E	(1 << 3)

#define		WALL_PART	(1 << 4)

#define		WALL_NW	(WALL_N | WALL_W)
#define		WALL_NE	(WALL_N | WALL_E)
#define		WALL_SW	(WALL_S | WALL_W)
#define		WALL_SE	(WALL_S | WALL_E)

enum theme {
	THEME_METAL,
	THEME_GRAY,
	THEME_GLASS,
	THEME_RED,
	THEME_BROKEN_GLASS,
	THEME_GREEN,
	THEME_FLOWER,
	NUM_THEMES
};

struct theme_info {
	int wall_w;
	int wall_n;
	int wall_e;
	int wall_s;

	int window_wall_h;
	int window_wall_v;

	int floor[2];
}; 

void mapgen_place_obstacles(struct dungeon_info *, int, int, unsigned char *, int *);

#endif
