/* 
 *
 *   Copyright (c) 1994, 2002, 2003 Johannes Prix
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
 * This file contains miscellaeous helpful functions for FreedroidRPG.
 */
#define _misc_c 1

#include "system.h"

#include "defs.h"
#include "struct.h"
#include "global.h"
#include "proto.h"
#include "savestruct.h"

#include "widgets/widgets.h"
#include "lvledit/lvledit.h"

#include <stdlib.h>
#if HAVE_EXECINFO_H
#  include <execinfo.h>
#endif
#if HAVE_SIGNAL_H
#  include <signal.h>
#endif

#ifdef __WIN32__
// For _access()
#include <io.h>
#endif

static int world_is_frozen = 0;
long oneframedelay = 0;
float FPSover1 = 10;
Uint32 Now_SDL_Ticks;
Uint32 One_Frame_SDL_Ticks;
Uint32 Ten_Frame_SDL_Ticks;
Uint32 Onehundred_Frame_SDL_Ticks;
int framenr = 0;
long Overall_Frames_Displayed = 0;

struct data_dir data_dirs[] = {
	[CONFIG_DIR]=      { "configdir",                   "" },
	[GUI_DIR]=         { "data/gui",                    "" },
	[GRAPHICS_DIR]=    { "data/graphics",               "" },
	[FONT_DIR]=        { "data/fonts",                  "" },
	[SOUND_DIR]=       { "data/sound",                  "" },
	[MUSIC_DIR]=       { "data/sound/music",            "" },
	[BASE_DIR]=        { "data/base",                   "" },
	[BASE_TITLES_DIR]= { "data/base/titles",            "" },
	[MAP_DIR]=         { "data/storyline/act1",         "" },
	[MAP_TITLES_DIR]=  { "data/storyline/act1/titles",  "" },
	[MAP_DIALOG_DIR]=  { "data/storyline/act1/dialogs", "" },
#ifdef ENABLE_NLS
	[LOCALE_DIR]=      { "locale",                      "" },
#endif
	[LUA_MOD_DIR]=     { "lua_modules",                 "" }
};
#define WELL_KNOWN_DATA_FILE "lua_modules/FDdialog.lua"

mouse_press_button AllMousePressButtons[MAX_MOUSE_PRESS_BUTTONS] = {
	[UP_BUTTON] = {EMPTY_IMAGE, "mouse_buttons/UpButton.png", {600, 94, 40, 40}, TRUE},
	[DOWN_BUTTON] = {EMPTY_IMAGE, "mouse_buttons/DownButton.png", {600, 316, 40, 40}, TRUE},

	[ITEM_BROWSER_LEFT_BUTTON] = {EMPTY_IMAGE, "THIS_DOESNT_NEED_BLITTING", {280, 44, 37, 37}, TRUE},
	[ITEM_BROWSER_RIGHT_BUTTON] = {EMPTY_IMAGE, "THIS_DOESNT_NEED_BLITTING", {536, 44, 37, 37}, TRUE},
	[ITEM_BROWSER_EXIT_BUTTON] = {EMPTY_IMAGE, "THIS_DOESNT_NEED_BLITTING", {201, 340, 47, 47}, TRUE},

	[LEFT_SHOP_BUTTON] = {EMPTY_IMAGE, "mouse_buttons/LeftButton.png", {23, 446, 23, 23}, TRUE},
	[RIGHT_SHOP_BUTTON] = {EMPTY_IMAGE, "mouse_buttons/RightButton.png", {580, 447, 23, 23}, TRUE},
	[LEFT_TUX_SHOP_BUTTON] = {EMPTY_IMAGE, "mouse_buttons/LeftShopButton.png", {6, 15, 23, 23}, TRUE},
	[RIGHT_TUX_SHOP_BUTTON] = {EMPTY_IMAGE, "mouse_buttons/RightShopButton.png", {584, 15, 23, 23}, TRUE},
	[LEFT_LEVEL_EDITOR_BUTTON] = {EMPTY_IMAGE, "mouse_buttons/LevelEditorObjectSelectorLeft.png", {3, 8, 15, 60}, FALSE},
	[LEFT_LEVEL_EDITOR_BUTTON_PUSHED] =
	    {EMPTY_IMAGE, "mouse_buttons/LevelEditorObjectSelectorLeftPushed.png", {2, 7, 15, 60}, FALSE},
	[RIGHT_LEVEL_EDITOR_BUTTON] =
	    {EMPTY_IMAGE, "mouse_buttons/LevelEditorObjectSelectorRight.png", {-16, 8, 15, 60}, FALSE},
	[RIGHT_LEVEL_EDITOR_BUTTON_PUSHED] =
	    {EMPTY_IMAGE, "mouse_buttons/LevelEditorObjectSelectorRightPushed.png", {-17, 7, 15, 60}, FALSE},

	[NUMBER_SELECTOR_OK_BUTTON] = {EMPTY_IMAGE, "mouse_buttons/number_selector_ok_button.png", {308, 288, 48, 48}, TRUE},
	[NUMBER_SELECTOR_LEFT_BUTTON] = {EMPTY_IMAGE, "THIS_DOESNT_NEED_BLITTING", {148, 244, 35, 35}, TRUE},
	[NUMBER_SELECTOR_RIGHT_BUTTON] = {EMPTY_IMAGE, "THIS_DOESNT_NEED_BLITTING", {404, 244, 35, 35}, TRUE},

	[BUY_BUTTON] = {EMPTY_IMAGE, "mouse_buttons/buy_button.png", {199, 98, 47, 47}, TRUE},
	[SELL_BUTTON] = {EMPTY_IMAGE, "mouse_buttons/sell_button.png", {199, 153, 47, 47}, TRUE},
	[REPAIR_BUTTON] = {EMPTY_IMAGE, "mouse_buttons/repair_button.png", {199, 225, 47, 47}, TRUE},

	[OPEN_CLOSE_SKILL_EXPLANATION_BUTTON] = {EMPTY_IMAGE, "THIS_DOESNT_NEED_BLITTING", {0 + 17, 424, 33, 33}, FALSE},


/* lower right area next to the mini map*/
	[LEVEL_EDITOR_DELETE_OBSTACLE_BUTTON] =
	    {EMPTY_IMAGE, "mouse_buttons/LevelEditorDeleteObstacleButton.png", {-270, -30, 0, 0}, FALSE},
	[LEVEL_EDITOR_DELETE_OBSTACLE_BUTTON_PUSHED] =
	    {EMPTY_IMAGE, "mouse_buttons/LevelEditorDeleteObstacleButtonPushed.png", {-271, -31, 0, 0}, FALSE},
	[LEVEL_EDITOR_NEXT_OBJECT_BUTTON] =
	    {EMPTY_IMAGE, "mouse_buttons/LevelEditorNextObstacleButton.png", {-240, -30, 0, 0}, FALSE},
	[LEVEL_EDITOR_NEXT_OBJECT_BUTTON_PUSHED] =
	    {EMPTY_IMAGE, "mouse_buttons/LevelEditorNextObstacleButtonPushed.png", {-241, -31, 0, 0}, FALSE},


/* upper right are directly under the object selector*/
	[LEVEL_EDITOR_SAVE_SHIP_BUTTON] =
	    {EMPTY_IMAGE, "mouse_buttons/LevelEditorSaveShipButton.png", {-60, 80, 0, 0}, FALSE},
	[LEVEL_EDITOR_SAVE_SHIP_BUTTON_PUSHED] =
	    {EMPTY_IMAGE, "mouse_buttons/LevelEditorSaveShipButtonPushed.png", {-59, 79, 0, 0}, FALSE},
	[LEVEL_EDITOR_SAVE_SHIP_BUTTON_OFF] =
	    {EMPTY_IMAGE, "mouse_buttons/LevelEditorSaveShipButtonOff.png", {-60, 80, 0, 0}, FALSE},
	[LEVEL_EDITOR_SAVE_SHIP_BUTTON_OFF_PUSHED] =
	    {EMPTY_IMAGE, "mouse_buttons/LevelEditorSaveShipButtonOffPushed.png", {-59, 79, 0, 0}, FALSE},
	[LEVEL_EDITOR_QUIT_BUTTON] =
		{EMPTY_IMAGE, "mouse_buttons/LevelEditorQuitButton.png", {-30, 80, 0, 0}, FALSE},
	[LEVEL_EDITOR_QUIT_BUTTON_PUSHED] =
	    {EMPTY_IMAGE, "mouse_buttons/LevelEditorQuitButtonPushed.png", {-29, 79, 0, 0}, FALSE},


/* above the upper row, the very upper row */
	[LEVEL_EDITOR_TOGGLE_MAP_LABELS_BUTTON] =
	    {EMPTY_IMAGE, "mouse_buttons/LevelEditorToggleMapLabelsButton.png", {-30, -340, 0, 0}, FALSE},
	[LEVEL_EDITOR_TOGGLE_MAP_LABELS_BUTTON_PUSHED] =
	    {EMPTY_IMAGE, "mouse_buttons/LevelEditorToggleMapLabelsButtonPushed.png", {-29, -339, 0, 0}, FALSE},
	[LEVEL_EDITOR_TOGGLE_MAP_LABELS_BUTTON_OFF] =
	    {EMPTY_IMAGE, "mouse_buttons/LevelEditorToggleMapLabelsButtonOff.png", {-30, -340, 0, 0}, FALSE},
	[LEVEL_EDITOR_TOGGLE_MAP_LABELS_BUTTON_OFF_PUSHED] =
	    {EMPTY_IMAGE, "mouse_buttons/LevelEditorToggleMapLabelsButtonOffPushed.png", {-29, -339, 0, 0}, FALSE},


/* above the obstacle selectors, upper row */
	[LEVEL_EDITOR_EDIT_CHEST_BUTTON] =
	    {EMPTY_IMAGE, "mouse_buttons/LevelEditorEditChestButton.png", {-150, -310, 0, 0}, FALSE},
	[LEVEL_EDITOR_EDIT_CHEST_BUTTON_PUSHED] =
	    {EMPTY_IMAGE, "mouse_buttons/LevelEditorEditChestButtonPushed.png", {-149, -309, 0, 0}, FALSE},
	[LEVEL_EDITOR_NEW_OBSTACLE_LABEL_BUTTON] =
	    {EMPTY_IMAGE, "mouse_buttons/LevelEditorNewObstacleLabelButton.png", {-120, -310, 0, 0}, FALSE},
	[LEVEL_EDITOR_NEW_OBSTACLE_LABEL_BUTTON_PUSHED] =
	    {EMPTY_IMAGE, "mouse_buttons/LevelEditorNewObstacleLabelButtonPushed.png", {-119, -309, 0, 0}, FALSE},
	[LEVEL_EDITOR_TOGGLE_WAYPOINT_CONNECTIONS_BUTTON] =
		{EMPTY_IMAGE, "mouse_buttons/LevelEditorToggleWaypointConnectionsButton.png", {-90, -310, 0, 0}, FALSE},
	[LEVEL_EDITOR_TOGGLE_WAYPOINT_CONNECTIONS_BUTTON_PUSHED] =
		{EMPTY_IMAGE, "mouse_buttons/LevelEditorToggleWaypointConnectionsButtonPushed.png", {-90, -310, 0, 0}, FALSE},
	[LEVEL_EDITOR_TOGGLE_WAYPOINT_CONNECTIONS_BUTTON_OFF] =
		{EMPTY_IMAGE, "mouse_buttons/LevelEditorToggleWaypointConnectionsButtonOff.png", {-90, -310, 0, 0}, FALSE},
	[LEVEL_EDITOR_TOGGLE_WAYPOINT_CONNECTIONS_BUTTON_OFF_PUSHED] =
		{EMPTY_IMAGE, "mouse_buttons/LevelEditorToggleWaypointConnectionsButtonOffPushed.png", {-90, -310, 0, 0}, FALSE},
	[LEVEL_EDITOR_ZOOM_IN_BUTTON] =
	    {EMPTY_IMAGE, "mouse_buttons/LevelEditorZoomInButton.png", {-60, -310, 0, 0}, FALSE},
	[LEVEL_EDITOR_ZOOM_IN_BUTTON_PUSHED] =
	    {EMPTY_IMAGE, "mouse_buttons/LevelEditorZoomInButtonPushed.png", {-59, -309, 0, 0}, FALSE},
	[LEVEL_EDITOR_ZOOM_OUT_BUTTON] =
	    {EMPTY_IMAGE, "mouse_buttons/LevelEditorZoomOutButton.png", {-60, -310, 0, 0}, FALSE},
	[LEVEL_EDITOR_ZOOM_OUT_BUTTON_PUSHED] =
	    {EMPTY_IMAGE, "mouse_buttons/LevelEditorZoomOutButtonPushed.png", {-59, -309, 0, 0}, FALSE},
	[LEVEL_EDITOR_BEAUTIFY_GRASS_BUTTON] =
	    {EMPTY_IMAGE, "mouse_buttons/LevelEditorBeautifyGrassButton.png", {-30, -310, 0, 0}, FALSE},
	[LEVEL_EDITOR_BEAUTIFY_GRASS_BUTTON_PUSHED] =
	    {EMPTY_IMAGE, "mouse_buttons/LevelEditorBeautifyGrassButtonPushed.png", {-29, -309, 0, 0}, FALSE},
	[LEVEL_EDITOR_ALL_FLOOR_LAYERS_BUTTON] =
		{EMPTY_IMAGE, "mouse_buttons/LevelEditorAllFloorLayersButton.png", {-120, -310, 0, 0}, FALSE},
	[LEVEL_EDITOR_ALL_FLOOR_LAYERS_BUTTON_PUSHED] =
		{EMPTY_IMAGE, "mouse_buttons/LevelEditorAllFloorLayersButtonPushed.png", {-120, -310, 0, 0}, FALSE},
	[LEVEL_EDITOR_SINGLE_FLOOR_LAYER_BUTTON] =
		{EMPTY_IMAGE, "mouse_buttons/LevelEditorSingleFloorLayerButton.png", {-120, -310, 0, 0}, FALSE},
	[LEVEL_EDITOR_SINGLE_FLOOR_LAYER_BUTTON_PUSHED] =
		{EMPTY_IMAGE, "mouse_buttons/LevelEditorSingleFloorLayerButtonPushed.png", {-120, -310, 0, 0}, FALSE},


/* above the obstacle selector, lower row*/
	[LEVEL_EDITOR_TOGGLE_GRID_BUTTON_OFF] =
	    {EMPTY_IMAGE, "mouse_buttons/LevelEditorToggleGridButtonOff.png", {-150, -280, 0, 0}, FALSE},
	[LEVEL_EDITOR_TOGGLE_GRID_BUTTON_OFF_PUSHED] =
	    {EMPTY_IMAGE, "mouse_buttons/LevelEditorToggleGridButtonOffPushed.png", {-149, -279, 0, 0}, FALSE},
	[LEVEL_EDITOR_TOGGLE_GRID_BUTTON] =
	    {EMPTY_IMAGE, "mouse_buttons/LevelEditorToggleGridButton.png", {-150, -280, 0, 0}, FALSE},
	[LEVEL_EDITOR_TOGGLE_GRID_BUTTON_PUSHED] =
	    {EMPTY_IMAGE, "mouse_buttons/LevelEditorToggleGridButtonPushed.png", {-149, -279, 0, 0}, FALSE},
	[LEVEL_EDITOR_TOGGLE_GRID_BUTTON_FULL] =
	    {EMPTY_IMAGE, "mouse_buttons/LevelEditorToggleGridButtonFull.png", {-150, -280, 0, 0}, FALSE},
	[LEVEL_EDITOR_TOGGLE_GRID_BUTTON_FULL_PUSHED] =
	    {EMPTY_IMAGE, "mouse_buttons/LevelEditorToggleGridButtonFullPushed.png", {-149, -279, 0, 0}, FALSE},
	[LEVEL_EDITOR_TOGGLE_ENEMIES_BUTTON] =
	    {EMPTY_IMAGE, "mouse_buttons/LevelEditorToggleEnemiesButton.png", {-120, -280, 0, 0}, FALSE},
	[LEVEL_EDITOR_TOGGLE_ENEMIES_BUTTON_PUSHED] =
	    {EMPTY_IMAGE, "mouse_buttons/LevelEditorToggleEnemiesButtonPushed.png", {-119, -279, 0, 0}, FALSE},
	[LEVEL_EDITOR_TOGGLE_ENEMIES_BUTTON_OFF] =
	    {EMPTY_IMAGE, "mouse_buttons/LevelEditorToggleEnemiesButtonOff.png", {-120, -280, 0, 0}, FALSE},
	[LEVEL_EDITOR_TOGGLE_ENEMIES_BUTTON_OFF_PUSHED] =
	    {EMPTY_IMAGE, "mouse_buttons/LevelEditorToggleEnemiesButtonOffPushed.png", {-119, -279, 0, 0}, FALSE},
	[LEVEL_EDITOR_TOGGLE_OBSTACLES_BUTTON] =
	    {EMPTY_IMAGE, "mouse_buttons/LevelEditorToggleObstaclesButton.png", {-90, -280, 0, 0}, FALSE},
	[LEVEL_EDITOR_TOGGLE_OBSTACLES_BUTTON_PUSHED] =
	    {EMPTY_IMAGE, "mouse_buttons/LevelEditorToggleObstaclesButtonPushed.png", {-89, -279, 0, 0}, FALSE},
	[LEVEL_EDITOR_TOGGLE_OBSTACLES_BUTTON_OFF] =
	    {EMPTY_IMAGE, "mouse_buttons/LevelEditorToggleObstaclesButtonOff.png", {-90, -280, 0, 0}, FALSE},
	[LEVEL_EDITOR_TOGGLE_OBSTACLES_BUTTON_OFF_PUSHED] =
	    {EMPTY_IMAGE, "mouse_buttons/LevelEditorToggleObstaclesButtonOffPushed.png", {-89, -279, 0, 0}, FALSE},
	[LEVEL_EDITOR_TOGGLE_TOOLTIPS_BUTTON] =
	    {EMPTY_IMAGE, "mouse_buttons/LevelEditorToggleTooltipsButton.png", {-60, -280, 0, 0}, FALSE},
	[LEVEL_EDITOR_TOGGLE_TOOLTIPS_BUTTON_PUSHED] =
	    {EMPTY_IMAGE, "mouse_buttons/LevelEditorToggleTooltipsButtonPushed.png", {-59, -279, 0, 0}, FALSE},
	[LEVEL_EDITOR_TOGGLE_TOOLTIPS_BUTTON_OFF] =
	    {EMPTY_IMAGE, "mouse_buttons/LevelEditorToggleTooltipsButtonOff.png", {-60, -280, 0, 0}, FALSE},
	[LEVEL_EDITOR_TOGGLE_TOOLTIPS_BUTTON_OFF_PUSHED] =
	    {EMPTY_IMAGE, "mouse_buttons/LevelEditorToggleTooltipsButtonOffPushed.png", {-59, -279, 0, 0}, FALSE},
	[LEVEL_EDITOR_TOGGLE_COLLISION_RECTS_BUTTON] =
	    {EMPTY_IMAGE, "mouse_buttons/LevelEditorToggleCollisionRectsButton.png", {-30, -280, 0, 0}, FALSE},
	[LEVEL_EDITOR_TOGGLE_COLLISION_RECTS_BUTTON_PUSHED] =
	    {EMPTY_IMAGE, "mouse_buttons/LevelEditorToggleCollisionRectsButtonPushed.png", {-29, -279, 0, 0}, FALSE},
	[LEVEL_EDITOR_TOGGLE_COLLISION_RECTS_BUTTON_OFF] =
	    {EMPTY_IMAGE, "mouse_buttons/LevelEditorToggleCollisionRectsButtonOff.png", {-30, -280, 0, 0}, FALSE},
	[LEVEL_EDITOR_TOGGLE_COLLISION_RECTS_BUTTON_OFF_PUSHED] =
	    {EMPTY_IMAGE, "mouse_buttons/LevelEditorToggleCollisionRectsButtonOffPushed.png", {-29, -279, 0, 0}, FALSE},


	[LEVEL_EDITOR_NEXT_ITEM_GROUP_BUTTON] =
	    {EMPTY_IMAGE, "mouse_buttons/RightButton.png", {55 + 64 * 8, 32 + 5 * 66, 0, 0}, TRUE},
	[LEVEL_EDITOR_PREV_ITEM_GROUP_BUTTON] =
	    {EMPTY_IMAGE, "mouse_buttons/LeftButton.png", {55, 32 + 5 * 66, 0, 0}, TRUE},

	[LEVEL_EDITOR_CANCEL_ITEM_DROP_BUTTON] =
	    {EMPTY_IMAGE, "mouse_buttons/LevelEditorCancelItemDrop.png", {55 + 80, 32 + 5 * 66, 0, 0}, TRUE},
	[LEVEL_EDITOR_UNDO_BUTTON] =
		 {EMPTY_IMAGE, "mouse_buttons/LevelEditorUndoButton.png", {-330, -30, 0, 0}, FALSE},
	[LEVEL_EDITOR_UNDO_BUTTON_PUSHED] =
	    {EMPTY_IMAGE, "mouse_buttons/LevelEditorUndoButtonPushed.png", {-331, -31, 0, 0}, FALSE},

	[LEVEL_EDITOR_REDO_BUTTON] =
		 {EMPTY_IMAGE, "mouse_buttons/LevelEditorRedoButton.png", {-300, -30, 0, 0}, FALSE},
	[LEVEL_EDITOR_REDO_BUTTON_PUSHED] =
	    {EMPTY_IMAGE, "mouse_buttons/LevelEditorRedoButtonPushed.png", {-301, -31, 0, 0}, FALSE},

	[LEVEL_EDITOR_TYPESELECT_OBSTACLE_BUTTON] =
		 {EMPTY_IMAGE, "mouse_buttons/LevelEditorTypeSelectorButton.png", {-152, -250, 0, 0}, FALSE},
	[LEVEL_EDITOR_TYPESELECT_OBSTACLE_BUTTON_PUSHED] =
	    {EMPTY_IMAGE, "mouse_buttons/LevelEditorTypeSelectorButtonPushed.png", {-152, -250, 0, 0}, FALSE},
	[LEVEL_EDITOR_TYPESELECT_OBSTACLE_BUTTON_OFF] =
		 {EMPTY_IMAGE, "mouse_buttons/LevelEditorTypeSelectorButtonOff.png", {-152, -250, 0, 0}, FALSE},
	[LEVEL_EDITOR_TYPESELECT_OBSTACLE_BUTTON_OFF_PUSHED] =
	    {EMPTY_IMAGE, "mouse_buttons/LevelEditorTypeSelectorButtonOffPushed.png", {-152, -250, 0, 0}, FALSE},

	[LEVEL_EDITOR_TYPESELECT_ENEMY_BUTTON] =
		{EMPTY_IMAGE, "mouse_buttons/LevelEditorTypeSelectorButton3.png", {-90, -174, 0, 0}, FALSE},
	[LEVEL_EDITOR_TYPESELECT_ENEMY_BUTTON_PUSHED] =
		{EMPTY_IMAGE, "mouse_buttons/LevelEditorTypeSelectorButton3Pushed.png", {-90, -174, 0, 0}, FALSE},
	[LEVEL_EDITOR_TYPESELECT_ENEMY_BUTTON_OFF] =
		{EMPTY_IMAGE, "mouse_buttons/LevelEditorTypeSelectorButton3Off.png", {-90, -174, 0, 0}, FALSE},
	[LEVEL_EDITOR_TYPESELECT_ENEMY_BUTTON_OFF_PUSHED] =
		{EMPTY_IMAGE, "mouse_buttons/LevelEditorTypeSelectorButton3OffPushed.png", {-90, -174, 0, 0}, FALSE},

	[LEVEL_EDITOR_TYPESELECT_FLOOR_BUTTON] =
		 {EMPTY_IMAGE, "mouse_buttons/LevelEditorTypeSelectorButton.png", {-152, -212, 0, 0}, FALSE},
	[LEVEL_EDITOR_TYPESELECT_FLOOR_BUTTON_PUSHED] =
	    {EMPTY_IMAGE, "mouse_buttons/LevelEditorTypeSelectorButtonPushed.png", {-152, -212, 0, 0}, FALSE},
	[LEVEL_EDITOR_TYPESELECT_FLOOR_BUTTON_OFF] =
		 {EMPTY_IMAGE, "mouse_buttons/LevelEditorTypeSelectorButtonOff.png", {-152, -212, 0, 0}, FALSE},
	[LEVEL_EDITOR_TYPESELECT_FLOOR_BUTTON_OFF_PUSHED] =
	    {EMPTY_IMAGE, "mouse_buttons/LevelEditorTypeSelectorButtonOffPushed.png", {-152, -212, 0, 0}, FALSE},

	[LEVEL_EDITOR_TYPESELECT_ITEM_BUTTON] =
		 {EMPTY_IMAGE, "mouse_buttons/LevelEditorTypeSelectorButton2.png", {-152, -174, 0, 0}, FALSE},
	[LEVEL_EDITOR_TYPESELECT_ITEM_BUTTON_PUSHED] =
	    {EMPTY_IMAGE, "mouse_buttons/LevelEditorTypeSelectorButton2Pushed.png", {-152, -174, 0, 0}, FALSE},
	[LEVEL_EDITOR_TYPESELECT_ITEM_BUTTON_OFF] =
		 {EMPTY_IMAGE, "mouse_buttons/LevelEditorTypeSelectorButton2Off.png", {-152, -174, 0, 0}, FALSE},
	[LEVEL_EDITOR_TYPESELECT_ITEM_BUTTON_OFF_PUSHED] =
	    {EMPTY_IMAGE, "mouse_buttons/LevelEditorTypeSelectorButton2OffPushed.png", {-152, -174, 0, 0}, FALSE},

	[LEVEL_EDITOR_TYPESELECT_WAYPOINT_BUTTON] =
		 {EMPTY_IMAGE, "mouse_buttons/LevelEditorTypeSelectorButton2.png", {-152, -136, 0, 0}, FALSE},
	[LEVEL_EDITOR_TYPESELECT_WAYPOINT_BUTTON_PUSHED] =
	    {EMPTY_IMAGE, "mouse_buttons/LevelEditorTypeSelectorButton2Pushed.png", {-152, -136, 0, 0}, FALSE},
	[LEVEL_EDITOR_TYPESELECT_WAYPOINT_BUTTON_OFF] =
		 {EMPTY_IMAGE, "mouse_buttons/LevelEditorTypeSelectorButton2Off.png", {-152, -136, 0, 0}, FALSE},
	[LEVEL_EDITOR_TYPESELECT_WAYPOINT_BUTTON_OFF_PUSHED] =
	    {EMPTY_IMAGE, "mouse_buttons/LevelEditorTypeSelectorButton2OffPushed.png", {-152, -136, 0, 0}, FALSE},

	[LEVEL_EDITOR_TYPESELECT_MAP_LABEL_BUTTON] =
		 {EMPTY_IMAGE, "mouse_buttons/LevelEditorTypeSelectorButton3.png", {-90, -136, 0, 0}, FALSE},
	[LEVEL_EDITOR_TYPESELECT_MAP_LABEL_BUTTON_PUSHED] =
	    {EMPTY_IMAGE, "mouse_buttons/LevelEditorTypeSelectorButton3Pushed.png", {-90, -136, 0, 0}, FALSE},
	[LEVEL_EDITOR_TYPESELECT_MAP_LABEL_BUTTON_OFF] =
		 {EMPTY_IMAGE, "mouse_buttons/LevelEditorTypeSelectorButton3Off.png", {-90, -136, 0, 0}, FALSE},
	[LEVEL_EDITOR_TYPESELECT_MAP_LABEL_BUTTON_OFF_PUSHED] =
	    {EMPTY_IMAGE, "mouse_buttons/LevelEditorTypeSelectorButton3OffPushed.png", {-90, -136, 0, 0}, FALSE},


	[WEAPON_RECT_BUTTON] =
	    {EMPTY_IMAGE, "THIS_DOESNT_NEED_BLITTING", {WEAPON_RECT_X, WEAPON_RECT_Y, WEAPON_RECT_WIDTH, WEAPON_RECT_HEIGHT}, FALSE},
	[DRIVE_RECT_BUTTON] =
	    {EMPTY_IMAGE, "THIS_DOESNT_NEED_BLITTING", {DRIVE_RECT_X, DRIVE_RECT_Y, DRIVE_RECT_WIDTH, DRIVE_RECT_HEIGHT}, FALSE},
	[SHIELD_RECT_BUTTON] =
	    {EMPTY_IMAGE, "THIS_DOESNT_NEED_BLITTING", {SHIELD_RECT_X, SHIELD_RECT_Y, SHIELD_RECT_WIDTH, SHIELD_RECT_HEIGHT}, FALSE},
	[HELMET_RECT_BUTTON] =
	    {EMPTY_IMAGE, "THIS_DOESNT_NEED_BLITTING", {HELMET_RECT_X, HELMET_RECT_Y, HELMET_RECT_WIDTH, HELMET_RECT_HEIGHT}, FALSE},
	[ARMOUR_RECT_BUTTON] =
	    {EMPTY_IMAGE, "THIS_DOESNT_NEED_BLITTING", {ARMOUR_RECT_X, ARMOUR_RECT_Y, ARMOUR_RECT_WIDTH, ARMOUR_RECT_HEIGHT}, FALSE},

	[MORE_STR_BUTTON] =
	    {EMPTY_IMAGE, "mouse_buttons/AttributePlusButton.png", {0 + STR_X + 53, STR_Y - 5, 38, 22}, FALSE},
	[MORE_MAG_BUTTON] =
	    {EMPTY_IMAGE, "mouse_buttons/AttributePlusButton.png", {0 + STR_X + 53, MAG_Y - 5, 38, 22}, FALSE},
	[MORE_DEX_BUTTON] =
	    {EMPTY_IMAGE, "mouse_buttons/AttributePlusButton.png", {0 + STR_X + 53, DEX_Y - 5, 38, 22}, FALSE},
	[MORE_VIT_BUTTON] =
	    {EMPTY_IMAGE, "mouse_buttons/AttributePlusButton.png", {0 + STR_X + 53, VIT_Y - 5, 38, 22}, FALSE},

	[DESCRIPTION_WINDOW_UP_BUTTON] = {EMPTY_IMAGE, "THIS_DOESNT_NEED_BLITTING", {607, 99, 26, 26}, TRUE},
	[DESCRIPTION_WINDOW_DOWN_BUTTON] = {EMPTY_IMAGE, "THIS_DOESNT_NEED_BLITTING", {607, 347, 26, 26}, TRUE},

	[DROID_SHOW_EXIT_BUTTON] = {EMPTY_IMAGE, "THIS_DOESNT_NEED_BLITTING", {202, 311, 47, 47}, TRUE},

	[QUEST_BROWSER_ITEM_SHORT_BUTTON] =
	    {EMPTY_IMAGE, "mouse_buttons/quest_browser_item_short.png", {108, 86, 300, 26}, FALSE},
	[QUEST_BROWSER_ITEM_LONG_BUTTON] =
	    {EMPTY_IMAGE, "mouse_buttons/quest_browser_item_long.png", {108, 86, 300, 26}, FALSE},

	[TAKEOVER_HELP_BUTTON] = {EMPTY_IMAGE, "mouse_buttons/takeover_help_button.png", {78, 23, 153, 38}, FALSE},

	// Buttons of the item upgrade UI.
	[ITEM_UPGRADE_APPLY_BUTTON] = { EMPTY_IMAGE, "item_upgrade/button_apply.png",
	    { ITEM_UPGRADE_RECT_X + 250, ITEM_UPGRADE_RECT_Y + 389, 48, 48 }, FALSE},
	[ITEM_UPGRADE_APPLY_BUTTON_DISABLED] = { EMPTY_IMAGE, "item_upgrade/button_apply_disabled.png",
	    { ITEM_UPGRADE_RECT_X + 250, ITEM_UPGRADE_RECT_Y + 389, 48, 48 }, FALSE},
	[ITEM_UPGRADE_CLOSE_BUTTON] = { EMPTY_IMAGE, "item_upgrade/button_close.png",
	    { ITEM_UPGRADE_RECT_X + 215, ITEM_UPGRADE_RECT_Y + 406, 32, 32 }, FALSE},

	// Buttons of the add-on crafting UI.
	[ADDON_CRAFTING_APPLY_BUTTON] = { EMPTY_IMAGE, "item_upgrade/button_apply.png",
	    { ADDON_CRAFTING_RECT_X + 250, ADDON_CRAFTING_RECT_Y + 389, 48, 48 }, FALSE},
	[ADDON_CRAFTING_APPLY_BUTTON_DISABLED] = { EMPTY_IMAGE, "item_upgrade/button_apply_disabled.png",
	    { ADDON_CRAFTING_RECT_X + 250, ADDON_CRAFTING_RECT_Y + 389, 48, 48 }, FALSE},
	[ADDON_CRAFTING_CLOSE_BUTTON] = { EMPTY_IMAGE, "item_upgrade/button_close.png",
	    { ADDON_CRAFTING_RECT_X + 215, ADDON_CRAFTING_RECT_Y + 406, 32, 32 }, FALSE},
	[ADDON_CRAFTING_SCROLL_UP_BUTTON] = { EMPTY_IMAGE, "mouse_buttons/crafting_scroll_up.png",
	    { ADDON_CRAFTING_RECT_X + 264, ADDON_CRAFTING_RECT_Y + 70, 32, 32 }, FALSE},
	[ADDON_CRAFTING_SCROLL_DOWN_BUTTON] = { EMPTY_IMAGE, "mouse_buttons/crafting_scroll_down.png",
	    { ADDON_CRAFTING_RECT_X + 264, ADDON_CRAFTING_RECT_Y + 198, 32, 32 }, FALSE},
	[ADDON_CRAFTING_SCROLL_DESC_UP_BUTTON] = { EMPTY_IMAGE, "mouse_buttons/crafting_scroll_up.png",
	    { ADDON_CRAFTING_RECT_X + 260, ADDON_CRAFTING_RECT_Y + 290, 32, 32 }, FALSE},
	[ADDON_CRAFTING_SCROLL_DESC_DOWN_BUTTON] = { EMPTY_IMAGE, "mouse_buttons/crafting_scroll_down.png",
	    { ADDON_CRAFTING_RECT_X + 260, ADDON_CRAFTING_RECT_Y + 350, 32, 32 }, FALSE},

};				// mouse_press_button AllMousePressButtons[ MAX_MOUSE_PRESS_BUTTONS ] 

//--------------------
// We make these global variables here, as we might want to use
// this function inside a signal handler and maybe also it's better
// not to mess too much around with the stack while trying to read
// out the stack...
//
#define MAX_CALLS_IN_BACKTRACE 200
void *backtrace_array[MAX_CALLS_IN_BACKTRACE];
size_t backtrace_size;
char **backtrace_strings;
size_t backtrace_counter;

/** 
 * Obtain a backtrace and print it to stdout.
 * If signum != 0, call Terminate()
 */
void print_trace(int signum)
{

#if (!defined __WIN32__) && (!defined __APPLE__) && (defined HAVE_BACKTRACE)

	// fprintf ( stderr , "print_trace:  Now attempting backtrace from within the code!\n" );
	// fprintf ( stderr , "print_trace:  Allowing a maximum of %d function calls on the stack!\n" , MAX_CALLS_IN_BACKTRACE );

	// We attempt to get a backtrace of all function calls so far, even
	// including the operating system (or rather libc) call to main() in 
	// the beginning of execution.
	//
	backtrace_size = backtrace(backtrace_array, MAX_CALLS_IN_BACKTRACE);

	fprintf(stderr, "print_trace:  Obtained %zu stack frames.\n", backtrace_size);

	// Now we attempt to translate the trace information we've got to the
	// symbol names that might still reside in the binary.
	//
	// NOTE: that in order for this to work, the -rdynamic switch must have
	//       been passed as on option to the LINKER!
	//       Also there might be a problem with non-ELF binaries, but let's
	//       hope that it still works...
	//
	backtrace_strings = backtrace_symbols(backtrace_array, backtrace_size);

	fprintf(stderr, "print_trace:  Obtaining symbols now done.\n");

	for (backtrace_counter = 0; backtrace_counter < backtrace_size; backtrace_counter++)
		fprintf(stderr, "%s\n", backtrace_strings[backtrace_counter]);

	// The strings generated in the backtrace_symbols function need to 
	// get freed.  Well, this isn't terribly important, but clean.
	//
	free(backtrace_strings);

#endif

	switch (signum) {
	case 0:
		return;
		break;
	case SIGSEGV:
		fprintf(stderr, "\n%s():  received SIGSEGV!\n", __FUNCTION__);
		break;
	case SIGFPE:
		fprintf(stderr, "\n%s():  received SIGFPE!\n", __FUNCTION__);
		break;
	default:
		fprintf(stderr, "\n%s():  received UNKNOWN SIGNAL %d!  ERROR! \n", __FUNCTION__, signum);
	}

	Terminate(EXIT_FAILURE);

};				// void print_trace ( int sig_num )

/**
 * If we want the screen resolution to be a runtime option and not a 
 * compile time option any more, we must not use it as a constant.  That
 * means we must adapt the button positions to the current screen 
 * resolution at runtime to, so we do it in this function, which will be
 * involved at program startup.
 */
void adapt_button_positions_to_screen_resolution(void)
{
	int i;

	for (i = 0; i < MAX_MOUSE_PRESS_BUTTONS; i++) {
		if (AllMousePressButtons[i].button_rect.x < 0)
			AllMousePressButtons[i].button_rect.x += GameConfig.screen_width;
		if (AllMousePressButtons[i].button_rect.y < 0)
			AllMousePressButtons[i].button_rect.y += GameConfig.screen_height;
	}

	AllMousePressButtons[OPEN_CLOSE_SKILL_EXPLANATION_BUTTON].button_rect.x += CHARACTERRECT_X;

	AllMousePressButtons[MORE_STR_BUTTON].button_rect.x += CHARACTERRECT_X;
	AllMousePressButtons[MORE_MAG_BUTTON].button_rect.x += CHARACTERRECT_X;
	AllMousePressButtons[MORE_DEX_BUTTON].button_rect.x += CHARACTERRECT_X;
	AllMousePressButtons[MORE_VIT_BUTTON].button_rect.x += CHARACTERRECT_X;

	Droid_Image_Window.x = 48 * GameConfig.screen_width / 640;
	Droid_Image_Window.y = 44 * GameConfig.screen_height / 480;
	Droid_Image_Window.w = 130 * GameConfig.screen_width / 640;
	Droid_Image_Window.h = 172 * GameConfig.screen_height / 480;

	Full_User_Rect.x = 0;
	Full_User_Rect.y = 0;
	Full_User_Rect.w = GameConfig.screen_width;
	Full_User_Rect.h = GameConfig.screen_height;

	Cons_Text_Rect.x = 175;
	Cons_Text_Rect.y = 180;
	Cons_Text_Rect.w = GameConfig.screen_width - 175;
	Cons_Text_Rect.h = 305;

};				// void adapt_button_positions_to_screen_resolution( void )

/**
 * This is a useful utility sub-function for the checks whether the
 * mouse cursor is on an enemy, a closed chest or a barrel, or any other object.
 * The function detects if the current mouse cursor is over the graphics
 * mentioned in that iso image, using the given position from the
 * parameter list.
 *
 * TRUE or FALSE is returned, depending on whether the cursor IS or
 * IS NOT on that particular iso_image, if positioned on that given spot.
 */
int mouse_cursor_is_on_that_image(float pos_x, float pos_y, struct image *our_image)
{
	// our_iso_image = & ( enemy_iso_images [ RotationModel ] [ RotationIndex ] [ (int) this_bot -> animation_phase ] ) ;
	SDL_Rect screen_rectangle;

	screen_rectangle.x = translate_map_point_to_screen_pixel_x(pos_x, pos_y) + our_image->offset_x;
	screen_rectangle.y = translate_map_point_to_screen_pixel_y(pos_x, pos_y) + our_image->offset_y;
	screen_rectangle.w = our_image->w;
	screen_rectangle.h = our_image->h;

	if (MouseCursorIsInRect(&(screen_rectangle),
				input_axis.x + User_Rect.w / 2 + User_Rect.x, input_axis.y + User_Rect.h / 2 + User_Rect.y)) {
		return (TRUE);
	}

	return (FALSE);
}

/**
 * This function checks if a given screen position lies within the 
 * inventory screen toggle button or not.
 */
int MouseCursorIsInRect(const SDL_Rect *our_rect, int x, int y)
{
	// Now we can start to check if the mouse cursor really is on that
	// rectangle or not.
	//
	if (x > our_rect->x + our_rect->w)
		return (FALSE);
	if (x < our_rect->x)
		return (FALSE);
	if (y > our_rect->y + our_rect->h)
		return (FALSE);
	if (y < our_rect->y)
		return (FALSE);

	// So since the cursor is not outside of this rectangle, it must
	// we inside, and so we'll return this answer.
	//
	return (TRUE);

};				// int MouseCursorIsInRect( SDL_rect* our_rect , int x , int y )

/**
 * This function checks if a given screen position lies within the 
 * inventory screen toggle button or not.
 */
int MouseCursorIsOnButton(int ButtonIndex, int x, int y)
{
	SDL_Rect temp_rect;

	// First a sanity check if the button index given does make
	// some sense.
	//
	if ((ButtonIndex >= MAX_MOUSE_PRESS_BUTTONS) || (ButtonIndex < 0)) {
		error_message(__FUNCTION__, "\
A Button that should be checked for mouse contact was requested, but the\n\
button index given exceeds the number of buttons defined in FreedroidRPG.", PLEASE_INFORM | IS_FATAL);
	}

	Copy_Rect(AllMousePressButtons[ButtonIndex].button_rect, temp_rect);
	// If this button needs scaling still, then we do it now...
	//
	if (AllMousePressButtons[ButtonIndex].scale_this_button) {
		temp_rect.x *= ((float)GameConfig.screen_width) / 640.0;
		temp_rect.w *= ((float)GameConfig.screen_width) / 640.0;
		temp_rect.y *= ((float)GameConfig.screen_height) / 480.0;
		temp_rect.h *= ((float)GameConfig.screen_height) / 480.0;
	}

	if (y < AllMousePressButtons[ButtonIndex].button_rect.y)
		return (FALSE);

	// So since the cursor is not outside of this rectangle, it must
	// we inside, and so we'll return this answer.
	//
	return (MouseCursorIsInRect(&(temp_rect), x, y));

};				// int MouseCursorIsOnButton( int ButtonIndex , int x , int y )

/**
 * Draw a button on the screen.
 */
void ShowGenericButtonFromList(int ButtonIndex)
{
	struct mouse_press_button *btn;

	// Safety check
	if ((ButtonIndex >= MAX_MOUSE_PRESS_BUTTONS) || (ButtonIndex < 0)) {
		error_message(__FUNCTION__, "Request to display button index %d could not be fulfilled: the\n\
				button index given exceeds the number of buttons defined in FreedroidRPG.", PLEASE_INFORM, ButtonIndex);
		return;
	}

	btn = &AllMousePressButtons[ButtonIndex];

	// Some buttons have no graphics, in this case there is nothing to do.
	if (!strcmp(AllMousePressButtons[ButtonIndex].button_image_file_name, "THIS_DOESNT_NEED_BLITTING")) {
		return;
	}

	// Compute scaling factors for button
	float scale_x = 1.0, scale_y = 1.0;
	if (btn->scale_this_button) {
		scale_x = ((float)GameConfig.screen_width) / 640.0;
		scale_y = ((float)GameConfig.screen_height) / 480.0;
	}

	// Load button image if required
	struct image *img = &btn->button_image;
	if (!image_loaded(img)) {
		load_image(img, GUI_DIR, btn->button_image_file_name, NO_MOD);

		// Maybe we had '0' entries for the height or width of this button in the list.
		// This means that we will take the real width and the real height from the image
		// and overwrite the 0 entries with this.
		//
		if (!btn->button_rect.w) {
			btn->button_rect.w = img->w;
		}

		if (!btn->button_rect.h) {
			btn->button_rect.h = img->h;
		}
	}

	display_image_on_screen(img, AllMousePressButtons[ButtonIndex].button_rect.x * scale_x, AllMousePressButtons[ButtonIndex].button_rect.y * scale_y, set_image_transformation(scale_x, scale_y, 1.0, 1.0, 1.0, 1.0, 0));
}

int init_data_dirs_path()
{
	int i, j;
	char file_path[PATH_MAX];
	const int FIRST_DATA_DIR = 1; // data_dirs[0] is CONFIG_DIR, set in prepare_execution()

	// Reset the data dirs paths
	for (i = FIRST_DATA_DIR; i < LAST_DATA_DIR; i++) {
		data_dirs[i].path[0] = '\0';
	}

	// Directories to look for the data dirs
	char *top_data_dir[]   = { ".", "..", "../..", FD_DATADIR };
#ifdef ENABLE_NLS
	char *top_locale_dir[] = { ".", "..", "../..", LOCALEDIR };
#endif
	int slen = sizeof(top_data_dir)/sizeof(top_data_dir[0]);

	// To find the root of the data dirs, we search a well known file that is
	// always needed for the game to work.
	for (i = 0; i < slen ; i++) {
		sprintf(file_path, "%s/" WELL_KNOWN_DATA_FILE, top_data_dir[i]);
		FILE *f = fopen(file_path, "r");
		if (f != NULL) {
			// File found, so now fill the data dir paths
			for (j = FIRST_DATA_DIR; j < LAST_DATA_DIR; j++) {
				char *dir = top_data_dir[i];
				const char *subdir = data_dirs[j].name;
#ifdef ENABLE_NLS
				if (j == LOCALE_DIR) {
					dir = top_locale_dir[i];
					// LOCALEDIR envvar already points to the locale subdir
					if (!strcmp(dir, LOCALEDIR))
						subdir = "";
				}
#endif
				int nb = snprintf(data_dirs[j].path, PATH_MAX, "%s/%s", dir, subdir);
				if (nb >= PATH_MAX) {
					error_message(__FUNCTION__, "data_dirs[].path is not big enough to store the following path: %s/%s",
					             PLEASE_INFORM | IS_FATAL, dir, data_dirs[j].name);
				}
			}
			fclose(f);
			return 1;
		}
	}

	// The searched file was not found. Complain.
	if (getcwd(file_path, PATH_MAX)) {
		error_message(__FUNCTION__, "Data dirs not found ! (current directory: %s)",
	                 PLEASE_INFORM | IS_FATAL, file_path);
	} else {
		error_message(__FUNCTION__, "Data dirs not found ! (and cannot find the current working directory)",
	                 PLEASE_INFORM | IS_FATAL);
	}

	return 0;
}

/**
 * Check if a directory exists
 *
 * @param dirname            Subdir to check, relatively to the directory specified by subdir_handle
 * @param subdir_handle      Handle to one of the well known data subdirs
 * @param check_if_writable  If TRUE, also check if the directory is writable
 * @param error_report       If TRUE, report any error to the user, else be silent
 *
 * @return  0 is directory exists and is writable (if requested),
 *          1 if directory exists but is not writable (if requested),
 *          2 if directory does not exist,
 *         -1 in case of errors.
 */
int check_directory(const char *dirname, int subdir_handle, int check_if_writable, int error_report)
{
	if (subdir_handle < 0 || subdir_handle >= LAST_DATA_DIR) {
		error_message(__FUNCTION__, "Called with a wrong subdir handle (%d)", PLEASE_INFORM, subdir_handle);
		return -1;
	}

	char dir_path[PATH_MAX];
	int nb = snprintf(dir_path, PATH_MAX, "%s/%s", data_dirs[subdir_handle].path, dirname);
	if (nb >= PATH_MAX) {
		*dir_path = 0;
		error_message(__FUNCTION__, "Pathname too long (max is %d): %s/%s",
		              PLEASE_INFORM, PATH_MAX, data_dirs[subdir_handle].path, dirname);
		return -1;
	}

	// First check if directory exists
#ifdef __WIN32__
	int rtn = _access(dir_path, 0x0);
#else
	int rtn = access(dir_path, F_OK);
#endif

	if (rtn == -1) {
		error_message(__FUNCTION__, "Directory not found: %s", error_report | PLEASE_INFORM, dir_path);
		return 2;
	}

	// Then check if the directory is writable, if requested
	if (!check_if_writable)
		return TRUE;

#ifdef __WIN32__
	rtn = _access(dir_path, 0x06);
#else
	rtn = access(dir_path, W_OK);
#endif

	if (rtn == -1) {
		error_message(__FUNCTION__, "Directory not writable: %s", error_report | PLEASE_INFORM, dir_path);
		return 1;
	}

	return 0;
}

/* -----------------------------------------------------------------
 * check if a given filename exists in subdir.
 *
 * fills in the (ALLOC'd) string and returns 1 if okay, 0 on error.
 * file_path's length HAS to be PATH_MAX.
 * ----------------------------------------------------------------- */
static int _file_exists(char *fpath, const char *subdir, const char *fname, const char *fext)
{
	int nb;
	if (fext)
		nb = snprintf(fpath, PATH_MAX, "%s/%s%s", subdir, fname, fext);
	else
		nb = snprintf(fpath, PATH_MAX, "%s/%s", subdir, fname);

	if (nb >= PATH_MAX) {
		*fpath = '\0';
		if (fext)
			error_message(__FUNCTION__, "Pathname too long (max is %d): %s/%s%s", PLEASE_INFORM, PATH_MAX, subdir, fname, fext);
		else
			error_message(__FUNCTION__, "Pathname too long (max is %d): %s/%s", PLEASE_INFORM, PATH_MAX, subdir, fname);
		return 0;
	}

#ifdef __WIN32__
	int access_rtn = _access(fpath, 0x04);
#else
	int access_rtn = access(fpath, R_OK);
#endif

	if (access_rtn == -1) {
		/* not found */
		return 0;
	}

	return 1;
}

/**
 * Find a filename in subdir (using a data_dir handle).
 *
 * Fills in the fpath with <subdir>/<fname><fext>, and check if that
 * file exists.
 *
 * \param fpath           pre-alloc'd string, filled by the function. Its length HAS to be PATH_MAX
 * \param subdir_handle   index to one of the known data dirs
 * \param fname           file name
 * \param fext            file extension (including the '.') (NULL if no file extension is to be added)
 * \param error_report    error output flag
 *
 * \return 1 is the file exists, 0 otherwise
**/
int find_file(char *fpath, int subdir_handle, const char *fname, const char *fext, int error_report)
{
	if (subdir_handle < 0 || subdir_handle >= LAST_DATA_DIR) {
		error_message(__FUNCTION__, "Called with a wrong subdir handle (%d)",
		              error_report | PLEASE_INFORM, subdir_handle);
		return 0;
	}

	if (!_file_exists(fpath, data_dirs[subdir_handle].path, fname, fext)) {
		if (fext)
			error_once_message(ONCE_PER_RUN, __FUNCTION__, "File %s.%s not found in %s",
			                   error_report, fname, fext, data_dirs[subdir_handle].name);
		else
			error_once_message(ONCE_PER_RUN, __FUNCTION__, "File %s not found in %s",
			                   error_report, fname, data_dirs[subdir_handle].name);
		return 0;
	}

	return 1;
}

/* -----------------------------------------------------------------
 * Find a suffixed filename in subdir (using a data_dir handle).
 *
 * The 'suffix' is added before the filename extension.
 *
 * fills in the (ALLOC'd) string and returns 1 if okay, 0 on error.
 * file_path's length HAS to be PATH_MAX.
 * ----------------------------------------------------------------- */
int find_suffixed_file(char *fpath, int subdir_handle, const char *fname, const char *suffix, int error_report)
{
	char suffixed_fname[PATH_MAX];
	char *actual_fname = (char *)fname;

	if (suffix) {
		int fname_length = strlen(fname);
		int suffix_length = strlen(suffix);
		if ((fname_length + suffix_length + 1) >= PATH_MAX) {
			*fpath = 0;
			error_message(__FUNCTION__, "Filename + suffix too long (max is %d): %s, with suffix: %s",
			              PLEASE_INFORM, PATH_MAX, fname, suffix);
			return 0;
		}

		int pos = strrchr(fname, '.') - fname;

		memcpy(suffixed_fname, fname, pos);
		memcpy(suffixed_fname + pos, suffix, suffix_length);
		memcpy(suffixed_fname + pos + suffix_length, fname + pos, fname_length - pos + 1);
		suffixed_fname[fname_length + suffix_length] = '\0';

		actual_fname = suffixed_fname;
	}

	return find_file(fpath, subdir_handle, actual_fname, NULL, error_report);
}

/* -----------------------------------------------------------------
 * Find a localized version of a filename in subdir (using a data_dir handle).
 *
 * The localized versions are to be put in subdirs, using locale names.
 * For instances, map/titles/fr ou map/titles/de.
 *             
 * As with gettext(), generalizations of the locale name are tried
 * in turn. So if the locale is 'fr_FR', but the 'fr_FR' subdir does
 * not exists, then the 'fr' subdir is checked.
 *
 * fills in the (ALLOC'd) string and returns 1 if okay, 0 on error.
 * file_path's length HAS to be PATH_MAX.
 * ----------------------------------------------------------------- */
int find_localized_file(char *fpath, int subdir_handle, const char *fname, int error_report)
{
#ifdef ENABLE_NLS
	if (subdir_handle < 0 || subdir_handle >= LAST_DATA_DIR) {
		error_message(__FUNCTION__, "Called with a wrong subdir handle (%d)",
		              error_report | PLEASE_INFORM, subdir_handle);
		return 0;
	}

	char *used_locale = lang_get();

	if (!used_locale || strlen(used_locale) == 0) {
		return find_file(fpath, subdir_handle, fname, NULL, error_report);
	}

	// A locale name is typically of the form language[_territory][.codeset][@modifier]
	// We try each possible locale name in turn from the whole one to 'language' only.
	char *locale = strdup(used_locale);
	char *sep = "@._";
	int i;

	for (i = -1; i < (int)strlen(sep); i++) {
		// 'i == -1' is a special case, to use the full locale name
		if (i != -1) {
			char *ptr = strchr(locale, sep[i]);
			if (!ptr) {
				continue;
			}
			*ptr = '\0';
		}

		char l10ndir[PATH_MAX];
		int nb = snprintf(l10ndir, PATH_MAX, "%s/%s", data_dirs[subdir_handle].path, locale);
		if (nb >= PATH_MAX) {
			error_message(__FUNCTION__, "Dirname too long (max is %d): %s/%s - Using untranslated version of %s",
			             error_report, PATH_MAX, data_dirs[subdir_handle].path, locale, fname);
			break;
		}
		if (_file_exists(fpath, l10ndir, fname, NULL)) {
			free(locale);
			return 1;
		}
	}

	free(locale);
#endif

	// Localized version not found. Use untranslated version.
	return find_file(fpath, subdir_handle, fname, NULL, error_report);
}

int find_encoded_file(char *fpath, int subdir_handle, const char *fname, int error_report)
{
#ifdef ENABLE_NLS
	if (subdir_handle < 0 || subdir_handle >= LAST_DATA_DIR) {
		error_message(__FUNCTION__, "Called with a wrong subdir handle (%d)",
		              error_report | PLEASE_INFORM, subdir_handle);
		return 0;
	}

	char *used_encoding = lang_get_encoding();

	if (!used_encoding || !strlen(used_encoding) || !strcmp(used_encoding, "ASCII")) {
		return find_file(fpath, subdir_handle, fname, NULL, error_report);
	}

	char encoded_dir[PATH_MAX];
	int nb = snprintf(encoded_dir, PATH_MAX, "%s/%s", data_dirs[subdir_handle].path, used_encoding);
	if (nb >= PATH_MAX) {
		error_message(__FUNCTION__, "Dirname too long (max is %d): %s/%s - Using default encoding version of %s",
		             error_report, PATH_MAX, data_dirs[subdir_handle].path, used_encoding, fname);
		return find_file(fpath, subdir_handle, fname, NULL, error_report);
	}
	if (_file_exists(fpath, encoded_dir, fname, NULL))
		return TRUE;
#endif

	// Encoded version not found. Use default encoding version.
	return find_file(fpath, subdir_handle, fname, NULL, error_report);
}

/**
 * This function realizes the Pause-Mode: the game process is halted,
 * while the graphics and animations are not.  This mode 
 * can further be toggled from PAUSE to CHEESE, which is
 * a feature from the original program that should probably
 * allow for better screenshots.
 */
void Pause(void)
{
	int Pause = TRUE;
	int cheese = FALSE;	/* cheese mode: do not display GAME PAUSED  - nicer screenshots */
	SDL_Event event;
	SDLKey key;

	Activate_Conservative_Frame_Computation();

	AssembleCombatPicture(DO_SCREEN_UPDATE | SHOW_ITEMS);

	input_get_keybind("pause", &key, NULL);

	while (Pause) {
		SDL_WaitEvent(&event);

		if (event.type == SDL_QUIT) {
			Terminate(EXIT_SUCCESS);
		}

		AssembleCombatPicture(SHOW_ITEMS);
		if (!cheese) {
			put_string_centered(Menu_Font, 200, _("GAME PAUSED"));
			put_string_centered(Menu_Font, 230, _("press p to resume"));
		}
		our_SDL_flip_wrapper();

		if (event.type == SDL_KEYDOWN) {
			if (event.key.keysym.sym == key) {
				Pause = FALSE;
			} else if (event.key.keysym.sym == SDLK_c) {
				cheese = !cheese;
			}
		}

		SDL_Delay(10);
	}

	return;
}

/**
 * This function prevents any action from taking place in the game world.
 *
 * Interaction with the user interface is unaffected.
 */
void freeze_world()
{
	// Because different UI elements may try to freeze the game world,
	// it's necessary to count how many times this function has been
	// called.
	world_is_frozen++;
}

/**
 * This function unfreezes the game world.
 *
 * NOTE: unfreeze_world() must be called for each call to freeze_world().
 */
void unfreeze_world()
{
	if (world_is_frozen > 0)
		world_is_frozen--;
}

/**
 * This function returns the current state of the game world.
 * @return TRUE if the game world is currently frozen.
 */
int world_frozen()
{
	return (world_is_frozen > 0);
}

/**
 * This function starts the time-taking process.  Later the results
 * of this function will be used to calculate the current framerate
 */
void StartTakingTimeForFPSCalculation(void)
{
	/* This ensures, that 0 is never an encountered framenr,
	 * therefore count to 100 here
	 * Take the time now for calculating the frame rate
	 * (DO NOT MOVE THIS COMMAND PLEASE!) */
	framenr++;

	One_Frame_SDL_Ticks = SDL_GetTicks();
	if (framenr % 10 == 1)
		Ten_Frame_SDL_Ticks = SDL_GetTicks();
	if (framenr % 100 == 1) {
		Onehundred_Frame_SDL_Ticks = SDL_GetTicks();
	}
};				// void StartTakingTimeForFPSCalculation(void)

/**
 * This function computes the framerate that has been experienced
 * in this frame.  It will be used to correctly calibrate all 
 * movements of game objects.
 * 
 * NOTE:  To query the actual framerate a DIFFERENT function must
 *        be used, namely Frame_Time().
 */
void ComputeFPSForThisFrame(void)
{

	Now_SDL_Ticks = SDL_GetTicks();
	oneframedelay = Now_SDL_Ticks - One_Frame_SDL_Ticks;

	if (!oneframedelay)
		FPSover1 = 1000 * 1 / 0.5;
	else
		FPSover1 = 1000 * 1 / (float)oneframedelay;
}

/**
 *
 * This function is the key to independence of the framerate for various game elements.
 * It returns the average time needed to draw one frame.
 * Other functions use this to calculate new positions of moving objects, etc..
 *
 * Also there is of course a serious problem when some interruption occurs, like e.g.
 * the options menu is called or the debug menu is called or the console or the elevator
 * is entered or a takeover game takes place.  This might cause HUGE framerates, that could
 * box the influencer out of the ship if used to calculate the new position.
 *
 * To counter unwanted effects after such events we have the SkipAFewFramerates counter,
 * which instructs Rate_To_Be_Returned to return only the overall default framerate since
 * no better substitute exists at this moment.  But on the other hand, this seems to
 * work REALLY well this way.
 *
 * This counter is most conveniently set via the function Activate_Conservative_Frame_Computation,
 * which can be conveniently called from eveywhere.
 *
 */
float Frame_Time(void)
{
	float Rate_To_Be_Returned;

	if (SkipAFewFrames) {
		Rate_To_Be_Returned = Overall_Average;
		return Rate_To_Be_Returned;
	}

	Rate_To_Be_Returned = (1.0 / FPSover1);

	return Rate_To_Be_Returned;
}

/**
 * 
 * With framerate computation, there is a problem when some interruption occurs, like e.g.
 * the options menu is called or the debug menu is called or the console or the elevator
 * is entered or a takeover game takes place.  This might cause HUGE framerates, that could
 * box the influencer out of the ship if used to calculate the new position.
 *
 * To counter unwanted effects after such events we have the SkipAFewFramerates counter,
 * which instructs Rate_To_Be_Returned to return only the overall default framerate since
 * no better substitute exists at this moment.
 *
 * This counter is most conveniently set via the function Activate_Conservative_Frame_Computation,
 * which can be conveniently called from everywhere.
 *
 */
void Activate_Conservative_Frame_Computation(void)
{
	// SkipAFewFrames=212;
	// SkipAFewFrames=22;
	SkipAFewFrames = 3;

	DebugPrintf(1, "\nConservative_Frame_Computation activated!");

};				// void Activate_Conservative_Frame_Computation(void)

/*
 * Should be called in every frame when counting FPS
 */
void update_frames_displayed(void)
{
	// The next couter counts the frames displayed by FreedroidRPG during this
	// whole run!!  DO NOT RESET THIS COUNTER WHEN THE GAME RESTARTS!!
	Overall_Frames_Displayed++;
	Overall_Average = (Overall_Average * (Overall_Frames_Displayed - 1)
			   + Frame_Time()) / Overall_Frames_Displayed;

	if (SkipAFewFrames)
		SkipAFewFrames--;
}

/**
 * This function is used to generate an integer in range of all
 * numbers from 0 to upper_bound.
 */
int MyRandom(int upper_bound)
{
	if (upper_bound == 0)
		return 0;

	float tmp;
	int pure_random;
	int dice_val;		/* the result in [0, UpperBound] */

	pure_random = rand();
	tmp = 1.0 * pure_random / RAND_MAX;	/* random number in [0;1] */

	/* 
	 * we always round OFF for the resulting int, therefore
	 * we first add 0.99999 to make sure that UpperBound has
	 * roughly the same probablity as the other numbers 
	 */
	dice_val = (int)(tmp * (1.0 * upper_bound + 0.99999));

	return (dice_val);
}

/**
 * This function teleports the influencer to a new position on the
 * ship.  THIS CAN BE A POSITION ON A DIFFERENT LEVEL.
 */
void Teleport(int LNum, float X, float Y, int with_sound_and_fading, int with_animation_reset)
{
	int old_lvl = Me.pos.z;

	// Check if we are in editor and not in game test mode then we store the level number
	//
	if(game_root_mode == ROOT_IS_LVLEDIT && game_status != INSIDE_GAME)
		GameConfig.last_edited_level = LNum;
	// Maybe the 'teleport' really comes from a teleportation device or
	// teleport spell or maybe even from accessing some sewer access way.
	// In that case we'll fade out the screen a bit using the gamma ramp
	// and then later back in again.  (Note that this is a blocking function
	// call, i.e. it will take a second or so each.)
	//
	if (with_sound_and_fading) {
		fade_out_screen();
	}

	if (LNum != Me.pos.z) {

		// In case a real level change has happened,
		// we need to do a lot of work.  Therefore we start by activating
		// the conservative frame time computation to avoid a 'jump'.
		//
		Activate_Conservative_Frame_Computation();

		Me.pos.x = X;
		Me.pos.y = Y;
		Me.pos.z = LNum;

		item_held_in_hand = NULL;

		// We add some sanity check against teleporting to non-allowed
		// locations (like outside of map that is)
		//
		if (!level_exists(LNum) || !pos_inside_level(Me.pos.x, Me.pos.y, curShip.AllLevels[LNum])) {
			fprintf(stderr, "\n\ntarget location was: lev=%d x=%f y=%f.\n", LNum, X, Y);
			fprintf(stderr, "source location was: lev=%d x=%f y=%f.", Me.pos.z, Me.pos.x, Me.pos.y);
			error_message(__FUNCTION__, "\
A Teleport was requested, but the location to teleport to lies beyond\n\
the bounds of this 'ship' which means the current collection of levels.\n\
This indicates an error in the map system of FreedroidRPG.", PLEASE_INFORM | IS_FATAL);
		}

		// Refresh some speed-up data structures
		get_visible_levels();

	} else {
		// If no real level change has occurred, everything
		// is simple and we just need to set the new coordinates, haha
		//
		Me.pos.x = X;
		Me.pos.y = Y;

		// Teleport could have been called by the leveleditor, due to
		// some changes in the current level (light values, for example),
		// so we refresh the speed-up data structures
		get_visible_levels();
	}

	// After the teleport, the mouse move target might be
	// completely out of date.  Therefore we simply delete it.  In cases
	// where the jump came from crossing a jump threshold (levels glued
	// together) we can still restore the move target in that (the calling!)
	// function.
	//
	Me.mouse_move_target.x = Me.pos.x;
	Me.mouse_move_target.y = Me.pos.y;
	Me.mouse_move_target.z = Me.pos.z;

	Me.mouse_move_target_combo_action_type = NO_COMBO_ACTION_SET;

	// Animate Tux as standing still
	if (with_animation_reset) {
		Me.walk_cycle_phase = 0.0;
		Me.phase = tux_anim.standing_keyframe;
	}

	if (with_sound_and_fading) {
		teleport_arrival_sound();
	}
	// Perhaps the player is visiting this level for the first time.  Then, the
	// tux should make it's initial statement about the location, if there is one.
	//
	if (!Me.HaveBeenToLevel[Me.pos.z]) {
		Me.HaveBeenToLevel[Me.pos.z] = TRUE;
	}

	switch_background_music(CURLEVEL()->Background_Song_Name);

	// Since we've mightily changed position now, we should clear the
	// position history, so that no one gets confused...
	//
	InitInfluPositionHistory();

	if (with_sound_and_fading) {
		append_new_game_message(_("Arrived at %s."), D_(curShip.AllLevels[Me.pos.z]->Levelname));
		fade_in_screen();
	}

	if (game_status == INSIDE_GAME) {
		// Notify level change events on this level.
		if (LNum != old_lvl)
			event_level_changed(old_lvl, LNum);

		// Notify position changed.
		event_position_changed(Me.pos, TRUE);
	}
}

/**
 * Teleport the influencer to the center of a level on the ship
 * \param level_num The number of the level where we want to be teleported
 */
void teleport_to_level_center(int level_num)
{
	// Calculate the center of the level
	float x = curShip.AllLevels[level_num]->xlen / 2;
	float y = curShip.AllLevels[level_num]->ylen / 2;

	// Teleporting to the center of the level
	Teleport(level_num, x, y, FALSE, TRUE);
}

/**
 * Check if a level exists.
 * \param level_num The number of the level.
 * \return TRUE if the level exists.
*/
int level_exists(int level_num)
{
	if (level_num < 0 || level_num >= curShip.num_levels) {
		return FALSE;
	}

	if (curShip.AllLevels[level_num] == NULL) {
		return FALSE;
	}

	return TRUE;
}

/*----------------------------------------------------------------------
 * load_game_config: load saved options from config-file
 *
 * this should be the first of all load/save functions called
 * as here we read the $HOME-dir and create the config-subdir if necessary
 *
 *----------------------------------------------------------------------*/
int load_game_config(void)
{
	char fname[PATH_MAX];
	FILE *configfile;

	if (!strlen(data_dirs[CONFIG_DIR].path)) {
		return OK;
	}

	find_file(fname, CONFIG_DIR, "fdrpg.cfg", NULL, SILENT);
	if ((configfile = fopen(fname, "rb")) == NULL) {
		fprintf(stderr, "\nUnable to open configuration file %s\n", fname);
		lang_set(GameConfig.locale, NULL);
		return ERR;
	}

	char *stuff = (char *)malloc(FS_filelength(configfile) + 1);
	if (fread(stuff, FS_filelength(configfile), 1, configfile) != 1) {
		error_message(__FUNCTION__, "\nFailed to read config file: %s.", NO_REPORT, fname);
		fclose(configfile);
		free(stuff);
		return ERR;
	}
	stuff[FS_filelength(configfile)] = 0;
	fclose(configfile);

	if (setjmp(saveload_jmpbuf)) {
		error_message(__FUNCTION__, "Failed to read config file: %s.", NO_REPORT, fname);
		configfile = NULL;
		free(stuff);
		ResetGameConfigToDefaultValues();
		return ERR;
	}

	// GameConfig.locale is initialized (in ResetGameConfigToDefaultValues())
	// before the configuration is loaded (so that any error message can be
	// translated). The memory pointed to by GameConfig.locale will be lost
	// when loading the configuration. So we have to free it, if needed.
	char *tmp_ptr = GameConfig.locale;
	load_freedroid_configuration(stuff);
	if (tmp_ptr && tmp_ptr != GameConfig.locale)
		free(tmp_ptr);
	lang_set(GameConfig.locale, NULL);

	configfile = NULL;
	free(stuff);

	if (!GameConfig.freedroid_version_string || strcmp(GameConfig.freedroid_version_string, VERSION)) {
		error_message(__FUNCTION__,
		              "Settings file found in your ~/.freedroid_rpg dir does not\n"
		              "seem to be from the same version as this installation of FreedroidRPG.\n"
		              "This is perfectly normal if you have just upgraded your version of\n"
		              "FreedroidRPG.  However, the loading of your settings will be canceled now,\n"
		              "because the format of the settings file is no longer supported.\n"
		              "No need to panic.  The default settings will be used instead and a new\n"
		              "settings file will be generated.",
		              NO_REPORT);
		ResetGameConfigToDefaultValues();
		return ERR;
	};

	// Now we will turn off the skills and inventory screen and that, cause
	// this should be off when the game starts...
	//
	GameConfig.CharacterScreen_Visible = FALSE;
	GameConfig.Inventory_Visible = FALSE;
	GameConfig.SkillScreen_Visible = FALSE;
	GameConfig.skill_explanation_screen_visible = FALSE;
	GameConfig.Automap_Visible = TRUE;

	return OK;
}

/*----------------------------------------------------------------------
 * save_game_config: do just that
 * Return: -1 on error, -2 if immediate exit is needed, 0 otherwise
 *----------------------------------------------------------------------*/
int save_game_config(void)
{
	char fname[PATH_MAX];
	int current_width;
	int current_height;
	FILE *config_file;

	// Maybe the Terminate function was invoked BEFORE the startup process
	// was complete at all (like e.g. some illegal command line parameter).
	// Then the config dir is not initialized.  We catch this case and return
	// control to the operating system immediately if that happens...

	if (!strlen(data_dirs[CONFIG_DIR].path)) {
		return -2;
	}

	// Now we know, that the config dir has been initialized already.
	// That indicates, that the game did start up already.
	// Therefore we can do the normal save config stuff...

	find_file(fname, CONFIG_DIR, "fdrpg.cfg", NULL, SILENT);
	if ((config_file = fopen(fname, "wb")) == NULL) {
		DebugPrintf(-4, "Unable to open configuration file %s for writing\n", fname);
		return -1;
	}

	// We put the current version number of FreedroidRPG into the 
	// version number string.  This will be useful so that later
	// versions of FreedroidRPG can identify old config files and decide
	// not to use them in some cases.

	if (GameConfig.freedroid_version_string) {
		free(GameConfig.freedroid_version_string);
	}
	GameConfig.freedroid_version_string = strdup(VERSION);

	// We preserve the current resolution, modify it a bit, such that
	// the preselected resolution will come to effect next time, save
	// it and then we restore the current settings again.

	current_width = GameConfig.screen_width;
	current_height = GameConfig.screen_height;
	GameConfig.screen_width = GameConfig.next_time_width_of_screen;
	GameConfig.screen_height = GameConfig.next_time_height_of_screen;

	// Now write the actual data

	savestruct_autostr = alloc_autostr(4096);
	save_freedroid_configuration(savestruct_autostr);
	if (fwrite(savestruct_autostr->value, savestruct_autostr->length, 1, config_file) != 1) {
		error_message(__FUNCTION__, "Failed to write configuration file: %s", NO_REPORT, fname);
		free_autostr(savestruct_autostr);
		fclose(config_file);
		return -1;
	}

	free_autostr(savestruct_autostr);

	GameConfig.screen_width = current_width;
	GameConfig.screen_height = current_height;
	fclose(config_file);

	return 0;
}

static void free_memory_before_exit(void)
{
	// free the entities
	clear_volatile_obstacles();
	clear_enemies();
	clear_npcs();
	free_tux();
	free_graphics();

	// free the widgets
	free_game_ui();
	free_lvledit_ui();
	free_chat_widgets();
	widget_free_image_resources();

	// free animations lists and visible levels
	reset_visible_levels();
	clear_animated_floor_tile_list();

	// other stuff
	delete_events();
	free_current_ship();
	leveleditor_cleanup();
	free_error_msg_store();
	gameconfig_clean();
}

/**
 * This function is used for terminating freedroid.  It will close
 * the SDL submodules and exit.
 */
void Terminate(int exit_code)
{
	if (!do_benchmark) {
		printf("\n---------------------------------------------------------------------------------");
		printf("\nTermination of freedroidRPG initiated... ");
	}

	// Save the config file only in case of success.

	if (exit_code == EXIT_SUCCESS) {
		if (save_game_config() == -2) {
			exit_code = -1;
			goto IMMEDIATE_EXIT;
		}
	}

	// Close active lua states, to force a call to garbage collector, in order
	// to call Lua binding 'destructors', and clean all the stuff

	close_lua();
	close_audio();
	free_memory_before_exit();

	if (!do_benchmark) {
		printf("Thank you for playing freedroidRPG.\n\n");
	}

IMMEDIATE_EXIT:

	if (SDL_WasInit(SDL_INIT_EVERYTHING))
		SDL_Quit();

	// If the game was not run from the command line, we should open an editor with
	// the last debug output, since people in general won't know how and where
	// to find the material for proper reporting of bugs.

	if (!run_from_term) {
		if (exit_code == EXIT_FAILURE) {
			int rtn;
			char outfn[PATH_MAX];
			fflush(stdout);
			fflush(stderr);
			find_file(outfn, CONFIG_DIR, "fdrpg_out.txt", NULL, NO_REPORT);
			char *cmd = MyMalloc(strlen(OPENTXT_CMD) + strlen(outfn) + 2); // +1 for the whitespace after the cmd name +1 for the terminating \0
			sprintf(cmd, "%s %s", OPENTXT_CMD, outfn);
			rtn = system(cmd);
			if (rtn == -1) // We use the return value mainly to avoid a compilation warning
				fprintf(stderr, "system call failed: \"%s\" returned %d", cmd, rtn);
			free(cmd);
		}
	}

	// Now we drop control back to the operating system.  The FreedroidRPG
	// program has finished.
	if (exit_code == EXIT_FAILURE)
		abort();
	exit(exit_code);
}

/**
 * Return a pointer towards the obstacle designated by the given (unique) label.
 * \param level_number If not NULL, this is set to the levelnumber where the obstacle was found.
 */
obstacle *give_pointer_to_obstacle_with_label(const char *obstacle_label, int *level_number)
{
	int i, j;

	// On each level, browse the obstacle extensions until we find the label we are looking for
	for (i = 0; i < curShip.num_levels; i++) {
		level *l = curShip.AllLevels[i];

		if (l == NULL)
			continue;

		for (j = 0; j < l->obstacle_extensions.size; j++) {
			struct obstacle_extension *ext = &ACCESS_OBSTACLE_EXTENSION(l->obstacle_extensions, j);

			if (ext->type == OBSTACLE_EXTENSION_LABEL) {
				if (!strcmp(ext->data, obstacle_label)) {
					if (level_number) {
						*level_number = l->levelnum;
					}
					return ext->obs;
				}
			}

		}
	}

	error_message(__FUNCTION__, "Obstacle label \"%s\" was not found on the map.", PLEASE_INFORM | IS_FATAL, obstacle_label);
	return NULL;
}

/*----------------------------------------------------------------------
 * Get the power of 2 greater than of equal to the argument
 * http://www-graphics.stanford.edu/~seander/bithacks.html#RoundUpPowerOf2
 *----------------------------------------------------------------------*/
uint32_t pot_gte(uint32_t v)
{
	uint32_t pot = v;
	--pot;
	pot |= pot >> 1;
	pot |= pot >> 2;
	pot |= pot >> 4;
	pot |= pot >> 8;
	pot |= pot >> 16;
	return (++pot);

}				// uint32_t pot_gte( uint32_t v)

/*
 * Some systems do not have setenv().
 * On those systems, we use putenv().
 */
int fd_setenv(const char *var, const char *val, int overwrite)
{
	int ret;

#ifdef HAVE_SETENV
	ret = setenv(var, val, overwrite);
#else
	// putenv does not make a copy of its argument, and inserts the pointer to
	// the argument into the environment array.
	// The following not trivial code is needed to avoid memleak.
	// (see POS34-C on https://www.securecoding.cert.org)
	static char *oldenv = NULL;
	const size_t len = strlen(var) + 1 + strlen(val) + 2;
	char *env = (char *)MyMalloc(len);
	snprintf(env, len, "%s=%s", var, val);
	ret = putenv(env);
	if (ret != 0) {
		free(env);
		error_message(__FUNCTION__, "Error when calling putenv() to set %s to %s\n", PLEASE_INFORM, var, val);
		return ret;
	}
	if (oldenv != NULL) {
		free(oldenv);
	}
	oldenv = env;
#endif
	return ret;
}

/*
 * Some systems do not have unsetenv().
 * On those systems, we use putenv().
 */
int fd_unsetenv(const char *var)
{
	int ret;

#ifdef HAVE_UNSETENV
	ret = unsetenv(var);
#else
	// See fdrpg_setenv() comment

	static char *oldenv = NULL;
	const size_t len = strlen(var) + 2;
	char *env = (char *)MyMalloc(len);
	snprintf(env, len, "%s=", var);
	ret = putenv(env);
	if (ret != 0) {
		free(env);
		error_message(__FUNCTION__, "Error when calling putenv() to unset %s\n", PLEASE_INFORM, var);
		return ret;
	}
	if (oldenv != NULL) {
		free(oldenv);
	}
	oldenv = env;
#endif
	return ret;
}

#undef _misc_c
