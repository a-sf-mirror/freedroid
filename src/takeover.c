/* 
 *
 *   Copyright (c) 2004-2010 Arthur Huillet 
 *   Copyright (c) 1994, 2002, 2003 Johannes Prix
 *   Copyright (c) 1994, 2002 Reinhard Prix
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
 * This file does everything that has to do with the takeover game from
 * the original paradroid game.
 */

#define _takeover_c 1

#include "system.h"

#include "defs.h"
#include "struct.h"
#include "global.h"
#include "proto.h"
#include "takeover.h"
#include "map.h"
#include "widgets/widgets.h"

Uint32 cur_time;		// current time in ms 

// Takeover images
static struct image FillBlocks[NUM_FILL_BLOCKS];
static struct image CapsuleBlocks[NUM_CAPS_BLOCKS];
static struct image ToGameBlocks[NUM_TO_BLOCKS];
static struct image ToGroundBlocks[NUM_GROUND_BLOCKS];
static struct image ToColumnBlock;
static struct image ToLeaderBlock;

//--------------------
// Class separation of the blocks 
//
int BlockClass[TO_BLOCKS] = {
	CONNECTOR,		// cable
	NON_CONNECTOR,		// end of cable
	CONNECTOR,		// re-enforcer
	CONNECTOR,		// color-change
	CONNECTOR,		// bridge above
	NON_CONNECTOR,		// bridge middle
	CONNECTOR,		// bridge below
	NON_CONNECTOR,		// uniter above
	CONNECTOR,		// uniter middle
	NON_CONNECTOR,		// uniter below
	NON_CONNECTOR		// empty
};

//--------------------
// Probability of the various elements 
//
#define MAX_PROB		100
int ElementProb[TO_ELEMENTS] = {
	100,			// EL_CABLE 
	2,			// EL_CABLE_END 
	5,			// EL_AMPLIFIER 
	5,			// EL_COLOR_EXCHANGER: only on last layer 
	5,			// EL_SEPARATOR 
	5			// EL_GATE 
};

int max_opponent_capsules;
int NumCapsules[TO_COLORS] = {
	0, 0
};

point LeftCapsulesStart[TO_COLORS] = {
	{YELLOW_LEFT_CAPSULES_X, YELLOW_LEFT_CAPSULES_Y},
	{PURPLE_LEFT_CAPSULES_X, PURPLE_LEFT_CAPSULES_Y}
};

point CurCapsuleStart[TO_COLORS] = {
	{YELLOW_CUR_CAPSULE_X, YELLOW_CUR_CAPSULE_Y},
	{PURPLE_CUR_CAPSULE_X, PURPLE_CUR_CAPSULE_Y}
};

point PlaygroundStart[TO_COLORS] = {
	{YELLOW_PLAYGROUND_X, YELLOW_PLAYGROUND_Y},
	{PURPLE_PLAYGROUND_X, PURPLE_PLAYGROUND_Y}
};

point DroidStart[TO_COLORS] = {
	{YELLOW_DROID_X, YELLOW_DROID_Y},
	{PURPLE_DROID_X, PURPLE_DROID_Y}
};

int CapsuleCurRow[TO_COLORS] = { 0, 0 };

int LeaderColor = YELLOW;		/* momentary leading color */
int LeaderCapsuleCount = 6;		/* capsule count of currently leading color */
int YourColor = YELLOW;
int OpponentColor = PURPLE;
int OpponentType;		/* The droid-type of your opponent */
enemy *cdroid;

/* the display  column */
int DisplayColumn[NUM_LINES] = {
	YELLOW, PURPLE, YELLOW, PURPLE, YELLOW, PURPLE, YELLOW, PURPLE, YELLOW, PURPLE,
	YELLOW, PURPLE
};

SDL_Color to_bg_color = { 199, 199, 199 };

playground_t ToPlayground;
playground_t ActivationMap;
playground_t CapsuleCountdown;

void EvaluatePlayground(void);
float EvaluatePosition(const int color, const int row, const int layer, const int endgame);
float EvaluateCenterPosition(const int color, const int row, const int layer, const int endgame);
void advanced_enemy_takeover_movements(const int countdown);

static void ShowPlayground(enemy *target);

static void display_takeover_help()
{
	play_title_file(BASE_TITLES_DIR, "TakeoverInstructions.lua");
}

/** 
 * Display the portrait of a droid
 */
static void show_droid_picture(int PosX, int PosY, int type)
{
	static char *last_gfx_prefix = NULL;
	static int last_rotation_nb = 0;
	static struct image *droid_images = NULL;

	if (Droidmap[type].portrait_rotations == 0)
		return;

	// Maybe we have to reload the whole image series

	if (!droid_images || !last_gfx_prefix || (last_gfx_prefix != Droidmap[type].gfx_prefix)) {
		int i;

		if (droid_images) {
			for (i = 0; i < last_rotation_nb; i++) {
				delete_image(&droid_images[i]);
			}
			free(droid_images);
			droid_images = NULL;
		}

		droid_images = (struct image *)MyMalloc(sizeof(struct image) * Droidmap[type].portrait_rotations);

		char filename[5000];
		for (i = 0; i < Droidmap[type].portrait_rotations; i++) {
			sprintf(filename, "%s/portrait_%04d.jpg", Droidmap[type].gfx_prefix, i + 1);
			load_image(&droid_images[i], GRAPHICS_DIR, filename, NO_MOD);
		}
			
		last_gfx_prefix = Droidmap[type].gfx_prefix;
		last_rotation_nb = Droidmap[type].portrait_rotations;
	}

	// Play the whole rotation in 1000 milliseconds.
	int rotation_index = ((SDL_GetTicks() * last_rotation_nb) / 1000) % last_rotation_nb;
	struct image *img = &droid_images[rotation_index];

	// Compute the maximum uniform scale to apply to the bot image so that it fills
	// the droid portrait image, and center the image.
	float scale = min((float)Droid_Image_Window.w / (float)img->w, (float)Droid_Image_Window.h / (float)img->h);
	pointf pos;
	pos.x = (float)PosX + ((float)Droid_Image_Window.w - (float)img->w * scale) / 2.0;
	pos.y = (float)PosY + ((float)Droid_Image_Window.h - (float)img->h * scale) / 2.0;

	display_image_on_screen(img, pos.x, pos.y, IMAGE_SCALE_TRANSFO(scale));
}

/**
 * Display infopage page of droidtype.
 */
static void show_droid_info(int droidtype)
{
	SDL_Rect clip;

	SDL_SetClipRect(Screen, NULL);

	// Show background
    blit_background("console_bg1.jpg");
	blit_background("takeover_browser.png");

	// Show droid portrait
	show_droid_picture(UNIVERSAL_COORD_W(45), UNIVERSAL_COORD_H(213), droidtype);

	// Show the droid name
	set_current_font(Menu_Font);
	clip.x = UNIVERSAL_COORD_W(330);
	clip.y = UNIVERSAL_COORD_H(57);
	clip.w = UNIVERSAL_COORD_W(200);
	clip.h = UNIVERSAL_COORD_H(30);
	display_text(D_(Droidmap[droidtype].default_short_description), clip.x, clip.y, &clip, 1.0);
}

/**
 * Initialize text widget with a description of the specified droidtype.
 */
static void init_droid_description(struct widget_text *w, int droidtype)
{
	const char *item_name;
	int weapon_type;

	autostr_append(w->text, _("Unit Type %s\n"), Droidmap[droidtype].droidname);
	autostr_append(w->text, _("Entry : %d\n"), droidtype + 1);

	if ((weapon_type = Droidmap[droidtype].weapon_id) >= 0)	// make sure item != -1
		item_name = D_(item_specs_get_name(weapon_type));	// does not segfault
	else
		item_name = _("none");

	autostr_append(w->text, _("\nArmament : %s\n"), item_name);
	autostr_append(w->text, _("\nCores : %d\n"), 2 + Droidmap[droidtype].class);


	if (Me.TakeoverSuccesses[droidtype]+Me.TakeoverFailures[droidtype]) {
		int success_ratio =
			((100*Me.TakeoverSuccesses[droidtype])/
			 (Me.TakeoverSuccesses[droidtype]+Me.TakeoverFailures[droidtype]));

		autostr_append(w->text, _("\nTakeover Success : %2d%%\n"), success_ratio);
	}

	autostr_append(w->text, _("\nNotes: %s\n"), D_(Droidmap[droidtype].notes));
}

/**
 * This function does the countdown where you still can changes your
 * color.
 */
static void ChooseColor(enemy *target)
{
	int countdown = 100;	// duration in 1/10 seconds given for color choosing 
	int ColorChosen = FALSE;
	char count_text[80];
	SDL_Event event;

	Uint32 prev_count_tick, count_tick_len;

	count_tick_len = 100;	// countdown in 1/10 second steps 

	prev_count_tick = SDL_GetTicks();

	while (!ColorChosen) {

		// wait for next countdown tick 
		while (SDL_GetTicks() < prev_count_tick + count_tick_len) ;

		prev_count_tick += count_tick_len;	// set for next tick 

		while (SDL_PollEvent(&event)) {

			if (event.type == SDL_QUIT) {
				Terminate(EXIT_SUCCESS);
			}

			if (event.type == SDL_MOUSEBUTTONDOWN) {
				switch (event.button.button) {
					//(clever?) hack : mouse wheel up and down behave
					//exactly like LEFT and RIGHT arrow, so we mangle the event
				case SDL_BUTTON_WHEELUP:
					event.type = SDL_KEYDOWN;
					event.key.keysym.sym = SDLK_LEFT;
					break;
				case SDL_BUTTON_WHEELDOWN:
					event.type = SDL_KEYDOWN;
					event.key.keysym.sym = SDLK_RIGHT;
					break;
				case SDL_BUTTON_LEFT:
					ColorChosen = TRUE;
					break;

				default:
					break;
				}
			}

			/* no else there! (mouse wheel) */
			if (event.type == SDL_KEYDOWN) {
				switch (event.key.keysym.sym) {
				case SDLK_RIGHT:
					YourColor = PURPLE;
					OpponentColor = YELLOW;
					break;
				case SDLK_LEFT:
					YourColor = YELLOW;
					OpponentColor = PURPLE;
					break;
				case SDLK_SPACE:
					ColorChosen = TRUE;
					break;
				default:
					break;
				}
			}

		}

		countdown--;	// Count down 
		sprintf(count_text, _("Color-%d.%d"), countdown/10, countdown%10);

		ShowPlayground(target);
		to_show_banner(count_text, NULL);
		our_SDL_flip_wrapper();

		if (countdown == 0)
			ColorChosen = TRUE;

	}			// while(!ColorChosen) 

	while (MouseLeftPressed())
		SDL_Delay(1);
};				// void ChooseColor ( void ) 

static void PlayGame(int countdown, enemy *target)
{
	char count_text[80];
	int FinishTakeover = FALSE;
	int row;

	Uint32 prev_count_tick, count_tick_len;	/* tick vars for count-down */
	Uint32 prev_move_tick, move_tick_len;	/* tick vars for motion */
	int wait_move_ticks;	/* number of move-ticks to wait before "key-repeat" */

	int up, down, set;
	int up_counter, down_counter;

	SDL_Event event;

	sprintf(count_text, " ");	/* Make sure a value gets assigned to count_text */
	count_tick_len = 100;	/* countdown in 1/10 second steps */
	move_tick_len = 60;	/* allow motion at this tick-speed in ms */

	up = down = set = FALSE;
	up_counter = down_counter = 0;

	wait_move_ticks = 2;

	prev_count_tick = prev_move_tick = SDL_GetTicks();	/* start tick clock */

	while (!FinishTakeover) {
		cur_time = SDL_GetTicks();

		/* 
		 * here we register if there have been key-press events in the
		 * "waiting period" between move-ticks :
		 */
		up = (up | UpPressed());
		down = (down | DownPressed());
		set = set | SpacePressed() | MouseLeftPressed();

		while (SDL_PollEvent(&event)) {
			if (event.type == SDL_MOUSEBUTTONDOWN) {
				switch (event.button.button) {
				case SDL_BUTTON_WHEELUP:
					up++;
					break;
				case SDL_BUTTON_WHEELDOWN:
					down++;
					break;
				default:
					break;
				}
			} else if (event.type == SDL_KEYDOWN) {
				/* allow for a WIN-key that gives immediate victory */
				event.key.keysym.mod &= ~(KMOD_CAPS | KMOD_NUM | KMOD_MODE);	/* We want to ignore "global" modifiers. */
				if (event.key.keysym.sym == SDLK_w && (event.key.keysym.mod == (KMOD_LCTRL | KMOD_LALT))) {
					LeaderColor = YourColor;	/* simple as that */
					return;	/* leave now, to avoid changing of LeaderColor! */
				}
			} else if (event.type == SDL_QUIT) {
				Terminate(EXIT_SUCCESS);
			}
		}

		if (!up)
			up_counter = 0;	/* reset counters for released keys */
		if (!down)
			down_counter = 0;

		if (cur_time > prev_count_tick + count_tick_len) {	/* time to count 1 down */
			prev_count_tick += count_tick_len;	/* set for next countdown tick */
			countdown--;
			sprintf(count_text, _("Finish-%d.%d"), countdown/10, countdown%10);

			if (countdown == 0)
				FinishTakeover = TRUE;

			AnimateCurrents();	/* do some animation on the active cables */

		}

		/* if (countdown_tick has occurred) */
		/* time for movement */
		if (cur_time > prev_move_tick + move_tick_len) {
			prev_move_tick += move_tick_len;	/* set for next motion tick */
			advanced_enemy_takeover_movements(countdown);

			if (up) {
				if (!up_counter || (up_counter > wait_move_ticks)) {
					// Here I have to change some things in order to make
					// mouse movement work properly with the wheel...
					//
					// CapsuleCurRow[YourColor]--;
					//
					CapsuleCurRow[YourColor] -= up;

					if (CapsuleCurRow[YourColor] < 1)
						CapsuleCurRow[YourColor] = NUM_LINES;
				}
				up = FALSE;
				up_counter++;
			}
			if (down) {
				if (!down_counter || (down_counter > wait_move_ticks)) {
					// Here I have to change some things in order to make
					// mouse movement work properly with the wheel...
					//
					// CapsuleCurRow[YourColor]++;
					//
					CapsuleCurRow[YourColor] += down;

					if (CapsuleCurRow[YourColor] > NUM_LINES)
						CapsuleCurRow[YourColor] = 1;
				}
				down = FALSE;
				down_counter++;
			}

			if (set && (NumCapsules[YOU] > 0)) {
				set = FALSE;
				row = CapsuleCurRow[YourColor] - 1;
				if ((row >= 0) &&
				    (ToPlayground[YourColor][0][row] != CABLE_END) && (ActivationMap[YourColor][0][row] == INACTIVE)) {
					NumCapsules[YOU]--;
					CapsuleCurRow[YourColor] = 0;
					ToPlayground[YourColor][0][row] = AMPLIFIER;
					ActivationMap[YourColor][0][row] = ACTIVE1;
					CapsuleCountdown[YourColor][0][row] = CAPSULE_COUNTDOWN * 2;

					Takeover_Set_Capsule_Sound();

					if (!NumCapsules[YOU]) {
						/* You placed your last capsule ? let's speed up the end */
						count_tick_len *= 0.75;
					}
				}	/* if (row > 0 && ... ) */
			}
			/* if ( set ) */
			ProcessCapsules();	/* count down the lifetime of the capsules */

			ProcessPlayground();
			ProcessPlayground();
			ProcessPlayground();
			ProcessPlayground();	/* this has to be done several times to be sure */

			ProcessDisplayColumn();

		}
		/* if (motion_tick has occurred) */
		ShowPlayground(target);
		to_show_banner(count_text, NULL);
		our_SDL_flip_wrapper();
		SDL_Delay(10);
	}			/* while !FinishTakeover */

	/* Final countdown */
	countdown = CAPSULE_COUNTDOWN + 10;

	while (countdown--) {
		// speed this up a little, some people get bored here...
		//      while ( SDL_GetTicks() < prev_count_tick + count_tick_len ) ;
		//      prev_count_tick += count_tick_len;
		ProcessCapsules();	/* count down the lifetime of the capsules */
		ProcessCapsules();	/* do it twice this time to be faster */
		//      AnimateCurrents ();
		ProcessPlayground();
		ProcessPlayground();
		ProcessPlayground();
		ProcessPlayground();	/* this has to be done several times to be sure */
		ProcessDisplayColumn();
		ShowPlayground(target);
		our_SDL_flip_wrapper();
	}			/* while (countdown) */

	return;

}

static void show_info_up_button() {
	ShowGenericButtonFromList(UP_BUTTON);
}

static void show_info_down_button() {
	ShowGenericButtonFromList(DOWN_BUTTON);
}

/**
 * This function manages the whole takeover game of Tux against 
 * some bot.
 *
 * The return value is TRUE/FALSE depending on whether the game was
 * finally won/lost.
 * The function also writes the ratio of capsules the player needed to win (out of all of his capsules) into needed_capsules_ratio
 * - Every capsule that has not been used within the game (numCapsules) is considered as not needed
 * - For every conquered row that exceeds 7 (the minimal number of rows to win) one capsule is considered as not needed
 */
int droid_takeover(struct enemy *target, float *needed_capsules_ratio)
{
	static struct widget_text droid_info;

	// Set up the droid description widget
	widget_text_init(&droid_info, "");
	init_droid_description(&droid_info, target->type);
	widget_set_rect(WIDGET(&droid_info), UNIVERSAL_COORD_W(258), UNIVERSAL_COORD_H(107), UNIVERSAL_COORD_W(346), UNIVERSAL_COORD_H(282));
	droid_info.font = FPS_Display_Font;
	droid_info.content_above_func = show_info_up_button;
	droid_info.content_below_func = show_info_down_button;

	// Prevent distortion of framerate by the delay coming from 
	// the time spent in the menu.
	Activate_Conservative_Frame_Computation();

	// We set the UserRect to full again, no matter what other windows might
	// be open right now...
	User_Rect.x = 0;
	User_Rect.y = 0;
	User_Rect.w = GameConfig.screen_width;
	User_Rect.h = GameConfig.screen_height;

	while (SpacePressed() || MouseLeftPressed()) ;	// make sure space is release before proceed 

	switch_background_music("Bleostrada.ogg");

	int menu_finished = FALSE;
	while (!menu_finished) {
		show_droid_info(target->type);
		widget_text_display(WIDGET(&droid_info));
		ShowGenericButtonFromList(TAKEOVER_HELP_BUTTON);
		set_current_font(Para_Font);
		put_string_centered(Para_Font, GameConfig.screen_height - 30, _("For more information, click the help button."));
		blit_mouse_cursor();
		our_SDL_flip_wrapper();

		SDL_Event event;
		while (SDL_PollEvent(&event)) {

			if (event.type == SDL_QUIT) {
				Terminate(EXIT_SUCCESS);
			}

			if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT) {
				if (MouseCursorIsOnButton(UP_BUTTON, GetMousePos_x(), GetMousePos_y())) {
					droid_info.scroll_offset--;
				} else if (MouseCursorIsOnButton(DOWN_BUTTON, GetMousePos_x(), GetMousePos_y())) {
					droid_info.scroll_offset++;
				} else if (MouseCursorIsOnButton(DROID_SHOW_EXIT_BUTTON, GetMousePos_x(), GetMousePos_y())) {
					menu_finished = TRUE;
				} else if (MouseCursorIsOnButton(TAKEOVER_HELP_BUTTON, GetMousePos_x(), GetMousePos_y())) {
					display_takeover_help();
				}
			} else if (event.type == SDL_KEYDOWN
				   && ((event.key.keysym.sym == SDLK_SPACE) || (event.key.keysym.sym == SDLK_ESCAPE))) {
				menu_finished = TRUE;
			}
		}
		SDL_Delay(1);
	}

	while (!(!SpacePressed() && !EscapePressed() && !MouseLeftPressed())) ;

	int player_capsules = 2 + Me.skill_level[get_program_index_with_name("Hacking")];
	max_opponent_capsules = 2 + Droidmap[target->type].class;

	if (do_takeover(player_capsules, max_opponent_capsules, 100, target)) {
		/* Won takeover */
		Me.marker = target->marker;

		int reward = Droidmap[target->type].experience_reward * Me.experience_factor;
		Me.Experience += reward;
		append_new_game_message(_("For taking control of your enemy, [s]%s[v], you receive %d experience."), D_(target->short_description_text), reward);

		event_enemy_hacked(target);

		// Maybe the enemy in question was a kind of 'boss monster' or it had
		// some special item, that is relevant to a mission or quest.  In that
		// case (like also when the bot is finally destroyed, the quest item
		// should be dropped after the successful takeover process, even if the
		// enemy isn't completely dead yet...
		//
		if (target->on_death_drop_item_code != (-1)) {
			DropItemAt(target->on_death_drop_item_code, target->pos.z, target->pos.x, target->pos.y, 1);
			target->on_death_drop_item_code = -1;
		}

		target->faction = FACTION_SELF;
		target->has_been_taken_over = TRUE;

		target->combat_state = WAYPOINTLESS_WANDERING;

		// Always use the AfterTakeover dialog
		free(target->dialog_section_name);
		target->dialog_section_name = strdup("AfterTakeover");

		// When the bot is taken over, it should not turn hostile when
		// the rest of his former combat group (identified by having the
		// same marker) is attacked by the Tux.
		//
		target->marker = 0;
		Me.TakeoverSuccesses[target->type]++;

	} else {
		Me.energy *= 0.5;
                Me.TakeoverFailures[target->type]++;
	}

	clear_screen();

	switch_background_music(CURLEVEL()->Background_Song_Name);

	if (LeaderColor == YourColor) {
		// set the ratio of needed capsules out of all (player) capsules
		// - Every capsule that has not been used within the game (numCapsules) is considered as not needed
		// - For every conquered row that exceeds 7 (the minimal number of rows to win) one capsule is considered as not needed
		*needed_capsules_ratio = 1 - (float)(NumCapsules[YOU] + LeaderCapsuleCount - 7) / (float)player_capsules;

		return TRUE;
	} else {
 		return FALSE;
	}
};				// int Takeover( int enemynum ) 

/*-----------------------------------------------------------------
 * This function performs the enemy movements in the takeover game,
 * but it does this in an advanced way, that has not been there in
 * the classic freedroid game.
 *-----------------------------------------------------------------*/
void advanced_enemy_takeover_movements(const int countdown)
{
	// static int Actions = 3;
	static int MoveProbability = 100;
	static int TurnProbability = 10;
	static int SetProbability = 80;

	int action;
	static int direction = 1;	/* start with this direction */
	int row = CapsuleCurRow[OpponentColor] - 1;
	int test_row;

	int BestTarget = -1;
	float BestValue = (-10000);	// less than any capsule can have

	int endgame = 0;
	if (countdown < NumCapsules[ENEMY]*7)
		endgame = 1;
        
#define TAKEOVER_MOVEMENT_DEBUG 1

	if (NumCapsules[ENEMY] == 0)
		return;

	// Disable AI waiting on easy
	if (GameConfig.difficulty_level !=  DIFFICULTY_EASY) {
		// Wait for the player to move
		if ((LeaderColor != YourColor) && ((NumCapsules[YOU]-NumCapsules[ENEMY]) >= 0) && (!endgame) && (NumCapsules[ENEMY] < max_opponent_capsules))
			return;
	}
        
	// First we're going to find out which target place is
	// best choice for the next capsule setting.

	for (test_row = 0; test_row < NUM_LINES; test_row++) {
		int test_value = EvaluatePosition(OpponentColor, test_row, 1, endgame) + 0.01*test_row;
		if (test_value > BestValue) {
			BestTarget = test_row;
			BestValue = test_value;
		}
	}
	DebugPrintf(TAKEOVER_MOVEMENT_DEBUG, "\nBest target row found : %d.", BestTarget);

	if ((BestValue < 0.5) && (!endgame) && (LeaderColor == OpponentColor)) //it isn't worth it
		return;
        
	// Now we can start to move into the right direction.

	if (row < BestTarget) {
		direction = 1;
		action = 0;
	} else if (row > BestTarget) {
		direction = -1;
		action = 0;
	} else {
		action = 2;
	}

	switch (action) {
	case 0:		/* Move along */
		if (MyRandom(100) <= MoveProbability) {
			row += direction;
			if (row > NUM_LINES - 1)
				row = 0;
			if (row < 0)
				row = NUM_LINES - 1;
		}
		break;

	case 1:		/* Turn around */
		if (MyRandom(100) <= TurnProbability) {
			direction *= -1;
		}
		break;

	case 2:		/* Try to set capsule */
		if (MyRandom(100) <= SetProbability) {
			if ((row >= 0) && 
			    (ToPlayground[OpponentColor][0][row] != CABLE_END) && (ActivationMap[OpponentColor][0][row] == INACTIVE)) {
				NumCapsules[ENEMY]--;
				Takeover_Set_Capsule_Sound();
				ToPlayground[OpponentColor][0][row] = AMPLIFIER;
				ActivationMap[OpponentColor][0][row] = ACTIVE1;
				CapsuleCountdown[OpponentColor][0][row] = CAPSULE_COUNTDOWN;
				row = -1;	/* For the next capsule: startpos */
			} else {
				row += direction;
			}
		}
		break;

	default:
		break;
	}

	CapsuleCurRow[OpponentColor] = row + 1;

	return;
}

static void GetTakeoverGraphics(void)
{
	static int TakeoverGraphicsAreAlreadyLoaded = FALSE;
	struct image img = EMPTY_IMAGE;
	int i, j;
	int curx = 0, cury = 0;
	SDL_Rect tmp;

	if (TakeoverGraphicsAreAlreadyLoaded)
		return;

	load_image(&img, GRAPHICS_DIR, TO_BLOCK_FILE, NO_MOD);

	// Get the fill-blocks 
	for (i = 0; i < NUM_FILL_BLOCKS; i++, curx += FILL_BLOCK_LEN + 2) {
		Set_Rect(tmp, curx, cury, FILL_BLOCK_LEN, FILL_BLOCK_HEIGHT);
		create_subimage(&img, &FillBlocks[i], &tmp);
	}

	// Get the capsule blocks 
	for (i = 0; i < NUM_CAPS_BLOCKS; i++, curx += CAPSULE_LEN + 2) {
		Set_Rect(tmp, curx, cury, CAPSULE_LEN, CAPSULE_HEIGHT);
		create_subimage(&img, &CapsuleBlocks[i], &tmp);
	}

	// Get the default background color, to be used when no background picture found! 
	curx = 0;
	cury += FILL_BLOCK_HEIGHT + 2;

	// get the game-blocks 
	for (j = 0; j < 2 * NUM_PHASES; j++) {
		for (i = 0; i < TO_BLOCKS; i++) {
			Set_Rect(tmp, curx, cury, TO_BLOCKLEN, TO_BLOCKHEIGHT);
			create_subimage(&img, &ToGameBlocks[j * TO_BLOCKS + i], &tmp);
			curx += TO_BLOCKLEN + 2;
		}
		curx = 0;
		cury += TO_BLOCKHEIGHT + 2;
	}

	// Get the ground, column and leader blocks 
	for (i = 0; i < NUM_GROUND_BLOCKS; i++) {
		Set_Rect(tmp, curx, cury, GROUNDBLOCKLEN, GROUNDBLOCKHEIGHT);
		create_subimage(&img, &ToGroundBlocks[i], &tmp);
		curx += GROUNDBLOCKLEN + 2;
	}
	cury += GROUNDBLOCKHEIGHT + 2;
	curx = 0;

	// Now the rectangle for the column blocks will be set and after
	// that we can create the new surface for blitting.
	//
	Set_Rect(tmp, curx, cury, COLUMNBLOCKLEN, COLUMNBLOCKHEIGHT);
	create_subimage(&img, &ToColumnBlock, &tmp);

	curx += COLUMNBLOCKLEN + 2;

	// Now the rectangle for the leader block will be set and after
	// that we can create the new surface for blitting.
	//
	Set_Rect(tmp, curx, cury, LEADERBLOCKLEN, LEADERBLOCKHEIGHT);
	create_subimage(&img, &ToLeaderBlock, &tmp);

	free_image_surface(&img);
	TakeoverGraphicsAreAlreadyLoaded = TRUE;
}

/**
 * Initiate the takeover game
 * @param game_length game time in tenths of a second
 * @param target a pointer to the enemy structure of the target. NULL means none
 * @return 1 on player success, 0 on player failure
 */
int do_takeover(int player_capsules, int opponent_capsules, int game_length, enemy* target)
{
	char *message;
	int player_won = 0;
	int FinishTakeover = FALSE;
	int row;
	int old_status;

	old_status = game_status;

	Activate_Conservative_Frame_Computation();

	// Maybe takeover graphics haven't been loaded yet.  Then we do this
	// here now and for once.  Later calls will be ignored inside the function.
	GetTakeoverGraphics();

	// eat pending events
	input_handle();

	while (!FinishTakeover) {
		// Init Color-column and Capsule-Number for each opponent and your color 
		//
		for (row = 0; row < NUM_LINES; row++) {
			DisplayColumn[row] = (row % 2);
			CapsuleCountdown[YELLOW][0][row] = -1;
			CapsuleCountdown[PURPLE][0][row] = -1;
		}		// for row 

		YourColor = YELLOW;
		OpponentColor = PURPLE;

		CapsuleCurRow[YELLOW] = 0;
		CapsuleCurRow[PURPLE] = 0;

		NumCapsules[YOU] = player_capsules;
		NumCapsules[ENEMY] = opponent_capsules;
		InventPlayground();

		EvaluatePlayground();

		ShowPlayground(target);
		our_SDL_flip_wrapper();

		ChooseColor(target);

		// This following function plays the takeover game, until one
		// of THREE states is reached, i.e. until YOU WON, YOU LOST
		// or until DEADLOCK is reached.  Well, so maybe after that
		// the takeover game is finished, but if it's a deadlock, then
		// the game must be played again in the next loop...
		//
		PlayGame(game_length, target);

		// We we evaluate the final score of the game.  Maybe we're done
		// already, maybe not...
		//
		if (LeaderColor == YourColor) {
			Takeover_Game_Won_Sound();
			message = _("Complete");
			FinishTakeover = TRUE;
			player_won = 1;
		} else if (LeaderColor == OpponentColor) {
			Takeover_Game_Lost_Sound();
			message = _("Rejected");
			FinishTakeover = TRUE;
			player_won = 0;
		} else {
			Takeover_Game_Deadlock_Sound();
			message = _("Deadlock");
		}

		ShowPlayground(target);
		to_show_banner(message, NULL);
		our_SDL_flip_wrapper();
		SDL_Delay(100);

	}

	game_status = old_status;
	return player_won;
}

/**
 * Draws the playground for the takeover game
 * @param target a pointer to the target's enemy structure. Use NULL for none
 */
static void ShowPlayground(enemy * target)
{
	int i, j;
	int color, player;
	int block;
	SDL_Rect Target_Rect;
	int xoffs = User_Rect.x + (User_Rect.w - 2 * 290) / 2;
	int yoffs = User_Rect.y + (User_Rect.h - 2 * 140) / 2 + 40;

	blit_background("console_bg1.jpg");
    
	static struct image bg;
	if (!image_loaded(&bg)) {
		load_image(&bg, GRAPHICS_DIR, "backgrounds/takeover_console.png", NO_MOD);
	}

	display_image_on_screen(&bg, GameConfig.screen_width / 2 - 340, GameConfig.screen_height / 2 - 294, IMAGE_NO_TRANSFO);

	if (target) {
		struct droidspec *droid_spec = &Droidmap[target->type];
		int offsetx = -droid_spec->droid_images[0][0].offset_x;
		int offsety = -droid_spec->droid_images[0][0].offset_y;
		Set_Rect(Target_Rect, xoffs + DroidStart[!YourColor].x + 20 + offsetx, (yoffs - 80) + offsety, User_Rect.w, User_Rect.h);
		PutIndividuallyShapedDroidBody(target, Target_Rect, FALSE, FALSE);
	}
	//  SDL_SetColorKey (Screen, 0, 0);
	SDL_SetClipRect(Screen, &User_Rect);

	struct tux_motion_class_images *current_motion_class_images = &tux_images[get_motion_class_id()];
	int phase = tux_anim.standing_keyframe;
	struct image *tux_feet_image = &current_motion_class_images->part_images[PART_GROUP_FEET][phase][0];
	struct image *tux_head_image = &current_motion_class_images->part_images[PART_GROUP_HEAD][phase][0];
	int tux_height = (tux_feet_image->offset_y + tux_feet_image->h) - (tux_head_image->offset_y);

	blit_tux(xoffs + DroidStart[YourColor].x + 20, (yoffs - 80) + (tux_height / 2) + tux_feet_image->offset_y);

	Set_Rect(Target_Rect, xoffs + LEFT_OFFS_X, yoffs + LEFT_OFFS_Y, User_Rect.w, User_Rect.h);

	display_image_on_screen (&ToGroundBlocks[YELLOW_HIGH], Target_Rect.x, Target_Rect.y, IMAGE_NO_TRANSFO);

	Target_Rect.y += GROUNDBLOCKHEIGHT;

	for (i = 0; i < 12; i++) {
		display_image_on_screen (&ToGroundBlocks[YELLOW_MIDDLE], Target_Rect.x, Target_Rect.y, IMAGE_NO_TRANSFO);
		Target_Rect.y += GROUNDBLOCKHEIGHT;
	}

	display_image_on_screen (&ToGroundBlocks[YELLOW_LOW], Target_Rect.x, Target_Rect.y, IMAGE_NO_TRANSFO);

	// the middle column
	Set_Rect(Target_Rect, xoffs + MID_OFFS_X, yoffs + MID_OFFS_Y, 0, 0);

	display_image_on_screen (&ToLeaderBlock, Target_Rect.x, Target_Rect.y, IMAGE_NO_TRANSFO);

	Target_Rect.y += LEADERBLOCKHEIGHT;
	for (i = 0; i < 12; i++, Target_Rect.y += COLUMNBLOCKHEIGHT) {
		display_image_on_screen (&ToColumnBlock, Target_Rect.x, Target_Rect.y, IMAGE_NO_TRANSFO);
	}

	// the right column
	Set_Rect(Target_Rect, xoffs + RIGHT_OFFS_X, yoffs + RIGHT_OFFS_Y, 0, 0);
	display_image_on_screen (&ToGroundBlocks[PURPLE_HIGH], Target_Rect.x, Target_Rect.y, IMAGE_NO_TRANSFO);

	Target_Rect.y += GROUNDBLOCKHEIGHT;

	for (i = 0; i < 12; i++, Target_Rect.y += GROUNDBLOCKHEIGHT) {
		display_image_on_screen (&ToGroundBlocks[PURPLE_MIDDLE], Target_Rect.x, Target_Rect.y, IMAGE_NO_TRANSFO);
	}

	display_image_on_screen (&ToGroundBlocks[PURPLE_LOW], Target_Rect.x, Target_Rect.y, IMAGE_NO_TRANSFO);

	// Fill the leader-LED with its color 
	Set_Rect(Target_Rect, xoffs + LEADERLED_X, yoffs + LEADERLED_Y, 0, 0);
	display_image_on_screen (&FillBlocks[LeaderColor], Target_Rect.x, Target_Rect.y, IMAGE_NO_TRANSFO);

	Target_Rect.y += FILL_BLOCK_HEIGHT;
	display_image_on_screen (&FillBlocks[LeaderColor], Target_Rect.x, Target_Rect.y, IMAGE_NO_TRANSFO);

	// Fill the display column with its colors 
	for (i = 0; i < NUM_LINES; i++) {
		Set_Rect(Target_Rect, xoffs + LEDCOLUMN_X, yoffs + LEDCOLUMN_Y + i * (FILL_BLOCK_HEIGHT + 2), 0, 0);
		display_image_on_screen (&FillBlocks[DisplayColumn[i]], Target_Rect.x, Target_Rect.y, IMAGE_NO_TRANSFO);
	}

	// Show the yellow playground 
	for (i = 0; i < NUM_LAYERS - 1; i++)
		for (j = 0; j < NUM_LINES; j++) {
			Set_Rect(Target_Rect, xoffs + PlaygroundStart[YELLOW].x + i * TO_BLOCKLEN,
				 yoffs + PlaygroundStart[YELLOW].y + j * TO_BLOCKHEIGHT, 0, 0);
			block = ToPlayground[YELLOW][i][j] + ActivationMap[YELLOW][i][j] * TO_BLOCKS;
			display_image_on_screen (&ToGameBlocks[block], Target_Rect.x, Target_Rect.y, IMAGE_NO_TRANSFO);
		}

	// Show the purple playground 
	for (i = 0; i < NUM_LAYERS - 1; i++)
		for (j = 0; j < NUM_LINES; j++) {
			Set_Rect(Target_Rect,
				 xoffs + PlaygroundStart[PURPLE].x + (NUM_LAYERS - i - 2) * TO_BLOCKLEN,
				 yoffs + PlaygroundStart[PURPLE].y + j * TO_BLOCKHEIGHT, 0, 0);
			block = ToPlayground[PURPLE][i][j] + (NUM_PHASES + ActivationMap[PURPLE][i][j]) * TO_BLOCKS;
			display_image_on_screen (&ToGameBlocks[block], Target_Rect.x, Target_Rect.y, IMAGE_NO_TRANSFO);
		}

	// Show the capsules left for each player 
	for (player = 0; player < 2; player++) {
		if (player == YOU)
			color = YourColor;
		else
			color = OpponentColor;

		Set_Rect(Target_Rect, xoffs + CurCapsuleStart[color].x,
			 yoffs + CurCapsuleStart[color].y + CapsuleCurRow[color] * (CAPSULE_HEIGHT + 2), 0, 0);
		if (NumCapsules[player]) {
			display_image_on_screen (&CapsuleBlocks[color], Target_Rect.x, Target_Rect.y, IMAGE_NO_TRANSFO);
		}

		for (i = 0; i < NumCapsules[player] - 1; i++) {
			Set_Rect(Target_Rect, xoffs + LeftCapsulesStart[color].x,
				 yoffs + LeftCapsulesStart[color].y + i * CAPSULE_HEIGHT, 0, 0);
			display_image_on_screen (&CapsuleBlocks[color], Target_Rect.x, Target_Rect.y, IMAGE_NO_TRANSFO);
		}		// for capsules 
	}			// for player 

	return;

};				// ShowPlayground 

/*-----------------------------------------------------------------
 * @Desc: Clears Playground (and ActivationMap) to default start-values
 * @Ret:  void
 *
 *-----------------------------------------------------------------*/
void ClearPlayground(void)
{
	int color, layer, row;

	for (color = YELLOW; color < TO_COLORS; color++)
		for (layer = 0; layer < NUM_LAYERS; layer++)
			for (row = 0; row < NUM_LINES; row++) {
				ActivationMap[color][layer][row] = INACTIVE;
				if (layer < TO_COLORS - 1)
					ToPlayground[color][layer][row] = CABLE;
				else
					ToPlayground[color][layer][row] = INACTIVE;
			}

	for (row = 0; row < NUM_LINES; row++)
		DisplayColumn[row] = row % 2;

};				// void ClearPlayground ( void )

/* -----------------------------------------------------------------
 * This function generates a random playground for the takeover game
 * ----------------------------------------------------------------- */
void InventPlayground(void)
{
	int anElement;
	int newElement;
	int row, layer;
	int color = YELLOW;

	// first clear the playground: we depend on this !! 
	//
	ClearPlayground();

	for (color = YELLOW; color < TO_COLORS; color++) {
		for (layer = 1; layer < NUM_LAYERS - 1; layer++) {
			for (row = 0; row < NUM_LINES; row++) {
				if (ToPlayground[color][layer][row] != CABLE)
					continue;

				newElement = MyRandom(TO_ELEMENTS - 1);
				if (MyRandom(MAX_PROB) > ElementProb[newElement]) {
					row--;
					continue;
				}

				switch (newElement) {
				case EL_CABLE:	/* has not to be set any more */
					anElement = ToPlayground[color][layer - 1][row];
					if (BlockClass[anElement] == NON_CONNECTOR)
						ToPlayground[color][layer][row] = EMPTY;
					break;

				case EL_CABLE_END:
					anElement = ToPlayground[color][layer - 1][row];
					if (BlockClass[anElement] == NON_CONNECTOR)
						ToPlayground[color][layer][row] = EMPTY;
					else
						ToPlayground[color][layer][row] = CABLE_END;
					break;

				case EL_AMPLIFIER:
					anElement = ToPlayground[color][layer - 1][row];
					if (BlockClass[anElement] == NON_CONNECTOR)
						ToPlayground[color][layer][row] = EMPTY;
					else
						ToPlayground[color][layer][row] = AMPLIFIER;
					break;

				case EL_COLOR_EXCHANGER:
					if (layer != 2) {	/* only existing on layer 2 */
						row--;
						continue;
					}

					anElement = ToPlayground[color][layer - 1][row];
					if (BlockClass[anElement] == NON_CONNECTOR)
						ToPlayground[color][layer][row] = EMPTY;
					else
						ToPlayground[color][layer][row] = COLOR_EXCHANGER;
					break;

				case EL_SEPARATOR:
					if (row > NUM_LINES - 3) {
						/* try again */
						row--;
						break;
					}

					anElement = ToPlayground[color][layer - 1][row + 1];
					if (BlockClass[anElement] == NON_CONNECTOR) {
						/* try again */
						row--;
						break;
					}

					/* don't destroy branch in prev. layer */
					anElement = ToPlayground[color][layer - 1][row];
					if (anElement == SEPARATOR_H || anElement == SEPARATOR_L) {
						row--;
						break;
					}
					anElement = ToPlayground[color][layer - 1][row + 2];
					if (anElement == SEPARATOR_H || anElement == SEPARATOR_L) {
						row--;
						break;
					}

					/* cut off cables in last layer, if any */
					anElement = ToPlayground[color][layer - 1][row];
					if (BlockClass[anElement] == CONNECTOR)
						ToPlayground[color][layer - 1][row] = CABLE_END;

					anElement = ToPlayground[color][layer - 1][row + 2];
					if (BlockClass[anElement] == CONNECTOR)
						ToPlayground[color][layer - 1][row + 2] = CABLE_END;

					/* set the branch itself */
					ToPlayground[color][layer][row] = SEPARATOR_H;
					ToPlayground[color][layer][row + 1] = SEPARATOR_M;
					ToPlayground[color][layer][row + 2] = SEPARATOR_L;

					row += 2;
					break;

				case EL_GATE:
					if (row > NUM_LINES - 3) {
						/* try again */
						row--;
						break;
					}

					anElement = ToPlayground[color][layer - 1][row];
					if (BlockClass[anElement] == NON_CONNECTOR) {
						/* try again */
						row--;
						break;
					}
					anElement = ToPlayground[color][layer - 1][row + 2];
					if (BlockClass[anElement] == NON_CONNECTOR) {
						/* try again */
						row--;
						break;
					}

					/* cut off cables in last layer, if any */
					anElement = ToPlayground[color][layer - 1][row + 1];
					if (BlockClass[anElement] == CONNECTOR)
						ToPlayground[color][layer - 1][row + 1] = CABLE_END;

					/* set the GATE itself */
					ToPlayground[color][layer][row] = GATE_H;
					ToPlayground[color][layer][row + 1] = GATE_M;
					ToPlayground[color][layer][row + 2] = GATE_L;

					row += 2;
					break;

				default:
					row--;
					break;

				}	/* switch NewElement */

			}	/* for row */

		}		/* for layer */

	}			/* for color */

}				/* InventPlayground */

/* -----------------------------------------------------------------
 * This function generates a random playground for the takeover game
 * ----------------------------------------------------------------- */
void EvaluatePlayground(void)
{
	int newElement;
	int row, layer;
	int color = YELLOW;
	float ScoreFound[TO_COLORS];

#define EVALUATE_PLAYGROUND_DEBUG 1

	for (color = YELLOW; color < TO_COLORS; color++) {

		DebugPrintf(EVALUATE_PLAYGROUND_DEBUG, "\n----------------------------------------------------------------------\n\
Starting to evaluate side nr. %d.  Results displayed below:\n", color);
		ScoreFound[color] = 0;

		for (layer = 1; layer < NUM_LAYERS - 1; layer++) {
			for (row = 0; row < NUM_LINES; row++) {

				// we examine this particular spot
				newElement = ToPlayground[color][layer][row];

				switch (newElement) {
				case CABLE:	/* has not to be set any more */
				case EMPTY:
					break;

				case CABLE_END:
					DebugPrintf(EVALUATE_PLAYGROUND_DEBUG, "CABLE_END found --> score -= 1.0\n");
					ScoreFound[color] -= 1.0;
					break;

				case AMPLIFIER:
					DebugPrintf(EVALUATE_PLAYGROUND_DEBUG, "AMPLIFIER found --> score += 0.5\n");
					ScoreFound[color] += 0.5;
					break;

				case COLOR_EXCHANGER:
					DebugPrintf(EVALUATE_PLAYGROUND_DEBUG, "COLOR_EXCHANGER found --> score -= 1.5\n");
					ScoreFound[color] -= 1.5;
					break;

				case SEPARATOR_H:
				case SEPARATOR_L:
				case SEPARATOR_M:
					DebugPrintf(EVALUATE_PLAYGROUND_DEBUG, "SEPARATOR found --> score += 1.0\n");
					ScoreFound[color] += 1.0;
					break;

				case GATE_M:
				case GATE_L:
				case GATE_H:
					DebugPrintf(EVALUATE_PLAYGROUND_DEBUG, "GATE found --> score -= 1.0\n");
					ScoreFound[color] -= 1.0;
					break;

				default:
					DebugPrintf(EVALUATE_PLAYGROUND_DEBUG, "UNHANDLED TILE FOUND!!\n");
					break;

				}	/* switch NewElement */

			}	/* for row */

		}		/* for layer */

		DebugPrintf(EVALUATE_PLAYGROUND_DEBUG, "\nResult for this side:  %f.\n", ScoreFound[color]);

	}			/* for color */

	DebugPrintf(EVALUATE_PLAYGROUND_DEBUG, "\n----------------------------------------------------------------------\n");

};				// EvaluatePlayground 

/* -----------------------------------------------------------------
 * This function Evaluates the AI side of the board for the takeover game
 * ----------------------------------------------------------------- */
float EvaluatePosition(const int color, const int row, const int layer, const int endgame)
{
        int player = YOU;
        if (color != YourColor)
                player = ENEMY;

        int opp_color = YELLOW;
        if (opp_color == color)
                opp_color = PURPLE;
        
	int newElement;
	// float ScoreFound [ TO_COLORS ];

#define EVAL_DEBUG 1
        
	DebugPrintf(EVAL_DEBUG, "\nEvaluatePlaygound ( %d , %d , %d ) called: ", color, row, layer);

	if (layer == NUM_LAYERS - 1) {
		DebugPrintf(EVAL_DEBUG, "End layer reached...");
                if (IsActive(color, row)) {
                        DebugPrintf(EVAL_DEBUG, "same color already active... returning 0.01 ");
                        return (0.01*EvaluateCenterPosition(opp_color, row, layer, endgame));
		} else if (DisplayColumn[row] == color) {
			DebugPrintf(EVAL_DEBUG, "same color... returning 0.05 ");
			return (0.05*EvaluateCenterPosition(opp_color, row, layer, endgame));
		} else if (IsActive(opp_color, row)) {
			DebugPrintf(EVAL_DEBUG, "different color, but active... returning 9 ");
                        if (endgame)
                                return (90*EvaluateCenterPosition(opp_color, row, layer, endgame));
                        else
                                return (8*EvaluateCenterPosition(opp_color, row, layer, endgame));
		} else {
			DebugPrintf(EVAL_DEBUG, "different color... returning 10 ");
                        if (endgame)
                                return (100*EvaluateCenterPosition(opp_color, row, layer, endgame));
                        else
                                return (10*EvaluateCenterPosition(opp_color, row, layer, endgame));
		}
	}

	if (ActivationMap[color][layer][row] == ACTIVE1) { return (0); }

	newElement = ToPlayground[color][layer][row];

	switch (newElement) {
	case CABLE:		/* has not to be set any more */
		DebugPrintf(EVAL_DEBUG, "CABLE reached... continuing...");
		return (EvaluatePosition(color, row, layer + 1, endgame));

	case AMPLIFIER:
		DebugPrintf(EVAL_DEBUG, "AMPLIFIER reached... continuing...");
                return ((1 + (0.2 * NumCapsules[player])) * EvaluatePosition(color, row, layer + 1, endgame));
		break;

	case COLOR_EXCHANGER:
		DebugPrintf(EVAL_DEBUG, "COLOR_EXCHANGER reached... continuing...");
                return (-1.5 * EvaluatePosition(color, row, layer + 1, endgame));
		break;

	case SEPARATOR_M:
		DebugPrintf(EVAL_DEBUG, "SEPARATOR reached... double-continuing...");
		return (EvaluatePosition(color, row + 1, layer + 1, endgame) + EvaluatePosition(color, row - 1, layer + 1, endgame));
		break;

	case GATE_H:
		DebugPrintf(EVAL_DEBUG, "GATE reached... stopping...\n");
                if (ActivationMap[color][layer][row + 2] >= ACTIVE1) {
                        return (5 * EvaluatePosition(color, row + 1, layer + 1, endgame));
                } else if (endgame) {
                        return (0.5 * EvaluatePosition(color, row + 1, layer + 1, endgame));
                } else {
                        return (0.3 * EvaluatePosition(color, row + 1, layer + 1, endgame));
                }
		break;

	case GATE_L:
		DebugPrintf(EVAL_DEBUG, "GATE reached... stopping...\n");
                if (ActivationMap[color][layer][row - 2] >= ACTIVE1) {
                        return (5 * EvaluatePosition(color, row - 1, layer + 1, endgame));
                } else if (endgame) {
                        return (0.5 * EvaluatePosition(color, row - 1, layer + 1, endgame));
                } else {
                        return (0.3 * EvaluatePosition(color, row - 1, layer + 1, endgame));
                }
		break;
        //treat the following cases the same:
	case EMPTY:
	case CABLE_END:
	case SEPARATOR_H:
	case SEPARATOR_L:
	case GATE_M:
		return (0);
		break;
	default:
		DebugPrintf(EVAL_DEBUG, "\nUNHANDLED TILE reached\n");
		break;

	}
	return (0);

};

/* -----------------------------------------------------------------
 * This function Evaluates the Player side of the board from AI perspective
 * ----------------------------------------------------------------- */
float EvaluateCenterPosition(const int color, const int row, const int layer, const int endgame)
{
        int player = YOU;  //should match the color owner (and be opposite the AI)
        if (color != YourColor)
                player = ENEMY;

        int newElement;
                
        if (layer == 1) {
                return (1); //hit the player's end
        }

	newElement = ToPlayground[color][layer][row];

	switch (newElement) {
	case CABLE:		// has not to be set any more 
		return (EvaluateCenterPosition(color, row, layer - 1, endgame));
	case AMPLIFIER:
                if (ActivationMap[color][layer+1][row] >= ACTIVE1) { //very low hope to take this over
                        return (0.06);
                } else if (endgame) {
                        return (1);
                } else { 
                        return ((1 - (0.1 * NumCapsules[player])) * EvaluateCenterPosition(color, row, layer - 1, endgame));
		}
                break;
        case SEPARATOR_H: //correctly evaluating this may lead to an infinite loop, using 0.5
                if ((NumCapsules[player]>0) && (!endgame))
                        return ((0.53 - (0.02 * NumCapsules[player])) * EvaluateCenterPosition(color, row - 1, layer - 1, endgame) );
                else
                        return (EvaluateCenterPosition(color, row - 1, layer - 1, endgame));
		break;
	case SEPARATOR_L:
                if ((NumCapsules[player]>0) && (!endgame))
                        return ((0.53 - (0.02 * NumCapsules[player])) * EvaluateCenterPosition(color, row + 1, layer - 1, endgame) );
                else
                        return (EvaluateCenterPosition(color, row + 1, layer - 1, endgame));
		break;

	case GATE_M:
                if (!endgame)
                        return ((0.5 + (0.09 * NumCapsules[player])) * (EvaluateCenterPosition(color, row + 1, layer - 1, endgame) + EvaluateCenterPosition(color, row - 1, layer - 1, endgame)));
                else
                        return (1);
		break;

        //Following cases act the same:
        case COLOR_EXCHANGER:
	case EMPTY:
	case CABLE_END:
	case SEPARATOR_M:
       	case GATE_H:
	case GATE_L:
                if (!endgame)
                        return (1 + (0.2 * NumCapsules[player]));
                else
                        return (1);
                break;
	default:
		DebugPrintf(EVAL_DEBUG, "\nUNHANDLED TILE reached\n");
		break;
	}

	return (0);

};

/*-----------------------------------------------------------------
 * @Desc: process the playground following its intrinsic logic
 *
 * @Ret: void
 *
 *-----------------------------------------------------------------*/
void ProcessPlayground(void)
{
	int color, layer, row;
	int TurnActive = FALSE;

	for (color = YELLOW; color < TO_COLORS; color++) {
		for (layer = 1; layer < NUM_LAYERS; layer++) {
			for (row = 0; row < NUM_LINES; row++) {
				if (layer == NUM_LAYERS - 1) {
					if (IsActive(color, row))
						ActivationMap[color][layer][row] = ACTIVE1;
					else
						ActivationMap[color][layer][row] = INACTIVE;

					continue;
				}
				/* if last layer */
				TurnActive = FALSE;

				switch (ToPlayground[color][layer][row]) {
				case COLOR_EXCHANGER:
				case SEPARATOR_M:
				case GATE_H:
				case GATE_L:
				case CABLE:
					if (ActivationMap[color][layer - 1][row] >= ACTIVE1)
						TurnActive = TRUE;
					break;

				case AMPLIFIER:
					if (ActivationMap[color][layer - 1][row] >= ACTIVE1)
						TurnActive = TRUE;

					// additional enforcers stay active by themselves...
					if (ActivationMap[color][layer][row] >= ACTIVE1)
						TurnActive = TRUE;

					break;

				case CABLE_END:
					break;

				case SEPARATOR_H:
					if (ActivationMap[color][layer][row + 1] >= ACTIVE1)
						TurnActive = TRUE;
					break;

				case SEPARATOR_L:
					if (ActivationMap[color][layer][row - 1] >= ACTIVE1)
						TurnActive = TRUE;
					break;

				case GATE_M:
					if ((ActivationMap[color][layer][row - 1] >= ACTIVE1)
					    && (ActivationMap[color][layer][row + 1] >= ACTIVE1))
						TurnActive = TRUE;

					break;

				default:
					break;
				}	/* switch */

				if (TurnActive) {
					if (ActivationMap[color][layer][row] == INACTIVE)
						ActivationMap[color][layer][row] = ACTIVE1;
				} else
					ActivationMap[color][layer][row] = INACTIVE;

			}	/* for row */

		}		/* for layer */

	}			/* for color */

	return;
};				// void ProcessPlayground ( void )

/** 
 * This function sets the correct values for the status column in the
 * middle of the takeover game field.
 * Blinking LEDs are realized here as well.
 */
void ProcessDisplayColumn(void)
{
	static int CLayer = 3;	/* the connection-layer to the Column */
	static int flicker_color = 0;
	int row;
	int YellowCounter, PurpleCounter;

	flicker_color = !flicker_color;

	for (row = 0; row < NUM_LINES; row++) {
		// unquestioned yellow
		if ((ActivationMap[YELLOW][CLayer][row] >= ACTIVE1) && (ActivationMap[PURPLE][CLayer][row] == INACTIVE)) {
			// change color?
			if (ToPlayground[YELLOW][CLayer - 1][row] == COLOR_EXCHANGER)
				DisplayColumn[row] = PURPLE;
			else
				DisplayColumn[row] = YELLOW;
			continue;
		}
		// clearly purple
		if ((ActivationMap[YELLOW][CLayer][row] == INACTIVE) && (ActivationMap[PURPLE][CLayer][row] >= ACTIVE1)) {
			// change color?
			if (ToPlayground[PURPLE][CLayer - 1][row] == COLOR_EXCHANGER)
				DisplayColumn[row] = YELLOW;
			else
				DisplayColumn[row] = PURPLE;

			continue;
		}
		// undecided: flimmering
		if ((ActivationMap[YELLOW][CLayer][row] >= ACTIVE1) && (ActivationMap[PURPLE][CLayer][row] >= ACTIVE1)) {
			// change color?
			if ((ToPlayground[YELLOW][CLayer - 1][row] == COLOR_EXCHANGER) &&
			    (ToPlayground[PURPLE][CLayer - 1][row] != COLOR_EXCHANGER))
				DisplayColumn[row] = PURPLE;
			else if ((ToPlayground[YELLOW][CLayer - 1][row] != COLOR_EXCHANGER) &&
				 (ToPlayground[PURPLE][CLayer - 1][row] == COLOR_EXCHANGER))
				DisplayColumn[row] = YELLOW;
			else {
				if (flicker_color == 0)
					DisplayColumn[row] = YELLOW;
				else
					DisplayColumn[row] = PURPLE;
			}	/* if - else if - else */

		}
		/* if undecided */
	}			/* for */

	// evaluate the winning color
	YellowCounter = 0;
	PurpleCounter = 0;
	for (row = 0; row < NUM_LINES; row++)
		if (DisplayColumn[row] == YELLOW)
			YellowCounter++;
		else
			PurpleCounter++;

	LeaderCapsuleCount = (PurpleCounter > YellowCounter) ? PurpleCounter : YellowCounter;
	if (PurpleCounter < YellowCounter)
		LeaderColor = YELLOW;
	else if (PurpleCounter > YellowCounter)
		LeaderColor = PURPLE;
	else
		LeaderColor = DRAW;

	return;
};				// void ProcessDisplayColumn 

/** 
 * This function does the countdown of the capsules and kills them if 
 * they are too old.
 */
void ProcessCapsules(void)
{
	int row;
	int color;

	for (color = YELLOW; color <= PURPLE; color++)
		for (row = 0; row < NUM_LINES; row++) {
			if (CapsuleCountdown[color][0][row] > 0)
				CapsuleCountdown[color][0][row]--;

			if (CapsuleCountdown[color][0][row] == 0) {
				CapsuleCountdown[color][0][row] = -1;
				ActivationMap[color][0][row] = INACTIVE;
				ToPlayground[color][0][row] = CABLE;
			}

		}		/* for row */

};				// void ProcessCapsules ( void )

/**
 * This function tells, whether a Column-connection is active or not.
 * It returns TRUE or FALSE accordingly.
 */
int IsActive(int color, int row)
{
	int CLayer = 3;		/* the connective Layer */
	int TestElement = ToPlayground[color][CLayer - 1][row];

	if ((ActivationMap[color][CLayer - 1][row] >= ACTIVE1) && (BlockClass[TestElement] == CONNECTOR))
		return TRUE;
	else
		return FALSE;
}				/* IsActive */

/* -----------------------------------------------------------------
 * This function animates the active cables: this is done by cycling 
 * over the active phases ACTIVE1-ACTIVE3, which are represented by 
 * different pictures in the playground
 * ----------------------------------------------------------------- */
void AnimateCurrents(void)
{
	int color, layer, row;

	for (color = YELLOW; color <= PURPLE; color++)
		for (layer = 0; layer < NUM_LAYERS; layer++)
			for (row = 0; row < NUM_LINES; row++)
				if (ActivationMap[color][layer][row] >= ACTIVE1) {
					ActivationMap[color][layer][row]++;
					if (ActivationMap[color][layer][row] == NUM_PHASES)
						ActivationMap[color][layer][row] = ACTIVE1;
				}

	return;
};				// void AnimateCurrents (void)

/**
 *
 *
 */
void to_show_banner(const char *left, const char *right)
{
	char left_box[LEFT_TEXT_LEN + 10];
	char right_box[RIGHT_TEXT_LEN + 10];
	int left_len, right_len;	// the actual string lengths

	// At first the text is prepared.  This can't hurt.
	// we will decide whether to display it or not later...
	//

	if (left == NULL)
		left = "0";

	if (right == NULL) {
		right = "";
	}
	// Now fill in the text
	left_len = strlen(left);
	if (left_len > LEFT_TEXT_LEN) {
		printf("\nWarning: String %s too long for Left Infoline!!", left);
		Terminate(EXIT_FAILURE);
	}
	right_len = strlen(right);
	if (right_len > RIGHT_TEXT_LEN) {
		printf("\nWarning: String %s too long for Right Infoline!!", right);
		Terminate(EXIT_FAILURE);
	}
	// Now prepare the left/right text-boxes 
	memset(left_box, ' ', LEFT_TEXT_LEN);	// pad with spaces 
	memset(right_box, ' ', RIGHT_TEXT_LEN);

	strncpy(left_box, left, left_len);	// this drops terminating \0 ! 
	strncpy(right_box, right, right_len);	// this drops terminating \0 ! 

	left_box[LEFT_TEXT_LEN] = '\0';	// that's right, we want padding!
	right_box[RIGHT_TEXT_LEN] = '\0';

	// Now the text should be ready and its
	// time to display it...
	DebugPrintf(2, "Takeover said: %s -- %s\n", left_box, right_box);
	set_current_font(Para_Font);
	display_text(left_box, LEFT_INFO_X, LEFT_INFO_Y, NULL, 1.0);
	display_text(right_box, RIGHT_INFO_X, RIGHT_INFO_Y, NULL, 1.0);

};				// void to_show_banner (const char* left, const char* right)

#undef _takeover_c
