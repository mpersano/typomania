#ifndef GL_TEXTURE_H_
#define GL_TEXTURE_H_

#include <GL/gl.h>

class gl_texture {
public:
	gl_texture();
	~gl_texture();

	bool load(const char *path);

	int get_width() const
	{ return width; }

	int get_height() const
	{ return height; }

	void bind();

private:
	int width, height;

	GLuint texture_id;
};

#endif // GL_TEXTURE_H_
