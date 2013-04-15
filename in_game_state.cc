#include <sstream>

#include <GL/gl.h>

#include "common.h"
#include "gl_util.h"
#include "in_game_state.h"

static const char *STREAM_DIR = "data/streams";

in_game_state::in_game_state(const kashi& cur_kashi)
: cur_kashi(cur_kashi)
, spectrum(player)
, cur_tic(0)
, cur_serifu(cur_kashi.serifu_list.begin())
, cur_serifu_ms(0)
{
	std::ostringstream path;
	path << STREAM_DIR << '/' << cur_kashi.stream;

	player.open(path.str());
	player.start(1);

	spectrum.update(0);
}

in_game_state::~in_game_state()
{ }

void
in_game_state::redraw() const
{
	spectrum.draw();
	draw_serifu();
}

void
in_game_state::update()
{
	++cur_tic;

	player.update();
	spectrum.update(cur_tic);

	if (cur_serifu != cur_kashi.serifu_list.end()) {
		cur_serifu_ms += 1000/TICS_PER_SECOND;

		const int duration = cur_serifu->duration;

		if (cur_serifu_ms >= duration) {
			cur_serifu_ms -= duration;
			++cur_serifu;
		}
	}
}

void
in_game_state::on_key_up(int keysym)
{ }

void
in_game_state::on_key_down(int keysym)
{ }

void
in_game_state::draw_serifu() const
{
	if (cur_serifu == cur_kashi.serifu_list.end())
		return;

	const kashi::serifu& serifu = *cur_serifu;

	const float x0 = 8, x1 = WINDOW_WIDTH - 8, xm = x0 + ((x1 - x0)*cur_serifu_ms/serifu.duration);
	const float y0 = 60, y1 = 65;

	glBegin(GL_QUADS);

	glColor3f(1, 1, 1);
	glVertex2f(x0, y0);
	glVertex2f(x0, y1);
	glVertex2f(xm, y1);
	glVertex2f(xm, y0);

	glColor3f(.5, .5, .5);
	glVertex2f(xm, y0);
	glVertex2f(xm, y1);
	glVertex2f(x1, y1);
	glVertex2f(x1, y0);

	glEnd();

	glColor3f(1, 1, 0);

	static gl_vertex_array_texuv gv(256);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	gv.reset();
	gv.add_string(tiny_font, &serifu.kana[0], 10, 32);

	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, tiny_font->texture_id);

	gv.draw(GL_QUADS);

	gv.reset();
	gv.add_string(small_font, &serifu.kanji[0], 10, 10);

	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, small_font->texture_id);

	gv.draw(GL_QUADS);

	glDisable(GL_TEXTURE_2D);
	glDisable(GL_BLEND);
}
