#ifndef COMMON_H_
#define COMMON_H_

enum {
	TICS_PER_SECOND = 60,

	WINDOW_WIDTH = 800,
	WINDOW_HEIGHT = 400,
};

struct gl_texture;
struct font;

#include "resource_cache.h"
extern resource_cache<gl_texture> texture_cache;
extern resource_cache<font> font_cache;

#include "game.h"
extern game_ptr the_game;

#endif // COMMON_H_
