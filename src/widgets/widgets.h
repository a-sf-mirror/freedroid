/* 
 *
 *   Copyright (c) 2009-2010 Arthur Huillet
 *   Copyright (c) 2014 Samuel Degrande
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
 * \file widgets.h
 * \brief This file contains the declaration of structure type and functions
 *        defining a base widget type, + some misc functions and types.
 */

/// \defgroup gui2d GUI subsystem
///
/// The GUI subsystem is used to create the interfaces between the game and the
/// user (head up display, menu, dialog screen, shop screen, craft/upgrade addon
/// panels, inventory/character/skill panels, quest log...).
///
/// \par General Overview
///   \n
///   A GUI is a tree of \e widgets. The leaves of the tree are interactive widgets,
///   the higher part of the hierarchy being composed of (possibly interactive)
///   \e container widgets. A container can have several children widgets. A
///   bounding box (rectangular area of the screen) is defined for each widget.\n
///   The game contains several trees (i.e. several GUIs), only one of them being
///   active at a given time.\n
///   \n
///   \b Note: This subsystem does not implement all classical features of a GUI lib.
///   All positions and sizes are statics, the widgets cannot be dynamically resized
///   or moved. The interaction system is also minimal.\n
///   \n
///   The current GUI is drawn from the top of the tree down to the leaves, and
///   from left to right for sibling widgets. Thus, the last child of a node
///   visually will cover its sibling, if drawn at the same position.\n
///   All widgets positions are absolute, expressed in screen coordinates.\n
///   \n
///   An interaction event is propagated from the top of the current active GUI
///   down to the leaves, and from right to left for sibling widgets (this ensures
///   that the most visible widget is the first one to receive the event).\n
///   As soon as a widget \e consumes the event, the propagation is stopped.\n
///   For a \e positioned event (event associated to a screen position, such as a
///   mouse event), the widgets's bounding boxes are used to limit the propagation
///   to the widgets covering the event's position.\n
///
/// \par Pseudo-Object Mechanism
///   \n
///   The widgets, no matter their types, share several common behaviors and
///   properties. For example, most of them have a bounding box, can be drawn and
///   can consume an event.\n
///   In order to be able to write \e widget-type-agnostic algorithms (i.e. in
///   order to avoid a lot of switches on the widget's type), an inheritance
///   mechanism is used, alongside pseudo-classes.\n
///   \n
///   This mechanism is based on C-structure composition and function pointers:\n
///   - A base widget pseudo-class is defined as a C-struct:\n
///     \code
/// struct widget {
///   SDL_Rect rect;                                          // Bounding box
///   ...
///   void (*display)(struct widget *);                       // Pseudo-virtual display function
///   void (WIDGET_ANONYMOUS_MARKER update)(struct widget *); // Pseudo_virtual update callback
///   int (*handle_event)(struct widget *, SDL_Event *);      // Pseudo-virtual event handler
///   void (*free)(struct widget *);                          // Pseudo-virtual 'release' function
///   ...
/// };
///     \endcode
///     A default behavior is implemented for each pseudo-virtual function.\n
///     \n
///   - A derived widget pseudo-class is defined by composition. The \e inherited
///     widget pseudo-class \b has to be the first field of the C-struct:\n
///     \code
/// struct widget_button {
///   struct widget base;   // Base widget pseudo-inheritance
///   struct image *image;  // Background image of the button
///   string text;          // Text displayed in center of the button's rectangle
///   ...
/// };
///     \endcode
///     \n
///   - Widget-specific functions are defined, to \e overload the base pseudo-virtual
///     functions:\n
///     \code
/// void button_display(struct widget *this_widget)
/// {
///   struct widget_button *this_button = (struct widget_button *)this_widget;
///
///   ... display the button, using 'this_widget' pointer to access base attributes
///   ... and 'this_button' pointer to access button-specific attributes
///   ... examples: this_widget->rect  this_button->image
/// }
///     \endcode
///     \n
///   - A button constructor can now be written. A macro is also introduced to
///     ease pointer type casting:\n
///     \code
/// struct widget_button *widget_button_create()
/// {
///   struct widget_button *button = MyMalloc(sizeof(struct widget_button));
///   widget_init(WIDGET(button));
///
///   button->image = NULL;
///   button->text = NULL;
///
///   base->display = button_display;
///   ...
///   return button;
/// }
///
/// #define WIDGET_BUTTON(x) ((struct widget_button *)x)
///     \endcode
///
/// \par
///   We can now write simple algorithms that call a pseudo-virtual function on a
///   set of widgets. Here is a very trivial example:
///   \code
/// struct widget *gui[3];
/// gui[0] = widget_create();
/// gui[1] = WIDGET(widget_button_create());
/// gui[2] = WIDGET(widget_text_create());
/// for (i=0; i<3; i++) gui[i]->display(gui[i]);
/// \endcode
///
/// \par Event handler
///   \n
///   Each widget class willing to handle interaction events has to implement a
///   specific \e handle_event() function.\n
///   If that function returns 0, then the event is propagated to the next widget
///   in the tree.\n
///   To indicate that the widget consumed the event, \e handle_event() must return 1.
///
/// \par Release function
///   \n
///   This function is called when a widget instance is deleted. All the memory
///   allocated during instance creation has to be freed in this function.
///   The main widget_free() has to be called before returning to the caller:
///   \code
/// void my_widget_free(struct widget *w)
/// {
///   ... free memory allocated during widget creation ...
///   widget_free(w);
/// }
/// \endcode
///
/// \par Dynamic update of widget attributes
///   \n
///   Some attributes of a widget can depend on external data. For example, the
///   visibility of a widget or the text of a button could depend on the current
///   state of the game.\n
///   We want to limit the intrusiveness of a GUI in the actual game code. It
///   means that it is not the responsibility of the game code to change a widget's
///   attribute. To update itself, a widget shall overload the \e update()
///   pseudo-virtual function. That function is called when needed by the gui
///   subsystem engine.\n
///   Note: An \e update() function \b has to be an anonymous function (see below).
///   \code
/// void my_button_update(struct widget *this_widget)
/// {
///   struct widget_button *this_button = WIDGET_BUTTON(this_widget);
///   if (some_global_variable == some_value)
///     this_button->image = img1;
///   else
///     this_button->image = img2;
/// }
///
/// struct widget_button *my_button = widget_button_create();
/// WIDGET(my_button)->update = WIDGET_ANONYMOUS(struct widget *w, { my_button_update(w); });
///   \endcode
///   \n
///   Most of the time, an \update() function is a single line of code, such as
///   setting one widget's attribute depending of an other value. Sometime, a
///   more complex code is to be called. We would thus like to be able to set
///   an \e update() function as a pointer to an actual named function or to an
///   anonymous function.\n
///   To simulate an anonymous function definition, a WIDGET_ANONYMOUS
///   macro has been defined (see http://en.wikipedia.org/wiki/Anonymous_function#C_.28non-standard_extension.29).
///   The code inside an anonymous function can be as complex as wanted, but
///   for readability large code should be avoided.
///   \n
///   Note: Due to the way WIDGET_ANONYMOUS is implemented with some compilers
///   (clang, especially), \b all \e update() functions have to be defined using
///   WIDGET_ANONYMOUS, even if a simple function's pointer would usually be enough
///   (see above example).
///   \n
///   The former example can, for instance be rewritten:
///   \code
/// WIDGET(my_button)->update = WIDGET_ANONYMOUS(struct widget *w, {
///                               WIDGET_BUTTON(w)->image = (some_global_variable == some_value) ? img1 : img2;
///                                });
///   \endcode
///   \n
///   To call of function returning a value that can be stored in the widget's
///   attribute, one will write:
///   \code
/// struct image *get_img() { ... }
/// WIDGET(my_button)->update = WIDGET_ANONYMOUS(struct widget *w, {
///                               WIDGET_BUTTON(w)->image = get_img();
///                             });
///   \endcode
///
/// \par Creating a GUI
///   \n
///   A GUI is a tree composed of containers (widget_group) and \e terminal widgets.
///   The inclusion of a GUI in the game needs:\n
///   \n
///   - A getter function.\n
///     Example of a small GUI composed of a root node and a single child:
///     \code
/// static widget_group *my_gui_root = NULL;
///
/// struct widget_group *get_my_gui()
/// {
///   if (my_gui_root)
///     // GUI already initialized.
///     return my_gui_root;
///
///   my_gui_root = widget_group_create();
///   widget_set_rect(WIDGET(my_gui_root), gui_x, gui_y, gui_width, gui_height);
///
///   struct widget_button *the_button = widget_button_create();
///   widget_set_rect(WIDGET(the_button), button_x, button_y, button_width, button_height);
///   ... set some other button properties
///
///   widget_group_add(my_gui_root, WIDGET(the_button));
///
///   return my_gui_root;
/// }
///     \endcode
///     \n
///   - An inclusion in the get_active_ui() global function.\n
///     Currently, a global \e game_status variable is used to define the current
///     active GUI. A new value has to be added to that enum, and the get_active_ui()
///     function has to be extended:
///     \code
/// enum { ..., INSIDE_MY_GUI } game_status;
///
/// static struct widget *get_active_ui()
/// {
///   ...
///   switch (game_status) {
///     ...
///     case INSIDE_MY_GUI:
///       new_active_ui = WIDGET(get_my_gui());
///       break;
///     ...
///   }
///   ...
/// }
///     \endcode
///
/// \par Panels
///   \n
///   During the run of the game, it is sometimes needed to temporarily open a
///   \e window (we use the term \e panel rather than \e window). Inventory panel is
///   an example of such a \e window.\n
///   \n
///   Since only one single GUI is displayed at a time, such panels have to be
///   included in the current GUI tree (under the root node of the tree), and be
///   made invisible by default. Their update() function can then be used to open
///   them when needed.\n
///   \n
///   Example:
///   \code
/// ...
/// my_gui_root = widget_group_create();
/// widget_set_rect(WIDGET(my_gui_root), gui_x, gui_y, gui_width, gui_height);
///
/// // create the main GUI and add it to the root node
/// main_gui = widget_group_create();
/// ...
/// widget_group_add(my_gui_root, WIDGET(main_gui));
///
/// // create the invisible panel and add it to the root node
/// panel = widget_group_create();
/// WIDGET(panel)->enabled = FALSE;
/// WIDGET(panel)->update = WIDGET_ANONYMOUS(struct widget *w, { w->enabled = Game.panel_opened; });
/// ...
/// widget_group_add(my_gui_root, WIDGET(panel));
///   \endcode
///
/// \par Simulating transient panel
///   \n
///   A transient panel is a GUI sub-tree (see above) that is displayed on
///   top of the screen and \e captures all events. It means that events outside
///   of the transient window must not be propagated to the other parts of the
///   GUI.\n
///   \n
///   Such a mechanism is not natively implemented, but can be simulated by
///   adding a widget_background covering the whole screen \e under the transient
///   panel, which captures all events. In freedroidRPG, we usually use a
///   semi-transparent image for that background widget.\n
///   \n
///   Example code:
///   \code
/// static int catch_all_events(struct widget *w, SDL_Event *evt)
/// {
///   return 1;
/// }
/// ...
/// transient_panel = widget_group_create();
/// widget_set_rect(WIDGET(transient_panel), 0, 0, screen_width, screen_height);
///
/// catch_all_bkg = widget_background_create();
/// widget_set_rect(WIDGET(catch_all_bkg), 0, 0, screen_width, screen_height);
/// widget_background_add(catch_all_bkg, "black_semi_transparent.png", 0, 0, screen_width, screen_height);
/// WIDGET(catch_all_bkg)->handle_event = call_all_events;
/// widget_group_add(transient_panel, WIDGET(catch_all_bkg));
/// ...
/// ... add other widgets to transient_panel
/// ...
///   \endcode
///

#ifndef _widgets_h_
#define _widgets_h_

#undef EXTERN
#ifndef _widgets_c
#define EXTERN extern
#else
#define EXTERN
#endif

#ifdef __clang__
#define WIDGET_ANONYMOUS_MARKER ^
#else
#define WIDGET_ANONYMOUS_MARKER *
#endif

///////////////////////////////////////////////////////////////////////////////
/// \defgroup gui2d_widget Base widget type
/// \ingroup gui2d
///
/// The \e widget C-struct is the base type on which all other widget types
/// are constructed.\n
/// \n
/// It contains the attributes common to all widgets (such as a bounding box
/// \e rect, an \e enabled flag...), as well as the mandatory pseudo-virtual
/// functions needed to display and interact with a widget, and a default
/// implementation of those functions.
///
///@{
// start gui2d_widget submodule

/**
 * \brief Enumeration of all known widget types.
 *
 * \deprecated
 * This enumeration is needed for compatibility with the lvleditor current code.
 * It will be removed as soon as the lvleditor is converted to fully use the new
 * gui subsystem.
 */
enum widget_type {
	WIDGET_BUTTON,
	WIDGET_TOOLBAR,
	WIDGET_MAP,
	WIDGET_CATEGORY_SELECTOR,
	WIDGET_MINIMAP,
};

/**
 * \brief Base widget type.
 *
 * This is the base type used by the widget system.\n
 * It contains basic information and callbacks used by all widget types.\n
 * \n
 * \b NOTE: Widget types inheriting this type must have it as their first attribute.
 */
struct widget {
	/// \name Public attributes
	///
	/// \var rect
	///       Widget position and size are not automatically computed, so if a
	///       container's child is moved or resized, the container position and size
	///       have to be recomputed by the user's code.
	/// \var enabled
	///       A disabled widget is not displayed, and does not handle input event.
	///       It's update() function is however called.
	/// @{
	SDL_Rect rect;         /**< Rectangle containing widget's size and position. */
	uint8_t enabled;       /**< Boolean flag used for enabling/disabling the widget. */
	enum widget_type type; /**< Enum representing the widget's type. (deprecated) */
	void *ext;             /**< Pointer to type specific data. (deprecated) */
	/// @}

	/// \name Public pseudo-virtual functions
	///       Define the actual rendering and behavior of a widget type.
	///       The basic implementation provided by the base widget can be overridden
	///       by inheriting widgets.
	/// @{
	void (*display) (struct widget *);                        /**< Display the widget. */
	void (WIDGET_ANONYMOUS_MARKER update) (struct widget *);  /**< Update the widget's attributes. */
	int (*handle_event) (struct widget *, SDL_Event *);       /**< General event handler. */
	void (*free) (struct widget *);                           /**< Free the widget. */
	/// @}

	/// \name Private internal functions and attributes
	///       Needed for the management of a widget's tree, not meant to changed
	///     by the user.
	/// @{
	void (*update_tree) (struct widget *);              /**< Update call propagation */
	struct list_head node;                              /**< Linked list node used for storing sibling widgets in a widget_group. */
	/// @}
};

/**
 * \brief Type casting macro.
 *
 * Cast a pointer to a type inheriting from a \e base \e widget into a pointer
 * to a \e base \e widget.
 */
#define WIDGET(x) ((struct widget *)x)

/**
 * \brief Macro used to create an anonymous function.
 *
 * Currenly only used to define \e update() functions.
 * \n
 * Usage examples:\n
 * - Set the \e enabled attribute to the value of one game data
 *   \code
 * the_widget->update = WIDGET_ANONYMOUS(struct widget *w, {
 *                          widget_type(w)->enabled = boolean_game_data;
 *                      });
 *   \endcode
 * - Set the \e enabled attribute depending on the value of one game data
 *   \code
 * the_widget->update = WIDGET_ANONYMOUS(struct widget *w, {
 *                          widget_type(w)->enabled = (int_game_data >= 0.5) ? TRUE : FALSE;
 *                      });
 *   \endcode
 * - Set the \e enabled attribute to be the result of a function call
 *   \code
 * the_widget->update = WIDGET_ANONYMOUS(struct widget *w, {
 *                          widget_type(w)->enabled = compute_value(some_game_data...);
 *                      });
 *   \endcode
 * - Call a function
 *   \code
 * the_widget->update = WIDGET_ANONYMOUS(struct widget *w, {
 *                          some_function(w, ...);
 *                      });
 *   \endcode
 *
 * \param param       'signature' of the callback function
 * \param code        the code to execute (has to be enclosed into curly brackets)
 */
/*
 * Implementation note.
 * The goal of the macro is to create an anonymous function and to return a
 * pointer to this function.
 * On gcc, this is done with the use of a compound statement expression and a
 * nested function declared inside the compound block (gcc extensions).
 * On clang, which does not implement nested functions, this is done with the
 * use of the clang 'block' extension.
 */
#ifdef __clang__
#define WIDGET_ANONYMOUS(param, code) \
  ^void(param) \
    code
#else
#define WIDGET_ANONYMOUS(param, code) \
  ({ \
    void anonymous_func(param) \
      code \
    anonymous_func; \
  })
#endif

struct widget *widget_create(void);
void widget_init(struct widget *);
void widget_set_rect(struct widget *, int, int, int, int);
void widget_free(struct widget *w);

// end gui2d_widget submodule
///@}
///////////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////
/// \defgroup gui2d_tooltip Tooltips
/// \ingroup gui2d
///
/// The GUI subsystem includes a minimalist implementation of tooltips:
/// only one tooltip is registered and displayed.\n
/// \n
/// It is thus the responsibility of the user code to set/unset the tooltip to
/// be used at a given time. This is usually done when a widget detects
/// a mouse enter/leave event.\n
/// \n
/// The text displayed by a tooltip can be static, or dynamic through the use
/// of a callback function, called just before to display the tooltip (see
/// struct tooltip).\n
/// \n
/// Example code:
/// \code
/// static string get_my_tooltip_text() {
///   ...
///   return some_computed_text;
/// }
///
/// struct tooltip my_tooltip = { get_my_tooltip_text, "" };
///
/// int mywidget_handle_event(struct widget *w, SDL_Event *event)
/// {
///   ...
///   case SDL_USEREVENT:
///     if (event->user.code == EVENT_MOUSE_ENTER) {
///       ...
///       // Register the tooltip
///       widget_set_tooltip(my_tooltip, &w->rect);
///     }
///     if (event->user.code == EVENT_MOUSE_LEAVE) {
///        ...
///       // Unregister the tooltip
///       widget_set_tooltip(NULL, NULL);
///     }
///   ...
/// }
/// \endcode
///
///@{
// start gui2d_tooltip submodule

/**
 * \brief Tooltip placeholder
 *
 * Structure used for storing and displaying tooltips.
 *
 * The tooltip text will be retrieved using the get_text function pointer. If this pointer is NULL,
 * the text field will be used instead.\n
 * \n
 * \b NOTE: get_text should be used for dynamic tooltips while the text field should be used for static tooltips.
 */
struct tooltip {
	string (*get_text)(void); /**< Returns the text of a dynamic tooltip. */
	string text;              /**< String used for static tooltip texts. */
};

void widget_set_tooltip(struct tooltip *, SDL_Rect *);

// end gui2d_tooltip submodule
///@}
///////////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////
/// \defgroup gui2d_misc Misc
/// \ingroup gui2d
///
/// Miscellaneous functions, definitions, or global variables.\n
/// Mainly used in the code of the widgets.
///
///@{
// start gui2d_misc submodule

/** User event code used for signaling mouse entering a widget. */
#define EVENT_MOUSE_ENTER 0
/** User event code used for signaling mouse leaving a widget. */
#define EVENT_MOUSE_LEAVE 1

/** Explicit name for mouse button #1 (usually left button) */
#define MOUSE_BUTTON_1 1
/** Explicit name for mouse button #2 (usually middle button) */
#define MOUSE_BUTTON_2 2
/** Explicit name for mouse button #3 (usually right button) */
#define MOUSE_BUTTON_3 3

struct image *widget_load_image_resource(char *, int);
void widget_free_image_resources();

// end gui2d_misc submodule
///@}
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
/// \defgroup gui2d_interface External entry points
/// \ingroup gui2d
///
/// Three functions are defined as \e interface to the gui subsystem.\n
/// They have to be called in the game loop in the following order, to ensure
/// a correct behavior of the widgets:
/// - handle_widget_event(): Propagate an interaction event inside the current
///                          active tree.
/// - update_widgets(): Ask the elements of the current active widget tree to
///                     update their attributes.
/// - display_widgets(): Displays the currently active widget tree.
///
/// The GUI tree used by those functions is the one returned by get_active_ui(),
/// which depends on the current value of the \e game_status global variable.
///
/// widget_set_tooltip(NULL, NULL) (see \ref gui2d_tooltip) must be called
/// before to start the game loop, in order to reset the display of tooltips.
///
///@{
// start gui2d_interface submodule

void handle_widget_event(SDL_Event *);
void update_widgets(void);
void display_widgets(void);

// end gui2d_interface submodule
///@}
///////////////////////////////////////////////////////////////////////////////

#undef EXTERN

#include "widgets/widget_group.h"
#include "widgets/widget_button.h"
#include "widgets/widget_text.h"
#include "widgets/widget_background.h"
#include "widgets/widget_text_list.h"

#endif // _widgets_h_
