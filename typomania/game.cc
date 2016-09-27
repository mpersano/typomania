#include <cstdio>
#include <cstring>
#include <cerrno>

#include <sstream>
#include <algorithm>

#include <sys/types.h>
#include <dirent.h>

#include "panic.h"
#include "song_menu_state.h"
#include "game.h"

static const char *KASHI_DIR = "data/lyrics";
static const char *KASHI_EXT = ".kashi";

game::game()
{
	load_song_list();

	if (kashi_list.empty())
		panic("no songs loaded?");

	push_state(new song_menu_state(kashi_list));
}

game_state *
game::cur_state()
{
	return state_stack.top().get();
}

const game_state *
game::cur_state() const
{
	return state_stack.top().get();
}

void
game::redraw() const
{
	cur_state()->redraw();
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
				kashi_list.push_back(std::move(p));
		}
	}

	closedir(dir);

	std::sort(kashi_list.begin(), kashi_list.end(),
			[](const kashi_ptr& a, const kashi_ptr& b)
			{
				return a->level < b->level;
			});
}

void
game::push_state(game_state *new_state)
{
	state_stack.push(std::unique_ptr<game_state>(new_state));
}

void
game::pop_state()
{
	state_stack.pop();
}
