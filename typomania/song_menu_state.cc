#include <SDL.h>

#include "resources.h"
#include "render.h"
#include "rgba.h"
#include "kashi.h"
#include "font.h"
#include "song_menu_state.h"

namespace {

static const int START_MOVE_TICS = 40;
static const int FAST_MOVE_TICS = 10;
static const int ARROW_ANIMATION_TICS = 20;

};

class menu_item
{
public:
	menu_item(int window_width, int window_height, const kashi *song);

	void render(float pos) const;

	const kashi *get_song() const
	{ return song_; }

private:
	vec2f get_position(const float t) const;
	float get_scale(float t) const;
	rgba get_color(float t) const;

	int window_width_;
	int window_height_;

	const kashi *song_;

	const font *small_font_;
	const font *tiny_font_;

	const gl_texture *border_texture_;
};

menu_item::menu_item(int window_width, int window_height, const kashi *song)
	: window_width_(window_width)
	, window_height_(window_height)
	, song_(song)
	, small_font_(get_font("data/fonts/small_font.fnt"))
	, tiny_font_(get_font("data/fonts/tiny_font.fnt"))
	, border_texture_(get_texture("data/images/item-border.png"))
{
}

void
menu_item::render(float pos) const
{
	const vec2f p = get_position(pos);

	const float s = get_scale(pos);

	const vec2f p0 = get_position(pos - .5);
	const vec2f p1 = get_position(pos + .5);

	const float y0 = p0.y;
	const float y1 = p1.y;

	const float y = .5*(y0 + y1);

	const float height = y0 - y1;

	// draw border

	rgba bg_color = get_color(pos);

	render::set_color(bg_color);

	render::add_quad(
			border_texture_,
			{ { p.x, y0 }, { p.x, y1 }, { p.x + height, y0 }, { p.x + height, y1 } },
			{ { 0, 0 }, { 0, 1 }, { .9, 0 }, { .9, 1 }  },
			-10);

	render::add_quad(
		border_texture_,
		{ { p.x + height, y0 }, { p.x + height, y1 }, { window_width_, y0 }, { window_width_, y1 } },
		{ { .9, 0 }, { .9, 1 }, { .95, 0 }, { .95, 1 } },
		-10);

	// draw text

	render::set_color({ 1, 1, 1, bg_color.a });

	render::push_matrix();

	render::translate(p.x, y);
	render::scale(s, s);

	const float base_x = 8;

	const font::glyph *small_glyph = small_font_->find_glyph(L'X');
	const float small_height = small_glyph->height;
	const float small_top = small_glyph->top;
	const float small_width = small_glyph->width;

	const float y_offset = .5*small_height - small_top;

	const font::glyph *tiny_glyph = tiny_font_->find_glyph(L'X');
	const float tiny_height = tiny_glyph->height;
	const float tiny_top = tiny_glyph->top;

	// TODO: check these coords!
	small_font_->draw_glyph(L'0' + song_->level/10, base_x, y_offset, 0);
	small_font_->draw_glyph(L'0' + song_->level%10, base_x + small_width, y_offset, 0);

	const float x_offset = base_x + 2.5*small_width;

	tiny_font_->draw_string(&song_->artist[0], x_offset + 2, y_offset + .5*small_height + (tiny_height - tiny_top) + 4, 0);
	tiny_font_->draw_string(&song_->genre[0], x_offset + 2, y_offset - .5*small_height - 4, 0);

	small_font_->draw_string(&song_->name[0], x_offset, y_offset, 0);

	render::pop_matrix();
}

vec2f
menu_item::get_position(const float t) const
{
	const float x = 500. + 15.*t*t;
	const float y = .5*window_height_ - 100.*t/(1. + .1*fabs(t));

	const float k = .8, w = 100.;
	const float x_offset = fabs(t) < k ? -w*(.5 + .5*cos((t / k)*M_PI)) : 0;

	return { x + x_offset, y };
}

float
menu_item::get_scale(const float t) const
{
	return 1.2 - .07*t*t;
}

rgba
menu_item::get_color(const float t) const
{
	const rgba from(.3, .6, 1, .5), to(1, 1, 1, 1);

	const float k = 1.;
	const float f = fabs(t) < k ? .5 + .5*cos((t/k)*M_PI) : 0;

	return from + f*(to - from);
}

song_menu_state::song_menu_state(game *parent, const std::vector<kashi_ptr>& kashi_list)
	: game_state(parent)
	, cur_state_(state::IDLE)
	, state_tics_(0)
	, cur_selection_(0)
	, cur_displayed_position_(0)
	, arrow_texture_(get_texture("data/images/arrow.png"))
	, bg_texture_(get_texture("data/images/menu-background.png"))
{
	for (auto& p : kashi_list)
		item_list_.emplace_back(new menu_item(parent_->get_window_width(), parent_->get_window_height(), p.get()));
}

song_menu_state::~song_menu_state()
{
}

void
song_menu_state::draw_background() const
{
	render::set_blend_mode(blend_mode::NO_BLEND);
	render::set_color({ 1, 1, 1, 1 });

	const int w = bg_texture_->get_image_width();
	const int h = bg_texture_->get_image_height();

	const float u = static_cast<float>(bg_texture_->get_image_width())/bg_texture_->get_texture_width();
	const float v = static_cast<float>(bg_texture_->get_image_height())/bg_texture_->get_texture_height();

	render::add_quad(
			bg_texture_,
			{ { 0, 0 }, { 0, h }, { w, 0 }, { w, h } },
			{ { 0, v }, { 0, 0 }, { u, v }, { u, 0 } },
			-20);
}

void
song_menu_state::redraw() const
{
	draw_background();

	render::set_blend_mode(blend_mode::ALPHA_BLEND);

	int from = std::max<int>(cur_displayed_position_ - /* 1.5 */ 2, 0);
	int to = std::min<int>(cur_displayed_position_ + /* 2.5 */ 3, item_list_.size() - 1);

	float pos = -cur_displayed_position_ + from;

	for (int i = from; i <= to; i++) {
		item_list_[i]->render(pos);
		++pos;
	}

	if (cur_state_ == state::IDLE) {
		const float f = static_cast<float>(state_tics_ % ARROW_ANIMATION_TICS)/ARROW_ANIMATION_TICS;

		const float t = 1. - (1. - f)*(1. - f);

		const float w = arrow_texture_->get_texture_width();
		const float h = arrow_texture_->get_texture_height();

		const float x = 370 + t*20 - .5*w;
		const float y = .5*parent_->get_window_height();

		render::set_color({ 1, 1, 1, t });

		render::add_quad(
			arrow_texture_,
			{ { x, y - .5*h }, { x, y + .5*h }, { x + w, y - .5*h }, { x + w, y + .5*h } },
			{ { 0, 0 }, { 0, 1 }, { 1, 0 }, { 1, 1 } },
			0);
	}
}

void
song_menu_state::update()
{
	static const float EPSILON = 1e-3;

	cur_displayed_position_ += .1*(cur_selection_ - cur_displayed_position_);
	if (fabs(cur_displayed_position_ - cur_selection_) < EPSILON)
		cur_displayed_position_ = cur_selection_;

	++state_tics_;

	switch (cur_state_) {
		case state::MOVING_UP:
			if (state_tics_ == move_tics_) {
				if (cur_selection_ > 0) {
					--cur_selection_;
					move_tics_ = FAST_MOVE_TICS;
					set_cur_state(state::MOVING_UP);
				} else {
					set_cur_state(state::IDLE);
				}
			}
			break;

		case state::MOVING_DOWN:
			if (state_tics_ == move_tics_) {
				if (cur_selection_ < static_cast<int>(item_list_.size()) - 1) {
					++cur_selection_;
					move_tics_ = FAST_MOVE_TICS;
					set_cur_state(state::MOVING_DOWN);
				} else {
					set_cur_state(state::IDLE);
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
	set_cur_state(state::IDLE);
}

void
song_menu_state::on_key_down(int keysym)
{
	switch (keysym) {
		case SDLK_UP:
			if (cur_selection_ > 0) {
				--cur_selection_;
				move_tics_ = START_MOVE_TICS;
				set_cur_state(state::MOVING_UP);
			}
			break;

		case SDLK_DOWN:
			if (cur_selection_ < static_cast<int>(item_list_.size()) - 1) {
				++cur_selection_;
				move_tics_ = START_MOVE_TICS;
				set_cur_state(state::MOVING_DOWN);
			}
			break;

		case SDLK_RETURN:
			if (cur_state_ == state::IDLE)
				parent_->enter_in_game_state(*item_list_[cur_selection_]->get_song());
			break;

		default:
			break;
	}
}

void
song_menu_state::set_cur_state(state s)
{
	cur_state_ = s;
	state_tics_ = 0;
}
