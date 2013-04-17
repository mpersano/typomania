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
	return vector2(300. + 8.*t*t, .5*WINDOW_HEIGHT - 100.*t/(1. + .1*fabs(t)));
}

static float
get_item_scale(const float t)
{
	return 1.2 - .07*t*t;
}

struct menu_item {
	menu_item(const kashi *song);

	void render() const;

	gl_vertex_array_texuv gv_artist;
	gl_vertex_array_texuv gv_name;

	const kashi *song;
};

menu_item::menu_item(const kashi *song)
: gv_artist(256)
, gv_name(256)
, song(song)
{
	const font::glyph *gi = small_font->find_glyph(L'X');

	const float height = gi->height;
	const float top = gi->top;

	const float y_offset = .5*height - top;

	// TODO: replace this 24/16 crap with combination of height/top
	gv_artist.add_string(tiny_font, &song->artist[0], 0, y_offset + 24);
	gv_artist.add_string(tiny_font, &song->genre[0], 0, y_offset - 16);

	gv_name.add_string(small_font, &song->name[0], 0, y_offset);
}

void
menu_item::render() const
{
	glEnable(GL_TEXTURE_2D);

	glBindTexture(GL_TEXTURE_2D, tiny_font->texture_id);
	gv_artist.draw(GL_QUADS);

	glBindTexture(GL_TEXTURE_2D, small_font->texture_id);
	gv_name.draw(GL_QUADS);
}

song_menu_state::song_menu_state(const kashi_cont& kashi_list)
: cur_state(STATE_IDLE)
, cur_selection(0)
{
	for (kashi_cont::const_iterator i = kashi_list.begin(); i != kashi_list.end(); i++)
		item_list.push_back(new menu_item(*i));
}

song_menu_state::~song_menu_state()
{
	for (item_cont::iterator i = item_list.begin(); i != item_list.end(); i++)
		delete *i;
}

void
song_menu_state::redraw() const
{
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	float item_t = -cur_selection;

	if (cur_state == STATE_MOVING_UP || cur_state == STATE_MOVING_DOWN) {
		const float f = static_cast<float>(state_tics)/MOVE_TICS;
		const float dt = 1. - powf(1. - f, 3);
		const float dir = cur_state == STATE_MOVING_UP ? 1 : -1;

		item_t += dt*dir;
	}

	// TODO: do the coloring in menu_item
	glColor3f(1, 1, 1);

	for (item_cont::const_iterator i = item_list.begin(); i != item_list.end(); i++) {
		// TODO: move positioning/scaling/border drawing to menu_item

		const vector2 p = get_item_position(item_t);

		const float s = get_item_scale(item_t);

		const vector2 p0 = get_item_position(item_t - .5);
		const vector2 p1 = get_item_position(item_t + .5);

		const float y0 = p0.y - 2;
		const float y1 = p1.y + 2;

		const float y = .5*(y0 + y1);

		glPushMatrix();

		glTranslatef(p.x, y, 0);
		glScalef(s, s, 1);
		(*i)->render();
		glPopMatrix();

		// TODO: textured border

		glDisable(GL_TEXTURE_2D);

		glBegin(GL_LINES);

		glVertex2f(p.x, y0);
		glVertex2f(WINDOW_WIDTH, y0);

		glVertex2f(p.x, y0);
		glVertex2f(p.x, y1);

		glVertex2f(p.x, y1);
		glVertex2f(WINDOW_WIDTH, y1);

		glEnd();

		++item_t;
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
			if (cur_state == STATE_IDLE && cur_selection < static_cast<int>(item_list.size()) - 1) {
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
