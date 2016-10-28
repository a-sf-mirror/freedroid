/*
 *   Copyright (c) 2013 Arthur Huillet
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
 * This file contains OpenGL debug-related code.
 * It makes use of KHR_debug if available.
 */

#define _open_gl_debug_c 1

#include "system.h"
#include "defs.h"
#include "struct.h"
#include "global.h"
#include "proto.h"

// For a not yet known reason, init_opengl_debug() crashes on some Win10
// computers (observed on 64b systems with NVidia GPU).
// Further test is needed to solve that issue.

#if defined(HAVE_LIBGL) && defined(GL_KHR_debug)

#define DBG_FLAG(f) { f, #f }
struct debug_flag {
	GLenum flag_value;
	char *flag_descr;
} debug_flags[] = {
	DBG_FLAG(GL_DEBUG_SOURCE_API),
	DBG_FLAG(GL_DEBUG_SOURCE_WINDOW_SYSTEM),
	DBG_FLAG(GL_DEBUG_SOURCE_SHADER_COMPILER),
	DBG_FLAG(GL_DEBUG_SOURCE_THIRD_PARTY),
	DBG_FLAG(GL_DEBUG_SOURCE_APPLICATION),
	DBG_FLAG(GL_DEBUG_SOURCE_OTHER),
	DBG_FLAG(GL_DEBUG_TYPE_ERROR),
	DBG_FLAG(GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR),
	DBG_FLAG(GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR),
	DBG_FLAG(GL_DEBUG_TYPE_PORTABILITY),
	DBG_FLAG(GL_DEBUG_TYPE_PERFORMANCE),
	DBG_FLAG(GL_DEBUG_TYPE_OTHER),
	DBG_FLAG(GL_DEBUG_TYPE_MARKER),
	DBG_FLAG(GL_DEBUG_TYPE_PUSH_GROUP),
	DBG_FLAG(GL_DEBUG_TYPE_POP_GROUP),
	DBG_FLAG(GL_DEBUG_SEVERITY_NOTIFICATION),
	DBG_FLAG(GL_DEBUG_SEVERITY_HIGH),
	DBG_FLAG(GL_DEBUG_SEVERITY_MEDIUM),
	DBG_FLAG(GL_DEBUG_SEVERITY_LOW),
};
#undef DBG_FLAG

static struct debug_flag *find_debug_flag(GLenum value)
{
	unsigned int i;
	for (i=0; i < sizeof(debug_flags)/sizeof(debug_flags[0]); i++) {
		if (debug_flags[i].flag_value == value)
			return &debug_flags[i];
	}
	return NULL;
}

static void GLAPIENTRY gl_debug_callback(GLenum source, GLenum type, GLuint id,
							GLenum severity, GLsizei length, const GLchar* message,
							const void *userParam)
{
	// Ignore certain message IDs

	if (id == 131204) {
		// "Waste of memory: Texture 0 has mipmaps, while it's min filter is inconsistent with mipmaps."
		// Possible nvidia bug in version 313.30, ignore it for now
		return;
	}
	if (severity == GL_DEBUG_SEVERITY_NOTIFICATION) {
		// Do not display notifications
		return;
	}

	// Report a good looking error message

	struct auto_string *msg = alloc_autostr(256);
	struct debug_flag *data = NULL;

	data = find_debug_flag(source);
	if (data) {
		autostr_append(msg, "Source = %s,", data->flag_descr);
	} else {
		autostr_append(msg, "Source = 0x%x,", source);
	}

	data = find_debug_flag(type);
	if (data) {
		autostr_append(msg, " type = %s,", data->flag_descr);
	} else {
		autostr_append(msg, " type = 0x%x,", type);
	}

	autostr_append(msg, " id = %d,", id);

	data = find_debug_flag(severity);
	if (data) {
		autostr_append(msg, " severity = %s,", data->flag_descr);
	} else {
		autostr_append(msg, " severity = 0x%x,", severity);
	}

	error_message(__FUNCTION__, "%s: %s", NO_REPORT, msg->value, message);

	free_autostr(msg);
}

/**
 * Initialize and enabled the OpenGL debug features.
 * @return 0 if OK, 1 if debug could not be enabled
 */
int init_opengl_debug(void)
{
	/* Check if KHR_debug is available */
	if (!GLEW_KHR_debug) {
		// no debug extension available
		// We cannot use ARB_debug_output because it doesn't allow glEnable(GL_DEBUG_OUTPUT)
		return 1;
	}

	glEnable(GL_DEBUG_OUTPUT);
	/* Note: uncomment the following line if you want to use gdb with a
	   breakpoint in gl_debug_callback() and want to have the OpenGL call
	   leading to the debug report be in the current execution calling stack */
	//glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
	glDebugMessageCallback(&gl_debug_callback, NULL);
	glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, NULL, GL_TRUE);

	return 0;
}

void gl_debug_marker(const char *str)
{
	if (GLEW_GREMEDY_string_marker) {
		glStringMarkerGREMEDY(strlen(str), str);
	}
}

#else // defined(HAVE_LIBGL) && defined(GL_KHR_debug)

int init_opengl_debug(void)
{
	return 1;
}

void gl_debug_marker(const char *str)
{
}

#endif // defined(HAVE_LIBGL) && defined(GL_KHR_debug)

/**
 * This function checks the error status of the OpenGL driver.  An error
 * will produce at least a warning message, maybe even program termination
 * if the errors are really severe.
 */
void open_gl_check_error_status(const char *name_of_calling_function)
{
#ifdef HAVE_LIBGL
	char *enum_str = "UNKNOWN";
	int error_type = PLEASE_INFORM;

	switch (glGetError()) {
		case GL_NO_ERROR:
			return;
		case GL_INVALID_ENUM:
			enum_str = "GL_INVALID_ENUM";
			break;
		case GL_INVALID_VALUE:
			enum_str = "GL_INVALID_VALUE";
			break;
		case GL_INVALID_OPERATION:
			enum_str = "GL_INVALID_OPERATION";
			break;
		case GL_STACK_OVERFLOW:
			enum_str = "GL_STACK_OVERFLOW";
			error_type |= IS_FATAL;
			break;
		case GL_STACK_UNDERFLOW:
			enum_str = "GL_STACK_UNDERFLOW";
			error_type |= IS_FATAL;
			break;
		case GL_OUT_OF_MEMORY:
			enum_str = "GL_OUT_OF_MEMORY";
			error_type |= IS_FATAL;
			break;
	}
	
	error_message(__FUNCTION__, "Error code %s received, called by %s.", error_type, enum_str, name_of_calling_function);
#endif
}


#undef _open_gl_debug_c
