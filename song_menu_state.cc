#include <SDL.h>
#include <GL/gl.h>

#include "common.h"
#include "kashi.h"
#include "gl_vertex_array.h"
#include "song_menu_state.h"

enum {
	MOVE_TICS = 20
};

static vector2
get_item_position(const float t)
{
	return vector2(300. + 15.*t*t, .5*WINDOW_HEIGHT - 100.*t/(1. + .1*fabs(t)));
}

static float
get_item_scale(const float t)
{
	return 1.2 - .07*t*t;
}

struct menu_item {
	menu_item(const kashi *song);

	void render(float pos) const;

	gl_vertex_array_texuv gv_artist;
	gl_vertex_array_texuv gv_name;
	gl_vertex_array_texuv gv_level;

	const kashi *song;

	font *small_font;
	font *tiny_font;

	gl_texture *border_texture;
};

menu_item::menu_item(const kashi *song)
: gv_artist(256)
, gv_name(256)
, gv_level(8)
, song(song)
, small_font(font_cache["data/fonts/small_font.fnt"])
, tiny_font(font_cache["data/fonts/tiny_font.fnt"])
, border_texture(texture_cache["data/images/item-border.png"])
{
	const float base_x = 8;

	const font::glyph *small_glyph = small_font->find_glyph(L'X');
	const float small_height = small_glyph->height;
	const float small_top = small_glyph->top;
	const float small_width = small_glyph->width;

	const float y_offset = .5*small_height - small_top;

	const font::glyph *tiny_glyph = tiny_font->find_glyph(L'X');
	const float tiny_height = tiny_glyph->height;
	const float tiny_top = tiny_glyph->top;

	// TODO: check these coords!
	gv_level.add_glyph(small_font->find_glyph(L'0' + song->level/10), base_x, y_offset);
	gv_level.add_glyph(small_font->find_glyph(L'0' + song->level%10), base_x + small_width, y_offset);

	const float x_offset = base_x + 2.5*small_width;

	gv_artist.add_string(tiny_font, &song->artist[0], x_offset + 2, y_offset + .5*small_height + (tiny_height - tiny_top) + 4);
	gv_artist.add_string(tiny_font, &song->genre[0], x_offset + 2, y_offset - .5*small_height - 4);

	gv_name.add_string(small_font, &song->name[0], x_offset, y_offset);
}

void
menu_item::render(float pos) const
{
	const vector2 p = get_item_position(pos);

	const float s = get_item_scale(pos);

	const vector2 p0 = get_item_position(pos - .5);
	const vector2 p1 = get_item_position(pos + .5);

	const float y0 = p0.y - 1;
	const float y1 = p1.y + 1;

	const float y = .5*(y0 + y1);

	const float height = y0 - y1;

	// draw border

	glColor3f(.3, .3, .6);

	glEnable(GL_TEXTURE_2D);

	gl_vertex_array_texuv gv_border(8);

	gv_border.add_vertex(p.x, y0, 0, 0);
	gv_border.add_vertex(p.x + height, y0, .9, 0);
	gv_border.add_vertex(p.x + height, y1, .9, 1);
	gv_border.add_vertex(p.x, y1, 0, 1);

	gv_border.add_vertex(p.x + height, y0, .9, 0);
	gv_border.add_vertex(WINDOW_WIDTH, y0, .95, 0);
	gv_border.add_vertex(WINDOW_WIDTH, y1, .95, 1);
	gv_border.add_vertex(p.x + height, y1, .9, 1);

	border_texture->bind();
	gv_border.draw(GL_QUADS);

	// draw text

	glColor3f(1, 1, 1);

	glPushMatrix();

	glTranslatef(p.x, y, 0);
	glScalef(s, s, 1);

	tiny_font->texture->bind();
	gv_artist.draw(GL_QUADS);

	small_font->texture->bind();
	gv_name.draw(GL_QUADS);
	gv_level.draw(GL_QUADS);

	glPopMatrix();
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

	float pos = -cur_selection;

	if (cur_state == STATE_MOVING_UP || cur_state == STATE_MOVING_DOWN) {
		const float f = static_cast<float>(state_tics)/MOVE_TICS;
		const float offs = 1. - powf(1. - f, 3);
		const float dir = cur_state == STATE_MOVING_UP ? 1 : -1;

		pos += offs*dir;
	}

	for (item_cont::const_iterator i = item_list.begin(); i != item_list.end(); i++) {
		(*i)->render(pos);
		++pos;
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

		case SDLK_RETURN:
			if (cur_state == STATE_IDLE)
				the_game->start_in_game(*item_list[cur_selection]->song);
			break;

		default:
			break;
	}
}
