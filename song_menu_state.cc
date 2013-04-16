#include <SDL.h>
#include <GL/gl.h>

#include "common.h"
#include "kashi.h"
#include "gl_util.h"
#include "song_menu_state.h"

enum {
	MOVE_TICS = 20
};

song_menu_state::song_menu_state(const kashi_cont& kashi_list)
: kashi_list(kashi_list)
, cur_state(STATE_IDLE)
, cur_selection(0)
{ }

song_menu_state::~song_menu_state()
{ }

void
song_menu_state::redraw() const
{
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glEnable(GL_TEXTURE_2D);

	const float ITEM_INTERVAL = .1;

	float item_t = -ITEM_INTERVAL*cur_selection;

	if (cur_state == STATE_MOVING_UP || cur_state == STATE_MOVING_DOWN) {
		const float t = static_cast<float>(state_tics)/MOVE_TICS;
		const float dt = cur_state == STATE_MOVING_UP ? 1 : -1;

		item_t += t*dt*ITEM_INTERVAL;
	}

	for (kashi_cont::const_iterator i = kashi_list.begin(); i != kashi_list.end(); i++) {
		const float x = 300 + item_t*item_t*800;
		const float y = .5*WINDOW_HEIGHT - item_t*800;

		const float s = 1. - 5.*item_t*item_t;

		glPushMatrix();
		glTranslatef(x, y, 0);
		glScalef(s, s, 1);

		draw_song_title(*i);

		glPopMatrix();

		item_t += ITEM_INTERVAL;
	}
}

void
song_menu_state::update()
{
	++state_tics;

	switch (cur_state) {
		case STATE_MOVING_UP:
			if (state_tics == MOVE_TICS) {
				--cur_selection;
				cur_state = STATE_IDLE;
			}
			break;

		case STATE_MOVING_DOWN:
			if (state_tics == MOVE_TICS) {
				++cur_selection;
				cur_state = STATE_IDLE;
			}
			break;

		default:
			break;
	}
}

void
song_menu_state::on_key_up(int keysym)
{
}

void
song_menu_state::on_key_down(int keysym)
{
	switch (keysym) {
		case SDLK_UP:
			if (cur_state == STATE_IDLE) {
				cur_state = STATE_MOVING_UP;
				state_tics = 0;
			}
			break;

		case SDLK_DOWN:
			if (cur_state == STATE_IDLE) {
				cur_state = STATE_MOVING_DOWN;
				state_tics = 0;
			}
			break;

		default:
			break;
	}
}

void
song_menu_state::draw_song_title(const kashi *p) const
{
	static gl_vertex_array_texuv gv(256);

	gv.reset();
	gv.add_string(tiny_font, &p->artist[0], 1, 24);
	gv.add_string(tiny_font, &p->genre[0], 1, -20);
	glBindTexture(GL_TEXTURE_2D, tiny_font->texture_id);
	gv.draw(GL_QUADS);

	gv.reset();
	gv.add_string(small_font, &p->name[0], 0, 0);
	glBindTexture(GL_TEXTURE_2D, small_font->texture_id);
	gv.draw(GL_QUADS);
}
