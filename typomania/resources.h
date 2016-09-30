#pragma once

#include <string>

struct font;
const font *get_font(const std::string& path);

struct gl_texture;
const gl_texture *get_texture(const std::string& path);
