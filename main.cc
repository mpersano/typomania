#include <cstdio>
#include <cmath>
#include <cassert>
#include <cstring>
#include <cerrno>

#include <sstream>
#include <vector>

#include <SDL.h>
#include <GL/gl.h>

#include "panic.h"
#include "fft.h"
#include "ogg_player.h"
#include "kashi.h"
#include "gl_util.h"
#include "font.h"

#include "common.h"
#include "game.h"

static const char *IMAGE_DIR = "data/images";
static const char *FONT_DIR = "data/fonts";

static ALCdevice *al_device;
static ALCcontext *al_context;

static bool running;

font *small_font, *tiny_font;

static std::auto_ptr<game> the_game;

static void
init_sdl()
{
	if (SDL_Init(SDL_INIT_VIDEO) < 0)
		panic("SDL_Init: %s", SDL_GetError());

	if (SDL_SetVideoMode(WINDOW_WIDTH, WINDOW_HEIGHT, 0, SDL_OPENGL) == 0)
		panic("SDL_SetVideoMode: %s", SDL_GetError());

	SDL_WM_SetCaption("foo", 0);
}

static void
release_sdl()
{
	SDL_Quit();
}

void
init_openal()
{
	if (!(al_device = alcOpenDevice(NULL)))
		panic("alcOpenDevice failed");

	if (!(al_context = alcCreateContext(al_device, NULL)))
		panic("alcCreateContext failed");

	alcMakeContextCurrent(al_context);
	alGetError();
}

void
release_openal()
{
	alcMakeContextCurrent(NULL);
	alcDestroyContext(al_context);
	alcCloseDevice(al_device);
}

static font *
load_font(const char *source)
{
	std::ostringstream texture_path;
	texture_path << IMAGE_DIR << '/' << source << ".png";

	std::ostringstream font_path;
	font_path << FONT_DIR << '/' << source << ".fnt";

	return font::load(texture_path.str().c_str(), font_path.str().c_str());
}

static void
init_fonts()
{
	tiny_font = load_font("tiny_font");
	small_font = load_font("small_font");
}

static void
redraw()
{
	glClearColor(0, 0, 0, 0);
	glClear(GL_COLOR_BUFFER_BIT);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, WINDOW_WIDTH, 0, WINDOW_HEIGHT, -1, 1);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	the_game->redraw();

	SDL_GL_SwapBuffers();
}

static void
handle_events()
{
	SDL_Event event;

	while (SDL_PollEvent(&event)) {
		switch (event.type) {
			case SDL_QUIT:
				running = false;
				break;

			case SDL_KEYDOWN:
				the_game->on_key_down(event.key.keysym.sym);
				break;

			case SDL_KEYUP:
				the_game->on_key_up(event.key.keysym.sym);
				break;
		}
	}
}

static void
event_loop()
{
	running = true;

	while (running) {
		int start = SDL_GetTicks();

		redraw();

		the_game->update();
		handle_events();

		int delay = 1000/TICS_PER_SECOND - (SDL_GetTicks() - start);
		if (delay > 0)
			SDL_Delay(delay);
	}
}

void
init()
{
	init_sdl();
	init_openal();
	init_fonts();
	the_game.reset(new game);
}

void
release()
{
	the_game.reset(0);
	release_sdl();
	release_openal();
}

int
main()
{
	init();
	event_loop();
	release();
}
