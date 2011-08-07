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

#define _game_ui_c

#include "system.h"

#include "defs.h"
#include "struct.h"
#include "global.h"
#include "proto.h"

#include "widgets/widgets.h"

static struct widget_group *game_widget_group = NULL;

/**
 * This function builds the hud bar widgets.
 */
static struct widget_group *create_hud_bar()
{
	struct widget_group *hud_bar = widget_group_create();
	widget_set_rect(WIDGET(hud_bar), 0, GameConfig.screen_height - 97, GameConfig.screen_width, 97);

	// hud bar background
	struct widget_background *panel = widget_background_create();
	WIDGET(panel)->rect = WIDGET(hud_bar)->rect;
	widget_group_add(hud_bar, WIDGET(panel));

	// Fixed size tiles
	struct image *img_1 = widget_load_image_resource("widgets/hud_background_1.png", 0);
	int left_panel_x = WIDGET(panel)->rect.x;
	widget_background_add(panel, img_1, left_panel_x, WIDGET(panel)->rect.y, img_1->w, img_1->h);

	struct image *img_3 = widget_load_image_resource("widgets/hud_background_3.png", 0);
	int center_panel_x = WIDGET(panel)->rect.x + (WIDGET(panel)->rect.w - img_3->w) / 2;
	widget_background_add(panel, img_3, center_panel_x, WIDGET(panel)->rect.y, img_3->w, img_3->h);

	struct image *img_5 = widget_load_image_resource("widgets/hud_background_5.png", 0);
	int right_panel_x = WIDGET(panel)->rect.x + WIDGET(panel)->rect.w - img_5->w;
	widget_background_add(panel, img_5, right_panel_x, WIDGET(panel)->rect.y, img_5->w, img_5->h);

	// Scalable tiles.
	// These tiles will expand to fill the gap between the 3 fixed tiles.
	struct image *img_2 = widget_load_image_resource("widgets/hud_background_2.png", 0);
	int left_scaling_panel_x = left_panel_x + img_1->w;
	int fill = center_panel_x - left_scaling_panel_x;
	widget_background_add(panel, img_2, left_scaling_panel_x, WIDGET(panel)->rect.y, fill, img_2->h);

	struct image *img_4 = widget_load_image_resource("widgets/hud_background_4.png", 0);
	int right_scaling_panel_x = center_panel_x + img_3->w;
	fill = right_panel_x - right_scaling_panel_x;
	widget_background_add(panel, img_4, right_scaling_panel_x, WIDGET(panel)->rect.y, fill, img_4->h);

	return hud_bar;
}

/**
 * This function returns the game top level widget and creates it if necessary.
 */
struct widget_group *get_game_ui()
{
	if (game_widget_group)
		// Widgets already initialized.
		return game_widget_group;	

	game_widget_group = widget_group_create();
	widget_set_rect(WIDGET(game_widget_group), 0, 0, GameConfig.screen_width, GameConfig.screen_height);

	// Create map widget.
	// This widget is required to know when a mouse event should be handled
	// by AnalyzePlayerMouseClicks.
	game_map = widget_button_create();
	widget_set_rect(WIDGET(game_map), 0, 0, GameConfig.screen_width, GameConfig.screen_height - 98); 
	widget_group_add(game_widget_group, WIDGET(game_map));

	// Create the hud bar.
	struct widget_group *hud_bar = create_hud_bar(); 
	widget_group_add(game_widget_group, WIDGET(hud_bar));

	return game_widget_group;
}
