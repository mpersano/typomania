#ifndef GL_TEXTURE_H_
#define GL_TEXTURE_H_

#include <GL/gl.h>

#include <string>

class gl_texture {
public:
	gl_texture();
	~gl_texture();

	bool load(const std::string& path);

	int get_image_width() const
	{ return image_width; }

	int get_image_height() const
	{ return image_height; }

	int get_texture_width() const
	{ return texture_width; }

	int get_texture_height() const
	{ return texture_height; }

	void bind() const;

private:
	int image_width, image_height;
	int texture_width, texture_height;

	GLuint texture_id;
};

#endif // GL_TEXTURE_H_
