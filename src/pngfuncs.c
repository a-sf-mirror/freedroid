/* SaveSurf: an example on how to save a SDLSurface in PNG
   Copyright (C) 2006 Angelo "Encelo" Theodorou
 
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
 
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
  
   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 
   NOTE: 
 
   This program is part of "Mars, Land of No Mercy" SDL examples, 
   you can find other examples on http://marsnomercy.org
*/

#include <stdlib.h>
#include <png.h>
#include <SDL.h>

enum pixelFormat {
	RGBA_PIXEL_FORMAT = 0,
	BGRA_PIXEL_FORMAT = 1
};

static enum pixelFormat surface_pixel_format(SDL_Surface *surface)
{
	// This defines BGRA pixel format
#if SDL_BYTEORDER == SDL_LIL_ENDIAN
	int rmask = 0x00FF0000;
	int gmask = 0x0000FF00;
	int bmask = 0x000000FF;
	//int amask = 0xFF000000;
#else
	int rmask = 0x0000FF00;
	int gmask = 0x00FF0000;
	int bmask = 0xFF000000;
	//int amask = 0x000000FF;
#endif

	if ((surface->format->Rmask == rmask) && (surface->format->Gmask == gmask) && (surface->format->Bmask == bmask))
		return BGRA_PIXEL_FORMAT;
	else
		return RGBA_PIXEL_FORMAT;
}

static int png_colortype_from_surface(SDL_Surface *surface)
{
	int colortype = PNG_COLOR_MASK_COLOR; /* grayscale not supported */

	if (surface->format->palette)
		colortype |= PNG_COLOR_MASK_PALETTE;
	else if (surface->format->Amask)
		colortype |= PNG_COLOR_MASK_ALPHA;
		
	return colortype;
}


void png_user_warn(png_structp ctx, png_const_charp str)
{
	fprintf(stderr, "libpng: warning: %s\n", str);
}


void png_user_error(png_structp ctx, png_const_charp str)
{
	fprintf(stderr, "libpng: error: %s\n", str);
}


int png_save_surface(const char *filename, SDL_Surface *surf)
{
	FILE *fp;
	png_structp png_ptr;
	png_infop info_ptr;
	int i, colortype;
	png_bytep *row_pointers;

	/* Opening output file */
	fp = fopen(filename, "wb");
	if (fp == NULL) {
		perror("fopen error");
		return -1;
	}

	/* Initializing png structures and callbacks */
	png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, 
		NULL, png_user_error, png_user_warn);
	if (png_ptr == NULL) {
		printf("png_create_write_struct error!\n");
		fclose(fp);
		return -1;
	}

	info_ptr = png_create_info_struct(png_ptr);
	if (info_ptr == NULL) {
		png_destroy_write_struct(&png_ptr, (png_infopp)NULL);
		printf("png_create_info_struct error!\n");
		fclose(fp);
		return -1;
	}

	if (setjmp(png_jmpbuf(png_ptr))) {
		png_destroy_write_struct(&png_ptr, &info_ptr);
		fclose(fp);
		return -1;
	}

	png_init_io(png_ptr, fp);

	colortype = png_colortype_from_surface(surf);
	png_set_IHDR(png_ptr, info_ptr, surf->w, surf->h, 8, colortype,	PNG_INTERLACE_NONE, 
		PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);

	/* Writing the image */
	png_write_info(png_ptr, info_ptr);
	png_set_packing(png_ptr);

    /* If the surface image is in BGR mode, then ask libpng to swap red and blue channels */
	if (surface_pixel_format(surf) == BGRA_PIXEL_FORMAT)
		png_set_bgr(png_ptr);

	/* Strip 4th byte */
	if (surf->format->BytesPerPixel == 4 && !surf->format->Amask)
		png_set_filler(png_ptr, 0, PNG_FILLER_AFTER);

	row_pointers = (png_bytep*) malloc(sizeof(png_bytep)*(size_t)(surf->h));
	for (i = 0; i < surf->h; i++)
		row_pointers[i] = (png_bytep)(Uint8 *)surf->pixels + i*surf->pitch;
	png_write_image(png_ptr, row_pointers);
	png_write_end(png_ptr, info_ptr);

	/* Cleaning out... */
	free(row_pointers);
	png_destroy_write_struct(&png_ptr, &info_ptr);
	fclose(fp);

	return 0;
}
