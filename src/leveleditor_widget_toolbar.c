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

#include "system.h"

#include "defs.h"
#include "struct.h"
#include "global.h"
#include "proto.h"

#include "SDL_rotozoom.h"

#include "leveleditor.h"
#include "leveleditor_actions.h"
#include "leveleditor_widgets.h"
#include "leveleditor_widget_toolbar.h"
#include "leveleditor_widget_typeselect.h"

static struct leveleditor_typeselect *previous_type = NULL;
static int num_blocks_per_line = 0;

void leveleditor_toolbar_mouseenter(SDL_Event *event, struct leveleditor_widget *vt)
{
    (void)vt;
}

void leveleditor_toolbar_mouseleave(SDL_Event *event, struct leveleditor_widget *vt)
{
    (void)vt;
}

void leveleditor_toolbar_mouserelease(SDL_Event *event, struct leveleditor_widget *vt)
{
    (void)vt;
}

void leveleditor_toolbar_mousepress(SDL_Event *event, struct leveleditor_widget *vt)
{
    (void)vt;

    struct leveleditor_typeselect *ts = get_current_object_type();
    int i;
    int x;

    for ( i = 0 ; i < num_blocks_per_line ; i ++ ) {
	x = INITIAL_BLOCK_WIDTH/2 + INITIAL_BLOCK_WIDTH * i; 

	if (event->button.x > x && event->button.x < x + INITIAL_BLOCK_WIDTH)
	    ts->selected_tile_nb = ts->toolbar_first_block + i;
    }

    for (i=0; ts->indices[i] != -1; i++);
    if (ts->selected_tile_nb >= i-1)
	ts->selected_tile_nb = i-1;

}

void leveleditor_toolbar_mouserightrelease(SDL_Event *event, struct leveleditor_widget *vt)
{
    (void)vt;
}

void leveleditor_toolbar_mouserightpress(SDL_Event *event, struct leveleditor_widget *vt)
{
    (void)vt;
    /*XXX maybe display info about the currently selected object?*/
}

void leveleditor_toolbar_mousewheelup(SDL_Event *event, struct leveleditor_widget *vt)
{
    (void)vt;
    leveleditor_toolbar_left();
}

void leveleditor_toolbar_mousewheeldown(SDL_Event *event, struct leveleditor_widget *vt)
{
    (void)vt;
    leveleditor_toolbar_right();
}

void leveleditor_toolbar_display(struct leveleditor_widget *vt)
{
    (void)vt;
    struct leveleditor_typeselect *ts = get_current_object_type();

    if (ts != previous_type) {
	previous_type = ts;
    }

    int number_of_tiles = 0;
    int i;
    int cindex = ts->toolbar_first_block;
    float zoom_factor;
    SDL_Surface *tmp;
    SDL_Rect TargetRectangle;

    // toolbar background
    SDL_Rect tr = {.x = 0, .y = 13, .w = GameConfig.screen_width, .h = 77 };
    our_SDL_fill_rect_wrapper(Screen, &tr, 0x556889);

    // now the tiles to be selected   
    
    // compute the number of tiles in this list
    for (i=0; ts->indices[i] != -1; i++);
    number_of_tiles = i;

    if (num_blocks_per_line == 0) {
	num_blocks_per_line = GameConfig.screen_width / INITIAL_BLOCK_WIDTH - 1;
    }

    for ( i = 0 ; i < num_blocks_per_line ; i ++ )  {

	if (cindex >= number_of_tiles) 
	    break;

	TargetRectangle.x = INITIAL_BLOCK_WIDTH/2 + INITIAL_BLOCK_WIDTH * i ;
	TargetRectangle.y = INITIAL_BLOCK_HEIGHT/3 ;
	TargetRectangle.w = INITIAL_BLOCK_WIDTH ;
	TargetRectangle.h = INITIAL_BLOCK_HEIGHT ;

	iso_image * img = leveleditor_get_object_image(ts->type, ts->indices, cindex);
	if (!img) break;

	// We find the proper zoom_factor, so that the obstacle/tile in question will
	// fit into one tile in the level editor top status selection row.
	//
	if ( use_open_gl )  {
	    zoom_factor = min ( 
		    ((float)INITIAL_BLOCK_WIDTH / (float)img -> original_image_width),
		    ((float)INITIAL_BLOCK_HEIGHT / (float)img -> original_image_height));
	} else {
	    zoom_factor = min ( 
		    ((float)INITIAL_BLOCK_WIDTH / (float)img -> surface->w),
		    ((float)INITIAL_BLOCK_HEIGHT / (float)img -> surface->h));
	}

	if (use_open_gl) {
	    draw_gl_scaled_textured_quad_at_screen_position (img, TargetRectangle.x, TargetRectangle.y, zoom_factor) ;
	} else {
	    // We create a scaled version of the obstacle/floorpiece in question
	    tmp = zoomSurface ( img -> surface , zoom_factor, zoom_factor, FALSE );

	    // Now we can show and free the scaled verion of the floor tile again.
	    our_SDL_blit_surface_wrapper( tmp , NULL , Screen, &TargetRectangle);
	    SDL_FreeSurface ( tmp );
	}

	if (cindex == ts->selected_tile_nb) 
	    HighlightRectangle ( Screen , TargetRectangle );

	cindex ++ ;
    }
}

void leveleditor_toolbar_left()
{
    struct leveleditor_typeselect *ts = get_current_object_type();
    
    ts->selected_tile_nb = ts->selected_tile_nb <= 1 ? 0 : ts->selected_tile_nb - 1;
    
    if (ts->selected_tile_nb < ts->toolbar_first_block)
	ts->toolbar_first_block--;
}

void leveleditor_toolbar_right()
{
    struct leveleditor_typeselect *ts = get_current_object_type();
    int i;

    for (i=0; ts->indices[i] != -1; i++);
    
    ts->selected_tile_nb = ts->selected_tile_nb >= i - 1 ? i - 1 : ts->selected_tile_nb + 1;
    
    if (ts->selected_tile_nb >= ts->toolbar_first_block + num_blocks_per_line) {
	ts->toolbar_first_block++;
    } 
}

void leveleditor_toolbar_scroll_left()
{
    struct leveleditor_typeselect *ts = get_current_object_type();
    
    if (ts->toolbar_first_block < 8)
	ts->toolbar_first_block = 0;
    else
	ts->toolbar_first_block -= 8;
    
    if (ts->toolbar_first_block + num_blocks_per_line <= ts->selected_tile_nb) {
	ts->selected_tile_nb = ts->toolbar_first_block + num_blocks_per_line - 1;
    } 
}

void leveleditor_toolbar_scroll_right()
{
    struct leveleditor_typeselect *ts = get_current_object_type();
    int i;

    for (i=0; ts->indices[i] != -1; i++);
    
    ts->toolbar_first_block += 8;
    if (ts->toolbar_first_block >= i - 1)
	ts->toolbar_first_block -= 8;
    
    if (ts->selected_tile_nb < ts->toolbar_first_block)
	ts->selected_tile_nb = ts->toolbar_first_block;
}

