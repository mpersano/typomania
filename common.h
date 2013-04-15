#ifndef COMMON_H_
#define COMMON_H_

enum {
	TICS_PER_SECOND = 30,
	WINDOW_WIDTH = 800,
	WINDOW_HEIGHT = 400,
};

struct font;
extern font *small_font, *tiny_font;

#endif // COMMON_H_
