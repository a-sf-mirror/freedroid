/* 
 *
 *   Copyright (c) 2004-2007 Arthur Huillet 
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

#define _takeover_c

#include "system.h"

#include "defs.h"
#include "struct.h"
#include "global.h"
#include "proto.h"
#include "takeover.h"
#include "map.h"

enemy *cDroid;
Uint32 cur_time;		// current time in ms 
SDL_Surface *to_blocks;		// the global surface containing all game-blocks 

//--------------------
// The rectangles containing the blocks for the takeover game
//
iso_image FillBlocks[NUM_FILL_BLOCKS];
SDL_Rect FillRects[NUM_FILL_BLOCKS];
iso_image CapsuleBlocks[NUM_CAPS_BLOCKS];
SDL_Rect CapsuleRects[NUM_CAPS_BLOCKS];
iso_image ToGameBlocks[NUM_TO_BLOCKS];
SDL_Rect ToGameRects[NUM_TO_BLOCKS];
iso_image ToGroundBlocks[NUM_GROUND_BLOCKS];
SDL_Rect ToGroundRects[NUM_GROUND_BLOCKS];
iso_image ToColumnBlock;
SDL_Rect ToColumnRect;
iso_image ToLeaderBlock;
SDL_Rect ToLeaderRect;

//--------------------
// Class seperation of the blocks 
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
	100,			// EL_KABEL 
	2,			// EL_KABELENDE 
	5,			// EL_VERSTAERKER 
	5,			// EL_FARBTAUSCHER: only on last layer 
	5,			// EL_VERZWEIGUNG 
	5			// EL_GATTER 
};

int NumCapsules[TO_COLORS] = {
	0, 0
};

point LeftCapsulesStart[TO_COLORS] = {
	{GELB_LEFT_CAPSULES_X, GELB_LEFT_CAPSULES_Y},
	{VIOLETT_LEFT_CAPSULES_X, VIOLETT_LEFT_CAPSULES_Y}
};

point CurCapsuleStart[TO_COLORS] = {
	{GELB_CUR_CAPSULE_X, GELB_CUR_CAPSULE_Y},
	{VIOLETT_CUR_CAPSULE_X, VIOLETT_CUR_CAPSULE_Y}
};

point PlaygroundStart[TO_COLORS] = {
	{GELB_PLAYGROUND_X, GELB_PLAYGROUND_Y},
	{VIOLETT_PLAYGROUND_X, VIOLETT_PLAYGROUND_Y}
};

point DruidStart[TO_COLORS] = {
	{GELB_DRUID_X, GELB_DRUID_Y},
	{VIOLETT_DRUID_X, VIOLETT_DRUID_Y}
};

int CapsuleCurRow[TO_COLORS] = { 0, 0 };

int LeaderColor = GELB;		/* momentary leading color */
int YourColor = GELB;
int OpponentColor = VIOLETT;
int OpponentType;		/* The druid-type of your opponent */
enemy *cdroid;

/* the display  column */
int DisplayColumn[NUM_LINES] = {
	GELB, VIOLETT, GELB, VIOLETT, GELB, VIOLETT, GELB, VIOLETT, GELB, VIOLETT,
	GELB, VIOLETT
};

SDL_Color to_bg_color = { 199, 199, 199 };

playground_t ToPlayground;
playground_t ActivationMap;
playground_t CapsuleCountdown;

void EvaluatePlayground(void);
float EvaluatePosition(int color, int row, int layer);
void AdvancedEnemyTakeoverMovements(void);

static void ShowPlayground(void);

static void display_takeover_help()
{
	PlayATitleFile("TakeoverInstructions.title");
}

/** 
 * Display the picture of a droid
 */
static void ShowDroidPicture(int PosX, int PosY, int Number)
{
	SDL_Surface *tmp;
	SDL_Rect target;
	char ConstructedFileName[5000];
	char fpath[2048];
	static char LastImageSeriesPrefix[1000] = "NONE_AT_ALL";
#define NUMBER_OF_IMAGES_IN_DROID_PORTRAIT_ROTATION 32
	static SDL_Surface *DroidRotationSurfaces[NUMBER_OF_IMAGES_IN_DROID_PORTRAIT_ROTATION] = { NULL };
	SDL_Surface *Whole_Image;
	int i;
	int RotationIndex;

	DebugPrintf(2, "\nvoid ShowDroidPicture(...): Function call confirmed.");

	if (!strcmp(Druidmap[Number].droid_portrait_rotation_series_prefix, "NONE_AVAILABLE_YET"))
		return;		// later this should be a default-correction instead

	//--------------------
	// Maybe we have to reload the whole image series
	//
	if (strcmp(LastImageSeriesPrefix, Druidmap[Number].droid_portrait_rotation_series_prefix)) {
		//--------------------
		// Maybe we have to free the series from an old item display first
		//
		if (DroidRotationSurfaces[0] != NULL) {
			for (i = 1; i < NUMBER_OF_IMAGES_IN_DROID_PORTRAIT_ROTATION; i++) {
				SDL_FreeSurface(DroidRotationSurfaces[i]);
			}
		}
		//--------------------
		// Now we can start to load the whole series into memory
		//
		for (i = 0; i < NUMBER_OF_IMAGES_IN_DROID_PORTRAIT_ROTATION; i++) {
			if (!strcmp(Druidmap[Number].droid_portrait_rotation_series_prefix, "NONE_AVAILABLE_YET")) {
				Terminate(ERR);
			} else {
				sprintf(ConstructedFileName, "droids/%s/portrait_%04d.jpg",
					Druidmap[Number].droid_portrait_rotation_series_prefix, i + 1);
				DebugPrintf(1, "\nConstructedFileName = %s ", ConstructedFileName);
			}

			// We must remember, that his is already loaded of course
			strcpy(LastImageSeriesPrefix, Druidmap[Number].droid_portrait_rotation_series_prefix);

			find_file(ConstructedFileName, GRAPHICS_DIR, fpath, 0);

			Whole_Image = our_IMG_load_wrapper(fpath);	// This is a surface with alpha channel, since the picture is one of this type
			if (Whole_Image == NULL) {
				fprintf(stderr, "\n\nfpath: %s. \n", fpath);
				ErrorMessage(__FUNCTION__, "\
Freedroid was unable to load an image of a rotated droid into memory.\n\
This error indicates some installation problem with freedroid.", PLEASE_INFORM, IS_FATAL);
			}

			SDL_SetAlpha(Whole_Image, 0, SDL_ALPHA_OPAQUE);

			DroidRotationSurfaces[i] = our_SDL_display_format_wrapperAlpha(Whole_Image);	// now we have an alpha-surf of right size
			SDL_SetColorKey(DroidRotationSurfaces[i], 0, 0);	// this should clear any color key in the dest surface

			SDL_FreeSurface(Whole_Image);
		}
	}

	RotationIndex = (SDL_GetTicks() / 50);

	RotationIndex =
	    RotationIndex - (RotationIndex / NUMBER_OF_IMAGES_IN_DROID_PORTRAIT_ROTATION) * NUMBER_OF_IMAGES_IN_DROID_PORTRAIT_ROTATION;

	tmp = DroidRotationSurfaces[RotationIndex];

	SDL_SetClipRect(Screen, NULL);
	Set_Rect(target, PosX, PosY, GameConfig.screen_width, GameConfig.screen_height);
	our_SDL_blit_surface_wrapper(tmp, NULL, Screen, &target);

	DebugPrintf(2, "\nvoid ShowDroidPicture(...): Usual end of function reached.");

};				// void ShowDroidPicture ( ... )

/* ------------------------------------------------------------
 * display infopage page of droidtype
 * does update the screen, no our_SDL_flip_wrapper() necesary !
 * ------------------------------------------------------------ */
static void ShowDroidInfo(int droidtype, int Displacement, char ShowArrows)
{
	char *item_name;
	int type;
	char InfoText[10000];
	char TextChunk[2000];

	//--------------------
	// We initialize the text rectangle
	//
	Cons_Text_Rect.x = 258 * GameConfig.screen_width / 640;
	Cons_Text_Rect.y = 89 * GameConfig.screen_height / 480;
	Cons_Text_Rect.w = 346 * GameConfig.screen_width / 640;
	Cons_Text_Rect.h = 282 * GameConfig.screen_height / 480;

	SDL_SetClipRect(Screen, NULL);

	blit_special_background(ITEM_BROWSER_BG_PIC_BACKGROUND_CODE);

	ShowDroidPicture(45 * GameConfig.screen_width / 640, 190 * GameConfig.screen_height / 480, droidtype);

	//--------------------
	// We fill out the header area of the items browser.
	//
	SetCurrentFont(Menu_BFont);
	strcpy(TextChunk, Druidmap[droidtype].druidname);
	CutDownStringToMaximalSize(TextChunk, 225);
	PutString(Screen, 330 * GameConfig.screen_width / 640, 38 * GameConfig.screen_height / 480, TextChunk);

	sprintf(InfoText, _("\
Unit type %s\n\
Entry : %d\n"), Druidmap[droidtype].druidname, droidtype + 1);

	if ((type = Druidmap[droidtype].weapon_item.type) >= 0)	// make sure item=-1 
		item_name = D_(ItemMap[type].item_name);	// does not segfault 
	else
		item_name = _("none");

	sprintf(TextChunk, _("\nArmament : %s\n"), item_name);
	strcat(InfoText, TextChunk);
	if (Me.TakeoverSuccesses[droidtype]+Me.TakeoverFailures[droidtype]) {
		sprintf(TextChunk, _("\nTakeover Success : %2d%%\n"),
        		((100*Me.TakeoverSuccesses[droidtype])/ (Me.TakeoverSuccesses[droidtype]+Me.TakeoverFailures[droidtype])) );
		strcat(InfoText, TextChunk);
	}

	sprintf(TextChunk, _("\nNotes: %s\n"), D_(Druidmap[droidtype].notes));
	strcat(InfoText, TextChunk);

	SetCurrentFont(FPS_Display_BFont);
	DisplayText(InfoText, Cons_Text_Rect.x, Cons_Text_Rect.y + Displacement, &Cons_Text_Rect, TEXT_STRETCH);

	if (ShowArrows) {
		ShowGenericButtonFromList(UP_BUTTON);
		ShowGenericButtonFromList(DOWN_BUTTON);
	}

};				// void ShowDroidInfo ( ... )

/**
 * This function does the countdown where you still can changes your
 * color.
 */
static void ChooseColor(void)
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
				Terminate(0);
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
					YourColor = VIOLETT;
					OpponentColor = GELB;
					break;
				case SDLK_LEFT:
					YourColor = GELB;
					OpponentColor = VIOLETT;
					break;
				case SDLK_SPACE:
					ColorChosen = TRUE;
					break;
				case SDLK_F1:
					display_takeover_help();
					break;
				default:
					break;
				}
			}

		}

		countdown--;	// Count down 
		sprintf(count_text, _("Color-%d"), countdown);

		ShowPlayground();
		to_show_banner(count_text, NULL);
		our_SDL_flip_wrapper();

		if (countdown == 0)
			ColorChosen = TRUE;

	}			// while(!ColorChosen) 

	while (MouseLeftPressed())
		SDL_Delay(1);
};				// void ChooseColor ( void ) 

static void PlayGame(int countdown)
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

	sprintf(count_text, _("Subliminal"));	/* Make sure a value gets assigned to count_text */
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
				/* allow for a WIN-key that give immedate victory */
				event.key.keysym.mod &= ~(KMOD_CAPS | KMOD_NUM | KMOD_MODE);	/* We want to ignore "global" modifiers. */
				if (event.key.keysym.sym == SDLK_w && (event.key.keysym.mod == (KMOD_LCTRL | KMOD_LALT))) {
					LeaderColor = YourColor;	/* simple as that */
					return;	/* leave now, to avoid changing of LeaderColor! */
				}
				if (event.key.keysym.sym == SDLK_F1)
					display_takeover_help();
			} else if (event.type == SDL_QUIT) {
				Terminate(0);
			}
		}

		if (!up)
			up_counter = 0;	/* reset counters for released keys */
		if (!down)
			down_counter = 0;

		if (cur_time > prev_count_tick + count_tick_len) {	/* time to count 1 down */
			prev_count_tick += count_tick_len;	/* set for next countdown tick */
			countdown--;
			sprintf(count_text, _("Finish-%d"), countdown);

			if (countdown == 0)
				FinishTakeover = TRUE;

			AnimateCurrents();	/* do some animation on the active cables */

		}

		/* if (countdown_tick has occurred) */
		/* time for movement */
		if (cur_time > prev_move_tick + move_tick_len) {
			prev_move_tick += move_tick_len;	/* set for next motion tick */
			AdvancedEnemyTakeoverMovements();

			if (up) {
				if (!up_counter || (up_counter > wait_move_ticks)) {
					//--------------------
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
					//--------------------
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
				    (ToPlayground[YourColor][0][row] != KABELENDE) && (ActivationMap[YourColor][0][row] == INACTIVE)) {
					NumCapsules[YOU]--;
					CapsuleCurRow[YourColor] = 0;
					ToPlayground[YourColor][0][row] = VERSTAERKER;
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
		ShowPlayground();
		to_show_banner(count_text, NULL);
		our_SDL_flip_wrapper();
		SDL_Delay(10);
	}			/* while !FinishTakeover */

	/* Final contdown */
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
		ShowPlayground();
		our_SDL_flip_wrapper();
	}			/* while (countdown) */

	return;

}

int do_takeover(int player_capsules, int opponent_capsules, int game_length)
{
	char *message;
	int player_won = 0;
	int FinishTakeover = FALSE;
	int row;
	int old_status;

	old_status = game_status;

	Activate_Conservative_Frame_Computation();

	//--------------------
	// Maybe takeover graphics haven't been loaded yet.  Then we do this
	// here now and for once.  Later calls will be ignored inside the function.
	//
	GetTakeoverGraphics();

	// eat pending events
	input_handle();

	while (!FinishTakeover) {
		//--------------------
		// Init Color-column and Capsule-Number for each opponenet and your color 
		//
		for (row = 0; row < NUM_LINES; row++) {
			DisplayColumn[row] = (row % 2);
			CapsuleCountdown[GELB][0][row] = -1;
			CapsuleCountdown[VIOLETT][0][row] = -1;
		}		// for row 

		YourColor = GELB;
		OpponentColor = VIOLETT;

		CapsuleCurRow[GELB] = 0;
		CapsuleCurRow[VIOLETT] = 0;

		NumCapsules[YOU] = player_capsules;
		NumCapsules[ENEMY] = opponent_capsules;
		InventPlayground();

		EvaluatePlayground();

		ShowPlayground();
		our_SDL_flip_wrapper();

		ChooseColor();

		//--------------------
		// This following function plays the takeover game, until one
		// of THREE states is reached, i.e. until YOU WON, YOU LOST
		// or until DEADLOCK is reached.  Well, so maybe after that
		// the takeover game is finished, but if it's a deadlock, then
		// the game must be played again in the next loop...
		//
		PlayGame(game_length);

		//--------------------
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

		ShowPlayground();
		to_show_banner(message, NULL);
		our_SDL_flip_wrapper();
		SDL_Delay(100);

	}

	game_status = old_status;
	return player_won;
}

/*-----------------------------------------------------------------
 *
 * This function manages the whole takeover game of Tux against 
 * some bot.
 *
 * The return value is TRUE/FALSE depending on whether the game was
 * finally won/lost.
 *
 *-----------------------------------------------------------------*/
int droid_takeover(enemy * target)
{
	int Finished = FALSE;
	int Displacement = 0;
	int reward = 0;
	SDL_Event event;

	//--------------------
	// Prevent distortion of framerate by the delay coming from 
	// the time spent in the menu.
	//
	Activate_Conservative_Frame_Computation();

	//--------------------
	// We set the UserRect to full again, no matter what other windows might
	// be open right now...
	//
	User_Rect.x = 0;
	User_Rect.y = 0;
	User_Rect.w = GameConfig.screen_width;
	User_Rect.h = GameConfig.screen_height;

	while (SpacePressed() || MouseLeftPressed()) ;	// make sure space is release before proceed 

	SwitchBackgroundMusicTo(TAKEOVER_BACKGROUND_MUSIC_SOUND);

	DisplayBanner();

	while (!Finished) {
		ShowDroidInfo(target->type, Displacement, TRUE);
		ShowGenericButtonFromList(TAKEOVER_HELP_BUTTON);
		blit_our_own_mouse_cursor();
		our_SDL_flip_wrapper();

		while (SDL_PollEvent(&event)) {

			if (event.type == SDL_QUIT) {
				Terminate(0);
			}

			if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT) {
				if (MouseCursorIsOnButton(UP_BUTTON, GetMousePos_x(), GetMousePos_y())) {
					MoveMenuPositionSound();
					Displacement += FontHeight(GetCurrentFont());
				} else if (MouseCursorIsOnButton(DOWN_BUTTON, GetMousePos_x(), GetMousePos_y())) {
					MoveMenuPositionSound();
					Displacement -= FontHeight(GetCurrentFont());
				} else if (MouseCursorIsOnButton(DRUID_SHOW_EXIT_BUTTON, GetMousePos_x(), GetMousePos_y())) {
					Finished = TRUE;
				} else if (MouseCursorIsOnButton(TAKEOVER_HELP_BUTTON, GetMousePos_x(), GetMousePos_y())) {
					PlayATitleFile("TakeoverInstructions.title");
				}
			} else if (event.type == SDL_KEYDOWN
				   && ((event.key.keysym.sym == SDLK_SPACE) || (event.key.keysym.sym == SDLK_ESCAPE))) {
				Finished = TRUE;
			}
		}
		SDL_Delay(1);
	}

	while (!(!SpacePressed() && !EscapePressed() && !MouseLeftPressed())) ;

	cDroid = target;
	int player_capsules = 2 + Me.base_skill_level[get_program_index_with_name("Hacking")];
	int opponent_capsules = 2 + Druidmap[target->type].class;

	if (do_takeover(player_capsules, opponent_capsules, 100)) {
		/* Won takeover */
		Me.marker = target->marker;

		reward = Druidmap[target->type].experience_reward * 1;
		Me.Experience += reward;
		append_new_game_message(_("For taking control of your enemy, you receive %d experience."), reward);

		//--------------------
		// Maybe the enemy in question was a kind of 'boss monster' or it had
		// some special item, that is relevant to a mission or quest.  In that
		// case (like also when the bot is finally destroyed, the quest item
		// should be dropped after the successful takeover process, even if the
		// enemy isn't completely dead yet...
		//
		if (target->on_death_drop_item_code != (-1)) {
			DropItemAt(target->on_death_drop_item_code, target->pos.z, target->pos.x, target->pos.y, -1, -1, 1);
			target->on_death_drop_item_code = -1;
		}

		target->is_friendly = TRUE;
		target->has_been_taken_over = TRUE;

		target->combat_state = WAYPOINTLESS_WANDERING;

		//--------------------
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

	cDroid = NULL;

	ClearGraphMem();

	SwitchBackgroundMusicTo(CURLEVEL()->Background_Song_Name);

	if (LeaderColor == YourColor)
		return TRUE;
	else
		return FALSE;

};				// int Takeover( int enemynum ) 

/*-----------------------------------------------------------------
 * This function performs the enemy movements in the takeover game,
 * but it does this in an advaned way, that has not been there in
 * the classic freedroid game.
 *-----------------------------------------------------------------*/
void AdvancedEnemyTakeoverMovements(void)
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

#define TAKEOVER_MOVEMENT_DEBUG 1

	if (NumCapsules[ENEMY] == 0)
		return;

	//--------------------
	// First we're going to find out which target place is
	// best choice for the next capsule setting.
	//
	for (test_row = 0; test_row < NUM_LINES; test_row++) {
		if (EvaluatePosition(OpponentColor, test_row, 1) > BestValue) {
			BestTarget = test_row;
			BestValue = EvaluatePosition(OpponentColor, test_row, 1);
		}
	}
	DebugPrintf(TAKEOVER_MOVEMENT_DEBUG, "\nBest target row found : %d.", BestTarget);

	//--------------------
	// Now we can start to move into the right direction.
	// Previously this was a pure random choice like
	//
	// action = MyRandom (Actions);
	//
	// but now we do it differently :)
	//
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

	case 2:		/* Try to set  capsule */
		if (MyRandom(100) <= SetProbability) {
			if ((row >= 0) &&
			    (ToPlayground[OpponentColor][0][row] != KABELENDE) && (ActivationMap[OpponentColor][0][row] == INACTIVE)) {
				NumCapsules[ENEMY]--;
				Takeover_Set_Capsule_Sound();
				ToPlayground[OpponentColor][0][row] = VERSTAERKER;
				ActivationMap[OpponentColor][0][row] = ACTIVE1;
				CapsuleCountdown[OpponentColor][0][row] = CAPSULE_COUNTDOWN;
				row = -1;	/* For the next capsule: startpos */
			}
		}
		/* if MyRandom */
		break;

	default:
		break;

	}			/* switch action */

	CapsuleCurRow[OpponentColor] = row + 1;

	return;
};				// AdvancedEnemyTakeoverMovements 

/**
 * This function reads in the takeover game elements for later blitting. 
 * It frees previous SDL-surfaces if they were allocated.  T
 * This allows to use this fct also for theme-switching.
 *-----------------------------------------------------------------*/
int GetTakeoverGraphics(void)
{
	static int TakeoverGraphicsAreAlreadyLoaded = FALSE;
	int i, j;
	int curx = 0, cury = 0;
	SDL_Rect tmp;
	SDL_Surface *TempLoadSurface;

	//--------------------
	// Maybe this function has been called before and everything
	// has been loaded already.  Then of course we don't need to
	// do anything any more and can just return.
	//
	if (TakeoverGraphicsAreAlreadyLoaded)
		return (OK);

	//--------------------
	// Now we start loading all the takeover graphics.
	//
	Set_Rect(tmp, User_Rect.x, User_Rect.y, 0, 0);
	char fp[2048];
	find_file(TO_BLOCK_FILE, GRAPHICS_DIR, fp, 0);
	TempLoadSurface = our_IMG_load_wrapper(fp);
	to_blocks = our_SDL_display_format_wrapperAlpha(TempLoadSurface);	// the surface is converted
	SDL_FreeSurface(TempLoadSurface);
	if (use_open_gl)
		flip_image_vertically(to_blocks);

	// Get the fill-blocks 
	for (i = 0; i < NUM_FILL_BLOCKS; i++, curx += FILL_BLOCK_LEN + 2) {
		Set_Rect(FillRects[i], curx, cury, FILL_BLOCK_LEN, FILL_BLOCK_HEIGHT);
		FillBlocks[i].surface = rip_rectangle_from_alpha_image(to_blocks, FillRects[i]);
		if (use_open_gl)
			make_texture_out_of_surface(&(FillBlocks[i]));
	}

	// Get the capsule blocks 
	for (i = 0; i < NUM_CAPS_BLOCKS; i++, curx += CAPSULE_LEN + 2) {
		Set_Rect(CapsuleRects[i], curx, cury, CAPSULE_LEN, CAPSULE_HEIGHT);
		CapsuleBlocks[i].surface = rip_rectangle_from_alpha_image(to_blocks, CapsuleRects[i]);
		if (use_open_gl)
			make_texture_out_of_surface(&(CapsuleBlocks[i]));
	}

	// Get the default background color, to be used when no background picture found! 
	curx += CAPSULE_LEN + 2;
	curx = 0;
	cury += FILL_BLOCK_HEIGHT + 2;

	// get the game-blocks 
	for (j = 0; j < 2 * NUM_PHASES; j++) {
		for (i = 0; i < TO_BLOCKS; i++) {
			Set_Rect(ToGameRects[j * TO_BLOCKS + i], curx, cury, TO_BLOCKLEN, TO_BLOCKHEIGHT);
			ToGameBlocks[j * TO_BLOCKS + i].surface = rip_rectangle_from_alpha_image(to_blocks, ToGameRects[j * TO_BLOCKS + i]);
			if (use_open_gl)
				make_texture_out_of_surface(&(ToGameBlocks[j * TO_BLOCKS + i]));
			curx += TO_BLOCKLEN + 2;
		}
		curx = 0;
		cury += TO_BLOCKHEIGHT + 2;
	}

	// Get the ground, column and leader blocks 
	for (i = 0; i < NUM_GROUND_BLOCKS; i++) {
		Set_Rect(ToGroundRects[i], curx, cury, GROUNDBLOCKLEN, GROUNDBLOCKHEIGHT);
		ToGroundBlocks[i].surface = rip_rectangle_from_alpha_image(to_blocks, ToGroundRects[i]);
		if (use_open_gl)
			make_texture_out_of_surface(&(ToGroundBlocks[i]));
		curx += GROUNDBLOCKLEN + 2;
	}
	cury += GROUNDBLOCKHEIGHT + 2;
	curx = 0;

	//--------------------
	// Now the rectangle for the column blocks will be set and after
	// that we can create the new surface for blitting.
	//
	Set_Rect(ToColumnRect, curx, cury, COLUMNBLOCKLEN, COLUMNBLOCKHEIGHT);
	ToColumnBlock.surface = rip_rectangle_from_alpha_image(to_blocks, ToColumnRect);
	if (use_open_gl)
		make_texture_out_of_surface(&(ToColumnBlock));

	//--------------------
	// 
	curx += COLUMNBLOCKLEN + 2;

	//--------------------
	// Now the rectangle for the leader block will be set and after
	// that we can create the new surface for blitting.
	//
	Set_Rect(ToLeaderRect, curx, cury, LEADERBLOCKLEN, LEADERBLOCKHEIGHT);
	ToLeaderBlock.surface = rip_rectangle_from_alpha_image(to_blocks, ToLeaderRect);
	if (use_open_gl)
		make_texture_out_of_surface(&(ToLeaderBlock));

	//--------------------
	// Now that everything was loaded, we should remember this, so we
	// don't load anything again next time...
	//
	TakeoverGraphicsAreAlreadyLoaded = TRUE;

	return OK;
};				// int GetTakeoverGraphics ( void )

static void ShowPlayground(void)
{
	int i, j;
	int color, player;
	int block;
	int xoffs, yoffs;
	SDL_Rect Target_Rect;

	xoffs = User_Rect.x + (User_Rect.w - 2 * 290) / 2;
	yoffs = User_Rect.y + (User_Rect.h - 2 * 140) / 2;

	//  SDL_SetColorKey (Screen, 0, 0);
	SDL_SetClipRect(Screen, &User_Rect);

	blit_special_background(TAKEOVER_BACKGROUND_CODE);

	blit_tux(xoffs + DruidStart[YourColor].x, yoffs + DruidStart[YourColor].y + 30);

	if (cDroid) {
		Set_Rect(Target_Rect, xoffs + DruidStart[!YourColor].x, yoffs + DruidStart[!YourColor].y, User_Rect.w, User_Rect.h);
		PutIndividuallyShapedDroidBody(cDroid, Target_Rect, FALSE, FALSE);
	}

	Set_Rect(Target_Rect, xoffs + LEFT_OFFS_X, yoffs + LEFT_OFFS_Y, User_Rect.w, User_Rect.h);

	if (use_open_gl)
		draw_gl_textured_quad_at_screen_position(&ToGroundBlocks[GELB_OBEN], Target_Rect.x, Target_Rect.y);
	else
		our_SDL_blit_surface_wrapper(ToGroundBlocks[GELB_OBEN].surface, NULL, Screen, &Target_Rect);

	Target_Rect.y += GROUNDBLOCKHEIGHT;

	for (i = 0; i < 12; i++) {
		if (use_open_gl)
			draw_gl_textured_quad_at_screen_position(&ToGroundBlocks[GELB_MITTE], Target_Rect.x, Target_Rect.y);
		else
			our_SDL_blit_surface_wrapper(ToGroundBlocks[GELB_MITTE].surface, NULL, Screen, &Target_Rect);
		Target_Rect.y += GROUNDBLOCKHEIGHT;
	}

	if (use_open_gl)
		draw_gl_textured_quad_at_screen_position(&ToGroundBlocks[GELB_UNTEN], Target_Rect.x, Target_Rect.y);
	else
		our_SDL_blit_surface_wrapper(ToGroundBlocks[GELB_UNTEN].surface, NULL, Screen, &Target_Rect);

	// the middle column
	Set_Rect(Target_Rect, xoffs + MID_OFFS_X, yoffs + MID_OFFS_Y, 0, 0);

	if (use_open_gl)
		draw_gl_textured_quad_at_screen_position(&ToLeaderBlock, Target_Rect.x, Target_Rect.y);
	else
		our_SDL_blit_surface_wrapper(ToLeaderBlock.surface, NULL, Screen, &Target_Rect);

	Target_Rect.y += LEADERBLOCKHEIGHT;
	for (i = 0; i < 12; i++, Target_Rect.y += COLUMNBLOCKHEIGHT) {
		if (use_open_gl)
			draw_gl_textured_quad_at_screen_position(&ToColumnBlock, Target_Rect.x, Target_Rect.y);
		else
			our_SDL_blit_surface_wrapper(ToColumnBlock.surface, NULL, Screen, &Target_Rect);
	}

	// the right column
	Set_Rect(Target_Rect, xoffs + RIGHT_OFFS_X, yoffs + RIGHT_OFFS_Y, 0, 0);
	if (use_open_gl)
		draw_gl_textured_quad_at_screen_position(&ToGroundBlocks[VIOLETT_OBEN], Target_Rect.x, Target_Rect.y);
	else
		our_SDL_blit_surface_wrapper(ToGroundBlocks[VIOLETT_OBEN].surface, NULL, Screen, &Target_Rect);

	Target_Rect.y += GROUNDBLOCKHEIGHT;

	for (i = 0; i < 12; i++, Target_Rect.y += GROUNDBLOCKHEIGHT) {
		if (use_open_gl)
			draw_gl_textured_quad_at_screen_position(&ToGroundBlocks[VIOLETT_MITTE], Target_Rect.x, Target_Rect.y);
		else
			our_SDL_blit_surface_wrapper(ToGroundBlocks[VIOLETT_MITTE].surface, NULL, Screen, &Target_Rect);
	}

	if (use_open_gl)
		draw_gl_textured_quad_at_screen_position(&ToGroundBlocks[VIOLETT_UNTEN], Target_Rect.x, Target_Rect.y);
	else
		our_SDL_blit_surface_wrapper(ToGroundBlocks[VIOLETT_UNTEN].surface, NULL, Screen, &Target_Rect);

	// Fill the leader-LED with its color 
	Set_Rect(Target_Rect, xoffs + LEADERLED_X, yoffs + LEADERLED_Y, 0, 0);
	if (use_open_gl)
		draw_gl_textured_quad_at_screen_position(&FillBlocks[LeaderColor], Target_Rect.x, Target_Rect.y);
	else
		our_SDL_blit_surface_wrapper(FillBlocks[LeaderColor].surface, NULL, Screen, &Target_Rect);

	Target_Rect.y += FILL_BLOCK_HEIGHT;
	if (use_open_gl)
		draw_gl_textured_quad_at_screen_position(&FillBlocks[LeaderColor], Target_Rect.x, Target_Rect.y);
	else
		our_SDL_blit_surface_wrapper(FillBlocks[LeaderColor].surface, NULL, Screen, &Target_Rect);

	// Fill the display column with its colors 
	for (i = 0; i < NUM_LINES; i++) {
		Set_Rect(Target_Rect, xoffs + LEDCOLUMN_X, yoffs + LEDCOLUMN_Y + i * (FILL_BLOCK_HEIGHT + 2), 0, 0);
		if (use_open_gl)
			draw_gl_textured_quad_at_screen_position(&FillBlocks[DisplayColumn[i]], Target_Rect.x, Target_Rect.y);
		else
			our_SDL_blit_surface_wrapper(FillBlocks[DisplayColumn[i]].surface, NULL, Screen, &Target_Rect);
	}

	// Show the yellow playground 
	for (i = 0; i < NUM_LAYERS - 1; i++)
		for (j = 0; j < NUM_LINES; j++) {
			Set_Rect(Target_Rect, xoffs + PlaygroundStart[GELB].x + i * TO_BLOCKLEN,
				 yoffs + PlaygroundStart[GELB].y + j * TO_BLOCKHEIGHT, 0, 0);
			block = ToPlayground[GELB][i][j] + ActivationMap[GELB][i][j] * TO_BLOCKS;
			if (use_open_gl)
				draw_gl_textured_quad_at_screen_position(&ToGameBlocks[block], Target_Rect.x, Target_Rect.y);
			else
				our_SDL_blit_surface_wrapper(ToGameBlocks[block].surface, NULL, Screen, &Target_Rect);
		}

	// Show the violett playground 
	for (i = 0; i < NUM_LAYERS - 1; i++)
		for (j = 0; j < NUM_LINES; j++) {
			Set_Rect(Target_Rect,
				 xoffs + PlaygroundStart[VIOLETT].x + (NUM_LAYERS - i - 2) * TO_BLOCKLEN,
				 yoffs + PlaygroundStart[VIOLETT].y + j * TO_BLOCKHEIGHT, 0, 0);
			block = ToPlayground[VIOLETT][i][j] + (NUM_PHASES + ActivationMap[VIOLETT][i][j]) * TO_BLOCKS;
			if (use_open_gl)
				draw_gl_textured_quad_at_screen_position(&ToGameBlocks[block], Target_Rect.x, Target_Rect.y);
			else
				our_SDL_blit_surface_wrapper(ToGameBlocks[block].surface, NULL, Screen, &Target_Rect);
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
			if (use_open_gl)
				draw_gl_textured_quad_at_screen_position(&CapsuleBlocks[color], Target_Rect.x, Target_Rect.y);
			else
				our_SDL_blit_surface_wrapper(CapsuleBlocks[color].surface, NULL, Screen, &Target_Rect);
		}

		for (i = 0; i < NumCapsules[player] - 1; i++) {
			Set_Rect(Target_Rect, xoffs + LeftCapsulesStart[color].x,
				 yoffs + LeftCapsulesStart[color].y + i * CAPSULE_HEIGHT, 0, 0);
			if (use_open_gl)
				draw_gl_textured_quad_at_screen_position(&CapsuleBlocks[color], Target_Rect.x, Target_Rect.y);
			else
				our_SDL_blit_surface_wrapper(CapsuleBlocks[color].surface, NULL, Screen, &Target_Rect);
		}		// for capsules 
	}			// for player 

	SetCurrentFont(Para_BFont);
	CenteredPutString(Screen, GameConfig.screen_height - 25, "Press F1 for help.");
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

	for (color = GELB; color < TO_COLORS; color++)
		for (layer = 0; layer < NUM_LAYERS; layer++)
			for (row = 0; row < NUM_LINES; row++) {
				ActivationMap[color][layer][row] = INACTIVE;
				if (layer < TO_COLORS - 1)
					ToPlayground[color][layer][row] = KABEL;
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
	int color = GELB;

	//--------------------
	// first clear the playground: we depend on this !! 
	//
	ClearPlayground();

	for (color = GELB; color < TO_COLORS; color++) {
		for (layer = 1; layer < NUM_LAYERS - 1; layer++) {
			for (row = 0; row < NUM_LINES; row++) {
				if (ToPlayground[color][layer][row] != KABEL)
					continue;

				newElement = MyRandom(TO_ELEMENTS);
				if (MyRandom(MAX_PROB) > ElementProb[newElement]) {
					row--;
					continue;
				}

				switch (newElement) {
				case EL_KABEL:	/* has not to be set any more */
					anElement = ToPlayground[color][layer - 1][row];
					if (BlockClass[anElement] == NON_CONNECTOR)
						ToPlayground[color][layer][row] = LEER;
					break;

				case EL_KABELENDE:
					anElement = ToPlayground[color][layer - 1][row];
					if (BlockClass[anElement] == NON_CONNECTOR)
						ToPlayground[color][layer][row] = LEER;
					else
						ToPlayground[color][layer][row] = KABELENDE;
					break;

				case EL_VERSTAERKER:
					anElement = ToPlayground[color][layer - 1][row];
					if (BlockClass[anElement] == NON_CONNECTOR)
						ToPlayground[color][layer][row] = LEER;
					else
						ToPlayground[color][layer][row] = VERSTAERKER;
					break;

				case EL_FARBTAUSCHER:
					if (layer != 2) {	/* only existing on layer 2 */
						row--;
						continue;
					}

					anElement = ToPlayground[color][layer - 1][row];
					if (BlockClass[anElement] == NON_CONNECTOR)
						ToPlayground[color][layer][row] = LEER;
					else
						ToPlayground[color][layer][row] = FARBTAUSCHER;
					break;

				case EL_VERZWEIGUNG:
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

					/* dont destroy verzweigungen in prev. layer */
					anElement = ToPlayground[color][layer - 1][row];
					if (anElement == VERZWEIGUNG_O || anElement == VERZWEIGUNG_U) {
						row--;
						break;
					}
					anElement = ToPlayground[color][layer - 1][row + 2];
					if (anElement == VERZWEIGUNG_O || anElement == VERZWEIGUNG_U) {
						row--;
						break;
					}

					/* cut off kabels in last layer, if any */
					anElement = ToPlayground[color][layer - 1][row];
					if (BlockClass[anElement] == CONNECTOR)
						ToPlayground[color][layer - 1][row] = KABELENDE;

					anElement = ToPlayground[color][layer - 1][row + 2];
					if (BlockClass[anElement] == CONNECTOR)
						ToPlayground[color][layer - 1][row + 2] = KABELENDE;

					/* set the verzweigung itself */
					ToPlayground[color][layer][row] = VERZWEIGUNG_O;
					ToPlayground[color][layer][row + 1] = VERZWEIGUNG_M;
					ToPlayground[color][layer][row + 2] = VERZWEIGUNG_U;

					row += 2;
					break;

				case EL_GATTER:
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

					/* cut off kabels in last layer, if any */
					anElement = ToPlayground[color][layer - 1][row + 1];
					if (BlockClass[anElement] == CONNECTOR)
						ToPlayground[color][layer - 1][row + 1] = KABELENDE;

					/* set the GATTER itself */
					ToPlayground[color][layer][row] = GATTER_O;
					ToPlayground[color][layer][row + 1] = GATTER_M;
					ToPlayground[color][layer][row + 2] = GATTER_U;

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
	int color = GELB;
	float ScoreFound[TO_COLORS];

#define EVALUATE_PLAYGROUND_DEBUG 1

	for (color = GELB; color < TO_COLORS; color++) {

		DebugPrintf(EVALUATE_PLAYGROUND_DEBUG, "\n----------------------------------------------------------------------\n\
Starting to evaluate side nr. %d.  Results displayed below:\n", color);
		ScoreFound[color] = 0;

		for (layer = 1; layer < NUM_LAYERS - 1; layer++) {
			for (row = 0; row < NUM_LINES; row++) {

				// we examine this particular spot
				newElement = ToPlayground[color][layer][row];

				switch (newElement) {
				case KABEL:	/* has not to be set any more */
				case LEER:
					break;

				case KABELENDE:
					DebugPrintf(EVALUATE_PLAYGROUND_DEBUG, "KABELENDE found --> score -= 1.0\n");
					ScoreFound[color] -= 1.0;
					break;

				case VERSTAERKER:
					DebugPrintf(EVALUATE_PLAYGROUND_DEBUG, "VERSTAERKER found --> score += 0.5\n");
					ScoreFound[color] += 0.5;
					break;

				case FARBTAUSCHER:
					DebugPrintf(EVALUATE_PLAYGROUND_DEBUG, "FARBTAUSCHER found --> score -= 1.5\n");
					ScoreFound[color] -= 1.5;
					break;

				case VERZWEIGUNG_O:
				case VERZWEIGUNG_U:
				case VERZWEIGUNG_M:
					DebugPrintf(EVALUATE_PLAYGROUND_DEBUG, "VERZWEIGUNG found --> score += 1.0\n");
					ScoreFound[color] += 1.0;
					break;

				case GATTER_M:
				case GATTER_U:
				case GATTER_O:
					DebugPrintf(EVALUATE_PLAYGROUND_DEBUG, "GATTER found --> score -= 1.0\n");
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
 * This function generates a random playground for the takeover game
 * ----------------------------------------------------------------- */
float EvaluatePosition(int color, int row, int layer)
{
	int newElement;
	// float ScoreFound [ TO_COLORS ];

#define EVAL_DEBUG 1

	DebugPrintf(EVAL_DEBUG, "\nEvaluatePlaygound ( %d , %d , %d ) called: ", color, row, layer);

	if (layer == NUM_LAYERS - 1) {
		DebugPrintf(EVAL_DEBUG, "End layer reached...");
		if (DisplayColumn[row] == color) {
			DebugPrintf(EVAL_DEBUG, "same color... returning 0.5 ");
			return (0.5);
		} else {
			DebugPrintf(EVAL_DEBUG, "different color... returning 1.5 ");
			return (1.5);
		}
	}

	newElement = ToPlayground[color][layer][row];

	switch (newElement) {
	case KABEL:		/* has not to be set any more */
		DebugPrintf(EVAL_DEBUG, "KABEL reached... continuing...");
		return (EvaluatePosition(color, row, layer + 1));
	case LEER:
		DebugPrintf(EVAL_DEBUG, "LEER reached... stopping...");
		return (0);
		break;

	case KABELENDE:
		DebugPrintf(EVAL_DEBUG, "KABELENDE reached... returning now...");
		return (0);
		break;

	case VERSTAERKER:
		DebugPrintf(EVAL_DEBUG, "VERSTAERKER reached... continuing...");
		return (1.5 * EvaluatePosition(color, row, layer + 1));
		break;

	case FARBTAUSCHER:
		DebugPrintf(EVAL_DEBUG, "FARBTAUSCHER reached... continuing...");
		return (-1.5 * EvaluatePosition(color, row, layer + 1));
		break;

	case VERZWEIGUNG_O:
		DebugPrintf(EVAL_DEBUG, "\nERROR:  REACHED UNREACHABLE SPOT: VERZWEIGUNG_O !!!\n");
		return (0);
		break;
	case VERZWEIGUNG_U:
		DebugPrintf(EVAL_DEBUG, "\nERROR:  REACHED UNREACHABLE SPOT: VERZWEIGUNG_U !!!\n");
		return (0);
		break;
	case GATTER_M:
		DebugPrintf(EVAL_DEBUG, "\nERROR:  REACHED UNREACHABLE SPOT: GATTER_M !!!\n");
		return (0);
		break;

	case VERZWEIGUNG_M:
		DebugPrintf(EVAL_DEBUG, "VERZWEIGUNG reached... double-continuing...");
		return (EvaluatePosition(color, row + 1, layer + 1) + EvaluatePosition(color, row - 1, layer + 1));
		break;

	case GATTER_O:
		DebugPrintf(EVAL_DEBUG, "GATTER reached... stopping...\n");
		return (0.3 * EvaluatePosition(color, row + 1, layer + 1));
		break;

	case GATTER_U:
		DebugPrintf(EVAL_DEBUG, "GATTER reached... stopping...\n");
		return (0.3 * EvaluatePosition(color, row - 1, layer + 1));
		break;

	default:
		DebugPrintf(EVAL_DEBUG, "\nUNHANDLED TILE reached\n");
		break;

	}			// switch NewElement 

	return (0);

};				// float EvaluatePosition ( col , row , layer )

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

	for (color = GELB; color < TO_COLORS; color++) {
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
				case FARBTAUSCHER:
				case VERZWEIGUNG_M:
				case GATTER_O:
				case GATTER_U:
				case KABEL:
					if (ActivationMap[color][layer - 1][row] >= ACTIVE1)
						TurnActive = TRUE;
					break;

				case VERSTAERKER:
					if (ActivationMap[color][layer - 1][row] >= ACTIVE1)
						TurnActive = TRUE;

					// additional enforcers stay active by themselves...
					if (ActivationMap[color][layer][row] >= ACTIVE1)
						TurnActive = TRUE;

					break;

				case KABELENDE:
					break;

				case VERZWEIGUNG_O:
					if (ActivationMap[color][layer][row + 1] >= ACTIVE1)
						TurnActive = TRUE;
					break;

				case VERZWEIGUNG_U:
					if (ActivationMap[color][layer][row - 1] >= ACTIVE1)
						TurnActive = TRUE;
					break;

				case GATTER_M:
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
					TurnActive = FALSE;
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
 * Binking leds are realized here as well.
 */
void ProcessDisplayColumn(void)
{
	static int CLayer = 3;	/* the connection-layer to the Column */
	static int flicker_color = 0;
	int row;
	int GelbCounter, ViolettCounter;

	flicker_color = !flicker_color;

	for (row = 0; row < NUM_LINES; row++) {
		// unquestioned yellow
		if ((ActivationMap[GELB][CLayer][row] >= ACTIVE1) && (ActivationMap[VIOLETT][CLayer][row] == INACTIVE)) {
			// change color?
			if (ToPlayground[GELB][CLayer - 1][row] == FARBTAUSCHER)
				DisplayColumn[row] = VIOLETT;
			else
				DisplayColumn[row] = GELB;
			continue;
		}
		// clearly magenta
		if ((ActivationMap[GELB][CLayer][row] == INACTIVE) && (ActivationMap[VIOLETT][CLayer][row] >= ACTIVE1)) {
			// change color?
			if (ToPlayground[VIOLETT][CLayer - 1][row] == FARBTAUSCHER)
				DisplayColumn[row] = GELB;
			else
				DisplayColumn[row] = VIOLETT;

			continue;
		}
		// undecided: flimmering
		if ((ActivationMap[GELB][CLayer][row] >= ACTIVE1) && (ActivationMap[VIOLETT][CLayer][row] >= ACTIVE1)) {
			// change color?
			if ((ToPlayground[GELB][CLayer - 1][row] == FARBTAUSCHER) &&
			    (ToPlayground[VIOLETT][CLayer - 1][row] != FARBTAUSCHER))
				DisplayColumn[row] = VIOLETT;
			else if ((ToPlayground[GELB][CLayer - 1][row] != FARBTAUSCHER) &&
				 (ToPlayground[VIOLETT][CLayer - 1][row] == FARBTAUSCHER))
				DisplayColumn[row] = GELB;
			else {
				if (flicker_color == 0)
					DisplayColumn[row] = GELB;
				else
					DisplayColumn[row] = VIOLETT;
			}	/* if - else if - else */

		}
		/* if undecided */
	}			/* for */

	// evaluate the winning color
	GelbCounter = 0;
	ViolettCounter = 0;
	for (row = 0; row < NUM_LINES; row++)
		if (DisplayColumn[row] == GELB)
			GelbCounter++;
		else
			ViolettCounter++;

	if (ViolettCounter < GelbCounter)
		LeaderColor = GELB;
	else if (ViolettCounter > GelbCounter)
		LeaderColor = VIOLETT;
	else
		LeaderColor = REMIS;

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

	for (color = GELB; color <= VIOLETT; color++)
		for (row = 0; row < NUM_LINES; row++) {
			if (CapsuleCountdown[color][0][row] > 0)
				CapsuleCountdown[color][0][row]--;

			if (CapsuleCountdown[color][0][row] == 0) {
				CapsuleCountdown[color][0][row] = -1;
				ActivationMap[color][0][row] = INACTIVE;
				ToPlayground[color][0][row] = KABEL;
			}

		}		/* for row */

};				// void ProcessCapsules ( void )

/**
 * This function tells, wether a Column-connection is active or not.
 * It returns TRUE or FALSE accordinly.
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

	for (color = GELB; color <= VIOLETT; color++)
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
	int left_len, right_len;	// the actualy string lengths

	// --------------------
	// At first the text is prepared.  This can't hurt.
	// we will decide whether to dispaly it or not later...
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
		left_len = LEFT_TEXT_LEN;	// too long, so we cut it! 
		Terminate(ERR);
	}
	right_len = strlen(right);
	if (right_len > RIGHT_TEXT_LEN) {
		printf("\nWarning: String %s too long for Right Infoline!!", right);
		right_len = RIGHT_TEXT_LEN;	// too long, so we cut it! 
		Terminate(ERR);
	}
	// Now prepare the left/right text-boxes 
	memset(left_box, ' ', LEFT_TEXT_LEN);	// pad with spaces 
	memset(right_box, ' ', RIGHT_TEXT_LEN);

	strncpy(left_box, left, left_len);	// this drops terminating \0 ! 
	strncpy(right_box, right, left_len);	// this drops terminating \0 ! 

	left_box[LEFT_TEXT_LEN] = '\0';	// that's right, we want padding!
	right_box[RIGHT_TEXT_LEN] = '\0';

	// Now the text should be ready and its
	// time to display it...
	DebugPrintf(2, "Takeover said: %s -- %s\n", left_box, right_box);
	SetCurrentFont(Para_BFont);
	DisplayText(left_box, LEFT_INFO_X, LEFT_INFO_Y, NULL, TEXT_STRETCH);
	DisplayText(right_box, RIGHT_INFO_X, RIGHT_INFO_Y, NULL, TEXT_STRETCH);

};				// void to_show_banner (const char* left, const char* right)

#undef _takeover_c
