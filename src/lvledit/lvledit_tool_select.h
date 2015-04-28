
/* 
 *
 *   Copyright (c) 2009-2010 Arthur Huillet
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

int leveleditor_select_input(SDL_Event * event);
int leveleditor_select_display(void);

void leveleditor_select_reset(void);

int selection_empty(void);
void *single_tile_selection(int);
int element_in_selection(void *);

point selection_start(void);
point selection_len(void);
int selection_type(void);

int level_editor_can_cycle_marked_object(void);
void level_editor_cycle_marked_object(void);
void level_editor_delete_selection(void);
void level_editor_cut_selection(void);
void level_editor_copy_selection(void);
void level_editor_paste_selection(void);

void clear_clipboard(int);
void clear_selection(int);
int remove_element_from_selection(void *);

void level_editor_switch_selection_type(int direction);
