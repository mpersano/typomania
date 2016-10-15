#pragma once

#include <string>

class font;
const font *get_font(const std::string& path);

namespace gl { class texture; }
const gl::texture *get_texture(const std::string& path);

namespace gl { class program; }
const gl::program *get_program(const std::string& path);
