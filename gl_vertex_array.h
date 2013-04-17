#ifndef GL_UTIL_H_
#define GL_UTIL_H_

#include <GL/gl.h>

#include "font.h"

#define NAME gl_vertex_array
#include "gl_vertex_array_base.h"

#define NAME gl_vertex_array_color
#define WITH_COLOR
#include "gl_vertex_array_base.h"

#define NAME gl_vertex_array_texuv
#define WITH_TEXUV
#include "gl_vertex_array_base.h"

#define NAME gl_vertex_array_texuv_color
#define WITH_TEXUV
#define WITH_COLOR
#include "gl_vertex_array_base.h"

#endif // GL_UTIL_H_
