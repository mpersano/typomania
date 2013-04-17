#include <memory>

#include "image.h"
#include "gl_texture.h"

gl_texture::gl_texture()
{
	glGenTextures(1, &texture_id);
}

gl_texture::~gl_texture()
{
	glDeleteTextures(1, &texture_id);
}

bool
gl_texture::load(const char *path)
{
	image img;

	if (!img.load(path))
		return false;

	width = img.get_width();
	height = img.get_height();

	bind();

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, img.get_bits());

	return true;
}

void
gl_texture::bind()
{
	glBindTexture(GL_TEXTURE_2D, texture_id);
}
