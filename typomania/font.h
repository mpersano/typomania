#pragma once

#include <string>
#include <memory>
#include <unordered_map>

#include <GL/gl.h>

#include "vec2.h"
#include "gl_texture.h"

class font
{
public:
	bool load(const std::string& path);

	struct glyph {
		int width, height;
		int left, top;
		int advance_x, advance_y;
		vec2f t0, t1, t2, t3; // texture coordinates (0-1)
	};

	const glyph *find_glyph(int ch) const;

	int get_string_width(const wchar_t *str) const;
	int get_string_width(const wchar_t *str, size_t len) const;

	float draw_string(const wchar_t *str, float x, float y, float z) const;
	float draw_stringn(const wchar_t *str, size_t len, float x, float y, float z) const;

	void draw_glyph(wchar_t ch, float x, float y, float z) const;

	const gl_texture *get_texture() const
	{ return &texture_; }

private:
	void draw_glyph(const glyph *gi, float x, float y, float z) const;

	using glyph_ptr = std::unique_ptr<glyph>;
	std::unordered_map<int, glyph_ptr> glyph_map;

	gl_texture texture_;
};
