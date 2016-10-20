#include <memory>

#include "image.h"
#include "gl_check.h"
#include "gl_texture.h"

namespace {

template <typename T>
static T
next_power_of_2(T n)
{
	--n;
	n |= n >> 1;
	n |= n >> 2;
	n |= n >> 4;
	n |= n >> 8;
	n |= n >> 16;
	return n + 1;
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

void
texture::allocate(int width, int height)
{
	image_width_ = texture_width_ = width;
	image_height_ = texture_height_ = height;

	initialize(nullptr);
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

	initialize(img.get_bits());

	return true;
}

void
texture::bind() const
{
	GL_CHECK(glBindTexture(GL_TEXTURE_2D, id_));
}

void
texture::initialize(const GLvoid *data)
{
	bind();

	GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP));
	GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP));
	GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
	GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
	GL_CHECK(glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE));

	GL_CHECK(glPixelStorei(GL_UNPACK_ALIGNMENT, 1));
	GL_CHECK(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texture_width_, texture_height_, 0, GL_RGBA, GL_UNSIGNED_BYTE, data));
}

}
