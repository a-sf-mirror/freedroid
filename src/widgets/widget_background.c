/* 
 *
 *   Copyright (c) 2011 Catalin Badea
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

/**
 * \file widget_background.c
 * \brief This file contains the implementation of the widget_background functions.
 *
 */

#include "system.h"
#include "defs.h"
#include "struct.h"
#include "global.h"
#include "proto.h"
#include "widgets/widgets.h"

/**
 * \brief Structure type used for storing images by background widgets.
 */
struct tile {
	SDL_Rect rect;		 /**< Position rectangle of the image. */
	struct image *image; /**< Pointer to the image structure */
	enum image_transformation_mode mode;
};

static void widget_background_free(struct widget *w);

//////////////////////////////////////////////////////////////////////
// Overloads of Base widget functions
//////////////////////////////////////////////////////////////////////

/**
 * \brief Display a background widget.
 * \relates widget_background
 *
 * \details Display all the images composing the background of the widget.\n
 * Position and size of the images have been computed when they have been added
 * to the widget.
 *
 * \param w Pointer to the widget_background object
 */
static void background_display(struct widget *w)
{
	// A background can be composed of several tiles.
	int i;
	struct tile *tile = WIDGET_BACKGROUND(w)->tiles.arr;
	for (i = 0; i < WIDGET_BACKGROUND(w)->tiles.size; i++) {
		struct image *img = tile[i].image;
		SDL_Rect rect = tile[i].rect;

		if (!img)
			return;

		struct image_transformation img_trsf = set_image_transformation(rect.w / (float)img->w, rect.h / (float)img->h, 1, 1, 1, 1, 0);
		img_trsf.mode = tile[i].mode;
		display_image_on_screen(img, rect.x, rect.y, img_trsf);
	}
}

//////////////////////////////////////////////////////////////////////
// Background Widget
//////////////////////////////////////////////////////////////////////

/**
 * \brief This function creates a background widget using default values.
 * \ingroup gui2d_background
 *
 * \return A pointer to the newly created widget_background.
 */
struct widget_background *widget_background_create()
{
	struct widget_background *wb = MyMalloc(sizeof(struct widget_background));
	widget_init(WIDGET(wb));
	WIDGET(wb)->display = background_display;
	WIDGET(wb)->free = widget_background_free;

	dynarray_init(&wb->tiles, 2, sizeof(struct tile));
	return wb;
}

static void widget_background_free(struct widget *w)
{
	struct widget_background *wb = WIDGET_BACKGROUND(w);

	dynarray_free(&wb->tiles);

	widget_free(w);
}

/**
 * \brief Adds a new tile to a background widget.
 * \ingroup gui2d_background
 *
 * \details This function adds an image to the array of the images
 * displayed by the background widget.\n
 * The position of the image is an absolute position, expressed in screen
 * coordinates.\n
 * The image will be un-uniformly scaled (or repeated) to fit the desired width
 * and height.
 *
 * \param wb  Pointer to the widget_background object
 * \param img Pointer to the image to add to the background
 * \param x   Absolute position of the image along X axis
 * \param y   Absolute position of the image along Y axis
 * \param w   Display width of the background
 * \param h   Display height of the background
 * \param mode Transformation mode to apply if the image size is lower than the background size (scaled or tiled)
 *
 */
void widget_background_add(struct widget_background *wb, struct image *img, int x, int y, int w, int h, enum image_transformation_mode mode)
{
	struct tile tile;	
	tile.image = img;
	tile.mode = mode;
	Set_Rect(tile.rect, x, y, w, h);
	dynarray_add(&wb->tiles, &tile, sizeof(struct tile));		
}

void widget_background_clear(struct widget_background *wb)
{
	dynarray_free(&wb->tiles);
}

/**
 * \brief Add a 3x3 set of tiles to a background widget.
 * \ingroup gui2d_background
 *
 * \details This function loads a 3x3 set of tiles into a background widget and
 * scales the 'middle' tiles to fill the panel.\n
 * \n
 * \note The panel's size must be set before calling this function.
 * \par
 * \note The image files' names must use the following format: "<base_name>_i_j.png"
 * where i represents the row and j represents the column.
 * 
 * \param panel     A pointer to the widget_background object.
 * \param base_name The base name of the files from which the tiles are loaded.
 */
void widget_background_load_3x3_tiles(struct widget_background *panel, char *base_name)
{
	struct tile tile[3][3];

	// Load main body's background tiles' images.
	int i, j;
	char file[strlen(base_name) + 16];
	for (i = 0; i < 3; i++) {
		for (j = 0; j < 3; j++) {
			sprintf(file,"%s_%i_%i.png", base_name, i, j);

			tile[i][j].image = widget_load_image_resource(file, NO_MOD);
		}
	}
	int width = tile[0][0].image->w + tile[1][1].image->w + tile[2][2].image->w;
	int height = tile[0][0].image->h + tile[1][1].image->h + tile[2][2].image->h;

	// Check if the original tiles' size is bigger than the panel's size.
	if (width > WIDGET(panel)->rect.w || height > WIDGET(panel)->rect.h)
		error_message(__FUNCTION__,
			"The following tile set cannot be loaded: %s.\n"
			"(minimum required widget size: %i x %i - available widget size: %i x %i)\n\n"
			"You need to run the game in a higher resolution mode",
			PLEASE_INFORM | IS_FATAL,
			base_name, width, height, WIDGET(panel)->rect.w, WIDGET(panel)->rect.h);

	// Compute tiles position
	//		center_column_x
	//		     | right_column_x
	//		     |    |
	//		     v    v
	//		+----+----+----+
	//		|0,0 |0,1 |0,2 |
	//		+----+----+----+ <-- center_row_y  ^
	//		|1,0 |1,1 |1,2 |                   | center_row_h
	//		+----+----+----+ <-- bottom_row_y  v
	//		|2,0 |2,1 |2,2 |
	//		+----+----+----+
	//		     <---->
	//		    center_column_w

	// Fixed tiles
	int right_column_x = WIDGET(panel)->rect.x + WIDGET(panel)->rect.w - tile[0][2].image->w;
	int bottom_row_y = WIDGET(panel)->rect.y + WIDGET(panel)->rect.h - tile[2][0].image->h;

	Set_Rect(tile[0][0].rect, WIDGET(panel)->rect.x, WIDGET(panel)->rect.y, tile[0][0].image->w, tile[0][0].image->h);
	Set_Rect(tile[0][2].rect, right_column_x, WIDGET(panel)->rect.y, tile[0][2].image->w, tile[0][2].image->h);
	Set_Rect(tile[2][0].rect, WIDGET(panel)->rect.x, bottom_row_y, tile[2][0].image->w, tile[2][0].image->h);
	Set_Rect(tile[2][2].rect, right_column_x, bottom_row_y, tile[2][2].image->w, tile[2][2].image->h);

	// Variable size tiles 
	int center_column_x = tile[0][0].rect.x + tile[0][0].rect.w;
	int center_row_y = tile[0][0].rect.y + tile[0][0].rect.h;
	int center_column_w = right_column_x - WIDGET(panel)->rect.x - tile[0][0].rect.w;
	int center_row_h = bottom_row_y - center_row_y;

	Set_Rect(tile[0][1].rect, center_column_x, WIDGET(panel)->rect.y, center_column_w, tile[0][1].image->h);
	Set_Rect(tile[1][0].rect, WIDGET(panel)->rect.x, center_row_y, tile[1][0].image->w, center_row_h);
	Set_Rect(tile[1][1].rect, center_column_x, center_row_y, center_column_w, center_row_h); 
	Set_Rect(tile[1][2].rect, right_column_x, center_row_y, tile[1][2].image->w, center_row_h);
	Set_Rect(tile[2][1].rect, center_column_x, bottom_row_y, center_column_w, tile[2][1].image->h);

	// Add tiles to the background.
	for (i = 0; i < 3; i++) {
		for (j = 0; j < 3; j++) {
			widget_background_add(panel, (tile[i][j]).image, tile[i][j].rect.x, tile[i][j].rect.y, tile[i][j].rect.w, tile[i][j].rect.h, 0);
		}
	}
}
