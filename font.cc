#include <cstdio>
#include <cstring>
#include <cassert>
#include <cerrno>

#include <memory>

#include "panic.h"
#include "vector2.h"
#include "gl_util.h"
#include "font.h"

font::~font()
{
	std::map<int, glyph *>::iterator it;

	for (it = glyph_map.begin(); it != glyph_map.end(); it++)
		delete it->second;
}

const font::glyph *
font::find_glyph(int code) const
{
	std::map<int, glyph *>::const_iterator it = glyph_map.find(code);

	if (it == glyph_map.end())
		panic("glyph %d not found", code);

	return it->second;
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

font *
font::load(const char *texture_path, const char *font_path)
{
	font *p = new font;

	if (!p->texture.load(texture_path))
		panic("failed to load texture");

	FILE *fp;

	if ((fp = fopen(font_path, "rb")) == 0)
		panic("failed to open font: %s", strerror(errno));

	char line[512];

	while (fgets(line, sizeof(line), fp)) {
		int code;
		font::glyph *g = new font::glyph;

		if (sscanf(line, "%d %d %d %d %d %d %d %f %f %f %f %f %f %f %f",
		  &code, &g->width, &g->height, &g->left, &g->top, &g->advance_x, &g->advance_y,
		  &g->t0.x, &g->t0.y, &g->t1.x, &g->t1.y, &g->t2.x, &g->t2.y, &g->t3.x, &g->t3.y) != 15)
			panic("borked font file?");

		p->glyph_map[code] = g;
	}

	fclose(fp);

	return p;
}
