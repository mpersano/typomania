#include <cstdio>
#include <cerrno>

#include <png.h>

#include "panic.h"
#include "image.h"

bool
image::load(const std::string& path)
{
	FILE *fp;

	if ((fp = fopen(path.c_str(), "rb")) == 0)
		panic("failed to open `%s': %s", path.c_str(), strerror(errno));

	png_structp png_ptr;

	if ((png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, 0, 0, 0)) == 0)
		panic("png_create_read_struct failed");

	png_infop info_ptr;

	if ((info_ptr = png_create_info_struct(png_ptr)) == 0)
		panic("png_create_info_struct failed");

	if (setjmp(png_jmpbuf(png_ptr)))
		panic("png error");

	png_init_io(png_ptr, fp);

	png_read_png(png_ptr, info_ptr, PNG_TRANSFORM_IDENTITY, 0);

	int color_type = png_get_color_type(png_ptr, info_ptr);
	int bit_depth = png_get_bit_depth(png_ptr, info_ptr);

	if (color_type != PNG_COLOR_TYPE_RGBA || bit_depth != 8)
		panic("invalid color type or bit depth in PNG");

	width = png_get_image_width(png_ptr, info_ptr);
	height = png_get_image_height(png_ptr, info_ptr);

	bits.resize(width*height);

	png_bytep *rows = png_get_rows(png_ptr, info_ptr);

	for (int i = 0; i < height; i++)
		memcpy(&bits[i*width], rows[i], width*sizeof(unsigned));

	png_destroy_read_struct(&png_ptr, &info_ptr, 0);

	fclose(fp);

	return true;
}

void
image::resize(int new_width, int new_height)
{
	std::vector<unsigned> new_bits(new_width*new_height, 0);

	for (int i = 0; i < std::min(height, new_height); i++) {
		unsigned *dest = &new_bits[i*new_width];
		unsigned *src = &bits[i*width];
		::memcpy(dest, src, std::min(width, new_width)*sizeof(unsigned));
	}

	width = new_width;
	height = new_height;
	bits = new_bits;
}
