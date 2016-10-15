#pragma once

#include <vector>
#include <string>

#include "noncopyable.h"

class image : private noncopyable
{
public:
	image()
	: width_(0), height_(0)
	{ }

	image(int width, int height)
	: width_(width), height_(height), bits_(width*height)
	{ }

	bool load(const std::string& path);

	int get_width() const
	{ return width_; }

	int get_height() const
	{ return height_; }

	const unsigned *get_bits() const
	{ return &bits_[0]; }

	void resize(int new_width, int new_height);

private:
	int width_, height_;
	std::vector<unsigned> bits_;
};
