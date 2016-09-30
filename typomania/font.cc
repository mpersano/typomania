#include <cstdio>
#include <cstring>
#include <cassert>
#include <cerrno>

#include <memory>

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
	FILE *fp;

	if ((fp = fopen(path.c_str(), "rb")) == 0)
		return false;

	char line[512], texture_path[512];

	if (!fgets(line, sizeof(line), fp) || sscanf(line, "%s\n", texture_path) != 1) {
		fclose(fp);
		return false;
	}

	if (!texture_.load(texture_path)) {
		fclose(fp);
		return false;
	}

	while (fgets(line, sizeof(line), fp)) {
		int code;

		glyph_ptr g(new glyph);

		if (sscanf(line, "%d %d %d %d %d %d %d %f %f %f %f %f %f %f %f",
		  &code, &g->width, &g->height, &g->left, &g->top, &g->advance_x, &g->advance_y,
		  &g->t0.x, &g->t0.y, &g->t1.x, &g->t1.y, &g->t2.x, &g->t2.y, &g->t3.x, &g->t3.y) != 15) {
			fclose(fp);
			return false;
		}

		glyph_map[code] = std::move(g);
	}

	fclose(fp);

	return true;
}

float
font::draw_string(const wchar_t *str, float x, float y, float z) const
{
	for (const wchar_t *p = str; *p; ++p) {
		const glyph *gi = find_glyph(*p);
		draw_glyph(gi, x, y, z);
		x += gi->advance_x;
	}

	return x;
}

float
font::draw_stringn(const wchar_t *str, size_t len, float x, float y, float z) const
{
	for (size_t i = 0; i < len; i++) {
		const glyph *gi = find_glyph(str[i]);
		draw_glyph(gi, x, y, z);
		x += gi->advance_x;
	}

	return x;
}

void
font::draw_glyph(wchar_t ch, float x, float y, float z) const
{
	draw_glyph(find_glyph(ch), x, y, z);
}

void
font::draw_glyph(const glyph *gi, float x, float y, float z) const
{
	float x_left = x + gi->left;
	float x_right = x + gi->left + gi->width;

	float y_top = y + gi->top;
	float y_bottom = y + gi->top - gi->height;

	render::add_quad(
		&texture_,
		{ { x_left, y_top }, { x_right, y_top }, { x_left, y_bottom }, { x_right, y_bottom }  },
		{ gi->t0, gi->t1, gi->t3, gi->t2 },
		z);
}
