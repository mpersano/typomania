#pragma once

#include "gl_texture.h"

namespace gl {

class framebuffer
{
public:
	framebuffer(int width, int height);
	~framebuffer();

	void bind() const;
	static void unbind();

	const texture *get_texture() const;

private:
	void init_texture();

	GLuint fbo_id_;
	texture texture_;
};

}
