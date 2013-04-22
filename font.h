#ifndef FONT_H_
#define FONT_H_

#include <string>
#include <map>

#include <GL/gl.h>

#include "vector2.h"
#include "gl_texture.h"

struct font {
	virtual ~font();

	bool load(const std::string& path);

	struct glyph {
		int width, height;
		int left, top;
		int advance_x, advance_y;
		vector2 t0, t1, t2, t3; // texture coordinates (0-1)
	};

	const glyph *find_glyph(int ch) const;
	int get_string_width(const wchar_t *str) const;
	int get_integer_width(int n) const;

	typedef std::map<int, glyph *> glyph_cont;
	glyph_cont glyph_map;

	gl_texture texture;
};

#endif /* FONT_H_ */
