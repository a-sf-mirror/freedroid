#include <stdio.h>
#include <string.h>
#include "../src/pngfuncs.h"
#include "../src/system.h"
#include "../src/proto.h"

static void write_offset_file(const char *basename, int x_offset, int y_offset)
{
	char fpath[4096];
	sprintf(fpath, "%s.offset", basename);

	FILE *offset_file = fopen(fpath, "w");
	if (!offset_file) {
		fprintf(stderr, "Cannot create offset file: %s\n", fpath);
		return;
	}

	fprintf(offset_file, "OffsetX=%d\n", x_offset);
	fprintf(offset_file, "OffsetY=%d\n", y_offset);
	fclose(offset_file);
}

static void write_subimage(const char *fpath, SDL_Rect *subimage_rect, int x_offset, int y_offset, SDL_Surface *atlas_surface)
{
	SDL_PixelFormat *atlas_pf = atlas_surface->format;
	SDL_Surface *subimage = SDL_CreateRGBSurface(0, subimage_rect->w, subimage_rect->h, 32,
		atlas_pf->Rmask, atlas_pf->Gmask, atlas_pf->Bmask, atlas_pf->Amask);
	SDL_BlitSurface(atlas_surface, subimage_rect, subimage, NULL);

	png_save_surface(fpath, subimage);

	SDL_FreeSurface(subimage);

	// Avoid double extensions
	char basename[4096];
	strcpy(basename, fpath);
	char *ext = strstr(basename, ".png");
	if (ext)
		*ext = '\0';
	write_offset_file(basename, x_offset, y_offset);
}

static void explode_atlas_image(char **atlas_data_ptr, const char *image_filename, const char *output_directory)
{
	char *dptr = *atlas_data_ptr;

	SDL_Surface *atlas_image = IMG_Load(image_filename);
	if (!atlas_image) {
		fprintf(stderr, "Could not load atlas image: %s\n", image_filename);
		exit(1);
	}

	SDL_SetAlpha(atlas_image, 0, 0);

	while (*dptr && *dptr != '*') {
		SDL_Rect dest_rect;
		char element_filename[4096];
		strcpy(element_filename, output_directory);

		int x, y, w, h;
		int x_offset, y_offset;
		if (!sscanf(dptr, "%s %d %d %d %d off %d %d", &element_filename[strlen(output_directory)], &x, &y, &w, &h, &x_offset, &y_offset))
			break;

		dest_rect.x = x;
		dest_rect.y = y;
		dest_rect.w = w;
		dest_rect.h = h;

		write_subimage(element_filename, &dest_rect, x_offset, y_offset, atlas_image);

		while (*dptr && *dptr != '\n')
			dptr++;
		dptr++;
	}

	SDL_FreeSurface(atlas_image);
	*atlas_data_ptr = dptr;
}

char *read_file(const char *fpath)
{
	FILE *fd = fopen(fpath, "r");
	if (!fd) {
		fprintf(stderr, "Could not open file: %s\n", fpath);
		exit(1);
	}

	fseek(fd, 0, SEEK_END);
	long file_size = ftell(fd);
	fseek(fd, 0, SEEK_SET);

	char *file_content = malloc(file_size + 1);
	size_t items_read = fread(file_content, 1, file_size, fd);
	fclose(fd);
	file_content[file_size] = '\0';

	if (items_read != file_size) {
		fprintf(stderr, "Could not read file: %s\n", fpath);
		exit(1);
	}

	return file_content;
}

int main(int argc, char **argv)
{
	char *atlas_filename;
	char atlas_directory[4096];
	char output_directory[4096];

	if (argc < 4) {
		fprintf(stderr, "Usage: %s <atlas_text_file> <atlas_image_directory> <output_directory>\n", argv[0]);
		return 1;
	}

	atlas_filename = argv[1];
	sprintf(atlas_directory, "%s/", argv[2]);
	sprintf(output_directory, "%s/", argv[3]);

	char *atlas_data = read_file(atlas_filename);

	char *dptr = atlas_data;
	while (*dptr) {
		char atlas_path[4096];
		strcpy(atlas_path, atlas_directory);

		if (!sscanf(dptr, "* %s size %*d %*d", &atlas_path[strlen(atlas_directory)]))
			break;

		while (*dptr && *dptr != '\n')
			dptr++;
		dptr++;

		explode_atlas_image(&dptr, atlas_path, output_directory);
	}

	free(atlas_data);
	return 0;
}
