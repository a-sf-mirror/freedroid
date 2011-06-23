/* 
 *
 *  Copyright (c) 2003 Johannes Prix
 *  Copyright (c) 2010 Arthur Huillet
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

#include "pngfuncs.h"
#include "../src/system.h"
#include "../src/defs.h"
#include "../src/getopt.h"
#include "../src/struct.h"
#include "../src/proto.h"

void Terminate(int exit_code, int save_config);

char *input_file;

Sint16 cooked_walk_object_phases;
Sint16 cooked_attack_object_phases;
Sint16 cooked_gethit_object_phases;
Sint16 cooked_death_object_phases;
Sint16 cooked_stand_object_phases;

int first_walk_animation_image;
int last_walk_animation_image;
int first_attack_animation_image;
int last_attack_animation_image;
int first_gethit_animation_image;
int last_gethit_animation_image;
int first_death_animation_image;
int last_death_animation_image;
int first_stand_animation_image;
int last_stand_animation_image;

#if SDL_BYTEORDER == SDL_LIL_ENDIAN
int rmask = 0x00FF0000;
int gmask = 0x0000FF00;
int bmask = 0x000000FF;
int amask = 0xFF000000;
#else
int rmask = 0x0000FF00;
int gmask = 0x00FF0000;
int bmask = 0xFF000000;
int amask = 0x000000FF;
#endif

SDL_Surface *Screen;

int extract_offset = 0;
int display = 0;
int debug_level = 0;
//--------------------
// Another dummy function, such that the (unused) parts of the
// text_public module will not cause undefined references...
//
void load_item_surfaces_for_item_type(int item_type)
{
};

void print_trace(int signum)
{
};

/* ----------------------------------------------------------------------
 * This function is used for terminating freedroid.  It will close
 * the SDL submodules and exit.
 * ---------------------------------------------------------------------- */
void Terminate(int exit_code, int save_config)
{
	printf("\n----------------------------------------------------------------------");
	printf("\nTermination of Gluem initiated...");

	printf("Thank you for using the FreedroidRPG Gluem Tool.\n\n");
	SDL_Quit();
	exit(exit_code);
}


static void parse_commandline(int argc, char *const argv[])
{
	int c;

	static struct option long_options[] = {
		{"version", 0, 0, 'v'},
		{"help", 0, 0, 'h'},
		{"extract-offset", 0, 0, 'o'},
		{0, 0, 0, 0}
	};

	while (1) {
		c = getopt_long(argc, argv, "odvh?i:", long_options, NULL);

		if (c == -1)
			break;

		switch (c) {
			case 'o':
				extract_offset = 1;
				break;

			case 'i':
				input_file = optarg;
				break;

			case 'v':
				exit(0);
				break;

			case 'd':
				display = 1;
				break;

			case 'h':
			case '?':
				fprintf(stderr, "Usage: %s [-o] [-h] [-d] -i input_file\n", argv[0]);
				exit(0);
				break;


			default:
				printf("\nOption %c unknown! Ignored.", c);
				break;
		}
	}
};				// void ParseCommandLine(...)

static void init_sdl(void)
{
	if (SDL_Init(SDL_INIT_VIDEO) == -1) {
		Terminate(EXIT_FAILURE, FALSE);
	}

	if (SDL_InitSubSystem(SDL_INIT_TIMER) == -1) {
		fprintf(stderr, "Couldn't initialize SDL: %s\n", SDL_GetError());
		Terminate(EXIT_FAILURE, FALSE);
	}

	if (display) {
		Screen = SDL_SetVideoMode(640, 480, 0, 0);
	}
	atexit(SDL_Quit);
}

static unsigned char *open_tux_image_archive_file(const char *fpath)
{
	FILE *DataFile;
	unsigned char *buf;
	int sz;

	if ((DataFile = fopen(fpath, "rb")) == NULL) {
		ErrorMessage(__FUNCTION__, "\
ungluem was unable to open tux image archive file %s.\n\
Please specify a valid file to open using -i on the commandline.", PLEASE_INFORM, IS_FATAL, fpath);
	} else {
	}

	if (!strstr(fpath, ".z")) {
		printf("This seems to be an uncompressed file.\n");
		buf = MyMalloc(FS_filelength(DataFile));

		if (fread(buf, FS_filelength(DataFile), 1, DataFile) != 1)
			ErrorMessage(__FUNCTION__, "ungluem was unable to read tux image archive file %s.", PLEASE_INFORM, IS_FATAL, fpath);
	} else {
		inflate_stream(DataFile, &buf, &sz);
	}

	fclose(DataFile);
	return buf;

}

static void get_string_for_phase(int p, char *out)
{
	const char *str = "unknown";
	int offset = 999;

	if (p >= first_walk_animation_image && p <= last_walk_animation_image) {
		str = "walk";
		offset = p - first_walk_animation_image;
	}
	
	if (p >= first_attack_animation_image && p <= last_attack_animation_image) {
		str = "attack";
		offset = p - first_attack_animation_image;
	}

	if (p >= first_gethit_animation_image && p <= last_gethit_animation_image) {
		str = "gethit";
		offset = p - first_gethit_animation_image;
	}

	if (p >= first_death_animation_image && p <= last_death_animation_image) {
		str = "death";
		offset = p - first_death_animation_image;
	}
	
	if (p >= first_stand_animation_image && p <= last_stand_animation_image) {
		str = "stand";
		offset = p - first_stand_animation_image;
	}

	if (offset == 999) 
		fprintf(stderr, "Unknown phase %d passed\n", p);
	sprintf(out, "%s-%02d", str, offset);
}

void endian_swap(char *pdata, size_t dsize, size_t nelements)
{
	unsigned int i, j, indx;
	char tempbyte;

	if (dsize <= 1)
		return;

	for (i = 0; i < nelements; i++) {
		indx = dsize;
		for (j = 0; j < dsize / 2; j++) {
			tempbyte = pdata[j];
			indx = indx - 1;
			pdata[j] = pdata[indx];
			pdata[indx] = tempbyte;
		}

		pdata = pdata + dsize;
	}

	return;

}				/* endian swap */

Sint16 ReadSint16(void *memory)
{
	Sint16 ret;

	memcpy(&ret, memory, sizeof(Sint16));
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
	endian_swap((char *)&ret, sizeof(Sint16), 1);
#endif

	return (ret);

}				/* ReadSint16() */

void flip_image_vertically(SDL_Surface * tmp1)
{
	SDL_LockSurface(tmp1);

	int nHH = tmp1->h >> 1;
	int nPitch = tmp1->pitch;

	unsigned char pBuf[nPitch + 1];
	unsigned char *pSrc = (unsigned char *)tmp1->pixels;
	unsigned char *pDst = (unsigned char *)tmp1->pixels + nPitch * (tmp1->h - 1);

	while (nHH--) {
		memcpy(pBuf, pSrc, nPitch);
		memcpy(pSrc, pDst, nPitch);
		memcpy(pDst, pBuf, nPitch);

		pSrc += nPitch;
		pDst -= nPitch;
	}

	SDL_UnlockSurface(tmp1);

};				// void flip_image_vertically ( SDL_Surface* tmp1 ) 

static void write_offset_file(const char *basename, Sint16 xoff, Sint16 yoff) 
{
	char name[1024];
	FILE *off;

	sprintf(name, "%s.offset", basename);

	off = fopen(name, "w");
	
	fprintf(off, "OffsetX=%d\nOffsetY=%d\nGraphicsFileName=%s\n", xoff, yoff, basename);

	fclose(off);

}

static void extract_and_write_file(const char *name, unsigned char **pos, int sdl)
{
	Sint16 img_xlen;
	Sint16 img_ylen;
	Sint16 img_x_offs;
	Sint16 img_y_offs;
	Sint16 orig_img_xlen;
	Sint16 orig_img_ylen;

	SDL_Surface *surf;

	unsigned char *ptr = *pos;

	img_xlen = ReadSint16(ptr);
	ptr += sizeof(Sint16);
	img_ylen = ReadSint16(ptr);
	ptr += sizeof(Sint16);
	img_x_offs = ReadSint16(ptr);
	ptr += sizeof(Sint16);
	img_y_offs = ReadSint16(ptr);
	ptr += sizeof(Sint16);

	if (!sdl) {
		orig_img_xlen = ReadSint16(ptr);
		ptr += sizeof(Sint16);
		orig_img_ylen = ReadSint16(ptr);
		ptr += sizeof(Sint16);
	}

	int len = 4 * img_xlen * img_ylen;
	char *tmpbuf = malloc(len);

	memcpy(tmpbuf, ptr, len);
	ptr += len;

	surf = SDL_CreateRGBSurfaceFrom(tmpbuf, img_xlen, img_ylen, 32, 4 * img_xlen,	rmask, gmask, bmask, amask);
	SDL_SetColorKey(surf, 0, 0);

	flip_image_vertically(surf);
	
	png_save_surface(name, surf);

	if (display) {
		surf = SDL_DisplayFormatAlpha(surf);
		SDL_FillRect(Screen, NULL, 0);
		SDL_BlitSurface(surf, NULL, Screen, NULL);
		SDL_Flip(Screen);
		SDL_Delay(40);
	}

	SDL_FreeSurface(surf);
	free(tmpbuf);

	if (extract_offset) {
		char basename[4096];
		strcpy(basename, name);
		char *ext = strstr(basename, ".png");
		if (ext)
			*ext = '\0';
		write_offset_file(basename, img_x_offs, img_y_offs);
	}

	*pos = ptr;
}

static void extract_tux_archive(unsigned char *ptr, int sdl)
{
	if (!sdl) {
		fprintf(stderr, "Non SDL-mode image archives of type Tux are not supported.\n");
		exit(1);
	}

	int rotation_index, our_phase;

	for (rotation_index = 0; rotation_index < MAX_TUX_DIRECTIONS; rotation_index++) {
		for (our_phase = 0; our_phase < TUX_TOTAL_PHASES; our_phase++) {
			char output_name[4096];

			sprintf(output_name, "tux_rot_%02d_phase_%02d.png", rotation_index, our_phase);
			extract_and_write_file(output_name, &ptr, sdl);
		}
	}
}

static void extract_enemy_archive(unsigned char *ptr, int sdl)
{
	first_walk_animation_image = 0;
	last_walk_animation_image = cooked_walk_object_phases - 1;
	first_attack_animation_image = last_walk_animation_image + 1;
	last_attack_animation_image = last_walk_animation_image + cooked_attack_object_phases;
	first_gethit_animation_image = last_attack_animation_image + 1;
	last_gethit_animation_image = last_attack_animation_image + cooked_gethit_object_phases;
	first_death_animation_image = last_gethit_animation_image + 1;
	last_death_animation_image = last_gethit_animation_image + cooked_death_object_phases;
	first_stand_animation_image = last_death_animation_image + 1;
	last_stand_animation_image = last_death_animation_image + cooked_stand_object_phases;

	int rotation_index;
	int enemy_phase;

	if (sdl) {
		printf("SDL mode for enemy archives not supported by ungluem (yet)\n");
	}

	printf("Walk phases: %d (%d -> %d)\nAttack phases: %d (%d -> %d)\nGethit phases: %d (%d -> %d)\nDeath phases: %d (%d -> %d)\nStand phases: %d (%d -> %d)\n",
		cooked_walk_object_phases, first_walk_animation_image, last_walk_animation_image, cooked_attack_object_phases, first_attack_animation_image, last_attack_animation_image,
		cooked_gethit_object_phases, first_gethit_animation_image, last_gethit_animation_image, cooked_death_object_phases, first_death_animation_image, last_death_animation_image,
		cooked_stand_object_phases, first_stand_animation_image, last_stand_animation_image);

	for (rotation_index = 0; rotation_index < ROTATION_ANGLES_PER_ROTATION_MODEL; rotation_index++) {
		for (enemy_phase = 0; enemy_phase <= last_stand_animation_image; enemy_phase++) {
			char phase_str[50];
			char output_name[4096];
			
			get_string_for_phase(enemy_phase, &phase_str[0]);

			sprintf(output_name, "enemy_rot_%02d_%s.png", rotation_index, phase_str);

		//	printf("Extracting %s at %p\n", output_name, ptr);
			extract_and_write_file(output_name, &ptr, sdl);
		}
	}
}

static void extract_archive(unsigned char *ptr)
{
	char archive_type_string[5] = { 0, 0, 0, 0, 0 };
	char ogl_support_string[5] = { 0, 0, 0, 0, 0 };
	int sdl_archive;
	
	memcpy(archive_type_string, ptr, 4);
	ptr += 4;
	memcpy(ogl_support_string, ptr, 4);
	ptr += 4;

	cooked_walk_object_phases = ReadSint16(ptr);
	ptr += sizeof(Sint16);
	cooked_attack_object_phases = ReadSint16(ptr);
	ptr += sizeof(Sint16);
	cooked_gethit_object_phases = ReadSint16(ptr);
	ptr += sizeof(Sint16);
	cooked_death_object_phases = ReadSint16(ptr);
	ptr += sizeof(Sint16);
	cooked_stand_object_phases = ReadSint16(ptr);
	ptr += sizeof(Sint16);

	if (!strncmp(ogl_support_string, "sdlX", 4)) {
		sdl_archive = 1;
	} else if (!strncmp(ogl_support_string, "oglX", 4)) {
		sdl_archive = 0;
	} else {
		fprintf(stderr, "Unknown OpenGL support string %s\n", ogl_support_string);
		exit(1);
	}

	if (!strncmp(archive_type_string, "tuxX", 4)) {
		extract_tux_archive(ptr, sdl_archive);
	} else if (!strncmp(archive_type_string, "eneX", 4)) {
		extract_enemy_archive(ptr, sdl_archive);
	} else {
		fprintf(stderr, "Unknown archive type string %s\n", archive_type_string);
		exit(1);
	}
}

int main(int argc, char *argv[])
{
	unsigned char *in;

	parse_commandline(argc, argv);

	init_sdl();

	in = open_tux_image_archive_file(input_file);

	extract_archive(in);

	return 0;

}
