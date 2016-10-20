#include <cstdio>
#include <cstring>
#include <cerrno>

#include <GL/glew.h>

#include <sstream>
#include <algorithm>

#include <sys/types.h>
#include <dirent.h>

#include "gl_check.h"
#include "panic.h"
#include "render.h"
#include "common.h"
#include "in_game_state.h"
#include "song_menu_state.h"
#include "game.h"

static const char *KASHI_DIR = "data/lyrics";
static const char *KASHI_EXT = ".kashi";

game_state::game_state(game *parent)
	: parent_ { parent }
{
}

game::game(int window_width, int window_height)
	: window_width_ { window_width }
	, window_height_ { window_height }
{
	load_song_list();

	if (kashi_list_.empty())
		panic("no songs loaded?");

	push_state(new song_menu_state(this, kashi_list_));
}

game_state *
game::cur_state()
{
	return state_stack_.top().get();
}

const game_state *
game::cur_state() const
{
	return state_stack_.top().get();
}

void
game::redraw() const
{
	GL_CHECK(glViewport(0, 0, window_width_, window_height_));

	GL_CHECK(glClearColor(0, 0, 0, 0));
	GL_CHECK(glClear(GL_COLOR_BUFFER_BIT));

	render::set_viewport(0, window_width_, 0, window_height_);

	render::begin_batch();
	cur_state()->redraw();
	render::end_batch();
}

void
game::update()
{
	cur_state()->update();
}

void
game::on_key_down(int keysym)
{
	cur_state()->on_key_down(keysym);
}

void
game::on_key_up(int keysym)
{
	cur_state()->on_key_up(keysym);
}

void
game::load_song_list()
{
	DIR *dir;

	if (!(dir = opendir(KASHI_DIR)))
		panic("failed to open %s: %s", KASHI_DIR, strerror(errno));

	struct dirent *de;

	while ((de = readdir(dir))) {
		const char *name = de->d_name;
		const size_t len = strlen(name);

		if (len >= strlen(KASHI_EXT) && !strcmp(name + len - strlen(KASHI_EXT), KASHI_EXT)) {
			std::ostringstream path;
			path << KASHI_DIR << '/' << name;

			fprintf(stderr, "loading %s\n", path.str().c_str());

			kashi_ptr p(new kashi);

			if (p->load(path.str().c_str()))
				kashi_list_.push_back(std::move(p));
		}
	}

	closedir(dir);

	std::sort(std::begin(kashi_list_), std::end(kashi_list_),
			[](const kashi_ptr& a, const kashi_ptr& b)
			{
				return a->level < b->level;
			});
}

void
game::push_state(game_state *new_state)
{
	state_stack_.push(std::unique_ptr<game_state>(new_state));
}

void
game::pop_state()
{
	state_stack_.pop();
}

void game::enter_in_game_state(const kashi& cur_kashi)
{
	push_state(new in_game_state(this, cur_kashi));
}

void game::leave_state()
{
	pop_state();
}
