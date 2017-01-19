#pragma once

#include <string>
#include <GL/gl.h>

#include <boost/noncopyable.hpp>

namespace gl {

class shader;

class program : private boost::noncopyable
{
public:
	program();

	bool load(const std::string& path);

	void use() const;

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

	uniform get_uniform(const std::string& name) const;

private:
	void attach(const shader& s);
	void link();

	GLuint id_;
};

} // gl
