#include <GL/glew.h>

#include "gl_check.h"
#include "gl_framebuffer.h"

namespace gl {

framebuffer::framebuffer(int width, int height)
{
	// initialize texture

	texture_.allocate(width, height);

	// initialize framebuffer

	GL_CHECK(glGenFramebuffers(1, &fbo_id_));

	bind();
	GL_CHECK(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture_.id_, 0));
	unbind();
}

framebuffer::~framebuffer()
{
	GL_CHECK(glDeleteFramebuffers(1, &fbo_id_));
}

void
framebuffer::bind() const
{
	GL_CHECK(glViewport(0, 0, texture_.texture_width_, texture_.texture_height_));
	GL_CHECK(glBindFramebuffer(GL_FRAMEBUFFER, fbo_id_));
}

void
framebuffer::unbind()
{
	GL_CHECK(glBindFramebuffer(GL_FRAMEBUFFER, 0));
}

const texture *
framebuffer::get_texture() const
{
	return &texture_;
}

}
