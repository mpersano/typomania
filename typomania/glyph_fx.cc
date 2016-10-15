#include <cmath>

#include <SDL.h>

#include "render.h"
#include "glyph_fx.h"

glyph_fx::glyph_fx(const font *f, wchar_t ch, const vec2f& p)
: texture_(f->get_texture())
, gi_(f->find_glyph(ch))
, x_(p.x + gi_->left + .5*gi_->width)
, y_(p.y + gi_->top - .5*gi_->height)
, tics_(0)
{
}

void
glyph_fx::draw() const
{
	const vec2f& t0 = gi_->t0;
	const vec2f& t1 = gi_->t1;
	const vec2f& t2 = gi_->t2;
	const vec2f& t3 = gi_->t3;

	const float t = static_cast<float>(tics_)/TTL;
	const float s = sinf(t*M_PI);

	static const int NUM_LAYERS = 8;

	for (int i = 0; i < NUM_LAYERS; i++) {
		const float q = 1. - static_cast<float>(i)/NUM_LAYERS;
		const float c = q*q*(1. - t)*(1. - t);

		const float f = 1 + .2*s*i;

		render::set_color({ c, c, c, 1 });

		const float xo = f*.5*gi_->width;
		const float yo = f*.5*gi_->height;

		render::draw_quad(
			texture_,
			{ { x_ - xo, y_ + yo }, { x_ - xo, y_ - yo }, { x_ + xo, y_ + yo }, { x_ + xo, y_ - yo } },
			{ t0, t3, t1, t2 },
			10);
	}
}
