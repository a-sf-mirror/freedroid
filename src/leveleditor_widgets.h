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
    WIDGET_OBJECTTYPESELECTORBUTTON, //hahaha what a name! behold!
    WIDGET_MENU,
};

/* A widget in the level editor */
struct leveleditor_widget {
    enum leveleditor_widget_type type; //Type of widget
    SDL_Rect rect; //Space occupied
    struct list_head node;
    int enabled;
    void (*mouseenter)(SDL_Event *, struct leveleditor_widget *);
    void (*mouseleave)(SDL_Event *, struct leveleditor_widget *);
    void (*mouserelease)(SDL_Event *, struct leveleditor_widget *);
    void (*mousepress)(SDL_Event *, struct leveleditor_widget *);
    void (*mouserightrelease)(SDL_Event *, struct leveleditor_widget *);
    void (*mouserightpress)(SDL_Event *, struct leveleditor_widget *);
    void (*mousewheelup)(SDL_Event *, struct leveleditor_widget *);
    void (*mousewheeldown)(SDL_Event *, struct leveleditor_widget *);
    void (*mousemove)(SDL_Event *, struct leveleditor_widget *);
    int (*keybevent)(SDL_Event *, struct leveleditor_widget *);
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

struct leveleditor_typeselect {
    unsigned int selected_tile_nb;
    unsigned int toolbar_first_block;
    char * title;
    enum leveleditor_object_type type;
    int * indices;
};

struct leveleditor_menu {
    char text[10][100];
    void *values[10];
    void (*done_cb)(void *);
    int currently_selected_idx;
    int ispressed;
};

void leveleditor_init_widgets(void);
void leveleditor_display_widgets(void);
void leveleditor_update_button_states(void);

struct leveleditor_widget *get_active_widget(int, int); 
struct leveleditor_widget *create_button(int);
struct leveleditor_widget *create_menu();
EXTERN struct list_head leveleditor_widget_list;
EXTERN struct leveleditor_widget * previously_active_widget;

#undef EXTERN

#include "leveleditor_widget_button.h"
#include "leveleditor_widget_map.h"
#include "leveleditor_widget_menu.h"
#include "leveleditor_widget_toolbar.h"
#include "leveleditor_widget_typeselect.h"

