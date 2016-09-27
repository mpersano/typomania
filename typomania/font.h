#pragma once

#include <string>
#include <memory>
#include <unordered_map>

#include <GL/gl.h>

#include "vector2.h"
#include "gl_texture.h"

struct font
{
	bool load(const std::string& path);

	struct glyph {
		int width, height;
		int left, top;
		int advance_x, advance_y;
		vector2 t0, t1, t2, t3; // texture coordinates (0-1)
	};

	using glyph_ptr = std::unique_ptr<glyph>;

	const glyph *find_glyph(int ch) const;

	int get_string_width(const wchar_t *str) const;
	int get_string_width(const wchar_t *str, size_t len) const;

	std::unordered_map<int, glyph_ptr> glyph_map;

	gl_texture texture;
};
