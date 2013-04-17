#include <cassert>

#include "gl_vertex_array.h"

#define NAME gl_vertex_array
#include "gl_vertex_array_base.cc"

#define NAME gl_vertex_array_color
#define WITH_COLOR
#include "gl_vertex_array_base.cc"

#define NAME gl_vertex_array_texuv
#define WITH_TEXUV
#include "gl_vertex_array_base.cc"

#define NAME gl_vertex_array_texuv_color
#define WITH_TEXUV
#define WITH_COLOR
#include "gl_vertex_array_base.cc"
