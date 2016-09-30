#pragma once

#include <memory>

#include "rgba.h"
#include "vec2.h"

class gl_texture;

enum class blend_mode { NO_BLEND, ALPHA_BLEND, ADDITIVE_BLEND };

struct quad
{
	vec2f v00, v01, v10, v11;
};

namespace render {

void begin_batch();
void end_batch();

void push_matrix();
void pop_matrix();

void translate(const vec2f& p);
void translate(float x, float y);

void scale(const vec2f& s);
void scale(float s);
void scale(float sx, float sy);

void rotate(float a);

void set_blend_mode(blend_mode mode);
void set_color(const rgba& color);

void add_quad(const quad& verts, float z);
void add_quad(const gl_texture *tex, const quad& verts, const quad& texcoords, float z);

}
