#ifndef IMAGE_H_
#define IMAGE_H_

#include <vector>

class image {
public:
	static image *load_from_png(const char *path);

	int get_width() const
	{ return width; }

	int get_height() const
	{ return height; }

	const unsigned *get_bits() const
	{ return &bits[0]; }

private:
	image(int width, int height)
	: width(width), height(height), bits(width*height)
	{ }

	int width, height;
	std::vector<unsigned> bits;

	image(const image&);
	image& operator=(const image&);
};

#endif // IMAGE_H_
