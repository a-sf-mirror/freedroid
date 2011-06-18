/* 
 *
 *   Copyright (c) 2010 Samuel Pitoiset
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

#include "lvledit/lvledit.h"
#include "lvledit/lvledit_actions.h"
#include "widgets.h"

/**
 * This file defines a minimap that displays the neighbors of the 
 * current level.
*/

static float minimap_scale = 175.0;

// The center position of the minimap on the screen
#define MINIMAP_CENTER_X (GameConfig.screen_width - (WIDGET_MINIMAP_WIDTH / 2))
#define MINIMAP_CENTER_Y (GameConfig.screen_height - (WIDGET_MINIMAP_HEIGHT -70))
	
/**
 * Translate coordinates screen space (relative to the minimap) to map space.
 * The minimap screen space is the real screen space, scaled down and translated.
 *
 */
static void screen_to_minimap(float x, float y, float *map_x, float *map_y)
{
	x -= MINIMAP_CENTER_X;
	y -= MINIMAP_CENTER_Y;
	x *= minimap_scale;
	y *= minimap_scale;

	*map_x = translate_pixel_to_map_location(x, y, TRUE);
	*map_y = translate_pixel_to_map_location(x, y, FALSE);
}

static void minimap_to_screen(float x, float y, int *screen_x, int *screen_y)
{
	float temp_x = translate_map_point_to_screen_pixel_x(x, y);
	float temp_y = translate_map_point_to_screen_pixel_y(x, y);

	temp_x /= minimap_scale;
	temp_y /= minimap_scale;
	*screen_x = temp_x + MINIMAP_CENTER_X;
	*screen_y = temp_y + MINIMAP_CENTER_Y;
}

void widget_lvledit_minimap_mouseenter(SDL_Event *event, struct widget *w)
{
	struct widget_lvledit_minimap *m = w->ext;
	(void)m;
}

void widget_lvledit_minimap_mouseleave(SDL_Event *event, struct widget *w)
{
	struct widget_lvledit_minimap *m = w->ext;
	(void)m;
}

void widget_lvledit_minimap_mouserelease(SDL_Event *event, struct widget *w)
{
	struct widget_lvledit_minimap *m = w->ext;
	(void)m;
}

void widget_lvledit_minimap_mousepress(SDL_Event *event, struct widget *w)
{
	gps vpos, rpos;

	// Translate a given screen coordinates to minimap point
	screen_to_minimap(event->button.x, event->button.y, &vpos.x, &vpos.y);
	vpos.z = EditLevel()->levelnum;

	if (!resolve_virtual_position(&rpos, &vpos)) {
		return;
	}

	action_jump_to_level(rpos.z, rpos.x, rpos.y);
}

void widget_lvledit_minimap_mouserightrelease(SDL_Event *event, struct widget *w)
{
	struct widget_lvledit_minimap *m = w->ext;
	(void)m;
}

void widget_lvledit_minimap_mouserightpress(SDL_Event *event, struct widget *w)
{
	struct widget_lvledit_minimap *m = w->ext;
	(void)m;
}

void widget_lvledit_minimap_mousewheelup(SDL_Event *event, struct widget *w)
{
	minimap_scale += 10.0;
	minimap_scale = min(500.0, minimap_scale);
}

void widget_lvledit_minimap_mousewheeldown(SDL_Event *event, struct widget *w)
{
	minimap_scale -= 10.0;
	minimap_scale = max(1.0, minimap_scale);
}

static void draw_line_at_minimap_position(float x1, float y1, float x2, float y2)
{
	int r1, c1, r2, c2;
	minimap_to_screen(x1, y1, &r1, &c1);
	minimap_to_screen(x2, y2, &r2, &c2);
	draw_line(r1, c1, r2, c2, SDL_MapRGB(Screen->format, 0xFF, 0xFF, 0xFF), 1);
}

void widget_lvledit_minimap_display(struct widget *w)
{
	int i, j;

	set_gl_clip_rect(&w->rect);

	// Display the background
	draw_rectangle(&w->rect, 85, 100, 100, 150);

	// Display the grid
	for (i = -1; i <= 2; i++) {
		draw_line_at_minimap_position(90.0 * i, 180.0, 90.0 * i, -90.0);
		draw_line_at_minimap_position(180.0, 90.0 * i, -90.0, 90.0 * i);
	}

	// Display the level numbers
	for (i = 0; i < 3; i++) {
		for (j = 0; j < 3; j++) {
			int r, c;
			int lvl_id = NEIGHBOR_ID(EditLevel()->levelnum, i, j);
			char text[3];
			sprintf(text, "%d", lvl_id);
			
			minimap_to_screen(45.0 + 90.0 * (i - 1), 45.0 + 90.0 * (j - 1),  &r, &c);
			if (!MouseCursorIsInRect(&w->rect, r, c))
				continue;

			SDL_Rect tr;
			tr.w = 90.0;
			tr.h = 90.0;
			tr.x = r - TextWidth(text) / 2;
			tr.y = c - FontHeight(GetCurrentFont()) / 2;
			display_text(text, tr.x, tr.y, &tr);
		}
	}

	// Display the current position
	draw_line_at_minimap_position(Me.pos.x - 2.0, Me.pos.y, Me.pos.x + 2.0, Me.pos.y);
	draw_line_at_minimap_position(Me.pos.x, Me.pos.y - 2.0, Me.pos.x, Me.pos.y + 2.0);
	
	unset_gl_clip_rect();
}
