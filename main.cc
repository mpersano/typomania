#include <cstdio>
#include <cmath>
#include <cassert>
#include <cstring>
#include <cerrno>

#include <sstream>
#include <vector>

#include <SDL.h>

#include <GL/gl.h>

#include <AL/alc.h>
#include <AL/al.h>

#include "panic.h"
#include "font.h"
#include "common.h"
#include "game.h"

resource_cache<gl_texture> texture_cache;
resource_cache<font> font_cache;

game_ptr the_game;

static ALCdevice *al_device;
static ALCcontext *al_context;

static bool running;

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
#ifndef MUTE
	init_openal();
#endif
	the_game.reset(new game);
}

void
release()
{
	the_game.reset(0);
	release_sdl();
#ifndef MUTE
	release_openal();
#endif
}

int
main()
{
	init();
	event_loop();
	release();
}
