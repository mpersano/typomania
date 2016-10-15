#pragma once

#include <string>
#include <GL/gl.h>

#include "noncopyable.h"

namespace gl {

class shader : private noncopyable
{
public:
	shader(GLenum type);

	void set_source(const std::string& source);
	void load_source(const std::string& filename);

private:
	friend class program;
	GLuint id_;
};

class program : private noncopyable
{
public:
	program();

	void attach(const shader& s);
	void link();

	void use();

	class uniform
	{
	public:
		uniform(GLint location);

		void set_f(GLfloat v0);
		void set_f(GLfloat v0, GLfloat v1);
		void set_f(GLfloat v0, GLfloat v1, GLfloat v2);
		void set_f(GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3);

		void set_i(GLint v0);
		void set_i(GLint v0, GLint v1);
		void set_i(GLint v0, GLint v1, GLint v2);
		void set_i(GLint v0, GLint v1, GLint v2, GLint v3);

		void set_mat4(const GLfloat *mat);

	private:
		GLint location_;
	};

	uniform get_uniform(const std::string& name);

private:
	GLuint id_;
};

} // gl
