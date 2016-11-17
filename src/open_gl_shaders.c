/*
 *
 *   Copyright (c) 2016 Arthur Huillet
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
 * This file contains OpenGL shaders.
 */

#include "system.h"

#include "defs.h"
#include "struct.h"
#include "global.h"
#include "proto.h"

/*
 "blitter_shader" is a simple blitter, used to present textured quads on screen.
 Because old/unextended GLSL doesn't allow picking a texture unit in the
 fragment shader at runtime the simple way (indexing a sampler2D array), the
 following trick is used:
   - "s" texture coordinate is modified by adding to it the texture unit to be
      used (with a multiplier to ensure that it doesn't alias with a "typically
      legal" texture coordinate). As it's normalized, the texture coordinate in
      FreedroidRPG will be between 0 and 1, so the fractional part is the
      coordinate, while the integer part is the texture unit.
      Example: s = 0.05  => unit 0, "real" s = 0.05
               s = 10.05 => unit 1, "real" s = 0.05
               s = 11.0  => unit 1, "real" s = 1.0
	- fix up the texture coordinates in the vertex shader, outputting the
	  texture unit in the "u" texture coordinate
	- use a collection of "if" in the fragment shader to determine which
	  texture to read from

 The fragment shader part is kind of ugly but I am aware of no other way to
 achieve this result, without using texture arrays or bindless texture
 extensions.

 Note: 0.5 is added to textcoord.z in the vertex shader, making sure that the
 value passed to the fragment shader is always slightly greater than the
 expected value, such that truncating yields that expected value exactly.
*/

static const char *blitter_shader_vs_source = "#version 120\n"
	"void main() {"
	"	gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;"
	"	float realx = mod(gl_MultiTexCoord0.s, 10);"
	"	gl_TexCoord[0].t = gl_MultiTexCoord0.t;"
	"	gl_TexCoord[0].s = realx;"
	"	gl_TexCoord[0].z = (gl_MultiTexCoord0.s - realx + 0.5);"
	"	gl_FrontColor = gl_Color;"
	"}";

static const char *blitter_shader_fs_source = "#version 120\n"
	"uniform sampler2D tex[4];"
	"void main() {"
	"	vec4 t;"
	"	int unit = int(gl_TexCoord[0].z);"
	"	if (unit == 0)       { t = texture2D(tex[0], gl_TexCoord[0].st); }"
	"	else if (unit == 10) { t = texture2D(tex[1], gl_TexCoord[0].st); }"
	"	else if (unit == 20) { t = texture2D(tex[2], gl_TexCoord[0].st); }"
	"	else if (unit == 30) { t = texture2D(tex[3], gl_TexCoord[0].st); }"
	"	else { t = vec4(1.0, 0.0, 0.0, 1.0); }"
	"	if (t.a <= 0.01) discard;"
	"	gl_FragColor = t * gl_Color;"
	"}";

/* Same shader, but with 2 textures only */
static const char *blitter_shader_fs_source_2tex = "#version 120\n"
	"uniform sampler2D tex[4];"
	"void main() {"
	"	vec4 t;"
	"	int unit = int(gl_TexCoord[0].z);"
	"	if (unit == 0)       { t = texture2D(tex[0], gl_TexCoord[0].st); }"
	"	else if (unit == 10) { t = texture2D(tex[1], gl_TexCoord[0].st); }"
	"	else { t = vec4(1.0, 0.0, 0.0, 1.0); }"
	"	if (t.a <= 0.01) discard;"
	"	gl_FragColor = t * gl_Color;"
	"}";

static unsigned int blitter_shader_vs;
static unsigned int blitter_shader_fs;
static unsigned int blitter_shader_prog;
static unsigned int blitter_shader_texID_uniform;

static enum shader current_shader = NO_SHADER;

/**
 * Change the shader currently in use. This function
 * avoids redundant changes.
 */
void use_shader(enum shader shader)
{
	if (current_shader == shader) {
		return;
	}

	switch (shader) {
		default:
			shader = NO_SHADER;
			// Fall-through
		case NO_SHADER:
			glUseProgram(0);
			break;
		case BLITTER_SHADER:
			glUseProgram(blitter_shader_prog);
			break;
	}

	current_shader = shader;
}

void init_shaders(void)
{
	// Initialize blitter shader
	blitter_shader_vs = glCreateShader(GL_VERTEX_SHADER);
	blitter_shader_fs = glCreateShader(GL_FRAGMENT_SHADER);
	struct {
		unsigned int id;
		const char *src;
	} shaders[] = {
		{ blitter_shader_vs, blitter_shader_vs_source },
		{ blitter_shader_fs, (get_opengl_quirks() & MULTITEX_MAX_2TEX) ? blitter_shader_fs_source_2tex : blitter_shader_fs_source },
	};

	int i;
	for (i = 0; i < sizeof(shaders) / sizeof(shaders[0]); i++) {
		glShaderSource(shaders[i].id, 1, &shaders[i].src, NULL);
		glCompileShader(shaders[i].id);
		int ret;
		glGetShaderiv(shaders[i].id, GL_COMPILE_STATUS, &ret);
		if (!ret) {
			char errlog[4096];
			glGetShaderInfoLog(shaders[i].id, sizeof(errlog) - 1, NULL, &errlog[0]);
			error_message(__FUNCTION__, "Error compiling shader: %s\n", PLEASE_INFORM | IS_FATAL, errlog);
		}
	}

	blitter_shader_prog = glCreateProgram();
	glAttachShader(blitter_shader_prog, blitter_shader_vs);
	glAttachShader(blitter_shader_prog, blitter_shader_fs);

	glLinkProgram(blitter_shader_prog);

	glDetachShader(blitter_shader_prog, blitter_shader_vs);
	glDetachShader(blitter_shader_prog, blitter_shader_fs);
	glDeleteShader(blitter_shader_vs);
	glDeleteShader(blitter_shader_fs);

	int ret;
	glGetProgramiv(blitter_shader_prog, GL_LINK_STATUS, &ret);
	if (!ret) {
		char errlog[4096];
		glGetProgramInfoLog(blitter_shader_prog, sizeof(errlog) - 1, NULL, &errlog[0]);
		error_message(__FUNCTION__, "Error linking shader: %s\n", PLEASE_INFORM | IS_FATAL, errlog);
	}

	// Set the texture unit IDs for the fragment program.
	blitter_shader_texID_uniform = glGetUniformLocation(blitter_shader_prog, "tex");

	use_shader(BLITTER_SHADER); // needed to set the uniform (we could rely on EXT_direct_state_access)
	int tmp[] = { 0, 1, 2, 3 };
	glUniform1iv(blitter_shader_texID_uniform, 4, &tmp[0]);
}
