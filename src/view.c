/* 
 *
 *   Copyright (c) 1994, 2002, 2003, 2004 Johannes Prix
 *   Copyright (c) 1994, 2002 Reinhard Prix
 *   Copyright (c) 2004-2010 Arthur Huillet 
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
 * This file contains all the functions managing the things one gets to see.
 * That includes assembling of enemies, assembling the currently
 * relevant porting of the map (the bricks I mean), drawing all visible
 * elements like bullets, blasts, enemies or influencer in a not visible
 * place in memory at first, and finally drawing them to the visible
 * screen for the user.
 */

#define _view_c

#include "system.h"

#include "defs.h"
#include "struct.h"
#include "global.h"
#include "map.h"
#include "proto.h"
#include "SDL_rotozoom.h"

#include "lvledit/lvledit.h"
#include "lvledit/lvledit_display.h"
#include "lvledit/lvledit_tools.h"

#include <zlib.h>
#include <math.h>

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

char *part_group_strings[ALL_PART_GROUPS] = {
	"head/",
	"shield/",
	"torso/",
	"feet/",
	"weapon/",
	"weaponarm/"
};

iso_image loaded_tux_images[ALL_PART_GROUPS][TUX_TOTAL_PHASES][MAX_TUX_DIRECTIONS];

static int old_current_level = -1;

void PutRadialBlueSparks(float PosX, float PosY, float Radius, int SparkType, char active_directions[RADIAL_SPELL_DIRECTIONS], float age);
void insert_new_element_into_blitting_list(float new_element_norm, int new_element_type, void *new_element_pointer, int code_number);
static void show_inventory_screen(void);

EXTERN char *PrefixToFilename[ENEMY_ROTATION_MODELS_AVAILABLE];

struct blitting_list_element {
	int element_type;
	void *element_pointer;
	float norm_of_elements_position;
	int code_number;
	struct list_head node;
};

LIST_HEAD(blitting_list);

enum {
	BLITTING_TYPE_NONE = 0,
	BLITTING_TYPE_OBSTACLE = 1,
	BLITTING_TYPE_ENEMY = 2,
	BLITTING_TYPE_TUX = 3,
	BLITTING_TYPE_BULLET = 4,
	BLITTING_TYPE_BLAST = 5,
	BLITTING_TYPE_THROWN_ITEM = 6,
	BLITTING_TYPE_MOVE_CURSOR = 7
};

LIST_HEAD(visible_level_list);

/**
 * This function displays an item at the current mouse cursor position.
 * The typical crosshair cursor is assumed.  The item is centered around
 * this crosshair cursor, depending on item size.
 */
void DisplayItemImageAtMouseCursor(int ItemImageCode)
{
	SDL_Rect TargetRect;

	if (ItemImageCode == (-1)) {
		DebugPrintf(2, "\nCurrently no (-1 code) item held in hand.");
		return;
	}
	// We define the target location for the item.  This will be the current
	// mouse cursor position of course, but -16 for the crosshair center, 
	// which is somewhat (16) to the lower right of the cursor top left 
	// corner.
	//
	// And then of course we also have to take into account the size of the
	// item, wich is also not always the same.
	//
	TargetRect.x = GetMousePos_x() - ItemMap[ItemImageCode].inv_image.inv_size.x * 16;
	TargetRect.y = GetMousePos_y() - ItemMap[ItemImageCode].inv_image.inv_size.y * 16;

	// Do not move an item out of the screen
	if (TargetRect.x < 0)
		TargetRect.x = 0;
	
	if (TargetRect.y < 0)
		TargetRect.y = 0;

	our_SDL_blit_surface_wrapper(ItemMap[ItemImageCode].inv_image.Surface, NULL, Screen, &TargetRect);

};				// void DisplayItemImageAtMouseCursor( int ItemImageCode )

/**
 * This function displays (several) blinking warning signs as soon as item
 * durations reach critical (<5) duration level.
 */
static void ShowOneItemAlarm(item * AlarmItem, int Position)
{
	SDL_Rect TargetRect;
	int ItemImageCode;

	if (AlarmItem->type == (-1))
		return;
	if (AlarmItem->max_duration == (-1))
		return;

	ItemImageCode = AlarmItem->type;
	
	if (GameConfig.screen_width > 800) {
		TargetRect.x = GameConfig.screen_width / 2 + 160 - 64 * Position; // Centered
	} else {
		TargetRect.x = GameConfig.screen_width - GameConfig.screen_width * 100/640 - 64 * Position; // Right-aligned
	}
	TargetRect.y = GameConfig.screen_height * 390/480;

	if (AlarmItem->current_duration <= 5) {
		if (AlarmItem->current_duration < 3)
			if (((int)(Me.MissionTimeElapsed * 2)) % 2 == 1)
				return;
#ifdef HAVE_LIBGL
		if (use_open_gl) {
			glPixelTransferf(GL_BLUE_SCALE, 0);
			glPixelTransferf(GL_GREEN_SCALE, (float)(AlarmItem->current_duration - 1) / (4));
			glPixelTransferf(GL_RED_SCALE, 1);
		}
#endif
		our_SDL_blit_surface_wrapper(ItemMap[ItemImageCode].inv_image.Surface, NULL, Screen, &TargetRect);
#ifdef HAVE_LIBGL
		if (use_open_gl) {
			glPixelTransferf(GL_BLUE_SCALE, 1);
			glPixelTransferf(GL_GREEN_SCALE, 1);
			glPixelTransferf(GL_RED_SCALE, 1);
		}
#endif
	}
};				// void ShowOneItemAlarm( item* AlarmItem )

/**
 * This function displays (several) blinking warning signs as soon as item
 * durations reach critical (<5) duration level.
 */
void ShowItemAlarm(void)
{

	ShowOneItemAlarm(&Me.weapon_item, 1);
	ShowOneItemAlarm(&Me.shield_item, 2);
	ShowOneItemAlarm(&Me.special_item, 3);
	ShowOneItemAlarm(&Me.armour_item, 4);
	ShowOneItemAlarm(&Me.drive_item, 5);




};				// void ShowItemAlarm( void )

/**
 * Now it's time to blit all the spell effects.
 */
void PutMiscellaneousSpellEffects(void)
{
	int i;

	// Now we put all the spells in the list of active spells
	//
	for (i = 0; i < MAX_ACTIVE_SPELLS; i++) {
		if (AllActiveSpells[i].img_type == (-1))
			continue;
		PutRadialBlueSparks(AllActiveSpells[i].spell_center.x,
				    AllActiveSpells[i].spell_center.y,
				    AllActiveSpells[i].spell_radius, AllActiveSpells[i].img_type,
				    AllActiveSpells[i].active_directions, AllActiveSpells[i].spell_age);
	}

};				// void PutMiscellaneousSpellEffects ( void )

/**
 * Calculate the current FPS and append it to the specified auto_string.
 */
static void print_framerate(struct auto_string *txt)
{
	static float TimeSinceLastFPSUpdate = 10;
	static int Frames_Counted = 1;
	static int FPS_Displayed;

	if (GameConfig.Draw_Framerate) {
		TimeSinceLastFPSUpdate += Frame_Time();
		Frames_Counted++;
		if (Frames_Counted > 50) {
			FPS_Displayed = Frames_Counted / TimeSinceLastFPSUpdate;
			TimeSinceLastFPSUpdate = 0;
			Frames_Counted = 0;
		}
		autostr_append(txt, _("FPS: %d\n"), FPS_Displayed);
	}
}

/**
 * The combat window can also contain some text, like the FPS and our current
 * position.  This function puts those texts onto the screen.
 */
void ShowCombatScreenTexts(int mask)
{
	int minutes;
	int seconds;
	int i;
	int remaining_bots;
	static struct auto_string *txt;
	if (txt == NULL)
		txt = alloc_autostr(200);
	autostr_printf(txt, "");

	print_framerate(txt);

	for (i = 0; i < MAX_MISSIONS_IN_GAME; i++) {
		if (!Me.AllMissions[i].MissionWasAssigned)
			continue;

		if (Me.AllMissions[i].MustLiveTime != (-1)) {
			minutes = floor((Me.AllMissions[i].MustLiveTime - Me.MissionTimeElapsed) / 60);
			seconds = rintf(Me.AllMissions[i].MustLiveTime - Me.MissionTimeElapsed) - 60 * minutes;
			if (minutes < 0) {
				minutes = 0;
				seconds = 0;
			}
			autostr_append(txt, _("Time to hold out still: %2d:%2d\n"), minutes, seconds);
		}

		if ((Me.AllMissions[i].must_clear_first_level == Me.pos.z) || (Me.AllMissions[i].must_clear_second_level == Me.pos.z)) {
			remaining_bots = 0;

			enemy *erot, *nerot;
			BROWSE_ALIVE_BOTS_SAFE(erot, nerot) {
				if ((erot->pos.z == Me.pos.z) && (!is_friendly(erot->faction, FACTION_SELF)))
					remaining_bots++;

			}
			autostr_append(txt, _("Bots remaining on level: %d\n"), remaining_bots);
		}
	}

	SetCurrentFont(FPS_Display_BFont);
	DisplayText(txt->value, User_Rect.x + 1, User_Rect.y + 1, NULL, 1.0);

	DisplayBigScreenMessage();
}

static void get_floor_boundaries(int mask, int *LineStart, int *LineEnd, int *ColStart, int *ColEnd)
{
	float zf = lvledit_zoomfact();
	if (mask & ZOOM_OUT) {
		*LineStart = floor(Me.pos.y - (float)(FLOOR_TILES_VISIBLE_AROUND_TUX * zf));
		*LineEnd = floor(Me.pos.y + (float)(FLOOR_TILES_VISIBLE_AROUND_TUX * zf));
		*ColStart = floor(Me.pos.x - (float)(FLOOR_TILES_VISIBLE_AROUND_TUX * zf));
		*ColEnd = floor(Me.pos.x + (float)(FLOOR_TILES_VISIBLE_AROUND_TUX * zf));
	} else {
		*LineStart = floor(translate_pixel_to_map_location(UserCenter_x, -UserCenter_y, FALSE));
		*LineEnd =
		    floor(translate_pixel_to_map_location
			  (-UserCenter_x - iso_floor_tile_width + 1, UserCenter_y + iso_floor_tile_height - 1, FALSE));
		*ColStart = floor(translate_pixel_to_map_location(-UserCenter_x, -UserCenter_y, TRUE));
		*ColEnd =
		    floor(translate_pixel_to_map_location
			  (UserCenter_x + iso_floor_tile_width - 1, UserCenter_y + iso_floor_tile_height - 1, TRUE));
	}
}

void object_vtx_color(void *data, float *r, float *g, float *b)
{
	if (element_in_selection(data)) {
		*r = ((SDL_GetTicks() >> 7) % 3) / 2.0;
		*g = (((SDL_GetTicks() >> 7) + 1) % 3) / 2.0;
		*b = (((SDL_GetTicks() >> 7) + 2) % 3) / 2.0;
	} else {
		*r = 1.0;
		*g = 1.0;
		*b = 1.0;
	}
}

/**
 * This function displays floor on the screen.
 */
static void show_floor(int mask)
{
	int LineStart, LineEnd, ColStart, ColEnd, line, col, MapBrick;
	static int use_atlas = -1;
	float r, g, b;

	Level DisplayLevel = curShip.AllLevels[Me.pos.z];

	get_floor_boundaries(mask, &LineStart, &LineEnd, &ColStart, &ColEnd);

	//  SDL_SetClipRect (Screen, &User_Rect);

	if (!use_open_gl) {
		/* SDL rendering path */
		for (line = LineStart; line < LineEnd; line++) {
			for (col = ColStart; col < ColEnd; col++) {
				MapBrick = GetMapBrick(DisplayLevel, col, line);

				// @TODO : the current position can be on an other level than DisplayLevel, so
				// the following call is somehow wrong. To avoid transforming again the current
				// position (already done inside GetMapBrick()) we should concatenate GetMapBrick() and
				// object_vtx_color().
				object_vtx_color(&DisplayLevel->map[line][col], &r, &g, &b);

				if (mask & ZOOM_OUT)
					blit_zoomed_iso_image_to_map_position(&(floor_iso_images[MapBrick % ALL_ISOMETRIC_FLOOR_TILES]),
									      ((float)col) + 0.5, ((float)line) + 0.5);
				else
					blit_iso_image_to_map_position(&(floor_iso_images[MapBrick % ALL_ISOMETRIC_FLOOR_TILES]),
								       ((float)col) + 0.5, ((float)line) + 0.5);
			}
		}
		return;
	} else {
		if (use_atlas == -1) {
			//determine if we are using a texture atlas for the ground
#ifdef HAVE_LIBGL
			if (floor_iso_images[0].texture == floor_iso_images[5].texture)
				use_atlas = 1;
			else
				use_atlas = 0;
#else
			use_atlas = 0;
#endif
		}

		if (use_atlas) {

#ifdef HAVE_LIBGL
			glBindTexture(GL_TEXTURE_2D, floor_iso_images[0].texture);
			glEnable(GL_ALPHA_TEST);
			glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
			glBegin(GL_QUADS);

			for (line = LineStart; line < LineEnd; line++) {
				for (col = ColStart; col < ColEnd; col++) {
					MapBrick = GetMapBrick(DisplayLevel, col, line);
					object_vtx_color(&DisplayLevel->map[line][col], &r, &g, &b);

					iso_image *ourimg = &(floor_iso_images[MapBrick % ALL_ISOMETRIC_FLOOR_TILES]);

					int x, y;
					float zf = ((mask & ZOOM_OUT) ? lvledit_zoomfact_inv() : 1.0);

					translate_map_point_to_screen_pixel(((float)col) + 0.5, ((float)line) + 0.5, &x, &y);
					x += ourimg->offset_x * zf;
					y += ourimg->offset_y * zf;

					glColor3f(r, g, b);
					glTexCoord2f(ourimg->tx0, ourimg->ty1);
					glVertex2i(x, y);
					glTexCoord2f(ourimg->tx0, ourimg->ty0);
					glVertex2i(x, y + ourimg->original_image_height * zf);
					glTexCoord2f(ourimg->tx1, ourimg->ty0);
					glVertex2i(x + ourimg->original_image_width * zf, y + ourimg->original_image_height * zf);
					glTexCoord2f(ourimg->tx1, ourimg->ty1);
					glVertex2i(x + ourimg->original_image_width * zf, y);

				}
			}

			glEnd();
			glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
			glDisable(GL_ALPHA_TEST);
#endif
		}		//use_atlas
		else {
			for (line = LineStart; line < LineEnd; line++) {
				for (col = ColStart; col < ColEnd; col++) {
					MapBrick = GetMapBrick(DisplayLevel, col, line);

					object_vtx_color(&DisplayLevel->map[line][col], &r, &g, &b);

					draw_gl_textured_quad_at_map_position(&floor_iso_images[MapBrick % ALL_ISOMETRIC_FLOOR_TILES],
									      ((float)col) + 0.5, ((float)line) + 0.5, r, g, b, FALSE,
									      FALSE, (mask & ZOOM_OUT) ? lvledit_zoomfact_inv() : 1.0);

				}
			}

		}
	}

};

void blit_leveleditor_point(int x, int y)
{
	if (!use_open_gl)
		return;
#ifdef HAVE_LIBGL
	glDisable(GL_TEXTURE_2D);
	glEnable(GL_POINT_SMOOTH);
	glPointSize(5.0);
	glBegin(GL_POINTS);
	glColor3f(1.0, 0.0, 0.0);
	glVertex2i(x, y);
	glEnd();
	glDisable(GL_POINT_SMOOTH);
	glEnable(GL_TEXTURE_2D);
	glPointSize(1.0);
#endif
};

/**
 * More for debugging purposes than for real gameplay, we add some 
 * function to illustrate the collision rectangle of a certain obstacle
 * on the floor via a bright ugly distorted rectangular shape.
 */
void skew_and_blit_rect(float x1, float y1, float x2, float y2, Uint32 color)
{
	int r1, r2, r3, r4, c1, c2, c3, c4;
	translate_map_point_to_screen_pixel(x1, y1, &r1, &c1);
	translate_map_point_to_screen_pixel(x1, y2, &r2, &c2);
	translate_map_point_to_screen_pixel(x2, y2, &r3, &c3);
	translate_map_point_to_screen_pixel(x2, y1, &r4, &c4);
	blit_quad(r1, c1, r2, c2, r3, c3, r4, c4, color);
}

void blit_obstacle_collision_rectangle(obstacle * our_obstacle)
{
	float up, left, right, low, x, y;
	gps vpos;

	update_virtual_position(&vpos, &our_obstacle->pos, Me.pos.z);

	left = obstacle_map[our_obstacle->type].left_border;
	up = obstacle_map[our_obstacle->type].upper_border;
	low = obstacle_map[our_obstacle->type].lower_border;
	right = obstacle_map[our_obstacle->type].right_border;
	x = vpos.x;
	y = vpos.y;

	// If collision rectangles are turned off, then we need not do 
	// anything more here...
	//
	if (!draw_collision_rectangles)
		return;

	// If there is no collision rectangle to draw, we are done
	//
	if (obstacle_map[our_obstacle->type].block_area_type == COLLISION_TYPE_NONE)
		return;

	// Now we draw the collision rectangle.  We use the same parameters
	// of the obstacle spec, that are also used for the collision checks.
	skew_and_blit_rect(x + left, y + up, x + right, y + low, 0x00FEEAA);

	//    x1 = translate_map_point_to_screen_pixel_x ( x + up , y + left );
	//    y1 = translate_map_point_to_screen_pixel_y ( x + up , y + left );
	//    x2 = translate_map_point_to_screen_pixel_x ( x + up , y + right);
	//    y2 = translate_map_point_to_screen_pixel_y ( x + up , y + right);
	//    x3 = translate_map_point_to_screen_pixel_x ( x + low , y + right);
	//    y3 = translate_map_point_to_screen_pixel_y ( x + low , y + right);
	//    x4 = translate_map_point_to_screen_pixel_x ( x + low , y + left);
	//    y4 = translate_map_point_to_screen_pixel_y ( x + low , y + left);
	//    blit_quad ( x1, y1, x2, y2, x3, y3, x4, y4, 0x00FEEAA ); 
}				// void blit_obstacle_collision_rectangle ( obstacle* our_obstacle )

/**
 * Draw an obstacle at its place on the screen.
 *
 * @param our_obstacle Point to the obstacle to blit.
 */
void blit_one_obstacle(obstacle * our_obstacle, int highlight, int zoom)
{
#define HIGHLIGHT 1
#define NOHIGHLIGHT 0

	iso_image tmp;
	gps obs_screen_position;
	float zf = zoom ? lvledit_zoomfact_inv() : 1.0;

	if ((our_obstacle->type <= -1) || (our_obstacle->type >= NUMBER_OF_OBSTACLE_TYPES)) {
		ErrorMessage(__FUNCTION__, "The obstacle type %d that was given exceeds the number of\n\
				obstacle types allowed and loaded in Freedroid.", PLEASE_INFORM, IS_FATAL, our_obstacle->type);
	}
	// Maybe the children friendly version is desired.  Then the blood on the floor
	// will not be blitted to the screen.
	if ((!GameConfig.show_blood) && (our_obstacle->type >= ISO_BLOOD_1) && (our_obstacle->type <= ISO_BLOOD_8))
		return;

	if (zoom && !use_open_gl) {
		make_sure_zoomed_surface_is_there(get_obstacle_image(our_obstacle->type));
	}

	update_virtual_position(&obs_screen_position, &our_obstacle->pos, CURLEVEL()->levelnum);

	// We blit the obstacle in question, but if we're in the level editor and this
	// obstacle has been marked, we apply a color filter to it.  Otherwise we blit
	// it just so.
	if (element_in_selection(our_obstacle)) {
		if (use_open_gl) {
			draw_gl_textured_quad_at_map_position(get_obstacle_image(our_obstacle->type),
							      obs_screen_position.x, obs_screen_position.y,
							      ((SDL_GetTicks() >> 7) % 3) / 2.0,
							      (((SDL_GetTicks() >> 7) + 1) % 3) / 2.0,
							      (((SDL_GetTicks() >> 7) + 2) % 3) / 2.0, highlight, FALSE, zf);
		} else {
			DebugPrintf(1, "\nColor filter for level editor invoked (via SDL!) for marked obstacle!");
			tmp.surface = our_SDL_display_format_wrapperAlpha(get_obstacle_image(our_obstacle->type)->surface);
			tmp.surface->format->Bmask = 0x0;
			tmp.surface->format->Rmask = 0x0;
			tmp.surface->format->Gmask = 0x0FFFFFFFF;
			tmp.offset_x = get_obstacle_image(our_obstacle->type)->offset_x;
			tmp.offset_y = get_obstacle_image(our_obstacle->type)->offset_y;
			if (zoom) {
				tmp.zoomed_out_surface = NULL;
				blit_zoomed_iso_image_to_map_position(&(tmp), our_obstacle->pos.x, our_obstacle->pos.y);
				SDL_FreeSurface(tmp.zoomed_out_surface);
			}

			else {
				blit_iso_image_to_map_position(&tmp, obs_screen_position.x, obs_screen_position.y);
			}
			SDL_FreeSurface(tmp.surface);
		}
	} else {
		if (use_open_gl) {
			// Not in all cases does it make sense to make the walls transparent.
			// Only those walls, that are really blocking the Tux from view should
			// be made transparent.
			if (obstacle_map[our_obstacle->type].transparent == TRANSPARENCY_FOR_WALLS) {
				if ((obs_screen_position.x > Me.pos.x - 1.0) &&
				    (obs_screen_position.y > Me.pos.y - 1.0) &&
				    (obs_screen_position.x < Me.pos.x + 1.5) && (obs_screen_position.y < Me.pos.y + 1.5)) {
					draw_gl_textured_quad_at_map_position(get_obstacle_image(our_obstacle->type), obs_screen_position.x,
									      obs_screen_position.y, 1, 1, 1, highlight,
									      obstacle_map[our_obstacle->type].transparent, zf);

				} else {
					draw_gl_textured_quad_at_map_position(get_obstacle_image(our_obstacle->type), obs_screen_position.x,
									      obs_screen_position.y, 1, 1, 1, highlight, 0, zf);

				}
			} else {
				draw_gl_textured_quad_at_map_position(get_obstacle_image(our_obstacle->type), obs_screen_position.x,
								      obs_screen_position.y, 1, 1, 1, highlight,
								      obstacle_map[our_obstacle->type].transparent, zf);
			}
		} else {
			if (!zoom) {
				blit_iso_image_to_map_position(get_obstacle_image(our_obstacle->type),
							       obs_screen_position.x, obs_screen_position.y);
				if (highlight)
					sdl_highlight_iso_image(get_obstacle_image(our_obstacle->type),
										  our_obstacle->pos.x, our_obstacle->pos.y);
			} else {
				blit_zoomed_iso_image_to_map_position(get_obstacle_image(our_obstacle->type),
								      obs_screen_position.x, obs_screen_position.y);
			}

		}
	}
}

/**
 * In order for the obstacles to be blitted, they must first be inserted
 * into the correctly ordered list of objects to be blitted this frame.
 */
void insert_obstacles_into_blitting_list(int mask)
{
	int i;
	level *obstacle_level;
	int LineStart, LineEnd, ColStart, ColEnd, line, col;
	int px, py;
	int tstamp = next_glue_timestamp();

	obstacle *OurObstacle;
	gps tile_vpos, tile_rpos;
	gps virtpos, reference;

	get_floor_boundaries(mask, &LineStart, &LineEnd, &ColStart, &ColEnd);

	tile_vpos.z = Me.pos.z;

	for (line = LineStart; line < LineEnd; line++) {
		tile_vpos.y = line;

		for (col = ColStart; col < ColEnd; col++) {
			tile_vpos.x = col;
			if (!resolve_virtual_position(&tile_rpos, &tile_vpos)) {
				continue;
			}
			px = (int)rintf(tile_rpos.x);
			py = (int)rintf(tile_rpos.y);
			obstacle_level = curShip.AllLevels[tile_rpos.z];

			for (i = 0; i < obstacle_level->map[py][px].glued_obstacles.size; i++) {
					// Now we have to insert this obstacle.  We do this of course respecting
					// the blitting order, as always...
					int idx = ((int *)obstacle_level->map[py][px].glued_obstacles.arr)[i];
					OurObstacle = &obstacle_level->obstacle_list[idx];

					if (OurObstacle->timestamp == tstamp) {
						continue;
					}

					reference.x = OurObstacle->pos.x;
					reference.y = OurObstacle->pos.y;
					reference.z = tile_rpos.z;

					update_virtual_position(&virtpos, &reference, Me.pos.z);

					if ((int) virtpos.x != col)
						continue;

					if ((int) virtpos.y != line)
						continue;

					// Could not find virtual position? Give up drawing.
					if (virtpos.z == -1)
						continue;

					OurObstacle->timestamp = tstamp;

					insert_new_element_into_blitting_list(virtpos.x + virtpos.y, BLITTING_TYPE_OBSTACLE,
									      OurObstacle,
									      idx);
			}
		}
	}
}

/**
 * Several different things must be inserted into the blitting list.
 * Therefore this function is an abstraction, that will insert a generic
 * object into the blitting list.
 */
void insert_new_element_into_blitting_list(float new_element_norm, int new_element_type, void *new_element_pointer, int code_number)
{
	struct blitting_list_element *e, *n;
	struct blitting_list_element *newe;

	newe = MyMalloc(sizeof(struct blitting_list_element));
	newe->norm_of_elements_position = new_element_norm;
	newe->element_type = new_element_type;
	newe->element_pointer = new_element_pointer;
	newe->code_number = code_number;

	list_for_each_entry_safe(e, n, &blitting_list, node) {

		if (new_element_norm < e->norm_of_elements_position) {
			// Insert before this element
			list_add_tail(&newe->node, &e->node);
			return;
		}
	}

	// Reached the end of the list?
	list_add_tail(&newe->node, &blitting_list);
};				// void insert_new_element_into_blitting_list ( ... )

/**
 *
 *
 */
void insert_tux_into_blitting_list(void)
{
	float tux_norm = Me.pos.x + Me.pos.y;

	insert_new_element_into_blitting_list(tux_norm, BLITTING_TYPE_TUX, NULL, -1);

};				// void insert_tux_into_blitting_list ( void )

/**
 *
 *
 */
void insert_one_enemy_into_blitting_list(enemy * erot)
{
	float enemy_norm;

	enemy_norm = erot->virt_pos.x + erot->virt_pos.y;

	insert_new_element_into_blitting_list(enemy_norm, BLITTING_TYPE_ENEMY, erot, 0);

};				// void insert_one_enemy_into_blitting_list ( int enemy_num )

/**
 *
 *
 */
void insert_one_thrown_item_into_blitting_list(level *item_lvl, int item_num)
{
	float item_norm;
	item *CurItem = &item_lvl->ItemList[item_num];

	update_virtual_position(&CurItem->virt_pos, &CurItem->pos, Me.pos.z);

	item_norm = CurItem->virt_pos.x + CurItem->virt_pos.y;

	insert_new_element_into_blitting_list(item_norm, BLITTING_TYPE_THROWN_ITEM, CurItem, item_num);
};				// void insert_one_item_into_blitting_list ( int enemy_num )

/**
 *
 *
 */
void insert_one_bullet_into_blitting_list(int bullet_num)
{
	gps virtpos;

	// Due to the use of a painter algorithm, we need to sort the objects depending of their 
	// isometric distance on the current level.
	// We thus have to get the bullet's position on the current level. 
	update_virtual_position(&virtpos, &AllBullets[bullet_num].pos, Me.pos.z);

	// Could not find virtual position? Give up drawing.
	if (virtpos.z == -1)
		return;

	insert_new_element_into_blitting_list(virtpos.x + virtpos.y, BLITTING_TYPE_BULLET, &(AllBullets[bullet_num]), bullet_num);

};				// void insert_one_bullet_into_blitting_list ( int enemy_num )

/**
 *
 *
 */
void insert_one_blast_into_blitting_list(int blast_num)
{
	gps virtpos;

	// Due to the use of a painter algorithm, we need to sort the objects depending of their 
	// isometric distance on the current level.
	// We thus have to get the blast's position on the current level. 
	update_virtual_position(&virtpos, &AllBlasts[blast_num].pos, Me.pos.z);

	// Could not find virtual position? Give up drawing.
	if (virtpos.z == -1)
		return;

	insert_new_element_into_blitting_list(virtpos.x + virtpos.y, BLITTING_TYPE_BLAST, &(AllBlasts[blast_num]), blast_num);
}				// void insert_one_blast_into_blitting_list ( int enemy_num )

/**
 * 
 * 
 */
void insert_move_cursor_into_blitting_list()
{
	float norm;

	norm = Me.mouse_move_target.x + Me.mouse_move_target.y;

	insert_new_element_into_blitting_list(norm, BLITTING_TYPE_MOVE_CURSOR, NULL, 0);
};

/**
 * We need to display bots, objects, bullets... that are on the current level or on one of the
 * levels glued to this one.
 */
int level_is_visible(int level_num)
{
	// Current level is for sure visible

	if (level_num == Me.pos.z)
		return TRUE;

	struct visible_level *l, *n;
	BROWSE_VISIBLE_LEVELS(l, n) {
		if (l->lvl_pointer->levelnum == level_num)
			return TRUE;
	}

	return FALSE;

}				// int level_is_visible ( int level_num )

/**
 * Construct a linked list of visible levels.
 * Also compute the distance between Tux and each level boundaries, and fill
 * the lists of animated obstacles.
 */
void get_visible_levels()
{
	struct visible_level *e, *n;
	
	// Invalidate all entries in the visible_level list
	
	list_for_each_entry(e, &visible_level_list, node) {
		e->valid = FALSE;
	}

	// Find the 4 visible levels
	//
	// Those 4 levels form a square (eventually a degenerated one), one corner 
	// of the square being the current level.
	// The 2 corners are initialized to be on the current level (idx = 1), and 
	// we will extend one of them, depending on Tux's position.
	// (see gps_transform_map_init() main comment for an explanation about neighbor index)

	int left_idx = 1, right_idx = 1;
	int top_idx = 1, bottom_idx = 1;
	float left_or_right_distance = 0.0;	// distance to the left or right neighbor
	float top_or_bottom_distance = 0.0;	// distance to the top or bottom neighbor

	if (Me.pos.x < FLOOR_TILES_VISIBLE_AROUND_TUX) {
		// left neighbors are potentially visible
		left_idx = 0;
		left_or_right_distance = Me.pos.x;
	} else if (Me.pos.x >= CURLEVEL()->xlen - FLOOR_TILES_VISIBLE_AROUND_TUX) {
		// right neighbors are potentially visible
		right_idx = 2;
		left_or_right_distance = CURLEVEL()->xlen - Me.pos.x;
	}

	if (Me.pos.y < FLOOR_TILES_VISIBLE_AROUND_TUX) {
		// top neighbors are potentially visible
		top_idx = 0;
		top_or_bottom_distance = Me.pos.y;
	} else if (Me.pos.y >= CURLEVEL()->ylen - FLOOR_TILES_VISIBLE_AROUND_TUX) {
		// bottom neighbors are potentially visible
		bottom_idx = 2;
		top_or_bottom_distance = CURLEVEL()->ylen - Me.pos.y;
	}
	
	// Add or re-validate entries in the visible_level list

	int i, j;
	float latitude;		// distance, along Y axis, between Tux and the current neighbor
	float longitude;	// distance, along X axis, between Tux and the current neighbor

	for (j = top_idx; j <= bottom_idx; j++) {
		// if j==1, then current neighbor is at the same 'latitude' than Tux's level,
		// so latitude = 0.0
		latitude = (j == 1) ? 0.0 : top_or_bottom_distance;

		for (i = left_idx; i <= right_idx; i++) {
			// if i==1, then current neighbor is at the same 'longitude' than Tux's level,
			// so longitude = 0.0
			longitude = (i == 1) ? 0.0 : left_or_right_distance;

			if (level_neighbors_map[Me.pos.z][j][i]) {
				// if there is already an entry in the visible_level list for 
				// this level, update the entry and re-validate it
				int in_list = FALSE;
				list_for_each_entry(e, &visible_level_list, node) {
					if (e->lvl_pointer->levelnum == level_neighbors_map[Me.pos.z][j][i]->lvl_idx) {
						e->valid = TRUE;
						e->boundary_squared_dist = longitude * longitude + latitude * latitude;
						in_list = TRUE;
						break;
					}
				}
				// if there was no corresponding entry, create a new one
				if (!in_list) {
					e = MyMalloc(sizeof(struct visible_level));
					e->valid = TRUE;
					e->lvl_pointer = curShip.AllLevels[level_neighbors_map[Me.pos.z][j][i]->lvl_idx];
					e->boundary_squared_dist = longitude * longitude + latitude * latitude;
					e->animated_obstacles_dirty_flag = TRUE;
					INIT_LIST_HEAD(&e->animated_obstacles_list);
					list_add_tail(&e->node, &visible_level_list);
				}
			}
		}
	}
	
	// If the current level changed, remove useless invalid entries.
	// An entry is useless if the associated level is not a neighbor of the
	// current level. To find if two levels are connected, we look at the content
	// of 'gps_transform_matrix' (there is no gps transformation between two
	// unconnected levels).
	
	struct neighbor_data_cell *transform_data;
	
	if (old_current_level != Me.pos.z) {
		old_current_level = Me.pos.z;
		
		list_for_each_entry_safe(e, n, &visible_level_list, node) {
			if (e->valid)
				continue;
			transform_data = &gps_transform_matrix[Me.pos.z][e->lvl_pointer->levelnum];
			if (!transform_data->valid) {
				// useless entry: removing it from the linked list
				clear_animated_obstacle_lists(e);
				list_del(&e->node);
				free(e);
			}
		}
	}
}

/*
 * This function resets the visible levels list, as well as all animated
 * obstacle lists.
 */
void reset_visible_levels()
{
	struct visible_level *e, *n;
	
	// Clear current list
	list_for_each_entry_safe(e, n, &visible_level_list, node) {
		clear_animated_obstacle_lists(e);
		list_del(&e->node);
		free(e);
	}
	
	old_current_level = -1;
}

/*
 * Initialization of the 2 arrays used to accelerate gps transformations.
 * 
 * gps_transform_matrix[lvl1][lvl2] is a matrix used in update_virtual_position().
 * It contains the data needed to transform a gps position defined relatively to one level
 * into a gps position defined relatively to an other level.
 * gps_transform_matrix[lvl1][lvl2] is used for a transformation from 'lvl1' to 'lvl2'.
 * 
 * level_neighbors_map[lvl][idY][idX] is used to retrieve all the neighbors (and the corresponding 
 * transformation data) of one given level. It is used in resolve_virtual_position().
 * The (idY, idX) pair defines one of the nine neighbors, using the following schema:
 *              N
 *    (0,0) | (0,1) | (0,2)
 *   -------+-------+-------
 *  W (1,0) |       | (1,2) E
 *   -------+-------+-------
 *    (2,0) | (2,1) | (2,2)
 *              S
 */

// Some helper macros
#define NEIGHBOR_TRANSFORM_NW(lvl)   level_neighbors_map[lvl][0][0]
#define NEIGHBOR_TRANSFORM_N(lvl)    level_neighbors_map[lvl][0][1]
#define NEIGHBOR_TRANSFORM_NE(lvl)   level_neighbors_map[lvl][0][2]
#define NEIGHBOR_TRANSFORM_W(lvl)    level_neighbors_map[lvl][1][0]
#define NEIGHBOR_TRANSFORM_SELF(lvl) level_neighbors_map[lvl][1][1]
#define NEIGHBOR_TRANSFORM_E(lvl)    level_neighbors_map[lvl][1][2]
#define NEIGHBOR_TRANSFORM_SW(lvl)   level_neighbors_map[lvl][2][0]
#define NEIGHBOR_TRANSFORM_S(lvl)    level_neighbors_map[lvl][2][1]
#define NEIGHBOR_TRANSFORM_SE(lvl)   level_neighbors_map[lvl][2][2]

void gps_transform_map_init()
{
	int lvl_idx;
	int x, y;
	int ngb_idx, diag_idx;

	if (!gps_transform_map_dirty_flag)
		return;

	// Reset maps
	//

	for (lvl_idx = 0; lvl_idx < MAX_LEVELS; lvl_idx++) {
		for (ngb_idx = 0; ngb_idx < MAX_LEVELS; ngb_idx++) {
			gps_transform_matrix[lvl_idx][ngb_idx].delta_x = 0;
			gps_transform_matrix[lvl_idx][ngb_idx].delta_y = 0;
			gps_transform_matrix[lvl_idx][ngb_idx].lvl_idx = -1;
			gps_transform_matrix[lvl_idx][ngb_idx].valid = FALSE;
		}

		for (y = 0; y < 3; y++) {
			for (x = 0; x < 3; x++) {
				level_neighbors_map[lvl_idx][y][x] = NULL;
			}
		}
	}

	// Scan direct neighbors and fill maps
	//

	for (lvl_idx = 0; lvl_idx < MAX_LEVELS; lvl_idx++) {
		// Undefined level -> continue  
		if (!level_exists(lvl_idx))
			continue;

		// Self
		gps_transform_matrix[lvl_idx][lvl_idx].delta_x = 0;
		gps_transform_matrix[lvl_idx][lvl_idx].delta_y = 0;
		gps_transform_matrix[lvl_idx][lvl_idx].lvl_idx = lvl_idx;
		gps_transform_matrix[lvl_idx][lvl_idx].valid = TRUE;

		NEIGHBOR_TRANSFORM_SELF(lvl_idx) = &gps_transform_matrix[lvl_idx][lvl_idx];

		// North
		ngb_idx = curShip.AllLevels[lvl_idx]->jump_target_north;
		if (ngb_idx != -1) {
			gps_transform_matrix[lvl_idx][ngb_idx].delta_x = 0;
			gps_transform_matrix[lvl_idx][ngb_idx].delta_y = +curShip.AllLevels[ngb_idx]->ylen;
			gps_transform_matrix[lvl_idx][ngb_idx].lvl_idx = ngb_idx;
			gps_transform_matrix[lvl_idx][ngb_idx].valid = TRUE;

			NEIGHBOR_TRANSFORM_N(lvl_idx) = &gps_transform_matrix[lvl_idx][ngb_idx];
		}
		// South
		ngb_idx = curShip.AllLevels[lvl_idx]->jump_target_south;
		if (ngb_idx != -1) {
			gps_transform_matrix[lvl_idx][ngb_idx].delta_x = 0;
			gps_transform_matrix[lvl_idx][ngb_idx].delta_y = -curShip.AllLevels[lvl_idx]->ylen;
			gps_transform_matrix[lvl_idx][ngb_idx].lvl_idx = ngb_idx;
			gps_transform_matrix[lvl_idx][ngb_idx].valid = TRUE;

			NEIGHBOR_TRANSFORM_S(lvl_idx) = &gps_transform_matrix[lvl_idx][ngb_idx];
		}
		// East
		ngb_idx = curShip.AllLevels[lvl_idx]->jump_target_east;
		if (ngb_idx != -1) {
			gps_transform_matrix[lvl_idx][ngb_idx].delta_x = -curShip.AllLevels[lvl_idx]->xlen;
			gps_transform_matrix[lvl_idx][ngb_idx].delta_y = 0;
			gps_transform_matrix[lvl_idx][ngb_idx].lvl_idx = ngb_idx;
			gps_transform_matrix[lvl_idx][ngb_idx].valid = TRUE;

			NEIGHBOR_TRANSFORM_E(lvl_idx) = &gps_transform_matrix[lvl_idx][ngb_idx];
		}
		// West
		ngb_idx = curShip.AllLevels[lvl_idx]->jump_target_west;
		if (ngb_idx != -1) {
			gps_transform_matrix[lvl_idx][ngb_idx].delta_x = +curShip.AllLevels[ngb_idx]->xlen;
			gps_transform_matrix[lvl_idx][ngb_idx].delta_y = 0;
			gps_transform_matrix[lvl_idx][ngb_idx].lvl_idx = ngb_idx;
			gps_transform_matrix[lvl_idx][ngb_idx].valid = TRUE;

			NEIGHBOR_TRANSFORM_W(lvl_idx) = &gps_transform_matrix[lvl_idx][ngb_idx];
		}
	}

	// Scan diagonal levels and fill maps
	//
	// Now that we know the direct neighbors, we can use that knowledge to ease
	// the finding of the diagonal levels.
	// The main difficulty is that the north-west neighbor, for example, can be
	// the west neighbor of the north neighbor or the north neighbor of the west neighbor.
	// On a 4-connected corner (4 levels around a corner), the two cases are equivalent.
	// However, on a 3-connected corner, one of the two cases is invalid. We thus have to
	// try the 2 ways to reach each diagonal levels.
	//

	for (lvl_idx = 0; lvl_idx < MAX_LEVELS; lvl_idx++) {
		// North-West neighbor.
		if (NEIGHBOR_TRANSFORM_N(lvl_idx) && NEIGHBOR_TRANSFORM_W(NEIGHBOR_ID_N(lvl_idx))) {
			diag_idx = NEIGHBOR_ID_W(NEIGHBOR_ID_N(lvl_idx));
			gps_transform_matrix[lvl_idx][diag_idx].delta_x =
			    NEIGHBOR_TRANSFORM_N(lvl_idx)->delta_x + NEIGHBOR_TRANSFORM_W(NEIGHBOR_ID_N(lvl_idx))->delta_x;
			gps_transform_matrix[lvl_idx][diag_idx].delta_y =
			    NEIGHBOR_TRANSFORM_N(lvl_idx)->delta_y + NEIGHBOR_TRANSFORM_W(NEIGHBOR_ID_N(lvl_idx))->delta_y;
			gps_transform_matrix[lvl_idx][diag_idx].lvl_idx = diag_idx;
			gps_transform_matrix[lvl_idx][diag_idx].valid = TRUE;

			NEIGHBOR_TRANSFORM_NW(lvl_idx) = &gps_transform_matrix[lvl_idx][diag_idx];
		}
		// North-East neighbor.
		if (NEIGHBOR_TRANSFORM_N(lvl_idx) && NEIGHBOR_TRANSFORM_E(NEIGHBOR_ID_N(lvl_idx))) {
			diag_idx = NEIGHBOR_ID_E(NEIGHBOR_ID_N(lvl_idx));
			gps_transform_matrix[lvl_idx][diag_idx].delta_x =
			    NEIGHBOR_TRANSFORM_N(lvl_idx)->delta_x + NEIGHBOR_TRANSFORM_E(NEIGHBOR_ID_N(lvl_idx))->delta_x;
			gps_transform_matrix[lvl_idx][diag_idx].delta_y =
			    NEIGHBOR_TRANSFORM_N(lvl_idx)->delta_y + NEIGHBOR_TRANSFORM_E(NEIGHBOR_ID_N(lvl_idx))->delta_y;
			gps_transform_matrix[lvl_idx][diag_idx].lvl_idx = diag_idx;
			gps_transform_matrix[lvl_idx][diag_idx].valid = TRUE;

			NEIGHBOR_TRANSFORM_NE(lvl_idx) = &gps_transform_matrix[lvl_idx][diag_idx];
		}
		// South-West neighbor.
		if (NEIGHBOR_TRANSFORM_S(lvl_idx) && NEIGHBOR_TRANSFORM_W(NEIGHBOR_ID_S(lvl_idx))) {
			diag_idx = NEIGHBOR_ID_W(NEIGHBOR_ID_S(lvl_idx));
			gps_transform_matrix[lvl_idx][diag_idx].delta_x =
			    NEIGHBOR_TRANSFORM_S(lvl_idx)->delta_x + NEIGHBOR_TRANSFORM_W(NEIGHBOR_ID_S(lvl_idx))->delta_x;
			gps_transform_matrix[lvl_idx][diag_idx].delta_y =
			    NEIGHBOR_TRANSFORM_S(lvl_idx)->delta_y + NEIGHBOR_TRANSFORM_W(NEIGHBOR_ID_S(lvl_idx))->delta_y;
			gps_transform_matrix[lvl_idx][diag_idx].lvl_idx = diag_idx;
			gps_transform_matrix[lvl_idx][diag_idx].valid = TRUE;

			NEIGHBOR_TRANSFORM_SW(lvl_idx) = &gps_transform_matrix[lvl_idx][diag_idx];
		}
		// South-East neighbor.
		if (NEIGHBOR_TRANSFORM_S(lvl_idx) && NEIGHBOR_TRANSFORM_E(NEIGHBOR_ID_S(lvl_idx))) {
			diag_idx = NEIGHBOR_ID_E(NEIGHBOR_ID_S(lvl_idx));
			gps_transform_matrix[lvl_idx][diag_idx].delta_x =
			    NEIGHBOR_TRANSFORM_S(lvl_idx)->delta_x + NEIGHBOR_TRANSFORM_E(NEIGHBOR_ID_S(lvl_idx))->delta_x;
			gps_transform_matrix[lvl_idx][diag_idx].delta_y =
			    NEIGHBOR_TRANSFORM_S(lvl_idx)->delta_y + NEIGHBOR_TRANSFORM_E(NEIGHBOR_ID_S(lvl_idx))->delta_y;
			gps_transform_matrix[lvl_idx][diag_idx].lvl_idx = diag_idx;
			gps_transform_matrix[lvl_idx][diag_idx].valid = TRUE;

			NEIGHBOR_TRANSFORM_SE(lvl_idx) = &gps_transform_matrix[lvl_idx][diag_idx];
		}
		// West-North neighbor, if needed (i.e. if north-west neighbor was not found).
		if (!NEIGHBOR_TRANSFORM_NW(lvl_idx)) {
			if (NEIGHBOR_TRANSFORM_W(lvl_idx) && NEIGHBOR_TRANSFORM_N(NEIGHBOR_ID_W(lvl_idx))) {
				diag_idx = NEIGHBOR_ID_N(NEIGHBOR_ID_W(lvl_idx));
				gps_transform_matrix[lvl_idx][diag_idx].delta_x =
				    NEIGHBOR_TRANSFORM_W(lvl_idx)->delta_x + NEIGHBOR_TRANSFORM_N(NEIGHBOR_ID_W(lvl_idx))->delta_x;
				gps_transform_matrix[lvl_idx][diag_idx].delta_y =
				    NEIGHBOR_TRANSFORM_W(lvl_idx)->delta_y + NEIGHBOR_TRANSFORM_N(NEIGHBOR_ID_W(lvl_idx))->delta_y;
				gps_transform_matrix[lvl_idx][diag_idx].lvl_idx = diag_idx;
				gps_transform_matrix[lvl_idx][diag_idx].valid = TRUE;

				NEIGHBOR_TRANSFORM_NW(lvl_idx) = &gps_transform_matrix[lvl_idx][diag_idx];
			}
		}
		// West-South neighbor, if needed (i.e. if south-west neighbor was not found).
		if (!NEIGHBOR_TRANSFORM_SW(lvl_idx)) {
			if (NEIGHBOR_TRANSFORM_W(lvl_idx) && NEIGHBOR_TRANSFORM_S(NEIGHBOR_ID_W(lvl_idx))) {
				diag_idx = NEIGHBOR_ID_S(NEIGHBOR_ID_W(lvl_idx));
				gps_transform_matrix[lvl_idx][diag_idx].delta_x =
				    NEIGHBOR_TRANSFORM_W(lvl_idx)->delta_x + NEIGHBOR_TRANSFORM_S(NEIGHBOR_ID_W(lvl_idx))->delta_x;
				gps_transform_matrix[lvl_idx][diag_idx].delta_y =
				    NEIGHBOR_TRANSFORM_W(lvl_idx)->delta_y + NEIGHBOR_TRANSFORM_S(NEIGHBOR_ID_W(lvl_idx))->delta_y;
				gps_transform_matrix[lvl_idx][diag_idx].lvl_idx = diag_idx;
				gps_transform_matrix[lvl_idx][diag_idx].valid = TRUE;

				NEIGHBOR_TRANSFORM_SW(lvl_idx) = &gps_transform_matrix[lvl_idx][diag_idx];
			}
		}
		// East-North neighbor, if needed (i.e. if north-east neighbor was not found).
		if (!NEIGHBOR_TRANSFORM_NE(lvl_idx)) {
			if (NEIGHBOR_TRANSFORM_E(lvl_idx) && NEIGHBOR_TRANSFORM_N(NEIGHBOR_ID_E(lvl_idx))) {
				diag_idx = NEIGHBOR_ID_N(NEIGHBOR_ID_E(lvl_idx));
				gps_transform_matrix[lvl_idx][diag_idx].delta_x =
				    NEIGHBOR_TRANSFORM_E(lvl_idx)->delta_x + NEIGHBOR_TRANSFORM_N(NEIGHBOR_ID_E(lvl_idx))->delta_x;
				gps_transform_matrix[lvl_idx][diag_idx].delta_y =
				    NEIGHBOR_TRANSFORM_E(lvl_idx)->delta_y + NEIGHBOR_TRANSFORM_N(NEIGHBOR_ID_E(lvl_idx))->delta_y;
				gps_transform_matrix[lvl_idx][diag_idx].lvl_idx = diag_idx;
				gps_transform_matrix[lvl_idx][diag_idx].valid = TRUE;

				NEIGHBOR_TRANSFORM_NE(lvl_idx) = &gps_transform_matrix[lvl_idx][diag_idx];
			}
		}
		// East-South neighbor, if needed (i.e. if south-east neighbor was not found).
		if (!NEIGHBOR_TRANSFORM_SE(lvl_idx)) {
			if (NEIGHBOR_TRANSFORM_E(lvl_idx) && NEIGHBOR_TRANSFORM_S(NEIGHBOR_ID_E(lvl_idx))) {
				diag_idx = NEIGHBOR_ID_S(NEIGHBOR_ID_E(lvl_idx));
				gps_transform_matrix[lvl_idx][diag_idx].delta_x =
				    NEIGHBOR_TRANSFORM_E(lvl_idx)->delta_x + NEIGHBOR_TRANSFORM_S(NEIGHBOR_ID_E(lvl_idx))->delta_x;
				gps_transform_matrix[lvl_idx][diag_idx].delta_y =
				    NEIGHBOR_TRANSFORM_E(lvl_idx)->delta_y + NEIGHBOR_TRANSFORM_S(NEIGHBOR_ID_E(lvl_idx))->delta_y;
				gps_transform_matrix[lvl_idx][diag_idx].lvl_idx = diag_idx;
				gps_transform_matrix[lvl_idx][diag_idx].valid = TRUE;

				NEIGHBOR_TRANSFORM_SE(lvl_idx) = &gps_transform_matrix[lvl_idx][diag_idx];
			}
		}
	}

	gps_transform_map_dirty_flag = FALSE;
}

/**
 * There are several cases where an object or a character (Tux or a bot)
 * could become visible or active when they are technically still not on the
 * current level.
 * 
 * Therefore we introduce 'virtual' positions, i.e. the position the object 
 * would have, if the object were in fact counted as part of a neighboring level,
 * mostly the level of the Tux.  Using this concept, we can more easily compute 
 * distances and compare positions.
 *
 * This function is an abstract approach to this problem, working with
 * the 'gps' notion.
 *
 */
void update_virtual_position(gps * target_pos, gps * source_pos, int level_num)
{
	// The case where the position in question is already directly on 
	// the virtual level, things are really simple and we can quit
	// almost immediately...
	//
	if (source_pos->z == level_num) {
		target_pos->x = source_pos->x;
		target_pos->y = source_pos->y;
		target_pos->z = source_pos->z;
		return;
	}
	// Transform the gps position
	//
	if (source_pos->z < 0 || source_pos->z >= sizeof(gps_transform_matrix)/sizeof(gps_transform_matrix[0]) ||
		level_num < 0 || level_num >= sizeof(gps_transform_matrix[0])/sizeof(gps_transform_matrix[0][0])) {
		ErrorMessage(__FUNCTION__, "Virtual position update was required for level %d relative to level %d - one of those are incorrect level numbers.\n", PLEASE_INFORM, IS_WARNING_ONLY, source_pos->z, level_num);
		target_pos->x = (-1);
		target_pos->y = (-1);
		target_pos->z = (-1);
		return;
	}

	struct neighbor_data_cell *ngb_data = &gps_transform_matrix[source_pos->z][level_num];

	if (ngb_data->valid) {
		target_pos->x = source_pos->x + ngb_data->delta_x;
		target_pos->y = source_pos->y + ngb_data->delta_y;
		target_pos->z = level_num;
		return;
	}
	// The gps position cannot be expressed in terms of the virtual level.
	// That means we'll best 'erase' the virtual positions, so that
	// no 'phantoms' will occur...
	//
	target_pos->x = (-1);
	target_pos->y = (-1);
	target_pos->z = (-1);

}				// void update_virtual_position ( gps* target_pos , gps* source_pos , int level_num )

/*
 * Transform a virtual position, defined in the 'lvl' coordinate system, into
 * its real level and position
 * 
 * Note: this function only works if the real position is one of the 8 neighbors of 'lvl'.
 * If not, the function returns FALSE. 
 * (a recursive call could be used to remove this limitation)
 */
int resolve_virtual_position(gps *rpos, gps *vpos)
{
	int valid = FALSE;
	level *lvl;

	// Check pre-conditions

	if (vpos->z == -1) {
		ErrorMessage(__FUNCTION__, "Resolve virtual position was called with an invalid virtual position (%f:%f:%d).\n", PLEASE_INFORM, IS_WARNING_ONLY, vpos->x, vpos->y, vpos->z);
		print_trace(0);
		rpos->x = vpos->x;
		rpos->y = vpos->y;
		rpos->z = vpos->z;
		return FALSE;
	}

	// Get the gps transformation data cell, according to virtual position value

	lvl = curShip.AllLevels[vpos->z];
	int idX = NEIGHBOR_IDX(vpos->x, lvl->xlen);
	int idY = NEIGHBOR_IDX(vpos->y, lvl->ylen);

	// If we don't have to transform the position, return immediately

	if (idX == 1 && idY == 1) {
		rpos->x = vpos->x;
		rpos->y = vpos->y;
		rpos->z = vpos->z;
		return TRUE;
	}

	// Do the transformation

	struct neighbor_data_cell *ngb_data = level_neighbors_map[vpos->z][idY][idX];

	if (ngb_data && ngb_data->valid) {
		rpos->x = vpos->x + ngb_data->delta_x;
		rpos->y = vpos->y + ngb_data->delta_y;
		rpos->z = ngb_data->lvl_idx;

		// Check that the transformed position is valid (i.e. inside level boundaries)
		level *rlvl = curShip.AllLevels[rpos->z];
		valid = pos_inside_level(rpos->x, rpos->y, rlvl);
	}

	if (!valid) {
		rpos->x = vpos->x;
		rpos->y = vpos->y;
		rpos->z = vpos->z;
		return FALSE;
	}

	return TRUE;
}

/**
 * Check if a position is inside a level's boundaries
 * 
 * return TRUE if '(x,y)' is inside 'lvl'
 */
int pos_inside_level(float x, float y, level * lvl)
{
	return ((x >= 0) && (x < (float)lvl->xlen) && (y >= 0) && (y < (float)lvl->ylen));
}				// pos_inside_level()

/**
 * Check if a position is inside or near a level's boundaries
 * 
 * return TRUE if (x,y) is inside 'lvl' or at less than 'dist' from
 * 'lvl' borders.
 */
int pos_near_level(float x, float y, level * lvl, float dist)
{
	return ((x >= -dist) && (x < (float)lvl->xlen + dist) && (y >= -dist) && (y < (float)lvl->ylen + dist));
}				// pos_near_level()

/**
 * The blitting list must contain the enemies too.  This function is 
 * responsible for inserting the enemies at the right positions.
 */
void insert_enemies_into_blitting_list(void)
{
	int i;
	enemy *ThisRobot;

	// Now that we plan to also show bots on other levels, we must be
	// a bit more general and proceed through all the levels...
	//
	// Those levels not in question will be filtered out anyway inside
	// the loop...
	//

	for (i = 0; i < 2; i++) {
		list_for_each_entry(ThisRobot, (i) ? &dead_bots_head : &alive_bots_head, global_list) {
			if (!level_is_visible(ThisRobot->pos.z))
				continue;

			// We update the virtual position of this bot, such that we can handle it 
			// with easier expressions later...
			//
			update_virtual_position(&(ThisRobot->virt_pos), &(ThisRobot->pos), Me.pos.z);

			if (fabsf(ThisRobot->virt_pos.x - Me.pos.x) > FLOOR_TILES_VISIBLE_AROUND_TUX + FLOOR_TILES_VISIBLE_AROUND_TUX)
				continue;
			if (fabsf(ThisRobot->virt_pos.y - Me.pos.y) > FLOOR_TILES_VISIBLE_AROUND_TUX + FLOOR_TILES_VISIBLE_AROUND_TUX)
				continue;

			insert_one_enemy_into_blitting_list(ThisRobot);
		}
	}

};				// void insert_enemies_into_blitting_list ( void )

/**
 *
 *
 */
void insert_bullets_into_blitting_list(void)
{
	int i;

	for (i = 0; i < MAXBULLETS; i++) {
		if (AllBullets[i].type != INFOUT)
			insert_one_bullet_into_blitting_list(i);
	}

};				// void insert_bullets_into_blitting_list ( void )

/**
 *
 *
 */
void insert_blasts_into_blitting_list(void)
{
	int i;

	for (i = 0; i < MAXBLASTS; i++) {
		if (AllBlasts[i].type != INFOUT)
			insert_one_blast_into_blitting_list(i);
	}

};				// void insert_enemies_into_blitting_list ( void )

/**
 *
 *
 */
void insert_thrown_items_into_blitting_list(void)
{
	int i;
	struct visible_level *vis_lvl, *n;
	
	BROWSE_VISIBLE_LEVELS(vis_lvl, n) {
		level *lvl = vis_lvl->lvl_pointer;
		for (i = 0; i < MAX_ITEMS_PER_LEVEL; i++) {
			if (lvl->ItemList[i].type != -1)
				insert_one_thrown_item_into_blitting_list(vis_lvl->lvl_pointer, i);
		}
	}
};				// void insert_enemies_into_blitting_list ( void )

/**
 * In isometric viewpoint setting, we need to respect visibility when
 * considering the order of things to blit.  Therefore we will first set
 * up a list of the things to be blitted for this frame.  Then we can
 * later use this list to fill in objects into the picture, automatically
 * having the right order.
 */
void set_up_ordered_blitting_list(int mask)
{
	struct blitting_list_element *e, *n;
	list_for_each_entry_safe(e, n, &blitting_list, node) {
		list_del(&e->node);
		free(e);
	}

	// Now we can start to fill in the obstacles around the
	// tux...
	//
	insert_obstacles_into_blitting_list(mask);

	insert_tux_into_blitting_list();

	insert_enemies_into_blitting_list();

	insert_bullets_into_blitting_list();

	insert_blasts_into_blitting_list();

	insert_move_cursor_into_blitting_list();

	insert_thrown_items_into_blitting_list();

};				// void set_up_ordered_blitting_list ( void )

static void show_obstacle(int mask, obstacle * o, int code_number)
{
	level *obst_lvl;


	// Safety checks
	if ((o->type <= -1) || (o->type >= NUMBER_OF_OBSTACLE_TYPES)) {
		ErrorMessage(__FUNCTION__, "The blitting list contained an illegal obstacle type %d.", PLEASE_INFORM, IS_FATAL, o->type);
	}

	if (!(mask & OMIT_OBSTACLES)) {
		if (mask & ZOOM_OUT) {
			blit_one_obstacle(o, NOHIGHLIGHT, ZOOM_OUT);
		} else {
			if (code_number == clickable_obstacle_below_mouse_cursor(&obst_lvl)) {
				blit_one_obstacle(o, HIGHLIGHT, !ZOOM_OUT);
			} else {
				// Do not blit "transp for water" obstacle when not in leveleditor mode
				if (game_status != INSIDE_LVLEDITOR && o->type == ISO_TRANSP_FOR_WATER)
					return;

				// Normal display
				blit_one_obstacle(o, NOHIGHLIGHT, !ZOOM_OUT);
			}
		}
	}
}

/**
 * Now that the blitting list has finally been assembled, we can start to
 * blit all the objects according to the blitting list set up.
 */
void blit_preput_objects_according_to_blitting_list(int mask)
{
	obstacle *our_obstacle = NULL;
	int item_under_cursor = -1;
	level *item_under_cursor_lvl = NULL;
	
	struct blitting_list_element *e, *n;
	list_for_each_entry_safe(e, n, &blitting_list, node) {
		switch (e->element_type) {
		case BLITTING_TYPE_OBSTACLE:
			// We do some sanity checking for illegal obstacle types.
			// Can't hurt to do that so as to be on the safe side.
			//
			if ((((obstacle *) e->element_pointer)->type <= -1) ||
			    ((obstacle *) e->element_pointer)->type >= NUMBER_OF_OBSTACLE_TYPES) {
				ErrorMessage(__FUNCTION__,
					     "The blitting list contained an illegal obstacle type %d, for obstacle at coordinates %f %f. Doing nothing.", PLEASE_INFORM, IS_WARNING_ONLY, 
						 ((obstacle *) e->element_pointer)->type, ((obstacle *) e->element_pointer)->pos.x, ((obstacle *) e->element_pointer)->pos.y);
				break;

			}

			our_obstacle = e->element_pointer;

			// If the obstacle has a shadow, it seems like now would be a good time
			// to blit it.
			//
			if (!GameConfig.skip_shadow_blitting) {
				gps vpos;
				update_virtual_position(&vpos, &our_obstacle->pos, Me.pos.z);
				if (use_open_gl) {
					if (obstacle_map[our_obstacle->type].shadow_image.texture_has_been_created) {
						if (mask & ZOOM_OUT) {
							draw_gl_textured_quad_at_map_position(&obstacle_map
											      [our_obstacle->type].shadow_image,
											      vpos.x, vpos.y,
											      1.0, 1.0, 1.0, FALSE,
											      TRANSPARENCY_FOR_SEE_THROUGH_OBJECTS,
											      lvledit_zoomfact_inv());
						} else {
							draw_gl_textured_quad_at_map_position(&obstacle_map
											      [our_obstacle->type].shadow_image,
											      vpos.x, vpos.y,
											      1.0, 1.0, 1.0, FALSE,
											      TRANSPARENCY_FOR_SEE_THROUGH_OBJECTS, 1.0);
						}
					}
				} else {
					if (obstacle_map[our_obstacle->type].shadow_image.surface != NULL) {
						if (mask & ZOOM_OUT) {
							blit_zoomed_iso_image_to_map_position(&
											      (obstacle_map
											       [our_obstacle->type].shadow_image),
											      vpos.x, vpos.y);
						} else {
							blit_iso_image_to_map_position(&obstacle_map[our_obstacle->type].shadow_image,
										       vpos.x, vpos.y);
						}
						// DebugPrintf ( -4 , "\n%s(): shadow has been drawn." , __FUNCTION__ );
					}
				}
			}
			// If the obstacle in question does have a collision rectangle, then we
			// draw that on the floor now.
			//
			blit_obstacle_collision_rectangle(our_obstacle);

			// Draw the obstacle by itself if it is a preput obstacle
			//
			if (obstacle_map[((obstacle *) e->element_pointer)->type].flags & NEEDS_PRE_PUT)
				show_obstacle(mask, ((obstacle *) e->element_pointer), e->code_number);
			break;
		
		case BLITTING_TYPE_ENEMY:
			// Enemies, which are dead already become like decoration on the floor.  
			// They should never hide the Tux, so we blit them beforehand and not
			// again later from the list.
			//
			if (((enemy *)e->element_pointer)->animation_type == DEATH_ANIMATION
			    || ((enemy *)e->element_pointer)->animation_type == DEAD_ANIMATION) {
				if (!(mask & OMIT_ENEMIES)) {
					PutEnemy((enemy *) (e->element_pointer), -1, -1, mask, FALSE);
				}
			}
			break;
			
		case BLITTING_TYPE_THROWN_ITEM:
			if (mask & SHOW_ITEMS) {
				// Preput thrown items when they are on the floor (i.e. not during the
				// throwing animation)
				item *the_item = (item*)e->element_pointer;
				item_under_cursor_lvl = NULL;
				if (the_item->throw_time <= 0) {
					item_under_cursor = get_floor_item_index_under_mouse_cursor(&item_under_cursor_lvl);
					if (item_under_cursor != -1 && item_under_cursor_lvl != NULL &&
						the_item->pos.z == item_under_cursor_lvl->levelnum && item_under_cursor == e->code_number)
						PutItem(the_item, e->code_number, mask, PUT_NO_THROWN_ITEMS, TRUE);
					else
						PutItem(the_item, e->code_number, mask, PUT_NO_THROWN_ITEMS, FALSE);				
				}
			}
			break;
			
		case BLITTING_TYPE_MOVE_CURSOR:
			PutMouseMoveCursor();
			break;
		}
	}

}				// void blit_preput_objects_according_to_blitting_list ( ... )

/**
 * Now that the blitting list has finally been assembled, we can start to
 * blit all the objects according to the blitting list set up.
 */
void blit_nonpreput_objects_according_to_blitting_list(int mask)
{
	enemy *enemy_under_cursor = NULL;
	int item_under_cursor = -1;
	level *item_under_cursor_lvl = NULL;
	struct blitting_list_element *e, *n;

	// We memorize which 'enemy' is currently under the mouse target, so that we
	// can properly highlight this enemy...
	//
	enemy_under_cursor = GetLivingDroidBelowMouseCursor();
	item_under_cursor = get_floor_item_index_under_mouse_cursor(&item_under_cursor_lvl);

	// Now it's time to blit all the elements from the list...
	//
	list_for_each_entry_safe(e, n, &blitting_list, node) {

		if (e->element_type == BLITTING_TYPE_NONE)
			break;
		switch (e->element_type) {
		case BLITTING_TYPE_OBSTACLE:
			// Skip preput obstacles
			if (obstacle_map[((obstacle*)(e->element_pointer))->type].flags & NEEDS_PRE_PUT)
				continue;
			show_obstacle(mask, e->element_pointer, e->code_number);
			break;
		case BLITTING_TYPE_TUX:
			if (!(mask & OMIT_TUX)) {
				if (Me.energy > 0)
					blit_tux(-1, -1);
			}
			break;
		case BLITTING_TYPE_ENEMY:
			if (!(mask & OMIT_ENEMIES)) {
				if (((enemy *) e->element_pointer)->energy < 0)
					continue;
				if (((enemy *) e->element_pointer)->animation_type == DEATH_ANIMATION)
					continue;
				if (((enemy *) e->element_pointer)->animation_type == DEAD_ANIMATION)
					continue;

				// A droid can either be rendered in normal mode or in highlighted
				// mode, depending in whether the mouse cursor is right over it or not.
				//
				if (e->element_pointer == enemy_under_cursor)
					PutEnemy((enemy *) e->element_pointer, -1, -1, mask, TRUE);
				else
					PutEnemy((enemy *) e->element_pointer, -1, -1, mask, FALSE);
			}
			break;
		case BLITTING_TYPE_BULLET:
			// DebugPrintf ( -1000 , "Bullet code_number: %d. " , blitting_list [ i ] . code_number );
			PutBullet(e->code_number, mask);
			break;
		case BLITTING_TYPE_BLAST:
			if (!(mask & OMIT_BLASTS))
				PutBlast(e->code_number);
			break;
		case BLITTING_TYPE_THROWN_ITEM:
			{
				item *the_item = (item*)e->element_pointer;
				if (the_item->throw_time > 0) {
					if (item_under_cursor != -1 && item_under_cursor == e->code_number
						&& item_under_cursor_lvl->levelnum == the_item->pos.z)
						PutItem(the_item, e->code_number, mask, PUT_ONLY_THROWN_ITEMS, TRUE);
					else
						PutItem(the_item, e->code_number, mask, PUT_ONLY_THROWN_ITEMS, FALSE);
				}
			}
			// DebugPrintf ( -1 , "\nThrown item now blitted..." );
			break;
		case BLITTING_TYPE_MOVE_CURSOR:
			break;
		default:
			ErrorMessage(__FUNCTION__, "\
						The blitting list contained an illegal blitting object type.", PLEASE_INFORM, IS_FATAL);
			break;
		}
	}

};				// void blit_nonpreput_objects_according_to_blitting_list ( ... )

static void show_obstacle_labels(int mask)
{
	int i;
	level *l = CURLEVEL();

	if (game_status != INSIDE_LVLEDITOR) {
		// Don't show obstacles labels when we are not in the editor
		return;
	}

	if (mask & OMIT_OBSTACLES) {
		// Don't show obstacles labels when obstacles are not displayed on the map
		return;
	}

	for (i = 0; i < l->obstacle_extensions.size; i++) {
		struct obstacle_extension *ext = &ACCESS_OBSTACLE_EXTENSION(l->obstacle_extensions, i);
		
		if (ext->type == OBSTACLE_EXTENSION_LABEL) {
			show_backgrounded_label_at_map_position(ext->data,
					0, ext->obs->pos.x,
					ext->obs->pos.y, mask & ZOOM_OUT);
		}
	}
}

/**
 * Each item is lying on the floor.  But that means some of the items,
 * especially the smaller but not necessary less valuable items will not
 * be easy to make out under all the bushed, trees, rubble and stuff.
 * So the solution is to offer a special key that when pressed will make
 * all item names flash up, so that you can't possibly miss an item that
 * you're standing close to.
 *
 * This function blits all the item names to the screen on the exact
 * positions that have been computed before (hopefully!) in other 
 * functions like update_item_text_slot_positions ( ... ) or so.
 */
static void blit_all_item_slots(int mask)
{
	int i;
	struct visible_level *vis_lvl, *n;
	
	if (mask & OMIT_ITEMS_LABEL) {
		// Do not show item slots
		return;
	}

	BROWSE_VISIBLE_LEVELS(vis_lvl, n) {
		
		level *item_level = vis_lvl->lvl_pointer;

		for (i = 0; i < MAX_ITEMS_PER_LEVEL; i++) {
			// We don't work with unused item slots...
			//
			if (item_level->ItemList[i].type == (-1))
				continue;
	
			// Now we check if the cursor is on that slot, because then the
			// background of the slot will be highlighted...
			//
			if (MouseCursorIsInRect(&(item_level->ItemList[i].text_slot_rectangle), GetMousePos_x(), GetMousePos_y()))
				our_SDL_fill_rect_wrapper(Screen, &(item_level->ItemList[i].text_slot_rectangle),
							  SDL_MapRGB(Screen->format, 0x000, 0x000, 0x099));
			else {
				if (use_open_gl) {
					if ((item_level->ItemList[i].text_slot_rectangle.x + item_level->ItemList[i].text_slot_rectangle.w <= 0) ||
						(item_level->ItemList[i].text_slot_rectangle.y + item_level->ItemList[i].text_slot_rectangle.h <= 0) ||
						(item_level->ItemList[i].text_slot_rectangle.x >= GameConfig.screen_width) ||
						(item_level->ItemList[i].text_slot_rectangle.y >= GameConfig.screen_height))
						continue;
					gl_draw_rectangle(&item_level->ItemList[i].text_slot_rectangle, 0, 0, 0,
								  BACKGROUND_TEXT_RECT_ALPHA);
				} else {
					SDL_Rect our_rect = item_level->ItemList[i].text_slot_rectangle;	//we need that because SDL_FillRect modifies the dstrect
					our_SDL_fill_rect_wrapper(Screen, &(our_rect), SDL_MapRGB(Screen->format, 0x000, 0x000, 0x000));
				}
			}
	
			// Finally it's time to insert the font into the item slot.  We
			// use the item name, but currently font color is not adapted for
			// special item properties...
			//
			PutStringFont(Screen, FPS_Display_BFont, item_level->ItemList[i].text_slot_rectangle.x,
					  item_level->ItemList[i].text_slot_rectangle.y, D_(ItemMap[item_level->ItemList[i].type].item_name));
	
		}
	}
};				// void blit_all_item_slots ( void )

/**
 *
 *
 */
int item_slot_position_blocked(item * given_item, int item_slot)
{
	int i;
	item *cur_item;
	struct visible_level *vis_lvl, *n;
	int item_level_reached = FALSE;
	int last_slot_to_check;
	
	// We will browse all visible levels, until we reach the item's level.
	// For each browsed level, we check against all items.
	// But, on the item's level, we stop when the current item is reached.
	
	BROWSE_VISIBLE_LEVELS(vis_lvl, n) {
		
		level *item_level = vis_lvl->lvl_pointer;

		if (item_level->levelnum == given_item->pos.z) {
			item_level_reached = TRUE;
			last_slot_to_check = item_slot;
		} else {
			last_slot_to_check = MAX_ITEMS_PER_LEVEL;
		}
		
		for (i = 0; i < last_slot_to_check + 1; i++) {
			cur_item = &(item_level->ItemList[i]);
	
			if (cur_item->type == (-1))
				continue;
	
			if (MouseCursorIsInRect(&(cur_item->text_slot_rectangle),
						given_item->text_slot_rectangle.x, given_item->text_slot_rectangle.y)) {
				return (TRUE);
			}
			if (MouseCursorIsInRect(&(cur_item->text_slot_rectangle),
						given_item->text_slot_rectangle.x,
						given_item->text_slot_rectangle.y + FontHeight(FPS_Display_BFont))) {
				return (TRUE);
			}
			if (MouseCursorIsInRect(&(cur_item->text_slot_rectangle),
						given_item->text_slot_rectangle.x +
						given_item->text_slot_rectangle.w, given_item->text_slot_rectangle.y)) {
				return (TRUE);
			}
			if (MouseCursorIsInRect(&(cur_item->text_slot_rectangle),
						given_item->text_slot_rectangle.x +
						given_item->text_slot_rectangle.w,
						given_item->text_slot_rectangle.y + FontHeight(FPS_Display_BFont))) {
				return (TRUE);
			}
			if (MouseCursorIsInRect(&(cur_item->text_slot_rectangle),
						given_item->text_slot_rectangle.x +
						given_item->text_slot_rectangle.w / 2, given_item->text_slot_rectangle.y)) {
				return (TRUE);
			}
			if (MouseCursorIsInRect(&(cur_item->text_slot_rectangle),
						given_item->text_slot_rectangle.x +
						given_item->text_slot_rectangle.w / 2,
						given_item->text_slot_rectangle.y + FontHeight(FPS_Display_BFont))) {
				return (TRUE);
			}
		}
		
		if (item_level_reached) 
			break;
	}
	
	return (FALSE);
};				// void item_slot_position_blocked ( int x , int y , int last_slot_to_check )

/**
 * Each item is lying on the floor.  But that means some of the items,
 * especially the smaller but not necessarily less valuable items will not
 * be easy to make out under all the bushed, trees, rubble and stuff.
 * So the solution is to offer a special key that when pressed will make
 * all item names flash up, so that you can't possibly miss an item that
 * you're standing close to.
 *
 * This function computes the best rectangles and positions for such 
 * item names to flash up.
 */
void update_item_text_slot_positions(void)
{
	int i;
	BFont_Info *BFont_to_use = FPS_Display_BFont;
	item *cur_item;
	struct visible_level *vis_lvl, *n;
	
	BROWSE_VISIBLE_LEVELS(vis_lvl, n) {
		
		level *item_level = vis_lvl->lvl_pointer;
	
		for (i = 0; i < MAX_ITEMS_PER_LEVEL; i++) {
			cur_item = &(item_level->ItemList[i]);
	
			if (cur_item->type == (-1))
				continue;
	
			// We try to use a text rectangle that is close to the
			// actual item...
			//
			update_virtual_position(&cur_item->virt_pos, &cur_item->pos, Me.pos.z);
			cur_item->text_slot_rectangle.h = FontHeight(BFont_to_use);
			cur_item->text_slot_rectangle.w = TextWidthFont(BFont_to_use, D_(ItemMap[cur_item->type].item_name));
			cur_item->text_slot_rectangle.x =
				translate_map_point_to_screen_pixel_x(cur_item->virt_pos.x, cur_item->virt_pos.y) - cur_item->text_slot_rectangle.w / 2;
			cur_item->text_slot_rectangle.y =
				translate_map_point_to_screen_pixel_y(cur_item->virt_pos.x, cur_item->virt_pos.y) - cur_item->text_slot_rectangle.h / 2;
	
			// But maybe the situation is already very crowded, i.e. maybe there are
			// already (a lot of) items there with slot positions conflicting...
			// Well, what to do?  If there is already an item there, we try to escape,
			// that's it.
			// We will however only try a given amount of times, to protect against
			// an "infinite" loop.
			if ((item_slot_position_blocked(cur_item, i - 1))) {
				int max_tries = 10;
				while (max_tries >= 0) {
					if (i % 2)
						cur_item->text_slot_rectangle.y += cur_item->text_slot_rectangle.h + 2;
					else
						cur_item->text_slot_rectangle.y -= cur_item->text_slot_rectangle.h + 2;

					if (!item_slot_position_blocked(cur_item, i - 1))
						break;
				    	
					// Maybe just a hundred left or right would also do...  but if it
					// doesn't, we'll undo the changes made.
					//
					Sint16 tmp = cur_item->text_slot_rectangle.x;
					
					cur_item->text_slot_rectangle.x += 100;
					if (!item_slot_position_blocked(cur_item, i - 1))
						break;
					
					cur_item->text_slot_rectangle.x = tmp - 100;
					if (!item_slot_position_blocked(cur_item, i - 1))
						break;
					
					cur_item->text_slot_rectangle.x = tmp;
					max_tries--;
				}
			}
		}
	}
};				// void update_item_text_slot_positions ( void )

static void draw_line_at_map_position(float x1, float y1, float x2, float y2, Uint32 color, int thickness)
{
	int c1, c2, r1, r2;

	translate_map_point_to_screen_pixel(x1, y1, &r1, &c1);
	translate_map_point_to_screen_pixel(x2, y2, &r2, &c2);

	draw_line(r1, c1, r2, c2, color, thickness);
}

void draw_grid_on_the_floor(int mask)
{
	if (!(draw_grid && (mask & SHOW_GRID)))
		return;

	int LineStart, LineEnd, ColStart, ColEnd;
	float x, y;
	Level our_level = curShip.AllLevels[Me.pos.z];

	get_floor_boundaries(mask, &LineStart, &LineEnd, &ColStart, &ColEnd);

	x = rintf(Me.pos.x + 0.5);
	y = rintf(Me.pos.y + 0.5);

	float dd;

	if (draw_grid >= 2) {	// large grid
		// Draw the horizontal lines
		for (dd = 0; dd <= our_level->ylen; dd++) {
			draw_line_at_map_position(0, dd, our_level->xlen, dd, 0x99FFFF, 1);	// light cyan
 		}

		// Draw the vertical lines
		for (dd = 0; dd <= our_level->xlen; dd++) {
			draw_line_at_map_position(dd, 0, dd, our_level->ylen, 0x99FFFF, 1);	// light cyan
		}
	}

	for (dd = 0; dd <= 1; dd += .5)	// quick-placement grid
	{
		draw_line_at_map_position(x - 1.5, y - dd, x + 0.5, y - dd, 0xFF00FF, 1);	// magenta
		draw_line_at_map_position(x - dd, y - 1.5, x - dd, y + 0.5, 0xFF00FF, 1);	// magenta
	}

	// display numbers, corresponding to the numpad keys for quick placing 
	BFont_Info *PreviousFont;
	PreviousFont = GetCurrentFont();
	SetCurrentFont(Messagevar_BFont);
	char *numbers[2][2] = { {"3", "9"}, {"1", "7"} };
	int ii, jj;
	for (ii = 0; ii <= 1; ii++)
		for (jj = 0; jj <= 1; jj++) {
			float xx, yy;
			int r, c;
			xx = x - ii;
			yy = y - jj;
			translate_map_point_to_screen_pixel(xx, yy, &r, &c);
			SDL_Rect tr;
			tr.x = r - 7;
			tr.y = c - 7;
			tr.w = 12;
			tr.h = 14;

			our_SDL_fill_rect_wrapper(Screen, &tr, 0x000000);
			DisplayText(numbers[ii][jj], r - 5, c - 5, &tr, TEXT_STRETCH);
		}
	SetCurrentFont(PreviousFont);

	// now display the level borders (red line)	
	draw_line_at_map_position(0, 0, 0, our_level->ylen, 0xFF0000, 3);
	draw_line_at_map_position(our_level->xlen, 0, our_level->xlen, our_level->ylen, 0xFF0000, 3);
	draw_line_at_map_position(0, 0, our_level->xlen, 0, 0xFF0000, 3);
	draw_line_at_map_position(0, our_level->ylen, our_level->xlen, our_level->ylen, 0xFF0000, 3);
}

/* -----------------------------------------------------------------
 * This function assembles the contents of the combat window 
 * in Screen.
 *
 * Several FLAGS can be used to control its behavior:
 *
 * (*) ONLY_SHOW_MAP = 1:  This flag indicates not do draw any
 *     game elements but the map blocks
 *
 * (*) DO_SCREEN_UPDATE = 2: This flag indicates for the function
 *     to also cause an SDL_Update of the portion of the screen
 *     that has been modified
 *
 * (*) ONLY_SHOW_MAP_AND_TEXT = 4: This flag indicates, that only
 *     the map and also info like the current coordinate position
 *     should be entered into the Screen.  This flag is mainly
 *     used for the level editor.
 *
 * ----------------------------------------------------------------- */
void AssembleCombatPicture(int mask)
{
	// We generate a list of obstacles (and other stuff) that might
	// emit some light.  It should be sufficient to establish this
	// list once in the code and the to use it for all light computations
	// of this frame.
	//
	update_light_list();

	show_floor(mask);

	draw_grid_on_the_floor(mask);

	set_up_ordered_blitting_list(mask);

	blit_preput_objects_according_to_blitting_list(mask);
	
	blit_nonpreput_objects_according_to_blitting_list(mask);

	if ((!GameConfig.skip_light_radius) && (!(mask & SKIP_LIGHT_RADIUS)))
		blit_light_radius();

	PutMiscellaneousSpellEffects();

	if (mask & ONLY_SHOW_MAP) {
		// in case we only draw the map, we are done here.  But
		// of course we must check if we should update the screen too.
		if (mask & DO_SCREEN_UPDATE)
			our_SDL_update_rect_wrapper(Screen, 0, 0, Screen->w, Screen->h);

		return;
	}

	show_obstacle_labels(mask);

	display_automap();

	ShowCombatScreenTexts(mask);

	if (XPressed() || GameConfig.show_item_labels) {
		update_item_text_slot_positions();
		blit_all_item_slots(mask);
	}
	// Here are some more things, that are not needed in the level editor
	// view...
	//
	if (!(mask & ONLY_SHOW_MAP_AND_TEXT)) {
		ShowItemAlarm();
		blit_special_background(HUD_BACKGROUND_CODE);
		show_text_widget(&message_log);
		if (!GameOver) {
			ShowCurrentHealthAndForceLevel();
			ShowCurrentSkill();
			ShowCurrentWeapon();
		}
		show_quick_inventory();
		ShowCharacterScreen();
		ShowSkillsScreen();
		show_addon_crafting_ui();
		show_item_upgrade_ui();
		show_inventory_screen();
		DisplayButtons();
		if (!GameOver)
			DisplayBanner();
	}

	if (GameConfig.Inventory_Visible || GameConfig.skill_explanation_screen_visible) {
		User_Rect.x = 320;
	} else
		User_Rect.x = 0;

	if (GameConfig.CharacterScreen_Visible || GameConfig.SkillScreen_Visible) {
		User_Rect.w = GameConfig.screen_width - 320 - User_Rect.x;
	} else {
		User_Rect.w = GameConfig.screen_width - User_Rect.x;
	}

	if (!(mask & NO_CURSOR))
		blit_our_own_mouse_cursor();

#if 0
	/* This code displays the player tracks with red dots. */
	glDisable(GL_TEXTURE_2D);
	glPointSize(2.0);
	glBegin(GL_POINTS);
	i = 0;
	for (; i < MAX_INFLU_POSITION_HISTORY; i++) {
		int x, y;
		translate_map_point_to_screen_pixel(Me.Position_History_Ring_Buffer[i].x, Me.Position_History_Ring_Buffer[i].y, &x, &y,
						    1.0);
		glColor3f(1.0, 0.0, 0.0);
		glVertex2i(x, y);
	}

	glEnd();
	glEnable(GL_TEXTURE_2D);
#endif

#if 0
	/* This code displays tux waypoints */
	glDisable(GL_TEXTURE_2D);
	glLineWidth(2.0);
	glBegin(GL_LINE_STRIP);
	i = 0;
	int x, y;
	translate_map_point_to_screen_pixel(Me.pos.x, Me.pos.y, &x, &y, 1.0);
	glColor3f(0.0, 1.0, 0.0);
	glVertex2i(x, y);
	while (Me.next_intermediate_point[i].x != -1) {
		translate_map_point_to_screen_pixel(Me.next_intermediate_point[i].x, Me.next_intermediate_point[i].y, &x, &y, 1.0);
		glColor3f(0.0, 1.0, 0.0);
		glVertex2i(x, y);
		i++;
	}

	glEnd();
	glEnable(GL_TEXTURE_2D);
#endif

	// At this point we are done with the drawing procedure
	// and all that remains to be done is updating the screen.
	//
	if (mask & DO_SCREEN_UPDATE) {
		our_SDL_update_rect_wrapper(Screen, 0, 0, Screen->w, Screen->h);
	}
}

/* -----------------------------------------------------------------
 * This function draws the mouse move cursor.
 * ----------------------------------------------------------------- */
void PutMouseMoveCursor(void)
{
	SDL_Rect TargetRectangle;

	if ((Me.mouse_move_target.x == (-1)) && (enemy_resolve_address(Me.current_enemy_target_n, &Me.current_enemy_target_addr) == NULL)) {
		return;
	}

	if (Me.mouse_move_target.x != (-1)) {
		TargetRectangle.x = translate_map_point_to_screen_pixel_x(Me.mouse_move_target.x, Me.mouse_move_target.y);
		TargetRectangle.y = translate_map_point_to_screen_pixel_y(Me.mouse_move_target.x, Me.mouse_move_target.y);
		if (use_open_gl) {
			TargetRectangle.x -= MouseCursorImageList[0].original_image_width / 2;
			TargetRectangle.y -= MouseCursorImageList[0].original_image_height / 2;
			draw_gl_textured_quad_at_screen_position(&MouseCursorImageList[0], TargetRectangle.x, TargetRectangle.y);
		} else {
			TargetRectangle.x -= MouseCursorImageList[0].surface->w / 2;
			TargetRectangle.y -= MouseCursorImageList[0].surface->h / 2;
			our_SDL_blit_surface_wrapper(MouseCursorImageList[0].surface, NULL, Screen, &TargetRectangle);
		}
	}

	enemy *t = enemy_resolve_address(Me.current_enemy_target_n, &Me.current_enemy_target_addr);
	if (t != NULL) {
		// translate_map_point_to_screen_pixel ( float x_map_pos , float y_map_pos , int give_x )
		update_virtual_position(&t->virt_pos, &t->pos, Me.pos.z);

		TargetRectangle.x = translate_map_point_to_screen_pixel_x(t->virt_pos.x, t->virt_pos.y);
		TargetRectangle.y = translate_map_point_to_screen_pixel_y(t->virt_pos.x, t->virt_pos.y);
		if (use_open_gl) {
			TargetRectangle.x -= MouseCursorImageList[1].original_image_width / 2;
			TargetRectangle.y -= MouseCursorImageList[1].original_image_height / 2;
			draw_gl_textured_quad_at_screen_position(&MouseCursorImageList[1], TargetRectangle.x, TargetRectangle.y);
		} else {
			TargetRectangle.x -= MouseCursorImageList[1].surface->w / 2;
			TargetRectangle.y -= MouseCursorImageList[1].surface->h / 2;
			our_SDL_blit_surface_wrapper(MouseCursorImageList[1].surface, NULL, Screen, &TargetRectangle);
		}

	}

};				// void PutMouseMoveCursor ( void )

/**
 *
 *
 */
void free_one_loaded_tux_image_series(int tux_part_group)
{
	int j;
	int k;

	if (strcmp(previous_part_strings[tux_part_group], NOT_LOADED_MARKER) == 0) {
		DebugPrintf(1, "\n%s(): refusing to free group %d because it's free already.", __FUNCTION__, tux_part_group);
		return;
	}

	DebugPrintf(1, "\n%s():  part_group = %d.", __FUNCTION__, tux_part_group);

	strcpy(previous_part_strings[tux_part_group], NOT_LOADED_MARKER);

	for (j = 0; j < TUX_TOTAL_PHASES; j++) {
		for (k = 0; k < MAX_TUX_DIRECTIONS; k++) {
			// if ( loaded_tux_images [ tux_part_group ] [ j ] [ k ] . surface != NULL )
			// SDL_FreeSurface ( loaded_tux_images [ tux_part_group ] [ j ] [ k ] . surface ) ;
			// loaded_tux_images [ tux_part_group ] [ j ] [ k ] . surface = NULL ;
			// free_single_tux_image ( tux_part_group , j , k );

			SDL_FreeSurface(loaded_tux_images[tux_part_group][j][k].surface);

			loaded_tux_images[tux_part_group][j][k].surface = NULL;
		}
	}

	DebugPrintf(1, "...done freeing group.");

};				// void free_one_loaded_tux_image_series ( int tux_part_group )

/**
 *
 *
 */
void clear_all_loaded_tux_images(int with_free)
{
	int i, j, k;

	// Some more debug output...
	//
	DebugPrintf(1, "\n%s(): clearing tux surfaces.  with_free=%d.", __FUNCTION__, with_free);

	if (with_free) {
		for (i = 0; i < ALL_PART_GROUPS; i++) {
			free_one_loaded_tux_image_series(i);
		}
	} else {
		for (i = 0; i < ALL_PART_GROUPS; i++) {
			strcpy(previous_part_strings[i], NOT_LOADED_MARKER);
			for (j = 0; j < TUX_TOTAL_PHASES; j++) {
				for (k = 0; k < MAX_TUX_DIRECTIONS; k++) {
					loaded_tux_images[i][j][k].surface = NULL;
				}
			}
		}
	}

};				// void clear_all_loaded_tux_images ( int force_free )

/**
 * We open a tux image archive file corresponding to the currently needed
 * tux image series.
 */
FILE *open_tux_image_archive_file(int tux_part_group, int motion_class, char *part_string)
{
	char constructed_filename[10000];
	char fpath[2048];
	FILE *DataFile;

	// We need a file name!
	//
	sprintf(constructed_filename, "tux_motion_parts/%s/%s%s.tux_image_archive.z",
		get_motion_class_name_by_id(motion_class), part_group_strings[tux_part_group], part_string);
	find_file(constructed_filename, GRAPHICS_DIR, fpath, 0);

	// First we need to open the file
	//
	if ((DataFile = fopen(fpath, "rb")) == NULL) {
		fprintf(stderr, "\n\nfilename: '%s'\n", fpath);

		ErrorMessage(__FUNCTION__, "\
Freedroid was unable to open a given tux image archive.\n\
This indicates a serious bug in this installation of Freedroid.", PLEASE_INFORM, IS_FATAL);
	} else {
		DebugPrintf(1, "\n%s(): Opening file succeeded...", __FUNCTION__);
	}

	return (DataFile);

};				// FILE* open_tux_image_archive_file ( int tux_part_group , int motion_class , char* part_string )

/**
 * While earlier we used lots and lots of isolated .png and .offset files
 * to store the information about the Tux, we've now moved over to using
 * a single archive file that holds all the image and all the offset 
 * information, even in uncompressed form, making access at runtime even
 * *much* faster than it was before.  This file grabs one tux part from
 * such an archive file.  It's typically called once or twice whenever 
 * either a fresh game is started/loaded or when the Tux is changing
 * equipment.
 */
void grab_tux_images_from_archive(int tux_part_group, int motion_class, char *part_string)
{
	int rotation_index;
	int our_phase;
	FILE *DataFile;
	char *tmp_buff;
	char archive_type_string[5] = { 0, 0, 0, 0, 0 };
	char ogl_support_string[5] = { 0, 0, 0, 0, 0 };
	unsigned char *DataBuffer, *ptr;
	int tmplen;

	Sint16 cooked_walk_object_phases;
	Sint16 cooked_attack_object_phases;
	Sint16 cooked_gethit_object_phases;
	Sint16 cooked_death_object_phases;
	Sint16 cooked_stand_object_phases;

	Sint16 img_xlen;
	Sint16 img_ylen;
	Sint16 img_x_offs;
	Sint16 img_y_offs;

	// A short message for debug purposes
	//
	DebugPrintf(1, "\n%s():  grabbing new image series...", __FUNCTION__);

	// reading binary-files requires endian swapping depending on platform
	// Therefore we read the whole file into memory first then read out the 
	// numbers using SDLNet_Read..(). The file have to be written using SDLNet_Write..()
	DataFile = open_tux_image_archive_file(tux_part_group, motion_class, part_string);

	inflate_stream(DataFile, &DataBuffer, NULL);
	fclose(DataFile);

	ptr = DataBuffer;
	// We store the currently loaded part string, so that we can later
	// decide if we need to do something upon an equipment change or
	// not.
	//
	strcpy(previous_part_strings[tux_part_group], part_string);
	DebugPrintf(1, "\n%s(): getting image series for group %d.", __FUNCTION__, tux_part_group);

	// Now we assume, that this is an image collection file for tux
	// and therefore it should have the right header bytes (keyword tuxX)
	// and it also should be suitable for pure SDL (keyword sdlX)
	//
	memcpy(archive_type_string, ptr, 4);
	ptr += 4;
	memcpy(ogl_support_string, ptr, 4);
	ptr += 4;

	// We check if this is really an image archive of ENEMY type...
	//
	if (strncmp("tuxX", archive_type_string, 4)) {
		ErrorMessage(__FUNCTION__, "\
Initial archive type string doesn't look like it's from an image archive of TUX type.\n\
This indicates a serious bug in this installation of Freedroid.", PLEASE_INFORM, IS_FATAL);
	}
	// We check if this is really an image archive of ENEMY type...
	//
	if (strncmp("sdlX", ogl_support_string, 4)) {
		ErrorMessage(__FUNCTION__, "\
Initial archive type string doesn't look like this is a pure-SDL\n\
arranged image archive.  While this is not impossible to use, it's\n\
still quite inefficient, and I can only recommend to use sdl-sized\n\
images.  Therefore I refuse to process this file any further here.", PLEASE_INFORM, IS_FATAL);
	}
	// Now we know that this is an archive of tux type.  Therefore
	// we can start to read out some entries, that are only found in
	// enemy image collections and then disregard them, because for
	// tux, we don't need this kind of information anyway.
	//

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

	// Now we can start to really load the images.
	//
	for (rotation_index = 0; rotation_index < MAX_TUX_DIRECTIONS; rotation_index++) {
		for (our_phase = 0; our_phase < TUX_TOTAL_PHASES; our_phase++) {
			// Now if the iso_image we want to blit right now has not yet been loaded,
			// then we need to do something about is and at least attempt to load the
			// surface
			//
			if (loaded_tux_images[tux_part_group][our_phase][rotation_index].surface == NULL) {
				img_xlen = ReadSint16(ptr);
				ptr += sizeof(Sint16);
				img_ylen = ReadSint16(ptr);
				ptr += sizeof(Sint16);
				img_x_offs = ReadSint16(ptr);
				ptr += sizeof(Sint16);
				img_y_offs = ReadSint16(ptr);
				ptr += sizeof(Sint16);

				// Some extra checks against illegal values for the length and height
				// of the tux images.
				//
				if ((img_xlen <= 0) || (img_ylen <= 0)) {
					ErrorMessage(__FUNCTION__, "\
Received some non-positive Tux surface dimensions.  That's a bug for sure!", PLEASE_INFORM, IS_FATAL);
				}
				// New code:  read data into some area.  Have SDL make a surface around the
				// loaded data.  That is much cleaner than hard-writing the data into the 
				// memory, that SDL has prepared internally.
				//
				tmplen = 4 * img_xlen * img_ylen;
				tmp_buff = MyMalloc(tmplen);
				memcpy(tmp_buff, ptr, tmplen);
				ptr += tmplen;
#               if SDL_BYTEORDER == SDL_BIG_ENDIAN
				endian_swap(tmp_buff, 4, img_xlen * img_ylen);
#               endif

				loaded_tux_images[tux_part_group][our_phase][rotation_index].surface =
				    SDL_CreateRGBSurfaceFrom(tmp_buff, img_xlen, img_ylen, 32, 4 * img_xlen,
							     0x00ff0000, 0x0000ff00, 0x000000ff, 0xff000000);

				if (loaded_tux_images[tux_part_group][our_phase][rotation_index].surface == NULL) {
					DebugPrintf(-1000, "\n\nError code from SDL: %s.", SDL_GetError());
					ErrorMessage(__FUNCTION__, "\
Creation of an Tux SDL software surface from pixel data failed.", PLEASE_INFORM, IS_FATAL);
				}

				loaded_tux_images[tux_part_group][our_phase][rotation_index].zoomed_out_surface = NULL;
				loaded_tux_images[tux_part_group][our_phase][rotation_index].texture_has_been_created = FALSE;
				loaded_tux_images[tux_part_group][our_phase][rotation_index].offset_x = img_x_offs;
				loaded_tux_images[tux_part_group][our_phase][rotation_index].offset_y = img_y_offs;

				// this should clear any color key in the dest surface
				SDL_SetColorKey(loaded_tux_images[tux_part_group][our_phase][rotation_index].surface, 0, 0);

				if (!use_open_gl)
					flip_image_vertically(loaded_tux_images[tux_part_group][our_phase][rotation_index].surface);
				else {
					make_texture_out_of_surface(&loaded_tux_images[tux_part_group][our_phase][rotation_index]);
					free(tmp_buff);
				}
			} else {
				// If the surface pointer hasn't been NULL in the first place, then
				// obviously something with the initialisation was wrong in the first
				// place...
				//
				ErrorMessage(__FUNCTION__, "\
Surface to be loaded didn't have empty (NULL) pointer in the first place.", PLEASE_INFORM, IS_FATAL);

			}
		}
	}			/* for rotation_index < MAX_TUX_DIRECTIONS */

	/* ok, we're done reading. Don't forget to free data-file */
	free(DataBuffer);

	return;

};				// void grab_tux_images_from_archive ( ... )

/**
 * While earlier we used lots and lots of isolated .png and .offset files
 * to store the information about an enemy, we've now moved over to using
 * a single archive file that holds all the image and all the offset 
 * information, even in uncompressed form, making access at runtime even
 * *much* faster than it was before.  This file grabs one enemy from
 * such an archive file.  It's typically called once whenever the enemy
 * type is first encountered in one run of the engine.
 */
void grab_enemy_images_from_archive(int enemy_model_nr)
{
	int rotation_index;
	int enemy_phase;
	FILE *DataFile;
	char constructed_filename[10000];
	char fpath[2048];
	char archive_type_string[5] = { 0, 0, 0, 0, 0 };
	char ogl_support_string[5] = { 0, 0, 0, 0, 0 };
	unsigned char *DataBuffer;
	unsigned char *ptr, *dest;
	int tmplen;

	Sint16 img_xlen;
	Sint16 img_ylen;
	Sint16 img_x_offs;
	Sint16 img_y_offs;
	Sint16 orig_img_xlen;
	Sint16 orig_img_ylen;

	Sint16 cooked_walk_object_phases;
	Sint16 cooked_attack_object_phases;
	Sint16 cooked_gethit_object_phases;
	Sint16 cooked_death_object_phases;
	Sint16 cooked_stand_object_phases;

	// A short message for debug purposes
	//
	DebugPrintf(1, "\n%s:  grabbing new image series...", __FUNCTION__);

	// We need a file name!
	//
	sprintf(constructed_filename, "droids/%s/%s.tux_image_archive.z",
		PrefixToFilename[enemy_model_nr], PrefixToFilename[enemy_model_nr]);
	find_file(constructed_filename, GRAPHICS_DIR, fpath, 1);

	// First we need to open the file
	//
	if ((DataFile = fopen(fpath, "rb")) == NULL) {
		fprintf(stderr, "\n\nfilename: '%s'\n", fpath);

		ErrorMessage(__FUNCTION__, "\
Freedroid was unable to open a given enemy image archive.\n\
This indicates a serious bug in this installation of Freedroid.", PLEASE_INFORM, IS_FATAL);
	} else {
		DebugPrintf(1, "\n%s() : Opening file succeeded...", __FUNCTION__);
	}

	inflate_stream(DataFile, &DataBuffer, NULL);
	fclose(DataFile);

	ptr = DataBuffer;

	// Now we assume, that this is an image collection file for an enemy
	// and therefore it should have the right header bytes (keyword eneX)
	// and it also should be suitable for use with OpenGl (keyword oglX)
	//
	memcpy(archive_type_string, ptr, 4);
	ptr += 4;
	memcpy(ogl_support_string, ptr, 4);
	ptr += 4;

	// We check if this is really an image archive of ENEMY type...
	//
	if (strncmp("eneX", archive_type_string, 4)) {
		ErrorMessage(__FUNCTION__, "\
Initial archive type string doesn't look like it's from an image archive of ENEMY type.\n\
This indicates a serious bug in this installation of Freedroid.", PLEASE_INFORM, IS_FATAL);
	}
	// Now we know that this is an archive of enemy type.  Therefore
	// we can start to read out some entries, that are only found in
	// enemy image collections.
	//
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

	// The information about cycle length needs to be entered into the 
	// corresponding arrays (usually initialized in blocks.c, for those
	// series, that don't have an image archive yet...)
	//
	first_walk_animation_image[enemy_model_nr] = 1;
	last_walk_animation_image[enemy_model_nr] = cooked_walk_object_phases;
	first_attack_animation_image[enemy_model_nr] = last_walk_animation_image[enemy_model_nr] + 1;
	last_attack_animation_image[enemy_model_nr] = last_walk_animation_image[enemy_model_nr] + cooked_attack_object_phases;
	first_gethit_animation_image[enemy_model_nr] = last_attack_animation_image[enemy_model_nr] + 1;
	last_gethit_animation_image[enemy_model_nr] = last_attack_animation_image[enemy_model_nr] + cooked_gethit_object_phases;
	first_death_animation_image[enemy_model_nr] = last_gethit_animation_image[enemy_model_nr] + 1;
	last_death_animation_image[enemy_model_nr] = last_gethit_animation_image[enemy_model_nr] + cooked_death_object_phases;
	first_stand_animation_image[enemy_model_nr] = last_death_animation_image[enemy_model_nr] + 1;
	last_stand_animation_image[enemy_model_nr] = last_death_animation_image[enemy_model_nr] + cooked_stand_object_phases;

	// Now some error checking against more phases in this enemy animation than
	// currently allowed from the array size...
	//
	if (last_stand_animation_image[enemy_model_nr] >= MAX_ENEMY_MOVEMENT_PHASES) {
		ErrorMessage(__FUNCTION__, "\
The number of images found in the image collection for enemy model %d is bigger than currently allowed (found %d images, max. %d).", PLEASE_INFORM, IS_FATAL, enemy_model_nr, last_stand_animation_image[enemy_model_nr], MAX_ENEMY_MOVEMENT_PHASES);
	}
	// Now we can proceed to read in the pure image data from the image
	// collection archive file
	//
	for (rotation_index = 0; rotation_index < ROTATION_ANGLES_PER_ROTATION_MODEL; rotation_index++) {
		for (enemy_phase = 0; enemy_phase < last_stand_animation_image[enemy_model_nr]; enemy_phase++) {
			// We read the image parameters.  We need those to construct the
			// surface.  Therefore this must come first.
			//
			img_xlen = ReadSint16(ptr);
			ptr += sizeof(Sint16);
			img_ylen = ReadSint16(ptr);
			ptr += sizeof(Sint16);
			img_x_offs = ReadSint16(ptr);
			ptr += sizeof(Sint16);
			img_y_offs = ReadSint16(ptr);
			ptr += sizeof(Sint16);
			orig_img_xlen = ReadSint16(ptr);
			ptr += sizeof(Sint16);
			orig_img_ylen = ReadSint16(ptr);
			ptr += sizeof(Sint16);

			enemy_iso_images[enemy_model_nr][rotation_index][enemy_phase].surface =
			    SDL_CreateRGBSurface(SDL_SWSURFACE, img_xlen, img_ylen, 32, rmask, gmask, bmask, amask);

			dest = enemy_iso_images[enemy_model_nr][rotation_index][enemy_phase].surface->pixels;
			tmplen = 4 * img_xlen * img_ylen;
			memcpy(dest, ptr, tmplen);
			ptr += tmplen;

			// This might be useful later, when using only SDL output...
			//
			// SDL_SetAlpha( Whole_Image , 0 , SDL_ALPHA_OPAQUE );
			// our_iso_image -> surface = our_SDL_display_format_wrapperAlpha( Whole_Image ); 
			// now we have an alpha-surf of right size
			enemy_iso_images[enemy_model_nr][rotation_index][enemy_phase].zoomed_out_surface = NULL;
			enemy_iso_images[enemy_model_nr][rotation_index][enemy_phase].texture_has_been_created = FALSE;
			enemy_iso_images[enemy_model_nr][rotation_index][enemy_phase].offset_x = img_x_offs;
			enemy_iso_images[enemy_model_nr][rotation_index][enemy_phase].offset_y = img_y_offs;
			enemy_iso_images[enemy_model_nr][rotation_index][enemy_phase].original_image_width = orig_img_xlen;
			enemy_iso_images[enemy_model_nr][rotation_index][enemy_phase].original_image_height = orig_img_ylen;
			enemy_iso_images[enemy_model_nr][rotation_index][enemy_phase].texture_width = img_xlen;
			enemy_iso_images[enemy_model_nr][rotation_index][enemy_phase].texture_height = img_ylen;

			SDL_SetColorKey(enemy_iso_images[enemy_model_nr][rotation_index][enemy_phase].surface, 0, 0);	// this should clear any color key in the dest surface

			if (!use_open_gl) {
				flip_image_vertically(enemy_iso_images[enemy_model_nr][rotation_index][enemy_phase].surface);
			} else {
				if (!strncmp("oglX", ogl_support_string, 4)) {
					make_texture_out_of_prepadded_image(&(enemy_iso_images[enemy_model_nr][rotation_index]
									      [enemy_phase]));
				} else {
					// Of course we could handle the case on non-open-gl optimized image
					// collection files used with OpenGL output.  But that would be a
					// sign of a bug, so we don't properly handle it (like below) but
					// rather give out a fatal error message, just to be safe against
					// non-open-gl-optimized image archives slipping undetected into some
					// release or something...
					//
					// make_texture_out_of_surface ( 
					// & ( enemy_iso_images [ enemy_model_nr ] [ rotation_index ] [ enemy_phase ] ) ) ;
					//
					ErrorMessage(__FUNCTION__, "\
This image collection archive is not optimized for OpenGL usage\n\
but still used in conjunction with OpenGL graphics output.\n\
This is strange.  While of course we could handle this (a bit)\n\
slower than optimized archive, it's an indication that something\n\
is wrong with this installation of FreedroidRPG.  So we terminate\n\
to draw attention to the possible problem...", PLEASE_INFORM, IS_FATAL);
				}
			}
		}
	}

	free(DataBuffer);

	DebugPrintf(1, "\n%s: grabbing new image series DONE.", __FUNCTION__);

	return;

};				// void grab_enemy_images_from_archive ( ... )

/**
 * When the Tux changes equipment and ONE NEW PART IS EQUIPPED, then
 * ALL THE IMAGES FOR THAT PART IN ALL DIRECTIONS AND ALL PHASES must
 * get loaded and that's what is done here...
 */
void make_sure_whole_part_group_is_ready(int tux_part_group, int motion_class, char *part_string)
{
	grab_tux_images_from_archive(tux_part_group, motion_class, part_string);

	// It can be expected, that this operation HAS TAKEN CONSIDERABLE TIME!
	// Therefore we must activate the conservative frame time compution now,
	// so as to prevent any unwanted jumps right now...
	//
	Activate_Conservative_Frame_Computation();

};				// void make_sure_whole_part_group_is_ready ( int tux_part_group , int motion_class , char* part_string )

/*----------------------------------------------------------------------
 * This function should blit the isometric version of the Tux to the
 * screen.
 *----------------------------------------------------------------------*/
void iso_put_tux_part(int tux_part_group, char *part_string, int x, int y, int motion_class, int our_phase, int rotation_index)
{
	// Sanity check on 'part_string'
	if (strlen(part_string) == 0) {
		ErrorMessage(__FUNCTION__, "Empty part string received!", PLEASE_INFORM, IS_FATAL);
	}

	// If some part string given is unlike the part string we were using so
	// far, then we'll need to free that old part and (later) load the new
	// part.
	//
	if (strcmp(previous_part_strings[tux_part_group], part_string)) {
		free_one_loaded_tux_image_series(tux_part_group);
		make_sure_whole_part_group_is_ready(tux_part_group, motion_class, part_string);
	}

	// Now everything should be loaded correctly and we just need to blit the Tux.  Anything
	// that isn't loaded yet should be considered a serious bug and a reason to terminate 
	// immediately...
	//
	if (!use_open_gl) {

		if (loaded_tux_images[tux_part_group][our_phase][rotation_index].surface == NULL) {
			ErrorMessage(__FUNCTION__, "Unable to load tux part's surface : tux_part_group=%d, our_phase=%d, rotation_index=%d",
				PLEASE_INFORM, IS_FATAL, tux_part_group, our_phase, rotation_index);
		}

		if (x == (-1)) {
			blit_iso_image_to_map_position(&loaded_tux_images[tux_part_group][our_phase][rotation_index],
					Me.pos.x, Me.pos.y);
		} else {
			blit_iso_image_to_screen_position(&loaded_tux_images[tux_part_group][our_phase][rotation_index],
					x + loaded_tux_images[tux_part_group][our_phase][rotation_index].offset_x,
					y + loaded_tux_images[tux_part_group][our_phase][rotation_index].offset_y);
		}

	}
#ifdef HAVE_LIBGL
	else {
		float r = 1.0, g = 1.0, b = 1.0;
		int blend = FALSE;

		if (loaded_tux_images[tux_part_group][our_phase][rotation_index].texture == 0) {
			ErrorMessage(__FUNCTION__, "Unable to load tux part's texture : tux_part_group=%d, our_phase=%d, rotation_index=%d",
				PLEASE_INFORM, IS_FATAL, tux_part_group, our_phase, rotation_index);
		}

		if (Me.paralyze_duration) {	/* Paralyzed ? tux turns red */
			g = 0.2;
			b = 0.2;
		} else if (Me.slowdown_duration) {	/* Slowed down ? tux turns blue */
			r = 0.2;
			g = 0.2;
		} else if (Me.energy < Me.maxenergy * 0.25) {	/* Low energy ? blink red */
			g = b = ((SDL_GetTicks() >> 5) & 31) * 0.03;
		}

		if (Me.invisible_duration) {	/* Invisible? Become transparent */
			blend = TRANSPARENCY_CUROBJECT;
		}

		if (x == (-1)) {
			draw_gl_textured_quad_at_map_position(&loaded_tux_images[tux_part_group][our_phase][rotation_index],
					Me.pos.x, Me.pos.y, r, g, b, FALSE, blend, 1.0);
		} else {
			draw_gl_textured_quad_at_screen_position(&loaded_tux_images[tux_part_group][our_phase][rotation_index],
					x + loaded_tux_images[tux_part_group][our_phase][rotation_index].offset_x,
					y + loaded_tux_images[tux_part_group][our_phase][rotation_index].offset_y);
		}
	}
#endif
}

/**
 * This function will put the Tux torso, i.e. it will put some torso with
 * the currently equipped armor on it.  Of course we can't have a unique
 * ingame representation of the Tux torso for every type of armor inside
 * the game.  Therefore several types of armor will each be mapped upon
 * the same ingame representation.  Typically the types of armor mapping
 * to the same ingame representation will be so similar, that you can
 * easily tell them apart in inventory, but it will be more or less ok to
 * use the very same ingame representation, because they are rather 
 * similar after all.
 */
void iso_put_tux_torso(int x, int y, int motion_class, int our_phase, int rotation_index)
{
	static int first_call = 1;
	static int jacket1, jacket2, jacket3, robe1, robe2;
	if (first_call) {
		jacket1 = GetItemIndexByName("Normal Jacket");
		jacket2 = GetItemIndexByName("Reinforced Jacket");
		jacket3 = GetItemIndexByName("Protective Jacket");
		robe1 = GetItemIndexByName("Red Guard's Light Robe");
		robe2 = GetItemIndexByName("Red Guard's Heavy Robe");
		first_call = 0;
	}

	if (Me.armour_item.type == -1) {
		iso_put_tux_part(PART_GROUP_TORSO, "iso_torso", x, y, motion_class, our_phase, rotation_index);
	} else if (Me.armour_item.type == jacket1 || Me.armour_item.type == jacket2 || Me.armour_item.type == jacket3) {
		iso_put_tux_part(PART_GROUP_TORSO, "iso_armour1", x, y, motion_class, our_phase, rotation_index);
	} else if (Me.armour_item.type == robe1 || Me.armour_item.type == robe2) {
		iso_put_tux_part(PART_GROUP_TORSO, "iso_robe", x, y, motion_class, our_phase, rotation_index);
	} else
		iso_put_tux_part(PART_GROUP_TORSO, "iso_armour1", x, y, motion_class, our_phase, rotation_index);
}

/**
 *
 *
 */
void iso_put_tux_weaponarm(int x, int y, int motion_class, int our_phase, int rotation_index)
{
	iso_put_tux_part(PART_GROUP_WEAPONARM, "iso_weaponarm", x, y, motion_class, our_phase, rotation_index);
}

/**
 *
 *
 */
void iso_put_tux_shieldarm(int x, int y, int motion_class, int our_phase, int rotation_index)
{
	static int first_call = 1;
	static int shield1, shield2, shield3, shield4, shield5;

	if (first_call) {
		shield1 = GetItemIndexByName("Improvised Buckler");
		shield2 = GetItemIndexByName("Bot Carapace");
		shield3 = GetItemIndexByName("Standard Shield");
		shield4 = GetItemIndexByName("Heavy Shield");
		shield5 = GetItemIndexByName("Riot Shield");
		first_call = 0;
	}

	// In case of no shield item present at all, display the empty shieldarm.
	if (Me.shield_item.type == -1) {
		iso_put_tux_part(PART_GROUP_SHIELD, "iso_shieldarm", x, y, motion_class, our_phase, rotation_index);
		return;
	}

	// In case of a weapon item present, if the weapon needs both hands, no shield can be used.
	if (Me.weapon_item.type != -1) {
		if (ItemMap[Me.weapon_item.type].item_gun_requires_both_hands == 1) {
			iso_put_tux_part(PART_GROUP_SHIELD, "iso_shieldarm", x, y, motion_class, our_phase, rotation_index);
			return;
		}
	}

	// Now at this point we know, that a 'one hand motion class' item is present, and that
	// we therefore need to blit the shield details.
	if (Me.shield_item.type == shield1) {
		iso_put_tux_part(PART_GROUP_SHIELD, "iso_buckler", x, y, motion_class, our_phase, rotation_index);
	} else if (Me.shield_item.type == shield2 || Me.shield_item.type == shield3) {
		iso_put_tux_part(PART_GROUP_SHIELD, "iso_standard_shield", x, y, motion_class, our_phase, rotation_index);
	} else if (Me.shield_item.type == shield4) {
		iso_put_tux_part(PART_GROUP_SHIELD, "iso_heavy_shield", x, y, motion_class, our_phase, rotation_index);
	} else if (Me.shield_item.type == shield5) {
		iso_put_tux_part(PART_GROUP_SHIELD, "iso_riot_shield", x, y, motion_class, our_phase, rotation_index);
	} else {
		ErrorMessage(__FUNCTION__, "Shield type %d is not yet rendered for Tux.", PLEASE_INFORM, IS_FATAL, Me.shield_item.type);
	}
}

/**
 *
 *
 */
void iso_put_tux_head(int x, int y, int motion_class, int our_phase, int rotation_index)
{
	if (Me.special_item.type == (-1))
		iso_put_tux_part(PART_GROUP_HEAD, "iso_head", x, y, motion_class, our_phase, rotation_index);
	else
		iso_put_tux_part(PART_GROUP_HEAD, "iso_helm1", x, y, motion_class, our_phase, rotation_index);
}

/**
 *
 *
 */
void iso_put_tux_feet(int x, int y, int motion_class, int our_phase, int rotation_index)
{
	if (Me.drive_item.type == (-1))
		iso_put_tux_part(PART_GROUP_FEET, "iso_feet", x, y, motion_class, our_phase, rotation_index);
	else
		iso_put_tux_part(PART_GROUP_FEET, "iso_boots1", x, y, motion_class, our_phase, rotation_index);
}

/**
 *
 *
 */
void iso_put_tux_weapon(int x, int y, int motion_class, int our_phase, int rotation_index)
{
	if (Me.weapon_item.type != (-1)) {
		if (ItemMap[Me.weapon_item.type].item_weapon_is_melee != 0) {
			if      (MatchItemWithName(Me.weapon_item.type, "Big kitchen knife"))
				iso_put_tux_part(PART_GROUP_WEAPON, "iso_big_kitchen_knife", x, y, motion_class, our_phase, rotation_index);
			else if (MatchItemWithName(Me.weapon_item.type, "Cutlass"))
				iso_put_tux_part(PART_GROUP_WEAPON, "iso_cutlass", x, y, motion_class, our_phase, rotation_index);
			else if (MatchItemWithName(Me.weapon_item.type, "Antique Greatsword"))
				iso_put_tux_part(PART_GROUP_WEAPON, "iso_antique_greatsword", x, y, motion_class, our_phase, rotation_index);
// Too broken anim			else if (MatchItemWithName(Me.weapon_item.type, "Chainsaw"))
//				iso_put_tux_part(PART_GROUP_WEAPON, "iso_chainsaw", x, y, rotation_index);
			else if (MatchItemWithName(Me.weapon_item.type, "Meat cleaver"))
				iso_put_tux_part(PART_GROUP_WEAPON, "iso_meat_cleaver", x, y, motion_class, our_phase, rotation_index);
			else if (
				(MatchItemWithName(Me.weapon_item.type, "Hunting knife")) ||
				(MatchItemWithName(Me.weapon_item.type, "Shock knife")) ||
				(MatchItemWithName(Me.weapon_item.type, "Laser Scalpel")) ||
				(MatchItemWithName(Me.weapon_item.type, "Fork")) ||
				(MatchItemWithName(Me.weapon_item.type, "Nobody's edge"))
				)
				iso_put_tux_part(PART_GROUP_WEAPON, "iso_hunting_knife", x, y, motion_class, our_phase, rotation_index);
			else if (MatchItemWithName(Me.weapon_item.type, "Iron pipe"))
				iso_put_tux_part(PART_GROUP_WEAPON, "iso_iron_pipe", x, y, motion_class, our_phase, rotation_index);
			else if (MatchItemWithName(Me.weapon_item.type, "Big wrench"))
				iso_put_tux_part(PART_GROUP_WEAPON, "iso_big_wrench", x, y, motion_class, our_phase, rotation_index);
			else if (MatchItemWithName(Me.weapon_item.type, "Crowbar"))
				iso_put_tux_part(PART_GROUP_WEAPON, "iso_crowbar", x, y, motion_class, our_phase, rotation_index);
			else if (MatchItemWithName(Me.weapon_item.type, "Power hammer"))
				iso_put_tux_part(PART_GROUP_WEAPON, "iso_power_hammer", x, y, motion_class, our_phase, rotation_index);
			else if (MatchItemWithName(Me.weapon_item.type, "Mace"))
				iso_put_tux_part(PART_GROUP_WEAPON, "iso_mace", x, y, motion_class, our_phase, rotation_index);
			else if (MatchItemWithName(Me.weapon_item.type, "Baseball bat"))
				iso_put_tux_part(PART_GROUP_WEAPON, "iso_baseball_bat", x, y, motion_class, our_phase, rotation_index);
			else if (MatchItemWithName(Me.weapon_item.type, "Sledgehammer"))
				iso_put_tux_part(PART_GROUP_WEAPON, "iso_sledgehammer", x, y, motion_class, our_phase, rotation_index);
			else if (MatchItemWithName(Me.weapon_item.type, "Energy whip"))
				iso_put_tux_part(PART_GROUP_WEAPON, "iso_energy_whip", x, y, motion_class, our_phase, rotation_index);
			else
				iso_put_tux_part(PART_GROUP_WEAPON, "iso_sword", x, y, motion_class, our_phase, rotation_index);
		} else {

			if      (
				(MatchItemWithName(Me.weapon_item.type, "Exterminator")) ||
				(MatchItemWithName(Me.weapon_item.type, "The Super Exterminator!!!"))
				)
				iso_put_tux_part(PART_GROUP_WEAPON, "iso_exterminator", x, y, motion_class, our_phase, rotation_index);
			else if (MatchItemWithName(Me.weapon_item.type, ".22 Hunting Rifle"))
				iso_put_tux_part(PART_GROUP_WEAPON, "iso_22_hunting_rifle", x, y, motion_class, our_phase, rotation_index);
			else if (MatchItemWithName(Me.weapon_item.type, "9mm Sub Machine Gun"))
				iso_put_tux_part(PART_GROUP_WEAPON, "iso_9mm_sub_machine_gun", x, y, motion_class, our_phase, rotation_index);
			else if (MatchItemWithName(Me.weapon_item.type, "7.62mm Hunting Rifle"))
				iso_put_tux_part(PART_GROUP_WEAPON, "iso_7_62mm_hunting_rifle", x, y, motion_class, our_phase, rotation_index);
			else if (MatchItemWithName(Me.weapon_item.type, "7.62mm AK-47"))
				iso_put_tux_part(PART_GROUP_WEAPON, "iso_7_62mm_ak47", x, y, motion_class, our_phase, rotation_index);
			else if (MatchItemWithName(Me.weapon_item.type, "Barrett M82 Sniper Rifle"))
				iso_put_tux_part(PART_GROUP_WEAPON, "iso_barrett_m82_sniper_rifle", x, y, motion_class, our_phase, rotation_index);
			else if (MatchItemWithName(Me.weapon_item.type, "Electro Laser Rifle"))
				iso_put_tux_part(PART_GROUP_WEAPON, "iso_electro_laser_rifle", x, y, motion_class, our_phase, rotation_index);
			else if (MatchItemWithName(Me.weapon_item.type, "Laser Rifle"))
				iso_put_tux_part(PART_GROUP_WEAPON, "iso_laser_rifle", x, y, motion_class, our_phase, rotation_index);
			else if (MatchItemWithName(Me.weapon_item.type, "Laser Pulse Rifle"))
				iso_put_tux_part(PART_GROUP_WEAPON, "iso_laser_pulse_rifle", x, y, motion_class, our_phase, rotation_index);
			else if (MatchItemWithName(Me.weapon_item.type, "Laser Pulse Cannon"))
				iso_put_tux_part(PART_GROUP_WEAPON, "iso_laser_pulse_cannon", x, y, motion_class, our_phase, rotation_index);
			else if (
				(MatchItemWithName(Me.weapon_item.type, "Two Barrel sawn off shotgun")) ||
				(MatchItemWithName(Me.weapon_item.type, "Two Barrel shotgun")) ||
				(MatchItemWithName(Me.weapon_item.type, "Pump action shotgun"))
				)
				iso_put_tux_part(PART_GROUP_WEAPON, "iso_shotgun", x, y, motion_class, our_phase, rotation_index);
			else
				iso_put_tux_part(PART_GROUP_WEAPON, "iso_gun1", x, y, motion_class, our_phase, rotation_index);
		}
	}
}

/**
 * Given the name of a Tux's part, returns the function that render it
 *
 * \param part_name A Tux's part name
 * \return A pointer to the function used to render the Tux's part
 */
void (*iso_put_tux_get_function(char *part_name))(int, int, int, int, int)
{
	if (part_name == NULL)
		return NULL;

	if (!strcmp(part_name, "head")) {
		return iso_put_tux_head;
	} else if (!strcmp(part_name, "torso")) {
		return iso_put_tux_torso;
	} else if (!strcmp(part_name, "feet")) {
		return iso_put_tux_feet;
	} else if (!strcmp(part_name, "shieldarm")) {
		return iso_put_tux_shieldarm;
	} else if (!strcmp(part_name, "weaponarm")) {
		return iso_put_tux_weaponarm;
	} else if (!strcmp(part_name, "weapon")) {
		return iso_put_tux_weapon;
	}

	return NULL;
}

/**
  * Initialize Tux's part rendering specification's data structures
  */
void tux_rendering_init()
{
	dynarray_init(&tux_rendering.motion_class_names, 0, 0);
	tux_rendering.render_order = NULL;
}

/**
 * Check mandatory specifications, needed to ensure that Tux can be rendered.
 */
void tux_rendering_validate()
{
	// At least one motion_class is needed
	if (tux_rendering.motion_class_names.size < 1) {
		ErrorMessage(__FUNCTION__,
			"Tux rendering specification is invalid: at least one motion_class is needed",
			PLEASE_INFORM, IS_FATAL);
	}

	// There must be a rendering order defined for each motion class, each rotation
	// and each animation phase
	int i, j;
	for (i = 0; i < tux_rendering.motion_class_names.size; i++) {
		for (j = 0; j < MAX_TUX_DIRECTIONS; j++) {
			if (tux_rendering.render_order[i][j] == NULL) {
				ErrorMessage(__FUNCTION__,
					"Tux rendering specification is invalid: no rendering order defined for motion_class \"%s\""
					" and rotation index %d",
					PLEASE_INFORM, IS_FATAL,
					get_motion_class_name_by_id(i), j);
			} else {
				// Check that there is a rendering order for every animation phases
				// Set a flag for each phase found, and then check that all flags are set
				int phase;
				int check_phase[TUX_TOTAL_PHASES];
				memset(check_phase, 0, TUX_TOTAL_PHASES*sizeof(int));

				struct tux_part_render_set *render_order_ptr = tux_rendering.render_order[i][j];
				while (render_order_ptr != NULL) {
					for (phase = render_order_ptr->phase_start; phase <= render_order_ptr->phase_end; phase++) {
						check_phase[phase] = 1;
					}
					render_order_ptr = render_order_ptr->next;
				}

				for (phase = 0; phase < TUX_TOTAL_PHASES; phase++) {
					if (check_phase[phase] == 0) {
						ErrorMessage(__FUNCTION__,
							"Tux rendering specification is invalid: no rendering order defined for motion_class \"%s\","
							" rotation index %d and animation phase %d",
							PLEASE_INFORM, IS_FATAL,
							get_motion_class_name_by_id(i), j, phase);
					}
				}
			}
		}
	}
}

/**
 * Returns the id (index number) of a motion_class, given its name
 *
 * \param name A motion_class name
 * \return Id associated to the motion_class name or -1 if 'name' is invalid or unknown
 */
int get_motion_class_id_by_name(char *name)
{
	// Check pre-condition
	if (name == NULL)
		return -1;

	// Search 'name' in the motion_class_names list
	int i;
	for (i = 0; i < tux_rendering.motion_class_names.size; i++) {
		if (!strcmp(name, ((char **)tux_rendering.motion_class_names.arr)[i]))
			return i;
	}

	// Fall-back if 'name' is not found
	return -1;
}

/**
 * Returns the name of a motion_class, given its id (index number).
 *
 * \param id A motion_class id
 * \return The name of the motion_class associated to 'id', or the first available motion_class if 'id' is invalid (not in range)
 */
char *get_motion_class_name_by_id(int id)
{
	// Check pre-condition. Returns first name if id is invalid
	if ((id < 0) || (id >= tux_rendering.motion_class_names.size))
		return ((char **)tux_rendering.motion_class_names.arr)[0];

	// Return the motion_class name
	return ((char **)tux_rendering.motion_class_names.arr)[id];
}

/**
 * Returns Tux's current motion_class id, which depends on its weapon.
 * Force a reload of Tux iso_images if motion_class has changed since last call.
 *
 * \return The current motion_class's id
 */
int get_motion_class_id()
{
	static int previously_used_motion_class = -4; // something we'll never really use
	int motion_class;

	if (Me.weapon_item.type == -1) {
		// It Tux has no weapon in hand, return the first motion_class
		motion_class = 0;
	} else {
		motion_class = ItemMap[Me.weapon_item.type].motion_class;
	}

	// If the motion_class has changed, then Tux's images have to be reloaded
	if (motion_class != previously_used_motion_class) {
		previously_used_motion_class = motion_class;
		clear_all_loaded_tux_images(TRUE);
	}

	return motion_class;
}

/*
 * This function blits the isometric version of the Tux to the screen
 */
void iso_put_tux(int x, int y)
{
	// Compute the 3 parameters defining how to render the animated Tux:
	// - motion_class: type of animation (sword_motion/gun_motion)
	// - our_phase: phase of animation (i.e. "animation keyframe number")
	// - rotation_index: Tux's rotation index
	//
	// When Tux is not animated (in the takeover minigame, for instance), that
	// is when x != -1, then we set those parameters to default values (apart
	// from motion_class.

	int motion_class = 0;
	int our_phase = (int)Me.phase; // Tux current animation keyframe
	int rotation_index = 0; // Facing front

	motion_class = get_motion_class_id();

	if (x == -1) {
		float angle;

		// Compute the rotation angle
		// If Tux is walking/running, compute its angle from the direction of movement,
		// else reuse the last computed angle.
		angle = Me.angle;

		if ((Me.phase < tux_anim.attack.first_keyframe) || (Me.phase > tux_anim.attack.last_keyframe)) {
			if (fabsf(Me.speed.x) + fabsf(Me.speed.y) > 0.1) {
				angle = -(atan2(Me.speed.y, Me.speed.x) * 180 / M_PI - 45 - 180);
				angle += 360 / (2 * MAX_TUX_DIRECTIONS);
				while (angle < 0)
					angle += 360;
				Me.angle = angle; // Store computed angle for later use
			}
		}

		// Compute the rotation index from the rotation angle
		rotation_index = (angle * MAX_TUX_DIRECTIONS) / 360.0 + (MAX_TUX_DIRECTIONS / 2);
		while (rotation_index >= MAX_TUX_DIRECTIONS)
			rotation_index -= MAX_TUX_DIRECTIONS;
		while (rotation_index < 0)
			rotation_index += MAX_TUX_DIRECTIONS;
	}

	// Depending on the rendering parameters, get the right rendering order
	// Loop on all rendering orders associated to the current motion_class and
	// rotation, until one set containing the current animation phase is found
	struct tux_part_render_set *one_render_set = tux_rendering.render_order[motion_class][rotation_index];

	while (one_render_set != NULL) {
		if (our_phase >= one_render_set->phase_start && our_phase <= one_render_set->phase_end) {
			break;
		}
		one_render_set = one_render_set->next;
	}

	// Render all Tux's parts in order
	int i;
	for (i = 0; i < 6; i++) {
		if (one_render_set->render_funcs[i] != NULL)
			one_render_set->render_funcs[i](x, y, motion_class, our_phase, rotation_index);
	}
}

/* -----------------------------------------------------------------
 * This function draws the influencer to the screen, either
 * to the center of the combat window if (-1,-1) was specified, or
 * to the specified coordinates anywhere on the screen, useful e.g.
 * for drawing the influencer in the takeover minigame.
 *
 * The given coordinates then indicate the UPPER LEFT CORNER for
 * the blit.
 * ----------------------------------------------------------------- */
void blit_tux(int x, int y)
{
	SDL_Rect Text_Rect;

	Text_Rect.x = UserCenter_x + 21;
	Text_Rect.y = UserCenter_y - 32;
	Text_Rect.w = (User_Rect.w / 2) - 21;
	Text_Rect.h = (User_Rect.h / 2);

	// Draw Tux
	iso_put_tux(x, y);

	// Maybe the influencer has something to say :)
	// so let him say it..
	if ((x == -1) && Me.TextToBeDisplayed && (Me.TextVisibleTime < GameConfig.WantedTextVisibleTime) &&
		GameConfig.All_Texts_Switch) {
		SetCurrentFont(FPS_Display_BFont);
		DisplayText(Me.TextToBeDisplayed, Text_Rect.x, Text_Rect.y, &Text_Rect, TEXT_STRETCH);
	}
}

/**
 * If the corresponding configuration flag is enabled, enemies might 'say'
 * some text comment on the screen, like 'ouch' or 'i'll getch' or 
 * something else more sensible.  This function is here to blit these
 * comments, that must have been set before, to the screen.
 */
void PrintCommentOfThisEnemy(enemy * e)
{
	int x_pos, y_pos;

	// At this point we can assume, that the enemys has been blittet to the
	// screen, whether it's a friendly enemy or not.
	// 
	// So now we can add some text the enemys says.  That might be fun.
	//
	if (!(e->TextToBeDisplayed))
		return;
	if ((e->TextVisibleTime < GameConfig.WantedTextVisibleTime)
	    && GameConfig.All_Texts_Switch) {
		x_pos = translate_map_point_to_screen_pixel_x(e->virt_pos.x, e->virt_pos.y);
		y_pos = translate_map_point_to_screen_pixel_y(e->virt_pos.x, e->virt_pos.y)
		    - 100;

		// First we display the normal text to be displayed...
		//
#	if 0
		char txt[256];
		sprintf(txt, "%d - %s", e->id, e->TextToBeDisplayed);
		PutStringFont(Screen, FPS_Display_BFont, x_pos, y_pos, txt);
#	else
		PutStringFont(Screen, FPS_Display_BFont, x_pos, y_pos, e->TextToBeDisplayed);
#	endif
	}

}

/**
 * Not every enemy has to be blitted onto the combat screen every time.
 * This function is here to find out whether this enemy has to be blitted
 * or whether we can skip it.
 */
static int must_blit_enemy(enemy *e, int x, int y)
{
	// if enemy is on other level, return 
	if (e->virt_pos.z != Me.pos.z) {
		return FALSE;
	}
	// if enemy is of type (-1), return 
	if (e->type == (-1)) {
		return FALSE;
	}
	// if the enemy is dead, show him anyways
	if (e->animation_type == DEAD_ANIMATION)
		return TRUE;
	// if the enemy is out of sight, we need not do anything more here
	if ((!show_all_droids) && (!IsVisible(&e->virt_pos))) {
		return FALSE;
	}

	return TRUE;
}

/**
 *
 *
 */
void PutEnemyEnergyBar(enemy *e, SDL_Rect TargetRectangle)
{
	float Percentage;
	SDL_Rect FillRect;
	static Uint32 full_color_enemy;
	static Uint32 full_color_friend;
	static Uint32 energy_empty_color;

#define ENEMY_ENERGY_BAR_OFFSET_X 0
#define ENEMY_ENERGY_BAR_OFFSET_Y (-20)
#define ENEMY_ENERGY_BAR_LENGTH 65

#define ENEMY_ENERGY_BAR_WIDTH 7

	// If the enemy is dead already, there's nothing to do here...
	//
	if (e->energy <= 0)
		return;

	// Now we need to find the right colors to fill our bars with...
	//
	full_color_enemy = SDL_MapRGB(Screen->format, 255, 0, 0);
	full_color_friend = SDL_MapRGB(Screen->format, 0, 255, 0);
	energy_empty_color = SDL_MapRGB(Screen->format, 0, 0, 0);

	// work out the percentage health
	//
	Percentage = (e->energy) / Druidmap[e->type].maxenergy;

	if (use_open_gl) {

#ifdef HAVE_LIBGL
		int x, y, w, h;
		myColor c1 = { 0, 0, 0, 255 };
		myColor c2 = { 0, 0, 0, 255 };
		float PercentageDone = 0;
		int barnum = 0;
		for (; Percentage > 0; Percentage -= PercentageDone, barnum++) {
			if (Percentage >= 1)
				PercentageDone = 1;
			else
				PercentageDone = Percentage;
			// draw cool bars here
			x = TargetRectangle.x;
			y = TargetRectangle.y - 10 * barnum;
			w = TargetRectangle.w;
			h = TargetRectangle.h;

			if (is_friendly(e->faction, FACTION_SELF))
				c1.g = 255;
			else
				c1.r = 255;

			// tweak as needed, this alters the transparency
			c1.a = 140;
			drawIsoEnergyBar(Z_DIR, x, y, 1, 5, 5, w, PercentageDone, &c1, &c2);
		}

#endif

	} else {
		//sdl stuff here

		// Calculates the width of the remaining health bar. Rounds the
		// width up to the nearest integer to ensure that at least one
		// pixel of health is always shown.
		//
		int health_pixels = (int) ceil(Percentage * TargetRectangle.w);

		FillRect.x = TargetRectangle.x;
		FillRect.y = TargetRectangle.y - ENEMY_ENERGY_BAR_WIDTH - ENEMY_ENERGY_BAR_OFFSET_Y;
		FillRect.h = ENEMY_ENERGY_BAR_WIDTH;
		FillRect.w = health_pixels;

		// The color of the bar depends on the friendly/hostile status
		if (is_friendly(e->faction, FACTION_SELF))
			our_SDL_fill_rect_wrapper(Screen, &FillRect, full_color_friend);
		else
			our_SDL_fill_rect_wrapper(Screen, &FillRect, full_color_enemy);

		// Now after the energy bar has been drawn, we can start to draw the
		// empty part of the energy bar (but only of course, if there is some
		// empty part at all! 
		FillRect.x = TargetRectangle.x + health_pixels;
		FillRect.w = TargetRectangle.w - health_pixels;

		if (Percentage < 1.0)
			our_SDL_fill_rect_wrapper(Screen, &FillRect, energy_empty_color);
	}

};				// void PutEnemyEnergyBar ( Enum , TargetRectangle )

/**
 * The direction this robot should be facing right now is determined and
 * properly set in this function.
 */
int set_rotation_index_for_this_robot(enemy * ThisRobot)
{
	int RotationIndex;

	// By now the angle the robot is facing is determined, so we just need to
	// translate this angle into an index within the image series, i.e. into 
	// a 'phase' of rotation. 
	//
	RotationIndex = ((ThisRobot->current_angle - 45.0 + 360.0 + 360 /
			  (2 * ROTATION_ANGLES_PER_ROTATION_MODEL)) * ROTATION_ANGLES_PER_ROTATION_MODEL / 360);

	// But it might happen, that the angle of rotation is 'out of scale' i.e.
	// it's more than 360 degree or less than 0 degree.  Therefore, we need to
	// be especially careful to generate only proper indices for our arrays.
	// Some would say, we identify the remainder classes with integers in the
	// range [ 0 - (rotation_angles-1) ], which is what's happening here.
	//
	while (RotationIndex < 0)
		RotationIndex += ROTATION_ANGLES_PER_ROTATION_MODEL;
	while (RotationIndex >= ROTATION_ANGLES_PER_ROTATION_MODEL)
		RotationIndex -= ROTATION_ANGLES_PER_ROTATION_MODEL;

	// Now to prevent some jittering in some cases, where the droid uses an angle that is
	// right at the borderline between two possible 8-way directions, we introduce some
	// enforced consistency onto the droid...
	//
	if (RotationIndex == ThisRobot->previous_phase) {
		ThisRobot->last_phase_change += Frame_Time();
	} else {
		if (ThisRobot->last_phase_change >= WAIT_BEFORE_ROTATE) {
			ThisRobot->last_phase_change = 0.0;
			ThisRobot->previous_phase = RotationIndex;
		} else {
			// In this case we don't permit to use a new 8-way direction now...
			//
			RotationIndex = ThisRobot->previous_phase;
			ThisRobot->last_phase_change += Frame_Time();
		}
	}

	// DebugPrintf ( 0 , "\nCurrent angle: %f Current RotationIndex: %d. " , angle, RotationIndex );

	return (RotationIndex);

};				// int set_rotation_index_for_this_robot ( enemy* ThisRobot ) 

/**
 *
 *
 */
int set_rotation_model_for_this_robot(enemy * ThisRobot)
{
	int RotationModel = Druidmap[ThisRobot->type].individual_shape_nr;

	// A sanity check for roation model to use can never hurt...
	//
	if ((RotationModel < 0) || (RotationModel >= ENEMY_ROTATION_MODELS_AVAILABLE)) {
		ErrorMessage(__FUNCTION__, "\
There was a rotation model type given, that exceeds the number of rotation models allowed and loaded in Freedroid.", PLEASE_INFORM, IS_FATAL);
	}

	return (RotationModel);

};				// int set_rotation_model_for_this_robot ( enemy* ThisRobot ) 

/**
 * This function is here to blit the 'body' of a droid to the screen.
 */
void PutIndividuallyShapedDroidBody(enemy * ThisRobot, SDL_Rect TargetRectangle, int mask, int highlight)
{
	int RotationModel;
	int RotationIndex;
	float darkness;
	moderately_finepoint bot_pos;
	float zf = 1.0;
	if (mask & ZOOM_OUT)
		zf = lvledit_zoomfact_inv();

	// if ( ThisRobot -> pos . z != Me . pos . z )
	// DebugPrintf ( -4 , "\n%s(): Now attempting to blit bot on truly virtual position..." , __FUNCTION__ );

	// We properly set the direction this robot is facing.
	//
	RotationIndex = set_rotation_index_for_this_robot(ThisRobot);

	// We properly set the rotation model number for this robot, i.e.
	// which shape (like 302, 247 or proffa) to use for drawing this bot.
	//
	RotationModel = set_rotation_model_for_this_robot(ThisRobot);

	// Maybe the rotation model we're going to use now isn't yet loaded. 
	// Now in this case, we must load it immediately, or a segfault may
	// result...
	//
	LoadAndPrepareEnemyRotationModelNr(RotationModel);

	// Maybe we don't have an enemy here that would really stick to the 
	// exact size of a block but be somewhat bigger or smaller instead.
	// In this case, we'll just adapt the given target rectangle a little
	// bit, cause this rectangle assumes exactly the same size as a map 
	// block and has the origin shifted accordingly.
	//
	if ((TargetRectangle.x != 0) && (TargetRectangle.y != 0)) {
		if (use_open_gl) {
			TargetRectangle.x -= (enemy_iso_images[RotationModel][RotationIndex][0].original_image_width) / 2;
			TargetRectangle.y -= (enemy_iso_images[RotationModel][RotationIndex][0].original_image_height) / 2;
			TargetRectangle.w = enemy_iso_images[RotationModel][RotationIndex][0].original_image_width;
			TargetRectangle.h = enemy_iso_images[RotationModel][RotationIndex][0].original_image_height;
		} else {
			TargetRectangle.x -= (enemy_iso_images[RotationModel][RotationIndex][0].surface->w) / 2;
			TargetRectangle.y -= (enemy_iso_images[RotationModel][RotationIndex][0].surface->h) / 2;
			TargetRectangle.w = enemy_iso_images[RotationModel][RotationIndex][0].surface->w;
			TargetRectangle.h = enemy_iso_images[RotationModel][RotationIndex][0].surface->h;
		}
	}
	// Maybe the enemy is desired e.g. for the takeover game, so a pixel position on
	// the screen is given and we blit the enemy to that position, not taking into 
	// account any map coordinates or stuff like that...
	//
	if ((TargetRectangle.x != 0) && (TargetRectangle.y != 0)) {
		if (use_open_gl) {
			draw_gl_textured_quad_at_screen_position(&enemy_iso_images[RotationModel][RotationIndex][0], TargetRectangle.x,
								 TargetRectangle.y);
		} else {
			our_SDL_blit_surface_wrapper(enemy_iso_images[RotationModel][RotationIndex][0].surface,
						     NULL, Screen, &TargetRectangle);
		}
		return;
	}
	// But here we know, that the enemy is desired inside the game, so we need to
	// taking into account map coordinates and all that stuff...
	//
	else {
		if (use_open_gl) {
			float r = 1.0, g = 1.0, b = 1.0;

			if (ThisRobot->paralysation_duration_left != 0) {
				g = 0.2;
				b = 0.2;
			} else if (ThisRobot->poison_duration_left != 0) {
				r = 0.2;
				b = 0.2;
			} else if (ThisRobot->frozen != 0) {
				r = 0.2;
				g = 0.2;
			}
			bot_pos.x = ThisRobot->virt_pos.x;
			bot_pos.y = ThisRobot->virt_pos.y;

			if (!(mask & SKIP_LIGHT_RADIUS) && !GameConfig.skip_light_radius) {
				darkness = (float)get_light_strength(bot_pos) / (float)(NUMBER_OF_SHADOW_IMAGES-1);
				if (darkness > 1.0)
					darkness = 1.0;
				if (darkness < 0.0)
					darkness = 0.0;
			} else {
				darkness = 1.0;
			}
			draw_gl_textured_quad_at_map_position(&enemy_iso_images[RotationModel][RotationIndex]
							      [(int)ThisRobot->animation_phase], bot_pos.x, bot_pos.y, darkness * r,
							      darkness * g, darkness * b, highlight, FALSE, zf);
		} else {	/*Using SDL */
			if (mask & ZOOM_OUT) {
				// When no OpenGL is used, we need to proceed with SDL for
				// blitting the small enemies...
				//
				blit_zoomed_iso_image_to_map_position(&(enemy_iso_images[RotationModel][RotationIndex][0]),
								      ThisRobot->virt_pos.x, ThisRobot->virt_pos.y);
			} else {

				// First we catch the case of a dead bot (no color filters SDL surfaces
				// available for that case).  In the other cases, we use the prepared color-
				// filtered stuff...
				// 
				if (ThisRobot->energy <= 0) {
					blit_iso_image_to_map_position(&enemy_iso_images[RotationModel][RotationIndex]
								       [(int)ThisRobot->animation_phase], ThisRobot->virt_pos.x,
								       ThisRobot->virt_pos.y);
				} else if (ThisRobot->paralysation_duration_left != 0) {
					LoadAndPrepareRedEnemyRotationModelNr(RotationModel);
					blit_iso_image_to_map_position(&RedEnemyRotationSurfacePointer[RotationModel][RotationIndex][0],
								       ThisRobot->virt_pos.x, ThisRobot->virt_pos.y);
				} else if (ThisRobot->poison_duration_left != 0) {
					LoadAndPrepareGreenEnemyRotationModelNr(RotationModel);
					blit_iso_image_to_map_position(&GreenEnemyRotationSurfacePointer[RotationModel][RotationIndex][0],
								       ThisRobot->virt_pos.x, ThisRobot->virt_pos.y);
				} else if (ThisRobot->frozen != 0) {
					LoadAndPrepareBlueEnemyRotationModelNr(RotationModel);
					blit_iso_image_to_map_position(&BlueEnemyRotationSurfacePointer[RotationModel][RotationIndex][0],
								       ThisRobot->virt_pos.x, ThisRobot->virt_pos.y);
				} else {
					blit_iso_image_to_map_position(&enemy_iso_images[RotationModel][RotationIndex]
								       [(int)ThisRobot->animation_phase], ThisRobot->virt_pos.x,
								       ThisRobot->virt_pos.y);
					if (highlight)
						sdl_highlight_iso_image(&enemy_iso_images[RotationModel][RotationIndex]
											  [(int)ThisRobot->animation_phase],
											  ThisRobot->virt_pos.x, ThisRobot->virt_pos.y);
				}

			}

		}

		int screen_x, screen_y;
		translate_map_point_to_screen_pixel(ThisRobot->virt_pos.x, ThisRobot->virt_pos.y, &screen_x, &screen_y);

		if (use_open_gl) {
			TargetRectangle.x = screen_x - (enemy_iso_images[RotationModel][RotationIndex][0].original_image_width * zf) / 2;
			TargetRectangle.y = screen_y - (enemy_iso_images[RotationModel][RotationIndex][0].original_image_height * zf) / 1;
			TargetRectangle.w = enemy_iso_images[RotationModel][RotationIndex][0].original_image_width * zf;
			TargetRectangle.h = enemy_iso_images[RotationModel][RotationIndex][0].original_image_height * zf;
		} else {
			TargetRectangle.x = screen_x - (enemy_iso_images[RotationModel][RotationIndex][0].surface->w * zf) / 2;
			TargetRectangle.y = screen_y - (enemy_iso_images[RotationModel][RotationIndex][0].surface->h * zf) / 1;
			TargetRectangle.w = enemy_iso_images[RotationModel][RotationIndex][0].surface->w * zf;
			TargetRectangle.h = enemy_iso_images[RotationModel][RotationIndex][0].surface->h * zf;
		}

		if (GameConfig.enemy_energy_bars_visible)
			PutEnemyEnergyBar(ThisRobot, TargetRectangle);
		return;
	}

};				// void PutIndividuallyShapedDroidBody ( int Enum , SDL_Rect TargetRectangle );

/**
 * This function draws an enemy into the combat window.
 * The only parameter given is the number of the enemy within the
 * AllEnemys array. Everything else is computed in here.
 */
void PutEnemy(enemy * e, int x, int y, int mask, int highlight)
{
	SDL_Rect TargetRectangle;

	// We check for things like visibility and distance and the like,
	// so that we know whether to consider this enemy for blitting to
	// the screen or not.  Since there are many things to consider, we
	// got a special function for this job.
	//
	if ((!must_blit_enemy(e, x, y)) && (!GameConfig.xray_vision_for_tux))
		return;

	// We check for incorrect droid types, which sometimes might occor, especially after
	// heavy editing of the crew initialisation functions ;)
	//
	if (e->type >= Number_Of_Droid_Types) {
		ErrorMessage(__FUNCTION__, "\
There was a droid type on this level, that does not really exist.", PLEASE_INFORM, IS_FATAL);
		e->type = 0;
	}
	// Since we will need that several times in the sequel, we find out the correct
	// target location on the screen for our surface blit once and remember it for
	// later.  ( THE TARGET RECTANGLE GETS MODIFIED IN THE SDL BLIT!!! )
	//
	if (x == (-1)) {
		TargetRectangle.x = 0;
		TargetRectangle.y = 0;
	} else {
		TargetRectangle.x = x;
		TargetRectangle.y = y;
	}

	PutIndividuallyShapedDroidBody(e, TargetRectangle, mask, highlight);

#if 0
	/* This code displays the pathway of the bots as well as their next waypoint */
	if (e->energy > 0) {
		glDisable(GL_TEXTURE_2D);
		glLineWidth(2.0);
		int a, b;
		glBegin(GL_LINE_STRIP);
		glColor3f(0.0, 1.0, 1.0);
		translate_map_point_to_screen_pixel(e->pos.x, e->pos.y, &a, &b, 1.0);
		glVertex2i(a, b);
		translate_map_point_to_screen_pixel(curShip.AllLevels[e->pos.z]->AllWaypoints[e->nextwaypoint].x + 0.5,
						    curShip.AllLevels[e->pos.z]->AllWaypoints[e->nextwaypoint].y + 0.5, &a, &b, 1.0);
		glVertex2i(a, b);
		glEnd();
		int aue = 0;
		glBegin(GL_LINE_STRIP);
		translate_map_point_to_screen_pixel(e->pos.x, e->pos.y, &a, &b, 1.0);
		glColor3f(0.0, 0.0, 1.0);
		glVertex2i(a, b);
		for (; aue < 5 && e->PrivatePathway[aue].x != -1; aue++) {
			translate_map_point_to_screen_pixel(e->PrivatePathway[aue].x, e->PrivatePathway[aue].y, &a, &b, 1.0);
			glVertex2i(a, b);
		}
		glEnd();
		glEnable(GL_TEXTURE_2D);
	}
#endif

	// Only if this robot is not dead, we consider printing the comments
	// this robot might have to make on the current situation.
	//
	if (e->energy > 0)
		PrintCommentOfThisEnemy(e);

};				// void PutEnemy(int Enum , int x , int y) 

/**
 * This function draws a Bullet into the combat window.  The only 
 * parameter given is the number of the bullet in the AllBullets 
 * array. Everything else is computed in here.
 */
void PutBullet(int bullet_index, int mask)
{
	bullet *CurBullet = &(AllBullets[bullet_index]);
	int PhaseOfBullet;
	int direction_index;

	if (CurBullet->time_to_hide_still > 0)
		return;

	// DebugPrintf( 0 , "\nBulletType before calculating phase : %d." , CurBullet->type );
	if ((CurBullet->type >= Number_Of_Bullet_Types) || (CurBullet->type < 0)) {
		fprintf(stderr, "\nPutBullet:  bullet type received: %d.", CurBullet->type);
		fflush(stderr);
		ErrorMessage(__FUNCTION__, "\
There was a bullet to be blitted of a type that does not really exist.", PLEASE_INFORM, IS_FATAL);
	}

	PhaseOfBullet = CurBullet->time_in_seconds * Bulletmap[CurBullet->type].phase_changes_per_second;

	PhaseOfBullet = PhaseOfBullet % Bulletmap[CurBullet->type].phases;
	// DebugPrintf( 0 , "\nPhaseOfBullet: %d.", PhaseOfBullet );

	direction_index = ((CurBullet->angle + 360.0 + 360 / (2 * BULLET_DIRECTIONS)) * BULLET_DIRECTIONS / 360);
	while (direction_index < 0)
		direction_index += BULLET_DIRECTIONS;	// just to make sure... a modulo ROTATION_ANGLES_PER_ROTATION_MODEL operation can't hurt
	while (direction_index >= BULLET_DIRECTIONS)
		direction_index -= BULLET_DIRECTIONS;	// just to make sure... a modulo ROTATION_ANGLES_PER_ROTATION_MODEL operation can't hurt

	// draw position is relative to current level, so compute the appropriate virtual position
	gps vpos;
	update_virtual_position(&vpos, &CurBullet->pos, Me.pos.z);
	if (vpos.x == -1)
		return;

	if (mask & ZOOM_OUT) {
		// blit_zoomed_iso_image_to_map_position ( & ( Bulletmap [ CurBullet -> type ] . image [ direction_index ] [ PhaseOfBullet ] ) , CurBullet -> pos . x , CurBullet -> pos . y );
	} else {
		blit_iso_image_to_map_position(&Bulletmap[CurBullet->type].image[direction_index][PhaseOfBullet], vpos.x, vpos.y);
	}
};				// void PutBullet (int Bulletnumber )

/**
 * This function draws an item into the combat window.
 * The only given parameter is the number of the item within
 * the AllItems array.
 */
void PutItem(item *CurItem, int ItemNumber, int mask, int put_thrown_items_flag, int highlight_item)
{
	float r, g, b;

	// The unwanted cases MUST be handled first...
	//
	if (CurItem->type == (-1)) {
		return;
		fprintf(stderr, "\n\nItemNumber '%d' on level %d\n", ItemNumber, CurItem->pos.z);
		ErrorMessage(__FUNCTION__, "\
There was -1 item type given to blit.  This must be a mistake! ", PLEASE_INFORM, IS_FATAL);
	}
	// We don't blit any item, that we're currently holding in our hand, do we?
	if (item_held_in_hand == CurItem)
		return;

	// In case the flag filters this item, we don't blit it
	//
	if ((put_thrown_items_flag == PUT_ONLY_THROWN_ITEMS) && (CurItem->throw_time <= 0))
		return;
	if ((put_thrown_items_flag == PUT_NO_THROWN_ITEMS) && (CurItem->throw_time > 0))
		return;

	// Now we can go take a look if maybe there is an ingame surface 
	// for this item available.  If not, the function will automatically
	// load the inventory surface instead, so we really can assume that
	// we have something to use afterwards.
	//
	if ((ItemMap[CurItem->type].inv_image.ingame_iso_image.surface == NULL) &&
	    (!ItemMap[CurItem->type].inv_image.ingame_iso_image.texture_has_been_created))
		try_to_load_ingame_item_surface(CurItem->type);


	// Apply disco mode when current item is selected
	object_vtx_color(CurItem, &r, &g, &b);

	// When zoomed out, you can't see any items clearly anyway...
	//
	if (mask & ZOOM_OUT) {
		if (use_open_gl) {
			draw_gl_textured_quad_at_map_position(&ItemMap[CurItem->type].inv_image.ingame_iso_image,
									CurItem->virt_pos.x, CurItem->virt_pos.y, r, g, b, 0.25, FALSE,
							      lvledit_zoomfact_inv());
		} else {
			blit_zoomed_iso_image_to_map_position(&(ItemMap[CurItem->type].inv_image.ingame_iso_image),
							      CurItem->virt_pos.x, CurItem->virt_pos.y);
		}
	} else {
		float anim_tr = (CurItem->throw_time <= 0) ? 0.0 : (3.0 * sinf(CurItem->throw_time * 3.0));
		if (use_open_gl) {
			draw_gl_textured_quad_at_map_position(&ItemMap[CurItem->type].inv_image.ingame_iso_image,
							      CurItem->virt_pos.x - anim_tr, CurItem->virt_pos.y - anim_tr,
							      r, g, b, highlight_item, FALSE, 1.0);
		} else {
			blit_iso_image_to_map_position(&ItemMap[CurItem->type].inv_image.ingame_iso_image,
						       CurItem->virt_pos.x - anim_tr, CurItem->virt_pos.y - anim_tr);
			if (highlight_item)
				sdl_highlight_iso_image(&ItemMap[CurItem->type].inv_image.ingame_iso_image,
									  CurItem->virt_pos.x - anim_tr, CurItem->virt_pos.y - anim_tr);
		}
	}

};				// void PutItem( int ItemNumber );

void PutRadialBlueSparks(float PosX, float PosY, float Radius, int SparkType, char active_direction[RADIAL_SPELL_DIRECTIONS], float age)
{
#define FIXED_NUMBER_OF_SPARK_ANGLES 12
#define FIXED_NUMBER_OF_PROTOTYPES 4
#define NUMBER_OF_SPARK_TYPES 3

	SDL_Rect TargetRectangle;
	static SDL_Surface *SparkPrototypeSurface[NUMBER_OF_SPARK_TYPES][FIXED_NUMBER_OF_PROTOTYPES] =
	    { {NULL, NULL, NULL, NULL}, {NULL, NULL, NULL, NULL} };
	static iso_image PrerotatedSparkSurfaces[NUMBER_OF_SPARK_TYPES][FIXED_NUMBER_OF_PROTOTYPES][FIXED_NUMBER_OF_SPARK_ANGLES];
	SDL_Surface *tmp_surf;
	char fpath[2048];
	int NumberOfPicturesToUse;
	int i, k;
	float Angle;
	int PrerotationIndex;
	moderately_finepoint Displacement;
	int PictureType;
	char ConstructedFilename[5000];
	int current_active_direction;

	// We do some sanity check against too small a radius
	// given as parameter.  This can be loosened later.
	//
	if (Radius <= 1.0)
		return;

	PictureType = (int)(4 * age) % 4;

	// Now if we do not yet have all the prototype images in memory,
	// we need to load them now and for once...
	//
	if (SparkPrototypeSurface[SparkType][0] == NULL) {
		for (k = 0; k < FIXED_NUMBER_OF_PROTOTYPES; k++) {
			if (SparkType >= NUMBER_OF_SPARK_TYPES) {
				fprintf(stderr, "\n\nSparkType: %d\n", SparkType);
				ErrorMessage(__FUNCTION__, "\
Freedroid encountered a radial wave type that exceeds the CONSTANT for wave types.", PLEASE_INFORM, IS_FATAL);
			}

			switch (SparkType) {
			case 0:
				sprintf(ConstructedFilename, "blue_sparks_%d.png", k);
				break;
			case 1:
				sprintf(ConstructedFilename, "green_mist_%d.png", k);
				break;
			case 2:
				sprintf(ConstructedFilename, "red_fire_%d.png", k);
				break;
			default:
				fprintf(stderr, "\n\nSparkType: %d\n", SparkType);
				ErrorMessage(__FUNCTION__, "\
Freedroid encountered a radial wave type that does not exist in Freedroid.", PLEASE_INFORM, IS_FATAL);
			}

			find_file(ConstructedFilename, GRAPHICS_DIR, fpath, 0);

			tmp_surf = our_IMG_load_wrapper(fpath);
			if (tmp_surf == NULL) {
				fprintf(stderr, "\n\nfpath: '%s'\n", fpath);
				ErrorMessage(__FUNCTION__, "\
Freedroid wanted to load a certain image file into memory, but the SDL\n\
function used for this did not succeed.", PLEASE_INFORM, IS_FATAL);
			}
			// SDL_SetColorKey( tmp_surf , 0 , 0 ); 
			SparkPrototypeSurface[SparkType][k] = our_SDL_display_format_wrapperAlpha(tmp_surf);
			SDL_FreeSurface(tmp_surf);

			// Now that the loading is successfully done, we can do the
			// prerotation of the images...using a constant for simplicity...
			//
			for (i = 0; i < FIXED_NUMBER_OF_SPARK_ANGLES; i++) {
				Angle = +45 - 360.0 * (float)i / (float)FIXED_NUMBER_OF_SPARK_ANGLES;

				tmp_surf = rotozoomSurface(SparkPrototypeSurface[SparkType][k], Angle, 1.0, FALSE);

				PrerotatedSparkSurfaces[SparkType][k][i].surface = our_SDL_display_format_wrapperAlpha(tmp_surf);

				// Maybe opengl is in use.  Then we need to prepare some textures too...
				//
				if (use_open_gl) {
					flip_image_vertically(PrerotatedSparkSurfaces[SparkType][k][i].surface);
					make_texture_out_of_surface(&(PrerotatedSparkSurfaces[SparkType][k][i]));
				}

				SDL_FreeSurface(tmp_surf);
			}
		}

	}

	NumberOfPicturesToUse = 2 * (2 * Radius * 64 * 3.14) / (float)SparkPrototypeSurface[SparkType][PictureType]->w;
	NumberOfPicturesToUse += 3;	// we want some overlap

	// Now we blit all the pictures we like to use...in this case using
	// multiple dynamic rotations (oh god!)...
	//
	for (i = 0; i < NumberOfPicturesToUse; i++) {
		Angle = 360.0 * (float)i / (float)NumberOfPicturesToUse;
		Displacement.x = Radius;
		Displacement.y = 0;
		RotateVectorByAngle(&Displacement, Angle);

		PrerotationIndex = rintf((Angle) * (float)FIXED_NUMBER_OF_SPARK_ANGLES / 360.0);
		if (PrerotationIndex >= FIXED_NUMBER_OF_SPARK_ANGLES)
			PrerotationIndex = 0;

		current_active_direction = rintf((Angle) * (float)RADIAL_SPELL_DIRECTIONS / 360.0);
		if (!active_direction[current_active_direction])
			continue;

		if (use_open_gl) {
			TargetRectangle.x =
			    translate_map_point_to_screen_pixel_x(PosX + Displacement.x,
								  PosY + Displacement.y) -
			    ((PrerotatedSparkSurfaces[SparkType][PictureType][PrerotationIndex].original_image_width) / 2);
			TargetRectangle.y =
			    translate_map_point_to_screen_pixel_y(PosX + Displacement.x,
								  PosY + Displacement.y) -
			    ((PrerotatedSparkSurfaces[SparkType][PictureType][PrerotationIndex].original_image_height) / 2);
		} else {
			TargetRectangle.x =
			    translate_map_point_to_screen_pixel_x(PosX + Displacement.x,
								  PosY + Displacement.y) -
			    ((PrerotatedSparkSurfaces[SparkType][PictureType][PrerotationIndex].surface->w) / 2);
			TargetRectangle.y =
			    translate_map_point_to_screen_pixel_y(PosX + Displacement.x,
								  PosY + Displacement.y) -
			    ((PrerotatedSparkSurfaces[SparkType][PictureType][PrerotationIndex].surface->h) / 2);
		}

		if (use_open_gl) {
			draw_gl_textured_quad_at_screen_position(&PrerotatedSparkSurfaces[SparkType][PictureType][PrerotationIndex],
								 TargetRectangle.x, TargetRectangle.y);
		} else {
			our_SDL_blit_surface_wrapper(PrerotatedSparkSurfaces[SparkType][PictureType][PrerotationIndex].surface, NULL,
						     Screen, &TargetRectangle);
		}

	}

}				// void PutRadialBlueSparks( float PosX, float PosY , float Radius )

/**
 * This function draws a blast into the combat window.
 * The only given parameter is the number of the blast within
 * the AllBlasts array.
 */
void PutBlast(int Blast_number)
{
	Blast CurBlast = &AllBlasts[Blast_number];

	// If the blast is already long dead, we need not do anything else here
	if (CurBlast->type == INFOUT)
		return;

	int phase = (int)floorf(CurBlast->phase);
	if (phase >= 20) {
		DeleteBlast(Blast_number);
		return;
	}
	// DebugPrintf( 0 , "\nBulletType before calculating phase : %d." , CurBullet->type );
	if (CurBlast->type >= ALLBLASTTYPES) {
		ErrorMessage(__FUNCTION__, "\
The PutBlast function should blit a blast of a type that does not\n\
exist at all.", PLEASE_INFORM, IS_FATAL);
	}
	// draw position is relative to current level, so compute the appropriate virtual position
	gps vpos;
	update_virtual_position(&vpos, &CurBlast->pos, Me.pos.z);
	if (vpos.x == -1)
		return;

	blit_iso_image_to_map_position(&Blastmap[CurBlast->type].image[phase], vpos.x, vpos.y);
}				// void PutBlast(int Blast_number)

/**
 * When the inventory screen is visible, we do not only show the items
 * present in inventory, but we also show the inventory squares, that each
 * item in the item pool takes away for storage.  This function blits a
 * part-transparent colored shadow under the item, such that the inventory
 * dimensions become apparent to the player immediately.
 */
void draw_inventory_occupied_rectangle(SDL_Rect TargetRect, int bgcolor)
{
#define RED_INVENTORY_SQUARE_OCCUPIED_FILE "backgrounds/TransparentRedPlate.png"
#define BLUE_INVENTORY_SQUARE_OCCUPIED_FILE "backgrounds/TransparentBluePlate.png"
#define GREY_INVENTORY_SQUARE_OCCUPIED_FILE "backgrounds/TransparentGreyPlate.png"
#define REQUIREMENTS_NOT_MET 1
#define IS_MAGICAL 2

	static SDL_Surface *TransparentRedPlateImage = NULL;
	static SDL_Surface *TransparentBluePlateImage = NULL;
	static SDL_Surface *TransparentGreyPlateImage = NULL;
	SDL_Surface *tmp;
	char fpath[2048];
	char fname1[] = RED_INVENTORY_SQUARE_OCCUPIED_FILE;
	char fname2[] = BLUE_INVENTORY_SQUARE_OCCUPIED_FILE;
	char fname3[] = GREY_INVENTORY_SQUARE_OCCUPIED_FILE;

	if (use_open_gl) {
		if (!bgcolor)
			gl_draw_rectangle(&TargetRect, 127, 127, 127, 100);
		if (bgcolor & IS_MAGICAL)
			gl_draw_rectangle(&TargetRect, 0, 0, 255, 100);
		if (bgcolor & REQUIREMENTS_NOT_MET)
			gl_draw_rectangle(&TargetRect, 255, 0, 0, 100);
	} else {
		// Some things like the loading of the inventory and initialisation of the
		// inventory rectangle need to be done only once at the first call of this
		// function. 
		//
		if (TransparentRedPlateImage == NULL) {
			// Now we load the red intentory plate
			//
			find_file(fname1, GRAPHICS_DIR, fpath, 0);
			tmp = our_IMG_load_wrapper(fpath);
			if (!tmp) {
				fprintf(stderr, "\n\nfname1: '%s'\n", fname1);
				ErrorMessage(__FUNCTION__, "\
The red transparent plate for the inventory could not be loaded.  This is a fatal error.", PLEASE_INFORM, IS_FATAL);
			}
			TransparentRedPlateImage = our_SDL_display_format_wrapperAlpha(tmp);
			SDL_FreeSurface(tmp);

			// Now we load the blue inventory plate
			//
			find_file(fname2, GRAPHICS_DIR, fpath, 0);
			tmp = our_IMG_load_wrapper(fpath);
			if (!tmp) {
				fprintf(stderr, "\n\nfname2: '%s'\n", fname2);
				ErrorMessage(__FUNCTION__, "\
The blue transparent plate for the inventory could not be loaded.  This is a fatal error.", PLEASE_INFORM, IS_FATAL);
			}
			TransparentBluePlateImage = our_SDL_display_format_wrapperAlpha(tmp);
			SDL_FreeSurface(tmp);

			// Now we load the grey inventory plate
			//
			find_file(fname3, GRAPHICS_DIR, fpath, 0);
			tmp = our_IMG_load_wrapper(fpath);
			if (!tmp) {
				fprintf(stderr, "\n\nfname3: '%s'\n", fname3);
				ErrorMessage(__FUNCTION__, "\
The grey transparent plate for the inventory could not be loaded.  This is a fatal error.", PLEASE_INFORM, IS_FATAL);
			}
			TransparentGreyPlateImage = our_SDL_display_format_wrapperAlpha(tmp);
			SDL_FreeSurface(tmp);

		}

		if (!bgcolor)
			our_SDL_blit_surface_wrapper(TransparentGreyPlateImage, NULL, Screen, &TargetRect);
		if (bgcolor & IS_MAGICAL)
			our_SDL_blit_surface_wrapper(TransparentBluePlateImage, NULL, Screen, &TargetRect);
		if (bgcolor & REQUIREMENTS_NOT_MET)
			our_SDL_blit_surface_wrapper(TransparentRedPlateImage, NULL, Screen, &TargetRect);
	}

};				// void draw_inventory_occupied_rectangle ( SDL_Rect TargetRect )

/**
 * This function displays the inventory screen and also fills in all the
 * items the influencer is carrying in his inventory and also all the 
 * items the influencer is fitted with.
 */
static void show_inventory_screen(void)
{
	SDL_Rect TargetRect;
	int SlotNum;
	int i, j;

	// We define the left side of the user screen as the rectangle
	// for our inventory screen.
	//
	InventoryRect.x = 0;
	InventoryRect.y = User_Rect.y;
	InventoryRect.w = 320;
	InventoryRect.h = 480;

	if (GameConfig.Inventory_Visible == FALSE)
		return;

	// At this point we know, that the inventory screen is desired and must be
	// displayed in-game:
	//
	blit_special_background(INVENTORY_SCREEN_BACKGROUND_CODE);

	// Now we display the item in the influencer drive slot
	//
	TargetRect.x = InventoryRect.x + DRIVE_RECT_X;
	TargetRect.y = InventoryRect.y + DRIVE_RECT_Y;
	if (item_held_in_hand != &Me.drive_item && (Me.drive_item.type != (-1))) {
		our_SDL_blit_surface_wrapper(ItemMap[Me.drive_item.type].inv_image.Surface, NULL, Screen, &TargetRect);
	}
	// Now we display the item in the influencer weapon slot
	// At this point we have to pay extra care, cause the weapons in Freedroid
	// really come in many different sizes.
	//
	TargetRect.x = InventoryRect.x + WEAPON_RECT_X;
	TargetRect.y = InventoryRect.y + WEAPON_RECT_Y;
	if (item_held_in_hand != &Me.weapon_item && (Me.weapon_item.type != (-1))) {
		TargetRect.x += INV_SUBSQUARE_WIDTH * 0.5 * (2 - ItemMap[Me.weapon_item.type].inv_image.inv_size.x);
		TargetRect.y += INV_SUBSQUARE_HEIGHT * 0.5 * (3 - ItemMap[Me.weapon_item.type].inv_image.inv_size.y);
		our_SDL_blit_surface_wrapper(ItemMap[Me.weapon_item.type].inv_image.Surface, NULL, Screen, &TargetRect);

		// Maybe this is also a 2-handed weapon.  In this case we need to blit the
		// weapon a second time, this time in the center of the shield rectangle to
		// visibly reflect the fact, that the shield hand is required too for this
		// weapon.
		//
		if (ItemMap[Me.weapon_item.type].item_gun_requires_both_hands) {
			// Display the weapon again
			TargetRect.x = InventoryRect.x + SHIELD_RECT_X;
			TargetRect.y = InventoryRect.y + SHIELD_RECT_Y;
			TargetRect.x += INV_SUBSQUARE_WIDTH * 0.5 * (2 - ItemMap[Me.weapon_item.type].inv_image.inv_size.x);
			TargetRect.y += INV_SUBSQUARE_HEIGHT * 0.5 * (3 - ItemMap[Me.weapon_item.type].inv_image.inv_size.y);
			TargetRect.w = ItemMap[Me.weapon_item.type].inv_image.Surface->w;
			TargetRect.h = ItemMap[Me.weapon_item.type].inv_image.Surface->h;
			our_SDL_blit_surface_wrapper(ItemMap[Me.weapon_item.type].inv_image.Surface, NULL, Screen, &TargetRect);

		}
	}
	// Now we display the item in the influencer armour slot
	//
	TargetRect.x = InventoryRect.x + ARMOUR_RECT_X;
	TargetRect.y = InventoryRect.y + ARMOUR_RECT_Y;
	if (item_held_in_hand != &Me.armour_item && (Me.armour_item.type != (-1))) {
		our_SDL_blit_surface_wrapper(ItemMap[Me.armour_item.type].inv_image.Surface, NULL, Screen, &TargetRect);
	}
	// Now we display the item in the influencer shield slot
	//
	TargetRect.x = InventoryRect.x + SHIELD_RECT_X;
	TargetRect.y = InventoryRect.y + SHIELD_RECT_Y;
	if (item_held_in_hand != &Me.shield_item && (Me.shield_item.type != (-1))) {
		// Not all shield have the same height, therefore we do a little safety
		// correction here, so that the shield will always appear in the center
		// of the shield slot
		//
		TargetRect.y += INV_SUBSQUARE_HEIGHT * 0.5 * (3 - ItemMap[Me.shield_item.type].inv_image.inv_size.y);
		our_SDL_blit_surface_wrapper(ItemMap[Me.shield_item.type].inv_image.Surface, NULL, Screen, &TargetRect);
	}
	// Now we display the item in the influencer special slot
	//
	TargetRect.x = InventoryRect.x + HELMET_RECT_X;
	TargetRect.y = InventoryRect.y + HELMET_RECT_Y;
	if (item_held_in_hand != &Me.special_item && (Me.special_item.type != (-1))) {
		our_SDL_blit_surface_wrapper(ItemMap[Me.special_item.type].inv_image.Surface, NULL, Screen, &TargetRect);
	}
	// Now we display all the items the influencer is carrying with him
	//
	for (SlotNum = 0; SlotNum < MAX_ITEMS_IN_INVENTORY - 1; SlotNum++) {
		// In case the item does not exist at all, we need not do anything more...
		if (Me.Inventory[SlotNum].type == -1 || Me.Inventory[SlotNum].type < 0) {
			// The < 0 test is to handle a case where the item type field gets corrupted and is -256. We could not reproduce the problem so at least try to hide it to players.
			continue;
		}
		// In case the item is currently held in hand, we need not do anything more HERE ...
		if (item_held_in_hand == &Me.Inventory[SlotNum] ) {
			continue;
		}

		for (i = 0; i < ItemMap[Me.Inventory[SlotNum].type].inv_image.inv_size.y; i++) {
			for (j = 0; j < ItemMap[Me.Inventory[SlotNum].type].inv_image.inv_size.x; j++) {
				TargetRect.x =
				    INVENTORY_RECT_X - 1 + INV_SUBSQUARE_WIDTH * (Me.Inventory[SlotNum].inventory_position.x + j);
				TargetRect.y =
				    User_Rect.y + INVENTORY_RECT_Y + INV_SUBSQUARE_HEIGHT * (Me.Inventory[SlotNum].inventory_position.y +
											     i);
				TargetRect.w = INV_SUBSQUARE_WIDTH;
				TargetRect.h = INV_SUBSQUARE_HEIGHT;
				if (ItemUsageRequirementsMet(&(Me.Inventory[SlotNum]), FALSE)) {
					// Draw a blue background for items with installed add-ons.
					int socketnum;
					int has_addons = FALSE;
					for (socketnum = 0; socketnum < Me.Inventory[SlotNum].upgrade_sockets.size; socketnum++) {
						if (Me.Inventory[SlotNum].upgrade_sockets.arr[socketnum].addon) {
							has_addons = TRUE;
							break;
						}
					}
					draw_inventory_occupied_rectangle(TargetRect, has_addons? 2 : 0);
				} else {
					draw_inventory_occupied_rectangle(TargetRect, 1);
				}
			}
		}

		TargetRect.x = INVENTORY_RECT_X - 1 + INV_SUBSQUARE_WIDTH * Me.Inventory[SlotNum].inventory_position.x;
		TargetRect.y = User_Rect.y + INVENTORY_RECT_Y + INV_SUBSQUARE_HEIGHT * Me.Inventory[SlotNum].inventory_position.y;

		our_SDL_blit_surface_wrapper(ItemMap[Me.Inventory[SlotNum].type].inv_image.Surface, NULL, Screen, &TargetRect);

		// Show amount
		if (ItemMap[Me.Inventory[SlotNum].type].item_group_together_in_inventory) {
			SetCurrentFont(Messagevar_BFont);
			// Only 3 characters fit in one inventory square.
			char amount[4];
			if (Me.Inventory[SlotNum].multiplicity < 999)
				sprintf(amount, "%d", Me.Inventory[SlotNum].multiplicity);
			else
				strcpy(amount, "+++");
			TargetRect.w = INV_SUBSQUARE_WIDTH * ItemMap[Me.Inventory[SlotNum].type].inv_image.inv_size.x;
			int xpos = TargetRect.x + TargetRect.w * ItemMap[Me.Inventory[SlotNum].type].inv_image.inv_size.y - TextWidth(amount) - 2;
			int ypos = TargetRect.y + TargetRect.h - FontHeight(Messagevar_BFont);
			DisplayText(amount, xpos, ypos, &TargetRect, 1.0);
		}
	}

	if (item_held_in_hand != NULL) {
		DisplayItemImageAtMouseCursor(item_held_in_hand->type);
	} else {
		// In case the player does not have anything in his hand, then of course we need to
		// unset everything as 'not in his hand'.
		//
		// printf("\n Mouse button should cause no image now."); 
	}
}

#undef _view_c
