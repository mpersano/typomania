#include <cstdio>
#include <cstring>
#include <cerrno>

#include <sstream>

#include <sys/types.h>
#include <dirent.h>

#include "panic.h"
#include "in_game_state.h"
#include "game.h"

static const char *KASHI_DIR = "data/lyrics";
static const char *KASHI_EXT = ".kashi";

game::game()
{
	load_song_list();

	if (kashi_list.empty())
		panic("no songs loaded?");

	start_in_game_state(*kashi_list[0]);
}

game::~game()
{
	for (std::vector<kashi *>::iterator i = kashi_list.begin(); i != kashi_list.end(); ++i)
		delete *i;
}

void
game::redraw() const
{
	cur_state->redraw();
}

void
game::update()
{
	cur_state->update();
}

void
game::on_key_down(int keysym)
{
	cur_state->on_key_down(keysym);
}

void
game::on_key_up(int keysym)
{
	cur_state->on_key_up(keysym);
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

			kashi *p = kashi::load(path.str().c_str());
			if (p)
				kashi_list.push_back(p);
		}
	}

	closedir(dir);
}

void
game::start_in_game_state(const kashi& cur_kashi)
{
	cur_state.reset(new in_game_state(cur_kashi));
}
