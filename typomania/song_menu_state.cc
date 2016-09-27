#include <SDL.h>
#include <GL/gl.h>

#include "common.h"
#include "rgba.h"
#include "kashi.h"
#include "gl_vertex_array.h"
#include "in_game_state.h"
#include "song_menu_state.h"

enum {
	START_MOVE_TICS = 40,
	FAST_MOVE_TICS = 10,
	ARROW_ANIMATION_TICS = 20,
};

static vector2
get_item_position(const float t)
{
	const float x = 500. + 15.*t*t;
	const float y = .5*WINDOW_HEIGHT - 100.*t/(1. + .1*fabs(t));

	const float k = .8, w = 100.;
	const float x_offset = fabs(t) < k ? -w*(.5 + .5*cos((t / k)*M_PI)) : 0;

	return vector2(x + x_offset, y);
}

static float
get_item_scale(const float t)
{
	return 1.2 - .07*t*t;
}

static rgba
get_item_color(const float t)
{
	const rgba from(.3, .6, 1, .5), to(1, 1, 1, 1);

	const float k = 1.;
	const float f = fabs(t) < k ? .5 + .5*cos((t/k)*M_PI) : 0;

	return from + f*(to - from);
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

	const float y0 = p0.y;
	const float y1 = p1.y;

	const float y = .5*(y0 + y1);

	const float height = y0 - y1;

	// draw border

	// glColor3f(.3, .3, .6);

	rgba bg_color = get_item_color(pos);
	glColor4f(bg_color.r, bg_color.g, bg_color.b, bg_color.a);

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

	glColor4f(1, 1, 1, bg_color.a);

	glPushMatrix();

	glTranslatef(p.x, y, 0);
	glScalef(s, s, 1);

	tiny_font->texture.bind();
	gv_artist.draw(GL_QUADS);

	small_font->texture.bind();
	gv_name.draw(GL_QUADS);
	gv_level.draw(GL_QUADS);

	glPopMatrix();
}

song_menu_state::song_menu_state(const std::vector<kashi_ptr>& kashi_list)
: cur_state(STATE_IDLE)
, state_tics(0)
, cur_selection(0)
, cur_displayed_position(0)
, arrow_texture(texture_cache["data/images/arrow.png"])
, bg_texture(texture_cache["data/images/menu-background.png"])
{
	for (auto& p : kashi_list)
		item_list.emplace_back(new menu_item(p.get()));
}

song_menu_state::~song_menu_state()
{
}

void
song_menu_state::draw_background() const
{
	const int w = bg_texture->get_image_width();
	const int h = bg_texture->get_image_height();

	const float u = static_cast<float>(bg_texture->get_image_width())/bg_texture->get_texture_width();
	const float v = static_cast<float>(bg_texture->get_image_height())/bg_texture->get_texture_height();

	gl_vertex_array_texuv gv(4);
	gv.add_vertex(0, 0, 0, v);
	gv.add_vertex(w, 0, u, v);
	gv.add_vertex(w, h, u, 0);
	gv.add_vertex(0, h, 0, 0);

	glColor4f(1, 1, 1, 1);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glEnable(GL_TEXTURE_2D);
	bg_texture->bind();

	gv.draw(GL_QUADS);
}

void
song_menu_state::redraw() const
{
	draw_background();

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

#if 1
	int from = std::max<int>(cur_displayed_position - /* 1.5 */ 2, 0);
	int to = std::min<int>(cur_displayed_position + /* 2.5 */ 3, item_list.size() - 1);

	float pos = -cur_displayed_position + from;

	for (int i = from; i <= to; i++) {
		item_list[i]->render(pos);
		++pos;
	}
#else
	float pos = -cur_displayed_position;
	 
	for (item_cont::const_iterator i = item_list.begin(); i != item_list.end(); i++) {
		(*i)->render(pos);
		++pos;
	}
#endif

	if (cur_state == STATE_IDLE) {
		const float f = static_cast<float>(state_tics % ARROW_ANIMATION_TICS)/ARROW_ANIMATION_TICS;

		const float t = 1. - (1. - f)*(1. - f);

		const float w = arrow_texture->get_texture_width();
		const float h = arrow_texture->get_texture_height();

		const float x = 370 + t*20 - .5*w;
		const float y = .5*WINDOW_HEIGHT;

		gl_vertex_array_texuv gv(4);
		gv.add_vertex(x, y - .5*h, 0, 0);
		gv.add_vertex(x + w, y - .5*h, 1, 0);
		gv.add_vertex(x + w, y + .5*h, 1, 1);
		gv.add_vertex(x, y + .5*h, 0, 1);

		glColor4f(1, 1, 1, t);

		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		glEnable(GL_TEXTURE_2D);
		arrow_texture->bind();

		gv.draw(GL_QUADS);
	}
}

void
song_menu_state::update()
{
	static const float EPSILON = 1e-3;

	cur_displayed_position += .1*(cur_selection - cur_displayed_position);
	if (fabs(cur_displayed_position - cur_selection) < EPSILON)
		cur_displayed_position = cur_selection;

	++state_tics;

	switch (cur_state) {
		case STATE_MOVING_UP:
			if (state_tics == move_tics) {
				if (cur_selection > 0) {
					--cur_selection;
					move_tics = FAST_MOVE_TICS;
					set_cur_state(STATE_MOVING_UP);
				} else {
					set_cur_state(STATE_IDLE);
				}
			}
			break;

		case STATE_MOVING_DOWN:
			if (state_tics == move_tics) {
				if (cur_selection < static_cast<int>(item_list.size()) - 1) {
					++cur_selection;
					move_tics = FAST_MOVE_TICS;
					set_cur_state(STATE_MOVING_DOWN);
				} else {
					set_cur_state(STATE_IDLE);
				}
			}
			break;

		default:
			break;
	}
}

void
song_menu_state::on_key_up(int keysym)
{
	set_cur_state(STATE_IDLE);
}

void
song_menu_state::on_key_down(int keysym)
{
	switch (keysym) {
		case SDLK_UP:
			if (cur_selection > 0) {
				--cur_selection;
				move_tics = START_MOVE_TICS;
				set_cur_state(STATE_MOVING_UP);
			}
			break;

		case SDLK_DOWN:
			if (cur_selection < static_cast<int>(item_list.size()) - 1) {
				++cur_selection;
				move_tics = START_MOVE_TICS;
				set_cur_state(STATE_MOVING_DOWN);
			}
			break;

		case SDLK_RETURN:
			if (cur_state == STATE_IDLE)
				the_game->push_state(new in_game_state(*item_list[cur_selection]->song));
			break;

		default:
			break;
	}
}

void
song_menu_state::set_cur_state(state s)
{
	cur_state = s;
	state_tics = 0;
}
