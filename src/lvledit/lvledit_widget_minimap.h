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

void leveleditor_minimap_mouseenter(SDL_Event *, struct leveleditor_widget *);
void leveleditor_minimap_mouseleave(SDL_Event *, struct leveleditor_widget *);
void leveleditor_minimap_mouserelease(SDL_Event *, struct leveleditor_widget *);
void leveleditor_minimap_mousepress(SDL_Event *, struct leveleditor_widget *);
void leveleditor_minimap_mouserightrelease(SDL_Event *, struct leveleditor_widget *);
void leveleditor_minimap_mouserightpress(SDL_Event *, struct leveleditor_widget *);
void leveleditor_minimap_mousewheelup(SDL_Event *, struct leveleditor_widget *);
void leveleditor_minimap_mousewheeldown(SDL_Event *, struct leveleditor_widget *);
void leveleditor_minimap_display(struct leveleditor_widget *);
