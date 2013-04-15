#ifndef GL_UTIL_H_
#define GL_UTIL_H_

#include <GL/gl.h>

#include "font.h"

#define NAME gl_vertex_array
#include "gl_vertex_array.h"

#define NAME gl_vertex_array_color
#define WITH_COLOR
#include "gl_vertex_array.h"

#define NAME gl_vertex_array_texuv
#define WITH_TEXUV
#include "gl_vertex_array.h"

#define NAME gl_vertex_array_texuv_color
#define WITH_TEXUV
#define WITH_COLOR
#include "gl_vertex_array.h"

GLuint
initialize_gl_texture(int texture_width, int texture_height, const unsigned char *texture_buf,
  GLint internal_format, GLenum pixel_format);

#endif // GL_UTIL_H_
