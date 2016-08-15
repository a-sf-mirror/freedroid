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

/**
 * \file widget_background.h
 * \brief This file contains the declaration of structure type and functions
 *        defining a background widget type.
 */

#ifndef _widget_background_h_
#define _widget_background_h_

///////////////////////////////////////////////////////////////////////////////
/// \defgroup gui2d_background Background widget type
/// \ingroup gui2d
///
/// The \e widget_background inherits from the base widget and implements a
/// simple widget that displays a set of images (or a \e tiled \e image).\n
/// Its main usage is to serve as a background for a widget_group.\n
/// \n
/// As many images as desired can be added to widget_background. Each image is
/// defined by a static position, expressed in screen coordinates, and a size.
/// The image is un-uniformly scaled to fit the desired size.\n
///
/// \par Screen-size independent background: Tiled image
///   \n
///   One of the classical ways to create a background independent of the
///   screen's size and ratio is to use a \e tiled \e image (a 9-tiled image,
///   usually).\n
///   With a 9-tiled image, the corner tiles are displayed using their intrinsic
///   sizes, while the others are scaled to fill the size of the whole background.\n
///   The widget_background_load_3x3_tiles() function is used to add a set of 9
///   images composing a 9-tiled background. It will compute all positions and
///   sizes to fit the widget_background.
///@{
// start gui2d_background submodule

/**
 * \brief Widget background type
 *
 * This widget is used for displaying a background.\n
 * The background can be composed of several tiles.
 */
struct widget_background {
	/// \name Base Type
	/// @{
	struct widget base;           /**< Pseudo-inheritance of the base widget type. */
	/// @}

	/// \name Public attributes
	///       Needed for the management of the widget and its representation
	/// @{
	struct dynarray tiles;        /**< Array of images used as tiles for the background. */
	/// @}
};

/**
 * \brief Type casting macro.
 *
 * Cast a pointer to a type inheriting from a \e base \e widget into a pointer to
 * a \e widget_background.
 */
#define WIDGET_BACKGROUND(x) ((struct widget_background *)x)

struct widget_background *widget_background_create(void);
void widget_background_add(struct widget_background *, struct image *, int, int, int, int,  enum image_transformation_mode);
void widget_background_clear(struct widget_background *);
void widget_background_load_3x3_tiles(struct widget_background *, char *);

// end gui2d_background submodule
///@}
///////////////////////////////////////////////////////////////////////////////

#endif // _widget_background_h_
