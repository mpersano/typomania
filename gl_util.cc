#include <cassert>

#include "gl_util.h"

#define NAME gl_vertex_array
#include "gl_vertex_array.cc"

#define NAME gl_vertex_array_color
#define WITH_COLOR
#include "gl_vertex_array.cc"

#define NAME gl_vertex_array_texuv
#define WITH_TEXUV
#include "gl_vertex_array.cc"

#define NAME gl_vertex_array_texuv_color
#define WITH_TEXUV
#define WITH_COLOR
#include "gl_vertex_array.cc"

GLuint
initialize_gl_texture(
  int texture_width, int texture_height,
  const unsigned char *texture_buf,
  GLint internal_format,
  GLenum pixel_format)
{
	GLuint texture_id;

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	
	glEnable(GL_TEXTURE_2D);
	glGenTextures(1, &texture_id);

	glBindTexture(GL_TEXTURE_2D, texture_id);

	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	glTexImage2D(
	  GL_TEXTURE_2D, 0, internal_format,
	  texture_width, texture_height,
	  0, pixel_format, GL_UNSIGNED_BYTE,
	  texture_buf);

	return texture_id;
}
