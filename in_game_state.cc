#include <cassert>

#include <sstream>

#include <algorithm>
#include <list>

#include <SDL.h>
#include <GL/gl.h>

#include "common.h"
#include "pattern.h"
#include "kana.h"
#include "gl_vertex_array.h"
#include "in_game_state.h"

static const char *STREAM_DIR = "data/streams";

enum {
	MISS_SCORE = 601,
	HIT_SCORE = 311,

	FADE_OUT_TICS = 60,
	RESULTS_START_TIC = 120,
};

class glyph_fx {
public:
	glyph_fx(const font *f, wchar_t ch, const vector2& p);

	void update()
	{ ++tics; }

	bool is_active() const
	{ return tics < TTL; }

	void draw() const;

private:
	enum { TTL = 30 };

	const font *f;
	const font::glyph *gi;
	float x, y;

	int tics;
};

glyph_fx::glyph_fx(const font *f, wchar_t ch, const vector2& p)
: f(f)
, gi(f->find_glyph(ch))
, x(p.x + gi->left + .5*gi->width)
, y(p.y + gi->top - .5*gi->height)
, tics(0)
{ }

void
glyph_fx::draw() const
{
	const vector2& t0 = gi->t0;
	const vector2& t1 = gi->t1;
	const vector2& t2 = gi->t2;
	const vector2& t3 = gi->t3;

	const float t = static_cast<float>(tics)/TTL;
	const float s = sin(t*M_PI);

	enum { NUM_LAYERS = 8, };

	static gl_vertex_array_texuv_color gv(4*NUM_LAYERS);
	gv.reset();

	for (int i = 0; i < NUM_LAYERS; i++) {
		const float q = 1. - static_cast<float>(i)/NUM_LAYERS;
		const int c = 255*q*q*(1. - t)*(1. - t);

		const float f = 1 + .2*s*i;

#define ADD_VERTEX(dx, dy, n) \
		gv.add_vertex(x + dx*f*.5*gi->width, y + dy*f*.5*gi->height, t ## n.x, t ## n.y, c, c, c, 255);
		ADD_VERTEX(-1, +1, 0)
		ADD_VERTEX(+1, +1, 1)
		ADD_VERTEX(+1, -1, 2)
		ADD_VERTEX(-1, -1, 3)
#undef ADD_VERTEX
	}

	f->texture.bind();
	gv.draw(GL_QUADS);
}

typedef std::list<glyph_fx *> glyph_fx_queue;
static glyph_fx_queue glyph_fxs;

static void
glyph_fxs_reset()
{
	for (glyph_fx_queue::iterator i = glyph_fxs.begin(); i != glyph_fxs.end(); i++)
		delete *i;

	glyph_fxs.empty();
}

static void
glyph_fxs_draw()
{
	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE, GL_ONE);

	glEnable(GL_TEXTURE_2D);

	for (glyph_fx_queue::const_iterator i = glyph_fxs.begin(); i != glyph_fxs.end(); i++)
		(*i)->draw();
}

static void
glyph_fxs_update()
{
	for (glyph_fx_queue::iterator i = glyph_fxs.begin(); i != glyph_fxs.end(); i++)
		(*i)->update();

	while (!glyph_fxs.empty()) {
		glyph_fx *p = glyph_fxs.front();

		if (p->is_active())
			break;

		delete p;

		glyph_fxs.pop_front();
	}
}

class kana_buffer {
public:
	kana_buffer()
	: cur_pattern(0)
	, num_consumed(0)
	{ }

	~kana_buffer();

	void set_serifu(const serifu *s);

	bool on_key_down(int keysym);

	bool finished() const
	{ return !cur_pattern; }

	int get_num_consumed() const
	{ return prev_num_consumed; }

	serifu_romaji_iterator get_romaji_iterator() const
	{ return serifu_romaji_iterator(cur_pattern, kana_iter); }

private:
	int consume_kana();
	void clear_prev_fx();

	const pattern_node *cur_pattern;
	serifu_kana_iterator kana_iter;

	int num_consumed, prev_num_consumed;

	typedef std::vector<glyph_fx *> fx_cont;
	fx_cont prev_fx;
};

kana_buffer::~kana_buffer()
{
	clear_prev_fx();
}

void
kana_buffer::set_serifu(const serifu *s)
{
	clear_prev_fx();

	kana_iter = serifu_kana_iterator(s);

	prev_num_consumed = 0;
	num_consumed = consume_kana();
}

int
kana_buffer::consume_kana()
{
	glyph_fxs.insert(glyph_fxs.end(), prev_fx.begin(), prev_fx.end());
	prev_fx.clear();

	if (*kana_iter) {
		if ((cur_pattern = kana_to_pattern::find_pair(kana_iter[0], kana_iter[1]))) {
			prev_fx.push_back(new glyph_fx(kana_iter.get_font(), *kana_iter, kana_iter.get_position()));
			++kana_iter;

			prev_fx.push_back(new glyph_fx(kana_iter.get_font(), *kana_iter, kana_iter.get_position()));
			++kana_iter;

			return 2;
		} else if ((cur_pattern = kana_to_pattern::find_single(*kana_iter))) {
			prev_fx.push_back(new glyph_fx(kana_iter.get_font(), *kana_iter, kana_iter.get_position()));
			++kana_iter;

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
	for (fx_cont::iterator i = prev_fx.begin(); i != prev_fx.end(); i++)
		delete *i;
	prev_fx.clear();
}

static kana_buffer input_buffer;

in_game_state::in_game_state(const kashi& cur_kashi)
: cur_kashi(cur_kashi)
, cur_state(PLAYING)
, state_tics(0)
#ifndef MUTE
, spectrum(player, 100, 50, 800, 128, 64)
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
, tiny_font(font_cache["data/fonts/tiny_font.fnt"])
, small_font(font_cache["data/fonts/small_font.fnt"])
, medium_font(font_cache["data/fonts/medium_font.fnt"])
, big_az_font(font_cache["data/fonts/big_az_font.fnt"])
{
	glyph_fxs_reset();

	input_buffer.set_serifu(*cur_serifu);

	std::ostringstream path;
	path << STREAM_DIR << '/' << cur_kashi.stream;

#ifndef MUTE
	player.open(path.str());

	song_duration = static_cast<int>(player.get_track_duration()*1000);

	player.set_gain(.1);
	player.start();

	spectrum.update(0);
#endif

	start_ms = start_serifu_ms = SDL_GetTicks();
}

in_game_state::~in_game_state()
{ }

void
in_game_state::redraw() const
{
	draw_song_info();

	switch (cur_state) {
		case PLAYING:
			draw_hud(1);
			break;

		case OUTRO:
			if (state_tics < FADE_OUT_TICS)
				draw_hud(1. - static_cast<float>(state_tics)/FADE_OUT_TICS);
			else if (state_tics >= RESULTS_START_TIC)
				draw_results(state_tics - RESULTS_START_TIC);
			break;

		default:
			assert(0);
	}
}

void
in_game_state::draw_hud(float alpha) const
{
#ifndef MUTE
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glColor4f(1, 1, 1, .1*alpha);
	spectrum.draw();
#endif

	draw_time_bars(alpha);
	draw_timers(alpha);
	draw_serifu(alpha);

	draw_input_buffer(alpha);
	draw_hud_counters(alpha);
}

void
in_game_state::update()
{
	++state_tics;

	if (cur_state == OUTRO && state_tics == FADE_OUT_TICS) {
		// oh god, this is getting ugly.

		if (cur_serifu != cur_kashi.end()) {
			int n = 0;

			for (serifu_romaji_iterator iter = input_buffer.get_romaji_iterator(); *iter; ++iter)
				++n;

			for (++cur_serifu; cur_serifu != cur_kashi.end(); ++cur_serifu) {
				for (serifu_romaji_iterator iter(*cur_serifu); *iter; ++iter)
					++n;
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
			const unsigned duration = (*cur_serifu)->duration;

			if (serifu_ms >= duration) {
				int n = 0;

				for (serifu_romaji_iterator iter = input_buffer.get_romaji_iterator(); *iter; ++iter)
					++n;

				if (n > 0) {
					miss += n;
					total_strokes += n;
					score -= MISS_SCORE*n;
					combo = 0;
				}

				start_serifu_ms += duration;
				serifu_ms -= duration;

				if (++cur_serifu == cur_kashi.end())
					set_state(OUTRO);
				else
					input_buffer.set_serifu(*cur_serifu);
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

	glyph_fxs_update();
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
		} else if (!input_buffer.finished()) {
			++total_strokes;

			if (!input_buffer.on_key_down(keysym)) {
				score -= MISS_SCORE;
				combo = 0;
				++miss;
			} else {
				score += HIT_SCORE;
				if (++combo > max_combo)
					max_combo = combo;
			}
		}
	} else if (cur_state == OUTRO) {
		// XXX
		the_game->start_song_menu();
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
draw_integer_r(gl_vertex_array_texuv& gv, const font *f, float x, float y, float w, bool zero_padded, int num_digits, int value)
{
	gv.add_glyph(f->find_glyph(value%10 + L'0'), x, y);

	if (value/10 || (zero_padded && num_digits > 1))
		return draw_integer_r(gv, f, x - w, y, w, zero_padded, num_digits - 1, value/10);
	else
		return x - w;
}

static void
draw_integer(const font *f, float base_x, float base_y, bool zero_padded, int num_digits, int value)
{
	static gl_vertex_array_texuv gv(256);
	gv.reset();

	const font::glyph *g = f->find_glyph(L'0');
	base_y += .5*g->height - g->top;

	const float x = draw_integer_r(gv, f, base_x, base_y, g->advance_x, zero_padded, num_digits, abs(value));

	if (value < 0)
		gv.add_glyph(f->find_glyph(L'-'), x, base_y);

	f->texture.bind();
	gv.draw(GL_QUADS);
}

static void
draw_string(const font *f, float x, float y, const wchar_t *str)
{
	static gl_vertex_array_texuv gv(256);
	gv.reset();

	const font::glyph *g = f->find_glyph(L'X');
	gv.add_string(f, str, x, y + .5*g->height - g->top);

	f->texture.bind();
	gv.draw(GL_QUADS);
}

void
in_game_state::draw_time_bar(float y, const wchar_t *label, int partial, int total, float alpha) const
{
	const float x = 120;
	const float w = WINDOW_WIDTH - x - 8;
	const float h = 5;

	const float x0 = x, x1 = x + w, xm = x0 + (x1 - x0)*partial/total;
	const float y0 = y - .5*h, y1 = y + .5*h;

	glColor4f(1, 1, 1, alpha);
	glEnable(GL_TEXTURE_2D);
	draw_string(tiny_font, x - tiny_font->get_string_width(label) - 8, y, label);

	glDisable(GL_TEXTURE_2D);

	glBegin(GL_QUADS);

	glColor4f(1, 1, 1, alpha);
	glVertex2f(x0, y0);
	glVertex2f(x0, y1);
	glVertex2f(xm, y1);
	glVertex2f(xm, y0);

	glColor4f(.5, .5, .5, alpha);
	glVertex2f(xm, y0);
	glVertex2f(xm, y1);
	glVertex2f(x1, y1);
	glVertex2f(x1, y0);

	glEnd();
}

void
in_game_state::draw_time_bars(float alpha) const
{
	if (cur_serifu == cur_kashi.end())
		return;

	draw_time_bar(170, L"INTERVAL", serifu_ms, (*cur_serifu)->duration, alpha);
#ifndef MUTE
	draw_time_bar(190, L"TOTAL TIME", total_ms, song_duration, alpha);
#endif
}

void
in_game_state::draw_timers(float alpha) const
{
	static gl_vertex_array_texuv gv(11*4);
	gv.reset();

	const font *f = tiny_font;
	const int dx = f->find_glyph(L'0')->advance_x;

	float x = WINDOW_WIDTH - 12 - 11*dx;
	float y = 204;

#define DRAW_GLYPH(c) \
	gv.add_glyph(f->find_glyph(c), x, y); \
	x += dx; \

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

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glEnable(GL_TEXTURE_2D);

	glColor4f(1, 1, 1, alpha);

	f->texture.bind();
	gv.draw(GL_QUADS);
}

void
in_game_state::draw_serifu(float alpha) const
{
	if (cur_serifu == cur_kashi.end())
		return;

	const serifu *serifu = 0;
	int highlighted;

	if (!input_buffer.finished()) {
		serifu = *cur_serifu;
		highlighted = input_buffer.get_num_consumed();
	} else {
		kashi::const_iterator next_serifu = cur_serifu + 1;

		if (next_serifu != cur_kashi.end()) {
			serifu = *next_serifu;
			alpha *= .5;
			highlighted = 0;
		}
	}

	const float base_x = 100;
	const float base_y = 70;

	if (serifu) {
		const rgba color[2] = { rgba(0, 1, 1, alpha), rgba(1, 1, 1, alpha) };

		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		glEnable(GL_TEXTURE_2D);

		glPushMatrix();
		glTranslatef(base_x, base_y, 0);
		serifu->draw(highlighted, color);
		glPopMatrix();
	}

	glPushMatrix();
	glTranslatef(base_x, base_y, 0);
	glyph_fxs_draw();
	glPopMatrix();
}

void
in_game_state::draw_input_buffer(float alpha) const
{
	const float base_y = 30;
	const float base_x = 20;

	const font::glyph *big_glyph = big_az_font->find_glyph(L'X');
	const float big_y = base_y + .5*big_glyph->height - big_glyph->top;

	const font::glyph *small_glyph = small_font->find_glyph(L'X');
	const float small_y = base_y + .5*small_glyph->height - small_glyph->top;

	static gl_vertex_array_texuv gv(256);

	glColor4f(1, 1, 1, alpha);

	bool is_first = true;

	float x = base_x;

	for (serifu_romaji_iterator iter = input_buffer.get_romaji_iterator(); *iter; ++iter) {
		const int ch = *iter;

		if (is_first) {
			gv.reset();
			gv.add_glyph(big_az_font->find_glyph(ch), x, big_y);
			big_az_font->texture.bind();
			gv.draw(GL_QUADS);

			x += big_glyph->advance_x;

			is_first = false;

			gv.reset();
		} else {
			const font::glyph *g = small_font->find_glyph(ch);
			gv.add_glyph(small_font->find_glyph(ch), x, small_y);

			x += g->advance_x;
		}
	}

	small_font->texture.bind();
	gv.draw(GL_QUADS);
}

float 
in_game_state::draw_hud_counter(float x, float y, const wchar_t *label, bool zero_padded, int num_digits, int value) const
{
	draw_string(tiny_font, x, y, label);
	x += tiny_font->get_string_width(label) + small_font->find_glyph(L'0')->advance_x*num_digits;
	draw_integer(small_font, x, y, zero_padded, num_digits, value);

	return x;
}

float 
in_game_state::draw_hud_counter(float x, float y, const wchar_t *label, const wchar_t *value) const
{
	draw_string(tiny_font, x, y, label);
	x += tiny_font->get_string_width(label) + small_font->find_glyph(L'0')->advance_x;
	draw_string(small_font, x, y, value);

	return x;
}

void
in_game_state::draw_hud_counters(float alpha) const
{
	const float base_y = 140;

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glColor4f(1, 1, 1, alpha);

	if (combo) {
		draw_integer(small_font, 40, base_y, false, 0, combo);
		draw_string(tiny_font, 60, base_y, L"COMBO");
	}

	float x = 140;
	x = draw_hud_counter(x, base_y, L"SCORE", false, 7, display_score) + 50;
	x = draw_hud_counter(x, base_y, L"MAX COMBO", true, 3, max_combo) + 50;
	x = draw_hud_counter(x, base_y, L"MISS", true, 3, miss) + 50;
	draw_hud_counter(x, base_y, L"CORRECT", get_correct_percent());
}

const wchar_t *
in_game_state::get_correct_percent() const
{
	static wchar_t buf[30] = {0};

	if (total_strokes > 0) {
		if (miss == 0) {
			wcscpy(buf, L"100%");
		} else if (miss == total_strokes) {
			wcscpy(buf, L"0%");
		} else {
			const float f = 100.*static_cast<float>(total_strokes - miss)/total_strokes;
			swprintf(buf, sizeof buf/sizeof *buf, L"%.1f%%", f);
		}
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
in_game_state::draw_song_info() const
{
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glEnable(GL_TEXTURE_2D);

	glColor4f(1, 1, 1, 1);

	draw_string(medium_font, 30, 320, &cur_kashi.name[0]);
	draw_string(tiny_font, 36, 290, &cur_kashi.genre[0]);
	draw_string(tiny_font, 36, 346, &cur_kashi.artist[0]);
}

void
in_game_state::draw_results(int tic) const
{
	enum {
		LINE_FADE_IN_TIC = 20,
		LINE_INTERVAL = 40,
	};

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	const int digit_width = small_font->find_glyph(L'0')->advance_x;

	float base_x = WINDOW_WIDTH/2;
	float base_y = 320;

#define DRAW_LABEL(f, str) \
	{ \
	glColor4f(1, 1, 1, LINE_FADE_IN_TIC ? static_cast<float>(tic)/LINE_FADE_IN_TIC : 1); \
	static gl_vertex_array_texuv gv(80); \
	gv.reset(); \
	const font::glyph *g = f->find_glyph(L'X'); \
	gv.add_string(f, str, base_x - f->get_string_width(str), base_y + .5*g->height - g->top); \
	f->texture.bind(); \
	gv.draw(GL_QUADS); \
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

	glColor4f(1, 1, 1, LINE_FADE_IN_TIC ? static_cast<float>(tic)/LINE_FADE_IN_TIC : 1);

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
