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
#ifndef _leveleditor_widgets_c
#define EXTERN extern
#else
#define EXTERN
#endif

enum leveleditor_widget_type {
    WIDGET_BUTTON,
    WIDGET_TOOLBAR,
    WIDGET_MAP,
};

/* A widget in the level editor */
struct leveleditor_widget {
    enum leveleditor_widget_type type; //Type of widget
    SDL_Rect rect; //Space occupied
    struct list_head node;
    int enabled;
    void (*mouseenter)(void *);
    void (*mouseleave)(void *);
    void (*mouserelease)(void *);
    void (*mousepress)(void *);
    void (*mouserightrelease)(void *);
    void (*mouserightpress)(void *);
    void (*mousewheelup)(void *);
    void (*mousewheeldown)(void *);
    void * ext; //Type specific information
};

struct leveleditor_button {
    int btn_index; //index in AllMousePressButtons array
    int pressed;
    int active;
};

struct leveleditor_toolbar {
};

struct leveleditor_mapwidget {
};

void leveleditor_display_widgets(void);
void leveleditor_update_button_states(void);
#undef EXTERN
