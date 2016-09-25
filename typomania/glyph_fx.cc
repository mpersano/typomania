#include <cmath>

#include <SDL.h>
#include <GL/gl.h>

#include "gl_vertex_array.h"
#include "glyph_fx.h"

glyph_fx::glyph_fx(const font *f, wchar_t ch, const vector2& p)
: f(f)
, gi(f->find_glyph(ch))
, x(p.x + gi->left + .5*gi->width)
, y(p.y + gi->top - .5*gi->height)
, tics(0)
{ }

void
glyph_fx::draw() const
{
	const vector2& t0 = gi->t0;
	const vector2& t1 = gi->t1;
	const vector2& t2 = gi->t2;
	const vector2& t3 = gi->t3;

	const float t = static_cast<float>(tics)/TTL;
	const float s = sinf(t*M_PI);

	enum { NUM_LAYERS = 8, };

	static gl_vertex_array_texuv_color gv(4*NUM_LAYERS);
	gv.reset();

	for (int i = 0; i < NUM_LAYERS; i++) {
		const float q = 1. - static_cast<float>(i)/NUM_LAYERS;
		const int c = 255*q*q*(1. - t)*(1. - t);

		const float f = 1 + .2*s*i;

#define ADD_VERTEX(dx, dy, n) \
		gv.add_vertex(x + dx*f*.5*gi->width, y + dy*f*.5*gi->height, t ## n.x, t ## n.y, c, c, c, 255);
		ADD_VERTEX(-1, +1, 0)
		ADD_VERTEX(+1, +1, 1)
		ADD_VERTEX(+1, -1, 2)
		ADD_VERTEX(-1, -1, 3)
#undef ADD_VERTEX
	}

	f->texture.bind();
	gv.draw(GL_QUADS);
}
