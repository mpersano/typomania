#include <cassert>
#include <algorithm>
#include <stack>

#include <GL/glew.h>

#include "resources.h"
#include "mat3.h"
#include "gl_check.h"
#include "gl_program.h"
#include "gl_texture.h"
#include "render.h"

namespace {

void gl_set_blend_mode(blend_mode mode)
{
	switch (mode) {
		case blend_mode::NO_BLEND:
			GL_CHECK(glDisable(GL_BLEND));
			break;

		case blend_mode::ALPHA_BLEND:
			GL_CHECK(glEnable(GL_BLEND));
			GL_CHECK(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
			break;

		case blend_mode::ADDITIVE_BLEND:
			GL_CHECK(glEnable(GL_BLEND));
			GL_CHECK(glBlendFunc(GL_ONE, GL_ONE));
			break;
	}
}

}

namespace render {

class render_queue : private noncopyable
{
public:
	render_queue();

	void set_viewport(int width, int height);

	void begin_batch();
	void end_batch();

	void push_matrix();
	void pop_matrix();

	void translate(const vec2f& p);
	void scale(const vec2f& s);

	void rotate(float a);

	void set_blend_mode(blend_mode mode);
	void set_color(const rgba& color);

	void add_quad(const gl::texture *tex, const quad& verts, const quad& texcoords, int layer);

private:
	struct sprite
	{
		int layer;
		const gl::texture *tex;
		quad verts;
		quad texcoords;
		blend_mode blend;
		rgba color;
	};

	void init_programs();

	void flush_queue();
	void render_sprites(const sprite *const *sprites, int num_sprites);
	void render_sprites(const gl::texture *tex, const sprite *const *sprites, int num_sprites);

	static const int SPRITE_QUEUE_CAPACITY = 1024;

	int sprite_queue_size_;
	sprite sprite_queue_[SPRITE_QUEUE_CAPACITY];

	blend_mode blend_mode_;
	rgba color_;
	mat3 matrix_;
	std::stack<mat3> matrix_stack_;

	const gl::program *prog_flat_;
	const gl::program *prog_texture_;

	std::array<GLfloat, 16> proj_matrix_;
} *g_render_queue;

render_queue::render_queue()
{
	init_programs();
}

void render_queue::init_programs()
{
	prog_flat_ = get_program("data/shaders/flat.prog");
	prog_texture_ = get_program("data/shaders/sprite.prog");
}

void render_queue::set_viewport(int width, int height)
{
	const float a = 2.f/width;
	const float b = 2.f/height;

	proj_matrix_ = { a, 0, 0, -1,
			 0, b, 0, -1,
			 0, 0, 0,  0,
			 0, 0, 0,  1 };

	prog_flat_->use();
	prog_flat_->get_uniform("proj_modelview").set_mat4(&proj_matrix_[0]);

	prog_texture_->use();
	prog_texture_->get_uniform("proj_modelview").set_mat4(&proj_matrix_[0]);
}

void render_queue::begin_batch()
{
	sprite_queue_size_ = 0;
	blend_mode_ = blend_mode::NO_BLEND;
	color_ = { 1, 1, 1, 1 };
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

void render_queue::add_quad(const gl::texture *tex, const quad& verts, const quad& texcoords, int layer)
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

	p->layer = layer;

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
			if (s0->layer != s1->layer) {
				return s0->layer < s1->layer;
			} else if (s0->blend != s1->blend) {
				return static_cast<int>(s0->blend) < static_cast<int>(s1->blend);
			} else {
				return s0->tex < s1->tex;
			}
		});

	blend_mode cur_blend_mode = sorted_sprites[0]->blend;
	gl_set_blend_mode(cur_blend_mode);

	const gl::texture *cur_texture = sorted_sprites[0]->tex;

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

	sprite_queue_size_ = 0;
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

	prog_flat_->use();

	GL_CHECK(glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 6*sizeof(GLfloat), &data[0]));
	GL_CHECK(glEnableVertexAttribArray(0));

	GL_CHECK(glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 6*sizeof(GLfloat), &data[2]));
	GL_CHECK(glEnableVertexAttribArray(1));

	GL_CHECK(glDrawArrays(GL_QUADS, 0, 4*num_sprites));

	GL_CHECK(glDisableVertexAttribArray(1));
	GL_CHECK(glDisableVertexAttribArray(0));
}

void render_queue::render_sprites(const gl::texture *tex, const sprite *const *sprites, int num_sprites)
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

	tex->bind();

	prog_texture_->use();

	GL_CHECK(glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 8*sizeof(GLfloat), &data[0]));
	GL_CHECK(glEnableVertexAttribArray(0));

	GL_CHECK(glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 8*sizeof(GLfloat), &data[2]));
	GL_CHECK(glEnableVertexAttribArray(1));

	GL_CHECK(glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, 8*sizeof(GLfloat), &data[4]));
	GL_CHECK(glEnableVertexAttribArray(2));

	GL_CHECK(glDrawArrays(GL_QUADS, 0, 4*num_sprites));

	GL_CHECK(glDisableVertexAttribArray(2));
	GL_CHECK(glDisableVertexAttribArray(1));
	GL_CHECK(glDisableVertexAttribArray(0));
}

void init()
{
	g_render_queue = new render_queue;
}

void set_viewport(int width, int height)
{
	g_render_queue->set_viewport(width, height);
}

void begin_batch()
{
	g_render_queue->begin_batch();
}

void end_batch()
{
	g_render_queue->end_batch();
}

void push_matrix()
{
	g_render_queue->push_matrix();
}

void pop_matrix()
{
	g_render_queue->pop_matrix();
}

void translate(const vec2f& p)
{
	g_render_queue->translate(p);
}

void translate(float x, float y)
{
	g_render_queue->translate({ x, y });
}

void scale(const vec2f& s)
{
	g_render_queue->scale(s);
}

void scale(float s)
{
	g_render_queue->scale({ s, s });
}

void scale(float sx, float sy)
{
	g_render_queue->scale({ sx, sy });
}

void rotate(float a)
{
	g_render_queue->rotate(a);
}

void set_blend_mode(blend_mode mode)
{
	g_render_queue->set_blend_mode(mode);
}

void set_color(const rgba& color)
{
	g_render_queue->set_color(color);
}

void draw_quad(const quad& verts, int layer)
{
	g_render_queue->add_quad(nullptr, verts, { { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 } }, layer);
}

void draw_quad(const gl::texture *tex, const quad& verts, const quad& texcoords, int layer)
{
	g_render_queue->add_quad(tex, verts, texcoords, layer);
}

void draw_quad(const gl::texture *tex, const quad& verts, int layer)
{
	const float u = static_cast<float>(tex->get_image_width())/tex->get_texture_width();
	const float v = static_cast<float>(tex->get_image_height())/tex->get_texture_height();

	g_render_queue->add_quad(tex, verts, { { 0, v }, { 0, 0 }, { u, v }, { u, 0 } }, layer);
}

void draw_quad(const gl::texture *tex, const vec2f& pos, int layer)
{
	const int w = tex->get_image_width();
	const int h = tex->get_image_height();

	draw_quad(tex, { { pos.x, pos.y }, { pos.x, pos.y + h }, { pos.x + w, pos.y }, { pos.x + w, pos.y + h } }, layer);
}

}
