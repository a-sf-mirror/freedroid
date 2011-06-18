/* 
 *
 *   Copyright (c) 2009 Arthur Huillet
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

#undef EXTERN
#ifndef _widgets_c
#define EXTERN extern
#endif

void widget_lvledit_toolbar_mouseenter(SDL_Event *, struct widget *);
void widget_lvledit_toolbar_mouseleave(SDL_Event *, struct widget *);
void widget_lvledit_toolbar_mouserelease(SDL_Event *, struct widget *);
void widget_lvledit_toolbar_mousepress(SDL_Event *, struct widget *);
void widget_lvledit_toolbar_mouserightrelease(SDL_Event *, struct widget *);
void widget_lvledit_toolbar_mouserightpress(SDL_Event *, struct widget *);
void widget_lvledit_toolbar_mousewheelup(SDL_Event *, struct widget *);
void widget_lvledit_toolbar_mousewheeldown(SDL_Event *, struct widget *);

void widget_lvledit_toolbar_display(struct widget *);

void widget_lvledit_toolbar_left(void);
void widget_lvledit_toolbar_right(void);
void widget_lvledit_toolbar_scroll_left(void);
void widget_lvledit_toolbar_scroll_right(void);
