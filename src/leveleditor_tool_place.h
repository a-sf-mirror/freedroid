
/* 
 *
 *   Copyright (c) 2009 Arthur Huillet
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

int leveleditor_place_input(SDL_Event *event);
int leveleditor_place_display();

struct leveleditor_place {
    enum { MODE_LINE, MODE_RECTANGLE, MODE_SINGLE } mode;

    /* Line mode */
    int l_direction;
    int l_id;
    line_element l_elements;

    /* Rectangle mode */
    point r_start;
    int r_len_x, r_len_y;
    int r_step_x, r_step_y;
    int r_tile_used;
};

