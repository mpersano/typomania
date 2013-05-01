#ifndef GLYPH_FX_H_
#define GLYPH_FX_H_

#include "vector2.h"
#include "font.h"

class glyph_fx {
public:
	glyph_fx(const font *f, wchar_t ch, const vector2& p);

	void update()
	{ ++tics; }

	bool is_active() const
	{ return tics < TTL; }

	void draw() const;

private:
	enum { TTL = 30 };

	const font *f;
	const font::glyph *gi;
	float x, y;

	int tics;
};

#endif // GLYPH_FX_H_
