#ifndef COMMON_H_
#define COMMON_H_

enum {
	TICS_PER_SECOND = 30,

	WINDOW_WIDTH = 800,
	WINDOW_HEIGHT = 400,
};

#include "resource_cache.h"

struct gl_texture;
struct font;

extern resource_cache<gl_texture> texture_cache;
extern resource_cache<font> font_cache;

#endif // COMMON_H_
