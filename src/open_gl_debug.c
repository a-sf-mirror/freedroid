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

#define _open_gl_debug_c

#include "system.h"
#include "defs.h"
#include "struct.h"
#include "global.h"
#include "proto.h"

// Copy-paste of glext.h because SDL's is outdated
#ifndef GL_KHR_debug
#define GL_DEBUG_OUTPUT_SYNCHRONOUS       0x8242
#define GL_DEBUG_NEXT_LOGGED_MESSAGE_LENGTH 0x8243
#define GL_DEBUG_CALLBACK_FUNCTION        0x8244
#define GL_DEBUG_CALLBACK_USER_PARAM      0x8245
#define GL_DEBUG_SOURCE_API               0x8246
#define GL_DEBUG_SOURCE_WINDOW_SYSTEM     0x8247
#define GL_DEBUG_SOURCE_SHADER_COMPILER   0x8248
#define GL_DEBUG_SOURCE_THIRD_PARTY       0x8249
#define GL_DEBUG_SOURCE_APPLICATION       0x824A
#define GL_DEBUG_SOURCE_OTHER             0x824B
#define GL_DEBUG_TYPE_ERROR               0x824C
#define GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR 0x824D
#define GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR  0x824E
#define GL_DEBUG_TYPE_PORTABILITY         0x824F
#define GL_DEBUG_TYPE_PERFORMANCE         0x8250
#define GL_DEBUG_TYPE_OTHER               0x8251
#define GL_DEBUG_TYPE_MARKER              0x8268
#define GL_DEBUG_TYPE_PUSH_GROUP          0x8269
#define GL_DEBUG_TYPE_POP_GROUP           0x826A
#define GL_DEBUG_SEVERITY_NOTIFICATION    0x826B
#define GL_MAX_DEBUG_GROUP_STACK_DEPTH    0x826C
#define GL_DEBUG_GROUP_STACK_DEPTH        0x826D
#define GL_BUFFER                         0x82E0
#define GL_SHADER                         0x82E1
#define GL_PROGRAM                        0x82E2
#define GL_QUERY                          0x82E3
#define GL_PROGRAM_PIPELINE               0x82E4
#define GL_SAMPLER                        0x82E6
#define GL_DISPLAY_LIST                   0x82E7
/* DISPLAY_LIST used in compatibility profile only */
#define GL_MAX_LABEL_LENGTH               0x82E8
#define GL_MAX_DEBUG_MESSAGE_LENGTH       0x9143
#define GL_MAX_DEBUG_LOGGED_MESSAGES      0x9144
#define GL_DEBUG_LOGGED_MESSAGES          0x9145
#define GL_DEBUG_SEVERITY_HIGH            0x9146
#define GL_DEBUG_SEVERITY_MEDIUM          0x9147
#define GL_DEBUG_SEVERITY_LOW             0x9148
#define GL_DEBUG_OUTPUT                   0x92E0
#define GL_CONTEXT_FLAG_DEBUG_BIT         0x00000002
/* reuse GL_STACK_UNDERFLOW */
/* reuse GL_STACK_OVERFLOW */
typedef void (APIENTRY *GLDEBUGPROC)(GLenum source,GLenum type,GLuint id,GLenum severity,GLsizei length,const GLchar *message,GLvoid *userParam);
typedef void (APIENTRYP PFNGLDEBUGMESSAGECONTROLPROC) (GLenum source, GLenum type, GLenum severity, GLsizei count, const GLuint *ids, GLboolean enabled);
typedef void (APIENTRYP PFNGLDEBUGMESSAGEINSERTPROC) (GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar *buf);
typedef void (APIENTRYP PFNGLDEBUGMESSAGECALLBACKPROC) (GLDEBUGPROC callback, const void *userParam);
typedef GLuint (APIENTRYP PFNGLGETDEBUGMESSAGELOGPROC) (GLuint count, GLsizei bufsize, GLenum *sources, GLenum *types, GLuint *ids, GLenum *severities, GLsizei *lengths, GLchar *messageLog);
typedef void (APIENTRYP PFNGLPUSHDEBUGGROUPPROC) (GLenum source, GLuint id, GLsizei length, const GLchar *message);
typedef void (APIENTRYP PFNGLPOPDEBUGGROUPPROC) (void);
typedef void (APIENTRYP PFNGLOBJECTLABELPROC) (GLenum identifier, GLuint name, GLsizei length, const GLchar *label);
typedef void (APIENTRYP PFNGLGETOBJECTLABELPROC) (GLenum identifier, GLuint name, GLsizei bufSize, GLsizei *length, GLchar *label);
typedef void (APIENTRYP PFNGLOBJECTPTRLABELPROC) (const void *ptr, GLsizei length, const GLchar *label);
typedef void (APIENTRYP PFNGLGETOBJECTPTRLABELPROC) (const void *ptr, GLsizei bufSize, GLsizei *length, GLchar *label);
#endif

PFNGLDEBUGMESSAGECONTROLPROC glDebugMessageControl;
PFNGLDEBUGMESSAGECALLBACKPROC glDebugMessageCallback;

static void gl_debug_callback(GLenum source, GLenum type, GLuint id,
							GLenum severity, GLsizei length, const GLchar* message,
							GLvoid* userParam)
{
	// Ignore certain message IDs
	if (id == 131204) {
		// "Waste of memory: Texture 0 has mipmaps, while it's min filter is inconsistent with mipmaps."
		// Possible nvidia bug in version 313.30, ignore it for now
		return;
	}

	// Report a good looking error message
	char buf[strlen(message) + 150];

	sprintf(buf, "Source = %x, type = %x, id = %d, severity = %x: %s ", source, type, id, severity, message);
	ErrorMessage(__FUNCTION__, buf, NO_NEED_TO_INFORM, IS_WARNING_ONLY);
}

/**
 * Initialize and enabled the OpenGL debug features.
 * @return 0 if OK, 1 if debug could not be enabled
 */
int init_opengl_debug(void)
{
#ifdef HAVE_LIBGL
	/* Check if KHR_debug is available */
	const char *extensions = glGetString(GL_EXTENSIONS);
	if (!strstr(extensions, "GL_KHR_debug")) {
		// no debug extension available
		// We cannot use ARB_debug_output because it doesn't allow glEnable(GL_DEBUG_OUTPUT)
		return 1;
	}

	glDebugMessageControl = SDL_GL_GetProcAddress("glDebugMessageControl");
	glDebugMessageCallback = SDL_GL_GetProcAddress("glDebugMessageCallback");

	if (!glDebugMessageCallback || !glDebugMessageControl) {
		ErrorMessage(__FUNCTION__, "Unable to retrieve function pointers for glDebugMessageCallback and glDebugMessageControl, but debug extension is present.", PLEASE_INFORM, IS_WARNING_ONLY);
		return 1;
	}

	glEnable(GL_DEBUG_OUTPUT);
	glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
	glDebugMessageCallback(&gl_debug_callback, NULL);
	glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, NULL, GL_TRUE);

	return 0;
#else
	return 1;
#endif
}

/**
 * This function checks the error status of the OpenGL driver.  An error
 * will produce at least a warning message, maybe even program termination
 * if the errors are really severe.
 */
void open_gl_check_error_status(const char *name_of_calling_function)
{
	char *enum_str = "UNKNOWN";
	int fatal = IS_WARNING_ONLY;

#ifdef HAVE_LIBGL
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
			fatal = IS_FATAL;
			break;
		case GL_STACK_UNDERFLOW:
			enum_str = "GL_STACK_UNDERFLOW";
			fatal = IS_FATAL;
			break;
		case GL_OUT_OF_MEMORY:
			enum_str = "GL_OUT_OF_MEMORY";
			fatal = IS_FATAL;
			break;
	}
	
	ErrorMessage(__FUNCTION__, "Error code %s received, called by %s.", PLEASE_INFORM, fatal, enum_str, name_of_calling_function);
#endif
}


#undef _open_gl_debug_c
