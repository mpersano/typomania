#include <memory>

#include "image.h"
#include "gl_texture.h"

static int
next_power_of_2(int n)
{
	int p = 1;

	while (p < n)
		p *= 2;

	return p;
}

gl_texture::gl_texture()
{
	glGenTextures(1, &texture_id);
}

gl_texture::~gl_texture()
{
	glDeleteTextures(1, &texture_id);
}

bool
gl_texture::load(const std::string& path)
{
	image img;

	if (!img.load(path))
		return false;

	image_width = img.get_width();
	image_height = img.get_height();

	texture_width = next_power_of_2(image_width);
	texture_height = next_power_of_2(image_height);

	img.resize(texture_width, texture_height);

	bind();

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texture_width, texture_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, img.get_bits());

	return true;
}

void
gl_texture::bind()
{
	glBindTexture(GL_TEXTURE_2D, texture_id);
}
