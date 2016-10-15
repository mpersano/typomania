#pragma once

#include <GL/gl.h>
#include <string>

#include "noncopyable.h"

namespace gl {

class texture : private noncopyable
{
public:
	texture();
	~texture();

	bool load(const std::string& path);

	int get_image_width() const
	{ return image_width_; }

	int get_image_height() const
	{ return image_height_; }

	int get_texture_width() const
	{ return texture_width_; }

	int get_texture_height() const
	{ return texture_height_; }

	void bind() const;

private:
	int image_width_, image_height_;
	int texture_width_, texture_height_;
	GLuint id_;
};

} // gl
