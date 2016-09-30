#include <cassert>
#include <algorithm>
#include <stack>

#include <GL/gl.h>

#include "mat3.h"
#include "gl_texture.h"
#include "render.h"

namespace {

void gl_set_blend_mode(blend_mode mode)
{
	switch (mode) {
		case blend_mode::NO_BLEND:
			glDisable(GL_BLEND);
			break;

		case blend_mode::ALPHA_BLEND:
			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			break;

		case blend_mode::ADDITIVE_BLEND:
			glEnable(GL_BLEND);
			glBlendFunc(GL_ONE, GL_ONE);
			break;
	}
}

}

namespace render {

class render_queue
{
public:
	static render_queue& instance();

	render_queue();

	void begin_batch();
	void end_batch();

	void push_matrix();
	void pop_matrix();

	void translate(const vec2f& p);
	void scale(const vec2f& s);

	void rotate(float a);

	void set_blend_mode(blend_mode mode);
	void set_color(const rgba& color);

	void add_quad(const gl_texture *tex, const quad& verts, const quad& texcoords, float z);

private:
	struct sprite
	{
		float depth;
		const gl_texture *tex;
		quad verts;
		quad texcoords;
		blend_mode blend;
		rgba color;
	};

	void flush_queue();
	void render_sprites(const sprite *const *sprites, int num_sprites);
	void render_sprites(const gl_texture *tex, const sprite *const *sprites, int num_sprites);

	static const int SPRITE_QUEUE_CAPACITY = 1024;

	int sprite_queue_size_;
	sprite sprite_queue_[SPRITE_QUEUE_CAPACITY];

	blend_mode blend_mode_;
	rgba color_;
	mat3 matrix_;
	std::stack<mat3> matrix_stack_;
};

render_queue& render_queue::instance()
{
	static render_queue rq;
	return rq;
}

render_queue::render_queue()
{
}

void render_queue::begin_batch()
{
	sprite_queue_size_ = 0;
	blend_mode_ = blend_mode::NO_BLEND;
	color_ = rgba { 1, 1, 1, 1 };
	matrix_ = mat3::identity();
	matrix_stack_ = std::stack<mat3>();
}

void render_queue::end_batch()
{
	flush_queue();
}

void render_queue::push_matrix()
{
	matrix_stack_.push(matrix_);
}

void render_queue::pop_matrix()
{
	assert(!matrix_stack_.empty());
	matrix_ = matrix_stack_.top();
	matrix_stack_.pop();
}

void render_queue::translate(const vec2f& p)
{
	matrix_ *= mat3::translation(p);
}

void render_queue::scale(const vec2f& s)
{
	matrix_ *= mat3::scale(s);
}

void render_queue::rotate(float a)
{
	matrix_ *= mat3::rotation(a);
}

void render_queue::set_blend_mode(blend_mode mode)
{
	blend_mode_ = mode;
}

void render_queue::set_color(const rgba& color)
{
	color_ = color;
}

void render_queue::add_quad(const gl_texture *tex, const quad& verts, const quad& texcoords, float z)
{
	if (sprite_queue_size_ == SPRITE_QUEUE_CAPACITY)
		flush_queue();

	auto *p = &sprite_queue_[sprite_queue_size_++];

	p->tex = tex;

	p->verts.v00 = matrix_*verts.v00;
	p->verts.v01 = matrix_*verts.v01;
	p->verts.v10 = matrix_*verts.v10;
	p->verts.v11 = matrix_*verts.v11;

	p->texcoords = texcoords;

	p->depth = z;

	p->blend = blend_mode_;
	p->color = color_;
}

void render_queue::flush_queue()
{
	if (sprite_queue_size_ == 0)
		return;

	static const sprite *sorted_sprites[SPRITE_QUEUE_CAPACITY];

	for (int i = 0; i < sprite_queue_size_; i++)
		sorted_sprites[i] = &sprite_queue_[i];

	std::stable_sort(
		&sorted_sprites[0],
		&sorted_sprites[sprite_queue_size_],
		[](const sprite *s0, const sprite *s1)
		{
			if (s0->depth != s1->depth) {
				return s0->depth < s1->depth;
			} else if (s0->blend != s1->blend) {
				return static_cast<int>(s0->blend) < static_cast<int>(s1->blend);
			} else {
				return s0->tex < s1->tex;
			}
		});

	blend_mode cur_blend_mode = sorted_sprites[0]->blend;
	gl_set_blend_mode(cur_blend_mode);

	const gl_texture *cur_texture = sorted_sprites[0]->tex;

	int batch_start = 0;

	auto do_render = [&](int batch_end)
		{
			int num_sprites = batch_end - batch_start;

			if (num_sprites) {
				if (cur_texture == nullptr) {
					render_sprites(&sorted_sprites[batch_start], num_sprites);
				} else {
					render_sprites(cur_texture, &sorted_sprites[batch_start], num_sprites);
				}
			}
		};

	for (int i = 1; i < sprite_queue_size_; i++) {
		auto p = sorted_sprites[i];

		if (p->blend != cur_blend_mode || p->tex != cur_texture) {
			do_render(i);

			batch_start = i;

			if (p->blend != cur_blend_mode) {
				cur_blend_mode = p->blend;
				gl_set_blend_mode(cur_blend_mode);
			}

			cur_texture = p->tex;
		}
	}

	do_render(sprite_queue_size_);
}

void render_queue::render_sprites(const sprite *const *sprites, int num_sprites)
{
	static GLfloat data[SPRITE_QUEUE_CAPACITY*4*6];

	struct {
		GLfloat *dest;

		void operator()(const vec2f& vert, const rgba& color)
		{
			*dest++ = vert.x;
			*dest++ = vert.y;

			*dest++ = color.r;
			*dest++ = color.g;
			*dest++ = color.b;
			*dest++ = color.a;
		};
	} add_vertex { data };

	for (int i = 0; i < num_sprites; i++) {
		auto p = sprites[i];

		add_vertex(p->verts.v00, p->color);
		add_vertex(p->verts.v01, p->color);
		add_vertex(p->verts.v11, p->color);
		add_vertex(p->verts.v10, p->color);
	}

	glDisable(GL_TEXTURE_2D);

	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_COLOR_ARRAY);

	glVertexPointer(2, GL_FLOAT, 6*sizeof(GLfloat), &data[0]);
	glColorPointer(4, GL_FLOAT, 6*sizeof(GLfloat), &data[2]);

	glDrawArrays(GL_QUADS, 0, 4*num_sprites);

	glDisableClientState(GL_COLOR_ARRAY);
	glDisableClientState(GL_VERTEX_ARRAY);
}

void render_queue::render_sprites(const gl_texture *tex, const sprite *const *sprites, int num_sprites)
{
	static GLfloat data[SPRITE_QUEUE_CAPACITY*4*8];

	struct {
		GLfloat *dest;

		void operator()(const vec2f& vert, const vec2f& texuv, const rgba& color)
		{
			*dest++ = vert.x;
			*dest++ = vert.y;

			*dest++ = texuv.x;
			*dest++ = texuv.y;

			*dest++ = color.r;
			*dest++ = color.g;
			*dest++ = color.b;
			*dest++ = color.a;
		}
	} add_vertex { data };

	for (int i = 0; i < num_sprites; i++) {
		auto p = sprites[i];

		add_vertex(p->verts.v00, p->texcoords.v00, p->color);
		add_vertex(p->verts.v01, p->texcoords.v01, p->color);
		add_vertex(p->verts.v11, p->texcoords.v11, p->color);
		add_vertex(p->verts.v10, p->texcoords.v10, p->color);
	}

	glEnable(GL_TEXTURE_2D);
	tex->bind();

	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glEnableClientState(GL_COLOR_ARRAY);

	glVertexPointer(2, GL_FLOAT, 8*sizeof(GLfloat), &data[0]);
	glTexCoordPointer(2, GL_FLOAT, 8*sizeof(GLfloat), &data[2]);
	glColorPointer(4, GL_FLOAT, 8*sizeof(GLfloat), &data[4]);

	glDrawArrays(GL_QUADS, 0, 4*num_sprites);

	glDisableClientState(GL_COLOR_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	glDisableClientState(GL_VERTEX_ARRAY);
}

void begin_batch()
{
	render_queue::instance().begin_batch();
}

void end_batch()
{
	render_queue::instance().end_batch();
}

void push_matrix()
{
	render_queue::instance().push_matrix();
}

void pop_matrix()
{
	render_queue::instance().pop_matrix();
}

void translate(const vec2f& p)
{
	render_queue::instance().translate(p);
}

void translate(float x, float y)
{
	render_queue::instance().translate({ x, y });
}

void scale(const vec2f& s)
{
	render_queue::instance().scale(s);
}

void scale(float s)
{
	render_queue::instance().scale({ s, s });
}

void scale(float sx, float sy)
{
	render_queue::instance().scale({ sx, sy });
}

void rotate(float a)
{
	render_queue::instance().rotate(a);
}

void set_blend_mode(blend_mode mode)
{
	render_queue::instance().set_blend_mode(mode);
}

void set_color(const rgba& color)
{
	render_queue::instance().set_color(color);
}

void add_quad(const quad& verts, float z)
{
	render_queue::instance().add_quad(nullptr, verts, { { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 } }, z);
}

void add_quad(const gl_texture *tex, const quad& verts, const quad& texcoords, float z)
{
	render_queue::instance().add_quad(tex, verts, texcoords, z);
}

}
