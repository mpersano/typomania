#include <cstdio>
#include <cstring>
#include <cassert>
#include <cerrno>

#include <memory>

#include "panic.h"
#include "vector2.h"
#include "gl_vertex_array.h"
#include "common.h"
#include "font.h"

font::~font()
{
	for (glyph_cont::iterator i = glyph_map.begin(); i != glyph_map.end(); i++)
		delete i->second;
}

const font::glyph *
font::find_glyph(int code) const
{
	glyph_cont::const_iterator i = glyph_map.find(code);
	return i != glyph_map.end() ? i->second : 0;
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
font::get_integer_width(int n) const
{
	int width = 0;

	if (n == 0) {
		width = find_glyph('0')->advance_x;
	} else {
		while (n) {
			width += find_glyph('0' + (n%10))->advance_x;
			n /= 10;
		}
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

	texture = texture_cache[texture_path];

	while (fgets(line, sizeof(line), fp)) {
		int code;
		font::glyph *g = new font::glyph;

		if (sscanf(line, "%d %d %d %d %d %d %d %f %f %f %f %f %f %f %f",
		  &code, &g->width, &g->height, &g->left, &g->top, &g->advance_x, &g->advance_y,
		  &g->t0.x, &g->t0.y, &g->t1.x, &g->t1.y, &g->t2.x, &g->t2.y, &g->t3.x, &g->t3.y) != 15) {
			fclose(fp);
			return false;
		}

		glyph_map[code] = g;
	}

	fclose(fp);

	return true;
}
