#include <SDL.h>
#include <GL/gl.h>

#include "common.h"
#include "kashi.h"
#include "gl_util.h"
#include "song_menu_state.h"

enum {
	MOVE_TICS = 20
};

static vector2
get_item_position(const float t)
{
	return vector2(300 + t*t*800, .5*WINDOW_HEIGHT - 1000.*t/(1. + fabs(t)));
}

static float
get_item_scale(const float t)
{
	return 1.2 - 7.*t*t;
}

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

	const float ITEM_INTERVAL = .1;

	float item_t = -ITEM_INTERVAL*cur_selection;

	if (cur_state == STATE_MOVING_UP || cur_state == STATE_MOVING_DOWN) {
		const float f = static_cast<float>(state_tics)/MOVE_TICS;
		const float t = 1. - powf(1. - f, 3);
		const float dt = cur_state == STATE_MOVING_UP ? 1 : -1;

		item_t += t*dt*ITEM_INTERVAL;
	}

	const font::glyph *gi = small_font->find_glyph(L'X');
	const float y_offset = .5*gi->height - gi->top;

	glColor3f(1, 1, 1);

	for (kashi_cont::const_iterator i = kashi_list.begin(); i != kashi_list.end(); i++) {
		const vector2 p = get_item_position(item_t);
		const float s = get_item_scale(item_t);

		glPushMatrix();

		glTranslatef(p.x, p.y + s*y_offset, 0);
		glScalef(s, s, 1);
		draw_song_title(*i);
		glPopMatrix();

		glDisable(GL_TEXTURE_2D);

		const vector2 p0 = get_item_position(item_t - .5*ITEM_INTERVAL);
		const vector2 p1 = get_item_position(item_t + .5*ITEM_INTERVAL);

		const float y0 = p0.y - 2;
		const float y1 = p1.y + 2;

		glBegin(GL_LINES);

		glVertex2f(p.x, y0);
		glVertex2f(WINDOW_WIDTH, y0);

		glVertex2f(p.x, y0);
		glVertex2f(p.x, y1);

		glVertex2f(p.x, y1);
		glVertex2f(WINDOW_WIDTH, y1);

		glEnd();

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
			if (cur_state == STATE_IDLE && cur_selection > 0) {
				cur_state = STATE_MOVING_UP;
				state_tics = 0;
			}
			break;

		case SDLK_DOWN:
			if (cur_state == STATE_IDLE && cur_selection < static_cast<int>(kashi_list.size()) - 1) {
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

	glEnable(GL_TEXTURE_2D);

	gv.reset();
	gv.add_string(tiny_font, &p->artist[0], 0, 24);
	gv.add_string(tiny_font, &p->genre[0], 0, -16);
	glBindTexture(GL_TEXTURE_2D, tiny_font->texture_id);
	gv.draw(GL_QUADS);

	gv.reset();
	gv.add_string(small_font, &p->name[0], 0, 0);
	glBindTexture(GL_TEXTURE_2D, small_font->texture_id);
	gv.draw(GL_QUADS);
}
