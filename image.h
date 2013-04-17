#ifndef IMAGE_H_
#define IMAGE_H_

#include <vector>

class image {
public:
	image()
	: width(0), height(0)
	{ }

	image(int width, int height)
	: width(width), height(height), bits(width*height)
	{ }

	bool load(const char *path);

	int get_width() const
	{ return width; }

	int get_height() const
	{ return height; }

	const unsigned *get_bits() const
	{ return &bits[0]; }

private:
	int width, height;
	std::vector<unsigned> bits;

	image(const image&); // disable copy ctor
	image& operator=(const image&); // disable assignment
};

#endif // IMAGE_H_
