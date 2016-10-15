#include <memory>

#include "image.h"
#include "gl_check.h"
#include "gl_texture.h"

namespace {

int
next_power_of_2(int n)
{
	int p = 1;

	while (p < n)
		p *= 2;

	return p;
}

}

namespace gl {

texture::texture()
{
	GL_CHECK(glGenTextures(1, &id_));
}

texture::~texture()
{
	GL_CHECK(glDeleteTextures(1, &id_));
}

bool
texture::load(const std::string& path)
{
	image img;

	if (!img.load(path))
		return false;

	image_width_ = img.get_width();
	image_height_ = img.get_height();

	texture_width_ = next_power_of_2(image_width_);
	texture_height_ = next_power_of_2(image_height_);

	img.resize(texture_width_, texture_height_);

	bind();

	GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP));
	GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP));
	GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
	GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
	GL_CHECK(glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE));

	GL_CHECK(glPixelStorei(GL_UNPACK_ALIGNMENT, 1));
	GL_CHECK(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texture_width_, texture_height_, 0, GL_RGBA, GL_UNSIGNED_BYTE, img.get_bits()));

	return true;
}

void
texture::bind() const
{
	GL_CHECK(glBindTexture(GL_TEXTURE_2D, id_));
}

}
