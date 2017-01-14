#include <cstdio>
#include <cmath>
#include <cassert>
#include <cstring>
#include <cerrno>

#include <sstream>
#include <vector>

#include <SDL.h>

#include <GL/glew.h>

#include <AL/alc.h>
#include <AL/al.h>

#include "panic.h"
#include "render.h"
#include "sfx.h"
#include "common.h"
#include "game.h"

class game_app
{
public:
	game_app(int window_width, int window_height);
	~game_app();

	void event_loop();

private:
	void redraw();
	void handle_events();

	void init_sdl(int window_width, int window_height);
	void release_sdl();

	void init_openal();
	void release_openal();

	bool running_;

	ALCdevice *al_device_;
	ALCcontext *al_context_;

	std::unique_ptr<game> game_;
};

game_app::game_app(int window_width, int window_height)
	: running_ { false }
{
	init_sdl(window_width, window_height);
	init_openal();

	render::init();
	sfx::init();

	game_.reset(new game(window_width, window_height));
}

game_app::~game_app()
{
	game_.reset(nullptr);

	release_openal();
	release_sdl();
}

void
game_app::init_sdl(int window_width, int window_height)
{
	if (SDL_Init(SDL_INIT_VIDEO) < 0)
		panic("SDL_Init: %s", SDL_GetError());

	if (SDL_SetVideoMode(window_width, window_height, 0, SDL_OPENGL) == 0)
		panic("SDL_SetVideoMode: %s", SDL_GetError());

	SDL_WM_SetCaption("typomania", nullptr);

	if (GLenum rv = glewInit())
		panic("glewInit: %s", glewGetErrorString(rv));
}

void
game_app::release_sdl()
{
	SDL_Quit();
}

void
game_app::init_openal()
{
	if (!(al_device_ = alcOpenDevice(nullptr)))
		panic("alcOpenDevice failed");

	if (!(al_context_ = alcCreateContext(al_device_, nullptr)))
		panic("alcCreateContext failed");

	alcMakeContextCurrent(al_context_);
	alGetError();
}

void
game_app::release_openal()
{
	alcMakeContextCurrent(nullptr);
	alcDestroyContext(al_context_);
	alcCloseDevice(al_device_);
}

void
game_app::redraw()
{
	game_->redraw();
	SDL_GL_SwapBuffers();
}

void
game_app::handle_events()
{
	SDL_Event event;

	while (SDL_PollEvent(&event)) {
		switch (event.type) {
			case SDL_QUIT:
				running_ = false;
				break;

			case SDL_KEYDOWN:
				game_->on_key_down(event.key.keysym.sym);
				break;

			case SDL_KEYUP:
				game_->on_key_up(event.key.keysym.sym);
				break;
		}
	}
}

void
game_app::event_loop()
{
	running_ = true;

	while (running_) {
		int start = SDL_GetTicks();

		redraw();

		game_->update();
		handle_events();

		int delay = 1000/TICS_PER_SECOND - (SDL_GetTicks() - start);
		if (delay > 0)
			SDL_Delay(delay);
	}
}

int
main(int argc, char *argv[])
{
	game_app(800, 400).event_loop();
}
