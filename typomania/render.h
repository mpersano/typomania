#pragma once

#include "rgba.h"
#include "vec2.h"

namespace gl {
class texture;
class program;
}

enum class blend_mode { NO_BLEND, ALPHA_BLEND, ADDITIVE_BLEND };

struct quad
{
	vec2f v00, v01, v10, v11;
};

namespace render {

void init();

void set_viewport(int x_min, int x_max, int y_min, int y_max);

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

void draw_quad(const quad& verts, int layer);
void draw_quad(const gl::texture *texture, const quad& verts, const quad& texcoords, int layer);
void draw_quad(const gl::texture *texture, const quad& verts, int layer);
void draw_quad(const gl::texture *texture, const vec2f& pos, int layer);

void draw_quad(const gl::program *program, const quad& verts, int layer);
void draw_quad(const gl::program *program, const gl::texture *texture, const quad& verts, const quad& texcoords, int layer);
void draw_quad(const gl::program *program, const gl::texture *texture, const quad& verts, int layer);
void draw_quad(const gl::program *program, const gl::texture *texture, const vec2f& pos, int layer);


}
