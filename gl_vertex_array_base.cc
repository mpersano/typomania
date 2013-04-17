void
NAME::add_vertex(
	float x, float y
#ifdef WITH_TEXUV
	, float u, float v
#endif
#ifdef WITH_COLOR
	, int r, int g, int b, int a
#endif
	)
{
	assert(num_vertices < max_vertices);

	gl_vertex *p = &vertices[num_vertices++];

	p->pos[0] = x;
	p->pos[1] = y;
#ifdef WITH_TEXUV
	p->texuv[0] = u;
	p->texuv[1] = v;
#endif
#ifdef WITH_COLOR
	p->color[0] = r;
	p->color[1] = g;
	p->color[2] = b;
	p->color[3] = a;
#endif
}

void
NAME::draw(GLenum mode, int first_vertex, int num_vertices) const
{
	if (num_vertices > 0) {
		glEnableClientState(GL_VERTEX_ARRAY);
#ifdef WITH_TEXUV
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);
#endif
#ifdef WITH_COLOR
		glEnableClientState(GL_COLOR_ARRAY);
#endif

		glVertexPointer(2, GL_FLOAT, sizeof(gl_vertex), &vertices[first_vertex].pos);
#ifdef WITH_TEXUV
		glTexCoordPointer(2, GL_FLOAT, sizeof(gl_vertex), &vertices[first_vertex].texuv);
#endif
#ifdef WITH_COLOR
		glColorPointer(4, GL_UNSIGNED_BYTE, sizeof(gl_vertex), &vertices[first_vertex].color);
#endif
		glDrawArrays(mode, 0, num_vertices);

#ifdef WITH_COLOR
		glDisableClientState(GL_COLOR_ARRAY);
#endif
#ifdef WITH_TEXUV
		glDisableClientState(GL_TEXTURE_COORD_ARRAY);
#endif
		glDisableClientState(GL_VERTEX_ARRAY);
	}
}

#ifdef WITH_TEXUV
void
NAME::add_glyph(const font::glyph *gi,
#ifdef WITH_COLOR
		int r, int g, int b, int a,
#endif
		float x, float y)
{
	float x_left = x + gi->left;
	float x_right = x + gi->left + gi->width;

	float y_top = y + gi->top;
	float y_bottom = y + gi->top - gi->height;

	const vector2& t0 = gi->t0;
	const vector2& t1 = gi->t1;
	const vector2& t2 = gi->t2;
	const vector2& t3 = gi->t3;

	add_vertex(x_left, y_top, t0.x, t0.y
#ifdef WITH_COLOR
		, r, g, b, a
#endif
	);
	add_vertex(x_right, y_top, t1.x, t1.y
#ifdef WITH_COLOR
		, r, g, b, a
#endif
	);
	add_vertex(x_right, y_bottom, t2.x, t2.y
#ifdef WITH_COLOR
		, r, g, b, a
#endif
	);
	add_vertex(x_left, y_bottom, t3.x, t3.y
#ifdef WITH_COLOR
		, r, g, b, a
#endif
	);
}
#endif

#ifdef WITH_TEXUV
void
NAME::add_string(const font *fi, const wchar_t *str,
#ifdef WITH_COLOR
		int r, int g, int b, int a,
#endif
		float x, float y)
{
	for (const wchar_t *p = str; *p; p++) {
		const font::glyph *gi = fi->find_glyph(*p);
		add_glyph(gi,
#ifdef WITH_COLOR
		  r, g, b, a, 
#endif 
		  x, y);
		x += gi->advance_x;
	}
}
#endif

#ifdef WITH_TEXUV
void
NAME::add_string_centered(const font *fi, const wchar_t *str,
#ifdef WITH_COLOR
		int r, int g, int b, int a,
#endif
		float x, float y)
{
	add_string(fi, str,
#ifdef WITH_COLOR
	  r, g, b, a,
#endif
	  x - .5*fi->get_string_width(str), y);
}
#endif

#ifdef WITH_TEXUV
int
NAME::add_integer(const font *fi, int n,
#ifdef WITH_COLOR
		int r, int g, int b, int a,
#endif
		float x, float y)
{
	int width = 0;

	if (n > 0) {
		const font::glyph *gi = fi->find_glyph((n%10) + '0');

		width = add_integer(fi, n/10,
#ifdef WITH_COLOR
		  r, g, b, a, 
#endif 
		  x, y);

		add_glyph(gi,
#ifdef WITH_COLOR
		  r, g, b, a, 
#endif 
		  x + width, y);

		width += gi->advance_x;
	}

	return width;
}
#endif

#undef WITH_TEXUV
#undef WITH_COLOR
#undef NAME
