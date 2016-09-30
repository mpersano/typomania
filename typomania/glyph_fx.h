#ifndef GLYPH_FX_H_
#define GLYPH_FX_H_

#include "vec2.h"
#include "font.h"

class glyph_fx {
public:
	glyph_fx(const font *f, wchar_t ch, const vec2f& p);

	void update()
	{ ++tics_; }

	bool is_active() const
	{ return tics_ < TTL; }

	void draw() const;

private:
	enum { TTL = 30 };

	const gl_texture *texture_;
	const font::glyph *gi_;
	float x_, y_;

	int tics_;
};

#endif // GLYPH_FX_H_
