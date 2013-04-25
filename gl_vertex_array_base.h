class NAME {
public:
	struct gl_vertex {
		GLfloat pos[2];
#ifdef WITH_TEXUV
		GLfloat texuv[2];
#endif
#ifdef WITH_COLOR
		GLubyte color[4];
#endif
	};

	NAME(int max_vertices)
	  : num_vertices(0)
	  , max_vertices(max_vertices)
	  , vertices(new gl_vertex[max_vertices])
	{ }

	void reset() { num_vertices = 0; }

	void add_vertex(
	  float x, float y
#ifdef WITH_TEXUV
	  , float u, float v
#endif
#ifdef WITH_COLOR
	  , int r, int g, int b, int a
#endif
	  );
	void draw(GLenum mode) const { draw(mode, 0, num_vertices); }
	void draw(GLenum mode, int num_vertices) const { draw(mode, 0, num_vertices); }
	void draw(GLenum mode, int first_vertex, int num_vertices) const;

#ifdef WITH_TEXUV
	void add_glyph(const font::glyph *gi,
#ifdef WITH_COLOR
	  int r, int g, int b, int a,
#endif
	  float x, float y);
#endif

#ifdef WITH_TEXUV
	float add_string(const font *fi, const wchar_t *str,
#ifdef WITH_COLOR
	  int r, int g, int b, int a,
#endif
	  float x, float y);
#endif

#ifdef WITH_TEXUV
	float add_stringn(const font *fi, const wchar_t *str, size_t len,
#ifdef WITH_COLOR
	  int r, int g, int b, int a,
#endif
	  float x, float y);
#endif

	int get_num_vertices() const
	{ return num_vertices; }

private:
  	NAME(const NAME&); // disable copy ctor
	NAME& operator=(const NAME&); // disable assignment

	int num_vertices;
	int max_vertices;
	struct gl_vertex *vertices;
};

#undef WITH_TEXUV
#undef WITH_COLOR
#undef NAME
