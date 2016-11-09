#include <fstream>
#include <sstream>
#include <memory>

#include "resources.h"
#include "panic.h"
#include "render.h"
#include "font.h"

const font::glyph *
font::find_glyph(int code) const
{
	auto i = glyph_map.find(code);

	if (i == glyph_map.end())
		panic("glyph %d not found\n", code);

	return i != glyph_map.end() ? i->second.get() : nullptr;
}

int
font::get_string_width(const wchar_t *str) const
{
	int width = 0;

	for (const wchar_t *p = str; *p; p++) {
		const glyph *g = find_glyph(*p);
		width += g->advance_x;
	}

	return width;
}

int
font::get_string_width(const wchar_t *str, size_t len) const
{
	int width = 0;

	for (size_t i = 0; i < len; i++) {
		const glyph *g = find_glyph(str[i]);
		width += g->advance_x;
	}

	return width;
}

bool
font::load(const std::string& path)
{
	std::ifstream file(path);
	if (!file)
		return false;

	std::string texture_path;
	if (!std::getline(file, texture_path))
		return false;

	texture_ = ::get_texture(texture_path);

	std::string line;

	while (std::getline(file, line)) {
		std::istringstream ss(line);

		int code;
		ss >> code;

		glyph_ptr g(new glyph);

		ss >> g->width >> g->height;
		ss >> g->left >> g->top;
		ss >> g->advance_x >> g->advance_y;
		ss >> g->t0.x >> g->t0.y;
		ss >> g->t1.x >> g->t1.y;
		ss >> g->t2.x >> g->t2.y;
		ss >> g->t3.x >> g->t3.y;

		glyph_map[code] = std::move(g);
	}

	return true;
}

float
font::draw_string(const wchar_t *str, float x, float y, int layer) const
{
	for (const wchar_t *p = str; *p; ++p) {
		const glyph *gi = find_glyph(*p);
		draw_glyph(gi, x, y, layer);
		x += gi->advance_x;
	}

	return x;
}

float
font::draw_stringn(const wchar_t *str, size_t len, float x, float y, int layer) const
{
	for (size_t i = 0; i < len; i++) {
		const glyph *gi = find_glyph(str[i]);
		draw_glyph(gi, x, y, layer);
		x += gi->advance_x;
	}

	return x;
}

void
font::draw_glyph(wchar_t ch, float x, float y, int layer) const
{
	draw_glyph(find_glyph(ch), x, y, layer);
}

void
font::draw_glyph(const glyph *gi, float x, float y, int layer) const
{
	float x_left = x + gi->left;
	float x_right = x + gi->left + gi->width;

	float y_top = y + gi->top;
	float y_bottom = y + gi->top - gi->height;

	render::draw_quad(
		texture_,
		{ { x_left, y_top }, { x_right, y_top }, { x_left, y_bottom }, { x_right, y_bottom }  },
		{ gi->t0, gi->t1, gi->t3, gi->t2 },
		layer);
}
