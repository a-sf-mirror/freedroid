/* 
 *
 *   Copyright (c) 2010 Arthur Huillet
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

/**
 * @struct widget_button
 * @brief Widget button type.
 *
 * This structure type is used for managing buttons.
 * Buttons have 3 main states (normal, hovered and pressed) and two
 * toggle states.
 */
struct widget_button {
	struct widget base;				/**< Base widget containing callbacks and position info. */
	enum { DEFAULT, HOVERED, PRESSED } state;	/**< Button primary state variable. */
	int active;					/**< Flag for switching between the two toggle states. */
	struct image *image[2][3];					/**< Image pointers for each toggle state. */
	void (*activate_button) (struct widget_button *);		/**< Left click callback. */
	void (*activate_button_secondary) (struct widget_button *);	/**< Right click callback. */
	string text;					/**< Text displayed in center of the button's rectangle. */
	struct tooltip tooltip;				/**< Tooltip displayed on mouse hover. */
};

struct widget_button *widget_button_create(void);

#define WIDGET_BUTTON(x) ((struct widget_button *)x)
