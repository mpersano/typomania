#include <cassert>

#include <sstream>

#include <algorithm>

#include <SDL.h>

#include "resources.h"
#include "render.h"
#include "gl_check.h"
#include "gl_framebuffer.h"
#include "pattern.h"
#include "kana.h"
#include "glyph_fx.h"
#include "in_game_state.h"

#ifdef WIN32
#define swprintf _snwprintf
#endif

static const char *STREAM_DIR = "data/streams";

enum {
	MISS_SCORE = 601,
	HIT_SCORE = 311,

	FADE_IN_TICS = 60,
	FADE_OUT_TICS = 60,
	RESULTS_START_TIC = 120,

	COMBO_BUMP_TICS = 20,
};

class kana_buffer
{
public:
	kana_buffer(in_game_state *parent)
	: parent_(parent)
	, cur_pattern(nullptr)
	, num_consumed(0)
	{ }

	void set_serifu(const serifu *s);

	bool on_key_down(int keysym);

	bool finished() const
	{ return !cur_pattern; }

	int get_num_consumed() const
	{ return prev_num_consumed; }

	serifu::romaji_iterator get_romaji_iterator() const
	{ return serifu::romaji_iterator(kana_iter_, cur_pattern); }

	using fx_cont = std::vector<std::unique_ptr<glyph_fx>>;

	const fx_cont& get_prev_fx() const
	{ return prev_fx; }

private:
	int consume_kana();
	void clear_prev_fx();

	in_game_state *parent_;
	const pattern_node *cur_pattern;
	serifu::kana_iterator kana_iter_, kana_end_;

	int num_consumed, prev_num_consumed;

	fx_cont prev_fx;
};

void
kana_buffer::set_serifu(const serifu *s)
{
	clear_prev_fx();

	cur_pattern = nullptr;

	kana_iter_ = s->kana_begin();
	kana_end_ = s->kana_end();

	prev_num_consumed = 0;
	num_consumed = consume_kana();
}

int
kana_buffer::consume_kana()
{
	for (auto& p : prev_fx)
		parent_->add_glyph_fx(std::move(p));
	prev_fx.clear();

	if (kana_iter_ != kana_end_) {
		if ((cur_pattern = kana::find_pattern(kana_iter_[0], kana_iter_[1], kana_iter_[2]))) {
			kana_iter_.get_glyph_fx(prev_fx);
			++kana_iter_;

			kana_iter_.get_glyph_fx(prev_fx);
			++kana_iter_;

			kana_iter_.get_glyph_fx(prev_fx);
			++kana_iter_;

			return 3;
		} else if ((cur_pattern = kana::find_pattern(kana_iter_[0], kana_iter_[1]))) {
			kana_iter_.get_glyph_fx(prev_fx);
			++kana_iter_;

			kana_iter_.get_glyph_fx(prev_fx);
			++kana_iter_;

			return 2;
		} else if ((cur_pattern = kana::find_pattern(*kana_iter_))) {
			kana_iter_.get_glyph_fx(prev_fx);
			++kana_iter_;

			return 1;
		}
	}

	return 0;
}

bool
kana_buffer::on_key_down(int keysym)
{
	assert(cur_pattern);

	if (keysym >= 'a' && keysym <= 'z')
		keysym += 'A' - 'a';

	if (!cur_pattern->match(keysym)) {
		if (!cur_pattern->is_optional) {
			return false;
		} else {
			// failed to match but was optional, so try next one

			if (cur_pattern->next && cur_pattern->next->match(keysym)) {
				cur_pattern = cur_pattern->next;
			} else {
				return false;
			}
		}
	}

	if (!(cur_pattern = cur_pattern->next)) {
		prev_num_consumed = num_consumed;
		num_consumed += consume_kana();
	}

	return true;
}

void
kana_buffer::clear_prev_fx()
{
	prev_fx.clear();
}

in_game_state::in_game_state(game *parent, const kashi& cur_kashi)
: game_state(parent)
, cur_kashi(cur_kashi)
, cur_state(INTRO)
, state_tics(0)
#ifndef MUTE
, spectrum(player, 16, 200, 48)
#endif
, cur_serifu(cur_kashi.begin())
, total_ms(0)
, serifu_ms(0)
, score(0)
, display_score(0)
, combo(0)
, max_combo(0)
, miss(0)
, total_strokes(0)
, hit_tics_(0)
, tiny_font(get_font("data/fonts/tiny_font.fnt"))
, small_font(get_font("data/fonts/small_font.fnt"))
, medium_font(get_font("data/fonts/medium_font.fnt"))
, big_az_font(get_font("data/fonts/big_az_font.fnt"))
, bg_overlay_texture_(get_texture("data/images/bg-overlay.png"))
, input_buffer_( new kana_buffer(this))
, blur_program_(get_program("data/shaders/blur.prog"))
{
	const int w = parent_->get_window_width();
	const int h = parent_->get_window_height();

	glow_framebuffers_[0].reset(new gl::framebuffer(w/2, h/2));
	glow_framebuffers_[1].reset(new gl::framebuffer(w/2, h/2));

	std::ostringstream path;
	path << STREAM_DIR << '/' << cur_kashi.stream;

#ifndef MUTE
	player.open(path.str());

	song_duration = static_cast<int>(player.get_track_duration()*1000);

	player.set_gain(1.);
#endif

	set_cur_serifu(cur_serifu->get(), cur_serifu + 1 == cur_kashi.end());
}

in_game_state::~in_game_state()
{ }

void
in_game_state::redraw() const
{
	float alpha;

	if (cur_state == INTRO)
		alpha = static_cast<float>(state_tics)/FADE_IN_TICS;
	else
		alpha = 1;

	draw_background(alpha);

	bind_glow_layer();
	draw_hud(true);
	draw_glow_layer();

	draw_hud(false);
}

void
in_game_state::draw_hud(bool glow_layer) const
{
	render::set_blend_mode(blend_mode::ALPHA_BLEND);

	float alpha;

	switch (cur_state) {
		case INTRO:
			alpha = static_cast<float>(state_tics)/FADE_IN_TICS;
			break;

		case OUTRO:
			alpha = 1. - static_cast<float>(state_tics)/FADE_OUT_TICS;
			break;

		default:
			alpha = 1;
			break;
	}

	render::set_color({ 1, 1, 1, cur_state == INTRO ? alpha : 1 });
	draw_song_info();

	if (cur_state != OUTRO || state_tics < FADE_OUT_TICS) {
		draw_time_bars(alpha, glow_layer);

		render::set_color({ 1, 1, 1, alpha });
		draw_hud_counters(glow_layer);

		if (!glow_layer) {
			draw_timers();
			draw_input_buffer();

			draw_serifu(alpha);

#ifndef MUTE
			render::push_matrix();
			render::translate(22, 60);
			render::set_blend_mode(blend_mode::ALPHA_BLEND);
			render::set_color({ 1, 1, 1, .1f*alpha });
			spectrum.draw();
			render::pop_matrix();
#endif
		}
	}

	if (cur_state == OUTRO && state_tics >= RESULTS_START_TIC) {
		draw_results(state_tics - RESULTS_START_TIC, glow_layer);
	}
}

void
in_game_state::update()
{
	++state_tics;

	if (hit_tics_ > 0)
		--hit_tics_;

	if (cur_state == INTRO) {
		if (state_tics == FADE_IN_TICS) {
			player.start();
			spectrum.update(0);
			start_ms = start_serifu_ms = SDL_GetTicks();
			set_state(PLAYING);
		}

		return;
	}

	if (cur_state == OUTRO && state_tics == FADE_OUT_TICS) {
		if (cur_serifu != cur_kashi.end()) {
			int n = std::distance(input_buffer_->get_romaji_iterator(), (*cur_serifu)->romaji_end());

			for (++cur_serifu; cur_serifu != cur_kashi.end(); ++cur_serifu) {
				n += std::distance((*cur_serifu)->romaji_begin(), (*cur_serifu)->romaji_end());
			}

			miss += n;
			total_strokes += n;
			score -= MISS_SCORE*n;
		}
	}

	unsigned now = SDL_GetTicks();

	total_ms = now - start_ms;

	if (cur_serifu != cur_kashi.end()) {
		serifu_ms = now - start_serifu_ms;

		if (cur_state == PLAYING) {
			if (serifu_ms >= cur_serifu_duration) {
				int n = std::distance(input_buffer_->get_romaji_iterator(), (*cur_serifu)->romaji_end());

				if (n > 0) {
					miss += n;
					total_strokes += n;
					score -= MISS_SCORE*n;
					combo = 0;
				}

				start_serifu_ms += cur_serifu_duration;
				serifu_ms -= cur_serifu_duration;

				if (++cur_serifu == cur_kashi.end()) {
					set_state(OUTRO);
				} else {
					set_cur_serifu(cur_serifu->get(), cur_serifu + 1 == cur_kashi.end());
				}
			}
		}
	}

	display_score += (score - display_score)/2;
	if (abs(display_score - score) == 1)
		display_score = score;

#ifndef MUTE
	player.update();
	spectrum.update(total_ms);
#endif

	update_glyph_fxs();
}

void
in_game_state::on_key_up(int keysym)
{ }

void
in_game_state::on_key_down(int keysym)
{
	if (cur_state == PLAYING) {
		if (keysym == SDLK_ESCAPE) {
			set_state(OUTRO);
#ifndef MUTE
			player.fade_out(FADE_OUT_TICS);
#endif
		} else if (!input_buffer_->finished()) {
			++total_strokes;

			if (!input_buffer_->on_key_down(keysym)) {
				score -= MISS_SCORE;
				combo = 0;
				++miss;
			} else {
				score += HIT_SCORE;
				if (++combo > max_combo)
					max_combo = combo;
				hit_tics_ = COMBO_BUMP_TICS;
			}
		}
	} else if (cur_state == OUTRO) {
		if (keysym == SDLK_ESCAPE || keysym == SDLK_SPACE)
			parent_->leave_state();
	}
}

static int
get_num_digits(int n)
{
	int num_digits = 0;

	if (n < 0) {
		++num_digits;
		n = -n;
	}

	while (n) {
		n /= 10;
		++num_digits;
	}

	return num_digits ? num_digits : 1;
}

static float
draw_integer_r(const font *f, float x, float y, float w, bool zero_padded, int num_digits, int value)
{
	f->draw_glyph(value%10 + L'0', x, y, 0);

	if (value/10 || (zero_padded && num_digits > 1))
		return draw_integer_r(f, x - w, y, w, zero_padded, num_digits - 1, value/10);
	else
		return x - w;
}

static void
draw_integer(const font *f, float base_x, float base_y, bool zero_padded, int num_digits, int value)
{
	const font::glyph *g = f->find_glyph(L'0');
	base_y += .5*g->height - g->top;

	const float x = draw_integer_r(f, base_x, base_y, g->advance_x, zero_padded, num_digits, abs(value));

	if (value < 0)
		f->draw_glyph(L'-', x, base_y, 0);
}

static void
draw_string(const font *f, float x, float y, const wchar_t *str)
{
	const font::glyph *g = f->find_glyph(L'X');
	f->draw_string(str, x, y + .5*g->height - g->top,0);
}

void
in_game_state::draw_time_bar(float y, const wchar_t *label, int partial, int total, float alpha, bool glow_layer) const
{
	const float x = 120;
	const float w = parent_->get_window_width() - x - 8;
	const float h = 5;

	const float x0 = x, x1 = x + w, xm = x0 + (x1 - x0)*partial/total;
	const float y0 = y - .5*h, y1 = y + .5*h;

	render::set_color({ 1, 1, 1, alpha });
	render::draw_quad({ { x0, y0 }, { x0, y1 }, { xm, y0 }, { xm, y1 } }, 0);

	if (!glow_layer) {
		draw_string(tiny_font, x - tiny_font->get_string_width(label) - 8, y, label);

		render::set_color({ 1, 1, 1, .25f*alpha });
		render::draw_quad({ { xm, y0 }, { xm, y1 }, { x1, y0 }, { x1, y1 } }, 0);
	}
}

void
in_game_state::draw_time_bars(float alpha, bool glow_layer) const
{
	if (cur_serifu == cur_kashi.end())
		return;

	draw_time_bar(170, L"INTERVAL", serifu_ms, cur_serifu_duration, alpha, glow_layer);
#ifndef MUTE
	draw_time_bar(190, L"TOTAL TIME", total_ms, song_duration, alpha, glow_layer);
#endif
}

void
in_game_state::draw_timers() const
{
	const font *f = tiny_font;
	const int dx = f->find_glyph(L'0')->advance_x;

	float x = parent_->get_window_width() - 12 - 11*dx;
	float y = 204;

#define DRAW_GLYPH(c) \
	f->draw_glyph(c, x, y, 0); \
	x += dx;

#define DRAW_PART(v) \
	DRAW_GLYPH(L'0' + (v)/10) \
	DRAW_GLYPH(L'0' + (v)%10)

#define DRAW_TIMER(ms) \
	DRAW_PART(((ms/1000)/60)%100) \
	DRAW_GLYPH(L':') \
	DRAW_PART((ms/1000)%60) \

	DRAW_TIMER(total_ms)
	DRAW_GLYPH(L'/')
	DRAW_TIMER(song_duration)

#undef DRAW_PART
#undef DRAW_GLYPH
}

void
in_game_state::draw_serifu(float alpha) const
{
	if (cur_serifu == cur_kashi.end())
		return;

	const serifu *serifu = nullptr;
	int highlighted;

	if (!input_buffer_->finished()) {
		serifu = cur_serifu->get();
		highlighted = input_buffer_->get_num_consumed();
	} else {
		kashi::const_iterator next_serifu = cur_serifu + 1;

		if (next_serifu != cur_kashi.end()) {
			serifu = next_serifu->get();
			alpha *= .5;
			highlighted = 0;
		}
	}

	const float base_x = 20;
	const float base_y = 70;

	if (serifu) {
		const rgba color[2] = { rgba(0, 1, 1, alpha), rgba(1, 1, 1, alpha) };

		render::set_blend_mode(blend_mode::ALPHA_BLEND);

		render::push_matrix();
		render::translate(base_x, base_y);

		serifu->draw(highlighted, color);

		render::pop_matrix();
	}

	render::push_matrix();
	render::translate(base_x, base_y);

	draw_glyph_fxs();

	render::pop_matrix();
}

void
in_game_state::draw_input_buffer() const
{
	const float base_y = 30;
	const float base_x = 20;

	const font::glyph *big_glyph = big_az_font->find_glyph(L'X');
	const float big_y = base_y + .5*big_glyph->height - big_glyph->top;

	const font::glyph *small_glyph = small_font->find_glyph(L'X');
	const float small_y = base_y + .5*small_glyph->height - small_glyph->top;

	bool is_first = true;

	float x = base_x;

	for (auto iter = input_buffer_->get_romaji_iterator(); *iter; ++iter) {
		const int ch = *iter;

		if (is_first) {
			big_az_font->draw_glyph(ch, x, big_y, 0);
			x += big_glyph->advance_x;
			is_first = false;
		} else {
			const font::glyph *g = small_font->find_glyph(ch);
			small_font->draw_glyph(ch, x, small_y, 0);
			x += g->advance_x;
		}
	}
}

void
in_game_state::draw_hud_counters(bool glow_layer) const
{
	const int base_y = 140;

	if (combo > 1) {
		render::push_matrix();
		render::translate(50, base_y);

		draw_string(tiny_font, 10, 0, L"COMBO");

		if (hit_tics_ > 0) {
			float t = static_cast<float>(hit_tics_)/COMBO_BUMP_TICS;
			render::scale(1. + .5*t*t);
		}

		draw_integer(small_font, -10, 0, false, 0, combo);

		render::pop_matrix();
	}

	float base_x = 140;

	if (!glow_layer) {
		draw_string(tiny_font, 140, base_y, L"SCORE");

		draw_string(tiny_font, 316, base_y, L"MAX COMBO");
		draw_integer(small_font, 418, base_y, false, 3, max_combo);

		draw_string(tiny_font, 520, base_y, L"MISS");
		draw_integer(small_font, 587, base_y, false, 3, miss);

		draw_string(tiny_font, 637, base_y, L"CORRECT");
		draw_string(small_font, 699, base_y, get_correct_percent());
	}

	draw_integer(small_font, 266, base_y, false, 7, display_score);
}

const wchar_t *
in_game_state::get_correct_percent() const
{
	static wchar_t buf[30] = {0};

	if (total_strokes == 0) {
		*buf = '\0';
	} else if (miss == 0) {
		wcscpy(buf, L"100%");
	} else if (miss == total_strokes) {
		wcscpy(buf, L"0%");
	} else {
		const float f = 100.*static_cast<float>(total_strokes - miss)/total_strokes;
		swprintf(buf, sizeof buf/sizeof *buf, L"%.1f%%", f);
	}

	return buf;
}

const wchar_t *
in_game_state::get_class() const
{
	assert(total_strokes > 0);

	const float f = static_cast<float>(total_strokes - miss)/total_strokes;

	if (f < .1)
		return L"F";
	else if (f < .3)
		return L"E";
	else if (f < .5)
		return L"D";
	else if (f < .7)
		return L"C";
	else if (f < .8)
		return L"B";
	else if (f < .9)
		return L"A";
	else if (f < .95)
		return L"AA";
	else if (f < 1)
		return L"AAA";
	else
		return L"S";
}

void
in_game_state::draw_background(float alpha) const
{
	if (cur_kashi.background) {
		render::set_color({ 1, 1, 1, alpha });

		render::set_blend_mode(blend_mode::NO_BLEND);
		render::draw_quad(cur_kashi.background, { 0, 0 }, -30);

		render::set_blend_mode(blend_mode::ALPHA_BLEND);
		render::draw_quad(bg_overlay_texture_, { 0, 0 }, -25);
	}
}

void
in_game_state::bind_glow_layer() const
{
	const int width = parent_->get_window_width();
	const int height = parent_->get_window_height();

	render::end_batch();

	auto fb_texture = glow_framebuffers_[0]->get_texture();

	const int fb_width = fb_texture->get_texture_width();
	const int fb_height = fb_texture->get_texture_height();

	glow_framebuffers_[0]->bind();

	render::set_viewport(0, width, height, 0);
	render::begin_batch();

	render::set_blend_mode(blend_mode::NO_BLEND);
	render::set_color({ 0, 0, 0, 0 });
	render::draw_quad({ { 0, 0 }, { 0, height }, { width, 0 }, { width, height } }, -1);
}

void
in_game_state::draw_glow_layer() const
{
	const int width = parent_->get_window_width();
	const int height = parent_->get_window_height();

	render::end_batch();

	// blur horizontally from fb0 to fb1

	auto fb_texture = glow_framebuffers_[0]->get_texture();
	const int fb_width = fb_texture->get_texture_width();
	const int fb_height = fb_texture->get_texture_height();

	glow_framebuffers_[1]->bind();

	render::begin_batch();
	render::set_color({ 0, 1.f/fb_height, 0, 0 });
	render::draw_quad(blur_program_, glow_framebuffers_[0]->get_texture(), { { 0, 0 }, { 0, height }, { width, 0 }, { width, height } }, 0);
	render::end_batch();

	// blur vertically from fb1 to fb0

	glow_framebuffers_[0]->bind();

	render::begin_batch();
	render::set_color({ 1.f/fb_width, 0, 0, 0 });
	render::draw_quad(blur_program_, glow_framebuffers_[1]->get_texture(), { { 0, 0 }, { 0, height }, { width, 0 }, { width, height } }, 0);
	render::end_batch();

	// fb0 to screen

	// HACK!
	glow_framebuffers_[0]->unbind();
	GL_CHECK(glViewport(0, 0, width, height));

	render::set_viewport(0, width, 0, height);

	// render the result

	render::begin_batch();
	render::set_blend_mode(blend_mode::ADDITIVE_BLEND);
	render::set_color({ 1, .61f, 0, 1 });
	render::draw_quad(glow_framebuffers_[0]->get_texture(), { { 0, 0 }, { 0, height }, { width, 0 }, { width, height } }, -20);
}

void
in_game_state::draw_song_info() const
{
	draw_string(medium_font, 30, 322, &cur_kashi.name[0]);
	draw_string(tiny_font, 36, 290, &cur_kashi.genre[0]);
	draw_string(tiny_font, 36, 346, &cur_kashi.artist[0]);
}

void
in_game_state::draw_results(int tic, bool glow_layer) const
{
	enum {
		LINE_FADE_IN_TIC = 20,
		LINE_INTERVAL = 40,
	};

	render::set_blend_mode(blend_mode::ALPHA_BLEND);

	const int digit_width = small_font->find_glyph(L'0')->advance_x;

	float base_x = parent_->get_window_width()/2;
	float base_y = 320;

#define DRAW_LABEL(f, str) \
	if (!glow_layer) { \
	render::set_color({ 1, 1, 1, std::min(static_cast<float>(tic)/LINE_FADE_IN_TIC, 1.f) }); \
	const font::glyph *g = f->find_glyph(L'X'); \
	f->draw_string(str, base_x - f->get_string_width(str), base_y + .5*g->height - g->top, 0); \
	}

#define NEXT_Y \
	if ((tic -= LINE_INTERVAL) < 0) \
		return; \
	base_y -= 30; \

#define DRAW_LINE(label, value) \
	DRAW_LABEL(tiny_font, label) \
	draw_integer(small_font, base_x + digit_width*(get_num_digits(value) + 1), base_y, false, 0, value); \
	NEXT_Y

	DRAW_LINE(L"MISS", miss)

	DRAW_LABEL(tiny_font, L"CORRECT")
	draw_string(small_font, base_x + 2*digit_width, base_y, get_correct_percent());
	NEXT_Y

	DRAW_LINE(L"MAX COMBO", max_combo)
	DRAW_LINE(L"SCORE", score)
#undef DRAW_LINE
#undef NEXT_Y

	base_y -= 30;

	render::set_color({ 1, 1, 1, LINE_FADE_IN_TIC ? static_cast<float>(tic)/LINE_FADE_IN_TIC : 1 });

	DRAW_LABEL(small_font, L"CLASS")
	draw_string(big_az_font, base_x + 2*digit_width, base_y, get_class());
#undef DRAW_LABEL
}

void
in_game_state::set_state(state next_state)
{
	cur_state = next_state;
	state_tics = 0;
}

void
in_game_state::set_cur_serifu(const serifu *s, bool is_last)
{
	input_buffer_->set_serifu(cur_serifu->get());

	cur_serifu_duration = (*cur_serifu)->duration();
#ifndef MUTE
	if (is_last || cur_serifu_duration > song_duration - total_ms)
		cur_serifu_duration = song_duration - total_ms;
#endif
}

void
in_game_state::draw_glyph_fxs() const
{
	render::set_blend_mode(blend_mode::ADDITIVE_BLEND);

	for (auto& p : glyph_fxs_)
		p->draw();
}

void
in_game_state::update_glyph_fxs()
{
	for (auto& p : glyph_fxs_)
		p->update();

	while (!glyph_fxs_.empty()) {
		auto& p = glyph_fxs_.front();

		if (p->is_active())
			break;

		glyph_fxs_.pop_front();
	}
}

void
in_game_state::add_glyph_fx(std::unique_ptr<glyph_fx> p)
{
	glyph_fxs_.push_back(std::move(p));
}
